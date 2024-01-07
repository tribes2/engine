//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHNODES_H_
#define _GRAPHNODES_H_

class    GraphSearch;
class    NavigationGraph;

class RegularNode : public GraphNode            // graphNodeBase.cc
{
      friend   class    GraphSearch;
   protected:
      Point3F           mNormal;
      
      GraphEdge *       pushTransientEdge(S32);
      void              popTransientEdge();
      
   public:
      RegularNode();
      GraphEdge&        pushEdge(GraphNode * node);
      GraphEdgeArray    getEdges(GraphEdge*) const;
      const Point3F&    getNormal() const;
      
      // one-liners:
      void              transientReserve()               {mEdges.reserve(mEdges.size()+2);}
      const Point3F&    location() const                 {return mLoc;}
      const Point3F&    fetchLoc(Point3F&) const         {return mLoc;}
      const GraphEdge*  getEdgePtr() const               {return mEdges.address();}
      GraphEdgeArray    getEdges() const                 {return getEdges(NULL);}
      S32               getIndex() const                 {return mIndex;}
      S32               edgeUsage() const                {return mEdges.memSize();}
      F32               radius() const                   {return 1.0;}
};

struct GraphBoundary                // graphIndoors.cc
{
   Point3F  seg[2];
   Point3F  normal;
   Point3F  seekPt;
   F32      distIn;
   
   GraphBoundary(const GraphEdgeInfo&);
   void setItUp(S32 N, const NodeInfoList&, const GraphVolumeList&);
   Point3F  midpoint() const  {return (seg[0] + seg[1]) * 0.5;}
};

typedef Vector<GraphBoundary> GraphBoundaries;

class InteriorNode : public RegularNode         // graphIndoors.cc
{
      typedef  RegularNode  Parent;
      F32      mMinDim, mArea;
      
   public:
      InteriorNode();
      void        init(const IndoorNodeInfo& data, S32 index, const Point3F& floor);
      void        setDims(F32 minDim, F32 area)    {mMinDim=minDim; mArea=area;}
      F32         minDim() const                   {return mMinDim;}
      F32         area() const                     {return mArea;}
      GraphEdge&  pushOneWay(const GraphEdgeInfo::OneWay&);
      NodeProximity containment(const Point3F& loc) const;
};

class OutdoorNode : public RegularNode          // graphOutdoors.cc
{
      friend class   NavigationGraph;
   protected:
      F32      mHeight;
   public:
      OutdoorNode();
      S32      getLevel() const;
      Point3F  getRenderPos() const;
      Point3F  randomLoc() const;
      F32      terrHeight() const   {return mHeight;}
      F32      radius() const       {return F32(1<<mLevel)*gNavGlobs.mSquareRadius;}
      F32      minDim() const       {return F32(1<<mLevel)*gNavGlobs.mSquareWidth;}
};

class IndoorNodeList : public Vector<InteriorNode>
{
      typedef  Vector<InteriorNode>    Parent;
   public:
      ~IndoorNodeList();
      void  clear();
      void  init(S32 sz);
};

#endif
