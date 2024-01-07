//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHDATA_H_
#define _GRAPHDATA_H_

#ifndef _GRAPHMATH_H_
#include "ai/graphMath.h"
#endif
#ifndef _GRAPHDEFINES_H_
#include "ai/graphDefines.h"
#endif
#ifndef _GRAPHBSP_H_
#include "ai/graphBSP.h"
#endif

#ifndef _BITVECTORW_H_
#include "Core/bitVectorW.h"
#endif
#ifndef _BITVECTOR_H_
#include "Core/bitVector.h"
#endif
#ifndef _BITSET_H_
#include "Core/bitSet.h"
#endif
#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif

enum NavGraphEnum
{  // node types:
   NavGraphNotANode = 0,
   NavGraphOutdoorNode,
   NavGraphIndoorNode, 
   NavGraphSubmergedNode, 
   // misc constants:
   MaxOnDemandEdges = 13,     // for nodes that create edges on demand (ie. grid nodes)
};

// This will be the next version of the of the edge data.  
struct GraphEdgeInfo
{
   enum
   {
      Jetting     =  BIT(0), 
      Algorithmic =  BIT(16), 
      Inventory   =  BIT(18)
   };
   struct OneWay{
      BitSet32       flags;
      S32            res; 
      S32            dest;
      OneWay()       {dest=res=-1;}
      bool           isJetting() const    {return flags.test(Jetting);}
   } to[2];
   
   GraphEdgeInfo();

   bool  isAlgorithmic() const;        void  setAlgorithmic(bool YN=true);
   bool  isInventory() const;          void  setInventory(bool YN = true);
   
   // These three points define the segment between the two nodes.  The segPoints
   // are the two points, and the normal points in direction from 1st to 2nd node.
   Point3F  segPoints[2];
   Point3F  segNormal;
   
   bool  read(Stream & s);
   bool  write(Stream & s) const;
};

struct IndoorNodeInfo
{
   enum
   { 
      Algorithmic    = BIT(1), 
      WallNode       = BIT(2), 
      Inventory      = BIT(4), 
      BelowPortal    = BIT(5),
      Seed           = BIT(6)
   };

   BitSet32          flags;
   U16               unused;
   S16               antecedent;
   Point3F           pos;
   
   IndoorNodeInfo();
   
   // Access to booleans- 
   bool  isAlgorithmic() const;        void  setAlgorithmic(bool YN = true);
   bool  isWallNode() const;           void  setWallNode(bool YN = true);
   bool  isInventory() const;          void  setInventory(bool YN = true);
   bool  isBelowPortal() const;        void  setBelowPortal(bool YN = true);
   bool  isSeed() const;               void  setSeed(bool YN = true);
   
   bool  read(Stream & s);
   bool  write(Stream & s) const;
};

// Persisted data for the terrain nodes.  
struct OutdoorNodeInfo 
{  
   U8    flags;
   S8    level;
   U16   height;     // Probably can be taken out if we do a big graph re-versioning.  
   S16   x, y;
   
   OutdoorNodeInfo();
   bool  read(Stream & s);
   bool  write(Stream & s) const;
   
   Point2I  getPoint()  const    {return Point2I(x,y);}
   S8       getLevel()  const    {return level;}
};

typedef  Vector<IndoorNodeInfo>  NodeInfoList;
typedef  Vector<GraphEdgeInfo>   EdgeInfoList;

class Consolidated : public Vector<OutdoorNodeInfo> 
{ 
   public: 
      bool  read(Stream& s)         {return readVector1(s, *this);}
      bool  write(Stream& s) const  {return writeVector1(s, *this);}
}; 

//-------------------------------------------------------------------------------------

struct SpawnLocations : Vector<Point3F> 
{
   struct Sphere 
   {
      SphereF  mSphere;
      bool     mInside;
      S32      mCount;
      S32      mOffset;
      S32      mRes0;
      
      Sphere(const Point3F& center, F32 radius);
      bool     read(Stream& s);
      bool     write(Stream& s) const;
   };
   typedef Vector<Sphere>  Spheres;
   
   S32      mRes0;
   Spheres  mSpheres;
   
   SpawnLocations();

   S32   getRandom(const SphereF&, bool inside, U32 rnd);
   void  printInfo() const;
   void  reset();
   bool  read(Stream& s);
   bool  write(Stream& s) const;
};

//-------------------------------------------------------------------------------------

// Learn information for potential bridges.  
//    This is old format now-  See *BridgeData* below
struct GraphBridgeInfo
{
   enum How {CanWalk=1, MustJet=2};       // ==> Would like to do this:  CanFall=4};  !
   S32   dstNode;
   S32   srcNode;
   F32   jetClear;
   U8    howTo;
   U8    res1, res2, res3;
   
   GraphBridgeInfo(S32 src, S32 dst)      {init(src, dst);}
   GraphBridgeInfo()                      {init(-1, -1);}
   
   void  init(S32 src, S32 dst);
   bool  read(Stream& s);
   bool  write(Stream& s) const;
   
   void  setWalkable()        {howTo=CanWalk;}
   bool  isWalkable()         {return (howTo & CanWalk);}
   bool  mustJet()            {return !isWalkable();}
};

class BridgeInfoList : public Vector<GraphBridgeInfo> 
{
   public: 
      bool  read(Stream& s)         {return readVector1(s, *this);}
      bool  write(Stream& s) const  {return writeVector1(s, *this);}
};

//-------------------------------------------------------------------------------------

inline U8 mapJetHopToU8(F32 hop) {
   return (U8(mFloor((hop + 0.124) * 8.0)));
}

inline F32 mapU8ToJetHop(U8 amt) {
   return (F32(amt) * 0.125);
}

struct GraphBridgeData
{
   // ==> Would like to have a CanFall field too....  
   enum How {CanWalk=1, MustJet=2, Replacement=4, Unreachable=8};
   U16   nodes[2];
   U8    jetClear;
   U8    howTo;
   
   GraphBridgeData(U16 src, U16 dst)      {init(src, dst);}
   GraphBridgeData()                      {init(-1, -1);}
   
   void  init(U16 n0, U16 n1);
   bool  read(Stream& s);
   bool  write(Stream& s) const;
   
   void  setWalkable()        {howTo |= CanWalk;}
   void  setReplacement()     {howTo |= Replacement;}
   void  setUnreachable()     {howTo |= Unreachable;}
   void  setHop(F32 dist)     {jetClear = mapJetHopToU8(dist);}
   bool  isWalkable()         {return (howTo & CanWalk);}
   bool  isReplacement()      {return (howTo & Replacement);}
   bool  isUnreachable()      {return (howTo & Unreachable);}
   bool  mustJet()            {return !isWalkable();}
};

class BridgeDataList : public Vector<GraphBridgeData> 
{
      S32   mReplaced[2];
      S32   mSaveTotal;
   public: 
      BridgeDataList();
      S32   replaced() const;
      S32   numPositiveBridges() const;
      S32   accumEdgeCounts(U16 * edgeCounts);
      bool  readOld(Stream& s);
      bool  read(Stream& s);
      bool  write(Stream& s) const;
};

//-------------------------------------------------------------------------------------
//          See graphVolume.cc

struct GraphVolume
{
   PlaneF            mFloor;
   PlaneF            mCeiling;
   const PlaneF *    mPlanes;
   S32               mCount;
   Vector<Point3F>   mCorners;
};

struct GraphVolInfo
{
   S32   mPlaneCount;
   S32   mPlaneIndex;
   bool  read(Stream& s)         {return s.read(&mPlaneCount) && s.read(&mPlaneIndex);}
   bool  write(Stream& s) const  {return s.write(mPlaneCount) && s.write(mPlaneIndex);}
};

class GraphVolumeList : public Vector<GraphVolInfo>
{
      bool     intersectWalls(S32 i, const PlaneF& with, Vector<Point3F>& list) const;
      PlaneF*  getPlaneList(S32 i)  {return &mPlanes[(*this)[i].mPlaneIndex];}

  public:
      Vector<PlaneF>    mPlanes;
      
      const PlaneF* planeArray(S32 i) const  {return &mPlanes[(*this)[i].mPlaneIndex];}
      S32      planeCount(S32 i) const       {return this->operator[](i).mPlaneCount;}
      PlaneF   floorPlane(S32 i) const       {return planeArray(i)[planeCount(i)-2];}
      PlaneF   abovePlane(S32 i) const       {return planeArray(i)[planeCount(i)-1];}
      S32      memSize() const               {return (Vector<GraphVolInfo>::memSize() + mPlanes.memSize());}
      F32      getMinExt(S32 i, Vector<Point3F>& ptBuffer);
      bool     closestPoint(S32 ind, const Point3F& point, Point3F& soln) const;
      S32      getCorners(S32 i, Vector<Point3F>& pts, bool noTop=false) const;
      void     addVolume(Point3F pos, F32 boxw, F32 boxh);
      void     cullUnusedPlanes();
      void     nudgeVolumesOut();
      
      bool     read(Stream& s);
      bool     write(Stream& s) const;
};

//-------------------------------------------------------------------------------------

struct ChuteHint : public Point3F
{
   bool  read(Stream& s)         {return s.read(&x) && s.read(&y) && s.read(&z);}
   bool  write(Stream& s) const  {return s.write(x) && s.write(y) && s.write(z);}
   const Point3F& location()     {return *this;}      // for BSP-er
};

typedef AxisAlignedBSP<ChuteHint>   ChuteBSP;
typedef VectorPtr<ChuteHint*>       ChutePtrList;

class ChuteHints : public Vector<ChuteHint>
{
      ChuteBSP mBSP;
      ChutePtrList   mQuery;
      void     makeBSP();
  public:
      enum Info {NotAChute, ChuteTop, ChuteMid};
      S32      findNear(const Point3F& P, S32 xy, S32 z, ChutePtrList& list) const;
      S32      findNear(const Box3F& box, ChutePtrList& list) const;
      void     init(const Vector<Point3F>& list);
      Info     info(Point3F bot, const Point3F& top);
      bool     read(Stream& s);
      bool     write(Stream& s) const;
};

//-------------------------------------------------------------------------------------

struct PathXRefEntry : public BitVectorW 
{
   bool  read(Stream& s);
   bool  write(Stream& s) const;
};

class PathXRefTable : public Vector<PathXRefEntry>
{
   public: 
      ~PathXRefTable();
      bool  read(Stream& s);
      bool  write(Stream& s) const;
      void  setSize(S32 size);
      void  clear();
};

//-------------------------------------------------------------------------------------

class LOSTable
{
   public: 
      enum TwoBitCodes {Hidden, MinorLOS, MuzzleLOS, FullLOS};
      virtual U32  value(S32 ind1, S32 ind2) const = 0;
      virtual bool valid(S32 numNodes) const = 0;
      virtual bool write(Stream& s) const = 0;
      virtual bool read(Stream& s) = 0;
      virtual void clear() = 0;
      
      bool  hidden(S32 i1, S32 i2) const       { return (value(i1, i2) == Hidden);    }
      bool  fullLOS(S32 i1, S32 i2) const      { return (value(i1, i2) == FullLOS);   }
      bool  muzzleLOS(S32 i1, S32 i2) const    { return (value(i1, i2) >= MuzzleLOS); }
};

//-------------------------------------------------------------------------------------

class LOSXRefTable : public BitVectorW, public LOSTable
{
   public: 
      void  setEntry(S32 ind1, S32 ind2, U32 val);
      U32   value(S32 ind1, S32 ind2) const;
      bool  valid(S32 numNodes) const;
      void  clear();
      bool  read(Stream& s);
      bool  write(Stream& s) const;
};

//-------------------------------------------------------------------------------------

class LOSHashTable : public LOSTable
{
   public:
      enum{ SegShift = 6,                    // seems to be the magic #
            SegSize = (1 << SegShift), 
            SegMask = (SegSize - 1), 
            SegAlloc = (1 << SegShift - 2)
         };
         
   protected:
      union Key {
         struct {
            U16   mNode, mSeg;
         } mKey;
         U32   mCompare;
         Key(U16 node = 0, U16 seg = 0)   {mKey.mNode = node; mKey.mSeg = seg;}
      };
      
      struct Segment {
         U8    mLOS[SegAlloc];
         Key   mKey;
         Segment(Key key, const U32 losInfo[SegSize]);
         U32   value(U32 i)  const {return 3 & mLOS[i >> 2] >> ((i & 3) << 1);}
         bool  read(Stream& s);
         bool  write(Stream& s) const;
      };
      
      typedef  Vector<Segment>   SegmentList;
      typedef  U16               IndexType;

   protected:
      static U32     mTabSz;
      IndexType   *  mTable;
      SegmentList    mSegments;
      U32            mNumNodes;
      U32            mRes0, mRes1;
      
   protected:
      static U32 calcHash(Key key);
      static S32 QSORT_CALLBACK cmpHashes(const void* ,const void* );
      U32   calcTabSize();
      void  sortByHashVal();
      U32   makeTheTable();
      
   public:
      LOSHashTable();
      ~LOSHashTable();
      U32   convertTable(const LOSXRefTable& from, S32 numNodes);
      U32   value(S32 ind1, S32 ind2) const;
      bool  valid(S32 numNodes) const;
      bool  write(Stream& s) const;
      bool  read(Stream& s);
      void  clear();
};

//-------------------------------------------------------------------------------------
// Data saved on graph object in MIS file- 

// Configures how nodes are consolidated.  
struct ConjoinConfig 
{
   ConjoinConfig();
   F32   maxAngleDev;
   F32   maxBowlDev;
   S32   maxLevel;
};

//-------------------------------------------------------------------------------------
// Consolidation data structures.  Used in computing, not persisted.  

struct GridNormalInfo
{
   bool     notEmpty;
   bool     hasSteep;
   VectorF  normals[2];
   Point2F  angles[2];
   GridNormalInfo();
   static Point2F normalToAngle(const VectorF& N);
   static VectorF angleToNormal(const Point2F& A);
};

#define  MaxConsolidateLevel     6 

class TrackLevels 
{
      Vector<U16>    achievedLevels;
      Vector<U8>     nodeTypes;
   public:
      TrackLevels(S32 sz);
      void  setAchievedLevel(S32 index, S32 level);
      S32   getAchievedLevel(S32 index) const;
      S32   originalNodeLevel(S32 idx) const;
      U16   getNodeType(S32 index) const;
      void  setNodeType(S32 index, U8 nodeType);
      S32   size() const;
      void  capLevelAt(S32 idx, U16 lev);
      void  init(S32 size);
};

//-------------------------------------------------------------------------------------

struct TerrainGraphInfo
{
   static const Point2I gridOffs[8];
   static S32           smVersion;
   
   // false until data is set or loaded:
   bool           haveGraph;

   // persisted data:
   S32            nodeCount;
   S32            numShadows;
   Point3F        originWorld;
   Point2I        originGrid;
   Point2I        gridDimensions;
   Point2I        gridTopRight;
   Vector<U8>     navigableFlags;
   Vector<U8>     neighborFlags;
   Vector<F32>    shadowHeights;
   Vector<U16>    roamRadii;
   Consolidated   consolidated;
   
   // Data computed from the above at load/initialize:
   LineSegment    boundarySegs[4];
   S32            indOffs[8];
   GridArea       gridArea;

   TerrainGraphInfo();

   // small utility functions. 
   Point2I&    indexToPos(S32 index) const; 
   Point3F*    indexToLoc(Point3F& locOut, S32 index);
   bool        posToLoc(Point3F& locOut, const Point2I& p);
   S32         posToIndex(Point2I pos) const;
   S32         locToIndex(Point3F loc) const;
   S32         locToIndexAndSphere(SphereF& s, const Point3F& L); 
   bool        obstructed(const Point3F& L) const;
   U8          squareType(S32 n)  const   {return (navigableFlags[n] & 7);}
   bool        obstructed(S32 n)  const   {return squareType(n)==GraphNodeObstructed;}
   bool        shadowed(S32 n)  const     {return squareType(n)==GraphNodeShadowed;}
   bool        submerged(S32 n)  const    {return squareType(n)==GraphNodeSubmerged;}
   bool        haveConsData()  const      {return consolidated.size() > 0;}
   F32         shadowHeight(S32 n)  const {return shadowHeights[n];}
   bool        inGraphArea(const Point3F& loc) const {return locToIndex(loc) >= 0;}
   bool        steep(S32 n) const         {return navigableFlags[n] & GroundNodeSteep;}
   void        setSteep(S32 n)            {navigableFlags[n] |= GroundNodeSteep;}

   // not-as-small utility functions
   // S32         needFloorPlanMojo(Vector<Point2I>& listOut, TerrainBlock* terr);
   Point3F     whereToInbound(const Point3F& loc);
   F32         checkOpenTerrain(const Point3F& from, Point3F& to);
   U32         onSideOfArea(S32 index);

   // persist
   bool        read(Stream & s);
   bool        write(Stream & s) const;
   
   // Calculation stuff (consolidation), precomputes, setup.. 
   bool  buildConsolidated(const TrackLevels & trackInf, const ConjoinConfig & config);
   bool  consolidateData(const ConjoinConfig & config);
   S32   markNodesNearSteep();
   void  computeRoamRadii();
   void  getGridNormals(Vector<GridNormalInfo>&);
   void  setSideSegs();
   
   // After new data is created or loaded, this doest last needed calcs, fixups, etc.
   void  doFinalDataSetup();  
};

#endif
