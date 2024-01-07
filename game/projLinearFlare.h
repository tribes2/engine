//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_LINEARFLAREPROJECTILE
#define _H_LINEARFLAREPROJECTILE

#ifndef _LINEARPROJECTILE_H_
#include "game/linearProjectile.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif

// -------------------------------------------------------------------------
class LinearFlareProjectileData : public LinearProjectileData
{
   typedef LinearProjectileData Parent;

  public:
   enum Consts
   {
      NUM_SIZES = 3,
   };

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   S32      numFlares;
   F32      size[NUM_SIZES];
   ColorF   flareColor;

   StringTableEntry flareModTexture;
   StringTableEntry flareBaseTexture;

   //-------------------------------------- load set variables
  public:
   TextureHandle baseHandle;
   TextureHandle modHandle;

  public:
   LinearFlareProjectileData();
   ~LinearFlareProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(LinearFlareProjectileData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class LinearFlareProjectile : public LinearProjectile
{
   typedef LinearProjectile Parent;

  private:
   LinearFlareProjectileData* mDataBlock;

   struct Flare {
      F32 minTheta;
      F32 maxTheta;

      F32 h0;
      F32 h1;
      F32 h2;

      F32 t0;
      F32 t1;
      F32 t2;

      F32 a0;
      F32 a1;
      F32 a2;

      F32 currT;
      bool active;
      Point3F normal;

      Point3F points[16];
      void generatePoints();
      void render(const ColorF&, const F32);
   };
   Flare* mFlares;
   TextureHandle mBaseHandle;
   TextureHandle mHandle;


  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

  public:
   LinearFlareProjectile();
   ~LinearFlareProjectile();

   DECLARE_CONOBJECT(LinearFlareProjectile);
   static void initPersistFields();

   void advanceTime(F32);
   void renderObject(SceneState*, SceneRenderImage*);

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_LINEARFLAREPROJECTILE

