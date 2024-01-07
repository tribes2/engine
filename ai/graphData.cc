//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graphData.h"
#include "terrain/terrRender.h"

//    4  6  7        Layout for how we translate the 
//    2  N  5        numbers 0 to 7 into the eight 
//    0  1  3        surrounding grid locations.  
const Point2I TerrainGraphInfo::gridOffs[8] = { 
            Point2I( -1, -1 ),           
      Point2I( 0, -1 ), Point2I( -1, 0 ),
      Point2I( 1, -1 ), Point2I( -1, 1 ),
      Point2I( 1,  0 ), Point2I(  0, 1 ),
               Point2I( 1, 1 )   };      

S32 TerrainGraphInfo::smVersion = 0;


//-------------------------------------------------------------------------------------
//                            Indoor Graph Edges

bool GraphEdgeInfo::isAlgorithmic() const {
   return to[0].flags.test(Algorithmic);
}
void GraphEdgeInfo::setAlgorithmic(bool yesOrNo) {
   to[0].flags.set(Algorithmic, yesOrNo);
}

GraphEdgeInfo::GraphEdgeInfo()
{
   segNormal.set(0,0,1);
   segPoints[0] = segPoints[1] = segNormal;
}

bool GraphEdgeInfo::read(Stream & s)
{
   for( S32 i = 0; i < 2; i++ ){
      U32   mask;
      if( s.read(&mask) ){
         to[i].flags = mask;
         if( s.read(&to[i].res) && s.read(&to[i].dest) )
            continue;
         return false;
      }
   }
   
   if (mathRead(s, &segPoints[0]))
      if (mathRead(s, &segPoints[1]))
         if (mathRead(s, &segNormal))
            return true;
   
   return false;
}
bool GraphEdgeInfo::write(Stream & s) const 
{
   for( S32 i = 0; i < 2; i++ ){
      U32   mask = to[i].flags;
      if( s.write(mask) && s.write(to[i].res) && s.write(to[i].dest) )
         continue;
      return false;
   }
   
   if (mathWrite(s, segPoints[0]))
      if (mathWrite(s, segPoints[1]))
         if (mathWrite(s, segNormal))
            return true;
   
   return false;
}

//-------------------------------------------------------------------------------------
// Indoor Nodes

IndoorNodeInfo::IndoorNodeInfo()
{
   pos.set(-1,-1,-1);
   antecedent = -1;
   unused = 0;
}

// The boolean setters & getters.  Avoiding inlines since these should check 
// invariant relationships among the flags.  
bool IndoorNodeInfo::isAlgorithmic() const 
{
   return flags.test( Algorithmic );
}

void IndoorNodeInfo::setAlgorithmic(bool yesOrNo) 
{
   flags.set( Algorithmic, yesOrNo );
}

bool IndoorNodeInfo::isInventory() const 
{
   return flags.test(Inventory);
}

void IndoorNodeInfo::setInventory(bool yesOrNo) 
{
   flags.set(Inventory, yesOrNo);
}

bool IndoorNodeInfo::isBelowPortal() const 
{
   return flags.test(BelowPortal);
}

void IndoorNodeInfo::setBelowPortal(bool yesOrNo) 
{
   flags.set(BelowPortal, yesOrNo);
}

bool IndoorNodeInfo::isSeed() const 
{
   return flags.test(Seed);
}

void IndoorNodeInfo::setSeed(bool yesOrNo) 
{
   flags.set(Seed, yesOrNo);
}

bool IndoorNodeInfo::read(Stream & s)
{
   U32   mask;
   if (s.read(&mask)) 
   {
      flags = mask;
      return s.read(&unused) && s.read(&antecedent) && mathRead(s, &pos);
   }
   return false;
}
bool IndoorNodeInfo::write(Stream & s) const
{
   U32   mask = flags;
   if (s.write(mask))
      return s.write(unused) && s.write(antecedent) && mathWrite(s, pos);
   return false;
}

//-------------------------------------------------------------------------------------
//          Outdoor (consolidated) node information

OutdoorNodeInfo::OutdoorNodeInfo()
{
   level = 0xff;
   flags = 0;
   height = 0;    //==> Not used - can be taken out.  
   x = y = -1;
}

bool OutdoorNodeInfo::read(Stream& s) 
{
   return   s.read(&level)    &&
            s.read(&flags)    &&
            s.read(&height)   &&
            s.read(&x)        &&
            s.read(&y)
         ;
}

bool OutdoorNodeInfo::write(Stream & s) const 
{
   return   s.write(level)    &&
            s.write(flags)    &&
            s.write(height)   &&
            s.write(x)        &&
            s.write(y)
         ;
}

//-------------------------------------------------------------------------------------

SpawnLocations::SpawnLocations()
{
   reset();
}

void SpawnLocations::reset()
{
   clear();
   compact();
   mSpheres.clear();
   mSpheres.compact();
   mRes0 = 0;
}

bool SpawnLocations::read(Stream& s)
{
   if (s.read(&mRes0))
      if (mathReadVector(*this, s))
         return readVector1(s, mSpheres);
   return false;
}

bool SpawnLocations::write(Stream& s) const 
{
   if (s.write(mRes0))
      if (mathWriteVector(*this, s))
         return writeVector1(s, mSpheres);
         
   return false;
}

SpawnLocations::Sphere::Sphere(const Point3F& center, F32 radius)
   :  mSphere(center, radius)
{
   mInside = false;
   mCount = 0;
   mOffset = 0;
   mRes0 = 0;
}

bool SpawnLocations::Sphere::read(Stream& s)
{
   if (s.read(&mRes0))
      if (mathRead(s, &mSphere))
         if (s.read(&mInside))
            if (s.read(&mCount))
               return s.read(&mOffset);
   return false;
}

bool SpawnLocations::Sphere::write(Stream& s) const 
{
   if (s.write(mRes0))
      if (mathWrite(s, mSphere))
         if (s.write(mInside))
            if (s.write(mCount))
               return s.write(mOffset);
   return false;
}

// Lookup a random location from our pre-computed data.  
S32 SpawnLocations::getRandom(const SphereF& sphere, bool inside, U32 rnd)
{
   // Find closest sphere matching inside/outside requirement:
   const Sphere   *  closest = NULL;
   F32               minDist = 1e22, d;
   for (S32 i = 0; i < mSpheres.size(); i++)
   {
      const Sphere * S = & mSpheres[i];
      if (S->mInside == inside && S->mCount > 0)
         if ((d = (S->mSphere.center - sphere.center).lenSquared()) < minDist)
         {
            closest = S;
            minDist = d;
         }
   }
   
   // Get the random index in the list.  
   if (closest)
   {
      U32   count = closest->mCount;
      return (closest->mOffset + ((rnd & 0x7FFFFF) % count));
   }
   else
      return -1;
}

void SpawnLocations::printInfo() const
{
   Con::printf("--- %d Spheres were generated", mSpheres.size());
   for (S32 i = 0; i < mSpheres.size(); i++)
   {
      const Sphere   *  S = & mSpheres[i];
      const Point3F  &  P = S->mSphere.center;
      
      Con::printf("%d : %s at center (%f, %f, %f) - generated %d", 
            i + 1, 
            S->mInside ? "Inside" : "Outside", 
            P.x, P.y, P.z, 
            S->mCount
         );
   }
}

//-------------------------------------------------------------------------------------
// Potential bridges.  

void GraphBridgeInfo::init(S32 from, S32 to)
{
   srcNode = from;
   dstNode = to;
   jetClear = 0;
   howTo = 0;
   res1 = 0;
   res2 = 0;
   res3 = 0;
}

bool GraphBridgeInfo::read(Stream& s)
{
   return
   (  s.read(&dstNode)  && 
      s.read(&srcNode)  && 
      s.read(&jetClear) && 
      s.read(&howTo)    && 
      s.read(&res1)     &&
      s.read(&res2)     &&
      s.read(&res3)
   );
}

bool GraphBridgeInfo::write(Stream & s) const 
{
   return
   (  s.write(dstNode)  && 
      s.write(srcNode)  && 
      s.write(jetClear) && 
      s.write(howTo)    && 
      s.write(res1)     &&
      s.write(res2)     &&
      s.write(res3)
   );
}

//-------------------------------------------------------------------------------------
//                   Graph Bridge Data

void GraphBridgeData::init(U16 n0, U16 n1)
{
   nodes[0] = n1;       // for some reason, these are backwards, which wound up causing
   nodes[1] = n0;       // a problem for the replacement edges.  The quick remedy is to 
   jetClear = 0;        // change the edge replacement ON LOAD (graphJetting.cc) rather
   howTo = 0;           // than rebuild just to change a non-intuitive ordering here...
}

bool GraphBridgeData::read(Stream& s)
{
   return(  s.read(&nodes[0])    && 
            s.read(&nodes[1])    && 
            s.read(&jetClear)    && 
            s.read(&howTo) 
         );
}

bool GraphBridgeData::write(Stream & s) const 
{
   return(  s.write(nodes[0])    && 
            s.write(nodes[1])    && 
            s.write(jetClear)    && 
            s.write(howTo)
         );
}

BridgeDataList::BridgeDataList()
{
   mReplaced[0] = mReplaced[1] = 0;
   mSaveTotal = 0;
}

bool BridgeDataList::read(Stream& s)
{
   mSaveTotal = 0;
   if (readVector1(s, *this))
   {
      mSaveTotal = size();
      return true;
   }
   return false;
}

bool BridgeDataList::write(Stream& s) const
{
   return writeVector1(s, *this);
}

// Given array of accumulation numbers for how many edges each node has, add
// in the bridges as well.  
S32 BridgeDataList::accumEdgeCounts(U16 * edgeCounts)
{
   S32   total = 0;
   mReplaced[0] = mReplaced[1] = 0;
   
   for (iterator it = begin(); it != end(); it++)
   {
      if (!it->isReplacement()) 
      {
         total += 2;
         for (S32 i = 0; i < 2; i++)
            edgeCounts[it->nodes[i]]++;
      }
      else
      {
         mReplaced[it->isUnreachable() != 0]++;
      }
   }
         
   return total;
}

// Number of non-replacement bridges- basically how many edges it adds 
// to the world (this is just used for memory tracking).  
S32 BridgeDataList::numPositiveBridges() const 
{
   return (mSaveTotal << 1) - replaced();
}

S32 BridgeDataList::replaced() const
{
   return mReplaced[0] + mReplaced[1];
}

// Read the old style and convert
#if 0
bool BridgeDataList::readOld(Stream& s)
{
   BridgeInfoList    oldList;
   
   if (oldList.read(s))
   {
      S32   oldSz = oldList.size();
      AssertFatal((oldSz % 2) == 0, "Old sizes were always even");
      setSize(oldSz >>= 1);

      // Old format very bloated and stores both ways, which turned out not to have any
      // differences right now (later systems, maybe...).
      while (oldSz--)
      {
         GraphBridgeInfo&  bridgeOld = oldList[oldSz << 1];
         GraphBridgeData&  bridgeNew = (*this)[oldSz];
         
         bridgeNew.nodes[0] = U16(bridgeOld.srcNode);
         bridgeNew.nodes[1] = U16(bridgeOld.dstNode);
         bridgeNew.howTo = bridgeOld.howTo;
         bridgeNew.jetClear = mapJetHopToU8(bridgeOld.jetClear);
      }
      return true;
   }
   return false;
}
#endif

//-------------------------------------------------------------------------------------

bool GraphVolumeList::read(Stream& s)
{  
   return readVector1(s, *this) && mathReadVector(mPlanes, s);
}

bool GraphVolumeList::write(Stream& s) const
{
   return writeVector1(s, *this) && mathWriteVector(mPlanes, s);
}

//-------------------------------------------------------------------------------------

S32 ChuteHints::findNear(const Box3F& box, ChutePtrList& list) const 
{
   list.clear();
   return mBSP.getIntersecting(list, box);
}

// Patch for the query we usually do- 
S32 ChuteHints::findNear(const Point3F& P, S32 xy, S32 z, ChutePtrList& list) const
{
   Point3F  boxMin(P.x - xy, P.y - xy, P.z - z);
   Point3F  boxMax(P.x + xy, P.y + xy, P.z + z);
   Box3F    theBox(boxMin, boxMax, true);
   return findNear(theBox, list);
}

// Create the BSP.  makeTree() needs a pointer list, so we must build one.
void ChuteHints::makeBSP()
{
   VectorPtr<ChuteHint *>  pointers;
   pointers.setSize(size());
   iterator c = begin();
   for (S32 i = size() - 1; i >= 0; i--)
      pointers[i] = c++;
   mBSP.makeTree(pointers);
}

void ChuteHints::init(const Vector<Point3F>& list)
{
   setSize(list.size());
   Vector<Point3F>::const_iterator p = list.begin();
   iterator c = begin();
   for (S32 i = list.size() - 1; i >= 0; i--, p++, c++) {
      c->x = p->x;
      c->y = p->y;
      c->z = p->z;
   }
   makeBSP();
}

bool ChuteHints::read(Stream& s)
{
   if (readVector1(s, *this)) {
      makeBSP();
      return true;
   }
   return false;
}

bool ChuteHints::write(Stream& s) const
{
   return writeVector1(s, *this);
}

// Given the verticle line of a (potential) jetting hop, see if it is one of the 
// chute types.  Build a box up as far as LOS goes to do this.  We're at the 
// top if the highest found chute hint is level with the given top point.  
ChuteHints::Info ChuteHints::info(Point3F bottom, const Point3F& top)
{
   const F32   boxR = 1.4;
   
   if (mFabs(top.z - bottom.z) > 4.0 * /*Take out for moment-*/1e9) { 
      // Do these queries as a first pass to avoid LOS calls- 
      if (findNear(bottom, boxR, 2.0, mQuery) > 0) {
         if (findNear(top, boxR, 2.0, mQuery) > 0) {
         
            bottom.z += 1.0;
         
            const U32   mask = (InteriorObjectType|TerrainObjectType);
            const F32   maxH = 500.0;
            Point3F     upper(bottom.x, bottom.y, bottom.z + maxH);
            RayInfo     coll;
         
            if (gServerContainer.castRay(bottom, upper, mask, &coll))
               upper.z = coll.point.z;
            
            Point3F  boxMin(bottom.x - boxR, bottom.y - boxR, bottom.z - 2.0);
            Point3F  boxMax(upper.x + boxR, upper.y + boxR, upper.z);
            Box3F    theBox(boxMin, boxMax, true);

            // Well, this should always return something at this point.. 
            if (S32 numHints = findNear(theBox, mQuery)) {
               // Find max- 
               F32   maxZ = -1e12, z;
               for (S32 i = 0; i < numHints; i++)
                  if ((z = mQuery[i]->z) > maxZ) 
                     maxZ = z;
            
               if (mFabs(maxZ - top.z) < 2.0)
                  return ChuteTop;
               else
                  return ChuteMid;
            }
         }
      }
   }
   return NotAChute;
}

//-------------------------------------------------------------------------------------

bool PathXRefEntry::read(Stream& s)
{
   U32   numEntries, bitWidth;
   if (s.read(&numEntries) && s.read(&bitWidth)) {
      setDims(numEntries, bitWidth);
      return s.read(numBytes(), dataPtr());
   }
   return false;
}

bool PathXRefEntry::write(Stream& s) const
{
   if (s.write(numEntries()) && s.write(bitWidth()))
      return s.write(numBytes(), dataPtr());
   return false;
}

PathXRefTable::~PathXRefTable()
{
   clear();
}

bool PathXRefTable::read(Stream& s)
{
   return constructVector(s, *this);
}

bool PathXRefTable::write(Stream& s) const
{
   return writeVector1(s, *this);
}

void PathXRefTable::setSize(S32 size)
{  
   clear();
   setSizeAndConstruct(*this, size);
}

void PathXRefTable::clear()
{
   destructAndClear(*this);
}

//-------------------------------------------------------------------------------------

bool LOSXRefTable::read(Stream& s)
{
   U32   numEntries, bitWidth;
   if (s.read(&numEntries) && s.read(&bitWidth)) {
      setDims(numEntries, bitWidth);
      return s.read(numBytes(), dataPtr());
   }
   return false;
}

bool LOSXRefTable::write(Stream& s) const
{
   if (s.write(numEntries()) && s.write(bitWidth()))
      return s.write(numBytes(), dataPtr());
   return false;
}

U32 LOSXRefTable::value(S32 i1, S32 i2) const
{
   if (i1 == i2)
      return FullLOS;
   else
   {
      U32   lookup = triangularTableIndex(i1, i2);
      AssertFatal(lookup < numEntries(), "LOSXRefTable::value() read beyond end");
      return getU17(lookup);
   }
}

void LOSXRefTable::setEntry(S32 i1, S32 i2, U32 val)
{
   setU17(triangularTableIndex(i1, i2), val);
}

bool LOSXRefTable::valid(S32 numNodes) const
{
   S32   expectedTableSize = triangularTableIndex(numNodes + 1, 0);
   return (numEntries() == expectedTableSize);
}

// Reset, free up memory.  
void LOSXRefTable::clear()
{
   setDims(0, 0);
}

//-------------------------------------------------------------------------------------
//                                  LOS Hash Table 

LOSHashTable::LOSHashTable()
{
   mTable = NULL;
   mNumNodes = 0;
   mRes0 = mRes1 = 0;
}

LOSHashTable::~LOSHashTable()
{
   delete [] mTable;
}

bool LOSHashTable::valid(S32 numNodes) const
{
   return (numNodes == mNumNodes);
}

// Do the node-to-node lookup.  
U32 LOSHashTable::value(S32 i1, S32 i2) const
{
   if (i1 == i2)
      return FullLOS;
   else 
   {
      U16   from, to, seg;
      
      if (i1 < i2) {       // Lookup goes UP
         from = i1;
         seg = ((to = i2) >> SegShift);
      }
      else {
         from = i2;
         seg = ((to = i1) >> SegShift);
      }
      
      Key   key(from, seg);
      U32   hash = calcHash(key);
      AssertFatal(hash < mTabSz, "LOSHashTable: bad hash value in lookup");

      // Do the bucket "walk" - 
      if (S32  bucketSize = mTable[hash + 1] - mTable[hash]) 
      {
         const Segment * seg = &mSegments[mTable[hash]];
         while (--bucketSize >= 0) 
         {
            if (seg->mKey.mCompare == key.mCompare)
               return seg->value(to & SegMask);
            seg++;
         }
      }
      
      // Nothing found, so it's hidden
      return Hidden;
   }
}

U32   LOSHashTable::mTabSz = 0;

// This needs to be improved (hence we sort after loading as well...)
U32 LOSHashTable::calcHash(Key key)
{
   U32   n = key.mKey.mNode;
   U32   s = key.mKey.mSeg;
   U32   val = (n << 8 + (s & 7)) ^ (n + (n >> 1)) ^ ((s << 2) | (s & 1));
   return val % mTabSz;
}

// Look for size that is relatively prime to first few primes (uh, can't hurt...)
U32 LOSHashTable::calcTabSize()
{
   static const U32 nPrimes = 12;
   static U32 primes[nPrimes] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

   for (U32 nSegs = mSegments.size(); /* until we find one */ ; nSegs++ )
   {
      bool  done = true;
      
      for (U32 i = 0; i < nPrimes; i++)
         if (!(nSegs % primes[i]))
            done = false;
      
      if (done) {
         mTabSz = nSegs;
         break;
      }
   }
   return mTabSz;
}

S32 QSORT_CALLBACK LOSHashTable::cmpHashes(const void * a,const void * b)
{
   U32   A = calcHash( ((Segment*)a)->mKey );
   U32   B = calcHash( ((Segment*)b)->mKey );
   AssertFatal(A < mTabSz && B < mTabSz, "LOSHashTable: hash has bad result");
   return (A < B ? -1 : (A > B ? 1 : 0));
}

// This assumes the table size has been calculated- 
void LOSHashTable::sortByHashVal()
{
   dQsort((void* )(mSegments.address()), mSegments.size(), sizeof(Segment), cmpHashes);
}

LOSHashTable::Segment::Segment(Key key, const U32 losInfo[SegSize])
{
   mKey = key;
   dMemset(mLOS, 0, sizeof(mLOS));
   for (U32 i = 0; i < SegAlloc; i++)
      for (U32 j = 0; j < 4; j++)
         mLOS[i] |= (losInfo[i * 4 + j] << (j * 2));
}

// Build the new better table from the old style.  Return memory usage (for measuring, 
// but it's also conceivable that we could try different seg sizes dynamically).  
U32 LOSHashTable::convertTable(const LOSXRefTable& XRefTable, S32 numNodes)
{
   S32   numUniform = 0;
   S32   numSameSeg = 0;

   mNumNodes = numNodes;
   mSegments.clear();
   
   for (S32 srcNode = 0; srcNode < numNodes - 1; srcNode++)
   {
      // Check all other segments (including ours if we're not the last in it). Note
      // inclusive compare to catch last segment!  
      for (S32 seg = (srcNode + 1 >> SegShift); seg <= (numNodes >> SegShift); seg++)
      {
         S32   begin = (seg << SegShift);
         S32   end = begin + SegSize;
         
         if (end > numNodes)
            end = numNodes;

         // If in our own segment, only check node numbers higher...  
         bool  sameSeg = (seg == (srcNode >> SegShift));
         if (sameSeg)
         {
            begin = srcNode + 1;
            end = ((srcNode + SegSize) & ~SegMask);
            if (end > numNodes)
               end = numNodes;
         }

         // Examine all the nodes in the segment for non-hidden
         S32   losCount = 0;
         U32   losInfo[SegSize];
         dMemset(losInfo, 0, sizeof(losInfo));
         for (U32 dstNode = begin; dstNode < end; dstNode++)
         {
            U32 info = XRefTable.value(U16(srcNode), U16(dstNode));
            losInfo[dstNode & SegMask] = info;
            losCount += (info != 0);
         }

         // Add the segment if something was seen.           
         if (losCount)
         {             
            Key      key(srcNode, seg);
            Segment  segment(key, losInfo);
            mSegments.push_back(segment);
            
            // See if they're all the same
            S32   a;
            for (a = 1; a < SegSize; a++)
               if (losInfo[a] != losInfo[0])
                  break;
            numUniform += (a == SegSize);
            numSameSeg += sameSeg;
         }
      }
   }
   
   Con::printf("Found %d Uniform entries", numUniform);
   Con::printf("Found %d SameSeg entries", numSameSeg);

   // Sorts segment vector-    
   calcTabSize();
   sortByHashVal();
   
   // Set up mTable
   U32 tabMemUse = makeTheTable();

   return (mSegments.memSize() + tabMemUse);
}

// Given that we've got a vector of segments sorted by hash value, make mTable.
// Assumes table size has already been calculated.  
//    Return memory size of table.  
U32 LOSHashTable::makeTheTable()
{
   U32   deepest = 0, depth;

   // Alloc and flag as uninitialized.  Extra 1 needed for bucket size math at end.  
   delete [] mTable;
   mTable = new IndexType[ mTabSz + 1 ];
   U32   memSize = (mTabSz + 1) * sizeof(IndexType);
   dMemset(mTable, 0xFF, memSize);
   
   // Set up hash pointers- 
   for (U32 seg = 0; seg < mSegments.size(); seg++)
   {
      U32   hash = calcHash(mSegments[seg].mKey);
      if (mTable[hash] == IndexType(-1))
         mTable[hash] = seg;
      else if ((depth = seg - mTable[hash]) > deepest)
         deepest = depth;
   }
   
   // Fill in empty elements so bucket size math works
   AssertFatal(mTable[mTabSz] == IndexType(-1), "LOS Hash table overwritten at end");
   U32   fillValue = mSegments.size();
   U32   numEmpty = 0, numFilled = 0;
   for (U32 H = mTabSz; H; H--) 
   {
      if (mTable[H] == IndexType(-1)) {
         mTable[H] = fillValue;
         numEmpty++;
      }
      else {
         fillValue = mTable[H];
         numFilled++;
      }
   }
   
   Con::printf("Table size = %d,    Segments = %d,    Filled = %d,  Deepest = %d", 
                  mTabSz,        mSegments.size(),    numFilled,     deepest );

   return memSize;
}

bool LOSHashTable::Segment::read(Stream& s)
{
   if (s.read(&mKey.mKey.mNode) && s.read(&mKey.mKey.mSeg))
      return s.read(SegAlloc, mLOS);
   return false;
}

bool LOSHashTable::Segment::write(Stream& s) const
{
   if (s.write(mKey.mKey.mNode) && s.write(mKey.mKey.mSeg))
      return s.write(SegAlloc, mLOS);
   return false;
}

void LOSHashTable::clear()
{
   mNumNodes = 0;
   delete [] mTable;
   mTable = NULL;
   mSegments.clear();
   mSegments.compact();
}

bool LOSHashTable::read(Stream& s)
{
   if (s.read(&mNumNodes))
      if (readVector1(s, mSegments))
         if (s.read(&mRes0) && s.read(&mRes1))
         {
            calcTabSize();    
            sortByHashVal();     // For now we sort Segments on load since we're still 
            makeTheTable();      //       tweaking the hash function
            return true;
         }
   return false;
}

bool LOSHashTable::write(Stream& s) const
{
   if (s.write(mNumNodes))
      if (writeVector1(s, mSegments))
         if (s.write(mRes0) && s.write(mRes1))
            return true;
   return false;
}

//-------------------------------------------------------------------------------------
// Header info for the Terrain Data.

TerrainGraphInfo::TerrainGraphInfo()
{
   haveGraph = false;
   dMemset(indOffs, 0xff, sizeof(indOffs));
   nodeCount = numShadows = -1;
   originWorld.set(-1,-1,-1);
   originGrid.set(-1,-1);
   gridDimensions = gridTopRight = originGrid;
}

bool TerrainGraphInfo::read(Stream & s)
{
   S32   version;

   haveGraph = false;
   
   if( !s.read(&version) || version != smVersion )
      return false;
   
   bool  Ok =  s.read( & nodeCount)             &&
               mathRead(s, & originWorld)       &&
               mathRead(s, & originGrid)        &&
               mathRead(s, & gridDimensions)    &&
               mathRead(s, & gridTopRight)      &&
               readVector2(s, navigableFlags)   &&
               readVector2(s, neighborFlags)    &&
               readVector2(s, shadowHeights)    && 
               readVector2(s, roamRadii)        &&
               consolidated.read(s)
               ;

   if( Ok )
      doFinalDataSetup();
      
   return( haveGraph = Ok );
}

bool TerrainGraphInfo::write(Stream & s) const
{
   bool  Ok =  s.write( smVersion )             &&
               s.write( nodeCount)              &&
               mathWrite(s, originWorld)        &&
               mathWrite(s, originGrid)         &&
               mathWrite(s, gridDimensions)     &&
               mathWrite(s, gridTopRight)       &&
               writeVector2(s, navigableFlags)  &&
               writeVector2(s, neighborFlags)   &&
               writeVector2(s, shadowHeights)   &&
               writeVector2(s, roamRadii)       &&
               consolidated.write(s)
               ;

   return Ok;
}

//------------------------------------------------------------------------------
// Terrain graph info utility functions.  


// Get the index of the given grid location if it's in our region, else -1.  
//  This doesn't check for obstruction though.  
S32 TerrainGraphInfo::posToIndex(Point2I p) const
{
   S32   idx = -1;

   if( haveGraph )
   {
      p -= originGrid;

      if( validArrayIndex(p.x, gridDimensions.x) )
         if( validArrayIndex(p.y, gridDimensions.y) )
            idx = p.y * gridDimensions.x + p.x;
         
      AssertFatal( idx >= -1 && idx < nodeCount, "TerrainGraphInfo::posToIndex()" );
   }
   
   return idx;
}

// Fetch the location at the given grid location - a straight mapping
//    to world space, whether it's a valid node or not. But then return
//       if it's a valid node.  
bool TerrainGraphInfo::posToLoc(Point3F& loc, const Point2I& p)
{
   S32   index    = posToIndex(p);
   bool  isValid  = (index >= 0 && !obstructed(index));
   
   loc.x = F32(p.x << gNavGlobs.mSquareShift);
   loc.y = F32(p.y << gNavGlobs.mSquareShift);
   loc.z = 0.0f;                 // need actual position at some point.
   loc += originWorld;
   
   return isValid;
}



// Map the given world coordinate into our graph grid.  
// Sort of a worldToGrid() routine.  
S32 TerrainGraphInfo::locToIndex(Point3F loc) const
{
   loc += gNavGlobs.mHalfSquare; 
   loc -= originWorld;
   loc *= gNavGlobs.mInverseWidth;
   return posToIndex( Point2I( mFloor(loc.x), mFloor(loc.y)) );
}

// Get the grid location.  Returning a pointer since we may 
// handle bad indices at some point with a NULL return value. 
Point3F * TerrainGraphInfo::indexToLoc(Point3F & loc, S32 index)
{
   Point2I  & pos = indexToPos(index);
   
   loc.x = F32(pos.x << gNavGlobs.mSquareShift);
   loc.y = F32(pos.y << gNavGlobs.mSquareShift);
   loc.z = 0.0f;           
   
   loc += originWorld;

   return & loc;
}

bool TerrainGraphInfo::obstructed(const Point3F& loc) const 
{ 
   S32   index = locToIndex(loc);
   return (index < 0) || obstructed(index);
}

Point2I & TerrainGraphInfo::indexToPos(S32 index) const 
{
   static Point2I sPoint;
   sPoint.x = originGrid.x + index % gridDimensions.x;
   sPoint.y = originGrid.y + index / gridDimensions.x;
   return sPoint;
}


S32 TerrainGraphInfo::locToIndexAndSphere(SphereF & sphereOut, const Point3F & loc)
{
   if( roamRadii.size() )
   {
      S32   idx = locToIndex(loc);
      if( validArrayIndex(idx, roamRadii.size()) && !obstructed(idx) )
      {
         indexToLoc(sphereOut.center, idx);
         sphereOut.radius = fixedToFloat(roamRadii[ idx ]);
         if( sphereOut.isContained(loc) )
            return idx;
      }
   }
   return -1;
}

// This function assumes that the grid and navigable flags have been set up.  
// 
void TerrainGraphInfo::computeRoamRadii()
{
   Vector<Point2I>   spiral;
   Vector<F32>       dists;
   
   S32   radius = getMin( getMax(gridDimensions.x, gridDimensions.y), 20);
   S32   scount = getGridSpiral(spiral, dists, radius);
   S32   n, s;
   
   // DMMNOTPRESENT
   bool deadlyLiquids = false;
//   bool  deadlyLiquids = (TerrainRender::mLiquidType >= 4);
   
   roamRadii.setSize(nodeCount);

   for (n = 0; n < nodeCount; n++)
   {
      F32   roamDist = 0.0;
      F32   extraAvoid = 0.0;
   
      if (!obstructed(n))
      {
         Point2I  basePos = indexToPos(n);
         U8       saveType = squareType(n);
         
         for (s = 0; s < scount; s++)
         {
            Point2I  roamPos = basePos + spiral[s];
            S32      roamInd = posToIndex( roamPos );
            
            if (!validArrayIndex(roamInd, nodeCount) || obstructed(roamInd))
               break;
           
            if (steep(roamInd))
            {
               extraAvoid = 0.5;
               break;
            }
               
            if (saveType != squareType(roamInd)) 
            {
               // If we run into deadly liquids, then cut our roam distance more- 
               if (deadlyLiquids && submerged(roamInd))
                  extraAvoid = 2.0;
               break;
            }
         }
         
         if( s == scount )             // no hit - can probably just use largest dist
            roamDist = dists[s - 1];
         else
            roamDist = (dists[s] - 1.4);
      }
      
      F32   roamGridDist = getMax(roamDist - extraAvoid, 0.0f);
      
      roamRadii[n] = floatToFixed(roamGridDist * gNavGlobs.mSquareWidth);
   }
}

// The following are used to find where to 'inbounds' when out of bounds.  That is, 
// a location on graph border to (find a closest node to) go to.  
void TerrainGraphInfo::setSideSegs()
{
   Point2I  corners[4] = {
      Point2I(originGrid.x                    + 0,  originGrid.y                    + 0 ), 
      Point2I(originGrid.x + gridDimensions.x - 1,  originGrid.y                    + 0 ), 
      Point2I(originGrid.x + gridDimensions.x - 1,  originGrid.y + gridDimensions.y - 1 ), 
      Point2I(originGrid.x                    + 0,  originGrid.y + gridDimensions.y - 1 )
     };

   bool        Ok[4];
   S32         numOk, p1, p2;
   Point3F     loc1, loc2;
   
   for( numOk = p1 = 0; p1 < 4; p1++ )
   {
      p2 = (p1 + 1) % 4;
      Ok[p1] = posToLoc(loc1, corners[p1] );
      Ok[p2] = posToLoc(loc2, corners[p2] );
      numOk += Ok[p1];
      boundarySegs[p1] = LineSegment(loc1, loc2);
   }
   
   // We need to have warnings in the system if the boundaries don't have nodes.  
   if( numOk < 4 )
      Con::printf( "Graph Warning!  Only %d corners of grid are navigable!", numOk );
}

void TerrainGraphInfo::doFinalDataSetup()
{
   // Set up grid area.
   gridArea.point = originGrid;
   gridArea.extent = gridDimensions;

   setSideSegs();
   
   // Offsets for finding neighboring squares when working with indices.  
   for (S32 dir = 0; dir < 8; dir++ ) {
      Point2I  off = gridOffs[dir];
      indOffs[dir] = off.y * gridDimensions.x + off.x;
   }
}

Point3F TerrainGraphInfo::whereToInbound(const Point3F & loc)
{
   // Assuming that passed parameter has already filtered with inGraphArea().
   AssertFatal(!inGraphArea(loc), "whereToInbound() only meant for out of bound locs");
   AssertFatal(haveGraph, "whereToInbound() called without a graph set up");
   
   S32   solution = -1;
   F32   bestDist = 1e9;
   
   for( S32 i = 0; i < 4; i++ )
   {
      F32   d = boundarySegs[i].distance(loc);
      
      if( d < bestDist )
         bestDist = d, solution = i;
   }
   
   return boundarySegs[solution].solution();
}

//
// See if you can go straight across terrain between the given points.  Return
// what percentage of the way the system believes can be achieved.  
//
// ==> Need to handle out of bounds checks properly.  Basically clip to mission 
//       area and then enter this loop.  If not intersecting, we probably return 
//          100%, or maybe the check for outside should happen earlier and 
//             the bot would already know they are free to roam.  
// 
F32 TerrainGraphInfo::checkOpenTerrain(const Point3F & from, Point3F & to)
{
   SphereF  sphere;
   S32      prevSphere = locToIndexAndSphere(sphere, from);
   
   if( prevSphere < 0 ) {
      to = from;
      return 0.0f;
   }
   
   LineStepper    linearPath(from, to);
   
   while(true)
   {
      // Find outbound intersection on sphere. (Note WE tell linearPath when to step)
      F32   dist = linearPath.getOutboundIntersection(sphere);

      // make it all the way?       
      if( dist > linearPath.remainingDist() )
         return 1.0;

      // no intersection.  only can happen theoretically due to rounding dullness.  
      if( dist < 0.0f ) 
         break;
         
      // See if we're in a new circle (after advancing the stepper).  
      S32   nextSphere = locToIndexAndSphere(sphere, linearPath.advanceToSolution());

      // Keep going while we're inside a sphere (and it's not the same one!)
      if( nextSphere >= 0 && nextSphere != prevSphere )
         prevSphere = nextSphere;
      else
         break;
   }
   
   to = linearPath.getSolution();
   return linearPath.distSoFar() / linearPath.totalDist();
}

//-------------------------------------------------------------------------------------

// Return if the given grid position is on the side of the area, and if so we return a 
// bitset of all the valid neighbors it _could_ have.  See consolidateData() for use.
U32 TerrainGraphInfo::onSideOfArea(S32 index)
{
   Point2I  p = indexToPos(index);
   U32      retVal = 0xff;

   // is it (well) inside?
   if(p.x>originGrid.x && p.x<gridTopRight.x && p.y>originGrid.y && p.y<gridTopRight.y)
      retVal = 0;
   else{
      // check left or right
      if(p.x == originGrid.x)
         retVal &= ~(BIT(GridBottomLeft)|BIT(GridTopLeft)|BIT(GridLeft));
      else if(p.x == gridTopRight.x)
         retVal &= ~(BIT(GridBottomRight)|BIT(GridTopRight)|BIT(GridRight));
      // check bottom or top
      if(p.y == originGrid.y)
         retVal &= ~(BIT(GridBottomLeft)|BIT(GridBottomRight)|BIT(GridBottom));
      else if(p.y == gridTopRight.y)
         retVal &= ~(BIT(GridTopLeft)|BIT(GridTopRight)|BIT(GridTop));
   }
   
   return retVal;
}


