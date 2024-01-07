//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projELF.h"
#include "core/bitStream.h"
#include "platformWIN32/platformGL.h"
#include "dgl/dgl.h"
#include "scenegraph/sceneState.h"
#include "console/consoleTypes.h"
#include "game/shapeBase.h"
#include "game/gameConnection.h"
#include "math/mRandom.h"
#include "math/mQuadPatch.h"
#include "dgl/splineUtil.h"
#include "game/particleEngine.h"

#define LIGHTNING_FREQ 30.0


IMPLEMENT_CO_DATABLOCK_V1(ELFProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(ELFProjectile);

//--------------------------------------------------------------------------
//--------------------------------------
//
ELFProjectileData::ELFProjectileData()
{
   beamRange         = 10.0f;
   beamHitWidth      = 30;
   mainBeamWidth     = 0.2;
   mainBeamSpeed     = 9.0;
   mainBeamRepeat    = 0.25;
   lightningWidth    = 0.15;
   lightningDist     = 0.15;

   textureNames[0] = "";
   textureNames[1] = "";
   textureNames[2] = "";

   dMemset( emitterList, 0, sizeof( emitterList ) );
   dMemset( emitterIDList, 0, sizeof( emitterIDList ) );
}

ELFProjectileData::~ELFProjectileData()
{

}


//--------------------------------------------------------------------------
void ELFProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("beamRange",         TypeF32,    Offset(beamRange,         ELFProjectileData));
   addField("beamHitWidth",      TypeF32,    Offset(beamHitWidth,      ELFProjectileData));
   addField("textures",          TypeString, Offset(textureNames,      ELFProjectileData), ET_NUM_TEX);
   addField("mainBeamWidth",     TypeF32,    Offset(mainBeamWidth,     ELFProjectileData));
   addField("mainBeamSpeed",     TypeF32,    Offset(mainBeamSpeed,     ELFProjectileData));
   addField("mainBeamRepeat",    TypeF32,    Offset(mainBeamRepeat,    ELFProjectileData));
   addField("lightningWidth",    TypeF32,    Offset(lightningWidth,    ELFProjectileData));
   addField("lightningDist",     TypeF32,    Offset(lightningDist,     ELFProjectileData));
   addField("emitter",           TypeParticleEmitterDataPtr,  Offset(emitterList,     ELFProjectileData), ELF_NUM_EMITTERS );

}


//--------------------------------------------------------------------------
bool ELFProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (beamRange < 2.0) {
      Con::warnf(ConsoleLogEntry::General, "ELFProjectileData(%s)::onAdd: beamRange must be >= 2", getName());
      beamRange = 2.0;
   }
   if (beamHitWidth < 0.0 || beamHitWidth > 90) {
      Con::warnf(ConsoleLogEntry::General, "ELFProjectileData(%s)::onAdd: beamHitWidth must be in range [0, 90]", getName());
      beamHitWidth = beamHitWidth < 0 ? 0 : 90.0;
   }

   cosBeamHitWidth = mCos(mDegToRad(beamHitWidth));
   
   U32 i;
   for( i=0; i<ELF_NUM_EMITTERS; i++ )
   {
      if( !emitterList[i] && emitterIDList[i] != 0 )
      {
         if( Sim::findObject( emitterIDList[i], emitterList[i] ) == false)
         {
            Con::errorf( ConsoleLogEntry::General, "ELFProjectileData::onAdd: Invalid packet, bad datablockId(particle emitter): 0x%x", emitterIDList[i] );
         }
      }
   }

   return true;
}


bool ELFProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if( server == false )
   {
      for( int i=0; i<ET_NUM_TEX; i++ )
      {
         if( textureNames[i][0] )
         {
            textureList[i] = TextureHandle( textureNames[i], MeshTexture );
         }
      }

   }
   else
   {
      for( int i=0; i<ET_NUM_TEX; i++ )
      {
         textureList[i] = TextureHandle();  // set default NULL tex
      }
   }
   
   return true;
}


bool ELFProjectileData::calculateAim(const Point3F& targetPos,
                                     const Point3F& /*targetVel*/,
                                     const Point3F& sourcePos,
                                     const Point3F& /*sourceVel*/,
                                     Point3F*       outputVectorMin,
                                     F32*           outputMinTime,
                                     Point3F*       outputVectorMax,
                                     F32*           outputMaxTime)
{
   if ((targetPos - sourcePos).len() > beamRange) {
      outputVectorMin->set(0, 0, 1);
      outputVectorMax->set(0, 0, 1);
      *outputMinTime = -1;
      *outputMaxTime = -1;

      return false;
   }

   *outputVectorMin = targetPos - sourcePos;
   *outputMinTime   = 0.0;
   outputVectorMin->normalize();
   *outputVectorMax = *outputVectorMin;
   *outputMaxTime   = *outputMinTime;

   return true;
}


//--------------------------------------------------------------------------
void ELFProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(beamRange);
   stream->write(mainBeamWidth);
   stream->write(mainBeamSpeed);
   stream->write(mainBeamRepeat);
   stream->write(lightningWidth);
   stream->write(lightningDist);

   S32 i;
   for( i=0; i<ET_NUM_TEX; i++ )
   {
      stream->writeString( textureNames[i] );
   }

   for( i=0; i<ELF_NUM_EMITTERS; i++ )
   {
      if( stream->writeFlag( emitterList[i] != NULL ) )
      {
         stream->writeRangedU32( emitterList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }
}

void ELFProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&beamRange);
   stream->read(&mainBeamWidth);
   stream->read(&mainBeamSpeed);
   stream->read(&mainBeamRepeat);
   stream->read(&lightningWidth);
   stream->read(&lightningDist);


   U32 i;
   for( i=0; i<ET_NUM_TEX; i++ )
   {
     textureNames[i] = stream->readSTString();
   }

   for( i=0; i<ELF_NUM_EMITTERS; i++ )
   {
      if( stream->readFlag() )
      {
         emitterIDList[i] = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
      }
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
ELFProjectile::ELFProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable|ScopeAlways);

   mElapsedTime = 0.0;
   mBoltTime = 0.0;
   mNewBolt = true;
   mWetFireHandle = 0;
   mFireHandle = 0;
   mEmittingParticles = false;
   
   dMemset( mEmitterList, 0, sizeof( mEmitterList ) );
}

ELFProjectile::~ELFProjectile()
{
}

//--------------------------------------------------------------------------
void ELFProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


//--------------------------------------------------------------------------
bool ELFProjectile::calculateImpact(float    /*simTime*/,
                                    Point3F& pointOfImpact,
                                    float&   impactTime)
{
   impactTime = 0;

   if (bool(mTargetObject)) {
      mTargetObject->getWorldBox().getCenter(&pointOfImpact);
      return true;
   } else {
      impactTime = 0;
      pointOfImpact.set(0, 0, 0);
      return false;
   }
}


//--------------------------------------------------------------------------
bool ELFProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   mObjBox.min.set(-1e7, -1e7, -1e7);
   mObjBox.max.set( 1e7,  1e7,  1e7);
   resetWorldBox();
   addToScene();

   if (isServerObject())
   {
      // The first order of business is to establish whether or not we really
      //  have a target...
      findTarget();
   }
   else
   {
      for( int i=0; i<ELFProjectileData::ELF_NUM_EMITTERS; i++ )
      {
         mEmitterList[i] = new ParticleEmitter;
         mEmitterList[i]->onNewDataBlock( mDataBlock->emitterList[i] );
         if( !mEmitterList[i]->registerObject() )
         {
            Con::warnf( ConsoleLogEntry::General, "Could not register dust emitter for class: %s", mDataBlock->getName() );
            delete mEmitterList[i];
            mEmitterList[i] = NULL;
         }
      }
   }
   
   
   
   return true;
}


void ELFProjectile::onRemove()
{
   if (isServerObject() && bool(mTargetObject))
      releaseTarget(mTargetObject);

   removeFromScene();
   if(mWetFireHandle)
      alxStop(mWetFireHandle);
   if(mFireHandle)
      alxStop(mFireHandle);

   U32 i;
   for( i=0; i<ELFProjectileData::ELF_NUM_EMITTERS; i++ )
   {
      if( mEmitterList[i] )
      {
         mEmitterList[i]->deleteWhenEmpty();
         mEmitterList[i] = NULL;
      }
   }

   Parent::onRemove();
}


bool ELFProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<ELFProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   // Todo: Uncomment if this is a "leaf" class
   //scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
void ELFProjectile::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isServerObject())
   {
      findTarget();
   }
}


void ELFProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if (bool(mTargetObject) == false || bool(mSourceObject) == false)
      return;

   mElapsedTime += dt;
   mBoltTime += dt;
   if( mBoltTime > 1.0/LIGHTNING_FREQ )
   {
      mBoltTime -= 1.0/LIGHTNING_FREQ;
      mNewBolt = true;
   }

   updateEmitters( dt );
   
   mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mInitialPosition);
   mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mInitialDirection);

   // make sure mCurrEndPoint is current
   ShapeBase *obj = mTargetObject;
   calcSnapPoint( *obj, mCurrEndPoint );
}


//--------------------------------------------------------------------------
bool ELFProjectile::prepRenderImage(SceneState* state, const U32 stateKey,
                                    const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   if (bool(mTargetObject) == false || bool(mSourceObject) == false)
      return false;

   SceneRenderImage* image = new SceneRenderImage;
   image->obj = this;
   image->isTranslucent = true;
   image->sortType = SceneRenderImage::EndSort;
   state->insertRenderImage(image);

   return false;
}

void ELFProjectile::acquireTarget(ShapeBase* target)
{
   char buff[16];
   dSprintf(buff, sizeof(buff), "%d", mSourceObjectId);
   Con::executef(mDataBlock, 4, "zapTarget", scriptThis(), target->scriptThis(), buff);
}

void ELFProjectile::releaseTarget(ShapeBase* target)
{
   char buff[16];
   dSprintf(buff, sizeof(buff), "%d", mSourceObjectId);
   Con::executef(mDataBlock, 4, "unzapTarget", scriptThis(), target->scriptThis(), buff);
}

void ELFProjectile::findTarget()
{
   AssertFatal(bool(mSourceObject), "Error, can't find a target without a source!");
   AssertFatal(getContainer(), "Error, must have a container");
   AssertFatal(isServerObject(), "Clients may not find their own targets...");
   if (bool(mSourceObject) == false)
      return;

   mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mInitialPosition);
   mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mInitialDirection);

   // hack-o-rama : this fixes problem where client and server have different mInitialPosition
   // possibly from player breathing animations.  Without this, the player can shoot object
   // through floor at certain angles.
   mInitialPosition.z -= 0.4;

   MatrixF xform(true);
   xform.setColumn(3, mInitialPosition);

   // See if there is something directly under our nose...
   Point3F endPoint = mInitialPosition + mInitialDirection * mDataBlock->beamRange;

   mSourceObject->disableCollision();
   RayInfo rayInfo;
   if (getContainer()->castRay(mInitialPosition, endPoint, csmStaticCollisionMask | csmDynamicCollisionMask, &rayInfo) == true)
   {
      // Yes, there is...

      U32 typeMask = rayInfo.object->getTypeMask();

      if( typeMask &= U32(csmDamageableMask) )
      {

         ShapeBase* pSB = dynamic_cast<ShapeBase*>(rayInfo.object);
         if (pSB != NULL) {

            calcSnapPoint( *pSB, mCurrEndPoint );

            QuadPatch qPatch;
            SceneObject *contactObj;
            SceneObject **contactObjPtr = &contactObj;
            bool hitObj = setupSpline( qPatch, contactObjPtr );

            if( !hitObj || contactObj == pSB )
            {
               if( !pSB->isInvincible() )
               {
                  if(pSB->getDamageState() != ShapeBase::Destroyed)
                  {
                     if (bool(mTargetObject) == false || (pSB != (ShapeBase*)mTargetObject))
                     {
                        if (bool(mTargetObject))
                        {
                           releaseTarget(mTargetObject);
                        }

                        setMaskBits(TargetChangedMask);
                        mTargetObject = pSB;
                        acquireTarget(mTargetObject);
                     }
                  }

                  mSourceObject->enableCollision();
                  return;
               }
            }
         }
      }
   }

   // Nothing directly under our gun, query the container to see if anything is close...
   //
   U32 i;
   Box3F queryBox;   
   queryBox.min = mInitialPosition;
   queryBox.max = mInitialPosition;
   queryBox.min.setMin(endPoint);
   queryBox.max.setMax(endPoint);

   // This is really lame, but it fixes a bug that was present in T1.
   static const Point3F sExtendArray[6] = {
      Point3F(0, 0, 1), Point3F(0, 0, -1),
      Point3F(0, 1, 0), Point3F(0, -1, 0),
      Point3F(1, 0, 0), Point3F(-1, 0, 0)
   };
   F32 extend = mDataBlock->beamRange * mSin(mDegToRad(mDataBlock->beamHitWidth));
   for (i = 0; i < 6; i++) {
      queryBox.min.setMin(endPoint + sExtendArray[i]*extend);
      queryBox.max.setMax(endPoint + sExtendArray[i]*extend);
   }

   SimpleQueryList sql;
   getContainer()->findObjects(queryBox, csmDamageableMask,
                               SimpleQueryList::insertionCallback, S32(&sql));
   mSourceObject->enableCollision();

   for (i = 0; i < sql.mList.size(); i++) {
      Point3F objectCenter;
      sql.mList[i]->getObjBox().getCenter(&objectCenter);
      objectCenter.convolve(sql.mList[i]->getScale());
      sql.mList[i]->getTransform().mulP(objectCenter);

      Point3F difVector = objectCenter - mInitialPosition;
      F32 len = difVector.len();
      if (len > mDataBlock->beamRange)
         continue;

      disableCollision();
      bool hit = true;
      if (getContainer()->castRay(mInitialPosition, objectCenter, csmStaticCollisionMask | csmDynamicCollisionMask, &rayInfo)) {
         if (rayInfo.object != sql.mList[i])
            hit = false;
      }
      enableCollision();
      if (!hit)
         continue;

      difVector /= len;
      F32 dot = mDot(difVector, mInitialDirection);
      if (dot >= mDataBlock->cosBeamHitWidth) {


         // Hit the object!
         ShapeBase* pSB = dynamic_cast<ShapeBase*>(sql.mList[i]);
         if (pSB != NULL) {

            calcSnapPoint( *pSB, mCurrEndPoint );

            QuadPatch qPatch;
            SceneObject *contactObj;
            SceneObject **contactObjPtr = &contactObj;
            bool hitObj = setupSpline( qPatch, contactObjPtr );

            if( !hitObj || contactObj == pSB )
            {
               if( !pSB->isInvincible() )
               {

                  if(pSB->getDamageState() != ShapeBase::Destroyed) {
                     if (bool(mTargetObject) == false || (pSB != (ShapeBase*)mTargetObject)) {
                        if (bool(mTargetObject))
                           releaseTarget(mTargetObject);

                        setMaskBits(TargetChangedMask);
                        mTargetObject = pSB;
                        acquireTarget(mTargetObject);
                     }
                  }
                  return;
               }
            }
         }
      }
   }

   // If we're here, we found nothing of note.  Notify the clients if that is necessary,
   //  and clear our target...
   if (bool(mTargetObject)) {
      setMaskBits(TargetChangedMask);
      releaseTarget(mTargetObject);
      mTargetObject = NULL;
   }
}


bool ELFProjectile::checkForFlare( const Point3F &camPos )
{

   if( !mTargetObject ) return false;

   // check if in first or third person mode
   bool firstPerson = true;

   GameConnection *gc = mSourceObject->getControllingClient();
   if( gc )
   {
      firstPerson = gc->isFirstPerson();
   }

   VectorF dirFromGun = mCurrEndPoint - mInitialPosition;
   dirFromGun.normalizeSafe();

   VectorF dirToFlare = mCurrEndPoint - camPos;
   dirToFlare.normalizeSafe();
   Point3F tweakedCamPos = camPos + dirToFlare * 0.5;

   if( mDot( dirFromGun, dirToFlare ) < -0.75 ) return false;


   bool rayHit = true;

   mTargetObject->disableCollision();

   if( firstPerson )
   {
      mSourceObject->disableCollision();

      RayInfo rayInfo;
      rayHit = getContainer()->castRay( tweakedCamPos, mCurrEndPoint, mTargetObject->getType(), &rayInfo );

      mSourceObject->enableCollision();
   }
   else
   {
      RayInfo rayInfo;
      rayHit = getContainer()->castRay( tweakedCamPos, mCurrEndPoint, mTargetObject->getType(), &rayInfo );

   }

   mTargetObject->enableCollision();


   return !rayHit;
}


bool ELFProjectile::findContactPoint( SplinePatch &spline, Point3F &point, SceneObject **contactObj )
{
   Point3F startPoint;
   Point3F endPoint;
   

   mSourceObject->disableCollision();

   RayInfo rayInfo;
   bool rayHit = false;

   spline.calc( 0.0, endPoint );

   for( int i=0; i<LC_NUM_POINTS; i++ )
   {
      startPoint = endPoint;
      F32 t = ((F32)(i+1)) / ((F32)LC_NUM_POINTS);
      spline.calc( t, endPoint );


      Container* container = isServerObject() ? &gServerContainer : &gClientContainer;
      rayHit = container->castRay( startPoint, endPoint,
                                   (csmStaticCollisionMask | csmDynamicCollisionMask | WaterObjectType),
                                   &rayInfo);

      if( rayHit ) break;
   }

   mSourceObject->enableCollision();

   point = rayInfo.point;

   if( contactObj )
   {
      *contactObj = rayInfo.object;
   }

   return rayHit;
}


void ELFProjectile::renderFlare( F32 flareSize )
{

   Point3F cPoint = mCurrEndPoint;

   glDisable(GL_DEPTH_TEST);
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureList[ELFProjectileData::ET_BALL].getGLName() );


   glColor4f( 1.0, 1.0, 1.0, 0.33333333 );

   dglDrawBillboard( cPoint, flareSize * 2.0,  mElapsedTime * .30 * M_PI * 2.0 );
   dglDrawBillboard( cPoint, flareSize * 1.7, -mElapsedTime * .20 * M_PI * 2.0 );
   dglDrawBillboard( cPoint, flareSize * 1.82, mElapsedTime * .10 * M_PI * 2.0 );


   glEnable(GL_DEPTH_TEST);


}

void ELFProjectile::createBolt( LightningBolt &bolt, SplinePatch &spline )
{

   for( int i=0; i<bolt.numPoints; i++ )
   {
      F32 t = ((F32)i) / ((F32)(bolt.numPoints - 1));
      
      Point3F boltPoint;
      spline.calc( t, boltPoint );

      bolt.points[i] = boltPoint + bolt.randPoints[i];

   }

   bolt.alpha = 1.0;
   bolt.elapsedTime = 0.0;

}

void ELFProjectile::createBoltRandPoints( LightningBolt &bolt, F32 sideMag )
{
   static MRandomLCG sRandom;
   
   for( int i=0; i<bolt.numPoints; i++ )
   {
      Point3F randVec;

      randVec.x = sRandom.randF(-1.0, 1.0);
      randVec.y = sRandom.randF(-1.0, 1.0);
      randVec.z = sRandom.randF(-1.0, 1.0);

      randVec.normalizeSafe();
      randVec *= sideMag;

      if( i == 0 || i == (bolt.numPoints - 1) )
      {
         randVec.set( 0.0, 0.0, 0.0 );
      }

      bolt.randPoints[i] = randVec;
   }
}

void ELFProjectile::renderBolt( LightningBolt &bolt, const Point3F &camPos, F32 width )
{
   glColor4f( 1.0, 1.0, 1.0, bolt.alpha );

   glBegin(GL_TRIANGLE_STRIP);

   for( int i=0; i<bolt.numPoints; i++ )
   {
      Point3F  curPoint = bolt.points[i];

      Point3F  nextPoint;
      Point3F  segDir; 

      if( i == (bolt.numPoints-1) )
      {
         nextPoint = -bolt.points[i-1];
         segDir = curPoint - nextPoint;
      }
      else
      {
         nextPoint = bolt.points[i+1];
         segDir = nextPoint - curPoint;
      }
      segDir.normalize();


      Point3F dirFromCam = curPoint - camPos;
      Point3F crossVec;
      mCross(dirFromCam, segDir, &crossVec);
      crossVec.normalize();
      crossVec *= width * 0.5;

      F32 u = i % 2;

      glTexCoord2f( u, 1.0 );
      glVertex3fv( curPoint - crossVec );

      glTexCoord2f( u, 0.0 );
      glVertex3fv( curPoint + crossVec );

   }


   glEnd();
}


bool ELFProjectile::setupSpline( QuadPatch &qPatch, SceneObject **contactObj )
{
   bool hitObject = false;

   Point3F cPoints[3];
   cPoints[0] = mInitialPosition;
   cPoints[1] = mInitialPosition + mInitialDirection * (mInitialPosition - mCurrEndPoint).len();
   cPoints[2] = mCurrEndPoint;

   SplCtrlPts ctrlPoints;
   ctrlPoints.submitPoints( cPoints, 3 );

   qPatch.submitControlPoints( ctrlPoints );

   Point3F newPoint;


   if( findContactPoint( qPatch, newPoint, contactObj ) )
   {
      mCurrEndPoint = newPoint;

      cPoints[1] = mInitialPosition + mInitialDirection * (mInitialPosition - mCurrEndPoint).len();
      qPatch.setControlPoint( cPoints[1], 1 );
      qPatch.setControlPoint( mCurrEndPoint, 2 );
      
      hitObject = true;
   }

   return hitObject;   
}


void ELFProjectile::renderObject(SceneState* state, SceneRenderImage*)
{
   // update begin/end points   
   mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mInitialPosition);
   mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mInitialDirection);

   ShapeBase *obj = mTargetObject;
   calcSnapPoint( *obj, mCurrEndPoint );

   QuadPatch qPatch;
   SceneObject *contactObj;
   SceneObject **contactObjPtr = &contactObj;
   bool hitObj = setupSpline( qPatch, contactObjPtr );

   if( hitObj && contactObj != mTargetObject )
   {
      return;
   }

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupBaseProjection();

   // set gl state
   glEnable(GL_BLEND);
   glDisable(GL_CULL_FACE);
   glEnable(GL_TEXTURE_2D);
   glDepthMask(GL_FALSE);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);

   // render collision flare
   if( checkForFlare( state->getCameraPosition() ) )
   {
      renderFlare( 0.5 );
   }


   // render middle beam
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureList[ELFProjectileData::ET_WAVE].getGLName() );

   glColor3f( 1.0, 1.0, 1.0 );

   SplineUtil::drawSplineBeam( state->getCameraPosition(), 16, mDataBlock->mainBeamWidth * 2.0,
                                qPatch, -mElapsedTime * mDataBlock->mainBeamSpeed, mDataBlock->mainBeamRepeat );



   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureList[ELFProjectileData::ET_BEAM].getGLName() );

   
   // render lightning
   if( mNewBolt )
   {
      createBoltRandPoints( mBoltList[0], mDataBlock->lightningDist );
      createBoltRandPoints( mBoltList[1], mDataBlock->lightningDist );
      createBoltRandPoints( mBoltList[2], mDataBlock->lightningDist );
      mNewBolt = false;
   }
   
   createBolt( mBoltList[0], qPatch );
   createBolt( mBoltList[1], qPatch );
   createBolt( mBoltList[2], qPatch );
   renderBolt( mBoltList[0], state->getCameraPosition(), mDataBlock->lightningWidth );
   renderBolt( mBoltList[1], state->getCameraPosition(), mDataBlock->lightningWidth );
   renderBolt( mBoltList[2], state->getCameraPosition(), mDataBlock->lightningWidth );


   // restore gl state
   glDisable(GL_TEXTURE_2D);
   glDepthMask(GL_TRUE);

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   glDisable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ZERO);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//--------------------------------------------------------------------------
U32 ELFProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(bool(mSourceObject) && bool(mTargetObject))) {
      S32 ghostIndexSource = con->getGhostIndex(mSourceObject);
      S32 ghostIndexDest   = con->getGhostIndex(mTargetObject);
      if (ghostIndexSource == -1 || ghostIndexDest == -1) {
         stream->writeFlag(false);
      } else {
         stream->writeFlag(true);

         stream->writeRangedU32(U32(ghostIndexSource), 0, NetConnection::MaxGhostCount);
         stream->writeRangedU32(U32(mSourceObjectSlot),
                                0, ShapeBase::MaxMountedImages - 1);
         stream->writeRangedU32(U32(ghostIndexDest), 0, NetConnection::MaxGhostCount);
      }
   }

   return retMask;
}

void ELFProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) {
      if (stream->readFlag()) {
         mSourceObjectId    = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         mSourceObjectSlot  = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);
         mTargetObjectId    = stream->readRangedU32(0, NetConnection::MaxGhostCount);

         NetObject* pObject = con->resolveGhost(mSourceObjectId);
         if (pObject != NULL)
            mSourceObject = dynamic_cast<ShapeBase*>(pObject);
         if (bool(mSourceObject) == false)
            Con::errorf(ConsoleLogEntry::General, "ELFProjectile::unpackUpdate: could not resolve source ghost properly on initial update");

         pObject = con->resolveGhost(mTargetObjectId);
         if (pObject != NULL) {
            mTargetObject = dynamic_cast<ShapeBase*>(pObject);
         }
         if (bool(mTargetObject) == false)
            Con::errorf(ConsoleLogEntry::General, "ELFProjectile::unpackUpdate: could not resolve dest ghost properly on initial update");
      } else {
         mSourceObject = NULL;
         mTargetObject = NULL;
      }
   } else {
      mSourceObject = NULL;
      mTargetObject = NULL;
   }
}


void ELFProjectile::calcSnapPoint( ShapeBase &obj, Point3F &point )
{
   // Check if object has polys at its center point.  If not, then try and
   // use a point near its bottom.  ( For inventory stations and such )
   Point3F end;
   obj.getObjBox().getCenter(&end);
   Point3F start = end + Point3F( 0.0, 0.0, 1000.0 );

   RayInfo rayInfo;
   if( !obj.castRay( end, start, &rayInfo ) )
   {

      start = end;
      end = start + Point3F( 0.0, 0.0, -1000.0 );

      if( obj.castRay( start, end, &rayInfo ) )
      {
         obj.getTransform().mulP( rayInfo.point );
         point = rayInfo.point;
         return;
      }
   }
   
   obj.getObjBox().getCenter( &point );
   point.convolve( obj.getScale() );
   obj.getTransform().mulP( point );

}

static bool cHasTarget(SimObject *obj, S32, const char **)
{
   ELFProjectile *proj = static_cast<ELFProjectile *>(obj);
   return(proj->hasTarget());
}


void ELFProjectile::consoleInit()
{
   Con::addCommand("ELFProjectile", "hasTarget", cHasTarget, "projectile.hasTarget()", 2, 2);
}

void ELFProjectile::updateEmitters( F32 dt )
{
   if( !mTargetObject )
   {
      return;
   }
   
   for( U32 i=0; i<ELFProjectileData::ELF_NUM_EMITTERS; i++ )
   {
      if( !mEmitterList[i] ) continue;
      mEmitterList[i]->emitParticles( mCurrEndPoint, mCurrEndPoint, VectorF( 0.0, 0.0, 1.0 ), VectorF( 0.0, 0.0, 0.0 ), dt * 1000 );
   }
}
