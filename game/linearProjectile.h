//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _LINEARPROJECTILE_H_
#define _LINEARPROJECTILE_H_

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif

class TSThread;

//--------------------------------------------------------------------------
class LinearProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   F32  dryVelocity;
   F32  wetVelocity;

   S32  fizzleTimeMS;
   S32  lifetimeMS;
   bool explodeOnDeath;

   F32  reflectOnWaterImpactAngle;
   F32  deflectionOnWaterImpact;
   S32  fizzleUnderwaterMS;

   S32  activateDelayMS;
   bool doDynamicClientHits;
   //-------------------------------------- load set variables
  public:
   S32 activateSeq;
   S32 maintainSeq;

  public:
   LinearProjectileData();
   ~LinearProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   bool calculateAim(const Point3F& targetPos,
                     const Point3F& targetVel,
                     const Point3F& sourcePos,
                     const Point3F& sourceVel,
                     Point3F*       outputVectorMin,
                     F32*           outputMinTime,
                     Point3F*       outputVectorMax,
                     F32*           outputMaxTime);

   DECLARE_CONOBJECT(LinearProjectileData);
   static void initPersistFields();
   static void consoleInit();
};


//--------------------------------------------------------------------------
class LinearProjectile : public Projectile
{
   typedef Projectile Parent;

  private:
   LinearProjectileData* mDataBlock;

   bool     mWetStart;
   U32      mSplashTick;
   Point3F  mSplashPos;

  public:
   enum LPConstants {
      InitialDirectionBits = 10,

      MaxLivingTicks       = 511
   };

   enum LPUpdateMasks {
      ExplosionMask = Parent::NextFreeMask,
      NextFreeMask  = Parent::NextFreeMask << 1
   };

  protected:
   struct Segment {
      Point3F start;
      Point3F end;
      Point3F segmentVel;

      bool    cutShort;
      U32     endTypeMask;
      Point3F endNormal;

      U32     msStart;
      U32     msEnd;

      SceneObject *hitObj;

      Segment()
      {
         dMemset( this, 0, sizeof( Segment ) );
      }

   };

	bool calculateImpact(float simTime,
	                     Point3F& pointOfImpact,
                        float& impactTime);

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

   virtual void renderObject(SceneState*, SceneRenderImage*);

   bool updatePos(const Point3F&, const Point3F&, F32*, Point3F*, Point3F*, SceneObject*&);
   void processTick(const Move*);
   void interpolateTick(F32);
   void advanceTime(F32);

   Point3F mExplosionPosition;
   Point3F mExplosionNormal;
   bool    mEndedWithDecal;
   static const U32 csmDecalMask;

   void explode(const Point3F& p, const Point3F& n, const bool dynamicObject);

   Point3F getVelocity() const;

   void  calcSplash();
   void  createSplash();
   bool  determineWetStart();

   // Only valid on tick boundaries...
   Point3F deriveExactPosition(const U32 tick) const;
   Point3F deriveExactVelocity(const U32 tick) const;

   Segment mSegments[2];
   U32     mNumSegments;
   U32     mDeleteTick;
   bool createSegments();   // sets the above variables

   // Warping and back delta variables.  Only valid on the client
   //
   Point3F mWarpStart;
   Point3F mWarpEnd;
   U32     mWarpTicksRemaining;

   Point3F mCurrDeltaBase;
   Point3F mCurrBackDelta;

   // activate/maintain threads
   TSThread* mActivateThread;
   TSThread* mMaintainThread;

   bool     mHitWater;
   bool     mPlayedSplash;

  public:
   LinearProjectile();
   ~LinearProjectile();

   DECLARE_CONOBJECT(LinearProjectile);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_LINEARPROJECTILE

