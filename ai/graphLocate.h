//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHLOCATE_H_
#define _GRAPHLOCATE_H_

struct ProximateNode
{
   GraphNode   *  mNode;
   NodeProximity  mProximity;
   ProximateNode(const NodeProximity& p, GraphNode* n);
};

class ProximateList : public Vector<ProximateNode>
{
   public:
      void     sort();
};

class GraphLocate : public GraphNodeList
{
   protected:
      Point3F        mLocation;
      Point3F        mLoc2D;
      F32            mTraveled;
      S32            mCounter;
      bool           mUpInAir;
      bool           mMounted;
      GraphNode *    mClosest;
      bool           mTerrain;
      NodeProximity  mMetric;
      GraphEdgeList  mConnections;
 
   protected:
      void              getNeighbors(GraphNode* of, bool outdoor, bool makeJet=false);
      bool              checkRegularEdge(Point3F from, GraphEdge* to);
      ProximateNode *   examineCandidates(ProximateList& proximate);
      void              beLikeNode(GraphNode* ourNode);
      void              pushEdge(GraphNode* to, F32* getCos=NULL);
      
   public:
      GraphLocate();
      void     update(const Point3F& loc);
      bool     canHookTo(const GraphLocate&, bool& jet) const;
      void     reset();
      void     setMounted(bool b);
      void     forceCheck();

      const GraphEdgeList& getEdges() const     {return mConnections;}
      GraphNode         *  bestMatch() const    {return mClosest;}
      NodeProximity        bestMetric() const   {return mMetric;}
      bool                 onTerrain() const    {return mTerrain;}
      bool                 isMounted() const    {return mMounted;}
      void                 cleanup()            {reset(); mMounted=false;}
};

class FindGraphNode                 // graphFind.cc
{
   private:
      GraphNode * mClosest;
      Point3F     mPoint;
      U32         calcHash(const Point3F& point);
   public:
      enum {HashTableSize = 307};
      FindGraphNode();
      FindGraphNode(const Point3F& pt, GraphNode* hint = 0);
      void        setPoint(const Point3F& pt, GraphNode * hint = 0);
      GraphNode * closest() const {return mClosest;}
      void        init();
};

#endif
