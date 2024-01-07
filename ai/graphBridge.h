//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHBRIDGE_H_
#define _GRAPHBRIDGE_H_

class GraphBridge : public GraphSearch
{
   protected:
      typedef GraphSearch Parent;
      struct Candidate {
         GraphNode * node;
         F32         heuristic;
         Candidate(GraphNode* n=NULL, F32 m=0.0)   {node = n; heuristic = m;}
      };
      class CandidateList : public Vector<Candidate> {
            static S32 QSORT_CALLBACK cmpCandidates(const void* , const void* );
         public:
            void  sort();
      };

   protected:
      // Parameters to this object- 
      const GraphNodeList& mMainList;
      const S32         mIslands;
      BridgeDataList&   mBridgesOut;
   
      // Working variables- 
      GraphNode      *  mCurrent;
      Point3F           mStartLoc;
      BitVector         mSameIslandMark;
      S32               mSameIslandCount;
      bool              mFromOutdoor;
      bool              mHeedSeeds;
      F32               mRatios[2];
      CandidateList     mCandidates;
      ChutePtrList      mChuteList;

   protected:   
      void  onQExtraction();
      F32   getEdgeTime(const GraphEdge* e);
      bool  earlyOut() {return true;}
      bool  heedThese(const GraphNode* from, const GraphNode* to);
      bool  tryToBridge(const GraphNode* from, const GraphNode* to, F32 ratio=1e13);
      void  checkOutdoor(const GraphNode* from, const GraphNode* to);
      
   public:
      GraphBridge(const GraphNodeList& mainList, S32 islands, BridgeDataList& listOut);
      S32   findAllBridges();
      void  trimOutdoorSteep();
      void  heedSeeds(bool b);
};

class GraphSeed
{
   public:
      enum Type {DropOff, Inventory, Regular};
      
   protected:
      S32            mAntecedent;
      Point3F        mLocation;
      Type           mType;
      
   public:
      GraphSeed(S32 A, const Point3F& P, Type T) : mAntecedent(A), mLocation(P), mType(T) {}
      bool           isDropOff() const             {return mType == DropOff;}
      bool           isInventory() const           {return mType == Inventory;}
      S32            antecedent() const            {return mAntecedent;}
      const Point3F& location() const              {return mLocation;}
};

class GraphSeeds : public Vector<GraphSeed>
{
   protected:
      NavigationGraph&  mGraph;
      GraphVolume       mVolume;
      Loser             mLoser;
      Vector<S32>       mAntecedents;
      S32               mOriginalCount;
      ChutePtrList      mNearChutes;

      void  pushSeed(GraphNode* antecedent, const Point3F&, GraphSeed::Type);
      void  seedDropOffs(GraphNode* antecedent);
      void  seedInventory(GraphNode* antecedent);
      
   public:
      GraphSeeds(NavigationGraph& G);
      void  markUsefulSeeds();
      S32   scatter();
};

#endif
