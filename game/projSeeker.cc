//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projSeeker.h"
#include "console/consoleTypes.h"
#include "core/bitStream.h"
#include "game/shapeBase.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "game/explosion.h"
#include "terrain/terrData.h"
#include "terrain/waterBlock.h"
#include "game/Debris.h"
#include "ts/tsShapeInstance.h"
#include "dgl/dgl.h"
#include "scenegraph/sceneState.h"
#include "math/mathUtils.h"
#include "scenegraph/sceneGraph.h"

IMPLEMENT_CO_DATABLOCK_V1(SeekerProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(SeekerProjectile);

//--------------------------------------------------------------------------
//--------------------------------------
//
SeekerProjectileData::SeekerProjectileData()
{
   lifetimeMS     = 3000;

   muzzleVelocity = 100.0f;
   turningSpeed   = 180.0f;
   maxVelocity    = 65.0;
   acceleration   = 10.0;

   proximityRadius = 10.0f;

   terrainAvoidanceSpeed  = 180;
   terrainScanAhead       = 25;
   terrainHeightFail      = 2;
   terrainAvoidanceRadius = 50;

   flareDistance = 200;
   flareAngle    = 20;

   useFlechette = false;
   flechetteDelayMs = 300;

   puffEmitter = NULL;
   puffEmitterID = 0;

   exhaustEmitter = NULL;
   exhaustEmitterID = 0;
   exhaustTimeMs = 1000;
   exhaustNodeName = NULL;
   
   casingDeb = NULL;
   casingDebID = 0;
   casingShapeName = NULL;
}

SeekerProjectileData::~SeekerProjectileData()
{

}


//--------------------------------------------------------------------------
void SeekerProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("lifetimeMS",             TypeS32,     Offset(lifetimeMS,             SeekerProjectileData));
   addField("muzzleVelocity",         TypeF32,     Offset(muzzleVelocity,         SeekerProjectileData));
   addField("turningSpeed",           TypeF32,     Offset(turningSpeed,           SeekerProjectileData));
   addField("terrainAvoidanceSpeed",  TypeF32,     Offset(terrainAvoidanceSpeed,  SeekerProjectileData));
   addField("terrainScanAhead",       TypeF32,     Offset(terrainScanAhead,       SeekerProjectileData));
   addField("terrainHeightFail",      TypeF32,     Offset(terrainHeightFail,      SeekerProjectileData));
   addField("terrainAvoidanceRadius", TypeF32,     Offset(terrainAvoidanceRadius, SeekerProjectileData));
   addField("flareDistance",          TypeF32,     Offset(flareDistance,          SeekerProjectileData));
   addField("flareAngle",             TypeF32,     Offset(flareAngle,             SeekerProjectileData));
   addField("useFlechette",           TypeBool,    Offset(useFlechette,           SeekerProjectileData));
   addField("maxVelocity",            TypeF32,     Offset(maxVelocity,            SeekerProjectileData));
   addField("acceleration",           TypeF32,     Offset(acceleration,           SeekerProjectileData));
   addField("flechetteDelayMs",       TypeS32,     Offset(flechetteDelayMs,       SeekerProjectileData));
   addField("exhaustTimeMs",          TypeS32,     Offset(exhaustTimeMs,          SeekerProjectileData));
   addField("exhaustNodeName",        TypeString,  Offset(exhaustNodeName,        SeekerProjectileData));
   addField("casingShapeName",        TypeString,  Offset(casingShapeName,        SeekerProjectileData));

   addField("puffEmitter",            TypeParticleEmitterDataPtr, Offset(puffEmitter,     SeekerProjectileData));
   addField("exhaustEmitter",         TypeParticleEmitterDataPtr, Offset(exhaustEmitter,  SeekerProjectileData));
   addField("casingDeb",              TypeDebrisDataPtr,          Offset(casingDeb,       SeekerProjectileData));

}


//---------------------------------------------------
bool SeekerProjectileData::calculateAim(const Point3F& targetPos,
                                        const Point3F& targetVel,
                                        const Point3F& sourcePos,
                                        const Point3F& /*sourceVel*/,
                                        Point3F*       outputVectorMin,
                                        F32*           outputMinTime,
                                        Point3F*       outputVectorMax,
                                        F32*           outputMaxTime)
{
   if (((targetPos - sourcePos) + targetVel * (lifetimeMS/1000.0)).len() > (lifetimeMS/1000.0)*maxVelocity) {
      outputVectorMin->set(0, 0, 1);
      outputVectorMax->set(0, 0, 1);
      *outputMinTime = -1;
      *outputMaxTime = -1;

      return false;
   }

   *outputVectorMin = targetPos - sourcePos;
   *outputMinTime   = outputVectorMin->len() / maxVelocity;
   outputVectorMin->normalize();
   *outputVectorMax = *outputVectorMin;
   *outputMaxTime   = *outputMinTime;

   return true;
}


//--------------------------------------------------------------------------
bool SeekerProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (lifetimeMS < 500 || lifetimeMS > 30000) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: lifetimeMS out of bounds [500, 30000]", getName());
      lifetimeMS = lifetimeMS < 500 ? 500 : 30000;
   }
   if (muzzleVelocity < 0.1) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: muzzleVelocity < 0.1", getName());
      muzzleVelocity = 0.1;
   }
   if (turningSpeed < 0.0f || turningSpeed > 2000) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: turningSpeed out of range [0, 2000]", getName());
      turningSpeed = turningSpeed < 0.0 ? 0.0 : 2000.0;
   }
   if (terrainAvoidanceSpeed < 0 || terrainAvoidanceSpeed > 2000) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: terrainAvoidanceSpeed out of range [0, 2000]", getName());
      terrainAvoidanceSpeed = terrainAvoidanceSpeed < 0.0 ? 0.0 : 2000.0;
   }
   if (terrainScanAhead < 0 || terrainScanAhead > 200) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: terrainScanAhead out of range [0, 200]", getName());
      terrainScanAhead = terrainScanAhead < 0.0 ? 0.0 : 200.0;
   }
   if (terrainHeightFail < 0) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: terrainHeightFail < 0", getName());
      terrainHeightFail = 0.0;
   }
   if (terrainAvoidanceRadius < 0) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: terrainAvoidanceRadius < 0", getName());
      terrainAvoidanceRadius = 0.0;
   }
   if (flareDistance < 0) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: flareDistance must be >= 0.  (0 Indicates missile unaffected by flares", getName());
      flareDistance = 0;
   }
   if (flareAngle < 0) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: flareAngle must be >= 0.  (0 Indicates missile unaffected by flares", getName());
      flareAngle = 0;
   }
   if (maxVelocity < 0.1 ) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: maxVelocity < 0.1", getName());
      maxVelocity = muzzleVelocity + 1.0;
   }
   if (flechetteDelayMs > 30000) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: flechetteDelayMs > 30000", getName());
      flechetteDelayMs = 500;
   }
   if (acceleration < 0.0 || acceleration > 30000 ) {
      Con::warnf(ConsoleLogEntry::General, "SeekerProjectileData(%s)::onAdd: acceleration out of range [0.0, 30000]", getName());
      acceleration = 0.0;
   }

   if (!puffEmitter && puffEmitterID != 0)
      if (Sim::findObject(puffEmitterID, puffEmitter) == false)
         Con::errorf(ConsoleLogEntry::General, "SeekerProjectileData::onAdd: Invalid packet, bad datablockId(puffEmitter): %d", puffEmitterID);

   if (!exhaustEmitter && exhaustEmitterID != 0)
      if (Sim::findObject(exhaustEmitterID, exhaustEmitter) == false)
         Con::errorf(ConsoleLogEntry::General, "SeekerProjectileData::onAdd: Invalid packet, bad datablockId(exhaustEmitter): %d", exhaustEmitterID);


   if( !casingDeb && casingDebID != 0 )
   {
      if( !Sim::findObject( SimObjectId( casingDebID ), casingDeb ) )
      {
         Con::errorf( ConsoleLogEntry::General, "SeekerProjectileData::onAdd: Invalid packet, bad datablockId(casingDeb): 0x%x", casingDebID );
      }
   }

   if (casingShapeName && casingShapeName[0] != '\0') {
      char fullName[256];
      char errorBuffer[256];
      dSprintf(fullName, sizeof(fullName), "shapes/%s", casingShapeName);

      casingShape = ResourceManager->load(fullName);
      if (bool(casingShape) == false) {
         dSprintf(errorBuffer, sizeof(errorBuffer), "SeekerProjectileData::onAdd: Couldn't load shape \"%s\"", casingShapeName);
         return false;
      }
   }


   // Make sure the lifetime is a multiple of TickMs;
   lifetimeMS = (lifetimeMS + TickMs - 1) & ~(TickMs - 1);

   F32 turnPerTick = turningSpeed * (TickMs / 1000.0f);
   AssertFatal(turnPerTick < 90.0f, "Error, bad boundary constraints on SeekerProjectileData::turningSpeed");
   cosTurningSpeed = mCos(mDegToRad(turnPerTick));
   cosAvoidSpeed   = mCos(mDegToRad(terrainAvoidanceSpeed * (TickMs / 1000.0f)));
   cosFlareAngle   = mCos(mDegToRad(flareAngle));

   return true;
}

//--------------------------------------------------------------------------
void SeekerProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(lifetimeMS);
   stream->write(muzzleVelocity);
   stream->write(turningSpeed);
   stream->write(proximityRadius);
   stream->write(terrainAvoidanceSpeed);
   stream->write(terrainScanAhead);
   stream->write(terrainHeightFail);
   stream->write(terrainAvoidanceRadius);
   stream->write(flareDistance);
   stream->write(flareAngle);
   stream->write(useFlechette);
   stream->write(maxVelocity);
   stream->write(acceleration);
   stream->write(flechetteDelayMs);
   stream->write(exhaustTimeMs);

   stream->writeString( exhaustNodeName );
   stream->writeString( casingShapeName );

   if( stream->writeFlag( casingDeb ) )
   {
      stream->writeRangedU32(packed? SimObjectId(casingDeb):
                             casingDeb->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);
   }

   if (stream->writeFlag(puffEmitter != NULL))
      stream->writeRangedU32(puffEmitter->getId(), DataBlockObjectIdFirst,
                             DataBlockObjectIdLast);


   if (stream->writeFlag(exhaustEmitter != NULL))
      stream->writeRangedU32(exhaustEmitter->getId(), DataBlockObjectIdFirst,
                             DataBlockObjectIdLast);
}

void SeekerProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&lifetimeMS);
   stream->read(&muzzleVelocity);
   stream->read(&turningSpeed);
   stream->read(&proximityRadius);
   stream->read(&terrainAvoidanceSpeed);
   stream->read(&terrainScanAhead);
   stream->read(&terrainHeightFail);
   stream->read(&terrainAvoidanceRadius);
   stream->read(&flareDistance);
   stream->read(&flareAngle);
   stream->read(&useFlechette);
   stream->read(&maxVelocity);
   stream->read(&acceleration);
   stream->read(&flechetteDelayMs);
   stream->read(&exhaustTimeMs);

   exhaustNodeName = stream->readSTString();
   casingShapeName = stream->readSTString();

   if(stream->readFlag())
   {
      casingDebID = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   }

   if (stream->readFlag())
      puffEmitterID = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      puffEmitterID = 0;


   if (stream->readFlag())
      exhaustEmitterID = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      exhaustEmitterID = 0;
}


//--------------------------------------------------------------------------
//--------------------------------------
//
SeekerProjectile::SeekerProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);

   mTargetId = 0;
   mTargetPosition.set(0, 0, 0);
   mTargetingString = "none";
   mTargetingMode = None;
   
   mNumWarpTicks = 0;

   mPlayerVel.set( 0.0, 0.0, 0.0 );

   mExhaustEmitter = NULL;
   mPuffEmitter = NULL;
   mFlechettePuff = false;

   mCurrVelocity.set( 0.0, 0.0, 0.0 );
   mLastServerVel = mCurrVelocity;

   mCasingShape = NULL;
   mLastPos.set( 0.0, 0.0, 0.0 );
   
   mOriginalTargetPtr = NULL;
   mTargetPtr = NULL;

   mExplosionPending = false;
   mHitWater = false;

   mFlechetteGhostTick = 0xFFFFFFFF;
}

SeekerProjectile::~SeekerProjectile()
{
   delete mCasingShape;
   mCasingShape = NULL;
}

//--------------------------------------------------------------------------
static S32 cGetTargetObject(SimObject *obj, S32, const char **)
{
   SeekerProjectile *proj = static_cast<SeekerProjectile*>(obj);
   return proj->getTargetObjectId();
}   

ConsoleMethod(SeekerProjectile,setObjectTarget,void,3,3,"obj.setObjectTarget(targetObj);")
{
   argc;
   SeekerProjectile *proj = static_cast<SeekerProjectile*>(object);
   GameBase *pTarget;
   if(!Sim::findObject(argv[2], pTarget))
      return;
   proj->mTargetId = pTarget->getId();
   proj->mOriginalTargetPtr = proj->mTargetPtr = pTarget;
   proj->mTargetingMode = SeekerProjectile::Object;

   pTarget->incHomingCount();
}

ConsoleMethod(SeekerProjectile,setNoTarget,void,2,2,"obj.setNoTarget();")
{
   argc;
   SeekerProjectile *proj = static_cast<SeekerProjectile*>(object);
   
   proj->mOriginalTargetPtr = proj->mTargetPtr = NULL;
   proj->mTargetingMode = SeekerProjectile::None;
}

ConsoleMethod(SeekerProjectile,setPositionTarget,void,3,3,"obj.setPositionTarget(targetPos);")
{
   argc;
   SeekerProjectile *proj = static_cast<SeekerProjectile*>(object);
   Point3F pos;
   dSscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   
   proj->mOriginalTargetPtr = proj->mTargetPtr = NULL;
   proj->mTargetPosition = pos;
   proj->mTargetingMode = SeekerProjectile::Position;
}

void SeekerProjectile::consoleInit()
{
   Con::addCommand("SeekerProjectile", "getTargetObject", cGetTargetObject, "proj.getTargetObject()", 2, 2);
}

//--------------------------------------------------------------------------
bool SeekerProjectile::calculateImpact(float    simTime,
                                       Point3F& pointOfImpact,
                                       float&   impactTime)
{
   if (mHidden == true) {
      impactTime = 0;
      pointOfImpact.set(0, 0, 0);
      return false;
   }

   Point3F startPt;
   getTransform().getColumn(3, &startPt);

   if (mTargetingMode == Object && (bool)mTargetPtr) {
      mTargetPtr->getWorldBox().getCenter(&pointOfImpact);
      impactTime = (pointOfImpact - startPt).len() / mDataBlock->muzzleVelocity;

      return (impactTime <= simTime);
   } else if (mTargetingMode == Position) {
      AssertFatal(false, "not yet allowed!");
      return false;
   } else {
      // Ray cast forward just like a linear projectile...
      //
      Point3F endPt = startPt + mCurrVelocity * simTime;

      U32 mask = PlayerObjectType | TerrainObjectType | InteriorObjectType | WaterObjectType;
      RayInfo rayInfo;
      if (!gServerContainer.castRay(startPt, endPt, mask, &rayInfo)) {
         impactTime = 0;
         pointOfImpact.set(0, 0, 0);
         return false;
      }
      else {
         pointOfImpact = rayInfo.point;
         float distToImpact = (pointOfImpact - startPt).len();
         impactTime = distToImpact / mDataBlock->muzzleVelocity;
         return true;
      }
   }
}


//--------------------------------------------------------------------------
bool SeekerProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (isServerObject()) {
      bool wetStart = false;

      mCurrPosition  = mInitialPosition;
      mCurrVelocity  = mInitialDirection * mDataBlock->muzzleVelocity;

      Point3F excessVel = mExcessDir * mExcessVel;
      Point3F eDir = mExcessDir;
      mExcessVel = F32(mExcessVel) * mFabs( mDot( eDir, mInitialDirection ) );
      mCurrVelocity += mExcessDir * mExcessVel;

      mDeathTick  = mDataBlock->lifetimeMS / TickMs;
      mDeleteTick = mDeathTick + DeleteWaitTicks;
   } else {
      if (bool(mSourceObject)) {
         // If we have a source object, we have to warp to the server position...
         //
         mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mCurrPosition);
         mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mInitialDirection);
         setupWarp();
      } else {
         // Otherwise, we can just start the sim...
         mCurrPosition  = mLastServerPos;
         mCurrVelocity  = mLastServerVel;
         mWarpTicksLeft = 0;
      }

      if( mDataBlock->puffEmitter != NULL )
      {
         ParticleEmitter* pEmitter = new ParticleEmitter;
         pEmitter->onNewDataBlock( mDataBlock->puffEmitter );
         if( pEmitter->registerObject() == false )
         {
            Con::warnf(ConsoleLogEntry::General, "Could not register base emitter for particle of class: %s", mDataBlock->getName());
            delete pEmitter;
            pEmitter = NULL;
         }
         mPuffEmitter = pEmitter;
      }
      if( mDataBlock->exhaustEmitter != NULL )
      {
         ParticleEmitter* pEmitter = new ParticleEmitter;
         pEmitter->onNewDataBlock( mDataBlock->exhaustEmitter );
         if( pEmitter->registerObject() == false )
         {
            Con::warnf(ConsoleLogEntry::General, "Could not register base emitter for particle of class: %s", mDataBlock->getName());
            delete pEmitter;
            pEmitter = NULL;
         }
         mExhaustEmitter = pEmitter;
      }

      if( bool(mDataBlock->casingShape) )
      {
         mCasingShape = new TSShapeInstance(mDataBlock->casingShape, isClientObject());
      }

      mLastPos = mCurrPosition;
   }

   mObjBox = Box3F(Point3F(-0.25, -0.25, -0.25), Point3F(0.25, 0.25, 0.25));
   MatrixF xform(true);
   xform.setColumn(3, mCurrPosition);
   setTransform(xform);
   resetWorldBox();

   addToScene();

   return true;
}


void SeekerProjectile::onRemove()
{

   if (bool(mPuffEmitter)) {
      mPuffEmitter->deleteWhenEmpty();
      mPuffEmitter = NULL;
   }

   if (bool(mExhaustEmitter)) {
      mExhaustEmitter->deleteWhenEmpty();
      mExhaustEmitter = NULL;
   }

   if (isClientObject())
      updateSound(Point3F(0, 0, 0), Point3F(0, 0, 0), false);
   removeFromScene();

   Parent::onRemove();
}


bool SeekerProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<SeekerProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
bool SeekerProjectile::getTargetPosition(Point3F* target)
{
   if (mTargetingMode == None)
      return false;

   if (mTargetingMode == Position) {
      *target = mTargetPosition;
      return true;
   }
   
   // Otherwise, we're targeting an object...
   if (bool(mTargetPtr) == false)  {
      if(bool(mOriginalTargetPtr) == false)
         return false;
      else {
         mTargetPtr = mOriginalTargetPtr;
         mTargetPtr->incHomingCount();
      }
   }
   // Ok, we need to do a quick scan for any flares in our path.  We define
   //  this as flares that are within flareDistance, and within flareAngle
   //  degrees of the seeker head.  If a flare meeting these conditions is
   //  detected, we choose the closest one, and seek at it.  We only do this
   //  scan on the server...
   //
   if((GameBase*)mTargetPtr == (GameBase*)mOriginalTargetPtr)
   {
      if (isServerObject() && mDataBlock->flareDistance > 0 && mDataBlock->flareAngle > 0) {
         SimSet* flareSet = Sim::getFlareSet();
         AssertFatal(flareSet, "No Flare set!");

         F32 minDist    = 1e8;
         GameBase* pMin = NULL;

         for (SimSet::iterator itr = flareSet->begin(); itr != flareSet->end(); itr++) {
            GameBase* pFlare = dynamic_cast<GameBase*>(*itr);
            if (pFlare == NULL || pFlare->getHomingCount() != 0)
               continue;

            Point3F boxCenterDif;
            pFlare->getWorldBox().getCenter(&boxCenterDif);
            boxCenterDif -= mCurrPosition;
            F32 dist = boxCenterDif.len();
            if (dist > mDataBlock->flareDistance || dist > minDist)
               continue;

            boxCenterDif.normalize();
            Point3F velNormal = mCurrVelocity;
            velNormal.normalize();

            F32 dot = mDot(boxCenterDif, velNormal);
            if (dot > mDataBlock->cosFlareAngle) {
               minDist = dist;
               pMin    = pFlare;
            }
         }

         // Flared!
         if (pMin != NULL) {
            if (pMin == (GameBase*)mTargetPtr) {
               // Same target, do nothing...
            } else {
               // New flare...
               if (bool(mTargetPtr))
                  mTargetPtr->decHomingCount();

               mTargetPtr = pMin;
               mTargetPtr->incHomingCount();
               setMaskBits(SeekerUpdateMask);
            }
         }
      }
   }
   mTargetPtr->getWorldBox().getCenter(target);
   return true;
}


Point3F SeekerProjectile::getVelocity() const
{
   return mCurrVelocity;
}


S32 SeekerProjectile::getTargetObjectId()
{
   if (bool(mTargetPtr))
      return mTargetPtr->getId();
   else
      return -1;
}
//--------------------------------------------------------------------------
void SeekerProjectile::explode(const Point3F& p, const Point3F& n)
{
   // Already blown up
   if (mHidden == true)
      return;
   mHidden = true;

   if (isServerObject()) {
      // If we're seeking on a flare, delete it when we are finished...
      //
      SimSet* flareSet = Sim::getFlareSet();
      AssertFatal(flareSet, "No Flare set!");
      if (flareSet != NULL && bool(mTargetPtr))
      {
         for (U32 i = 0; i < flareSet->size(); i++)
         {
            GameBase* pGBase = dynamic_cast<GameBase*>((*flareSet)[i]);
            if (pGBase != NULL && mTargetPtr == pGBase)
            {
               pGBase->deleteObject();
            }
         }
      }

      // do radius damage if appropriate
      mExplosionPosition = p + (n*0.01);
      mExplosionNormal   = n;

      char buffer[128];
      dSprintf(buffer, sizeof(buffer),  "%f %f %f",
               mExplosionPosition.x,
               mExplosionPosition.y,
               mExplosionPosition.z);
      Con::executef(mDataBlock, 4, "onExplode", scriptThis(), buffer, "1.0");

      // Todo: event over explosion
   } else {
      // Client just plays the explosion at the right place...
      //
      if (mDataBlock->explosion) {
         Explosion* pExplosion = new Explosion;
         pExplosion->onNewDataBlock(mDataBlock->explosion);

         MatrixF xform(true);
         xform.setColumn(3, p);
         pExplosion->setTransform(xform);
         pExplosion->setInitialState(p, n);
         if (pExplosion->registerObject() == false) {
            Con::errorf(ConsoleLogEntry::General, "LinearProjectile(%s)::explode: couldn't register explosion(%s)",
                        mDataBlock->getName(), mDataBlock->explosion->getName());
            delete pExplosion;
            pExplosion = NULL;
         }
      }

      updateSound(Point3F(0, 0, 0), Point3F(0, 0, 0), false);
   }
}


//----------------------------------------------------------------------------
bool SeekerProjectile::updatePos(const Point3F& oldPos,
                                 const Point3F& newPos,
                                 const bool     includeDynamics,
                                 F32*           hitDelta,
                                 Point3F*       hitPos,
                                 Point3F*       hitNormal,
                                 SceneObject*&  hitObject)
{
   if (bool(mSourceObject))
      mSourceObject->disableCollision();

   RayInfo rInfo;
   U32 mask = csmStaticCollisionMask | (includeDynamics ? csmDynamicCollisionMask : 0);

   if( mDataBlock->explodeOnWaterImpact )
   {
      mask |= WaterObjectType;
   }

   if (mContainer->castRay(oldPos, newPos, mask, &rInfo) == true) {
      *hitPos    = rInfo.point;
      *hitNormal = rInfo.normal;
      *hitDelta  = rInfo.t;
      hitObject  = rInfo.object;


      // shoot another ray to see if it hit water
      RayInfo waterRayInf;
      if( mContainer->castRay( oldPos, newPos, WaterObjectType, &waterRayInf ) )
      {
         mHitWater = true;
      }


      if (bool(mSourceObject))
         mSourceObject->enableCollision();

      return false;
   } else {
      *hitDelta = 1.0;
      hitObject = NULL;

      if (bool(mSourceObject))
         mSourceObject->enableCollision();

      return true;
   }
}


//--------------------------------------------------------------------------
bool SeekerProjectile::getNewVelocity(const Point3F& currPosition,
                                      const Point3F& currVelocity,
                                      const Point3F& targetPosition,
                                      Point3F*       newVelocity)
{
   // This is fairly simple.  We have to determine two things:
   //  1. Have we passed the target? If so, newV = currV, return false
   //  2. Otherwise, turn newV from currV as much as allowed by the
   //      datablock parameters...
   //

   Point3F vecToTarget  = targetPosition - currPosition;
   Point3F normVelocity = currVelocity;

   // Oh, god.  I feel so dirty.
   //
   F32 vecLen = vecToTarget.len();
   F32 normLen = normVelocity.len();
   if (vecLen > 1e-9)
      vecToTarget /= vecLen;
   else
      vecToTarget.set(0, 0, 1);
   if (normLen > 1e-9)
      normVelocity /= normLen;
   else
      normVelocity.set(0, 0, 1);
   //
   /////


   bool status = true;
   F32 dotProd = mDot(vecToTarget, normVelocity);
   if (dotProd < 0.0)
      status = false;

   if (dotProd < 0.99985) {
      // Ok, if we're here, then the above dot product represents the angle between
      //  the current direction and the direction we want to go.  Let's find the axis
      //  to turn along...
      Point3F turnAxis;
      mCross(vecToTarget, normVelocity, &turnAxis);
      turnAxis.normalize();

      // And now, let's determine the final cos(theta) we'll be using.
      F32 cosTheta = dotProd;
      if (dotProd < mDataBlock->cosTurningSpeed)
         cosTheta = mDataBlock->cosTurningSpeed;

      F32 theta = mAcos(cosTheta);
      if (mTargetingMode == Object) 
      {
         F32 heat = mTargetPtr->getHeat();
         if (theta > mDataBlock->turningSpeed * heat)
            theta = mDataBlock->turningSpeed * heat;
      
         if( theta == 0 )
         {
            *newVelocity = currVelocity;
            return status;
         }
      }

      QuatF turn(AngAxisF(turnAxis, theta));

      // Ok, we know have a quaternion that represents our desired rotation.  Lets construct
      //  a transform from it...
      MatrixF rot(true);
      turn.setMatrix(&rot);
      rot.mulV(currVelocity, newVelocity);
   } else {
      // We're already pointed straight at it essentially, skip
      *newVelocity = currVelocity;
   }

   if (mDataBlock->terrainScanAhead <= 0.0 || mContainer == NULL)
      return status;

   U32 mask = mTargetingMode == Object ? TerrainObjectType | mTargetPtr->getType() : TerrainObjectType;

   RayInfo rinfo;
   F32 dist = (currPosition - targetPosition).len();
   if (dist < mDataBlock->terrainAvoidanceRadius) {
      // Make sure we don't engage the terrain following logic unless the terrain obstructs
      //  our view of the target...
      if (mContainer->castRay(currPosition, targetPosition, mask, &rinfo)) {
         // clear shot?
         if (dynamic_cast<TerrainBlock*>(rinfo.object) == NULL)
            return status;
      } else {
         return status;
      }
   }

   // Now that we have the new velocity, let's see if we can avoid crashing into the terrain
   Point3F end = *newVelocity;
   end.normalize();
   end *= mDataBlock->terrainScanAhead;
   end += currPosition - Point3F(0, 0, mDataBlock->terrainHeightFail);

   if (mContainer->castRay(currPosition, end, TerrainObjectType, &rinfo) ||
       mContainer->castRay(currPosition, currPosition - Point3F(0, 0, mDataBlock->terrainHeightFail), TerrainObjectType, &rinfo))
   {
      // Otherwise, we have to raise the velocity to avoid the terrain.  It is assumed that
      //  the missile won't get here if the velocity vector points straight up.
      Point3F stor = *newVelocity;
      stor.normalize();
      F32 dot = mDot(Point3F(0, 0, 1), stor);
      Point3F axis;
      mCross(Point3F(0, 0, 1), stor, &axis);
      axis.normalize();

      if (dot < mDataBlock->cosAvoidSpeed)
         dot = mDataBlock->cosAvoidSpeed;

      F32 theta = mAcos(dot);
      QuatF newturn(AngAxisF(axis, theta));

      MatrixF newrot(true);
      newturn.setMatrix(&newrot);
      stor = *newVelocity;
      newrot.mulV(stor, newVelocity);

      return status;
   }

   // No terrain collision predicted
   return status;
}


//--------------------------------------------------------------------------
void SeekerProjectile::setupWarp()
{
   // This is a little tricky.  Here's the deal.  LastServerPos and LastServerVel
   //  should be simulated forward to find the new position after smProjectileWarpTicks
   //  We have to check to see if we collide with any static objects in the interim,
   //  in which case, we'll be warping over a reduced number of ticks toward that
   //  position.  mCurrVelocity will be set based on tick distance travelled during
   //  the warp, and set to the warp end velocity at the end of the warp.

   Point3F simCurrPos = mLastServerPos;
   Point3F simCurrVel = mLastServerVel;

   U32 simTicksLeft = smProjectileWarpTicks;
   mNumWarpTicks = 0;

   while (simTicksLeft-- != 0) {
      mNumWarpTicks++;

      Point3F targPos;
      if (getTargetPosition(&targPos) == true) {
         Point3F newVelocity;
         if (getNewVelocity(simCurrPos, simCurrVel, targPos, &newVelocity) == false) {
            // Go unguided...
            mTargetingMode = None;
         }
         simCurrVel = newVelocity;
      } else {
         if (mTargetingMode == Object)
            mTargetPtr = NULL;
         mTargetingMode = None;
      }

      simCurrPos += simCurrVel * (TickMs / 1000.0);
//      F32     hitDelta;
//      Point3F hitPos;
//      Point3F hitNormal;
//      SceneObject* hitObject;
//      if (updatePos(simCurrPos, newSimPos, false, &hitDelta, &hitPos, &hitNormal, hitObject) == false) {
//         // Bang!  Hit something.  Need to predict the explosion here?
//
//      }
   }

   mWarpTicksLeft   = mNumWarpTicks;
   mWarpStart       = mCurrPosition;
   mWarpEnd         = simCurrPos;
   mWarpEndVelocity = simCurrVel;
}


//--------------------------------------------------------------------------
void SeekerProjectile::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isServerObject()) {
      // Check out the early exit conditions...
      //
      if (mCurrTick >= mDeleteTick) {
         deleteObject();
         return;
      }
      else if (mHidden == true) {
         // Already exploded
         return;
      }
      else if (mCurrTick >= mDeathTick) {
         // Past our lifetime, blow up
         Point3F normal = mCurrVelocity * -1;
         normal.normalize();
         if (bool(mTargetPtr))
            mTargetPtr->decHomingCount();
         
         explode(mCurrPosition, normal);

         if (bool(mTargetPtr))
            mOriginalTargetPtr = mTargetPtr = NULL;

         setMaskBits(ExplosionMask);
         return;
      }

      Point3F targetPos;
      if (getTargetPosition(&targetPos) == true) {
         // We're tracking ok still, see if we modify our velocity...
         //
         Point3F newVelocity;
         if (getNewVelocity(mCurrPosition, mCurrVelocity, targetPos, &newVelocity)) {
            mCurrVelocity = newVelocity;
         } else {
            mCurrVelocity = newVelocity;

            F32 distToTarget = (mCurrPosition - targetPos).len();
            if (distToTarget < mDataBlock->proximityRadius) {
               // Close enough to blow up the target...
               Point3F normal = mCurrVelocity * -1;
               normal.normalize();
               if (bool(mTargetPtr))
                  mTargetPtr->decHomingCount();
         
               explode(mCurrPosition, normal);

               if (bool(mTargetPtr))
                  mOriginalTargetPtr = mTargetPtr = NULL;
               mDeleteTick = mCurrTick + DeleteWaitTicks;
               setMaskBits(ExplosionMask);
               return;
            }
         }
      } else {
         if (mTargetingMode != None) {
            mTargetingMode = None;
            setMaskBits(SeekerUpdateMask);
         }
      }


      // accelerate missile
      bool activeWarhead = mCurrTick >= (mDataBlock->flechetteDelayMs/TickMs);
      if( (mCurrTick >= (mDataBlock->flechetteDelayMs/TickMs)) &&
          (mCurrVelocity.len() < mDataBlock->maxVelocity) )
      {
         Point3F excessVel = mExcessDir * mExcessVel;
         mCurrVelocity -= excessVel;


         Point3F curDir = mCurrVelocity;
         curDir.normalizeSafe();
         mCurrVelocity += curDir * mDataBlock->acceleration * (TickMs / 1000.0);

         mCurrVelocity += excessVel;
      }


      Point3F newPos = mCurrPosition + mCurrVelocity * (TickMs / 1000.0);

      F32     hitDelta;
      Point3F hitPos;
      Point3F hitNormal;
      SceneObject* hitObject;

      if (updatePos(mCurrPosition, newPos, true, &hitDelta, &hitPos, &hitNormal, hitObject) == false) {
         // Collided with something here...
         //
         if (bool(mTargetPtr))
            mTargetPtr->decHomingCount();
         
         if( mHitWater || activeWarhead == false)
         {
            mHidden = true;
         }
         else
         {
            explode(hitPos, hitNormal);
            setMaskBits(ExplosionMask);
         }

         if (bool(mTargetPtr))
            mOriginalTargetPtr = mTargetPtr = NULL;
         
         mTargetingMode = None;
         
         // Need to reset our delete wait time...
         mDeleteTick = mCurrTick + DeleteWaitTicks;
         return;
      } else {
         // No collision, set the new position
         mCurrPosition = newPos;
         MatrixF xform(true);
         xform.setColumn(3, mCurrPosition);
         setTransform(xform);
      }

      // If we're in object mode, we need to udpate frequently to account for
      //  objects going in and out of scope...
      if (mTargetingMode == Object)
         setMaskBits(SeekerUpdateMask);
   } else {
      // Client process tick...
      //
      if (mHidden == true) {
         // Already exploded
         return;
      }

      if( mPlayedSplash )
         return;

      if (mTargetingMode == Object) {
         if (bool(mTargetPtr) == false) {
            // The ghost of our target was deleted.  Switch to tracking the
            //  last known position
            mTargetingMode = Position;
         } else {
            // Otherwise, update the last known position if we're in object mode so we
            //  have a good record of position
            mTargetPtr->getWorldBox().getCenter(&mTargetPosition);
         }
      }

      // accelerate missile
      bool activeWarhead = mCurrTick >= (mDataBlock->flechetteDelayMs/TickMs);
      if( (mCurrTick >= (mDataBlock->flechetteDelayMs/TickMs)) &&
          (mCurrVelocity.len() < mDataBlock->maxVelocity) )
      {
         mCurrVelocity -= mPlayerVel;
         Point3F curDir = mCurrVelocity;
         curDir.normalizeSafe();
         mCurrVelocity += curDir * mDataBlock->acceleration * (TickMs / 1000.0);
         mCurrVelocity += mPlayerVel;
      }


      Point3F newPosition;
      if (mWarpTicksLeft != 0) {
         // Do a warp rather than a prediction...
         mWarpTicksLeft--;
         F32 interpFraction = F32(mNumWarpTicks - mWarpTicksLeft) / F32(mNumWarpTicks);

         newPosition = mWarpStart * (1.0 - interpFraction) + mWarpEnd * interpFraction;

         if (mWarpTicksLeft == 0)
            mCurrVelocity = mWarpEndVelocity;
         else
            mCurrVelocity = (mWarpEnd - mWarpStart) * (1000.0 / F32(mNumWarpTicks * TickMs));

      } else {
         mNumWarpTicks = 0;

         // Prediction...
         Point3F targetPos;
         if (getTargetPosition(&targetPos) == true) {
            Point3F newVelocity;
            if (getNewVelocity(mCurrPosition, mCurrVelocity, targetPos, &newVelocity) == false) {
               // Unguided.  Prediction explosion?
               mTargetingMode = None;
            }
            mCurrVelocity = newVelocity;
         }

         newPosition = mCurrPosition + mCurrVelocity * (TickMs / 1000.0);
      }

      updateFlechette(newPosition);

      // play exhaust particles out the back of the missile launcher
      if( mExhaustEmitter && mCurrTick < mDataBlock->exhaustTimeMs / TickMs )
      {
         if( bool(mSourceObject) )
         {
            MatrixF imageTrans;
            mSourceObject->getImageTransform( mSourceObjectSlot, mDataBlock->exhaustNodeName, &imageTrans );

            Point3F pos = imageTrans.getPosition();
            Point3F dir;
            imageTrans.getColumn( 1, &dir );
            mExhaustEmitter->emitParticles( pos, pos, dir, -mSourceObject->getVelocity(), TickMs );
         }
      }

      F32     hitDelta;
      Point3F hitPos;
      Point3F hitNormal;
      SceneObject* hitObject;
      if (updatePos(mCurrPosition, newPosition, false, &hitDelta, &hitPos, &hitNormal, hitObject) == false) {
         // Collided with something here...
         //
         U32 flechetteTime = (F32)mDataBlock->flechetteDelayMs/(F32)TickMs * 0.5;
         if ((mFlechetteGhostTick != 0xFFFFFFFF) && (mCurrTick - mFlechetteGhostTick) >= flechetteTime)
            explode(hitPos, hitNormal);
         else
            mHidden = true;

         // Need to reset our delete wait time...
         mDeleteTick = mCurrTick + DeleteWaitTicks;
         setMaskBits(ExplosionMask);
         return;
      } else {
         // No collision, set the new position
         mCurrPosition = newPosition;
         MatrixF xform(true);
         xform.setColumn(3, mCurrPosition);
         setTransform(xform);
      }


      // test for splash
      if( bool(mSourceObject) )
         mSourceObject->disableCollision();

      RayInfo rInfo;
      if( mContainer->castRay( mCurrPosition, newPosition, WaterObjectType, &rInfo ) )
      {
         createSplash( rInfo.point );
         mHidden = true;
      }

      if( bool(mSourceObject) )
         mSourceObject->enableCollision();


      if( !mPlayedSplash )
      {
         mCurrDelta     = mCurrPosition - newPosition;
         mCurrPosition  = newPosition;
         mCurrDeltaBase = newPosition;
      }

   }
}


void SeekerProjectile::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);
   
   if (mHidden == true )
      return;

   Point3F newPos = mCurrDeltaBase + mCurrDelta * delta;
   VectorF dir;

   U32 flechetteTime = (F32)mDataBlock->flechetteDelayMs/(F32)TickMs * 0.5;
   if( (mCurrTick - mFlechetteGhostTick) >= flechetteTime )
   {
      dir = mCurrVelocity;
   }
   else
   {
      dir = mInitialDirection;
   }

   if( dir.len() > 0.0001 )
   {
      dir.normalize();

      MatrixF xform;
      xform = MathUtils::createOrientFromDir( dir );
      xform.setPosition(newPos);
      setRenderTransform(xform);
   }

   updateSound(newPos, getVelocity(), true);
}


//--------------------------------------------------------------------------
U32 SeekerProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   Point3F tarPos;

   if (stream->writeFlag(mask & InitialUpdateMask)) {
      mathWrite(*stream, mCurrPosition);
      mathWrite(*stream, mCurrVelocity);

      mathWrite(*stream, mExcessDir * mExcessVel);

      if (bool(mSourceObject)) {
         // Potentially have to write this to the client, let's make sure it has a
         //  ghost on the other side...
         S32 ghostIndex = con->getGhostIndex(mSourceObject);
         if (stream->writeFlag(ghostIndex != -1)) {
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
            stream->writeRangedU32(U32(mSourceObjectSlot),
                                   0, ShapeBase::MaxMountedImages - 1);
         }
      } else {
         stream->writeFlag(false);
      }

      if (stream->writeFlag(getTargetPosition(&tarPos))) {
         S32 gIndex = -1;
         if (mTargetingMode == Object && bool(mTargetPtr))
            gIndex = con->getGhostIndex(mTargetPtr);

         if (stream->writeFlag(gIndex != -1)) {
            // Write ghost index
            stream->writeRangedU32(gIndex, 0, NetConnection::MaxGhostCount);
         } else {
            // Write target position
            mathWrite(*stream, tarPos);
         }
      }

      stream->writeFlag(mCurrTick < (mDataBlock->flechetteDelayMs/TickMs));
   } else {
      if (stream->writeFlag(mask & ExplosionMask)) {
         // We've exploded.  All we need to do is write the position and the normal
         Point3F normal = mCurrVelocity * -1;
         normal.normalize();
         mathWrite(*stream, mCurrPosition);
         mathWrite(*stream, normal);
      } else {
         // Normal update
         mathWrite(*stream, mCurrPosition);
         mathWrite(*stream, mCurrVelocity);
         if (stream->writeFlag(getTargetPosition(&tarPos))) {
            S32 gIndex = -1;
            if (mTargetingMode == Object && bool(mTargetPtr))
               gIndex = con->getGhostIndex(mTargetPtr);

            if (stream->writeFlag(gIndex != -1)) {
               // Write ghost index
               stream->writeRangedU32(gIndex, 0, NetConnection::MaxGhostCount);
            } else {
               // Write target position
               mathWrite(*stream, tarPos);
            }
         }
      }
   }

   return retMask;
}

void SeekerProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) {
      mathRead(*stream, &mLastServerPos);
      mathRead(*stream, &mLastServerVel);

      mathRead(*stream, &mPlayerVel);

      if (stream->readFlag()) {
         mSourceObjectId   = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         mSourceObjectSlot = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);

         NetObject* pObject = con->resolveGhost(mSourceObjectId);
         if (pObject != NULL)
            mSourceObject = dynamic_cast<ShapeBase*>(pObject);

         if (bool(mSourceObject) == false)
            Con::errorf(ConsoleLogEntry::General, "LinearProjectile::unpackUpdate: could not resolve source ghost properly on initial update");
      } else {
         mSourceObjectId   = -1;
         mSourceObjectSlot = -1;
         mSourceObject     = NULL;
      }

      if (stream->readFlag()) {
         if (stream->readFlag()) {
            U32 id = stream->readRangedU32(0, NetConnection::MaxGhostCount);

            NetObject* pObject = con->resolveGhost(id);
            if (pObject != NULL) {
               GameBase* pSB = dynamic_cast<GameBase*>(pObject);
               if (pSB != NULL) {
                  mTargetPtr = pSB;
                  mTargetPtr->getWorldBox().getCenter(&mTargetPosition);
                  mTargetingMode = Object;
               } else {
                  Con::errorf(ConsoleLogEntry::General, "Error in SeekerProjectile::unpackUpdate: resolved ghost isn't a gamebase");
                  mTargetingMode = None;
               }
            } else {
               Con::errorf(ConsoleLogEntry::General, "Error in SeekerProjectile::unpackUpdate: could not resolve ghost properly");
               mTargetingMode = None;
            }
         } else {
            mathRead(*stream, &mTargetPosition);
            mTargetingMode = Position;
         }
      } else {
         mTargetingMode = None;
      }

      if (stream->readFlag()) {
         mFlechetteGhostTick = 0;
      } else {
         mFlechetteGhostTick = 0xFFFFFFFF;
      }
   } else {
      if (stream->readFlag()) {
         // Explosion...
         Point3F point, normal;
         mathRead(*stream, &point);
         mathRead(*stream, &normal);
         explode(point, normal);
         return;
      } else {
         // Targeting update...
         mathRead(*stream, &mLastServerPos);
         mathRead(*stream, &mLastServerVel);

         if (stream->readFlag()) {
            if (stream->readFlag()) {
               U32 id = stream->readRangedU32(0, NetConnection::MaxGhostCount);

               NetObject* pObject = con->resolveGhost(id);
               if (pObject != NULL) {
                  GameBase* pSB = dynamic_cast<GameBase*>(pObject);
                  if (pSB != NULL) {
                     mTargetPtr = pSB;
                     mTargetPtr->getWorldBox().getCenter(&mTargetPosition);
                     mTargetingMode = Object;
                  } else {
                     Con::errorf(ConsoleLogEntry::General, "Error in SeekerProjectile::unpackUpdate: resolved ghost isn't a shapebase");
                     mTargetingMode = None;
                  }
               } else {
                  Con::errorf(ConsoleLogEntry::General, "Error in SeekerProjectile::unpackUpdate: could not resolve ghost properly");
                  mTargetingMode = None;
               }
            } else {
               mathRead(*stream, &mTargetPosition);
               mTargetingMode = Position;
            }
         } else {
            mTargetingMode = None;
         }

         setupWarp();
      }
   }
}

//--------------------------------------------------------------------------
// Update flechette
//--------------------------------------------------------------------------
void SeekerProjectile::updateFlechette(const Point3F& pos)
{
   if (!mDataBlock->useFlechette) return;
   if (mFlechetteGhostTick == 0xFFFFFFFF) return;
   if (mFlechettePuff) return;

   U32 flechetteTime = (F32)mDataBlock->flechetteDelayMs/(F32)TickMs * 0.5;
   if( (mCurrTick - mFlechetteGhostTick) >= flechetteTime )
   {
      if( !mFlechettePuff )
      {
         mFlechettePuff = true;

         if( mPuffEmitter )
         {
            mPuffEmitter->emitParticles( mCurrPosition, mCurrPosition, Point3F( 0.0, 0.0, 1.0 ), Point3F( 0.0, 0.0, 0.0 ), 1000 );
         }

         MatrixF trans = getRenderTransform();
         trans.setPosition( pos );

         Debris *debList[4];

         for( int i=0; i<4; i++ )
         {
            debList[i] = new Debris;
            debList[i]->onNewDataBlock( mDataBlock->casingDeb );

            if( !debList[i]->registerObject() )
            {
               delete debList[i];
               debList[i] = NULL;
            }
            else
            {
               debList[i]->setTransform( trans );
            }
         }

         // ughhh, hard code the velocities and rotations for flechettes for now.
         // if more tweaking is necessary, move to script - bramage
         F32 debVel = 8.0;

         if( debList[0] )
         {
            Point3F dir( -gRandGen.randF( 1.0, 2.0 ), 5.0, -gRandGen.randF( 1.0, 2.0 ) );
            dir.normalize();
            dir *= debVel;
            trans.mulV( dir );
            debList[0]->init( pos, dir );

            debList[0]->setRotAngles( Point3F(450.0, 00.0, -450.0) );
         }

         if( debList[1] )
         {
            Point3F dir(  gRandGen.randF( 1.0, 2.0 ), 5.0, -gRandGen.randF( 1.0, 2.0 ) );
            dir.normalize();
            dir *= debVel;
            trans.mulV( dir );
            debList[1]->init( pos, dir );

            debList[1]->setRotAngles( Point3F(450.0, 00.0, 450.0) );
         }

         if( debList[2] )
         {
            Point3F dir(  gRandGen.randF( 1.0, 2.0 ), 5.0, gRandGen.randF( 1.0, 2.0 ) );
            dir.normalize();
            dir *= debVel;
            trans.mulV( dir );
            debList[2]->init( pos, dir );

            debList[2]->setRotAngles( Point3F(-450.0, 00.0, 450.0) );
         }

         if( debList[3] )
         {
            Point3F dir( -gRandGen.randF( 1.0, 2.0 ), 5.0, gRandGen.randF( 1.0, 2.0 ) );
            dir.normalize();
            dir *= debVel;
            trans.mulV( dir );
            debList[3]->init( pos, dir );

            debList[3]->setRotAngles( Point3F(-450.0, 00.0, -450.0) );
         }

      }
   }

}

//--------------------------------------------------------------------------
// RENDER
//--------------------------------------------------------------------------
void SeekerProjectile::renderObject(SceneState* state, SceneRenderImage*)
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
   dglMultMatrix(&mRenderObjToWorld);
   glScalef(mObjScale.x, mObjScale.y, mObjScale.z);

   TSShapeInstance *shape;

   if( mDataBlock->useFlechette && !mFlechettePuff )
   {
      shape = mCasingShape;
   }
   else
   {
      shape = mProjectileShape;
   }


   shape->selectCurrentDetail();
   shape->animate();

   Point3F cameraOffset;
   getRenderTransform().getColumn(3,&cameraOffset);
   cameraOffset -= state->getCameraPosition();
   F32 fogAmount = state->getHazeAndFog(cameraOffset.len(),cameraOffset.z);

   if (mFadeValue == 1.0) {
      shape->setupFog(fogAmount, state->getFogColor());
   } else {
      shape->setupFog(0.0, state->getFogColor());
      shape->setAlphaAlways(mFadeValue * (1.0 - fogAmount));
   }
   shape->render();

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
// Advance time
//--------------------------------------------------------------------------
void SeekerProjectile::advanceTime( F32 dt )
{
   Parent::advanceTime(dt);

   if( mPlayedSplash ) return;

   // delay emitting smoke and fire from projectile until after it is done ejecting
   if( !mDataBlock->useFlechette || mCurrTick >= (mDataBlock->flechetteDelayMs/TickMs) )
   {
      emitParticles( getRenderPosition(), mLastPos, mCurrVelocity, dt*1000.0 );
   }

   mLastPos = getRenderPosition();
}

//--------------------------------------------------------------------------
// Register lights
//--------------------------------------------------------------------------
void SeekerProjectile::registerLights( LightManager * lightManager, bool lightingScene )
{
   if(lightingScene)
      return;

   if( !mDataBlock->useFlechette || mCurrTick >= (mDataBlock->flechetteDelayMs/TickMs) )
   {
      if (mDataBlock->hasLight && mHidden == false) {
         mLight.mType = LightInfo::Point;
         getRenderTransform().getColumn(3, &mLight.mPos);
         mLight.mRadius = mDataBlock->lightRadius;
         mLight.mColor  = mDataBlock->lightColor;
         lightManager->addLight(&mLight);
      }
   }
}
