//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PROJBOMB_H_
#define _PROJBOMB_H_

#ifndef _PROJGRENADE_H_
#include "game/projGrenade.h"
#endif

//--------------------------------------------------------------------------
// Bomb Projectile Data
//--------------------------------------------------------------------------
class BombProjectileData : public GrenadeProjectileData
{
   typedef GrenadeProjectileData Parent;

   enum Constants
   {
      NUM_TEX = 2,
   };

  protected:
   bool onAdd();

  public:

   StringTableEntry  textureName[NUM_TEX];
   TextureHandle     textureHandle[NUM_TEX];
   VectorF           minRotSpeed;
   VectorF           maxRotSpeed;


  public:
   BombProjectileData();
   ~BombProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);


   DECLARE_CONOBJECT(BombProjectileData);
   static void initPersistFields();
   bool preload(bool server, char errorBuffer[256]);
};

//--------------------------------------------------------------------------
// Bomb Projectile
//--------------------------------------------------------------------------
class BombProjectile : public GrenadeProjectile
{
  typedef GrenadeProjectile Parent;

  private:
   BombProjectileData*     mDataBlock;
   VectorF                 mRotSpeed;

  protected:
   bool     onAdd();
   void     onRemove();
   bool     onNewDataBlock(GameBaseData*);
   void     processTick(const Move*);
   void     advanceTime(F32);
   void     renderObject(SceneState*, SceneRenderImage*);
//   void     renderProjectile( const Point3F &camPos );
//   void     renderCrossSection( const Point3F &camPos );
   bool     prepRenderImage(SceneState*, const U32, const U32, const bool);
   void     rotate( F32 dt );

  public:
   BombProjectile();
   ~BombProjectile();

   static void initPersistFields();
   static void consoleInit();

//   U32   packUpdate(NetConnection*, U32 mask, BitStream* stream);
//   void  unpackUpdate(NetConnection*, BitStream* stream);

   DECLARE_CONOBJECT(BombProjectile);
};

#endif // _H_PROJBOMB

