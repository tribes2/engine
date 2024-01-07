//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHBASE_H_
#define _GRAPHBASE_H_

#ifndef _OVECTOR_H_
#include "ai/oVector.h"
#endif

class          GraphNodeList;
class          NavigationGraph;
typedef S32    GraphQIndex;
typedef U64    GraphThreatSet;

//-------------------------------------------------------------------------------------
//       Graph Edges - See graphNodeBase.cc for methods, comments.  

class GraphEdge
{
      enum {InvSpdBits = 6, InvTabSz = 1 << InvSpdBits};
      static const F32 csInvSpdTab[InvTabSz + 1];

      U8          mSteep : 1;
      U8          mInverse : InvSpdBits;
      U8          mOnPath : 1;
      U8          mJet : 1;         // These are only private since they're not 
      U8          mDown : 1;        // straight regular types.  Otherwise this 
      U8          mJump : 1;        // class is kept pretty open for the sake of 
      U8          mTeam : 5;        // speed in the debug build (since edges are
      U8          mLateral;         // used a lot inlines slow things down 
      U8          mHopOver;         // by a noticeable amount).  
      
   public:
      F32         mDist;
      S16         mDest;
      S16         mBorder;
   
      GraphEdge();
      bool        empty() const                       {return mDest < 0;}
      bool        isBorder() const                    {return mBorder >= 0;}
      bool        isJetting() const                   {return mJet;}
      bool        isDown() const                      {return mDown;}
      bool        isJump() const                      {return mJump;}
      bool        isSteep() const                     {return mSteep;}
      U8          getTeam() const                     {return mTeam;}
      U8          getLateral() const                  {return mLateral;}
      F32         getHop() const                      {return mapU8ToJetHop(mHopOver);}
      F32         getInverse() const                  {return csInvSpdTab[mInverse];}
      bool        hasHop() const                      {return mHopOver!=0;}
      void        setJetting()                        {mJet = 1;}
      void        setDown(bool b)                     {mDown = b;}
      void        setJump(bool b)                     {mJump = b;}
      void        setTeam(U8 team)                    {mTeam = team;}
      void        setLateral(U8 amt)                  {mLateral = amt;}
      void        setSteep(bool b)                    {mSteep = b;}
      void        setImpossible()                     {setInverse(1e37);}
      void        setHop(F32 amt)                     {mHopOver = mapJetHopToU8(amt);}
      void        setHop(U8 persistAmt)               {mHopOver = persistAmt;}
      F32         getTime() const                     {return (mDist * getInverse());}
      void        copyInverse(const GraphEdge* e)     {mInverse = e->mInverse;}
      bool        canJet(const F32* ratings, bool both=false) const;
      void        setInverse(F32 inv);
      const char* problems() const;
};

typedef OVector<GraphEdge> GraphEdgeList;
typedef Vector<GraphEdge*> GraphEdgePtrs;

class GraphEdgeArray
{
      S32            count;
      GraphEdge   *  edges;
   public:
      GraphEdgeArray()                       {count = 0; edges = NULL;}
      GraphEdgeArray(S32 c, GraphEdge* e)    {count = c; edges = e;}
      GraphEdge * operator++(int)            {return (count ? (count--, edges++) : 0);}
      S32         numEdges() const           {return count;}
      void        incCount()                 {count++;}
};

struct NodeProximity
{
   F32   mLateral, mHeight, mAboveC;
   void makeBad()       {mLateral=1e13;}
   void makeGood()      {mAboveC = mLateral = -1e13;}
   operator F32&()      {return mLateral;}
   NodeProximity()      {makeBad();}
   bool inside() const  {return (mLateral <= 0 && mAboveC <= 0);}
   bool insideZ() const {return (mAboveC <= 0);}
   bool possible() const;
};

//-------------------------------------------------------------------------------------
//                   Virtual Base Class For Run Time Graph Nodes

#define  GraphMaxOnPath    31

class GraphNode                                 // graphNodeBase.cc
{
      friend class   NavigationGraph;
      friend class   GraphNodeList;

   protected:
      enum{
         NodeSpecific =    0xFF, 
         Grid =            BIT(8), 
         Outdoor =         BIT(9), 
         Transient =       BIT(10),
         ExtendedGrid =    BIT(11), 
         Indoor =          BIT(12),
         BelowPortal =     BIT(13),
         GotIsland =       BIT(14), 
         Transient0 =      BIT(15),
         Transient1 =      (Transient0<<1),
         HaveTransient =   (Transient0|Transient1),
         PotentialSeed =   BIT(19), 
         UsefulSeed =      BIT(20), 
         Flat =            BIT(21),
         Algorithmic =     BIT(22), 
         StuckAvoid =      BIT(23),
         Inventory    =    BIT(24), 
         Render0 =         BIT(25), 
         Shadowed =        BIT(26),
         // Reserve three contiguous bits for shoreline, which is wider for lava.  
         LiquidPos =       27, 
         LiquidZone =      (0x7 << LiquidPos),
         Submerged =       BIT(LiquidPos + 0),
         ShoreLine =       BIT(LiquidPos + 1),
         LavaBuffer =      BIT(LiquidPos + 2), 
      };
      BitSet32       mFlags;
      S16            mIsland;
      S16            mIndex;
      S8             mOnPath;
      S8             mLevel;
      // U16            mOwnedCnt;
      // GraphEdge *    mOwnedPtr;
      U32            mAvoidUntil;
      GraphThreatSet mThreats;
      GraphEdgeList  mEdges;
      Point3F        mLoc;
      
   protected:
      GraphEdge *  pushTransientEdge(S32 dest);
      void         popTransientEdge();
      
   public:
      GraphNode();

      // One-liners
      void  set(U32 bits)           {mFlags.set(bits);}
      void  clear(U32 bits)         {mFlags.clear(bits);}
      bool  test(U32 bits) const    {return mFlags.test(bits);}
      bool  grid() const            {return test(Grid);}
      bool  outdoor() const         {return test(Outdoor);}
      bool  indoor() const          {return test(Indoor);}
      bool  belowPortal() const     {return test(BelowPortal);}
      bool  shoreline() const       {return test(ShoreLine);}
      bool  shadowed() const        {return test(Shadowed);}
      bool  submerged() const       {return test(Submerged);}
      bool  stuckAvoid() const      {return test(StuckAvoid);}
      bool  transient() const       {return test(Transient);}
      bool  gotIsland() const       {return test(GotIsland);}
      bool  inventory() const       {return test(Inventory);}
      bool  render0() const         {return test(Render0);}
      bool  algorithmic() const     {return test(Algorithmic);}
      bool  flat() const            {return test(Flat);}
      bool  canHaveBorders() const  {return test(Indoor|Transient);}
      bool  liquidZone() const      {return test(LiquidZone);}
      bool  potentialSeed() const   {return test(PotentialSeed);}
      bool  usefulSeed() const      {return test(UsefulSeed);}
      bool  uselessSeed() const     {return potentialSeed() && !usefulSeed();}
      S32   island() const          {return mIsland;}
      U32   onPath() const          {return mOnPath;}
      U32   avoidUntil() const      {return mAvoidUntil;}
      void  setOnPath()             {mOnPath = GraphMaxOnPath;}
      void  decOnPath()             {if(mOnPath) mOnPath--;}
      void  setUsefulSeed()         {set(UsefulSeed);}
      GraphThreatSet& threats()     {return mThreats;}

      // Pure virtuals.  
      virtual const Point3F&  fetchLoc(Point3F& buff) const       = 0;
      virtual GraphEdgeArray  getEdges(GraphEdge* buff) const     = 0;
      virtual S32             getIndex() const                    = 0;

      // Virtuals- 
      virtual const Point3F&  location() const;
      virtual const Point3F&  getNormal() const;
      virtual const GraphEdge* getEdgePtr() const;
      virtual S32             getLevel() const;
      virtual S32             volumeIndex() const;
      virtual F32             edgeScale(const GraphNode* to) const;
      virtual NodeProximity   containment(const Point3F& loc) const;
      virtual Point3F         getRenderPos() const;
      virtual Point3F         randomLoc() const;
      virtual F32             terrHeight() const;
      virtual F32             radius() const;
      virtual F32             minDim() const;
      virtual F32             area() const;
      
      // Other:
      void  setIsland(S32 islandNum);
      void  setAvoid(U32 duration, bool important=false);
      bool  neighbors(const GraphNode*) const;
      GraphEdge* getEdgeTo(S32 to) const;
      GraphEdge* getEdgeTo(const GraphNode* n) const  {return getEdgeTo(n->getIndex());}
};

//-------------------------------------------------------------------------------------

class GraphNodeList : public VectorPtr<GraphNode *>
{
   public:
      void setFlags(U32 bitset);
      void clearFlags(U32 bitset);
      S32 searchSphere(const SphereF& sphere, const GraphNodeList& listIn);
      GraphNode* closest(const Point3F& loc, bool los=false);
      bool addUnique(GraphNode * node);
};

typedef AxisAlignedBSP<GraphNode>   GraphBSPTree;

#endif
