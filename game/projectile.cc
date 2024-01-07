//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "scenegraph/sceneState.h"
#include "scenegraph/sceneGraph.h"
#include "console/consoleTypes.h"
#include "core/bitStream.h"
#include "game/particleEngine.h"
#include "game/explosion.h"
#include "game/shapeBase.h"
#include "ts/tsShapeInstance.h"
#include "game/projectile.h"
#include "audio/audio.h"
#include "sim/decalManager.h"
#include "game/splash.h"
#include "terrain/waterBlock.h"
#include "math/mathUtils.h"

IMPLEMENT_CO_DATABLOCK_V1(ProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(Projectile);

const U32 Projectile::csmStaticCollisionMask =  TerrainObjectType    |
                                                InteriorObjectType   |
                                                ForceFieldObjectType |
                                                StaticObjectType;

const U32 Projectile::csmDynamicCollisionMask = PlayerObjectType        |
                                                VehicleObjectType       |
                                                StationObjectType       |
                                                GeneratorObjectType     |
                                                SensorObjectType        |
                                                DamagableItemObjectType |
                                                TurretObjectType;
const U32 Projectile::csmDamageableMask = Projectile::csmDynamicCollisionMask;

U32 Projectile::smProjectileWarpTicks = 5;


//--------------------------------------------------------------------------
//--------------------------------------
//
ProjectileData::ProjectileData()
{
   projectileShapeName = NULL;
   emitterDelay        = -1;

   velInheritFactor    = 1.0;
   isBallistic         = false;  

   directDamage        = 0.0f;
   hasDamageRadius     = false;
   indirectDamage      = 0.0f;
   damageRadius        = 0.0f;
   radiusDamageType    = 0;

   kickBackStrength    = 0.0;

   baseEmitter         = NULL;
   delayEmitter        = NULL;
   bubbleEmitter       = NULL;
   explosion           = NULL;
   underwaterExplosion = NULL;
   splash              = NULL;
   sound               = NULL;
   wetFireSound        = NULL;
   fireSound           = NULL;
   baseEmitterId       = 0;
   delayEmitterId      = 0;
   bubbleEmitterId     = 0;
   explosionId         = 0;
   underwaterExplosionId = 0;
   splashId            = 0;
   soundId             = 0;
   wetFireSoundId      = 0;
   fireSoundId         = 0;

   hasLight            = false;
   lightRadius         = 1;
   lightColor.set(1, 1, 1);

   hasLightUnderwaterColor = false;
   underWaterLightColor.set(1, 1, 1);

   explodeOnWaterImpact = false;
   depthTolerance = 5.0;
   bubbleEmitTime = 0.5;
   
   for (U32 i = 0; i < 6; i++) {
      decalData[i] = NULL;
      decalID[i]   = 0;
   }
   numDecals = 0;
   faceViewer = false;
   scale.set( 1.0, 1.0, 1.0 );

}

ProjectileData::~ProjectileData()
{
   //
}

//--------------------------------------------------------------------------
IMPLEMENT_GETDATATYPE(ProjectileData)
IMPLEMENT_SETDATATYPE(ProjectileData)

void ProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   Con::registerType(TypeProjectileDataPtr, sizeof(ProjectileData*),
                     REF_GETDATATYPE(ProjectileData),
                     REF_SETDATATYPE(ProjectileData));

   addField("projectileShapeName", TypeString, Offset(projectileShapeName, ProjectileData));
   addField("emitterDelay",        TypeS32,    Offset(emitterDelay,        ProjectileData));

   addField("velInheritFactor",    TypeF32,    Offset(velInheritFactor,    ProjectileData));

   addField("directDamage",        TypeF32,    Offset(directDamage,        ProjectileData));
   addField("hasDamageRadius",     TypeBool,   Offset(hasDamageRadius,     ProjectileData));
   addField("indirectDamage",      TypeF32,    Offset(indirectDamage,      ProjectileData));
   addField("damageRadius",        TypeF32,    Offset(damageRadius,        ProjectileData));
   addField("radiusDamageType",    TypeS32,    Offset(radiusDamageType,    ProjectileData));
   addField("kickBackStrength",    TypeF32,    Offset(kickBackStrength,    ProjectileData));

   addField("baseEmitter",  TypeParticleEmitterDataPtr, Offset(baseEmitter,  ProjectileData));
   addField("delayEmitter", TypeParticleEmitterDataPtr, Offset(delayEmitter, ProjectileData));
   addField("bubbleEmitter", TypeParticleEmitterDataPtr, Offset(bubbleEmitter, ProjectileData));
   addField("explosion",    TypeExplosionDataPtr,       Offset(explosion,    ProjectileData));
   addField("underwaterExplosion",  TypeExplosionDataPtr, Offset(underwaterExplosion, ProjectileData));
   addField("splash",       TypeSplashDataPtr,          Offset(splash,       ProjectileData));
   addField("sound",        TypeAudioProfilePtr,        Offset(sound,        ProjectileData));
   addField("wetFireSound", TypeAudioProfilePtr,        Offset(wetFireSound, ProjectileData));
   addField("fireSound",    TypeAudioProfilePtr,        Offset(fireSound,    ProjectileData));

   addField("hasLight",     TypeBool,   Offset(hasLight,    ProjectileData));
   addField("lightRadius",  TypeF32,    Offset(lightRadius, ProjectileData));
   addField("lightColor",   TypeColorF, Offset(lightColor,  ProjectileData));

   addField("hasLightUnderwaterColor", TypeBool,   Offset(hasLightUnderwaterColor, ProjectileData));
   addField("underWaterLightColor",    TypeColorF, Offset(underWaterLightColor,    ProjectileData));

   addField("explodeOnWaterImpact", TypeBool, Offset(explodeOnWaterImpact, ProjectileData));
   addField("depthTolerance",       TypeF32,  Offset(depthTolerance,       ProjectileData));

   addField("decalData",    TypeDecalDataPtr, Offset(decalData,      ProjectileData), 6);
   addField("bubbleEmitTime",  TypeF32,          Offset(bubbleEmitTime, ProjectileData));
   addField("faceViewer",  TypeBool,            Offset(faceViewer,   ProjectileData));
   addField("scale",       TypePoint3F,         Offset(scale,        ProjectileData));

}


//--------------------------------------------------------------------------
bool ProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (!baseEmitter && baseEmitterId != 0)
      if (Sim::findObject(baseEmitterId, baseEmitter) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(baseEmitter): %d", baseEmitterId);
   if (!delayEmitter && delayEmitterId != 0)
      if (Sim::findObject(delayEmitterId, delayEmitter) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(delayEmitter): %d", delayEmitterId);
   if (!bubbleEmitter && bubbleEmitterId != 0)
      if (Sim::findObject(bubbleEmitterId, bubbleEmitter) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(bubbleEmitter): %d", bubbleEmitterId);
   if (!explosion && explosionId != 0)
      if (Sim::findObject(explosionId, explosion) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(explosion): %d", explosionId);
   if (!underwaterExplosion && underwaterExplosionId != 0)
      if (Sim::findObject(underwaterExplosionId, underwaterExplosion) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(underwaterExplosion): %d", underwaterExplosionId);
   if (!splash && splashId != 0)
      if (Sim::findObject(splashId, splash) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(splash): %d", splashId);
   if (!sound && soundId != 0)
      if (Sim::findObject(soundId, sound) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(sound): %d", soundId);
   if (!wetFireSound && wetFireSoundId != 0)
      if (Sim::findObject(wetFireSoundId, wetFireSound) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(wetFireSound): %d", wetFireSoundId);
   if (!fireSound && fireSoundId != 0)
      if (Sim::findObject(fireSoundId, fireSound) == false)
         Con::errorf(ConsoleLogEntry::General, "ProjectileData::onAdd: Invalid packet, bad datablockId(fireSound): %d", fireSoundId);

   if (emitterDelay < 0)
      emitterDelay = -1;
   if (directDamage < 0.0) {
      Con::warnf(ConsoleLogEntry::General, "ProjectileData(%s)::onAdd: directDamage < 0.0", getName());
      directDamage = 0.0;
   }
   if (damageRadius < 0.0) {
      Con::warnf(ConsoleLogEntry::General, "ProjectileData(%s)::onAdd: damageRadius < 0.0", getName());
      damageRadius = 0.0;
   }
   if (indirectDamage < 0.0) {
      Con::warnf(ConsoleLogEntry::General, "ProjectileData(%s)::onAdd: indirectDamage < 0.0", getName());
      indirectDamage = 0.0;
   }
   if ((damageRadius == 0.0 || indirectDamage == 0.0) && hasDamageRadius == true) {
      Con::warnf(ConsoleLogEntry::General, "ProjectileData(%s)::onAdd: (damageRadius || indirectDamage) == 0 && hasDamageRadius == true", getName());
      hasDamageRadius = false;
   }
   if (velInheritFactor < 0.0f || velInheritFactor > 1.0f) {
      Con::warnf(ConsoleLogEntry::General, "ProjectileData(%s)::onAdd: velInheritFactor out of range", getName());
      velInheritFactor = velInheritFactor < 0.0f ? 0.0f : 1.0f;
   }
   if (kickBackStrength < 0.0f) {
      Con::warnf(ConsoleLogEntry::General, "ProjectileData(%s)::onAdd: kickBackStrength must be >= 0", getName());
      kickBackStrength = 0.0f;
   }

   if (hasLight == true) {
      if (lightRadius < 1 || lightRadius > 20) {
         Con::warnf(ConsoleLogEntry::General, "ProjectileData(%s)::onAdd: lightRadius must be in range [1, 20]", getName());
         lightRadius = lightRadius < 1 ? 1.0 : 20.0;
      }
   }

   lightColor.clamp();
   underWaterLightColor.clamp();

   return true;
}


bool ProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (projectileShapeName && projectileShapeName[0] != '\0') {
      char fullName[256];
      dSprintf(fullName, sizeof(fullName), "shapes/%s", projectileShapeName);

      projectileShape = ResourceManager->load(fullName);
      if (bool(projectileShape) == false) {
         dSprintf(errorBuffer, sizeof(errorBuffer), "ProjectileData::load: Couldn't load shape \"%s\"", projectileShapeName);
         return false;
      }

      ambientSeq  = projectileShape->findSequence("ambient");
   }
   
   if (bool(projectileShape)) {
      TSShapeInstance* pDummy = new TSShapeInstance(projectileShape, !server);
      delete pDummy;
   }

   for (U32 i = 0; i < 6; i++) {
      if( !decalData && decalID != 0 )
      {
         if( !Sim::findObject( decalID[i], decalData[i] ) )
         {
            Con::errorf( ConsoleLogEntry::General, "ProjectileData::preload Invalid packet, bad datablockId(decalData): 0x%x", decalID[i]);
         }

      }
      if (!server && decalData[i])
         numDecals++;
   }
   if (!server) {
      // DMM: order the decals so the non-null ones are in the front...
   }

   return true;
}


bool ProjectileData::calculateAim(const Point3F&,
                                  const Point3F&,
                                  const Point3F&,
                                  const Point3F&,
                                  Point3F*, F32*,
                                  Point3F*, F32*)
{
   //AssertFatal(false, "Projectile::calculateAim: this function is (essentially) pure virtual.  Should never be called");
   Con::warnf(ConsoleLogEntry::General, "Projectile::calculateAim: this function is (essentially) pure virtual.  Should never be called");
   return false;
}


//--------------------------------------------------------------------------
void ProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeString(projectileShapeName);
   stream->write(emitterDelay);
   stream->write(bubbleEmitTime);
   stream->writeFlag(faceViewer);
   if(stream->writeFlag(scale.x != 1 || scale.y != 1 || scale.z != 1))
   {
      stream->write(scale.x);
      stream->write(scale.y);
      stream->write(scale.z);
   }

   if (stream->writeFlag(baseEmitter != NULL))
      stream->writeRangedU32(baseEmitter->getId(), DataBlockObjectIdFirst,
                                                   DataBlockObjectIdLast);
   if (stream->writeFlag(delayEmitter != NULL))
      stream->writeRangedU32(delayEmitter->getId(), DataBlockObjectIdFirst,
                                                    DataBlockObjectIdLast);
   if (stream->writeFlag(bubbleEmitter != NULL))
      stream->writeRangedU32(bubbleEmitter->getId(), DataBlockObjectIdFirst,
                                                    DataBlockObjectIdLast);
   if (stream->writeFlag(explosion != NULL))
      stream->writeRangedU32(explosion->getId(), DataBlockObjectIdFirst,
                                                 DataBlockObjectIdLast);

   if (stream->writeFlag(underwaterExplosion != NULL))
      stream->writeRangedU32(underwaterExplosion->getId(), DataBlockObjectIdFirst,
                                                 DataBlockObjectIdLast);

   if (stream->writeFlag(splash != NULL))
      stream->writeRangedU32(splash->getId(), DataBlockObjectIdFirst,
                                              DataBlockObjectIdLast);
   if (stream->writeFlag(sound != NULL))
      stream->writeRangedU32(sound->getId(), DataBlockObjectIdFirst,
                                             DataBlockObjectIdLast);
   if (stream->writeFlag(wetFireSound != NULL))
      stream->writeRangedU32(wetFireSound->getId(), DataBlockObjectIdFirst,
                                                    DataBlockObjectIdLast);
   if (stream->writeFlag(fireSound != NULL))
      stream->writeRangedU32(fireSound->getId(), DataBlockObjectIdFirst,
                                                 DataBlockObjectIdLast);
   for (U32 i = 0; i < 6; i++) {
      if (stream->writeFlag(decalData[i] != NULL))
         stream->writeRangedU32(decalData[i]->getId(), DataBlockObjectIdFirst,
                                                       DataBlockObjectIdLast);
   }

   if(stream->writeFlag(hasLight))
   {
      stream->writeFloat(lightRadius/20.0, 8);
      stream->writeFloat(lightColor.red,7);
      stream->writeFloat(lightColor.green,7);
      stream->writeFloat(lightColor.blue,7);
   }
   if(stream->writeFlag(hasLightUnderwaterColor))
   {
      stream->writeFloat(underWaterLightColor.red,7);
      stream->writeFloat(underWaterLightColor.green,7);
      stream->writeFloat(underWaterLightColor.blue,7);
   }
   stream->write(explodeOnWaterImpact);
   stream->write(depthTolerance);
}

void ProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   projectileShapeName = stream->readSTString();
   stream->read(&emitterDelay);

   stream->read(&bubbleEmitTime);
   faceViewer = stream->readFlag();
   if(stream->readFlag())
   {
      stream->read(&scale.x);
      stream->read(&scale.y);
      stream->read(&scale.z);
   }
   else
      scale.set(1,1,1);

   if (stream->readFlag())
      baseEmitterId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      baseEmitterId = 0;
   
   if (stream->readFlag())
      delayEmitterId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      delayEmitterId = 0;
   
   if (stream->readFlag())
      bubbleEmitterId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      bubbleEmitterId = 0;
   
   if (stream->readFlag())
      explosionId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      explosionId = 0;

   if (stream->readFlag())
      underwaterExplosionId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      underwaterExplosionId = 0;
      
   if (stream->readFlag())
      splashId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      splashId = 0;
   
   if (stream->readFlag())
      soundId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      soundId = 0;
   if (stream->readFlag())
      wetFireSoundId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      wetFireSoundId = 0;
   if (stream->readFlag())
      fireSoundId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      fireSoundId = 0;

   for (U32 i = 0; i < 6; i++) {
      if (stream->readFlag()) {
         decalID[i] = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
      }
   }

   hasLight = stream->readFlag();
   if(hasLight)
   {
      lightRadius = stream->readFloat(8) * 20;
      lightColor.red = stream->readFloat(7);
      lightColor.green = stream->readFloat(7);
      lightColor.blue = stream->readFloat(7);
   }
   hasLightUnderwaterColor = stream->readFlag();
   if(hasLightUnderwaterColor)
   {
      underWaterLightColor.red   = stream->readFloat(7);
      underWaterLightColor.green = stream->readFloat(7);
      underWaterLightColor.blue  = stream->readFloat(7);
   }
   stream->read(&explodeOnWaterImpact);
   stream->read(&depthTolerance);
}


//--------------------------------------------------------------------------
//--------------------------------------
//
Projectile::Projectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);
   mTypeMask |= ProjectileObjectType;

   mInitialPosition.set(0, 0, 0);
   mInitialDirection.set(0, 0, 1);
   mSourceObjectId     = -1;
   mVehicleObjectId    = -1;
   mSourceObjectSlot = -1;

   mCurrTick         = 0;

   mProjectileShape  = NULL;
   mAmbientThread    = NULL;

   mHidden           = false;
   mFadeValue        = 1.0;
   mPlayedSplash     = false;

   mBaseEmitter      = NULL;
   mDelayEmitter     = NULL;
   mBubbleEmitter    = NULL;

   mBubbleEmitterTime = U32( -1 );

   mUseUnderwaterLight = false;
   
   mProjectileSound  = NULL_AUDIOHANDLE;
}

Projectile::~Projectile()
{
   delete mProjectileShape;
   mProjectileShape = NULL;
}

//--------------------------------------------------------------------------
void Projectile::initPersistFields()
{
   Parent::initPersistFields();

   addField("initialPosition",  TypePoint3F, Offset(mInitialPosition, Projectile));
   addField("initialDirection", TypePoint3F, Offset(mInitialDirection, Projectile));
   addField("sourceObject",     TypeS32,     Offset(mSourceObjectId, Projectile));
   addField("vehicleObject",   TypeS32,     Offset(mVehicleObjectId, Projectile));
   addField("sourceSlot",       TypeS32,     Offset(mSourceObjectSlot, Projectile));
}

void Projectile::consoleInit()
{
   //
}


bool Projectile::calculateImpact(float,
                                 Point3F& pointOfImpact,
                                 float&   impactTime)
{
   Con::warnf(ConsoleLogEntry::General, "Projectile::calculateImpact: this function is (essentially) pure virtual.  Should never be called");
   
   impactTime = 0;
   pointOfImpact.set(0, 0, 0);
   return false;
}


//--------------------------------------------------------------------------
F32 Projectile::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
   F32 ret = Parent::getUpdatePriority(camInfo, updateMask, updateSkips);
   // if the camera "owns" this object, it should have a slightly higher priority
   if(mSourceObject == camInfo->camera || mVehicleObject == camInfo->camera)
      return ret + 0.2;
   return ret; 
}

bool Projectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (isServerObject()) {
      ShapeBase* ptr;
      if (Sim::findObject(mSourceObjectId, ptr)) {
         mSourceObject = ptr;
      } else {
         if (mSourceObjectId != -1)
            Con::errorf(ConsoleLogEntry::General, "Projectile::onAdd: mSourceObjectId is invalid");
         mSourceObject = NULL;
      }

      mVehicleObject = (Sim::findObject(mVehicleObjectId, ptr)) ? ptr : NULL;

      // If we're on the server, we need to inherit some of our parent's velocity
      //
      Point3F sourceObjectVel(0, 0, 0);
      if (bool(mSourceObject))
      {
         Point3F velocity = mSourceObject->getVelocity();
         if(bool(mVehicleObject))
            velocity = mVehicleObject->getVelocity();
         
         sourceObjectVel = velocity * mDataBlock->velInheritFactor;
      }
      F32 len    = sourceObjectVel.len();
      mExcessVel = U32(len + 0.5f);
      if (mExcessVel != 0)
         mExcessDir = sourceObjectVel / len;
      else
         mExcessDir.set(0, 0, 1);
      mExcessDirDumb = BitStream::dumbDownNormal(mExcessDir, ExcessVelDirBits);

      mCurrTick = 0;
   } else {
      if (bool(mDataBlock->projectileShape)) {
         mProjectileShape = new TSShapeInstance(mDataBlock->projectileShape, isClientObject());

         if (mDataBlock->ambientSeq != -1) {
            mAmbientThread = mProjectileShape->addThread();
            mProjectileShape->setTimeScale(mAmbientThread, 1);
            mProjectileShape->setSequence(mAmbientThread, mDataBlock->ambientSeq, 0);
         }
      }

      if (mDataBlock->baseEmitter != NULL) {
         ParticleEmitter* pEmitter = new ParticleEmitter;
         pEmitter->onNewDataBlock(mDataBlock->baseEmitter);
         if (pEmitter->registerObject() == false) {
            Con::warnf(ConsoleLogEntry::General, "Could not register base emitter for particle of class: %s", mDataBlock->getName());
            delete pEmitter;
            pEmitter = NULL;
         }
         mBaseEmitter = pEmitter;
      }
      if (mDataBlock->delayEmitter != NULL) {
         ParticleEmitter* pEmitter = new ParticleEmitter;
         pEmitter->onNewDataBlock(mDataBlock->delayEmitter);
         if (pEmitter->registerObject() == false) {
            Con::warnf(ConsoleLogEntry::General, "Could not register delay emitter for particle of class: %s", mDataBlock->getName());
            delete pEmitter;
            pEmitter = NULL;
         }
         mDelayEmitter = pEmitter;
      }
      if (mDataBlock->bubbleEmitter != NULL) {
         ParticleEmitter* pEmitter = new ParticleEmitter;
         pEmitter->onNewDataBlock(mDataBlock->bubbleEmitter);
         if (pEmitter->registerObject() == false) {
            Con::warnf(ConsoleLogEntry::General, "Could not register delay emitter for particle of class: %s", mDataBlock->getName());
            delete pEmitter;
            pEmitter = NULL;
         }
         mBubbleEmitter = pEmitter;
      }

      if (mDataBlock->hasLight == true)
         Sim::getLightSet()->addObject(this);

      mBubbleEmitterTime = mDataBlock->bubbleEmitTime;
   }
   mSourceIdTimeoutTicks = SourceIdTimeoutTicks;

   if (bool(mSourceObject))
      processAfter(mSourceObject);


   // Setup our bounding box
   if (bool(mDataBlock->projectileShape) == true)
      mObjBox = mDataBlock->projectileShape->bounds;
   else
      mObjBox = Box3F(Point3F(0, 0, 0), Point3F(0, 0, 0));
   resetWorldBox();

   mLastPos = getPosition();

   return true;
}


void Projectile::onRemove()
{
   if (bool(mBaseEmitter)) {
      mBaseEmitter->deleteWhenEmpty();
      mBaseEmitter = NULL;
   }
   if (bool(mDelayEmitter)) {
      mDelayEmitter->deleteWhenEmpty();
      mDelayEmitter = NULL;
   }
   if (bool(mBubbleEmitter)) {
      mBubbleEmitter->deleteWhenEmpty();
      mBubbleEmitter = NULL;
   }

   Parent::onRemove();
}


bool Projectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<ProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   return true;
}


//--------------------------------------------------------------------------
void Projectile::registerLights(LightManager * lightManager, bool lightingScene)
{
   if(lightingScene)
      return;
      
   if (mDataBlock->hasLight && mHidden == false) {
      mLight.mType = LightInfo::Point;
      getRenderTransform().getColumn(3, &mLight.mPos);
      mLight.mRadius = mDataBlock->lightRadius;

      if (mDataBlock->hasLightUnderwaterColor && mUseUnderwaterLight)
      {
         mLight.mColor  = mDataBlock->underWaterLightColor;
      }
      else
      {
         mLight.mColor  = mDataBlock->lightColor;
      }
      lightManager->addLight(&mLight);
   }
}

//----------------------------------------------------------------------------
void Projectile::processTick(const Move* move)
{
   Parent::processTick(move);

   mCurrTick++;
   if (mSourceIdTimeoutTicks)
      mSourceIdTimeoutTicks--;
}


void Projectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   mLastPos = getRenderPosition();

   if (mAmbientThread)
      mProjectileShape->advanceTime(dt, mAmbientThread);
}


void Projectile::emitParticles(const Point3F& from, const Point3F& to, const Point3F& vel, const U32 ms)
{
   if( mHidden ) return;

   Point3F axis = -vel;

   if( axis.isZero() )
   {
      axis.set( 0.0, 0.0, 1.0 );
   }
   else
   {
      axis.normalize();
   }

   if (bool(mBaseEmitter)) {
      mBaseEmitter->emitParticles(from, to,
                                  axis, vel,
                                  ms);
   }
   if (bool(mDelayEmitter) && mCurrTick > (mDataBlock->emitterDelay / TickMs)) {
      mDelayEmitter->emitParticles(from, to,
                                   axis, vel,
                                   ms);
   }
}


void Projectile::updateSound(const Point3F& pos, const Point3F& vel, const bool active)
{
   AssertFatal(isClientObject(), "Error, server projectiles play no sounds!");
   if (mDataBlock->sound == NULL)
      return;

   MatrixF xForm(true);
   xForm.setColumn(3, pos);

   if (active == true && mProjectileSound == NULL_AUDIOHANDLE) {
      mProjectileSound = alxPlay(mDataBlock->sound, &xForm, &vel);
      return;
   } else if (active == false && mProjectileSound != NULL_AUDIOHANDLE) {
      alxStop(mProjectileSound);
      mProjectileSound = NULL_AUDIOHANDLE;
   }

   if (mProjectileSound != NULL_AUDIOHANDLE) {
      alxSourceMatrixF(mProjectileSound, &xForm);
//      alxSourcePoint3F(mProjectileSound, AL_VELOCITY, &vel);
   }
}


//--------------------------------------------------------------------------
void Projectile::onCollision(const Point3F& hitPosition,
                             const Point3F& hitNormal,
                             SceneObject*   hitObject)
{
   if (!isClientObject() && hitObject != NULL) {
      char *posArg = Con::getArgBuffer(64);
      char *normalArg = Con::getArgBuffer(64);

      dSprintf(posArg, 64, "%f %f %f", hitPosition.x, hitPosition.y, hitPosition.z);
      dSprintf(normalArg, 64, "%f %f %f", hitNormal.x, hitNormal.y, hitNormal.z);

      Con::executef(mDataBlock, 6, "onCollision", 
         Con::getIntArg(getId()), 
         Con::getIntArg(hitObject->getId()),
         Con::getFloatArg(mFadeValue),
         posArg,
         normalArg);
   }
}

//--------------------------------------------------------------------------
void Projectile::prepModelView(SceneState* state)
{
   Point3F targetVector;
   if( mDataBlock->faceViewer )
   {
      targetVector = state->getCameraPosition() - getRenderPosition();
      targetVector.normalize();

      MatrixF explOrient = MathUtils::createOrientFromDir( targetVector );
      explOrient.setPosition( getRenderPosition() );
      dglMultMatrix( &explOrient ); 
   }
   else
   {
      dglMultMatrix( &getRenderTransform() ); 
   }
}

//--------------------------------------------------------------------------
bool Projectile::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   if (mHidden == true || mFadeValue <= (1.0/255.0))
      return false;

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->isTranslucent = true;
      image->sortType = SceneRenderImage::Point;
      state->setImageRefPoint(this, image);

      // For projectiles, the datablock pointer is a good enough sort key, since they aren't
      //  skinned at all...
      image->textureSortKey = U32(mDataBlock);

      state->insertRenderImage(image);
   }

   return false;
}


void Projectile::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   prepModelView( state );

   glScalef( mDataBlock->scale.x, mDataBlock->scale.y, mDataBlock->scale.z );

   AssertFatal(mProjectileShape != NULL,
               "Projectile::renderObject: Error, projectile shape should always be present in renderObject");
   mProjectileShape->selectCurrentDetail();
   mProjectileShape->animate();

   Point3F cameraOffset;
   mObjToWorld.getColumn(3,&cameraOffset);
   cameraOffset -= state->getCameraPosition();
   F32 fogAmount = state->getHazeAndFog(cameraOffset.len(),cameraOffset.z);

   if (mFadeValue == 1.0) {
      mProjectileShape->setupFog(fogAmount, state->getFogColor());
   } else {
      mProjectileShape->setupFog(0.0, state->getFogColor());
      mProjectileShape->setAlphaAlways(mFadeValue * (1.0 - fogAmount));
   }
   mProjectileShape->render();

   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------------------------------------------
bool Projectile::pointInWater( Point3F &point, F32 *waterHeight )
{
   SimpleQueryList sql;
   if (isServerObject())
      gServerSceneGraph->getWaterObjectList(sql);
   else
      gClientSceneGraph->getWaterObjectList(sql);

   for (U32 i = 0; i < sql.mList.size(); i++)
   {
      WaterBlock* pBlock = dynamic_cast<WaterBlock*>(sql.mList[i]);
      if (pBlock && pBlock->isPointSubmergedSimple( point ))
      {
         if(pBlock->isLava(pBlock->getLiquidType()))
            return false;
         
         if( waterHeight )
         {
            *waterHeight = pBlock->getSurfaceHeight();
         }
         return true;
      }
   }

   return false;
}

//--------------------------------------------------------------------------
void Projectile::createSplash( Point3F &pos )
{
   if( mDataBlock->splash && !mPlayedSplash )
   {
      MatrixF trans = getTransform();
      trans.setPosition( pos );
      Splash *splash = new Splash;
      splash->onNewDataBlock( mDataBlock->splash );
      splash->setTransform( trans );
      splash->setInitialState( trans.getPosition(), Point3F( 0.0, 0.0, 1.0 ) );
      if (!splash->registerObject())
         delete splash;

      mPlayedSplash = true;
   }

}
