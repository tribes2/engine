//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPH_H_
#define _GRAPH_H_

#define  _GRAPH_DEBUG_
#define  _GRAPH_WARNINGS_     0
#define  _GRAPH_PART_         1

#ifndef _GRAPHDATA_H_
#include "ai/graphData.h"
#endif
#ifndef _GRAPHBASE_H_
#include "ai/graphBase.h"
#endif
#ifndef _GRAPHPARTITION_H_
#include "ai/graphPartition.h"
#endif
#ifndef _GRAPHNODES_H_
#include "ai/graphNodes.h"
#endif
#ifndef _GRAPHTRANSIENT_H_
#include "ai/graphTransient.h"
#endif
#ifndef _GRAPHLOCATE_H_
#include "ai/graphLocate.h"
#endif
#ifndef _GRAPHTHREATS_H_
#include "ai/graphThreats.h"
#endif
#ifndef _GRAPHFORCEFIELD_H_
#include "ai/graphForceField.h"
#endif
#ifndef _GRAPHJETTING_H_
#include "ai/graphJetting.h"
#endif
#ifndef _GRAPHSEARCHES_H_
#include "ai/graphSearches.h"
#endif
#ifndef _GRAPHPATH_H_
#include "ai/graphPath.h"
#endif
#ifndef _GRAPHGROUNDPLAN_H_
#include "ai/graphGroundPlan.h"
#endif
#ifndef _TEXTUREPRELOAD_H_
#include "ai/texturePreload.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

class NavigationGraph : public SimObject 
{
      typedef  SimObject Parent;
      static   S32   sVersion;
      friend class NavigationPath;
   
   public:
      static   bool  sDrawOutdoorNodes, sDrawIndoorNodes, sDrawJetConnections;
      static   S32   sEdgeRenderMaxOutdoor, sEdgeRenderMaxIndoor;
      static   S32   sProfCtrl0, sProfCtrl1;
      static   S32   sTotalEdgeCount;
      static   F32   sProcessPercent;
      static   U32   sLoadMemUsed;
      static   S32   sShowThreatened;
      static   bool  sSeedDropOffs;
      
      static   void  consoleInit();
      static   void  initPersistFields();
      static   bool  gotOneWeCanUse();
      static   bool  hasSpawnLocs();
      static   void  warning(const char* msg);
      static   F32   fastDistance(const Point3F& p1, const Point3F& p2);
      static   Point3F  hideOnSlope(const Point3F& from, const Point3F& avoid, 
                           F32 rad, F32 deg);
      static   Point3F  hideOnDistance(const Point3F& from, const Point3F& avoid, 
                           F32 rad, F32 hideLen = 10.0);
      static   Point3F  findLOSLocation(const Point3F& from, const Point3F& wantToSee, 
                           F32 minAway, const SphereF& getCloseTo, F32 capDist=1e11);
      static   S32   getChokePoints(const Point3F& srcPoint, Vector<Point3F>& points, 
                           F32 minHideDist, F32 stopSearchDist=1e9);
      static   F32   whereToLook(Point3F P);
   
   protected:
      // .NAV or .SPN file data:
      SpawnLocations          mSpawnList;
      EdgeInfoList            mEdgeInfoList;
      NodeInfoList            mNodeInfoList;
      GraphVolumeList         mNodeVolumes;
      TerrainGraphInfo        mTerrainInfo;
      BridgeDataList          mBridgeList;
      PathXRefTable           mPathXRef;
      ChuteHints              mChutes;
      LOSHashTable            mLOSHashTable;
      
      // Run time node / edge lists:
      GraphNodeList           mNodeList;
      GraphNodeList           mNodeGrid;
      GraphNodeList           mNonTransient;
      GraphNodeList           mIndoorPtrs;
      IndoorNodeList          mIndoorNodes;
      OutdoorNode *           mOutdoorNodes;
      GraphNodeList           mIslandPtrs;
      Vector<S32>             mIslandSizes;
      GraphBSPTree            mIndoorTree;
      GraphEdge *             mEdgePool;
      LOSXRefTable            mLOSXRef;
      LOSTable *              mLOSTable;

      S32                     mVersion;
      S32                     mLargestIsland;
      S32                     mNumOutdoor;
      S32                     mNumIndoor;
      S32                     mTransientStart;
      static S32              mIncarnation;
      S32                     mCheckNode;
      const S32               mMaxTransients;
      bool                    mValidLOSTable;
      bool                    mValidPathTable;
      bool                    mHaveVolumes;
      bool                    mPushedBridges;
      bool                    mIsSpawnGraph;
      bool                    mDeadlyLiquid;
      F32                     mSubmergedScale;
      F32                     mShoreLineScale;
      GraphSearch *           mTableBuilder;
      GraphSearch *           mMainSearcher;
      GraphSearchLOS *        mLOSSearcher;
      GraphSearchDist *       mDistSearcher;
      GraphBoundaries         mBoundaries;
      SimObjectPtr<TerrainBlock> mTerrainBlock;
      Vector<LineSegment>     mRenderThese;
      Vector<Point3F>         mRenderBoxes;
      GraphEdge               mEdgeBuffer[MaxOnDemandEdges];
      GraphEdgePtrs           mVisibleEdges;
      SearchThreats           mThreats;
      MonitorForceFields      mForceFields;
      JetManager              mJetManager;

      // Temp buffers returned (as const T &) by misc fetch methods-
      Vector<S32>             mTempNodeBuf;
      GraphNodeList           mUtilityNodeList1;
      GraphNodeList           mUtilityNodeList2;
      GraphVolume             mTempVolumeBuf;
      
      // Inspect persist variables.
      F32                     mCullDensity;
      ConjoinConfig           mConjoin;
      GridArea                mCustomArea;
      
   public:
      FindGraphNode           mFoundNodes[FindGraphNode::HashTableSize];

   protected:
      bool           onAdd();
      void           onRemove();
      S32            markIslands();
      S32            doFinalFixups();
      void           clearLoadData();
      void           purgeForSpawn();
      bool           initInteriorNodes(const EdgeInfoList&, const NodeInfoList&, S32);
      S32            getNodesInArea(GraphNodeList& listOut, GridArea gridArea);
      void           makeRunTimeNodes(bool useEdgePool = false);
      S32            setupOutdoorNodes(const GridArea& area,const Consolidated& cons,
                        GraphNodeList& grid,GraphNodeList& list); 
      F32            distancef(const Point3F& p1, const Point3F& p2);
      void           chokePoints(const Point3F& S, Vector<Point3F>& pts, F32 H, F32 M);
      S32            crossingSegs(const GraphNode*, const GraphEdge*, LineSegment* const);
      void           getInSphere(const SphereF&, GraphNodeList& I, GraphNodeList& O);
      void           getOutdoorList(GraphNodeList& list);
      void           newIncarnation();
      S32            hookTransient(TransientNode&);
      void           unhookTransient(TransientNode&);
      S32            floodPartition (U32 seed, bool needBoth, F32 jetRating);
      S32            partitionOneArmor(PartitionList& list, F32 jetRating);
     
   public:
      NavigationGraph();
      ~NavigationGraph();
      DECLARE_CONOBJECT(NavigationGraph);
      
      F32            performTests(S32 numTests, bool aStar = 0);
      bool           testPlayerCanReach(Point3F A, Point3F B, const F32* ratings);
      F32            lineAcrossTerrain(const Point3F& from, Point3F& to);
      bool           inNodeVolume(const GraphNode* node, const Point3F& point);
      NodeProximity  getContainment(S32 indoorNodeIndex, const Point3F& point);
      bool           verticallyInside(S32 indoorIndex, const Point3F& point);
      bool           closestPointOnVol(GraphNode*, const Point3F& pt, Point3F& soln) const;
      bool           possibleToJet(const Point3F& from, const Point3F& to, U32 armor=0);
      F32            heightAboveFloor(S32 indoorIndex, const Point3F& point);
      S32            indoorIndex(const GraphNode* node);
      PlaneF         getFloorPlane(GraphNode* node);
      
      bool           useVolumeTraverse(const GraphNode* from, const GraphEdge* to);
      bool           volumeTraverse(const GraphNode* from, const GraphEdge* to, NavigationPath::State&);
      S32            checkIndoorSkip(NavigationPath::State & state);
      
      bool           volumeTraverse(const GraphNode*, const GraphEdge*, const Point3F&, U8&, Point3F&);
      bool           volumeTraverse(const GraphNode*, const GraphEdge*, const NavigationPath::State&);
      S32            checkIndoorSkip(const GraphEdgePtrs& edges, const Point3F& cur, S32 from, Vector<U8>& visit, Point3F& seek);
      bool           installThreat(ShapeBase * threat, S32 team, F32 rad);
      bool           updateThreat(ShapeBase * threat, S32 team, F32 rad);
      void           patrolForProblems();

      // Three types of search machines get used- 
      GraphSearch*      getMainSearcher();
      GraphSearchLOS*   getLOSSearcher();
      GraphSearchDist*  getDistSearcher();
      
      // Transient management
      void           destroyPairOfHooks(S32 idx);
      S32            createPairOfHooks();
      bool           pushTransientPair(TransientNode& src, TransientNode& dst, 
                              U32 team, const JetManager::ID& jetCaps);
      bool           canReachLoc(const FindGraphNode& src, const FindGraphNode& dst, 
                              U32 team, const JetManager::ID& jetCaps);
      void           popTransientPair(TransientNode& src, TransientNode& dst);

      // Graph construction methods called from console, console queries, ...
      bool           load(Stream&, bool isSpawn = false);
      bool           save(Stream&);
      bool           loadGraph();
      bool           saveGraph();
      bool           assemble();
      bool           makeGraph(bool usePool=false);
      S32            makeTables();
      const char *   findBridges();
      const char *   pushBridges();
      S32            scatterSeeds();
      void           installSeed(const Point3F& at);
      S32            compactIndoorData(const Vector<S32>& itrList, S32 numOutdoor);
      void           setGenMagnify(const SphereF*,const Point3F*,const Point3F*,F32 xy=1,F32 z=2);
      bool           prepLOSTableWork(Point3F viewLoc);
      bool           makeLOSTableEntries();
      S32            cullIslands();
      bool           setGround(GroundPlan * gp);
      S32            randNode(const Point3F& P, F32 R, bool in, bool out);
      void           detectForceFields() {mForceFields.atMissionStart();}
      void           checkHashTable(S32 nodeCount) const;
      bool           sanctionSpawnLoc(GraphNode * node, const Point3F& pt) const;
      const Point3F* getRandSpawnLoc(S32 nodeIndex);
      const Point3F* getSpawnLoc(S32 nodeIndex);
      U32            makeLOSHashTable();
      void           makeSpawnList();
      U32            reckonMemory() const;

      // Queries for nodes and edges: for hand edited; and for algorithmic
      S32            setNodesAndEdges(const EdgeInfoList&, const NodeInfoList&);
      S32            getNodesAndEdges(EdgeInfoList& edges, NodeInfoList& nodes);
      S32            setEdgesAndNodes(const EdgeInfoList&, const NodeInfoList&);
      S32            setNodeVolumes(const GraphVolumeList& volumes);
      S32            setAlgorithmic(const EdgeInfoList& E, const NodeInfoList& N);
      S32            setChuteHints(const Vector<Point3F>& chutes);
      S32            clearAlgorithmic();
      S32            findJumpableNodes();
      void           expandShoreline(U32 wave = 0);
      
      // Node and edge finding - 
      const GraphEdgePtrs& getBlockedEdges(GameBase* byThis, U32 objTypeMask);
      const GraphNodeList& getVisibleNodes(GraphNode * S, const Point3F& loc, F32 rad);
      const GraphNodeList& getVisibleNodes(const Point3F& loc, F32 rad);
      const GraphVolume& fetchVolume(const GraphNode* N, bool above);
      S32            getNodesInBox(Box3F worldBox, GraphNodeList& listOut, bool justIndoor=false);
      S32            crossNearbyVolumes(const Point3F& from, Vector<Point3F>& points);
      GraphNode   *  terrainHookNode(const Point3F& pos, S32& index);
      GraphNode   *  findTerrainNode(const Point3F& loc);
      GraphNode   *  nearbyTerrainNode(const Point3F& loc);
      GraphNode   *  closestNode(const Point3F& loc, F32 * containment = NULL);
      bool           haveMuzzleLOS(S32 nodeInd1, S32 nodeInd2);
      bool           terrainHeight(Point3F pos, F32* height, Point3F* normal=0);
      F32            getRoamRadius(const Point3F &loc);
      void           worldToGrid(const Point3F& wPos, Point2I& gPos);
      Point3F        gridToWorld(const Point2I& gPos);
      GridArea       getGridRectangle(const Point3F& atPos, S32 gridRad);
      bool           customArea(GridArea& fetch) const;
      GridArea       getWorldRect();
      
      // render methods      
      void           drawNodeInfo(GraphNode * node);
      const char*    drawNodeInfo(const Point3F& loc);
      void           render(Point3F &camPos, bool drawClipped);
      void           drawNeighbors(GraphNode*, GraphEdgeArray, const Point3F&, F32 w=1e9);
      void           markNodesInSight(const Point3F& from, F32 in, F32 out, U32 cond);
      void           clearRenderBoxes()   {mRenderBoxes.clear();}
      void           clearRenderSegs()    {mRenderThese.clear();}
      void           getCrossingPoints(S32 from, Point3F* pts, GraphEdge* to);
      void           setRenderList(const Vector<LineSegment>& segs) {mRenderThese=segs;}
      void           renderInteriorNodes(Point3F camPos);
      void           pushRenderSeg(const LineSegment& ls);
      void           pushRenderBox(Point3F boxLoc, F32 zadd=1.5);
      void           renderTransientNodes();
      GraphThreatSet showWhichThreats() const;

      // One-liners
      S32         incarnation() const           {return mIncarnation;}
      GraphNode*  lookupNode(S32 X) const       {return mNodeList[X];}
      S32         numIslands() const            {return mIslandPtrs.size();}
      S32         numOutdoor() const            {return mNumOutdoor;}
      S32         numNodesAll() const           {return mNodeList.size();}
      S32         numNodes() const              {return mNonTransient.size();}
      bool        validLOSXref() const          {return mValidLOSTable;}
      bool        validPathXRef() const         {return mValidPathTable;}
      bool        haveVolumes() const           {return mHaveVolumes;}
      bool        haveForceFields() const       {return (mForceFields.count() != 0);}
      F32         submergedScale() const        {return mSubmergedScale;}
      F32         shoreLineScale() const        {return mShoreLineScale;}
      bool        deadlyLiquids() const         {return mDeadlyLiquid;}
      bool        haveTerrain() const           {return bool(mTerrainBlock);}
      S32         numBridges() const            {return mBridgeList.numPositiveBridges();}
      S32         largestIsland() const         {return mLargestIsland;}
      S32         numSpawns() const             {return mSpawnList.size();}
      Point3F     getSpawn(S32 i) const         {return mSpawnList[i];}
      void        printSpawnInfo() const        {mSpawnList.printInfo();}
      void        setGenMode(bool spawn)        {mIsSpawnGraph = spawn;}
      void        monitorForceFields()          {mForceFields.monitor();}
      Vector<S32>&  tempNodeBuff()              {return mTempNodeBuf;}
      const LOSTable* getLOSXref() const        {return mValidLOSTable ? mLOSTable : NULL;}
      const GraphBoundary& getBoundary(S32 i)   {return mBoundaries[i];}
      GraphThreatSet getThreatSet(S32 T) const  {return mThreats.getThreatSet(T);}
      void newPartition(GraphSearch* S, U32 T)  {mForceFields.informSearchFailed(S, T);}
      SearchThreats* threats()                  {return &mThreats;}
      JetManager& jetManager()                  {return mJetManager;}
      ChuteHints& getChutes()                   {return mChutes;}

      // We manage the data that is shared (for memory optimization) among searchers
      GraphSearch::Globals    mSharedSearchLists;

      // Temporary - store on graph until we have a special object for it- 
      PreloadTextures      mTextures;
};

extern   NavigationGraph * gNavGraph;

#endif
