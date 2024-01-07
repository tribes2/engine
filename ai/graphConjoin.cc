//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graphData.h"
#include "Core/BitTables.h"
#include "ai/graphGroundPlan.h"
//                               Two pass consolidation:
//    
//    1. Find what is maximum level that can be consolidated based on flatness (+ same-
//          type-ness of the squares inside).  
//    2. Actually assign the levels, limiting some based on condition II below.  
//    
//    Here are the conditions for consolidating squares.  
// 
//    I. A square can only be consolidated if there are no level zero squares inside
//             and if all of the four subsquares have managed to be consolidated (plus 
//                being inside the graph region).  ====>  plus no empty squares.  
//    II. No terrain node can have a neighbor whose level differs by more than one. So 
//          we can't have a huge consolidated square with a bunch of little neighbors, 
//             for example.  There are two motivations for this:  We'd like to be able 
//                to cap the neighbor count; Second, it will help us insure that large
//                   nodes still make sense as distinct entries in the LOS xref table.
//    III. A square can be consolidated if all sub-squares are of the same type, plus
//          this square is of size 1<<L for some L,and aligned on like boundary (i.e.
//             the L low bits of X and Y will be zero).  
//
//=====================================================================================

#define  Whatever    true

GridNormalInfo::GridNormalInfo()
{
   notEmpty = false;
   hasSteep = false;
}

//
// Straight-up vectors are a singularity with Eulers, so just to be safe we make our 
// own pseudo-Euler, a pair of angles along X and Y, like so- 
//
// angle.x     Counter-clockwise angle of projection of normal into XZ plane.
// angle.y     Same thing in YZ plane.  Ex:  (5,2,1) -> PI/6.  
//
Point2F GridNormalInfo::normalToAngle(const VectorF& normal)
{
   Point2F  angle;
   angle.x = (M_PI / 2.0) - mAtan( normal.x, normal.z );
   angle.y = (M_PI / 2.0) - mAtan( normal.y, normal.z );
   return angle;
}
VectorF GridNormalInfo::angleToNormal(const Point2F& A)
{
   VectorF  normal( mCos(A.x)/mSin(A.x), mCos(A.y)/mSin(A.y), 1.0);
   normal.normalize();
   return normal;
}

//-------------------------------------------------------------------------------------

//
// The following 3 classes perform separate passes of the data consolidation.
//    Note they only build the data (what gets persisted) - creating run time
//       nodes from this data is done elsewhere.  
//
class FindBestConsolidations : public GridVisitor 
{
   protected:
      Vector<GridNormalInfo>  mGridNormals;
      TrackLevels & mTrackLevels; 
      const ConjoinConfig & mConfigure;
      F32 mDotThreshold;
      void getGridNormals();
      bool allSameType(const GridArea& R, U8& nodeType);
      bool areaIsFlat(const GridArea& R);
      bool atLevelZero(const GridArea& R);                              // virtual
      bool afterDivide(const GridArea& R, S32 level, bool success);     // virtual
      
   public:
      FindBestConsolidations(const GridArea& G, const ConjoinConfig& C, TrackLevels& T);
};
class SmoothOutLevels : public GridVisitor
{
   protected:
      const S32      mLevel;
      TrackLevels &  mTrackLevels;
      bool allWater(const GridArea& R);
      void capLevelAt(const GridArea& R, S32 L);
      void doCurrentBottom(const GridArea& R);
      bool atLevelZero(const GridArea& R);                              // virtual
      bool beforeDivide(const GridArea& R, S32 level);                  // virtual
      
   public:
      SmoothOutLevels(const GridArea& G, TrackLevels& T, S32 L);
};
class BuildConsolidated : public GridVisitor
{
   protected:
      const TrackLevels &  mTrackLevels;
      Consolidated &       mConsolidated;
      bool checkAddToList(const GridArea& R, S32 level);
      bool beforeDivide(const GridArea& R, S32 level);                  // virtual
      bool atLevelZero(const GridArea& R);                              // virtual
      
   public:
      BuildConsolidated(const GridArea& G, const TrackLevels& T, Consolidated& C);
};


//-------------------------------------------------------------------------------------

ConjoinConfig::ConjoinConfig()
{
   maxAngleDev = 45;       // (only one actually used right now...)
   maxBowlDev = 70;
   maxLevel = 6;
}

//-------------------------------------------------------------------------------------

// Just check that it's a valid rect for that level.  
static bool checkArea(const GridArea & G, S32 L, const char * caller)
{
   U16            ext = (1 << L);
   U16            mask = ext - 1;
   const char  *  problem = NULL;
   
   if( G.point.x & mask )
      problem = "X point isn't aligned";
   else if ( G.point.y & mask )
      problem = "Y point isn't aligned";
   else if( G.extent.x != ext )
      problem = "X extent is bad";
   else if( G.extent.y != ext )
      problem = "Y extent is bad";
   
   AssertFatal( caller && ! problem, avar("Problem= %s in %s", problem, caller) );
   
   return ! problem && caller;
}

TrackLevels::TrackLevels(S32 sz)
{
   init(sz);
}

void TrackLevels::init(S32 size)
{
   setSizeAndClear(achievedLevels, size);
   setSizeAndClear(nodeTypes, size);
}

S32 TrackLevels::size() const 
{
   return achievedLevels.size();
}

U16 TrackLevels::getNodeType(S32 idx) const 
{
   return nodeTypes[idx];
}

void TrackLevels::setNodeType(S32 idx, U8 nodeType)
{
   nodeTypes[idx] = nodeType;
}

// Achieved level is the highest set bit.  
void TrackLevels::setAchievedLevel(S32 i, S32 level)
{
   AssertFatal(i < achievedLevels.size(), "TrackLevels::setAchievedLevel()");
   
   if(level < 0) {
      achievedLevels[i] = 0;
   }
   else{
      U16   mask = (1 << level);
      AssertFatal(achievedLevels[i]==mask-1, "setAchievedLevel");
      achievedLevels[i] |= mask;
   }
}

// Note that TrackLevels maps the case of zero not being achieved into a -1 value.
S32 TrackLevels::getAchievedLevel(S32 idx) const 
{
   U16   achieved = achievedLevels[idx];
   U16   L = BitTables::getPower16(achieved);
   
   if( L-- > 0 )
      return S32(L);
   else
      return -1;
}

// There's a problem with -1 and 0 not carrying enough information.  We have some
// nodes which need to be considered as -1 for the purposes of the consolidation 
// since they don't really span a square, but we need to consider them zero below
// when we're assembling the list.  Pretty klunky...  
S32 TrackLevels::originalNodeLevel(S32 idx) const 
{
   S32 level = getAchievedLevel(idx);
   if(level == -1 && nodeTypes[idx])
      return 0;
   else
      return level;
}

void TrackLevels::capLevelAt(S32 i, U16 lev)
{
   AssertFatal(validArrayIndex(i, achievedLevels.size()), "TrackLevels::capLevelAt()");
   U16    mask = (1 << lev + 1) - 1;
   achievedLevels[i] &= mask;
}

//-------------------------------------------------------------------------------------
//       Visitor to find best consolidations 

FindBestConsolidations::FindBestConsolidations( const GridArea& gridArea, 
                                                const ConjoinConfig& thresholds, 
                                                TrackLevels& trackArrayOut 
                                                )
   :  mTrackLevels(trackArrayOut), 
      mConfigure(thresholds), 
      GridVisitor(gridArea)
{ 
   // Pre-gather terrain normal information; convert angle values to what we need
   getGridNormals();
   mDotThreshold = F32(mCos(mDegToRad(thresholds.maxAngleDev)));
}

// Go through and fetch the triangle normals from the terrain, plus compute the our
// pseudo-Euler for each normal.  
void FindBestConsolidations::getGridNormals()
{
   TerrainBlock * terr = GroundPlan::getTerrainObj();
   F32   sqrW = gNavGlobs.mSquareWidth;
   F32   stepIn = (sqrW * 0.1);
   
   Point2I  stepper;
   mGridNormals.clear();
   
   for (mArea.start(stepper); mArea.pointInRect(stepper); mArea.step(stepper))
   {
      // this is empty by default
      GridNormalInfo    info;
   
      // find out, based on split, good points to use for normal check:
      Point2F  loc(stepper.x * sqrW, stepper.y * sqrW);
      Point2F  upperCheckPt = loc;
      Point2F  lowerCheckPt = loc;
      if( (stepper.x + stepper.y) & 1 ){
         lowerCheckPt += Point2F(stepIn, stepIn);
         upperCheckPt += Point2F(sqrW - stepIn, sqrW - stepIn);
      }
      else{
         lowerCheckPt += Point2F(sqrW - stepIn, stepIn);
         upperCheckPt += Point2F(stepIn, sqrW - stepIn);
      }
  
      // get slopes and convert to the pseudo-Euler
      if (terr->getNormal(lowerCheckPt, &info.normals[0]))
         if (terr->getNormal(upperCheckPt, &info.normals[1])) {
            for (S32 both = 0; both < 2; both++) {
               info.angles[both] = info.normalToAngle(info.normals[both]);
               if (info.normals[both].z < gNavGlobs.mWalkableDot)
                  info.hasSteep = true;
            }
            info.notEmpty = true;
         }
      
      mGridNormals.push_back( info );
   }
} 

bool FindBestConsolidations::allSameType(const GridArea& R, U8& nodeType)
{
   nodeType = mTrackLevels.getNodeType(mArea.getIndex(R.point));
   Point2I  stepper;
   for (R.start(stepper); R.pointInRect(stepper); R.step(stepper)) 
      if (mTrackLevels.getNodeType(mArea.getIndex(stepper)) != nodeType)
         return false;
   return true;
}

// ==>   
// ==> This routine is where all the consolidation work happens and will eventually
// ==> do a lot more work to do more robust checks.  Example:
// ==>   We want to allow more flatness for bowls than for hills.
// ==>   We may want to consolidate more in places far from action.
// ==>   
bool FindBestConsolidations::areaIsFlat(const GridArea& R)
{
   // Area must be of same type.  If that type is submerged - then we consider 
   // it flat right away.
   U8    nodeType;
   if (!allSameType(R, nodeType))
      return false;
   else if (nodeType == GraphNodeSubmerged)
      return true;

   Point2F  minAngle(M_PI, M_PI);
   Point2F  maxAngle(0,0);
   Point2I  stepper;

   // First accumulate angle bounds, 
   for (R.start(stepper); R.pointInRect(stepper); R.step(stepper)) 
   {
      const GridNormalInfo & info = mGridNormals[ mArea.getIndex(stepper) ];
      
      //       Don't consolidate if there are non-walkable surfaces- 
      //    Actually... 
      // Turns out this makes too many nodes on some maps - we'll do this differently
      // if (info.hasSteep)
      //    return false;
      
      for (S32 triangle = 0; triangle < 2; triangle++) 
      {
         minAngle.x = getMin( info.angles[triangle].x, minAngle.x );
         minAngle.y = getMin( info.angles[triangle].y, minAngle.y );
         maxAngle.x = getMax( info.angles[triangle].x, maxAngle.x );
         maxAngle.y = getMax( info.angles[triangle].y, maxAngle.y );
      }
   }
   
   // Get the middle angle and the corresponding unit vector:  
   Point2F  medianAngle = (minAngle + maxAngle) * 0.5;
   VectorF  medianNormal = GridNormalInfo::angleToNormal (medianAngle);
   
   // Find maximum deviation from median slope.  
   F32   minDot = 1.0;
   for (R.start(stepper); R.pointInRect(stepper); R.step(stepper)) 
   {
      const GridNormalInfo & info = mGridNormals[ mArea.getIndex(stepper) ];
      for (S32 triangle = 0; triangle < 2; triangle++) {
         // == > We can check early out here, but want to watch behavior for now
         minDot = getMin( mDot(info.normals[triangle], medianNormal), minDot );
      }
   }

   // if all dot products are sufficiently close to 1, we're hopefully flat- 
   return minDot > mDotThreshold;
}

// pass one of consolidation - finds maximum possible squares.
bool FindBestConsolidations::atLevelZero(const GridArea& R)
{
   S32   idx = mArea.getIndex(R.point);
   S32   achieved = mTrackLevels.getAchievedLevel(idx);

   if( achieved < 0 )
      return false;

   AssertFatal( achieved == 0, "FindBest messed up at level zero." );
   
   return true;
}

bool FindBestConsolidations::afterDivide(const GridArea& R, S32 level, bool success)
{
   S32   idx = mArea.getIndex(R.point);
   AssertFatal( validArrayIndex(idx, mTrackLevels.size()), "Conjoin weird idx");
   
   // ==> Next:  Look at node type as part of consolidation.  Mainly Water.
   // ==>         Q: Will other volume types be important?  (i.e. fog, ..?). 
   if( success && level <= MaxConsolidateLevel )
   {
      if (areaIsFlat( R ))
      {
         Point2I stepper;    // set achieved level in all sub-squares.  
         for (R.start(stepper); R.pointInRect(stepper); R.step(stepper)) 
            mTrackLevels.setAchievedLevel( mArea.getIndex(stepper), level );
         return true;
      }
   }
   
   return false;
}

//-------------------------------------------------------------------------------------
//       VISITOR TO SMOOTH OUT LEVELS.  
//    Some of the best ones may be eliminated to satisfy condition II above.  

static GridArea getParentArea(const GridArea & R, S32 L)
{
   checkArea( R, L, "getParentArea one" );
   L = L + 1;
   Point2I  roundDownPoint((R.point.x >> L) << L, (R.point.y >> L) << L);
   Point2I  doubleTheExtent( R.extent.x << 1, R.extent.y << 1 );
   GridArea parent(roundDownPoint, doubleTheExtent);
   checkArea( parent, L, "getParentArea two" );
   return parent;
}

SmoothOutLevels::SmoothOutLevels(const GridArea& G, TrackLevels& T, S32 L)
   : mTrackLevels(T), mLevel(L), GridVisitor(G)    { }
   

// Cap all within the given area at L.     
void SmoothOutLevels::capLevelAt(const GridArea& R, S32 L)
{
   checkArea( R, L, "capping level in smoother" );
   
   Point2I  stepper;
   for (R.start(stepper); R.pointInRect(stepper); R.step(stepper)) {
      S32   index = mArea.getIndex(stepper);
      if (index >= 0)
         mTrackLevels.capLevelAt(index, L);
   }
}

bool SmoothOutLevels::allWater(const GridArea& R)
{
   Point2I  stepper;
   for (R.start(stepper); R.pointInRect(stepper); R.step(stepper)) 
      if (mTrackLevels.getNodeType(mArea.getIndex(stepper)) != GraphNodeSubmerged)
         return false;
   return true;
}

// Each pass does a different bottom level to smooth it out.  Within this routine we
// are guaranteed that we're at the bottom level for this pass.  
void SmoothOutLevels::doCurrentBottom(const GridArea& R)
{
   bool  needToCapSurrounding;
   S32   idx = mArea.getIndex(R.point);
   S32   achieved = mTrackLevels.getAchievedLevel(idx);

   // if (allWater(R))
   //    needToCapSurrounding = false;
   // else 
   {
      // At level 0, we cap if either the achieved is 0, or -1 (an empty square).
      if (mLevel == 0)
         needToCapSurrounding = (achieved <= 0);
      else
         needToCapSurrounding = (achieved == mLevel);
   }

   if (needToCapSurrounding) 
   {
      // THIS IS THE TRICKY STEP IN SMOOTHING OUT THE LEVELS!  We cap all eight of our 
      // same-sized neighbors- but we choose the cap level based on whether or not they 
      // fall inside OUR parent, or if they fall in a NEIGHBOR of our parent. If in our
      // parent, then cap at our level.  If in neighbor's parent, we cap at THAT level.
      
      GridArea    ourParent = getParentArea(R, mLevel);
      
      // Loop on all 8 neighbors:
      for(S32 y = -1; y <= 1; y++)  for(S32 x = -1; x <= 1; x++)  if (x || y)
      {
         Point2I  neighborPoint = Point2I(x << mLevel, y << mLevel) + R.point;
         S32   neighborIndex = mArea.getIndex( neighborPoint );
         if (neighborIndex >= 0) 
         {
            GridArea neighborArea(neighborPoint, R.extent);
            if (ourParent.contains(neighborArea))
               capLevelAt(neighborArea, mLevel);
            else 
            {
               GridArea    neighborParent = getParentArea( neighborArea, mLevel );
               if(mArea.contains(neighborParent)) 
                  capLevelAt(neighborParent, mLevel + 1);
            }
         }
      }
   }
}

// If the current bottom level we are looking at is the best consolidation
// that can happen for this square, go around to all neighbors and cap
// their best-so-far levels. 
bool SmoothOutLevels::atLevelZero(const GridArea& R)
{
   if(mLevel == 0) {
      doCurrentBottom(R);
      return true;
   }
   return false;
}

bool SmoothOutLevels::beforeDivide(const GridArea& R, S32 level)
{
   checkArea( R, level, "smoother before divide" );
   // AssertFatal( level >= mLevel, "Smoothing making it too far down somehow" );
   
   if( level > mLevel )          // still at high level - tell it to divide further.
      return true;
   else if(level == mLevel){     // 
      doCurrentBottom(R);
      return false;
   }
   else
      return false;
}

//-------------------------------------------------------------------------------------
//    This Visitor assembles the list into mConsolidated:

BuildConsolidated::BuildConsolidated(const GridArea& G, const TrackLevels& T, Consolidated& C)
   : GridVisitor(G), mTrackLevels(T), mConsolidated(C)
{ }

bool BuildConsolidated::checkAddToList(const GridArea& R, S32 level)
{
   checkArea(R, level, "checkAddToList");
      
   if (mTrackLevels.originalNodeLevel(mArea.getIndex(R.point)) == level)
   {
      OutdoorNodeInfo   nodeInfo;
      nodeInfo.level = level;
      nodeInfo.x = R.point.x;
      nodeInfo.y = R.point.y;
      mConsolidated.push_back(nodeInfo);
      return false;
   }  // (ret vals needed for beforeDivide(), false will stop the depth recurse)
   return true;
}
bool BuildConsolidated::beforeDivide(const GridArea& R, S32 level)
{
   return checkAddToList( R, level );
}
bool BuildConsolidated::atLevelZero(const GridArea& R)
{
   checkAddToList( R, 0 );
   return Whatever;
}

//-------------------------------------------------------------------------------------

// This routine is supplied with those grid squares that can't conjoin.  
//    The 'output' is to set up the consolidated list member variable, 
//       and return if successful.  
bool TerrainGraphInfo::buildConsolidated(const TrackLevels & whichZero, 
                                       const ConjoinConfig & configInfo)
{
   TrackLevels    trackData = whichZero;
   GridArea       gridArea(originGrid, gridDimensions);

   // pass one to find best possible consolidations
   FindBestConsolidations  findLargestPossible(gridArea,configInfo,trackData);
   findLargestPossible.traverse();
   
   // Next, enforce maximum difference of one on the levels.  
   for (S32 bottom = 0; bottom < MaxConsolidateLevel; bottom++)
   {
      // two passes needed to properly propagate the smoothing...  
      for (S32 pass = 0; pass < 2; pass++)
      {
         SmoothOutLevels doSmoothing(gridArea, trackData, bottom);
         doSmoothing.traverse();
      }
   }
   
   // Now build the node list.  The caller is responsible for making it 
   //    part of the graph.  
   BuildConsolidated  buildIt(gridArea, trackData, consolidated);
   consolidated.clear();
   buildIt.traverse();
   
   return true;
}

//-------------------------------------------------------------------------------------
// Mark bit on grid square signalling if it's near steep.  Will cause roam radii to 
// be capped so bots don't walk off slopes, and don't advance through a slope they must
// walk around.  

S32 TerrainGraphInfo::markNodesNearSteep()
{
   TerrainBlock * terr = GroundPlan::getTerrainObj();
   F32            startAng = M_PI * (15.0 / 8.0);
   F32            angDec = M_PI / 4.0;
   Point3F        loc, normal;
   S32            nSteep = 0;

   for (S32 i = 0; i < nodeCount; i++) 
   {
      // Get position- terrain system is grid aligned... 
      indexToLoc(loc, i);
      loc -= originWorld;
      
      // Could be smarter here, but it's a drop in the preprocessing bucket. Just 
      // check enough points to be sure to find if any triangle is steep.  
      for (F32 ang = startAng; ang > 0; ang -= angDec) 
      {
         Point2F  checkPos(mCos(ang) * 0.2 + loc.x, mSin(ang) * 0.2 + loc.y);
         if (terr->getNormal(checkPos, &normal))
            if (normal.z < gNavGlobs.mWalkableDot) {
               setSteep(i);
               nSteep++;
               break;
            }
      }
   }
   return nSteep;
}

//-------------------------------------------------------------------------------------
// This was being done off of run time nodes- convert to just using the grid data.  

bool TerrainGraphInfo::consolidateData(const ConjoinConfig & config)
{
   if (nodeCount <= 0)
      return false;
      
   // This is a good place to mark the steep bits on the navigableFlags.  Note it's not
   // used quite the same as the hasSteep in the consolidation, which shouldn't carry
   // over to other grid squares.  The bit does (used for roamRadii)
   markNodesNearSteep();

   // Initialize NULL track data (keeps track of conjoin levels achieved per square).
   TrackLevels levelData(nodeCount);
   const U32   checkOpenSquare = 0xff;

   // Assemble the data for the consolidator.
   for (S32 i = 0; i < nodeCount; i++)
   {
      U8    nodeType = NavGraphNotANode;
      S32   nodeLevel = -1;

      if (!obstructed(i))
      {
         // Check for shadowed - we flag like indoor as well to make them all stay
         // at the low level. We need them dense in shadow so there will be enough
         // jetting connections going up.  
         if (shadowed(i))
         {
            nodeType = NavGraphIndoorNode;
         }
         else if (submerged(i)) 
         {
            nodeType = NavGraphSubmergedNode;
            nodeLevel = 0;
         }
         else
         {
            // We have to use level -1 for things which don't have all their neighbors,
            // with one exception being on the edge of the mission area.  Here we just 
            // see if it has the neighbors it should have (returned by onSideOfArea()).
            nodeType = NavGraphOutdoorNode;
            if (const U32 neighbors = onSideOfArea(i)) {
               if (neighborFlags[i] == neighbors)
                  nodeLevel = 0;
            }
            else if (neighborFlags[i] == checkOpenSquare)
               nodeLevel = 0;
         }
         
         levelData.setAchievedLevel(i, nodeLevel);
         levelData.setNodeType(i, nodeType);
      }
   }
   
   bool Ok = buildConsolidated(levelData, config);
   
   return Ok;
}
