//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------
//          Interior Node Methods


NodeProximity InteriorNode::containment(const Point3F& loc) const
{
   if (gNavGraph->haveVolumes())
      return gNavGraph->getContainment(mIndex, loc);
   else
      return Parent::containment(loc);
}

void InteriorNode::init(const IndoorNodeInfo & g, S32 index, const Point3F& normal)
{
   mIndex = index;
   mLoc = g.pos;
   mNormal = normal;
   mFlags.set(Inventory, g.isInventory());
   mFlags.set(Algorithmic, g.isAlgorithmic());
   mFlags.set(BelowPortal, g.isBelowPortal());
   mFlags.set(PotentialSeed, g.isSeed());
}

GraphEdge& InteriorNode::pushOneWay(const GraphEdgeInfo::OneWay & edgeIn)
{
   GraphEdge   edgeOn;
   edgeOn.mDest = edgeIn.dest;
   if(edgeIn.isJetting())
      edgeOn.setJetting();
   mEdges.push_back(edgeOn);
   return mEdges.last();
}

InteriorNode::InteriorNode()
{
   mFlags.set(Indoor);
   mMinDim = 0.5;
   mArea = (mMinDim * mMinDim);
}

//-------------------------------------------------------------------------------------

// Must destruct the nodes.  
void IndoorNodeList::clear()
{
   for (iterator it = begin(); it != end(); it++)
      it->~InteriorNode();
   Parent::clear();
}

void IndoorNodeList::init(S32 size)
{
   clear();
   setSizeAndConstruct(*this, size);
}

// Must destruct elements.  
IndoorNodeList::~IndoorNodeList()
{
   clear();
}

//-------------------------------------------------------------------------------------

GraphBoundary::GraphBoundary(const GraphEdgeInfo& edgeInfo)
{
   seg[0] = edgeInfo.segPoints[0];
   seg[1] = edgeInfo.segPoints[1];
   
   normal.x = seg[0].y - seg[1].y;
   normal.y = seg[1].x - seg[0].x;
   normal.z = 0;
   F32   len = normal.len();
   AssertFatal(len > 0.001, "GraphBoundry has small border");
   normal *= (1.0f / len);
}

//==> Note - we may want to just have one boundary segment per edge - if so 
// we can still compute both and then merge them.  I think we may need both though
// since we will want to define a complete segment that they can cross over
// at some point.  

// Some computations needed to set it up to know how to navigate through there.  
void GraphBoundary::setItUp(S32 nodeIndex, const NodeInfoList& nodes, 
            const GraphVolumeList& volumes)
{
   Point3F  midpoint = scaleBetween(seg[0], seg[1], 0.5f);
   
   // Set normal to point in the direction (outbound) we need- 
   Point3F  nodeCenter = nodes[nodeIndex].pos;
   Point3F  outbound = (midpoint - nodeCenter);
   if (mDot(outbound, normal) < 0)
      normal *= -1.0;
   
   // Find minimum of the midpoint from all planes, skipping current one.  
   //==>  SKIP??
   //==>  This all needs to be better smoothed.  
   F32      D, minDist = 1e9;
   S32      numWalls = volumes.planeCount(nodeIndex) - 2;
   const PlaneF* planes = volumes.planeArray(nodeIndex);
   AssertFatal(numWalls > 2, "Graph node has no meaningful volume");
   while (numWalls--)
      if ((D = -planes[numWalls].distToPlane(midpoint)) > 0.01)
         if (D < minDist)
            minDist = D;
         
   distIn = getMin(minDist, 1.2f);
   
   // Now set up the point we need to go through.  
   seekPt = midpoint - (distIn * normal);
}

//-------------------------------------------------------------------------------------

bool NavigationGraph::initInteriorNodes(const EdgeInfoList & edges, 
         const NodeInfoList & nodes, S32 startIndex)
{
   // This allocation must happen before (in caller) for edge pool purposes- 
   mBoundaries.clear();
   
   mHaveVolumes = (mNodeVolumes.size() == nodes.size());

   // Build node list   
   Vector<Point3F>   utilityBuffer;
   for (S32 i = 0; i < nodes.size(); i++) {
      Point3F  normal(0,0,1);
      F32      minDim = 1.0;
      F32      area = 1.0;
      if (mHaveVolumes) {
         Point3F floorVec = mNodeVolumes.floorPlane(i);
         if (floorVec.z != 0)
            normal = -floorVec;
         else
            warning("Graph needs regeneration (missing floor plane)");
            
         minDim = mNodeVolumes.getMinExt(i, utilityBuffer);
      }
      mIndoorNodes[i].init(nodes[i], startIndex + i, normal);
      mIndoorNodes[i].setDims(minDim, area);
   }

   for (S32 j = 0; j < edges.size(); j++)
   {
      const GraphEdgeInfo&    E = edges[j];
   
      for (U32 k = 0; k < 2; k++) 
      {
         GraphEdgeInfo::OneWay   edgeOneWay = E.to[k^1];
         S32   dstIndex = edgeOneWay.dest;
         S32   srcIndex = E.to[k].dest;
         edgeOneWay.dest += startIndex;
         GraphEdge& edge = mIndoorNodes[srcIndex].pushOneWay(edgeOneWay);
      
         // set up the boundary information
         if (mHaveVolumes)
         {
            edge.mBorder = mBoundaries.size();
            GraphBoundary  boundary(E);
            boundary.setItUp(srcIndex, nodes, mNodeVolumes);
            mBoundaries.push_back(boundary);
            AssertFatal(mBoundaries.size() < (1 << 15), "Maximum Boundaries = 32K");
         }
      }
   }

   return nodes.size();
}

//-------------------------------------------------------------------------------------

// Given a list of interior node indices to get rid of, compact those nodes (and any
// edges left hanging) out of the lists (mEdgeInfoList & mNodeInfoList).  Also
// compacts out of the volume list if such is present.  
S32 NavigationGraph::compactIndoorData(const Vector<S32>& cullList, S32 numOutdoor)
{
   // On balance, it's more convenient to map into the indoor node list 'space' here...
   S32            i;
   Vector<S32>    axeList(cullList.size());
   for (i = 0; i < cullList.size(); i++)
      axeList.push_back(cullList[i] - numOutdoor);

   // Minimize fragmentation by pre-reserving.  
   NodeInfoList      culledNodes(mNodeInfoList.size());
   EdgeInfoList      culledEdges(mEdgeInfoList.size());
   BridgeDataList    culledBridges;
   GraphVolumeList   culledVolumes;
   culledVolumes.reserve(mNodeVolumes.size());
   culledBridges.reserve(mBridgeList.size());
   
   // ==> This looks wasteful, these need to be culled
   if (mHaveVolumes)   
      culledVolumes.mPlanes = mNodeVolumes.mPlanes;

   // Mark nodes-to-axe with -1, the rest at zero.  
   Vector<S32>    remap;
   for (i = 0, setSizeAndClear(remap,mNodeInfoList.size()); i < axeList.size(); i++)
   {
      S32   removedNode = axeList[i];
      AssertFatal(removedNode >= 0, "NavigationGraph::compactIndoorData()");
      remap[removedNode] = -1;
   }
      
   // Fetch nodes to keep into new list and build remap indices for remaining
   for (i = 0; i < mNodeInfoList.size(); i++)   if (remap[i] == 0) 
   {
      remap[i] = culledNodes.size();
      culledNodes.push_back(mNodeInfoList[i]);
      if (mHaveVolumes)
         culledVolumes.push_back(mNodeVolumes[i]);
   }

   // Copy down culled edge list and remap the "to" pointers.  
   for (i = 0; i < mEdgeInfoList.size(); i++) 
   {
      GraphEdgeInfo  edge = mEdgeInfoList[i];
      if ((edge.to[0].dest = remap[edge.to[0].dest]) >= 0) 
      {
         edge.to[1].dest = remap[edge.to[1].dest];
         culledEdges.push_back(edge);
      }
   }
   
   // Remap bridge indices, and remove any invalid.  Note that outdoor nodes don't get
   // culled, even if they have a stranded pocket.  
   for (i = 0; i < mBridgeList.size(); i++) 
   {
      GraphBridgeData   bridge = mBridgeList[i];
      bool              stillGood = true;
      
      for (S32 j = 0; stillGood && (j < 2); j++)
      {
         // Remap indoor nodes, or see if it's been culled- 
         S32   nodeInd = bridge.nodes[j];
         if (nodeInd >= numOutdoor)
         {
            S32   mappedNode = remap[nodeInd - numOutdoor];
            if (mappedNode >= 0)
               bridge.nodes[j] = (mappedNode + numOutdoor);
            else
               stillGood = false;
         }
      }
      
      if (stillGood)
         culledBridges.push_back(bridge);
   }
   
   // install the new lists:
   mEdgeInfoList = culledEdges;
   mNodeInfoList = culledNodes;
   mNodeVolumes = culledVolumes;
   mNodeVolumes.cullUnusedPlanes();
   mBridgeList = culledBridges;

   // whatever-    
   return culledNodes.size();
}

