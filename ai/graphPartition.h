//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHPARTITION_H_
#define _GRAPHPARTITION_H_

class    GraphSearch;

class GraphPartition  
{
   public:
      enum Types  {Armor, ForceField};
      enum Answer {CannotReach, CanReach, Ambiguous};

   protected:
      Types       mType;
      bool        mCanJetDown;
      BitVector   mPartition;
      BitVector   mDownhill;
      
   public:
      GraphPartition();
      
      void  install (const GraphPartition& p);
      void  setDownhill(const GraphPartition& downhill);
      void  setPartition(GraphSearch * searcher);
      Answer reachable(S32 from, S32 to);
      void  setSize(S32 N);
      
      void  setType(Types t)  {mType = t;}
      bool  test(S32 i)       {return mPartition.test(i);}
      void  set(S32 i)        {mPartition.set(i);}
      void  clear()           {mPartition.clear();}
};

class PartitionList : public Vector<GraphPartition>
{  
   protected:
      GraphPartition::Types   mType;
      
   public:
      PartitionList();
      ~PartitionList();
      
      void  pushPartition(const GraphPartition& partition);
      void  setType(GraphPartition::Types t) {mType = t;}
      GraphPartition::Answer reachable(S32 from, S32 to);
      S32   haveEntry(S32 forNode);
      void  clear();
};

#endif
