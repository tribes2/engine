//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

#define  PlaneAdjErr          0.014f
#define  MapIndoorIndex(i)    ((i)-mNumOutdoor)

//-------------------------------------------------------------------------------------

// General query for volume info - package up everything user might want, including
// list of corners.  Bool parameter tells if ceiling corners should be fetched too.  
const GraphVolume& NavigationGraph::fetchVolume(const GraphNode* node, bool above)
{
   S32   Index = MapIndoorIndex(node->getIndex());
   mTempVolumeBuf.mFloor   = mNodeVolumes.floorPlane(Index);
   mTempVolumeBuf.mCeiling = mNodeVolumes.abovePlane(Index);
   mTempVolumeBuf.mPlanes  = mNodeVolumes.planeArray(Index);
   mTempVolumeBuf.mCount   = mNodeVolumes.planeCount(Index);
   mNodeVolumes.getCorners(Index, mTempVolumeBuf.mCorners, !above);
   return mTempVolumeBuf;
}

//-------------------------------------------------------------------------------------

static const NodeProximity    sFarProximity;

// Points inside the volume return a negative distance from planes, so we take the 
// maximum of these, and then "good" containment means that the maximum of these
// numbers is still pretty small.  The check with floor is either/or though.  We start
// with ceiling distance as the beginning metric.  
NodeProximity NavigationGraph::getContainment(S32 indoorIndex, const Point3F& point)
{
   NodeProximity  metric;
   
   PlaneF   floor = mNodeVolumes.floorPlane(indoorIndex = MapIndoorIndex(indoorIndex));

   // Below floor -> not a candidate, return low containment (far)
   metric.mHeight = floor.distToPlane(point);
   if (metric.mHeight > PlaneAdjErr) 
      return sFarProximity;

   F32   ceilingMetric = mNodeVolumes.abovePlane(indoorIndex).distToPlane(point);
   metric.mAboveC = ceilingMetric;
      
   if (ceilingMetric > 0.0) {
      // Tricky number here - probably don't want to make huge because it's sometimes
      // important that we're near the top of volumes - other times not.  Really
      // need more info about the volume...  This fixes problem in Beggar's Run.  
      ceilingMetric = getMax(ceilingMetric, 7.0f);
   }

   // Find the closest wall, use ceiling as start (if we're below, else larger #).
   metric.mLateral = ceilingMetric;
   const PlaneF * planes = mNodeVolumes.planeArray(indoorIndex);
   S32   numWalls = mNodeVolumes.planeCount(indoorIndex) - 2;
   
   while (--numWalls >= 0) 
   {
      F32   D = (planes++)->distToPlane(point);
      if (D > metric.mLateral)
         metric.mLateral = D;
   }

   return metric;
}

//-------------------------------------------------------------------------------------
// Find closest point and store result in soln if it exists.  

bool NavigationGraph::closestPointOnVol(GraphNode * node, const Point3F& point, Point3F& soln) const 
{
   S32   indoorIndex = MapIndoorIndex(node->getIndex());
   return mNodeVolumes.closestPoint(indoorIndex, point, soln);
}

bool GraphVolumeList::closestPoint(S32 ind, const Point3F& point, Point3F& soln) const 
{
   Vector<Point3F>   corners;
   bool     foundIt = false;
   PlaneF   floor = floorPlane(ind);
   
   if (intersectWalls(ind, floor, corners))
   {
      S32   numCorners = corners.size();
      F32   minDist = 1e13;
      corners.push_back(corners[0]);
      
      for (S32 i = 0; i < numCorners; i++)
      {
         LineSegment    segment(corners[i], corners[i+1]);
         F32   D = segment.distance(point);
         if (D < minDist)
         {
            foundIt = true;
            minDist = D;
            soln = segment.solution();
            soln.z = point.z;
         }
      }
   }
   return foundIt;
}

//-------------------------------------------------------------------------------------

PlaneF NavigationGraph::getFloorPlane(GraphNode * node)
{
   return mNodeVolumes.floorPlane(MapIndoorIndex(node->getIndex()));
}

//-------------------------------------------------------------------------------------
// Are we between floor and ceiling of this node?

bool NavigationGraph::verticallyInside(S32 indoorIndex, const Point3F& point)
{
   PlaneF   floor = mNodeVolumes.floorPlane(indoorIndex = MapIndoorIndex(indoorIndex));
   if (floor.distToPlane(point) <= PlaneAdjErr) 
      if (mNodeVolumes.abovePlane(indoorIndex).distToPlane(point) <= PlaneAdjErr)
         return true;
   return false;
}

F32 NavigationGraph::heightAboveFloor(S32 indoorIndex, const Point3F& point)
{
   PlaneF   floor = mNodeVolumes.floorPlane(MapIndoorIndex(indoorIndex));
   return -(floor.distToPlane(point) + PlaneAdjErr);
}

//-------------------------------------------------------------------------------------

bool GraphVolumeList::intersectWalls(S32 i, const PlaneF& with, Vector<Point3F>& list) const 
{
   bool  noneWereParallel = true;
   const PlaneF* planes = planeArray(i);
   S32   N = planeCount(i) - 2;
   
   for (S32 w = 0; w < N; w++) {
      Point3F  point;
      if (intersectPlanes(with, planes[w], planes[(w+1)%N], &point))
         list.push_back(point);
      else {
         // Parallel planes occasionally happen - since intersection points are 
         // not vital to system (just used to find bounding box) - we just 
         // continue, but return false;
         // 
         noneWereParallel = false;
      }
   }
   return noneWereParallel;
}
      
//-------------------------------------------------------------------------------------
// Get intersections of the volume.  

S32 GraphVolumeList::getCorners(S32 ind, Vector<Point3F>& points, bool justFloor) const
{
   points.clear();
   PlaneF   floor = floorPlane(ind);
   intersectWalls(ind, floor, points);
   if (!justFloor) 
   {
      PlaneF   ceiling = abovePlane(ind);
      intersectWalls(ind, ceiling, points);
   }
   return points.size();
}

//-------------------------------------------------------------------------------------
// Get minimum extent of the volume.  Want minimum of max distance taken from
//    each wall plane.  
// The point buffer is just passed in to avoid vector reallocs since this method is 
//    called a lot at startup, and Point3F vecs are showing up on memory profiles.  

F32 GraphVolumeList::getMinExt(S32 ind, Vector<Point3F>& ptBuffer)
{
   ptBuffer.clear();
   if (intersectWalls(ind, floorPlane(ind), ptBuffer)) 
   {
      const PlaneF* planes = planeArray(ind);
      S32   N = planeCount(ind) - 2;
      F32  minD = 1e9; 
      for (S32 w = 0; w < N; w++) 
      {
         F32   maxD = -10, d;
         for (S32 p = 0; p < ptBuffer.size(); p++) 
            if ((d = (-planes[w].distToPlane(ptBuffer[p]))) > maxD)
               maxD = d;
         if (maxD < minD)
            minD = maxD;
      }
      return minD;
   }
   else {
      return 0.5;
   }
}

//-------------------------------------------------------------------------------------

// Push a box of given height and width at P.
void GraphVolumeList::addVolume(Point3F pos, F32 boxw, F32 boxh)
{
   GraphVolInfo   entry;
   entry.mPlaneCount = 6;
   entry.mPlaneIndex = mPlanes.size();

   static const VectorF walls[4]= 
         {VectorF(-1,0,0), VectorF(0,1,0), VectorF(1,0,0), VectorF(0,-1,0)};

   // Add the walls- 
   for (S32 i = 0; i < 4; i++)
   {
      Point3F  point = pos + walls[i] * boxw;
      PlaneF   wall(point, walls[i]);
      mPlanes.push_back(wall);
   }            
   PlaneF   floor(pos, VectorF(0, 0, -1));
   mPlanes.push_back(floor);
 
   pos.z += boxh;  
   PlaneF   ceiling(pos, VectorF(0, 0, 1));
   mPlanes.push_back(ceiling);
   
   push_back(entry);
}

//-------------------------------------------------------------------------------------

// When islands are culled, lists are compacted, and this service is needed.  It's 
// called after the GraphVolInfo list is culled down.  
void GraphVolumeList::cullUnusedPlanes()
{
   // Copy down the planes and remap the indices into the plane list.  
   Vector<PlaneF> keepers(mPlanes.size());
   for (iterator it = begin(); it != end(); it++)
   {
      S32   newIndex = keepers.size();
      for (S32 p = 0; p < it->mPlaneCount; p++)
         keepers.push_back(mPlanes[it->mPlaneIndex + p]);
      it->mPlaneIndex = newIndex;
   }
   
   // Replace the plane list with the keepers- 
   mPlanes = keepers;
}

//-------------------------------------------------------------------------------------

// Ideally always go through this for remapping to the indoor list:
S32 NavigationGraph::indoorIndex(const GraphNode* node)
{
   return MapIndoorIndex(node->getIndex());
}

//-------------------------------------------------------------------------------------

bool NavigationGraph::inNodeVolume(const GraphNode* node, const Point3F& point)
{
   S32   nodeIndex = MapIndoorIndex(node->volumeIndex());
   AssertFatal(validArrayIndex(nodeIndex, mNodeVolumes.size()), "nonmatching volume");
   S32   numWalls = mNodeVolumes.planeCount(nodeIndex) - 2;
   const PlaneF* planes = mNodeVolumes.planeArray(nodeIndex);
   
   while (numWalls--)
      // if (planes[numWalls].whichSide(point) != PlaneF::Back)
      if (planes[numWalls].distToPlane(point) > PlaneAdjErr)
         return false;
         
   return true;
}

//-------------------------------------------------------------------------------------

void NavigationGraph::drawNodeInfo(GraphNode * node)
{
   if (mHaveVolumes) {
      clearRenderBoxes();
      Vector<Point3F> corners;
      mNodeVolumes.getCorners(indoorIndex(node), corners, false);
      for (S32 c = 0; c < corners.size(); c++)
         pushRenderBox(corners[c], 0);
   }
}

// This is a debugging method- see how well we match nodes and such.  
const char * NavigationGraph::drawNodeInfo(const Point3F& loc)
{
   const char * info = NULL;
   
   if (GraphNode * node = closestNode(loc)) {
      if (node->indoor()) {
         // Put boxes around the node
         drawNodeInfo(node);
         info = "Found indoor node";
      }
      else
         info = "Found outdoor node";
   }
   else 
      info = "Couldn't find a closest node to this location";

   return info;
}


//-------------------------------------------------------------------------------------

static const U32 sMask = InteriorObjectType|StaticShapeObjectType|StaticObjectType;

static bool intersectsAnything(const Box3F& bounds, ClippedPolyList& polyList)
{
   if (gServerContainer.buildPolyList(bounds, sMask, &polyList)) 
      return (polyList.mPolyList.size() != 0);
   return false;
}

//-------------------------------------------------------------------------------------

#define  JustUnshrinkIt       (NodeVolumeShrink + 0.002)
#define  ExtrudeALittle       (NodeVolumeShrink + 0.22)

// 
// We want to slightly oversize node volumes which can.  This accomplishes two things:
//    1. The containment metrics are better.  Example is when you're staning on a ledge
//       but the node from below (which is along a wall) matches you better.
//    2. It should help with the indoor lookahead smoothing.  It makes sure we are 
//       always inside nodes.
//
// This ALSO moves the volumes out by the amount that Greg's system pulls it in.  
// (We do this here because of the extra checks which want them still shrunk).
// 
void GraphVolumeList::nudgeVolumesOut()
{
   Point3F           outSize(1.0, 1.0, 1.0);
   Vector<F32>       nudgeAmounts;
   Vector<Point3F>   corners;
   
   for (S32 i = 0; i < size(); i++)
   {
      // Calculate a bounding box of the node volume- 
      getCorners(i, corners, false);
      Box3F bounds(Point3F(1e9, 1e9, 1e9), Point3F(-1e9, -1e9, -1e9), true);
      for (Vector<Point3F>::iterator c = corners.begin(); c != corners.end(); c++)
         bounds.min.setMin(*c), bounds.max.setMax(*c);
         
      // Perform our extrusion checks on all the walls (_separately_, hence bool array)
      S32   numPlanes = planeCount(i);
      PlaneF* planeList = getPlaneList(i);
      nudgeAmounts.setSize(numPlanes);
      bounds.min -= outSize;
      bounds.max += outSize;
      for (S32 n = 0; n < numPlanes; n++) 
      {  
         if (n == numPlanes - 2) {
            // Floor- don't extrude this one
            nudgeAmounts[n] = JustUnshrinkIt;
         }
         else {
            // Build poly list extruded in one direction- 
            ClippedPolyList   polyList;
            polyList.mPlaneList.clear();
            polyList.mNormal.set(0, 0, 0);
            for (S32 p = 0; p < numPlanes; p++) {
               PlaneF   plane = planeList[p];
               if (p == n)
                  plane.d -= ExtrudeALittle;
               polyList.mPlaneList.push_back(plane);
            }
         
            // See if we nudged without hitting anything. 
            if (intersectsAnything(bounds, polyList))
               nudgeAmounts[n] = JustUnshrinkIt;
            else
               nudgeAmounts[n] = ExtrudeALittle;
         }
      }

      // Now move the planes
      for (S32 j = 0; j < numPlanes; j++)
         planeList[j].d -= nudgeAmounts[j];
   }
}
