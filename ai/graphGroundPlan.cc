//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "terrain/terrData.h"
#include "Editor/terrainActions.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "Collision/clippedPolyList.h"
#include "ai/graphGroundVisit.h"
#include "terrain/waterBlock.h"

GroundPlan *gGroundPlanTest = NULL;
IMPLEMENT_CONOBJECT(GroundPlan);

//  4 6 7
//  2 N 5
//  0 1 3
Point2I GroundPlan::gridOffset[8] = 
{
   Point2I(-1, -1),// 0
   Point2I(0, -1),   Point2I(-1, 0), // 1 2
   Point2I(1, -1),   Point2I(-1, 1), // 3 4
   Point2I(1, 0),    Point2I(0, 1),  // 5 6
   Point2I(1, 1)   // 7
};


GroundPlan::GroundPlan()
{
   mTerrainBlock = 0;
   mTotalVisited = 0;
}

//----------------------------------------------------------------------------

GroundPlan::~GroundPlan()
{
}

//----------------------------------------------------------------------------

GridDetail GroundPlan::defaultDetail()
{
   GridDetail detail;
   detail.mNavFlags[0] = GridDetail::clear;
   detail.mNavFlags[1] = GridDetail::clear;
   detail.mShadowHts[0] = -1;
   detail.mShadowHts[1] = -1;
   return detail;
}

//----------------------------------------------------------------------------

void GroundPlan::initPersistFields()
{
	Parent::initPersistFields();
}

//----------------------------------------------------------------------------

bool GroundPlan::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mTerrainBlock = getTerrainObj();
   gGroundPlanTest = this;
   return true;   
}

//----------------------------------------------------------------------------

void GroundPlan::onRemove()
{
   Parent::onRemove();
   gGroundPlanTest = NULL;
}

//----------------------------------------------------------------------------

void GroundPlan::getClippedRegion(S32 index, S32 &st, S32 &ed, S32 &xspan, S32 &yspan)
{
   xspan = 16;
   yspan = 16;
   st = (index - 8) - (8 * mGridDims.x);
   ed = (index + 8) + (8 * mGridDims.x);
}

//----------------------------------------------------------------------------

bool GroundPlan::isOnBoundary(S32 i)
{
   if (i % mGridDims.x == 0)
      return true;
   if (i < mGridDims.x)
      return true;
   if (((i+1) % mGridDims.x) == 0)
      return true;
   if (i >= ((mGridDims.x * mGridDims.y) - mGridDims.x))
      return true;
      
   return false;
}

//----------------------------------------------------------------------------

S32 GroundPlan::gridToIndex(const Point2I &gPos)
{
   S32 index;
   index = gPos.x - mGridOrigin.x;
   index += ((gPos.y - mGridOrigin.y) * mGridDims.x);
   return index;
}

//----------------------------------------------------------------------------

void GroundPlan::gridToWorld(const Point2I &gPos, Point3F &wPos)
{
   const MatrixF &mat = mTerrainBlock->getTransform();
   Point3F origin;
   mat.getColumn(3, &origin);

   wPos.x = gPos.x * (float)mTerrainBlock->getSquareSize() + origin.x;
   wPos.y = gPos.y * (float)mTerrainBlock->getSquareSize() + origin.y;
   wPos.z = getGridHeight(gPos);
}

//----------------------------------------------------------------------------

void GroundPlan::worldToGrid(const Point3F &wPos, Point2I &gPos)
{
   const MatrixF & mat = mTerrainBlock->getTransform();
   Point3F origin;
   mat.getColumn(3, &origin);

   float x = (wPos.x - origin.x) / (float)mTerrainBlock->getSquareSize();
   float y = (wPos.y - origin.y) / (float)mTerrainBlock->getSquareSize();
   
   gPos.x = (S32)mFloor(x);
   gPos.y = (S32)mFloor(y);
}

//----------------------------------------------------------------------------

void GroundPlan::gridToCenter(const Point2I &gPos, Point2I &cPos)
{
   cPos.x = gPos.x & TerrainBlock::BlockMask;
   cPos.y = gPos.y & TerrainBlock::BlockMask;
}

//----------------------------------------------------------------------------

F32 GroundPlan::getGridHeight(const Point2I &gPos)
{
   Point2I cPos;
   gridToCenter(gPos, cPos);
   return(fixedToFloat(mTerrainBlock->getHeight(cPos.x, cPos.y)));
}

//----------------------------------------------------------------------------
                                 
TerrainBlock *GroundPlan::getTerrainObj()
{
   SimObject *obj = Sim::findObject("Terrain");
   if(!obj)
      return(0);
   
   TerrainBlock *terrain = static_cast<TerrainBlock *>(obj);
   return(terrain);
}

//----------------------------------------------------------------------------

bool GroundPlan::inspect(GridArea &area)
{
   if(!mTerrainBlock)
   {
      mTerrainBlock = getTerrainObj();
      if(!mTerrainBlock)
      {   
         Con::printf("no Terrain object");
         return false;   
      }
   }
   
   AssertFatal(area.extent.x > 0 && area.extent.y > 0, "no extents to mission area");

   GridArea inspectArea;
   Point3F wPos(area.point.x, area.point.y, 0);
   worldToGrid(wPos, inspectArea.point);
   
   mGridOrigin = inspectArea.point;
   mGridDims.x = (area.extent.x >> 3);
   mGridDims.y = (area.extent.y >> 3);
   mGridDatabase.clear();
   
   // build the initial database with default details
   for(S32 y = mGridOrigin.y; y < (mGridOrigin.y + mGridDims.y); y++)
   {
      for(S32 x = mGridOrigin.x; x < (mGridOrigin.x + mGridDims.x); x++)
      {
         GridDetail detail;
         detail = defaultDetail();
         mGridDatabase.push_back(detail);
      }
   }
   
   // perform mission area inspection
   GridArea inspectionArea(mGridOrigin, mGridDims);
   InspectionVisitor inspector(inspectionArea, *this);
   inspector.traverse();
   computeNavFlags();
   findNeighbors();
   
   Con::printf("total zero squares inspected: %d\n", mTotalVisited);
   
   // were done!
   return true;
}

//-------------------------------------------------------------------------------------

// static void doTempBreak() {}
// static Point3F minBreakPt(936, 1184, 0);
// static Point3F maxBreakPt(944, 1192, 0);

//-------------------------------------------------------------------------------------

void GroundPlan::inspectSquare(const GridArea &gridArea, GridDetail &square)
{
   S32 sqSize = mTerrainBlock->getSquareSize();
   Point2I cPos;
   gridToCenter(gridArea.point, cPos);
   bool is45 = isSplit45(cPos);
   bool isEmpty = false;
   Point3F verts[2][3];
   Point3F mid1;
   Point3F mid2;
   Point3F wPos;
   
   gridToWorld(gridArea.point+gridOffset[6], mid1); // 0,1
   gridToWorld(gridArea.point+gridOffset[5], mid2); // 1,0
   gridToWorld(gridArea.point, wPos);
   
   Box3F area;
   area.min = wPos;
   area.max.x = area.min.x + sqSize;
   area.max.y = area.min.y + sqSize;
   area.max.z = getGridHeight(gridArea.point + gridOffset[7]);
   
   // if (within_2D(area.min, minBreakPt, 0.1))
   //    if (within_2D(area.max, maxBreakPt, 0.1))
   //       doTempBreak();
   
   if(is45)
   {
      verts[0][0] = area.min;
      verts[0][1] = mid1;
      verts[0][2] = area.max;
      verts[1][0] = area.min;
      verts[1][1] = area.max;
      verts[1][2] = mid2;
   }
   else
   {
      verts[0][0] = area.min;
      verts[0][1] = mid1;
      verts[0][2] = mid2;
      verts[1][0] = mid1;
      verts[1][1] = area.max;
      verts[1][2] = mid2;
   }

   square.mWorld[0][0] = verts[0][0];
   square.mWorld[0][1] = verts[0][1];
   square.mWorld[0][2] = verts[0][2];
   square.mWorld[1][0] = verts[1][0];
   square.mWorld[1][1] = verts[1][1];
   square.mWorld[1][2] = verts[1][2];

   // check for empty square
   GridSquare *gs = mTerrainBlock->findSquare(0, gridArea.point);
   if(gs->flags & GridSquare::Empty)
      isEmpty = true;
   
   if(!isEmpty)
   {
      U32 mask = InteriorObjectType | TurretObjectType; 
      
      RayInfo collision;
      Point3F endPt1, endPt2, endPt3, endPt4;

      endPt1 = verts[0][1];         // mid1 (0,1)
      endPt2 = verts[1][1];         // area max
      endPt3 = verts[1][2];         // mid2 (1,0)
      endPt4 = verts[0][0];         // area min

      for (S32 i = 0; i < 2; i++)
      {
         bool  done = false;
         if (is45) {
            if (i == 0) {
               if(gServerContainer.castRay(endPt4, endPt1, mask, &collision)  ||
                  gServerContainer.castRay(endPt1, endPt2, mask, &collision)  ||
                  gServerContainer.castRay(endPt2, endPt4, mask, &collision))
               {
                  square.mNavFlags[i] = GridDetail::obstructed;
                  square.mShadowHt = -1;
                  done = true;
               }
            }
            else
            {
               if(gServerContainer.castRay(endPt4, endPt2, mask, &collision)  ||
                  gServerContainer.castRay(endPt2, endPt3, mask, &collision)  ||
                  gServerContainer.castRay(endPt3, endPt4, mask, &collision))
               {   
                  square.mNavFlags[i] = GridDetail::obstructed;
                  square.mShadowHt = -1;
                  done = true;
               }
            }
         }
         else
         {
            if(i == 0)
            {
               if(gServerContainer.castRay(endPt4, endPt1, mask, &collision)  ||
                  gServerContainer.castRay(endPt1, endPt3, mask, &collision)  ||
                  gServerContainer.castRay(endPt3, endPt4, mask, &collision))
               {   
                  square.mNavFlags[i] = GridDetail::obstructed;
                  square.mShadowHt = -1;
                  done = true;
               }
            }
            else
            {
               if(gServerContainer.castRay(endPt1, endPt2, mask, &collision)  ||
                  gServerContainer.castRay(endPt2, endPt3, mask, &collision)  ||
                  gServerContainer.castRay(endPt3, endPt1, mask, &collision))
               {   
                  square.mNavFlags[i] = GridDetail::obstructed;
                  square.mShadowHt = -1;
                  done = true;
               }
            }
         }
   
         if(!done)
         {
            Point3F  min, mid, max;
            VectorF  norm1, norm2, norm3;
            Box3F    bBox(endPt4, endPt2);
            Point3F  extra(1, 1, 0);
            bBox.min -= extra;
            bBox.max += extra;
            bBox.max.z += 1000;
         
            if(is45) {
               if(i == 0) {
                  min = endPt4;
                  mid = endPt1;
                  max = endPt2;
                  norm1.set(-1, 0, 0);
                  norm2.set(0, 1, 0);
                  norm3.set(0.7071, -0.7071, 0);
               }
               else {
                  min = endPt4;
                  mid = endPt2;
                  max = endPt3;
                  norm1.set(-0.7071, 0.7071, 0);
                  norm2.set(1, 0, 0);
                  norm3.set(0, -1, 0);
               }
            }
            else {
               if(i == 0) {
                  min = endPt4;
                  mid = endPt1;
                  max = endPt3;
                  norm1.set(-1, 0, 0);
                  norm2.set(0.7071, 0.7071, 0);
                  norm3.set(0, -1, 0);
               }
               else {
                  min = endPt1;
                  mid = endPt2;
                  max = endPt3;
                  norm1.set(0, 1, 0);
                  norm2.set(1, 0, 0);
                  norm3.set(-0.7071, -0.7071, 0);
               }
            }

            F32   threshold = 2.2, heightDiff;
            
            if(setupPolyList(bBox, min, mid, max, norm1, norm2, norm3, &heightDiff))
            {
               if(heightDiff > threshold)
               {
                  square.mNavFlags[i] = GridDetail::shadowed;
                  square.mShadowHts[i] = heightDiff;
               }
               else 
               {
                  square.mNavFlags[i] = GridDetail::obstructed;
                  square.mShadowHts[i] = -1;
               }
            }
            else
            {
               square.mNavFlags[i] = GridDetail::clear;
               square.mShadowHts[i] = -1;
            }
         }
      }
   }
   else
   {
      square.mNavFlags[0] = GridDetail::obstructed;
      square.mNavFlags[1] = GridDetail::obstructed;
      square.mShadowHts[0] = -1;
      square.mShadowHts[1] = -1;
   }
}

//-------------------------------------------------------------------------------------

// Changed to account for actual worst case height in the triangle.  As it was, it 
// would yeild negative heights on some obstructed areas on slopes, which would cause
// the inspect to mark it clear. 

bool GroundPlan::setupPolyList(const Box3F   &bBox,       
                               const Point3F &min,
                               const Point3F &mid,
                               const Point3F &max,
                               const VectorF &norm1,
                               const VectorF &norm2,
                               const VectorF &norm3,
                                     F32     *height  )
{   
   ClippedPolyList polyList;
   
   polyList.mPlaneList.clear();
   polyList.mPlaneList.setSize(5);
   
   // Build a bottom plane using the three points.  
   Point3F  vec1 = (mid - max);
   Point3F  vec2 = (mid - min);
   VectorF  bottomNormal;
   mCross(vec1, vec2, &bottomNormal);
   if (bottomNormal.z > 0)
      bottomNormal.z = -bottomNormal.z;
      
   // Need to find maximum Z value as basis for determining height.
   F32 highestZ = min.z;
   if (max.z > highestZ)
      highestZ = max.z;
   if (mid.z > highestZ)
      highestZ = mid.z;
   
   // Adjust out the triangle to ensure more "breathing room" around obstacles
   Point3F adjusted1 = (min + (norm1 * 0.8f));
   Point3F adjusted2 = (mid + (norm2 * 0.8f));
   Point3F adjusted3 = (max + (norm3 * 0.8f));
   
   polyList.mNormal.set(0, 0, 0);
   polyList.mPlaneList[0].set(adjusted1, norm1);
   polyList.mPlaneList[1].set(adjusted2, norm2);
   polyList.mPlaneList[2].set(adjusted3, norm3);
   // polyList.mPlaneList[3].set(min, VectorF(0, 0, -1));
   polyList.mPlaneList[3].set(min, bottomNormal);
   polyList.mPlaneList[4].set(bBox.max, VectorF(0, 0, 1));
   
   U32 mask = InteriorObjectType | TurretObjectType; 
   
   // build the poly list
   if(gServerContainer.buildPolyList(bBox, mask, &polyList))
   {
      F32   lowestZ = 10000000, lowestDeltaZ;
      bool  found = false;
      
      for (S32 j = 0; j < polyList.mPolyList.size(); j++) {
         ClippedPolyList::Poly *p = &(polyList.mPolyList[j]);
         for (S32 k = p->vertexStart; k < (p->vertexStart + p->vertexCount); k++) {   
            F32   ht = polyList.mVertexList[polyList.mIndexList[k]].point.z;
            if(ht < lowestZ) {
               lowestZ = ht;
               lowestDeltaZ = (ht - highestZ);
               found = true;
            }
         }
      }
      
      if (found) {
         *height = lowestDeltaZ;
         return true;
      }
   }
   return false;
}

//----------------------------------------------------------------------------

void GroundPlan::computeNavFlags()
{
   bool isBottomRow = false;
   bool isLeftEdge = false;
   bool is45 = false;
   bool hasWater = missionHasWater();
   BitSet32 obsBits;
   S32 totalNodes = mGridDatabase.size();
   
   // detection of water or liquids
   Vector<WaterBlock *> blocks;
   if(hasWater)
      getAllWaterBlocks(blocks);
   
   U32 i;
   for(i = 0; i < totalNodes; i++)
   {
      is45 = isSplit45(*(getPosition(i)));
      isBottomRow = false;
      isLeftEdge = false;
      
      if(i % mGridDims.x == 0)
         isLeftEdge = true;
      if(i < mGridDims.x)
         isBottomRow = true;
      
      GridDetail &g = mGridDatabase[i];
      packTriangleBits(g, obsBits, GridDetail::obstructed, i, isLeftEdge, isBottomRow);
      
      if(is45)
      {
         if(obsBits.testStrict(0xff))
            g.mNavFlag = GridDetail::obstructed;
         else
         {   
            // check for shadowed node based on surrounding triangles
            BitSet32 shBits;
            packTriangleBits(g, shBits, GridDetail::shadowed, i, isLeftEdge, isBottomRow);
            if(shBits > 0)
            {   
               g.mNavFlag = GridDetail::shadowed;
               //g.mShadowHt = 20;
               g.mShadowHt = lowestShadowHt(g, shBits, i);
            }
            else
               g.mNavFlag = GridDetail::clear;
         }
      }
      else
      {
         if(obsBits.test(BIT(1)) && obsBits.test(BIT(2)) && obsBits.test(BIT(5)) && obsBits.test(BIT(6)))
            g.mNavFlag = GridDetail::obstructed;
         else
         {   
            // check for shadowed node based on surrounding triangles
            BitSet32 shBits;
            packTriangleBits(g, shBits, GridDetail::shadowed, i, isLeftEdge, isBottomRow);
            if(shBits > 0)
            {   
               g.mNavFlag = GridDetail::shadowed;
               g.mShadowHt = 20;
               g.mShadowHt = lowestShadowHt(g, shBits, i);
            }
            else
               g.mNavFlag = GridDetail::clear;
         }
      }
      
      // set water bit if needed
      if(hasWater)
      {
         U32 blockCount;
         for(blockCount = 0; blockCount < blocks.size(); blockCount++)
         {
            Point2I *gPos = getPosition(i);
            Point3F wPos;
            gridToWorld((*gPos), wPos); 
            if((wPos.x < -292 && wPos.x > -298) && (wPos.y < -130 && wPos.y > -135))
               bool test = true;
            
            if(blocks[blockCount]->isPointSubmergedSimple(wPos))
            {   
               g.mNavFlag = GridDetail::submerged;
               break; // once the bit is set, it can't change.
            }   
         }
      }
   }
}

//----------------------------------------------------------------------------

void GroundPlan::findNeighbors()
{
   bool isBottomRow = false;
   bool isLeftEdge = false;
   bool is45;
   BitSet32 obsBits;
   
   S32 totalNodes = mGridDatabase.size();
   
   U32 i;
   for(i = 0; i < totalNodes; i++)
   {
      isBottomRow = false;
      isLeftEdge = false;
      
      if(i % mGridDims.x == 0)
         isLeftEdge = true;
      if(i < mGridDims.x)
         isBottomRow = true;
      
      is45 = isSplit45(*(getPosition(i)));
      GridDetail &g = mGridDatabase[i];
      packTriangleBits(g, obsBits, GridDetail::obstructed, i, isLeftEdge, isBottomRow);
      
      // we have the info, now find the neighbors
      g.mNeighbors.clear();

      if(is45)
      {   
         if(!obsBits.test(BIT(0)) || !obsBits.test(BIT(1)))
            g.mNeighbors.set(BIT(0));
         if(!obsBits.test(BIT(1)) || !obsBits.test(BIT(2)))
            g.mNeighbors.set(BIT(1));
         if(!obsBits.test(BIT(4)) || !obsBits.test(BIT(0)))
            g.mNeighbors.set(BIT(2));
         if(!obsBits.test(BIT(2)) || !obsBits.test(BIT(3)))
            g.mNeighbors.set(BIT(3));
         if(!obsBits.test(BIT(4)) || !obsBits.test(BIT(5)))
            g.mNeighbors.set(BIT(4));
         if(!obsBits.test(BIT(3)) || !obsBits.test(BIT(7)))
            g.mNeighbors.set(BIT(5));
         if(!obsBits.test(BIT(5)) || !obsBits.test(BIT(6)))
            g.mNeighbors.set(BIT(6));
         if(!obsBits.test(BIT(6)) || !obsBits.test(BIT(7)))
            g.mNeighbors.set(BIT(7));
      }
      else
      {
         // todo:
         // check for slope when crossing triangle boundry.
         if(!obsBits.test(BIT(0)) && !obsBits.test(BIT(1)))
            g.mNeighbors.set(BIT(0));
         if(!obsBits.test(BIT(1)) || !obsBits.test(BIT(2)))
            g.mNeighbors.set(BIT(1));
         if(!obsBits.test(BIT(1)) || !obsBits.test(BIT(5)))
            g.mNeighbors.set(BIT(2));
         if(!obsBits.test(BIT(2)) && !obsBits.test(BIT(3)))
            g.mNeighbors.set(BIT(3));
         if(!obsBits.test(BIT(4)) && !obsBits.test(BIT(5)))
            g.mNeighbors.set(BIT(4));
         if(!obsBits.test(BIT(6)) || !obsBits.test(BIT(2)))
            g.mNeighbors.set(BIT(5));
         if(!obsBits.test(BIT(5)) || !obsBits.test(BIT(6)))
            g.mNeighbors.set(BIT(6));
         if(!obsBits.test(BIT(6)) && !obsBits.test(BIT(7)))
            g.mNeighbors.set(BIT(7));
      }   
   }
}

//----------------------------------------------------------------------------

S32 GroundPlan::getNavigable(Vector<U8> &navFlags, Vector<F32> &shadowHts)
{
   U32 i;
   S32 shadowed = 0;
   S32 totalNodes = mGridDatabase.size();
   navFlags.setSize( totalNodes );
   shadowHts.setSize( totalNodes );

   for(i = 0; i < totalNodes; i++)
   {
      navFlags[i] = mGridDatabase[i].mNavFlag;
      shadowHts[i] = mGridDatabase[i].mShadowHt;
      if(mGridDatabase[i].mNavFlag == GridDetail::shadowed)
         shadowed++;            
   }
   return shadowed;
}

//----------------------------------------------------------------------------

Vector<U8> &GroundPlan::getNeighbors(Vector<U8> &neighbors)
{
   U32 i;
   S32 totalNodes = mGridDatabase.size();
   neighbors.setSize( totalNodes );
   for(i = 0; i < totalNodes; i++)
      neighbors[i] = mGridDatabase[i].mNeighbors;
   return neighbors;
}

//----------------------------------------------------------------------------

void GroundPlan::packTriangleBits(const GridDetail &g, BitSet32 &bits, S32 bitType, S32 index, bool isLeftEdge, bool isBottomRow)
{
   GridDetail *grid;

   bool isRightEdge = false;
   bool isTopRow = false;
   
   if(((index+1) % mGridDims.x) == 0)
      isRightEdge = true;
   if(index >= ((mGridDims.x * mGridDims.y) - mGridDims.x))
      isTopRow = true;
   
   bits.clear();
   if(isBottomRow && isRightEdge)
   {
      // these are considered obstructed triangles
      // since we are sitting at the origin
      if(bitType == GridDetail::obstructed)
         bits.set(BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(6)|BIT(7));
      
      grid = &(mGridDatabase[getNeighborDetails(index, 2)]);
      if(grid->mNavFlags[0] == bitType)
         bits.set(BIT(4));
      if(grid->mNavFlags[1] == bitType)
         bits.set(BIT(5));
   }
   else if(isLeftEdge && isTopRow)
   {
      // these are considered obstructed triangles
      // since we are sitting at the origin
      if(bitType == GridDetail::obstructed)
         bits.set(BIT(0)|BIT(1)|BIT(4)|BIT(5)|BIT(6)|BIT(7));
      
      grid = &(mGridDatabase[getNeighborDetails(index, 1)]);
      if(grid->mNavFlags[0] == bitType)
         bits.set(BIT(2));
      if(grid->mNavFlags[1] == bitType)
         bits.set(BIT(3));
   }
   else if(!isBottomRow && !isLeftEdge)
   {
      // checks for right and top edges
      if(isTopRow && !isRightEdge)
      {
         if(bitType == GridDetail::obstructed)
            bits.set(BIT(6)|BIT(7)|BIT(4)|BIT(5));

         grid = &(mGridDatabase[getNeighborDetails(index, 0)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(0));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(1));
   
         grid = &(mGridDatabase[getNeighborDetails(index, 1)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(2));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(3));
      }
      else if(isRightEdge && !isTopRow)
      {
         if(bitType == GridDetail::obstructed)
            bits.set(BIT(2)|BIT(3)|BIT(6)|BIT(7));

         grid = &(mGridDatabase[getNeighborDetails(index, 0)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(0));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(1));

         grid = &(mGridDatabase[getNeighborDetails(index, 2)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(4));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(5));
      }
      else if(index != mGridDatabase.size()-1)
      {
         // need to test all triangles
         grid = &(mGridDatabase[getNeighborDetails(index, 0)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(0));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(1));
   
         grid = &(mGridDatabase[getNeighborDetails(index, 1)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(2));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(3));
   
         grid = &(mGridDatabase[getNeighborDetails(index, 2)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(4));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(5));
   
         if(g.mNavFlags[0] == bitType)
            bits.set(BIT(6));
         if(g.mNavFlags[1] == bitType)
            bits.set(BIT(7));
      }
      else
      {
         if(bitType == GridDetail::obstructed)
            bits.set(BIT(6)|BIT(7)|BIT(2)|BIT(3)|BIT(4)|BIT(5));
         
         grid = &(mGridDatabase[getNeighborDetails(index, 0)]);
         if(grid->mNavFlags[0] == bitType)
            bits.set(BIT(0));
         if(grid->mNavFlags[1] == bitType)
            bits.set(BIT(1));
      }
   }
   else if(!isBottomRow)
   {
      // these are considered obstructed triangles
      // since we are sitting on the left edge
      if(bitType == GridDetail::obstructed)
         bits.set(BIT(0)|BIT(1)|BIT(5)|BIT(4));
      grid = &(mGridDatabase[getNeighborDetails(index, 1)]);
      if(grid->mNavFlags[0] == bitType)
         bits.set(BIT(2));
      if(grid->mNavFlags[1] == bitType)
         bits.set(BIT(3));
      
      if(g.mNavFlags[0] == bitType)
         bits.set(BIT(6));
      if(g.mNavFlags[1] == bitType)
         bits.set(BIT(7));
         
   }
   else if(!isLeftEdge)
   {
      // these are considered obstructed triangles
      // since we are sitting on the bottom row
      if(bitType == GridDetail::obstructed)
         bits.set(BIT(0)|BIT(1)|BIT(2)|BIT(3));
      if(g.mNavFlags[0] == bitType)
         bits.set(BIT(6));
      if(g.mNavFlags[1] == bitType)
         bits.set(BIT(7));
      
      grid = &(mGridDatabase[getNeighborDetails(index, 2)]);
      if(grid->mNavFlags[0] == bitType)
         bits.set(BIT(4));
      if(grid->mNavFlags[1] == bitType)
         bits.set(BIT(5));
   }
   else
   {
      // these are considered obstructed triangles
      // since we are sitting at the origin
      if (bitType == GridDetail::obstructed)
         bits.set(BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5));
         
      // we are at the origin
      if(g.mNavFlags[0] == bitType)
         bits.set(BIT(6));
      if(g.mNavFlags[1] == bitType)
         bits.set(BIT(7));
   }
}

//----------------------------------------------------------------------------

// static Point3F breakPt(160, -272, 0);
static void doBreak() {}

//----------------------------------------------------------------------------

F32 GroundPlan::lowestShadowHt(const GridDetail &g, BitSet32 &bits, S32 index)
{
   F32   minList[8];
   S32   N = 0;
   
   // if (within_2D(g.mWorld[0][0], breakPt, 1.0))
   //    doBreak();
   
   GridDetail * grid = &(mGridDatabase[getNeighborDetails(index, 0)]);
   if(bits.test(BIT(0)))
      minList[N++] = grid->mShadowHts[0];
   if(bits.test(BIT(1)))
      minList[N++] = grid->mShadowHts[1];

   grid = &(mGridDatabase[getNeighborDetails(index, 1)]);
   if(bits.test(BIT(2)))
      minList[N++] = grid->mShadowHts[0];
   if(bits.test(BIT(3)))
      minList[N++] = grid->mShadowHts[1];

   grid = &(mGridDatabase[getNeighborDetails(index, 2)]);
   if(bits.test(BIT(4)))
      minList[N++] = grid->mShadowHts[0];
   if(bits.test(BIT(5)))
      minList[N++] = grid->mShadowHts[1];

   if(bits.test(BIT(6)))
      minList[N++] = g.mShadowHts[0];
   if(bits.test(BIT(7)))
      minList[N++] = g.mShadowHts[1];
      
   // Not sure if this is the perfect fix - only assume there is any height to this if 
   // any of the shadow heights were checked.  Is small value for N==0 small enough?
   F32   lowHt;
   if (N)
   {
      lowHt = 10000;
      while(N--)
         lowHt = getMin(lowHt, minList[N]);
   }
   else
   {
      doBreak();
      lowHt = 2.0;
   }
      
   return lowHt;
}

//----------------------------------------------------------------------------

static void findWaterCallback(SceneObject *obj, S32 val)
{  
   Vector<SceneObject*> *list = (Vector<SceneObject *> *)val;
   list->push_back(obj);
}

//----------------------------------------------------------------------------

bool GroundPlan::missionHasWater()
{
   Vector<SceneObject *> objects;
   gServerContainer.findObjects(WaterObjectType, findWaterCallback, (S32)&objects);
   return objects.size();
}

//----------------------------------------------------------------------------

void GroundPlan::getAllWaterBlocks(Vector<WaterBlock *> &blocks)
{
   Vector<SceneObject *> objects;
   gServerContainer.findObjects(WaterObjectType, findWaterCallback, (S32)&objects);
   blocks.setSize(objects.size());
   
   U32 i;
   for(i = 0; i < objects.size(); i++)
      blocks[i] = dynamic_cast<WaterBlock*>(objects[i]);
}

//----------------------------------------------------------------

bool GroundPlan::setTerrainGraphInfo(TerrainGraphInfo* info)
{
   Point2I  dims(getGridDimWidth(), getGridDimHeight());
   
   info->originGrid = getOrigin();
   info->gridTopRight = info->originGrid;
   info->gridDimensions = dims;
   info->gridTopRight += info->gridDimensions;
   info->gridTopRight += TerrainGraphInfo::gridOffs[GridBottomLeft];

   // Query the ground plan database for neighbors and such:
   info->numShadows  = getNavigable(info->navigableFlags, info->shadowHeights);
   info->nodeCount = getNeighbors(info->neighborFlags).size();
   
   AssertFatal(info->nodeCount == dims.x * dims.y, "Weird Ground Plan Information");
 
   // This gets set up with a separate call (since it takes a while to compute).    
   setSizeAndClear(info->roamRadii, info->nodeCount);

   // Set up the world origin for translations.  
   TerrainBlock * block = getTerrainObj();
   AssertFatal(block, "NavGraph setGround() can't find a terrain block....");
   block->getTransform().getColumn(3, &info->originWorld);
   AssertFatal(info->originWorld.z == 0.0f, "Graph expects terrain origin z == 0");

   return true;
}

//----------------------------------------------------------------------------

#define  AlignmentMask     31

// Round out the area to power of two boundary (AlignmentMask + 1), taking 
// into the account the terrain block origin.  
GridArea GroundPlan::alignTheArea(const GridArea& areaIn)
{
   Point3F  offsetF;
   TerrainBlock * block = getTerrainObj();
   block->getTransform().getColumn(3, &offsetF);
   Point2I  offset(mFloor(offsetF.x), mFloor(offsetF.y));
   
   // Figure how much needs to be added adjusted area to get it aligned nice,
   // then we'll add that to the original one - i.e. unadjust.  
   Point2I     adjPointBy(areaIn.point);
   adjPointBy -= offset;
   adjPointBy.x &= AlignmentMask;
   adjPointBy.y &= AlignmentMask;

   // Add to extent to account for moving point back, then round up.  
   Point2I     newExtent(areaIn.extent);
   newExtent += adjPointBy;
   newExtent.x += AlignmentMask;
   newExtent.y += AlignmentMask;
   newExtent.x &= (~AlignmentMask);
   newExtent.y &= (~AlignmentMask);
   
   // Assemble the area we want- 
   GridArea    newArea(areaIn);
   newArea.point -= adjPointBy;
   newArea.extent = newExtent;
   
   return newArea;
}

//----------------------------------------------------------------------------

void GroundPlan::genExterior(Point2I &min, Point2I &max, NavigationGraph *nav)
{
   GridArea area;
   
   // Persist fields can specify custom area, else use passed in mission area.
   if (!nav->customArea(area))
   {
      area.point = min;
      area.extent = max;
   }
   area = alignTheArea(area);
   inspect(area);
}         

//----------------------------------------------------------------------------

// Inspect the terrain if it is present, return bool indicating if so.
static bool cInspect(SimObject *obj, S32, const char **argv)
{
   GroundPlan *plan = static_cast<GroundPlan*>(obj);
   NavigationGraph *nav = static_cast<NavigationGraph*>(Sim::findObject("NavGraph"));
   if(plan && nav && nav->haveTerrain())
   {   
      Point2I point, extent;
      dSscanf(argv[2], "%d %d", &point.x, &point.y);
      dSscanf(argv[3], "%d %d", &extent.x, &extent.y);
      plan->genExterior(point, extent, nav);
      nav->setGround(plan);
      return true;
   }
   return false;
}

//----------------------------------------------------------------------------

void GroundPlan::consoleInit()
{
   Parent::consoleInit();
   Con::linkNamespaces("SimObject", "GroundPlan");
   Con::addCommand("GroundPlan", "inspect", cInspect, "gp.inspect(min, max);", 4, 4);
}

