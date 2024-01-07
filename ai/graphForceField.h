//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHFORCEFIELD_H_
#define _GRAPHFORCEFIELD_H_

#ifndef _FORCEFIELDBARE_H_
#include "game/forceFieldBare.h"
#endif

class MonitorForceFields 
{
   public:
      struct ForceField 
      {
         ForceField();
         void  updateEdges();
         SimObjectPtr<ForceFieldBare> mFF;
         GraphEdgePtrs     mEffects;
         bool              mActive;
         U32               mTeam;
      };
   
   protected:
      ForceField  *  mFFList;
      S32            mCount;
      U32            mSaveTimeMS;
      PartitionList  mTeamPartitions[GraphMaxTeams];
      
   public:
      MonitorForceFields();
      ~MonitorForceFields();
      GraphPartition::Answer reachable(U32 team, S32 from, S32 to);
      void  clearPartitions(U32 allBut);
      void  informSearchFailed(GraphSearch * searcher, U32 team);
      S32   count() const  {return mCount;}
      void  atMissionStart();
      void  monitor();
};

#endif
