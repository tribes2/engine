//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AINAVSTEP_H_
#define _AINAVSTEP_H_

#ifndef _AISTEP_H_
#include "ai/aiStep.h"
#endif
#ifndef _GRAPH_H_
#include "ai/graph.h"
#endif
#ifndef _AINAVJETTING_H_
#include "ai/aiNavJetting.h"
#endif

class AIStepJet : public AIStep
{
   protected:
      typedef  AIStep Parent;

      AIJetting   mJetting;      
  public:
      AIStepJet(const Point3F& dest);
	   void process(AIConnection* ai, Player* player);
};

#endif
