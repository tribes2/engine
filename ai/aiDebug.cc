//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/aiConnection.h"
#include "ai/aiTask.h"
#include "ai/aiStep.h"
#include "game/vehicle.h"

void AIConnection::aiDebugStuff()
{
   //DEBUG - setup
   S32 lineNum = 16;
   char idBuf[16];
   dSprintf(idBuf, sizeof(idBuf), "%d", getId());
   
   //DEBUG - check if we're debugging this client
   if (Con::getIntVariable("$AIDebugClient") != getId())
      return;

   //make sure we have a valid control object
	ShapeBase *ctrlObject = getControlObject();
	if (! ctrlObject)
	{
      Con::executef(4, "aiDebugText", idBuf, "15", "CLIENT IS DEAD!");
		return;
	}

	//get the control object
   Player *myPlayer = NULL;
   myPlayer = dynamic_cast<Player*>(ctrlObject);

	Vehicle *myVehicle = NULL;
	myVehicle = dynamic_cast<Vehicle*>(ctrlObject);
   
   //DEBUG - Clear text lines
   char textBuf[256];
   for (S32 i = lineNum; i < 64; i++)
      Con::executef(4, "aiDebugText", idBuf, avar("%d", i), "");
   
   //DEBUG - current task
   if (mCurrentTask)
      dSprintf(textBuf, sizeof(textBuf), "Current task: %d: %s %d", mCurrentTask->getId(), mCurrentTask->getName(), mCurrentTask->getWeight());
   else
      dSprintf(textBuf, sizeof(textBuf), "NO TASK!");
   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
   
   //DEBUG - current step
   if (mStep)
      dSprintf(textBuf, sizeof(textBuf), "Current step: %s", mStep->getName());
   else
      dSprintf(textBuf, sizeof(textBuf), "NO STEP!");
   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

	//find out if the control object is a player
	if (myPlayer)
	{
	   //Debug - Move Mode
	   switch (mMoveMode)
	   {
	      case ModeStop:       	dSprintf(textBuf, sizeof(textBuf), "Move Mode = %d: Stop", mMoveMode); break;
	      case ModeWalk:       	dSprintf(textBuf, sizeof(textBuf), "Move Mode = %d: Walk", mMoveMode); break;
	      case ModeGainHeight: 	dSprintf(textBuf, sizeof(textBuf), "Move Mode = %d: GainHeight", mMoveMode); break;
	      case ModeExpress:    	dSprintf(textBuf, sizeof(textBuf), "Move Mode = %d: Express", mMoveMode); break;
	      case ModeMountVehicle:  dSprintf(textBuf, sizeof(textBuf), "Move Mode = %d: MountVehicle", mMoveMode); break;
	      case ModeStuck:    		dSprintf(textBuf, sizeof(textBuf), "Move Mode = %d: Stuck", mMoveMode); break;
	      default:             	dSprintf(textBuf, sizeof(textBuf), "Move Mode unknown"); break;
	   }
		//if we're using the jet state machine, indicate this as well
		if (mNavUsingJet)
			dStrcpy(&textBuf[dStrlen(textBuf)], "  USING JET STATE MACHINE");

	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - distance to destination
	   dSprintf(textBuf, sizeof(textBuf), "Dist to node = %.3f", mDistToNode2D);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - final destination
	   dSprintf(textBuf, sizeof(textBuf), "Destination is %.3f %.3f %.3f", mMoveDestination.x, mMoveDestination.y, mMoveDestination.z);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - velocity
	   dSprintf(textBuf, sizeof(textBuf), "Velocity = %.3f", mVelocity.len());
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - energy
		F32 jetEnergy = mPath.jetWillNeedEnergy(4);              
	   dSprintf(textBuf, sizeof(textBuf), "Energy = %.3f,  jet will require: %.3f", mEnergy, jetEnergy);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - damage
	   dSprintf(textBuf, sizeof(textBuf), "Damage = %.3f", mDamage);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - downhill
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), mHeadingDownhill ? "heading DOWNHILL" : "heading UPHILL");
	   
	   //DEBUG - outdoors
	   F32 freedomRadius;
	   if (mPath.locationIsOutdoors(mLocation, &freedomRadius))
	      dSprintf(textBuf, sizeof(textBuf), "OUTDOORS: %.3f", freedomRadius);
	   else
	      dSprintf(textBuf, sizeof(textBuf), "Indoors");
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

	   //DEBUG - me/targ in water
	   dSprintf(textBuf, sizeof(textBuf), "%s | %s", mInWater ? "IN WATER" : "not in water", mTargInWater ? "TARGET IN WATER" : " targ not in water");
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

	   //DEBUG - jetting
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), mTriggers[JetTrigger] ? "JETTING" : "");
	   
	   //DEBUG - is stuck
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), mMoveMode == ModeStuck ? "STUCK!!!" : "");
	   
	   //DEBUG - engage target
	   dSprintf(textBuf, sizeof(textBuf), "Engage Target: %d", bool(mEngageTarget) ? mEngageTarget->getId() : -1);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - target object
	   dSprintf(textBuf, sizeof(textBuf), "Target Object: %d", bool(mTargetObject) ? mTargetObject->getId() : -1);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - target object
	   dSprintf(textBuf, sizeof(textBuf), "%s  %s", (mTargetInSight ? "TARG IN SIGHT" : "targ not in sight"),
	   															(mTargetInRange ? "TARG IN RANGE" : "targ not in range"));
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - engage state
	   switch (mEngageState)
	   {
	      case ChooseWeapon:      dSprintf(textBuf, sizeof(textBuf), "Engage State = %d: ChooseWeapon", mEngageState);      break;
	      case OutOfRange:        dSprintf(textBuf, sizeof(textBuf), "Engage State = %d: OutOfRange", mEngageState);        break;
	      case ReloadWeapon:      dSprintf(textBuf, sizeof(textBuf), "Engage State = %d: ReloadWeapon", mEngageState);      break;
	      case FindTargetPoint:   dSprintf(textBuf, sizeof(textBuf), "Engage State = %d: FindTargetPoint", mEngageState);   break;
	      case AimAtTarget:       dSprintf(textBuf, sizeof(textBuf), "Engage State = %d: AimAtTarget", mEngageState);       break;
	      case FireWeapon:        dSprintf(textBuf, sizeof(textBuf), "Engage State = %d: FireWeapon", mEngageState);        break;
	      default:                dSprintf(textBuf, sizeof(textBuf), "Engage State unknown");                               break;
	   }
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - current weapon
	   dSprintf(textBuf, sizeof(textBuf), "Current weapon: %s", mProjectileName);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
	   
	   //DEBUG - firing
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), mTriggers[FireTrigger] ? "FIRING" : "");
	   
	   //DEBUG - distance to target
	   if (bool(mTargetPlayer))
	      dSprintf(textBuf, sizeof(textBuf), "Dist to target = %.3f", mDistToTarg2D);
	   else if (bool(mTargetObject))
	      dSprintf(textBuf, sizeof(textBuf), "Dist to target = %.3f", mDistToObject2D);
	   else
	      dSprintf(textBuf, sizeof(textBuf), "Dist to target = NO TARGET");
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);
   
	   //DEBUG - clear lines
	   Con::executef(2, "aiDebugClearLines", idBuf);
	   
	   //DEBUG - draw the projectile path
	   char startPt[64], endPt[64];
	   if (bool(mEnemyProjectile))
	   {
	      Point3F projLocation;
	      MatrixF const& projTransform = mEnemyProjectile->getTransform();
	      projTransform.getColumn(3, &projLocation);
	      dSprintf(startPt, sizeof(startPt), "%.3f %.3f %.3f", projLocation.x, projLocation.y, projLocation.z);
	      dSprintf(endPt, sizeof(endPt), "%.3f %.3f %.3f", mImpactLocation.x, mImpactLocation.y, mImpactLocation.z);
	      Con::executef(5, "aiDebugLine", idBuf, startPt, endPt, "1.0 0.0 0.0");
	   }
	   
	   //Debug - draw the node location
	   dSprintf(startPt, sizeof(startPt), "%.3f %.3f %.3f", mLocation.x, mLocation.y, mLocation.z + 0.1f);
	   dSprintf(endPt, sizeof(endPt), "%.3f %.3f %.3f", mNodeLocation.x, mNodeLocation.y, mNodeLocation.z + 0.1f);
	   Con::executef(5, "aiDebugLine", idBuf, startPt, endPt, "1.0 0.0 1.0");

		//DEBUG - draw the move location
	   dSprintf(startPt, sizeof(startPt), "%.3f %.3f %.3f", mLocation.x, mLocation.y, mLocation.z + 0.2f);
	   dSprintf(endPt, sizeof(endPt), "%.3f %.3f %.3f", mMoveLocation.x, mMoveLocation.y, mMoveLocation.z + 0.2f);
	   Con::executef(5, "aiDebugLine", idBuf, startPt, endPt, "0.0 1.0 1.0");
	   
	   //Debug - draw the move destination
	   dSprintf(startPt, sizeof(startPt), "%.3f %.3f %.3f", mLocation.x, mLocation.y, mLocation.z);
	   dSprintf(endPt, sizeof(endPt), "%.3f %.3f %.3f", mMoveDestination.x, mMoveDestination.y, mMoveDestination.z);
	   Con::executef(5, "aiDebugLine", idBuf, startPt, endPt, "0.0 0.0 1.0");
	   
	   //Debug - draw the aim location
	   dSprintf(startPt, sizeof(startPt), "%.3f %.3f %.3f", mMuzzlePosition.x, mMuzzlePosition.y, mMuzzlePosition.z);
	   dSprintf(endPt, sizeof(endPt), "%.3f %.3f %.3f", mAimLocation.x, mAimLocation.y, mAimLocation.z);
	   Con::executef(5, "aiDebugLine", idBuf, startPt, endPt, "1.0 0.0 0.0");
	}

	//debugging code for if the bot is piloting a vehicle
	else if (myVehicle)
	{
		//Debug - destination
	   dSprintf(textBuf, sizeof(textBuf), "Destination: %.3f %.3f %.3f", mPilotDestination.x, mPilotDestination.y, mPilotDestination.z);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

		//Debug - distance to dest
	   dSprintf(textBuf, sizeof(textBuf), "Distance to dest2D: %.3f", mPilotDistToDest2D);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

		//Debug - aim location
	   dSprintf(textBuf, sizeof(textBuf), "Destination: %.3f %.3f %.3f", mPilotAimLocation.x, mPilotAimLocation.y, mPilotAimLocation.z);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

		//Debug - current speed/thrust
	   dSprintf(textBuf, sizeof(textBuf), "Velocity: %.3f,  Thrust: %.3f", mPilotCurVelocity, mPilotCurThrust);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

		//Debug - current pitch, desired pitch, pitch inc
	   dSprintf(textBuf, sizeof(textBuf), "current pitch: %.3f,  desired pitch: %.3f,  pitch inc: %.3f", mCurrentPitch, mDesiredPitch, mPitchIncrement);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

		//Debug - current pitch, desired pitch, pitch inc
	   dSprintf(textBuf, sizeof(textBuf), "pitchUpMax: %.3f,  pitchDownMax: %.3f", mPitchUpMax, mPitchDownMax);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

		//Debug - current yaw, yaw diff
	   dSprintf(textBuf, sizeof(textBuf), "curYaw: %.3f", mCurrentYaw);
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), textBuf);

	   //DEBUG - jetting
	   Con::executef(4, "aiDebugText", idBuf, avar("%d", lineNum++), mTriggers[JetTrigger] ? "JETTING" : "");
	   
	   //DEBUG - clear lines
	   char startPt[64], endPt[64];
	   Con::executef(2, "aiDebugClearLines", idBuf);

	   //Debug - draw the vehicle destination
	   dSprintf(startPt, sizeof(startPt), "%.3f %.3f %.3f", mVehicleLocation.x, mVehicleLocation.y, mVehicleLocation.z);
	   dSprintf(endPt, sizeof(endPt), "%.3f %.3f %.3f", mPilotDestination.x, mPilotDestination.y, mPilotDestination.z);
	   Con::executef(5, "aiDebugLine", idBuf, startPt, endPt, "0.0 0.0 1.0");

	   //Debug - draw the vehicle aim location
	   dSprintf(startPt, sizeof(startPt), "%.3f %.3f %.3f", mVehicleLocation.x, mVehicleLocation.y, mVehicleLocation.z + 0.1f);
	   dSprintf(endPt, sizeof(endPt), "%.3f %.3f %.3f", mPilotAimLocation.x, mPilotAimLocation.y, mPilotAimLocation.z + 0.1f);
	   Con::executef(5, "aiDebugLine", idBuf, startPt, endPt, "1.0 0.0 0.0");
	}
}
