//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHJETTING_H_
#define _GRAPHJETTING_H_

class JetManager
{
   public: 
      class ID 
      {
            friend class JetManager;  
            S32   id;
            S32   incarnation;
         public: 
            ID(); 
            ~ID(); 
            void  reset();
            
      };
      struct Ability{ F32 acc; F32 dur; F32 v0; Ability(); bool operator==(const Ability&); };
      friend class ID;

      static F32 modifiedLateral(F32 xyDist);
      static F32 modifiedDistance(F32 xyDist, F32 zDist);
      static F32 invertLateralMod(F32 lateralField);
      
   protected:
      struct JetPartition : public PartitionList 
      {
         F32      ratings[2];
         Ability  ability;
         S32      users; 
         bool     used;
         U32      time;
         JetPartition();
         void     doConstruct();
      };
      
      U16 *             mFloodInds[2];
      GraphPartition *  mArmorPart;
      JetPartition      mPartitions[AbsMaxBotCount];
   
   protected:
      S32   floodPartition(U32 seed, bool needBoth, F32 ratings[2]);
      void  decRefCount(const ID& id);
      
   public:
      JetManager();
      ~JetManager();
      S32   partitionOneArmor(JetPartition& list);
      GraphPartition::Answer reachable(const ID& jetCaps, S32 from, S32 to);
      const F32* getRatings(const ID& jetCaps);
      F32   jetDistance(const Point3F& from, const Point3F& to) const;
      F32   calcJetScale(const GraphNode* from, const GraphNode* to) const;
      void  initEdge(GraphEdge& edge, const GraphNode* src, const GraphNode* dst);
      void  replaceEdge(GraphBridgeData& replaceInfo);
      F32   estimateEnergy(const ID& jetCaps, const GraphEdge * edge);
      F32   calcJetRating(F32 thrust, F32 duration);
      void  calcJetRatings(F32 * ratings, const Ability& ability);
      bool  update(ID& id, const Ability& ability);
      void  clear();
};

#endif
