//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/vehicles/vehicle.h"

#include "platform/platform.h"
#include "dgl/dgl.h"
#include "game/game.h"
#include "math/mMath.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "collision/clippedPolyList.h"
#include "collision/planeExtractor.h"
#include "game/moveManager.h"
#include "core/bitStream.h"
#include "core/dnet.h"
#include "game/gameConnection.h"
#include "ts/tsShapeInstance.h"
#include "game/particleEngine.h"
#include "audio/audio.h"
#include "math/mathIO.h"
#include "sceneGraph/sceneState.h"
#include "terrain/terrData.h"
#include "dgl/materialPropertyMap.h"
#include "game/player.h"

//----------------------------------------------------------------------------

namespace {

const U32 sMoveRetryCount = 3;

// Client prediction
const S32 sMaxWarpTicks = 3;          // Max warp duration in ticks
const S32 sMaxPredictionTicks = 30;   // Number of ticks to predict
const F32 sVehicleGravity = -20;

const F32 sCollisionTol = 0.07;       // Distance to maintain
const F32 sIntersectionTol = 0.01;   // Min collision distance
const F32 sContactTol = 0.5;          // Collision contact velocity
const F32 sF = 400;                   // Spring Force
const F32 sD = 2;                     // Spring Damping

} // namespace {}

S32 Vehicle::sVehicleCount = 0;
ClippedPolyList* Vehicle::sPolyList;
static U32 sDirtySetMask = (ForceFieldObjectType | 
                            PlayerObjectType     | VehicleObjectType);

IMPLEMENT_CONOBJECT(VehicleData);

ConsoleMethod(Vehicle, setFrozenState, void, 3, 3, "[obj].setFrozenState(t|f)")
{
   argc;
   AssertFatal(dynamic_cast<Vehicle*>(object) != NULL, "Error, must be a vehicle here!");
   Vehicle* pVeh = static_cast<Vehicle*>(object);
   pVeh->setFrozenState(dAtob(argv[2]));
}


//----------------------------------------------------------------------------

VehicleData::VehicleData()
{
   body.friction = 0;
   body.restitution = 1;
   
   minImpactSpeed = 25;
   softImpactSpeed = 25;
   hardImpactSpeed = 50;
   minRollSpeed = 0;
   maxSteeringAngle = 0.785; // 45 deg.

   cameraOffset = 0;
   cameraLag = 0;

   minDrag = 0;
   maxDrag = 0;

   jetForce = 500;
   jetEnergyDrain =  0.8;
   minJetEnergy = 1;

   massCenter.set(0,0,0);
   drag = 0.7;
   density = 4;

   for (S32 i = 0; i < Body::MaxSounds; i++)
      body.sound[i] = 0;

   dustEmitter = NULL;
   dustID = 0;
   triggerDustHeight = 3.0;
   dustHeight = 1.0;

   dMemset( damageEmitterList, 0, sizeof( damageEmitterList ) );
   dMemset( damageEmitterIDList, 0, sizeof( damageEmitterIDList ) );
   dMemset( damageLevelTolerance, 0, sizeof( damageLevelTolerance ) );
   dMemset( splashEmitterList, 0, sizeof( splashEmitterList ) );
   dMemset( splashEmitterIDList, 0, sizeof( splashEmitterIDList ) );

   numDmgEmitterAreas = 0;

   splashFreqMod = 300.0;
   splashVelEpsilon = 0.50;
   exitSplashSoundVel = 2.0;
   softSplashSoundVel = 1.0;
   medSplashSoundVel = 2.0;
   hardSplashSoundVel = 3.0;
   
   genericShadowLevel = Vehicle_GenericShadowLevel;
   noShadowLevel = Vehicle_NoShadowLevel;

   dMemset(waterSound, 0, sizeof(waterSound));

   collDamageThresholdVel = 20;
   collDamageMultiplier   = 0.05;

   stuckTimerTicks = 1;
   stuckTimerAngle = 180;
}


//----------------------------------------------------------------------------

bool VehicleData::preload(bool server, char errorBuffer[256])
{
   if (!Parent::preload(server, errorBuffer))
      return false;

   // Resolve objects transmitted from server
   if (!server) {
      for (S32 i = 0; i < Body::MaxSounds; i++)
         if (body.sound[i])
            Sim::findObject(SimObjectId(body.sound[i]),body.sound[i]);
   }

   if (server)
   {
      if (stuckTimerTicks <= 0)
      {
         Con::warnf("VehicleData::preload: stuck timer ticks must be >= 1");
         stuckTimerTicks = 1;
      }
      if (stuckTimerAngle < 0.0 || stuckTimerAngle > 180.0)
      {
         Con::warnf("VehicleData::preload: stuck timer angle must be in range [0, 180]");
         stuckTimerAngle = stuckTimerAngle < 0.0 ? 0.0 : 180.0;
      }
      stuckTimerZ = mCos(stuckTimerAngle);
   }

   
//   // Center of mass in object space
//   S32 mass = shape->findNode("mass");
//   if (mass != -1) {
//      massCenter = shape->nodeStates[mass].transform.getTranslate();
//      massCenter.x = 0;
//   }
   massCenter.set(0, 0, 0);

   if( !dustEmitter && dustID != 0 )
   {
      if( !Sim::findObject( dustID, dustEmitter ) )
      {
         Con::errorf( ConsoleLogEntry::General, "VehicleData::preload Invalid packet, bad datablockId(dustEmitter): 0x%x", dustID );
      }
   }

   U32 i;
   for( i=0; i<VC_NUM_DAMAGE_EMITTERS; i++ )
   {
      if( !damageEmitterList[i] && damageEmitterIDList[i] != 0 )
      {
         if( !Sim::findObject( damageEmitterIDList[i], damageEmitterList[i] ) )
         {
            Con::errorf( ConsoleLogEntry::General, "VehicleData::preload Invalid packet, bad datablockId(damageEmitter): 0x%x", damageEmitterIDList[i] );
         }
      }
   }

   for( i=0; i<VC_NUM_SPLASH_EMITTERS; i++ )
   {
      if( !splashEmitterList[i] && splashEmitterIDList[i] != 0 )
      {
         if( !Sim::findObject( splashEmitterIDList[i], splashEmitterList[i] ) )
         {
            Con::errorf( ConsoleLogEntry::General, "VehicleData::preload Invalid packet, bad datablockId(splashEmitter): 0x%x", splashEmitterIDList[i] );
         }
      }
   }

   return true;
}   


//----------------------------------------------------------------------------

void VehicleData::packData(BitStream* stream)
{
   S32 i;
   Parent::packData(stream);

   stream->write(body.restitution);
   stream->write(body.friction);
   for (i = 0; i < Body::MaxSounds; i++)
      if (stream->writeFlag(body.sound[i]))
         stream->writeRangedU32(packed? SimObjectId(body.sound[i]):
                                body.sound[i]->getId(),DataBlockObjectIdFirst,
                                DataBlockObjectIdLast);
   
   stream->write(minImpactSpeed);
   stream->write(softImpactSpeed);
   stream->write(hardImpactSpeed);
   stream->write(minRollSpeed);
   stream->write(maxSteeringAngle);

   stream->write(maxDrag);
   stream->write(minDrag);

   stream->write(jetForce);
   stream->write(jetEnergyDrain);
   stream->write(minJetEnergy);

   stream->write(cameraOffset);
   stream->write(cameraLag);

   stream->write( triggerDustHeight );
   stream->write( dustHeight );

   stream->write( numDmgEmitterAreas );

   stream->write(exitSplashSoundVel);
   stream->write(softSplashSoundVel);
   stream->write(medSplashSoundVel);
   stream->write(hardSplashSoundVel);

   // write the water sound profiles
   for(i = 0; i < MaxSounds; i++)
      if(stream->writeFlag(waterSound[i]))
         stream->writeRangedU32(waterSound[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast);

   if (stream->writeFlag( dustEmitter ))
   {
      stream->writeRangedU32( dustEmitter->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   for (i = 0; i < VC_NUM_DAMAGE_EMITTERS; i++)
   {
      if( stream->writeFlag( damageEmitterList[i] != NULL ) )
      {
         stream->writeRangedU32( damageEmitterList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }

   for (i = 0; i < VC_NUM_SPLASH_EMITTERS; i++)
   {
      if( stream->writeFlag( splashEmitterList[i] != NULL ) )
      {
         stream->writeRangedU32( splashEmitterList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }

   for (int j = 0;  j < VC_NUM_DAMAGE_EMITTER_AREAS; j++)
   {
      stream->write( damageEmitterOffset[j].x );
      stream->write( damageEmitterOffset[j].y );
      stream->write( damageEmitterOffset[j].z );
   }

   for (int k = 0; k < VC_NUM_DAMAGE_LEVELS; k++)
   {
      stream->write( damageLevelTolerance[k] );
   }

   stream->write(splashFreqMod);
   stream->write(splashVelEpsilon);

   stream->write(collDamageThresholdVel);
   stream->write(collDamageMultiplier);
}   

void VehicleData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&body.restitution);
   stream->read(&body.friction);
   S32 i;
   for (i = 0; i < Body::MaxSounds; i++) {
      body.sound[i] = NULL;
      if (stream->readFlag())
         body.sound[i] = (AudioProfile*)stream->readRangedU32(DataBlockObjectIdFirst,
                                                              DataBlockObjectIdLast);
   }
   
   stream->read(&minImpactSpeed);
   stream->read(&softImpactSpeed);
   stream->read(&hardImpactSpeed);
   stream->read(&minRollSpeed);
   stream->read(&maxSteeringAngle);

   stream->read(&maxDrag);
   stream->read(&minDrag);

   stream->read(&jetForce);
   stream->read(&jetEnergyDrain);
   stream->read(&minJetEnergy);

   stream->read(&cameraOffset);
   stream->read(&cameraLag);

   stream->read( &triggerDustHeight );
   stream->read( &dustHeight );

   stream->read( &numDmgEmitterAreas );

   stream->read(&exitSplashSoundVel);
   stream->read(&softSplashSoundVel);
   stream->read(&medSplashSoundVel);
   stream->read(&hardSplashSoundVel);

   // write the water sound profiles
   for(i = 0; i < MaxSounds; i++)
      if(stream->readFlag())
      {
         U32 id = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
         waterSound[i] = dynamic_cast<AudioProfile*>( Sim::findObject(id) );
      }

   if( stream->readFlag() )
   {
      dustID = (S32) stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   }

   for (i = 0; i < VC_NUM_DAMAGE_EMITTERS; i++)
   {
      if( stream->readFlag() )
      {
         damageEmitterIDList[i] = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
      }
   }

   for (i = 0; i < VC_NUM_SPLASH_EMITTERS; i++)
   {
      if( stream->readFlag() )
      {
         splashEmitterIDList[i] = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
      }
   }

   for( int j=0; j<VC_NUM_DAMAGE_EMITTER_AREAS; j++ )
   {
      stream->read( &damageEmitterOffset[j].x );
      stream->read( &damageEmitterOffset[j].y );
      stream->read( &damageEmitterOffset[j].z );
   }

   for( int k=0; k<VC_NUM_DAMAGE_LEVELS; k++ )
   {
      stream->read( &damageLevelTolerance[k] );
   }

   stream->read(&splashFreqMod);
   stream->read(&splashVelEpsilon);

   stream->read(&collDamageThresholdVel);
   stream->read(&collDamageMultiplier);
}   


//----------------------------------------------------------------------------

void VehicleData::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("jetForce", TypeF32, Offset(jetForce, VehicleData));
   addField("jetEnergyDrain", TypeF32, Offset(jetEnergyDrain, VehicleData));
   addField("minJetEnergy", TypeF32, Offset(minJetEnergy, VehicleData));

   addField("bodyRestitution", TypeF32, Offset(body.restitution, VehicleData));
   addField("bodyFriction", TypeF32, Offset(body.friction, VehicleData));
   addField("softImpactSound", TypeAudioProfilePtr, Offset(body.sound[Body::SoftImpactSound], VehicleData));
   addField("hardImpactSound", TypeAudioProfilePtr, Offset(body.sound[Body::HardImpactSound], VehicleData));

   addField("minImpactSpeed", TypeF32, Offset(minImpactSpeed, VehicleData));
   addField("softImpactSpeed", TypeF32, Offset(softImpactSpeed, VehicleData));
   addField("hardImpactSpeed", TypeF32, Offset(hardImpactSpeed, VehicleData));
   addField("minRollSpeed", TypeF32, Offset(minRollSpeed, VehicleData));
   addField("maxSteerinAngle", TypeF32, Offset(maxSteeringAngle, VehicleData));

   addField("maxDrag", TypeF32, Offset(maxDrag, VehicleData));
   addField("minDrag", TypeF32, Offset(minDrag, VehicleData));

   addField("cameraOffset",   TypeF32,        Offset(cameraOffset,   VehicleData));
   addField("cameraLag",      TypeF32,        Offset(cameraLag,      VehicleData));

   addField("dustEmitter",       TypeParticleEmitterDataPtr,   Offset(dustEmitter,        VehicleData));
   addField("triggerDustHeight", TypeF32,                      Offset(triggerDustHeight,  VehicleData));
   addField("dustHeight",        TypeF32,                      Offset(dustHeight,         VehicleData));

   addField("damageEmitter",        TypeParticleEmitterDataPtr,   Offset(damageEmitterList,     VehicleData), VC_NUM_DAMAGE_EMITTERS);
   addField("splashEmitter",        TypeParticleEmitterDataPtr,   Offset(splashEmitterList,     VehicleData), VC_NUM_SPLASH_EMITTERS);
   addField("damageEmitterOffset",  TypePoint3F,                  Offset(damageEmitterOffset,   VehicleData), VC_NUM_DAMAGE_EMITTER_AREAS);
   addField("damageLevelTolerance", TypeF32,                      Offset(damageLevelTolerance,  VehicleData), VC_NUM_DAMAGE_LEVELS);
   addField("numDmgEmitterAreas",   TypeF32,                      Offset(numDmgEmitterAreas,    VehicleData));

   addField("splashFreqMod",  TypeF32,                Offset(splashFreqMod,   VehicleData));
   addField("splashVelEpsilon", TypeF32,              Offset(splashVelEpsilon, VehicleData));

   addField("exitSplashSoundVelocity", TypeF32,       Offset(exitSplashSoundVel, VehicleData));
   addField("softSplashSoundVelocity", TypeF32,       Offset(softSplashSoundVel, VehicleData));
   addField("mediumSplashSoundVelocity", TypeF32,     Offset(medSplashSoundVel, VehicleData));
   addField("hardSplashSoundVelocity", TypeF32,       Offset(hardSplashSoundVel, VehicleData));
   addField("exitingWater",      TypeAudioProfilePtr, Offset(waterSound[ExitWater],   VehicleData));
   addField("impactWaterEasy",   TypeAudioProfilePtr, Offset(waterSound[ImpactSoft],   VehicleData));
   addField("impactWaterMedium", TypeAudioProfilePtr, Offset(waterSound[ImpactMedium],   VehicleData));
   addField("impactWaterHard",   TypeAudioProfilePtr, Offset(waterSound[ImpactHard],   VehicleData));
   addField("waterWakeSound",    TypeAudioProfilePtr, Offset(waterSound[Wake],   VehicleData));

   addField("collDamageThresholdVel", TypeF32, Offset(collDamageThresholdVel, VehicleData));
   addField("collDamageMultiplier",   TypeF32, Offset(collDamageMultiplier,   VehicleData));

   addField("stuckTimerTicks", TypeS32, Offset(stuckTimerTicks, VehicleData));
   addField("stuckTimerAngle", TypeF32, Offset(stuckTimerAngle, VehicleData));
}   


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(Vehicle);

Vehicle::Vehicle()
{
   mTypeMask |= VehicleObjectType;

   mDelta.pos = Point3F(0,0,0);
   mDelta.posVec = Point3F(0,0,0);
   mDelta.warpTicks = mDelta.warpCount = 0;
   mDelta.dt = 1;
   mDelta.move = NullMove;
   mPredictionCount = 0;
   mDelta.cameraOffset.set(0,0,0);
   mDelta.cameraVec.set(0,0,0);
   mDelta.cameraRot.set(0,0,0);
   mDelta.cameraRotVec.set(0,0,0);

   mRigid.state.linPosition.set(0, 0, 0);
   mRigid.state.linVelocity.set(0, 0, 0);
   mRigid.state.angPosition.identity();
   mRigid.state.angVelocity.set(0, 0, 0);
   mRigid.state.linMomentum.set(0, 0, 0);
   mRigid.state.angMomentum.set(0, 0, 0);

   mMinRoll = false;

   mSteering.set(0,0);
   mThrottle = 0;
   mJetting = false;

   dMemset( mDustEmitterList, 0, sizeof( mDustEmitterList ) );
   dMemset( mDamageEmitterList, 0, sizeof( mDamageEmitterList ) );
   dMemset( mSplashEmitterList, 0, sizeof( mSplashEmitterList ) );
   
   mDisableMove = false;

   inLiquid = false;
   mFrozen = false;
   waterWakeHandle = 0;

   mStuckTimer = 0;
}   


void Vehicle::consoleInit()
{
   
}

void Vehicle::updateWarp()
{
   AssertFatal(false, "Pure virtual (sorta) function called!");
}

U32 Vehicle::getCollisionMask()
{
   AssertFatal(false, "Pure virtual (sorta) function called!");
   return 0;
}

Point3F Vehicle::getVelocity() const
{
   return mRigid.state.linVelocity;
}

//----------------------------------------------------------------------------

bool Vehicle::onAdd()
{
   if (!sVehicleCount++)
      sPolyList = new ClippedPolyList;

   if (!Parent::onAdd())
      return false;

   mRigid.state.setTransform(mObjToWorld);
   mRigid.mass = 1;
   mRigid.oneOverMass = 1 / mRigid.mass;
   mRigid.setObjectInertia((mObjBox.max - mObjBox.min) * 0.5);

   mDelta.rot[1] = mDelta.rot[0] = mRigid.state.angPosition;
   mDelta.pos = mRigid.state.linPosition;
   mDelta.posVec = Point3F(0,0,0);

   if( !isServerObject() )
   {
      if( mDataBlock->dustEmitter )
      {
         for( int i=0; i<VehicleData::VC_NUM_DUST_EMITTERS; i++ )
         {
            mDustEmitterList[i] = new ParticleEmitter;
            mDustEmitterList[i]->onNewDataBlock( mDataBlock->dustEmitter );
            if( !mDustEmitterList[i]->registerObject() )
            {
               Con::warnf( ConsoleLogEntry::General, "Could not register dust emitter for class: %s", mDataBlock->getName() );
               delete mDustEmitterList[i];
               mDustEmitterList[i] = NULL;
            }
         }
      }
      
      U32 j;
      for( j=0; j<VehicleData::VC_NUM_DAMAGE_EMITTERS; j++ )
      {
         if( mDataBlock->damageEmitterList[j] )
         {
            mDamageEmitterList[j] = new ParticleEmitter;
            mDamageEmitterList[j]->onNewDataBlock( mDataBlock->damageEmitterList[j] );
            if( !mDamageEmitterList[j]->registerObject() )
            {
               Con::warnf( ConsoleLogEntry::General, "Could not register damage emitter for class: %s", mDataBlock->getName() );
               delete mDamageEmitterList[j];
               mDamageEmitterList[j] = NULL;
            }
            
         }
      }

      for( j=0; j<VehicleData::VC_NUM_SPLASH_EMITTERS; j++ )
      {
         if( mDataBlock->splashEmitterList[j] )
         {
            mSplashEmitterList[j] = new ParticleEmitter;
            mSplashEmitterList[j]->onNewDataBlock( mDataBlock->splashEmitterList[j] );
            if( !mSplashEmitterList[j]->registerObject() )
            {
               Con::warnf( ConsoleLogEntry::General, "Could not register splash emitter for class: %s", mDataBlock->getName() );
               delete mSplashEmitterList[j];
               mSplashEmitterList[j] = NULL;
            }
            
         }
      }
   }

   // Create a new convex.
   AssertFatal(mDataBlock->collisionDetails[0] != -1, "Error, a vehicle must have a collision-1 detail!");
   mConvex.mObject    = this;
   mConvex.pShapeBase = this;
   mConvex.hullId     = 0;
   mConvex.box        = mObjBox;
   mConvex.box.min.convolve(mObjScale);
   mConvex.box.max.convolve(mObjScale);

   return true;
}

void Vehicle::onRemove()
{
   if (!--sVehicleCount) {
      delete sPolyList;
      sPolyList = 0;
   }

   U32 i=0;
   for( i=0; i<VehicleData::VC_NUM_DUST_EMITTERS; i++ )
   {
      if( mDustEmitterList[i] )
      {
         mDustEmitterList[i]->deleteWhenEmpty();
         mDustEmitterList[i] = NULL;
      }
   }

   for( i=0; i<VehicleData::VC_NUM_DAMAGE_EMITTERS; i++ )
   {
      if( mDamageEmitterList[i] )
      {
         mDamageEmitterList[i]->deleteWhenEmpty();
         mDamageEmitterList[i] = NULL;
      }
   }

   for( i=0; i<VehicleData::VC_NUM_SPLASH_EMITTERS; i++ )
   {
      if( mSplashEmitterList[i] )
      {
         mSplashEmitterList[i]->deleteWhenEmpty();
         mSplashEmitterList[i] = NULL;
      }
   }

   Parent::onRemove();
}


//----------------------------------------------------------------------------

void Vehicle::processTick(const Move* move)
{
   Parent::processTick(move);

   // Warp to catch up to server
   if (mDelta.warpCount < mDelta.warpTicks) {
      mDelta.warpCount++;

      // Set new pos.
      mObjToWorld.getColumn(3,&mDelta.pos);
      mDelta.pos += mDelta.warpOffset;
      mDelta.rot[0] = mDelta.rot[1];
      mDelta.rot[1].interpolate(mDelta.warpRot[0],mDelta.warpRot[1],F32(mDelta.warpCount)/mDelta.warpTicks);
      setPosition(mDelta.pos,mDelta.rot[1]);
      updateWarp();

      // Pos backstepping
      mDelta.posVec.x = -mDelta.warpOffset.x;
      mDelta.posVec.y = -mDelta.warpOffset.y;
      mDelta.posVec.z = -mDelta.warpOffset.z;
   }
   else {
      if (!move) {
         if (isGhost()) {
            // If we haven't run out of prediction time,
            // predict using the last known move.
            if (mPredictionCount-- <= 0)
               return;
            move = &mDelta.move;
         }
         else
            move = &NullMove;
      }

      if (mFrozen == false)
      {
         updateWorkingCollisionSet(getCollisionMask());
         updateMove(move);

         mDelta.posVec = mRigid.state.linPosition;
         mDelta.rot[0] = mRigid.state.angPosition;

         for (U32 i = 0; i < 1; i++) {
            mRigid.clearForces();
            updateForces(TickSec);
            updatePos(TickSec);
         }

         mDelta.pos     = mRigid.state.linPosition;
         mDelta.posVec -= mRigid.state.linPosition;
         mDelta.rot[1]  = mRigid.state.angPosition;

         setPosition(mRigid.state.linPosition, mRigid.state.angPosition);
         setMaskBits(PositionMask);
         updateContainer();
      }
      else
      {
         mDelta.posVec = mRigid.state.linPosition;
         mDelta.rot[0] = mRigid.state.angPosition;
         mDelta.pos     = mRigid.state.linPosition;
         mDelta.posVec -= mRigid.state.linPosition;
         mDelta.rot[1]  = mRigid.state.angPosition;
         setPosition(mRigid.state.linPosition, mRigid.state.angPosition);
      }
   }
}

void Vehicle::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);

   if (mFrozen == false)
   {
      if(dt == 0.0f)
         setRenderPosition(mDelta.pos, mDelta.rot[1]);
      else
      {
         QuatF rot;
         rot.interpolate(mDelta.rot[1], mDelta.rot[0], dt);
         Point3F pos = mDelta.pos + mDelta.posVec * dt;
         setRenderPosition(pos,rot);
      }
      mDelta.dt = dt;
   }
   else
   {
      mDelta.dt = 0;
   }
}

void Vehicle::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   updateLiftoffDust( dt );
   updateDamageSmoke( dt );

   updateFroth(dt);
}

//----------------------------------------------------------------------------

bool Vehicle::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<VehicleData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   return true;
}


//----------------------------------------------------------------------------

void Vehicle::getCameraParameters(F32 *min,F32* max,Point3F* off,MatrixF* rot)
{
   *min = mDataBlock->cameraMinDist;
   *max = mDataBlock->cameraMaxDist;

   off->set(0,0,mDataBlock->cameraOffset);
   rot->identity();
}


//----------------------------------------------------------------------------

void Vehicle::getVelocity(const Point3F& r, Point3F* v)
{
   mRigid.state.getVelocity(r, v);
}

void Vehicle::applyImpulse(const Point3F &pos, const Point3F &impulse)
{
   Point3F r = pos, massCenter;
   mObjToWorld.mulP(mDataBlock->massCenter,&massCenter);
   r -= massCenter;
   localImpulse(r, impulse);
}

void Vehicle::localImpulse(const Point3F &r,const Point3F &impulse)
{
   Point3F actualImpulse = impulse * mOneOverMass;
   mRigid.applyImpulse(mRigid.state, r, actualImpulse);
}

F32 Vehicle::getImpulse(const Point3F& r,const Point3F& normal)
{
   // Returns impulse value need to stop velocity along the
   // given normal.
   Point3F v;
   getVelocity(r,&v);
   F32 n = -mDot(v,normal);

   Point3F a,b;
   mCross(r,normal,&a);
   mCross(a,r,&b);
   F32 d = mRigid.oneOverMass + (mDot(b,normal) * mRigid.oneOverMass);

   return n/d;
}


//----------------------------------------------------------------------------
void Vehicle::updateWorkingCollisionSet(const U32 mask)
{
   // First, we need to adjust our velocity for possible acceleration.  It is assumed
   //  that we will never accelerate more than 20 m/s for gravity, plus 30 m/s for
   //  jetting, and an equivalent 10 m/s for vehicle accel.  We also assume that our
   //  working list is updated on a Tick basis, which means we only expand our box by
   //  the possible movement in that tick, plus some extra for caching purposes
   Point3F scaledVelocity = mRigid.state.linVelocity * TickSec;
   F32 len    = scaledVelocity.len();
   F32 newLen = len + (50 * TickSec);

   // Check to see if it is actually necessary to construct the new working list,
   //  or if we can use the cached version from the last query.  We use the x
   //  component of the min member of the mWorkingQueryBox, which is lame, but
   //  it works ok.
   bool updateSet = false;

   Box3F convexBox = mConvex.getBoundingBox(getTransform(), getScale());
   F32 l = (newLen * 1.1) + 0.1;  // fudge factor
   convexBox.min -= Point3F(l, l, l);
   convexBox.max += Point3F(l, l, l);
   
   disableCollision();
   mConvex.updateWorkingList(convexBox, mask);
   enableCollision();
}


//----------------------------------------------------------------------------

void Vehicle::disableCollision()
{
   Parent::disableCollision();
   for (ShapeBase* ptr = getMountList(); ptr; ptr = ptr->getMountLink())
      ptr->disableCollision();
}
 
void Vehicle::enableCollision()
{
   Parent::enableCollision();
   for (ShapeBase* ptr = getMountList(); ptr; ptr = ptr->getMountLink())
      ptr->enableCollision();
}   


//----------------------------------------------------------------------------

void Vehicle::updateMove(const Move* move)
{
   mDelta.move = *move;

   // Image Triggers
   if (mDamageState == Enabled) {
      setImageTriggerState(0,move->trigger[0]);
      setImageTriggerState(1,move->trigger[1]);
   }

   // Throttle
   if(!mDisableMove)
      mThrottle = move->y;
   
   // Steering
   if (move != &NullMove)
   {
      F32 y = move->yaw;
      mSteering.x = mClampF(mSteering.x + y,-mDataBlock->maxSteeringAngle,
                            mDataBlock->maxSteeringAngle);
      F32 p = move->pitch;
      mSteering.y = mClampF(mSteering.y + p,-mDataBlock->maxSteeringAngle,
                            mDataBlock->maxSteeringAngle);
   }
   else
   {
      mSteering.x = 0;
      mSteering.y = 0;      
   }
   // Jetting
   if (move->trigger[3])
   {
      if (!mJetting && getEnergyLevel() >= mDataBlock->minJetEnergy)
         mJetting = true;
      if (mJetting) {
         F32 newEnergy = getEnergyLevel() - mDataBlock->jetEnergyDrain;
         if (newEnergy < 0) {
            newEnergy = 0;
            mJetting = false;
         }
         setEnergyLevel(newEnergy);
      }
   }
   else
   {
      mJetting = false;
   }

   if (!isGhost()) {
      if(!inLiquid && mWaterCoverage != 0.0f) {
         Con::executef(mDataBlock,4,"onEnterLiquid",scriptThis(), Con::getFloatArg(mWaterCoverage), Con::getIntArg(mLiquidType));
         inLiquid = true;
         mHeat = 0.0;
      }
      else if(inLiquid && mWaterCoverage == 0.0f) {
         Con::executef(mDataBlock,3,"onLeaveLiquid",scriptThis(), Con::getIntArg(mLiquidType));
         inLiquid = false;
         mHeat = 1.0;
      }
   }
   else {
      F32 vSpeed = getVelocity().len();
      if(!inLiquid && mWaterCoverage >= 0.8f) {
         if(vSpeed >= mDataBlock->hardSplashSoundVel) 
            alxPlay(mDataBlock->waterSound[VehicleData::ImpactHard], &getTransform());
         else if( vSpeed >= mDataBlock->medSplashSoundVel)
            alxPlay(mDataBlock->waterSound[VehicleData::ImpactMedium], &getTransform());
         else if( vSpeed >= mDataBlock->softSplashSoundVel)
            alxPlay(mDataBlock->waterSound[VehicleData::ImpactSoft], &getTransform());
         inLiquid = true;
      }   
      else if(inLiquid && mWaterCoverage < 0.8f) {
         if(vSpeed >= mDataBlock->exitSplashSoundVel)
            alxPlay(mDataBlock->waterSound[VehicleData::ExitWater], &getTransform());
         inLiquid = false;
      }
   }   
   mMinRoll = mJetting;
}


void Vehicle::updateForces(F32 /*dt*/)
{
   // Nothing here.
}


//----------------------------------------------------------------------------
void Vehicle::setPosition(const Point3F& pos,const QuatF& rot)
{
   MatrixF mat;
   rot.setMatrix(&mat);
   mat.setColumn(3,pos);
   Parent::setTransform(mat);
}

void Vehicle::setRenderPosition(const Point3F& pos, const QuatF& rot)
{
   MatrixF mat;
   rot.setMatrix(&mat);
   mat.setColumn(3,pos);
   Parent::setRenderTransform(mat);
}

void Vehicle::setTransform(const MatrixF& newMat)
{
   mRigid.state.setTransform(newMat);
   Parent::setTransform(newMat);
}


void Vehicle::updatePos(F32 dt)
{
   advanceToCollision(dt);

//   // Check for rest condition
//   F32 k = mRigid.getKineticEnergy(mWorldToObj);
//   F32 G = -mRigid.state.force.z * mRigid.oneOverMass * TickSec;
//   F32 Kg = 0.5 * mRigid.mass * G * G;
//   if (k < Kg * sRestTol)
//      mRigid.setAtRest();
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool Vehicle::advanceToCollision(F32 time)
{
   F32 ct = 0,dt = time;
   Rigid::State ns = mRigid.state;

   MatrixF mat;
   mRigid.state.getTransform(&mat);
   CollisionState *state = mConvex.findClosestStateBounded(mat, getScale(), sCollisionTol);

   CollisionList info;
   F32 mt = time / 2.0;
   dt = mt;

   bool collided = false;
   bool displaced = false;
   Point3F origVelocity = mRigid.state.linVelocity;

   bool success = true;
   do {
      F32 prevDist = state != NULL ? state->dist : 1e7;
      info.count = 0;
      if (state && state->dist < sCollisionTol) {
         // Try to displace the object out of the way...
         SceneObject* obj = NULL;
         if (state->a->getObject() == this)
            obj = state->b->getObject();
         else
            obj = state->a->getObject();
         AssertFatal(obj != NULL, "Well, that's odd.");
         if (obj->isDisplacable() && ((obj->getTypeMask() & ShapeBaseObjectType) != 0))
         {
            // Try to displace the object by the amount we're trying to move
            Point3F objNewMom = ns.linVelocity * obj->getMass() * 1.1;
            Point3F objOldMom = obj->getMomentum();
            Point3F objNewVel = objNewMom / obj->getMass();

            Point3F myCenter;
            Point3F theirCenter;
            getWorldBox().getCenter(&myCenter);
            obj->getWorldBox().getCenter(&theirCenter);
            if (mDot(myCenter - theirCenter, objNewMom) >= 0.0f || objNewVel.len() < 0.01)
            {
               objNewMom = (theirCenter - myCenter);
               objNewMom.normalize();
               objNewMom *= 1.0f * obj->getMass();
               objNewVel = objNewMom / obj->getMass();
            }
            
            obj->setMomentum(objNewMom);
            if (obj->displaceObject(objNewVel * 1.1 * mt) == true)
            {
               // Determine the speed at which we will damage this object
               objOldMom /= obj->getMass();
               objNewMom /= obj->getMass();
               F32 len = (objOldMom - objNewMom).len();

               queueCollision(static_cast<ShapeBase*>(obj), len);
               state = 0;
               displaced = true;
               continue;
            }
         }

         mConvex.getCollisionInfo(mat, getScale(), &info, sCollisionTol * 1.25);
         collided |= resolveCollision(ns, info);
         resolveContacts(ns, info, dt);
         if (collided)
         {
            ns.force.set(0, 0, 0);
            ns.torque.set(0, 0, 0);
         }
      }

      mRigid.integrate(ns,dt);
      ns.getTransform(&mat);

      state = mConvex.findClosestStateBounded(mat, getScale(), sCollisionTol);
      if (state && state->dist <= sIntersectionTol && state->dist <= prevDist) {
         if ((dt *= 0.25) < 0.0001) {
            // Make sure we check the collision damage...
            collided = true;
            success = false;
            mRigid.state.linVelocity.set(0,0,0);
            mRigid.state.linMomentum.set(0,0,0);
            mRigid.state.angVelocity.set(0,0,0);
            mRigid.state.angMomentum.set(0,0,0);
            goto exitRoutine;
         }

         state = 0;
         ns = mRigid.state;
         continue;
      }
      
      mRigid.state = ns;
      ct += dt;
      if (dt < mt)
         dt *= 1.2;
      if (dt > (time - ct))
         dt = time - ct;
   } while (ct < time);

exitRoutine:
   if (collided || displaced)
   {
      F32 collVel = (origVelocity - mRigid.state.linVelocity).len();
      if (origVelocity.isZero() == false)
         origVelocity.normalize();
      else
         origVelocity = Point3F(0, 0, 1);

      if (isClientObject())
      {
         S32 impactSound = -1;
         if (collVel >= mDataBlock->hardImpactSpeed)
            impactSound = VehicleData::Body::HardImpactSound;
         else if (collVel >= mDataBlock->softImpactSpeed)
            impactSound = VehicleData::Body::SoftImpactSound;

         if (impactSound != -1 && mDataBlock->body.sound[impactSound] != NULL)
            alxPlay(mDataBlock->body.sound[impactSound], &getTransform());
      }

      if (isServerObject())
      {
         if (collVel > mDataBlock->minImpactSpeed)
            onImpact(origVelocity * collVel);

         damageQueuedObjects(collVel);
         
         MatrixF mat;
         mRigid.state.getTransform(&mat);
         Point3F up;
         mat.getColumn(2, &up);
         bool blowup = false;
         if (up.z < -0.25f)
            blowup = true;
         else
         {
            if (up.z <= mDataBlock->stuckTimerZ)
               mStuckTimer++;
            else
               mStuckTimer = 0;

            if (mStuckTimer >= mDataBlock->stuckTimerTicks)
               blowup = true;
         }

         if (blowup)
         {
            char buffer1[256];
            char buffer2[256];
            dSprintf(buffer1, 255, "%f %f %f",
                     mRigid.state.linPosition.x,
                     mRigid.state.linPosition.y,
                     mRigid.state.linPosition.z);
            dSprintf(buffer2, 255, "%d", Con::getIntVariable("$DamageType::Ground"));
            Con::executef(mDataBlock, 6, "damageObject", scriptThis(), "0", buffer1, "1000", buffer2);
         }
      }
   }
   else
   {
      if (isServerObject())
         mStuckTimer = 0;
   }
   
   return success;
}


void Vehicle::damageQueuedObjects(const F32 collisionVel)
{
   AssertFatal(isServerObject(), "Error, does not happen on the client");

   F32 damageVal = (collisionVel - mDataBlock->collDamageThresholdVel) * mDataBlock->collDamageMultiplier;
   if (damageVal < 0.0f)
      damageVal = 0.0f;
   
   // Notify all the objects that were just stamped during the queueing
   // process.
   SimTime expireTime = Sim::getCurrentTime() + CollisionTimeoutValue;

   char buffer2[256];
   dSprintf(buffer2, 255, "%d", Con::getIntVariable("$DamageType::Impact"));
   char damageBuffer[64];
   dSprintf(damageBuffer, 63, "%f", damageVal);
   
   for (CollisionTimeout* ptr = mTimeoutList; ptr; ptr = ptr->next)
   {
      SimObjectPtr<ShapeBase> safePtr(ptr->object);
      SimObjectPtr<ShapeBase> safeThis(this);
      onCollision(ptr->object);
      ptr->object = 0;

      if(!bool(safeThis))
         return;

      if(bool(safePtr))
      {
         if (ptr->useData == false && damageVal != 0.0f)
         {
            char buffer1[256];
            dSprintf(buffer1, 255, "%f %f %f",
                     mRigid.state.linPosition.x,
                     mRigid.state.linPosition.y,
                     mRigid.state.linPosition.z);
            Con::executef(safePtr->getDataBlock(), 6, "damageObject",
                          safePtr->scriptThis(), scriptThis(), buffer1, damageBuffer, buffer2);
         }
         else if (ptr->data > mDataBlock->collDamageThresholdVel)
         {
            F32 damageValLocal = (ptr->data - mDataBlock->collDamageThresholdVel) * mDataBlock->collDamageMultiplier;
               
            char buffer1[256];
            char buffer3[64];
            dSprintf(buffer1, 255, "%f %f %f",
                     mRigid.state.linPosition.x,
                     mRigid.state.linPosition.y,
                     mRigid.state.linPosition.z);
            dSprintf(buffer3, 63, "%f", damageValLocal);
            Con::executef(safePtr->getDataBlock(), 6, "damageObject",
                          safePtr->scriptThis(), scriptThis(), buffer1, buffer3, buffer2);
         }

         if (bool(safePtr) && bool(safeThis))
            safePtr->onCollision(safeThis);
      }

      if(!bool(safeThis))
         return;
   }

   CollisionTimeout* walk = mTimeoutList;
   mTimeoutList = NULL;
   while (walk)
   {
      CollisionTimeout* sFreeTimeoutList;
      CollisionTimeout* next = walk->next;
      walk->next = sFreeTimeoutList;
      sFreeTimeoutList = walk;
      walk = next;
   }
}

//----------------------------------------------------------------------------

bool Vehicle::resolveCollision(Rigid::State&  ns,CollisionList& cList)
{
   // Apply impulses to resolve collision
   bool collided = false;
   bool colliding;
   do {
      colliding = false;
      for (S32 i = 0; i < cList.count; i++) {
         Collision& c = cList.collision[i];
         if (c.distance < sCollisionTol) {
            Point3F v,r = c.point - ns.linPosition;
            ns.getVelocity(r,&v);
            F32 vn = mDot(v,c.normal);

            U32 objectMask = c.object->getTypeMask();

            if(objectMask & sDirtySetMask)
            {
               setControlDirty();
               if(objectMask & ShapeBaseObjectType)
                  static_cast<ShapeBase *>(c.object)->setControlDirty();
            }

            if (vn < -sContactTol) {
               mRigid.resolveCollision(ns,
                                       cList.collision[i].point,
                                       cList.collision[i].normal);
               colliding = true;
               collided  = true;

               // Track collisions
               if (!isGhost() && c.object->getTypeMask() & ShapeBaseObjectType)
                  queueCollision(static_cast<ShapeBase*>(c.object));
            }
         }
      }
   } while (colliding);

   return collided;
}


//----------------------------------------------------------------------------

F32 Vehicle::resolveContacts(Rigid::State& ns,CollisionList& cList,F32 dt)
{
   // Apply impulse to resolve contacts
   Point3F t,p(0,0,0),l(0,0,0);
   for (S32 i = 0; i < cList.count; i++) {
      Collision& c = cList.collision[i];
      if (c.distance < sCollisionTol) {
         Point3F v,r = c.point - ns.linPosition;
         ns.getVelocity(r,&v);
         F32 vn = mDot(v,c.normal);
         if (vn > -sContactTol) {
            // Penetration force
            F32 zi = mRigid.getZeroImpulse(mRigid.state,r,c.normal);
            F32 d = (sCollisionTol - c.distance) / sCollisionTol;
            F32 s = (d * d) * zi * sF - vn * sD;
            Point3F f = c.normal * (s * dt);

            // Frictional force
            Point3F uv = v - (c.normal * vn);
            F32 ul = uv.len();
            if (s > 0 && ul) {
               F32 u = s * mRigid.friction;
               f -= uv * (u * dt / ul);
            }

            //
            p += f;
            mCross(r,f,&t);
            l += t;
         }
      }
   }

   ns.linMomentum += p;
   ns.angMomentum += l;
   mRigid.updateVelocity(ns);
   return 0;
}


//----------------------------------------------------------------------------

void Vehicle::updateLiftoffDust( F32 dt )
{
   if( !mDustEmitterList[0] ) return;

   Point3F startPos = getPosition();
   Point3F endPos = startPos + Point3F( 0.0, 0.0, -mDataBlock->triggerDustHeight );


   RayInfo rayInfo;
   if( !getContainer()->castRay( startPos, endPos, TerrainObjectType, &rayInfo ) )
   {
      return;
   }

   TerrainBlock* tBlock = static_cast<TerrainBlock*>(rayInfo.object);
   S32 mapIndex = tBlock->mMPMIndex[0];

   MaterialPropertyMap* pMatMap = static_cast<MaterialPropertyMap*>(Sim::findObject("MaterialPropertyMap"));
   const MaterialPropertyMap::MapEntry* pEntry = pMatMap->getMapEntryFromIndex(mapIndex);

   if(pEntry)
   {
      S32 x;
      ColorF colorList[ParticleEngine::PC_COLOR_KEYS];
      
      for(x = 0; x < 2; ++x)
         colorList[x].set( pEntry->puffColor[x].red, pEntry->puffColor[x].green, pEntry->puffColor[x].blue, pEntry->puffColor[x].alpha );
      for(x = 2; x < ParticleEngine::PC_COLOR_KEYS; ++x)
         colorList[x].set( 1.0, 1.0, 1.0, 0.0 );
   
      mDustEmitterList[0]->setColors( colorList );
   }
   Point3F contactPoint = rayInfo.point + Point3F( 0.0, 0.0, mDataBlock->dustHeight );
   mDustEmitterList[0]->emitParticles( contactPoint, contactPoint, rayInfo.normal, getVelocity(), dt * 1000 );
}

//----------------------------------------------------------------------------

void Vehicle::updateDamageSmoke( F32 dt )
{

   for( S32 j=VehicleData::VC_NUM_DAMAGE_LEVELS-1; j>=0; j-- )
   {
      F32 damagePercent = mDamage / mDataBlock->maxDamage;
      if( damagePercent >= mDataBlock->damageLevelTolerance[j] )
      {
         for( int i=0; i<mDataBlock->numDmgEmitterAreas; i++ )
         {
            MatrixF trans = getTransform();
            Point3F offset = mDataBlock->damageEmitterOffset[i];
            trans.mulP( offset );
            Point3F emitterPoint = offset;

            if( pointInWater(offset ) )
            {
               U32 emitterOffset = VehicleData::VC_BUBBLE_EMITTER;
               if( mDamageEmitterList[emitterOffset] )
               {
                  mDamageEmitterList[emitterOffset]->emitParticles( emitterPoint, emitterPoint, Point3F( 0.0, 0.0, 1.0 ), getVelocity(), dt * 1000 );
               }
            }
            else
            {
               if( mDamageEmitterList[j] )
               {
                  mDamageEmitterList[j]->emitParticles( emitterPoint, emitterPoint, Point3F( 0.0, 0.0, 1.0 ), getVelocity(), dt * 1000 );
               }
            }
         }
         break;
      }
   }
   
}


//----------------------------------------------------------------------------

bool Vehicle::writePacketData(GameConnection *connection, BitStream *stream)
{
   bool ret = Parent::writePacketData(connection, stream);
   mathWrite(*stream, mSteering);

   mathWrite(*stream, mRigid.state.linPosition);
   mathWrite(*stream, mRigid.state.angPosition);
   mathWrite(*stream, mRigid.state.linMomentum);
   mathWrite(*stream, mRigid.state.angMomentum);

   stream->writeFlag(mDisableMove);
   stream->writeFlag(mFrozen);
   connection->setCompressionPoint(mRigid.state.linPosition);
   return ret;
}

void Vehicle::readPacketData(GameConnection *connection, BitStream *stream)
{
   Parent::readPacketData(connection, stream);
   mathRead(*stream, &mSteering);

   mathRead(*stream, &mRigid.state.linPosition);
   mathRead(*stream, &mRigid.state.angPosition);
   mathRead(*stream, &mRigid.state.linMomentum);
   mathRead(*stream, &mRigid.state.angMomentum);
   
   mRigid.updateVelocity(mRigid.state);
   mDisableMove = stream->readFlag();
   mFrozen = stream->readFlag();
   connection->setCompressionPoint(mRigid.state.linPosition);
}   


//----------------------------------------------------------------------------

U32 Vehicle::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   stream->writeFlag(mJetting);

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if (stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return retMask;

   F32 yaw = (mSteering.x + mDataBlock->maxSteeringAngle) / (2 * mDataBlock->maxSteeringAngle);
   F32 pitch = (mSteering.y + mDataBlock->maxSteeringAngle) / (2 * mDataBlock->maxSteeringAngle);
   stream->writeFloat(yaw,9);
   stream->writeFloat(pitch,9);
   mDelta.move.pack(stream);

   stream->writeFlag(mFrozen);

   if (stream->writeFlag(mask & PositionMask))
   {
      con->writeCompressed(stream, mRigid.state.linPosition);
      mathWrite(*stream, mRigid.state.angPosition);
      mathWrite(*stream, mRigid.state.linMomentum);
      mathWrite(*stream, mRigid.state.angMomentum);
   }

   // send energy only to clients which need it
   bool found = false;
   if(mask & EnergyMask)
   {
      for (ShapeBase* ptr = getMountList(); ptr; ptr = ptr->getMountLink())
      {
         if(!dynamic_cast<Player*>(ptr))
            continue;

         GameConnection * controllingClient = ptr->getControllingClient();
         if(controllingClient == con)
         {
            if(controllingClient->getControlObject() != this)
               found = true;
            break;
         }
      }
   }

   // write it...
   if(stream->writeFlag(found))
      stream->writeFloat(mClampF(getEnergyValue(), 0.f, 1.f), 8);

   return retMask;
}   

void Vehicle::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con,stream);

   mJetting = stream->readFlag();

   if (stream->readFlag())
      return;

   F32 yaw = stream->readFloat(9);
   F32 pitch = stream->readFloat(9);
   mSteering.x = (2 * yaw * mDataBlock->maxSteeringAngle) - mDataBlock->maxSteeringAngle;
   mSteering.y = (2 * pitch * mDataBlock->maxSteeringAngle) - mDataBlock->maxSteeringAngle;
   mDelta.move.unpack(stream);

   mFrozen = stream->readFlag();
   if (stream->readFlag()) {
      F32 speed = mRigid.state.linVelocity.len();
      mDelta.warpRot[0] = mRigid.state.angPosition;
      con->readCompressed(stream, &mRigid.state.linPosition);
      mathRead(*stream, &mRigid.state.angPosition);
      mathRead(*stream, &mRigid.state.linMomentum);
      mathRead(*stream, &mRigid.state.angMomentum);
      mRigid.updateVelocity(mRigid.state);

      mPredictionCount = sMaxPredictionTicks;

      if (isProperlyAdded() && mFrozen == false) {
         // Determin number of ticks to warp based on the average
         // of the client and server velocities.
         Point3F cp;
         mObjToWorld.getColumn(3,&cp);
         mDelta.warpOffset = mRigid.state.linPosition - cp;
         F32 dt,as = (speed + mRigid.state.linVelocity.len()) * 0.5 * TickSec;
         if (!as || (dt = mDelta.warpOffset.len() / as) > sMaxWarpTicks)
            dt = mDelta.dt + sMaxWarpTicks;
         else
            dt = (dt <= mDelta.dt)? mDelta.dt : mCeil(dt - mDelta.dt) + mDelta.dt;

         // Adjust current frame interpolation
         if (mDelta.dt) {
            mDelta.pos = cp + (mDelta.warpOffset * (mDelta.dt / dt));
            mDelta.posVec = (cp - mDelta.pos) / mDelta.dt;
            QuatF cr;
            cr.interpolate(mDelta.rot[1],mDelta.rot[0],mDelta.dt);
            mDelta.rot[1].interpolate(cr,mRigid.state.angPosition,mDelta.dt / dt);
            mDelta.rot[0].extrapolate(mDelta.rot[1],cr,mDelta.dt);
         }

         // Calculated multi-tick warp
         mDelta.warpCount = 0;
         mDelta.warpTicks = (S32)(mFloor(dt));
         if (mDelta.warpTicks) {
            mDelta.warpOffset = mRigid.state.linPosition - mDelta.pos;
            mDelta.warpOffset /= mDelta.warpTicks;
            mDelta.warpRot[0] = mDelta.rot[1];
            mDelta.warpRot[1] = mRigid.state.angPosition;
         }
      }
      else {
         // Set the vehicle to the server position
         mDelta.dt  = 0;
         mDelta.pos = mRigid.state.linPosition;
         mDelta.posVec.set(0,0,0);
         mDelta.rot[1] = mDelta.rot[0] = mRigid.state.angPosition;
         mDelta.warpCount = mDelta.warpTicks = 0;
         setPosition(mRigid.state.linPosition, mRigid.state.angPosition);
      }
   }

   // energy?
   if(stream->readFlag())
      setEnergyLevel(stream->readFloat(8) * mDataBlock->maxEnergy);
}


//----------------------------------------------------------------------------

void Vehicle::initPersistFields()
{
   Parent::initPersistFields();

   addField("disableMove",   TypeBool,   Offset(mDisableMove, Vehicle));
}


void Vehicle::mountObject(ShapeBase* obj, U32 node)
{
   Parent::mountObject(obj, node);

   // Clear objects off the working list that are from objects mounted to us.
   //  (This applies mostly to players...)
   for (CollisionWorkingList* itr = mConvex.getWorkingList().wLink.mNext; itr != &mConvex.getWorkingList(); itr = itr->wLink.mNext) {
      if (itr->mConvex->getObject() == obj) {
         CollisionWorkingList* cl = itr;
         itr = itr->wLink.mPrev;
         cl->free();
      }
   }
}

//--------------------------------------------------------------------------
void Vehicle::updateFroth( F32 dt )
{
   // update bubbles
   Point3F moveDir = getVelocity();

   Point3F contactPoint;
   if( !collidingWithWater( contactPoint ) )
   {
      if(waterWakeHandle)
      {
         alxStop(waterWakeHandle);
         waterWakeHandle = 0;
      }
      return;
   }

   F32 speed = moveDir.len();
   if( speed < mDataBlock->splashVelEpsilon ) speed = 0.0;

   U32 emitRate = speed * mDataBlock->splashFreqMod * dt;

   U32 i;
   if(!waterWakeHandle)
      waterWakeHandle = alxPlay(mDataBlock->waterSound[VehicleData::Wake], &getTransform());
   alxSourceMatrixF(waterWakeHandle, &getTransform());
   
   for( i=0; i<VehicleData::VC_NUM_SPLASH_EMITTERS; i++ )
   {
      if( mSplashEmitterList[i] )
      {
         mSplashEmitterList[i]->emitParticles( contactPoint, contactPoint, Point3F( 0.0, 0.0, 1.0 ), 
                                               moveDir, emitRate );
      }
   }

}


//--------------------------------------------------------------------------
// Returns true if vehicle is intersecting a water surface (roughly)
//--------------------------------------------------------------------------
bool Vehicle::collidingWithWater( Point3F &waterHeight )
{
   Point3F curPos = getPosition();

   F32 height = mFabs( mObjBox.max.z - mObjBox.min.z );

   RayInfo rInfo;
   if( gClientContainer.castRay( curPos + Point3F(0.0, 0.0, height), curPos, WaterObjectType, &rInfo) )
   {
      waterHeight = rInfo.point;
      return true;
   }

   return false;
}

void Vehicle::setEnergyLevel(F32 energy)
{
   Parent::setEnergyLevel(energy);
   setMaskBits(EnergyMask);
}

// F32 Vehicle::getHeat() const
// {
//    return 1.0;
// }


void Vehicle::setFrozenState(const bool _frozen)
{
   mFrozen = _frozen;
   setControlDirty();
}


void Vehicle::renderObject(SceneState* state, SceneRenderImage* image)
{
   Parent::renderObject(state, image);

   if (gShowBoundingBox) {
      RectI viewport;
      dglGetViewport(&viewport);

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      state->setupObjectProjection(this);

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      dglMultMatrix(&getRenderTransform());

      glDisable(GL_DEPTH_TEST);
      //--------------------------------------
      glColor3f(1, 0, 1);
      wireCube(Point3F(0.25,0.25,0.25),Point3F(0,0,0));

      glColor3f(1, 1, 1);
      wireCube(Point3F(0.25,0.25,0.25),mDataBlock->massCenter);

      //--------------------------------------
      glEnable(GL_DEPTH_TEST);

      glPopMatrix();

      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      dglSetViewport(viewport);


      // Show some collision points...
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      state->setupObjectProjection(this);

      ConvexFeature fa;
      MatrixF mat;
      mRigid.state.getTransform(&mat);
      mConvex.getFeatures(mat, Point3F(0, -1, 0), &fa);

      glDisable(GL_DEPTH_TEST);
      glColor3f(1, 0, 1);
      for (U32 i = 0; i < fa.mVertexList.size(); i++)
      {
         wireCube(Point3F(0.25, 0.25, 0.25), fa.mVertexList[i]);
      }

      glColor3f(1, 1, 0);
      for (U32 i = 0; i < fa.mEdgeList.size(); i++)
      {
         glBegin(GL_LINES);
         glVertex3fv(fa.mVertexList[fa.mEdgeList[i].vertex[0]]);
         glVertex3fv(fa.mVertexList[fa.mEdgeList[i].vertex[1]]);
         glEnd();
      }

      ClippedPolyList polyList;
      // Planes bounding the square.
      polyList.mPlaneList.setSize(6);
      polyList.mPlaneList[0].set(getWorldBox().min - Point3F(100, 100, 100),VectorF(-1,0,0));
      polyList.mPlaneList[1].set(getWorldBox().max + Point3F(100, 100, 100), VectorF(0,1,0));
      polyList.mPlaneList[2].set(getWorldBox().max - Point3F(100, 100, 100), VectorF(1,0,0));
      polyList.mPlaneList[3].set(getWorldBox().min + Point3F(100, 100, 100),VectorF(0,-1,0));
      polyList.mPlaneList[4].set(getWorldBox().min - Point3F(100, 100, 100),VectorF(0,0,-1));
      polyList.mPlaneList[5].set(getWorldBox().max + Point3F(100, 100, 100),VectorF(0,0,1));
      Box3F dummyBox;
      SphereF dummySphere;
      buildPolyList(&polyList, dummyBox, dummySphere);

      glColor3f(0, 1, 1);
      for (U32 i = 0; i < polyList.mVertexList.size(); i++)
      {
         wireCube(Point3F(0.25, 0.25, 0.25), polyList.mVertexList[i].point);
      }

      glEnable(GL_DEPTH_TEST);
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
   }
}
