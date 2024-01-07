//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/aiConnection.h"
#include "ai/aiNavStep.h"


AIStepJet::AIStepJet(const Point3F& dest)
{
   mJetting.init(dest);
}

void AIStepJet::process(AIConnection* ai, Player* player)
{
   Parent::process(ai, player);
   if (mStatus != InProgress)
      return;
      
   ai->setMoveMode(AIConnection::ModeWalk);
      
   if( mJetting.process(ai, player) )
      mStatus = (mJetting.status() == AIJetSuccess ? Finished : Failed);
}

