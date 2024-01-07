//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------
// Mark islands using the visitation callback of GraphSearch, set up mIslandPtrs
// list.  Happens on a freshly made graph (island mark bit on nodes is clear).  

class IslandMarker : public GraphSearch
{
   public:
      S32      mCurIsland;
      void     onQExtraction();
      F32      getEdgeTime(const GraphEdge*);
};

void IslandMarker::onQExtraction()
{
   extractedNode()->setIsland(mCurIsland);
}

// We now have worlds where the island filler won't flood up, but will flood
// down, which results in an error.  The default searcher stops when an 
// "infinite" distance is encountered, so we need to override it here.  
F32 IslandMarker::getEdgeTime(const GraphEdge*)
{
   return 1.0;
}

S32 NavigationGraph::markIslands()
{
   mIslandPtrs.clear();
   mIslandSizes.clear();
   mNonTransient.clearFlags(GraphNode::GotIsland);
   mLargestIsland = -1;   
   
   S32   i, N, maxCount = -1;

   // Do island-mark expansions until we have no more unmarked nodes
   IslandMarker  islandMarker;
   islandMarker.mCurIsland = 0;
   for (i = 0; i < mNonTransient.size(); i++)
   {
      if (GraphNode * node = mNonTransient[i]) 
      {
         if (!node->gotIsland()) 
         {
            N = islandMarker.performSearch(node, NULL);
            if (N > maxCount) 
            {
               maxCount = N;
               mLargestIsland = islandMarker.mCurIsland;
            }
            mIslandSizes.push_back(N);
            mIslandPtrs.push_back(node);
            islandMarker.mCurIsland++;
         }
         
         if (RegularNode * regular = dynamic_cast<RegularNode*>(node))
            regular->transientReserve();
      }
   }
   
   // return total island count 
   return mIslandPtrs.size();
}

//-------------------------------------------------------------------------------------

//
// Culls all indoor nodes that are not part of the largest island.  
//
S32 NavigationGraph::cullIslands()
{
   S32            originalCount = mNonTransient.size();
   Vector<S32>    cullList(originalCount);

   for(S32 i = 0; i < mNonTransient.size(); i++)
      if (mNonTransient[i]->indoor() && mNonTransient[i]->island() != mLargestIsland)
         cullList.push_back(i);
   
   // remap indices and do whatever culling was found- 
   if (cullList.size())
      compactIndoorData(cullList, mNumOutdoor);
   
   Con::printf("Original count was %d nodes", originalCount);
   Con::printf("%d nodes were culled", cullList.size());

   return cullList.size();
}
