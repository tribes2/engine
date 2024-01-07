//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "ai/graphLOS.h"
#include "ai/graphBridge.h"

//-------------------------------------------------------------------------------------

// Controls generation- 
static bool sSpawnGraph = false;

// Numbers that are different between the two types of graphs.  Most jet connections 
// on NAV generation use the smaller lateral value, but we special case for large 
// terrain-to-terrain hops that are passed through.  cf.  FireStorm.  
#define  NAVLateralMax           60.0
#define  IslandHopLateralMax     120.0
#define  SPNLateralMax           100.0
static const Point3F sBoxOffNAV(IslandHopLateralMax, IslandHopLateralMax, 10000.0);
static const Point3F sBoxOffSPN(SPNLateralMax, SPNLateralMax, 10000.0);

//-------------------------------------------------------------------------------------

static const U32 sLOSMask =   InteriorObjectType
                           |  StaticShapeObjectType
                           |  StaticObjectType
                           |  WaterObjectType
                           |  TerrainObjectType;
                           
static Point3F * sBreakPt1 = 0, * sBreakPt2 = 0;
static SphereF * sMagSphere;
static F32 sBreakRangeXY = 0.5, sBreakRangeZ = 2.0;
static const S32 sGraphFanCount = 10;      // ==> 1-11-1, reduced, should be fine.  
static const F32 sGraphFanWidth = 1.0;
static const F32 sWideFanRaise = 1.3;

// Clear distance assures they can get over a ledge, land Dist assures there's purchase.
// Second clear dist is for case when one node is a portal (i.e. chutes)
static const F32 sClearDists[2] = {1.3, 1.0};
static const F32 sLandDists[2] = {0.4, 0.2};
static const F32 sJumpAngDot = 0.707;

// NOT SURE ABOUT THIS #  Sometimes we want more - maybe allow other checks to insure
// that things work when it's small (clear dist, etc.)  There are some small ledge
// hops which are needed - but hop over code will probably assure...  
static const F32 sMinXY = 2.0;
static const F32 sMinXYSqrd = (sMinXY * sMinXY);

static const Point3F sMinAboveVec(0,0,2.0);

#define  SlightlyOffFloor     0.157
#define  MaxHopOverXY         37.0
// lhpatch- Make this # go on graph (was barely too small in one map at 17)
#define  MaxWalkableXY        20.0
#define  MinLateralThresh     12.0

//-------------------------------------------------------------------------------------

#define  SquareOf(x)    ((x)*(x))

static bool bridgeable(const Point3F& from, const Point3F& to, bool bothOutdoor)
{
   F32   lateralThresh;
   
   if (sSpawnGraph)
   {
      lateralThresh = SPNLateralMax;
   }
   else 
   {
      if (bothOutdoor)                                                          
         lateralThresh = IslandHopLateralMax;
      else
         lateralThresh = NAVLateralMax;

      // Pull down the lateral thresh for tall hops.  Basically subtract off of it for
      // every meter of Z, but keep a minimum amount, which is necessary to assure 
      // floating bases do get bridged (if only the downward hop is doable).  
      if ( (lateralThresh -= mFabs(from.z - to.z)) < MinLateralThresh )
         lateralThresh = MinLateralThresh;
   }
   
   Point2F  lateral(from.x - to.x, from.y - to.y);
   return (lateral.lenSquared() < SquareOf(lateralThresh));
}

//-------------------------------------------------------------------------------------

// Get the fan vector in passed ref, and return number of increments.  horzVec
// is a horizontal vector guaranteed to have length.  
static S32 calcFanVector(const VectorF& horzVec, VectorF& fanIncOut)
{
   fanIncOut.set(-horzVec.y, horzVec.x, 0);
   fanIncOut.normalize(sGraphFanWidth / F32(sGraphFanCount));
   return sGraphFanCount;
}

//-------------------------------------------------------------------------------------
// Some code for isolating cases in the debugger.  Mainly for focusing on a couple of 
// nodes to see why they're not getting bridged.  See navGraph.genDebug() in graph.cc

static bool doBreak() {return true;}

static bool checkEnds(const Point3F& n1, const Point3F& n2)
{
   if (mFabs(n1.x - sBreakPt1->x) < sBreakRangeXY)
   if (mFabs(n2.x - sBreakPt2->x) < sBreakRangeXY)
      if (mFabs(n2.y - sBreakPt2->y) < sBreakRangeXY)
      if (mFabs(n1.y - sBreakPt1->y) < sBreakRangeXY)
         if (mFabs(n1.z - sBreakPt1->z) < sBreakRangeZ)
         if (mFabs(n2.z - sBreakPt2->z) < sBreakRangeZ)
            return doBreak();
            
   return false;
}

//-------------------------------------------------------------------------------------

GraphBridge::GraphBridge(const GraphNodeList& mainList, S32 islands, 
                                    BridgeDataList& listOut)
   :  mMainList(mainList), 
      mIslands(islands), 
      mBridgesOut(listOut)
{
   heedSeeds(false);
}

//-------------------------------------------------------------------------------------

// This is a bit indirect... but here's where we control which brdiges to attempt on 
// the two passes.  On the first, all seeds are ignored.  On the second we only bridge
// where at least one useful seed is present, and NO useless seed is involved.  
bool GraphBridge::heedThese(const GraphNode* from, const GraphNode* to)
{
   if (mHeedSeeds)
      if (from->usefulSeed() || to->usefulSeed())
         return !(from->uselessSeed() || to->uselessSeed());
      else
         return false;
   else
      return !(from->potentialSeed() || to->potentialSeed());
}

void GraphBridge::heedSeeds(bool b)
{
   mHeedSeeds = b;
}

//-------------------------------------------------------------------------------------

// Try straight distance for the ratios.  After we added good scale values on the 
// edges, our bridger makes too many edges since the ratios are off.  Until this is
// addressed (if indeed it really needs it), let's use straight distance on edges. 
F32 GraphBridge::getEdgeTime(const GraphEdge * edge)
{
   return edge->mDist;
}

//-------------------------------------------------------------------------------------

// Here's where we do all the work - try to see if a bridge exists.  
// 
bool GraphBridge::tryToBridge(const GraphNode* from, const GraphNode* to, F32 heuristic)
{
   // This is actually where the separate passes (normal, then seeding) is controlled- 
   if (!heedThese(from, to))
      return false;

   // Check for in, or near, liquids.
   if (from->liquidZone() || to->liquidZone())
      return false;
      
   S32   fromIndex = from->getIndex();
   S32   toIndex = to->getIndex();

   Point3F  fromLoc = from->location();
   Point3F  toLoc = to->location();
   
   bool  oneIsPortal = (from->belowPortal() ^ to->belowPortal());

   // Debug stopping points.  
   if (sBreakPt1 && sBreakPt2)
   {
      if (checkEnds(fromLoc, toLoc) || checkEnds(toLoc, fromLoc))
         if (oneIsPortal)
            doBreak();
   }

   // NOTE: we can make the OffFloor amount smaller if outdoor node locations are 
   //       raised up to always be above terrain.  (U16 rounding I think)
   fromLoc.z += SlightlyOffFloor;
   toLoc.z += SlightlyOffFloor;
   
   bool  bothOutside = (from->outdoor() && to->outdoor());
   
   if (from->neighbors(to))
      return false;
   else 
   {
      // Prefer to bridge going UP to insure that the ratio check gets handled (larger
      // travel times going up).
      if (fromLoc.z > toLoc.z)
         return false;
      else if (fromLoc.z == toLoc.z && fromIndex > toIndex)
         return false;
   }
      
   // Create low, high, mid (point above low, but on level with high).  
   Point3F  mid, low, high;
   low = fromLoc;
   high = toLoc;
   (mid = low).z = high.z;
   F32   zDist = (high.z - low.z);

   // Get 2D dist
   Point2F  xyVec(toLoc.x - fromLoc.x, toLoc.y - fromLoc.y);
   F32      xyDistSq = xyVec.lenSquared();
   
   // Handles all the LOS work
   Loser    los(sLOSMask, true);

   if ((xyDistSq > 0.0001) && bridgeable(fromLoc, toLoc, bothOutside))
   {
      Point3F  aboveM, aboveH, clearPt, landRoom, across;
      bool     isChute = false, chuteTop = false;
      bool     walkable = false, gapWalk = false;
      bool     canCheckJet = true, direct = false;
      bool     makeBridge = false;
      F32      wall;
      
      // See if we shouldn't look for jetting.  
      F32   xyDist = mSqrt(xyDistSq);
      if (from->inventory() || to->inventory() || xyDistSq < sMinXYSqrd)
         canCheckJet = false;
      else 
      {
         if (from->getNormal().z < sJumpAngDot || to->getNormal().z < sJumpAngDot)
            canCheckJet = false;
      }

      bool  checkLandRoom = (zDist > 1.0 && to->indoor());
      
      Point3F  aboveVec = sMinAboveVec;
      
      aboveM = mid + aboveVec;
      aboveH = high + aboveVec;
      across = (high - mid);
      across.normalize();
      clearPt = mid + (sClearDists[oneIsPortal] * across);
      landRoom = high - (sLandDists[oneIsPortal] * across);
      
      // Move back on the up checks so the guy has room behind him.  We have to 
      // raise up the low a little bit to account for slope, but of course
      // we can't raise more than height.  
      if (canCheckJet)
      {
         // There are two types of chute situations here, one for if we have this 
         // connection goes to the top of the chute, the other for middle ones.
         // For the top we do a slope check off the collision up there and are lax
         // about other LOS checks.  For the middle we require a couple more.  
         if (ChuteHints::Info info = gNavGraph->getChutes().info(low, mid)) 
         {
            isChute = true;
            if (info == ChuteHints::ChuteTop)
               chuteTop = true;
         }

         // Might still want to do a little of this on chutes... ?
         if (!isChute) 
         {
            VectorF  backUpVec = (across * 0.8);
            low -= backUpVec;
            low.z += getMin(2.0f, zDist);
            mid -= backUpVec;
            aboveM -= backUpVec;
         }
      }
      
      bool  walkOff = los.haveLOS(mid, high);
      bool  hopOver = false;
      if (!walkOff && xyDist < MaxHopOverXY) 
      {
         hopOver = los.findHopLine(mid, high, 2, wall);
         if (hopOver) 
         {
            mid.z += wall;
            high.z += wall;
            aboveM.z += wall;
            aboveH.z += wall;
         }
      }
      
      if (walkOff || hopOver)
      if (los.haveLOS(high, aboveH))
      if (chuteTop || los.haveLOS(low, aboveM))
      if (chuteTop || los.haveLOS(aboveM, aboveH))
      {
         // Don't do walk checks between two outdoor nodes- 
         if (!isChute && (!from->outdoor() || !to->outdoor())) 
         {
            // do gap walk for special case of small zdiff or direct LOS
            direct = los.haveLOS(low, high);
            if (direct)
               gapWalk = los.walkOverGaps(low, high, 0.8);
            else if (zDist < gNavGlobs.mStepHeight)
               gapWalk = los.walkOverGaps(mid, high, 0.8);

            // If there's a wall to hop over, don't do the walk code.  Also, if both 
            // nodes are in the same island, we only consider walking if the 
            // heuristic (ratio of best-path-so-far to straight line dist) is high.
            if (walkOff)
               if ((from->island() != to->island()) || (heuristic > 3.7))
                  walkable = los.walkOverBumps(aboveM, aboveH);
         }
         
         if (walkable || canCheckJet)
         {
            // Do the fanned LOS - just replicate the above slew of checks.  
            VectorF  fanInc;
            S32      numFan = calcFanVector(across, fanInc);
         
            // Added extra checks 1-11-1:  Must do a wider fanned check on 
            //    low -> aboveM.  Connections without room are slipping through
            //    the tests because they are diagonal.  Also must from a point 
            //    below the clear point, else they effectively can't clear on 
            //    tall hops.  
            bool     ignoreWide = (chuteTop || (zDist < sWideFanRaise + 0.1));
            Point3F  aboveL, belowC;
            if (!ignoreWide)
            {
               aboveL.set(low.x, low.y, low.z + sWideFanRaise);
               belowC.set(clearPt.x, clearPt.y, low.z + sWideFanRaise);
            }
            
            if (los.fannedLOS1(high, aboveH, fanInc, numFan))
            if (los.fannedLOS1(high, mid, fanInc, numFan))
            if (chuteTop || los.fannedLOS1(low, aboveM, fanInc, numFan)) 
            if (chuteTop || los.fannedLOS2(aboveM, aboveH, fanInc, numFan))
            if (ignoreWide || los.fannedLOS2(aboveL, aboveM, fanInc, numFan))
            {
               if (!isChute) 
               {
                  if (gapWalk)
                     if (direct) // in the direct case we need one more fanned LOS
                        gapWalk = los.fannedLOS1(low, high, fanInc, numFan);
            
                  if (gapWalk)
                     walkable = true;
                  else if (walkable)      // (i.e. walkable so far in our checks)
                     walkable = los.fannedBumpWalk(aboveM, aboveH, fanInc, numFan);
               }

               if (walkable) 
               {
                  // Well, it's too bad we had to do all this work on to find long 
                  // connections that we throw out, but we have to figure out if it 
                  // needs jet.  
                  makeBridge = (xyDist < MaxWalkableXY);
               }
               else 
               {
                  if (canCheckJet && (xyDist > GraphJetBridgeXY)) 
                  {
                     // The clear check is only needed for jetting connections, 
                     // same with landing room check - which we should only use when
                     // going up to indoor nodes (and one LOS failure is adequate)
                     if (!checkLandRoom || !los.haveLOS(low, landRoom))
                     if (isChute || los.haveLOS(low, clearPt))
                     if (isChute || los.fannedLOS1(low, clearPt, fanInc, numFan)) 
                     if (ignoreWide || los.fannedLOS2(belowC, clearPt, fanInc, numFan))
                     {
                        // Check for bonk due to jumps, which add a couple meters.
                        // We want this to pass chutes and other indoor situations Ok, 
                        // so do LOS up at each end and use that as endpoints.  
                        
                        F32   jumpAdd = (oneIsPortal && xyDist < LiberalBonkXY ? 0.6 : gNavGlobs.mJumpAdd);
                        
                        aboveM.z += (los.heightUp(aboveM, jumpAdd) - 0.1);
                        aboveH.z += (los.heightUp(aboveH, jumpAdd) - 0.1);
                        
                        if (isChute || los.fannedLOS2(aboveM, aboveH, fanInc, numFan))
                           makeBridge = true;
                     }
                     
                     // LOS keeps track of this at lowest level- don't allow
                     // jetting connections through force fields.  
                     if (los.mHitForceField)
                        makeBridge = false;
                  }
               }
            }
         }
      }

      if (makeBridge)
      {
         GraphBridgeData   bridge1(fromIndex, toIndex);
      
         if (walkable) 
            bridge1.setWalkable();
         
         if (hopOver)
            bridge1.setHop(wall);

         mBridgesOut.push_back(bridge1);
         return true;
      }
   }
   
   return false;
}

// We use a searcher to find those same-island nodes which we should try to bridge
// to - namely those who are far on the path relative to straight line distance.  
// This will hopefully find for us all indoor jetting connections which are 'useful', 
// as well as finding which terrain nodes should be bridged (like over water).  
void GraphBridge::onQExtraction()
{
   GraphNode * mExtractNode = extractedNode();
   if (mExtractNode != mCurrent) 
   {
      if (mSameIslandMark.test(mHead.mIndex)) 
      {
         AssertFatal(mSameIslandCount > 0, "GraphBridge::onQExtraction()");
         
         // done when we've visited all the possibilities in the same island
         if (--mSameIslandCount == 0)
            setEarlyOut(true);
         
         Point3F  nodeLoc = mExtractNode->location();
         F32      straightLineDist = (mStartLoc - nodeLoc).len();
         if (straightLineDist > 0.01) 
         {
            bool  bothOutdoor = (mFromOutdoor && mExtractNode->outdoor());
            F32   ratio = distSoFar() / straightLineDist;
            F32   thresh = mRatios[bothOutdoor];
            
            if (ratio > thresh) 
            {
               // Same-island bridges with good ratios are considered first...  
               Candidate   candidate(mExtractNode, -ratio);
               mCandidates.push_back(candidate);
            }
         }
         
         mSameIslandMark.clear(mHead.mIndex);
      }
   }
}

// Do all the bridging.  Has two modes- spawn and regular nav.  Each iteration performs
// one loop of bridging- from one source node to all nodes that need to be bridged.  
S32 GraphBridge::findAllBridges()
{
   S32            nodeCount = mMainList.size();
   Vector<S32>    islandCross(mIslands);
   GraphNodeList  considerList;
   
   mSameIslandMark.setSize(nodeCount);
   mSameIslandMark.clear();
   
   mRatios[1] = 1.6;      // Outdoor-to-outdoor- try a little smaller ratio than- 
   mRatios[0] = 2.2;      //    Other combinations of connections
   
   setSizeAndClear(islandCross, mIslands);
   
   for (S32 i = 0; i < nodeCount; i++) 
   {
      if ((mCurrent = mMainList[i]) != NULL) 
      {
         // Get list of all possibilities (bound box query)
         mStartLoc = mCurrent->location();

         if (sMagSphere)
         {
            Point3F  pt = sMagSphere->center;
            if ((pt -= mStartLoc).lenSquared() > SquareOf(sMagSphere->radius))
               continue;
         }
         
         Point3F  boxOff = (sSpawnGraph ? sBoxOffSPN : sBoxOffNAV);
         Box3F    area(mStartLoc - boxOff, mStartLoc + boxOff);
         S32      curIsland = mCurrent->island();
         
         gNavGraph->getNodesInBox(area, considerList);
      
         // Divy out those in separate island, and mark those in the same island for 
         // consideration by the special seacher.  
         mCandidates.clear();
         mSameIslandCount = 0;
         for (S32 j = 0; j < considerList.size(); j++) {
            GraphNode   *  consider = considerList[j];
            if (consider != mCurrent) {
               if (consider->island() == curIsland) {
                  if (!sSpawnGraph) {
                     mSameIslandMark.set(consider->getIndex());
                     mSameIslandCount++;
                  }
               }
               else {
                  // Bridges to different islands sorted by distance
                  F32         dist = (mStartLoc - consider->location()).lenSquared();
                  Candidate   candidate(consider, dist);
                  mCandidates.push_back(candidate);
               }
            }
         }
            
         // Special search to find which same-island nodes need consideration.  We want
         // to bridge same-island nodes if their ground travel distance is far relative
         // to their straight-line distance.  
         if (!sSpawnGraph) 
         {
            setEarlyOut(false);
            mFromOutdoor = mCurrent->outdoor();
            performSearch(mCurrent, NULL);
         }
         
         // Candidates considered in order of best-ness heuristic
         mCandidates.sort();
      
         // Try to bridge to all candidates which remain.  
         setSizeAndClear(islandCross, mIslands);
         for (CandidateList::iterator c = mCandidates.begin(); c != mCandidates.end(); c++)
         {
            if (sSpawnGraph) 
            {
               // For spawn we only need to cross to an island once.  
               if (!islandCross[c->node->island()])
                  if (tryToBridge(mCurrent, c->node))
                     islandCross[c->node->island()] = true;
            }
            else 
            {
               // For regular graph, allow a certain number of bridges to each island. 
               // Need to sort list, and vary base on island size.  We pass in the 
               // heuristic # for avoiding too many walking connections.
               if (islandCross[c->node->island()] < 3)
                  if (tryToBridge(mCurrent, c->node, -(c->heuristic)))
                     islandCross[c->node->island()]++;
            }
         }
      }
   }
   
   return mBridgesOut.size();
}

S32    gCountReplacementEdges, gCountUnreachable;

// This checks to see if outdoor neighbor edges need to be modified or removed due
// to steepness.  If so, a special 'replacement' bridge is created, which either 
// removes the default one at startup, or converts it to a jetting connection.  Note 
// that this also needs to remove steep edges underwater as well.  
void GraphBridge::checkOutdoor(const GraphNode* from, const GraphNode* to)
{
   static const F32 stepD = 0.3;

   Point3F  fromLoc = from->location();
   Point3F  toLoc = to->location();
   
   // Vector for stepping sideways, and across.  We're given that these are 
   // non-zero vectors since from and to are separate neighbors.  
   Point3F  across = (toLoc - fromLoc);
   Point3F  sideVec(across.y, -across.x, 0);
   sideVec.normalize(0.8);

   // Figure out step across vector, and number of iterations. 
   across.z = 0;
   F32   dist2D = across.len();
   S32   numSteps = S32(dist2D / stepD + 1.0);
   across /= F32(numSteps);

   // Get top and bottom points.  Add amount should be enough given that the
   // consolidator generally won't pass really large peaks between nodes. 
   F32      maxZ = getMax(fromLoc.z, toLoc.z);
   Point3F  stepVec(fromLoc.x, fromLoc.y, maxZ);

   // Step three separate lines across to account for player width.
   VectorF  normal;
   F32      height, previousZ, highestZ = -1e12;
   bool     removeEdge = false;
   bool     walkable = true;
   stepVec -= sideVec;
   
   for (F32 sideWise = 0.0; sideWise < 2.1 && !removeEdge; sideWise += 1.0)
   {  
      Point3F  point = (stepVec + sideVec * sideWise);

      if (gNavGraph->terrainHeight(point, &height))
         highestZ = getMax(previousZ = height, highestZ);
      else
         removeEdge = true;
      
      // Do loop plus one extra for last point.  Look for too steep a slope if our Z 
      // is increasing.  This means we can't walk.  Also check to see if it's possible 
      // to jet - namely if the highest point isn't too far above higher node.  
      for (S32 N = 0; N <= numSteps && !removeEdge; N++, point += across)
      {
         if (gNavGraph->terrainHeight(point, &height, &normal))
         {
            // Going up and too steep?
            if (normal.z < gNavGlobs.mWalkableDot && height > previousZ)
               walkable = false;
            highestZ = getMax(previousZ = height, highestZ);
         }
         else
            removeEdge = true;   
      }
   } 

   // Most of the time, this should happen-    
   if (walkable && !removeEdge)
      return;
   
   GraphBridgeData   bridge(from->getIndex(), to->getIndex());
   bridge.setReplacement();
   
   // See if we can at least jet.  
   if (!removeEdge                                                                  && 
      (from->getNormal().z >= sJumpAngDot && to->getNormal().z >= sJumpAngDot)      && 
      (!from->liquidZone() && !to->liquidZone())                                    && 
      (highestZ - maxZ < 5.0)
     )
   {
      gCountReplacementEdges++;
      bridge.setHop(highestZ - maxZ + 1.0);
   }
   else
   {
      gCountUnreachable++;
      bridge.setUnreachable();
   }
   
   mBridgesOut.push_back(bridge);
}

// Special pass to find those outdoor neighbors that are too steep to walk up.  The 
// collision checking in checkOutdoor() only masks with terrain since it is already 
// established that there are no other obstructions along the ground.  
void GraphBridge::trimOutdoorSteep()
{
   gCountReplacementEdges = 0;
   gCountUnreachable = 0;

   S32   nodeCount = mMainList.size();
   for (S32 i = 0; i < nodeCount; i++)
      if (GraphNode * from = mMainList[i])
         if (from->outdoor()) {
            GraphEdgeArray edges = from->getEdges(NULL);
            while (GraphEdge * e = edges++)
               if (!e->isJetting())
                  if (GraphNode * to = gNavGraph->lookupNode(e->mDest))
                     if (to->outdoor())
                        checkOutdoor(from, to);
         }
}

//-------------------------------------------------------------------------------------

S32 QSORT_CALLBACK GraphBridge::CandidateList::cmpCandidates(const void * a,const void * b)
{
   F32   A = ((Candidate*)a)->heuristic;
   F32   B = ((Candidate*)b)->heuristic;
   return (A < B ? -1 : (A > B ? 1 : 0));
}

void GraphBridge::CandidateList::sort()
{
   dQsort((void* )(this->address()), this->size(), sizeof(Candidate), cmpCandidates);
}

//-------------------------------------------------------------------------------------

// Something to make it easy to quickly check out the graph generation in a particular
// area by only doing bridge building in that vicinity.  
void NavigationGraph::setGenMagnify(const SphereF * sphere, 
         const Point3F * end1, const Point3F * end2, 
         F32 xyCheck, F32 zCheck)
{
   static SphereF sSphere;
   static Point3F sPoint1, sPoint2;
   
   sMagSphere = NULL;
   sBreakPt2 = sBreakPt1 = NULL;
   
   if (sphere)    
      *(sMagSphere = &sSphere) = *sphere;
   if (end1)
      *(sBreakPt1 = &sPoint1) = *end1;
   if (end2)
      *(sBreakPt2 = &sPoint2) = *end2;
      
   sBreakRangeXY = xyCheck;
   sBreakRangeZ = zCheck;
}

//-------------------------------------------------------------------------------------
      
const char * NavigationGraph::findBridges()
{
   const char * errorText = NULL;
   
   if (!mIslandPtrs.size())
      errorText = "Error: islands haven't been marked in graph";
   else 
   {
      BridgeDataList  bridges;
      
      GraphBridge  builder(mNodeList, numIslands(), bridges);
      
      sSpawnGraph = mIsSpawnGraph;
      Loser::mCasts = 0;
      
      builder.findAllBridges(); 
      builder.trimOutdoorSteep(); 

      // set the graph variable:
      mBridgeList = bridges;
   }
   return errorText;
}

const char * NavigationGraph::pushBridges()
{
   if (mPushedBridges)
      return "Bridges already pushed - do MakeGraph() to remove them";
      
   if (mBridgeList.size() == 0)
      return "No bridges exist";
      
   for (S32 i = 0; i < mBridgeList.size(); i++) 
   {
      GraphBridgeData & B = mBridgeList[i];

      if (!B.isReplacement())
      {
         for (S32 k = 0; k < 2; k++) 
         {         
            RegularNode * src = dynamic_cast<RegularNode*>(mNodeList[B.nodes[k^0]]);
            RegularNode * dst = dynamic_cast<RegularNode*>(mNodeList[B.nodes[k^1]]);
      
            GraphEdge& edge = src->pushEdge(dst);
            if (B.mustJet()) {
               edge.setJetting();
               if (B.jetClear)
                  edge.setHop(B.jetClear);
            }
            
            mJetManager.initEdge(edge, src, dst);
         }
      }
      else 
      {
         mJetManager.replaceEdge(B);
      }
   }
   
   Con::printf("Connected %d bridges", mBridgeList.size());
   mPushedBridges = true;
   markIslands();
   return NULL;
}

//-------------------------------------------------------------------------------------

GraphSeeds::GraphSeeds(NavigationGraph& graph)
   :  mGraph(graph),
      mLoser(InteriorObjectType)
{
   // Save node count for later mapping into antecedent index table.  
   mOriginalCount = graph.numNodes();
}

// Add the seed.  We have to remember our parent (antecedent) that spawned us thinking 
// that we might be potentially useful.  
void GraphSeeds::pushSeed(GraphNode* antecedent, const Point3F& pos, GraphSeed::Type type)
{
   S32         index = antecedent->getIndex();
   GraphSeed   seed(index, pos, type);
   mAntecedents.push_back(index);
   push_back(seed);
}

Point3F sgCheckSeedUpper(-149.0, -136.5, 166.42);

// Look at drop off from each edge of node.  If it lands inside of an interior volume 
// with a modicum of containment, then we want to add a seed node there.  
void GraphSeeds::seedDropOffs(GraphNode * antecedent)
{
   // Just for quick testing - look at our shape and see if we can get the behavior we
   // want locally here- 
   // if (!within(antecedent->location(), sgCheckSeedUpper, 0.6))
   //    return;

   mVolume = mGraph.fetchVolume(antecedent, false);
   Vector<Point3F> &corners = mVolume.mCorners;
   
   if (S32 N = corners.size())
   {
      // Simplify the loop- 
      corners.push_back(corners[0]);
      
      // Do drop offs on each edge.
      for (S32 i = 0; i < N; i++)
      {
         Point3F  p0 = corners[i + 0];
         Point3F  p1 = corners[i + 1];
         Point3F  drop = scaleBetween(p0, p1, 0.5);
         VectorF  normal(p0.y - p1.y, p1.x - p0.x, 0);
         F32      len = normal.len();
         
         if (len > 0.01)
         {
            // Go out at least by clear distance.  Insure we're above our plane.  
            normal *= ((sClearDists[0] * 1.2) / len); 
            drop += normal;
            F32   floorZ = solveForZ(mVolume.mFloor, drop);
            drop.z = (floorZ + 0.5);
            if (mLoser.hitBelow(drop, 50.0f))
            {
               // Now see if we landed inside of a (different) node that's large, 
               // as defined by having good containment on this point. 
               drop.z += 0.2;
               F32   containment;
               // if (within(drop, sgCheckSeedLower, 3.0))
               if (GraphNode * closest = mGraph.closestNode(drop, &containment))
               {
                  if (closest == antecedent) 
                  {
                     AssertFatal(containment > 0, "GraphSeeds::seedDropOffs()");
                  }
                  else
                  {
                     if (containment < -1.3)
                     {
                        GraphSeed::Type   seedType = GraphSeed::DropOff;
                        
                        // For those close to chute nodes, we make these regular nodes 
                        // that go through full bridging process.  
                        if (mGraph.getChutes().findNear(drop, 2.2, 1.0, mNearChutes))
                           seedType = GraphSeed::Regular;
                     }
                  }
               }
            }
         }
      }
   }
}

// Procedure is then to cast across any of the walls we are outside of and see if 
// the point that is cast across has decent containment within this selfsame node.
// Probably the containment we are looking for is a little less than the cast 
// across distance.  If Ok, then we create the node with our given antecedent.
// See crossNearbyVolumes() in graphFind.cc where most of this work happens.  
void GraphSeeds::seedInventory(GraphNode * antecedent)
{
   Vector<Point3F>   crossings;
   const Point3F&    nodeLoc = antecedent->location();
   if (S32 N = mGraph.crossNearbyVolumes(antecedent->location(), crossings))
      for (S32 i = 0; i < N; i++)
         if (mLoser.haveLOS(nodeLoc, crossings[i]))
            pushSeed(antecedent, crossings[i], GraphSeed::Inventory);
}

S32 GraphSeeds::scatter()
{
   for (S32 i = 0; i < mGraph.numNodes(); i++)
      if (GraphNode * node = mGraph.lookupNode(i))
         if (node->indoor())
            if (node->inventory())
               seedInventory(node);
            else if (NavigationGraph::sSeedDropOffs)
               seedDropOffs(node);
   
   return size();
}

// Find those seeds which have antecedents that aren't part of the largest island
// in the graph.  These are the seeds we will want to use.  For the drop off seeds
// we only attempt to bridge to their antecedent.  
void GraphSeeds::markUsefulSeeds()
{
   // Make sure offsets are managed Ok - all seeds should now be nodes.  
   AssertFatal(mOriginalCount + size() == mGraph.numNodes(), "markUsefulSeeds()");
   
   for (iterator it = begin(); it != end(); it++)
   {
      GraphNode * antecedent = mGraph.lookupNode(it->antecedent());
      
      if (antecedent->island() != mGraph.largestIsland())
      {
         S32         mapIndex = mOriginalCount + (it - begin());
         GraphNode * seed = mGraph.lookupNode(mapIndex);
      
         // Bridge builder uses this bit- 
         seed->setUsefulSeed();
      }
   }
}

//-------------------------------------------------------------------------------------

#define  SeedBoxW    0.3
#define  SeedBoxH    2.0

// Here's where we're adding the seed into the (persisted) data lists.  They will 
// take effect as run time nodes on the next call to makeGraph().
void NavigationGraph::installSeed(const Point3F& pos)
{
   IndoorNodeInfo    nodeInfo;
   nodeInfo.pos = pos;
   nodeInfo.setSeed(true);
   mNodeInfoList.push_back(nodeInfo);
   mNodeVolumes.addVolume(pos, SeedBoxW, SeedBoxH);
}

//
// What was previously several calls from script (makeGraph, findBridges, pushBridges)
// has now been rolled altogether for the purposes of trying to fill the last holes
// that we are seeing in the graph.  This assumes that script has already uploaded
// the groundPlan and floorPlan information.  
//
bool NavigationGraph::assemble()
{
   sSpawnGraph = mIsSpawnGraph;
   Loser::mCasts = 0;

   // First we need a made graph for the seeder to work.  
   makeGraph();
   
   // Look for problem areas and scatter seeds.  
   GraphSeeds  seeds(*this);
   seeds.scatter();
   for (S32 i = 0; i < seeds.size(); i++)
      installSeed(seeds[i].location());

   // Remake to create the run time seed nodes.  
   makeGraph();

   // Now build bridges WITHOUT using the seed nodes- 
   BridgeDataList  bridges;
   GraphBridge  bridgeBuilder(mNodeList, numIslands(), bridges);
   bridgeBuilder.heedSeeds(false);
   bridgeBuilder.findAllBridges();
   
   // Set the bridges and connect.  This finds largest island, which puts us - 
   mBridgeList = bridges;
   pushBridges();
   
   // - in a position to know which seeds might matter (those with stranded parents)
   seeds.markUsefulSeeds();
   
   // Now bridge seed nodes- 
   bridges.clear();
   bridgeBuilder.heedSeeds(true);
   bridgeBuilder.findAllBridges();
   
   // Still need to trim outdoor steep connections (this adds to bridges)
   bridgeBuilder.trimOutdoorSteep(); 
   
   // Add the new bridges to the list and rebuild everything- 
   mBridgeList.merge(bridges);
   makeGraph();
   pushBridges();

   // Done
   return 0;
}
