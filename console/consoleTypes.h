//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CONSOLETYPES_H_
#define _CONSOLETYPES_H_

#define Offset(x, cls) (S32)&(((cls *)0)->x)

enum ConsoleDynamicTypes {

   //Registered in ConsoleTypes.cc
   TypeS8 = 0,
   TypeS32,
   TypeS32Vector,
   TypeBool,
   TypeBoolVector,
   TypeF32,
   TypeF32Vector,
   TypeString,
   TypeCaseString,
   TypeEnum,
   TypeFlag,
   TypeColorI,
   TypeColorF,
   TypeSimObjectPtr,

   //Registered in MathTypes.cc
   TypePoint2I,
   TypePoint2F,
   TypePoint3F,
   TypePoint4F,
   TypeRectI,
   TypeRectF,
   TypeMatrixPosition,
   TypeMatrixRotation,
   TypeBox3F,
   
   //Registered in GuiTypes.cc
   TypeGuiProfile,

   // Game types
   TypeGameBaseDataPtr,
   TypeExplosionDataPtr,
   TypeShockwaveDataPtr,
   TypeSplashDataPtr,
   TypeEnergyProjectileDataPtr,
   TypeBombProjectileDataPtr,
   TypeParticleEmitterDataPtr,
   TypeAudioDescriptionPtr,
   TypeAudioProfilePtr,
   TypeTriggerPolyhedron,
   TypeProjectileDataPtr,
   TypeCannedChatItemPtr,
   TypeWayPointTeam,
   TypeDebrisDataPtr,
   TypeCommanderIconDataPtr,
   TypeDecalDataPtr,
   TypeEffectProfilePtr,
   TypeAudioEnvironmentPtr,
   TypeAudioSampleEnvironmentPtr,

   NumConsoleTypes
};

void RegisterCoreTypes(void);

#endif
