//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_SHOCKWAVE
#define _H_SHOCKWAVE

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif

class ParticleEmitter;
class ParticleEmitterData;
class AudioProfile;

//--------------------------------------------------------------------------
// Shockwave Data
//--------------------------------------------------------------------------
class ShockwaveData : public GameBaseData {
  public:
   typedef GameBaseData Parent;

   enum Constants
   {
      NUM_EMITTERS = 3,
      NUM_TIME_KEYS = 4,
      NUM_TEX = 2,
   };

  public:

   AudioProfile*        soundProfile;
   S32                  soundProfileId;

   ParticleEmitterData*    emitterList[NUM_EMITTERS];
   S32                     emitterIDList[NUM_EMITTERS];

   S32      delayMS;
   S32      delayVariance;
   S32      lifetimeMS;
   S32      lifetimeVariance;
   Point3F  scale;
   F32      width;
   F32      height;
   U32      numSegments;
   F32      velocity;
   U32      numVertSegments;
   F32      verticalCurve;
   F32      acceleration;
   F32      texWrap;
   bool     is2D;
   bool     mapToTerrain;
   bool     orientToNormal;
   bool     renderBottom;
   bool     renderSquare;

   F32      times[ NUM_TIME_KEYS ];
   ColorF   colors[ NUM_TIME_KEYS ];

   StringTableEntry  textureName[NUM_TEX];
   TextureHandle     textureHandle[NUM_TEX];


   ShockwaveData();
   DECLARE_CONOBJECT(ShockwaveData);
   bool onAdd();
   bool preload(bool server, char errorBuffer[256]);
   static void  initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};


//--------------------------------------------------------------------------
// Shockwave
//--------------------------------------------------------------------------
class Shockwave : public GameBase
{
   typedef GameBase Parent;

  private:
   ShockwaveData*    mDataBlock;

   ParticleEmitter * mEmitterList[ ShockwaveData::NUM_EMITTERS ];
   
   U32               mCurrMS;
   U32               mEndingMS;
   F32               mRandAngle;
   F32               mRadius;
   F32               mVelocity;
   F32               mHeight;
   ColorF            mColor;

  protected:
   Point3F  mInitialPosition;
   Point3F  mInitialNormal;
   F32      mFade;
   F32      mFog;
   bool     mActive;
   S32      mDelayMS;

  protected:
   bool onAdd();
   void onRemove();

   void  processTick(const Move*);
   void  advanceTime(F32 dt);
   void  updateEmitters( F32 dt );
   void  updateWave( F32 dt );
   void  renderWave();
   void  render2DWave();
   void  renderSquare();
   F32   findHeight( Point3F &point );
   void  renderVerticalWall( Point3F &pnt1, Point3F pnt2 );
   void  updateColor();

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);

  public:
   Shockwave();
   ~Shockwave();
   void setInitialState(const Point3F& point, const Point3F& normal, const F32 fade = 1.0);

   bool onNewDataBlock(GameBaseData* dptr);
   DECLARE_CONOBJECT(Shockwave);

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);

   static void consoleInit();
   static void initPersistFields();
};


#endif // _H_SHOCKWAVE
