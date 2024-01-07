//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHFLOORPLAN_H_
#define _GRAPHFLOORPLAN_H_

#define TESTSPECIAL 1
#define ITERATEMODE 0 
   
// ************************************************************************************
#define DEBUGMODE 0 // Only use this if you want to debug ONE interior. More than
                    // one interior in a mission will crash with this defined to 1
                    // Also, this doesn't support graph builds. Use the build function 
                    // called fpStart() and fpend() to generate nodes for the interior
                    // you want to debug. These functions are defined in navGraph.cs
// ************************************************************************************

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _INTERIOR_H_
#include "interior/interior.h"
#endif
#ifndef _INTERIORINSTANCE_H_
#include "interior/interiorInstance.h"
#endif
#ifndef _GRAPH_H_
#include "ai/graph.h"
#endif
#ifndef _GRAPHMATH_H_
#include "ai/graphMath.h"
#endif
#ifndef _BITSET_H_
#include "core/bitSet.h"
#endif
#ifndef _GRAPHGENUTILS_H_
#include "ai/graphGenUtils.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
#include "ts/tsShapeInstance.h"
#endif
#ifndef _SHAPEBASE_H_
#include "game/shapeBase.h"
#endif

//----------------------------------------------------------------

class FloorPlan: public SimObject          
{
   private:
   
   // members
   typedef SimObject       Parent;
   GraphDetails            *mGraphDetails;
   Interior                *currInterior;
   InteriorInstance        *currInstance;
   F32                     mSlopeThresh;
   U32                     mNumInteriors;
   U32                     mExtraInteriors;
   bool                    mDataCollected;
   U8                      mDivLevel;
   bool                    newLog;
   S32                     numZones;
   
   struct surfCollisionInfo
   {
      bool     specialCase;
      S32      divideEdge1;
      S32      divideEdge2;
      F32      factor;
      
      surfCollisionInfo() { specialCase = false; divideEdge1 = divideEdge2 = -1; factor = 0.f; }
   };
   
#if DEBUGMODE
   
   InteriorDetail          detail;
      
#endif
   
   // static members
   static Point3F          sDebugBreakPoint;
   static F32              sFloorThresh;
   static F32              sMinDivRadius;
   static F32              sMaxDivRadius;
   static bool             sSubDivide;
   static bool             sDrawConnections;
   static bool             sDrawVolumeHts;
   static F32              sHeightDiff; 
   static F32              sSpecialMaxRadius; 
   static bool             sUseSpecial;
   static S32              amountToDivide;
   static bool             sDisableAsserts;
   static bool             LogMode;
   
   public:
      Vector<ShapeBase *>  mStaticShapeCenterList;
      Vector<ShapeBase *>  mStaticShapeGeometryList;
      Vector<Point3F>      portalPoints;
   
   // constructors/destructors
   public:
      FloorPlan();
      ~FloorPlan();
      DECLARE_CONOBJECT(FloorPlan);
      static void consoleInit();
      static void initPersistFields();
      static void setBreakPoint(const Point3F& pt)    {sDebugBreakPoint = pt;}

   // public interface
   public:
      void     render();
      void     generate();
      void     upload2NavGraph(NavigationGraph *ng);

   // simbase methods
   protected:
      bool     onAdd();
      void     onRemove();
      
   // interior extraction methods
   private:   
      void     snoopInteriors();
      void     extractData(Interior *interior, InteriorDetail *detail, const MatrixF &transform);
      void     buildEdgeTable(InteriorDetail *detail);
      void     buildStaticShapeGeometryNodes();
      void     buildStaticShapeCenterNodes();
      void     buildNodesFromPortals(InteriorDetail *d);
      void     processShape(ShapeBase **s, bool center);
      void     graphExtraction(InteriorDetail *d, GraphDetails *g);
      
   // subdivision methods
   private:
      U32      subdivideSurfaces(InteriorDetail *d);
      void     splitSurface(InteriorDetail *d, DSurf &surf, U32 surfIndex, surfCollisionInfo &info);
      void     subdivideSurface(InteriorDetail *d, DSurf &surf, U32 surfIdx);
      void     subdivideEdge(InteriorDetail *d, DEdge *edge, F32 factor);
      void     createNewSurfaces(InteriorDetail *d, U32 surfIdx, DEdge **edges, BitSet32 &edgeTracker);
      void     newSurfacesSpecialCase(InteriorDetail *d, U32 surfIdx, BitSet32 &edgeTracker);
      bool     shouldDivideEdge(DSurf &surf, DEdge *edge);
      bool     shouldDivideSurf(DSurf &surf);
      void     createCenterNode(InteriorDetail *d, DSurf *surf, DPoint *CM);
      void     generateNewEdges(InteriorDetail *d, U32 cenIdx, DSurf &surf, DEdge **edges, BitSet32 &edgeTracker);
   
   // polylist collision methods   
   private:   
      bool     setupPolyList(InteriorDetail *d, DSurf &surf, F32 *ht, DVolume *vol, surfCollisionInfo &info);
      Point3F  getMinBoxPoint(InteriorDetail *d, U8 n_edges, DEdge **edges);
      Point3F  getMaxBoxPoint(InteriorDetail *d, U8 n_edges, DEdge **edges);
      void     buildEdgeNormals(InteriorDetail *d, DSurf &surf, VectorF *normalList);
      bool     obstructedSurf(InteriorDetail *d, DSurf &surf, DVolume *vol, surfCollisionInfo &info);
      
   // internal helpers
   private:
      void     setToWallNodes(InteriorDetail *d, DSurf &surf);
      void     findLongestEdge(DSurf *surf);
      bool     validatePoint(DPoint &p);
      void     computeSurfaceRadius(InteriorDetail *d, DSurf *surf);
      F32      maxZforSurf(InteriorDetail *d, DSurf &surf);
      void     newConnection(InteriorDetail *d, U32 start, U32 end, DConnection &con);
      bool     isSharedEdge(InteriorDetail *d, DEdge *edge_1, DEdge *edge_2, DConnection &con);
      bool     passDistanceCheck(DSurf &surf_1, DSurf &surf_2);
      void     buildConnections(InteriorDetail *d);
      void     findDuplicateCons(InteriorDetail *d);
      bool     isUsableInterface(DConnection &con);
      bool     offHeightDistribution(InteriorDetail *d, DSurf &surf);
      bool     htsDifferAlot(F32 *hts, U8 n);
      void     createInventoryVol(DPoint p, DVolume &vol);
      void     setCollisionInfoData(surfCollisionInfo &info, DSide *side, DSurf &surf);
      bool     isQuadSurface(InteriorDetail *d, DSurf &surf);
      bool     haveGoodData(DSide *sides);
      bool     isSteppable(InteriorDetail *d, DEdge *big, DEdge *small);
      bool     specSurfTest(DSurf &surf, ClippedPolyList &list, Box3F &b);
      void     log(const char *string);
      void     extractPortals(Interior *i, InteriorDetail *d, const MatrixF &transform);
      bool     buildPortalCenter(InteriorDetail *d, Interior *interior, Interior::Portal *sourcePortal, DPortal *portal);
      bool     pointsAreEqual(Point3F &a, Point3F &b);
      bool     inSolid(DPoint &center);
      void     buildZoneInfo(InteriorDetail *d);
      void     sortSurfaceList(InteriorDetail *d);                                       
   
   // rendering methods
   private:   
      void     drawEdge(Point3F, Point3F);
      void     drawSurface(InteriorDetail *d, DSurf &surf);
      void     drawNode(DPoint pos);
      void     renderEdgeTable(InteriorDetail *d);
      void     drawPoly(U8 n, Point3F *pts, bool unObstruct, bool special, S32 zone);
      void     drawConnections(InteriorDetail *d);
      void     drawVolumes(InteriorDetail *d);
      void     renderPolyPoints(InteriorDetail *d);
      void     renderSpecialNodes(InteriorDetail *d);
      void     drawInterfaces(InteriorDetail *d);
};

extern FloorPlan *gFloorPlan;       // just for testing

// inlines
//-------------------------------------------------------------------------

inline bool FloorPlan::shouldDivideEdge(DSurf &surf, DEdge *edge)
{
   return ((edge->length / surf.mLongest->length) > 0.4);     
}

//-------------------------------------------------------------------------

inline bool FloorPlan::validatePoint(DPoint &p)
{
   return (p.x == p.x) && (p.y == p.y) && (p.z == p.z);
}

//-------------------------------------------------------------------------

inline bool FloorPlan::isUsableInterface(DConnection &con)
{
   F32 length = (con.segNodes[1] - con.segNodes[0]).len();
   // return (length > 0.8);
   return (length > 1.2);
}

//-------------------------------------------------------------------------

#endif



