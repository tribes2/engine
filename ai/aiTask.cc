//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleInternal.h"
#include "ai/aiTask.h"

IMPLEMENT_CONOBJECT(AITask);

static void cAISetWeightFreq(SimObject *obj, S32, const char **argv)
{
   AITask *task = static_cast<AITask*>(obj);
   // task->setWeightFreq(dAtoi(argv[2]));
   // LH - did this until proper optimizations are performed since calcWeight()
   //          was 10% of some profiles.  
   task->setWeightFreq(dAtoi(argv[2]) * 3);  
}

static void cAISetWeight(SimObject *obj, S32, const char **argv)
{
   AITask *task = static_cast<AITask*>(obj);
   task->setWeight(dAtoi(argv[2]));
}

static const char* cAIGetWeight(SimObject *obj, S32, const char **)
{
   AITask *task = static_cast<AITask*>(obj);
   S32 weight = task->getWeight();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d", weight);
   return returnBuffer;
}

static void cAIReWeight(SimObject *obj, S32, const char **)
{
   AITask *task = static_cast<AITask*>(obj);
   task->reWeight();
}

static void cAISetMonitorFreq(SimObject *obj, S32, const char **argv)
{
   AITask *task = static_cast<AITask*>(obj);
   task->setMonitorFreq(dAtoi(argv[2]));
}

static void cAIReMonitor(SimObject *obj, S32, const char **)
{
   AITask *task = static_cast<AITask*>(obj);
   task->reMonitor();
}

void AITask::consoleInit()
{
   Con::addCommand("AITask", "setWeightFreq", cAISetWeightFreq, "ai.setWeightFreq(freq)", 3, 3);
   Con::addCommand("AITask", "setWeight", cAISetWeight, "ai.setWeight(weight)", 3, 3);
   Con::addCommand("AITask", "getWeight", cAIGetWeight, "ai.getWeight()", 2, 2);
   Con::addCommand("AITask", "reWeight", cAIReWeight, "ai.reWeight()", 2, 2);
   
   Con::addCommand("AITask", "setMonitorFreq", cAISetMonitorFreq, "ai.setMonitorFreq(freq)", 3, 3);
   Con::addCommand("AITask", "reMonitor", cAIReMonitor, "ai.reMonitor()", 2, 2);
}

AITask::AITask()
{
   mWeightFreq = 30;
   mMonitorFreq = 30;
   mWeightCounter = 0;
   mMonitorCounter = 0;
   mWeight = 0;
}

bool AITask::onAdd()
{
   if (! Parent::onAdd())
      return false;
      
   //set the task namespace
   const char *name = getName();
   if(name && name[0] && getClassRep())
   {
      Namespace *parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);
   }
   
   return true;
}

void AITask::calcWeight(AIConnection *ai)
{
   //make sure task is valid
   if (! ai)
      return;
      
   //see if it's time to re-weight the task
   // if (--mWeightCounter <= 0)
   if (gCalcWeightSlicer.ready(mWeightCounter, mWeightFreq))
   {
      // mWeightCounter = mWeightFreq;
      Con::executef(this, 2, "weight", avar("%d", ai->getId()));
   }
}

void AITask::monitor(AIConnection *ai)
{
   //make sure task is valid
   if (! ai)
      return;
      
   if (--mMonitorCounter <= 0)
   {
      mMonitorCounter = mMonitorFreq;
      Con::executef(this, 2, "monitor", avar("%d", ai->getId()));
   }
}

void AITask::assume(AIConnection *ai)
{
   //make sure task is valid
   if (! ai)
      return;
      
   mMonitorCounter = 0;
   Con::executef(this, 2, "assume", avar("%d", ai->getId()));
}

void AITask::retire(AIConnection *ai)
{
   //make sure task is valid
   if (! ai)
      return;
      
   Con::executef(this, 2, "retire", avar("%d", ai->getId()));
}

//-------------------------------------------------------------------------------------

AISlicer::AISlicer()
{
   reset();
}

void AISlicer::init(U32 delay, S32 maxPerTic)
{
   // mRand.setSeed(0x88);
   mDelay = delay;
   mLastTime = Sim::getCurrentTime();
   // mCapPileUp = (getMax(maxPerTic - 1, 0) * mDelay);
   maxPerTic;
   mDebugTotal = 0;
   mBudget = 2.0;
   // mEnabled = true;
}

void AISlicer::reset()
{
   init();
   // mEnabled = false;
}

#if 0
// Return true if this counter goes to zero AND sufficient time has elapsed.  If
// the time hasn't elapsed - leave the counter.  This should ensure that every
// body gets their shot without super-high frequency guys shutting out slow guys.
bool AISlicer::ready(S32& counter, S32 resetValue)
{
   U32   T = Sim::getCurrentTime();
   
   if (T - mLastTime >= mDelay && --counter <= 0)
   {
      // 1/4 of time, add stagger noise to those that won't really notice it
      if (resetValue > 3)
         resetValue += !(mRand.randI() & 3);
      counter = resetValue;
      
      // We want to allow multiple times per tic, but also to cap how much "reserve"
      // readiness is allowed to pile up.  
      mLastTime = getMax(mLastTime + mDelay, T - mCapPileUp);

      // Verify it's handling more than one per tick Ok
      mDebugTotal++;
      
      return true;
   }
   return false;
}
#else

// The above is very mis-conceived, let's try something a little better- 
// Note mDelay is a millisecond amount, while counter & reset value are game ticks.
bool AISlicer::ready(S32& counter, S32 resetValue)
{
   bool  goodToGo = false;

   counter--;
   
   if (mBudget > 0)
   {
      if (counter < 0)
      {
         mBudget -= 1.0;
         counter = resetValue;
         goodToGo = true;
      }
   }
   else
   {
      // Budget keeps track of how many we currently are allowed to let through.  We'd
      // like to keep it to no more than one per mDelay milliseconds.  
      U32   T = Sim::getCurrentTime();
      mBudget += (F32(T - mLastTime) / F32(mDelay));
      mBudget = getMin(mBudget, 40.0f);
      mLastTime = T;
      
      // We have to let through tasks that have been waiting a while.  Nothing will
      // wait in vain for more than about 1.2 extra seconds- 
      if (counter < -37)
      {
         counter = resetValue;
         goodToGo = true;
      }
   }

   // Track total to make sure we're getting about the percentage that we want.  
   mDebugTotal += goodToGo;

   return goodToGo;
}

#endif
