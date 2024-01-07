//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

static void cantFind(const Point3F& P)
{
   Con::printf("Can't find node near (%f %f %f)", P.x, P.y, P.z);
}

// See if the given player can get from point A to point B.  
bool NavigationGraph::testPlayerCanReach(Point3F A, Point3F B, const F32* ratings)
{
   Vector<S32>    indexList;
   bool           success = false;
   GraphSearch *  searcher = getMainSearcher();
      
   if (GraphNode * src = closestNode(A))
   {
      if (GraphNode * dst = closestNode(B))
      {
         searcher->setAStar(true);
         searcher->performSearch(src, dst);
         searcher->setRatings(ratings);
         success = searcher->getPathIndices(indexList);
      }
      else
         cantFind(B);
   }
   else 
      cantFind(A);

   return success;
}

//-------------------------------------------------------------------------------------

F32 NavigationGraph::performTests(S32 count, bool enableAStar)
{
   // Seed off count - so test happens same for same count.
   MRandomR250    rand(count);
   S64            dijkstraIters = 0;
   S64            relaxCount = 0;
   U32            numSearchesDone = 0;
   Vector<S32>    indexList;

   // Get our searcher- 
   GraphSearch *  searcher = getMainSearcher();
      
   // Perform random path searches.  numNodes() ignores transients.  
   while (--count >= 0)
   {
      if (GraphNode * src = mNodeList[(rand.randI()&0xFFFFFFF) % numNodes()])
      {
         if (GraphNode * dst = mNodeList[(rand.randI()&0xFFFFFFF) % numNodes()]) 
         {
            numSearchesDone++;
            searcher->setAStar(enableAStar);
            dijkstraIters += searcher->performSearch(src, dst);
            relaxCount += searcher->relaxCount();
            searcher->getPathIndices(indexList);
         }
      }
   }

   if (numSearchesDone) 
   {
      if (relaxCount) 
      {
         F64   relaxAverage = F64(relaxCount) / F64(numSearchesDone);
         Con::printf("Average edge relaxations = %f", F32(relaxAverage));
      }
      return F32(F64(dijkstraIters) / F64(numSearchesDone));
   }
   
   return 0;
}

//-------------------------------------------------------------------------------------

// Flag (for render) those nodes having LOS from the given closest point.  We check
// in and out radii, and the condition is what we compare to.  Used for testing.  
void NavigationGraph::markNodesInSight(const Point3F& from, F32 in, F32 out, U32 cond)
{
   if (!validLOSXref()) 
   {
      warning("markNodesInSight() called without valid XRef table");
      return;
   }

   if (GraphNode * src = closestNode(from))
   {
      S32   srcInd = src->getIndex();
      for (S32 i = 0; i < mNonTransient.size(); i++) 
      {
         GraphNode   * cur = mNonTransient[i];
         F32   dist = (from - cur->location()).len();
         S32   curInd = cur->getIndex();
         bool  markIt = false;
         
         if (dist > in && dist < out && curInd != srcInd)
            markIt = (mLOSTable->value(srcInd, curInd) == cond);
         
         if (markIt)
            cur->set(GraphNode::Render0);
         else
            cur->clear(GraphNode::Render0);
      }
   }
   else
      warning("markNodesInSight() couldn't find closest node");
}

