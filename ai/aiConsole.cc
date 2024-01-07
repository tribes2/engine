//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "math/mMatrix.h"
#include "console/console.h"
#include "console/simBase.h"
#include "game/gameBase.h"
#include "game/moveManager.h"
#include "game/player.h"
#include "ai/aiConnection.h"
#include "ai/aiStep.h"
#include "ai/aiNavStep.h"
#include "ai/aiTask.h"
#include "core/idGenerator.h"

static S32 cAIConnect(SimObject *, S32 argc, const char** argv)
{
   //create the connection
   AIConnection *aiConnection = new AIConnection();
   aiConnection->registerObject();
   
   //add the AI to the client group
   SimGroup *g = Sim::getClientGroup();
   g->addObject(aiConnection);
   
   //execute the console function
   const char *teamStr = "-2";
   const char *skillStr = "0.5";
   const char *offenseStr = "1";
	const char *voicePitchStr = "0";
	const char *voiceStr = "";
   if (argc >= 3)
      teamStr = argv[2];
   if (argc >= 4)
      skillStr = argv[3];
   if (argc >= 5)
	{
		if (!dStricmp(argv[4], "true") || dAtoi(argv[4]) > 0) 
	      offenseStr = "1";
		else
	      offenseStr = "0";
	}
   if (argc >= 6)
      voiceStr = argv[5];
	if (argc >= 7)
		voicePitchStr = argv[6];
   Con::executef(aiConnection, 7, "onAIConnect", argv[1], teamStr, skillStr, offenseStr, voiceStr, voicePitchStr);

   return aiConnection->getId();
}

static void cAIDrop(SimObject *obj, S32, const char**)
{
   //call the script function
   AIConnection *ai = static_cast<AIConnection*>(obj);
   Con::executef(ai, 1, "onAIDrop");

   //next, call the game disconnection code...
   Con::printf("AI Client %d is disconnected.", ai->getId());
   ai->setDisconnectReason( "Removed" );

   //finally, delete the object
   ai->deleteObject();
}

static F32 cAIGetGraphDistance(SimObject *, S32, const char** argv)
{
   Point3F dest(0, 0, 0);
   dSscanf(argv[1], "%f %f %f", &dest.x, &dest.y, &dest.z);
   Point3F source(0, 0, 0);
   dSscanf(argv[2], "%f %f %f", &source.x, &source.y, &source.z);
   return NavigationGraph::fastDistance(source, dest);
}

// Intended for initialization of whatever management is central to all bots.  
AISlicer gCalcWeightSlicer;
AISlicer gScriptEngageSlicer;

static bool cAISlicerInit(SimObject *, S32, const char**)
{
   gCalcWeightSlicer.init(30, 1);
   gScriptEngageSlicer.init(15, 2);
   return true;
}

static bool cAISlicerReset(SimObject *, S32, const char**)
{
   gCalcWeightSlicer.reset();
   gScriptEngageSlicer.reset();
   return true;
}

static void cAISystemEnabled(SimObject *, S32 argc, const char **argv)
{
   bool status = true;
   if (argc == 2)
      status = ((!dStricmp(argv[1], "true")) || (dAtoi(argv[1]) != 0));
   gAISystemEnabled = status;
}

bool gAISystemEnabled = false;

void AIInit()
{
   Con::addCommand("aiConnect", cAIConnect, "aiConnect(name [, team , skill, offense, voice, voicePitch]);", 2, 7);
   Con::addCommand("AIGetPathDistance", cAIGetGraphDistance, "AIGetPathDistance(fromPoint, toPoint);", 3, 3);
   Con::addCommand("AISlicerInit", cAISlicerInit, "AISlicerInit();", 1, 1);
   Con::addCommand("AISlicerReset", cAISlicerReset, "AISlicerReset();", 1, 1);
   Con::addCommand("AISystemEnabled", cAISystemEnabled, "AISystemEnabled([bool]);", 1, 2);
}

static void cAISetSkillLevel(SimObject *obj, S32, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->setSkillLevel(dAtof(argv[2]));
}

static F32 cAIGetSkillLevel(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getSkillLevel();
}

static void cAISetMoveSpeed(SimObject *obj, S32, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->setMoveSpeed(dAtoi(argv[2]));
}

static void cAIStop(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->setMoveMode(AIConnection::ModeStop);
}

static void cAIStepMove(SimObject *obj, S32 argc, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   
   //first, clear the current step
   ai->clearStep();
   
   //set the "move to" destination
   VectorF v(0,0,0);
   dSscanf(argv[2], "%f %f %f", &v.x, &v.y, &v.z);
   ai->setMoveDestination(Point3F(v.x, v.y, v.z));
   
   //set the move mode if given
   if (argc >= 4)
      ai->setMoveTolerance(dAtof(argv[3]));
   else
      ai->setMoveTolerance(0.25f);

   //don't change the move mode if we're stuck...
   if (ai->getMoveMode() != AIConnection::ModeStuck)
   {
      if (argc >= 5)
         ai->setMoveMode(dAtoi(argv[4]));
      else
         ai->setMoveMode(AIConnection::ModeExpress);
   }
}

static bool cAIAimAtLocation(SimObject *obj, S32 argc, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   VectorF v(0,0,0);
   dSscanf(argv[2], "%f %f %f", &v.x, &v.y, &v.z);
   S32 duration = 1500;
   if (argc == 4)
      duration = getMax(0, dAtoi(argv[3]));
   return ai->setScriptAimLocation(Point3F(v.x, v.y, v.z), duration);
}

static const char* cAIGetAimLocation(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   Point3F aimPoint = ai->getAimLocation();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%f %f %f", aimPoint.x, aimPoint.y, aimPoint.z);
   return returnBuffer;
}
   
static bool cAIIsMountingVehicle(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return (ai->getMoveMode() == AIConnection::ModeMountVehicle);
}

static void cAISetTurretMounted(SimObject *obj, S32, const char**argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->setTurretMounted(dAtoi(argv[2]));
}

static void cAISetPilotDestination(SimObject *obj, S32 argc, const char**argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);

   VectorF v(0,0,0);
   dSscanf(argv[2], "%f %f %f", &v.x, &v.y, &v.z);

	F32 speed = 1.0f;
	if (argc >= 4)
		speed = dAtof(argv[3]);
   ai->setPilotDestination(v, speed);
}

static void cAISetPilotAimLocation(SimObject *obj, S32, const char**argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   VectorF v(0,0,0);
   dSscanf(argv[2], "%f %f %f", &v.x, &v.y, &v.z);
   ai->setPilotAimLocation(v);
}

static void cAISetPilotPitchRange(SimObject *obj, S32, const char**argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->setPilotPitchRange(dAtof(argv[2]), dAtof(argv[3]), dAtof(argv[4]));
}

static F32 cAIGetPathDistance(SimObject *obj, S32 argc, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   Point3F dest(0, 0, 0);
   dSscanf(argv[2], "%f %f %f", &dest.x, &dest.y, &dest.z);
   Point3F source(-1, -1, -1);
   if (argc == 4)
      dSscanf(argv[3], "%f %f %f", &source.x, &source.y, &source.z);
   return ai->getPathDistance(dest, source);
}

static F32 cAIPathDistRemaining(SimObject *obj, S32, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getPathDistRemaining(dAtof(argv[2]));
}

static const char* cAIGetLOSLocation(SimObject *obj, S32 argc, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   Point3F dest(0, 0, 0);
   dSscanf(argv[2], "%f %f %f", &dest.x, &dest.y, &dest.z);
   F32 minDist = -1;
   if (argc >= 4)
      minDist = dAtof(argv[3]);
   F32 maxDist = 1e6;
   if (argc >= 5)
      maxDist = dAtof(argv[4]);
   Point3F nearPoint(-1, -1, -1);
   if (argc >= 6)
      dSscanf(argv[5], "%f %f %f", &nearPoint.x, &nearPoint.y, &nearPoint.z);

   Point3F graphPoint = ai->getLOSLocation(dest, minDist, maxDist, nearPoint);

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%f %f %f", graphPoint.x, graphPoint.y, graphPoint.z);
   return returnBuffer;
}

static const char* cAIGetHideLocation(SimObject *obj, S32, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   Point3F dest(0, 0, 0);
   dSscanf(argv[2], "%f %f %f", &dest.x, &dest.y, &dest.z);
   F32 range = dAtof(argv[3]);
   Point3F nearPoint(0, 0, 0);
   dSscanf(argv[4], "%f %f %f", &nearPoint.x, &nearPoint.y, &nearPoint.z);
   F32 hideLength = dAtof(argv[5]);

   Point3F graphPoint = ai->getHideLocation(dest, range, nearPoint, hideLength);
        
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%f %f %f", graphPoint.x, graphPoint.y, graphPoint.z);
   return returnBuffer;
}

static void cAISetEngageTarget(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   
   //find the target
   GameConnection *target;
   if (Sim::findObject(argv[2], target))
      ai->setEngageTarget(target);
   else
      ai->setEngageTarget(NULL);
}

static const char* cAIGetEngageTarget(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   S32 targetId = ai->getEngageTarget();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d", targetId);
   return returnBuffer;
}

static void cAISetVictim(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);

   //find the victim client
   GameConnection *victim;
   if (! Sim::findObject(argv[2], victim))
      return;

   //find the corpse
   Player *corpse;
   if (! Sim::findObject(argv[3], corpse))
      return;

   ai->setVictim(victim, corpse);
}

static S32 cAIGetVictimCorpse(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getVictimCorpse();
}

static S32 cAIGetVictimTime(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getVictimTime();
}

static bool cAIHasLOSToClient(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   S32 losTime;
   Point3F losLocation;
   return ai->hasLOSToClient(dAtoi(argv[2]), losTime, losLocation);
}

static S32 cAIGetClientLOSTime(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   S32 losTime;
   Point3F losLocation;
   ai->hasLOSToClient(dAtoi(argv[2]), losTime, losLocation);
   return losTime;
}

static const char* cAIGetDetectLocation(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   S32 losTime;
   Point3F losLocation(0, 0, 0);
   ai->hasLOSToClient(dAtoi(argv[2]), losTime, losLocation);

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%f %f %f", losLocation.x, losLocation.y, losLocation.z);
   return returnBuffer;
}

static void cAIClientDetected(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->clientDetected(dAtoi(argv[2]));
}

static void cAISetDetectPeriod(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->setDetectPeriod(dAtoi(argv[2]));
}

static S32 cAIGetDetectPeriod(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getDetectPeriod();
}

static void cAISetBlinded(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->setBlinded(dAtoi(argv[2]));
}

static void cAISetDangerLocation(SimObject *obj, S32 argc, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   Point3F dest(0, 0, 0);
   dSscanf(argv[2], "%f %f %f", &dest.x, &dest.y, &dest.z);
   S32 duration = 0;
   if (argc == 4)
      duration = dAtoi(argv[3]);
   ai->setEvadeLocation(dest, duration);
}

const char *gTargetObjectMode[AIConnection::NumObjectModes] =
{
   "Destroy",
   "Repair",
   "Laze",
   "Mortar",
   "Missile",
   "MissileNoLock",
   "AttackMode1",
   "AttackMode2",
   "AttackMode3",
   "AttackMode4"
};

static void cAISetTargetObject(SimObject *obj, S32 argc, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   
   //find the target
   ShapeBase *targetObject;
   if (Sim::findObject(argv[2], targetObject))
   {
      F32 range = 30;
      if (argc >= 4)
         range = dAtof(argv[3]);
      S32 objectMode = AIConnection::DestroyObject;
      if (argc == 5)
      {
         for (int i = 0; i < AIConnection::NumObjectModes; i++)
         {
            if (! dStricmp(argv[4], gTargetObjectMode[i]))
            {
               objectMode = i;
               break;
            }
         }
      }
      ai->setTargetObject(targetObject, range, objectMode);
   }
   else
      ai->setTargetObject(NULL);
}

static const char* cAIGetTargetObject(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   S32 targetId = ai->getTargetObject();
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d", targetId);
   return returnBuffer;
}

static void cAIStepRangeObject(SimObject *obj, S32 argc, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   
   //find the target
   GameBase *targetObject;
   if (Sim::findObject(argv[2], targetObject))
   {
      F32 minDist, maxDist;
      minDist = dAtof(argv[4]);
      maxDist = dAtof(argv[5]);
      Point3F fromLocation(0, 0, 0);
      AIStep *step;
      if (argc == 7)
      {
         dSscanf(argv[6], "%f %f %f", &fromLocation.x, &fromLocation.y, &fromLocation.z);
         step = new AIStepRangeObject(targetObject, argv[3], &minDist, &maxDist, &fromLocation);
      }
      else
      {
         step = new AIStepRangeObject(targetObject, argv[3], &minDist, &maxDist, NULL);
      }
      step->registerObject("AIStepRangeObject");
      ai->addObject(step);
      ai->setStep(step);
   }
}

static bool cAITargetInSight(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->TargetInSight();
}

static bool cAITargetInRange(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->TargetInRange();
}

static void cAIPressFire(SimObject *obj, S32 argc, const char** argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   S32 count = 1;
   if (argc == 3)
      count = dAtoi(argv[2]);
   ai->scriptSustainFire(count);
}

static void cAIPressJump(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->pressJump();
}

static void cAIPressJet(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->pressJet();
}

static void cAIPressGrenade(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->pressGrenade();
}

static void cAIPressMine(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->pressMine();
}

static void cAISetWeaponInfo(SimObject *obj, S32 argc, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   S32 triggerCount = (argc >= 6 ? dAtoi(argv[5]) : 1);
   F32 requiredEnergy = (argc >= 7 ? dAtof(argv[6]) : 0.0);
   F32 errorFactor = (argc >= 8 ? dAtof(argv[7]) : 1.0);
   ai->setWeaponInfo(argv[2], dAtoi(argv[3]), dAtoi(argv[4]), triggerCount, requiredEnergy, errorFactor);
}

static void cAIClearStep(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->clearStep();
}

static void cAIStepEscort(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->clearStep();
   
   //find the target
   GameConnection *target;
   if (Sim::findObject(argv[2], target))
   {
      AIStep *step = new AIStepEscort(target);
      step->registerObject("AIStepEscort");
      ai->addObject(step);
      ai->setStep(step);
   }
}

static void cAIStepJet(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   
   Point3F dest(0,0,0);
   dSscanf(argv[2], "%f %f %f", &dest.x, &dest.y, &dest.z);
   
   if (AIStep * step = new AIStepJet(dest)) {
      ai->clearStep();
      step->registerObject("AIStepJet");
      ai->addObject(step);
      ai->setStep(step);
   }
}

static void cAISetPath(SimObject *obj, S32 argc, const char **argv)
{
   if (AIConnection * ai = dynamic_cast<AIConnection*>(obj)) {
      if (argc == 3) {
         Point3F dest(0,0,0);
         dSscanf(argv[2], "%f %f %f", &dest.x, &dest.y, &dest.z);
         ai->setPathDest(&dest);
      }  
      else
         ai->setPathDest();
   }
}

static void cAIStepEngage(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);

   //the engage step functions better if it can be left to run uninterrupted -
   //find out if we're already engaging this target before resetting the step...
   GameConnection *target;
   if (Sim::findObject(argv[2], target))
   {
      S32 curTarget = ai->getEngageTarget();
      const char *curStepName = ai->getStepName();
      if (!dStricmp(curStepName, "AIStepEngage") && (curTarget == target->getId()))
         return;

      //must be someone new we're to engage...
      AIStep *step = new AIStepEngage(target);
      step->registerObject("AIStepEngage");
      ai->addObject(step);
      ai->setStep(step);
   }
   else
      ai->clearStep();
}

static void cAIStepIdle(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->clearStep();

   Point3F idleLocation(0,0,0);
   dSscanf(argv[2], "%f %f %f", &idleLocation.x, &idleLocation.y, &idleLocation.z);

   AIStep *step = new AIStepIdlePatrol(&idleLocation);
   step->registerObject("AIStepIdlePatrol");
   ai->addObject(step);
   ai->setStep(step);
}

static const char* cAIStepGetStatus(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getStepStatus();
}

static const char* cAIStepGetName(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getStepName();
}

static void cAIClearTasks(SimObject *obj, S32, const char**)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->clearTasks();
}

static const char* cAIAddTask(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   AITask *newTask = new AITask();
   newTask->registerObject(argv[2]);
   ai->addObject(newTask);
   ai->addTask(newTask);
   return avar("%d", newTask->getId());
}

static void cAIRemoveTask(SimObject *obj, S32, const char **argv)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->removeTask(dAtoi(argv[2]));
}

static void cAIListTasks(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->listTasks();
}

static S32 cAIGetTaskId(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   AITask *curTask = ai->getCurrentTask();
   if (bool(curTask))
      return curTask->getId();
   else
      return -1;
}

static const char* cAIGetTaskName(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   AITask *curTask = ai->getCurrentTask();
   if (bool(curTask))
      return curTask->getName();
   else
      return "";
}

static S32 cAIGetTaskTime(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   return ai->getTaskTime();
}

static void cAIMissionCycleCleanup(SimObject *obj, S32, const char **)
{
   AIConnection *ai = static_cast<AIConnection*>(obj);
   ai->missionCycleCleanup();
}

void AIConnection::consoleInit()
{
   Con::addCommand("AIConnection", "drop", cAIDrop, "ai.drop()", 2, 2);

   Con::addCommand("AIConnection", "setSkillLevel", cAISetSkillLevel, "ai.setSkillLevel(float)", 3, 3);
   Con::addCommand("AIConnection", "getSkillLevel", cAIGetSkillLevel, "ai.getSkillLevel()", 2, 2);

   Con::addCommand("AIConnection", "setEngageTarget", cAISetEngageTarget, "ai.setEngageTarget(client)", 3, 3);
   Con::addCommand("AIConnection", "getEngageTarget", cAIGetEngageTarget, "ai.getEngageTarget()", 2, 2);
   Con::addCommand("AIConnection", "setVictim", cAISetVictim, "ai.setVictim(client, corpseObject)", 4, 4);
   Con::addCommand("AIConnection", "getVictimCorpse", cAIGetVictimCorpse, "ai.getVictimCorpse()", 2, 2);
   Con::addCommand("AIConnection", "getVictimTime", cAIGetVictimTime, "ai.getVictimTime()", 2, 2);

   Con::addCommand("AIConnection", "hasLOSToClient", cAIHasLOSToClient, "ai.hasLOSToClient(client)", 3, 3);
   Con::addCommand("AIConnection", "getClientLOSTime", cAIGetClientLOSTime, "ai.getClientLOSTime(client)", 3, 3);
   Con::addCommand("AIConnection", "getDetectLocation", cAIGetDetectLocation, "ai.getDetectLocation(client)", 3, 3);
   Con::addCommand("AIConnection", "clientDetected", cAIClientDetected, "ai.clientDetected(client)", 3, 3);
   Con::addCommand("AIConnection", "setDetectPeriod", cAISetDetectPeriod, "ai.setDetectPeriod()", 3, 3);
   Con::addCommand("AIConnection", "getDetectPeriod", cAIGetDetectPeriod, "ai.getDetectPeriod()", 2, 2);
   Con::addCommand("AIConnection", "setBlinded", cAISetBlinded, "ai.setBlinded(durationMS)", 3, 3);
   Con::addCommand("AIConnection", "setDangerLocation", cAISetDangerLocation, "ai.setDangerLocation(point3F [, durationTicks])", 3, 4);
   
   Con::addCommand("AIConnection", "setTargetObject", cAISetTargetObject, "ai.setTargetObject(object [, range, mode: destroy/repair/laze])", 3, 5);
   Con::addCommand("AIConnection", "getTargetObject", cAIGetTargetObject, "ai.getTargetObject()", 2, 2);
   Con::addCommand("AIConnection", "targetInSight", cAITargetInSight, "ai.targetInSight()", 2, 2);
   Con::addCommand("AIConnection", "targetInRange", cAITargetInRange, "ai.targetInRange()", 2, 2);
   
   Con::addCommand("AIConnection", "pressFire", cAIPressFire, "ai.pressFire([sustain count])", 2, 3);
   Con::addCommand("AIConnection", "pressJump", cAIPressJump, "ai.pressJump()", 2, 2);
   Con::addCommand("AIConnection", "pressJet", cAIPressJet, "ai.pressJet()", 2, 2);
   Con::addCommand("AIConnection", "pressGrenade", cAIPressGrenade, "ai.pressGrenade()", 2, 2);
   Con::addCommand("AIConnection", "pressMine", cAIPressMine, "ai.pressMine()", 2, 2);
   
   Con::addCommand("AIConnection", "setWeaponInfo", cAISetWeaponInfo, "ai.setWeaponInfo(projectile, minDist, maxDist [, triggerCount, requiredEnergy, errorFactor]);", 5, 8);
   
   Con::addCommand("AIConnection", "aimAt", cAIAimAtLocation, "bool ai.aimAt(point [, duration MS])", 3, 4);
   Con::addCommand("AIConnection", "getAimLocation", cAIGetAimLocation, "ai.getAimLocation()", 2, 2);

   Con::addCommand("AIConnection", "isMountingVehicle", cAIIsMountingVehicle, "ai.isMountingVehicle()", 2, 2);
   Con::addCommand("AIConnection", "setTurretMounted", cAISetTurretMounted, "ai.setTurretMounted(turretId)", 3, 3);

   Con::addCommand("AIConnection", "setPilotDestination", cAISetPilotDestination, "ai.setPilotDestination(point3F [, maxSpeed])", 3, 4);
   Con::addCommand("AIConnection", "setPilotAim", cAISetPilotAimLocation, "ai.setPilotAim(point3F)", 3, 3);
   Con::addCommand("AIConnection", "setPilotPitchRange", cAISetPilotPitchRange, "ai.setPilotPitchRange(pitchUpMax, pitchDownMax, pitchIncMax)", 5, 5);

   Con::addCommand("AIConnection", "getPathDistance", cAIGetPathDistance, "ai.getPathDistance(destination [, source])", 3, 4);
   Con::addCommand("AIConnection", "pathDistRemaining", cAIPathDistRemaining, "ai.pathDistRemaining(maxDist)", 3, 3);
   Con::addCommand("AIConnection", "getLOSLocation", cAIGetLOSLocation, "ai.getLOSLocation(targetPoint [, minDist, maxDist, nearPoint])", 3, 6);
   Con::addCommand("AIConnection", "getHideLocation", cAIGetHideLocation, "ai.getHideLocation(targetPoint, range, nearPoint, hideLength)", 6, 6);

   //Step script fuctions
   Con::addCommand("AIConnection", "clearStep", cAIClearStep, "ai.clearStep()", 2, 2);
   Con::addCommand("AIConnection", "getStepStatus", cAIStepGetStatus, "ai.getStepStatus()", 2, 2);
   Con::addCommand("AIConnection", "getStepName", cAIStepGetName, "ai.getStepName()", 2, 2);
   
   Con::addCommand("AIConnection", "stop", cAIStop, "ai.stop()", 2, 2);
   Con::addCommand("AIConnection", "stepMove", cAIStepMove, "ai.stepMove(point3 [, tolerance, mode])", 3, 5);
   Con::addCommand("AIConnection", "stepEscort", cAIStepEscort, "ai.stepEscort(client)", 3, 3);
   Con::addCommand("AIConnection", "stepEngage", cAIStepEngage, "ai.stepEngage(client)", 3, 3);
   Con::addCommand("AIConnection", "stepRangeObject", cAIStepRangeObject, "ai.stepRangeObject(object, weapon, minDist, maxDist [, nearLocation])", 6, 7);
   Con::addCommand("AIConnection", "stepIdle", cAIStepIdle, "ai.stepIdle(point3)", 3, 3);
   Con::addCommand("AIConnection", "stepJet", cAIStepJet, "ai.stepJet(toLoc)", 3, 3);
   Con::addCommand("AIConnection", "setPath", cAISetPath, "ai.setPath([toLoc])", 2, 3);
   
   Con::addCommand("AIConnection", "clearTasks", cAIClearTasks, "ai.clearTasks()", 2, 2);
   Con::addCommand("AIConnection", "addTask", cAIAddTask, "ai.addTask(taskName)", 3, 3);
   Con::addCommand("AIConnection", "removeTask", cAIRemoveTask, "ai.removeTask(id)", 3, 3);
   Con::addCommand("AIConnection", "listTasks", cAIListTasks, "ai.listTasks()", 2, 2);
   Con::addCommand("AIConnection", "getTaskId", cAIGetTaskId, "ai.getTaskId()", 2, 2);
   Con::addCommand("AIConnection", "getTaskName", cAIGetTaskName, "ai.getTaskName()", 2, 2);
   Con::addCommand("AIConnection", "getTaskTime", cAIGetTaskTime, "ai.getTaskTime()", 2, 2);

   Con::addCommand("AIConnection", "missionCycleCleanup", cAIMissionCycleCleanup, "ai.missionCycleCleanup()", 2, 2);
}
