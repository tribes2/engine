//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AICONNECTION_H_
#define _AICONNECTION_H_

#ifndef _MPOINT_H_
#include "math/mPoint.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GAMECONNECTION_H_
#include "game/gameConnection.h"
#endif
#ifndef _MOVEMANAGER_H_
#include "game/moveManager.h"
#endif
#ifndef _PLAYER_H_
#include "game/player.h"
#endif
#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif
#ifndef _GRAPH_H_
#include "ai/graph.h"
#endif
#ifndef _AINAVJETTING_H_
#include "ai/aiNavJetting.h"
#endif

//#define DEBUG_AI 1

class AIStep;
class AITask;
class AIConnection : public GameConnection
{
   typedef GameConnection Parent;

   //moves for the AI are not queued up, since they are moved on the server
   Move mMove;

   F32 mMoveSpeed;
   S32 mMoveMode;
   S32 mMoveModePending;
   F32 mMoveTolerance;           //how close to the destination before we stop
   Point3F mMoveDestination;     //this is the desired final destination
   Point3F mNodeLocation;        //this is a node along the path towards our final destination
   Point3F mMoveLocation;        //this is direction to take to arrive at the node, or the destination
   Point3F mEvadeLocation;       //if mEvadingCounter > 0, this location will override the above two
   Point3F mAimLocation;
   S32 mLookAtTargetTimeMS;      //used to slow down the aim twitching...

   //vars used to determine when to force a path recalc
   Point3F mPrevNodeLocation;
   Point3F mInitialLocation;

   //Engagement vars - should be able to shoot at anyone regardless of step/task
   SimObjectPtr<GameConnection> mEngageTarget;
   enum EngageStates
   {
      ChooseWeapon,
      OutOfRange,
      ReloadWeapon,
      FindTargetPoint,
      AimAtTarget,
      FireWeapon,
      Finished
   };
   S32 mEngageState;
   S32 mStateCounter;
   S32 mDelayCounter;
   S32 mPackCheckCounter;
   bool mAimAtLazedTarget;
   S32 mTurretMountedId;

   //vehicle piloting vars
   Point3F mPilotDestination;
   Point3F mPilotAimLocation;
   F32 mPilotSpeed;
   F32 mPitchUpMax;
   F32 mPitchDownMax;
   F32 mPitchIncMax;

   //piloting debug vars
   Point3F mVehicleLocation;
   F32 mPilotDistToDest2D;
   F32 mPilotCurVelocity;
   F32 mPilotCurThrust;
   F32 mCurrentPitch;
   F32 mPreviousPitch;
   F32 mDesiredPitch;
   F32 mPitchIncrement;
   F32 mCurrentYaw;

   StringTableEntry mProjectileName;
   ProjectileData *mProjectile;

   SimObjectPtr<Projectile> mEnemyProjectile;
   S32 mProjectileCounter;
   S32 mEvadingCounter;
   bool mIsEvading;
   Point3F mImpactLocation;         //not really used for anything but debugging...

   SimObjectPtr<GameConnection> mVictim;
   SimObjectPtr<Player> mCorpse;
   Point3F mCorpseLocation;
   S32 mVictimTime;

   //if no engagement target, we may have a gamebase object to destroy/repair
   SimObjectPtr<ShapeBase> mTargetObject;
   S32 mObjectMode;

   //these vars are used for both engagetarget and targetobject
   F32 mRangeToTarget;
   S32 mCheckTargetLOSCounter;
   bool mTargetInRange;
   bool mTargetInSight;
   
public:
   enum ObjectModes
   {
      DestroyObject = 0,
      RepairObject,
      LazeObject,
      MortarObject,
      MissileVehicle,
      MissileNoLock,
      AttackMode1,
      AttackMode2,
      AttackMode3,
      AttackMode4,
      NumObjectModes
   };

private:
   //NAV Graph vars
   NavigationPath mPath;
   AIJetting   mJetting;
   Point3F  mPathDest;
   bool     mNewPath;
   bool     mNavUsingJet;

public:
   S32 mEngageMinDistance;
   S32 mEngageMaxDistance;
   S32 mTriggerCounter;
   S32 mScriptTriggerCounter;
   F32 mWeaponEnergy;
   F32 mWeaponErrorFactor;
   bool mFiring;
   ShapeBaseImageData* mMountedImage;

   //temp vars recalculated and used each process loop
   Point3F mLocation;
   Point3F mVelocity;
   Point3F mVelocity2D;
   Point3F mRotation;
   Point3F mHeadRotation;
   Point3F mMuzzlePosition;
   Point3F mEyePosition;
   F32 mDamage;
   F32 mEnergy;
   F32 mEnergyReserve;        //navigation must not use energy below the reserve
   F32 mEnergyFloat;          //used to determine whether we are recharging
   bool mEnergyRecharge;
   bool mEnergyAvailable;
   bool mHeadingDownhill;
   bool mInWater;
   bool mOutdoors;
   F32 mDotOffCourse;
   
   Player *mTargetPlayer;
   Point3F mTargLocation;
   Point3F mTargVelocity;
   Point3F mTargVelocity2D;
   Point3F mTargRotation;
   F32 mTargEnergy;
   F32 mTargDamage;
   bool mTargInWater;

   Point3F mObjectLocation;
   bool mObjectInWater;

   //skill related vars
   F32 mSkillLevel;
   S32 mTargStillTimeMS;
   F32 mTargPrevTimeMS;
   Point3F mTargPrevLocation[4];
   S32 mChangeWeaponCounter;

   F32 mDistToTarg2D;
   F32 mDistToNode2D;
   F32 mDistToObject2D;

   enum
   {
      ModeStop = 0,
      ModeWalk,
      ModeGainHeight,
      ModeExpress,
      ModeMountVehicle,
      ModeStuck,
      ModeCount
   };

   //vars to determine/handle if we're stuck
   Point3F mStuckLocation;
   Point3F mStuckDestination;
   bool mStuckInitialized;
   S32 mStuckTimer;
   bool mStuckTryJump;
   bool mStuckJumpInitialized;
   S32 mStuckJumpTimer;

   SimObjectPtr<GameBase> mAvoidingObject;
   Point3F mAvoidSourcePoint;
   Point3F mAvoidDestinationPoint;
   Point3F mAvoidMovePoint;
   bool mAvoidForcedPath;
   
protected: 
   void  scriptProcessEngagement();
   void  scriptChooseEngageWeapon(F32 distToTarg);
   void  scriptChooseObjectWeapon(F32 distToTarg);
   void  aiDebugStuff();

private:
   enum
   {
      FireTrigger = 0,
      JumpTrigger = 2,
      JetTrigger = 3,
      GrenadeTrigger = 4,
      MineTrigger = 5
   };
   bool mTriggers[MaxTriggerKeys];

   //Step and Task members
   AIStep *mStep;

   Vector<AITask*> mTaskList;
   AITask *mCurrentTask;
   S32 mCurrentTaskTime;

   //DETECTION MEMBERS
   struct PlayerDetectionEntry
   {
      S32 playerId;     
      bool playerLOS;
      S32 playerLOSTime;
      Point3F playerLastPosition; 
   };
   Vector<PlayerDetectionEntry> mPlayerDetectionTable;
   S32 mPlayerDetectionIndex;
   S32 mPlayerDetectionCounter;
   S32 mDetectHiddenPeriod;      //even without LOS, the player will know where the enemy is until
                                 //this detectHiddenPeriod is over... after that, they'll only use
                                 //the last known location...
   S32 mBlindedTimer;            //used 

public:
   AIConnection();
   ~AIConnection();

public:   
   DECLARE_CONOBJECT(AIConnection);
   static void consoleInit();

   F32 fixRadAngle(F32 angle);
   static F32 get2DDot(const Point3F &vec1, const Point3F &vec2);
   static F32 get2DAngle(const Point3F &endPt, const Point3F &basePt);
   F32 getOutdoorRadius(const Point3F &location);  //returns the outdoor freedom radius, or -1 if indoors

   void setSkillLevel(F32 level);
   F32 getSkillLevel() { return mSkillLevel; }

   void setMoveSpeed(F32 speed);
   void setMoveMode(S32 mode, bool abortStuckCode = false);
   void setMoveTolerance(F32 tolerance);
   void setMoveLocation(const Point3F &location);
   void setMoveDestination(const Point3F &location);
   void setTurretMounted(S32 turretId) { mTurretMountedId = turretId; }
   void setAimLocation(const Point3F &location);
   void setWeaponInfo(StringTableEntry projectile, S32 minDist, S32 maxDist, S32 triggerCount, F32 energyRequired, F32 errorFactor = 1.0f);
   void setEnergyLevels(F32 eReserve, F32 eFloat = 0.10f);

   bool setScriptAimLocation(const Point3F &location, S32 duration);  //returns false if we have a target object/player

   //here are the vehicle piloting methods
   void setPilotPitchRange(F32 pitchUpMax, F32 pitchDownMax, F32 pitchIncMax);
   void setPilotDestination(const Point3F &dest, F32 maxSpeed);
   void setPilotAimLocation(const Point3F &dest);

   //bool findCrossVector(const Point3F &v1, const Point3F &v2, Point3F *result);
   Point3F dopeAimLocation(const Point3F &startLocation, const Point3F &aimLocation);

   F32 angleDifference(F32 angle1, F32 angle2);
   Point3F correctHeading();
   Point3F avoidPlayers(Player *player, const Point3F &desiredDestination, bool destIsFinal);

   void setEngageTarget(GameConnection *target);
   S32 getEngageTarget();
   void setTargetObject(ShapeBase *targetObject, F32 range = 35.0f, S32 objectMode = DestroyObject);
   S32 getTargetObject();
   bool TargetInRange() { return mTargetInRange || mTargetInSight; }
   bool TargetInSight() { return mTargetInSight; }

   void setVictim(GameConnection *victim, Player *corpse);
   S32 getVictimCorpse();
   S32 getVictimTime();

   F32 getMoveSpeed() { return mMoveSpeed; }
   S32 getMoveMode() { return mMoveMode; }
   F32 getMoveTolerance() { return mMoveTolerance; }
   Point3F getMoveDestination() { return mMoveDestination; }
   Point3F getMoveLocation() { return mMoveLocation; }
   Point3F getAimLocation() { return mAimLocation; }
	bool scriptIsAiming() { return mLookAtTargetTimeMS > Sim::getCurrentTime(); }

   void setPathDest(const Point3F * dest = NULL);
   const Point3F * getPathDest();
   void setPathCapabilities(Player * player);
   F32 getPathDistance(const Point3F &destination, const Point3F &source);
   F32 getPathDistRemaining(F32 maxDist);
   Point3F getLOSLocation(const Point3F &targetPoint, F32 minDistance, F32 maxDistance, const Point3F &nearPoint);
   Point3F getHideLocation(const Point3F &targetPoint, F32 range, const Point3F &nearPoint, F32 hideLength);

   void scriptSustainFire(S32 count = 1);
   void pressFire(bool value = true) { mTriggers[FireTrigger] = value; }
   void pressJump() { mTriggers[JumpTrigger] = true; }
   void pressJet() { mTriggers[JetTrigger] = true; }
   void pressGrenade() { mTriggers[GrenadeTrigger] = true; }
   void pressMine() { mTriggers[MineTrigger] = true; }

   bool isFiring() { return mTriggers[FireTrigger]; }
   bool isJumping() { return mTriggers[JumpTrigger]; }
   bool isJetting() { return mTriggers[JetTrigger]; }

   void getMoveList(Move**,U32* numMoves);
   void clearMoves(U32) { }
   bool areMovesPending() { return true; }

   //function to update tasks, etc...
   void process(ShapeBase *ctrlObject);
   void processMovement(Player *player);
   void processVehicleMovement(Player *player);
   void processPilotVehicle(Vehicle *myVehicle);
   void processEngagement(Player *player);
   void setEvadeLocation(const Point3F &dangerLocation, S32 durationTicks = 0);
   void initProcessVars(Player *player);
   void updateDetectionTable(Player *player);
   bool hasLOSToClient(S32 clientId, S32 &losTime, Point3F &lastLocation);
   void clientDetected(S32 targId);
   void setDetectPeriod(S32 value) { mDetectHiddenPeriod = value; }
   S32 getDetectPeriod() { return mDetectHiddenPeriod; }
   void setBlinded(S32 duration);

   //step and task methods
   void setStep(AIStep *step);
   void clearStep();
   const char *getStepStatus();
   const char *getStepName();

   void clearTasks();
   void addTask(AITask *task);
   void removeTask(S32 id);
   AITask *getCurrentTask() { return mCurrentTask; }
   S32 getTaskTime() { return mCurrentTaskTime; }
   void listTasks();

   void missionCycleCleanup();
};

extern const char *gTargetObjectMode[AIConnection::NumObjectModes];

class AISlicer 
{
      // MRandomLCG  mRand;
      // bool        mEnabled;
      U32         mDelay;
      U32         mLastTime;
      // U32         mCapPileUp;
      U32         mDebugTotal;
      F32         mBudget;
   
   public:
      AISlicer();
      void  init(U32 delay = 22, S32 maxPerTick = 1);
      void  reset();
      bool  ready(S32 &counter, S32 resetVal);
};

extern AISlicer gCalcWeightSlicer;
extern AISlicer gScriptEngageSlicer;
extern bool gAISystemEnabled;


#endif
