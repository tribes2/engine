//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_SHOCKLANCEPROJECTILE
#define _H_SHOCKLANCEPROJECTILE

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif

class ShockLanceProjectile;
class ShockLanceProjectileData;
class ShockwaveData;

// -------------------------------------------------------------------------
class ShockLanceElectricity : public SceneObject
{
   typedef SceneObject Parent;
   friend class ShockLanceProjectile;

   ShockLanceProjectileData * mDataBlock;

   Vector < Point2F * > mTexCoordList;

  protected:
   SimObjectPtr<ShapeBase> mTarget;
   TSMaterialList*         mCopiedList;

   U32 mStartTime;
   F32 mElapsedTime;
   U32 mShapeDetail;

  protected:
   bool onAdd();
   void onRemove();
   void calcTexPlanes( F32 *S, F32 *T );

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);
   void renderElectricity();


  public:
   ShockLanceElectricity();
   ~ShockLanceElectricity();

   void processTick( const Move* );
   void advanceTime( F32 dt );
   void setDataBlock( ShockLanceProjectileData *block ){ mDataBlock = block; }
};


//--------------------------------------------------------------------------
class ShockLanceProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  public:
   enum Constants
   {
      NUM_ELECTRIC_TEX = 3,
      NUM_TEX = 4,
      PROJECTILE_TEX = 3,
      NUM_BOLTS = 2,
      NUM_EMITTERS = 1,
   };

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   F32              zapDuration;
   F32              boltLength;
   F32              startWidth[NUM_BOLTS];
   F32              endWidth[NUM_BOLTS];
   F32              boltSpeed[NUM_BOLTS];
   F32              texWrap[NUM_BOLTS];
   U32              numParts;
   F32              lightningFreq;
   F32              lightningDensity;
   F32              lightningAmp;
   F32              lightningWidth;

   StringTableEntry        textureName[NUM_TEX];
   ParticleEmitterData*    emitterList[NUM_EMITTERS];
   S32                     emitterIDList[NUM_EMITTERS];
   ShockwaveData *         shockwave;
   S32                     shockwaveID;

   //-------------------------------------- load set variables
  public:
   TextureHandle    textureHandle[NUM_TEX];

  public:
   ShockLanceProjectileData();
   ~ShockLanceProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

	bool calculateAim(const Point3F& targetPos,
                      const Point3F& /*targetVel*/,
                      const Point3F& sourcePos,
                      const Point3F& /*sourceVel*/,
                      Point3F*       outputVectorMin,
                      F32*           outputMinTime,
                      Point3F*       outputVectorMax,
                      F32*           outputMaxTime);

   DECLARE_CONOBJECT(ShockLanceProjectileData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class ShockLanceProjectile : public Projectile
{
  typedef Projectile Parent;

  public:
   enum Constants
   {
      NUM_LIGHTNING_BOLTS = 2,
      NUM_POINTS = 50,
   };

  private:
   ShockLanceProjectileData* mDataBlock;
   ParticleEmitter * mEmitterList[ ShockLanceProjectileData::NUM_EMITTERS ];

   struct LightningBolt
   {
      Point3F  points[NUM_POINTS];
      U32      numPoints;

      LightningBolt()
      {
         dMemset( this, 0, sizeof( LightningBolt ) );
         numPoints = 0;
      }

   };

   LightningBolt  mBoltList[NUM_LIGHTNING_BOLTS];
   F32            mTimeSinceLastLightningBolt;
   bool           mHitObject;

  protected:
   S32 mDeleteWaitTicks;

   S32                                 mTargetId;
   SimObjectPtr<ShapeBase>             mTargetPtr;
   SimObjectPtr<ShockLanceElectricity> mElectricity;

   Point3F mStart;
   Point3F mEnd;

   F32 mBeamWidthVel[ ShockLanceProjectileData::NUM_BOLTS ];
   F32 mElapsedTime;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);
   void updateLightningPos();

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderBeam( const Point3F &camPos, U32 boltNum );
   void renderObject(SceneState*, SceneRenderImage*);

   void createBolt( LightningBolt &bolt, U32 density, F32 amplitude );
   void renderBolt( LightningBolt &bolt, const Point3F &camPos, F32 width );
   void renderLightning( const Point3F &camPos );


   // Time/Move Management
  public:
   void processTick(const Move*);
   void advanceTime(F32);

  public:
   ShockLanceProjectile();
   ~ShockLanceProjectile();

   DECLARE_CONOBJECT(ShockLanceProjectile);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);

};

#endif // _H_SHOCKLANCEPROJECTILE

