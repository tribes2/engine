//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _WHEELEDVEHICLE_H_
#define _WHEELEDVEHICLE_H_

#ifndef _VEHICLE_H_
#include "game/vehicles/vehicle.h"
#endif

#ifndef _CLIPPEDPOLYLIST_H_
#include "collision/clippedPolyList.h"
#endif

class ParticleEmitter;
class ParticleEmitterData;


//----------------------------------------------------------------------------

struct WheeledVehicleData: public VehicleData {
   typedef VehicleData Parent;

   enum Constants {
      MaxWheels = 8,
      MaxWheelBits = 3
   };

   enum Sounds {
      JetSound,
      EngineSound,
      SqueelSound,
      WheelImpactSound,
      MaxSounds,
   };
   AudioProfile* sound[MaxSounds];

   struct Tire {
      F32 friction;
      F32 restitution;
      F32 radius;
      F32 lateralForce;
      F32 lateralDamping;
      F32 lateralRelaxation;
      F32 longitudinalForce;
      F32 longitudinalDamping;
      F32 longitudinalRelaxation;
      ParticleEmitterData* emitter;
   } tire;

   F32 staticLoadScale;
   F32 springForce;
   F32 springDamping;
   F32 antiSwayForce;
   F32 antiRockForce;
   F32 maxWheelSpeed;
   F32 engineTorque;
   F32 breakTorque;

   F32 stabilizerForce;
   F32 gyroForce;
   F32 gyroDamping;

   // Initialized onAdd
   struct Wheel {
      enum Steering {
         None,
         Forward,
         Backward,
      } steering;
      S32 opposite;
      F32 springForce;     // Spring Constant in 1G
      F32 springDamping;      // Shock absorber
      F32 springRest;         // Resting point at 1G
      Point3F safePos;
      Point3F pos;
      Point3F spring;
      S32 springNode;
      S32 springSequence;
      S32 rotationSequence;
      S32 steeringSequence;
   } wheel[MaxWheels];
   U32 wheelCount;
   ClippedPolyList rigidBody; // Planes extracted from shape
   S32 tailLightSequence;

   //
   WheeledVehicleData();
   DECLARE_CONOBJECT(WheeledVehicleData);
   static void initPersistFields();
   bool preload(bool, char errorBuffer[256]);
   bool mirrorWheel(Wheel* we);
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};


//----------------------------------------------------------------------------

class WheeledVehicle: public Vehicle
{
   typedef Vehicle Parent;

//   struct StateDelta: Vehicle::StateDelta {
//      struct Wheel {
//         F32 pos;
//         F32 posVec;
//      } wheel[WheeledVehicleData::MaxWheels];
//   };
//   StateDelta delta;

   WheeledVehicleData* mDataBlock;

   bool mBraking;
   bool mWheelContact;
   TSThread* mTailLightThread;

   AUDIOHANDLE mJetSound;
   AUDIOHANDLE mEngineSound;
   AUDIOHANDLE mSqueelSound;

   struct Wheel {
      F32 k;                  // Spring coefficient
      F32 s;                  // Shock absorber coefficient
      F32 center;             // Neutral point of spring
      F32 extension;          // Spring extension (0-1)
      F32 evel;
      F32 avel;
      F32 apos;
      F32 Dy,Dx;
      struct Surface {
         bool contact;
         Point3F normal;
         U32 material;
         Point3F pos;
         SceneObject* object;
      } surface;
      F32 traction;
      TSThread* springThread;
      TSThread* rotationThread;
      TSThread* steeringThread;

      F32 torqueScale;        // 0-1
      F32 particleSlip;       // 0-1 slip
      Point3F particleAxis;   // Last tire axis
      SimObjectPtr<ParticleEmitter> emitter;
   };
   Wheel mWheel[WheeledVehicleData::MaxWheels];
   
   //
   bool onNewDataBlock(GameBaseData* dptr);
   void processTick(const Move* move);
   void updateMove(const Move *move);
   void updateWarp();
   void updateWheels();
   void updateForces(F32 dt);
   void updateWheelThreads();
   void renderImage(SceneState* state, SceneRenderImage*);

   // Client sounds & particles
   void updateJet(F32 dt);
   void updateEngineSound(F32 level);
   void updateSqueelSound(F32 level);

   U32 getCollisionMask();
  public:
   DECLARE_CONOBJECT(WheeledVehicle);
   static void initPersistFields();

   WheeledVehicle();
   ~WheeledVehicle();

   bool onAdd();
   void onRemove();
   void advanceTime(F32 dt);
   bool buildPolyList(AbstractPolyList* polyList, const Box3F&, const SphereF&);

   bool writePacketData(GameConnection *, BitStream *stream);
   void readPacketData(GameConnection *, BitStream *stream);
   U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *, BitStream *stream);
};


#endif
