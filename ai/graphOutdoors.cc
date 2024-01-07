//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------
//    Visitor for making run-time node list.  

class UnrollOutdoorList : public GridVisitor
{
  protected:
   const GraphNodeList&    mGrid;
   GraphNodeList&          mListOut;
         
   S32      getIndex(const GridArea& area);
   
  public:
   UnrollOutdoorList(const GridArea& world, const GraphNodeList& grid, GraphNodeList& list)
         :  GridVisitor(world), 
            mGrid(grid), 
            mListOut(list)      
            {  
            }
            
   bool     beforeDivide(const GridArea& R, S32 level);
   bool     atLevelZero(const GridArea& R);
};

S32 UnrollOutdoorList::getIndex(const GridArea& R)
{
   S32   index = mArea.getIndex( R.point );
   AssertFatal( validArrayIndex(index, mGrid.size()), "Node unroll bad index" );
   return index;
}

bool UnrollOutdoorList::beforeDivide(const GridArea& R, S32 level)
{
   S32   index = getIndex(R);
   if (GraphNode * node = mGrid[index])
   {
      if (node->getLevel() == level)
      {
         mListOut.push_back(node);
         return false;                             // stop the sub-divide
      }
   }
   return true;                                    // recurse further
}

bool UnrollOutdoorList::atLevelZero(const GridArea& R)
{
   if (GraphNode * node = mGrid[getIndex(R)])
      mListOut.push_back(node);
   return true;      // (N/A)
}

//-------------------------------------------------------------------------------------

// Make the grid of node pointers in the given area.  Makes the grid of 
//    empty nodes, and fills in the list, and sets indices.  
S32 NavigationGraph::setupOutdoorNodes(const GridArea& area_in, const Consolidated& cons, 
         GraphNodeList& grid_out, GraphNodeList& list_out)
{
   S32   i;
   
   mOutdoorNodes = new OutdoorNode [cons.size()];

   // Set up grid with pointers to outdoor nodes.  Consolidated areas have the whole
   // square within the grid set to point at them.  
   setSizeAndClear(grid_out, area_in.len_x() * area_in.len_y());
   for (i = 0; i < cons.size(); i++) 
   {
      const OutdoorNodeInfo & nodeInfo = cons[i];
      OutdoorNode *  outdoorNode = (mOutdoorNodes + i);
      
      Point2I gridPoint = nodeInfo.getPoint();
      mTerrainInfo.posToLoc(outdoorNode->mLoc, gridPoint);
      S32 level = nodeInfo.getLevel();
      outdoorNode->mLevel = level;
      
      S32   ind = mTerrainInfo.posToIndex(gridPoint);
      AssertFatal(ind >= 0, "setupOutdoorNodes: bad pos");
      if (mTerrainInfo.shadowed(ind)) {
         outdoorNode->mHeight = mTerrainInfo.shadowHeight(ind);
         outdoorNode->set(GraphNode::Shadowed);
      }
      else {
         outdoorNode->mHeight = 1e17;
         if (mTerrainInfo.submerged(ind))
            outdoorNode->set(GraphNode::Submerged);
      }

      //==> Make this get the average normal. 
      S32      gridShift = gNavGlobs.mSquareShift;
      Point2F  terrGridLoc(gridPoint.x << gridShift, gridPoint.y << gridShift);
      if (!mTerrainBlock->getNormal(terrGridLoc, &outdoorNode->mNormal))
         outdoorNode->mNormal.set(0,0,1);
      
      AssertFatal(level > -2, "Need level > -2 for outdoor nodes");
         
      if (level < 0) 
         level = 0;
      
      // fill in the grid with pointers to this node
      S32   gridWidth = 1 << level;
      for (S32 y = 0; y < gridWidth; y++)
         for (S32 x = 0; x < gridWidth; x++) 
         {
            Point2I  P = Point2I(x,y) + gridPoint;
            grid_out[ area_in.getIndex( P ) ] = outdoorNode;
         }
         
      // Construct the outdoor node type with position and level information.  
      // Fill in the grid here.  
      // if (level > 0) 
      {
         Point3F     middleOff (gridWidth << gridShift-1, gridWidth << gridShift-1, 0);
         
         if (level == 0)        //==>  Need to handle -1, 0 differently.  
            middleOff *= 0.0;
            
         outdoorNode->mLoc += middleOff;
         if (!terrainHeight(outdoorNode->mLoc, & outdoorNode->mLoc.z))
         {
            // This was put here to make obvious a bug a while ago that seems
            // to have been gone for quite a while.  
            outdoorNode->mLoc.z = 120;
            warning("graphOutdoors.cc:  No terrain found in middle of grid node");
         }
      }
   }

   // Do the unroll loop to put into the node list proper.  
   UnrollOutdoorList  listUnroller(area_in, grid_out, list_out);
   listUnroller.traverse();
   
   // Make sure it matches our data:
   AssertFatal(list_out.size() == cons.size(), "setupOutdoorNodes: data doesn't jibe");
   
   // Set indices. Our approach is desgined to have larger squares first in the list.
   // Uh, there was a potential reason for that at some point, not utilized I think... 
   for (i = 0; i < list_out.size(); i++)
      (dynamic_cast<OutdoorNode *>(list_out[i]))->mIndex = i;

   return list_out.size();
}

//-------------------------------------------------------------------------------------
//       Visitor to hook up the nodes.  

class HookOutdoorNodes : public GridVisitor
{
  protected:
   const GraphNodeList&    mGrid;
   const Vector<U8>&       mNeighbors;
   U16 * const             mCounting;
   
   bool     hookBothWays(GraphNode* node, const Point2I& off);
   S32      getIndex(const GridArea& area);
   
  public:
   HookOutdoorNodes(const GridArea& world, const GraphNodeList& grid, 
                     const Vector<U8>& neighbors, U16 * const counts = NULL)
         : GridVisitor(world), mGrid(grid), mNeighbors(neighbors), mCounting(counts) {}

   bool     beforeDivide(const GridArea& R, S32 level);
   bool     atLevelZero(const GridArea& R);
};


// Points for hooking to 12 neighbors.  Offsets are in multiples of half the 
//    square width- further going across.  See cornerStep & sideStep variables
//       in loop below for order of these.  Basically Left->Right, and then Up.
static const Point2I hookCorners[4] = {
   Point2I( 0, 0 ), 
   Point2I( 2, 0 ), 
   Point2I( 0, 2 ), 
   Point2I( 2, 2 )
};
static const Point2I hookSides[8] = {
   Point2I( 0, 0 ), Point2I( 1, 0 ),
   Point2I( 0, 0 ), Point2I( 0, 1 ),
   Point2I( 2, 0 ), Point2I( 2, 1 ),
   Point2I( 0, 2 ), Point2I( 1, 2 )
};

S32 HookOutdoorNodes::getIndex(const GridArea& R)
{
   S32   index = mArea.getIndex( R.point );
   AssertFatal( validArrayIndex(index, mGrid.size()), "Node unroll bad index" );
   return index;
}


// Hook the node, and return true if it was successful and it was a DOWNWARD hook.
// (i.e. down by ONE level).  For downward hooks we also hook self into their 
// list.  Also do single hook ACROSS, and return false.  
bool HookOutdoorNodes::hookBothWays(GraphNode* node, const Point2I& where)
{
   S32   index = mArea.getIndex( where );
   
   if (index >= 0)
   {
      if (GraphNode * neighbor = mGrid[ index ]) 
      {
         S32   srcLevel = node->getLevel();
         S32   dstLevel = neighbor->getLevel();
         
         OutdoorNode *  from = dynamic_cast<OutdoorNode *>(node);
         OutdoorNode *  to = dynamic_cast<OutdoorNode *>(neighbor);

         // check our assumptions- 
         AssertFatal (from && to && srcLevel>-2 && dstLevel>-2, "hookBothWays- ASKEW");
         AssertFatal (mAbs(srcLevel - dstLevel) <= 1, "hookBothWays- UNSMOOTH BORDER");
         
         // consider -1 and 0 to be same level for purposes of this routine- 
         if (srcLevel < 0)    srcLevel = 0;
         if (dstLevel < 0)    dstLevel = 0;
         
         // Hook to down-by-1 level, or across.  Former case hooks both & returns true.
         // 10/12/2000:  Added counting mode which doesn't push (for pool allocation).
         if( srcLevel == dstLevel + 1 )
         {
            if (mCounting) {
               mCounting[from->getIndex()]++;
               mCounting[to->getIndex()]++;
            }
            else {
               from->pushEdge(to);
               to->pushEdge(from);
            }
            return true;
         }
         else if( srcLevel == dstLevel )
         {
            if (mCounting)
               mCounting[from->getIndex()]++;
            else
               from->pushEdge(to);
            // fall through return false
         }
      }
   }
   return false;
}


// Hook to all neighbors of lower or equal level.  When the level is equal, then
//    we don't do the hook back (they'll do it themselves).
//
// Assert that level difference is preserved.  
// 
bool HookOutdoorNodes::beforeDivide(const GridArea& R, S32 level)
{
   GraphNode * node = mGrid[ getIndex (R) ];
   
   if( node && node->getLevel() == level )
   {
      // Look at all 12 neighbors - two on each side, and the four corners.  
      S32   cornerStep = 0, sideStep = 0;
      S32   halfWidth = (1 << level-1);
      S32   x, y;

      // Loop through sides and corners, skipping middle- 
      for (y = -1; y <= 1; y++)  for (x = -1; x <= 1; x++)  if (x || y)
      {
         // Get offset from the boundaries computed above.  We only offset for
         // negative components, hence the strange math:  
         Point2I  gridOffset((x < 0) * x, (y < 0) * y);
         
         if (x && y)
         {
            Point2I  cornerOff = hookCorners[cornerStep++];
            (cornerOff *= halfWidth) += gridOffset;
            hookBothWays(node, cornerOff += R.point);
         }
         else // (x XOR y)
         {
            for (S32 adjacent = 0; adjacent < 2; adjacent++)
            {
               Point2I  sideOff = hookSides[sideStep * 2 + adjacent];
               sideOff *= halfWidth;
               sideOff += gridOffset;
               sideOff += R.point;
               if( ! hookBothWays(node, sideOff) )
                  break;
            }
            sideStep++;
         }
      }
      
      // Stop the Visitor subdivision-
      return false;
   }
   
   // Tell Visitor to recurse further-
   return true;
}

bool HookOutdoorNodes::atLevelZero(const GridArea& R)
{
   // Hook to all neighbors.  At this level we don't hook back (each hooks itself).  
   S32   index = getIndex(R);
   if (GraphNode * node = mGrid[index])
      for (S32 dir = 0; dir < 8; dir++)
         if (mNeighbors[index] & (1 << dir))
         {
            S32   x = TerrainGraphInfo::gridOffs[dir].x;
            S32   y = TerrainGraphInfo::gridOffs[dir].y;
         
            bool  didBoth = true;
         
            if(x && y) {
               // diagonal hooks must gaurantee that there are valid nodes on either 
               //    side of the line we're proposing to connect.  
               Point2I  acrossDiagP1(x,0);
               Point2I  acrossDiagP2(0,y);
               S32      ind1 = mArea.getIndex(R.point + acrossDiagP1);
               S32      ind2 = mArea.getIndex(R.point + acrossDiagP2);
            
               if (ind1 >= 0 && ind2 >= 0 && mGrid[ind1] && mGrid[ind2])
                  didBoth = hookBothWays(node, Point2I(x,y) + R.point);
               else
                  didBoth = false;
            }
            else
               didBoth = hookBothWays(node, Point2I(x,y) + R.point);
            
            AssertFatal(!didBoth, "HookOutdoor: only lateral hook at level zero");
         }

   return true;
}

//-------------------------------------------------------------------------------------

// Create our run time consolidated nodes from the data that has been generated. 
void NavigationGraph::makeRunTimeNodes(bool useEdgePool)
{
   GridArea          worldArea(mTerrainInfo.originGrid, mTerrainInfo.gridDimensions);
   HookOutdoorNodes  hookingVisitor(worldArea, mNodeGrid, mTerrainInfo.neighborFlags);
   U16   *           edgeCounts = NULL;
   
   S32   totalNodeCount = mTerrainInfo.consolidated.size() + mNodeInfoList.size();
   
   delete [] mOutdoorNodes;
   mOutdoorNodes = NULL;
   mNumOutdoor = 0;
   mNodeList.clear();
   mNodeGrid.clear();
   mNodeList.reserve(mTerrainInfo.consolidated.size() + mNodeInfoList.size());
   
   if (haveTerrain())
   {
      mNodeGrid.reserve(worldArea.extent.x * worldArea.extent.y);
      // Makes outdoor grid and starts the node list- 
      mNumOutdoor = setupOutdoorNodes(worldArea, mTerrainInfo.consolidated, mNodeGrid, mNodeList);
   }
   
   // Need to allocate these here for edge pool-    
   mIndoorNodes.init(mNodeInfoList.size());
   
   // Here is where we precompute edges if we're using a pool.  To know outdoor counts,
   // we run the traverser in a different mode.  Then roll in stored edges and bridges.
   delete [] mEdgePool;
   mEdgePool = NULL;
   
   if (useEdgePool) 
   {
      edgeCounts = new U16 [totalNodeCount];

      // For now it reserves space for Transient push, will change (since that is 
      // a wasteful (though simple) approach).  
      for (S32 i = 0; i < totalNodeCount; i++)
         edgeCounts[i] = 2;

      // Count outdoor edges- 
      if (haveTerrain())
      {
         HookOutdoorNodes counter(worldArea, mNodeGrid, mTerrainInfo.neighborFlags, edgeCounts);
         counter.traverse();
      }
                              
      // Count indoor- 
      for (S32 e = 0; e < mEdgeInfoList.size(); e++) 
         for (S32 dir = 0; dir < 2; dir++) 
            edgeCounts[mNumOutdoor + mEdgeInfoList[e].to[dir].dest]++;
      
      // Count bridge edges- 
      mBridgeList.accumEdgeCounts(edgeCounts);

      // Get the total size needed- 
      U32   totalEdgeCount = 0;
      for (S32 t = 0; t < totalNodeCount; t++)
         totalEdgeCount += edgeCounts[t];

      // Allocate the pool (a NavigationGraph member)
      mEdgePool = new GraphEdge[totalEdgeCount];
      Con::printf("Allocated edge pool of size %d", totalEdgeCount);

      // Set the outdoor pointers into the pool- 
      U32   offset = 0;
      for (S32 o = 0; o < mNumOutdoor; o++) 
      {
         mNodeList[o]->mEdges.setOwned(&mEdgePool[offset], edgeCounts[o]);
         offset += edgeCounts[o];
      }

      // Set the indoor edge pool pointers. 
      for (S32 x = 0; x < mIndoorNodes.size(); x++) 
      {
         U16   count = edgeCounts[x + mNumOutdoor];
         mIndoorNodes[x].mEdges.setOwned(&mEdgePool[offset], count);
         offset += count;
      }
   
      AssertFatal(offset == totalEdgeCount, "makeRunTimeNodes()");
   }
   
   // hooks up the edges.  
   hookingVisitor.traverse();
   
   // Set up the interior nodes.  Last param gives starting index- 
   initInteriorNodes(mEdgeInfoList, mNodeInfoList, mNodeList.size());
   mNumIndoor = mIndoorNodes.size();
   
   // Put the indoor nodes on our pointer list.  
   IndoorNodeList::iterator   in;
   for (in = mIndoorNodes.begin(); in != mIndoorNodes.end(); in++) 
      mNodeList.push_back(in);
   
   findJumpableNodes();
   
   // Set up the shoreline.  Push it back more for Lava so bots give it wider berth. 
   expandShoreline(0);
   if (mDeadlyLiquid)
      expandShoreline(1);
   
   S32 numHanging = doFinalFixups();
   
   delete [] edgeCounts;
      
   // AssertFatal(numHanging < 1, "Run time consolidate hookup making hanging nodes");
}

//-------------------------------------------------------------------------------------

// This sets the shoreline bit, which will affect edge scaling.  Note that this method 
// can be called multiple times to expand what is considered shoreline.  This only 
// operates on outdoor nodes and their outdoor neighbors.  
void NavigationGraph::expandShoreline(U32 wave)
{
   AssertFatal(wave < 3, "Only three shoreline bits available");
   
   U32   extendFromMask = (GraphNode::ShoreLine << wave);
   U32   extendToMask = (extendFromMask << 1);
   
   for (S32 i = 0; i < mNumOutdoor; i++) {
      if (GraphNode * node = lookupNode(i)) {
         if (node->test(extendFromMask)) {
            GraphEdgeArray    edges = node->getEdges(mEdgeBuffer);
            while (GraphEdge * edge = edges++) {
               if (!edge->isJetting() && (edge->mDest < mNumOutdoor)) {
                  GraphNode * neighbor = lookupNode(edge->mDest);
                  if (!neighbor->test(extendFromMask))
                     neighbor->set(extendToMask);
               }
            }
         }
      }
   }
}

//-------------------------------------------------------------------------------------
//                               Outdoor Node Methods


S32 OutdoorNode::getLevel() const
{ 
   return S32(mLevel); 
}

OutdoorNode::OutdoorNode()
{
   mIndex = -1;
   mLoc.set(-1,-1,-1);
   mFlags.set(Outdoor);
}

Point3F OutdoorNode::getRenderPos() const
{
   Point3F  adjustPos = location();
   
   if( mLevel > 0 ){
      F32 up = F32(mLevel);
      adjustPos.z += (up * 0.7);
   }
   else 
      adjustPos.z += 0.2;
   
   return adjustPos;
}

// Randomize a location within an outdoor node.  Choose random offsets that are at least 1
// unit away from grid axes, and at least 0.5 units in from sides- 
Point3F OutdoorNode::randomLoc() const 
{
   Point3F  loc = location();
   F32      *xy = loc;
   F32      R = radius();

   for (S32 i = 0; i < 2; i++) 
   {
      F32   off = gRandGen.randF() * (R - 1.5) + 1.0;
      if (gRandGen.randI() & 1)
         xy[i] += off;
      else
         xy[i] -= off;
   }

   gNavGraph->terrainHeight(loc, &loc.z);
   loc.z += 1.0;     // ===> Extra should depend on slope (if issue now w/ preprocess)?
   
   return loc;
}

