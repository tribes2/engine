//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projGrenade.h"
#include "core/bitStream.h"
#include "sim/netConnection.h"
#include "game/shapeBase.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"
#include "game/explosion.h"
#include "terrain/waterBlock.h"
#include "game/splash.h"
#include "game/particleEngine.h"

IMPLEMENT_CO_NETOBJECT_V1(GrenadeProjectile);
IMPLEMENT_CO_DATABLOCK_V1(GrenadeProjectileData);

//--------------------------------------------------------------------------
//--------------------------------------
//
GrenadeProjectileData::GrenadeProjectileData()
{
   grenadeElasticity = 0.999;
   grenadeFriction   = 0.3;
   
   gravityMod        = 1.0;

   isBallistic       = true;

   armingDelayMS     = 3000;
   muzzleVelocity    = 75;

   drag    = 0;
   density = 1;

   lifetimeMS        = 20000;  // default 20 sec max lifetime
}

GrenadeProjectileData::~GrenadeProjectileData()
{

}


//--------------------------------------------------------------------------
void GrenadeProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("armingDelayMS",     TypeS32, Offset(armingDelayMS,     GrenadeProjectileData));
   addField("muzzleVelocity",    TypeF32, Offset(muzzleVelocity,    GrenadeProjectileData));
   addField("grenadeElasticity", TypeF32, Offset(grenadeElasticity, GrenadeProjectileData));
   addField("grenadeFriction",   TypeF32, Offset(grenadeFriction,   GrenadeProjectileData));
   addField("drag",              TypeF32, Offset(drag,              GrenadeProjectileData));
   addField("density",           TypeF32, Offset(density,           GrenadeProjectileData));
   addField("gravityMod",        TypeF32, Offset(gravityMod,        GrenadeProjectileData));
   addField("lifetimeMS",        TypeS32, Offset(lifetimeMS,        GrenadeProjectileData));
}


bool GrenadeProjectileData::calculateAim(const Point3F& targetPos,
                                         const Point3F& targetVel,
                                         const Point3F& sourcePos,
                                         const Point3F& sourceVel,
                                         Point3F*       outputVectorMin,
                                         F32*           outputMinTime,
                                         Point3F*       outputVectorMax,
                                         F32*           outputMaxTime)
{
   // Ok, this is a quartic calculation to determine the time to target.  TODO:
   //  get the gravity from the environment.  TODO: handle water
   //
   Point3F p = targetPos - sourcePos;
   Point3F v = targetVel - sourceVel*velInheritFactor;

   F32 g = 9.81f * gravityMod;

   F32 a = 0.25 * (g * g);
   F32 b = v.z * g;
   F32 c = p.z * g + v.lenSquared() - (muzzleVelocity * muzzleVelocity);
   F32 d = 2 * mDot(p, v);
   F32 e = p.lenSquared();

   F32 x[4];
   U32 numRealSolutions = mSolveQuartic(a, b, c, d, e, x);

   if (numRealSolutions == 0) {
      // No solution is possible in the real numbers
      outputVectorMin->set(0, 0, 1);
      outputVectorMax->set(0, 0, 1);
      *outputMinTime = -1;
      *outputMaxTime = -1;
      return false;
   }

   F32 minPos[2] = { 1e8, 1e8 };
   for (U32 i = 0; i < numRealSolutions; i++) {
      if (x[i] > 0.0) {
         if (x[i] < minPos[0]) {
            minPos[1] = minPos[0];
            minPos[0] = x[i];
         } else if (x[i] < minPos[1]) {
            minPos[1] = x[i];
         }
      }
   }
   if (minPos[0] == 1e8) {
      outputVectorMin->set(0, 0, 1);
      outputVectorMax->set(0, 0, 1);
      *outputMinTime = -1;
      *outputMaxTime = -1;
      return false;
   }
   if (minPos[1] == 1e8)
      minPos[1] = minPos[0];
   AssertFatal(minPos[0] != 1e8 && minPos[1] != 1e8 && minPos[0] > 0.0 && minPos[1] > 0.0,
               "Error, bad calculation of minimum positive times!");

   // Ok, we have solutions!
   *outputVectorMin  = targetPos;
   *outputVectorMin += targetVel * minPos[0];
   *outputVectorMin -= Point3F(0, 0, -g) * 0.5 * minPos[0] * minPos[0];
   *outputVectorMin -= sourcePos;
   *outputVectorMin /= minPos[0];
   *outputVectorMin -= sourceVel * velInheritFactor;
   *outputVectorMin /= muzzleVelocity;
   outputVectorMin->normalize();

   *outputVectorMax  = targetPos;
   *outputVectorMax += targetVel * minPos[1];
   *outputVectorMax -= Point3F(0, 0, -g) * 0.5 * minPos[1] * minPos[1];
   *outputVectorMax -= sourcePos;
   *outputVectorMax /= minPos[1];
   *outputVectorMax -= sourceVel * velInheritFactor;
   *outputVectorMax /= muzzleVelocity;
   outputVectorMax->normalize();

   *outputMinTime = minPos[0];
   *outputMaxTime = minPos[1];
   return (*outputMinTime * 1000.0) < lifetimeMS;
}


//--------------------------------------------------------------------------
bool GrenadeProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (armingDelayMS < 250) {
      Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: arming delay below minimum allowed (250 ms)", getName());
      armingDelayMS = 250;
   }
   if (muzzleVelocity < 0.1f) {
      Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: muzzle velocity beneath threshold", getName());
      muzzleVelocity = 0.1f;
   }
   if (grenadeElasticity < 0.0 || grenadeElasticity > 0.999) {
      Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: elasticity bounded by range [0, 0.999] to prevent FP errors from accumulating", getName());
      grenadeElasticity = grenadeElasticity < 0.0 ? 0.0 : 0.999;
   }
   if (grenadeFriction < 0.0 || grenadeFriction > 1) {
      Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: friction bounded by range [0, 1]", getName());
      grenadeFriction = grenadeFriction < 0.0 ? 0.0 : 1;
   }
   if (drag < 0.0) {
      Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: drag bounded by range [0, inf]", getName());
      drag = 0.0;
   }
   if (density < 0.1 || density > 10) {
      Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: density bounded by range [0.1, 10]", getName());
      density = density < 0.1 ? 0.1 : 10;
   }

   if( lifetimeMS < 50 ){
      Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: lifetimeMS < 50", getName());
      if( lifetimeMS == 0 )
      {
         Con::warnf(ConsoleLogEntry::General, "GrenadeProjectileData(%s)::onAdd: lifetimeMS == 0", getName());
         lifetimeMS = 1000;
      }
   }

   // Make sure that arming delay is an even multiple of a tick
   armingDelayMS = (armingDelayMS + TickMs - 1) & ~(TickMs - 1);

   return true;
}

//--------------------------------------------------------------------------
void GrenadeProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(armingDelayMS);
   stream->write(muzzleVelocity);
   stream->write(grenadeElasticity);
   stream->write(grenadeFriction);
   stream->write(drag);
   stream->write(density);
   stream->write(gravityMod);
   stream->write(lifetimeMS);
}

void GrenadeProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&armingDelayMS);
   stream->read(&muzzleVelocity);
   stream->read(&grenadeElasticity);
   stream->read(&grenadeFriction);
   stream->read(&drag);
   stream->read(&density);
   stream->read(&gravityMod);
   stream->read(&lifetimeMS);
}


//--------------------------------------------------------------------------
//--------------------------------------
//
GrenadeProjectile::GrenadeProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);

   mDeleteTick = -1;
   mInWater = false;
   mLastInWater = false;
   mQuickSplash = false;
   mSentInitialUpdate = false;
}

GrenadeProjectile::~GrenadeProjectile()
{
   //
}

//--------------------------------------------------------------------------
void GrenadeProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


void GrenadeProjectile::consoleInit()
{
   //
}


bool GrenadeProjectile::calculateImpact(float    simTime,
                                        Point3F& pointOfImpact,
                                        float&   impactTime)
{
   if (mHidden == true) {
      impactTime = 0;
      pointOfImpact.set(0, 0, 0);
      return false;
   }

   const F32 timeSlice = 0.25;

   Point3F currPos = mCurrPosition;
   Point3F currVel = mCurrVelocity;

   for (F32 currT = 0.0; currT <= simTime; currT += timeSlice) {
      Point3F newVel = currVel + Point3F(0, 0, -9.81 * mDataBlock->gravityMod) * timeSlice;
      Point3F newPos = currPos + newVel * timeSlice;

      U32 mask = PlayerObjectType | TerrainObjectType | InteriorObjectType | WaterObjectType | ForceFieldObjectType;
      RayInfo rayInfo;
      if (gServerContainer.castRay(currPos, newPos, mask, &rayInfo)) {
         pointOfImpact = rayInfo.point;
         impactTime  = currT;
         impactTime += ((currPos - pointOfImpact).len() / (currPos - newPos).len()) * timeSlice;
         return true;
      }

      currPos = newPos;
      currVel = newVel;
   }

   impactTime = 0;
   pointOfImpact.set(0, 0, 0);
   return false;
}


//--------------------------------------------------------------------------
bool GrenadeProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   mArmedTick = mDataBlock->armingDelayMS / TickMs;
   mDeleteTick = mDataBlock->lifetimeMS / TickMs;
   
   if (isServerObject()) {
      mCurrPosition  = mInitialPosition;
      mCurrVelocity  = mInitialDirection * mDataBlock->muzzleVelocity;
      mCurrVelocity += mExcessDir * mExcessVel;

      // Pull back the projectile a bit.  This prevents problems that occur when the
      // weapon is jammed against the wall and the projectile starts on the other side
      // of it. - bramage
      Point3F dir = mCurrVelocity;
      dir.normalizeSafe();
      mCurrPosition -= dir * 0.15;

   }
   else
   {
      Point3F newPos = mInitialPosition + mCurrVelocity * mCurrTick * TickMs / 1000.0;

      if( determineSplash( mInitialPosition, newPos) )
      {
         createSplash( mSplashPos );
      }
   }

   addToScene();

   return true;
}


void GrenadeProjectile::onRemove()
{
   if (isClientObject())
      updateSound(Point3F(0, 0, 0), Point3F(0, 0, 0), false);
   removeFromScene();

   Parent::onRemove();
}


bool GrenadeProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<GrenadeProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}

//--------------------------------------------------------------------------
void GrenadeProjectile::explode(const Point3F& p, const Point3F& n, const U32 collideType )
{
   // Make sure we don't explode twice...
   if (mHidden == true)
      return;

   if (isServerObject()) {
      // Do what the server needs to do, damage the surrounding objects, etc.
      mExplosionPosition = p + (n*0.01);
      mExplosionNormal = n;

      char buffer[128];
      dSprintf(buffer, sizeof(buffer),  "%f %f %f", mExplosionPosition.x,
                                                    mExplosionPosition.y,
                                                    mExplosionPosition.z);
      Con::executef(mDataBlock, 4, "onExplode", scriptThis(), buffer, "1.0");

      setMaskBits(ExplosionMask);
   } else {
      // Client just plays the explosion at the right place...
      //
      Explosion* pExplosion = NULL;

      F32 waterHeight;
      if( pointInWater( (Point3F &)p, &waterHeight ) && mDataBlock->underwaterExplosion )
      {
         pExplosion = new Explosion;

         F32 depth = waterHeight - p.z;
         if( depth >= mDataBlock->depthTolerance )
         {
            pExplosion->onNewDataBlock(mDataBlock->underwaterExplosion);
         }
         else
         {
            pExplosion->onNewDataBlock(mDataBlock->explosion);
         }
      }
      else
      {
         if (mDataBlock->explosion)
         {
            pExplosion = new Explosion;
            pExplosion->onNewDataBlock(mDataBlock->explosion);
         }
      }

      if( pExplosion )
      {
         MatrixF xform(true);
         xform.setPosition(p);
         pExplosion->setTransform(xform);
         pExplosion->setInitialState(p, n);
         pExplosion->setCollideType( collideType );
         if (pExplosion->registerObject() == false)
         {
            Con::errorf(ConsoleLogEntry::General, "LinearProjectile(%s)::explode: couldn't register explosion",
                        mDataBlock->getName() );
            delete pExplosion;
            pExplosion = NULL;
         }
      }

      // Client object
      updateSound(Point3F(0, 0, 0), Point3F(0, 0, 0), false);
   }

   mHidden = true;
}


//--------------------------------------------------------------------------
Point3F GrenadeProjectile::getVelocity() const
{
   return mCurrVelocity;
}


//--------------------------------------------------------------------------
void GrenadeProjectile::processTick(const Move* move)
{
   F32 timeLeft;
   RayInfo rInfo;
   Point3F oldPosition;
   Point3F newPosition;

   Parent::processTick(move);

   if (isServerObject()) {
      // server
      if (mDeleteTick != -1 && mCurrTick >= mDeleteTick) {
         deleteObject();
         return;
      } else if (mHidden == true) {
         // already exploded, back out
         return;
      }

      // Otherwise, we have to do some simulation work.
      oldPosition = mCurrPosition;
      computeNewState(&newPosition, &mCurrVelocity);

      if (mSourceIdTimeoutTicks && bool(mSourceObject))
         mSourceObject->disableCollision();
      if (mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->disableCollision();

      timeLeft = 1.0;
      
      // if we hit a lava block explode
      if(getContainer()->castRay(oldPosition, newPosition, WaterObjectType, &rInfo) == true)
      {
         WaterBlock *wb = dynamic_cast<WaterBlock *>(rInfo.object); 
         if(wb)
         {  
            if(wb->isLava(wb->getLiquidType()))
            {
               MatrixF xform(true);
               xform.setColumn(3, rInfo.point);
               setTransform(xform);
               mCurrPosition    = rInfo.point;
               mCurrVelocity    = Point3F(0, 0, 0);

               if (mSourceIdTimeoutTicks && bool(mSourceObject))
                  mSourceObject->enableCollision();
               if (mSourceIdTimeoutTicks && bool(mVehicleObject))
                  mVehicleObject->enableCollision();

               explode(rInfo.point, rInfo.normal);

               mDeleteTick = mCurrTick + DeleteWaitTicks;
               return;
            }
         }
      }
      
      // Make sure we escape if we get stuck somehow...
      static U32 sMaxBounceCount = 5;
      U32 bounceCount = 0;
      while (bounceCount++ < sMaxBounceCount) {
         if (getContainer()->castRay(oldPosition, newPosition,
                                     csmDynamicCollisionMask | csmStaticCollisionMask,
                                     &rInfo) == true)
         {
            
            
            if((rInfo.object->getType() & csmStaticCollisionMask) == 0)
               setMaskBits(BounceMask);

            // Next order of business: do we explode on this hit?
            if (mCurrTick > mArmedTick) {
               MatrixF xform(true);
               xform.setColumn(3, rInfo.point);
               setTransform(xform);
               mCurrPosition    = rInfo.point;
               mCurrVelocity    = Point3F(0, 0, 0);

               if (mSourceIdTimeoutTicks && bool(mSourceObject))
                  mSourceObject->enableCollision();
               if (mSourceIdTimeoutTicks && bool(mVehicleObject))
                  mVehicleObject->enableCollision();

               explode(rInfo.point, rInfo.normal);
               if (mSourceIdTimeoutTicks && bool(mSourceObject))
                  mSourceObject->disableCollision();
               if (mSourceIdTimeoutTicks && bool(mVehicleObject))
                  mVehicleObject->disableCollision();

               mDeleteTick = mCurrTick + DeleteWaitTicks;
               break;
            }

            // Otherwise, this represents a bounce.  First, reflect our velocity
            //  around the normal...
            Point3F bounceVel = mCurrVelocity - rInfo.normal * (mDot( mCurrVelocity, rInfo.normal ) * 2.0);;
            mCurrVelocity = bounceVel;

            // Add in surface friction...
            Point3F tangent = bounceVel - rInfo.normal * mDot(bounceVel, rInfo.normal);
            mCurrVelocity  -= tangent * mDataBlock->grenadeFriction;

            // Now, take elasticity into account for modulating the speed of the grenade
            mCurrVelocity *= mDataBlock->grenadeElasticity;

            timeLeft = timeLeft * (1.0 - rInfo.t);
            oldPosition = rInfo.point + rInfo.normal * 0.05;
            newPosition = oldPosition + (mCurrVelocity * ((timeLeft/1000.0) * TickMs));
         } else {
            // No problems, just set the end position, and we're golden.
            mCurrPosition = newPosition;
            MatrixF xform(true);
            xform.setColumn(3, mCurrPosition);
            setTransform(xform);

            break;
         }
      }

      if( !mSentInitialUpdate && determineSplash( oldPosition, newPosition ) )
      {
         mQuickSplash = true;
      }
      
      if (mSourceIdTimeoutTicks && bool(mSourceObject))
         mSourceObject->enableCollision();
      if (mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->enableCollision();
   } else {
      // client
      if (mHidden == true)
         return;

      // Otherwise, we have to do some simulation work.
      oldPosition = mCurrPosition;
      computeNewState(&newPosition, &mCurrVelocity);

      if (bool(mSourceObject))
         mSourceObject->disableCollision();
      if (mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->disableCollision();

      timeLeft = 1.0;
      
      // Make sure we escape if we get stuck somehow...
      static U32 sMaxBounceCount = 5;
      U32 bounceCount = 0;
      while (bounceCount++ < sMaxBounceCount) {
         if (getContainer()->castRay(oldPosition, newPosition,
                                     csmDynamicCollisionMask | csmStaticCollisionMask,
                                     &rInfo) == true) {

            // Next order of business: do we explode on this hit?
            if (mCurrTick > mArmedTick) {
               MatrixF xform(true);
               xform.setColumn(3, rInfo.point);
               setTransform(xform);
               mCurrPosition    = rInfo.point;
               mCurrVelocity    = Point3F(0, 0, 0);

               explode(rInfo.point, rInfo.normal, rInfo.object->getType() );
               break;
            }

            // Otherwise, this represents a bounce.  First, reflect our velocity
            //  around the normal...
            Point3F bounceVel = mCurrVelocity - rInfo.normal * (mDot( mCurrVelocity, rInfo.normal ) * 2.0);;
            mCurrVelocity = bounceVel;

            // Add in surface friction...
            Point3F tangent = bounceVel - rInfo.normal * mDot(bounceVel, rInfo.normal);
            mCurrVelocity  -= tangent * mDataBlock->grenadeFriction;

            // Now, take elasticity into account for modulating the speed of the grenade
            mCurrVelocity *= mDataBlock->grenadeElasticity;

            timeLeft = timeLeft * (1.0 - rInfo.t);
            oldPosition = rInfo.point + rInfo.normal * 0.05;
            newPosition = oldPosition + (mCurrVelocity * ((timeLeft/1000.0) * TickMs));
         } else {
            // No problems, just set the end position, and we're golden.
            break;
         }
      }
      mCurrDeltaBase  = newPosition;
      mCurrBackVector = mCurrPosition - newPosition;
      if( !mInWater )
      {
         emitParticles(mCurrPosition, newPosition, mCurrVelocity, TickMs);
      }
      mCurrPosition   = newPosition;

      if( determineSplash( oldPosition, newPosition ) )
      {
         createSplash( mSplashPos );
      }

      if (bool(mSourceObject))
         mSourceObject->enableCollision();
      if (mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->enableCollision();

      MatrixF temp(true);
      temp.setColumn(3, mCurrDeltaBase);
      setTransform(temp);
   }
}


void GrenadeProjectile::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);

   Point3F interpPos = mCurrDeltaBase + mCurrBackVector * delta;
   MatrixF xform = getTransform();
   xform.setColumn(3, interpPos);
   setRenderTransform(xform);

   updateSound(interpPos, getVelocity(), true);
}


void GrenadeProjectile::advanceTime(F32 dt)
{
   updateBubbles( dt );
   Parent::advanceTime(dt);

   if (mInWater)
   {
      mUseUnderwaterLight = true;
   }
   else
   {
      mUseUnderwaterLight = false;
   }
   
   if (mHidden == true || dt == 0.0)
      return;
}


//--------------------------------------------------------------------------
U32 GrenadeProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & GameBase::InitialUpdateMask))
   {
      // Initial update
      
      mathWrite(*stream, mCurrPosition);
      mathWrite(*stream, mCurrVelocity);
      stream->writeRangedU32(mCurrTick, 0, MaxLivingTicks);

      stream->writeFlag( mQuickSplash );

      if (stream->writeFlag((mask & ExplosionMask) && mHidden))
      {
         mathWrite(*stream, mCurrPosition);
         mathWrite(*stream, mExplosionNormal);
      }


      if (bool(mSourceObject))
      {
         // Potentially have to write this to the client, let's make sure it has a
         //  ghost on the other side...
         S32 ghostIndex = con->getGhostIndex(mSourceObject);
         if (stream->writeFlag(ghostIndex != -1))
         {
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
            stream->writeRangedU32(U32(mSourceObjectSlot),
                                   0, ShapeBase::MaxMountedImages - 1);
         }
      }
      else
      {
         stream->writeFlag(false);
      }
      
      if (bool(mVehicleObject))
      {
         // Potentially have to write this to the client, let's make sure it has a
         //  ghost on the other side...
         S32 ghostIndex = con->getGhostIndex(mVehicleObject);
         if (stream->writeFlag(ghostIndex != -1))
         {
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
         }
      } 
      else
      {
         stream->writeFlag(false);
      }

      mSentInitialUpdate = true;
   }
   else
   {
      if (stream->writeFlag(mask & BounceMask)) {
         // Bounce against dynamic object
         mathWrite(*stream, mCurrPosition);
         mathWrite(*stream, mCurrVelocity);
      }

      if (stream->writeFlag(mask & ExplosionMask)) {
         mathWrite(*stream, mCurrPosition);
         mathWrite(*stream, mExplosionNormal);
      }
   }

   return retMask;
}

void GrenadeProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      // Initial update

      
      mathRead(*stream, &mCurrPosition);
      mathRead(*stream, &mCurrVelocity);
      mCurrTick = stream->readRangedU32(0, MaxLivingTicks);

      if( stream->readFlag() )
      {
         resolveInitialSplash();
      }

 
      if (stream->readFlag())
      {
         // explosion on the server...
         Point3F explodePoint;
         Point3F explodeNormal;
         mathRead(*stream, &explodePoint);
         mathRead(*stream, &explodeNormal);

         explode(explodePoint, explodeNormal);
      }


      if( pointInWater( mCurrPosition ) )
      {
         mInWater = true;
         mLastInWater = true;
      }

      if (stream->readFlag())
      {
         mSourceObjectId   = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         mSourceObjectSlot = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);

         NetObject* pObject = con->resolveGhost(mSourceObjectId);
         if (pObject != NULL)
            mSourceObject = dynamic_cast<ShapeBase*>(pObject);
      }
      else
      {
         mSourceObjectId   = -1;
         mSourceObjectSlot = -1;
         mSourceObject     = NULL;
      }

      if (stream->readFlag())
      {
         mVehicleObjectId  = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         NetObject* pObject = con->resolveGhost(mVehicleObjectId);
         if (pObject != NULL)
            mVehicleObject = dynamic_cast<ShapeBase*>(pObject);
         if (bool(mVehicleObject) == false)
            Con::errorf(ConsoleLogEntry::General, "LinearProjectile::unpackUpdate: could not resolve source ghost properly on initial update");
      }
      else
      {
         mVehicleObjectId = 0;
         mVehicleObject   = NULL;
      }
   }
   else
   {
      if (stream->readFlag())
      {
         mathRead(*stream, &mCurrPosition);
         mathRead(*stream, &mCurrVelocity);
      }

      if (stream->readFlag())
      {
         // explosion on the server...
         Point3F explodePoint;
         Point3F explodeNormal;
         mathRead(*stream, &explodePoint);
         mathRead(*stream, &explodeNormal);

         explode(explodePoint, explodeNormal);
      }
   }
}

//--------------------------------------------------------------------------
void GrenadeProjectile::computeNewState(Point3F* newPosition,
                                        Point3F* newVelocity)
{
   // We simulate forward TickMs milliseconds.  Remember that we have to use
   //  dumb velocity to keep the server and the client in agreement.  We return a
   //  fully precise velocity vector tho.
   // DMMTODO: we need adjustable gravity in this function...
   // DMMNOTE: we might want to step up to a RK solver here rather than the
   //           lame Euler solver below...
   //
   Point3F force = Point3F(0, 0, -9.81 * mDataBlock->gravityMod);

   if (mDataBlock->drag != 0.0 || mDataBlock->density != 1) {

      bool wetStart = pointInWater( mCurrPosition );

      mLastInWater = mInWater;
      mInWater = wetStart;

      if (wetStart) {

         // Compute new forces based on drag and density...
         if (mDataBlock->drag != 0.0)
            force += -mCurrVelocity * (mDataBlock->drag * 15);
         if (mDataBlock->density != 0.0) {

         }
      }

   }

   *newVelocity  = mCurrVelocity;
   *newVelocity += force * (F32(TickMs) / 1000.0f);

   *newPosition  = mCurrPosition;
   *newPosition += (*newVelocity) * (F32(TickMs) / 1000.0f);
}

//--------------------------------------------------------------------------
bool GrenadeProjectile::determineSplash( Point3F &oldPos, Point3F &newPos )
{
   bool startInWater = pointInWater( oldPos );
   bool endInWater = pointInWater( newPos );
   bool noRet = (!startInWater && endInWater) || (startInWater && !endInWater);
   if( !noRet ) return false;


   Point3F dir = newPos - oldPos;
   if( dir.isZero() ) return false;
   dir.normalize();

   Point3F start = oldPos - dir;
   Point3F end = newPos + dir;

   RayInfo rInfo;
   Container* pContainer = isServerObject() ? &gServerContainer : &gClientContainer;
   if( pContainer->castRay( start, end, WaterObjectType, &rInfo) )
   {
      mSplashPos = rInfo.point;
      return true;
   }

   return false;
}

//--------------------------------------------------------------------------
void GrenadeProjectile::updateBubbles( F32 dt )
{
   // update bubbles
   Point3F moveDir = getVelocity();

   if( mInWater )
   {
      if( mBubbleEmitter )
      {
         Point3F emissionPoint = getPosition();
         mBubbleEmitter->emitParticles( mLastPos, emissionPoint, Point3F( 0.0, 0.0, 1.0 ), 
                                        moveDir, dt * 1000.0 );
      }
   }
}

//--------------------------------------------------------------------------
// Server detected splash between mInitialPosition and the mCurrPosition that
// it sent to the client.  This function finds the splash point and plays
// the animation
//--------------------------------------------------------------------------
void GrenadeProjectile::resolveInitialSplash()
{
   VectorF dir = mCurrVelocity;
   dir.normalize();
   Point3F oldPos = mCurrPosition - dir * 10.0;
   determineSplash( oldPos, mCurrPosition );
   createSplash( mSplashPos );
}
