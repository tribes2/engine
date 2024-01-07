//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "game/gameBase.h"
#include "console/consoleTypes.h"
#include "console/consoleInternal.h"
#include "core/bitStream.h"
#include "sim/netConnection.h"
#include "game/targetManager.h"
#include "game/sensor.h"

IMPLEMENT_CO_DATABLOCK_V1(SensorData);


SensorData::SensorData()
{
   detects = true;
   detectsUsingLOS = true;
   detectsPassiveJammed = false;
   detectsActiveJammed = false;
   detectsCloaked = false;
   detectionPings = true;
   detectMinVelocity = 0;
   detectRadius = 250;
   detectsFOVOnly = false;
   detectFOV = 90.0;
   jams = false;
   jamsOnlyGroup = false;
   jamsUsingLOS = false;
   jamRadius = 0;
   detectFOVPercent = 0.f;
   useObjectFOV = false;
}

bool SensorData::onAdd()
{
   if(!Parent::onAdd())
      return false;
   halfFovCos = mCos(mDegToRad(detectFOV / 2.f));
   detectMinVSquared = detectMinVelocity * detectMinVelocity;
   jamRSquared = jamRadius * jamRadius;
   detectRSquared = detectRadius * detectRadius;
   return true;
}

void SensorData::initPersistFields()
{
   Parent::initPersistFields();
   addField("detects", TypeBool, Offset(detects, SensorData));
   addField("detectsUsingLOS", TypeBool, Offset(detectsUsingLOS, SensorData));
   addField("detectsActiveJammed", TypeBool, Offset(detectsActiveJammed, SensorData));
   addField("detectsPassiveJammed", TypeBool, Offset(detectsPassiveJammed, SensorData));
   addField("detectsCloaked", TypeBool, Offset(detectsCloaked, SensorData));
   addField("detectionPings", TypeBool, Offset(detectionPings, SensorData));
   addField("detectsFOVOnly", TypeBool, Offset(detectsFOVOnly, SensorData));
   addField("jams", TypeBool, Offset(jams, SensorData));
   addField("jamsOnlyGroup", TypeBool, Offset(jamsOnlyGroup, SensorData));
   addField("jamsUsingLOS", TypeBool, Offset(jamsUsingLOS, SensorData));
   addField("jamRadius", TypeF32, Offset(jamRadius, SensorData));
   addField("detectFOV", TypeF32, Offset(detectFOV, SensorData));
   addField("detectRadius", TypeF32, Offset(detectRadius, SensorData));
   addField("detectMinVelocity", TypeF32, Offset(detectMinVelocity, SensorData));
   addField("detectFOVPercent", TypeF32, Offset(detectFOVPercent, SensorData));
   addField("useObjectFOV", TypeBool, Offset(useObjectFOV, SensorData));
}

void SensorData::packData(BitStream* /*stream*/)
{
}

void SensorData::unpackData(BitStream* /*stream*/)
{
}


