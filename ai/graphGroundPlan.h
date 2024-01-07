//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHGROUNDPLAN_H_
#define _GRAPHGROUNDPLAN_H_

#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif
#ifndef _CLIPPEDPOLYLIST_H_
#include "Collision/clippedPolyList.h"
#endif
#ifndef _GRAPHGENUTILS_H_
#include "ai/graphGenUtils.h"
#endif

class WaterBlock;
class NavigationGraph;

//---------------------------------------------------------------

class GroundPlan: public SimObject          
{
   friend class InspectionVisitor;
   
   // members
   typedef SimObject 	      Parent;
   TerrainBlock 		         *mTerrainBlock;
   Vector<GridDetail>         mGridDatabase;
   Point2I                    mGridOrigin;
   Point2I                    mGridDims;
   S32                        mTotalVisited;
   
   // static members
   static Point2I gridOffset[8];
   static bool renderMe;
   static bool drawclipped;
   
   // constructors/destructors
   public:
      GroundPlan();
      ~GroundPlan();
      DECLARE_CONOBJECT(GroundPlan);
      static void consoleInit();
      static void initPersistFields();

   // specific to SceneObject
   protected:
      bool 	   onAdd();
      void 		onRemove();
      void     drawObstruct(GridDetail &grid, S32 index);
      void     drawShadow(GridDetail &grid, S32 index);
      void     drawNeighbor(GridDetail &grid, S32 i);
      void     drawConnection(Point3F &st, Point3F &ed);
      
   // terrain block details
   protected:    
      void 		gridToWorld(const Point2I &gPos, Point3F &wPos);
	   void 		worldToGrid(const Point3F &wPos, Point2I &gPos);
	   void 		gridToCenter(const Point2I &gPos, Point2I &cPos);
      F32      getGridHeight(const Point2I &gPos);
      S32      gridToIndex(const Point2I &gPos);
      void     getClippedRegion(S32 index, S32 &st, S32 &ed, S32 &xspan, S32 &yspan);
      bool     isSplit45(const Point2I &gPos);
      bool     isOnBoundary(S32 i);
      bool     missionHasWater();
      void     getAllWaterBlocks(Vector<WaterBlock*> &blocks);
  
   // inspection details
   private:
      void        inspectSquare(const GridArea &gridArea, GridDetail &square);
      S32         getNeighborDetails(S32 index, S32 key);
      void        packTriangleBits(const GridDetail &g, BitSet32 &bits, S32 bitType, S32 index, bool isLeftEdge, bool isBottomRow);
      F32         lowestShadowHt(const GridDetail &g, BitSet32 &bits, S32 index);   
      GridArea    alignTheArea(const GridArea& areaIn);
      void        findNeighbors();
      void        computeNavFlags();
      bool        setupPolyList(const Box3F& bBox, const Point3F& min, const Point3F& mid,
                     const Point3F& max, const VectorF& norm1, const VectorF& norm2,
                     const VectorF& norm3, F32* height);
      GridDetail  defaultDetail();
   
   // interface
   public:
      Point2I              getOrigin() const;
      Point2I              *getPosition(S32 index);
      bool                 inspect(GridArea &area);
      S32                  getNavigable(Vector<U8> &navFlags, Vector<F32> &shadowHts);
      Vector<U8>           &getNeighbors(Vector<U8> &neighbors); 
      static TerrainBlock  *getTerrainObj();
      S32                  getGridDimWidth() const;
      S32                  getGridDimHeight() const;
      void                 render(Point3F &camPos, bool drawClipped);
      bool                 setTerrainGraphInfo(TerrainGraphInfo* info);
      void                 genExterior(Point2I &min, Point2I &max, NavigationGraph *nav);
};

extern GroundPlan *gGroundPlanTest;

//----------------------------------------------------------------------------

inline S32 GroundPlan::getGridDimWidth() const
{
   return mGridDims.x;
}

//----------------------------------------------------------------------------

inline S32 GroundPlan::getGridDimHeight() const
{
   return mGridDims.y;
}

//----------------------------------------------------------------------------

inline Point2I GroundPlan::getOrigin() const
{
   return mGridOrigin;
}

//----------------------------------------------------------------------------

inline Point2I *GroundPlan::getPosition(S32 index)
{
   static Point2I sPoint;
   sPoint.x = mGridOrigin.x + index % mGridDims.x;
   sPoint.y = mGridOrigin.y + index / mGridDims.x;
   return &sPoint;   
}

//----------------------------------------------------------------------------

inline bool GroundPlan::isSplit45(const Point2I &gPos)
{
   GridSquare *gs = mTerrainBlock->findSquare(0, gPos);
   if(gs->flags & GridSquare::Split45)
      return true;
   else
      return false;
}

//----------------------------------------------------------------------------

inline S32 GroundPlan::getNeighborDetails(S32 index, S32 key)
{
   if(key < 0 || key > 7)
      return index;

   switch(key)
   {
      case 0:
         index = (index - mGridDims.x) - 1;
         break;
      case 1:   
         index -= mGridDims.x;
         break;
      case 2:   
         index--;
         break;
      case 3:
         index = (index - mGridDims.x) + 1;
         break;
      case 4:   
         index = (index + mGridDims.x) - 1;
         break;
      case 5:   
         index++;
         break;
      case 6:   
         index = index + mGridDims.x;
         break;
      case 7:   
         index = (index + mGridDims.x) + 1;
         break;
   }
   
   AssertFatal((index <= mGridDatabase.size() - 1) || (index > 0), "GroundPlan::getNeighborDetails() - index out of bounds!");
   return index;
}

#endif
