//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "ai/graphLOS.h"

U32 Loser::mCasts = 0;     // Just for informal profiling, counting casts. 
static const F32 scGraphStepCheck = 0.75;
                           
Loser::Loser(U32 mask, bool checkFF)
   :  mCheckingFF(checkFF),
      mMask(mask)
{
   mHitForceField = false;
}

bool Loser::haveLOS(const Point3F& src, const Point3F& dst)
{
   if (mCheckingFF)
   {
      // Keep track if a force field is encountered.  Walkable connections allow, 
      // jettable connections can't go through them.  
      U32   maskWithFF = (mMask | ForceFieldObjectType);
      mCasts++;
      mColl.object = NULL;    // (Not sure if system clears)
      if (gServerContainer.castRay(src, dst, maskWithFF, &mColl)) 
      {
         if (mColl.object->getTypeMask() & ForceFieldObjectType)
            mHitForceField = true;     // and fall through do normal LOS. 
         else
            return false;
      }
   }
   mCasts++;
   mColl.object = NULL; 
   return !gServerContainer.castRay(src, dst, mMask, &mColl);
}

// Drop the point down to collision (if any) below.  If no collision, we don't
// change the drop point, and return false.  
bool Loser::hitBelow(Point3F& drop, F32 down)
{
   Point3F  below(drop.x, drop.y, drop.z - down);
   if (gServerContainer.castRay(drop, below, mMask, &mColl))
   {
      drop = mColl.point;
      return true;
   }
   return false;
}

// Height above, capped at UpHeightMax.
F32 Loser::heightUp(const Point3F& from, F32 maxUp)
{
   Point3F  up(from.x, from.y, from.z + maxUp);
   if (haveLOS(from, up))
      return maxUp;
   else
      return (mColl.point.z - from.z);
}

// Do LOS fanning one of the points.  Just do a bunch of lines.  We pre-increment
// and pre-decrement since the S-to-D los has already been checked.  
bool Loser::fannedLOS1(const Point3F& S, const Point3F& D, const VectorF& inc, S32 N)
{
   Point3F  D0 = D, D1 = D;
   while(N--) 
      if(!haveLOS(S, D0 += inc) || !haveLOS(S, D1 -= inc))
         return false;
   return true;
}

// Do LOS fanning both points.  
bool Loser::fannedLOS2(const Point3F& S, const Point3F& D, const VectorF& inc, S32 N)
{
   Point3F  D0 = D, D1 = D, S0 = S, S1 = S;
   while(N--) 
      if(!haveLOS(S0 += inc, D0 += inc) || !haveLOS(S1 -= inc, D1 -= inc))
         return false;
   return true;
}

// Do LOS checks downward to look for changes greater than step height.  
bool Loser::walkOverBumps(Point3F S, Point3F D, F32 inc)
{
   VectorF  step = (D - S);
   S32      N = getMax(S32(step.len() / inc), 2);

   // Note D becomes our stepper directly under S. 
   F32  downDist = 140;
   (D = S).z -= downDist;

   if (haveLOS(S, D))  
      return false;
      
   // Set up initial Z down.  
   F32   zdown = mColl.point.z;
   step *= (1.0f / F32(N));
   
   while (N--) 
   {
      if (haveLOS(S += step, D += step))
         return false;
         
      if (mColl.object->getTypeMask() & WaterObjectType)
         return false;
         
      F32   getZ = mColl.point.z;
      
      if (mFabs(getZ - zdown) > scGraphStepCheck)
         return false;

      // Save the Z, unless we're not walkable, in which case accumulate.  
      //==> Use proper slope values from PlayerData.  
      if (mColl.normal.z > gNavGlobs.mWalkableDot)
         zdown = getZ;
   }
      
   return true;
}

// Fan the walk - this is SLOW - we'll use less resolution.  Note we could try:
//    ==>  Walk from outside in (on all fanned checks), which will probably 
//    ==>      result in earlier exits whenever false is the result.  
//    ==>  Note we have to profile this whole process.  
bool Loser::fannedBumpWalk(const Point3F& S, const Point3F& D, VectorF inc, S32 N)
{
   Point3F  D0 = D, D1 = D, S0 = S, S1 = S;
   inc *= 2.0f;
   while ((N -= 2) >= 0)
      if (!walkOverBumps(S0 += inc, D0 += inc) || !walkOverBumps(S1 -= inc, D1 -= inc))
         return false;
   return true;
}

// (If both are outdoor, we'll look for a larger hop amount).  
static const F32 sHopLookInc = 0.2;

// Move lines up, looking for a free line.  We also want to assure that while there's 
// not a free line, we at least have a minimum amount of breathing room - about 1.5m.
bool Loser::findHopLine(Point3F S, Point3F D, F32 maxUp, F32& freeHt)
{
   F32   tThresh = (1.5f / (S - D).len());

   F32   total = 0;
   while (total < maxUp) 
   { 
      S.z += sHopLookInc;
      D.z += sHopLookInc;
      total += sHopLookInc;
      if (haveLOS(S, D)) 
      {
         freeHt = total;
         return true;
      }
      else 
      {
         if (mColl.t < tThresh)     // look for minimum amount in 
            break;
         haveLOS(D, S);             //    ... from both directions
         if (mColl.t < tThresh)     
            break;
      }
   }
   return false;
}

#define  GapWalkInc     0.15f

bool Loser::walkOverGaps(Point3F S, Point3F D, F32 allowedGap)
{
   VectorF  incVec = (D - S);
   VectorF  step2D(incVec.x, incVec.y, 0);
   F32      inc2D = step2D.len();
   S32      N = getMax(S32(inc2D / GapWalkInc) + 1, 2);
   F32      bandWid = gNavGlobs.mStepHeight;
   F32      saveL = 0, total2D = 0;
   bool     wasBelow = false;

   // D serves as our stepper to do LOS down to
   (D = S).z -= 100;
   incVec *= (1.0f / F32(N));
   inc2D  *= (1.0f / F32(N));
   
   while (N--) 
   {
      bool  los = !gServerContainer.castRay(S, D, mMask, &mColl);
   
      // Must handle water-    
      if (!los)
         if (mColl.object->getTypeMask() & WaterObjectType)
            return false;
      
      // check if we're below the allowed band- 
      if (los || mColl.point.z < (S.z - bandWid)) 
      {
         if (wasBelow) 
         {
            if (total2D - saveL > allowedGap) 
               return false;
         }
         else 
         {
            wasBelow = true;
            saveL = total2D;
         }
      }
      else 
      {
         wasBelow = false;
         saveL = total2D;
      }
         
      S += incVec;
      D += incVec; 
      total2D += inc2D;
   }
      
   return true;
}

