//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

#define  MuzzleHt    0.82f
#define  HeadHt      2.2f


//-------------------------------------------------------------------------------------

// Want to see stuff on screen, so we scamble node order (bunch of random swaps)
static void makeScrambler(Vector<S32>& vec, S32 size)
{
   S32         i, i1, i2, count = size * 10;

   for (i = 0, vec.setSize(size); i < size; i++)
      vec[i] = i;
      
   while (--count >= 0) 
   {
      S32   t = vec[i1 = (gRandGen.randI()&0xFFFFFF) % size];
      vec[i1] = vec[i2 = (gRandGen.randI()&0xFFFFFF) % size];
      vec[i2] = t;
   }
}

static RayInfo sColl;
static bool haveLOS(Point3F src, Point3F dst, F32 above)
{
   static const U32 sMask = InteriorObjectType|TerrainObjectType;
   src.z += above;
   dst.z += above;
   return !gServerContainer.castRay(src, dst, sMask, &sColl);
}
      
class MakeLOSEntries : public GraphSearch
{
      const GraphNodeList& mList;
      LOSXRefTable&        mLOSTable;
      S32                  mCurrent;
      GraphNode         *  mFromNode;
      Point3F              mFromLoc;
      F32                  mThreshDist;
      U32                  mStartTime, mSaveMS;
      S32                  mLowButNotHigh;
      S32                  mFromIndex;
      Vector<S32>          mScramble;
      S32                  mLOSCalls;
      
   public:
      Vector<LineSegment>  mRenderSegs;
      Point3F              mViewLoc;
   
   protected:  
      // virtuals- 
      void  onQExtraction();
      F32   getEdgeTime(const GraphEdge*);
      bool  earlyOut();

   public:
      MakeLOSEntries(const GraphNodeList& list, LOSXRefTable& xRef, Point3F view);

      bool  isDone() const          {return mCurrent>=mList.size();}
      U32   elapsedTime()  const    {return Platform::getRealMilliseconds()-mSaveMS;}
      S32   lowNotHigh()  const     {return mLowButNotHigh;}
      S32   numLOSCalls()  const    {return mLOSCalls;}
      bool  nextOneReady();
      void  workAWhile();
};

MakeLOSEntries::MakeLOSEntries(const GraphNodeList& list, LOSXRefTable& los, Point3F v)
   :  mList(list), mLOSTable(los), mViewLoc(v)
{
   S32   N = list.size();
   mLOSTable.setDims(N * (N + 1) >> 1, 2);      // Alloc our 2-bit table
   mViewLoc.set(-44,-31,90);
   mThreshDist = 700.0f;
   mCurrent = -1;
   mSaveMS = Platform::getRealMilliseconds();
   mLOSCalls = mLowButNotHigh = 0;
   makeScrambler(mScramble, list.size());
}

// Run LOS from base to this node.  We truncate the search at a certain path distance, 
// which should make sense in all but a few cases that we should be aware of.  It 
// will make the search quicker for most worlds though.  
void MakeLOSEntries::onQExtraction()
{
   S32   toIndex = extractedNode()->getIndex();
   
   if (mFromIndex < toIndex) {
      //==> Path distance really not right- just need a better local query.  
      if (distSoFar() < mThreshDist) {
         Point3F  toLoc = extractedNode()->location();
         bool     losLow = haveLOS(mFromLoc, toLoc, MuzzleHt);
         bool     losHigh = haveLOS(mFromLoc, toLoc, HeadHt);
         U32      tabEntry = LOSXRefTable::Hidden;

         // Speed tracking         
         mLOSCalls += 2;
      
         // These are strange- add to list of warning log for graph maker
         mLowButNotHigh += (losLow && !losHigh);
         
         // Crude calc of entry for now- 
         if(losLow)
            tabEntry = LOSXRefTable::FullLOS;
         else if (losHigh)
            tabEntry = LOSXRefTable::MinorLOS;

         // Enter into table-             
         mLOSTable.setEntry(mFromIndex, toIndex, tabEntry);

         // Stuff to render-       
         if (mRenderSegs.size() < 600) {
            LineSegment segment(mFromLoc, tabEntry ? toLoc : sColl.point);
            if (segment.distance(mViewLoc) < 120.0f)
               mRenderSegs.push_back(segment);
         }
      }
      else // outside
         setDone();
   }
}

// Virtual - this will cause graph time to equal distance.  
F32 MakeLOSEntries::getEdgeTime(const GraphEdge * edge)
{
   return edge->mDist;
}

// Prepare a new search.  Note mCurrent constructs at -1 for proper entry.  
bool MakeLOSEntries::nextOneReady()
{
   if (++mCurrent < mList.size()) {
      mFromNode = mList[mScramble[mCurrent]];
      mFromLoc = mFromNode->location();
      mFromIndex = mFromNode->getIndex();
      return true;
   }
   return false;
}

#define  MillisecondWorkShift    90

// Virtual - runSearch() can be stopped in the middle.  
bool MakeLOSEntries::earlyOut()
{
   U32   timeElapsed = (Platform::getRealMilliseconds() - mStartTime);
   return (timeElapsed > MillisecondWorkShift);
}

void MakeLOSEntries::workAWhile()
{
   mStartTime = Platform::getRealMilliseconds();
   
   while (!isDone() && !earlyOut())
      if (inProgress())
         runSearch(mFromNode);
      else if (nextOneReady())
         runSearch(mFromNode);

   NavigationGraph::sProcessPercent = F32(mCurrent) / F32(mList.size());
}

bool NavigationGraph::prepLOSTableWork(Point3F viewLoc)
{
   if (mTableBuilder) {
      delete mTableBuilder;
      mTableBuilder = NULL;
   }
   
   mTableBuilder = new MakeLOSEntries(mNonTransient, mLOSXRef, viewLoc);
   return true;
}

// This gets called repeatedly until the table is built.  Idea here is to slice
// the process so that some rendering can occur during this lengthy process.  
bool NavigationGraph::makeLOSTableEntries()
{
   MakeLOSEntries *  makeEntries = dynamic_cast<MakeLOSEntries*>(mTableBuilder);
   AssertFatal(makeEntries, "Graph preprocess:  prepLOSTable() needed");
   
   makeEntries->workAWhile();

   mRenderThese = makeEntries->mRenderSegs;
   makeEntries->mRenderSegs.clear();
   
   if (makeEntries->isDone()) {
      clearRenderSegs();
      Con::printf("Total table size = %d", mLOSXRef.numBytes());
      Con::printf("Performed %d LOS calls", makeEntries->numLOSCalls());
      Con::printf("Elapsed time = %d milliseconds", makeEntries->elapsedTime());
      if (makeEntries->lowNotHigh()) 
         Con::printf("%d Low-not-high entries found", makeEntries->lowNotHigh());
      delete mTableBuilder;
      mTableBuilder = NULL;
      // Convert data to better hasher- 
      makeLOSHashTable();
      return false;
   }
   return true;
}
