//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHPATH_H_
#define _GRAPHPATH_H_

struct NavJetting
{
   NavJetting()      {init();}
   void  init()      {mHopOver = 0.0f; mChuteUp = mChuteDown = false;}
   F32   mHopOver;
   bool  mChuteUp;
   bool  mChuteDown;
};

class NavigationPath 
{
   public:
      typedef Vector<BitSet32>   VisitState;
      struct State 
      {
         enum {Visited  = (1 << 0), 
               Outbound = (1 << 1), 
               Skipped  = (1 << 2)};
         void constructThis();
         Vector<S32>       path;
         VisitState        visit;
         GraphEdgePtrs     edges;
         Point3F           seekLoc;
         Point3F           hereLoc;
         Point3F           destLoc;
         bool              thisEdgeJetting;
         bool              nextEdgeJetting;
         bool              nextEdgePrecise;
         S32               curSeekNode;
      };

   private:
      enum HowToRedo {OnDist, OnPercent};
      void        constructThis();
      bool        checkPathUpdate(TransientNode& here, TransientNode& there);
      bool        updateTransients(TransientNode& src, TransientNode& dst, bool redo);
      void        saveEndpoints(TransientNode* from, TransientNode* to);
      void        computePath(TransientNode& from, TransientNode& to);
      F32         estimateEnergy(const GraphEdge * edge);
      void        checkWallAvoid();
      void        randomization();
      void        markRenderPath();
      void        afterCompute();
      bool        canAdvance();
      void        setEdgeConstraints();
      void        advanceByOne();
      
      S32         checkOutsideAdvance();
      GraphNode * getNode(S32 idx);
      Point3F     getLoc(S32 index);

      State             mState;      
      Point3F           mHereLoc;
      Point3F           mDestLoc;
      Point3F           mAdjustSeek;
      HowToRedo         mRedoMode;
      bool              mForceSearch;
      bool              mAreIndoors;
      bool              mPathIndoors;
      bool              mAwaitingSearch;
      bool              mBusyJetting;
      F32               mPctThreshSqrd;
      F32               mRedoDistSqrd;
      F32               mRedoPercent;
      F32               mSearchDist;
      U32               mTeam;
      Point3F           mSaveDest;
      S32               mRepathCounter, mTimeSlice;
      S32               mStopCounter;
      S32               mSaveSeekNode;
      bool              mUserStuck;
      bool              mSearchWasValid;
      GraphEdge *       mCurEdge;
      GraphLocate       mLocateHere;
      GraphLocate       mLocateDest;
      GraphHookRequest  mHook;
      Point3F           mSavedEndpoints[2];
      GraphEdge         mSaveLastEdge;
      JetManager::ID    mJetCaps;
      NavJetting        mNavJetting;
      FindGraphNode     mFindHere;
      F64               mCastAng, mCastZ;
      Point3F           mCastAverage;
      const GraphEdge * mEstimatedEdge;
      F32               mEstimatedEnergy;

   public:
      NavigationPath();
      
      bool        updateLocations(const Point3F& src, const Point3F& dst);
      F32         checkOpenTerrain(const Point3F& from, Point3F& to);
      bool        locationIsOutdoors(const Point3F &loc, F32* roamDist=0);
      bool        weAreIndoors() const;
      bool        userMustJet() const;
      bool        intoMount() const;
      void        informJetDone();
      void        setJetAbility(const JetManager::Ability& ability);
      void        informStuck(const Point3F& stuckLoc, const Point3F& stuckDest);
      void        informProgress(const VectorF& vel);
      F32         jetWillNeedEnergy(F32 maxDist);
      Point3F     getLocOnPath(F32 dist);
      F32         distRemaining(F32 maxCare=1e9);
      bool        getPathNodeLoc(S32 ahead, Point3F& loc);
      const       Point3F& getSeekLoc(const VectorF& vel);
      bool        canReachLoc(const Point3F& dst);
      void        missionCycleCleanup();
      F32         visitRadius();
      void        forceSearch();
      
      // One-liners- 
      NavJetting* getJetInfo()                  {return &mNavJetting;}
      F32         searchDist() const            {return mSearchDist;}
      bool        isPathCurrent() const         {return !mAwaitingSearch;}
      void        getLastPath(Vector<S32>& L)   {L=mState.path;}
      void        setTeam(U32 whichTeam)        {mTeam = whichTeam;}
      void        setRedoDist(F32 D=0.8f)       {mRedoDistSqrd=D*D; mRedoMode=OnDist;}
      void        setRedoPercent(F32 P=0.15f)   {mRedoPercent=P; mRedoMode=OnPercent;}
      void        setDestMounted(bool b)        {mLocateDest.setMounted(b);}
      void        setSourceMounted(bool b)      {mLocateHere.setMounted(b);}
      void        informJetBusy()               {mBusyJetting = true;}
};

#endif
