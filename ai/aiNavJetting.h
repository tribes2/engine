//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AINAVJETTING_H_
#define _AINAVJETTING_H_

enum AIJetStatus 
{
   AIJetInactive, 
   AIJetWorking, 
   AIJetFail, 
   AIJetSuccess
};

class AIConnection;

class AIJetting 
{
   protected:
      enum JettingStates{
         AssureClear, 
         AwaitEnergy, 
         PrepareToJump, 
         InTheAir, 
         SlowToLand,
         WalkToPoint,
      };
   
      bool           mFirstTime;
      bool           mUpChute;
      bool           mIntoMount;
      bool           mShouldAim;
      bool           mCanInterupt;
      bool           mWillBonk;
      Point3F        mSeekDest;
      Point3F        mLandPoint;
      VectorF        mWallNormal;
      Point3F        mJumpPoint;
      Point3F        mWallPoint;
      Point3F        mTopOfChute;
      F32            mTotal2D;
      F32            mLaunchSpeed;
      F32            mSlope;
      S32            mState;
      S32            mCounter;
      AIJetStatus    mStatus;
      NavJetting *   mJetInfo;
      
      void  newState(S16 state, S16 counter = 0);
      F32   distFromWall(const Point3F& loc);
      void  setAim(const Point3F& at);
      bool  willBonk(Point3F src, Point3F dst);
      bool  figureLandingDist(AIConnection* ai, F32& dist);
      bool  firstTimeStuff(AIConnection* ai, Player* player);
      
      // States:
      bool  assureClear(AIConnection* ai, Player* player);
      bool  awaitEnergy(AIConnection* ai, Player* player);
      bool  prepareToJump(AIConnection* ai, Player* player);
      bool  inTheAir(AIConnection* ai, Player* player);
      bool  slowToLand(AIConnection* ai, Player* player);
      bool  walkToPoint(AIConnection* ai, Player* player);
   
  public:
      AIJetting()             {reset();}
      AIJetStatus status()    {return mStatus;}
      void reset()            {mStatus = AIJetInactive;}
      
      bool     init(const Point3F& dest, bool intoMount = false, NavJetting * inf = 0);
	   bool     process(AIConnection* ai, Player* player);
      bool     shouldAimAt(Point3F& atWhere);
      bool     badTimeToSearch() const {return true;} // {return !mCanInterupt;}
};

#endif
