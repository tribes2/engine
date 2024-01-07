//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------

GraphPartition::GraphPartition()
{
   mType = ForceField;
   mCanJetDown = false;
   // mPartition.setSize(gNavGraph->numNodesAll());
   // mPartition.clear();
}

void GraphPartition::install(const GraphPartition& p)
{
   mPartition.copy(p.mPartition);
}

// Allocate the bit vector and clear it.
void GraphPartition::setSize(S32 N)
{
   mPartition.setSize(N);
}

// Consumer uses a regular partition to build the downhill partition.  We check here to
// see if it's any different from the regular partition.  
void GraphPartition::setDownhill(const GraphPartition& downhill)
{
   U32   bytes = mPartition.getByteSize();
   
   AssertFatal(mType == Armor, "Only armor types can have a downhill component");
   AssertFatal(bytes == downhill.mPartition.getByteSize(), "Unequal partition sizes");

   if (dMemcmp(mPartition.getBits(), downhill.mPartition.getBits(), bytes)) {
      mCanJetDown = true;
      mDownhill.copy(downhill.mPartition);
   }
   else {
      mCanJetDown = false;
   }
}

// See if partition can answer question of whether or not we can get from A to B.  
GraphPartition::Answer GraphPartition::reachable(S32 A, S32 B)
{
   if (mPartition.test(A)) 
   {
      if (mPartition.test(B) || (mCanJetDown && mDownhill.test(B)))
         return CanReach;
      else
         return CannotReach;
   }
   else if (mPartition.test(B))
      return CannotReach;
   else 
      return Ambiguous;
}

//-------------------------------------------------------------------------------------

PartitionList::PartitionList()
{  
   mType = GraphPartition::ForceField;
}

PartitionList::~PartitionList()
{
   clear();
}

//-------------------------------------------------------------------------------------

void PartitionList::clear()
{
   while (size()) {
      last().~GraphPartition();
      pop_back();
   }
}

// This is called after searches that have failed to reach their target to register the
// new partition that has been discovered.  ie. when a force field has changed states,
// then partition lists are invalid and get rebuilt as searches happen.  
void PartitionList::pushPartition(const GraphPartition& partition)
{
   GraphPartition    empty;

   // I'd like to construct the entry in the vector, but can't get it to go...     
   //    i.e  increment();  last().GraphPartition();
   
   // But this should work, given that BitVector doesn't override assignment... ugh.  
   push_back(empty);
   last().install(partition);
   last().setType(mType);
}

// Our partition lists will probably never have the complete answer to this.  Need an 
// ambiguous return value.  
GraphPartition::Answer PartitionList::reachable(S32 A, S32 B)
{
   for (iterator partition = begin(); partition != end(); partition++) 
   {
      GraphPartition::Answer answer = partition->reachable(A, B);
      if (answer != GraphPartition::Ambiguous)
         return answer;
   }
   AssertFatal(mType != GraphPartition::Armor, "Armor partitions are never ambiguous");
   return GraphPartition::Ambiguous;
}

// When partitions are built at mission start, it has to check and see which nodes have 
// entries after finishing each pass.  It goes until graph is filled.  
S32 PartitionList::haveEntry(S32 forNode)
{
   for (iterator partition = begin(); partition != end(); partition++)
      if (partition->test(forNode))
         return true;
         
   return false;
}

