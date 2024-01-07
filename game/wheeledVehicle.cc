//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

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
#include "game/wheeledVehicle.h"
#include "game/particleEngine.h"
#include "audio/audio.h"
#include "game/forceFieldBare.h"
#include "scenegraph/sceneGraph.h"
#include "sim/decalManager.h"
#include "dgl/materialPropertyMap.h"
#include "terrain/terrData.h"

//----------------------------------------------------------------------------

static U32 sCollisionMoveMask = (TerrainObjectType    | InteriorObjectType       |
                                 PlayerObjectType     | StaticShapeObjectType    |
                                 VehicleObjectType    | VehicleBlockerObjectType |
                                 ForceFieldObjectType | StaticTSObjectType);
static U32 sServerCollisionMask = sCollisionMoveMask; // ItemObjectType
static U32 sClientCollisionMask = sCollisionMoveMask;

static F32 sWheeledVehicleGravity = -20;
static F32 sBreakZeroEpsilon = 0.02;         // m/s
static F32 sTireEmitterVerticalScale = 0.7;
static F32 sTireCollisionExpansion = 0.02;   // Expanded size in buildPolyList

// Sound
static F32 sMinSqueelVolume = 0.05;
static F32 sIdleEngineVolume = 0.2;


//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(WheeledVehicleData);

WheeledVehicleData::WheeledVehicleData()
{
   tire.friction = 0.3;
   tire.restitution = 1;
   tire.radius = 0.6;
   tire.lateralForce = 1000;
   tire.lateralDamping = 100;
   tire.lateralRelaxation = 1;
   tire.longitudinalForce = 1000;
   tire.longitudinalDamping = 100;
   tire.longitudinalRelaxation = 1;
   tire.emitter = 0;
   maxWheelSpeed = 40;
   engineTorque = 1;
   breakTorque = 1;
   staticLoadScale = 1;
   antiSwayForce = 1;
   antiRockForce = 0;
   springForce = 0.6;
   tailLightSequence = -1;

   stabilizerForce = 0;
   gyroForce = 0;
   gyroDamping = 0;

   for (S32 i = 0; i < MaxSounds; i++)
      sound[i] = 0;
}

bool WheeledVehicleData::preload(bool server, char errorBuffer[256])
{
   if (!Parent::preload(server, errorBuffer))
      return false;

   TSShapeInstance* si = new TSShapeInstance(shape, false);

   // Resolve objects transmitted from server
   if (!server) {
      for (S32 i = 0; i < MaxSounds; i++)
         if (sound[i])
            Sim::findObject(SimObjectId(sound[i]),sound[i]);

      if (tire.emitter)
         Sim::findObject(SimObjectId(tire.emitter),tire.emitter);
   }

   // Extract wheel information from the shape
   TSThread* thread = si->addThread();
   Wheel* wp = wheel;
   for (S32 i = 0; i < MaxWheels; i++) {
      char buff[10];

      // Spring and ground information have to exist for
      // the wheel to operate at all.
      dSprintf(buff,sizeof(buff),"ground%d",i);
      wp->springNode = shape->findNode(buff);
      dSprintf(buff,sizeof(buff),"spring%d",i);
      wp->springSequence = shape->findSequence(buff);
      if (wp->springSequence != -1 && wp->springNode != -1) {

         // Extract spring pos & movement
         si->setSequence(thread,wp->springSequence,0);
         si->animate();
         si->mNodeTransforms[wp->springNode].getColumn(3,&wp->spring);
         si->setPos(thread,1);
         si->animate();
         si->mNodeTransforms[wp->springNode].getColumn(3, &wp->pos);
         wp->safePos.set(wp->pos.x, wp->pos.y, wp->pos.z + tire.radius);
         if (!mirrorWheel(wp)) {
            wp->spring.x = wp->spring.y = 0;
            wp->spring.z -= wp->pos.z;
         }

         // More animation sequences
         dSprintf(buff,sizeof(buff),"turn%d",i);
         wp->steeringSequence = shape->findSequence(buff);
         dSprintf(buff,sizeof(buff),"wheel%d",i);
         wp->rotationSequence = shape->findSequence(buff);

         //
         wp->springRest = 0.5;
         wp->springForce = springForce;
         wp->springDamping = springDamping;
         wp->steering = (wp->steeringSequence != -1)? Wheel::Forward: Wheel::None;
         wp++;
      }
   }
   wheelCount = wp - wheel;

   //
   tailLightSequence = shape->findSequence("taillight");

   // Extract collision planes from shape collision detail level
   if (collisionDetails[0] != -1) {
      MatrixF imat(1);
      SphereF sphere;
      sphere.center = shape->center;
      sphere.radius = shape->radius;
      PlaneExtractorPolyList polyList;
      polyList.mPlaneList = &rigidBody.mPlaneList;
      polyList.setTransform(&imat, Point3F(1,1,1));
      si->buildPolyList(&polyList,collisionDetails[0]);
   }

   delete si;
   return true;
}

bool WheeledVehicleData::mirrorWheel(Wheel* we)
{
   we->opposite = -1;
   // Find matching wheel mirrored along Y axis
   for (Wheel* wp = wheel; wp != we; wp++)
      if (mFabs(wp->pos.y - we->pos.y) < 0.5) {
         we->pos.x = -wp->pos.x;
         we->pos.y = wp->pos.y;
         we->pos.z = wp->pos.z;
         we->spring = wp->spring;
         we->opposite = wp - wheel;
         wp->opposite = we - wheel;
         return true;
      }
   return false;
}


void WheeledVehicleData::initPersistFields()
{
   Parent::initPersistFields();

   addField("jetSound", TypeAudioProfilePtr, Offset(sound[JetSound], WheeledVehicleData));
   addField("engineSound", TypeAudioProfilePtr, Offset(sound[EngineSound], WheeledVehicleData));
   addField("squeelSound", TypeAudioProfilePtr, Offset(sound[SqueelSound], WheeledVehicleData));
   addField("WheelImpactSound", TypeAudioProfilePtr, Offset(sound[WheelImpactSound], WheeledVehicleData));

   addField("tireRadius", TypeF32, Offset(tire.radius, WheeledVehicleData));
   addField("tireFriction", TypeF32, Offset(tire.friction, WheeledVehicleData));
   addField("tireRestitution", TypeF32, Offset(tire.restitution, WheeledVehicleData));
   addField("tireLateralForce", TypeF32, Offset(tire.lateralForce, WheeledVehicleData));
   addField("tireLateralDamping", TypeF32, Offset(tire.lateralDamping, WheeledVehicleData));
   addField("tireLateralRelaxation", TypeF32, Offset(tire.lateralRelaxation, WheeledVehicleData));
   addField("tireLongitudinalForce", TypeF32, Offset(tire.longitudinalForce, WheeledVehicleData));
   addField("tireLongitudinalDamping", TypeF32, Offset(tire.longitudinalDamping, WheeledVehicleData));
   addField("tireLogitudinalRelaxation", TypeF32, Offset(tire.longitudinalRelaxation, WheeledVehicleData));
   addField("tireEmitter",TypeParticleEmitterDataPtr, Offset(tire.emitter, WheeledVehicleData));

   addField("maxWheelSpeed", TypeF32, Offset(maxWheelSpeed, WheeledVehicleData));
   addField("engineTorque", TypeF32, Offset(engineTorque, WheeledVehicleData));
   addField("breakTorque", TypeF32, Offset(breakTorque, WheeledVehicleData));
   addField("staticLoadScale", TypeF32, Offset(staticLoadScale, WheeledVehicleData));
   addField("springForce", TypeF32, Offset(springForce, WheeledVehicleData));
   addField("springDamping", TypeF32, Offset(springDamping, WheeledVehicleData));
   addField("antiSwayForce", TypeF32, Offset(antiSwayForce, WheeledVehicleData));
   addField("antiRockForce", TypeF32, Offset(antiRockForce, WheeledVehicleData));

   addField("stabilizerForce", TypeF32, Offset(stabilizerForce, WheeledVehicleData));
   addField("gyroForce", TypeF32, Offset(gyroForce, WheeledVehicleData));
   addField("gyroDamping", TypeF32, Offset(gyroDamping, WheeledVehicleData));
}

void WheeledVehicleData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(tire.friction);
   stream->write(tire.restitution);
   stream->write(tire.radius);
   stream->write(tire.lateralForce);
   stream->write(tire.lateralDamping);
   stream->write(tire.lateralRelaxation);
   stream->write(tire.longitudinalForce);
   stream->write(tire.longitudinalDamping);
   stream->write(tire.longitudinalRelaxation);

   if (stream->writeFlag(tire.emitter))
      stream->writeRangedU32(packed? SimObjectId(tire.emitter):
                             tire.emitter->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);

   for (S32 i = 0; i < MaxSounds; i++)
      if (stream->writeFlag(sound[i]))
         stream->writeRangedU32(packed? SimObjectId(sound[i]):
                                sound[i]->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);

   stream->write(springForce);
   stream->write(springDamping);
   stream->write(antiSwayForce);
   stream->write(antiRockForce);
   stream->write(maxWheelSpeed);
   stream->write(engineTorque);
   stream->write(breakTorque);
   stream->write(staticLoadScale);

   stream->write(stabilizerForce);
   stream->write(gyroForce);
   stream->write(gyroDamping);
}

void WheeledVehicleData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&tire.friction);
   stream->read(&tire.restitution);
   stream->read(&tire.radius);
   stream->read(&tire.lateralForce);
   stream->read(&tire.lateralDamping);
   stream->read(&tire.lateralRelaxation);
   stream->read(&tire.longitudinalForce);
   stream->read(&tire.longitudinalDamping);
   stream->read(&tire.longitudinalRelaxation);

   tire.emitter = stream->readFlag()?
      (ParticleEmitterData*) stream->readRangedU32(DataBlockObjectIdFirst,
                                                   DataBlockObjectIdLast): 0;
   
   for (S32 i = 0; i < MaxSounds; i++)
      sound[i] = stream->readFlag()?
         (AudioProfile*) stream->readRangedU32(DataBlockObjectIdFirst,
                                               DataBlockObjectIdLast): 0;

   stream->read(&springForce);
   stream->read(&springDamping);
   stream->read(&antiSwayForce);
   stream->read(&antiRockForce);
   stream->read(&maxWheelSpeed);
   stream->read(&engineTorque);
   stream->read(&breakTorque);
   stream->read(&staticLoadScale);

   stream->read(&stabilizerForce);
   stream->read(&gyroForce);
   stream->read(&gyroDamping);
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(WheeledVehicle);

WheeledVehicle::WheeledVehicle()
{
   mGenerateShadow = true;

   mBraking = false;
   mWheelContact = false;
   mTailLightThread = 0;

   for (S32 i = 0; i < WheeledVehicleData::MaxWheels; i++) {
      mWheel[i].springThread = 0;
      mWheel[i].steeringThread = 0;
      mWheel[i].rotationThread = 0;
      mWheel[i].Dy = mWheel[i].Dx = 0;
   }

   mJetSound = 0;
   mEngineSound = 0;
   mSqueelSound = 0;

   mDataBlock = NULL;
}

WheeledVehicle::~WheeledVehicle()
{
   if (mJetSound)
      alxStop(mJetSound);
   if (mEngineSound)
      alxStop(mEngineSound);
   if (mSqueelSound)
      alxStop(mSqueelSound);
}


//----------------------------------------------------------------------------

bool WheeledVehicle::onAdd()
{
   if(!Parent::onAdd())
      return false;

   addToScene();
   if (isServerObject())
      scriptOnAdd();
   return true;
}

bool WheeledVehicle::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<WheeledVehicleData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   F32 frontStatic = 0;
   F32 backStatic = 0;
   F32 fCount = 0;
   F32 bCount = 0;

   // Wheel threads
   for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
      WheeledVehicleData::Wheel* wd = &mDataBlock->wheel[i];
      Wheel* wp = &mWheel[i];

      wp->surface.contact = false;
      wp->surface.object  = NULL;
      wp->evel = 0;
      wp->avel = 0;
      wp->apos = 0;
      wp->steeringThread = 0;
      wp->springThread = 0;
      wp->rotationThread = 0;

      if (wd->steeringSequence != -1) {
         wp->steeringThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(wp->steeringThread,wd->steeringSequence,0);
      }
      if (wd->springSequence != -1) {
         wp->springThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(wp->springThread,wd->springSequence,0);
      }
      if (wd->rotationSequence != -1) {
         wp->rotationThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(wp->rotationThread,wd->rotationSequence,0);
      }

      // Set springs to currenty gravity
      wp->k = wd->springForce;
      wp->s = wd->springDamping;
//      F32 staticForce = (mMass * -sWheeledVehicleGravity) / mDataBlock->wheelCount;
//      F32 sprungForce = staticForce * 0.8;
//      wp->center = wd->springRest + (sprungForce / wp->k);
//      if (wp->center > 1)
      wp->center = 1;
      wp->extension = wp->center;

      //
      wp->particleSlip = 0;
      if (mDataBlock->tire.emitter) {
         wp->emitter = new ParticleEmitter;
         wp->emitter->onNewDataBlock(mDataBlock->tire.emitter);
         wp->emitter->registerObject();
      }
      else
         wp->emitter = NULL;
   }

   //
   if (mDataBlock->tailLightSequence != -1) {
      mTailLightThread = mShapeInstance->addThread();
      mShapeInstance->setSequence(mTailLightThread,mDataBlock->tailLightSequence,0);
   }
   else
      mTailLightThread = 0;

   // Sounds
   if (mJetSound) {
      alxStop(mJetSound);
      mJetSound = 0;
   }
   if (mEngineSound) {
      alxStop(mEngineSound);
      mEngineSound = 0;
   }
   if (mSqueelSound) {
      alxStop(mSqueelSound);
      mSqueelSound = 0;
   }
   if (isGhost()) {
      if (mDataBlock->sound[WheeledVehicleData::EngineSound])
         mEngineSound = alxPlay(mDataBlock->sound[WheeledVehicleData::EngineSound], &getTransform());
   }

   scriptOnNewDataBlock();
   return true;
}

void WheeledVehicle::onRemove()
{
   if (mDataBlock != NULL)
   {
      for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
         Wheel* wp = &mWheel[i];
         if (bool(wp->emitter)) {
            wp->emitter->deleteWhenEmpty();
            wp->emitter = 0;
         }
      }

   }
   scriptOnRemove();
   removeFromScene();
   Parent::onRemove();
}


//----------------------------------------------------------------------------

void WheeledVehicle::updateWarp()
{
   updateWheels();
}

void WheeledVehicle::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   Point3F bz;
   mObjToWorld.getColumn(2,&bz);

   if (mTailLightThread)
      mShapeInstance->advanceTime(dt,mTailLightThread);

   // Update wheels
   if (mFrozen == false)
   {
      F32 slipTotal = 0;
      F32 torqueTotal = 0;
      for (S32 i = 0; i < mDataBlock->wheelCount; i++)
      {
         Wheel* wp = &mWheel[i];

         // Update angular position
         wp->apos += (wp->avel * dt) / M_2PI;
         wp->apos -= mFloor(wp->apos);
         if (wp->apos < 0)
            wp->apos = 1 - wp->apos;

         // Keep track of largest slip
         slipTotal += wp->particleSlip;
         torqueTotal += wp->torqueScale;

         Point3F wv = Parent::getVelocity();

         // emit dust if moving
         if( wv.len() > 1.0 && wp->surface.object && wp->surface.object->getTypeMask() & TerrainObjectType )
         {
            TerrainBlock* tBlock = static_cast<TerrainBlock*>(wp->surface.object);
            S32 mapIndex = tBlock->mMPMIndex[0];

            MaterialPropertyMap* pMatMap = static_cast<MaterialPropertyMap*>(Sim::findObject("MaterialPropertyMap"));
            const MaterialPropertyMap::MapEntry* pEntry = pMatMap->getMapEntryFromIndex(mapIndex);
    
            if(pEntry)
            {
               S32 x;
               ColorF colorList[ParticleEngine::PC_COLOR_KEYS];
            
               for(x = 0; x < 2; ++x)
                  colorList[x].set(pEntry->puffColor[x].red,
                                   pEntry->puffColor[x].green,
                                   pEntry->puffColor[x].blue,
                                   pEntry->puffColor[x].alpha);

               for(x = 2; x < ParticleEngine::PC_COLOR_KEYS; ++x)
                  colorList[x].set( 1.0, 1.0, 1.0, 0.0 );

               wp->emitter->setColors( colorList );
            }
            Point3F axis = wv;
            axis.normalize();

            wp->emitter->emitParticles(wp->surface.pos + Point3F( 0.0, 0.0, 0.5 ),true,
                                       axis, wv, dt * 1000 * (wv.len() / 15.0) );
         }
      }

      //
      updateJet(dt);
      updateSqueelSound(slipTotal / mDataBlock->wheelCount);
      updateEngineSound(sIdleEngineVolume + (1 - sIdleEngineVolume) *
                        (1 - (torqueTotal / mDataBlock->wheelCount)));
   }
}


//----------------------------------------------------------------------------

bool WheeledVehicle::buildPolyList(AbstractPolyList* polyList, const Box3F& box, const SphereF& sphere)
{
   // Parent will take care of body collision.
   Parent::buildPolyList(polyList,box,sphere);

   // Add wheels.
   Box3F wbox;
   wbox.min.x = -(wbox.max.x = (mDataBlock->tire.radius / 2) + sTireCollisionExpansion);
   wbox.min.y = -(wbox.max.y = mDataBlock->tire.radius + sTireCollisionExpansion);
   wbox.max.z = 2 * mDataBlock->tire.radius;
   wbox.min.z = 0;
   MatrixF mat = mObjToWorld;

   for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
      WheeledVehicleData::Wheel* wd = &mDataBlock->wheel[i];
      Wheel* wp = &mWheel[i];

      Point3F sp,vec;
      mObjToWorld.mulP(wd->pos,&sp);
      mObjToWorld.mulV(wd->spring,&vec);
      Point3F ep = sp + (vec * wp->extension);
      mat.setColumn(3,ep);
      polyList->setTransform(&mat,Point3F(1,1,1));
      polyList->addBox(wbox);
   }
   return !polyList->isEmpty();
}


//----------------------------------------------------------------------------
void WheeledVehicle::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isServerObject() && mWaterCoverage > 0.5)
   {
      char buffer1[256], buffer2[256];
      dSprintf(buffer1, 255, "%f %f %f",
               mRigid.state.linPosition.x,
               mRigid.state.linPosition.y,
               mRigid.state.linPosition.z);
      dSprintf(buffer2, 255, "%d", Con::getIntVariable("$DamageType::Water"));
      Con::executef(mDataBlock, 6, "damageObject", scriptThis(), "0", buffer1, "1000", buffer2);
         
   }
}

void WheeledVehicle::updateMove(const Move* move)
{
   Parent::updateMove(move);

   // Breaking
   F32 wvel = 0;
   for (S32 i = 0; i < mDataBlock->wheelCount; i++)
      wvel += mWheel[i].avel;
   if (mFabs(wvel * mDataBlock->tire.radius) < sBreakZeroEpsilon)
      wvel = 0;
   if (mBraking) {
      // If the throttle is not set, or is set in the same direction
      // as are wheel velocity, then we are no longer breaking.
      if (!wvel || !mThrottle || (mThrottle > 0 && wvel > 0) || (mThrottle < 0 && wvel < 0))
         mBraking = false;
   }
   else
      // If the throttle is opposite to our wheel velocity, then break.
      if ((mThrottle > 0 && wvel < 0) || (mThrottle < 0 && wvel > 0))
         mBraking = true;

   if (mTailLightThread)
      mShapeInstance->setTimeScale(mTailLightThread,mBraking? 1: -1);

   //
   updateWheels();

   if (mWheelContact)
      mSteering.y = 0;
}


//----------------------------------------------------------------------------

void WheeledVehicle::updateForces(F32 dt)
{
   S32 j;

   MatrixF currMatrix;
   mRigid.state.getTransform(&currMatrix);
   MatrixF currMatrixInv = currMatrix;
   currMatrixInv.inverse();

   //
   F32 oneOverSprungMass = 1 / (mMass * 0.8);
   F32 maxAvel           = mDataBlock->maxWheelSpeed / mDataBlock->tire.radius;
   F32 aMomentum         = mMass / mDataBlock->wheelCount;

   mRigid.state.force.set(0, 0, 0);
   mRigid.state.torque.set(0, 0, 0);

   // Drag 
   {
      mRigid.state.force  -= mRigid.state.linVelocity * mDataBlock->minDrag * mOneOverMass;
      mRigid.state.torque -= mRigid.state.angMomentum * mDataBlock->antiRockForce * mOneOverMass;
   }

   // Body & Steering Vectors
   Point3F bx,by,bz;
   {
      currMatrix.getColumn(0,&bx);
      currMatrix.getColumn(1,&by);
      currMatrix.getColumn(2,&bz);
   }

   Point3F worldZ(0, 0, 1);
   Point3F worldY(0, 1, 0);
   currMatrix.getColumn(1, &worldY);
   currMatrix.getColumn(2, &worldZ);

   F32 quadraticSteering = -(mSteering.x * mFabs(mSteering.x));
   F32 cosSteering;
   F32 sinSteering;
   mSinCos(quadraticSteering, sinSteering, cosSteering);

   // Center of mass in world space
   Point3F massCenter;
   currMatrix.mulP(mDataBlock->massCenter, &massCenter);
   currMatrix.mulP(Point3F(0, 0, 0), &massCenter);

   Point3F force  = Point3F(0,0,0);
   Point3F torque = Point3F(0,0,0);

   AssertFatal(mDataBlock->wheelCount < 8, "Error, only <= 8 wheels supported");
   Point3F wheelForce[8];

   // Calculate vertical load for friction.  This was marked by TimG as "a hack"
   U32 contactCount = 0;
   F32 verticalLoad = 0;
   {
      for (j = 0; j < mDataBlock->wheelCount; j++) {
         if (mWheel[j].surface.contact)
            contactCount++;
      }

      if (contactCount != 0) {
         verticalLoad = (mDataBlock->staticLoadScale *
                         (mMass * -sWheeledVehicleGravity) / contactCount);
      }
   }

   // Engine and break torque
   F32 engineTorque,breakTorque,maxBreakVel;
   if (mBraking) {
      breakTorque = mDataBlock->breakTorque * mFabs(mThrottle);
      maxBreakVel = (breakTorque / aMomentum) * dt;
      engineTorque = 0;
   }
   else {
      if (mThrottle) {
         engineTorque = mDataBlock->engineTorque * mThrottle;
         maxBreakVel = breakTorque = 0;
         if (mThrottle > 0 && mJetting)
            // Double the engineTorque to help out the jets
            engineTorque += mDataBlock->engineTorque;
      }
      else {
         // Engine break.
         breakTorque = mDataBlock->engineTorque;
         maxBreakVel = (breakTorque / aMomentum) * dt;
         engineTorque = 0;
      }
   }

   force = Point3F(0, 0, sWheeledVehicleGravity);

   // Jet Force
   if (mJetting)
   {
      force += worldY * (mDataBlock->jetForce * mOneOverMass);
   }

   // Calculate wheel forces
   //
   for (j = 0; j < mDataBlock->wheelCount; j++)
   {
      Wheel* wheel = &mWheel[j];
      WheeledVehicleData::Wheel* wheelData = &mDataBlock->wheel[j];

      // Zero the force on this wheel
      Point3F& forceVector = wheelForce[j];
      forceVector.set(0, 0, 0);

      if (wheel->surface.contact) {
         // Wheel in contact with the ground.
         //  Forces acting on the body due to this wheel:
         //   - Spring forces

         //  Torques acting on the body due to this wheel:
         //   - None

         // First, let's compute the wheels position, and worldspace velocity
         Point3F pos, r, localVel;
         currMatrix.mulP(wheelData->pos, &pos);
         r = pos - massCenter;
         getVelocity(r, &localVel);

         // Spring forces on this wheel act in the z direction of the body, at the point
         //  of contact.
         {
            // Spring force
            F32 spring  = wheel->k * (wheel->center - wheel->extension);

            // Damping in the spring
            F32 damping = wheel->s * -mDot(bz, localVel);
            if (damping < 0)
               damping = 0;

            // Anti-sway force based on difference in suspension extension
            F32 antiSway = 0;
            if (wheelData->opposite != -1)
            {
               Wheel* oppositeWheel = &mWheel[wheelData->opposite];
               if (oppositeWheel->surface.contact) {
                  antiSway = ((oppositeWheel->extension - wheel->extension) *
                              mDataBlock->antiSwayForce);
               }
               if (antiSway < 0)
                  antiSway = 0;
            }

            forceVector += bz * ((spring + damping + antiSway) * oneOverSprungMass);
         }

         // Tire direction vectors perpendicular to surface normal
         Point3F wheelXVec;
         if (wheelData->steering == WheeledVehicleData::Wheel::Forward) {
            wheelXVec  = bx * cosSteering;
            wheelXVec += by * sinSteering;
         }
         else if (wheelData->steering == WheeledVehicleData::Wheel::Backward) {
            wheelXVec  = bx * cosSteering;
            wheelXVec -= by * sinSteering;
         }
         else {
            wheelXVec = bx;
         }

         Point3F tireX, tireY;
         mCross(wheel->surface.normal, wheelXVec, &tireY);
         tireY.normalize();
         mCross(tireY, wheel->surface.normal, &tireX);
         tireX.normalize();

         // Velocity of wheel at surface contact
         Point3F wheelContact = wheel->surface.pos - massCenter;
         Point3F wheelVelocity;
         getVelocity(wheelContact, &wheelVelocity);
         F32 xVelocity = mDot(tireX, wheelVelocity);
         F32 yVelocity = mDot(tireY, wheelVelocity);

         // Longitudinal deformation force
         F32 ddy = ((wheel->avel * mDataBlock->tire.radius) -
                    yVelocity -
                    mDataBlock->tire.longitudinalRelaxation * mFabs(wheel->avel) * wheel->Dy);

         wheel->Dy += ddy * dt;

         F32 Fy = (mDataBlock->tire.longitudinalForce   * wheel->Dy +
                   mDataBlock->tire.longitudinalDamping * ddy);

         // Lateral deformation force
         F32 ddx = (xVelocity -
                    (mDataBlock->tire.lateralRelaxation * mFabs(wheel->avel) * wheel->Dx));

         wheel->Dx += ddx * dt;

         F32 Fx = -(mDataBlock->tire.lateralForce * wheel->Dx +
                    mDataBlock->tire.lateralDamping * ddx);

         // Vertical load on tire.
         F32 Fz = verticalLoad;

         // Reduce forces based on friction limit
         F32 sN = mDot(wheel->surface.normal,Point3F(0,0,1));
         if (sN > 0) {
            F32 muS  = Fz * mDataBlock->tire.friction * sN;
            F32 muS2 = muS * muS;
            F32 Fn = (Fz * Fz) * muS2;
            F32 Ff = (Fx * Fx + Fy * Fy) * muS2;
            if (Ff > Fn) {
               F32 K = mSqrt(Fn / Ff);
               Fy *= K;
               Fx *= K;
               wheel->Dy *= K;
               wheel->Dx *= K;
            }
         }
         else {
            Fy = Fx = 0;
         }

         // Apply forces to wheel ground contact point
         forceVector += (tireX * (Fx * mOneOverMass)) + (tireY * (Fy * mOneOverMass));

         // Wheel angular acceleration from engine torque and tire 
         // deformation force.
         wheel->torqueScale = (mFabs(wheel->avel) > maxAvel) ? 0 :
            1 - (mFabs(wheel->avel) / maxAvel);

         wheel->avel += (((wheel->torqueScale * engineTorque) -
                          Fy *
                          mDataBlock->tire.radius) / aMomentum) * dt;

         // Wheel angular acceleration from break torque
         if (maxBreakVel > mFabs(wheel->avel))
         {
            wheel->avel = 0;
         } 
         else
         {
            if (wheel->avel > 0)
               wheel->avel -= maxBreakVel;
            else
               wheel->avel += maxBreakVel;
         }
      }
      else {
         // Wheel not in contact with the ground
         //  Forces acting on the body due to this wheel:
         //   - None
         //  Torques acting on the body due to this wheel:
         //   - None

         wheel->torqueScale  = 0;
         wheel->particleSlip = 0;
         wheel->Dy += (-mDataBlock->tire.longitudinalRelaxation *
                       mFabs(wheel->avel) * wheel->Dy) * dt;
         wheel->Dx += (-mDataBlock->tire.lateralRelaxation *
                       mFabs(wheel->avel) * wheel->Dx) * dt;
      }
   }

   // Sum up the forces
   {
      // Now sum up the torques and forces

      for (j = 0; j < mDataBlock->wheelCount; j++) {
         WheeledVehicleData::Wheel* wheelData = &mDataBlock->wheel[j];
         Point3F pos, r, t;
         currMatrix.mulP(wheelData->pos, &pos);
         r = pos - massCenter;
         mCross(r, wheelForce[j], &t);
         torque += t;
         force  += wheelForce[j];
      }
   }

   // Container buoyancy & drag
   force  += Point3F(0, 0, -mBuoyancy * sWheeledVehicleGravity);
   force  -= mRigid.state.linVelocity * mDrag;
   torque -= mRigid.state.angMomentum * mDrag;

   // Apply forces to body
   mRigid.state.force  += force;
   mRigid.state.torque += torque;
}


//----------------------------------------------------------------------------

U32 WheeledVehicle::getCollisionMask()
{
   if (isServerObject())
      return sServerCollisionMask;
   else
      return sClientCollisionMask;
}


//bool WheeledVehicle::collideBody(const MatrixF& mat,Collision* info)
//{
//   // Database bounding box
//   Box3F wBox = mObjBox;
//   mat.mul(wBox);
//
//   // Test the body against the database
//   Box3F box;
//   SphereF sphere;
//   MatrixF imat(1);
//   PlaneExtractorPolyList extractor;
//   sPolyList->mPlaneList.clear();
//   extractor.mPlaneList = &sPolyList->mPlaneList;
//   extractor.setTransform(&mat, Point3F(1,1,1));
//   mShapeInstance->buildPolyList(&extractor,mDataBlock->collisionDetails[0]);
//
//   sPolyList->clear();
//
//   // Build list from convex states here...
//   CollisionWorkingList& rList = mConvex.getWorkingList();
//   CollisionWorkingList* pList = rList.wLink.mNext;
//   while (pList != &rList) {
//      Convex* pConvex = pList->mConvex;
//      if (pConvex->getObject()->getTypeMask() & sCollisionMoveMask) {
//         pConvex->getPolyList(sPolyList);
//      }
//      pList = pList->wLink.mNext;
//   }
//
//   S32 count = sPolyList->mPolyList.size();
//   info->face = count? BodyCollision: 0;
//
//   // Test the wheels against the database
//   Point3F xvec,yvec,zvec;
//   mat.getColumn(0,&xvec);
//   xvec *= mObjBox.len_x();
//   mat.getColumn(1,&yvec);
//   yvec *= mObjBox.len_y();
//   mat.getColumn(2,&zvec);
//   zvec *= mObjBox.len_z();
//
//   for (S32 i = 0; i != mDataBlock->wheelCount; i++) {
//      WheeledVehicleData::Wheel* wd = &mDataBlock->wheel[i];
//      Wheel* wp = &mWheel[i];
//
//      Box3F box;
//      F32 wr = mDataBlock->tire.radius, ww = wr / 2;
//      box.min.set(wd->pos.x - ww,wd->pos.y - wr,wd->pos.z);
//      box.max.set(wd->pos.x + ww,wd->pos.y + wr,wd->pos.z + 2*wr);
//
//      Point3F min,max;
//      mat.mulP(box.min,&min);
//      mat.mulP(box.max,&max);
//
//      sPolyList->mPlaneList.clear();
//      sPolyList->mNormal.set(0,0,0);
//      sPolyList->mPlaneList.setSize(6);
//      sPolyList->mPlaneList[0].set(min,xvec);
//      sPolyList->mPlaneList[0].invert();
//      sPolyList->mPlaneList[1].set(max,yvec);
//      sPolyList->mPlaneList[2].set(max,xvec);
//      sPolyList->mPlaneList[3].set(min,yvec);
//      sPolyList->mPlaneList[3].invert();
//      sPolyList->mPlaneList[4].set(min,zvec);
//      sPolyList->mPlaneList[4].invert();
//      sPolyList->mPlaneList[5].set(max,zvec);
//
//      // Build list from convex states here...
//      CollisionWorkingList& rList = mConvex.getWorkingList();
//      CollisionWorkingList* pList = rList.wLink.mNext;
//      while (pList != &rList) {
//         Convex* pConvex = pList->mConvex;
//         if (pConvex->getObject()->getTypeMask() & sCollisionMoveMask) {
//            pConvex->getPolyList(sPolyList);
//         }
//         pList = pList->wLink.mNext;
//      }
//   }
//
//   if (sPolyList->mPolyList.size() != count)
//      info->face |= WheelCollision;
//
//   // Pick best collision point
//   F32 bestDist = 1.0E30;
//   ClippedPolyList::Poly* bestPoly = 0;
//   ClippedPolyList::Vertex* bestVertex;
//
//   if (sPolyList->mPolyList.size()) {
//      Point3F massCenter;
//      mat.mulP(mDataBlock->massCenter,&massCenter);
//   
//      // Pick surface with best vertex velocity
//      F32 bestVd = -1;
//      ClippedPolyList::Poly* poly = sPolyList->mPolyList.begin();
//      ClippedPolyList::Poly* end = sPolyList->mPolyList.end();
//      for (; poly != end; poly++) {
//         U32* vi = &sPolyList->mIndexList[poly->vertexStart];
//         U32* ve = vi + poly->vertexCount;
//         for (; vi != ve; vi++) {
//            ClippedPolyList::Vertex* ev = &sPolyList->mVertexList[*vi];
//
//            Point3F v,r;
//            r = ev->point - massCenter;
//            getVelocity(r,&v);
//
//            F32 dist = mDot(poly->plane,v);
//            if (dist < 0 && dist < bestDist) {
//               bestDist = dist;
//               bestVertex = ev;
//               bestPoly = poly;
//            }
//         }
//      }
//   }
//
//   //
//   if (bestPoly) {
//      info->point = bestVertex->point;
//      info->object = bestPoly->object;
//      info->normal = bestPoly->plane;
//      info->material = bestPoly->material;
//      return true;
//   }
//
//   return false;
//}


//----------------------------------------------------------------------------

void WheeledVehicle::updateWheels()
{
   disableCollision();
   static Polyhedron polyh;
//   static ExtrudedPolyList sExtrudedList;
   mWheelContact = false;

   MatrixF currMatrix;
   mRigid.state.getTransform(&currMatrix);

   for (S32 i = 0; i != mDataBlock->wheelCount; i++) {
      WheeledVehicleData::Wheel* wheelData = &mDataBlock->wheel[i];
      Wheel* currWheel = &mWheel[i];

      currWheel->extension = 1;

      Point3F sp;
      Point3F vec;
      currMatrix.mulP(wheelData->pos,&sp);
      currMatrix.mulV(wheelData->spring,&vec);
      Point3F hp = sp - vec * currWheel->extension;
      Point3F ep = sp + vec * currWheel->extension;

      MatrixF wmat = currMatrix;
      wmat.setColumn(3,hp);
      F32 wr = mDataBlock->tire.radius, ww = wr / 2;
      Box3F box;
      box.min.set(-ww,-wr,0);
      box.max.set(+ww,+wr,2*wr);
      polyh.buildBox(wmat,box);

      // DMMTODO: Replace with search through working set...
      //
      CollisionList collisionList;
      if (mContainer->buildCollisionList(polyh,
                                         hp, ep, vec * 2.0f,
                                         sClientCollisionMask & ~PlayerObjectType,
                                         &collisionList)) {
         currWheel->evel = 0;

         if (collisionList.t > 0.5)
            currWheel->extension *= (collisionList.t - 0.5) * 2.0f;
         else  
            currWheel->extension = 0;

         currWheel->surface.contact = true;
         currWheel->surface.pos = sp + vec * currWheel->extension;
         currWheel->surface.object = collisionList.collision[0].object;
         mWheelContact = true;

         // Pick flatest surface normal
         F32 dot = 1E30f;
         Collision *collision, *cp = collisionList.collision;
         Collision *ep = cp + collisionList.count;
         for (; cp != ep; cp++) {
            F32 d = mDot(cp->normal,vec);
            if (d < dot) {
               collision = cp;
               dot = d;
            }
         }

         currWheel->surface.normal   = collision->normal;
         currWheel->surface.material = collision->material;
      }
      else {
         // Make sure that we haven't sunk into the ground...
         Point3F safety;
         currMatrix.mulP(wheelData->safePos,&safety);

         RayInfo rInfo;
         if (mContainer->castRay(safety, sp, sClientCollisionMask & ~PlayerObjectType, &rInfo)) {
            // Actually stuck at this point
            currWheel->evel             = 0;
            currWheel->extension        = 0;
            currWheel->surface.normal   = rInfo.normal;
            currWheel->surface.pos      = rInfo.point;
            currWheel->surface.material = rInfo.material;
            currWheel->surface.contact  = true;
            currWheel->surface.object   = rInfo.object;
            mWheelContact = true;
         } else {
            // Ok, no collision.
            currWheel->surface.contact = false;
            currWheel->surface.object  = NULL;
         }
      }
   }
   enableCollision();
}


//----------------------------------------------------------------------------

void WheeledVehicle::updateWheelThreads()
{
   for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
      Wheel* wp = &mWheel[i];
      if (wp->springThread) {
         F32 p = wp->extension;
         if (p > wp->center)
            p = wp->center;
         mShapeInstance->setPos(wp->springThread,1 - p);
      }
      if (wp->steeringThread) {
         F32 t = (mSteering.x * mFabs(mSteering.x)) / mDataBlock->maxSteeringAngle;
         mShapeInstance->setPos(wp->steeringThread,0.5 - t * 0.5);
      }
      if (wp->rotationThread)
         mShapeInstance->setPos(wp->rotationThread,wp->apos);
   }
}   


//----------------------------------------------------------------------------

void WheeledVehicle::updateEngineSound(F32 level)
{
   if (mEngineSound) {
      alxSourceMatrixF(mEngineSound, &getTransform());
      alxSourcef(mEngineSound, AL_GAIN_LINEAR, level);
   }
}

void WheeledVehicle::updateSqueelSound(F32 level)
{
   if (!mDataBlock->sound[WheeledVehicleData::SqueelSound])
      return;
   // Allocate/Deallocate voice on demand.
   if (level < sMinSqueelVolume) {
      if (mSqueelSound) {
         alxStop(mSqueelSound);
         mSqueelSound = 0;
      }
   }
   else {
      if (!mSqueelSound)
         mSqueelSound = alxPlay(mDataBlock->sound[WheeledVehicleData::SqueelSound], &getTransform());

      alxSourceMatrixF(mSqueelSound, &getTransform());
      alxSourcef(mSqueelSound, AL_GAIN_LINEAR, level);
   }
}   

void WheeledVehicle::updateJet(F32 )
{
   if (!mDataBlock->sound[WheeledVehicleData::JetSound])
      return;
   // Allocate/Deallocate voice on demand.
   if (!mJetting) {
      if (mJetSound) {
         alxStop(mJetSound);
         mJetSound = 0;
      }
   }
   else {
      if (!mJetSound)
         mJetSound = alxPlay(mDataBlock->sound[WheeledVehicleData::JetSound], &getTransform());
      
      alxSourceMatrixF(mJetSound, &getTransform());
   }
}


//----------------------------------------------------------------------------

void WheeledVehicle::renderImage(SceneState* state, SceneRenderImage* image)
{
   updateWheelThreads();
   Parent::renderImage(state, image);
}   


//----------------------------------------------------------------------------

bool WheeledVehicle::writePacketData(GameConnection *connection, BitStream *stream)
{
   bool ret = Parent::writePacketData(connection, stream);
   stream->writeFlag(mBraking);

   for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
      Wheel* wp = &mWheel[i];
      stream->write(wp->avel);
      stream->write(wp->Dy);
      stream->write(wp->Dx);
   }
   return ret;
}

void WheeledVehicle::readPacketData(GameConnection *connection, BitStream *stream)
{
   Parent::readPacketData(connection, stream);
   mBraking = stream->readFlag();

   for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
      Wheel* wp = &mWheel[i];
      stream->read(&wp->avel);
      stream->read(&wp->Dy);
      stream->read(&wp->Dx);
   }

   setPosition(mRigid.state.linPosition,mRigid.state.angPosition);
   mDelta.pos = mRigid.state.linPosition;
   mDelta.rot[1] = mRigid.state.angPosition;
}

U32 WheeledVehicle::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if( ((GameConnection *) con)->getControlObject() == this && !(mask & InitialUpdateMask))
      return retMask;

   stream->writeFlag(mBraking);

   if (stream->writeFlag(mask & PositionMask)) {
      for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
         Wheel* wp = &mWheel[i];
         stream->write(wp->avel);
         stream->write(wp->Dy);
         stream->write(wp->Dx);
      }
   }
   return retMask;
}

void WheeledVehicle::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con,stream);

   if( ((GameConnection *) con)->getControlObject() == this)
      return;

   mBraking = stream->readFlag();

   if (stream->readFlag()) {
      for (S32 i = 0; i < mDataBlock->wheelCount; i++) {
         Wheel* wp = &mWheel[i];
         stream->read(&wp->avel);
         stream->read(&wp->Dy);
         stream->read(&wp->Dx);
      }
   }
}

void WheeledVehicle::initPersistFields()
{
   Parent::initPersistFields();
}   

