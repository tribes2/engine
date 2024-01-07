//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#ifndef _SHAPEBASE_H_
#include "game/shapeBase.h"
#endif
#ifndef _RIGID_H_
#include "game/rigid.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif

class ParticleEmitter;
class ParticleEmitterData;
class ClippedPolyList;


//----------------------------------------------------------------------------

struct VehicleData: public ShapeBaseData
{
   typedef ShapeBaseData Parent;

   struct Body {
      enum Sounds {
         SoftImpactSound,
         HardImpactSound,
         MaxSounds,
      };
      AudioProfile* sound[MaxSounds];
      F32 restitution;
      F32 friction;
   } body;

   enum VehicleConsts
   {
      VC_NUM_DUST_EMITTERS = 1,
      VC_NUM_DAMAGE_EMITTER_AREAS = 2,
      VC_NUM_DAMAGE_LEVELS = 2,
      VC_NUM_BUBBLE_EMITTERS = 1,
      VC_NUM_DAMAGE_EMITTERS = VC_NUM_DAMAGE_LEVELS + VC_NUM_BUBBLE_EMITTERS,
      VC_NUM_SPLASH_EMITTERS = 2,
      VC_BUBBLE_EMITTER = VC_NUM_DAMAGE_EMITTERS - VC_NUM_BUBBLE_EMITTERS,
   };

  enum Sounds {
      ExitWater,
      ImpactSoft,
      ImpactMedium,
      ImpactHard,
      Wake,
      MaxSounds
   };
   AudioProfile* waterSound[MaxSounds];
   F32 exitSplashSoundVel;
   F32 softSplashSoundVel;
   F32 medSplashSoundVel;
   F32 hardSplashSoundVel;
 
   F32 minImpactSpeed;
   F32 softImpactSpeed;
   F32 hardImpactSpeed;
   F32 minRollSpeed;
   F32 maxSteeringAngle;

   F32 collDamageThresholdVel;
   F32 collDamageMultiplier;
   
   F32 cameraLag;
   F32 cameraOffset;                // Vertical offset

   F32 minDrag;
   F32 maxDrag;

   F32 jetForce;
   F32 jetEnergyDrain;        // Energy drain/tick
   F32 minJetEnergy;

   S32 stuckTimerTicks;
   F32 stuckTimerAngle;
   F32 stuckTimerZ;           // calculated in preload...
   
   ParticleEmitterData * dustEmitter;
   S32                   dustID;
   F32                   triggerDustHeight;  // height vehicle has to be under to kick up dust
   F32                   dustHeight;         // dust height above ground

   ParticleEmitterData *   damageEmitterList[ VC_NUM_DAMAGE_EMITTERS ];
   S32                     damageEmitterIDList[ VC_NUM_DAMAGE_EMITTERS ];
   Point3F                 damageEmitterOffset[ VC_NUM_DAMAGE_EMITTER_AREAS ];
   F32                     damageLevelTolerance[ VC_NUM_DAMAGE_LEVELS ];
   F32                     numDmgEmitterAreas;

   ParticleEmitterData*    splashEmitterList[VC_NUM_SPLASH_EMITTERS];
   S32                     splashEmitterIDList[VC_NUM_SPLASH_EMITTERS];
   F32                     splashFreqMod;
   F32                     splashVelEpsilon;


   // Initialized in load()
   Point3F massCenter;

   //
   VehicleData();
   bool preload(bool server, char errorBuffer[256]);
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   DECLARE_CONOBJECT(VehicleData);
};


//----------------------------------------------------------------------------

class Vehicle: public ShapeBase
{
   typedef ShapeBase Parent;

  protected:
   enum CollisionFaceFlags {
      BodyCollision =  0x1,
      WheelCollision = 0x2,
   };
   enum MaskBits {
      PositionMask = Parent::NextFreeMask << 0,
      FrozenMask   = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2,
      EnergyMask   = Parent::NextFreeMask << 3
   };

   struct StateDelta {
      Move move;                    // Last move from server
      F32 dt;                       // Last interpolation time
      // Interpolation data
      Point3F pos;
      Point3F posVec;
      QuatF rot[2];
      // Warp data
      S32 warpTicks;                // Number of ticks to warp
      S32 warpCount;                // Current pos in warp
      Point3F warpOffset;
      QuatF warpRot[2];
      //
      Point3F cameraOffset;
      Point3F cameraVec;
      Point3F cameraRot;
      Point3F cameraRotVec;
   };

   StateDelta mDelta;
   S32 mPredictionCount;            // Number of ticks to predict
   VehicleData* mDataBlock;
   bool inLiquid;
   AUDIOHANDLE waterWakeHandle;
   
   bool mFrozen;

   // Control
   Point2F mSteering;
   F32 mThrottle;
   bool mJetting;

   // Rigid Body
   bool mMinRoll;
   bool mDisableMove;

   // Stuck-ness timer
   S32  mStuckTimer;
   
   ShapeBaseConvex mConvex;

   Rigid mRigid;

   ParticleEmitter *mDustEmitterList[VehicleData::VC_NUM_DUST_EMITTERS];
   ParticleEmitter *mDamageEmitterList[VehicleData::VC_NUM_DAMAGE_EMITTERS];
   ParticleEmitter *mSplashEmitterList[VehicleData::VC_NUM_SPLASH_EMITTERS];

   //
   bool onNewDataBlock(GameBaseData* dptr);
   void updatePos(F32 dt);
   bool advanceToCollision(F32 time);
   bool resolveCollision(Rigid::State& ns,CollisionList& cList);
   F32  resolveContacts(Rigid::State& ns,CollisionList& cList,F32 dt);
   void localImpulse(const Point3F &r,const Point3F &impulse);

   void damageQueuedObjects(const F32 speed);
   
   void setPosition(const Point3F& pos,const QuatF& rot);
   void setRenderPosition(const Point3F& pos,const QuatF& rot);
   void setTransform(const MatrixF& newMat);

//   virtual bool collideBody(const MatrixF& mat,Collision* info) = 0;
   virtual void updateMove(const Move*);
   virtual void updateForces(F32 dt);
   virtual void updateWarp();

   bool writePacketData(GameConnection *, BitStream *stream);
   void readPacketData(GameConnection *, BitStream *stream);
   U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *con, BitStream *stream);

   void updateLiftoffDust( F32 dt );
   void updateDamageSmoke( F32 dt );

   void updateWorkingCollisionSet(const U32 mask);
   virtual U32 getCollisionMask();

   void updateFroth( F32 dt );
   bool collidingWithWater( Point3F &waterHeight );

   void renderObject(SceneState*, SceneRenderImage*);

  public:
   // Test code...
   static ClippedPolyList* sPolyList;
   static S32 sVehicleCount;

   //
   Vehicle();
   static void initPersistFields();
   void processTick(const Move*);
   bool onAdd();
   void onRemove();
   void interpolateTick(F32 dt);
   void advanceTime(F32 dt);

   void disableCollision();
   void enableCollision();

   Point3F getVelocity() const;

   void setEnergyLevel(F32 energy);
   //F32 getHeat() const;

   void setFrozenState(const bool _frozen);
   
   // Rigid body methods
   void getVelocity(const Point3F& r, Point3F* vel);
   void applyImpulse(const Point3F &r,const Point3F &impulse);
   F32  getImpulse(const Point3F& r,const Point3F& normal);

   void getCameraParameters(F32 *min,F32* max,Point3F* off,MatrixF* rot);
   void mountObject(ShapeBase* obj, U32 node);

   DECLARE_CONOBJECT(Vehicle);
   static void consoleInit();
};


#endif
