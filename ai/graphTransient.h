//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHTRANSIENT_H_
#define _GRAPHTRANSIENT_H_

class TransientNode : public RegularNode
{
      friend class   NavigationGraph;
      GraphEdgeList  mSearchEdges;
      S32            mGroundStart;
      const GraphNode * mClosest, * mSaveClosest;
      const GraphNode * mFirstHook;
   public:
      TransientNode(S32 index);
      
      GraphEdgeArray  getEdges(GraphEdge*) const;
      GraphEdgeArray  getHookedEdges() const;
      void     informPushed();
      void     makeEdgeList(const Point3F& loc, const GraphNodeList& list);
      void     connectOutdoor(GraphNode* outdoorNode, bool all=true);
      void     setLoc(const Point3F& loc)             {mLoc = loc;}
      void     setEdges(const GraphEdgeList& edges)   {mEdges=edges;}
      void     setClosest(const GraphNode* closest)   {mClosest=closest;}
      Point3F  getRenderPos() const;
      S32      volumeIndex() const;
};

class GraphHookRequest 
{
   private:
      TransientNode&    getHook(S32 N);
      S32               mIncarnation;
      S32               mPairLookup;
      
   public:
      GraphHookRequest();
      ~GraphHookRequest();
      TransientNode&    getHook1()        { return getHook(0); }
      TransientNode&    getHook2()        { return getHook(1); }
      bool              iGrowOld() const;
      void              reset();
};

#endif
