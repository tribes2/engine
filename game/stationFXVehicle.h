//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_STATIONFXVEHICLE
#define _H_STATIONFXVEHICLE

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif

class ShapeBase;


//--------------------------------------------------------------------------
// StationFXVehicleData
//--------------------------------------------------------------------------
class StationFXVehicleData : public GameBaseData
{
  public:

   enum StationFXConsts
   {
      SFXC_NUM_TEX = 2,
      SFXC_NUM_NODES = 4,
   };


   typedef GameBaseData Parent;

   F32               glowTopHeight;
   F32               glowBottomHeight;
   F32               glowTopRadius;
   F32               glowBottomRadius;
   U32               numGlowSegments;
   F32               glowFadeTime;

   F32               armLightDelay;
   F32               armLightLifetime;
   F32               armLightFadeTime;

   F32               lifetime;
   U32               numArcSegments;

   ColorF            sphereColor;
   U32               spherePhiSegments;
   U32               sphereThetaSegments;
   F32               sphereRadius;
   VectorF           sphereScale;

   StringTableEntry  glowNodeName;
   StringTableEntry  leftNodeName[SFXC_NUM_NODES];
   StringTableEntry  rightNodeName[SFXC_NUM_NODES];

   StringTableEntry  textureName[SFXC_NUM_TEX];
   TextureHandle     textureHandle[SFXC_NUM_TEX];



   StationFXVehicleData();
   DECLARE_CONOBJECT(StationFXVehicleData);
   bool onAdd();
   bool preload(bool server, char errorBuffer[256]);
   static void  initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};



//--------------------------------------------------------------------------
// StationFXVehicle
//--------------------------------------------------------------------------
class StationFXVehicle : public GameBase
{
   typedef GameBase Parent;

  private:
   StationFXVehicleData *     mDataBlock;
   S32                        mCurrMS;
   S32                        mStationObjectID;
   SimObjectPtr<ShapeBase>    mStationObject;
   F32                        mElapsedTime;

   void     renderGlow();
   void     renderWall( F32 topRadius, F32 bottomRadius, F32 numDegrees, 
                        StringTableEntry topNode, StringTableEntry bottomNode );

   void     renderHemisphere( F32 numPhiSegments, F32 numThetaSegments, F32 radius );

   F32      findRadius( StringTableEntry node1, StringTableEntry node2 );
   MatrixF  getNodeTransform( StringTableEntry nodeName );

  protected:
   bool onAdd();
   void onRemove();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);

   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);

   void advanceTime(F32);
   bool onNewDataBlock(GameBaseData* dptr);
   void processTick(const Move*);

  public:
   StationFXVehicle();

   DECLARE_CONOBJECT(StationFXVehicle);
   static void initPersistFields();
};

#endif // _H_STATIONFXVEHICLE

