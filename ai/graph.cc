//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/fileStream.h"
#include "ai/graphFloorPlan.h"
#include "terrain/terrRender.h"
#include "game/player.h"
#include "game/gameConnection.h"
#include "ai/graphLOS.h"

IMPLEMENT_CONOBJECT(NavigationGraph);

//-------------------------------------------------------------------------------------

NavigationGraph   *  gNavGraph = NULL;
NavGraphGlobals      gNavGlobs;
S32   NavigationGraph::mIncarnation = 0;
bool  NavigationGraph::sDrawOutdoorNodes = true;
bool  NavigationGraph::sDrawIndoorNodes = true;
bool  NavigationGraph::sDrawJetConnections = true;
bool  NavigationGraph::sSeedDropOffs = false;
F32   NavigationGraph::sProcessPercent = 0.0f;
S32   NavigationGraph::sTotalEdgeCount = 0;
S32   NavigationGraph::sEdgeRenderMaxOutdoor = 300;
S32   NavigationGraph::sEdgeRenderMaxIndoor = 300;
U32   NavigationGraph::sLoadMemUsed = 0;
S32   NavigationGraph::sProfCtrl0 = 0;
S32   NavigationGraph::sProfCtrl1 = 0;
S32   NavigationGraph::sShowThreatened = -1;
static const char sSpawnPath[] = "terrains/%s.spn";
static const char sRegularPath[] = "terrains/%s.nav";

//-------------------------------------------------------------------------------------

// Version history. Only from ChangedToFloorPlan on is now supported.  I'll leave the 
// list here for aetiological reasons... :)
enum Updates {
   TerrainOnly, 
   HandLaidInterior, 
   AddedBridges, 
   AddedPathTable, 
   AddedLOSTable, 
   ChangedToFloorPlan,
   TrimmedBridges, 
   RevisedLOSToHash, 
   BetterSpawnMode 
   // AddedChuteHints
};

S32 NavigationGraph::sVersion = BetterSpawnMode;

//-------------------------------------------------------------------------------------

static const char * getGraphName(char * fileBuf, S32 bufSz, bool spawn)
{
   const char *   name = Con::getVariable("CurrentMission");
   if (spawn)
      dSprintf(fileBuf, bufSz, sSpawnPath, name);
   else
      dSprintf(fileBuf, bufSz, sRegularPath, name);
   return fileBuf;
}

//-------------------------------------------------------------------------------------

NavigationGraph::NavigationGraph()
   :  mMaxTransients(AbsMaxBotCount * 2),
      mCustomArea(0,0,0,0)
{
   AssertFatal(!gNavGraph, "May not create more than one NavigationGraph!");
   gNavGraph = this;
   mNumOutdoor = 0;
   mNumIndoor = 0;
   mTransientStart = -1;
   mCullDensity = 0.3f;
   mLargestIsland = -1;
   mPushedBridges = 0;
   mTableBuilder = NULL;
   mMainSearcher = NULL;
   mLOSSearcher = NULL;
   mDistSearcher = NULL;
   mHaveVolumes = false;
   mValidLOSTable = false;
   mValidPathTable = false;
   mIsSpawnGraph = false;
   mCheckNode = 0;
   mDeadlyLiquid = false;
   mSubmergedScale = 1.0;
   mShoreLineScale = 1.0;
   mEdgePool = NULL;
   mOutdoorNodes = NULL;
   mLOSTable = NULL;
   mVersion = -1;
   sLoadMemUsed = 0;   
   setGenMagnify(0, 0, 0);
}

NavigationGraph::~NavigationGraph()
{
   if (mTransientStart != -1)
      for (S32 j = 0; j < mMaxTransients; j++)
         if (GraphNode * transientNode = mNodeList[mTransientStart + j])
            delete transientNode;

   newIncarnation();       // (This deletes the searchers)
   
   delete [] mEdgePool;
   delete [] mOutdoorNodes;
   
   gNavGraph = NULL;
}

bool NavigationGraph::onAdd()
{
   U32   memUsedBefore = Memory::getMemoryUsed();
   
   // Temp to avoid old small devs causing huge graph builds- 
   mConjoin.maxAngleDev = getMax(mConjoin.maxAngleDev, 45.0f);

   if (!Parent::onAdd())
      return false;
      
   mTerrainBlock = GroundPlan::getTerrainObj();
   
   // DMMNOTPRESENT
//   if (TerrainRender::mLiquidType >= 4) {
//      mDeadlyLiquid = true;
//      mSubmergedScale = 1000.0;
//      mShoreLineScale = 2.0;
//   }
//   else {
      mDeadlyLiquid = false;
      mSubmergedScale = 3.7;
      mShoreLineScale = 1.41;
//   }

   bool  forceNavLoad = Con::getBoolVariable("$GraphForceLoad");
   
   if (forceNavLoad)
   {
      // Need to insure presence of NAV due to how code works below...  
      char  txt[256];
      if (Stream * S = ResourceManager->openStream(getGraphName(txt, sizeof(txt), 0)))
         ResourceManager->closeStream(S);
      else
         forceNavLoad = false;
   }
   
   if (Con::getBoolVariable("$OFFLINE_NAV_BUILD"))
      forceNavLoad = true;
   
   // If we're not in a single player game, we check HostGameBotCount to see
   // if we want to skip NAV (if one is even there).  If we are in SinglePlayer,
   // a NAV file is required.  
   mIsSpawnGraph = false;
   if (!forceNavLoad)
   {
      const char * missionType = Con::getVariable("CurrentMissionType");
      
      // If not single player, then check the bot count.  
      // If it is single player, we need the graph.
      if (dStricmp(missionType, "SinglePlayer")) 
         mIsSpawnGraph = !Con::getIntVariable("HostGameBotCount");
   }

   if (loadGraph())
   {
      if (!mIsSpawnGraph)
      {
         makeGraph(true);
         pushBridges();
         clearLoadData();        // remove load data not needed at run time.
      }
   }
   
   sLoadMemUsed = Memory::getMemoryUsed() - memUsedBefore;
   Con::printf("Memory consumed = %d", sLoadMemUsed);
   
   return true;
}

void NavigationGraph::onRemove()
{
   Parent::onRemove();
}

void NavigationGraph::clearLoadData()
{
   mEdgeInfoList.clear();                    mEdgeInfoList.compact();
   mNodeInfoList.clear();                    mNodeInfoList.compact();
   mBridgeList.clear();                      mBridgeList.compact();
   mTerrainInfo.consolidated.clear();        mTerrainInfo.consolidated.compact();
   mTerrainInfo.shadowHeights.clear();       mTerrainInfo.shadowHeights.compact();
}

// Called before saving on spawn graphs- clean out all unneeded data.  
void NavigationGraph::purgeForSpawn()
{
   mEdgeInfoList.clear();                    mEdgeInfoList.compact();
   mBridgeList.clear();                      mBridgeList.compact();
   // mNodeVolumes.clear();                     mNodeVolumes.compact();
   if (mLOSTable)
      mLOSTable->clear();
}

// Called when new graph is made.  Incarnation # used so bots can (theoretically) 
// run around which graph editing is taking place.  
void NavigationGraph::newIncarnation()
{
   mIncarnation = mIncarnation + 1;

   // Searchers-    
   delete mTableBuilder;
   delete mMainSearcher;
   delete mLOSSearcher;
   delete mDistSearcher;
   mTableBuilder = NULL;
   mMainSearcher = NULL;
   mLOSSearcher = NULL;
   mDistSearcher = NULL;
   
   mJetManager.clear();
}

//-------------------------------------------------------------------------------------

// Print size of anything which supports memSize(), accumulate total.  
#define  PrintMemSize(V, msg)   {                                          \
            Con::printf("%s: usage = %d", msg, V.memSize());               \
            total += V.memSize();  }

// Try to account for all contributors to the overall footprint- 
//    (This isn't complete yet)- 
U32 NavigationGraph::reckonMemory() const
{
   // TerrainGraphInfo        mTerrainInfo.memSize();
   // ChuteHints              mChutes;
   // OutdoorNode *           mOutdoorNodes;
   // GraphBSPTree            mIndoorTree;
   // GraphEdge *             mEdgePool;
   U32   total = 0;
   PrintMemSize(mNodeList, "mNodeList");                    // GraphNodeList  
   PrintMemSize(mNodeGrid, "mNodeGrid");                    // GraphNodeList  
   PrintMemSize(mNonTransient, "mNonTransient");            // GraphNodeList  
   PrintMemSize(mIndoorPtrs, "mIndoorPtrs");                // GraphNodeList  
   PrintMemSize(mIndoorNodes, "mIndoorNodes");              // IndoorNodeList 
   PrintMemSize(mIslandPtrs, "mIslandPtrs");                // GraphNodeList  
   PrintMemSize(mIslandSizes, "mIslandSizes");              // Vector<S32>    
   PrintMemSize(mBoundaries, "mBoundaries");                // GraphBoundaries
   PrintMemSize(mNodeVolumes, "mNodeVolumes");              // GraphVolumeList
   
   PrintMemSize(mRenderThese, "mRenderThese");              // Vector<LineSegment>
   PrintMemSize(mRenderBoxes, "mRenderBoxes");              // Vector<Point3F>    
   PrintMemSize(mTempNodeBuf, "mTempNodeBuf");              // Vector<S32>        
   PrintMemSize(mVisibleEdges, "mVisibleEdges");            // GraphEdgePtrs      
   // SearchThreats           mThreats;
   // MonitorForceFields      mForceFields;
   // JetManager              mJetManager;
   Con::printf("*** Total accounted for = %d", total);
   return total;
}

//-------------------------------------------------------------------------------------

void NavigationGraph::checkHashTable(S32 nodeCount) const
{
   #if 0
      S32   accum[4] = {0, 0, 0, 0};
      S32   errCount = 0;
   
      for (S32 i = 0; i < nodeCount; i++) {
         for (S32 j = 0; j < nodeCount; j++) {
            U32   vOld = mLOSXRef.value(i, j);
            U32   vNew = mLOSHashTable.value(i, j);
            if (vOld != vNew) {
               errCount++;
               Con::printf("LOSHash Error at %d <-> %d", i, j);
            }
            else {
               AssertFatal(vOld < 4, "Bad LOS hash table entry value");
               accum[vOld]++;
            }
         }
      }
   
      if (errCount)
         Con::printf("There were %d errors in conversion of LOS table", errCount);
      for (S32 d = 0; d < 4; d++)
         Con::printf("LOS distribution for %d = %d", d, accum[d]);
   #else
      nodeCount;
   #endif
}

// From original XREF data, convert to hash version.  
U32 NavigationGraph::makeLOSHashTable()
{
   U32   memUsage = 0;
   S32   nodeCount = numNodes();
   
   memUsage = mLOSHashTable.convertTable(mLOSXRef, nodeCount);
   checkHashTable(nodeCount);
   mLOSXRef.clear();
   mLOSTable = & mLOSHashTable;
   
   return memUsage;
}
   
//-------------------------------------------------------------------------------------

GraphSearch * NavigationGraph::getMainSearcher()
{
   if (!mMainSearcher)
      mMainSearcher = new GraphSearch();
   return mMainSearcher;
}

GraphSearchLOS * NavigationGraph::getLOSSearcher()
{
   if (!mLOSSearcher)
      mLOSSearcher = new GraphSearchLOS();
   return mLOSSearcher;
}

GraphSearchDist * NavigationGraph::getDistSearcher()
{
   if (!mDistSearcher)
      mDistSearcher = new GraphSearchDist();
   return mDistSearcher;
}

//-------------------------------------------------------------------------------------

bool NavigationGraph::gotOneWeCanUse()    // i.e. for navigation (not spawning)
{
   if (gNavGraph) {
      if (gNavGraph->mNumOutdoor || gNavGraph->mNumIndoor)
         return true;
   }
   return false;
}

bool NavigationGraph::hasSpawnLocs()
{
   return (gNavGraph && gNavGraph->mSpawnList.size());
}

bool NavigationGraph::customArea(GridArea& areaOut) const
{
   if (mCustomArea.extent.x > 0 && mCustomArea.extent.y > 0) {
      areaOut = mCustomArea;
      return true;
   }
   return false;
}

// Practically speaking, warnings might be part of graph fine tuning until Beta.  
void NavigationGraph::warning(const char* message)
{
   message;
#ifdef DEBUG
   static S32  warnCount = 0;
   Con::printf("GraphWarning #%d! %s", ++warnCount, message);
#endif
}

void NavigationGraph::initPersistFields()
{
   Parent::initPersistFields();
   
   // Slope deviation at which terrain squares are considered flat.  
   addField("conjoinAngleDev", TypeF32, Offset(mConjoin.maxAngleDev, NavigationGraph));

   // ratio of largest island to total nodes:   
   addField("cullDensity", TypeF32, Offset(mCullDensity, NavigationGraph));
   
   // If user doesn't want the mission area as default- 
   addField("customArea", TypeRectI, Offset(mCustomArea, NavigationGraph));
}

//-------------------------------------------------------------------------------------
//    GRAPH LOAD & SAVE. 

bool NavigationGraph::loadGraph()
{
   bool     Ok = false;
   char     fileBuf[256];

   getGraphName(fileBuf, sizeof(fileBuf), mIsSpawnGraph);

   // Load if found-    
   if (Stream * stream = ResourceManager->openStream(fileBuf)) 
   {
      if (! load(* stream, mIsSpawnGraph))
         Con::printf("loadGraph: Failed to load '%s'", fileBuf);
      else
         Ok = true;
      ResourceManager->closeStream(stream);

      if (Ok && !mIsSpawnGraph)
      {
         // NAV graph needs the spawn data.  I know, this isn't all that organized... 
         getGraphName(fileBuf, sizeof(fileBuf), true);
         if (Stream * spawnStream = ResourceManager->openStream(fileBuf))
         {
            if (!load(* spawnStream, true))
               Con::printf("Error loading spawn file into NAV graph");
            ResourceManager->closeStream(spawnStream);
         }
         else
            Con::printf("loadGraph: NAV Ok, but SPN open failed (%s)", fileBuf);
      }
   }
   else
      Con::printf("loadGraph: Couldn't open '%s' for read", fileBuf);
      
   return Ok;
}

bool NavigationGraph::saveGraph()
{
   FileStream     fStream;
   bool           Ok = false;
   char           fileBuf[256];
   
   getGraphName(fileBuf, sizeof(fileBuf), mIsSpawnGraph);
   
   if (ResourceManager->openFileForWrite(fStream, ResourceManager->getModPathOf(fileBuf), fileBuf)) 
   {
      if (mIsSpawnGraph)
      {
         makeSpawnList();
         purgeForSpawn();
      }
      if (!save(fStream))
         Con::printf("saveGraph: Failed to save '%s'", fileBuf);
      else
         Ok = true;
      fStream.close();
     
      // call scripts that were done.
      Con::executef(this, 1, "navBuildComplete");
   }
   else
      Con::printf("saveGraph: Couldn't open '%s' for write", fileBuf);
      
   return Ok;
}

//-------------------------------------------------------------------------------------
//       GRAPH STREAM PERSISTENCE 

bool NavigationGraph::load(Stream & s, bool isSpawn)
{
   bool     Ok = true;
   
   Ok &= s.read(&mVersion); 
   
   // Some reserved- 
   S32   res;
   Ok &= (s.read(&res) && s.read(&res) && s.read(&res) && s.read(&res));

   // The reduced spawn list-    
   if (mVersion >= BetterSpawnMode)
   {
      Ok &= mSpawnList.read(s);
      
      // Before we're loaded, this condition signals that graph is just a spawn list.
      // After loaded, non-empty mSpawnList is the condition....  
      if (isSpawn)
         return Ok;
      else
         mSpawnList.reset();
   }
   
   // Read edges & nodes/volumes. Volume list is separate from indoor node list only 
   // because of order things were developed in.  Otherwise they are one-to-one.  
   Ok &= readVector1(s, mEdgeInfoList);
   Ok &= readVector1(s, mNodeInfoList);
   Ok &= mNodeVolumes.read(s);
   
   // Outside- 
   Ok &= mTerrainInfo.read(s);
   
   // Bridges-
   AssertISV(mVersion >= TrimmedBridges, "Graph needs rebuilt for this mission");
      Ok &= mBridgeList.read(s);
   // else
   //    Ok &= mBridgeList.readOld(s);
   
   // Tables- 
   if(mVersion >= AddedPathTable)
      Ok &= mPathXRef.read(s);
   
   if(mVersion >= AddedLOSTable) 
   {
      if (mVersion >= RevisedLOSToHash)
      {
         Ok &= mLOSHashTable.read(s);
         mLOSTable = & mLOSHashTable;
      }
      else
      {
         Ok &= mLOSXRef.read(s);
         if (sVersion >= RevisedLOSToHash)
         {
            // Temporary conversion code
            S32 numNodes = mNodeInfoList.size() + mTerrainInfo.consolidated.size();
            if (mLOSXRef.valid(numNodes))
            {
               mLOSHashTable.convertTable(mLOSXRef, numNodes);
               checkHashTable(numNodes);
               mLOSXRef.clear();
               mLOSTable = & mLOSHashTable;
            }
         }
         else
            mLOSTable = & mLOSXRef;
      }
   }
      
   // if(mVersion >= AddedChuteHints)
   //    Ok &= mChutes.read(s);
   
   return Ok;
}

bool NavigationGraph::save(Stream & s)
{
   bool     Ok = true;
   
   Ok &= s.write(sVersion);

   // Reserveds- 
   S32      res = 0;
   Ok &= (s.write(res) && s.write(res) && s.write(res) && s.write(res));
   
   if (sVersion >= BetterSpawnMode)
   {
      if (!mIsSpawnGraph)
         mSpawnList.reset();
      Ok &= mSpawnList.write(s);
      if (mIsSpawnGraph)
         return Ok;
   }
   
   // Indoors-
   Ok &= writeVector1(s, mEdgeInfoList);
   Ok &= writeVector1(s, mNodeInfoList);
   Ok &= mNodeVolumes.write(s);
   
   // Outside-
   Ok &= mTerrainInfo.write(s);
   
   // Bridges-
   Ok &= mBridgeList.write(s);
   
   // Tables- 
   Ok &= mPathXRef.write(s);

   // Write out the table-    
   if (sVersion < RevisedLOSToHash)
      Ok &= mLOSXRef.write(s);
   else 
      Ok &= mLOSHashTable.write(s);
   
   // Chute hints- 
   // Ok &= mChutes.write(s);
   
   return Ok;
}

//-------------------------------------------------------------------------------------
//                              Graph Console Functions
//-------------------------------------------------------------------------------------

static bool cSaveGraph(SimObject *ptr, S32, const char **)
{
   NavigationGraph *navGraph = static_cast<NavigationGraph*>(ptr);
   return navGraph->saveGraph();
}

//-------------------------------------------------------------------------------------

static bool cSetGround(SimObject *ptr, S32, const char **argv)
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   GroundPlan  * gp = dynamic_cast<GroundPlan *>(Sim::findObject(argv[2]));
   
   if (!gp)
      Con::printf("Couldn't find ground plan %s", argv[2]);
   else
      return navGraph->setGround(gp);
      
   return false;
}

//-------------------------------------------------------------------------------------

static bool cMakeGraph(SimObject *ptr, S32, const char **)
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   navGraph->makeGraph(false);
   return true;
}

//-------------------------------------------------------------------------------------

static bool cLoadGraph(SimObject *ptr, S32, const char **)
{
   NavigationGraph *navGraph = static_cast<NavigationGraph*>(ptr);
   return navGraph->loadGraph();
}

//-------------------------------------------------------------------------------------
//    LOS XRef table construction.  

static bool cPrepLOS(SimObject * ptr, S32 argc, const char **argv)
{
   Point3F  viewLoc(0,0,100);
   if (argc == 3)
      dSscanf(argv[2], "%f %f %f", &viewLoc.x, &viewLoc.y, &viewLoc.z);
   NavigationGraph *navGraph = static_cast<NavigationGraph*>(ptr);
   return navGraph->prepLOSTableWork(viewLoc);
}

static bool cMakeLOS(SimObject * ptr, S32, const char **)
{
   NavigationGraph *navGraph = static_cast<NavigationGraph*>(ptr);
   return navGraph->makeLOSTableEntries();
}

//-------------------------------------------------------------------------------------
//       Bridge building and management

// Processing routine - just builds the data
static bool cFindBridges(SimObject *ptr, S32, const char **)
{
   NavigationGraph *navGraph = static_cast<NavigationGraph*>(ptr);
   if (const char * errorText = navGraph->findBridges()) 
   {
      Con::printf(errorText);
      return false;
   }
   return true;
}

// Installs the edges onto the nodes of a made graph
static bool cPushBridges(SimObject *ptr, S32, const char **)
{
   NavigationGraph *navGraph = static_cast<NavigationGraph*>(ptr);
   
   if (S32 islandsBefore = navGraph->numIslands())
   {
      if (const char * errorText = navGraph->pushBridges()) 
         Con::printf(errorText);
      else 
      {
         if (S32 islandsBridged = islandsBefore - navGraph->numIslands())
            Con::printf("%d islands have been bridged", islandsBridged);
         return true;
      }
   }
   else
      Con::printf("Graph hasn't been made");
      
   return false;
}

//-------------------------------------------------------------------------------------

// Try to cull out dense clusters of nodes.  Pass in a param if you want to 
// cull out everything except biggest island.  
static bool cCullIslands(SimObject *ptr, S32, const char **)
{
   NavigationGraph *navGraph = static_cast<NavigationGraph*>(ptr);
   navGraph->cullIslands();
   return true;
}

//-------------------------------------------------------------------------------------

static bool cMakeTables(SimObject * ptr, S32, const char* * )
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   navGraph->makeTables();
   return true;
}

//-------------------------------------------------------------------------------------

static bool cAssemble(SimObject * ptr, S32, const char* *)
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   return navGraph->assemble();
}

//-------------------------------------------------------------------------------------

static S32 cNumNodes(SimObject* ptr, S32, const char* [])
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   return navGraph->numNodes();
}

static bool cSetGenMode(SimObject* ptr, S32 argc, const char* argv[])
{
   if (argc == 3) {
      NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
      navGraph->setGenMode(!dStricmp(argv[2], "spawn"));
   }
   else
      Con::printf("Set graph generation mode to Nav (default) or Spawn");
   return true;
}
   
//-------------------------------------------------------------------------------------

static S32 cRandNode(SimObject* ptr, S32 argc, const char* argv[])
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);

   if (argc >= 4) 
   {
      Point3F  pt;
      dSscanf(argv[2], "%f %f %f", &pt.x, &pt.y, &pt.z);
      F32   radius = dAtoi(argv[3]);
      bool  inside = argc > 4 ? dAtob(argv[4]) : false;
      bool  outside = argc > 5 ? dAtob(argv[5]) : false;

      return navGraph->randNode(pt, radius, inside, outside);
   }
   else 
   {
      Con::printf ("Given a point, radius, and flags for indoor or outdoor");
      Con::printf ("inclusion, %s returns a node index (-1 if not found).", argv[1]);
      Con::printf ("Note use navGraph.nodeLoc() to get location from index");
   }
   return -1;
}
   
static const char bogusLocation[] = "0 0 500";
   
static const char * cNodeLoc(SimObject* ptr, S32 argc, const char* argv[])
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   
   if (argc == 3) 
   {
      S32   nodeIndex =dAtoi(argv[2]);
      
      if (const Point3F * pt = navGraph->getSpawnLoc(nodeIndex))
      {
         char * buff = Con::getReturnBuffer(100);
         dSprintf(buff, 100, "%f %f %f", pt->x, pt->y, pt->z);
         return buff;
      }
      else
         Con::printf("Invalid index (%d) passed to nodeLoc()", nodeIndex);
   }
   else
      Con::printf("Gets the location of the specified node or spawn index");
   
   return bogusLocation;
}

// Go up slightly and cast down to ground.  
static void adjustSpawnLoc(Point3F & point)
{
   point.z += 1.3;
   Loser    cast(-1);
   if (cast.hitBelow(point, 20.0))
   {
      Point2F  sideways(cast.mColl.normal.x, cast.mColl.normal.y);
      point.z += sideways.len() / 2.0;
      point.z += 0.2;      // <<=== some shapes have problems...  cf. ThinInce
   }
}
   
static const char * cRandNodeLoc(SimObject* ptr, S32 argc, const char* argv[])
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   
   if (argc == 3) 
   {
      S32   nodeIndex =dAtoi(argv[2]);
      
      if (const Point3F * pt = navGraph->getRandSpawnLoc(nodeIndex))
      {
         // if (NavigationGraph::sProfCtrl0 == 337) {
         //    // Testing steepness on Escalade- want spawns near high building- 
         //    static U32        stagger = 0;
         //    static Point3F    testLocs[5] = {
         //       Point3F(438, -42, 297),
         //       Point3F(434.4, -53.5, 270),
         //       Point3F(428, -45, 264),
         //       Point3F(463, -14, 264),
         //       Point3F(473, -44, 223)
         //    };
         //    pt = &testLocs[stagger++ % 5];
         // }
         
         Point3F  point = * pt;
         adjustSpawnLoc(point);
         char * buff = Con::getReturnBuffer(100);
         dSprintf(buff, 100, "%f %f %f", point.x, point.y, point.z);
         return buff;
      }
      else
         Con::printf("Invalid index (%d) passed to %s()", nodeIndex, argv[1]);
   }
   else
      Con::printf("Finds a random location within the space of the given node");
   
   return bogusLocation;
}

// Get a good direction to face, given a location.  
static const char * cWhereToLook(SimObject* , S32 argc, const char* argv[])
{
   if (argc >= 2) {
      Point3F  point;
      dSscanf(argv[1], "%f %f %f", &point.x, &point.y, &point.z);

      // We get a Z rotation back- 
      F32   angle = - NavigationGraph::whereToLook(point);

      // Build our return string for use by setTransform().
      char * buff = Con::getReturnBuffer(120);
      dSprintf(buff, 120, "%f %f %f 0 0 1 %f", point.x, point.y, point.z, angle);
      return buff;
   }
   else {
      Con::printf ("Given a point, return a transform in a 'nice' direction to look.");
      return NULL;
   }
}
   
static bool cNavGraphExists(SimObject*, S32, const char **)
{
   return (NavigationGraph::gotOneWeCanUse() || NavigationGraph::hasSpawnLocs());
}

static void cDumpInfo2File(SimObject *ptr, S32, const char**)
{
   NavigationGraph * ng = static_cast<NavigationGraph*>(ptr);
   
   #define printDump(a) {const char * b = a; LogFile.write(dStrlen(b),b);}
   
   FileStream LogFile;
   
   LogFile.open("NavMetrics.log", FileStream::ReadWrite);

   if(LogFile.getStatus() == Stream::Ok)
   {
      LogFile.setPosition(LogFile.getStreamSize());
      
      printDump("\r\n\r\n");
      // printDump(avar("Mission: %s", ng->mGraphFile));
      printDump("---------------------------\r\n");
      printDump(avar("Graph Stats: %d nodes (%d outdoor)\r\n", 
                     ng->numNodes(), ng->numOutdoor()));
      printDump(avar("--> %d bridges\r\n", ng->numBridges()));
      printDump(avar("--> %d edges\r\n", NavigationGraph::sTotalEdgeCount));
      printDump(avar("Graph load memory used:  %d", NavigationGraph::sLoadMemUsed));
      printDump("\r\n");
   }
   
   LogFile.close();
}
 
static bool cGraphInfo(SimObject* ptr, S32 argc, const char* argv[])
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);

   // Test it out
   // U32   memUse = navGraph->makeLOSHashTable();
   // Con::printf("Hash table segment pool takes up %d bytes", memUse);
   
   S32   bridges = navGraph->numBridges();
   S32   edges = NavigationGraph::sTotalEdgeCount;
   S32   totalNodes = navGraph->numNodes();
   S32   numOutdoor = navGraph->numOutdoor();
   
   Con::printf("Graph Stats: %d nodes (%d outdoor)", totalNodes, numOutdoor);
   Con::printf("--> %d islands", navGraph->numIslands());
   Con::printf("--> %d bridges", bridges);
   Con::printf("--> %d edges", edges);
   Con::printf("Graph load memory used:  %d", NavigationGraph::sLoadMemUsed);
   Con::printf("Edge alloc = (%d + %d + 2 x %d) x %d == %d", 
                  bridges, edges, totalNodes, sizeof(GraphEdge), 
                  (bridges + edges + 2 * totalNodes) * sizeof(GraphEdge) 
                  );
   Con::printf("NavGraph structure = %d bytes", sizeof(NavigationGraph));
   
   if (argc > 2) {
      Point3F  from;
      dSscanf(argv[2], "%f %f %f", &from.x, &from.y, &from.z);
      F32   inner = (argc > 3 ? dAtof(argv[3]) : 50);
      F32   outer = (argc > 4 ? dAtof(argv[4]) : 1e9);
      U32   cond  = (argc > 5 ? dAtoi(argv[5]) : 3);

      navGraph->clearRenderSegs();
      navGraph->markNodesInSight(from, inner, outer, cond);
      Con::printf(navGraph->drawNodeInfo(from));
   }
   return true;
}

//-------------------------------------------------------------------------------------

static void cSpawnInfo(SimObject* ptr, S32, const char* [])
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   navGraph->printSpawnInfo();
}

//-------------------------------------------------------------------------------------

// Args to installThreat and updateThreat are same, parse here.  Also checks for 
// presence of adequate graph.  
static ShapeBase * fetchThreatInfo(S32 argc, const char* argv[], S32& team, F32& rad)
{
   if (NavigationGraph::gotOneWeCanUse()) {
      ShapeBase * threatObject;
      if (argc >= 4 && Sim::findObject(argv[2], threatObject)) {
         team = dAtoi(argv[3]);
         rad = argc > 4 ? dAtof(argv[4]) : 50.0f;
         return threatObject;
      }
   }
   return NULL;
}

static bool cInstallThreat(SimObject* , S32 argc, const char* argv[])
{
   S32   team;
   F32   rad;

   if (ShapeBase * threat = fetchThreatInfo(argc, argv, team, rad))
      return gNavGraph->installThreat(threat, team, rad);
      
   Con::printf("%s(): register permanent (static object) threat on the graph", argv[1]);
   return false;
}

//-------------------------------------------------------------------------------------

static bool cDetectForceFields(SimObject* , S32, const char**)
{
   if (NavigationGraph::gotOneWeCanUse()) {
      gNavGraph->detectForceFields();
   }
   return false;
}

//-------------------------------------------------------------------------------------
// These are patches to use for suspected script function bottlenecks so they show up 
// in the profiler.  We pass down arguments.  Do some timing, might give info.  

struct TrackPatch 
{
   enum { MaxDepth = 12 };
   U32   recursed[MaxDepth + 1];
   U32   totalMS, numCalls, depth, maxDepth, lastMS;
   F32   average;
   const char * name;
   TrackPatch()
   {
      totalMS = numCalls = depth = maxDepth = lastMS = 0;
      dMemset(recursed, 0, sizeof(recursed));
      average = 0.0f;
   }
   const char * patch(S32, const char**);
};

const char * TrackPatch::patch(S32 argc, const char * * argv)
{
   name = argv[0];
   recursed[depth++]++;
   if (depth > maxDepth) 
   {
      AssertFatal(depth < MaxDepth, avar("Patched too deep: %s", name));
      maxDepth = depth;
   }
   // Call and time it- 
   U32   saveMS = Platform::getRealMilliseconds();
   const char * result = Con::execute(argc - 1, argv + 1);
	lastMS = (Platform::getRealMilliseconds() - saveMS);
   totalMS += lastMS;
   average = F32(totalMS) / F32(++numCalls);
   depth--;
   return result;
}

TrackPatch gTrackProfPatch1;
const char * cProfilePatch1(SimObject* , S32 argc, const char** argv)
{
   return gTrackProfPatch1.patch(argc, argv);
}

TrackPatch gTrackProfPatch2;
const char * cProfilePatch2(SimObject* , S32 argc, const char** argv)
{
   return gTrackProfPatch2.patch(argc, argv);
}

//-------------------------------------------------------------------------------------


// For checking out a couple of LOS bugs.  Also mulitple casts for getting idea
// of LOS "budget".   ==>  Needs to go back into DEBUG
static const char * cGetLOSPoint(SimObject* , S32 argc, const char * * argv)
{
   if (argc >= 4) 
   {
      Point3F  from, losPt, to;
      dSscanf(argv[2], "%f %f %f", &from.x, &from.y, &from.z);
      dSscanf(argv[3], "%f %f %f", &to.x, &to.y, &to.z);
      S32      iters = (argc > 4 ? dAtoi(argv[4]) : 1);

      RayInfo  coll;
      while (--iters >= 0)      
         if (gServerContainer.castRay(from, to, U32(-1), &coll))
            if (!iters)
            {
               Point3F  S = coll.point;
               Con::printf("Found solution = (%f, %f, %f)", S.x, S.y, S.z);
            }
   }
   return 0;
}

//-------------------------------------------------------------------------------------

// Want to test the spawning:  
static S32 cNumSpawns(SimObject * ptr, S32, const char **)
{
   return (static_cast<NavigationGraph * >(ptr))->numSpawns();
}
static const char * cGetSpawn(SimObject * ptr, S32 argc, const char * * argv)
{
   if (argc == 3)
   {
      char *            buff = Con::getReturnBuffer(100);
      NavigationGraph * graph = static_cast<NavigationGraph * >(ptr);
      S32               which = dAtoi(argv[2]);
      
      if (validArrayIndex(which, graph->numSpawns()))
      {
         Point3F  point = graph->getSpawn(which);
         adjustSpawnLoc(point);
         dSprintf(buff, 100, "%f %f %f", point.x, point.y, point.z);
         return buff;
      }
      else
         Con::printf("Spawn index %d out of range!", which);
   }
   return 0;
}

//-------------------------------------------------------------------------------------

#ifdef DEBUG

static const char * cHidingPlace(SimObject* , S32 argc, const char * * argv)
{
   if (argc >= 4) {
      Point3F  from, avoidPt;
      dSscanf(argv[2], "%f %f %f", &from.x, &from.y, &from.z);
      dSscanf(argv[3], "%f %f %f", &avoidPt.x, &avoidPt.y, &avoidPt.z);
      F32   rad      = (argc > 4 ? dAtof(argv[4]) : 13.0f);
      F32   hideLen  = (argc > 5 ? dAtof(argv[5]) : 22.0f);
      Point3F  seek;

      // Two hide queries- first seeks those with further hide length byeond, other
      // seeks a slope away from LOS point (used for sniping)
      if (hideLen > 0)
         seek = NavigationGraph::hideOnDistance(from, avoidPt, rad, hideLen);
      else
         seek = NavigationGraph::hideOnSlope(from, avoidPt, rad, -hideLen);
      
      Con::printf("Seek point is (%f, %f, %f)", seek.x, seek.y, seek.z);
   }
   else {
      Con::printf("From src, find hiding place from avoid at least rad away.");
      Con::printf("Hides based on hidden distance beyond or slope angle.");
   }
   return 0;
}

//-------------------------------------------------------------------------------------

static const char * cChokePoints(SimObject* , S32 argc, const char * * argv)
{
   if (argc >= 4) {
      Point3F  from;
      dSscanf(argv[2], "%f %f %f", &from.x, &from.y, &from.z);
      F32   hideDist  = dAtof(argv[3]);
      F32   maxDist   = (argc > 4 ? dAtof(argv[4]) : 1e9);
      Vector<Point3F>   points;
      NavigationGraph::getChokePoints(from, points, hideDist, maxDist);
   }
   else {
      Con::printf("Get list of choke points from source.  HideDist tells how long of");
      Con::printf("obstructed length must exist out of LOS.  maxDist truncs search");
   }
   return 0;
}

//-------------------------------------------------------------------------------------

// Tracking has been removed, but we may put it back in a different for.  Anyway, 
// this has some other info. 
static bool cTrackObject(SimObject* ptr, S32 argc, const char* argv[])
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   GameConnection    * target;
   if (argc >= 3 && Sim::findObject(argv[2], target)) 
   {
      if (Player * player = dynamic_cast<Player*>(target->getControlObject())) 
      {
         // We just sort of dump all our test stuff here....  
         F32   thrust, dur, jumpSpeed;
         player->getJetAbility(thrust, dur, jumpSpeed);
         F32   rating = gNavGraph->jetManager().calcJetRating(thrust, dur);
         Con::printf("Jet rating for player = %f", rating);
         rating += (jumpSpeed * dur * TickSec);
         Con::printf("With jump, rating is = %f", rating);

         return true;
      }
   }
   return true;
}

//-------------------------------------------------------------------------------------

// See if payer can get from A to B.  
static bool cPlayerCanGo(SimObject* ptr, S32 argc, const char* argv[])
{
   if (argc == 5)
   {
      Point3F  A, B;
      Player * player;
      if (Sim::findObject(argv[2], player)) 
      {
         NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
         dSscanf(argv[3], "%f %f %f", &A.x, &A.y, &A.z);
         dSscanf(argv[4], "%f %f %f", &B.x, &B.y, &B.z);
         
         F32   ratings[2];
         JetManager::Ability  ability;
         player->getJetAbility(ability.acc, ability.dur, ability.v0);
         gNavGraph->jetManager().calcJetRatings(ratings, ability);
         Con::printf("Ratings = %d and %d", ratings[0], ratings[1]);
         if (gNavGraph->testPlayerCanReach(A, B, ratings))
            Con::printf("Made it");
      }
      else
         Con::printf("Could not find Player %s", argv[2]);
   }
   return false;
}


// debug
#endif

#ifdef INTERNAL_RELEASE
#define  DO_MATH_TESTS     1
#else
#define  DO_MATH_TESTS     0
#endif

#if DO_MATH_TESTS
extern void Athlon_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *res);
extern void SSE_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *res);
extern void default_matF_x_matF_C(const F32 *matA, const F32 *matB, F32 *res);

extern U32 gSSE_MatXMat_Calls;

static Point3F& rangify(Point3F& cycle)      // keep in range- 
{
   if (cycle.x > 4.0)      cycle.x -= (3.14159 * 2.0);
   if (cycle.y > 5.0)      cycle.y -= (3.14159 * 2.0);
   if (cycle.z > 6.0)      cycle.z -= (3.14159 * 2.0);
   return cycle;
}

F32 sWorstResultDiff = 0.0;         // insure same results- 
static bool verify(const F32 * m1, const F32 * m2, S32 N = 16)
{
   while(N--) {
      F32   D = mFabs(*m1++ - *m2++);
      sWorstResultDiff = getMax(D, sWorstResultDiff);
      if (D > 0.01)
         return false;
   }
   return true;
}

// Test out the Athlon code- 
static S32 cTestMath(SimObject*, S32 argc, const char* argv[])
{
   // May just want this info (so pass in zero to add nothing to it)
   Con::printf("Num calls to SSE func = %d", gSSE_MatXMat_Calls);
   
   // Default to a million calls- 
   S32      numTests = (argc > 1 ? dAtoi(argv[1]) : 10);
   S32      numIters = (argc > 2 ? dAtoi(argv[2]) : 100000);
   S32      whatTest = (argc > 3 ? dAtoi(argv[3]) : 0);
   void     (*mo_Betta_Math)(const F32*, const F32 *, F32*) = default_matF_x_matF_C;
   S32      failures = 0;
   
   U32   properties = Platform::SystemInfo.processor.properties;  
   switch(whatTest)
   {
      case  0:
         if (properties & CPU_PROP_SSE) {
            Con::errorf("Testing SSE");         
            mo_Betta_Math = SSE_MatrixF_x_MatrixF;         
         } else {
            Con::errorf("SSE not detected!"); 
            return 0;
         }
         break;         
      case  1:
         if (properties & CPU_PROP_3DNOW) {
            Con::errorf("Trying to test 3DNow");
            mo_Betta_Math = Athlon_MatrixF_x_MatrixF;
         } else {
            Con::errorf("3DNow not detected!");
            return 0;
         }
         break;
   }

   // Make random matrices.  
   for (S32 which = 0; which < 2; which++)
   {
      Point3F  point1(-1e9, 1000.1, -2), point2(1, 2, 1e17);
      EulerF   cycle1(0,0,0), cycle2(M_PI,M_PI,M_PI);
      EulerF   add1(0.2, 0.3, 0.5), add2(0.7, 0.13, 0.37);
      U32      ms = Platform::getRealMilliseconds();
      
      for (S32 i = 0; i < numTests; i++)
      {
         MatrixF     mat1(cycle1);
         MatrixF     mat2(cycle2);
         mat1.setColumn(3, point1);
         mat1.setColumn(3, point2);
         MatrixF     res1, res2;

         // Check we're getting same results-          
         default_matF_x_matF_C(mat1, mat2, res1);
         mo_Betta_Math(mat1, mat2, res2);
         if (!verify(res1, res2)) {
            Con::printf("Matrix outputs differ on case %d!", i);
            failures++;
         }
            
         // Now do repeated calls to get timing.  Use separate loops to avoid dilution.
         if (which)  for (S32 j = 0; j < numIters; j++)
            default_matF_x_matF_C(mat1, mat2, res1);
         else for (S32 j = 0; j < numIters; j++)
            mo_Betta_Math(mat1, mat2, res1);
         
         rangify(cycle1 += add1);
         rangify(cycle2 += add2);
         point1 -= add2;
         point2 += add1;
      }
      Con::printf("%d ms on pass %d", Platform::getRealMilliseconds() - ms, which);
   }
   
   Con::printf("Worst result difference = %f", sWorstResultDiff);

   return failures;
}
#endif


//-------------------------------------------------------------------------------------

// Test out the closestNode() function on the graph.  Also other misc. test/debug stuff
static bool cCheckFunction(SimObject* ptr, S32 argc, const char* argv[])
{
   if (argc == 3) 
   {
      Point3F  center;
      F32      contain;
      NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
      dSscanf(argv[2], "%f %f %f", &center.x, &center.y, &center.z);
      GraphNode   * best = navGraph->closestNode(center, & contain);
      if (best)
      {
         Con::printf("********** Containment = %f **********", contain);
         Point3F  P = best->location();
         Con::printf("Supplied loc = (%f, %f, %f)", center.x, center.y, center.z);
         Con::printf("Node location = (%f, %f, %f)", P.x, P.y, P.z);
         NodeProximity  prox = best->containment(center);
         Con::printf("Prox = (%f, %f, %f)", prox.mLateral, prox.mHeight, prox.mAboveC);
      }
      else
         Con::printf("Nothing found");
         
      FloorPlan::setBreakPoint(center);
   }
   return true;
}

//-------------------------------------------------------------------------------------

//
// This probably needs to remain ISV because it can be useful in determining if a graph
// is feasible / efficient.  
//
static bool cTimeTest(SimObject * ptr, S32 argc, const char * * argv)
{
   if (argc >= 3) 
   {
      S32   numSearches = dAtoi(argv[2]);
      NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
      bool  doAStar = (argc > 3 ? dAtob(argv[3]) : false);
      
      U32   saveMS = Platform::getRealMilliseconds();
      F32   average = navGraph->performTests(numSearches, doAStar);
      F32   elapsed = F32(Platform::getRealMilliseconds() - saveMS);
      
      Con::printf("Average number of Q extractions in search = %f", average);
      Con::printf("Elapsed time = %f seconds", elapsed * 0.001);
   }
   return true;
}

//-------------------------------------------------------------------------------------

PreloadTextures::PreloadTextures()
{
   mNext = 0;
}

PreloadTextures::~PreloadTextures()       // just want to watch destruct in debugger...
{
   mNext = 0;
}

void PreloadTextures::load(const char * name, bool clamp)
{
   AssertFatal(mNext < MaxHandles-1, "PreloadTextures::load()");
   mTextures[mNext++] = TextureHandle(name, MeshTexture, clamp);
}

static bool cPreload(SimObject * ptr, S32, const char * * argv)
{
   NavigationGraph * navGraph = static_cast<NavigationGraph*>(ptr);
   navGraph->mTextures.load(argv[2], dAtob(argv[3]));
   return true;
}

//-------------------------------------------------------------------------------------

static bool cGenDebug(SimObject * ptr, S32 argc, const char * * argv)
{
   NavigationGraph * navGraph = static_cast<NavigationGraph* >(ptr);
   
   if (argc > 2)
   {
      Point3F  magnifyLoc, dbg1, dbg2;
      Point3F  * p1 = NULL, * p2 = NULL;
      F32      magnifyRad = 40;
      F32      xyCheck = 0.5, zCheck = 2.0;
   
      dSscanf(argv[2], "%f %f %f", &magnifyLoc.x, &magnifyLoc.y, &magnifyLoc.z);
      
      if (argc > 3)
      {
         magnifyRad = dAtof(argv[3]);
         
         if (argc > 4)
         {
            // These aren't documented and are just for internal debugging. They're 
            // kept ISV since the build process is tryingly slow in DTEST, and they
            // don't effect operation, or significant speed/space, in ship verion.  
            p1 = & dbg1;
            dSscanf(argv[4], "%f %f %f", &p1->x, &p1->y, &p1->z);
            if (argc > 5)
            {
               p2 = & dbg2;
               dSscanf(argv[5], "%f %f %f", &p2->x, &p2->y, &p2->z);
               if (argc > 6)
               {
                  xyCheck = dAtof(argv[6]);
                  if (argc > 7)
                     zCheck = dAtof(argv[7]);
               }
            }
         }
      }

      SphereF  magnifySphere(magnifyLoc, magnifyRad);
      navGraph->setGenMagnify(&magnifySphere, p1, p2, xyCheck, zCheck);
      return true;
   }

   navGraph->setGenMagnify(NULL, NULL, NULL);
   Con::printf("This function is for testing the graph generation by focusing");
   Con::printf("on a smaller area (magnify sphere) to speed things up.");
   return false;
}

//-------------------------------------------------------------------------------------

static const char navGraphTxt[] = "NavigationGraph";

#define  GraphCmd1(method, cfunc, usage, minparams, maxparams)    \
            Con::addCommand(navGraphTxt, method, cfunc, usage, minparams, maxparams)
#define  GraphCmd2(function, cfunc, usage, minparams, maxparams)  \
            Con::addCommand(function, cfunc, usage, minparams, maxparams)
#define  GraphVar1(name, type, svar)                              \
            Con::addVariable(name, type, &NavigationGraph::##svar)
#define  GraphVar2(name, type, varname)                           \
            Con::addVariable(name, type, &varname)


static void addGraphVariables()
{
   GraphVar1("$pref::NavGraph::drawOutdoor",    TypeBool,   sDrawOutdoorNodes);
   GraphVar1("$pref::NavGraph::drawIndoor",     TypeBool,   sDrawIndoorNodes);
   GraphVar1("$pref::NavGraph::drawJetEdges",   TypeBool,   sDrawJetConnections);
   GraphVar1("graphProcessPercent",             TypeF32,    sProcessPercent);

   // Average MS per call to patch functions-    
   GraphVar2("patch1Avg",                       TypeF32,    gTrackProfPatch1.average);
   GraphVar2("patch2Avg",                       TypeF32,    gTrackProfPatch2.average);
   GraphVar2("patch1Total",                     TypeS32,    gTrackProfPatch1.totalMS);
   GraphVar2("patch2Total",                     TypeS32,    gTrackProfPatch2.totalMS);
   GraphVar2("patch1Last",                     	TypeS32,    gTrackProfPatch1.lastMS);
   GraphVar2("patch2Last",                     	TypeS32,    gTrackProfPatch2.lastMS);
   GraphVar2("patch1Calls",                     TypeS32,    gTrackProfPatch1.numCalls);
   GraphVar2("patch2Calls",                     TypeS32,    gTrackProfPatch2.numCalls);

   // Number of edges rendered in the AI editor-
   GraphVar1("edgeRenderMaxOutdoor",            TypeS32,    sEdgeRenderMaxOutdoor);
   GraphVar1("edgeRenderMaxIndoor",             TypeS32,    sEdgeRenderMaxIndoor);
   
   // Nodes in view of this threat should be highlighted- 
   GraphVar1("showNodeThreat",                  TypeS32,    sShowThreatened);

   // Some control variables for channeling the profiles. eg. focusing on indoors, etc
   GraphVar1("ProfileControl0",                 TypeS32,    sProfCtrl0);
   GraphVar1("ProfileControl1",                 TypeS32,    sProfCtrl1);
   
   GraphVar1("Graph::SeedDropOffs",          TypeBool,      sSeedDropOffs);
}


void NavigationGraph::consoleInit()
{
   Parent::consoleInit();
   
   addGraphVariables();

   // Temp- 
   GraphCmd1("Preload", cPreload, "navGraph.preload(name,clamp);", 4, 4);
   
   GraphCmd1("makeGraph", cMakeGraph, "navGraph.makeGraph();", 2, 2);
   GraphCmd1("setGenMode", cSetGenMode, "navGraph.setGenMode(nav|spawn);", 2, 3);
   
   GraphCmd1("saveGraph", cSaveGraph, "navGraph.saveGraph();", 2, 2);
   GraphCmd1("loadGraph", cLoadGraph, "navGraph.loadGraph();", 2, 2);

   // Misc. graph building functions- 
   GraphCmd1("setGround", cSetGround, "navGraph.setGround(GroundPlan);", 3, 3);
   GraphCmd1("prepLOS", cPrepLOS, "navGraph.prepLOS();", 2, 3);
   GraphCmd1("makeLOS", cMakeLOS, "navGraph.makeLOS();", 2, 2);
   GraphCmd1("findBridges", cFindBridges, "navGraph.findBridges();", 2, 2);
   GraphCmd1("pushBridges", cPushBridges, "navGraph.pushBridges();", 2, 2);
   GraphCmd1("cullIslands", cCullIslands, "navGraph.cullIslands();", 2, 2);
   GraphCmd1("makeTables", cMakeTables, "navGraph.makeTables();", 2, 2);
   GraphCmd1("assemble", cAssemble, "navGraph.assemble();", 2, 2);

   // Tests / diagnostics
   GraphCmd1("timeTest", cTimeTest, "navGraph.timeTest(iterations[, doAStar])", 2, 4);
   GraphCmd1("genDebug", cGenDebug, "navGraph.genDebug(magnifyLoc[, magnifyRad])", 2, 8);
      GraphCmd1("check", cCheckFunction, "navGraph.check(loc);", 3, 3);
      GraphCmd1("getLOSPoint", cGetLOSPoint, "navGraph.getLOSPoint(from, to [,N]);", 4, 5);
      GraphCmd1("numSpawns", cNumSpawns, "navGraph.numSpawns()", 2, 2);
      GraphCmd1("getSpawn", cGetSpawn, "navGraph.getSpawn(which)", 2, 3);
   #ifdef DEBUG
      GraphCmd1("track", cTrackObject,       "navGraph.track(clientId);", 2, 3);
      GraphCmd1("hidingPlace", cHidingPlace, "navGraph.hidingPlace(src,avoid,rad,len/ang);", 2, 6);
      GraphCmd1("chokePoints", cChokePoints, "navGraph.chokePoints(here,hideD,maxD);", 2, 5);
      GraphCmd1("playerCanGo", cPlayerCanGo, "navGraph.playerCanGo(fromA,toB,player);", 5, 5);
   #endif
   
   // Queries
   GraphCmd1("randNode", cRandNode, "navGraph.randNode(pt, rad, indoor, outdoor);", 2, 6);
   GraphCmd1("numNodes", cNumNodes, "navGraph.numNodes();", 2, 2);
   GraphCmd1("nodeLoc", cNodeLoc, "navGraph.nodeLoc(nodeIndex);", 2, 3);
   GraphCmd1("randNodeLoc", cRandNodeLoc, "navGraph.randNodeLoc(nodeIndex);", 2, 3);
   GraphCmd2("WhereToLook", cWhereToLook, "WhereToLook(playerLoc);", 2, 2);
   GraphCmd2("navGraphExists", cNavGraphExists, "navGraphExists();", 1, 1);
   GraphCmd1("info", cGraphInfo, "navGraph.info([loc], [rad]);", 2, 7);
   GraphCmd1("dumpInfo2File", cDumpInfo2File, "navGraph.dumpInfo2File();", 2, 2);
   GraphCmd1("spawnInfo", cSpawnInfo, "navGraph.spawnInfo();", 2, 2);
   
   // Script should register threats with these- 
   GraphCmd1("installThreat", cInstallThreat, "navGraph.installThreat(id, team [,R]);", 2, 5);
   GraphCmd2("NavDetectForceFields", cDetectForceFields, "NavDetectForceFields();", 1, 1);
   
   // Utilities for profiling script functions- 
   GraphCmd2("ProfilePatch1", cProfilePatch1, "ProfilePatch1(func, args...);", 2, 20);
   GraphCmd2("ProfilePatch2", cProfilePatch2, "ProfilePatch2(func, args...);", 2, 20);
   
   
   #if DO_MATH_TESTS
      GraphCmd2("TestMath", cTestMath, "TestMath(nTests, nIters);", 1, 4);
   #endif
}
