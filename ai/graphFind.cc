//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

U32   gCallsToClosestNode = 0;

//-------------------------------------------------------------------------------------

bool NavigationGraph::possibleToJet(const Point3F& from, const Point3F& to, U32)
{
   // To be refined- 
   return (from - to).len() < 60;
}

//-------------------------------------------------------------------------------------
// This routine is used by the seed scattering approach - generate points that are 
// inside nearby volumes, reflecting across one of the walls.  

S32 NavigationGraph::crossNearbyVolumes(const Point3F& from, Vector<Point3F>& points)
{
   Point3F  size(20, 20, 20);
   Box3F    checkBox(from, from);
   checkBox.min -= size;
   checkBox.max += size;
   GraphNodeList  nodeList;
   
   points.clear();
   if (S32 N = getNodesInBox(checkBox, nodeList, true))
   {
      // Find ones we're outside of a little ways - i.e. positive containment, but not
      // too low a number.  The caller will do LOS checks...   Also, we verify that we're
      // below the cieling as well.  
      for (S32 i = 0; i < N; i++)
      {
         GraphNode      *  node = nodeList[i];
         NodeProximity     prox = node->containment(from);

         // Make sure we're somewhat near.  Note Z check needs to come last here...  
         if ((F32(prox) > 0.1) && (F32(prox) < 6.0) && prox.insideZ())
         {
            // It's a candidate.  Now find closest point on volume....
            Point3F  soln;
            if (closestPointOnVol(node, from, soln))
            {
               // ...and reflect across point..
               VectorF  inVec(soln.x - from.x, soln.y - from.y, 0.0);
               inVec.normalize(0.8);
               soln += inVec;
               soln.z = solveForZ(getFloorPlane(node), soln) + 0.2;
               
               // See if that point is reasonably well inside the volume... 
               if (node->containment(soln) < -0.3)
                  points.push_back(soln);
            }
         }
      }
   }
   return points.size();
}

//-------------------------------------------------------------------------------------

S32 NavigationGraph::getNodesInBox(Box3F worldBox, GraphNodeList& listOut, bool justIndoor)
{
   // Box is assumed to be square in XY, get "radius"  
   Point3F  center;
   worldBox.getCenter(&center);

   listOut.clear();
   
   if (!justIndoor && haveTerrain())
   {
      // Fetch terrain nodes- 
      F32         rad = worldBox.len_x() * 0.5;
      S32         gridRadius = S32(rad / gNavGlobs.mSquareWidth) + 1;
      GridArea    gridArea = getGridRectangle(center, gridRadius);
      getNodesInArea(listOut, gridArea);
   }
   
   // Fetch from the indoor node BSP (this one doesn't clear the list)
   Point3F  radExt(MaxGraphNodeVolRad, MaxGraphNodeVolRad, MaxGraphNodeVolRad);
   worldBox.min -= radExt;
   worldBox.max += radExt;
   mIndoorTree.getIntersecting(listOut, worldBox);
   
   return listOut.size();
}

bool NavigationGraph::haveMuzzleLOS(S32 nodeInd1, S32 nodeInd2)
{
   if (nodeInd1 != nodeInd2 && mValidLOSTable)
      return mLOSTable->muzzleLOS(nodeInd1, nodeInd2);
   else
      return true;
}

//-------------------------------------------------------------------------------------

const GraphNodeList& NavigationGraph::getVisibleNodes(GraphNode * from, const Point3F& loc, F32 rad)
{
   mUtilityNodeList1.clear();
   mUtilityNodeList2.clear();
   
   if (from) 
   {
      SphereF     sphere(loc, rad);
   
      mUtilityNodeList1.clear();
      
      if (haveTerrain())
      {
         // Fetch terrain.  
         S32         gridRadius = S32(rad / gNavGlobs.mSquareWidth) + 1;
         GridArea    gridArea = getGridRectangle(loc, gridRadius);
         getNodesInArea(mUtilityNodeList1, gridArea);
      }
      
      // Fetch from the indoor node BSP (this one doesn't clear the list)
      Box3F    checkBox(loc, loc, true);
      rad += MaxGraphNodeVolRad;
      Point3F  radExt(rad, rad, rad);
      checkBox.min -= radExt;
      checkBox.max += radExt;
      mIndoorTree.getIntersecting(mUtilityNodeList1, checkBox);
      
      S32   fromInd = from->getIndex();
      
      for (S32 i = 0; i < mUtilityNodeList1.size(); i++)
         if (haveMuzzleLOS(fromInd, mUtilityNodeList1[i]->getIndex()))
            mUtilityNodeList2.push_back(mUtilityNodeList1[i]);
   }
   return mUtilityNodeList2;
}

const GraphNodeList& NavigationGraph::getVisibleNodes(const Point3F& loc, F32 rad)
{
   FindGraphNode  finder(loc);
   return getVisibleNodes(finder.closest(), loc, rad);
}

//-------------------------------------------------------------------------------------

#define  CrossingSegAdjustUp  0.3

// Figure out what constitutes a crossing path.  
S32 NavigationGraph::crossingSegs(const GraphNode * node, const GraphEdge * edge, 
         LineSegment * const segBuffer)
{
   LineSegment *  segs = segBuffer;
   Point3F  nodeLoc = node->location();
   Point3F  destLoc = lookupNode(edge->mDest)->location();
   
   nodeLoc.z += CrossingSegAdjustUp;
   destLoc.z += CrossingSegAdjustUp;
   
   if (edge->isBorder()) {
      Point3F  crossingPt = getBoundary(edge->mBorder).midpoint();
      crossingPt.z += CrossingSegAdjustUp;
      (segs++)->set(nodeLoc, crossingPt);
      (segs++)->set(crossingPt, destLoc);
   }
   else {
      // Do non-jetting like jetting as well - otherwise we miss collisions.  
   // if (edge->isJetting()) {
      Point3F  arcCorner;
      if (nodeLoc.z > destLoc.z)
         (arcCorner = destLoc).z = nodeLoc.z;
      else
         (arcCorner = nodeLoc).z = destLoc.z;
      (segs++)->set(nodeLoc, arcCorner);
      (segs++)->set(arcCorner, destLoc);
   }
   
   return (segs - segBuffer);
}

// Find all edges that are blocked by the given object. This is used by the force field
// monitor code, called at mission start to find affected edges.  
const GraphEdgePtrs& NavigationGraph::getBlockedEdges(GameBase* object, U32 typeMask)
{
   Box3F          objBox = object->getWorldBox();
   GraphEdge      edgeBuffer[MaxOnDemandEdges];
   LineSegment    segments[4];
   RayInfo        collision;
   GraphNodeList  consider;
   
   mVisibleEdges.clear();
   if (S32  numNodes = getNodesInBox(objBox, consider)) {
      for (S32 i = 0; i < numNodes; i++) {
         GraphNode * node = consider[i];
         GraphEdgeArray edges = node->getEdges(edgeBuffer);
         while (GraphEdge * edge = edges++)
            for (S32 j = crossingSegs(node, edge, segments) - 1; j >= 0; j--) {
               Point3F  A = segments[j].getEnd(0);
               Point3F  B = segments[j].getEnd(1);
               // Quick crude test- 
               if (objBox.collideLine(A, B))       
                  // Now do LOS, must hit object in question.  We must be careful
                  //    that force fields don't overlap in navigable area.  
                  if (gServerContainer.castRay(A, B, typeMask, &collision))
                     if (collision.object == object) {
                        mVisibleEdges.push_back(edge);
                        break;
                     }
            }
      }
   }
   
   // mVisibleEdges is a generic utility buffer for several queries, user copies off
   return mVisibleEdges;
}
      
//-------------------------------------------------------------------------------------

// For an outdoor node location, fetch the roaming radius off the terrain graph
// information.  Return 0.0 if nothing found.  
F32 NavigationGraph::getRoamRadius(const Point3F &loc)
{
   SphereF  sphere;
   Point3F  loc2D(loc.x, loc.y, 0.0f);
   
   if (mTerrainInfo.inGraphArea(loc2D))
      if (mTerrainInfo.locToIndexAndSphere(sphere, loc2D) >= 0)
         return sphere.radius;
         
   return 0.0f;
}

//-------------------------------------------------------------------------------------

// Patch function to translate into block space.  10/27/00- added optional normal
bool NavigationGraph::terrainHeight(Point3F pos, F32 * height, Point3F * normal)
{
   pos -= mTerrainInfo.originWorld;
   if (mTerrainBlock) {
      Point2F  point2F(pos.x, pos.y);
      if (mTerrainBlock->getHeight(point2F, height)) {
         if (normal != NULL)
            mTerrainBlock->getNormal(point2F, normal, true);
         return true;
      }
   }
   return false;
}

// Look for terrain node here using grid lookup. 
GraphNode * NavigationGraph::findTerrainNode(const Point3F & atLocation)
{
   Point3F     loc;
   bool        inArea;

   if (haveTerrain()) {
      // Bots can enter outside of area- we don't do height check below for such locs.
      if (mTerrainInfo.inGraphArea(atLocation)) {
         inArea = true;
         loc = atLocation;
      }
      else {
         inArea = false;
         loc = mTerrainInfo.whereToInbound(atLocation);
      }
   
      GraphNode * node = NULL;
      S32         gridIndex = mTerrainInfo.locToIndex(loc);
   
      if (gridIndex >= 0) {
         if (GraphNode * node = mNodeGrid[gridIndex]) {
            if (inArea) {
               F32   height;
               if (terrainHeight(loc, &height))
               {
                  if (height < (loc.z + 0.04))
                  {
                     // Point3F  terrLoc = node->location();
                     // F32   nodeZ = terrLoc.z;
                     F32   terrHt = node->terrHeight();
                     if (loc.z < (height + terrHt))
                        return node;
                  }
               }
            }
            else {
               return node;
            }
         }
      }
   }
      
   return NULL;
}

// If the findTerrainNode() fails, we see if we're on terrain with a 
// node that is within one grid location.  
GraphNode * NavigationGraph::nearbyTerrainNode(const Point3F& loc)
{
   GraphNode * node = NULL;
   
   if (haveTerrain())
   {   
      F32   H;
      S32   gridIndex = mTerrainInfo.locToIndex(loc);
      
      if( (gridIndex >= 0) && terrainHeight(loc, &H) && (H < loc.z + 0.04) )
      {
         // look for an immediately neighboring node - find the nearest one
         S32      *offs = mTerrainInfo.indOffs, i, n;
         F32      bestDist = 1e9, dSq;
         for (i = 0; i < 8; i++)
            if (validArrayIndex(n = gridIndex + offs[i], mTerrainInfo.nodeCount))
               if (GraphNode* N = mNodeGrid[n])
                  if ( (dSq = (loc - N->location()).lenSquared()) < bestDist )
                     bestDist = dSq, node = N;
      }
   }
   
   return node;
}

GraphNode * NavigationGraph::closestNode(const Point3F& loc, F32 * containment)
{
   GraphNode   * terrNode = findTerrainNode(loc);
   
   gCallsToClosestNode++;
   
   if (terrNode)
      return terrNode;
   else
   {
      terrNode = nearbyTerrainNode(loc);

      Box3F    box(loc, loc, true);
      Point3F  boundUp(0, 0, 20);
      Point3F  boundDown(0, 0, 80);
      Point3F  boundOut(35, 35, 0);
      
      box.max += boundUp;
      box.max += boundOut;
      box.min -= boundDown;
      box.min -= boundOut;
      
      GraphNodeList  indoor;
      
      // We really need to use two trees - one for big nodes, and one for smaller.  
      // This will keep the searches much more intelligent- oversize only a little
      // for the tree with small nodes - and so we'll not get so many.  Oversize
      // a lot for the big nodes - but there aren't nearly as many of them anyway.  
      mIndoorTree.getIntersecting(indoor, box);
      
      // Find the node with the best containment metric
      GraphNode * bestNode = NULL;
      F32         bestMetric = 1e13;
      F32         metric;
      for (GraphNodeList::iterator i = indoor.begin(); i != indoor.end(); i++) {
         metric = F32((*i)->containment(loc));
         if (metric < bestMetric) {
            bestMetric = metric;
            bestNode = *i;
         }
      }
            
      if (! bestNode)
         return terrNode;
      else {
         if (terrNode) {
            //==> Move this to a terrain containment function -
            F32 terrMetric = (terrNode->location() - loc).len() - terrNode->radius();
            if (terrMetric < bestMetric)
               return terrNode;
         }
         if (containment)
            *containment = bestMetric;
         return bestNode;
      }
   }
}

//-------------------------------------------------------------------------------------
// Structure which remembers results of searches, and which are also kept in a hash 
// table as well.  Shows up in profiles now that we use canReachLoc() a lot in the 
// objective weighting.  

FindGraphNode::FindGraphNode()
{
   init();
}

// Manage loc-to-node searches since many are the same.  
FindGraphNode::FindGraphNode(const Point3F& pt, GraphNode* hint)
{
   init();
   setPoint(pt, hint);
}

void FindGraphNode::init()
{
   mClosest = NULL;
   // Should be safe value that no-one will use:
   mPoint.set(-3.14159e14, 3.1e13, -2.22e22);
}

U32 FindGraphNode::calcHash(const Point3F& point)
{
   register const U32 * hashNums = (const U32 *)(&point);
   U32   val0 = (hashNums[0] >> 7) ^ (hashNums[0] << 5);
   U32   val1 = (hashNums[1] + 77773) & 0xFFFFFFF;
   U32   val2 = (hashNums[2] * 37) & 0xFFFFFFF;
   return (val0 + val1 + val2) % HashTableSize;
}

void FindGraphNode::setPoint(const Point3F& point, GraphNode * hint/*=0*/)
{
   // If point has not changed- we're done.
   if (point == mPoint)
      return;

   // Hash point into our cache (which NavigationGraph holds for us)-    
   mPoint = point;
   U32   hash = calcHash(mPoint);
   FindGraphNode  & cacheEntry = gNavGraph->mFoundNodes[hash];
   
   // See if hash table contains it. 
   if (cacheEntry.mPoint == point)
   {
      mClosest = cacheEntry.mClosest;
   }
   else 
   {
      // If there's a hint - see if we're inside it - that is then the closest.  Don't
      // put these in the hash table though since the hints usually refer to locations
      // that are moving, and we don't want them walking over our cache.  
      if (hint && hint->containment(point) < 0) 
      {
         mClosest = hint;
      }
      else 
      {
         // Otherwise do the search- 
         mClosest = gNavGraph->closestNode(point);
         
         // Put into cache.  We do this even for NULL results.  
         cacheEntry = * this;
      }
   }
}

//-------------------------------------------------------------------------------------
//                Grid <-> World  Translation Methods

GridArea NavigationGraph::getGridRectangle(const Point3F& atPos, S32 gridRadius)
{
   GridArea atRect;
   Point2I  atGridLoc;
   
   worldToGrid(atPos, atGridLoc);
   atRect.point.x = atGridLoc.x - gridRadius;
   atRect.point.y = atGridLoc.y - gridRadius;
   atRect.extent.x = atRect.extent.y = (gridRadius << 1);
   return atRect;
}

void NavigationGraph::worldToGrid(const Point3F &wPos, Point2I &gPos)
{
   TerrainBlock *terrain = GroundPlan::getTerrainObj();
   Point3F origin;
   terrain->getTransform().getColumn(3, &origin);

   float x = (wPos.x - origin.x) / (float)terrain->getSquareSize();
   float y = (wPos.y - origin.y) / (float)terrain->getSquareSize();
   
   gPos.x = (S32)mFloor(x);
   gPos.y = (S32)mFloor(y);
}

Point3F NavigationGraph::gridToWorld(const Point2I& gPos)
{
   Point3F  retVal;
   if (!mTerrainInfo.posToLoc(retVal, gPos))
      retVal.set(-1, -1, -1);
   return retVal;
}

GridArea NavigationGraph::getWorldRect()
{
   return GridArea(mTerrainInfo.originGrid, mTerrainInfo.gridDimensions);
}

//-------------------------------------------------------------------------------------
//                Visitor to find terrain nodes in rectangular area.

class VisitNodeGrid : public GridVisitor
{
  protected:
   const GridArea&         mWorld;        // invariant is that the mNodeGrid
   const GraphNodeList&    mGrid;         //    corresponds to mWorld area.  
         GraphNodeList&    mListOut;
         
   GraphNode * getNodeAt(const GridArea& R);
   
  public:
   VisitNodeGrid( const GridArea& toVisit, const GridArea& worldArea, 
                  const GraphNodeList& gridList, GraphNodeList& listOut ) 
      :  GridVisitor(toVisit), mWorld(worldArea), 
         mGrid(gridList), mListOut(listOut)       {  }
   
   bool beforeDivide(const GridArea& R, S32 level);
   bool atLevelZero(const GridArea& R);
};

GraphNode * VisitNodeGrid::getNodeAt(const GridArea& R)
{
   S32   index = mWorld.getIndex(R.point);
   AssertFatal( validArrayIndex(index, mGrid.size()), "VisitNodeGrid- bad index" );
   return mGrid[index];
}

bool VisitNodeGrid::beforeDivide(const GridArea& R, S32 level)
{
   if (GraphNode * node = getNodeAt(R)) {
      if(node->getLevel() == level) {
         mListOut.push_back(node);
         return false;
      }
   }
   return true; 
}

bool VisitNodeGrid::atLevelZero(const GridArea& R)
{
   if (GraphNode * node = getNodeAt(R))
      mListOut.push_back( node );
   return true;   //N/A
}

// Query list for list of nodes within a certain area.  
S32 NavigationGraph::getNodesInArea(GraphNodeList& listOut, GridArea toVisit)
{
   listOut.clear();
   if (haveTerrain())
   {
      GridArea    worldRect = getWorldRect();
      if (mNodeGrid.size() == (worldRect.extent.x * worldRect.extent.y)) 
      {
         if (toVisit.intersect(worldRect)) 
         {
            VisitNodeGrid  visitor(toVisit, worldRect, mNodeGrid, listOut);
            visitor.traverse();
         }
      }
   }
   return listOut.size();
}

void NavigationGraph::getOutdoorList(GraphNodeList& to)
{
   to.setSize(mNumOutdoor);
   dMemcpy(to.address(), mNodeList.address(), mNumOutdoor * sizeof(GraphNode*));
}
