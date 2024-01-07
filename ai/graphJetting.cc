//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

#define  RatingsCloseEnough      0.1

JetManager::JetManager()
{
   mFloodInds[0] = NULL;
   mFloodInds[1] = NULL;
   mArmorPart = NULL;
}

JetManager::~JetManager()
{
   clear();
}

void JetManager::clear()
{
   for (S32 i = 0; i < AbsMaxBotCount; i++)
      mPartitions[i].doConstruct();
      
   delete mArmorPart;
   delete [] mFloodInds[0];
   delete [] mFloodInds[1];
   mArmorPart = NULL;
   mFloodInds[0] = NULL;
   mFloodInds[1] = NULL;
}

// Used for both passes of the armor partitioning.  Basically do a flood fill expansion
// with the armor partition telling us what has been "extracted".  We use two flipping 
// lists for this, each containing the last round of extractions.  This routine is then
// used twice - reused for finding downhill.  
S32 JetManager::floodPartition(U32 seed, bool needBoth, F32 ratings[2])
{
   // Set up the loop-    
   S32   numFound = 0;
   S32   floodSizes[2] = {1, 0};
   S32   curExtract = 0, curDeposit;
   mArmorPart->set(mFloodInds[0][0] = seed);
   
   // Keep extracting until nothing gets put into next time list
   while (floodSizes[curExtract])
   {
      U16 * extractFrom = mFloodInds[curExtract];
      U16 * depositInto = mFloodInds[curDeposit = (curExtract ^ 1)];
      
      for (S32 i = floodSizes[curExtract] - 1; i >= 0; i--)
      {
         GraphNode * node = gNavGraph->lookupNode(extractFrom[i]);
         GraphEdgeArray    edges = node->getEdges(NULL);
      
         while (GraphEdge * edge = edges++) {
            if (!mArmorPart->test(edge->mDest)) {
               if (!edge->isJetting() || edge->canJet(ratings, needBoth)) {
                  mArmorPart->set(edge->mDest);
                  depositInto[floodSizes[curDeposit]++] = edge->mDest;
               }
            }
         }
      }
      numFound += floodSizes[curExtract];
      floodSizes[curExtract] = 0;
         // Here's the flip.  Note the previous line clears deposit size for next loop.
      curExtract = curDeposit;
   }
   
   return numFound;
}

// Code to find a partition for one armor / energy configuration.  
S32 JetManager::partitionOneArmor(JetPartition& list)
{
   U32   memUsedBefore = Memory::getMemoryUsed();
   S32   nodeCount = gNavGraph->numNodes();
   
   // First time allocations.  Also, newIncarnation() will clear them.  Partition size
   // spends a couple bytes to include transients so search code doesn't have to check.
   if (! mArmorPart) {
      mArmorPart = new GraphPartition();
      mArmorPart->setSize(gNavGraph->numNodesAll());
      mFloodInds[0] = new U16 [nodeCount];
      mFloodInds[1] = new U16 [nodeCount];
   }
   
   S32            seedNode = 0;
   S32            total = 0;
   Vector<S32>    saveSeedNodes;

   //                            PASS 1
   //
   // First pass finds the 'stranded' component of each partition- this floods along 
   // only those edges that can be traversed BOTH WAYS.  
   //
   list.clear();
   mArmorPart->clear();
   while (seedNode < nodeCount)
   {
      if (!mArmorPart->test(seedNode)) 
      {
         total += floodPartition (seedNode, true, list.ratings);
         
         // Add the partition to the list, and remember the seed for next pass.  
         list.pushPartition(* mArmorPart);
         saveSeedNodes.push_back(seedNode);
         
         if (total == nodeCount)
            break;
      }
      seedNode++;
   }
   
   //                            PASS 2
   //
   // This pass finds the downhill component of each partition, if different.  Note that
   // setDownhill() only adds a separate partition if the downhill one is different.  
   //
   for (S32 i = 0; i < list.size(); i++)
   {
      mArmorPart->clear();
      floodPartition (saveSeedNodes[i], false, list.ratings);
      list[i].setDownhill(* mArmorPart);
   }   
   
   U32   memUsedAfter = Memory::getMemoryUsed();
   if (memUsedAfter > memUsedBefore)
      NavigationGraph::sLoadMemUsed += (memUsedAfter - memUsedBefore);
   
   return list.size();
}

//-------------------------------------------------------------------------------------

JetManager::Ability::Ability()
{ 
   acc = dur = v0 = -111;
}

// Check for close enough abilities.  So far the only thing I've seen change is the
// duration number.  
bool JetManager::Ability::operator==(const Ability& ability)
{
   return(  mFabs(acc - ability.acc) < 0.01     &&
            mFabs(dur - ability.dur) < 0.5      &&
            mFabs(v0 - ability.v0) < 0.1  
         );
}

JetManager::ID::ID()
{
   id = -1;
   incarnation = -1;
}

void JetManager::ID::reset()
{
   if (NavigationGraph::gotOneWeCanUse())
      gNavGraph->jetManager().decRefCount(* this);
   id = -1;
}

JetManager::ID::~ID()
{
   // Could probably just use reset() here... 
   if (NavigationGraph::gotOneWeCanUse())
      gNavGraph->jetManager().decRefCount(* this);
}

void JetManager::JetPartition::doConstruct()
{
   ratings[0] = 0.0;
   ratings[1] = 0.0;
   users = 0;
   used = false;
   time = 0;
}

JetManager::JetPartition::JetPartition()
{
   doConstruct();
}

//-------------------------------------------------------------------------------------

void JetManager::decRefCount(const ID& id)
{
   if (id.id >= 0) {
      JetPartition   &  jetPartition = mPartitions[id.id];
      AssertFatal(jetPartition.users > 0, "JetManager ID management is not in sync");
      // This is when it starts to grow old- 
      if (--jetPartition.users == 0)
         jetPartition.time = Sim::getCurrentTime();
   }
}

// Users update this with an id and a rating.  Return true if a new one is created, 
// telling consumer (nav path) to try to put off slow operations a little bit.  
bool JetManager::update(ID& id, const Ability& ability)
{
   // If missions cycles, or graph is rebuilt, id is out of date and should be reset- 
   if (id.incarnation != gNavGraph->incarnation()) 
   {
      id.incarnation = gNavGraph->incarnation();
      id.id = -1;
   }

   // If new entry or no longer valid, find match or create new.  
   if (id.id < 0 || !(mPartitions[id.id].ability == ability))
   {
      decRefCount(id);

      // Look for existing partition that matches this ability.  We keep unused ones- 
      //  If all have had abilities set on them, then take oldest one.  
      U32   oldAge = U32(-1);
      JetPartition * slot = NULL;
      JetPartition * oldest = NULL;
      JetPartition * j = mPartitions;
      for (S32 i = 0; i < AbsMaxBotCount; i++, j++)
      {
         if (j->ability == ability) {
            j->users++;
            id.id = i;
            return false;
         }
         else {
            // Look for free entry in case it's needed.  First choice are those not yet
            // in use.  Second choice is the oldest one (smallest creation timestamp). 
            if (!slot && !j->users) {
               if (!j->used)
                  slot = j;
               else if (j->time < oldAge)
                  oldAge = j->time, oldest = j;
            }
         }
      }
      
      // No match found- create a new one with the slot found.
      if (!slot)
         slot = oldest;
      AssertFatal(slot != NULL, "JetManager couldn't find empty slot");
      id.id = (slot - mPartitions);
      slot->ability = ability;
      slot->users = 1;
      slot->used = true;
      calcJetRatings(slot->ratings, ability);
      slot->setType(GraphPartition::Armor);

      // Do the work-       
      partitionOneArmor(* slot);
      
      // Inform caller that we did some slow work- 
      return true;
   }
   return false;
}

//-------------------------------------------------------------------------------------
// Here's the math.  We've done the phsyics math accurately for the vertical distance,
// but we're just tweaking it for the lateral travel distance, which looks in practice
// to be about the same.  That is, a bot can jet a flat XY distance that is close to 
// how high straight up it can jet.  But we scale down the lateral (increase what it 
// _thinks_ it has to do) since this is just a heuristic.  I'm not sure what the exact
// math is for estimating how far a bot can jet in any arc...  

#define  LateralExtra   2.00f
#define  LateralScale   1.28f

F32 JetManager::modifiedLateral(F32 lateralDist)
{
   return (lateralDist * LateralScale + LateralExtra);
}

F32 JetManager::modifiedDistance(F32 lateralDist, F32 verticalDist)
{
   return verticalDist + modifiedLateral(lateralDist);
}

// In one case the public needs the actual XY from the lateral field, so this function
// is the inverse of modifiedLateral().
F32 JetManager::invertLateralMod(F32 lateralField)
{
   F32   result = (lateralField - LateralExtra) * (1.0 / LateralScale);
   return getMax(0.0f, result);
}

//-------------------------------------------------------------------------------------
// Public function that the aiNavJetting module uses for knowing when it has enough 
// energy to launch.  That code must use the same adjusted distance that 
// the jetting edges use.  

F32 JetManager::jetDistance(const Point3F& from, const Point3F& to) const
{
   F32      vertical = (to.z - from.z);
   Point2F  lateral(to.x - from.x, to.y - from.y);
   
   if (vertical > 0)
      return modifiedDistance(lateral.len(), vertical);
   else
      return modifiedLateral(lateral.len());
}

//-------------------------------------------------------------------------------------

// 
// All edge initialization comes through here, especially required since the 
// partitioning needs at least one invariants holding on the edges, namely that
// the mDist field of a pair of edges between two nodes must be exactly the same. 
// 
void JetManager::initEdge(GraphEdge& edge, const GraphNode* src, const GraphNode* dst)
{
   // The jet scale probably needs a little bit of work (and it seems like it really
   // depends on the bot's energy levels.  Any hop that uses less than half the 
   // bot's energy would probably be a good one to take, all else being equal.  
   if (!edge.isJetting())
      edge.setInverse(src->edgeScale(dst));
   else 
      edge.setInverse(calcJetScale(src, dst));
      
   // Must assure that distances are the same both ways, so just to be sure we 
   // sort the locations.  Partitioning flooding relies on this.  
   const Point3F& srcLoc = src->location();
   const Point3F& dstLoc = dst->location();
   Point3F  high;
   Point3F  low;
   if (srcLoc.z > dstLoc.z) 
   {
      high = srcLoc;
      low = dstLoc;
      edge.setDown(edge.isJetting());
   }
   else 
   {
      high = dstLoc;
      low = srcLoc;
   }

   // Distance on edge is sum of horizontal + some extra + vertical.  The extra
   // is a buffer so they can get the lateral velicity going.  
   F32   absHeight = (high.z - low.z);
   (high -= low).z = 0;
   F32   lateral = high.len();
   // edge.mDist = (lateral + absHeight + 2.0);
   edge.mDist = modifiedDistance(lateral, absHeight);

   // Get lateral distance for hops downward, little extra for buffer.  
   if (edge.isDown())
      edge.setLateral(U8(modifiedLateral(lateral)));

   // Set flatness.  Must be conservative with it so that edges evaluate the 
   // same both ways for partioning purposes.  
   if (src->flat() && dst->flat())
      edge.setJump(true);
}

// Ok - the indices are backwards from what's intuitive, but it's not a bug
// per se.  We'll change to the intuitive if there's a convenient rebuild time... 
// cf.  The GraphBridgeData constructor.  
void JetManager::replaceEdge(GraphBridgeData & B)
{
   GraphNode   *  src = gNavGraph->lookupNode(B.nodes[1]);
   GraphNode   *  dst = gNavGraph->lookupNode(B.nodes[0]);
   GraphEdge   *  edge = src->getEdgeTo(dst);
   
   AssertFatal(src && dst && edge, "JetManager::replaceEdge()");
   
   if (B.isUnreachable())
   {
      edge->setSteep(true);
      edge->setJetting();
      edge->setImpossible();
   }
   else
   {
      edge->setJetting();
      edge->setHop(B.jetClear);
      initEdge(* edge, src, dst);
   }
}

// Calculates scale factor on jetting edges for deciding when to do them.  
F32 JetManager::calcJetScale(const GraphNode* from, const GraphNode* to) const 
{
   F32   scale;
   
   F32   zdiff = (to->location().z - from->location().z);
   
   if (zdiff > 0) {
      // Jetting up adds to factor.  Base amount is 3.0, then we 
      // also add 1 for every 10 meters going up beyond 20.  
      F32   per10 = mapValueLinear(zdiff, 20.0f, 120.0f, 0.0f, 10.0f);
      scale = 3.0 + per10;
   }
   else {
      // We'll make this number better once we have detection of 
      // which are walkable (walk-off-able) and which aren't. 
      scale = 1.4;
   }
   
   return scale;
}

#define  GravityNum  0.625 

// Given thrust and duration, see how far we can go.  
F32 JetManager::calcJetRating(F32 thrust, F32 duration)
{
   F32   tSqrd = duration * duration;
   thrust -= GravityNum;

   // How far we travel while applying the continuous force-    
   F32   pureJetDist = (0.5 * thrust * tSqrd * TickSec);

   // Once jet is gone- final velocity carries up.  More jet remains, but that's our 
   //    reserve so we can be sure to cover the distance.  
   F32   finalVel = (thrust * duration);
   F32   driftUp = (0.5 * finalVel / GravityNum) * finalVel;
   
   return (pureJetDist + driftUp * TickSec);
}


// Calculate both ratings:
//    0  -  without jumping
//    1  -  can jump
//
// The ratings are a little bit conservative, but we need that buffer since the math 
// isn't perfect for long hops.  Mainly, we assume duration lasts as long as 
// energy, whereas you actually get a little bit more from recharge- that's our buffer.
//
void JetManager::calcJetRatings(F32 * ratings, const Ability& ability)
{
   ratings[0] = calcJetRating(ability.acc, ability.dur);
   ratings[1] = ratings[0] + (ability.v0 * ability.dur * TickSec);
}

const F32 * JetManager::getRatings(const ID& jetCaps)
{
   JetPartition & J = mPartitions[jetCaps.id];
   AssertFatal(J.users > 0, "Bad call to JetManager::getRatings()");
   return J.ratings;
}

GraphPartition::Answer JetManager::reachable(const ID& jetCaps, S32 from, S32 to)
{
   AssertFatal(jetCaps.id >= 0, "Uninitialized ID given to JetManager::reachable()");
   return mPartitions[jetCaps.id].reachable(from, to);
}

//-------------------------------------------------------------------------------------
// Estimate what percentage of energy is needed to make the given hop.  Actually going 
// from distance/ability to energy required is not performed anywhere, and instead of
// trying to invert the math, we'll just do a quick binary search.  This only happens
// once when an edge is encountered in a path.  And we save square roots!  Ok I'm lazy.
F32 JetManager::estimateEnergy(const ID& jetCaps, const GraphEdge * edge)
{
   JetPartition & J = mPartitions[jetCaps.id];
   AssertFatal(J.users > 0, "JetManager::estimateEnergy()");
   
   Ability  interpAbility = J.ability;
   F32      lower = 0.0;
   F32      upper = 1.0;
   F32      ratings[2];
   // Eight iterations will get us within 1% 
   for (S32 i = 0; i < 8; i++)
   {
      F32      interpPercent = ((lower + upper) * 0.5);
      interpAbility.dur = (interpPercent * J.ability.dur);
      calcJetRatings(ratings, interpAbility);
      if (edge->canJet(ratings))
         upper = interpPercent;
      else
         lower = interpPercent;
   }
   
   // This is the lowest percentage we found which still works (or it's 1.0).
   return upper;
}

