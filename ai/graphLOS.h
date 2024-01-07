//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHLOS_H_
#define _GRAPHLOS_H_

struct Loser 
{
   static U32  mCasts;
   const bool  mCheckingFF;
   const U32   mMask;
   RayInfo     mColl;
   bool        mHitForceField;
   
   Loser(U32 mask, bool checkFF = false);
   
   bool  haveLOS(const Point3F& src, const Point3F& dst);
   bool  fannedLOS1(const Point3F& S, const Point3F& D, const VectorF& inc, S32 N);
   bool  fannedLOS2(const Point3F& S, const Point3F& D, const VectorF& inc, S32 N);
   bool  walkOverBumps(Point3F S, Point3F D, F32 inc = 0.2);
   bool  fannedBumpWalk(const Point3F& S, const Point3F& D, VectorF inc, S32 N);
   bool  findHopLine(Point3F S, Point3F D, F32 maxUp, F32& freeHt);
   bool  walkOverGaps(Point3F S, Point3F D, F32 allowedGap);
   bool  hitBelow(Point3F& dropPoint, F32 castDown);
   F32   heightUp(const Point3F& from, F32 max);
};

#endif
