//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "core/realComp.h"
#include "math/mMatrix.h"
#include "console/console.h"
#include "game/gameBase.h"
#include "ai/aiConnection.h"
#include "ai/aiStep.h"
#include "ai/aiNavStep.h"
#include "ai/aiTask.h"
#include "ai/graphMath.h"
#include "terrain/waterBlock.h"
#include "scenegraph/sceneGraph.h"
#include "game/vehicle.h"
#include "platform/profiler.h"

IMPLEMENT_CONOBJECT(AIConnection);

static S32 gAIDetectionOffset = 0;

AIConnection::AIConnection()
{
   mAIControlled = true;
   mMoveMode = mMoveModePending = ModeStop;
   mMoveLocation.set(0, 0, 0);
   mNodeLocation.set(0, 0, 0);
   mAimLocation.set(0, 0, 0);
   mMoveSpeed = 0.0f;
   mMoveTolerance = 0.25f;
   mStep = NULL;
   mCurrentTask = NULL;
   mCurrentTaskTime = 0;
   mPathDest.set(0, 0, 0);
   mNewPath = false;

   //clear the triggers
   for (int i = 0; i < MaxTriggerKeys; i++)
      mTriggers[i] = false;

   mPrevNodeLocation.set(0, 0, 0);
   mInitialLocation.set(0, 0, 0);
   
   mEnergyReserve = 0.0f;
   mEnergyFloat = 0.10f;
   mEnergyRecharge = false;
   
   //init engage vars
   mEngageState = ChooseWeapon;
   mProjectileName = StringTable->insert("");
   mProjectile = NULL;
   mEngageMinDistance = 20;
   mEngageMaxDistance = 75;
   mDistToTarg2D = -1.0f;
   mLookAtTargetTimeMS = 0;
   mFiring = false;
   mTriggerCounter = 0;
   mScriptTriggerCounter = -1;
   mVictimTime = 0;
   mSkillLevel = 0.5;
   mChangeWeaponCounter = 0;
   mTurretMountedId = 0;
   mMountedImage = NULL;

   //piloting vars
   mPilotDestination.set(0, 0, 0);
   mPilotAimLocation.set(0, 0, 0);
   mPilotSpeed = 1.0f;
   mPitchUpMax = -0.25;
   mPitchDownMax = 0.1f;
   mPitchIncMax = 0.05f;
   mCurrentPitch = 0;
   mPreviousPitch = 0;
   mDesiredPitch = 0;
   mPitchIncrement = 0;

   mTargStillTimeMS = 0;
   mTargPrevTimeMS = 0;
   mTargPrevLocation[0].set(0, 0, 0);
   mTargPrevLocation[1].set(0, 0, 0);
   mTargPrevLocation[2].set(0, 0, 0);
   mTargPrevLocation[3].set(0, 0, 0);

   mPlayerDetectionIndex = 0;
   mPlayerDetectionCounter = gAIDetectionOffset;
   gAIDetectionOffset++;
   if (gAIDetectionOffset >= 3)
      gAIDetectionOffset = 0;
   mDetectHiddenPeriod = 6000;
   mBlindedTimer = 0;

   //init target object vars
   mTargetObject = NULL;
   mObjectMode = DestroyObject;
   mDistToObject2D = -1.0f;
   
   //these are used for both mEngageTarget and mTargetObject
   mRangeToTarget = 30;
   mCheckTargetLOSCounter = 0;
   mTargetInRange = false;
   mTargetInSight = false;
   mWeaponEnergy = 0.0f;
   mWeaponErrorFactor = 1.0f;
   
   //init evading vars
   mEnemyProjectile = NULL;
   mEvadingCounter = 0;
   mIsEvading = false;

   mAvoidingObject = NULL;
   mStuckInitialized = false;
   
   // LH.  Just for the heck of it (and as way to familiarize with code) - added in all
   // these that weren't being constructed.  
   mMoveDestination.set(0, 0, 0);
   mEvadeLocation.set(0, 0, 0);
   mImpactLocation.set(0, 0, 0);
   mCorpseLocation.set(0, 0, 0);
   mStateCounter = 0;
   mDelayCounter = 0;
   mPackCheckCounter = 0;
   mAimAtLazedTarget = false;
   mProjectileCounter = 0;
   mPath.setTeam(getSensorGroup());
   mStuckDestination.set(0,0,0);
   mTargetPlayer = NULL;
   mLocation.set(0,0,0);
   mVelocity = mVelocity2D = mRotation = mHeadRotation = 
   mMuzzlePosition = mEyePosition = mTargLocation = 
   mTargVelocity = mTargVelocity2D = mTargRotation = mLocation;
   mTargEnergy = 1.0;
   mTargDamage = 0;
}

AIConnection::~AIConnection()
{
}

void AIConnection::setSkillLevel(F32 level)
{
   mSkillLevel = getMax(0.0f, getMin(1.0f, level));
}

void AIConnection::setMoveSpeed(F32 speed)
{
   if (speed <= 0.0f)
      mMoveSpeed = 0.0f;
   else
      mMoveSpeed = getMin(1.0f, speed);
}

void AIConnection::setMoveMode(S32 mode, bool abortStuckCode)
{
   if (mode < 0 || mode >= ModeCount)
      mode = 0;

   //if we're setting mode::express and we're currently stuck, let the stuck code
   //finish "unsticking" the bot...  calling setMoveDestination will abort the stuck code
   if (mMoveMode == ModeStuck && mode != ModeStop && !abortStuckCode)
      return;

   //make sure we're not moving if in the middle of a jet...
   mMoveModePending = mode;
   if (!mNavUsingJet)
      mMoveMode = mode;
}

void AIConnection::setMoveTolerance(F32 tolerance)
{
   mMoveTolerance = getMax(0.1f, tolerance);
}

void AIConnection::setMoveDestination(const Point3F &location)
{
   if (mMoveDestination != location && mMoveMode == ModeStuck)
   {
      setMoveMode(ModeExpress, true);
      mStuckLocation.set(0, 0, 0);
   }
   mMoveDestination = location;
}

void AIConnection::setMoveLocation(const Point3F &location)
{
   mMoveLocation = location;
}

void AIConnection::setAimLocation(const Point3F &location)
{
   mAimLocation = location;
}

bool AIConnection::setScriptAimLocation(const Point3F &location, S32 duration)
{
   //can't set through scripts if the bots are aiming at either an object, or an engage player
   if (mTargetPlayer || bool(mTargetObject))
      return false;

   setAimLocation(location);
   mLookAtTargetTimeMS = getMax(mLookAtTargetTimeMS, (S32)Sim::getCurrentTime() + duration);

   return true;
}

void AIConnection::setPilotPitchRange(F32 pitchUpMax, F32 pitchDownMax, F32 pitchIncMax)
{
   mPitchUpMax = pitchUpMax;
   mPitchDownMax = pitchDownMax;
   mPitchIncMax = pitchIncMax;
}

void AIConnection::setPilotDestination(const Point3F &dest, F32 maxSpeed)
{
   //must always aim where you're flying to
   mPilotDestination = dest;
   mPilotAimLocation = dest;
   mPilotSpeed = getMax(0.0f, getMin(1.0f, maxSpeed));
}

void AIConnection::setPilotAimLocation(const Point3F &aimLocation)
{
   //if you're aiming, you can't move
   mPilotAimLocation = aimLocation;
   mPilotSpeed = 0;
}

void AIConnection::setWeaponInfo(const char *projectile, S32 minDist, S32 maxDist, S32 triggerCount, F32 energyRequired, F32 errorFactor)
{
   //set the non-pointer params
   mEngageMinDistance = minDist;
   mEngageMaxDistance = maxDist;
   mTriggerCounter = triggerCount;
   mScriptTriggerCounter = -1;
   mWeaponEnergy = energyRequired;
   mWeaponErrorFactor = errorFactor;
      
   if ((! projectile) || (! projectile[0]) || (! dStricmp(projectile, "NoAmmo")))
      mProjectile = NULL;
   else
   {
      if (! Sim::findObject(projectile, mProjectile))
      {
         Con::printf("setWeaponInfo() failed - unable to find datablock: %s", projectile);
         mProjectile = NULL; 
      }
   }
   if (mProjectile)
      mProjectileName = StringTable->insert(projectile);
   else
      mProjectileName = StringTable->insert("");
}

void AIConnection::setEnergyLevels(F32 eReserve, F32 eFloat)
{
   mEnergyReserve = eReserve;
   mEnergyFloat = eFloat;
}

void AIConnection::setEngageTarget(GameConnection *target)
{
   //reset the bools if we're aiming at someone new...
   //note, these extraneous retarded checks are because mTargetObject is a SimObjectPtr...
   //won't compile just comparing target != mEngageTarget...
   if (target != NULL && ((! bool(mEngageTarget)) || (target->getId() != mEngageTarget->getId())))
   {
      //reset some engagement vars
      mTargetInRange = false;
      mTargetInSight = false;
      mEngageState = ChooseWeapon;
      mFiring = false;
      mTriggerCounter = 0;
      mScriptTriggerCounter = -1;
   }
   
   mEngageTarget = target;
   mTargetPlayer = NULL;
   if ((! bool(mEngageTarget)) || (! mEngageTarget->getControlObject()))
      mEngageTarget = NULL;
   else
      mTargetPlayer = dynamic_cast<Player*>(mEngageTarget->getControlObject());   
   
   //make sure we actually got a target
   if (! mTargetPlayer)
      mEngageTarget = NULL;
   
}

S32 AIConnection::getEngageTarget()
{
   //see if have someone to shoot at
   Player *targetPlayer = NULL;
   if ((! bool(mEngageTarget)) || (! mEngageTarget->getControlObject()))
   {
      mTargetPlayer = NULL;
      mEngageTarget = NULL;
      return -1;
   }
   else
      targetPlayer = dynamic_cast<Player*>(mEngageTarget->getControlObject());   
      
   //see if the target is dead
   if (! targetPlayer || ! dStricmp(targetPlayer->getStateName(), "dead"))
   {
      mTargetPlayer = NULL;
      mEngageTarget = NULL;
      return -1;
   }
   
   //return the id of the target
   return mEngageTarget->getId();
}

void AIConnection::setVictim(GameConnection *victim, Player *corpse)
{
   mVictim = victim;
   mCorpse = corpse;
   mVictimTime = Sim::getCurrentTime();
}

S32 AIConnection::getVictimCorpse()
{
   if (bool(mCorpse))
      return mCorpse->getId();
   else
      return -1;
}

S32 AIConnection::getVictimTime()
{
   return mVictimTime;
}

void AIConnection::setTargetObject(ShapeBase *targetObject, F32 range, S32 objectMode)
{
   //reset the bools if we're not aiming at a player, and we have a new target object
   //note, these extraneous retarded checks are because mTargetObject is a SimObjectPtr...
   //won't compile just comparing targetObject != mTargetObject...
   if (! mTargetPlayer && (! targetObject || ! bool(mTargetObject) || targetObject->getId() != mTargetObject->getId()))
   {
      mTargetInRange = false;
      mTargetInSight = false;
      mEngageState = ChooseWeapon;
      mFiring = false;
      mTriggerCounter = 0;
      mScriptTriggerCounter = -1;
   }
   
   mTargetObject = targetObject;
   mRangeToTarget = range;
   mObjectMode = objectMode;
}

S32 AIConnection::getTargetObject()
{
   if (bool(mTargetObject))
      return mTargetObject->getId();
   else
      return -1;
}

void AIConnection::setPathDest(const Point3F * dest)
{
   if( dest )  
      mPathDest = * dest;
   mNewPath = true;
}

const Point3F * AIConnection::getPathDest()
{
   if( mNewPath ){
      mNewPath = false;
      return & mPathDest;
   }
   return NULL;
}

// Pass down jetting abilities to the path machinery- 
void AIConnection::setPathCapabilities(Player * )
{
   PROFILE_START(AI_setPathCapabilities);
   JetManager::Ability  ability;

// Tribes player jetting was remove from the player class.
#if 0
   player->getJetAbility(ability.acc, ability.dur, ability.v0);
#else
   ability.acc = 0;
   ability.dur = 0;
   ability.v0 = 0;
#endif

   mPath.setJetAbility(ability);
   PROFILE_END();
}

F32 AIConnection::getPathDistance(const Point3F &destination, const Point3F &source)
{
   //find our current location
   Point3F sourceLocation = source;
   if (sourceLocation == Point3F(-1, -1, -1))
   {
      //make sure we have a valid player control object
      Player *myPlayer = NULL;
      if (! getControlObject())
         return -1;
      myPlayer = dynamic_cast<Player*>(getControlObject());
      if (! myPlayer)
         return -1;
         
      setPathCapabilities(myPlayer);

      MatrixF const& tempTransform = myPlayer->getTransform();
      tempTransform.getColumn(3, &sourceLocation);

      //make sure the client can actually get to the destination
      if (! mPath.canReachLoc(destination))
         return -1;
   }

   //this is a weird change - want the bots to avoid vertical movement, so the distance function will
   //exaggerate the z component...
   Point3F distVec = sourceLocation - destination;
   return mSqrt(distVec.x * distVec.x + distVec.y * distVec.y + 9 * distVec.z * distVec.z);

   //this is just a euclidean distance anyways...  if it becomes an actual path distance again, use it...
   //return NavigationGraph::fastDistance(sourceLocation, destination);
}

F32 AIConnection::getPathDistRemaining(F32 maxDist)
{
   //if the path is current, use the path distRemaining function
   if (mPath.isPathCurrent())
      return mPath.distRemaining(maxDist);

   //otherwise, return the euclidean dist
   else
      return getMin(maxDist, (mLocation - mMoveDestination).len());
}

Point3F AIConnection::getLOSLocation(const Point3F &targetPoint, F32 minDistance, F32 maxDistance, const Point3F &nearPoint)
{
   //find our current location
   Point3F sourceLocation;
   Player *myPlayer = NULL;
   if (! getControlObject())
      return targetPoint;
   myPlayer = dynamic_cast<Player*>(getControlObject());
   if (! myPlayer)
      return targetPoint;
   MatrixF const& tempTransform = myPlayer->getTransform();
   tempTransform.getColumn(3, &sourceLocation);

   Point3F nearLocation = nearPoint;
   if (nearLocation == Point3F(-1, -1, -1))
      nearLocation = sourceLocation;

   Point3F graphPoint;
   if (minDistance < 0)
      graphPoint = NavigationGraph::findLOSLocation(sourceLocation, targetPoint, 0, SphereF(nearLocation, 1e6), maxDistance);
   else
      graphPoint = NavigationGraph::findLOSLocation(sourceLocation, targetPoint, minDistance, SphereF(nearLocation, minDistance / 2.0f), maxDistance);
   return graphPoint;
}

Point3F AIConnection::getHideLocation(const Point3F &targetPoint, F32 range, const Point3F &nearPoint, F32 hideLength)
{
   //find our current location
   Point3F sourceLocation = nearPoint;
   if (sourceLocation == Point3F(-1, -1, -1))
   {
      Player *myPlayer = NULL;
      if (! getControlObject())
         return targetPoint;
      myPlayer = dynamic_cast<Player*>(getControlObject());
      if (! myPlayer)
         return targetPoint;
      MatrixF const& tempTransform = myPlayer->getTransform();
      tempTransform.getColumn(3, &sourceLocation);
   }

   if (hideLength <= 0)
      return NavigationGraph::hideOnSlope(sourceLocation, targetPoint, range, 20);
   else
      return NavigationGraph::hideOnDistance(sourceLocation, targetPoint, range, hideLength);
}

void AIConnection::process(ShapeBase *ctrlObject)
{
   if (!ctrlObject)
      return;

   PROFILE_START(AI_process);

   //update the task queue
   AITask *highestWeightTask = NULL;
   S32 i;
   for (i = 0; i < mTaskList.size(); i++)
   {
      AITask *task = mTaskList[i];
      task->calcWeight(this);
      
      //see if it's the highest so far
      if ((! highestWeightTask) || (task->getWeight() > highestWeightTask->getWeight()))
         highestWeightTask = task;
   }
   
   // Path needs team for avoiding threats- 
   mPath.setTeam(getSensorGroup());
   
   //now see if we have a new task
   if (highestWeightTask && mCurrentTask != highestWeightTask)
   {
      if (bool(mCurrentTask))
         mCurrentTask->retire(this);
      highestWeightTask->assume(this);
      mCurrentTask = highestWeightTask;
      mCurrentTaskTime = Sim::getCurrentTime();
   }
   
   //monitor the current task
   if (mCurrentTask) {
      PROFILE_START(AI_MonitorTask);
      mCurrentTask->monitor(this);
      PROFILE_END();
   }

   //see if our control object is a player
   Player *myPlayer = NULL;
   myPlayer = dynamic_cast<Player*>(ctrlObject);
   if (myPlayer)
   {
      //initialize the process vars
      initProcessVars(myPlayer);
      
      //next, process the current step
      if (bool(mStep))
         mStep->process(this, myPlayer);
   
      //process the engagement
      if (mTurretMountedId <= 0) {
         PROFILE_START(AI_EngagementOuter);
         processEngagement(myPlayer);
         PROFILE_END();
      }
      
      //finally, process the movement itself
      if (!myPlayer->isMounted())
         processMovement(myPlayer);
      else
         processVehicleMovement(myPlayer);
   }

   //else see if we're trying to pilot a vehicle
   else
   {
      Vehicle *myVehicle;
      myVehicle = dynamic_cast<Vehicle*>(ctrlObject);
      if (myVehicle)
      {
         processPilotVehicle(myVehicle);
      }
   }

   //debug
   aiDebugStuff();
   PROFILE_END();
}

// LH- changed this to not normalize when there's a zero length.  Dividing by zero
// results in interupts.  Instead do normalization here checking lengths first.  
F32 AIConnection::get2DDot(const Point3F &vec1, const Point3F &vec2)
{
   //create the 2D vectors and check for zero length.  
   Point3F vec1_2D(vec1.x, vec1.y, 0);
   F32 len1 = vec1_2D.len();
   if (len1 < __EQUAL_CONST_F)
      return 1;
      
   Point3F vec2_2D(vec2.x, vec2.y, 0);
   F32 len2 = vec2_2D.len();
   if (len2 < __EQUAL_CONST_F)
      return 1;
      
   // Normalize and return the dot- 
   return mDot(vec1_2D /= len1, vec2_2D /= len2);
}

F32 AIConnection::get2DAngle(const Point3F &endPt, const Point3F &basePt)
{
   Point3F direction2D = endPt - basePt;
   F32 angularDirection;
   direction2D.z = 0;
   
   if (! isZero(direction2D.y))
      angularDirection = mAtan(direction2D.x, direction2D.y);
   else if (direction2D.x < 0)
      angularDirection = -M_PI / 2.0f;
   else
      angularDirection = M_PI / 2.0f;
      
   if (angularDirection < 0)
      angularDirection += M_2PI;
   else if (angularDirection >= M_2PI)
      angularDirection -= M_2PI;
      
   //note: return value is always between 0 and (2 * Pi)
   return angularDirection;
}

Point3F AIConnection::dopeAimLocation(const Point3F &startLocation, const Point3F &aimLocation)
{
   //find the "horizontal" orthogonal vector
   Point3F horzOrth;
   if (! findCrossVector(startLocation - aimLocation, Point3F(0, 0, 1), &horzOrth))
      return aimLocation;
      
   //now find the "vertical" orthogonal vector
   Point3F vertOrth;
   if (! findCrossVector(startLocation - aimLocation, horzOrth, &vertOrth))
      return aimLocation;
      
   //now we have the horizonal and vertical components of the plane which is perpendicular to
   //the direction we are firing...

   //determine the radius factor
   F32 radiusFactor;
   if (mSkillLevel == 1.0f)
      radiusFactor = 0.0f;
   else
      radiusFactor = 0.04 + (1.0f - mSkillLevel) * 0.2;
      
   //calculate the radius error
   F32 radiusError = (startLocation - aimLocation).len() * radiusFactor * mWeaponErrorFactor;
   if (! mProjectile->isBallistic && mTargStillTimeMS > 0 && radiusError > 0)
   {
      //begin honing in after 3 seconds, and over the next 3 seconds
      S32 elapsedTime = Sim::getCurrentTime() - mTargStillTimeMS - 3000;
      if (elapsedTime > 0)
         radiusError = radiusError * (F32(3000 - getMin(elapsedTime, 3000)) / 3000.0f);
   }
   
   F32 horzError = gRandGen.randF() * radiusError * (gRandGen.randF() < 0.5f ? -1.0f : 1.0f);
   F32 vertError = gRandGen.randF() * radiusError * (gRandGen.randF() < 0.5f ? -1.0f : 1.0f);
   
   Point3F dopedAimLocation = aimLocation + (horzOrth * horzError) + (vertOrth * vertError);
   return dopedAimLocation;
}
      
F32 AIConnection::getOutdoorRadius(const Point3F &location)
{
   F32 freedomRadius;
   if (mPath.locationIsOutdoors(location, &freedomRadius))
      return freedomRadius;
   else
      return -1;
}

void AIConnection::initProcessVars(Player *player)
{
   PROFILE_START(AI_initProcessVars);
   
   //find my current location (global coords), velocity, energy, damage, etc...
   MatrixF const& tempTransform = player->getTransform();
   tempTransform.getColumn(3, &mLocation);
   mVelocity = player->getVelocity();
   mVelocity2D = mVelocity;
   mVelocity2D.z = 0;
   mRotation = player->getRotation();
   mHeadRotation = player->getHeadRotation();
   mEnergy = player->getEnergyValue();
   mDamage = player->getDamageValue();
   
   if (mEnergy < mEnergyReserve)
      mEnergyRecharge = true;
   else if (mEnergy > mEnergyReserve + mEnergyFloat)
      mEnergyRecharge = false;
      
   if (((mEnergy > mEnergyReserve + mEnergyFloat) || (mEnergy > mEnergyReserve && !mEnergyRecharge)) &&
       (mEnergy > mWeaponEnergy - mEnergyFloat))
      mEnergyAvailable = true;
   else
      mEnergyAvailable = false;

   //find the muzzle point
   //player->getMuzzlePoint(0, &mMuzzlePosition);
   player->getMuzzlePointAI(0, &mMuzzlePosition);

   //find the eye transform
   MatrixF eyeTransform;
   player->getEyeTransform(&eyeTransform);
   eyeTransform.getColumn(3, &mEyePosition);

   //find out far we have to go, and if we're heading in the right direction
   mDistToNode2D = (Point3F(mNodeLocation.x, mNodeLocation.y, 0) -
                    Point3F(mLocation.x, mLocation.y, 0)).len();

   //find out how far off course our velocity is taking us
   mDotOffCourse = get2DDot(mNodeLocation - mLocation, mVelocity2D);
   mDotOffCourse = mClampF(mDotOffCourse, -1.0f, 1.0f);
   
   //find out the difference between them
   F32 dummy;
   bool outdoors = mPath.locationIsOutdoors(mLocation, &dummy);
   mHeadingDownhill = false;
   if (mVelocity2D.len() >= player->getMaxForwardVelocity() * 0.85f && outdoors && !mInWater)
   {
      //make sure we're heading in approximately the right direction (+- 30 deg or so)
      if (mDotOffCourse > 0.85)
      {
         Point3F myDirection2D = mVelocity2D;
         if (myDirection2D.len() < 0.001f)
            myDirection2D.set(0, 1, 0);
         myDirection2D.normalize();
         U32 mask = TerrainObjectType | InteriorObjectType | WaterObjectType;
         RayInfo ray1Info, ray2Info;
         
         Point3F startPt = mLocation;
         Point3F endPt = mLocation;
         endPt.z -= 5.0f;
         
         //if we're not within 1.0 m of the ground, we can't ski...
         player->disableCollision();
         if (! gServerContainer.castRay(startPt, endPt, mask, &ray1Info))
            mHeadingDownhill = true;
         else
         {
            //run another LOS to see if we are heading downhill
            startPt += myDirection2D;
            endPt += myDirection2D;
            
            //if we don't hit anything, we're (on the edge of a cliff?) heading downhill
            if (! gServerContainer.castRay(startPt, endPt, mask, &ray2Info))
               mHeadingDownhill = true;
            else if (ray1Info.point.z > ray2Info.point.z)
               mHeadingDownhill = true;
         }
         player->enableCollision();
      }
   }
   
   //see if we have a target object
   if (bool(mTargetObject))
   {
      MatrixF const& objTransform = mTargetObject->getTransform();
      objTransform.getColumn(3, &mObjectLocation);
      mDistToObject2D = (Point3F(mLocation.x, mLocation.y, 0.0f) -
                         Point3F(mObjectLocation.x, mObjectLocation.y, 0.0f)).len();
   }
   
   //see if have someone to shoot at
   mTargetPlayer = NULL;
   if ((! bool(mEngageTarget)) || (! mEngageTarget->getControlObject()))
      mEngageTarget = NULL;
   else
      mTargetPlayer = dynamic_cast<Player*>(mEngageTarget->getControlObject());   
      
   if (mTargetPlayer)
   {
      //find the target location (global coords), velocity, energy, damage, etc...
      MatrixF const& targTransform = mTargetPlayer->getTransform();
      targTransform.getColumn(3, &mTargLocation);
      mTargVelocity = mTargetPlayer->getVelocity();
      mTargVelocity2D = mTargVelocity;
      mTargVelocity2D.z = 0;
      mTargRotation = mTargetPlayer->getRotation();
      mTargEnergy = mTargetPlayer->getEnergyValue();
      mTargDamage = mTargetPlayer->getDamageValue();

      //see if the target is standing still
      if (mTargVelocity.len() > 4.0f)
         mTargStillTimeMS = 0;
      else if (mTargStillTimeMS == 0)
         mTargStillTimeMS = Sim::getCurrentTime();

      //keep an array of prev locations to simulate a slower response time... (up to 600 ms)
      if (Sim::getCurrentTime() - mTargPrevTimeMS > 300)
      {
         mTargPrevTimeMS = Sim::getCurrentTime();
         mTargPrevLocation[3] = mTargPrevLocation[2];
         mTargPrevLocation[2] = mTargPrevLocation[1];
         mTargPrevLocation[1] = mTargPrevLocation[0];
         mTargetPlayer->getWorldBox().getCenter(&mTargPrevLocation[0]);
      }
   }
   else
      mEngageTarget = NULL;
   
   if (mTargetPlayer)
   {
      //see if the target is dead
      if (! dStricmp(mTargetPlayer->getStateName(), "dead"))
      {
         mTargetPlayer = NULL;
         mEngageTarget = NULL;
      }
      else
      {
         //find the 2D distance between you and the target
         Point3F dist2DVector = mTargLocation - mLocation;
         dist2DVector.z = 0;
         mDistToTarg2D = dist2DVector.len();
      }
   }

   //see if we're outdoors
   mOutdoors = (getOutdoorRadius(mLocation) > 0);

   //see if either we or the targ is in water
   mInWater = false;
   mTargInWater = false;
   mObjectInWater = false;
   SimpleQueryList sql;

   gServerSceneGraph->getWaterObjectList(sql);

//   gServerContainer.findObjects(WaterObjectType, SimpleQueryList::insertionCallback, S32(&sql));

   for (U32 i = 0; i < sql.mList.size(); i++)
   {
      WaterBlock* pBlock = dynamic_cast<WaterBlock*>(sql.mList[i]);
      if (pBlock)
      {
         if (pBlock->isPointSubmergedSimple(mLocation) || pBlock->isPointSubmergedSimple(mMuzzlePosition))
            mInWater = true;

         if (mTargetPlayer)
         {
            if (pBlock->isPointSubmergedSimple(mTargLocation))
               mTargInWater = true;
         }

         if (bool(mTargetObject))
         {
            if (pBlock->isPointSubmergedSimple(mObjectLocation))
               mObjectInWater = true;
         }
      }
   }
   
   //now see if we're within range of either the engageTarget, or the targetObject
   if (--mCheckTargetLOSCounter <= 0)
   {
      mCheckTargetLOSCounter = 10;
      mTargetInSight = false;
      if (mTargetPlayer || bool(mTargetObject))
      {
         F32 rangeDist;
         Point3F rangeLocation;
         if (mTargetPlayer)
         {
            rangeDist = mDistToTarg2D;
            mTargetPlayer->getWorldBox().getCenter(&rangeLocation);
         }
         else
         {
            rangeDist = mDistToObject2D;
            mTargetObject->getWorldBox().getCenter(&rangeLocation);
         }
         if (rangeDist <= mRangeToTarget)
         {
            //see if we have line of site
            RayInfo rayInfo;
            Point3F startPt = mMuzzlePosition;
            Point3F endPt = rangeLocation;
            U32 mask = TerrainObjectType | InteriorObjectType;
            if (! gServerContainer.castRay(startPt, endPt, mask, &rayInfo))
               mTargetInSight = true;
         }
      }
   }

   //reset the weaponEnergy level if req'd
   if (! bool(mTargetPlayer))
      mWeaponEnergy = 0.0f;

   //set the corpse vars
   if (bool(mCorpse))
   {
      MatrixF const& corpseTransform = mCorpse->getTransform();
      corpseTransform.getColumn(3, &mCorpseLocation);
   }
   PROFILE_END();
}

void AIConnection::updateDetectionTable(Player *player)
{
   //if the player has been blinded, no updates to the table can be made
   if (Sim::getCurrentTime() < mBlindedTimer)
      return;

   //time slice the detection LOS calls...
   mPlayerDetectionCounter--;
   if (mPlayerDetectionCounter <= 0)
   {
      SimGroup *clientGroup = Sim::getClientGroup();
      AssertFatal(clientGroup, "Unable to get the client group");

      //make sure we have more than one client
      if (clientGroup->size() <= 1)
         return;

      //find the next client index
      mPlayerDetectionIndex++;
      if (mPlayerDetectionIndex >= clientGroup->size())
         mPlayerDetectionIndex = 0;

      //get the client from the group
      GameConnection *targClient = static_cast<GameConnection*>((*clientGroup)[mPlayerDetectionIndex]);

      //make sure it's not me...
      S32 targClientId = targClient->getId();
      if (getId() == targClientId)
      {
         mPlayerDetectionIndex++;
         if (mPlayerDetectionIndex >= clientGroup->size())
            mPlayerDetectionIndex = 0;
         targClient = static_cast<GameConnection*>((*clientGroup)[mPlayerDetectionIndex]);
         targClientId = targClient->getId();
      }

      //now find this target in the table
      PlayerDetectionEntry *targEntry = NULL;
      for (S32 i = 0; i < mPlayerDetectionTable.size(); i++)
      {
         if (mPlayerDetectionTable[i].playerId == targClientId)
            targEntry = &(mPlayerDetectionTable[i]);
      }
      //if the entry wasn't found, create one...  (and yes, this table will never shrink)
      if (!targEntry)
      {
         PlayerDetectionEntry tempEntry;
         tempEntry.playerId = targClientId;
         tempEntry.playerLOS = false;
         tempEntry.playerLOSTime = 0;
         tempEntry.playerLastPosition.set(0, 0, 0);
         mPlayerDetectionTable.push_front(tempEntry);
         targEntry = &(mPlayerDetectionTable.first());
      }

      //make sure the client has a player
      Player *targPlayer = NULL;
      if (! targClient->getControlObject())
      {
         if (targEntry->playerLOS)
         {
            targEntry->playerLOS = false;
            targEntry->playerLOSTime = Sim::getCurrentTime();
         }
         return;
      }
      targPlayer = dynamic_cast<Player*>(targClient->getControlObject());   
      if (! targPlayer)
      {
         if (targEntry->playerLOS)
         {
            targEntry->playerLOS = false;
            targEntry->playerLOSTime = Sim::getCurrentTime();
         }
         return;
      }

      //now cast line of sight, see if we can see the player
      MatrixF myEyeTransform, targEyeTransform;
      Point3F myEyePosition, targEyePosition;
      player->getEyeTransform(&myEyeTransform);
      myEyeTransform.getColumn(3, &myEyePosition);
      targPlayer->getEyeTransform(&targEyeTransform);
      targEyeTransform.getColumn(3, &targEyePosition);

      //if the target player is cloaked, they can't be detected
      bool targPlayerIsCloaked = targPlayer->getCloakedState();

      Point3F losVector = targEyePosition - myEyePosition;
      F32 distToTarg = losVector.len();
      bool clearLOSToTarg;
      if (mSkillLevel >= 1.0f)
         clearLOSToTarg = true;
      else if (distToTarg < 0.5f)
         clearLOSToTarg = true;
      else if (targPlayerIsCloaked)
         clearLOSToTarg = false;
      else if (distToTarg > 300.0f)
         clearLOSToTarg = false;
      else
      {
         //first, see if we're facing the right way...
         Point3F facingVector = mAimLocation - mLocation;
         F32 visibleRange = 0.4 - (0.4 * mSkillLevel);
         if (facingVector.len() < 0.5f)
            clearLOSToTarg = false;
         else if (get2DDot(facingVector, losVector) < visibleRange && distToTarg > 1.5f)
            clearLOSToTarg = false;
         else
         {
            //see if we have line of site
            if (losVector.len() < 0.001f)
               losVector.set(0, 1, 0);
            losVector.normalize();
            RayInfo rayInfo;
            Point3F startPt = myEyePosition;
            Point3F endPt = myEyePosition + (getMin(300.0f, distToTarg) * losVector);
            U32 mask = TerrainObjectType | InteriorObjectType;
            if (! gServerContainer.castRay(startPt, endPt, mask, &rayInfo))
               clearLOSToTarg = true;
            else
               clearLOSToTarg = false;
         }
      }

      //set the time if the status changed
      if (clearLOSToTarg != targEntry->playerLOS)
         targEntry->playerLOSTime = Sim::getCurrentTime();

      //set the bool
      targEntry->playerLOS = clearLOSToTarg;

      //update the position if required
      if (clearLOSToTarg)
         targPlayer->getWorldBox().getCenter(&targEntry->playerLastPosition);

      //don't forget to set the timeslice counter
      mPlayerDetectionCounter = 30 / (clientGroup->size() - 1);
      if (mPlayerDetectionCounter < 3)
         mPlayerDetectionCounter = 3;
   }
}

void AIConnection::setBlinded(S32 duration)
{
   //can't blind the Kidney Bot!!!
   if (mSkillLevel >= 1.0f)
      return;

   //first, set the blinded timer
   mBlindedTimer = Sim::getCurrentTime() + duration;

   //now loop through the table, and anyone who he has LOS to, he now doesn't
   PlayerDetectionEntry *targEntry = NULL;
   for (S32 i = 0; i < mPlayerDetectionTable.size(); i++)
   {
      targEntry = &(mPlayerDetectionTable[i]);
      if (targEntry->playerLOS)
      {
         targEntry->playerLOS = false;
         targEntry->playerLOSTime = Sim::getCurrentTime();
      }
   }

   //next, set the evade location to make us react...
   Point3F dangerLocation = mLocation;
   dangerLocation.x += -2.0f + (gRandGen.randF() * 4.0f);
   dangerLocation.y += -2.0f + (gRandGen.randF() * 4.0f);
   setEvadeLocation(dangerLocation);
   mEvadingCounter = 30;
}

void AIConnection::clientDetected(S32 targId)
{
   //first, make sure we're not detecting ourself
   if (getId() == targId)
      return;

   SimGroup *clientGroup = Sim::getClientGroup();
   AssertFatal(clientGroup, "Unable to get the client group");

   GameConnection *targClient = NULL;
   for (SimGroup::iterator itr = clientGroup->begin(); itr != clientGroup->end(); itr++)
   {
      GameConnection *client = static_cast<GameConnection*>(*itr);
      if (client->getId() == targId)
      {
         targClient = client;
         break;
      }
   }
   if (! targClient)
      return;

   //now find this target in the table
   PlayerDetectionEntry *targEntry = NULL;
   for (S32 i = 0; i < mPlayerDetectionTable.size(); i++)
   {
      if (mPlayerDetectionTable[i].playerId == targId)
         targEntry = &(mPlayerDetectionTable[i]);
   }
   //if the entry wasn't found, create one...  (and yes, this table will never shrink)
   if (!targEntry)
   {
      PlayerDetectionEntry tempEntry;
      tempEntry.playerId = targId;
      tempEntry.playerLOS = false;
      tempEntry.playerLOSTime = 0;
      tempEntry.playerLastPosition.set(0, 0, 0);
      mPlayerDetectionTable.push_front(tempEntry);
      targEntry = &(mPlayerDetectionTable.first());
   }

   //make sure the client has a player
   Player *targPlayer = NULL;
   if (! targClient->getControlObject())
      return;
   targPlayer = dynamic_cast<Player*>(targClient->getControlObject());   
   if (! targPlayer)
      return;

   //set the time if the status changed
   if (!targEntry->playerLOS)
      targEntry->playerLOSTime = Sim::getCurrentTime();

   //set the bool
   targEntry->playerLOS = true;

   //update the position
   targPlayer->getWorldBox().getCenter(&targEntry->playerLastPosition);
}

bool AIConnection::hasLOSToClient(S32 clientId, S32 &losTime, Point3F &lastLocation)
{
   //now find this target in the table
   PlayerDetectionEntry *targEntry = NULL;
   for (S32 i = 0; i < mPlayerDetectionTable.size(); i++)
   {
      if (mPlayerDetectionTable[i].playerId == clientId)
         targEntry = &(mPlayerDetectionTable[i]);
   }

   if (! targEntry)
   {
      losTime = Sim::getCurrentTime();
      lastLocation.set(0, 0, 0);
      return false;
   }
   else
   {
      losTime = Sim::getCurrentTime() - targEntry->playerLOSTime;
      lastLocation = targEntry->playerLastPosition;
      return targEntry->playerLOS;
   }
}

// LH- put script calls into separate methods so profiler can measure them.
//  Then added slicing since it was weighing in on profiles.  
void AIConnection::scriptProcessEngagement()
{
   if (!gScriptEngageSlicer.ready(mPackCheckCounter, 6))
      return;

   char idStr[32], targetIdStr[32], targetTypeStr[32], projectileStr[64];
   dSprintf(idStr, sizeof(idStr), "%d", getId());
   if (mTargetPlayer)
   {
      dSprintf(targetTypeStr, sizeof(targetTypeStr), "%s", "player");
      dSprintf(targetIdStr, sizeof(targetIdStr), "%d", mEngageTarget->getId());
   }
   else if (bool(mTargetObject) && mObjectMode == AIConnection::DestroyObject)
   {
      dSprintf(targetTypeStr, sizeof(targetTypeStr), "%s", "object");
      dSprintf(targetIdStr, sizeof(targetIdStr), "%d", mTargetObject->getId());
   }
   else
   {
      dSprintf(targetTypeStr, sizeof(targetTypeStr), "%s", "none");
      dSprintf(targetIdStr, sizeof(targetIdStr), "%d", -1);
   }
   if (bool(mEnemyProjectile) && mEvadingCounter > 0 && mEvadingCounter < 45)
      dSprintf(projectileStr, sizeof(projectileStr), "%d", mEnemyProjectile->getId());
   else
      dSprintf(projectileStr, sizeof(projectileStr), "%d", -1);

   Con::executef(5, "AIProcessEngagement", idStr, targetIdStr, targetTypeStr, projectileStr);
}

void AIConnection::scriptChooseEngageWeapon(F32 distToTarg)
{
   char idStr[32], targetIdStr[32], distTotargStr[32];
   
   dSprintf(idStr, sizeof(idStr), "%d", getId());
   dSprintf(targetIdStr, sizeof(targetIdStr), "%d", mEngageTarget->getId());
   dSprintf(distTotargStr, sizeof(distTotargStr), "%f", distToTarg);
   const char* canUseEnergyStr = mNavUsingJet ? "false" : "true";
   const char* environmentStr = (mInWater || mTargInWater ? "water" : (mOutdoors ? "outdoors" : "indoors"));
   Con::executef(6, "AIChooseEngageWeapon", idStr, targetIdStr, distTotargStr, canUseEnergyStr, environmentStr);
}

void AIConnection::scriptChooseObjectWeapon(F32 distToTarg)
{
   char idStr[32], targetIdStr[32], distTotargStr[32];
   
   dSprintf(idStr, sizeof(idStr), "%d", getId());
   dSprintf(targetIdStr, sizeof(targetIdStr), "%d", mTargetObject->getId());
   dSprintf(distTotargStr, sizeof(distTotargStr), "%f", distToTarg);
   const char* canUseEnergyStr = mNavUsingJet ? "false" : "true";
   const char* environmentStr = (mInWater || mTargInWater ? "water" : (mOutdoors ? "outdoors" : "indoors"));
   Con::executef(7, "AIChooseObjectWeapon", idStr, targetIdStr, distTotargStr, gTargetObjectMode[mObjectMode], canUseEnergyStr, environmentStr);
}

void AIConnection::setEvadeLocation(const Point3F &dangerLocation, S32 durationTicks)
{
   //find a new location orthogonal to the danger location to head to
   Point3F dangerVector = dangerLocation - mLocation;
   Point3F myVector = mMoveLocation - mLocation;
   if (dangerVector.len() < 0.001f)
      dangerVector.set(0, 1, 0);
   dangerVector.normalize();
   if (myVector.len() < 0.001f)
      myVector.set(0, 1, 0);
   myVector.normalize();
   Point3F orthDanger;
   mCross(dangerVector, Point3F(0, 0, 1), &orthDanger);
   if (orthDanger.len() < 0.001f)
      orthDanger.set(0, 1, 0);
   orthDanger.normalize();

   //if we have a NAN vector, the danger location is us!
   if (orthDanger.x != orthDanger.x || orthDanger.y != orthDanger.y)
   {
      dangerVector = mTargLocation - mLocation;
      if (dangerVector.len() < 0.001f)
         dangerVector.set(0, 1, 0);
      dangerVector.normalize();
      mCross(dangerVector, Point3F(0, 0, 1), &orthDanger);
      orthDanger.normalize();

      //if we have *STILL* have a NAN vector, head north!
      if (orthDanger.x != orthDanger.x || orthDanger.y != orthDanger.y)
         orthDanger.set(0, 1, 0);
   }

   //choose the point closest to the direction we're already moving...
   F32 myAngle = get2DAngle(mLocation + mVelocity2D, mLocation);
   F32 tempAngle1 = get2DAngle(mLocation, mTargLocation + orthDanger);
   F32 tempAngle2 = get2DAngle(mLocation, mTargLocation - orthDanger);
   F32 angleDiff1, angleDiff2;
   if (myAngle < tempAngle1)
      angleDiff1 = getMin(tempAngle1 - myAngle, 256 + myAngle - tempAngle1);
   else
      angleDiff1 = getMin(myAngle - tempAngle1, 256 + tempAngle1 - myAngle);
   if (myAngle < tempAngle2)
      angleDiff2 = getMin(tempAngle2 - myAngle, 256 + myAngle - tempAngle2);
   else
      tempAngle2 = getMin(myAngle - tempAngle2, 256 + tempAngle2 - myAngle);

   if (angleDiff1 < angleDiff2)
      mEvadeLocation = mLocation + (orthDanger * 30.0f);
   else
      mEvadeLocation = mLocation - (orthDanger * 30.0f);

   if (durationTicks > 0)
      mEvadingCounter = durationTicks;
}

void AIConnection::processEngagement(Player *player)
{
   //updatate the detection table
   PROFILE_START(AI_updateDetectionTable);
      updateDetectionTable(player);
   PROFILE_END();

   //call the script function once each frame to handle packs, grenades, health kits, etc...
   scriptProcessEngagement();

   //see if we need to reset the script trigger counter (if we changed weapons...)
   ShapeBaseImageData* tempImage = player->getMountedImage(0);
   if (tempImage != mMountedImage)
      mScriptTriggerCounter = -1;
   mMountedImage = tempImage;

   //if the script wants us to fire (using the repair pack to self repair mainly)...
   if (mScriptTriggerCounter-- >= 0)
      pressFire(true);

   //make sure we have someone to shoot at
   if (! mTargetPlayer && ! bool(mTargetObject))
      return;

   //if we have something or someone to shoot at, let the engage code determine whether to fire
   pressFire(false);
      
   //reset the counter
   mLookAtTargetTimeMS = Sim::getCurrentTime() + 1500;
      
   //see if we're shooting a player, or a static object
   bool engagingPlayer;
   Point3F targetLocation;
   F32 distToTarg;
   if (mTargetPlayer)
   {
      engagingPlayer = true;

      //if the player has been blinded, use the last known location only
      if (Sim::getCurrentTime() < mBlindedTimer)
      {
         S32 dummyTime;
         hasLOSToClient(mEngageTarget->getId(), dummyTime, targetLocation);
      }
      else
      {
         //see if we have LOS to the client
         S32 losTime;
         Point3F losLocation;
         bool hasLOS = hasLOSToClient(mEngageTarget->getId(), losTime, losLocation);

         //if we have just lost LOS to the target, assume he went around a corner, and fire at the last known location
         if (!hasLOS && losTime < 2500)
            targetLocation = losLocation;

         //this simulates slower response - the bot doesn't quite keep up to the target's current location
         else if (mSkillLevel >= 0.9f)
            targetLocation = mTargPrevLocation[0];
         else if (mSkillLevel >= 0.7f)
            targetLocation = mTargPrevLocation[1];
         else if (mSkillLevel >= 0.3f)
            targetLocation = mTargPrevLocation[2];
         else
            targetLocation = mTargPrevLocation[3];
      }

      //now calculate the 2D distance
      distToTarg = (Point3F(targetLocation.x, targetLocation.y, 0) - Point3F(mLocation.x, mLocation.y, 0)).len();
   }
   else
   {
      engagingPlayer = false;

      //if a repair node has been added to the object, use that, otherwise, use the worldBoxCenter
      targetLocation = mTargetObject->getAIRepairPoint();
      if (targetLocation == Point3F(0, 0, 0))
         mTargetObject->getWorldBox().getCenter(&targetLocation);
      distToTarg = mDistToObject2D;
   }
      
   //detect the projectiles from the target
   mProjectileCounter--;
   if (engagingPlayer)
   {
      const char *incoming = mEngageTarget->getDataField(StringTable->insert("projectile"), NULL);
      if (incoming && incoming[0])
      {
         Projectile *projectile;
         if (Sim::findObject(incoming, projectile))
         {
            //see if it's a new threat, or time to re-evaluate the current one
            if (projectile != (Projectile*)mEnemyProjectile || mProjectileCounter <= 0)
            {
               //reset the projectile counter
               mProjectileCounter = 15;
            
               F32 timeToImpact;
               ProjectileData* projData = projectile->getDataBlock() != NULL ?
                                             dynamic_cast<ProjectileData*>(projectile->getDataBlock()) :
                                             NULL;
               if (projData && projectile->calculateImpact(4.0f, mImpactLocation, timeToImpact)) {
                  //see if the impact location is within range
                  Point3F predictMyLocation = mLocation + mVelocity * timeToImpact;
                  F32 distToDanger = (predictMyLocation - mImpactLocation).len();
                  if (distToDanger < getMax(projData->damageRadius, 2.0f))
                  {
                     setEvadeLocation(mImpactLocation);
                     mEnemyProjectile = projectile;

                     //set the evade counter - any value above 45 is simulated response time
                     S32 responseTime = S32(30 * (1.0f - mSkillLevel));
                     mEvadingCounter = 45 + responseTime;
                  }
               }
            }
         }
      }
   }

   //if we're in the middle of a NavGraph Jet, we can't shoot...
   Point3F dummyPoint;
   if (mNavUsingJet && mJetting.shouldAimAt(dummyPoint))
      return;

   //here we only fire at someone if we've seen them recently enough
   if (bool(mEngageTarget))
   {
      S32 detectLOSTime;
      Point3F lastLOSLocation;
      bool hasLOSDetect = hasLOSToClient(mEngageTarget->getId(), detectLOSTime, lastLOSLocation);
      if (!hasLOSDetect && (detectLOSTime > mDetectHiddenPeriod))
         return;
   }

   //process the engagement state machine
   switch (mEngageState)
   {
      case ChooseWeapon:
      {
         //set the aim location
         if (! mTargetInRange)
            setAimLocation(targetLocation);
         else
            setAimLocation(Point3F(targetLocation.x, targetLocation.y, mAimLocation.z));
   
         //set the bool
         mFiring = false;

         //limit changing the weapon too often for low skill
         if (--mChangeWeaponCounter > 0)
         {
            mEngageState = ReloadWeapon;
            mStateCounter = 45;

            //set the delay counter
            mDelayCounter = S32(20.0f * (1.0f - mSkillLevel));
         }

         //else reset the counter
         else
            mChangeWeaponCounter = S32((1.0f - mSkillLevel) / 0.10f);
         
         //keep track of the previous weapon
         StringTableEntry prevWeapon = mProjectileName;

         //call the script function to set the weapon vars
         if (engagingPlayer)
            scriptChooseEngageWeapon(distToTarg);
         else
            scriptChooseObjectWeapon(distToTarg);

         if (distToTarg > mEngageMaxDistance)
         {
            mEngageState = OutOfRange;
            mStateCounter = 30;
         }
         else
         {
            mEngageState = ReloadWeapon;
            mStateCounter = 60;

            //set the delay counter
            mDelayCounter = S32(20.0f * (1.0f - mSkillLevel));

            //see if we've switched weapons
            if (dStricmp(mProjectileName, prevWeapon))
               mDelayCounter *= 3;
         }
         break;
      }
      
      case OutOfRange:
      {
         //set the aim location
         if (! mTargetInRange)
            setAimLocation(targetLocation);
         else
            setAimLocation(Point3F(targetLocation.x, targetLocation.y, mAimLocation.z));
   
         if (distToTarg <= mEngageMaxDistance || (--mStateCounter <= 0))
            mEngageState = ChooseWeapon;
         break;
      }
      
      case ReloadWeapon:
      {
         //set the aim location
         if (! mTargetInRange)
            setAimLocation(targetLocation);
         else
            setAimLocation(Point3F(targetLocation.x, targetLocation.y, mAimLocation.z));

         //make sure we're not stuck in this state
         if (--mStateCounter <= 0)
         {
            mEngageState = ChooseWeapon;
            break;
         }
         
         //make sure we're not trying to use an energy weapon while jetting
         if (mWeaponEnergy > 0 && mNavUsingJet)
         {
            mEngageState = ChooseWeapon;
            break;
         }

         //if the weapon is ready, time to aim
         bool weaponReady = player->isImageReady(0);
         bool hasEnergy = (mEnergy >= mWeaponEnergy);

         //add in factor so they're not unloading on you at lower skill settings...
         F32 skillVelocityFactor = 6.0f + (mSkillLevel * mSkillLevel * mSkillLevel * 300.0f);
         bool mustSlowDown = (mVelocity.len() > skillVelocityFactor);

         if (weaponReady && hasEnergy && !mustSlowDown)
         {
            mStateCounter = 15;
            if (--mDelayCounter <= 0)
               mEngageState = FindTargetPoint;
         }

         //if firing is sustained, keep firing
         if (mFiring && mTriggerCounter > 0)
         {
            mTriggerCounter--;
            pressFire();
         }
            
         break;
      }
      
      case FindTargetPoint:
      {
         //set the aim location
         if (! mTargetInRange)
            setAimLocation(targetLocation);
         else
            setAimLocation(Point3F(targetLocation.x, targetLocation.y, mAimLocation.z));

         //make sure we're not stuck in this state
         if (--mStateCounter <= 0)
         {
            mEngageState = ChooseWeapon;
            break;
         }
         
         //make sure we're not trying to use an energy weapon while jetting
         if (mWeaponEnergy > 0 && mNavUsingJet)
         {
            mEngageState = ChooseWeapon;
            break;
         }
   
         //should we go for center mass, or splash damage, or are we shooting at a lazed target
         Point3F aimAtTargetPoint;
         mAimAtLazedTarget = false;
         
         //only shoot at lazed targets if we're using a ballistic weapon
         if (mProjectile && mProjectile->isBallistic)
         {
            //see if we have a lazed target point...
            SimSet *targetSet = Sim::getServerTargetSet();
            if (targetSet)
            {
               SimSet::iterator i;
               for (i = targetSet->begin(); i != targetSet->end(); i++)
               {
                  GameBase *target = static_cast<GameBase*>(*i);
                  Point3F targetPoint;
                  U32 dummyTeam;  //when the sensor network is finished, then the target's
                                 //visibility can be properly determined.
                  target->getTarget(&targetPoint, &dummyTeam);
                  //see if the target is within 20m of what we're trying to shoot at...
                  if ((targetPoint - targetLocation).len() < 10.0f)
                  {
                     aimAtTargetPoint = targetPoint;
                     mAimAtLazedTarget = true;
                  }
               }
            }
         }
         
         if (! mAimAtLazedTarget)
         {
            //see if we should aim for the target's feet
            if (engagingPlayer && mProjectile && mProjectile->hasDamageRadius && mProjectile->damageRadius > 5.0f)
               aimAtTargetPoint = mTargLocation;
               
            //else aim for the target's center mass
            else
               aimAtTargetPoint = targetLocation;
         }
         
         //find the aim vector
         Point3F aimVectorMin, aimVectorMax;
         F32 timeMin, timeMax;
         if (mProjectile)
         {
            bool canShoot;
            if (engagingPlayer)
               canShoot = mProjectile->calculateAim(aimAtTargetPoint, mTargVelocity, mMuzzlePosition, mVelocity, &aimVectorMin, &timeMin, &aimVectorMax, &timeMax);
            else
            {
               canShoot = false;
               if (mObjectMode != MissileVehicle || player->getLockedTargetId() == mTargetObject->getId())
                  canShoot = mProjectile->calculateAim(aimAtTargetPoint, Point3F(0, 0, 0), mMuzzlePosition, mVelocity, &aimVectorMin, &timeMin, &aimVectorMax, &timeMax);
            }
            if (canShoot)
            {
               //find the aimlocation from the normalized aim vector
               Point3F aimLocation = mMuzzlePosition + aimVectorMin * (aimAtTargetPoint - mMuzzlePosition).len();
            
               //now add the tone down the accuracy for moving targets
               bool needToDope = mVelocity.len() > 4.0f || engagingPlayer || (mProjectile->isBallistic && ! mAimAtLazedTarget);
               if (needToDope)
                  aimLocation = dopeAimLocation(mMuzzlePosition, aimLocation);
               
               //set the aim location
               setAimLocation(aimLocation);
         
               mEngageState = AimAtTarget;
               mStateCounter = 15;
         
               //call processEngagement again immediately
               PROFILE_START(AI_EngagementInner);
               processEngagement(player);
               PROFILE_END();
            }
            else
            {
               //reset the bool
               mTargetInRange = false;
               
               //if firing is sustained, keep firing
               if (mFiring && mTriggerCounter > 0)
               {
                  mTriggerCounter--;
                  pressFire();
               }
            }
         }
         
         //if no projectile, we need to find one...
         else
         {
            mEngageState = ChooseWeapon;
            mTargetInRange = false;
         }
         break;
      }
      
      case AimAtTarget:
      {
         //make sure we're not trying to use an energy weapon while jetting
         if (mWeaponEnergy > 0 && mNavUsingJet)
         {
            mEngageState = ChooseWeapon;
            break;
         }

         //the process order is: fire, then move + aim, we need to aim as if we're
         //shooting next frame
         if (engagingPlayer)
         {
            Point3F nextFrameAimPoint = mAimLocation - mVelocity / 30.0f;
            setAimLocation(nextFrameAimPoint);
         }
         
         //see if we have line of site
         bool clearLOSToTarget = false;
         RayInfo rayInfo;
         Point3F startPt = mMuzzlePosition + mVelocity / 30.0f;  //as if it's next frame already
         Point3F endPt = mAimLocation;
         U32 mask = TerrainObjectType | InteriorObjectType | PlayerObjectType | ForceFieldObjectType;

         //if the player is mounted on a vehicle, mask in vehicle types as well...
         if (player->isMounted())
            mask |= VehicleObjectType;
            
         player->disableCollision();
         if (! gServerContainer.castRay(startPt, endPt, mask, &rayInfo))
            clearLOSToTarget = true;
         else if (engagingPlayer && bool(rayInfo.object) && bool(mTargetPlayer))
         {
            if (rayInfo.object->getId() == mTargetPlayer->getId())
               clearLOSToTarget = true;
         }
         else if (!engagingPlayer && bool(rayInfo.object) && bool(mTargetObject))
         {
            if (rayInfo.object->getId() == mTargetObject->getId())
               clearLOSToTarget = true;
         }
         player->enableCollision();
            
         bool readyToFire = false;
         //if we have a clear LOS, or if we're firing a ballistic weapon and have enough skill to shoot from behind hills
         if (clearLOSToTarget || (mProjectile->isBallistic && mSkillLevel >= 0.7f))
            readyToFire = true;
         
         //else if we're firing a linear weapon with splash damage
         else if (! clearLOSToTarget && ! mProjectile->isBallistic)
         {
            //see if the impact point is within the damage radius of the projectile
            if (mProjectile->hasDamageRadius && (rayInfo.point - mAimLocation).len() < getMax(10.0f, mProjectile->damageRadius))
            {
               //need to see if the impact point has LOS to the target, otherwise, bot will try to shoot through walls...
               //readyToFire = true;
               readyToFire = false;
            }
         }
         if (readyToFire)
         {
            mTargetInRange = true;
            mEngageState = FireWeapon;
         }
         else
         {
            mTargetInRange = false;
            mEngageState = FindTargetPoint;
         }
         
         //if firing is sustained, keep firing
         if (mFiring && mTriggerCounter > 0)
         {
            mTriggerCounter--;
            pressFire();
         }
         
         break;
      }
      
      case FireWeapon:
      {
         //make sure we're not trying to use an energy weapon while jetting
         if (mWeaponEnergy > 0 && mNavUsingJet)
         {
            mEngageState = ChooseWeapon;
            break;
         }

         //if we're firing, we must be in range
         mTargetInRange = true;
         
         //press fire and move to reload
         pressFire();
         mFiring = true;
         mTriggerCounter--;
         
         //see if firing is to be sustained (chaingun, elf gun, etc...)
         if (mTriggerCounter > 0)
         {
            mEngageState = FindTargetPoint;
            mStateCounter = 15; 
         }
         else
         {
            mEngageState = ChooseWeapon;
            mStateCounter = 60;
         }
         
         break;
      }
   }
}

F32 AIConnection::angleDifference(F32 angle1, F32 angle2)
{
   F32 tempAngle1 = getMin(angle1, angle2);
   F32 tempAngle2 = getMax(angle1, angle2);
   F32 difference = getMin(F32(tempAngle2 - tempAngle1),
                           F32(tempAngle1 + M_2PI - tempAngle2));
   return difference;
}

Point3F AIConnection::correctHeading()
{
   Point3F newLocation = mNodeLocation;

   //make sure we're heading in approximately the right direction (+- 30 deg or so)
   //no need to overcorrect if we're not moving very fast
   if (mVelocity2D.len() > 4)
   {
      //if we're heading more than 90 deg in the wrong direction, or we're heading *way* too fast
      //to hit the node, choose the exact opposite direction of the velocity
      if (mDotOffCourse <= 0)
      {
         //find out our current heading (sub 90 deg since our axis is rotated)
         F32 heading = mAtan(mVelocity2D.x, mVelocity2D.y) - (M_PI / 2.0f);
         F32 newHeading = heading + M_PI;

         newLocation.x = mLocation.x + mDistToNode2D * mCos(newHeading);
         newLocation.y = mLocation.y - mDistToNode2D * mSin(newHeading);
      }

      //else mirror the angle through the desired heading
      else
      {
         //find out our current heading (sub 90 deg since our axis is rotated)
         F32 heading = mAtan(mVelocity2D.x, mVelocity2D.y) - (M_PI / 2.0f);

         //find out what angle we're currently off by
         F32 headingDiff = mAcos(mDotOffCourse);

         //determine the "overcompensate" factor
         F32 overCompFactor;
         if (mDotOffCourse > 0.85)
            overCompFactor = 3.0f;
         else
            overCompFactor = 2.0f;
            
         //now see find out which direction
         F32 angle1 = heading + headingDiff;
         F32 angle2 = heading - headingDiff;
         Point3F newVec1(mCos(angle1), -mSin(angle1), 0);
         Point3F newVec2(mCos(angle2), -mSin(angle2), 0);

         //the dot with one of these angles should have a dot near 1 -
         //create the "correcting" angle to overcompensate the headingDiff
         F32 newAngle;
         F32 angle1Dot = get2DDot(mNodeLocation - mLocation, newVec1);
         F32 angle2Dot = get2DDot(mNodeLocation - mLocation, newVec2);
         if (angle1Dot > angle2Dot)
            newAngle = heading + overCompFactor * headingDiff;
         else
            newAngle = heading - overCompFactor * headingDiff;

         //find the new point
         newLocation.x = mLocation.x + mDistToNode2D * mCos(newAngle);
         newLocation.y = mLocation.y - mDistToNode2D * mSin(newAngle);
      }
   }

   return newLocation;
}

Point3F AIConnection::avoidPlayers(Player *player, const Point3F &desiredDestination, bool destIsFinal)
{
   F32 avoidObjectAngle = -1;
   //see if we're near another player
   Box3F queryBox;
   queryBox.min = mLocation;
   queryBox.max = mLocation;
   queryBox.min -= Point3F(2.0f, 2.0f, 0.5f);
   queryBox.max += Point3F(2.0f, 2.0f, 2.5f);
   
   ShapeBase *closestObject = NULL;
   F32 closestDist = 32767;
   Point3F closestLocation;
   SimpleQueryList result;
   //U32 mask = PlayerObjectType;
   U32 mask = ShapeBaseObjectType | StaticTSObjectType;
   gServerContainer.findObjects(queryBox, mask, SimpleQueryList::insertionCallback, S32(&result));
   if (result.mList.size() > 1)
   {
      //find out if the closest person is to the left or right...
      for (S32 i = 0; i < result.mList.size(); i++)
      {
         ShapeBase *neighborObject = static_cast<ShapeBase*>(result.mList[i]);

         //can't bump into yourself
         if (neighborObject->getId() == player->getId())
            continue;

         //if it's not a static TS object, see if it's a shape base with the aiAvoidThis flag set...
         if (!(neighborObject->getType() & StaticTSObjectType))
         {
            ShapeBaseData *db = static_cast<ShapeBaseData*>(neighborObject->getDataBlock());
            if (!db || ! db->aiAvoidThis)
               continue;
         }

         //make sure it's not a corpse!
         if (neighborObject->getType() & CorpseObjectType)
            continue;

         //now we see if it's actually in our way, or if it's just the bounding box...
         Point3F tempVector = neighborObject->getBoxCenter() - player->getBoxCenter();
         if (tempVector.len() > 1.0f)
         {
            tempVector.normalize();
            player->disableCollision();
            RayInfo rayInfo;
            Point3F startPt = player->getBoxCenter();
            Point3F endPt = startPt + (tempVector * 2.0f);
            bool losResult = gServerContainer.castRay(startPt, endPt, neighborObject->getType(), &rayInfo);
            player->enableCollision();
         
            //we disregard this object if we didn't intersect with it
            if (!losResult || (rayInfo.object->getId() != neighborObject->getId()))
               continue;
         }
            
         MatrixF const& tempTransform = neighborObject->getTransform();
         Point3F tempLocation;
         tempTransform.getColumn(3, &tempLocation);
         F32 tempDist = (tempLocation - mLocation).len();
         if (tempDist < closestDist)
         {
            closestDist = tempDist;
            closestObject = neighborObject;
            closestLocation = tempLocation;
         }
      }
   }
   
   //if we didn't find anyone, return the desired dest
   if (! closestObject)
   {
      mAvoidingObject = NULL;
      return desiredDestination;
   }
      
   //otherwise, we've got a neighbor...
   else
   {
      //update the detection table for this bot
      Player *closestPlayer = dynamic_cast<Player*>(closestObject);
      if (bool(closestPlayer))
      {
         GameConnection *closestClient = closestPlayer->getControllingClient();
         if (closestClient)
            clientDetected(closestClient->getId());
      }

      //find out how close we are to our destination 
      Point3F newLocation;
      F32 newHeading;
      F32 distToDest2D = (Point3F(desiredDestination.x, desiredDestination.y, 0.0f) -
                                          Point3F(mLocation.x, mLocation.y, 0.0f)).len();

      //create and normalize the vectors
      Point3F directionVector = desiredDestination - mLocation;
      Point3F neighborVector = closestLocation - mLocation;
      directionVector.z = 0;

      //normalize the vector
      if (directionVector.len() < 0.001f)
         directionVector.set(0, 1, 0);
      directionVector.normalize();

      //make sure we didn't get any -NAN values...
      if (directionVector.x != directionVector.x || directionVector.y != directionVector.y)
         directionVector.set(0, 1, 0);

      neighborVector.z = 0;

      //normalize the vector
      if (neighborVector.len() < 0.001f)
         neighborVector.set(0, 1, 0);
      neighborVector.normalize();

      //make sure we didn't get any -NAN values...
      if (neighborVector.x != neighborVector.x || neighborVector.y != neighborVector.y)
         neighborVector.set(0, 1, 0);

      //dot the direction vector with the neighbor vector - if directly in our path, move at 90 deg...
      F32 neighborDot = get2DDot(neighborVector, directionVector);

      //if the neighbor is behind us, not at all in our way, and we're still moving
      if (! destIsFinal && neighborDot < 0.5)
      {
         mAvoidingObject = NULL;
         return desiredDestination;
      }

      //we've stopped - move at 45 away from target
      else if (destIsFinal)
      {
         F32 neighborHeading = mAtan(neighborVector.x, neighborVector.y) - (M_PI / 2.0f);
         F32 angle1 = neighborHeading + ((M_PI / 2.0f) + (M_PI / 4.0f));
         F32 angle2 = neighborHeading - ((M_PI / 2.0f) + (M_PI / 4.0f));
         Point3F newVec1(mCos(angle1), -mSin(angle1), 0);
         Point3F newVec2(mCos(angle2), -mSin(angle2), 0);

         //find out which newVec will take us furthest away from the neighbor (smallest dot product)
         F32 angle1Dot = get2DDot(neighborVector, newVec1);
         F32 angle2Dot = get2DDot(neighborVector, newVec2);
         if (angle1Dot < angle2Dot)
            newHeading = angle1;
         else
            newHeading = angle2;

         //now that we've chosen the new heading, calculate the new destination location
         newLocation.x = mLocation.x + getMax(6.0f, distToDest2D) * mCos(newHeading);
         newLocation.y = mLocation.y - getMax(6.0f, distToDest2D) * mSin(newHeading);
         newLocation.z = desiredDestination.z;

         //note that since we're already at our final destination, all we need to do is
         //get bumped out of the way, so no need to save the avoidance vars...
         mAvoidingObject = NULL;
      }

      else if ((GameBase*)mAvoidingObject != closestObject || desiredDestination != mAvoidDestinationPoint)
      {

         F32 neighborHeading = mAtan(neighborVector.x, neighborVector.y) - (M_PI / 2.0f);
         F32 angle1 = neighborHeading + (M_PI / 2.0f);
         F32 angle2 = neighborHeading - (M_PI / 2.0f);
         Point3F newVec1(mCos(angle1), -mSin(angle1), 0);
         Point3F newVec2(mCos(angle2), -mSin(angle2), 0);
         F32 angle1Dot = get2DDot(directionVector, newVec1);
         F32 angle2Dot  = get2DDot(directionVector, newVec2);
         if (angle1Dot > angle2Dot)
            newHeading = angle1;
         else
            newHeading = angle2;

         //now that we've chosen the new heading, calculate the new destination location
         newLocation.x = mLocation.x + getMax(6.0f, distToDest2D) * mCos(newHeading);
         newLocation.y = mLocation.y - getMax(6.0f, distToDest2D) * mSin(newHeading);
         newLocation.z = desiredDestination.z;

         //now save off the avoidance vars
         mAvoidingObject = closestObject;
         mAvoidSourcePoint = mLocation;
         mAvoidDestinationPoint = desiredDestination;
         mAvoidMovePoint = newLocation;
         mAvoidForcedPath = false;
      }

      //otherwise we hit this object the last time we were trying to move to our dest
      else
      {
         if ((mAvoidMovePoint - mLocation).len() < 1.5f)
         {
            if (! mAvoidForcedPath)
            {
               mPath.forceSearch();
               mAvoidForcedPath = true;
            }
            newLocation = desiredDestination;
         }
         else
            newLocation = mAvoidMovePoint;
      }

      return newLocation;
   }
}

void AIConnection::processVehicleMovement(Player *player)
{
   //first, kill the nav jetting state machine...
   if (mNavUsingJet)
   {
      mNavUsingJet = false;
      mJetting.reset();
      mPath.forceSearch();
   }

   //now set the aim location if the bot is a passenger.
   //note - if the passenger is in a fixed mounted position, this will be ignored anyways...
   if (player->isMounted() && bool(player->getObjectMount()))
   {
      Point3F vec;
      MatrixF vehicleMat;
      player->getObjectMount()->getMountTransform(player->getMountNode(), &vehicleMat);
      vehicleMat.getColumn(1,&vec);
      F32 vehicleRot = mAtan(vec.x,vec.y) - (M_PI / 2.0f);

      //choose a point so the bot will face the same direction as the vehicle...
      Point3F aimVector(mCos(vehicleRot), -mSin(vehicleRot), 0);
      Point3F aimLocation = mLocation + 30.0f * aimVector;
      aimLocation.z += 2.0f;
      setScriptAimLocation(aimLocation, 1000);
   }

   //finally, the script callback (mainly to handle vehicle firing, etc...)
   char idStr[32];
   dSprintf(idStr, sizeof(idStr), "%d", getId());
   Con::executef(2, "AIProcessVehicle", idStr);
}

void AIConnection::processPilotVehicle(Vehicle *myVehicle)
{
   //eventually, I may move some of the pilot code from ::getMoveList() to here...
   myVehicle;

   //kill the nav jetting state machine...
   if (mNavUsingJet)
   {
      mNavUsingJet = false;
      mJetting.reset();
      mPath.forceSearch();
   }

   //finally, the script callback (mainly to handle vehicle firing, etc...)
   char idStr[32];
   dSprintf(idStr, sizeof(idStr), "%d", getId());
   Con::executef(2, "AIPilotVehicle", idStr);
}

void AIConnection::processMovement(Player *player)
{
   player->updateWorkingCollisionSet();

   // Inform path machinery about our jetting ability.  
   setPathCapabilities(player);

   //if we're avoiding danger, override the current destination and move mode...
   //note, if mEvadingCounter > 45, we use this to slow down reaction time...
   mEvadingCounter--;
   if (mEvadingCounter > 0 && mEvadingCounter < 45)
   {
      //make sure we don't get stuckin in the nav jet state machine
      mJetting.reset();
      mNavUsingJet = false;

      mIsEvading = true;
      setMoveLocation(Point3F(mEvadeLocation.x, mEvadeLocation.y, 0));
      setMoveSpeed(1.0f);
      
      //jump if we can - should encourage skiing
      if (player->canJump())
         pressJump();
      else
         pressJet();
   }
   
   //regular navigation...
   else
   {
      // For debugging movement code of single bot (lh)- 
      S32 focusOnBot = Con::getIntVariable("$AIFocusOnBot");
      if (focusOnBot) 
      {
         if (focusOnBot != getId())
            return;
         else
         {                                                    
            F32 E = mPath.jetWillNeedEnergy(12.0);              
            if (E > 0.0)                                          
               Con::printf("%d wants %f energy", focusOnBot, E);   
         }                                                         
      }

      //reset the nav jet stuff if we've just finished evading
      if (mIsEvading)
      {
         mIsEvading = false;
         mNavUsingJet = false;
         mPath.forceSearch();
      }
         
      mPath.setDestMounted(false);
      if (!mNavUsingJet)
         mMoveMode = mMoveModePending;

      switch (mMoveMode)
      {
         case ModeStop:
         {
            //update the path anyways - mainly to correct aiming probs...
            mPath.updateLocations(mLocation, mMoveDestination);
            mNodeLocation = mPath.getSeekLoc(mVelocity);

            mNavUsingJet = false;
            //even stopped, we need to move out of the way of teammates
            Point3F moveLocation = avoidPlayers(player, mLocation, true);
            if (moveLocation != mLocation)
            {
               setMoveSpeed(0.6f);
               setMoveLocation(moveLocation);
            }
            else
            {
               setMoveSpeed(0.0f);
            }
            break;
         }
      
         case ModeGainHeight:
         {
            mNavUsingJet = false;
            //jump if we can - should encourage skiing
            if (player->canJump())
               pressJump();
            else
               pressJet();
            setMoveLocation(mLocation);
            break;
         }
         
         case ModeMountVehicle:
            //set the path var
            mPath.setDestMounted(true);

            //if we're already mounted, no need to keep moving...
            if (player->isMounted())
            {
               setMoveSpeed(0.0f);
               break;
            }

            //fall through to mode express...
         case ModeWalk:
         case ModeExpress:
         {
            //see if we have a corpse to walk to first
            mPath.updateLocations(mLocation, mMoveDestination);
            mNodeLocation = mPath.getSeekLoc(mVelocity);

            //see if we have a brand new node location...
            if (mNodeLocation != mPrevNodeLocation)
            {
               mPrevNodeLocation = mNodeLocation;
               mInitialLocation = mLocation;
            }

            //else see if we've overshot the node and have to recalc the path
            else
            {
               if (!mNavUsingJet)
               {
                  Point3F vec1 = mInitialLocation - mNodeLocation;
                  Point3F vec2 = mLocation - mNodeLocation;
                  // Moved 2d check here - problems picking up packs with 3d dist check.  
                  // Also, vectors don't need to be normalized if dot compare is with 0.  
                  vec2.z = vec1.z = 0; 
                  if (vec1.lenSquared() > 1.0f && vec2.lenSquared() > 1.0f)
                     if (mDot(vec1, vec2) < 0.0f)
                     {
                        mPrevNodeLocation.set(0, 0, 0);
                        mPath.forceSearch();
                     }
               }
            }

            
            if (mPath.userMustJet() && !mInWater)
            {
               mNavUsingJet = true;
               if (mJetting.status() == AIJetWorking)
               {
                  mJetting.process(this, player);
                  if (mJetting.status() != AIJetWorking)
                  {
                     if (mJetting.status() == AIJetSuccess)
                        mPath.informJetDone();
                     else
                     {
                        if (mMoveMode == ModeMountVehicle)
                           pressJump();
                        mPath.forceSearch();
                     }
                     mJetting.reset();
                  }
                  else if (mJetting.badTimeToSearch())
                     mPath.informJetBusy();
               }
               else
                  mJetting.init(mNodeLocation, mPath.intoMount(), mPath.getJetInfo());
            }
            else
            {
               if (mNavUsingJet)
               {
                  //force a path search after jetting is done...
                  mPath.forceSearch();
                  mNavUsingJet = false;
               }
               //jet if we're falling too quickly
               if (mVelocity.z < -20.0f)
                  pressJet();
            
               //see if we're getting near the end of the path
               //F32 distToEnd = getMax(mPath.distRemaining(2 * mMoveTolerance), (mLocation - mMoveDestination).len());
               bool pathIsCurrent = mPath.isPathCurrent();
               F32 distToEnd = (pathIsCurrent ? mPath.distRemaining(2 * mMoveTolerance) : (2 * mMoveTolerance));
               F32 tolerance = distToEnd > mMoveTolerance ? 0.25f : mMoveTolerance;
               F32 jetEnergy = mPath.jetWillNeedEnergy(30);              
               
               //find out where we're really heading...
               Point3F moveLocation = mNodeLocation;
               if (mDistToNode2D > 0.25f)
                  moveLocation = correctHeading();
                  
               //calculate the distance to where we're moving
               F32 distToMove2D = (Point3F(mLocation.x, mLocation.y, 0) -
                                    Point3F(moveLocation.x, moveLocation.y, 0)).len();
               F32 distToEnd2D = (Point3F(mLocation.x, mLocation.y, 0) -
                                    Point3F(mMoveDestination.x, mMoveDestination.y, 0)).len();
               distToEnd = getMax(distToEnd, distToEnd2D);

               //now adjust the heading if we're bumping into other players
               if (mMoveMode != ModeMountVehicle || distToEnd > 3.0f)
                  moveLocation = avoidPlayers(player, moveLocation, false);
               
               //now move in the calculated direction
               setMoveLocation(moveLocation);

               //see if we're stuck...
               if (distToEnd > tolerance && distToEnd > 1.0f)
               {
                  Point3F checkStuck2D = mLocation - mStuckLocation;
                  checkStuck2D.z = 0.0f;
                  if ((mNodeLocation - mStuckDestination).len() > 1.0f || (checkStuck2D).len() > 1.0f)
                  {
                     mStuckLocation = mLocation;
                     mStuckDestination = mNodeLocation;
                     mStuckTimer = Sim::getCurrentTime();
                  }
                  else if (Sim::getCurrentTime() - mStuckTimer > 2000)
                  {
                     setMoveMode(ModeStuck);
                     mStuckInitialized = false;
                     mStuckTryJump = true;
                     mStuckJumpInitialized = false;
                     return;
                  }
                  else 
                  {
                     // Know we want to be moving- inform Path of how well we're doing- 
                     mPath.informProgress(mVelocity);
                  }
               }

               //look ahead in the path, and see if the next point is collinear with our current destination
               Point3F nextNodeLocation;
               bool isCollinearPath = false;
               if (mPath.getPathNodeLoc(1, nextNodeLocation))
               {
                  F32 tempDot = get2DDot(moveLocation - mLocation, nextNodeLocation - moveLocation);
                  if (tempDot > 0.9f)
                     isCollinearPath = true;
               }
               
               //add jumping and jetting to speed the player up...
               if (distToMove2D > 30.0f)
               {
                  //full speed with skiing
                  setMoveSpeed(1.0f);
      
                  //jump if we can - should encourage skiing
                  if (mHeadingDownhill)
                  {
                     if (player->canJump() && mSkillLevel >= 0.25f)
                        pressJump();
                  }
                  else
                  {
                     if (mEnergyAvailable && mEnergy > jetEnergy && mMoveMode != ModeWalk)
                     {
                        if (player->canJump())
                           pressJump();
                        else
                           pressJet();
                     }
                  }
               }
               else if (distToEnd > 10.0f && isCollinearPath)
               {
                  //full speed - no skiing 
                  setMoveSpeed(1.0f);

                  if (mEnergyAvailable && mEnergy > jetEnergy && mMoveMode != ModeWalk)
                     pressJet();
               }

               else if (distToMove2D > getMax(4.0f, tolerance))
               {
                  //full speed - no skiing
                  setMoveSpeed(1.0f);
               }
               else if (distToMove2D > tolerance)
               {
                  //slower speed with no skiing
                  setMoveSpeed(0.4f);
               }
               else if (distToEnd < tolerance)
               {
                  //stopped - we've arrived
                  setMoveMode(ModeStop);
               }
            }
         }
         break;

         case ModeStuck:
         {
            if (mStuckTryJump)
            {
               if (! mStuckJumpInitialized)
               {
                  mStuckJumpInitialized = true;
                  mStuckJumpTimer = Sim::getCurrentTime();
                  setMoveSpeed(0.0f);
                  setMoveLocation(mLocation);
               }
               else if (Sim::getCurrentTime() - mStuckJumpTimer < 500)
               {
                  if (mEnergy < 0.25f)
                     mStuckJumpTimer = Sim::getCurrentTime();
               }
               else if (Sim::getCurrentTime() - mStuckJumpTimer < 1000)
               {
                  pressJump();
                  pressJet();
               }
               else if (Sim::getCurrentTime() - mStuckJumpTimer < 2000)
               {
                  setMoveLocation(mStuckDestination);
                  setMoveSpeed(1.0f);
                  pressJet();
               }
               else
               {
                  //set the bool so it'll try to back up if we're still stuck...
                  mStuckTryJump = false;

                  //see if that unstuck us...
                  if ((mLocation - mStuckLocation).len() > 1.0f)
                  {
                     mPath.forceSearch();
                     setMoveSpeed(1.0f);
                     setMoveMode(ModeExpress, true);
                     mStuckLocation.set(0, 0, 0);
                     return;
                  }
               }
            }
            
            //else try backing up, and researching the path
            else
            {
               //I guess we're still stuck... try backing up
               if (! mStuckInitialized)
               {
                  mStuckInitialized = true;

                  //find a new direction to move
                  Point3F newLocation;
                  F32 foundLength = 32767;
                  Point3F curHeading = mNodeLocation - mLocation;
                  F32 curAngle = mAtan(curHeading.x, curHeading.y) - (M_PI / 2.0f);

                  for (F32 i = 0.0f; i < 5.0f; i += 1.0f)
                  {
                     //run an LOS to see if the way is clear...
                     F32 testAngle = curAngle + ((i + 2.0) * M_PI / 4.0f);
                     Point3F newVec(mCos(testAngle), -mSin(testAngle), 0.0f);
                     U32 mask = TerrainObjectType | InteriorObjectType;
                     RayInfo rayInfo;
                     Point3F startPt = mLocation;
                     Point3F endPt = mLocation + (4.0f * newVec);
                     startPt.z += 0.3f;
                     endPt.z += 0.3f;
                     if (! gServerContainer.castRay(startPt, endPt, mask, &rayInfo))
                     {
                        newLocation = endPt;
                        break;
                     }
                     else if ((rayInfo.point - startPt).len() < foundLength)
                     {
                        foundLength = (rayInfo.point - startPt).len();
                        newLocation = endPt;
                     }
                  }

                  //set the move location, and set the time stamp
                  mStuckTimer = Sim::getCurrentTime();
                  setMoveLocation(newLocation);
                  setMoveSpeed(1.0f);
               }

               //see if our time has run out, or if we're close to our dest
               if ((mLocation - mMoveLocation).len() < 1.0f || Sim::getCurrentTime() - mStuckTimer > 2000)
               {
                  mStuckLocation.set(0, 0, 0);
                  mPath.informStuck(mStuckLocation, mStuckDestination);
                  setMoveMode(ModeExpress, true);
               }
            }
         }
         break;

      }
   }
   

   //if we're in the middle of a nav jet, aim at the landing place...
   Point3F jetAimLocation;
   if (mNavUsingJet && mJetting.shouldAimAt(jetAimLocation))
   {
      setAimLocation(jetAimLocation);
   }

   //else set the aiming if we don't have a target to shoot at
   else if (mMoveMode != ModeStop && mPath.isPathCurrent() && !bool(mTargetPlayer) && !bool(mTargetObject) && (Sim::getCurrentTime() > mLookAtTargetTimeMS))
   {
      //set the player to always look 10m ahead on the path
      Point3F directionVector;
      Point3F aimPoint;
      F32 distToEnd = mPath.distRemaining(10);
      if (distToEnd >= 10)
         aimPoint = mPath.getLocOnPath(10);
      else
         aimPoint = mNodeLocation;

      //now extend that point by 30m so we don't look down
      directionVector = aimPoint - mLocation;
      directionVector.z = 0;
      F32   directionLen = directionVector.len();
      // LH pulled apart the normalize() to avoid (interupt causing) divides by zero
      if (directionLen < 0.001)
      {
         aimPoint.x = mLocation.x + 30.0f * mCos(mRotation.z);
         aimPoint.y = mLocation.y - 30.0f * mSin(mRotation.z);
         aimPoint.z = mMuzzlePosition.z;
      }
      else
      {                                         // LH- here's the normalize:
         aimPoint.x = mLocation.x + 30.0f * (directionVector.x / directionLen);
         aimPoint.y = mLocation.y + 30.0f * (directionVector.y / directionLen);
         aimPoint.z = mMuzzlePosition.z;
      }
      setAimLocation(aimPoint);
   }
}


void AIConnection::scriptSustainFire(S32 count)
{
   mScriptTriggerCounter = count;
}

static Point3F extractRotation(const MatrixF & matrix)
{
   const F32 * mat = (const F32*)matrix;
   
   Point3F r;   
   r.x = mAsin(mat[MatrixF::idx(2,1)]);

   if(mCos(r.x) != 0.f)
   {
      r.y = mAtan(-mat[MatrixF::idx(2,0)], mat[MatrixF::idx(2,2)]);
      r.z = mAtan(-mat[MatrixF::idx(0,1)], mat[MatrixF::idx(1,1)]);
   }
   else
   {
      r.y = 0.f;
      r.z = mAtan(mat[MatrixF::idx(1,0)], mat[MatrixF::idx(0,0)]);
   }

   return(r);
}

void AIConnection::getMoveList(Move** movePtr,U32* numMoves)
{
   //initialize the move structure and return pointers
   mMove = NullMove;
   *movePtr = &mMove;
   *numMoves = 1;

   //if the system is not enabled, return having set the movePtr to a NullMove
   if (!gAISystemEnabled)
      return;
   
   //movement is done in the object's rotation - create a matrix
   MatrixF moveMatrix;
   moveMatrix.set(EulerF(0, 0, 0));
   moveMatrix.setColumn(3, Point3F(0, 0, 0));
   moveMatrix.transpose();
      
   //aiming variables
   Point3F curLocation;
   F32 curYaw, curPitch;
   F32 newYaw, newPitch;
   Point3F rotation;
   F32 xDiff, yDiff, zDiff;

   //make sure we have a valid control object
   ShapeBase *ctrlObject = getControlObject();
   if (! ctrlObject)
      return;

   //get the control object
   Player *myPlayer = NULL;
   myPlayer = dynamic_cast<Player*>(ctrlObject);

   //make sure we're either controlling a player or a vehicle
   if (myPlayer)
   {
      //if we're dead, no need to call any of the script functions...
      if (! dStricmp(myPlayer->getStateName(), "dead"))
         return;

      //process the ai tasks and steps
      process(myPlayer);
         
      F32 vehicleRot = 0;
      // Find the rotation around the Z axis from the mounted vehicle
      if (myPlayer->isMounted() && bool(myPlayer->getObjectMount()))
      {
         Point3F vec, pos;
         MatrixF vehicleMat;
         myPlayer->getObjectMount()->getMountTransform(myPlayer->getMountNode(), &vehicleMat);
         vehicleMat.getColumn(1,&vec);
         vehicleMat.getColumn(3,&pos);
         vehicleRot = -mAtan(-vec.x,vec.y);
      }

      //get the current location (global coords)   
      MatrixF const& myTransform = myPlayer->getTransform();
      myTransform.getColumn(3, &curLocation);
      
      //initialize the aiming variables
      rotation = myPlayer->getRotation();
      Point3F headRotation = myPlayer->getHeadRotation();
      curYaw = rotation.z + vehicleRot;
      curPitch = headRotation.x;
      xDiff = mAimLocation.x - curLocation.x;
      yDiff = mAimLocation.y - curLocation.y;

      //first do Yaw
      if (! isZero(xDiff) || ! isZero(yDiff))
      {
         //use the cur yaw between -Pi and Pi
         while (curYaw > M_2PI)
            curYaw -= M_2PI;
         while (curYaw < -M_2PI)
            curYaw += M_2PI;

         //find the new yaw
         newYaw = mAtan(xDiff, yDiff);
            
         //find the yaw diff 
         F32 yawDiff = newYaw - curYaw;

         //make it between 0 and 2PI
         if (yawDiff < 0.0f)
            yawDiff += M_2PI;
         else if (yawDiff >= M_2PI)
            yawDiff -= M_2PI;

         //now make sure we take the short way around the circle
         if (yawDiff > M_PI)
            yawDiff -= M_2PI;
         else if (yawDiff < -M_PI)
            yawDiff += M_2PI;

         mMove.yaw = yawDiff;

         //set up the movement matrix
         moveMatrix.set(EulerF(0, 0, newYaw));
      }
      else
         moveMatrix.set(EulerF(0, 0, curYaw));
      
      //next do pitch
//    F32 horzDist = Point2F(yDiff, xDiff).len();
//    F32 horzDist = Point2F(mAimLocation.x - mMuzzlePosition.x, mAimLocation.y - mMuzzlePosition.y).len();
      F32 horzDist = Point2F(mAimLocation.x - mEyePosition.x, mAimLocation.y - mEyePosition.y).len();
      if (! isZero(horzDist))
      {
         //we shoot from the gun, not the eye...
//       F32 vertDist = mAimLocation.z - mMuzzlePosition.z;
         F32 vertDist = mAimLocation.z - mEyePosition.z;
         
         newPitch = mAtan(horzDist, vertDist) - (M_PI / 2.0f);
         
         F32 pitchDiff = newPitch - curPitch;
         mMove.pitch = pitchDiff;
      }
      
      //finally, move towards mMoveLocation
      xDiff = mMoveLocation.x - curLocation.x;
      yDiff = mMoveLocation.y - curLocation.y;
      if (((mFabs(xDiff) > 0) || (mFabs(yDiff) > 0)) && (! isZero(mMoveSpeed)))
      {
         if (isZero(xDiff))
            mMove.y = (curLocation.y > mMoveLocation.y ? -mMoveSpeed : mMoveSpeed);
         else if (isZero(yDiff))
            mMove.x = (curLocation.x > mMoveLocation.x ? -mMoveSpeed : mMoveSpeed);
         else if (mFabs(xDiff) > mFabs(yDiff))
         {
            F32 value = mFabs(yDiff / xDiff) * mMoveSpeed;
            mMove.y = (curLocation.y > mMoveLocation.y ? -value : value);
            mMove.x = (curLocation.x > mMoveLocation.x ? -mMoveSpeed : mMoveSpeed);
         }
         else
         {
            F32 value = mFabs(xDiff / yDiff) * mMoveSpeed;
            mMove.x = (curLocation.x > mMoveLocation.x ? -value : value);
            mMove.y = (curLocation.y > mMoveLocation.y ? -mMoveSpeed : mMoveSpeed);
         }
         
         //now multiply the move vector by the transpose of the object rotation matrix
         moveMatrix.transpose();
         Point3F newMove;
         moveMatrix.mulP(Point3F(mMove.x, mMove.y, 0), &newMove);
         
         //and sub the result back in the move structure
         mMove.x = newMove.x;
         mMove.y = newMove.y;
      }
   }

   else
   {
      Vehicle *myVehicle;
      myVehicle = dynamic_cast<Vehicle*>(ctrlObject);
      if (myVehicle)
      {
         //process the ai tasks and steps (well, not steps get ignored for piloting vehicles...)
         process(myVehicle);
         
         //get the current location (global coords)   
         MatrixF const& myTransform = myVehicle->getTransform();
         myTransform.getColumn(3, &curLocation);
         
         //initialize the aiming variables
         rotation = extractRotation(myTransform);
         xDiff = mPilotAimLocation.x - curLocation.x;
         yDiff = mPilotAimLocation.y - curLocation.y;
         zDiff = mPilotAimLocation.z - curLocation.z;
         F32 dist2D = Point2F(yDiff, xDiff).len();

         //first do Yaw
         curYaw = rotation.z;
         F32 yawDiff = 0;
         Point3F velocity = myVehicle->getVelocity();
         Point3F velocity2D(velocity.x, velocity.y, 0.0f);
         F32 velocityDot = get2DDot(velocity2D, (mPilotDestination - curLocation));

         //if ((mFabs(xDiff) > 5.0f || mFabs(yDiff) > 5.0f) && (mPilotSpeed == 0.0f || (velocityDot < 0.8f && velocity2D.len() > 5.0f)))
         if (mFabs(xDiff) > 5.0f || mFabs(yDiff) > 5.0f)
         {
            //use the cur yaw between -Pi and Pi
            while (curYaw > M_2PI)
               curYaw -= M_2PI;
            while (curYaw < -M_2PI)
               curYaw += M_2PI;

            //find the new yaw
            newYaw = mAtan(xDiff, yDiff);

            //find the yaw diff 
            yawDiff = newYaw - curYaw;

            //make it between 0 and 2PI
            if (yawDiff < 0.0f)
               yawDiff += M_2PI;
            else if (yawDiff >= M_2PI)
               yawDiff -= M_2PI;

            //now make sure we take the short way around the circle
            if (yawDiff > M_PI)
               yawDiff -= M_2PI;
            else if (yawDiff < -M_PI)
               yawDiff += M_2PI;

            //set up the movement matrix
            mMove.yaw = yawDiff;
            moveMatrix.set(EulerF(0, 0, newYaw));
         }
         else
            moveMatrix.set(EulerF(0, 0, curYaw));

         //both pitch and destination should use the pilotDestination, not the aim location
         xDiff = mPilotDestination.x - curLocation.x;
         yDiff = mPilotDestination.y - curLocation.y;
         zDiff = mPilotDestination.z - curLocation.z;
         dist2D = Point2F(yDiff, xDiff).len();

         //get the yAxis - the vertical slope of which is our current pitch
         Point3F yAxis;
         myTransform.mulV(Point3F(0, 1, 0), &yAxis);
         yAxis.normalize();
         curPitch = mAtan(1.0f, yAxis.z) - (M_PI / 2.0f);

         //if we're below our destination, and don't have enough vertical velocity, pitch up...
         newPitch = mAtan(dist2D, zDiff) - (M_PI / 2.0f);
         newPitch = getMax(mPitchUpMax, getMin(mPitchDownMax, newPitch));

         //use a pitch delta to prevent from pitching too fast...
         S32 pitchDir = 0;
         F32 pitchDelta = curPitch - mPreviousPitch;
         mPreviousPitch = curPitch;

         //make sure we've got some horizontal speed
         if (mPilotSpeed > 0.0f && velocity2D.len() > 5.0f)
         {

            //at our current rate of ascent/descent, 
            F32 timeToDest = dist2D / velocity2D.len();
            F32 vertDist = velocity.z * timeToDest;
            
            //if we're below our dest and our vertical lift is too low, pitch up
            if (zDiff > 5.0f && (vertDist < zDiff - 2.0f || curPitch > 0.0f))
               pitchDir = 1;

            //else if we're higher than our dest, and our vertical drop is too low, pitch down
            else if (zDiff < -5.0f && (vertDist > zDiff + 2.0f || curPitch < 0.0f))
               pitchDir = -1;

            //make sure we're within our pitch limits, and that we're not pitching too fast
            if (pitchDir > 0 && (curPitch < mPitchUpMax || pitchDelta < -mPitchIncMax))
               pitchDir = -1;
            else if (pitchDir < 0 && (curPitch > mPitchDownMax || pitchDelta > mPitchIncMax))
               pitchDir = 1;

            //if we didn't meet the above conditions, our velocity is on track, and we should level out
            if (pitchDir == 0)
            {
               //if we're pitched up, and our change in pitch is also in the up direction
               if (curPitch < 0.0f && pitchDelta < 0)
                  pitchDir = -1;
               else if (curPitch > 0.0f && pitchDelta > 0)
                  pitchDir = 1;
            }

            //now put the pitch into the pitch move struct
            if (pitchDir > 0)
               mMove.pitch = mPitchUpMax;
            else if (pitchDir < 0)
               mMove.pitch = mPitchDownMax;
         }

         //now calculate the actual movement towards mPilotDestination
         //first calculate speed - scale by whether we're already heading in the right direction...
         F32 speed = 0.0f;
         if ((mFabs(xDiff) > 5.0f) || (mFabs(yDiff) > 5.0f))
         {
            //if we're heading within a 45 deg cone, add some speed
            if (mFabs(yawDiff < 0.8f))
               speed = 1.0f - (mFabs(yawDiff) / M_PI);
         }

         //slow our movespeed down according to how far we are from our target...
         if (speed > 0.0f && dist2D >= 5.0f && dist2D <= 20.0f)
            speed *= 0.3f * getMin(0.7f, dist2D / 20.0f); 

         //see if we need to reverse thrust...
         if (dist2D < 6.0f && velocity2D.len() > 5.0f && mFabs(velocityDot) >= 0.78f)
            speed = getMax(-1.0f, -2.0f * speed);

         //now factor in our pilot speed
         speed *= mPilotSpeed;

         //see if we should add some jetting
         if (velocity2D.len() < 8.0f && curPitch <= 0.05f && zDiff > 5.0f)
            mTriggers[JetTrigger] = true;

         //set some debugging vars
         mVehicleLocation = curLocation;
         mCurrentPitch = curPitch;
         mDesiredPitch = newPitch;
         mPitchIncrement = mMove.pitch;
         mPilotDistToDest2D = dist2D;
         mPilotCurVelocity = myVehicle->getVelocity().len();
         mPilotCurThrust = speed;
         mCurrentYaw = curYaw;

         //fill in the move struction
         if (((mFabs(xDiff) > 5.0f) || (mFabs(yDiff) > 5.0f)) && (! isZero(speed)))
         {
            if (mFabs(xDiff) <= 5.0f)
               mMove.y = (curLocation.y > mPilotDestination.y ? -speed : speed);
            else if (mFabs(yDiff) <= 5.0f)
               mMove.x = (curLocation.x > mPilotDestination.x ? -speed : speed);
            else if (mFabs(xDiff) > mFabs(yDiff))
            {
               F32 value = mFabs(yDiff / xDiff) * speed;
               mMove.y = (curLocation.y > mPilotDestination.y ? -value : value);
               mMove.x = (curLocation.x > mPilotDestination.x ? -speed : speed);
            }
            else
            {
               F32 value = mFabs(xDiff / yDiff) * speed;
               mMove.x = (curLocation.x > mPilotDestination.x ? -value : value);
               mMove.y = (curLocation.y > mPilotDestination.y ? -speed : speed);
            }
            
            //now multiply the move vector by the transpose of the object rotation matrix
            moveMatrix.transpose();
            Point3F newMove;
            moveMatrix.mulP(Point3F(mMove.x, mMove.y, 0), &newMove);
            
            //and sub the result back in the move structure
            mMove.x = newMove.x;
            mMove.y = newMove.y;
         }
      }
   }
   
   //copy the triggers into the move
   for (int i = 0; i < MaxTriggerKeys; i++)
   {
      mMove.trigger[i] = mTriggers[i];
      mTriggers[i] = false;
   }
}

//-----------------------------------------------------------------------------
//STEP AND TASK FUNCTIONS

void AIConnection::clearStep()
{
   if (mStep)
      mStep->deleteObject();
   mStep = NULL;
}

void AIConnection::setStep(AIStep *step)
{
   clearStep();
   mStep = step;
}

const char *AIConnection::getStepStatus()
{
   if (! mStep)
      return "Finished";
   else
   {
      int status = mStep->getStatus();
      switch (status)
      {
         case AIStep::InProgress:
            return "InProgress";
            
         case AIStep::Failed:
            return "Failed";
            
         case AIStep::Finished:
         default:
            return "Finished";
      }
   }
}

const char *AIConnection::getStepName()
{
   if (! mStep)
      return "NONE";
   else
   {
      const char *stepName = mStep->getName();
      if (!stepName || !stepName[0])
         return "NONE";
      else
         return stepName;
   }
}

void AIConnection::clearTasks()
{
   while (mTaskList.size())
   {
      AITask *tempPtr = mTaskList[0];
      mTaskList.pop_front();
      tempPtr->deleteObject();
   }
   mCurrentTask = NULL;
}

void AIConnection::addTask(AITask *task)
{
   if (! task)
      return;
      
   mTaskList.push_back(task);
}

void AIConnection::removeTask(S32 id)
{
   for (S32 i = 0; i < mTaskList.size(); i++)
   {
      if (mTaskList[i]->getId() == id)
      {
         if (mCurrentTask == mTaskList[i])
         {
            mCurrentTask->retire(this);
            mCurrentTask = NULL;
         }
         mTaskList[i]->deleteObject();
         mTaskList.erase(i);
         break;
      }
   }
}

void AIConnection::listTasks()
{
   for (S32 i = 0; i < mTaskList.size(); i++)
   {
      Con::printf("%d: %s", mTaskList[i]->getId(), mTaskList[i]->getName());
   }
}

void AIConnection::missionCycleCleanup()
{
   setMoveMode(ModeStop);
   clearTasks();
   clearStep();
   mPath.forceSearch();
   mPath.missionCycleCleanup();
}

