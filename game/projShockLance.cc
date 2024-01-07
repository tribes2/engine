//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projShockLance.h"
#include "Core/bitStream.h"
#include "game/shapeBase.h"
#include "PlatformWin32/platformGL.h"
#include "dgl/dgl.h"
#include "sceneGraph/sceneState.h"
#include "sceneGraph/sceneGraph.h"
#include "console/consoleTypes.h"
#include "ts/tsShapeInstance.h"
#include "Sim/netConnection.h"
#include "Math/mathIO.h"
#include "game/particleEngine.h"
#include "Math/mRandom.h"
#include "game/shockwave.h"
#include "game/gameConnection.h"
#include "Math/mathUtils.h"

// Not worth the effort, much less the effort to comment, but if the draw types
// are consecutive use addition rather than a table to go from index to command value...
#if ((GL_TRIANGLES+1==GL_TRIANGLE_STRIP) && (GL_TRIANGLE_STRIP+1==GL_TRIANGLE_FAN))
   #define getDrawType(a) (GL_TRIANGLES+(a))
#else
   U32 drawTypes[] = { GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN };
   #define getDrawType(a) (drawTypes[a])
#endif

#define NO_HIT_LENGTH (0.2)

IMPLEMENT_CO_DATABLOCK_V1(ShockLanceProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(ShockLanceProjectile);

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
ShockLanceProjectileData::ShockLanceProjectileData()
{
   zapDuration = 0.5;
   boltLength  = 2.0;

   startWidth[0] = 0.2;
   startWidth[1] = 0.2;
   endWidth[0] = 1.0;
   endWidth[1] = 1.0;
   boltSpeed[0] = 1.0;
   boltSpeed[1] = 1.2;
   texWrap[0] = 1.0;
   texWrap[1] = 1.0;
   numParts = 25;
   lightningFreq = 10.0;
   lightningDensity = 3.0;
   lightningAmp = 0.5;
   lightningWidth = 0.1;
   shockwave = NULL;
   shockwaveID = 0;
   
   dMemset( textureName, 0, sizeof( textureName ) );
   dMemset( emitterList, 0, sizeof( emitterList ) );
   dMemset( emitterIDList, 0, sizeof( emitterIDList ) );
}

ShockLanceProjectileData::~ShockLanceProjectileData()
{

}

bool ShockLanceProjectileData::calculateAim(const Point3F& targetPos,
                                        const Point3F& /*targetVel*/,
                                        const Point3F& sourcePos,
                                        const Point3F& /*sourceVel*/,
                                        Point3F*       outputVectorMin,
                                        F32*           outputMinTime,
                                        Point3F*       outputVectorMax,
                                        F32*           outputMaxTime)
{
   //make sure the source and target points are within range
   if ((targetPos - sourcePos).len() >= boltLength + 2.0f)
      return false;
   else
   {
      *outputVectorMin = targetPos - sourcePos;
      *outputMinTime   = 0.001f;
      outputVectorMin->normalizeSafe();
      *outputVectorMax = *outputVectorMin;
      *outputMaxTime   = *outputMinTime;

      return true;
   }
}


//--------------------------------------------------------------------------
void ShockLanceProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("zapDuration",       TypeF32,    Offset(zapDuration,     ShockLanceProjectileData));
   addField("boltLength",        TypeF32,    Offset(boltLength,      ShockLanceProjectileData));
   addField("texture",           TypeString, Offset(textureName,     ShockLanceProjectileData), NUM_TEX );
   addField("startWidth",        TypeF32,    Offset(startWidth,      ShockLanceProjectileData), NUM_BOLTS );
   addField("endWidth",          TypeF32,    Offset(endWidth,        ShockLanceProjectileData), NUM_BOLTS );
   addField("boltSpeed",         TypeF32,    Offset(boltSpeed,       ShockLanceProjectileData), NUM_BOLTS );
   addField("texWrap",           TypeF32,    Offset(texWrap,         ShockLanceProjectileData), NUM_BOLTS );
   addField("numParts",          TypeS32,    Offset(numParts,        ShockLanceProjectileData));
   addField("lightningFreq",     TypeF32,    Offset(lightningFreq,   ShockLanceProjectileData));
   addField("lightningDensity",  TypeF32,    Offset(lightningDensity,ShockLanceProjectileData));
   addField("lightningAmp",      TypeF32,    Offset(lightningAmp,    ShockLanceProjectileData));
   addField("lightningWidth",    TypeF32,    Offset(lightningWidth,  ShockLanceProjectileData));
   addField("emitter",           TypeParticleEmitterDataPtr,   Offset(emitterList, ShockLanceProjectileData), NUM_EMITTERS);
   addField("shockwave",         TypeShockwaveDataPtr,         Offset(shockwave,   ShockLanceProjectileData));
}


//--------------------------------------------------------------------------
bool ShockLanceProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;


   if (zapDuration < 0.05 || zapDuration >= 30.0) {
      Con::warnf(ConsoleLogEntry::General,
                 "ShockLanceProjectileData(%s)::onAdd: zapduration must be in range [0.05, 2.0]",
                 getName());
      zapDuration = zapDuration < 0.05 ? 0.05 : 2.0;
   }


  if (boltLength < 0.5 || boltLength >= 50) {
      Con::warnf(ConsoleLogEntry::General,
                 "ShockLanceProjectileData(%s)::onAdd: boltLength must be in range [0.5, 50.0]",
                 getName());
      boltLength = boltLength < 0.5 ? 0.5 : 5.0;
   }


   return true;
}


bool ShockLanceProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (server == false)
   {

      U32 i;
      for( i=0; i<NUM_TEX; i++ )
      {
         if (textureName[i] && textureName[i][0])
         {
            textureHandle[i] = TextureHandle(textureName[i], MeshTexture );
         }
      }

      for( i=0; i<NUM_EMITTERS; i++ )
      {
         if( !emitterList[i] && emitterIDList[i] != 0 )
         {
            if( Sim::findObject( emitterIDList[i], emitterList[i] ) == false)
            {
               Con::errorf( ConsoleLogEntry::General, "ShockLanceProjectileData::onAdd: Invalid packet, bad datablockId(particle emitter): 0x%x", emitterIDList[i] );
            }
         }
      }

      if( !shockwave && shockwaveID )
      {
         if( !Sim::findObject( shockwaveID, shockwave ) )
         {
            Con::errorf( ConsoleLogEntry::General, "ShockLanceProjectileData::onAdd: Invalid packet, bad datablockId(shockwave): 0x%x", shockwaveID );
         }
      }
      
   }
   else
   {
      return true;
   }

   return true;
}


//--------------------------------------------------------------------------
void ShockLanceProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(zapDuration);
   stream->write(boltLength);
   stream->write(numParts);
   stream->write(lightningFreq);
   stream->write(lightningDensity);
   stream->write(lightningAmp);
   stream->write(lightningWidth);

   if( stream->writeFlag( shockwave ) )
   {
      stream->writeRangedU32( shockwave->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   U32 i;
   for( i=0; i<NUM_BOLTS; i++ )
   {
      stream->write( startWidth[i] );
      stream->write( endWidth[i] );
      stream->write( boltSpeed[i] );
      stream->write( texWrap[i] );
   }

   for( i=0; i<NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }

   for( i=0; i<NUM_EMITTERS; i++ )
   {
      if( stream->writeFlag( emitterList[i] != NULL ) )
      {
         stream->writeRangedU32( emitterList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }

}

void ShockLanceProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&zapDuration);
   stream->read(&boltLength);
   stream->read(&numParts);
   stream->read(&lightningFreq);
   stream->read(&lightningDensity);
   stream->read(&lightningAmp);
   stream->read(&lightningWidth);

   if( stream->readFlag() )
   {
      shockwaveID = (S32) stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   U32 i;
   for( i=0; i<NUM_BOLTS; i++ )
   {
      stream->read( &startWidth[i] );
      stream->read( &endWidth[i] );
      stream->read( &boltSpeed[i] );
      stream->read( &texWrap[i] );
   }

   for( i=0; i<NUM_TEX; i++ )
   {
      textureName[i] = stream->readSTString();
   }

   for( i=0; i<NUM_EMITTERS; i++ )
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
ShockLanceProjectile::ShockLanceProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);
   mElapsedTime = 0;
   mTargetId = 0;
   mElectricity = NULL;
   mTimeSinceLastLightningBolt = 0.0;
   mHitObject = false;
   
   dMemset( mEmitterList, 0, sizeof( mEmitterList ) );
}

ShockLanceProjectile::~ShockLanceProjectile()
{
   //
}

//--------------------------------------------------------------------------
void ShockLanceProjectile::initPersistFields()
{
   Parent::initPersistFields();

   addField("targetId", TypeS32, Offset(mTargetId, ShockLanceProjectile));
}


void ShockLanceProjectile::consoleInit()
{
   
}


//--------------------------------------------------------------------------
bool ShockLanceProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (isServerObject())
   {
      ShapeBase* pTarget;
      if (mTargetId != 0 && Sim::findObject(mTargetId, pTarget)) {
         mTargetPtr = pTarget;
      }

      mDeleteWaitTicks = ((mDataBlock->zapDuration) * 1000.0f) / TickMs;

      mStart = mInitialPosition;
      mEnd   = mInitialPosition + mInitialDirection * mDataBlock->boltLength;

      RayInfo rInfo;
      if( gServerContainer.castRay( mStart, mEnd, csmStaticCollisionMask | csmDynamicCollisionMask, &rInfo) == true)
      {

         U32 typeMask = rInfo.object->getTypeMask();
         if( typeMask &= U32(csmDamageableMask) )
         {

            if( mTargetPtr && !mTargetPtr->isInvincible() )
            {
               mEnd = rInfo.point;
               mHitObject = true;
            }
         }
      }
   }
   else
   {
      if (bool(mTargetPtr) && mHitObject ) {
         // Need to spawn electricity
         ShockLanceElectricity* pElectricity = new ShockLanceElectricity;
         pElectricity->mTarget = mTargetPtr;
         pElectricity->registerObject();
         mElectricity = pElectricity;
         mElectricity->setDataBlock( mDataBlock );
      }

      if( mDataBlock->shockwave && mHitObject )
      {
         VectorF normal = mStart - mEnd;
         normal.normalizeSafe();

         MatrixF trans = getTransform();
         trans.setPosition( mEnd );
         Shockwave* shockwave = new Shockwave;
         shockwave->onNewDataBlock( mDataBlock->shockwave );
         shockwave->setTransform( trans );
         shockwave->setInitialState( trans.getPosition(), normal );
         if (!shockwave->registerObject())
            delete shockwave;
      }
      
      
      U32 i;
      for( i=0; i<ShockLanceProjectileData::NUM_BOLTS; i++ )
      {
         mBeamWidthVel[i] = ( mDataBlock->endWidth[i] - mDataBlock->startWidth[i] ) / mDataBlock->zapDuration;
      }

      for( i=0; i<ShockLanceProjectileData::NUM_EMITTERS; i++ )
      {
         if( mDataBlock->emitterList[i] != NULL )
         {
            ParticleEmitter * pEmitter = new ParticleEmitter;
            pEmitter->onNewDataBlock( mDataBlock->emitterList[i] );
            if( !pEmitter->registerObject() )
            {
               Con::warnf( ConsoleLogEntry::General, "Could not register emitter for particle of class: %s", mDataBlock->getName() );
               delete pEmitter;
               pEmitter = NULL;
            }
            mEmitterList[i] = pEmitter;
         }
      }

      if( mHitObject )
      {
         VectorF dir = mEnd - mStart;
         dir.normalizeSafe();

         if( dir.magnitudeSafe() > 0.0 && mEmitterList[0] )
         {
            mEmitterList[0]->emitParticles( mStart, mEnd, dir, Point3F( 0.0, 0.0, 0.0 ), mDataBlock->numParts );
         }
      }
   }

   mObjBox.min = mEnd;
   mObjBox.max = mEnd;
   mObjBox.min.setMin(mStart);
   mObjBox.max.setMax(mStart);

   resetWorldBox();

   addToScene();

   return true;
}


void ShockLanceProjectile::onRemove()
{
   removeFromScene();

   if (bool(mElectricity))
      mElectricity->deleteObject();

   for( int i=0; i<ShockLanceProjectileData::NUM_EMITTERS; i++ )
   {
      if( mEmitterList[i] )
      {
         mEmitterList[i]->deleteWhenEmpty();
         mEmitterList[i] = NULL;
      }
   }

   Parent::onRemove();
}


bool ShockLanceProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<ShockLanceProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}



//--------------------------------------------------------------------------
void ShockLanceProjectile::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isServerObject()) {
      if (--mDeleteWaitTicks <= 0)
         deleteObject();
   }
}


void ShockLanceProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if( mElectricity )
   {
      mElectricity->advanceTime( dt );
   }

   mElapsedTime += dt;

   mTimeSinceLastLightningBolt += dt;
   
   F32 freqRecip = (1.0/mDataBlock->lightningFreq);
   if( mTimeSinceLastLightningBolt >= freqRecip )
   {
      mTimeSinceLastLightningBolt -= freqRecip;

      for( U32 i=0; i<NUM_LIGHTNING_BOLTS; i++ )
      {
         F32 density = mDataBlock->lightningDensity;
         F32 amp = mDataBlock->lightningAmp;
         if( !mHitObject )
         {
            density = 20.0;
            amp = 0.1;
            
            updateLightningPos();
         }
         createBolt( mBoltList[i], density, amp );
      }
   }
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void ShockLanceProjectile::updateLightningPos()
{

   VectorF muzzleDir;
   if( mSourceObject )
   {
      mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mStart);
      mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &muzzleDir);
      muzzleDir.normalizeSafe();
      mEnd = mStart + muzzleDir * NO_HIT_LENGTH;

      mObjBox.min = mEnd;
      mObjBox.max = mEnd;
      mObjBox.min.setMin(mStart);
      mObjBox.max.setMax(mStart);

      resetWorldBox();
   }
}


//--------------------------------------------------------------------------
bool ShockLanceProjectile::prepRenderImage(SceneState* state,
                                           const U32   stateKey,
                                           const U32   /*startZone*/,
                                           const bool  /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->isTranslucent = true;
      image->sortType = SceneRenderImage::Point;
      state->setImageRefPoint(this, image);
      image->tieBreaker = true;

      state->insertRenderImage(image);
   }

   return false;
}


void ShockLanceProjectile::renderObject(SceneState*       state,
                                        SceneRenderImage* /*sceneImage*/)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);

   if( !mHitObject )
   {
      updateLightningPos();
   }
   else
   {
      renderBeam( state->getCameraPosition(), 0 );
      renderBeam( state->getCameraPosition(), 1 );
   }

   renderLightning( state->getCameraPosition() );

   
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glDepthMask(GL_TRUE);
   
   dglSetCanonicalState();
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------------------------------------------
// Render lightning
//--------------------------------------------------------------------------
void ShockLanceProjectile::renderLightning( const Point3F &camPos )
{
   
   F32 lightningAlpha = 1.0 - (mElapsedTime / mDataBlock->zapDuration);

   glDepthMask(GL_FALSE);
   glEnable( GL_DEPTH_TEST );
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[ShockLanceProjectileData::PROJECTILE_TEX].getGLName() );
   glColor4f( 1.0, 1.0, 1.0, lightningAlpha );
   

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   VectorF dir = mEnd - mStart;
   dir.normalizeSafe();
   MatrixF m = MathUtils::createOrientFromDir( dir );
   m.setPosition( mStart );

   dglMultMatrix( &m );   
   
   for( U32 i=0; i<NUM_LIGHTNING_BOLTS; i++ )
   {
      renderBolt( mBoltList[i], camPos, mDataBlock->lightningWidth );
   }

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   
}

//--------------------------------------------------------------------------
void ShockLanceProjectile::renderBeam( const Point3F &camPos, U32 boltNum )
{
   Point3F dir = mEnd - mStart;
   dir.normalizeSafe(); 
 
   Point3F pos = mStart;
   Point3F dirFromCam = pos - camPos;
   Point3F crossDir;
   mCross( dirFromCam, dir, &crossDir );
   crossDir.normalizeSafe();


   F32 width = (mDataBlock->startWidth[boltNum] + (mBeamWidthVel[boltNum] * mElapsedTime)) * 0.5;
   crossDir *= width;


   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable( GL_DEPTH_TEST );
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   glEnable(GL_TEXTURE_2D);

   F32 endPointAlpha = 1.0 - (mElapsedTime / mDataBlock->zapDuration);
   F32 startPointAlpha = 0.0;
   
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[ShockLanceProjectileData::PROJECTILE_TEX].getGLName() );

   glBegin(GL_QUADS);

      glColor4f( 1.0, 1.0, 1.0, startPointAlpha );
      glTexCoord2f(mDataBlock->boltSpeed[boltNum] * mElapsedTime, 0);
      glVertex3fv( mStart + crossDir );

      glTexCoord2f(mDataBlock->boltSpeed[boltNum] * mElapsedTime, 1);
      glVertex3fv( mStart - crossDir );

      glColor4f( 1.0, 1.0, 1.0, endPointAlpha );
      glTexCoord2f(mDataBlock->boltSpeed[boltNum] * mElapsedTime - (1.0 * mDataBlock->texWrap[boltNum]), 1);
      glVertex3fv( mEnd - crossDir );

      glTexCoord2f(mDataBlock->boltSpeed[boltNum] * mElapsedTime - (1.0 * mDataBlock->texWrap[boltNum]), 0);
      glVertex3fv( mEnd + crossDir );

   glEnd();
   
}


//--------------------------------------------------------------------------
U32 ShockLanceProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (bool(mTargetPtr)) {
      // Potentially have to write this to the client, let's make sure it has a
      //  ghost on the other side...
      S32 ghostIndex = con->getGhostIndex(mTargetPtr);
      if (stream->writeFlag(ghostIndex != -1))
         stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
   } else {
      stream->writeFlag(false);
   }

   if (stream->writeFlag(mask & GameBase::InitialUpdateMask))
   {
      mathWrite(*stream, mStart);
      mathWrite(*stream, mEnd);
      stream->writeFlag( mHitObject );

      if (bool(mSourceObject)) {
         // Potentially have to write this to the client, let's make sure it has a
         //  ghost on the other side...
         bool isFiredOnClient = mSourceObject->getControllingClient() == con;

         S32 ghostIndex = con->getGhostIndex(mSourceObject);
         if (stream->writeFlag(ghostIndex != -1)) {
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
            stream->writeRangedU32(U32(mSourceObjectSlot),
                                   0, ShapeBase::MaxMountedImages - 1);
         }
      } else {
         stream->writeFlag(false);
      }

   }
      
   return retMask;
}

//--------------------------------------------------------------------------
void ShockLanceProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) {
      mTargetId = stream->readRangedU32(0, NetConnection::MaxGhostCount);

      NetObject* pObject = con->resolveGhost(mTargetId);
      if (pObject != NULL)
         mTargetPtr = dynamic_cast<ShapeBase*>(pObject);

      if (bool(mTargetPtr) == false)
         Con::errorf(ConsoleLogEntry::General, "LinearProjectile::unpackUpdate: could not resolve source ghost properly on initial update");
   } else {
      mTargetId = 0;
      mTargetPtr = NULL;
   }

   // initial update
   if( stream->readFlag() )
   {
      mathRead(*stream, &mStart);
      mathRead(*stream, &mEnd);
      mHitObject = stream->readFlag();

      if (stream->readFlag()) {
         mSourceObjectId   = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         mSourceObjectSlot = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);

         NetObject* pObject = con->resolveGhost(mSourceObjectId);
         if (pObject != NULL)
            mSourceObject = dynamic_cast<ShapeBase*>(pObject);
      } else {
         mSourceObjectId   = -1;
         mSourceObjectSlot = -1;
         mSourceObject     = NULL;
      }
   }
   
}

//--------------------------------------------------------------------------
// Create bolt
//--------------------------------------------------------------------------
void ShockLanceProjectile::createBolt( LightningBolt &bolt, U32 density, F32 amplitude )
{
   
   F32 lanceLength = Point3F( mEnd - mStart ).magnitudeSafe();
   U32 numPoints = lanceLength * density;
   F32 pointInc = lanceLength / numPoints;
   VectorF dir( 0.0, 1.0, 0.0 );


   bolt.numPoints = numPoints;
   
   if( numPoints > NUM_POINTS )
   {
      numPoints = NUM_POINTS;
   }

   for( U32 i=0; i<numPoints; i++ )
   {
      Point3F randVec;

      randVec.x = gRandGen.randF(-1.0, 1.0);
      randVec.y = gRandGen.randF(-1.0, 1.0);
      randVec.z = gRandGen.randF(-1.0, 1.0);

      randVec.normalizeSafe();
      randVec *= amplitude;

      if( i == 0 || i == (bolt.numPoints - 1) )
      {
         randVec.set( 0.0, 0.0, 0.0 );
      }

      bolt.points[i] = (dir * pointInc * i) + randVec;
   }

}

//--------------------------------------------------------------------------
// Render bolt
//--------------------------------------------------------------------------
void ShockLanceProjectile::renderBolt( LightningBolt &bolt, const Point3F &camPos, F32 width )
{
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
      segDir.normalizeSafe();


      Point3F dirFromCam = curPoint - camPos;
      Point3F crossVec;
      mCross(dirFromCam, segDir, &crossVec);
      crossVec.normalizeSafe();
      crossVec *= width * 0.5;

      F32 u = i % 2;

      glTexCoord2f( u, 1.0 );
      glVertex3fv( curPoint - crossVec );

      glTexCoord2f( u, 0.0 );
      glVertex3fv( curPoint + crossVec );

   }

   glEnd();
}

//--------------------------------------------------------------------------
// SHOCKLANCE ELECTRICITY
//--------------------------------------------------------------------------
ShockLanceElectricity::ShockLanceElectricity()
{
   mTypeMask |= ProjectileObjectType;
   mNetFlags.clear(Ghostable);
   mCopiedList = NULL;
   mElapsedTime = 0.0;
   mDataBlock = NULL;
   mShapeDetail = 0;
}

ShockLanceElectricity::~ShockLanceElectricity()
{
   //
}

//--------------------------------------------------------------------------
bool ShockLanceElectricity::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if( bool(mTarget) == false )
      return false;

   // First, get the shape instance, and copy it's material
   //  list.  Then replace any base. entries with our own
   //  texture...
   TSShapeInstance* pInstance = mTarget->getShapeInstance();
   if (pInstance == NULL)
      return false;

   TSMaterialList* pBase = pInstance->getMaterialList();
   mCopiedList = new TSMaterialList(pBase);

   mObjBox = mTarget->getObjBox();
   resetWorldBox();
   MatrixF transform = mTarget->getTransform();
   setTransform(transform);

   mStartTime = Sim::getCurrentTime();

   gClientContainer.addObject(this);
   gClientSceneGraph->addObjectToScene(this);

   NetConnection* pNC = NetConnection::getServerConnection();
   AssertFatal(pNC != NULL, "Error, must have a connection to the server!");
   pNC->addObject(this);

   if( isClientObject() )
   {
      TSShapeInstance* pInstance = mTarget->getShapeInstance();
      mShapeDetail = pInstance->getNumDetails();
   }

   return true;
}

//--------------------------------------------------------------------------
void ShockLanceElectricity::onRemove()
{
   while( mTexCoordList.size() )
   {
      Point2F *ptr = mTexCoordList[ mTexCoordList.size() - 1 ];
      mTexCoordList.pop_back();
      delete [] ptr;
   }


   delete mCopiedList;
   mCopiedList = NULL;
   mTarget = NULL;

   if (mSceneManager != NULL)
      mSceneManager->removeObjectFromScene(this);
   if (getContainer() != NULL)
      getContainer()->removeObject(this);
 
      
   Parent::onRemove();
}



//--------------------------------------------------------------------------
bool ShockLanceElectricity::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   if (bool(mTarget) == false )
      return false;

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->isTranslucent = true;
      image->sortType = SceneRenderImage::Point;
      state->setImageRefPoint(this, image);
      
      state->insertRenderImage(image);
   }

   return false;
}

//--------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------
void ShockLanceElectricity::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);

   renderElectricity();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   dglSetCanonicalState();
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//--------------------------------------------------------------------------
void ShockLanceElectricity::processTick( const Move* )
{
   if (bool(mTarget))
   {
      MatrixF transform = mTarget->getTransform();
      setTransform(transform);
   }
}

void ShockLanceElectricity::advanceTime( F32 dt )
{
   mElapsedTime += dt;
}

//--------------------------------------------------------------------------
void ShockLanceElectricity::renderElectricity()
{
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();


   dglMultMatrix( &mTarget->getRenderTransform() );
   glScalef( 1.05, 1.05, 1.05 );

   TSShapeInstance* pInstance = mTarget->getShapeInstance();

   pInstance->setStatics(mShapeDetail);

   // setup texture
	F32 flickerPerSec = 10.0;
	F32 numFrames = ShockLanceProjectileData::NUM_ELECTRIC_TEX - 0.0001;
	U32 texNum = mFmod( mElapsedTime, 1.0 / flickerPerSec ) * flickerPerSec * (numFrames);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE); // should leave alpha alone since emap has no alpha
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[texNum].getGLName() );

   glEnable( GL_CULL_FACE );
   glDepthMask(GL_FALSE);
   glEnable( GL_DEPTH_TEST );

   // set up texture motion
   F32 x = mElapsedTime * 2.0;
   F32 y = mElapsedTime * 1.0;

   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glScalef(1, 1, 1);
   glTranslatef(x, y, 0);
   glMatrixMode(GL_MODELVIEW);



   glEnable(GL_TEXTURE_GEN_S);
   glEnable(GL_TEXTURE_GEN_T);
   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

   F32 SBase[4];
   F32 TBase[4];

   calcTexPlanes( SBase, TBase );

   glTexGenfv(GL_S, GL_OBJECT_PLANE, SBase );
   glTexGenfv(GL_T, GL_OBJECT_PLANE, TBase );

   
   glColor4f(1.0, 1.0, 1.0, 1.0 - (mElapsedTime / mDataBlock->zapDuration) );
   
   
   const TSDetail * detail = &pInstance->getShape()->details[mShapeDetail];
   S32 ss = detail->subShapeNum;

   S32 start = pInstance->smNoRenderNonTranslucent ? pInstance->getShape()->subShapeFirstTranslucentObject[ss] : pInstance->getShape()->subShapeFirstObject[ss];
   S32 end   = pInstance->smNoRenderTranslucent ? pInstance->getShape()->subShapeFirstTranslucentObject[ss] : pInstance->getShape()->subShapeFirstObject[ss] + pInstance->getShape()->subShapeNumObjects[ss];

   U32 texPtr = 0;
   U32 i=0;
   for (i=start; i<end; i++)
   {

      TSMesh *curMesh = pInstance->mMeshObjects[i].getMesh(mShapeDetail);
      if( !curMesh ) continue;
      if( pInstance->mMeshObjects[i].visible <= 0.01f ) continue;

      glPushMatrix();

      MatrixF *trans = pInstance->mMeshObjects[i].getTransform();

      dglMultMatrix( trans );

      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);

      S32 firstVert  = curMesh->vertsPerFrame * pInstance->mMeshObjects[i].frame;
      glVertexPointer(3,GL_FLOAT,0,&curMesh->verts[firstVert]);
      glNormalPointer(GL_FLOAT,0,&curMesh->norms[firstVert]);


      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE);

      for (S32 j=0; j<curMesh->primitives.size(); j++)
      {
         TSDrawPrimitive & draw = curMesh->primitives[j];
         S32 drawType = getDrawType(draw.matIndex>>30);
         glDrawElements(drawType,draw.numElements,GL_UNSIGNED_SHORT,&curMesh->indices[draw.start]);
      }

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);

      glPopMatrix();
   }

   glDisable(GL_TEXTURE_GEN_S);
   glDisable(GL_TEXTURE_GEN_T);

   glMatrixMode(GL_TEXTURE);
   glPopMatrix();


   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glDepthMask(GL_TRUE);

}

//--------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------
void ShockLanceElectricity::calcTexPlanes( F32 *S, F32 *T )
{
   TSShapeInstance* pInstance = mTarget->getShapeInstance();

   Box3F bounds;
   pInstance->computeBounds( mShapeDetail, bounds );
   VectorF bNorm = bounds.max - bounds.min;

   // you MUST set this after calling computeBounds!
   pInstance->setStatics(mShapeDetail);

   if( fabs(bNorm.x) > fabs(bNorm.y) && fabs(bNorm.x) > fabs(bNorm.z) )
   {
      F32 size = (1.0 / bNorm.x) * bNorm.x * 0.25;

      S[0] = size;
      S[1] = 0.0;
      S[2] = 0.0;
      S[3] = 0.0;

      T[0] = 0.0;
      T[1] = 0.0;
      T[2] = size;
      T[3] = 0.0;
   }
   else
   {
      if( fabs(bNorm.y) > fabs(bNorm.x) && fabs(bNorm.y) > fabs(bNorm.z) )
      {
         F32 size = (1.0 / bNorm.y) * bNorm.y * 0.25;

         S[0] = 0.0;
         S[1] = size;
         S[2] = 0.0;
         S[3] = 0.0;

         T[0] = 0.0;
         T[1] = 0.0;
         T[2] = size;
         T[3] = 0.0;
      }
      else
      {
         if( fabs(bNorm.z) > fabs(bNorm.x) && fabs(bNorm.z) > fabs(bNorm.y) )
         {
            F32 size = (1.0 / bNorm.z) * bNorm.z * 0.25;

            S[0] = size;
            S[1] = 0.0;
            S[2] = 0.0;
            S[3] = 0.0;

            T[0] = 0.0;
            T[1] = 0.0;
            T[2] = size;
            T[3] = 0.0;
         }
      }
   }

}

