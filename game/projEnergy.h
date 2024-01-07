//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PROJENERGY_H_
#define _PROJENERGY_H_

#ifndef _PROJGRENADE_H_
#include "game/projGrenade.h"
#endif
#ifndef _MOTIONBLURLINE_H_
#include "game/motionBlurLine.h"
#endif

//--------------------------------------------------------------------------
// Energy Projectile Data
//--------------------------------------------------------------------------
class EnergyProjectileData : public GrenadeProjectileData
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

   F32               crossViewAng;
   F32               crossSize;
   F32               blurLifetime;
   F32               blurWidth;
   ColorF            blurColor;



  public:
   EnergyProjectileData();
   ~EnergyProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool calculateAim(const Point3F&, const Point3F&, const Point3F&, const Point3F&, Point3F*, F32*, Point3F*, F32*);

   DECLARE_CONOBJECT(EnergyProjectileData);
   static void initPersistFields();
   bool preload(bool server, char errorBuffer[256]);
};

//--------------------------------------------------------------------------
// Energy Projectile
//--------------------------------------------------------------------------
class EnergyProjectile : public GrenadeProjectile
{
  typedef GrenadeProjectile Parent;

  private:
   EnergyProjectileData*   mDataBlock;
   MotionBlurLine          mMotionBlur;
   Point3F                 mLastPos;

  protected:
   void     collisionOn();
   void     collisionOff();
   bool     onAdd();
   void     onRemove();
   bool     onNewDataBlock(GameBaseData*);
   void     processTick(const Move*);
   void     advanceTime(F32);
   void     renderObject(SceneState*, SceneRenderImage*);
   void     renderProjectile( const Point3F &camPos );
   void     renderCrossSection( const Point3F &camPos );
   bool     prepRenderImage(SceneState*, const U32, const U32, const bool);

  public:
   EnergyProjectile();
   ~EnergyProjectile();

   static void initPersistFields();
   static void consoleInit();

   U32   packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void  unpackUpdate(NetConnection*, BitStream* stream);

   DECLARE_CONOBJECT(EnergyProjectile);
};

#endif // _H_PROJENERGY

