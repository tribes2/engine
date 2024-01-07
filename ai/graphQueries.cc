//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------
// Get fast travel distance if we have a graph with a path XRef table.  First method is 
// static and checks for presence of graph, second does the path table walk.  

F32 NavigationGraph::fastDistance(const Point3F& p1, const Point3F& p2)
{
   if(gotOneWeCanUse())
      return gNavGraph->distancef(p1, p2);
   else 
   {
      warning("fastDistance() called without usable graph present");
      return (p1 - p2).len();
   }
}

F32 NavigationGraph::distancef(const Point3F& p1, const Point3F& p2)
{
   //       REMOVED TABLE SEARCHES
   // if (mValidPathTable) {
   //    GraphNode   *  src = closestNode(p1);
   //    GraphNode   *  dst = closestNode(p2);
   //    Vector<S32>    nodes;
   // 
   //    if (src && dst)
   //       if (src->island() == dst->island())
   //          return walkPathTable(src, dst, nodes);
   //       else
   //          warning("distancef() attempted across separate islands");
   //    else 
   //       warning("distancef() failed to find closest node(s)");
   // }
   
   return (p1 - p2).len();
}

//-------------------------------------------------------------------------------------

void NavigationGraph::chokePoints(const Point3F& srcPoint, Vector<Point3F>& points, 
                        F32 minHideD, F32 stopSearchD)
{
   if (GraphNode * srcNode = closestNode(srcPoint))
      getLOSSearcher()->findChokePoints(srcNode, points, minHideD, stopSearchD);
   else
      warning("getChokePoints() failed to find a closest node");
}

// Static function:
S32 NavigationGraph::getChokePoints(const Point3F& srcPoint, Vector<Point3F>& points, 
                        F32 minHideDist, F32 stopSearchDist)
{
   points.clear();
   
   if (gotOneWeCanUse())
      if (gNavGraph->validLOSXref()) 
         gNavGraph->chokePoints(srcPoint, points, minHideDist, stopSearchDist);
      else
         warning("getChokePoints() called without valid LOS XRef table");
   else 
      warning("getChokePoints() called without (usable) graph in place");
      
   return points.size();
}

//-------------------------------------------------------------------------------------

Point3F NavigationGraph::hideOnSlope(const Point3F& from, const Point3F& avoid, F32 rad, F32 deg)
{
   const char * warnMsg = NULL;

   if (gotOneWeCanUse()) 
   {
      if (gNavGraph->validLOSXref()) 
      {
         GraphSearchLOS * searcher = gNavGraph->getLOSSearcher();
         return searcher->hidingPlace(from, avoid, rad, mDegToRad(90-deg), true);
      }
      else
         warnMsg = "hideOnSlope() called without valid LOS XRef table";
   }
   else
      warnMsg = "hideOnSlope() called without (usable) graph in place";
      
   if (warnMsg && _GRAPH_WARNINGS_)
      warning(warnMsg);
      
   return from;
}

//-------------------------------------------------------------------------------------

Point3F NavigationGraph::hideOnDistance(const Point3F& from, const Point3F& avoid, F32 rad, F32 hideLen)
{
   const char * warnMsg = NULL;

   if (gotOneWeCanUse()) 
   {
      if (gNavGraph->validLOSXref()) 
      {
         GraphSearchLOS * searcher = gNavGraph->getLOSSearcher();
         return searcher->hidingPlace(from, avoid, rad, hideLen, false);
      }
      else
         warnMsg = "hideOnDistance() called without valid LOS XRef table";
   }
   else
      warnMsg = "hideOnDistance() called without (usable) graph in place";
      
   if (warnMsg && _GRAPH_WARNINGS_)
      warning(warnMsg);
      
   return from;
}

//-------------------------------------------------------------------------------------

Point3F NavigationGraph::findLOSLocation(const Point3F& from, const Point3F& wantToSee, 
               F32 minDist, const SphereF& getCloseTo, F32 capDist)
{
   const char * warnMsg = NULL;

   if (gotOneWeCanUse()) 
   {
      if (gNavGraph->validLOSXref()) 
      {
         GraphSearchLOS * searcher = gNavGraph->getLOSSearcher();
         return searcher->findLOSLoc(from, wantToSee, minDist, getCloseTo, capDist);
      }
      else
         warnMsg = "findLOSLocation() called without valid LOS XRef table";
   }
   else
      warnMsg = "findLOSLocation() called without (usable) graph in place";
      
   if (warnMsg && _GRAPH_WARNINGS_)
      warning(warnMsg);
      
   return from;
}

