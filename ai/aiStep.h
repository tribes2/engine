//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AISTEP_H_
#define _AISTEP_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GAMECONNECTION_H_
#include "game/gameConnection.h"
#endif
#ifndef _PLAYER_H_
#include "game/player.h"
#endif

class AIStep : public SimObject
{
	public:
		enum StepResult
		{
			InProgress = 0,
			Failed,
			Finished
		};
		int mStatus;

	protected:
		float get2DAngle(const Point3F &endPt, const Point3F &basePt);

	public:
		AIStep();
		int getStatus() { return mStatus; }
		virtual void process(AIConnection *ai, Player *player);
};

class AIStepEscort : public AIStep
{
   typedef AIStep Parent;

	bool mInitialized;
	bool mProximityBuffer;
	S32 mResetDestinationCounter;
	SimObjectPtr<GameConnection> mClientToEscort;

	//add some idle vars
	S32 mStoppedTime;
	Vector<Point3F> mChokePoints;
	bool mIdleStarted;
	S32 mIdleNextTime;
   
   public:
      AIStepEscort(GameConnection *clientToEscort = NULL);
		void process(AIConnection *ai, Player *player);
};

class AIStepEngage : public AIStep
{
   typedef AIStep Parent;

	bool mInitialized;
	SimObjectPtr<GameConnection> mTarget;

	S32 mStraifeCounter;
	S32 mPauseCounter;
	S32 mCheckLOSCounter;
	bool mClearLOSToTarget;
	bool mPausing;
	Point3F mStraifeLocation;

	bool mUsingEnergyWeapon;
	float mEnergyWeaponRecharge;

	bool mSearching;
	bool mSearchInitialized;
	Point3F mChokeLocation;
	Vector<Point3F> mChokePoints;
	S32 mChokeIndex;
	S32 mSearchTimer;
	bool mSearchMove;

   public:
      AIStepEngage(GameConnection *target = NULL);
		void process(AIConnection *ai, Player *player);

		void initProcessVars(Player *player, Player *targPlayer);
		Point3F findStraifeLocation(AIConnection *client);
};

class AIStepRangeObject : public AIStep
{
   typedef AIStep Parent;

	bool mInitialized;
	SimObjectPtr<GameBase> mTargetObject;
   ProjectileData *mProjectile;
	U32 mLOSMask;

	F32 mMinDistance;
	F32 mMaxDistance;
	Point3F mTargetPoint;
	Point3F mGraphDestination;
	Point3F mFromLocation;
	Point3F mPrevLocation;
	S32 mCheckLOSCounter;

   public:
      AIStepRangeObject(GameBase *targetObject = NULL, const char *projectile = NULL, F32 *minDist = NULL, F32 *maxDist = NULL, Point3F *fromLocation = NULL);
		void process(AIConnection *ai, Player *player);
};

class AIStepIdlePatrol : public AIStep
{
   typedef AIStep Parent;

	bool mInitialized;
	Point3F mIdleLocation;
	Point3F mMoveLocation;
	Vector<Point3F> mChokePoints;
	S32 mChokeIndex;
	F32 mOutdoorRadius;

	enum IdleStates
	{
		MoveToLocation,
		LookAround,
	};
	S32 mIdleState;
	S32 mIdleNextTime;
	S32 mIdleEndTime;
	bool mStateInit;
	bool mHeadingHome;

   public:
      AIStepIdlePatrol(Point3F *idleLocation = NULL);
		void process(AIConnection *ai, Player *player);
};

#endif
