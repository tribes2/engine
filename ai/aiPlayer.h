//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AIPLAYER_H_
#define _AIPLAYER_H_

#include "ai/aiConnection.h"
#include "game/player.h"

class AIPlayer : public AIConnection {

	typedef AIConnection Parent;

	private:
		enum {
			FireTrigger = 0,
			JumpTrigger = 2,
			JetTrigger = 3,
			GrenadeTrigger = 4,
			MineTrigger = 5
		};

		F32 mMoveSpeed;
		S32 mMoveMode;
		F32 mMoveTolerance; // How close to the destination before we stop

		bool mTriggers[MaxTriggerKeys];

		bool mTargetInSight;
		Player *mPlayer;

		Point3F mMoveDestination;
		Point3F mLocation;
		Point3F mRotation; // Euler really

		bool mAimToDestination; // Why is this in here? 
		Point3F mAimLocation;	// Because objects would have facing as well.

		
		SimObjectPtr<ShapeBase> mTargetObject;
	public:
		
		DECLARE_CONOBJECT( AIPlayer );

		enum
		{
			ModeStop = 0,
			ModeWalk,		// Walk is runSpeed / 2
			ModeRun,
			ModeIdle,
			ModeCount		// This is in there as a max index value
		};

		AIPlayer();

		void getMoveList( Move **movePtr,U32 *numMoves );
		static void consoleInit();

		// ---Targeting and aiming sets/gets
		void setTargetObject( ShapeBase *targetObject );
		S32 getTargetObject();
		bool targetInSight() { return mTargetInSight; }

		// ---Movement sets/gets
		void setMoveSpeed( F32 speed );
		F32 getMoveSpeed() { return mMoveSpeed; }

		void setMoveMode( S32 mode );
		S32 getMoveMode() { return mMoveMode; }

		void setMoveTolerance( F32 tolerance );
		F32 getMoveTolerance() { return mMoveTolerance; }

		void setMoveDestination( const Point3F &location );
		Point3F getMoveDestination() { return mMoveDestination; }

		// ---Facing(Aiming) sets/gets
		void setAimLocation( const Point3F &location );
		Point3F getAimLocation() { return mAimLocation; }
		void clearAim();

		// ---Other
		void missionCycleCleanup();

		// ---Callbacks
};

#endif