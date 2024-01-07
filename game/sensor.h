//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SENSOR_H_
#define _SENSOR_H_

struct SensorData : public SimDataBlock {
  private:
   typedef SimDataBlock Parent;
   
  public:
   bool detects;
   bool detectsUsingLOS;
   bool detectsPassiveJammed;
   bool detectsActiveJammed;
   bool detectsCloaked;
   bool detectionPings;
   bool detectsFOVOnly;
   F32 detectMinVelocity;
   F32 detectMinVSquared;
   F32 detectRadius;
   F32 detectRSquared;
   F32 detectFOV;
   F32 halfFovCos;
   
   bool jams;
   bool jamsOnlyGroup;
   bool jamsUsingLOS;
   F32 jamRadius;
   F32 jamRSquared;

   F32 detectFOVPercent;
   bool useObjectFOV;

   SensorData();
   DECLARE_CONOBJECT(SensorData);
   static void initPersistFields();
   bool onAdd();
   void packData(BitStream* stream);
   void unpackData(BitStream* stream);
};

#endif
