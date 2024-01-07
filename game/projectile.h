//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _LIGHTMANAGER_H_
#include "scenegraph/lightManager.h"
#endif
#ifndef _PLATFORMAUDIO_H_
#include "platform/platformAudio.h"
#endif

class ParticleEmitterData;
class ParticleEmitter;
class ExplosionData;
class ShapeBase;
class TSShapeInstance;
class TSThread;
class DecalData;
class SplashData;

//--------------------------------------------------------------------------
class ProjectileData : public GameBaseData
{
   typedef GameBaseData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   // Shape related
   const char* projectileShapeName;
   S32         emitterDelay;

   DecalData* decalData[6];
   S32        decalID[6];
   U32        numDecals;

   // Physics related
   F32  velInheritFactor;
	bool isBallistic;

   // Damage related
   F32   directDamage;
   bool  hasDamageRadius;
   F32   indirectDamage;
   F32   damageRadius;
   S32   radiusDamageType;

   F32   kickBackStrength;

   bool   hasLight;
   F32    lightRadius;
   ColorF lightColor;

   bool   hasLightUnderwaterColor;
   ColorF underWaterLightColor;
   
   bool  explodeOnWaterImpact;
   F32   bubbleEmitTime;

   bool     faceViewer;
   Point3F  scale;

   F32 depthTolerance;

   // Particle emission
   ParticleEmitterData* baseEmitter;
   ParticleEmitterData* delayEmitter;
   ParticleEmitterData* bubbleEmitter;
   ExplosionData*       explosion;
   ExplosionData*       underwaterExplosion;
   SplashData*          splash;
   AudioProfile*        sound;
   AudioProfile*        wetFireSound;
   AudioProfile*        fireSound;

   S32                  baseEmitterId;
   S32                  delayEmitterId;
   S32                  bubbleEmitterId;
   S32                  explosionId;
   S32                  underwaterExplosionId;
   S32                  splashId;
   S32                  soundId;
   S32                  wetFireSoundId;
   S32                  fireSoundId;

   //-------------------------------------- load() set variables
  public:
   Resource<TSShape> projectileShape;
   S32               ambientSeq;

  public:
   ProjectileData();
   ~ProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   virtual bool calculateAim(const Point3F& targetPos,
                             const Point3F& targetVel,
                             const Point3F& sourcePos,
                             const Point3F& sourceVel,
                             Point3F*       outputVectorMin,
                             F32*           outputMinTime,
                             Point3F*       outputVectorMax,
                             F32*           outputMaxTime);

   static void initPersistFields();
   DECLARE_CONOBJECT(ProjectileData);
};


//--------------------------------------------------------------------------
class Projectile : public GameBase
{
   typedef GameBase Parent;

  protected:
   enum ProjectileConstants {
      SourceIdTimeoutTicks = 7,  // = 231 ms
      DeleteWaitTicks      = 13, // = 416 ms
      ExcessVelDirBits     = 7
   };

  private:
   ProjectileData* mDataBlock;

   // Initial conditions
  protected:
   Point3F  mInitialPosition;
   Point3F  mInitialDirection;
   S32      mSourceObjectId;
   S32      mVehicleObjectId;
   S32      mSourceObjectSlot;

   // Time related variables common to all projectiles, managed by processTick
  protected:
   U32                     mCurrTick;     // Current time in ticks

   U32                     mSourceIdTimeoutTicks;
   SimObjectPtr<ShapeBase> mSourceObject; // Actual pointer to the source object, times out
                                          //  after SourceIdTimeoutTicks
   SimObjectPtr<ShapeBase> mVehicleObject; // Actual pointer to player mounted to vehicle

   // Rendering related variables
  protected:
   TSShapeInstance* mProjectileShape;
   TSThread*        mAmbientThread;

   ParticleEmitter* mBaseEmitter;
   ParticleEmitter* mDelayEmitter;
   ParticleEmitter* mBubbleEmitter;

   AUDIOHANDLE      mProjectileSound;

   bool mUseUnderwaterLight;
   
   void registerLights(LightManager * lightManager, bool lightingScene);
   LightInfo mLight;

   void emitParticles(const Point3F&, const Point3F&, const Point3F&, const U32);
   void updateSound(const Point3F& pos, const Point3F& vel, const bool active);

   bool             mHidden;     // set by the derived class, if true, projectile doesn't render
   F32              mFadeValue;  //  "                   "  , controls alpha fade out
   bool             mPlayedSplash;
   F32              mBubbleEmitterTime; // time since projectile entered water
   Point3F          mLastPos;

   // A note about these variables.  The server has the fully precise version
   //  of mExcessDir, the client has a "dumbed down" version, compressed for
   //  network space reasons.  The server must dumb down its vector before using
   //  it for calculation...
   Point3F mExcessDir;     // Normalized vel, transmitted as 7 bit compressed
   Point3F mExcessDirDumb; // Dumb version of the above
   U32     mExcessVel;     // Excess velocity inherited from firing object, ranges
                           //  from 0 to 255, in 1 m/s increments

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

   void processTick(const Move*);
   void advanceTime(F32);

   virtual void onCollision(const Point3F& p, const Point3F& n, SceneObject*);

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void prepModelView(SceneState*);
   void renderObject(SceneState*, SceneRenderImage*);
   bool pointInWater( Point3F &point, F32 *waterHeight = NULL );
   void createSplash( Point3F &pos );

  public:
   F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);

   Projectile();
   ~Projectile();

   DECLARE_CONOBJECT(Projectile);
   static void initPersistFields();
   static void consoleInit();

   virtual bool calculateImpact(float    simTime,
                                Point3F& pointOfImpact,
                                float&   impactTime);

  public:
   static U32 smProjectileWarpTicks;

  protected:
   static const U32 csmStaticCollisionMask;
   static const U32 csmDynamicCollisionMask;
   static const U32 csmDamageableMask;
};

#endif // _H_PROJECTILE

