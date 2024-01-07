//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/aiConnection.h"
#include "ai/aiNavJetting.h"
#include "ai/graphLOS.h"

//-------------------------------------------------------------------------------------

#define  GravityConstant   20.0
#define  AmountInFront     0.37
#define  FastVelocity      10.0
#define  PullInPercent     0.63
#define  HitPointThresh    1.2
#define  JumpWaitCount     3
#define  SeekAboveChute    17.0f

//-------------------------------------------------------------------------------------

bool AIJetting::init(const Point3F& dest, bool intoMount, NavJetting * jetInfo)
{
   mSeekDest = dest;
   mFirstTime = true; 
   mWillBonk = false;
   mLaunchSpeed = 0.1;
   mIntoMount = intoMount;
   mStatus = AIJetWorking;
   mJetInfo = jetInfo;
   return true;
}

//-------------------------------------------------------------------------------------
// Run LOS to see if we can safely jump to our destination.  

static const U32 scBonkMask = InteriorObjectType|StaticShapeObjectType
                              |StaticObjectType|TerrainObjectType;

// We don't worry about this if we're jetting up more than 2 meters (in that case
// there shouldn't be a chance of bonking).  Also, we don't want to suppress
// take-off jump for long hops (these edges should already be well checked).  Mainly
// this check is for those chute connections which had the bonk check suppressed... 
bool AIJetting::willBonk(Point3F src, Point3F dst)
{
   if (dst.z - src.z < 2.0)
   {
      Point2F  vec2D(src.x-dst.x, src.y-dst.y);
      if (vec2D.lenSquared() < (LiberalBonkXY * LiberalBonkXY))
      {
         src.z = dst.z = (getMax(src.z, dst.z) + 3.7);
      
         Loser    los(scBonkMask);
      
         if (!los.haveLOS(src, dst))
            return false;     // Disable temporarily... 
      }
   }
   return false;
}

//-------------------------------------------------------------------------------------
// Called whenever we move the state along.  

void  AIJetting::newState(S16 state, S16 counter /*=0*/)
{
   mCounter = counter; 
   mState = state;
}

//-------------------------------------------------------------------------------------

// Get distance from landing "wall" - with (dist < 0) meaning we're that much beyond it
F32 AIJetting::distFromWall(const Point3F& loc)
{
   VectorF  vecToWall = (mWallPoint - loc);
   vecToWall.z = 0;
   return mDot(vecToWall, mWallNormal);
}

// Assuming we're jetting up - see if it looks like the top of a chute up there. 
static bool upChute(const Point3F& from, F32 top, const VectorF& normal, Point3F& soln)
{
   RayInfo  coll;
   Point3F  to(from.x, from.y, top + 20.0f);
   
   if (gServerContainer.castRay(from, to, InteriorObjectType, &coll)) {
      if (mDot(normal, coll.normal) > 0.05) {
         coll.normal.z = 0;
         coll.normal.normalize();
         if (mDot(normal, coll.normal) > 0.9) {     // ~ 25 deg
            soln = coll.point;
            // return true;
            return false;     // this has problems with larger chutes... oops. 
         }
      }
   }
   return false;
}

bool AIJetting::figureLandingDist(AIConnection* ai, F32& dist)
{
   F32   A = -(GravityConstant * 0.5);
   F32   B = ai->mVelocity.z;
   F32   C = (ai->mLocation.z - mLandPoint.z);
   F32   solutions[2];
   U32   N = mSolveQuadratic(A, B, C, solutions);
   
   if (N > 0) 
   {  
      // use the larger time solution (first will be negative, or coming up on soln)
      F32   T = solutions[N-1];

      // see where this will put us in XY - 
      dist = T * ai->mVelocity2D.len();
      return true;
   }
   return false;
}

// Do first time setup, plus handle other processing on each frame.  
bool AIJetting::firstTimeStuff(AIConnection* ai, Player * player)
{
   if (mFirstTime)
   {
      mFirstTime = false;
      mCanInterupt = true;
      mWillBonk = false;
      mLandPoint = mSeekDest;
      mWallNormal = (mSeekDest - ai->mLocation);
      mWallNormal.z = 0;
      if ((mTotal2D = mWallNormal.len()) < GraphJetFailXY) {
         mStatus = AIJetFail;
         return false;
      }

      F32   zDiff = (mSeekDest.z - ai->mLocation.z);
      
      mSlope = (zDiff / mTotal2D);
      
      // Set our desired launch speed based on slope.  Zero slope -> full speed, 
      // Steep slope -> Zero speed.  Maps negative slopes to full speed.  
      mLaunchSpeed = mapValueQuadratic(mSlope, 1.3f, 0.0f,   0.0f, 1.0f);
      
      // Wall normal points along our path in XY plane.  
      mWallNormal /= mTotal2D;
      
      mUpChute = (mJetInfo && mJetInfo->mChuteUp);
      
      // our dest will be a unit or so beyond for sake of aiming
      mWallPoint = mLandPoint;
      mSeekDest += (mWallNormal * 1.1);

      // Hop over walls-       
      if (mJetInfo)
         mSeekDest.z += mJetInfo->mHopOver;
      
      newState(AssureClear);
   }

   // Variable that is set when they should look where their heading for right effect-
   mShouldAim = false;
   
   if (mIntoMount && player->isMounted()) {
      mStatus = AIJetSuccess;
      return false;
   }
   
   return true;
}

//-------------------------------------------------------------------------------------

// Local function - call it when you should aim - bool variable is always cleared 
//  unless this is called.  
void AIJetting::setAim(const Point3F& /*always mSeekDest now*/)
{
   mShouldAim = true;
}

// Public function - find out if should and where at.  
bool AIJetting::shouldAimAt(Point3F& atWhere)
{
   if (mShouldAim)
      atWhere = mSeekDest;
   return mShouldAim;
}

//-------------------------------------------------------------------------------------
// We may still need to nudge a little bit to assure we have clearance (the generated
// jetting edges need to sometimes hug a bit close (else some makeable connections get
// get thrown out) so this is how we handle the occasional problem).  

static const U32  sLOSMask =  InteriorObjectType |  StaticShapeObjectType   |
                              StaticObjectType   |  TerrainObjectType;
static const F32  sClearDistance = 1.4f;

bool AIJetting::assureClear(AIConnection* ai, Player* )
{
   // Get loc a little bit above the feet- 
   Point3F  botLoc(ai->mLocation.x, ai->mLocation.y, ai->mLocation.z + 0.2);
   
   if (botLoc.z < mSeekDest.z - 2.0)
   {
      // This shouldn't go on very long, just need a nudge if anything...
      if (++mCounter < 32)
      {
         Point3F  vec = botLoc;
         (vec -= mSeekDest).z = 0.0f;
      
         // Usual checks just in case....  
         F32   len = vec.len();
         if (len > 0.1)
         {
            Loser    loser(sLOSMask);
            Point3F  clearLoc(botLoc.x, botLoc.y, mSeekDest.z);

            // Compute vector to come in by-             
            vec *= (sClearDistance / len);
            clearLoc -= vec;
            
            if (!loser.haveLOS(botLoc, clearLoc))
            {
               // Seek away, our vec contains which way to go...  
               ai->setMoveLocation(botLoc += vec);
               ai->setMoveSpeed(0.6);
               return false;
            }
         }
      }
   }
   
   mWillBonk = willBonk(ai->mLocation, mSeekDest);
   
   newState(AwaitEnergy);
   return false;
}

//-------------------------------------------------------------------------------------

// Wait for amount of energy we think we need.  
bool AIJetting::awaitEnergy(AIConnection* ai, Player* player)
{
   ai->setMoveLocation(mSeekDest);
   if (ai->mVelocity.lenSquared() < 0.2)
   {
      if (player->getEnergyValue() > 0.99)
      {
         newState(PrepareToJump);
      }
      else
      {
         // Run the energy calculation every frame.  We need to use the same methods
         // that the graph uses to decide that a given hop is makeable with a certain
         // amount of energy ability configuration.  
         F32                  ratings[2];
         JetManager::Ability  ability;
         
// Tribes player jetting was remove from the player class.
#if 0
         ability.dur = player->getJetAbility(ability.acc, ability.dur, ability.v0);
#else
         ability.acc = 0;
         ability.dur = 0;
         ability.v0 = 0;
#endif
         gNavGraph->jetManager().calcJetRatings(ratings, ability);
         
         F32   jetD = gNavGraph->jetManager().jetDistance(ai->mLocation, mSeekDest);
         
         if (jetD < ratings[!mWillBonk && player->canJump()])
            newState(PrepareToJump);
      }
   }
   else
      ai->setMoveSpeed(0);
   return false;
}

//-------------------------------------------------------------------------------------

bool AIJetting::prepareToJump(AIConnection* ai, Player* player)
{
   ai->setMoveLocation(mSeekDest);
   ai->setMoveSpeed(0);
   if (++mCounter >= JumpWaitCount)
   {
      // Can't check if can jump so often right now - so just do it once...
      bool  jumpReady = (mCounter==JumpWaitCount) && (player->haveContact() || player->isMounted());

      // HACK to remedy problem with never finding a contact surface sometimes.
      if (!jumpReady && (mCounter > JumpWaitCount * 8))
         jumpReady = (ai->mVelocity.lenSquared() < 0.04);
      
      if (jumpReady)
      {
         mJumpPoint = ai->mLocation;
         Point3F  here = ai->mLocation;
         here.z += 100;
         ai->setMoveLocation(here);
         if (!mWillBonk || player->isMounted())
            ai->pressJump();
         ai->pressJet();
         newState(InTheAir);
         mCanInterupt = false;
      }
   }
   return false;
}

//-------------------------------------------------------------------------------------

bool AIJetting::inTheAir(AIConnection* ai, Player* player)
{
   bool  advanceState = false;
   
   if (player->haveContact()) {
      if (++mCounter >= 2) {
         mStatus = AIJetFail;
         return true;
      }
   }
   else
      mCounter = 0;

   ai->setMoveLocation(mSeekDest);
   
   if (mUpChute) {
      // Basically jet until bonk as long as we're going vertically
      if (ai->mVelocity2D.lenSquared() > 0.04 || ai->mVelocity.z < -0.1)
         mUpChute = false;
      else {
         ai->setMoveSpeed(0.0f);
         ai->pressJet();
      }
   }
   
   if (!mUpChute) {
      if (ai->mLocation.z > mSeekDest.z) {
         // Must monitor our Z velocity- 
         if (ai->mVelocity.z < 0)
            ai->setMoveSpeed(0.0f);
         else
            ai->setMoveSpeed(1.0f);
         ai->pressJet();
      }
      else {
         ai->setMoveSpeed(0.0f);
         F32   zSpeed = ai->mVelocity.z;
         F32   howHigh = zSpeed * (zSpeed / GravityConstant);
         if ((zSpeed < 0.01) || (ai->mLocation.z + howHigh) < (mSeekDest.z + 1.2))
            ai->pressJet();
      }
   }

   // get 2D distance to wall:
   F32   wallD = distFromWall(ai->mLocation);

   if (wallD < mTotal2D * 0.5)   
      setAim(mSeekDest);

   if (wallD < 0.1)
      advanceState = true;
      
   // We need a good check for failure.  One simple measure for failure might be to 
   // look at component of lateral velocity along the vector to the destination.  
   //  Also, if we run out of energy and are far from destination... 
   if (!advanceState) 
   { 
      F32   landD;
      if (AIJetting::figureLandingDist(ai, landD))
         if( landD > (wallD - AmountInFront))
            advanceState = true;
   }

   if (advanceState)
      newState(SlowToLand);
      
   return false;
}

//-------------------------------------------------------------------------------------

bool AIJetting::slowToLand(AIConnection* ai, Player* player)
{
   if (player->haveContact())
   {
      // First clause meant to handle case where we didn't get off the ledge- 
      if (ai->mLocation.z - mLandPoint.z > 1.3 && distFromWall(ai->mLocation) > 1.0) {
         newState(PrepareToJump, JumpWaitCount-1);
      }
      //==> We need to use some volume information from the destination.  
      else {
         if (!within(ai->mLocation, mLandPoint, 4.0))
         {
            mStatus = AIJetFail;
            return true;
         }
         else {
            mCanInterupt = true;
            newState(WalkToPoint);
         }
      }
   }
   else
   {
      Point2F  vel2(ai->mVelocity.x, ai->mVelocity.y);
      F32      speed2 = vel2.len();
      
      setAim(mSeekDest);

      // Use velocity checks to finish it up- 
      F32   zvel = mFabs(ai->mVelocity.z);
      if (speed2 < 0.1 && zvel < 0.1)
         return true;
      
      F32      wallD = distFromWall(ai->mLocation), landD;
      bool     beyond = figureLandingDist(ai, landD) && (landD > wallD- AmountInFront);
      bool     pullIn = (speed2 * PullInPercent) > wallD && speed2 > 0.7;
      
      if (beyond && pullIn)      // try to slow down
      {
         ai->setMoveLocation(mJumpPoint);
         ai->setMoveSpeed(1.0f);
         ai->pressJet();
      }
      else 
      {
         ai->setMoveLocation(ai->mLocation);
         ai->setMoveSpeed(0.0f);
         if(! beyond)
            ai->pressJet();
      }
   }
   return false;
}

//-------------------------------------------------------------------------------------

bool AIJetting::walkToPoint(AIConnection* ai, Player*)
{
#if 0
   mStatus = AIJetSuccess;
   ai->setMoveSpeed(0.0f);
   return true;
#else
   ai->setMoveLocation(mLandPoint);
   ai->setMoveTolerance(HitPointThresh * 0.6);
   ai->setMoveSpeed(0.3f);
   if (within_2D(ai->mLocation, mLandPoint, HitPointThresh) || ++mCounter > 10)
   {
      // wound up under or over the point...  
      if (mFabs(ai->mLocation.z - mLandPoint.z) > HitPointThresh)
         mStatus = AIJetFail;
      else
         mStatus = AIJetSuccess;
      ai->setMoveSpeed(0.0f);
      return true;
   }
   return false;
#endif
}

//-------------------------------------------------------------------------------------

// Returns true when done.  
bool AIJetting::process(AIConnection* ai, Player* player)
{
   if (!firstTimeStuff(ai, player))
      return true;

   switch(mState) 
   {
      case  AssureClear:         return assureClear(ai, player);
      case  AwaitEnergy:         return awaitEnergy(ai, player);
      case  PrepareToJump:       return prepareToJump(ai, player);
      case  InTheAir:            return inTheAir(ai, player);
      case  SlowToLand:          return slowToLand(ai, player);
      case  WalkToPoint:         return walkToPoint(ai, player);
   }
   return true;
}

