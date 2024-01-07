//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "platform/profiler.h"

//-------------------------------------------------------------------------------------

// Go through the construct function since we want to reconstruct on mission cycle. 
NavigationPath::NavigationPath()
{
   constructThis();
}

void NavigationPath::constructThis()
{
   mState.constructThis();
   mCurEdge = NULL;
   mTimeSlice = 0;
   mSaveSeekNode = -1;
   mSearchDist = 0.0f;
   mUserStuck = true;
   mForceSearch = true;
   mRepathCounter = 10000;
   mSaveDest.set(1e7, 1e7, 1e7);
   setRedoDist();
   mPctThreshSqrd = 1.0;
   mPathIndoors = mAreIndoors = false;
   mAwaitingSearch = false;
   mTeam = 0;
   mBusyJetting = false;
   mSearchWasValid = true;
   mAdjustSeek.set(0,0,0);
   dMemset(mSavedEndpoints, 0, sizeof(mSavedEndpoints));
   GraphEdge   newEdge;
   mSaveLastEdge = newEdge;
   mStopCounter = 0;
   mCastZ = mCastAng = 0.0;
   mCastAverage.set(0,0,0);
   mEstimatedEdge = NULL;
   mEstimatedEnergy = 0.0;
   mFindHere.init();
}

// Since some of the code is in elsewhere (graphSmooth), we should package up the 
// relevant state for it...  Doesn't seem like the best organization, but the idea is 
// that the volume code "knows" more about smoothing paths indoor than we do here, 
// and so that code should be separated...  
void NavigationPath::State::constructThis()
{
   thisEdgeJetting = false;
   nextEdgeJetting = false;
   nextEdgePrecise = false;
   curSeekNode = 0;
   seekLoc.set(0,0,0);
   path.clear();
   visit.clear();
   edges.clear();
   hereLoc.set(0,0,0);
   destLoc.set(0,0,0);
}

// Fetch the node along the path, undoing any bit-flipped Transient indices.
GraphNode * NavigationPath::getNode(S32 idx)
{
   S32   node = mState.path[idx];
   S32   toggle = -S32(node < 0);
   return gNavGraph->lookupNode(node ^ toggle);
}

// Get location in path, handling case where endpoints were transients.  
Point3F NavigationPath::getLoc(S32 idx)
{
   S32  node = mState.path[idx];
   if(node >= 0)
      return gNavGraph->lookupNode(node)->location();
   else
      return mSavedEndpoints[idx > 0];
}

// Save endpoints of search (flagged with negative indices).  A NULL dstNode
// is passed for searches where destination is a node in graph.  
void NavigationPath::saveEndpoints(TransientNode* srcNode, TransientNode* dstNode)
{
   mSavedEndpoints[0] = srcNode->location();
   mState.path.first() ^= S32(-1);
   if (dstNode)
   {
      mSavedEndpoints[1] = dstNode->location();
      mState.path.last() ^= S32(-1);
   }
   
   // These edges are used for the path advancing.  Must take care with edges 
   // coming off of the transients - the last one must be saved since it is popped 
   // after the search (source connection INTO graph remains though)
   setSizeAndClear(mState.edges, getMax(mState.path.size()-1, 0));
   for (S32 i = mState.edges.size(); i > 0; i--) 
   {
      GraphEdge   * edgePtr = getNode(i-1)->getEdgeTo(getNode(i));
      AssertFatal(edgePtr, "All edges should exist in path");
      mState.edges[i - 1] = edgePtr;
   }
   if (mState.edges.size()) 
   {
      mSaveLastEdge = * mState.edges.last();
      mState.edges.last() = & mSaveLastEdge;
   }
}

// This is for renderer - though we'll use a similar node marking scheme for 
// implementing avoidance / randomization of paths.  
void NavigationPath::markRenderPath()
{
   for(S32 i = getMax(mState.curSeekNode - 1, 0); i < mState.path.size(); i++)
      getNode(i)->setOnPath();
}

// Revised path search to go off of the transient nodes.  
void NavigationPath::computePath(TransientNode& srcNode, TransientNode& dstNode)
{
   mSearchDist = 0.0f;
   mSearchWasValid = false;

   // Perform the search if the push indicates these two can (probably) reach.  
   if (gNavGraph->pushTransientPair(srcNode, dstNode, mTeam, mJetCaps))
   {
      PROFILE_START(PathComputation);
      
      mState.path.clear();
      mState.curSeekNode = 0;
      GraphSearch * searcher = gNavGraph->getMainSearcher();
      searcher->setAStar(true);
      searcher->setTeam(mTeam);
      searcher->setThreats(gNavGraph->getThreatSet(mTeam));
      searcher->setRandomize(true);
      #if _GRAPH_PART_
      searcher->setRatings(gNavGraph->jetManager().getRatings(mJetCaps));
      #endif
      
      searcher->performSearch(&srcNode, &dstNode);

      // Path fetcher tells us if search failed. 
      if (searcher->getPathIndices(mState.path))
      {
      
      
      
         mSearchDist = searcher->searchDist();
         setSizeAndClear(mState.visit, mState.path.size());
         saveEndpoints(&srcNode, &dstNode);
         mState.curSeekNode = 1;
         setEdgeConstraints();
         mAwaitingSearch = false;
         mSearchWasValid = true;
      }
      else
      {  
         // Note - we need to know if the transient connections into the graph were
         // all make-able by the bot (they should all go through the same evaluation 
         // to check the connections).  I think this assert arose because a bot had 
         // transient connection that couldn't be made.  
         //  // A search should only fail due to force fields.  Armor capabilities should be 
         //  // adequately predicted in advance.  
         //  AssertFatal(gNavGraph->haveForceFields(), "Failed to predict failure");
         //  
         //  // Force field management will update this team's partition accordingly.  
         //  gNavGraph->newPartition(searcher, mTeam);
         
         if (gNavGraph->haveForceFields())
         {
            // Force field management will update this team's partition accordingly.  
            gNavGraph->newPartition(searcher, mTeam);
         }
      }
      PROFILE_END();    // PathComputation
   }
   else {
      #if _GRAPH_WARNINGS_
         NavigationGraph::warning("Search tried across islands or partitions");
      #endif
   }
   
   // Unconnects from graph.  
   gNavGraph->popTransientPair(srcNode, dstNode);
}

// Just want all the post-pathCompute stuff separated out:
void NavigationPath::afterCompute()
{
   mSaveDest = mState.destLoc;
   mForceSearch = false;
   mRepathCounter = 0;
   if ( mRedoMode == OnPercent ) {
      F32   pctThresh = (mRedoPercent * mSearchDist);
      mPctThreshSqrd = (pctThresh * pctThresh);
   }
   mUserStuck = false;
}

bool NavigationPath::updateTransients(TransientNode& hereNode, 
                           TransientNode& destNode, bool forceRedo)
{
   // Nodes could be invalid- 
   if (mHook.iGrowOld())
      mLocateHere.reset(), mLocateDest.reset();

   if (forceRedo)
      mLocateHere.forceCheck(), mLocateDest.forceCheck();

   // This does all the work of figuring out how to connect- 
   mLocateHere.update(mState.hereLoc);
   mLocateDest.update(mState.destLoc);
   
   // Install connections found, and update locs- 
   hereNode.setEdges(mLocateHere.getEdges());
   destNode.setEdges(mLocateDest.getEdges());
   hereNode.setClosest(mLocateHere.bestMatch());
   destNode.setClosest(mLocateDest.bestMatch());
   hereNode.setLoc(mState.hereLoc);
   destNode.setLoc(mState.destLoc);
   
   // NOTE!  Only hook one way due to how Push-Search-Pop works (computePath() above). 
   bool  needToJet;
   if (mLocateHere.canHookTo(mLocateDest, needToJet)) {
      GraphEdge&  edge = hereNode.pushEdge(&destNode);
      if (needToJet)
         edge.setJetting();
      gNavGraph->jetManager().initEdge(edge, &hereNode, &destNode);
   }
   
   return forceRedo;       // not used
}

//-------------------------------------------------------------------------------------
// Periodic path updates.  Return true if there are nodes to traverse.  

#define  SearchWaitTicks   20
#define  AllowVisitTicks   21

bool NavigationPath::checkPathUpdate(TransientNode& hereNode, TransientNode& destNode)
{
   bool  recompute =  mForceSearch;
   // bool  recompute =  false;
   
   bool  changedSeekNode = (mSaveSeekNode != mState.curSeekNode);
   mSaveSeekNode = mState.curSeekNode;
   
   if (!recompute) {
      F32   threshold = (mRedoMode == OnDist ? mRedoDistSqrd : mPctThreshSqrd);
      bool  canSearch = (++mRepathCounter > SearchWaitTicks) && !userMustJet();
      bool  needSearch = ((mState.destLoc - mSaveDest).lenSquared() > threshold);
      
      if (mUserStuck)
         needSearch = true;

      if (needSearch) {
         // We need to make the canSearch check a little more strict and wait a little
         // extra time, or search when the node has been updated. 
         if (canSearch && (changedSeekNode || mRepathCounter > AllowVisitTicks))
            recompute = true;
         else
            mAwaitingSearch = true;
      }
   }
   if (recompute)
      mAwaitingSearch = true;
   
   updateTransients(hereNode, destNode, recompute);
   
   if (recompute) {
      computePath(hereNode, destNode);
      afterCompute();
   }

   if (weAreIndoors())
      mAreIndoors = true;
   else {
      if(checkOutsideAdvance())
         advanceByOne();
   }
      
   // Update path
   if (mState.curSeekNode < mState.path.size())
   {
      GraphNode * fromNode = (mState.curSeekNode > 0 ? getNode(mState.curSeekNode-1) : NULL);
      
      if (fromNode && gNavGraph->useVolumeTraverse(fromNode, mCurEdge))
      {
         S32 adv = gNavGraph->checkIndoorSkip(mState);
                                    
         if (adv)
         {
            // Con::printf("Advanced through %d indoor nodes", adv);
            while (adv--) 
            {
               AssertFatal(canAdvance(), "Nav path tried illegal advance");
               advanceByOne();
            }
         }
         else 
         {
            if (gNavGraph->volumeTraverse(fromNode, mCurEdge, mState))
            {
               if (canAdvance())    //==> Shouldn't it always be possible to advance?
                  advanceByOne();
            }
         }
      }
      else
      {
         mState.seekLoc = getLoc(mState.curSeekNode);
         F32      threshold = visitRadius() + 0.22;
         if(within_2D(mState.hereLoc, mState.seekLoc, threshold) && canAdvance()) {
            advanceByOne();
            if (mState.curSeekNode < mState.path.size())
               mState.seekLoc = getLoc(mState.curSeekNode);    // re-fetch!  
         }
      }
   }
   
   return (mState.curSeekNode < mState.path.size());
}

//-------------------------------------------------------------------------------------

// The consumer of the NavigationPath must call this every frame.  
bool NavigationPath::updateLocations(const Point3F& here, const Point3F& there)
{
   // This addition to the Z is important because we don't match to indoor nodes that 
   // we are below.  Not pretty, but the alternative is most extra LOS calls, or other 
   // potential ambiguities in the node matching process.  Also see the wall scanning
   // code below, which is 'cognizant' of these numbers.  
   (mState.hereLoc = here).z += 0.4;
   (mState.destLoc = there).z += 0.4;
   
   mAreIndoors = false;       // (default - get's changed if indoors)
   
   if( !NavigationGraph::gotOneWeCanUse()) {
      mState.seekLoc = mState.destLoc;
      mSearchWasValid = false;
   }
   else {
      // Threat manager needs to be called frequently- 
      gNavGraph->threats()->monitorThreats();
      gNavGraph->monitorForceFields();
   
      // Note hook nodes must be fetched every frame (in case of graph revisions).
      if (!checkPathUpdate(mHook.getHook1(), mHook.getHook2())) 
         mState.seekLoc = mState.destLoc;

      // Debug info-
      markRenderPath();
      
      // This just walks the graph looking for malfeasance
      // gNavGraph->patrolForProblems();
   }

   // Movement code sets while jetting, each set lasts for one frame
   mBusyJetting = false;

   // Busy-dec the stop counter, and check wall avoidance when non-zero.
   if (--mStopCounter <= 0)
      mStopCounter = 0;
   else
      checkWallAvoid();
      
   // Return status of the last search performed-
   return mSearchWasValid;
}

//-------------------------------------------------------------------------------------
//             Management of path advancing.  


// This assumes we have a graph and are seeking a node within the list.  
S32 NavigationPath::checkOutsideAdvance()
{
   if(canAdvance()) 
   {
      S32   seekNode = mState.curSeekNode + 1;
      if (seekNode < mState.path.size() && !mState.nextEdgeJetting)
      {  // See if we can get to the next one. We're doing this in 2D, which may be Ok
         // in the long run too, though we might track maximum heights along the way.  
         // First we check for registered threats along this segment.  
         Point3F  srcLoc(mState.hereLoc);
         Point3F  dstLoc(getLoc(seekNode));
         if (gNavGraph->threats()->sanction(srcLoc, dstLoc, mTeam))
         {
            srcLoc.z = dstLoc.z = 0.0f;
            F32  pct = gNavGraph->mTerrainInfo.checkOpenTerrain(srcLoc, dstLoc);
            if (pct == 1.0)
               return 1;
         }
      }
   }
   return 0;
}

bool NavigationPath::canAdvance()
{
   if (mState.curSeekNode < mState.path.size())
      if (!mState.thisEdgeJetting)
         return true;
   return false;
}

F32 NavigationPath::visitRadius()
{
   // if (mState.nextEdgeJetting || mAreIndoors || mPathIndoors)
   if (mState.nextEdgePrecise || mAreIndoors || mPathIndoors)
      return 0.4;
   else
      return 2.6;
}

// See if the next destination is a Jetting edge.  An assumption of this routine 
// is that it is only called when a new path is computed, or when the path is
// advanced.  Else problems - such as transient source having new connections
// which don't relate to the current path, etc.  
void NavigationPath::setEdgeConstraints()
{
   mState.thisEdgeJetting = false;
   mState.nextEdgeJetting = false;
   mState.nextEdgePrecise = false;
   mPathIndoors = false;
   mCurEdge = NULL;

   if (mState.curSeekNode > 0 && mState.curSeekNode < mState.path.size()) 
   {
      GraphNode   *  curSeekNode = getNode(mState.curSeekNode);
      GraphNode   *  prevSeekNode = getNode(mState.curSeekNode-1);
      
      mCurEdge = mState.edges[mState.curSeekNode - 1];
      
      if (curSeekNode->indoor() || prevSeekNode->indoor())
         mPathIndoors = true;
   
      mState.thisEdgeJetting = mCurEdge->isJetting();

      // Configure the edge.  Hop over is a U8 with 3 bits of precision.  
      if (mState.thisEdgeJetting) 
      {
         mNavJetting.init();
         if (mCurEdge->hasHop())
            mNavJetting.mHopOver = mCurEdge->getHop();
      }
      
      if (mState.thisEdgeJetting)
         mState.seekLoc = getLoc(mState.curSeekNode);
      
      if (mState.curSeekNode+1 < mState.path.size())
      {
         GraphEdge * nextEdge = mState.edges[mState.curSeekNode];
         mState.nextEdgeJetting = nextEdge->isJetting();
         
         // This flags that we need to go to the node location.  
         mState.nextEdgePrecise = (mState.nextEdgeJetting || !nextEdge->isBorder());
      }
   }
}

// Called when a node has been visited.  Mark it for later avoidance (path randomizing)
// if such is enabled and reasonable.  Don't mark large terrain nodes or transients. 
void NavigationPath::randomization()
{
   GraphNode * node = getNode(mState.curSeekNode);
   if (!node->transient() && node->getLevel() <= 2)
      node->setAvoid(60000);
}

// The path is advanced through only two interface functions, this one and the next.  
void NavigationPath::advanceByOne()
{
   AssertFatal(canAdvance(), "advanceByOne() called incorrectly");
   randomization();
   mState.curSeekNode++;
   setEdgeConstraints();
}

// When user is done - they inform the system so it can advance.  
void NavigationPath::informJetDone()
{
   AssertFatal(mState.thisEdgeJetting, "informJetDone() only called when jetting");
   randomization();
   mState.curSeekNode++;
   setEdgeConstraints();
}

// User queries to see if they now must jet.  
bool NavigationPath::userMustJet() const
{
   return (mState.path.size() > 0 && mState.thisEdgeJetting);
}

void NavigationPath::forceSearch()
{
   if (userMustJet() && _GRAPH_WARNINGS_)
      NavigationGraph::warning("Searched forced while user is jetting");
   mForceSearch = true;
}

// Here we use the path randomization to hopefully get the guy off the path. We back up
// through the list of any that were skipped - since we may not have even come to those
// yet.  And then we go forward by one. 
void NavigationPath::informStuck(const Point3F& stuckLoc, const Point3F& stuckDest)
{
   stuckLoc; stuckDest;
   S32   start = getMax(mState.curSeekNode - 1, 1);
   S32   end = getMin(mState.curSeekNode + 1, mState.path.size() - 1);

   while (start > 1 && mState.visit[start].test(State::Skipped))
      start--;

   while (start < end) {
      GraphNode * node = getNode(start++);
      AssertFatal(!node->transient(), "informStuck() didn't skip transients");
      node->setAvoid(15 * 1000, true);
   }
      
   mUserStuck = true;
}

//-------------------------------------------------------------------------------------

U32   gCountStoppedTicks = 0;
// F32   gVelSquaredThresh = 0.1;
F32   gVelSquaredThresh = -1.0;     // Take out for moment - needs more testing.

// When not moving but should be, this gets called - when in Express/Walk mode.  
void NavigationPath::informProgress(const VectorF& vel)
{
   F32   velSquared = vel.lenSquared();
   if (velSquared < gVelSquaredThresh)
   {
      if ((mStopCounter += 2) > 10)
         gCountStoppedTicks++;
   }
   else
   {
      // Once not stopped, then limit how much we have to count down.  But we do 
      // want that time as well for fading the running mCastAverage below.  
      mStopCounter = getMin(mStopCounter, 7);
   }
}

// A nice angle for scanning around with good coverage and consistent change.  These 
// are relative prime numbers a little over 3/8 of the cycle points (360.0, 1.90000)
static const F64  sCycleAngle = mDegToRad(F64(137.3));
static const F64  sCycleHeight = 0.37747;
static const U32  sCycleMask = (InteriorObjectType|TerrainObjectType);
static const F32  sAverageTheNew = (1.0 / 8.0);
static const F32  sAverageTheOld = (1.0 - sAverageTheOld);

// Monitor the stop counter.  When it persists, look for walls and such to move 
// away from.  We do this with rotating LOS checks, which also have to move up and 
// down to have the best chance of finding obstacles.  Cycling on 1.9 height covers
// range from 0.4 to 2.3 (see above addition to supplied here loc z).  
void NavigationPath::checkWallAvoid()
{
   if (mStopCounter > 3) 
   {
      if ((mCastAng += sCycleAngle) > M_2PI)
         mCastAng -= M_2PI;
      if ((mCastZ += sCycleHeight) > 1.9)
         mCastAng -= 1.9;
         
      F32      angle32 = F32(mCastAng);
      Point3F  startPoint( mState.hereLoc.x, mState.hereLoc.y, mState.hereLoc.z + F32(mCastZ) );
      Point3F  lineOut( mCos(angle32), mSin(angle32), 0);

      // Get the endpoint out a little ways.  Find intersection and convert back
      // to an offset which we'll roll into running average of offsets.
      (lineOut *= 7.0) += startPoint;
      RayInfo  coll;
      gServerContainer.castRay(startPoint, lineOut, sCycleMask, &coll);
      coll.point -= startPoint;
      
      // Get the running average- 
      coll.point *= sAverageTheNew;
      mCastAverage *= sAverageTheOld;
      mCastAverage += coll.point;
   }
   else
   {
      // In first few frames free, or first frames stuck, fade down the average.  
      mCastAverage *= (2.0 / 3.0);
   }
}

static Point3F reignIn(const Point3F& S, Point3F D, F32 cap)
{
   D -= S; 
   if (D.lenSquared() > (cap * cap))
      D.normalize(cap);
   return D += S;
}

// This is where user fetches location to seek.  We now do additional checks to 
// reign in this distance when near steep things. 
const Point3F& NavigationPath::getSeekLoc(const VectorF&) 
{
   // if (NavigationGraph::gotOneWeCanUse())
   //    mAdjustSeek = reignIn(mState.hereLoc, mState.seekLoc, 10.0);
   // else 
   if (mStopCounter)
   {
      // Seek away from collision point if that exists- 
      mAdjustSeek = mCastAverage;
      mAdjustSeek *= F32(mStopCounter);
      mAdjustSeek += mState.hereLoc;
   }
   else
      mAdjustSeek = mState.seekLoc;
      
   return mAdjustSeek;
}

//-------------------------------------------------------------------------------------

// Convey the jetting ability to the jet manager.  We let the aiConnection go ahead
// and retrieve this data from Player since we've so far kept from #including Player.h.
void NavigationPath::setJetAbility(const JetManager::Ability & ability)
{
   if (gNavGraph) {
      if (_GRAPH_PART_ && gNavGraph->jetManager().update(mJetCaps, ability)) {
         // A bunch of work just happened, let's forestall searches a little- 
         mRepathCounter = getMin(SearchWaitTicks >> 1, mRepathCounter);
      }
   }
}

//-------------------------------------------------------------------------------------

// How far across open terrain user can go from src to dst.  dstLoc parameter is 
// updated accordingly, and a percentage from 0 to 1 is returned.  
F32 NavigationPath::checkOpenTerrain(const Point3F& srcLoc, Point3F& dstLoc)
{
   if (NavigationGraph::gotOneWeCanUse())
      return gNavGraph->mTerrainInfo.checkOpenTerrain(srcLoc, dstLoc);
   dstLoc = srcLoc;
   return 0.0;
}

//-------------------------------------------------------------------------------------
//          A couple of path lookahead functions- 

// Get a point that is dist along the path.  
Point3F NavigationPath::getLocOnPath(F32 dist)
{
   Point3F  currLoc(mState.hereLoc);
   
   for (S32 i = mState.curSeekNode; i < mState.path.size() && dist > 0.01; i++)
   {
      Point3F  nextLoc = getLoc(i);
      VectorF  diffVec = nextLoc;
      F32      len = (diffVec -= currLoc).len();
      
      if (len < dist)
      {
         currLoc = nextLoc;
         dist -= len;
      }
      else 
      {
         currLoc += (diffVec *= (dist / len));
         break;
      }
   }
   return currLoc;
}

// Find distance remaining on path by stepping through it - stop if user has given a
// threshold dist outside of which they don't care (defaults to huge).  
F32 NavigationPath::distRemaining(F32 maxCare)
{
   Point3F  currLoc(mState.hereLoc);
   F32      dist = 0.0f;
   
   for (S32 i = mState.curSeekNode; i < mState.path.size(); i++) 
   {
      Point3F  nextLoc = getLoc(i);
      
      if ((dist += (nextLoc - currLoc).len()) > maxCare)
         return maxCare;
         
      currLoc = nextLoc;
   }
   return dist;
}

//-------------------------------------------------------------------------------------

// Find next edge that requires jetting if it's within maxDist.  If found, then we 
// estimate the needed energy to complete the hop.  Since the estimation is a little
// lengthy - we remember which edge we computed for and only do it once.  
F32 NavigationPath::jetWillNeedEnergy(F32 maxDist)
{
   if (!mState.thisEdgeJetting)
   {
      F32   accumDist = 0.0f;
      for (S32 i = mState.curSeekNode; i < mState.edges.size(); i++) 
      {
         // Let's only do a len() calculation to the first node, otherwise use distance 
         // on the edge. (First len() required because the bot's moving).
         F32   legDist;
         if (i == mState.curSeekNode)
            legDist = (getLoc(i) - mState.hereLoc).len();
         else
            legDist = mState.edges[i - 1]->mDist;
         
         if ((accumDist += legDist) < maxDist)
         {
            GraphEdge * nextEdge = mState.edges[i];
            if (nextEdge->isJetting())
               return estimateEnergy(nextEdge);
         }
      }
   }
   return 0.0;
}

// Get the jet manager's estimation, and remember that we did this work for this 
// edge since JetManager::estimateEnergy() does a bit o' math. 
F32 NavigationPath::estimateEnergy(const GraphEdge * edge)
{
   if (mEstimatedEdge != edge)
   {
      mEstimatedEnergy = gNavGraph->jetManager().estimateEnergy(mJetCaps, edge);
      mEstimatedEdge = edge;
   }
   return mEstimatedEnergy;
}

//-------------------------------------------------------------------------------------

// Return true if we are outside, and set the roam radius (in param) if so.
bool NavigationPath::locationIsOutdoors(const Point3F &location, F32* roamRadPtr)
{
   F32   roamRadius = 1e12;      // Default is outdoors, with unlimited roam room.  
   
   if (NavigationGraph::gotOneWeCanUse())
   {
      if (!gNavGraph->haveTerrain())
         return false;
   
      Point3F  tempLocation(location.x, location.y, 0.0f);
      if (gNavGraph->mTerrainInfo.inGraphArea(tempLocation))
      {
         SphereF  sphere;
         if (gNavGraph->mTerrainInfo.locToIndexAndSphere(sphere, tempLocation) >= 0)
            roamRadius = sphere.radius;      // fall through to true
         else
            return false;
      }
   }
           
   if (roamRadPtr)
      *roamRadPtr = roamRadius;
      
   return true;
}

// By default (with no graph), this is false.  
bool NavigationPath::weAreIndoors() const
{
   if (NavigationGraph::gotOneWeCanUse())
      if (!gNavGraph->haveTerrain())
         return true;
      else if (gNavGraph->mTerrainInfo.inGraphArea(mState.hereLoc))
         return !gNavGraph->findTerrainNode(mState.hereLoc);
         
   return false;
}

bool NavigationPath::getPathNodeLoc(S32 ahead, Point3F& loc)
{
   S32   dest = mState.curSeekNode + ahead;
   
   if (dest < mState.path.size() && dest > 0)
   {
      GraphEdge   * beforeEdge = mState.edges[dest - 1];
      if (!beforeEdge->isJetting())
      {
         if (beforeEdge->isBorder())
            loc = gNavGraph->getBoundary(beforeEdge->mBorder).seekPt;
         else
            loc = getNode(dest)->location();
         return true;
      }
   }
   return false;
}

// Does the bot have to jet into a vehicle?  
bool NavigationPath::intoMount() const
{
   if (mLocateDest.isMounted())
      if (mState.curSeekNode == mState.path.size() - 1)
         return true;
   return false;
}

bool NavigationPath::canReachLoc(const Point3F& dst)
{
   if (NavigationGraph::gotOneWeCanUse())
   {
      // mFindHere is a FindGraphNode which remembers results of closest node searches.  
      mFindHere.setPoint(mState.hereLoc, mFindHere.closest());
      FindGraphNode  findDest(dst);
      return gNavGraph->canReachLoc(mFindHere, findDest, mTeam, mJetCaps);
   }
   else
      return true;
}


void NavigationPath::missionCycleCleanup()
{
   constructThis();
   mJetCaps.reset();
   mNavJetting.init();
   mHook.reset();
   mFindHere.init();
   mLocateHere.cleanup();
   mLocateDest.cleanup();
}

