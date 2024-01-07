//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------

#define  ThreatCheckFrequency    1.0f
#define  ThreatCheckDelay        U32( 1000.0f / ThreatCheckFrequency )

//-------------------------------------------------------------------------------------
//                Search Threat Object

SearchThreat::SearchThreat()
{
   mActive = false;
   mTeam = 0;
}

SearchThreat::SearchThreat(ShapeBase * threat, F32 R, S32 team)
{
   mThreat = threat;
   MatrixF const& threatTransform = threat->getTransform();
   threatTransform.getColumn(3, &center);
   radius = R;
   mActive = (threat->getDamageState() == ShapeBase::Enabled);
   mTeam = team;
}

// Update the threat bit set on the nodes that this threat effects.  Want it to 
// be fast, so the two loops are separate.  
void SearchThreat::updateNodes()
{
   GraphNodeList::iterator    n = mEffects.begin(); 
   GraphThreatSet             threatSetOn = 1;
   GraphThreatSet             threatSetOff = ~(threatSetOn <<= mSlot);
   S32                        count = mEffects.size(); 

   if (mActive)
      while( --count >= 0 )
         (* n++)->threats() |= threatSetOn;
   else 
      while( --count >= 0 )
         (* n++)->threats() &= threatSetOff;
}

// Check for changes to team or enabled status.  If the enabled status changes, then
// bunch of edges change - let caller know there was a big change.  
bool SearchThreat::checkEnableChange()
{
   bool  enabled = (mThreat->getDamageState() == ShapeBase::Enabled);
   if (enabled != mActive) {
      mActive = enabled;
      updateNodes();
      return true;
   }
   return false;
}

//-------------------------------------------------------------------------------------
//                   Threat List Management / Configuration

SearchThreats::SearchThreats()
{
   dMemset(mTeamCaresAbout, 0, sizeof(mTeamCaresAbout));

   // Team zero is reserved for threats temporarily installed for the purpose of the 
   // LOS queries.  (These queries temporarily flag edges, and then unflags them).  
   // All teams "worry" about threat zero.  
   mCheck = mCount = 1;
   informOtherTeams(-1, 0, true);
   mThreats[0].mActive = false;
   
   mSaveTimeMS = Sim::getCurrentTime();
}

// Inform all other teams of this threat's active status (update their care-about set).
void SearchThreats::informOtherTeams(S32 thisTeam, S32 thisThreat, bool isOn)
{
   GraphThreatSet  threatSet = 1;
   threatSet <<= thisThreat;
   for (S32 i = 0; i < GraphMaxTeams; i++)
      if (i != thisTeam && isOn)
         mTeamCaresAbout[i] |= threatSet;
      else
         mTeamCaresAbout[i] &= ~threatSet;
}

GraphThreatSet SearchThreats::getThreatSet(S32 team) const
{
   AssertFatal(validArrayIndex(team, GraphMaxTeams), "Bad team # to getThreatSet()");
   return mTeamCaresAbout[team];
}

void SearchThreats::pushTempThreat(GraphNode* losNode, const Point3F& avoidPt, F32 avoidRad)
{
   AssertFatal(mThreats[0].mEffects.size() == 0, "Temp threat push not balanced");
   mThreats[0].mEffects = gNavGraph->getVisibleNodes(losNode, avoidPt, avoidRad);
   mThreats[0].mActive = true;
   mThreats[0].updateNodes();
}

void SearchThreats::popTempThreat()
{
   mThreats[0].mActive = false;
   mThreats[0].updateNodes();
   mThreats[0].mEffects.clear();
}

// Add the threat. This is called once per MIS file threat at mission startup.  
bool SearchThreats::add(const SearchThreat& threat)
{
   if (mCount < MaxThreats) {
      S32   X = mCount++;
      mThreats[X] = threat;
      informOtherTeams(threat.mTeam, mThreats[X].mSlot = X, true);
      mThreats[X].mEffects = gNavGraph->getVisibleNodes(threat.center, threat.radius);
      mThreats[X].updateNodes();
      return true;
   }
   return false;
}

// Fetch into autoclass buffer, meant for immediate use.
S32 SearchThreats::getThreatPtrs(const SearchThreat* list[MaxThreats], GraphThreatSet mask) const
{
   S32               numActive = 0;
   GraphThreatSet    bit = 1;
   
   for (S32 i = 0; i < mCount; i++, bit <<= 1)
      if ((mask & bit) && mThreats[i].mActive)
         list[numActive++] = &mThreats[i];
            
   return numActive;
}

// This is called every tick to efficiently keep the list clean of threats
// that have gone away, or to monitor status of the threat.  
//
// We return true if we can afford to do some more stuff and we're not at list's
// end.  We can't afford to do more stuff if some threat had to be rescanned.  
bool SearchThreats::checkOne()
{
   bool  canCheckMore = true;

   if (mCheck < mCount) {
      if (!bool(mThreats[mCheck].mThreat)) {
         // AssertFatal(0, "Can static threats go away?");
         // if (mCheck < --mCount)        // fast erase 
         //    mThreats[mCheck] = mThreats[mCount];
         // mThreats[mCount].mThreat = NULL;
         // mThreats[mCount].mActive = false;
      }
      else {
            // getSensorGroup() has problem - for now just leave it at assigned team in order
            //    to get it tested.  
         // S32   team = mThreats[mCheck].mThreat->getSensorGroup();
         // if (mThreats[mCheck].mTeam != team) {
         //    informOtherTeams(mThreats[mCheck].mTeam, mCheck, false);
         //    informOtherTeams(team, mCheck, mThreats[mCheck].mActive);
         //    mThreats[mCheck].mTeam = team;
         // }
      
         if (mThreats[mCheck].checkEnableChange()) {
            canCheckMore = false;
            informOtherTeams(mThreats[mCheck].mTeam, mCheck, mThreats[mCheck].mActive);
         }
      }
      mCheck++;
   }
   else {
      // Skip the reserved first threat- 
      mCheck = 1;
      canCheckMore = false;
   }
   return canCheckMore;
}

// Called frequently - but only do at most ThreatCheckFrequency times a second.  
void SearchThreats::monitorThreats()
{
   U32   T = Sim::getCurrentTime();

   if ( (T - mSaveTimeMS) > ThreatCheckDelay )
   {
      mSaveTimeMS = T;
      while (checkOne())
         ;
   }
}

// The path advancer uses this to see if a leg it wants to advance along
// goes through any registered threats.  
bool SearchThreats::sanction(const Point3F& from, const Point3F& to, S32 team) const
{
   if (mCount) 
   {
      const SearchThreat   *  threatList[MaxThreats];
      if (S32 N = getThreatPtrs(threatList, getThreatSet(team))) 
      {
         LineSegment    travelLine(from, to);
         while (--N >= 0) 
         {
            const SphereF  *  sphere = threatList[N];
            if (sphere->isContained(from) || sphere->isContained(to))
               return false;
            if (travelLine.distance(sphere->center) < sphere->radius)
               return false;
         }
      }
   }
   return true;
}

// Just used for some debug rendering of those nodes in view of threats specified
// by console debug vars.  
GraphThreatSet NavigationGraph::showWhichThreats() const 
{
   if (sShowThreatened >= 0)
      if (S32 N = mThreats.numThreats()) 
         return GraphThreatSet(1) << (sShowThreatened % N);
   return 0;
}

bool NavigationGraph::installThreat(ShapeBase * threat, S32 team, F32 rad)
{
   if (mIsSpawnGraph)
   {
      // Just a spawn graph...  
      return true;
   }
   else
   {
      U32   memUsedBefore = Memory::getMemoryUsed();
      SearchThreat   newThreat(threat, rad, team);
      bool  success = mThreats.add(newThreat);
      sLoadMemUsed += (Memory::getMemoryUsed() - memUsedBefore);
      return success;
   }
}

