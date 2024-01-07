//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "core/bitStream.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "game/shapeBase.h"
#include "ts/tsShapeInstance.h"
#include "game/explosion.h"
#include "terrain/waterBlock.h"
#include "sim/decalManager.h"
#include "game/linearProjectile.h"
#include "scenegraph/sceneGraph.h"
#include "game/splash.h"

#include "dgl/dgl.h"
#include "platformWIN32/platformGL.h"
#include "scenegraph/sceneState.h"
#include "math/mathUtils.h"

IMPLEMENT_CO_DATABLOCK_V1(LinearProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(LinearProjectile);

const U32 LinearProjectile::csmDecalMask = TerrainObjectType | InteriorObjectType;

//--------------------------------------------------------------------------
//--------------------------------------
//
LinearProjectileData::LinearProjectileData()
{
   dryVelocity       = 5.0;
   wetVelocity       = 5.0;

   fizzleTimeMS      = 1000;
   lifetimeMS        = 1000;
   explodeOnDeath    = false;

   reflectOnWaterImpactAngle = 0.0;
   deflectionOnWaterImpact   = 0.0;
   fizzleUnderwaterMS        = -1;

   activateDelayMS = -1;

   activateSeq = -1;
   maintainSeq = -1;

   doDynamicClientHits = false;
}

LinearProjectileData::~LinearProjectileData()
{

}

//--------------------------------------------------------------------------
void LinearProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("dryVelocity",               TypeF32,  Offset(dryVelocity,               LinearProjectileData));
   addField("wetVelocity",               TypeF32,  Offset(wetVelocity,               LinearProjectileData));
   addField("fizzleTimeMS",              TypeS32,  Offset(fizzleTimeMS,              LinearProjectileData));
   addField("lifetimeMS",                TypeS32,  Offset(lifetimeMS,                LinearProjectileData));
   addField("explodeOnDeath",            TypeBool, Offset(explodeOnDeath,            LinearProjectileData));
   addField("reflectOnWaterImpactAngle", TypeF32,  Offset(reflectOnWaterImpactAngle, LinearProjectileData));
   addField("deflectionOnWaterImpact",   TypeF32,  Offset(deflectionOnWaterImpact,   LinearProjectileData));

   addField("fizzleUnderwaterMS",        TypeS32,  Offset(fizzleUnderwaterMS,        LinearProjectileData));
   addField("activateDelayMS",           TypeS32,  Offset(activateDelayMS,           LinearProjectileData));
   addField("doDynamicClientHits",       TypeBool, Offset(doDynamicClientHits,       LinearProjectileData));
}

void LinearProjectileData::consoleInit()
{
   //
}


//--------------------------------------------------------------------------
bool LinearProjectileData::calculateAim(const Point3F& targetPos,
                                        const Point3F& targetVel,
                                        const Point3F& sourcePos,
                                        const Point3F& sourceVel,
                                        Point3F*       outputVectorMin,
                                        F32*           outputMinTime,
                                        Point3F*       outputVectorMax,
                                        F32*           outputMaxTime)
{
   // Not implemented: underwater shots...

   Point3F effTargetPos = targetPos - sourcePos;
   Point3F effTargetVel = targetVel - sourceVel * velInheritFactor;

   // Without underwater aiming, this is a straightforward law of cosines
   //  calculation.  We're not being especially efficient here, but hey.
   //
   Point3F normPos = effTargetPos;
   Point3F normVel = effTargetVel;
   if (normPos.isZero() == false)
      normPos.normalize();
   if (normVel.isZero() == false)
      normVel.normalize();

   F32 a = effTargetVel.lenSquared() - (dryVelocity*dryVelocity);
   F32 b = 2 * effTargetPos.len() * effTargetVel.len() * mDot(normPos, normVel);
   F32 c = effTargetPos.lenSquared();

   F32 det = b*b - 4*a*c;
   if (det < 0.0) {
      // No solution is possible in the real numbers
      outputVectorMin->set(0, 0, 1);
      outputVectorMax->set(0, 0, 1);
      *outputMinTime = -1;
      *outputMaxTime = -1;
      return false;
   }

   F32 sol1 = (-b + mSqrt(det)) / (2 * a);
   F32 sol2 = (-b - mSqrt(det)) / (2 * a);

   F32 t;
   if (sol2 > 0.0) {
      t = sol2;
   } else {
      t = sol1;
   }
   if (t < 0.0) {
      outputVectorMin->set(0, 0, 1);
      outputVectorMax->set(0, 0, 1);
      *outputMinTime = -1;
      *outputMaxTime = -1;
      return false;
   }

   // Once we know how long the projectile's path will take, it's straightforward to
   //  find out where it should go...
   Point3F finalAnswer = (effTargetPos / (dryVelocity * t)) + (effTargetVel / dryVelocity);
   finalAnswer.normalize();

   *outputVectorMin = finalAnswer;
   *outputVectorMax = finalAnswer;
   *outputMinTime   = t;
   *outputMaxTime   = t;

   return (t * 1000.0) <= lifetimeMS;
}


//--------------------------------------------------------------------------
bool LinearProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   // Check for validity, and fix what we can...
   //
   if (dryVelocity < 0.1) {
      Con::warnf(ConsoleLogEntry::General, "LinearProjectileData(%s)::onAdd: dryVelocity < .1, resetting", getName());
      dryVelocity = 0.1;
   }
   if (wetVelocity < 0.1 && wetVelocity != -1) {
      Con::warnf(ConsoleLogEntry::General, "LinearProjectileData(%s)::onAdd: wetVelocity < .1, resetting", getName());
      wetVelocity = 0.1;
   }
   if (lifetimeMS < TickMs || lifetimeMS > ((LinearProjectile::MaxLivingTicks-1) * TickMs)) {
      Con::warnf(ConsoleLogEntry::General, "LinearProjectileData(%s)::onAdd: lifetime out of range [%d, %d]", getName(), TickMs, ((LinearProjectile::MaxLivingTicks-1) * TickMs));
      lifetimeMS = lifetimeMS < TickMs ? TickMs : ((LinearProjectile::MaxLivingTicks-1) * TickMs);
   }
   if (fizzleTimeMS < 0) {
      Con::warnf(ConsoleLogEntry::General, "LinearProjectileData(%s)::onAdd: fizzle time < 0", getName());
      fizzleTimeMS = 0;
   }
   if (fizzleTimeMS > lifetimeMS) {
      Con::warnf(ConsoleLogEntry::General, "LinearProjectileData(%s)::onAdd: fizzleTimeMS > lifetimeMS", getName());
      fizzleTimeMS = lifetimeMS;
   }

   // Bound the angle variables
   if (reflectOnWaterImpactAngle > 90.0f)
      reflectOnWaterImpactAngle = 90.0f;
   if (reflectOnWaterImpactAngle < 0.0f)
      reflectOnWaterImpactAngle = -1.0f;
   if (deflectionOnWaterImpact > 90.0f)
      deflectionOnWaterImpact = 90.0f;
   if (deflectionOnWaterImpact < 0.0f)
      deflectionOnWaterImpact = 0.0f;

   reflectOnWaterImpactAngle = U32(reflectOnWaterImpactAngle);
   deflectionOnWaterImpact = U32(deflectionOnWaterImpact);

   if (fizzleUnderwaterMS < 0 || fizzleUnderwaterMS > lifetimeMS)
      fizzleUnderwaterMS = -1;
   else 
      fizzleUnderwaterMS = (fizzleUnderwaterMS + TickMs - 1) & ~(TickMs - 1);

   if (activateDelayMS < 0 || activateDelayMS > lifetimeMS)
      activateDelayMS = -1;

   // Make sure lifetimeMS falls on a tick boundary
   lifetimeMS   = (lifetimeMS   + TickMs - 1) & ~(TickMs - 1);
   fizzleTimeMS = (fizzleTimeMS + TickMs - 1) & ~(TickMs - 1);

   return true;
}


bool LinearProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (bool(projectileShape) && activateDelayMS != -1) {
      activateSeq  = projectileShape->findSequence("activate");
      maintainSeq  = projectileShape->findSequence("maintain");

      if (activateSeq == -1 || maintainSeq == -1) {
         Con::warnf(ConsoleLogEntry::General,
                    "LinearProjectileData(%s)::load: activateDelayMS != -1, but no activate/maintain sequence on shape %s",
                    getName(), projectileShapeName);
         activateDelayMS = -1;
         activateSeq = -1;
         maintainSeq = -1;
      }
   }

   return true;
}

//--------------------------------------------------------------------------
void LinearProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(dryVelocity);
   stream->write(wetVelocity);
   stream->write(fizzleTimeMS);
   stream->write(lifetimeMS);
   stream->writeFlag(explodeOnDeath);
   stream->writeRangedU32(reflectOnWaterImpactAngle, 0, 90);
   stream->writeRangedU32(deflectionOnWaterImpact, 0, 90);
   stream->write(fizzleUnderwaterMS);
   stream->write(activateDelayMS);
   stream->writeFlag(doDynamicClientHits);
}

void LinearProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&dryVelocity);
   stream->read(&wetVelocity);
   stream->read(&fizzleTimeMS);
   stream->read(&lifetimeMS);
   explodeOnDeath = stream->readFlag();
   reflectOnWaterImpactAngle = stream->readRangedU32(0, 90);
   deflectionOnWaterImpact = stream->readRangedU32(0, 90);
   stream->read(&fizzleUnderwaterMS);
   stream->read(&activateDelayMS);
   doDynamicClientHits = stream->readFlag();
}

//--------------------------------------------------------------------------
//--------------------------------------
//
LinearProjectile::LinearProjectile()
{
   mActivateThread = NULL;
   mMaintainThread = NULL;

   mSplashTick = 999999999999;
   mPlayedSplash = false;

   mEndedWithDecal = false;
   mWetStart = false;
   mHitWater = false;

   mCurrBackDelta.set( 0.0f, 0.0f, 0.0f );
   mCurrDeltaBase.set( 0.0f, 0.0f, 0.0f );
}

LinearProjectile::~LinearProjectile()
{

}

//--------------------------------------------------------------------------
void LinearProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


void LinearProjectile::consoleInit()
{
   //
}


//--------------------------------------------------------------------------
bool LinearProjectile::calculateImpact(float    simTime,
							                  Point3F& pointOfImpact,
                                       float&   impactTime)
{
   AssertFatal(isServerObject() == true, "Error, bad assumption.  This should only be called on the server...");

   if (mHidden == true) {
      impactTime = 0;
      pointOfImpact.set(0, 0, 0);
      return false;
   }

   Point3F startPt;
   getTransform().getColumn(3, &startPt);

   U32 forwardTicks = U32((simTime * 1000.0f) / TickMs);
   if ((mCurrTick + forwardTicks) * TickMs > mSegments[0].msEnd) {
      // Hit a wall or the terrain...
      U32 currMS = mCurrTick * TickMs;
      impactTime = (F32(mSegments[0].msEnd) - F32(currMS)) / 1000.0;
      if (impactTime < 0.0)
         impactTime = 0.0;
      pointOfImpact = mSegments[0].end;
      return true;
   }

   Point3F endPt = deriveExactPosition(mCurrTick + forwardTicks);

   U32 mask = PlayerObjectType | TerrainObjectType | InteriorObjectType | WaterObjectType;
   RayInfo rayInfo;
   if (! gServerContainer.castRay(startPt, endPt, mask, &rayInfo))
   {
      impactTime = 0;
      pointOfImpact.set(0, 0, 0);
      return false;
   }
   else
   {
      Point3F currVel = deriveExactVelocity(mCurrTick);

      pointOfImpact = rayInfo.point;
      float distToImpact = (pointOfImpact - startPt).len();
      if (distToImpact > 1.0f && !currVel.isZero())
         impactTime = distToImpact / currVel.len();
      else
         impactTime = 0.0f;
      return true;
   }
}

bool LinearProjectile::determineWetStart()
{
   SimpleQueryList sql;
   if (isServerObject())
      gServerSceneGraph->getWaterObjectList(sql);
   else
      gClientSceneGraph->getWaterObjectList(sql);

   for (U32 i = 0; i < sql.mList.size(); i++)
   {
      WaterBlock* pBlock = dynamic_cast<WaterBlock*>(sql.mList[i]);
      if (pBlock && pBlock->isPointSubmergedSimple(mInitialPosition))
         return true;
   }

   return false;
}



//--------------------------------------------------------------------------
bool LinearProjectile::createSegments()
{
   // Assumption: a linear projectile can have at most two segments.
   // Not handled in this version: compressed intial position

   // First, check to see if the fire point is wet...
   mWetStart = determineWetStart();

   Point3F compressedVelocity(0, 0, 0);
   if (isServerObject()) {
      if (mWetStart == false)
         compressedVelocity += (BitStream::dumbDownNormal(mInitialDirection, InitialDirectionBits) *
                                mDataBlock->dryVelocity);
      else
         compressedVelocity += (BitStream::dumbDownNormal(mInitialDirection, InitialDirectionBits) *
                                mDataBlock->wetVelocity);
      compressedVelocity += mExcessDirDumb * F32(mExcessVel);
   } else {
      if (mWetStart == false)
         compressedVelocity += mInitialDirection * mDataBlock->dryVelocity;
      else
         compressedVelocity += mInitialDirection * mDataBlock->wetVelocity;
      compressedVelocity += mExcessDirDumb * F32(mExcessVel);
   }

   mSegments[0].start = mInitialPosition;
   mSegments[0].end   = mInitialPosition + compressedVelocity * (F32(mDataBlock->lifetimeMS) / 1000.0);
   mSegments[0].segmentVel = compressedVelocity;

   RayInfo rInfo;
   rInfo.object = NULL;
   Container* pContainer = isServerObject() ? &gServerContainer : &gClientContainer;
   if (pContainer->castRay(mSegments[0].start, mSegments[0].end,
                           csmStaticCollisionMask, &rInfo) == false) {
      // Didn't hit anything, lifetime goes to the end...
      mSegments[0].msStart = 0;
      mSegments[0].msEnd   = mDataBlock->lifetimeMS;

      mSegments[0].cutShort  = false;
      mSegments[0].endNormal = compressedVelocity;
      mSegments[0].endNormal.normalize();
      mSegments[0].endNormal.neg();
   } else {
      // Hit something, cut the segment short...
      mSegments[0].msStart   = 0;
      mSegments[0].msEnd     = U32(mDataBlock->lifetimeMS * rInfo.t);
      mSegments[0].end       = rInfo.point;

      mSegments[0].cutShort    = true;
      mSegments[0].endNormal   = rInfo.normal;
      mSegments[0].endTypeMask = rInfo.object->getTypeMask();
   }

   mSegments[0].hitObj = rInfo.object;

   mNumSegments = 1;

   if (mWetStart == false) {
      // If we started in air, then we potentially have to modify the segments if
      //  we collide with a water block...
      if (pContainer->castRay(mSegments[0].start, mSegments[0].end, WaterObjectType, &rInfo)) {
         // We're going to have to fix this...

         if (mDataBlock->explodeOnWaterImpact) {

            // check if we actually hit water
            bool hitWater = false;

            WaterBlock* waterBlock = dynamic_cast<WaterBlock*>(rInfo.object);
            if( waterBlock )
            {
               if( waterBlock->isWater( waterBlock->getLiquidType() ) )
               {
                  hitWater = true;
               }
            }

            // Check to see if we explode or bounce off...
            Point3F normVel = compressedVelocity;
            normVel.normalize();
            if (mFabs(mDot(normVel, rInfo.normal)) >= mCos(mDegToRad(90 - mDataBlock->reflectOnWaterImpactAngle)) || !hitWater ) {
               // Explode
               mHitWater = true;
               mSegments[0].msEnd     = (U32)(mSegments[0].msEnd * rInfo.t);
               mSegments[0].end       = rInfo.point;
               mSegments[0].cutShort  = true;
               mSegments[0].endNormal = rInfo.normal;
               mSegments[0].endTypeMask = rInfo.object->getTypeMask();

               mDeleteTick = (mSegments[0].msEnd / TickMs) + DeleteWaitTicks;
            } else {
               mSegments[0].msEnd     = (U32)(mSegments[0].msEnd * rInfo.t);
               mSegments[0].end       = rInfo.point;
               mSegments[0].endNormal = rInfo.normal;
               mSegments[0].cutShort  = false;

               Point3F n = rInfo.normal;
               MatrixF refMatrix(true);
               F32* ra = refMatrix;
               ra[0]  = 1.0f - 2.0f*(n.x*n.x); ra[1]  = 0.0f - 2.0f*(n.x*n.y); ra[2]  = 0.0f - 2.0f*(n.x*n.z); ra[3]  = 0.0f;
               ra[4]  = 0.0f - 2.0f*(n.y*n.x); ra[5]  = 1.0f - 2.0f*(n.y*n.y); ra[6]  = 0.0f - 2.0f*(n.y*n.z); ra[7]  = 0.0f;
               ra[8]  = 0.0f - 2.0f*(n.z*n.x); ra[9]  = 0.0f - 2.0f*(n.z*n.y); ra[10] = 1.0f - 2.0f*(n.z*n.z); ra[11] = 0.0f;
               ra[12] = 0;
               ra[13] = 0;
               ra[14] = 0;
               ra[15] = 1.0f;
               // the GGems series (as of v1) uses row vectors (arg)
               refMatrix.transpose();

               // Reflect the velocity around the normal...
               Point3F refVelocity = compressedVelocity;
               refMatrix.mulV(refVelocity);


               Point3F newStart = mSegments[0].end;
               Point3F newEnd   = newStart;
               newEnd          += refVelocity * (F32(mDataBlock->lifetimeMS - mSegments[0].msEnd) / 1000);

               if (pContainer->castRay(newStart, newEnd,
                                       csmStaticCollisionMask, &rInfo) == false) {
                  // Didn't hit anything, lifetime goes to the end...
                  mSegments[1].msStart = mSegments[0].msEnd;
                  mSegments[1].msEnd   = mDataBlock->lifetimeMS;
                  mSegments[1].start   = mSegments[0].end;
                  mSegments[1].end     = newEnd;

                  mSegments[1].cutShort  = false;
                  mSegments[1].endNormal = refVelocity;
                  mSegments[1].endNormal.normalize();
                  mSegments[1].endNormal.neg();
                  mSegments[1].segmentVel = refVelocity;
               } else {
                  // Hit something, cut the segment short...
                  mSegments[1].msStart = mSegments[0].msEnd;
                  mSegments[1].msEnd   = (U32)(mSegments[1].msStart + (mDataBlock->lifetimeMS - mSegments[0].msEnd) * rInfo.t);
                  mSegments[1].start   = mSegments[0].end;
                  mSegments[1].end     = rInfo.point;

                  mSegments[1].cutShort    = true;
                  mSegments[1].endNormal   = rInfo.normal;
                  mSegments[1].endTypeMask = rInfo.object->getTypeMask();
                  mSegments[1].segmentVel  = refVelocity;
               }

               mDeleteTick = (mSegments[1].msEnd / TickMs) + DeleteWaitTicks;
               mNumSegments = 2;
            }
         } else {
            // Continue through, at wetVelocity...
            mSegments[1].start  = rInfo.point;
            mSegments[0].end    = rInfo.point;
            mSegments[0].msEnd *= rInfo.t;
            mSegments[0].endNormal = rInfo.normal;
            mSegments[0].cutShort = false;

            compressedVelocity.set(0, 0, 0);
            if (isServerObject()) {
               compressedVelocity += (BitStream::dumbDownNormal(mInitialDirection, InitialDirectionBits) *
                                      mDataBlock->wetVelocity);
               compressedVelocity += mExcessDirDumb * F32(mExcessVel);
            } else {
               compressedVelocity += mInitialDirection * mDataBlock->wetVelocity;
               compressedVelocity += mExcessDirDumb * F32(mExcessVel);
            }

            Point3F newEnd = mSegments[1].start;
            newEnd += compressedVelocity * (F32(mDataBlock->lifetimeMS - mSegments[0].msEnd) / 1000.0);

            if (pContainer->castRay(mSegments[1].start, newEnd,
                                    csmStaticCollisionMask, &rInfo) == false) {
               // Didn't hit anything, lifetime goes to the end...
               mSegments[1].msStart = mSegments[0].msEnd;
               mSegments[1].msEnd   = mDataBlock->lifetimeMS;
               mSegments[1].end     = newEnd;

               mSegments[1].cutShort  = false;
               mSegments[1].endNormal = compressedVelocity;
               mSegments[1].endNormal.normalize();
               mSegments[1].endNormal.neg();
            } else {
               // Hit something, cut the segment short...
               mSegments[1].msStart = mSegments[0].msEnd;
               mSegments[1].msEnd   = (U32)(mSegments[1].msStart + (mDataBlock->lifetimeMS - mSegments[0].msEnd) * rInfo.t);
               mSegments[1].end     = rInfo.point;

               mSegments[1].cutShort    = true;
               mSegments[1].endNormal   = rInfo.normal;
               mSegments[1].endTypeMask = rInfo.object->getTypeMask();
            }

            mSegments[1].hitObj = rInfo.object;
            mSegments[1].segmentVel = compressedVelocity;

            mNumSegments = 2;
            mDeleteTick = (mSegments[1].msEnd / TickMs) + DeleteWaitTicks;
         }
      } else {
         // No water collision, we're cool...
         mDeleteTick = (mSegments[0].msEnd / TickMs) + DeleteWaitTicks;
      }
   } else {
      mDeleteTick = (mSegments[0].msEnd / TickMs) + DeleteWaitTicks;
   }

   AssertFatal(mSegments[0].msEnd >= mSegments[0].msStart, "Error, 1st segment is bad!");
   if (mNumSegments > 1) {
      AssertFatal(mSegments[1].msStart == mSegments[0].msEnd, "Error, 2nd segment is bad!");
      AssertFatal(mSegments[1].msEnd >= mSegments[1].msStart, "Error, 2nd segment is bad!");
   }

   return true;
}

bool LinearProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (isServerObject()) {
      // Set the initial position
      MatrixF xform(true);
      xform.setColumn(3, mInitialPosition);
      setTransform(xform);

      // Ok, we have all the variables we need to forecast our own demise.  Do that now
      if (createSegments() == false)
      {
         return false;
      }
   } else {
      if (mHidden == false) {
         createSegments();
         calcSplash();

         if (bool(mSourceObject)) {
            // Setup the warping.  We have to take into account the fact that we might be
            //  in the middle of a tick.
            mWarpTicksRemaining = smProjectileWarpTicks;
            mWarpEnd = deriveExactPosition(mCurrTick + smProjectileWarpTicks);
            mSourceObject->getMuzzlePoint(mSourceObjectSlot, &mWarpStart);
         } else {
            mWarpTicksRemaining = 0;
         }
      } else {
         setPosition( mInitialPosition );

         mWetStart = determineWetStart();
         calcSplash();

         if( mCurrTick >= mSplashTick )
         {
            createSplash();
         }

         // All we need to do is explode...
         if (mDataBlock->explosion) {
            Explosion* pExplosion = new Explosion;
            pExplosion->onNewDataBlock(mDataBlock->explosion);

            MatrixF xform(true);
            xform.setColumn(3, mExplosionPosition);
            pExplosion->setTransform(xform);
            pExplosion->setInitialState(mExplosionPosition, mExplosionNormal, 1.0);
            if (pExplosion->registerObject() == false) {
               Con::errorf(ConsoleLogEntry::General, "LinearProjectile(%s)::explode: couldn't register explosion(%s)",
                           mDataBlock->getName(), mDataBlock->explosion->getName());
               delete pExplosion;
               pExplosion = NULL;
            } else {
               if (mEndedWithDecal == true && gClientSceneGraph->getCurrentDecalManager() && mDataBlock->numDecals != 0) {
                  // DMM: randomly choose from [0 .. numDecals - 1];
                  if (mDataBlock->decalData[0]) {
                     gClientSceneGraph->getCurrentDecalManager()->addDecal(mExplosionPosition, 
                                                                           mExplosionNormal, 
                                                                           mDataBlock->decalData[0]);
                  }
               }
            }
         }
      }
   }

   addToScene();

   return true;
}


void LinearProjectile::onRemove()
{
   if (isClientObject())
      updateSound(Point3F(0, 0, 0), Point3F(0, 0, 0), false);
   removeFromScene();

   Parent::onRemove();
}


bool LinearProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<LinearProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//----------------------------------------------------------------------------
bool LinearProjectile::updatePos(const Point3F& oldPos,
                                 const Point3F& newPos,
                                 F32*           hitDelta,
                                 Point3F*       hitPos,
                                 Point3F*       hitNormal,
                                 SceneObject*&  hitObject)
{
   if (mSourceIdTimeoutTicks && bool(mSourceObject))
      mSourceObject->disableCollision();
   if(mSourceIdTimeoutTicks && bool(mVehicleObject))
      mVehicleObject->disableCollision();
   
   RayInfo rInfo;
   if (mContainer->castRay(oldPos, newPos, csmDynamicCollisionMask, &rInfo) == true) {
      *hitPos    = rInfo.point;
      *hitNormal = rInfo.normal;
      *hitDelta  = rInfo.t;
      hitObject  = rInfo.object;

      if (mSourceIdTimeoutTicks && bool(mSourceObject))
         mSourceObject->enableCollision();
      if(mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->enableCollision();
      return false;
   } else {
      *hitDelta = 1.0;
      hitObject = NULL;

      if (mSourceIdTimeoutTicks && bool(mSourceObject))
         mSourceObject->enableCollision();
      if(mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->enableCollision();
      return true;
   }
}


//--------------------------------------------------------------------------
void LinearProjectile::explode(const Point3F& p, const Point3F& n, const bool dynamicObject)
{
   // don't explode if projectile dies on water impact - play big splash instead
   if( mHitWater && !mDataBlock->explodeOnWaterImpact )
   {
      mHidden = true;
      return;
   }

   // Already blown up
   if (mHidden == true)
      return;
   mHidden = true;

   if (isServerObject()) {
      mExplosionPosition = p + (n * 0.01);
      mExplosionNormal   = n;

      // do radius damage if appropriate
      char posBuffer[128];
      char fadeBuffer[128];
      dSprintf(posBuffer, sizeof(posBuffer),  "%f %f %f", mExplosionPosition.x,
                                                          mExplosionPosition.y,
                                                          mExplosionPosition.z);
      dSprintf(fadeBuffer, sizeof(fadeBuffer), "%f", mFadeValue);

      Con::executef(mDataBlock, 4, "onExplode", scriptThis(), posBuffer, fadeBuffer);

      if (dynamicObject)
         setMaskBits(ExplosionMask);
   } else {
      // Client just plays the explosion at the right place...
      //
      Explosion* pExplosion = new Explosion;

      F32 waterHeight;
      if( pointInWater( (Point3F &)p, &waterHeight ) && mDataBlock->underwaterExplosion )
      {
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
            pExplosion->onNewDataBlock(mDataBlock->explosion);
         }
      }

      if( pExplosion )
      {
         MatrixF xform(true);
         xform.setPosition(p);
         pExplosion->setTransform(xform);
         pExplosion->setInitialState(p, n);
         if (pExplosion->registerObject() == false)
         {
            Con::errorf(ConsoleLogEntry::General, "LinearProjectile(%s)::explode: couldn't register explosion",
                        mDataBlock->getName() );
            delete pExplosion;
            pExplosion = NULL;
         }

         if (mEndedWithDecal == true && gClientSceneGraph->getCurrentDecalManager() && mDataBlock->numDecals != 0) {
            // DMM: randomly choose from [0 .. numDecals - 1];
            if (mDataBlock->decalData[0])
               gClientSceneGraph->getCurrentDecalManager()->addDecal(p, n, mDataBlock->decalData[0]);
         }
      }

      // Client object
      updateSound(Point3F(0, 0, 0), Point3F(0, 0, 0), false);
   }
}


//--------------------------------------------------------------------------
Point3F LinearProjectile::getVelocity() const
{
   return deriveExactVelocity(mCurrTick);
}


//--------------------------------------------------------------------------
void LinearProjectile::processTick(const Move* move)
{
   Parent::processTick(move);
   AssertFatal(mCurrTick >= 1, "Hm, bad assumption somewhere");

   if (isServerObject()) {
      if (mCurrTick >= mDeleteTick) {
         deleteObject();
         return;
      }
      else if (mHidden == true) {
         // Already exploded
         return;
      }
      else if (((mCurrTick - 1) * TickMs) >= mSegments[mNumSegments - 1].msEnd) {
         if (mSegments[mNumSegments - 1].cutShort == true && (mSegments[mNumSegments - 1].endTypeMask & csmDecalMask))
            mEndedWithDecal = true;

         if (mSegments[mNumSegments - 1].cutShort == true || mDataBlock->explodeOnDeath == true)
         {
            SceneObject *so = mSegments[mNumSegments-1].hitObj;
            if( so )
            {
               Point3F normal = mSegments[mNumSegments-1].endNormal;
               onCollision( mSegments[mNumSegments-1].end, normal, so );
            }
            explode(mSegments[mNumSegments - 1].end, mSegments[mNumSegments - 1].endNormal, false);
         }
         else
         {
            // End of the line without explosion.  Just delete the object.  If we don't, the
            //  current unpackUpdate logic can break the client, since it interprets hidden
            //  on initial update to mean: explosion.
            mHidden = true;
            deleteObject();
         }

         return;
      }

      // Otherwise update our position
      Point3F oldPos;
      getTransform().getColumn(3, &oldPos);
      Point3F newPos = deriveExactPosition(mCurrTick);

      F32 hitDelta;
      Point3F hitPos, hitNormal;
      SceneObject* hitObject;
      if (updatePos(oldPos, newPos, &hitDelta, &hitPos, &hitNormal, hitObject) == false) {
         // Hit something dynamic in this pass

         MatrixF xform(true);
         xform.setColumn(3, hitPos);
         setTransform(xform);

         onCollision(hitPos, hitNormal, hitObject);

         if (hitObject->getTypeMask() & csmDecalMask)
            mEndedWithDecal = true;
         explode(hitPos, hitNormal, true);
      } else {
         // No hit, continue on...
         MatrixF xform(true);
         xform.setColumn(3, newPos);
         setTransform(xform);
      }
   } else {
      if( mCurrTick >= mSplashTick )
      {
         createSplash();
      }

      if (mHidden == true)
         return;
      else if (((mCurrTick - 1) * TickMs) >= mSegments[mNumSegments - 1].msEnd) {
         if (mSegments[mNumSegments - 1].cutShort == true && (mSegments[mNumSegments - 1].endTypeMask & csmDecalMask))
            mEndedWithDecal = true;

         if (mSegments[mNumSegments - 1].cutShort == true || mDataBlock->explodeOnDeath == true) {
            explode(mSegments[mNumSegments - 1].end, mSegments[mNumSegments - 1].endNormal, false);
         } else {
            mHidden = true;
         }

         return;
      }

      Point3F currPos;
      Point3F nextPos;

      if (mWarpTicksRemaining) {
         currPos = mWarpStart * (mWarpTicksRemaining) +
                   mWarpEnd   * (smProjectileWarpTicks - mWarpTicksRemaining);
         nextPos = mWarpStart * (mWarpTicksRemaining - 1) +
                   mWarpEnd   * (smProjectileWarpTicks - mWarpTicksRemaining + 1);

         currPos /= smProjectileWarpTicks;
         nextPos /= smProjectileWarpTicks;

         mWarpTicksRemaining--;
      }
      else {
         // Normal update...
         currPos = deriveExactPosition(mCurrTick - 1);
         nextPos = deriveExactPosition(mCurrTick);
      }

      Point3F partVel = deriveExactVelocity(mCurrTick);
      if (partVel.isZero())
         partVel = mSegments[mNumSegments - 1].segmentVel;

      emitParticles(currPos, nextPos, partVel, TickMs);

      // Check for collision
      F32 hitDelta;
      Point3F hitPos, hitNormal;
      SceneObject* hitObject;
      if (mDataBlock->doDynamicClientHits == true &&
          updatePos(currPos, nextPos, &hitDelta, &hitPos, &hitNormal, hitObject) == false) {
         if (hitObject->getTypeMask() & csmDecalMask)
            mEndedWithDecal = true;

         // Hit something dynamic in this pass
         explode(hitPos, hitNormal, true);

         mCurrBackDelta     = currPos - hitPos;
         mCurrDeltaBase     = hitPos;
         mSegments[0].msEnd = (U32)(mCurrTick * TickMs - (hitDelta * TickMs));
         mSegments[1].msEnd = (U32)(mCurrTick * TickMs - (hitDelta * TickMs));
      }
      else {
         // No hit, continue on...
         mCurrBackDelta = currPos - nextPos;
         mCurrDeltaBase = nextPos;
      }
      MatrixF temp(true);
      temp.setColumn(3, mCurrDeltaBase);
      setTransform(temp);
      
      // If we're past our activate time, create our activate sequence
      if (mDataBlock->activateDelayMS != -1 && mProjectileShape != NULL &&
          mCurrTick * TickMs >= mDataBlock->activateDelayMS &&
          !mActivateThread) {
         mActivateThread = mProjectileShape->addThread();
         mProjectileShape->setTimeScale(mActivateThread, 1);
         mProjectileShape->setSequence(mActivateThread, mDataBlock->activateSeq, 0);
      }
   }
}

void LinearProjectile::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);

   // Client object.  Must check to see if there are any warp ticks left...
   if (mHidden == true)
      return;

   Point3F newPos = mCurrDeltaBase + (mCurrBackDelta * delta);
   MatrixF xform(true);

   Point3F dir = deriveExactVelocity(getMax(S32(mCurrTick) - 1, 0));
   if( dir.isZero() )
   {
      dir.set( 0.0, 0.0, 1.0 );  // odd but prevents crash
   }
   else
   {
      dir.normalize();
   }

   xform = MathUtils::createOrientFromDir( dir );
   xform.setPosition( newPos );
   setRenderTransform( xform );


   S32 currMs = (S32)(mCurrTick * TickMs - (delta * TickMs));
   if (currMs > mDataBlock->fizzleTimeMS) {
      U32 fizzle = currMs - mDataBlock->fizzleTimeMS;
      mFadeValue = 1.0 - (F32(fizzle) /
                          F32(mDataBlock->lifetimeMS - mDataBlock->fizzleTimeMS));
   } else {
      mFadeValue = 1.0;
   }

   updateSound(newPos, deriveExactVelocity(mCurrTick), true);
}

void LinearProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if (mHidden == true || dt == 0.0)
      return;

   if (mActivateThread &&
       mProjectileShape->getDuration(mActivateThread) > mProjectileShape->getTime(mActivateThread) + dt) {
      mProjectileShape->advanceTime(dt, mActivateThread);
   } else {
      if (mMaintainThread) {
         mProjectileShape->advanceTime(dt, mMaintainThread);
      } else if (mActivateThread && mDataBlock->maintainSeq != -1) {
         mMaintainThread = mProjectileShape->addThread();
         mProjectileShape->setTimeScale(mMaintainThread, 1);
         mProjectileShape->setSequence(mMaintainThread, mDataBlock->maintainSeq, 0);
         mProjectileShape->advanceTime(dt, mMaintainThread);
      }
   }
}


//--------------------------------------------------------------------------
U32 LinearProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & GameBase::InitialUpdateMask)) {
      // Initial update

      // Write: initialPosition
      //        initialVector (compressed to 10b standard)
      //        currServerTime
      //        if (sourceId)
      //           sourceId
      //           sourceSlot
      if (stream->writeFlag(mHidden) == false) {
         con->writeCompressed(stream, mInitialPosition);
         stream->writeNormalVector(mInitialDirection, InitialDirectionBits);
         stream->writeRangedU32(mCurrTick, 0, MaxLivingTicks);

         // Potentially have to write this to the client, let's make sure it has a
         //  ghost on the other side...
         S32 ghostIndex = -1;
         if (bool(mSourceObject))
            ghostIndex = con->getGhostIndex(mSourceObject);

         if (stream->writeFlag(ghostIndex != -1))
         {
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount - 1);
            stream->writeRangedU32(U32(mSourceObjectSlot),
                                   0, ShapeBase::MaxMountedImages - 1);

            if (stream->writeFlag(mExcessVel != 0))
            {
               stream->writeRangedU32(mExcessVel, 0, 255);
               stream->writeNormalVector(mExcessDir, ExcessVelDirBits);
            }
         }

         if (bool(mVehicleObject)) {
            // Potentially have to write this to the client, let's make sure it has a
            //  ghost on the other side...
            S32 ghostIndex = con->getGhostIndex(mVehicleObject);
            if (stream->writeFlag(ghostIndex != -1)) {
               stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount - 1);
            }
         } else {
            stream->writeFlag(false);
         }
      } else {
         // We've already exploded.  Write out our explosion position and normal...
         con->writeCompressed(stream, mExplosionPosition);
         stream->writeNormalVector(mExplosionNormal, InitialDirectionBits);
         stream->writeFlag(mEndedWithDecal);
      }
   } else {
      // Explosion update
      AssertFatal((mask & ExplosionMask) != 0,
                  "LinearProjectile::packUpdate: only two types of packets, initial and explosion, this is neither?");
      con->writeCompressed(stream, mExplosionPosition);
      stream->writeNormalVector(mExplosionNormal, InitialDirectionBits);
      stream->writeFlag(mEndedWithDecal);
   }

   return retMask;
}

void LinearProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) {
      // Initial update
      if (stream->readFlag() == false) {
         con->readCompressed(stream, &mInitialPosition);
         stream->readNormalVector(&mInitialDirection, InitialDirectionBits);
         mCurrTick = stream->readRangedU32(0, MaxLivingTicks);

         if (stream->readFlag()) {
            mSourceObjectId   = stream->readRangedU32(0, NetConnection::MaxGhostCount - 1);
            mSourceObjectSlot = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);

            NetObject* pObject = con->resolveGhost(mSourceObjectId);
            if (pObject != NULL)
               mSourceObject = dynamic_cast<ShapeBase*>(pObject);

            if (bool(mSourceObject) == false)
               Con::errorf(ConsoleLogEntry::General, "LinearProjectile::unpackUpdate: could not resolve source ghost properly on initial update");

            if (stream->readFlag()) {
               mExcessVel = stream->readRangedU32(0, 255);
               stream->readNormalVector(&mExcessDir, ExcessVelDirBits);
            } else {
               mExcessDir.set(0, 0, 1);
               mExcessVel = 0;
            }
            mExcessDirDumb = mExcessDir;
         } else {
            mSourceObjectId   = -1;
            mSourceObjectSlot = -1;
            mSourceObject     = NULL;

            mExcessDir.set(0, 0, 1);
            mExcessDirDumb = mExcessDir;
            mExcessVel = 0;
         }

         if (stream->readFlag()) {
            mVehicleObjectId  = stream->readRangedU32(0, NetConnection::MaxGhostCount - 1);
            NetObject* pObject = con->resolveGhost(mVehicleObjectId);
            if (pObject != NULL)
               mVehicleObject = dynamic_cast<ShapeBase*>(pObject);
            if (bool(mVehicleObject) == false)
               Con::errorf(ConsoleLogEntry::General, "LinearProjectile::unpackUpdate: could not resolve source ghost properly on initial update");
         } else {
            mVehicleObjectId = 0;
            mVehicleObject   = NULL;
         }
      } else {
         mHidden = true;
         con->readCompressed(stream, &mExplosionPosition);
         stream->readNormalVector(&mExplosionNormal, InitialDirectionBits);
         mInitialPosition = mExplosionPosition + mExplosionNormal;
         mInitialDirection = - mExplosionNormal;

         mEndedWithDecal = stream->readFlag();
      }
   } else {
      // Explosion notification
      con->readCompressed(stream, &mExplosionPosition);
      stream->readNormalVector(&mExplosionNormal, InitialDirectionBits);
      mEndedWithDecal = stream->readFlag();
      explode(mExplosionPosition, mExplosionNormal, false);
   }
}


//--------------------------------------------------------------------------
Point3F LinearProjectile::deriveExactPosition(const U32 tick) const
{
   U32 currMs = tick * TickMs;

   // The hard way...
   if (currMs == 0) {
      return mSegments[0].start;
   } else if (currMs <= mSegments[0].msEnd) {
      return mSegments[0].start + (mSegments[0].segmentVel * (F32(currMs) / 1000.0f));
   } else if (mNumSegments > 1 && currMs <= mSegments[1].msEnd) {
      AssertFatal(currMs > mSegments[1].msStart, "Error, should have been greater here");
      return mSegments[1].start + (mSegments[1].segmentVel * (F32(currMs - mSegments[1].msStart) / 1000.0f));
   } else {
      if (mNumSegments == 1) {
         return mSegments[0].end;
      } else {
         return mSegments[1].end;
      }
   }
}

//--------------------------------------------------------------------------
Point3F LinearProjectile::deriveExactVelocity(const U32 tick) const
{
   U32 currMs = tick * TickMs;

   if (currMs < mSegments[0].msEnd)
      return mSegments[0].segmentVel;
   else if (mNumSegments > 1 && currMs < mSegments[1].msEnd)
      return mSegments[1].segmentVel;
   else {
      return Point3F(0, 0, 0);
   }
}

//--------------------------------------------------------------------------
void LinearProjectile::calcSplash()
{
   Point3F vel = mInitialDirection;

   if( mWetStart )
   {
      vel *= mDataBlock->wetVelocity;
   }
   else
   {
      vel *= mDataBlock->dryVelocity;
   }

   // If the hidden flag is set at this point then the server has indicated that the
   // collision point is so close that the projectile has already hit and exploded.
   // It also means the mExcess variables haven't come down to the client and are invalid
   if( !mHidden )
   {
      vel += mExcessDirDumb * F32(mExcessVel);
   }

   Point3F start = mInitialPosition;
   Point3F end = mInitialPosition + vel * (F32(mDataBlock->lifetimeMS) / 1000.0);
   
   bool startUnder = pointInWater( start );
   bool endUnder = pointInWater( end );

   mSplashTick = U32(-1);

   if( startUnder && endUnder )
   {
      return;
   }

   RayInfo rInfo;

   if( startUnder )
   {
      if( gClientContainer.castRay(end, start, WaterObjectType, &rInfo) )
      {
         mSplashPos = rInfo.point;
         mSplashTick = U32(mDataBlock->lifetimeMS * (1.0 - rInfo.t) / TickMs);
      }
   }
   else
   {
      if( gClientContainer.castRay(start, end, WaterObjectType, &rInfo) )
      {
         mSplashPos = rInfo.point;
         mSplashTick = U32(mDataBlock->lifetimeMS * rInfo.t / TickMs);
      }
   }
}

//--------------------------------------------------------------------------
void LinearProjectile::renderObject(SceneState* state, SceneRenderImage* sri)
{
   if( !mHitWater || !mPlayedSplash )
   {
      Parent::renderObject( state, sri );
   }
}

//--------------------------------------------------------------------------
void LinearProjectile::createSplash()
{
   if( mDataBlock->splash && !mPlayedSplash )
   {
      MatrixF trans = getTransform();
      trans.setPosition( mSplashPos );
      Splash *splash = new Splash;
      splash->onNewDataBlock( mDataBlock->splash );
      splash->setTransform( trans );
      splash->setInitialState( trans.getPosition(), Point3F( 0.0, 0.0, 1.0 ) );
      if (!splash->registerObject())
         delete splash;

      mPlayedSplash = true;
   }
}
