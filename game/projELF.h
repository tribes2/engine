//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PROJELF_H_
#define _PROJELF_H_

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif

class SplinePatch;
class QuadPatch;
class ParticleEmitter;
class ParticleEmitterData;

// -------------------------------------------------------------------------
class ELFProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   enum ELFTextures
   {
      ET_WAVE = 0,
      ET_BEAM,
      ET_BALL,
      ET_NUM_TEX
   };

   enum ELFConst
   {
      ELF_NUM_EMITTERS = 1,
   };

   F32 beamRange;
   F32 beamHitWidth;

   StringTableEntry textureNames[ET_NUM_TEX];

   ParticleEmitterData*    emitterList[ELF_NUM_EMITTERS];
   S32                     emitterIDList[ELF_NUM_EMITTERS];

   F32 mainBeamWidth;
   F32 mainBeamSpeed;
   F32 mainBeamRepeat;
   F32 lightningWidth;
   F32 lightningDist;

   //-------------------------------------- preload set variables
  public:
   F32            cosBeamHitWidth;
   TextureHandle  textureList[ET_NUM_TEX];

  public:
   ELFProjectileData();
   ~ELFProjectileData();


   bool calculateAim(const Point3F& targetPos,
                     const Point3F& targetVel,
                     const Point3F& sourcePos,
                     const Point3F& sourceVel,
                     Point3F*       outputVectorMin,
                     F32*           outputMinTime,
                     Point3F*       outputVectorMax,
                     F32*           outputMaxTime);

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(ELFProjectileData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class ELFProjectile : public Projectile
{
   typedef Projectile Parent;

  protected:
   enum ELFNetMasks {
      TargetChangedMask = Parent::NextFreeMask << 0,
      NextFreeMask      = Parent::NextFreeMask << 1
   };

  private:
   ELFProjectileData* mDataBlock;

   SimObjectPtr<ShapeBase> mTargetObject;
   S32                     mTargetObjectId;

   Point3F                 mCurrEndPoint;
   AUDIOHANDLE 			   mWetFireHandle;
   AUDIOHANDLE             mFireHandle;

   ParticleEmitter *       mEmitterList[ELFProjectileData::ELF_NUM_EMITTERS];
   bool                    mEmittingParticles;

   void  updateEmitters( F32 dt );

  public:

   enum LightningConst
   {
      LC_NUM_POINTS = 16,
      LC_NUM_BOLTS = 3
   };


   struct LightningBolt
   {
      Point3F  points[LC_NUM_POINTS];
      Point3F  randPoints[LC_NUM_POINTS];
      U32      numPoints;
      F32      alpha;
      F32      fadeTime;
      F32      elapsedTime;

      LightningBolt()
      {
         dMemset( this, 0, sizeof( LightningBolt ) );
         numPoints = LC_NUM_POINTS;
         fadeTime = 0.2;
      }

   };

   LightningBolt  mBoltList[LC_NUM_BOLTS];

  protected:
   F32         mElapsedTime;
   F32         mBoltTime;
   bool        mNewBolt;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);

   void findTarget();

   bool findContactPoint( SplinePatch &spline, Point3F &point, SceneObject **contactObj=NULL );

   void createBolt( LightningBolt &bolt, SplinePatch &spline );
   void createBoltRandPoints( LightningBolt &bolt, F32 sideMag );
   void renderBolt( LightningBolt &bolt, const Point3F &camPos, F32 width );


   void releaseTarget(ShapeBase*);
   void acquireTarget(ShapeBase*);
   
	bool calculateImpact(float    simTime,
	                     Point3F& pointOfImpact,
                        float&   impactTime);

   void renderFlare( F32 flareSize );
   bool checkForFlare( const Point3F &camPos );

   bool setupSpline( QuadPatch &spline, SceneObject **contactObj=NULL  );

   void calcSnapPoint( ShapeBase &obj, Point3F &point );

  public:
   ELFProjectile();
   ~ELFProjectile();

   // Time/Move Management
  public:
   void processTick(const Move*);
   void advanceTime(F32);
   bool hasTarget(); 

   DECLARE_CONOBJECT(ELFProjectile);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

inline bool ELFProjectile::hasTarget()
{
   return !mTargetObject.isNull();
}

#endif // _H_ELFPROJECTILE
