//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AITASK_H_
#define _AITASK_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _AICONNECTION_H_
#include "ai/aiConnection.h"
#endif
#ifndef _PLAYER_H_
#include "Test/player.h"
#endif

class AITask : public SimObject
{
	typedef SimObject Parent;

	S32 mWeightFreq;
	S32 mWeightCounter;

	S32 mMonitorFreq;
	S32 mMonitorCounter;

	S32 mWeight;

	public:
	   DECLARE_CONOBJECT(AITask);
	   static void consoleInit();

		AITask();
		bool onAdd();

		void setWeightFreq(S32 freq) { mWeightFreq = getMax(1, freq); }
		void setWeight(S32 weight) { mWeight = weight; }
		void reWeight() { mWeightCounter = 0; }
		S32 getWeight() { return mWeight; }

		void setMonitorFreq(S32 freq) { mMonitorFreq = getMax(1, freq); }
		void reMonitor() { mMonitorCounter = 0; }

		//methods to call the script functions
		void calcWeight(AIConnection *ai);
		void monitor(AIConnection *ai);
		void assume(AIConnection *ai);
		void retire(AIConnection *ai);
};

#endif
