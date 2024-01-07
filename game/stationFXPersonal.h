//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_STATIONFXPERSONAL
#define _H_STATIONFXPERSONAL

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif

class ShapeBase;


//--------------------------------------------------------------------------
// StationFXPersonalData
//--------------------------------------------------------------------------
class StationFXPersonalData : public GameBaseData
{
  public:

   enum StationFXConsts
   {
      SFXC_NUM_TEX = 2,
   };


   typedef GameBaseData Parent;

   F32               delay;
   F32               fadeDelay;
   F32               lifetime;
   F32               height;
   F32               leftRadius;
   F32               rightRadius;
   U32               numArcSegments;
   F32               numDegrees;
   F32               trailFadeTime;
   
   StringTableEntry  leftNodeName;
   StringTableEntry  rightNodeName;

   StringTableEntry  textureName[SFXC_NUM_TEX];
   TextureHandle     textureHandle[SFXC_NUM_TEX];



   StationFXPersonalData();
   DECLARE_CONOBJECT(StationFXPersonalData);
   bool onAdd();
   bool preload(bool server, char errorBuffer[256]);
   static void  initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};



//--------------------------------------------------------------------------
// StationFXPersonal
//--------------------------------------------------------------------------
class StationFXPersonal : public GameBase
{
   typedef GameBase Parent;

  private:
   StationFXPersonalData *    mDataBlock;
   S32                        mCurrMS;
   S32                        mStationObjectID;
   SimObjectPtr<ShapeBase>    mStationObject;
   F32                        mElapsedTime;

   void renderWall( F32 numSegments, F32 radius, F32 height, StringTableEntry nodeName, F32 numDegrees );

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
   StationFXPersonal();

   DECLARE_CONOBJECT(StationFXPersonal);
   static void initPersistFields();
};

#endif // _H_STATIONFXPERSONAL

