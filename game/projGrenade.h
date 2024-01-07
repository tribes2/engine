//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRENADEPROJECTILE_H_
#define _GRENADEPROJECTILE_H_

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif

//--------------------------------------------------------------------------
class GrenadeProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   F32 grenadeElasticity;
   F32 grenadeFriction;

   F32 gravityMod;

   F32 muzzleVelocity;
   S32 armingDelayMS;

   F32 drag;
   F32 density;

   U32 lifetimeMS;

   //-------------------------------------- load set variables
  public:

  public:
   GrenadeProjectileData();
   ~GrenadeProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);

   bool calculateAim(const Point3F& targetPos,
                     const Point3F& targetVel,
                     const Point3F& sourcePos,
                     const Point3F& sourceVel,
                     Point3F*       outputVectorMin,
                     F32*           outputMinTime,
                     Point3F*       outputVectorMax,
                     F32*           outputMaxTime);

   DECLARE_CONOBJECT(GrenadeProjectileData);
   static void initPersistFields();
};


class GrenadeProjectile : public Projectile
{
   typedef Projectile Parent;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

  protected:
   void processTick(const Move*);
   void interpolateTick(F32);
   void advanceTime(F32);

   Point3F getVelocity() const;

  public:
   enum GrenConstants {
      InitialDirectionBits = 11,
      MaxLivingTicks       = 4095
   };

   enum GrenUpdateMasks {
      BounceMask    = Parent::NextFreeMask,
      ExplosionMask = Parent::NextFreeMask << 1,
      NextFreeMask  = Parent::NextFreeMask << 2
   };

  private:
   GrenadeProjectileData* mDataBlock;

   // splash / water related
  protected:
   Point3F  mSplashPos;
   bool     mLastInWater;
   bool     mInWater;
   bool     mQuickSplash;
   bool     mSentInitialUpdate;

   bool determineSplash( Point3F &oldPos, Point3F &newPos );
   void resolveInitialSplash();
   void updateBubbles( F32 dt );


  protected:
   S32      mDeleteTick;
   S32      mArmedTick;

   Point3F  mCurrPosition;
   Point3F  mCurrVelocity;

   Point3F  mCurrDeltaBase;      // only valid on the client
   Point3F  mCurrBackVector;

   Point3F  mExplosionPosition;
   Point3F  mExplosionNormal;


   void computeNewState(Point3F* newPosition,
                        Point3F* newVelocity);
   void explode(const Point3F&, const Point3F&, const U32 collideType = 0 );

   bool calculateImpact(float    simTime,
                        Point3F& pointOfImpact,
                        float&   impactTime);


  public:
   GrenadeProjectile();
   ~GrenadeProjectile();

   DECLARE_CONOBJECT(GrenadeProjectile);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_GRENADEPROJECTILE

