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

enum SimObjectTypes
{
   #define bit(x) (1 << (x))

   // Types used by the SceneObject class
   DefaultObjectType =           0,
   StaticObjectType =            bit(0),

   // Basic Engine Types
   EnvironmentObjectType =       bit(1),
   TerrainObjectType =           bit(2),
   InteriorObjectType =          bit(3),
   WaterObjectType =             bit(4),
   TriggerObjectType =           bit(5),
   MarkerObjectType =            bit(6),
   UNUSED_AVAILABLE =            bit(7),
   ForceFieldObjectType =        bit(8),
   DecalManagerObjectType =      bit(9),

   // Game Types
   GameBaseObjectType =          bit(10),
   ShapeBaseObjectType =         bit(11),
   CameraObjectType =            bit(12),
   StaticShapeObjectType =       bit(13),
   PlayerObjectType =            bit(14),
   ItemObjectType =              bit(15),
   VehicleObjectType =           bit(16),
   VehicleBlockerObjectType =    bit(17),
   ProjectileObjectType =        bit(18),
   ExplosionObjectType  =        bit(19),
   CorpseObjectType =            bit(20),
   TurretObjectType =            bit(21),
   DebrisObjectType =            bit(22),
   PhysicalZoneObjectType =      bit(23),
   StaticTSObjectType =          bit(24),
   GuiControlObjectType =        bit(25),

   StaticRenderedObjectType =    bit(26),
   
   // The following are allowed types that can be set on datablocks for static shapes
   //
   DamagableItemObjectType =     bit(27),
   SensorObjectType =            bit(28),
   StationObjectType =           bit(29),
   GeneratorObjectType =         bit(30)
};

#define STATIC_COLLISION_MASK   (   TerrainObjectType    |  \
                                    InteriorObjectType   |  \
                                    ForceFieldObjectType |  \
                                    StaticObjectType )      \

#define DAMAGEABLE_MASK  ( PlayerObjectType        |  \
                           VehicleObjectType       |  \
                           StationObjectType       |  \
                           GeneratorObjectType     |  \
                           SensorObjectType        |  \
                           DamagableItemObjectType |  \
                           TurretObjectType )         \

#endif
