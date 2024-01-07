//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_PROJFLAREGRENADE
#define _H_PROJFLAREGRENADE

#ifndef _PROJGRENADE_H_
#include "game/projGrenade.h"
#endif
#ifndef _LENSFLARE_H_
#include "dgl/lensFlare.h"
#endif

class LensFlare;

// -------------------------------------------------------------------------
class FlareProjectileData : public GrenadeProjectileData
{
   typedef GrenadeProjectileData Parent;
  
  public:
   enum FPDConsts
   {
      FPD_FLARE_TEX=0,
      FPD_LENS0_TEX,
      FPD_NUM_TEX,
   };

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   F32   size;
   F32   delay;
   bool  useLensFlare;

   StringTableEntry textureNameList[FPD_NUM_TEX];


   //-------------------------------------- load set variables
  public:
   TextureHandle textureList[FPD_NUM_TEX];


  public:
   FlareProjectileData();
   ~FlareProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(FlareProjectileData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class FlareProjectile : public GrenadeProjectile
{
   typedef GrenadeProjectile Parent;

   enum FPConsts
   {
      FP_NUM_BLUR_FLARES = 5,
   };

   struct BlurFlare
   {
      F32      alpha;
      Point3F  pos;
      F32      startAlpha;
      Point3F  screenPoint;
      
      BlurFlare()
      {
         alpha = 0.0;
         startAlpha = 0.5;
         pos.set( 0.0, 0.0, 0.0 );
      }

   };

  private:
   FlareProjectileData* mDataBlock;
   F32                  mLifetime;
   F32                  mElapsedTime;
   BlurFlare            mBlurList[ FP_NUM_BLUR_FLARES ];
   LensFlare            mLensFlare;

   void  renderFlare( const Point3F &camPos, F32 ang );
   void  setupLensFlare();

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);
   void processTick(const Move*);
   void advanceTime( F32 dt );
   void renderObject(SceneState*, SceneRenderImage*);
   void updateBlur();

  public:
   FlareProjectile();
   ~FlareProjectile();

   virtual F32 getHeat() const;

   DECLARE_CONOBJECT(FlareProjectile);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_PROJFLAREGRENADE

