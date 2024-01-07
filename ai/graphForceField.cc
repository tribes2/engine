//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

#define  ForceFieldCheckFrequency   2.0f
#define  ForceFieldCheckDelay       U32(1000.0f / ForceFieldCheckFrequency)

//-------------------------------------------------------------------------------------

MonitorForceFields::MonitorForceFields()
{
   mFFList = NULL;
   mCount = 0;
   mSaveTimeMS = 0;
   for (S32 i = 0; i < GraphMaxTeams; i++)
      mTeamPartitions[i].setType(GraphPartition::ForceField);
}

MonitorForceFields::~MonitorForceFields()
{
   delete [] mFFList;
}

//-------------------------------------------------------------------------------------

static S32 forceFieldTeam(ForceFieldBare * ff)
{
   if (ff)
      return (ff->isTeamControlled() ? ff->getSensorGroup() : 0);
   return 0;
}

static bool forceFieldUp(ForceFieldBare * ff)
{
   if (ff)
      return dAtob(Con::executef(ff, 1, "isPowered"));
   return false;
}

//-------------------------------------------------------------------------------------

MonitorForceFields::ForceField::ForceField()
{
   mActive = false;
   mTeam = 0;
}

void MonitorForceFields::ForceField::updateEdges()
{
   // Want this quick, avoid functions calls in loop in debug exe.  
   register GraphEdgePtrs::iterator  e = mEffects.begin();
   register S32 count = mEffects.size();
   U32   team = (mActive ? mTeam : 0);
   while( --count >= 0 )
      (* e++)->setTeam(team);
}

void MonitorForceFields::atMissionStart()
{
   SimpleQueryList   sql;
   gServerContainer.findObjects(ForceFieldObjectType, 
            SimpleQueryList::insertionCallback, S32(&sql));
   
   delete [] mFFList;
   mFFList = new ForceField [ mCount = sql.mList.size() ];

   // Initialze array, and disable LOS intersections on all the force fields.  
   S32   i;
   for (i = 0; i < mCount; i++) {
      ForceFieldBare * ffPtr = dynamic_cast<ForceFieldBare*>(sql.mList[i]);
      AssertFatal(ffPtr, "MonitorForceFields query didn't work");
      mFFList[i].mFF = ffPtr;
      mFFList[i].mTeam = forceFieldTeam(ffPtr);
      mFFList[i].mActive = forceFieldUp(ffPtr);
      ffPtr->disableCollision();
   }

   // Do the collision work, only enabling the current object-
   for (i = 0; i < mCount; i++) {
      ForceFieldBare * ffPtr = mFFList[i].mFF;
      ffPtr->enableCollision();
      mFFList[i].mEffects = gNavGraph->getBlockedEdges(ffPtr, ForceFieldObjectType);
      ffPtr->disableCollision();
   }
   
   // Restore collision enablement state- perform initial update- 
   for (i = 0; i < mCount; i++) {
      mFFList[i].mFF->enableCollision();
      mFFList[i].updateEdges();
   }
}

// Clear partitions on all teams except one supplied.  
void MonitorForceFields::clearPartitions(U32 exceptTeam)
{
   for (S32 i = 0; i < GraphMaxTeams; i++)
      if (i != exceptTeam)
         mTeamPartitions[i].clear();
}

// This team had a search fail (which we assure is only due to force fields), and so we
// update their FF partition list- we mark all nodes visited on search.  
void MonitorForceFields::informSearchFailed(GraphSearch * searcher, U32 team)
{
   AssertFatal(team >= 0 && team < GraphMaxTeams, "Bad team index in FF monitor");
   mTeamPartitions[team].pushPartition(searcher->getPartition());
}

GraphPartition::Answer MonitorForceFields::reachable(U32 team, S32 from, S32 to)
{
   return mTeamPartitions[team].reachable(from, to);
}

// Check them periodically.  Just call this often.  
void MonitorForceFields::monitor()
{
   U32   T = Sim::getCurrentTime();

   if ((T - mSaveTimeMS) > ForceFieldCheckDelay) 
   {
      mSaveTimeMS = T;
      // Check for changes to team or active status.  
      for (S32 i = 0; i < mCount; i++) 
      {
         ForceField  &  ff = mFFList[i];
         
         // if (ForceFieldBare * forceFieldObject = (ForceFieldBare*)ff.mFF)
         {
            U32   team = forceFieldTeam(ff.mFF);
            bool  active = forceFieldUp((ForceFieldBare*)ff.mFF);
         
            if (team == ff.mTeam && active == ff.mActive)
               continue;

            // Here we only invalidate the old team and the new team, unless either
            // of them is team zero (which means everybody's partition gets changed).
            if (team != ff.mTeam) {
               if (team && ff.mTeam) {
                  mTeamPartitions[team].clear();
                  mTeamPartitions[ff.mTeam].clear();
               }
               else {
                  clearPartitions(0);
               }
               ff.mTeam = team;
            }

            if (active != ff.mActive) {
               clearPartitions(ff.mTeam);
               ff.mActive = active;
            }

            mFFList[i].updateEdges();
         }
      }
   }
}
