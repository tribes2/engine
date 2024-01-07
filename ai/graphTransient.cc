//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------

GraphEdge * GraphNode::pushTransientEdge(S32 dest)
{
   // if (mEdges.isOwned())
   // {
   //    AssertFatal(!mOwnedPtr, "GraphNode::pushTransientEdge()");
   //    mOwnedPtr = mEdges.address();
   //    mOwnedCnt = mEdges.size();
   //    mEdges.clearOwned();
   //    mEdges.reserve(mOwnedCnt + 4);
   //    mEdges.setSize(mOwnedCnt);
   //    dMemcpy(mEdges.address(), mOwnedPtr, mOwnedCnt * sizeof(GraphEdge));
   // }

   // Hook back to the supplied destination-    
   GraphEdge   edge;
   edge.mDest = dest;
   edge.mDist = (mLoc - gNavGraph->lookupNode(dest)->location()).len();
   mEdges.push_back(edge);
   return & mEdges.last();
}

void GraphNode::popTransientEdge()
{
   // if (mOwnedPtr)
   // {
   //    AssertFatal(!mEdges.isOwned(), "GraphNode::popTransientEdge()");
   //    mEdges.setOwned(mOwnedPtr, mOwnedCnt, true);
   //    mOwnedPtr = NULL;
   //    mOwnedCnt = 0;
   // }
   // if (!mEdges.isOwned())
   //    mEdges.pop_back();
      
   mEdges.pop_back();
}

//-------------------------------------------------------------------------------------


TransientNode::TransientNode(S32 index)
{
   mGroundStart = -1;
   mFlags.clear();
   mFlags.set(Transient);
   mIndex = index;
   mClosest = NULL;
   mSaveClosest = NULL;
}

Point3F TransientNode::getRenderPos() const
{
   Point3F  buff, adjusted;
   adjusted = fetchLoc(buff);
   adjusted.z += 0.2;
   return adjusted;
}

// When asked for its list of edges, the transient node must return the list
// it used in the last search, not whatever it might currently think is it's
// best connection (it maintains these separately).  This is a little confusing,
// might want to re-think if it's the best way to handle it.  
GraphEdgeArray TransientNode::getEdges(GraphEdge*) const
{
   return GraphEdgeArray(mSearchEdges.size(), mSearchEdges.address());
}

// This method is used below to get the current best hooks.  
GraphEdgeArray TransientNode::getHookedEdges() const
{
   return GraphEdgeArray(mEdges.size(), mEdges.address());
}

// This is weird, but I'm avoiding casts in the middle of the processing.  Allows
// node traverser to handle transients Ok.  
S32 TransientNode::volumeIndex() const 
{
   if (mSaveClosest && mSaveClosest->indoor())
      return mSaveClosest->volumeIndex();
   return -1;
}

//-------------------------------------------------------------------------------------
//             Interface function for pairs of dynamic nodes

// Remove the two hook nodes referenced by the index. Should always be nodes there
// if proper version management (using 'incarnation' variables) is happening.  
void NavigationGraph::destroyPairOfHooks(S32 pairLookup)
{
   AssertFatal(validArrayIndex(pairLookup,mNodeList.size()-1) && 
         mNodeList[pairLookup] && mNodeList[pairLookup + 1], 
               "Poor GraphHookRequest communication" );

   for(S32 i = 0; i < 2; i++) {
      delete mNodeList[pairLookup + i];
      mNodeList[pairLookup + i] = NULL;
   }
}

// Note that NULL entries are fleshed out at the end of graphMake (in doFinalFixups()).
S32 NavigationGraph::createPairOfHooks()
{
   S32   findPair = mMaxTransients;
   while( (findPair -= 2) >= 0 )
      if(!mNodeList[ mTransientStart + findPair])
         break;
         
   AssertISV(findPair >= 0, "Out of dynamic graph connections (bot max exceeded?)");
   findPair += mTransientStart;
   AssertFatal(!mNodeList[findPair+1], "Free graph hooks must come in pairs");

   // allocate 'em
   for (S32 i = 0; i < 2; i++)
      mNodeList[findPair + i] = new TransientNode(findPair + i);
   
   return findPair;
}

//-------------------------------------------------------------------------------------

void mayBeFineButTrapIt()
{
}

// Transient nodes point INTO the graph, but the graph doesn't need or maintain hooks 
// back, except when a search is performed - see navPath.cc.  
//
// We also set the search edges here - we copy over those nodes that had LOS.  If 
// none have LOS, we keep at least the closest one.  
//
S32 NavigationGraph::hookTransient(TransientNode& transient)
{
   S32            retIsland = -7;
   S32            thisIndex = transient.getIndex();
   GraphEdgeArray edgeList = transient.getHookedEdges();
   Point3F        thisLoc = transient.mLoc;
   
   transient.mSearchEdges.clear();
   transient.mSaveClosest = transient.mClosest;
   transient.mFirstHook = NULL;
   
   while (GraphEdge * edge = edgeList++) 
   {
      GraphNode * hookTo = lookupNode(edge->mDest);
      S32         hookIsland = hookTo->mIsland;
      Point3F     destLoc = hookTo->location();
      
      if((retIsland != -7) && (hookIsland != retIsland))
         mayBeFineButTrapIt();
      else
         retIsland = hookIsland;

      // The first hook is used as a representative node to determine reachability 
      // across partitions.  
      if (!transient.mFirstHook && !hookTo->transient())
         transient.mFirstHook = hookTo;
      
      if (GraphEdge * edgeBack = hookTo->pushTransientEdge(thisIndex))
      {
         // Last edge of path may contain a border.  Note we have to reflect the 
         // border, which is easy since they come in pairs :)
         if (edge->isBorder())
            edgeBack->mBorder = (edge->mBorder ^ 1);
            
         if (edge->isJetting())
            edgeBack->setJetting();
            
         if (edge->getTeam())
            edgeBack->setTeam(edge->getTeam());
      }
      transient.mSearchEdges.push_back(*edge);
   }

   // restore the regular list.  
   transient.mEdges = transient.mSearchEdges;
   
   return (transient.mIsland = retIsland);
}

void NavigationGraph::unhookTransient(TransientNode & transient)
{
   GraphEdgeArray edgeList = transient.getEdges(NULL);
   while (GraphEdge * edge = edgeList++)
      lookupNode(edge->mDest)->popTransientEdge();
}


//-------------------------------------------------------------------------------------
// Balanced calls for hooking in pair of transients.  Hook returns if the two locations
// can find each other (or if it's ambiguous).  Determined by islands and partitions. 

bool NavigationGraph::pushTransientPair(TransientNode& srcNode, TransientNode& dstNode, 
         U32 team, const JetManager::ID& jetCaps)
{
   S32   island0 = hookTransient(dstNode);
   S32   island1 = hookTransient(srcNode);
   
   if (island0 == island1 && island0 >= 0)
   {
      // Islands Ok- check partitions- 
      S32   srcInd = srcNode.mFirstHook->getIndex();
      S32   dstInd = dstNode.mFirstHook->getIndex();
      
      GraphPartition::Answer  answer;
      jetCaps;
      
      // First check armor partition- it never gives ambiguous answers (since they are 
      // computed in full when a new energy/armor configuration occurs).  
      #if _GRAPH_PART_
      answer = mJetManager.reachable(jetCaps, srcInd, dstInd);
      if (answer == GraphPartition::CanReach)
      #endif
      {
         // Then check force fields.  Answer can be ambiguous (in which case the search
         // will try, and add to the partition list if it fails).  
         answer = mForceFields.reachable(team, srcInd, dstInd);
         if (answer == GraphPartition::CanReach || answer == GraphPartition::Ambiguous)
            return true;
      }
   }
   return false;
}
      
void NavigationGraph::popTransientPair(TransientNode& srcNode, TransientNode& dstNode)
{
   unhookTransient(srcNode);
   unhookTransient(dstNode);
}

//-------------------------------------------------------------------------------------

bool NavigationGraph::canReachLoc(const FindGraphNode& src, const FindGraphNode& dst, 
                              U32 team, const JetManager::ID& jetCaps)
{
   GraphNode * srcNode = src.closest();
   GraphNode * dstNode = dst.closest();
   
   if (srcNode && dstNode) {
      if (srcNode->island() == dstNode->island()) {
         // Islands Ok- check partitions- 
         S32   srcInd = srcNode->getIndex();
         S32   dstInd = dstNode->getIndex();
      
         GraphPartition::Answer  ans;
      
         // First check armor partition- it never gives ambiguous answers- 
         #if _GRAPH_PART_
         ans = mJetManager.reachable(jetCaps, srcInd, dstInd);
         if (ans == GraphPartition::CanReach)
         #endif
         {
            // Then check force fields.  Answer can be ambiguous- 
            ans = mForceFields.reachable(team, srcInd, dstInd);
            if (ans == GraphPartition::CanReach || ans == GraphPartition::Ambiguous)
               return true;
         }
      }
   }
   return false;
}

//-------------------------------------------------------------------------------------
// A path searcher goes through this interface to get a handle to a transient node.  

GraphHookRequest::GraphHookRequest()
{
   mIncarnation = -1;
   mPairLookup = -1;
}

GraphHookRequest::~GraphHookRequest()
{
   if (NavigationGraph::gotOneWeCanUse())
      if (gNavGraph->incarnation() == mIncarnation)
         gNavGraph->destroyPairOfHooks(mPairLookup);
}

// Called on mission cycle - basically a re-construct.  
void GraphHookRequest::reset()
{
   mIncarnation = -1;
   mPairLookup = -1;
}

// There CAN be GraphHookRequest objects constructed without there being a graph, but NOT 
// actual requests for nodes. This is why we don't assert above, but do in the following.
TransientNode & GraphHookRequest::getHook(S32 firstOrSecond)
{
   AssertFatal(NavigationGraph::gotOneWeCanUse(), "Called GraphHookRequest w/o graph");
   if(mIncarnation != gNavGraph->incarnation()) {
      mPairLookup = gNavGraph->createPairOfHooks();
      mIncarnation = gNavGraph->incarnation();
   }
   GraphNode   * theNode = gNavGraph->lookupNode(mPairLookup + firstOrSecond);
   AssertFatal(mPairLookup >= 0 && theNode != NULL, "Couldn not alloc transient node");
   AssertFatal(theNode->transient(), "Node must be transient!");

   TransientNode * t = static_cast<TransientNode *>(theNode);
   return *t;
}

bool GraphHookRequest::iGrowOld() const 
{
   AssertFatal( NavigationGraph::gotOneWeCanUse(), 
      "I hear the mermaids singing, each to each...   " 
      "I do not think they sing to me...  " );
   return gNavGraph->incarnation() != mIncarnation;
}
