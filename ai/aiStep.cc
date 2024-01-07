//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "core/realComp.h"
#include "console/simBase.h"
#include "ai/aiConnection.h"
#include "game/player.h"
#include "game/projectile.h"
#include "math/mRandom.h"
#include "ai/aiStep.h"

AIStep::AIStep()
{
   mStatus = InProgress;
}

void AIStep::process(AIConnection *client, Player *player)
{
   //only process if we're still "in progress"
   if (mStatus != InProgress)
      return;
      
   //make sure we have a client and a player
   if (!client || !player)
      mStatus = Failed;
      
   //make sure we're not dead
   if (! dStricmp(player->getStateName(), "dead"))
      mStatus = Failed;
}

//-----------------------------------------------------------------------------
//ESCORT step
AIStepEscort::AIStepEscort(GameConnection *clientToEscort)
{
   mInitialized = false;
   mClientToEscort = clientToEscort;
	mResetDestinationCounter = 0;
}

void AIStepEscort::process(AIConnection *client, Player *player)
{
   Parent::process(client, player);
   if (mStatus != InProgress)
      return;
      
   //make sure we have a valid target to escort   
   Player *targetPlayer = NULL;
   if ((! bool(mClientToEscort)) || (! mClientToEscort->getControlObject()))
      mStatus = Failed;
   else
      targetPlayer = dynamic_cast<Player*>(mClientToEscort->getControlObject());   
   if (! targetPlayer)
   {
      mStatus = Failed;
      return;
   }
   
   //set the energy levels
   if (! mInitialized)
   {
      mInitialized = true;
		if (targetPlayer->isMounted())
	      client->setMoveMode(AIConnection::ModeMountVehicle);
		else
	      client->setMoveMode(AIConnection::ModeExpress);
      client->setMoveTolerance(4.0f);
      client->setEnergyLevels(0.05f, 0.15f);
   }
   
   //get the target's current location (global coords)  
   MatrixF const& targetTransform = targetPlayer->getTransform();
   Point3F targetLocation;
   targetTransform.getColumn(3, &targetLocation);

	//go back to using the euclidean distance
	F32 distToTarget = getMax(client->getPathDistRemaining(20), (client->mLocation - targetLocation).len());
   
	//cut down on the number of path searches
	if (--mResetDestinationCounter <= 0)
	{
		mResetDestinationCounter = 5;
	   client->setMoveDestination(targetLocation);      
	}

	//get the current time
	S32 curTime = Sim::getCurrentTime();

   //see if we're close enough to the target yet
   if (distToTarget < 10.0f)
   {
      mProximityBuffer = true;
		if (client->getMoveMode() != AIConnection::ModeStop)
		{
	      client->setMoveMode(AIConnection::ModeStop);
			mStoppedTime = curTime;
			mIdleStarted = false;
		}
   }
   else if (distToTarget < 16.0f)
   {
      if (mProximityBuffer)
		{
			if (client->getMoveMode() != AIConnection::ModeStop)
			{
		      client->setMoveMode(AIConnection::ModeStop);
				mStoppedTime = curTime;
				mIdleStarted = false;
			}
		}
      else
		{
			//make sure we aim at the player momentarily...
			if (client->getMoveMode() == AIConnection::ModeStop)
				client->setScriptAimLocation(Point3F(targetLocation.x, targetLocation.y, targetLocation.z + 1.6f), 500);

		if (targetPlayer->isMounted())
	      client->setMoveMode(AIConnection::ModeMountVehicle);
		else
	      client->setMoveMode(AIConnection::ModeExpress);
		}
   }
   else
   {
		//make sure we aim at the player momentarily...
		if (client->getMoveMode() == AIConnection::ModeStop)
			client->setScriptAimLocation(Point3F(targetLocation.x, targetLocation.y, targetLocation.z + 1.6f), 500);
      mProximityBuffer = false;
		if (targetPlayer->isMounted())
	      client->setMoveMode(AIConnection::ModeMountVehicle);
		else
	      client->setMoveMode(AIConnection::ModeExpress);
   }

	//now see if it's time to "idle"
	if (client->getMoveMode() == AIConnection::ModeStop && curTime - mStoppedTime > 5000)
	{
		//init the idle vars
		if (!mIdleStarted)
		{
			mIdleStarted = true;
			mChokePoints.clear();
	      NavigationGraph::getChokePoints(client->mLocation, mChokePoints, 10, 65);
			mIdleNextTime = 0;
		}

		//see if it's time to look around some more
		if (curTime > mIdleNextTime)
		{
			mIdleNextTime = curTime + 2000 + gRandGen.randF() * 3000;

			//see if we should look or play an animation
			if (gRandGen.randF() < 0.06f)
			{
			   player->setActionThread("pda", false, true, false);
			}
			else
			{
				Point3F lookLocation;
				if (mChokePoints.size() == 0)
				{
					if (gRandGen.randF() < 0.3f)
						lookLocation = targetLocation;
					else
						lookLocation = client->mLocation;
				}
				else
				{
					S32 index = S32(gRandGen.randF() * (mChokePoints.size() + 0.9));
					if (index == mChokePoints.size())
						lookLocation = targetLocation;
					else
						lookLocation = mChokePoints[index];
				}

				//add some noise to the lookLocation
				Point3F tempVector = lookLocation - client->mLocation;
				tempVector.z = 0;
				if (tempVector.len() < 0.1f)
					tempVector.set(1, 0, 0);
				tempVector.normalize();
				tempVector *= 10.0f;
				F32 noise = 2.0f;
				tempVector.x += -noise + gRandGen.randF() * 2 * noise;
				tempVector.y += -noise + gRandGen.randF() * 2 * noise;
				tempVector.z = 1.5f + gRandGen.randF() * 2 * noise;
				Point3F newAimLocation = client->mLocation + tempVector;

				//now aim at that point
            if (!client->scriptIsAiming())
				   client->setScriptAimLocation(newAimLocation, 3000);
			}
		}
	}
}

//-----------------------------------------------------------------------------
//ENGAGE step
AIStepEngage::AIStepEngage(GameConnection *target)
{
   mInitialized = false;
   mTarget = target;
   mStraifeCounter = 0;
	mPauseCounter = 0;
	mPausing = false;
   mCheckLOSCounter = 0;
   mClearLOSToTarget = false;

	mSearching = false;
   mSearchInitialized = false;
   
	mUsingEnergyWeapon = false;
	mEnergyWeaponRecharge = 0.0f;
}

Point3F AIStepEngage::findStraifeLocation(AIConnection *client)
{
   Point3F newLocation;
   Point3F tempV = client->mLocation - client->mTargLocation;
   Point3F newLocationVector;
	if (tempV.len() < 0.001f)
		tempV.set(0, 1, 0);
   tempV.normalize();
   mCross(tempV, Point3F(0, 0, 1), &newLocationVector);
	if (newLocationVector.len() < 0.001f)
		newLocationVector.set(0, 1, 0);
   newLocationVector.normalize();
   
   F32 newLocationLength = newLocationVector.len();
   if (newLocationLength > 0.9f && newLocationLength < 1.1f)
   {
      //if we're not moving, choose randomly whether we straife right or left
      if (client->mVelocity2D.len() < 1.0f)
         newLocation = client->mTargLocation + (newLocationVector * client->mEngageMinDistance);
      else
      {
         //otherwise, choose the point closest to the direction we're already moving...
         F32 myAngle = AIConnection::get2DAngle(client->mLocation + client->mVelocity2D, client->mLocation);
         F32 tempAngle1 = AIConnection::get2DAngle(client->mLocation, client->mTargLocation + newLocationVector);
         F32 tempAngle2 = AIConnection::get2DAngle(client->mLocation, client->mTargLocation - newLocationVector);
         
         if (mFabs(tempAngle1 - myAngle) < mFabs(tempAngle2 - myAngle))
            newLocation = client->mTargLocation + (newLocationVector * client->mEngageMinDistance);
         else
            newLocation = client->mTargLocation - (newLocationVector * client->mEngageMinDistance);
      }
   }
   else 
   {
      Con::printf("DEBUG AIStepEngage::findStraifeLocation() - player and target are on top of each other...");
      newLocation = client->mTargLocation;
   }

   //return the result
   return newLocation;
}

void AIStepEngage::process(AIConnection *client, Player *player)
{
   Parent::process(client, player);
   if (mStatus != InProgress)
      return;
      
   //make sure the target is set
   if (! mInitialized)
   {
      mInitialized = true;
      
      //set the target and the energy levels
      client->setMoveTolerance(2.0f);
      client->setEngageTarget(mTarget);
      client->setEnergyLevels(0.3f, 0.2f);
      return;
   }
   
   //make sure we still have a target
   if (! client->mTargetPlayer)
   {
      mStatus = Finished;
      client->setMoveMode(AIConnection::ModeStop);
      return;
   }
      
   //set the outdoor bools
   bool targIsOutdoors = (client->getOutdoorRadius(client->mTargLocation) > 0);
   bool playerIsOutdoors = (client->getOutdoorRadius(client->mLocation) > 0);

	//set the LOS variables
	S32 losTime;
	Point3F losLocation;
	bool hasLOS = client->hasLOSToClient(client->getEngageTarget(), losTime, losLocation);
   
   //only gain height or straife if the target is outdoors
   if (targIsOutdoors)
   {
	   //decriment the straife counter
	   bool timeToStraife = false;
	   mStraifeCounter--;

      //see if we should try to gain height
      S32 mode = client->getMoveMode();
      if (mode == AIConnection::ModeGainHeight)
      {
         //see if we need to cancel the gaining of height
         if (client->mEnergy < 0.2f || client->mLocation.z - client->mTargLocation.z > 15.0f ||
											client->mEnergy < client->mWeaponEnergy - client->mEnergyReserve)
				{
					timeToStraife = true;
				}
      }
      else
      {
         //see if we should try to gain height - only if we're outdoors
         if (playerIsOutdoors && mPauseCounter <= 0 && (client->mEnergy > 0.5f || client->mEnergy - client->mTargEnergy > 0.3f) &&
											client->mEnergy > client->mWeaponEnergy + client->mEnergyFloat &&
                                 client->mDistToTarg2D < 45.0f && client->mVelocity2D.len() > 5.0f &&
                                 client->mLocation.z - client->mTargLocation.z < 15.0f && client->mSkillLevel >= 0.3f)
            client->setMoveMode(AIConnection::ModeGainHeight);
   
         //see if we need to straife
         else
         {
				//see if we're nearish our destination
				Point3F straifeVector = client->mLocation - mStraifeLocation;
				straifeVector.z = 0;
				if (straifeVector.len() < getMin(8, client->mEngageMinDistance))
				{
		         client->setMoveMode(AIConnection::ModeStop);
					if (!mPausing)
					{
						//if we have had LOS to the client within the last 5 seconds at this moment, then pause
						//no need to pause if we're behind a hill
						if (hasLOS || losTime < 5000)
						{
							F32 skillAdjust = (1.0f - client->mSkillLevel) *
													(1.0f - client->mSkillLevel) *
													(1.0f - client->mSkillLevel);
							mPauseCounter = S32(gRandGen.randF() * 400.0f * skillAdjust);
							mPausing = true;
						}
					}
					mPauseCounter--;
				}

				if (mStraifeCounter <= 0 || mPauseCounter <= 0)
					timeToStraife = true;
			}
      }
   
      //see if we need to straife
      if (timeToStraife)
      {
			F32 skillAdjust = (1.0f - client->mSkillLevel) *
									(1.0f - client->mSkillLevel);
			mStraifeCounter = 70 + S32(400.0f * skillAdjust);
			mPauseCounter = 32767;
			mPausing = false;
         mStraifeLocation = findStraifeLocation(client);
         client->setMoveMode(AIConnection::ModeExpress);
         client->setMoveDestination(mStraifeLocation);
      }
   }
   else
   {
		//if we've never even detected the client, technically, this is a script error -
		//bots shouldn't be fighting anyone they've never seen.  However...
		if (losLocation == Point3F(0, 0, 0))
			losLocation = client->mTargLocation;

		//if the targ is Indoors, and we're outdoors, move to the client's position so that
		//it's not so rediculously easy to lose the bots just by running into a doorway..
		if (playerIsOutdoors)
			losLocation = client->mTargLocation;

      //see if we have LOS to the target (needed since we use a slightly different point than the LOS detection table
      if (--mCheckLOSCounter <= 0)
      {
	      player->disableCollision();
         mCheckLOSCounter = 10;
         mClearLOSToTarget = false;
         RayInfo rayInfo;
         Point3F startPt = player->getBoxCenter();
         Point3F endPt = client->mTargLocation;
         U32 mask = TerrainObjectType | InteriorObjectType | WaterObjectType | ForceFieldObjectType;
         mClearLOSToTarget = (! gServerContainer.castRay(startPt, endPt, mask, &rayInfo));
	      player->enableCollision();
      }

		//see if we're already at our losLocation, is it time to start idling (searching)?
		if (!mClearLOSToTarget && !mSearching && ((client->mLocation - losLocation).len() < 3.0f) && (client->getMoveMode() == AIConnection::ModeStop))
		{
			mSearching = true;
			mSearchInitialized = false;
		}
		else if (mSearching)
		{
			//see if we should abort the search (we found our target)
			if (mClearLOSToTarget)
			{
				mSearching = false;
			}

			//else initialize the search
		   else if (! mSearchInitialized)
		   {
		      mSearchInitialized = true;
				mChokeLocation = losLocation;
				mChokeIndex = -1;
		      NavigationGraph::getChokePoints(losLocation, mChokePoints, 10, 65);
				mSearchTimer = Sim::getCurrentTime() + 3000;
		   }

			else
			{
				//see if we're at the choke location, and we've been there for a few seconds...
				if (((client->mLocation - mChokeLocation).len() < 3.0f) && (client->getMoveMode() == AIConnection::ModeStop) && (Sim::getCurrentTime() > mSearchTimer))
				{
					//remove the current choke index from the array
					if (mChokeIndex >= 0)
						mChokePoints.erase(mChokeIndex);

					//if we've run out of choke points to check, we've lost our target...
					if (mChokePoints.size() <= 0)
					{
						mStatus = Failed;
						return;
					}
					else
					{
						mChokeIndex = S32(gRandGen.randF() * (mChokePoints.size() - 0.1f));
						mChokeLocation = mChokePoints[mChokeIndex];
						mSearchTimer = Sim::getCurrentTime() + (gRandGen.randF() * 1000) + 2000;
				      client->setMoveMode(AIConnection::ModeExpress);
					}
				}

				//move to the choke point
				client->setMoveDestination(mChokeLocation);
			}
		}

		//else we either have LOS, or we're moving to the target's last known location...
		else
		{
	      client->setMoveDestination(losLocation);

	      //if we can see the target, and we're close enough, don't get any closer
	      if (mClearLOSToTarget && client->mDistToTarg2D < 15.0f)
	         client->setMoveMode(AIConnection::ModeStop);
	      else
	         client->setMoveMode(AIConnection::ModeExpress);
		}
   }
}

//-----------------------------------------------------------------------------
//RangeObject step
AIStepRangeObject::AIStepRangeObject(GameBase *targetObject, const char *projectile, F32 *minDist, F32 *maxDist, Point3F *fromLocation)
{
   mInitialized = false;
   mTargetObject = targetObject;
	mCheckLOSCounter = 0;
   
   //set the range distances
   F32 minDistance = 0;
   F32 maxDistance = 1000;
   if (*minDist)
      minDistance = *minDist;
   if (*maxDist)
      maxDistance = *maxDist;
      
   mMinDistance = getMax(0.0f, getMin(minDistance, maxDistance));
   mMaxDistance = getMax(1.0f, getMax(minDistance, maxDistance));
	mFromLocation.set(-1, -1, -1);
	if (fromLocation)
		mFromLocation = *fromLocation;
	mPrevLocation.set(0, 0, 0);

	//set the mask
	mLOSMask = TerrainObjectType | InteriorObjectType | PlayerObjectType | ForceFieldObjectType;
	if (bool(mTargetObject))
	{
		Player *plyr = dynamic_cast<Player*>(targetObject);
		if (bool(plyr))
			mLOSMask = TerrainObjectType | InteriorObjectType | StaticShapeObjectType | ForceFieldObjectType;
	}

   if ((! projectile) || (! projectile[0]) || (! dStricmp(projectile, "NoAmmo")))
      mProjectile = NULL;
   else
   {
      if (! Sim::findObject(projectile, mProjectile))
      {
         Con::printf("AIStepRangeObject() failed - unable to find projectile datablock: %s", projectile);
         mProjectile = NULL; 
      }
   }
   
   if (! bool(mTargetObject) || ! bool(mProjectile))
      mStatus = Failed;
}

void AIStepRangeObject::process(AIConnection *client, Player *player)
{
   //make sure we have a client and a player
   if (!client || !player)
   {
      mStatus = Failed;
      return;
   }
      
   //make sure we're not dead
   if (! dStricmp(player->getStateName(), "Dead"))
   {
      mStatus = Failed;
      return;
   }
      
   //make sure we still have a target object
   if (! bool(mTargetObject) || ! bool(mProjectile))
   {
      mStatus = Failed;
      return;
   }
      
   //make sure the target is set
   if (! mInitialized)
   {
      mInitialized = true;
      
      //set the target and the energy levels
      client->setMoveTolerance(0.25f);
      client->setEngageTarget(NULL);

	   mTargetObject->getWorldBox().getCenter(&mTargetPoint);
		if (mFromLocation != Point3F(-1, -1, -1))
	      mGraphDestination = NavigationGraph::findLOSLocation(client->mLocation, mTargetPoint, mMinDistance, SphereF(mFromLocation, mMinDistance / 2.0f), mMaxDistance);
		else
	      mGraphDestination = NavigationGraph::findLOSLocation(client->mLocation, mTargetPoint, mMinDistance, SphereF(client->mLocation, mMinDistance / 2.0f), mMaxDistance);

		//if the range is small, the graph may not have enough resolution...
		if (mMaxDistance <= 10.0f)
		{
			if ((mTargetPoint - mGraphDestination).len() > 10.0f)
				mGraphDestination = mTargetPoint;
		}

      client->setMoveDestination(mGraphDestination);
      client->setMoveMode(AIConnection::ModeExpress);
      mStatus = InProgress;
   }
   
	//only check every 6 frames
	if (--mCheckLOSCounter <= 0)
	{
		mCheckLOSCounter = 6;

		//see if we're within range, or if we're where the graph told us to go...
		F32 directionDot = AIConnection::get2DDot(mGraphDestination - client->mLocation, mTargetPoint - client->mLocation);
		F32 distToTarg = (mTargetPoint - client->mLocation).len();
		if (((distToTarg <= mMaxDistance || directionDot <= 0.0f) &&
				(distToTarg > mMinDistance || directionDot > 0.0f)) ||
				((client->mLocation - mGraphDestination).len() < 8.0f))
		{
		   Point3F aimVectorMin, aimVectorMax;
		   F32 timeMin, timeMax;
		   bool targetInRange = mProjectile->calculateAim(mTargetPoint, Point3F(0, 0, 0), client->mMuzzlePosition, client->mVelocity, &aimVectorMin, &timeMin, &aimVectorMax, &timeMax);
		   if (targetInRange)
		   {
				//if we're shooting a ballistic projectile (outside), we don't need 
			   bool playerIsOutdoors = (client->getOutdoorRadius(client->mLocation) > 0);
		      bool clearLOSToTarget = false;
				if (mProjectile->isBallistic && playerIsOutdoors)
					clearLOSToTarget = true;
				else
				{
			      //see if we have line of site
			      player->disableCollision();
			      RayInfo rayInfo;
			      Point3F startPt = client->mMuzzlePosition;
			      Point3F endPt = mTargetPoint;
			      if (! gServerContainer.castRay(startPt, endPt, mLOSMask, &rayInfo))
					{
			         clearLOSToTarget = true;
					}
					else
					{
						//if we're here, castRay() hit something, and rayInfo should have been initialized...
						if (bool(rayInfo.object) && rayInfo.object->getId() == mTargetObject->getId())
							clearLOSToTarget = true;
					}
			      player->enableCollision();
				}
		   
				//reset the var based on LOS
	         targetInRange = clearLOSToTarget;
		   }

			if (targetInRange)
			{
				//stop the client
		      client->setMoveMode(AIConnection::ModeStop);
				mPrevLocation = client->mLocation;
				if (mPrevLocation == client->mLocation)
					mStatus = Finished;
				else
					mStatus = InProgress;
			}
			else
			{
				//if we're at our graph location, but still aren't within range...
				if (((client->mLocation - mGraphDestination).len() < 8.0f) && (client->getMoveMode() == AIConnection::ModeStop))
				{
					//try a shorter max distance and redo the graph query
					mMaxDistance = getMax(mMaxDistance - 10.0f, mMinDistance);
					mInitialized = false;
				}

				//else keep moving
				else
				{
			      client->setMoveDestination(mGraphDestination);
			      client->setMoveMode(AIConnection::ModeExpress);
				}
				mStatus = InProgress;
			}
		}
		else
		{
			//if we're at our graph location, but still aren't within range...
			if (((client->mLocation - mGraphDestination).len() < 8.0f) && (client->getMoveMode() == AIConnection::ModeStop))
			{
				//try a shorter max distance and redo the graph query
				mMaxDistance = getMax(mMaxDistance - 10.0f, mMinDistance);
				mInitialized = false;
			}

			//else keep moving
			else
			{
		      client->setMoveDestination(mGraphDestination);
		      client->setMoveMode(AIConnection::ModeExpress);
			}
			mStatus = InProgress;
		}
	}
}

//-----------------------------------------------------------------------------
//RangeObject step
AIStepIdlePatrol::AIStepIdlePatrol(Point3F *idleLocation)
{
   mInitialized = false;
	mIdleLocation.set(0, 0, 0);
	if (idleLocation)
		mIdleLocation = *idleLocation;
   mStatus = InProgress;
}

void AIStepIdlePatrol::process(AIConnection *client, Player *player)
{
   Parent::process(client, player);
   if (mStatus != InProgress)
      return;
      
   //make sure the target is set
   if (! mInitialized)
   {
      mInitialized = true;
      
		if (mIdleLocation == Point3F(0, 0, 0))
			mIdleLocation = client->mLocation;

      NavigationGraph::getChokePoints(mIdleLocation, mChokePoints, 25, 65);
		mOutdoorRadius = client->getOutdoorRadius(mIdleLocation);

		mIdleState = MoveToLocation;
		mStateInit = false;
		mMoveLocation = mIdleLocation;
		mHeadingHome = true;
		return;
   }

	switch (mIdleState)
	{
		case MoveToLocation:
			if (! mStateInit)
			{
				mStateInit = true;
			   client->setMoveDestination(mMoveLocation);
			   client->setMoveMode(AIConnection::ModeExpress);
			   client->setMoveTolerance(4.0f);
			}
			else if (client->getPathDistRemaining(20.0f) < 8.0f)
			{
				client->setMoveMode(AIConnection::ModeStop);
				mIdleState = LookAround;
				mStateInit = false;
			}
			break;
			

		case LookAround:
		{	
			S32 curTime = Sim::getCurrentTime();
			if (! mStateInit)
			{
				mStateInit = true;
				mIdleNextTime = curTime + 2000 + gRandGen.randF() * 3000;
				mIdleEndTime = curTime + 8000 + gRandGen.randF() * 8000;
			}

			//see if it's time to move somewhere
			if (curTime > mIdleEndTime)
			{
				//choose a choke point
				if (mHeadingHome)
				{
					mHeadingHome = false;
					if (mChokePoints.size() == 0)
					{
						mMoveLocation = mIdleLocation;
						if (mOutdoorRadius > 0.0f)
						{
							F32 noise = getMin(mOutdoorRadius, 30.0f);
							mMoveLocation.x += -noise / 2 + gRandGen.randF() * noise;
							mMoveLocation.y += -noise / 2 + gRandGen.randF() * noise;
						}
					}
					else
					{	
						mChokeIndex = S32(gRandGen.randF() * (mChokePoints.size() - 0.1f));
						mMoveLocation = mChokePoints[mChokeIndex];
					}
				}
				else
				{
					//40% chance of heading to the idle point
					if (gRandGen.randF() < 0.4f || mChokePoints.size() <= 1)
					{
						mHeadingHome = true;
						mMoveLocation = mIdleLocation;
					}
					else
					{
						mChokeIndex = S32(gRandGen.randF() * (mChokePoints.size() - 0.1f));
						mMoveLocation = mChokePoints[mChokeIndex];
					}
				}

				//set the new state
				mIdleState = MoveToLocation;
				mStateInit = false;
			}

			//alternate between choosing a new place to look, and pausing to look there
			else if (curTime > mIdleNextTime)
			{
				mIdleNextTime = curTime + 2000 + gRandGen.randF() * 3000;

				//see if we should look or play an animation
				if (gRandGen.randF() < 0.06f)
				{
				   player->setActionThread("pda", false, true, false);
				}
				else
				{
					Point3F lookLocation;
					if (mChokePoints.size() == 0)
						lookLocation = mIdleLocation;
					else
					{
						S32 index = S32(gRandGen.randF() * (mChokePoints.size() + 0.9));
						if (index == mChokePoints.size())
							lookLocation = mIdleLocation;
						else
							lookLocation = mChokePoints[index];
					}

					//add some noise to the lookLocation
					Point3F tempVector = lookLocation - client->mLocation;
					tempVector.z = 0;
					if (tempVector.len() < 0.001f)
						tempVector.set(0, 1, 0);
					tempVector.normalize();
					tempVector *= 10.0f;
					F32 noise = 2.0f;
					tempVector.x += -noise + gRandGen.randF() * 2 * noise;
					tempVector.y += -noise + gRandGen.randF() * 2 * noise;
					tempVector.z = 1.5f + gRandGen.randF() * 2 * noise;
					Point3F newAimLocation = client->mLocation + tempVector;

					//now aim at that point
               if (!client->scriptIsAiming())
				      client->setScriptAimLocation(newAimLocation, 3000);
				}
			}
		}
		break;
	}
}
