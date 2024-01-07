//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------

#ifdef _GRAPH_DEBUG_
static void debugDrawBorderSeg(const GraphBoundary& border);
static void debugDrawParaNormal(const Point3F& midpoint, VectorF paraNormal);
static void debugDrawSeekVector(Point3F curLoc, Point3F seekLoc);
#else
#define  debugDrawBorderSeg(border)
#define  debugDrawParaNormal(midpoint,paranormal)
#define  debugDrawSeekVector(curloc,seekloc)
#endif

//-------------------------------------------------------------------------------------

// define  SegCheck3D  (2.0f)
#define  SegCheck3D  (4.0f)
#define  SegCheck2D  (0.5f)

#define     RoomNeeded              (0.5f)
#define     InsideThreshSquared     (RoomNeeded * RoomNeeded)

// User has to call this this first- 
bool NavigationGraph::useVolumeTraverse(const GraphNode* from, const GraphEdge* to)
{
   if (mHaveVolumes)
      if (from->canHaveBorders())      // i.e (indoor | transient)
         if (from->volumeIndex() >= 0)
            if (to && !to->isJetting() && to->isBorder())
               return true;
   return false;
}

// 
// See if line from currLoc to seekLoc can pass through the border.  
// 
// Ok, I'm beginning to see the wisdom of pursuing a BSP-based approach to the 
// NAV graphs.  We're doing a lot of work here and still not getting nearly enough
// free-form movement as could/should be had.  
//
//==>  This should probably account for different bot sizes.  
// 
static bool canFitThrough(const Point3F& currLoc, const Point3F& seekLoc, 
                  const GraphBoundary& border)
{
   // We rely on fact that boundary normal is parallel to ground, and just do 2D math
   // here.  The source and destination locations should be on opposite sides.  These
   // dot products then also give us an interpolation value to find the crossing point.
   VectorF  seek2D = seekLoc, curr2D = currLoc;
   (seek2D -= border.seg[0]).z = 0;
   (curr2D -= border.seg[0]).z = 0;
   
   F32   seekDot = mDot(seek2D, border.normal);
   F32   currDot = mDot(curr2D, border.normal);
   
   if (seekDot > 0.03 && currDot < -0.03) 
   {
      F32      interp = -currDot / (seekDot - currDot);
      Point3F  crossPt = scaleBetween(currLoc, seekLoc, interp);
      
      // Want to make sure the crossing point is well inside.  We've done plenty of
      // math already, let's avoid square roots and just check 2 sufficient conditions:
      // Dist from endpoints is enough, vectors to endpoints point away from each other
      VectorF  from0 = crossPt, from1 = crossPt;
      (from0 -= border.seg[0]).z = 0;
      (from1 -= border.seg[1]).z = 0;
      
      if (from0.lenSquared() > InsideThreshSquared)
         if (from1.lenSquared() > InsideThreshSquared)
            if (mDot(from0, from1) < -0.9)
               return true;
   }
   
   return false;
}

//-------------------------------------------------------------------------------------

bool NavigationGraph::volumeTraverse(const GraphNode* from, const GraphEdge* to, 
                  NavigationPath::State & S)
{
   bool                    canAdvance = false;
   const GraphBoundary &   border = mBoundaries[to->mBorder];
   Point3F                 midpoint = (border.seg[0] + border.seg[1]) * 0.5f;
   BitSet32 &              visit = S.visit[S.curSeekNode-1];
   
   debugDrawBorderSeg(border);

   // See if the node advancer below skipped this fromNode.  
   if (visit.test(NavigationPath::State::Skipped))
   {
      // If so then we just want to get on the other side of it.  
      F32   dot = mDot(S.hereLoc - border.seg[0], border.normal);
      if (dot > 0.05)
         canAdvance = true;
   }
   else 
   {
      if (inNodeVolume(from, S.hereLoc)) 
      {
         visit.set(NavigationPath::State::Visited);
      
         // Get "line" to check proximity to (just use a really large segment)
         // When near, head across, else seek the proper point.  
         Point3F        paraNormal = (border.normal * 33.0);
         LineSegment    crossingLine(midpoint - paraNormal, midpoint + paraNormal);

         debugDrawParaNormal(midpoint, paraNormal);

         // This state was flipping on some small nodes near walls - should work to 
         // just only head out once in this case.  
         if (!visit.test(NavigationPath::State::Outbound))
         {
            if (crossingLine.botDistCheck(S.hereLoc, SegCheck3D, SegCheck2D))
               visit.set(NavigationPath::State::Outbound);
            else
               S.seekLoc = border.seekPt;
         }
         
         if (visit.test(NavigationPath::State::Outbound))
            S.seekLoc = midpoint + (border.normal * 2.0);
      }
      else 
      {
         // The source node has been visited, but we're in the next node and want to 
         // decide if we can begin tracking next segment (if that's what's next).  
         if (visit.test(NavigationPath::State::Visited))
         {
            LineSegment    borderSeg(border.seg[0], border.seg[1]);

            //==> THIS CHECK NEEDS TO LOOK AT SEGMENT WHEN LARGE (use projected pulled in loc on seg)
            // We could modifiy this pull-in a little based on player distance, and it would 
            // then traverse a little differently.  
            
            // This has a problem in a certain case- they might not be able to achieve getting
            // inside the volume for narrow ones.  Need to handle being here for a couple of 
            // frames, plus we should make the approach to the border be better - based
            // on where you want to go.  
            if (!borderSeg.botDistCheck(S.hereLoc, SegCheck3D, 0.2))
            {
               // When the next edge is either jetting or laid walk connection, then
               // we need to actually go to the node location before advancing.  
               if (S.nextEdgePrecise)
               {
                  S.seekLoc = lookupNode(to->mDest)->location();
                  if (within_2D(S.hereLoc, S.seekLoc, 0.3))
                     canAdvance = true;
               }
               else
               {
                  mRenderThese.clear();
                  canAdvance = true;
               }
            }
            else
            {
               S.seekLoc = (midpoint + (border.normal * 2.0));
            }
         }
         else 
         {
            Point3F  inVec = (border.seg[1] - border.seg[0]);
            F32      len = inVec.len();
         
            AssertFatal(len > 0.01, "There is a way-too-short border segment");

            inVec.normalize(len > 1.2 ? 0.4 : len / 3);     // get 'pulled in' segment
            
            LineSegment borderSeg(border.seg[0]+inVec, border.seg[1]-inVec);
         
            if (borderSeg.botDistCheck(S.hereLoc, SegCheck3D, SegCheck2D))
            {
               mRenderThese.clear();
               canAdvance = true;
            }
            else
            {
               S.seekLoc = from->location();
            }
         }
      }
   }
   
   debugDrawSeekVector(S.hereLoc, S.seekLoc);

   return canAdvance;
}

// This just tries to do further skips from the current location.  The above code
// performs advancement in the case of skipped nodes.  Like the outdoor advance, 
// we only attempt one at a time, but unlike it the advancement doesn't happen 
// right away.  
S32 NavigationGraph::checkIndoorSkip(NavigationPath::State & S)
{
   S32   startingFrom = (S.curSeekNode - 1);
   S32   numberSkipped = 0;
   
   if (!S.visit[startingFrom].test(NavigationPath::State::Skipped))
   {
      // See if we can skip ahead through outbound segments.  
      for (S32 from = startingFrom; from < S.edges.size() - 1; from++)
      {
         GraphEdge * fromOut = S.edges[from];
         GraphEdge * toOut = S.edges[from+1];
         
         if (fromOut->isBorder() && toOut->isBorder()) 
         {
            AssertFatal(!fromOut->isJetting() && !toOut->isJetting(), "Indoor skip");
            
            const GraphBoundary &   toBorder = mBoundaries[toOut->mBorder];
            const GraphBoundary &   fromBorder = mBoundaries[fromOut->mBorder];
            
            // See if segment to the TO outbound place passes Ok through FROM outbound
            if (canFitThrough(S.hereLoc, toBorder.seekPt, fromBorder)) 
            {
               // The first from has already been visited....
               if (from > startingFrom) 
               {
                  S.seekLoc = toBorder.seekPt;
                  S.visit[from].set(NavigationPath::State::Skipped);
                  numberSkipped++;
                  continue;
               }
            }
         }
         break;
      }
   }
   return numberSkipped;
}

//-------------------------------------------------------------------------------------

#ifdef _GRAPH_DEBUG_
static void debugDrawBorderSeg(const GraphBoundary& border)
{
   Point3F  bseg1(border.seg[0]);
   Point3F  bseg2(border.seg[1]);
   bseg1.z += 1.0f; 
   bseg2.z += 1.0f;
   LineSegment borderSeg(bseg1, bseg2);
   gNavGraph->pushRenderSeg(borderSeg);
}
static void debugDrawParaNormal(const Point3F& midpoint, VectorF paraNormal)
{
   paraNormal.normalize(4.0);
   Point3F  p1(midpoint - paraNormal);
   Point3F  p2(midpoint + paraNormal);
   p1.z += 1.4f; 
   p2.z += 1.4f;
   LineSegment paraNormalSeg(p1, p2);
   gNavGraph->pushRenderSeg(paraNormalSeg);
}
static void debugDrawSeekVector(Point3F curLoc, Point3F seekLoc)
{   
   curLoc.z += 1.8f;  
   seekLoc.z += 1.8f;
   LineSegment  seekVecSeg(curLoc, seekLoc);
   gNavGraph->pushRenderSeg(seekVecSeg);
}
#endif

