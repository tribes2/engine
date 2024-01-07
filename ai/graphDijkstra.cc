//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

// Two special back-index entries are defined for if element has been extracted or 
// has not been in the queue yet.  DefNotInQueue uses -1 so we can memset() array. 
#define  DefExtracted         (1<<30)
#define  DefNotInQueue        GraphQIndex(-1)
#define  MaskExtracted        (DefExtracted-1)
#define  NodeExtracted(x)     (((x) & DefExtracted) && (x) > 0)
#define  FlagExtracted(x)     ((x)|= DefExtracted)
#define  NotInQueue(x)        ((x)==DefNotInQueue)
#define  DoDiagnostic(x)      {x;}

// The Q and Q indices are shared among all searchers.  They're kept relatively private
// by using a GraphSearch subclass which is only friends with GraphSearch.  
// 
GraphSearch::GraphSearch() 
   :  mHead(-1, F32(-1), F32(-1)), 
      mQueue(gNavGraph->mSharedSearchLists.searchQ), 
      mQIndices(gNavGraph->mSharedSearchLists.qIndices), 
      mHeuristicsVec(gNavGraph->mSharedSearchLists.heuristics), 
      mPartition(gNavGraph->mSharedSearchLists.partition)
{
   mHead.mTime = -1;
   mExtractNode = NULL;
   mVisitOnExtract = true;
   mVisitOnRelax = true;
   mEarlyOut = true;
   mSearchDist = 0.0f;
   mTargetNode = -1;
   mSourceNode = -1;
   mIterations = 0;
   mRelaxCount = 0;
   mThreatSet = 0;
   mInformThreats = 0;
   mInformTeam = 0;
   mInformRatings = NULL;
   mInProgress = false;
   mAStar = false;
   mTargetLoc.set(0,0,0);
   initializeIndices();
   mHeuristicsPtr = NULL;
   mRandomize = false;
}

bool GraphSearch::initializeIndices()
{
   if (NavigationGraph::gotOneWeCanUse())
   {
      if (gNavGraph->numNodesAll() != mQIndices.size()) 
      {
         S32   totalNodes = gNavGraph->numNodesAll();
         AssertFatal(totalNodes < (1 << 15), "Graph size can't exceed 32K");
         mHeuristicsVec.setSize(totalNodes);
         mQIndices.setSize(totalNodes);
         mPartition.setSize(totalNodes);
         
         // Largest search includes just two transients
         mQueue.reserve(gNavGraph->numNodes() + 2);
         
         doLargeReset();
         return true;
      }
   }
   return false;
}

void GraphSearch::doSmallReset()
{
   mPartition.clear();
#if 1
   register    S32            i = mQueue.size();
   register    SearchRef   *  searchRef = &mQueue[0];
   while (--i >= 0) {
      mQIndices[searchRef->mIndex] = DefNotInQueue;
      mHeuristicsVec[ (searchRef++)->mIndex ] = 0;
   }
#else
   for (S32 i = mQueue.size() - 1; i >= 0; i--) {
      S32   whichNode = mQueue[i].mIndex;
      mQIndices[whichNode] = DefNotInQueue;
      mHeuristicsVec[whichNode] = 0;
   }
#endif
}

void GraphSearch::doLargeReset()
{
   dMemset(mQIndices.address(), 0xff, mQIndices.memSize());
   dMemset(mHeuristicsVec.address(), 0, mHeuristicsVec.memSize());
   mPartition.clear();
}

void GraphSearch::resetIndices()
{
   if (! initializeIndices()) {
      if (mQueue.size()) {    // Compare size of search queue to total node count.
         S32   ratio = mQIndices.size() / mQueue.size();
         if (ratio > 7) {
            doSmallReset();
            return;
         }
      }
      //==> Note could queue size sometimes be clear and the following would happen?
      doLargeReset();
   }
}

// Mainly for access to accumulated userdata on the nodes- 
SearchRef* GraphSearch::getSearchRef(S32 nodeIndex) 
{
   S32   queueInd = mQIndices[nodeIndex];
   AssertFatal(!NotInQueue(queueInd), "getSearchRef() called with non-queued node");
   return &mQueue[queueInd & MaskExtracted];
}

// This version is just a straight lookup using a Q index.  
SearchRef* GraphSearch::lookupSearchRef(S32 queueIndex) 
{
   return &mQueue[queueIndex & MaskExtracted];
}

inline SearchRef* GraphSearch::insertSearchRef(S32 nodeIndex, F32 dist, F32 sort)
{
   S32   vecIndex = mQueue.size();
   mQIndices[nodeIndex] = vecIndex;
   SearchRef   assemble(nodeIndex, dist, sort);
   mQueue.insert(assemble);
   return &mQueue[vecIndex];
}

// Called when randomizing to get edge scale factor. Turn off randomizing after a point
// to limit the slowdown caused by the (sometimes much) larger search expansion.
S32 GraphSearch::getAvoidFactor()
{
   if (mIterations > 80)
      mRandomize = false;
   else {
      // Note unsigned compare important - 
      U32   timeDiff = (mExtractNode->avoidUntil() - mCurrentSimTime);
      if (timeDiff < GraphMaxNodeAvoidMS)
         if (mExtractNode->stuckAvoid())
            return 400;
         else
            return 25;
   }
   return 0;
}

F32 GraphSearch::calcHeuristic(const GraphEdge* edge)
{
   F32   dist = mHeuristicsPtr[edge->mDest];
   if (dist == 0) 
   {
      #if 0
      GraphNode * destNode = gNavGraph->lookupNode(edge->mDest);
      dist = (destNode->location() - mTargetLoc).len();
      #else
      // ==>  Want to try a crude distance function here to avoid square root.
      RegularNode *  destNode = static_cast<RegularNode *>(gNavGraph->lookupNode(edge->mDest));
      dist = (destNode->mLoc - mTargetLoc).len();
      #endif
      mHeuristicsPtr[edge->mDest] = dist;
   }
   return dist;
}

// Compute new time along the edge.  This is virtual since special searches 
// want to play with it (cf. dist-only based searches, LOS avoid searches).
F32 GraphSearch::getEdgeTime(const GraphEdge* edge)
{
   F32   edgeTime = (edge->mDist * edge->getInverse());
   
   AssertFatal(!edge->problems(), edge->problems());

   // Weight for jetting capabilities- edges that can't be traversed.  Don't do it with
   // transient connections (partition predictions must work out true).  
   if (mJetRatings && edge->isJetting() && edge->mDest < mTransientStart)
      if (!edge->canJet(mJetRatings))
         return SearchFailureAssure;
      
   // Edges that only one team can go through, namely (working) force fields-
   if (mTeam && edge->getTeam()) 
      if (edge->getTeam() != mTeam)
         return SearchFailureAssure;
      else 
         edgeTime *= 1.3;

   return edgeTime;
}

bool GraphSearch::runDijkstra()
{
   mCurrentSimTime = Sim::getCurrentTime();

   while (SearchRef * head = mQueue.head())
   {
      // Make COPY of head since list can move!  Also, 'visitors' make use of the info.
      mHead = * head;
      mQueue.removeHead();

      // This means search failed
      if (mHead.mTime > SearchFailureThresh)
         break;

      // Mark visited-
      mPartition.set(mHead.mIndex);
      
      // Check if done- 
      if (mTargetNode == mHead.mIndex) {
         mSearchDist = mHead.mDist;
         break;
      }

      // set the extracted node.  visitor may use it.  
      mExtractNode = gNavGraph->lookupNode(mHead.mIndex);
      
      // callback visit - subclass uses extractedNode() to access current
      if (mVisitOnExtract)
         onQExtraction();

      // Avoidance of nodes for randomize or for stuckness.  
      S32   avoidThisNode = (mRandomize ? getAvoidFactor() : false);
      
      // Check for threat avoidance.  
      bool  nodeThreatened = (mThreatSet && (mExtractNode->threats() & mThreatSet));

      // Mark this node as extracted, remove from Queue.  Do after onQExtraction().  
      GraphQIndex headIndex = mQIndices[mHead.mIndex];
      FlagExtracted(mQIndices[mHead.mIndex]);
      
      // Relax all neighbors (or add to queue in first place)
      GraphEdgeArray  edgeList = mExtractNode->getEdges(mEdgeBuffer);
      while (GraphEdge * edge = edgeList++)
      {
         GraphQIndex             queueInd = mQIndices[edge->mDest];
         register SearchRef   *  searchRef;
         
         if (! NodeExtracted(queueInd))
         {
            F32   newDist = mHead.mDist + edge->mDist;
            F32   edgeTime = getEdgeTime(edge);
            if (nodeThreatened)
               edgeTime *= 10;
            if (avoidThisNode)
               edgeTime += avoidThisNode;
            F32   newTime = mHead.mTime + edgeTime;
            F32   sortOnThis = (mAStar ? newTime + calcHeuristic(edge) : newTime);
            
            // Relax dist for neighbor (1st time is a "relax" down from infinity)
            if (NotInQueue(queueInd)) {
               searchRef = insertSearchRef(edge->mDest, newDist, sortOnThis);
               searchRef->mPrev = headIndex;
               searchRef->mTime = newTime;
            }
            else if (sortOnThis < (searchRef = &mQueue[queueInd])->mSort) {
               searchRef->mTime = newTime;
               searchRef->mSort = sortOnThis;
               searchRef->mDist = newDist;
               searchRef->mPrev = headIndex;
               mQueue.changeKey(queueInd);
            }
            else
               continue;   // didn't relax - must skip relax callback

            DoDiagnostic(mRelaxCount++);
            if (mVisitOnRelax)
               onRelaxEdge(edge);
         }
      }
      
      mIterations++;
      
      if (mEarlyOut && earlyOut())
         return (mInProgress = true);
   }

   return (mInProgress = false);
}

void GraphSearch::initSearch(GraphNode * S, GraphNode * D)
{
   mIterations = 0;
   mRelaxCount = 0;
   mInProgress = true;
   mSearchDist = 0.0f;
   mTransientStart = gNavGraph->numNodes();
   mSourceNode = S->getIndex();

   // Set up target if present.  A* requires target.  
   if (D)
      mTargetNode = D->getIndex(), mTargetLoc = D->location();
   else 
      mTargetNode = -1, mInformAStar = false; 
   
   // These four search-modifying variables hold their value for only one search- 
   mAStar         = mInformAStar;      /*---------*/     mInformAStar      = false;
   mThreatSet     = mInformThreats;    /*---------*/     mInformThreats    = 0;
   mTeam          = mInformTeam;       /*---------*/     mInformTeam       = 0;
   mJetRatings    = mInformRatings;    /*---------*/     mInformRatings    = NULL;
   
   // Avoid bad inlining in debug build...
   mHeuristicsPtr = mHeuristicsVec.address();
   
   // Cleanup from last search-
   resetIndices();
   
   // Set source as first element in Q- 
   mQueue.clear();
   insertSearchRef(S->getIndex(), 0, 0)->mTime = 0;
   mQueue.buildHeap();
}

//-------------------------------------------------------------------------------------

// Find list of node indices on search that just happened. If search had target start 
// back from there, else use parameter (custom searches).  Return if target reached. 
bool GraphSearch::getPathIndices(Vector<S32> & indices, S32 target)
{
   indices.clear();
   
   if(target < 0)
      target = mTargetNode;
      
   AssertFatal(target >= 0, "Bad use of getPathIndices");

   if (mPartition.test(target)) 
   {
      // Get the list of node indices going back from target to source. Since these are
      // queue indices, 0 is start (source node is first in Q), hence the while (prev).
      S32   prev = (mQIndices[target] & MaskExtracted);
      if (prev < mQIndices.size()) 
      {
         while (prev) 
         {
            indices.push_back(mQueue[prev].mIndex);
            prev = mQueue[prev].mPrev;
         }
         indices.push_back(mSourceNode);
         reverseVec(indices);
         return true;
      }
   }
   return false;
}

// D can be NULL (used for visitation expansions).  
S32 GraphSearch::performSearch(GraphNode * S, GraphNode * D)
{
   if( D && D->island() != S->island() )
      return -1;
      
   initSearch(S, D);
   runDijkstra();
   
   return mIterations;
}

// Search that can be interupted.  Returns true while in progress.  User is responsible
// for not calling it again if the search has finished.  cf. earlyOut() virtual.  
bool GraphSearch::runSearch(GraphNode * S, GraphNode * D)
{
   if (D && D->island() != S->island())
      return false;

   if (!mInProgress)
      initSearch(S, D);
      
   return runDijkstra();
}

