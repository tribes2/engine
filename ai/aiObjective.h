//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AIOBJECTIVE_H_
#define _AIOBJECTIVE_H_

#ifndef _BITSTREAM_H_
#include "core/bitStream.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _SHAPEBASE_H_
#include "game/shapeBase.h"
#endif
#ifndef _MATHIO_H_
#include "math/mathIO.h"
#endif
#ifndef _SPHERE_H_
#include "game/sphere.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _MISSIONMARKER_H_
#include "game/missionMarker.h"
#endif


class AIObjective : public MissionMarker
{
	private:
		typedef MissionMarker Parent;
      static Sphere         smSphere;

		StringTableEntry mDescription;
		StringTableEntry mMode;
		StringTableEntry mTargetClient;
      StringTableEntry mTargetObject;
      S32 mTargetClientId;
		S32 mTargetObjectId;
		Point3F mLocation;

		S32 mWeightLevel1;
		S32 mWeightLevel2;
		S32 mWeightLevel3;
		S32 mWeightLevel4;
		bool mOffense;
		bool mDefense;

		StringTableEntry mRequiredEquipment;
		StringTableEntry mDesiredEquipment;
		StringTableEntry mBuyEquipmentSets;

		StringTableEntry mCannedChat;
      bool             mLocked;

		bool mIssuedByHuman;
		S32 mIssuedByClientId;
		S32 mForceClientId;

	public:
		AIObjective();
		S32 getSortWeight() const { return mWeightLevel1; }
      
      static void initPersistFields();
	   static void consoleInit();

      // SimObject      
      bool onAdd();
      void inspectPostApply();
      
      // NetObject
      enum SpawnSphereMasks {
         UpdateSphereMask = Parent::NextFreeMask,
         NextFreeMask = Parent::NextFreeMask << 1
      };

      // NetObject
      U32 packUpdate(NetConnection *, U32, BitStream *);
      void unpackUpdate(NetConnection *, BitStream *);

      DECLARE_CONOBJECT(AIObjective);
};


class	AIObjectiveQ : public SimSet
{
	private:
		typedef SimSet Parent;

	public:
	   DECLARE_CONOBJECT(AIObjectiveQ);
	   static void consoleInit();
		void addObject(SimObject *obj);

	   static S32 QSORT_CALLBACK compareWeight(const void* a,const void* b);
		void sortByWeight();
};

#endif
