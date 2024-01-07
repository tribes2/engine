//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _OBJECTTYPES_H_
#define _OBJECTTYPES_H_

// Types used for SimObject type masks (SimObject::mTypeMask)
//

/* NB!  If a new object type is added, don't forget to add it to the
 *      consoleInit function in simBase.cc
 */

// SimObjectTypes

// Types used by the SceneObject class
#define   DefaultObjectType         0
#define   StaticObjectType          (1<<0)

// Basic Engine Types
#define   EnvironmentObjectType     (1<<1)
#define   TerrainObjectType         (1<<2)
#define   InteriorObjectType        (1<<3)
#define   WaterObjectType           (1<<4)
#define   TriggerObjectType         (1<<5)
#define   MarkerObjectType          (1<<6)
#define   PathedObjectType          (1<<7)
#define   ForceFieldObjectType      (1<<8)
#define   DecalManagerObjectType    (1<<9)

#define   PlayerObjectType          (1<<14)
#define   GuiControlObjectType      (1<<25)
#define   StaticRenderedObjectType  (1<<26)


// Game Types
#define   GameBaseObjectType        (1<<10)
#define   ShapeBaseObjectType       (1<<11)
#define   CameraObjectType          (1<<12)
#define   StaticShapeObjectType     (1<<13)
#define   ItemObjectType            (1<<15)
#define   VehicleObjectType         (1<<16)
#define   MoveableObjectType        (1<<17)
#define   ProjectileObjectType      (1<<18)
#define   ExplosionObjectType       (1<<19)
#define   CorpseObjectType          (1<<20)
#define   TurretObjectType          (1<<21)
#define   DebrisObjectType          (1<<22)
#define   PhysicalZoneObjectType    (1<<23)
#define   StaticTSObjectType        (1<<24)
   
// The following are allowed types that can be set on datablocks for static shapes
//
#define   DamagableItemObjectType   (1<<27)
#define   SensorObjectType          (1<<28)
#define   StationObjectType         (1<<29)
#define   GeneratorObjectType       (1<<30)


#define STATIC_COLLISION_MASK   (   TerrainObjectType    |  \
                                    InteriorObjectType   |  \
                                    ForceFieldObjectType |  \
                                    StaticObjectType )      \

#define DAMAGEABLE_MASK  ( PlayerObjectType        |  \
                           VehicleObjectType       |  \
                           MoveableObjectType      |  \
                           StationObjectType       |  \
                           GeneratorObjectType     |  \
                           SensorObjectType        |  \
                           PathedObjectType        |  \
                           DamagableItemObjectType |  \
                           TurretObjectType )         \

#endif
