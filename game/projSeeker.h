//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_SEEKERPROJECTILE
#define _H_SEEKERPROJECTILE

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif
#ifndef _PARTICLEENGINE_H_
#include "game/particleEngine.h"
#endif

struct DebrisData;
class TSShapeInstance;
class TSShape;

//--------------------------------------------------------------------------
class SeekerProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   U32   lifetimeMS;

   F32   muzzleVelocity;
   F32   turningSpeed;
   F32   maxVelocity;
   F32   acceleration;

   F32   proximityRadius;

   F32   terrainAvoidanceSpeed;
   F32   terrainScanAhead;
   F32   terrainHeightFail;
   F32   terrainAvoidanceRadius;

   F32   flareDistance;
   F32   flareAngle;

   bool  useFlechette;
   U32   flechetteDelayMs;

   ParticleEmitterData* puffEmitter;
   S32                  puffEmitterID;

   ParticleEmitterData* exhaustEmitter;
   S32                  exhaustEmitterID;
   S32                  exhaustTimeMs;
   StringTableEntry     exhaustNodeName;

   DebrisData *         casingDeb;
   S32                  casingDebID;

   StringTableEntry     casingShapeName;


   //-------------------------------------- load set variables
  public:
   Resource<TSShape> casingShape;

   F32   cosTurningSpeed;
   F32   cosAvoidSpeed;
   F32   cosFlareAngle;

  public:
   SeekerProjectileData();
   ~SeekerProjectileData();

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

   DECLARE_CONOBJECT(SeekerProjectileData);
   static void initPersistFields();
};


class SeekerProjectile : public Projectile
{
   typedef Projectile Parent;
   friend void cSetTarget(SimObject*, S32, const char**);

  private:
   SeekerProjectileData* mDataBlock;
   TSShapeInstance* mCasingShape;

  public:
   enum LPUpdateMasks {
      SeekerUpdateMask = Parent::NextFreeMask,
      ExplosionMask    = Parent::NextFreeMask << 1,
      NextFreeMask     = Parent::NextFreeMask << 2
   };
   enum TargetingMode {
      Object,
      Position,
      None
   };

   Point3F                 mTargetPosition;
   S32                     mTargetId;
   TargetingMode           mTargetingMode;
   StringTableEntry        mTargetingString;

   SimObjectPtr<GameBase> mTargetPtr;
   SimObjectPtr<GameBase> mOriginalTargetPtr;

  protected:
   bool                    mExplosionPending;
   bool                    mHitWater;
   S32                     mDeleteTick;
   S32                     mDeathTick;

   Point3F                 mCurrPosition;
   Point3F                 mCurrVelocity;
   Point3F                 mPlayerVel;
   Point3F                 mLastPos;

   // Client side only variables
   Point3F     mLastServerPos;
   Point3F     mLastServerVel;
   Point3F     mServerTarget;
   bool        mServerTargetValid;

   U32         mWarpTicksLeft;
   U32         mNumWarpTicks;
   Point3F     mWarpStart;
   Point3F     mWarpEnd;
   Point3F     mWarpEndVelocity;

   Point3F     mCurrDelta;
   Point3F     mCurrDeltaBase;

   ParticleEmitter* mPuffEmitter;
   ParticleEmitter* mExhaustEmitter;

   bool        mFlechettePuff;

   U32         mFlechetteGhostTick;

   Point3F     mExplosionPosition;
   Point3F     mExplosionNormal;


  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);
   void advanceTime( F32 dt );
   void registerLights(LightManager * lightManager, bool lightingScene );

  protected:
   bool getTargetPosition(Point3F*);
   Point3F getVelocity() const;
   void explode(const Point3F&, const Point3F&);
   bool updatePos(const Point3F&, const Point3F&, const bool,
                  F32*, Point3F*, Point3F*, SceneObject*&);
   bool getNewVelocity(const Point3F& currPosition,
                       const Point3F& currVelocity,
                       const Point3F& targetPosition,
                       Point3F*       newVelocity);
   void setupWarp();


   bool calculateImpact(float    simTime,
                        Point3F& pointOfImpact,
                        float&   impactTime);
   
   void updateFlechette(const Point3F&);
   void renderObject(SceneState*, SceneRenderImage*);

  public:
   SeekerProjectile();
   ~SeekerProjectile();

   void processTick(const Move*);
   void interpolateTick(F32);

   DECLARE_CONOBJECT(SeekerProjectile);
   static void consoleInit();

   S32 getTargetObjectId();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_SEEKERPROJECTILE

