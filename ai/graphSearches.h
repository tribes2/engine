//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHSEARCHES_H_
#define _GRAPHSEARCHES_H_

#ifndef _TBINHEAP_H_
#include "ai/tBinHeap.h"
#endif

#define  SearchFailureThresh  1e17
#define  SearchFailureAssure  1e19

struct SearchRef
{
   SearchRef(S32 x, F32 d, F32 t) {mIndex=x; mDist=d; mSort=t; mPrev=-1;}
   union UserData {F32 f; U32 u; S32 s; UserData() {f=0;} };
   S32         operator<(const SearchRef& sr2)  {return mSort > sr2.mSort;}
   S16         mIndex;
   S16         mPrev;
   F32         mDist;
   UserData    mUser;
   F32         mTime;
   F32         mSort;
};

typedef Vector<GraphQIndex>  QIndexList;
typedef BinHeap<SearchRef>   GraphQueue;

class GraphSearch                   // graphDijkstra.cc
{
      GraphQueue  &  mQueue;
      QIndexList  &  mQIndices;
      Vector<F32> &  mHeuristicsVec;
      GraphPartition & mPartition;
      // ^^^^ Shared lists ^^^^
      
      Point3F        mTargetLoc;
      F32         *  mHeuristicsPtr;
      S32            mTargetNode, mSourceNode;
      F32            mSearchDist;
      bool           mInProgress;
      S32            mIterations;
      S32            mRelaxCount;
      S32            mTransientStart;
      U32            mCurrentSimTime;
      U32            mTeam, mInformTeam;
      const F32   *  mInformRatings, * mJetRatings;
      GraphThreatSet mThreatSet, mInformThreats;
      GraphEdge      mEdgeBuffer[MaxOnDemandEdges];
      bool           mVisitOnExtract, mVisitOnRelax;
      bool           mEarlyOut, mAStar, mInformAStar, mRandomize;

      bool           initializeIndices();
      void           doSmallReset();
      void           doLargeReset();
      void           resetIndices();
      SearchRef   *  insertSearchRef(S32 idx, F32 dist, F32 time);
      S32            getAvoidFactor();
      F32            calcHeuristic(const GraphEdge* to);
      void           initSearch(GraphNode * S, GraphNode * D = NULL);
      bool           runDijkstra();

   protected:      
      GraphNode      *  mExtractNode;
      SearchRef         mHead;
      void              setDone()                           {mInProgress=false;}
      F32               distSoFar() const                   {return mHead.mDist;}
      F32               timeSoFar() const                   {return mHead.mTime;}
      GraphNode      *  extractedNode() const               {return mExtractNode;}
      SearchRef      *  getSearchRef(S32 nodeInd);
      SearchRef      *  lookupSearchRef(S32 qInd);
      virtual void      onQExtraction()                     {mVisitOnExtract = false;}
      virtual void      onRelaxEdge(const GraphEdge*)       {mVisitOnRelax = false;}
      virtual bool      earlyOut()                          {return (mEarlyOut=false);}
      virtual F32       getEdgeTime(const GraphEdge* e);
      
   public:
      GraphSearch();
      F32         doTableSearch(GraphNode* S, GraphNode* D, Vector<S32>& nodes);
      S32         performSearch(GraphNode* S, GraphNode* D = 0);
      bool        getPathIndices(Vector<S32>& nodeIndexList, S32 target = -1);
      bool        runSearch(GraphNode* S, GraphNode* D = 0);
      void        markVisited(BitVector& emptyVec);
      void        setThreats(GraphThreatSet T)           {mInformThreats = T;}
      void        setRatings(const F32* r)               {mInformRatings = r;}
      void        setTeam(U32 team)                      {mInformTeam = team;}
      const       GraphPartition& getPartition() const   {return mPartition;}
      bool        inProgress() const                     {return mInProgress;}
      F32         searchDist() const                     {return mSearchDist;}
      S32         getTarget() const                      {return mTargetNode;}
      S32         relaxCount() const                     {return mRelaxCount;}
      void        setRandomize(bool b)                   {mRandomize = b;}
      void        setTarget(S32 T)                       {mTargetNode = T;}
      void        setEarlyOut(bool b)                    {mEarlyOut = b;}
      void        setOnRelax(bool b)                     {mVisitOnRelax = b;}
      void        setAStar(bool b = 1)                   {mInformAStar = b;}
      class Globals {
         friend class   GraphSearch;
         GraphQueue     searchQ; 
         QIndexList     qIndices;
         Vector<F32>    heuristics;
         GraphPartition partition;
      };
};

class GraphSearchLOS : public GraphSearch       // graphSearchLOS.cc
{
      typedef GraphSearch Parent;
      const SearchThreat * *  mConsider;
      const LOSTable     *    mLOSTable;
      const SphereF* mNearSphere;
      Vector<LineSegment>  mDebugSegs;
      S32         mThreatCount;
      S32         mLOSKey, mExtractInd;
      Point3F     mExtractLoc, mLOSLoc;
      SphereF     mOuterSphere;
      Vector<S32> mChokePoints;
      F32         mNearWidth, mFarDist, mMaxChokeDist;
      F32         mBestDistSqrd, mBestHidden, mMinHidden;
      F32         mMinSlopeDot, mBestSlopeDot;
      bool        mSeekingLOS, mAvoidingLOS;
      bool        mDistanceBased;
      bool        mChokePtQuery;
      bool        mNeedInside;
      void        setNullDefaults();
   protected:
      void        onQExtraction();
      void        onRelaxEdge(const GraphEdge*);
      F32         getEdgeTime(const GraphEdge* e);
      bool        earlyOut() {return true;}
      
   public:      
      S32      performSearch(GraphNode* S, GraphNode* D, const SearchThreats& T);
      S32      findChokePoints(GraphNode* S, Vector<Point3F>& pts, F32 hideD, F32 maxD);
      Point3F  hidingPlace(const Point3F& from, const Point3F& avoid, 
                        F32 avoidRad, F32 avoidBasis, bool avoidOnSlope);
      Point3F  findLOSLoc(const Point3F& from, const Point3F& wantToSee, F32 minDist, 
                        const SphereF& getCloseTo, F32 capDist);
};

class GraphSearchDist : public GraphSearch      // graphLocate.cc
{
      GraphNode *    mBestMatch;
      NodeProximity  mBestMetric;
      Point3F        mPoint;
      ProximateList  mProximate;
      F32            mCurThreshold;
      F32            mMaxSearchDist;
      bool           mSearchOnDepth;
      
   protected:
      F32         getEdgeTime(const GraphEdge* edge);
      void        onQExtraction();
      
   public:
      GraphSearchDist();
      NodeProximity  findBestMatch(GraphNode* cur, const Point3F& loc);
      GraphNode *    bestMatch() const       {return mBestMatch;}
      void           setOnDepth(bool b)      {mSearchOnDepth = b;}
      bool           earlyOut()              {return true;}
      ProximateList& getProximate()          {return mProximate;}
      void           setDistCap(F32 d);
};

#endif
