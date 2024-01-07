//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHGENUTILS_H_
#define _GRAPHGENUTILS_H_

#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _BITSET_H_
#include "Core/bitSet.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MPLANE_H_
#include "Math/mPlane.h"
#endif
#ifndef _MBOX_H_
#include "Math/mBox.h"
#endif
#ifndef _GRAPHDATA_H_
#include "ai/graphData.h"
#endif

//----------------------------------------------------------------

class DEdge
{
   public:
      enum
      {
         divGenerated   = BIT(0),
         wall           = BIT(1),
         divided        = BIT(2),
         drawn          = BIT(3),
         traversable    = BIT(4), 
      };
   
   public:
      U32         start, end, midPoint;
      DEdge       *right, *left;
      DEdge       *next;
      BitSet32    flags;
      F32         length;

   public:
      bool        operator==(const DEdge &edge);
};

inline bool DEdge::operator==(const DEdge &edge)
{
   return (((start == edge.start) && (end == edge.end)) || 
               ((end == edge.start) && (start == edge.end)));
}           

//----------------------------------------------------------------
// hashtable to make graph edge building very quick.
class EdgeTable
{
   public:
      enum
      {
         DefaultTableSize = 32,
      };
      
   private:   
      DEdge        **hashTable;
      U32          hashTableSize;
      U32          hashEntryCount;

   public:
      EdgeTable();
      ~EdgeTable();
      
      void        insert(DEdge edge);
      bool        contains(DEdge edge);
      DEdge       *find(DEdge edge);
      DEdge       *getBucket(U32 bucket);
      U32         size() const;
      U32         tableSize() const;
      void        clear();

   private:
      void  init();
      S32   hash(DEdge edge);
};

//----------------------------------------------------------------

inline DEdge *EdgeTable::getBucket(U32 bucket)
{
   return hashTable[bucket];   
}

//----------------------------------------------------------------

inline U32 EdgeTable::size() const
{
   return hashEntryCount;
}

//----------------------------------------------------------------

inline U32 EdgeTable::tableSize() const
{
   return hashTableSize;
}

//----------------------------------------------------------------

class DSpecNode
{
   public:
      enum
      {
         chuteNode = BIT(0),
      };
   
   public:
      BitSet32          flags;
      Point3F           pos;
      StringTableEntry  name;
};

//----------------------------------------------------------------

class DPoint : public Point3F
{
   public:
      enum
      {
         divGenerated   = BIT(0),
         wall           = BIT(1),
         center         = BIT(2),
         inventory      = BIT(3),
      };

   public:
      BitSet32    flags;
      DEdge       *e;
      U32         surfIdx;
      
   public:
      //operator Point3F &(){ return *this; }
};

//----------------------------------------------------------------

class DSurf
{  
   public:     
      enum
      { 
         isWall         = BIT(0),
         shouldBeCulled = BIT(1),
         divided        = BIT(2),
         split          = BIT(3),
         unObstructed   = BIT(4),
         specialSplit   = BIT(5),
         tooSmall       = BIT(6),   
      };
      
      DEdge          **mEdges;
      DEdge          *mLongest;
      BitSet32       mFlags;
      BitSet32       mFlipStates;
      U32            mNumEdges;
      U16            mSubSurfs[32];
      U16            mSubSurfCount;
      VectorF        mNormal;
      S16            mDivLevel;
      F32            mMaxRadius;
      F32            mMinRadius;
      U32            fullWinding[32];
      DSurf          *parent;
      DPoint         mCntr;
      U32            mCtrIdx;
      U32            volIdx;
      S32            mZone;
      
      DSurf();
};                           

//----------------------------------------------------------------

class DConnection
{
   public:
      U32      start;
      U32      end;
      
      // interface points 
      Point3F  segNodes[2];
  
   public:
      bool operator==(const DConnection &con);
};

//----------------------------------------------------------------

inline bool DConnection::operator==(const DConnection &con)
{
   return (((start == con.start) && (end == con.end)) || 
               ((end == con.start) && (start == con.end)));
}           

//----------------------------------------------------------------

class DVolume
{
   public:
      U32      mNumPlanes;
      PlaneF   mPlanes[32];
      U32      surfIdx;
      F32      ht;
      Point3F  capPt;
      DVolume();
      
      bool  checkInside(const Point3F& pt);
};

class DPortal
{
   public:
      Point3F center;   
};

//----------------------------------------------------------------

class InteriorDetail
{
   public:      
      Vector<DSurf>           mSurfaces;
      Vector<DPoint>          mPoints;
      Vector<DConnection>     mConnections;
      Vector<DPoint>          mConPoints;
      Vector<DVolume>         mVolumes;
      Vector<U32>             mTraversable;
      Vector<U32>             mUnobstructed;
      Vector<DSpecNode>       mSpecialNodes;
      Vector<Point3F>         polyTestPoints;
      Vector<DPortal>         mPortals;
      Vector<U32>             sortedSurfList;
      
      //ClippedPolyList         *polyLists;
      EdgeTable               mEdgeTable;
      S16                     mIndex;
      S32                     mNumUnObstructed;
      Box3F                   mBbox;
      S32                     numPolyLists;
      
      InteriorDetail();
      bool                    haveSurfaceNear(const Point3F& pos, F32 rad);
};

class GraphDetails
{
   public:
      EdgeInfoList      edges;
      NodeInfoList      nodes;
      EdgeInfoList      segs;
      GraphVolumeList   volumes;
      Vector<Point3F>   chutes;
   
      GraphDetails() { }
};

//----------------------------------------------------------------

class GridDetail
{
   public:
      enum {
         clear       = 0,
         shadowed    = 1,
         obstructed  = 2,
         submerged   = 3,
      };
      
      // these are at the node level
      U8          mNavFlag;
      BitSet32    mNeighbors;
      F32         mShadowHt;
      
      // these are at the triangle level
      U8          mNavFlags[2];
      F32         mShadowHts[2];
      Point3F     mWorld[2][3];
};

struct DSide
{
   S32 closePts;
   F32 bestDist;
   
   DSide() { closePts = 0; bestDist = 0.f; }
};

#endif
