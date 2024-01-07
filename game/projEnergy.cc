//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projEnergy.h"
#include "core/bitStream.h"
#include "sim/netConnection.h"
#include "game/shapeBase.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"
#include "game/explosion.h"
#include "terrain/waterBlock.h"
#include "game/splash.h"
#include "game/particleEngine.h"
#include "dgl/dgl.h"
#include "scenegraph/sceneState.h"
#include "math/mathUtils.h"

#define MAX_BOUNCE_COUNT 5

IMPLEMENT_CO_NETOBJECT_V1(EnergyProjectile);
IMPLEMENT_CO_DATABLOCK_V1(EnergyProjectileData);

//--------------------------------------------------------------------------
//--------------------------------------
//
EnergyProjectileData::EnergyProjectileData()
{
   dMemset( textureName, 0, sizeof( textureName ) );
   dMemset( textureHandle, 0, sizeof( textureHandle ) );

   crossViewAng = 0.98;
   crossSize = 0.45;
   blurLifetime = 0.5;
   blurWidth = 0.25;
   blurColor.set( 0.4, 0.0, 0.0, 1.0 );
}

//--------------------------------------------------------------------------
EnergyProjectileData::~EnergyProjectileData()
{

}


//--------------------------------------------------------------------------
void EnergyProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("texture",        TypeString,    Offset( textureName,    EnergyProjectileData), NUM_TEX);
   addField("crossViewAng",   TypeF32,       Offset( crossViewAng,   EnergyProjectileData));
   addField("crossSize",      TypeF32,       Offset( crossSize,      EnergyProjectileData));
   addField("blurLifetime",   TypeF32,       Offset( blurLifetime,   EnergyProjectileData));
   addField("blurWidth",      TypeF32,       Offset( blurWidth,      EnergyProjectileData));
   addField("blurColor",      TypeColorF,    Offset( blurColor,      EnergyProjectileData));
}


//--------------------------------------------------------------------------
// Preload data - load resources
//--------------------------------------------------------------------------
bool EnergyProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (!server) {
      U32 i;
      for( i=0; i<NUM_TEX; i++ )
      {
         if (textureName[i] && textureName[i][0])
         {
            textureHandle[i] = TextureHandle(textureName[i], MeshTexture );
         }
      }
   }

   return true;
}

//--------------------------------------------------------------------------
bool EnergyProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}

//--------------------------------------------------------------------------
void EnergyProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(crossViewAng);
   stream->write(crossSize);
   stream->write(blurLifetime);
   stream->write(blurWidth);
   stream->write(blurColor.red);
   stream->write(blurColor.green);
   stream->write(blurColor.blue);


   U32 i;
   for( i=0; i<NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }

}

//--------------------------------------------------------------------------
void EnergyProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&crossViewAng);
   stream->read(&crossSize);
   stream->read(&blurLifetime);
   stream->read(&blurWidth);
   stream->read(&blurColor.red);
   stream->read(&blurColor.green);
   stream->read(&blurColor.blue);


   for( U32 i=0; i<NUM_TEX; i++ )
   {
      textureName[i] = stream->readSTString();
   }

}

// same as ::linearProjectileData
bool EnergyProjectileData::calculateAim(const Point3F& targetPos,
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

   F32 a = effTargetVel.lenSquared() - (muzzleVelocity*muzzleVelocity);
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
   Point3F finalAnswer = (effTargetPos / (muzzleVelocity * t)) + (effTargetVel / muzzleVelocity);
   finalAnswer.normalize();

   *outputVectorMin = finalAnswer;
   *outputVectorMax = finalAnswer;
   *outputMinTime   = t;
   *outputMaxTime   = t;

   return (t * 1000.0) <= lifetimeMS;
}

//--------------------------------------------------------------------------
//--------------------------------------
//
EnergyProjectile::EnergyProjectile()
{

}

EnergyProjectile::~EnergyProjectile()
{
   //
}

//--------------------------------------------------------------------------
void EnergyProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


void EnergyProjectile::consoleInit()
{
   //
}


//--------------------------------------------------------------------------
bool EnergyProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if( isClientObject() )
   {
      mMotionBlur.setColor( mDataBlock->blurColor );
      mMotionBlur.setWidth( mDataBlock->blurWidth );
      mMotionBlur.setLifetime( mDataBlock->blurLifetime );

      mLastPos = mCurrPosition;
   }

   mObjBox = Box3F( Point3F( -0.5, -0.5, -0.5 ), Point3F( 0.5, 0.5, 0.5 ) );
   resetWorldBox();

   return true;
}


//--------------------------------------------------------------------------
void EnergyProjectile::onRemove()
{
   Parent::onRemove();
}


//--------------------------------------------------------------------------
bool EnergyProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<EnergyProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
void EnergyProjectile::processTick(const Move* move)
{
   Projectile::processTick(move);

   if (isServerObject()) {
      // server
      if (mDeleteTick != -1 && mCurrTick >= mDeleteTick) {
         deleteObject();
         return;
      } else if (mHidden == true) {
         // already exploded, back out
         return;
      }

      bool hitDynamic = false;
      bool reflected = false;
      Point3F oldPosition;
      Point3F newPosition;

      // Otherwise, we have to do some simulation work.
      oldPosition = mCurrPosition;
      computeNewState(&newPosition, &mCurrVelocity);


      VectorF currDir = mCurrVelocity;
      currDir.normalizeSafe();

      F32 dot = mDot( currDir, mInitialDirection );
      reflected = (1.0 - dot) >= 0.1;

      if( !reflected )
      {
         collisionOff();
      }


      F32 timeLeft = 1.0;
      S32 bounces = MAX_BOUNCE_COUNT;
      while( bounces-- )
      {

         RayInfo rInfo;
         if (getContainer()->castRay(oldPosition, newPosition,
                                     csmDynamicCollisionMask | csmStaticCollisionMask,
                                     &rInfo) == true)
         {
            // First order of business.  Is what we hit a dynamic object?  If so,
            //  we need to inform the client.
            if( (rInfo.object->getType() & csmDynamicCollisionMask) )
            {
               hitDynamic = true;
               setMaskBits(BounceMask);
            }

            // Next order of business: do we explode on this hit?
            if( mCurrTick > mArmedTick || hitDynamic )
            {
               MatrixF xform(true);
               xform.setColumn(3, rInfo.point + rInfo.normal * 0.1);
               setTransform(xform);
               mCurrPosition    = rInfo.point;
               mCurrVelocity    = Point3F(0, 0, 0);

               if( !reflected )
               {
                  collisionOn();
               }

               explode(rInfo.point, rInfo.normal);
               onCollision( rInfo.point, rInfo.normal, rInfo.object );
               mDeleteTick = mCurrTick + mDataBlock->blurLifetime * 1000 / TickMs;

               if( !reflected )
               {
                  collisionOff();
               }
               break;
            }

            mCurrVelocity = mCurrVelocity - rInfo.normal * (mDot( mCurrVelocity, rInfo.normal ) * 2.0);;
            timeLeft = timeLeft * (1.0 - rInfo.t);
            oldPosition = rInfo.point + rInfo.normal * 0.05;
            newPosition = oldPosition + (mCurrVelocity * ((timeLeft/1000.0) * TickMs));

            if( !reflected )
            {
               collisionOn();
               reflected = true;
            }
            
         } else {
            // No problems, just set the end position, and we're golden.

            mCurrPosition = newPosition;
            MatrixF xform(true);
            xform.setColumn(3, mCurrPosition);
            setTransform(xform);

            mMotionBlur.addSegment( oldPosition, newPosition );
            mLastPos = newPosition;

            break;
         }
      }

      // remove object if it is stuck
      if( bounces < 0 )
      {
         deleteObject();
         return;
      }

      if( !reflected )
      {
         collisionOn();
      }
     
      mMotionBlur.update( TickMs );

   } else {
      // client
      if (mHidden == true)
         return;

      bool hitDynamic = false;
      Point3F oldPosition;
      Point3F newPosition;

      // Otherwise, we have to do some simulation work.
      oldPosition = mCurrPosition;
      computeNewState(&newPosition, &mCurrVelocity);

      collisionOff();
      
      F32 timeLeft = 1.0;
      S32 bounces = MAX_BOUNCE_COUNT;
      while( bounces-- )
      {

         RayInfo rInfo;
         if (getContainer()->castRay(oldPosition, newPosition,
                                     csmStaticCollisionMask | csmDynamicCollisionMask,
                                     &rInfo) == true)
         {
            // First order of business.  Is what we hit a dynamic object?  If so,
            //  we need to inform the client.
            if( (rInfo.object->getType() & csmDynamicCollisionMask) )
            {
               hitDynamic = true;
               setMaskBits(BounceMask);
            }

               // Next order of business: do we explode on this hit?
            if( mCurrTick > mArmedTick || hitDynamic )
            {
               MatrixF xform(true);
               xform.setColumn(3, rInfo.point + rInfo.normal * 0.1 );
               setTransform(xform);
               mCurrPosition    = rInfo.point;
               mCurrVelocity    = Point3F(0, 0, 0);

               if( !oldPosition.equal( rInfo.point ) )
               {
                  mMotionBlur.addSegment( oldPosition, rInfo.point );
               }
               explode( rInfo.point + rInfo.normal * 0.1, rInfo.normal);
               break;
            }

            mCurrVelocity = mCurrVelocity - rInfo.normal * (mDot( mCurrVelocity, rInfo.normal ) * 2.0);;
            timeLeft = timeLeft * (1.0 - rInfo.t);
            oldPosition = rInfo.point + rInfo.normal * 0.05;
            newPosition = oldPosition + (mCurrVelocity * ((timeLeft/1000.0) * TickMs));
            
         }
         else
         {
            // No problems, just set the end position, and we're golden.

            mCurrDeltaBase  = newPosition;
            mCurrBackVector = mCurrPosition - newPosition;

            if( !mLastPos.equal( newPosition ) )
            {
               mMotionBlur.addSegment( mLastPos, newPosition );
               mLastPos = newPosition;
            }

            mCurrPosition   = newPosition;

            break;
         }
      }


      if( determineSplash( oldPosition, newPosition ) )
      {
         createSplash( mSplashPos );
      }

      collisionOn();
      
//      mObjBox = mMotionBlur.getBox( getPosition() );

      MatrixF temp(true);
      temp.setColumn(3, mCurrDeltaBase);
      setTransform(temp);
   }
}


//--------------------------------------------------------------------------
// Advance time
//--------------------------------------------------------------------------
void EnergyProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   mMotionBlur.update( dt );
}


//--------------------------------------------------------------------------
U32 EnergyProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   return retMask;
}

void EnergyProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      // Initial update

      
      mathRead(*stream, &mCurrPosition);
      mathRead(*stream, &mCurrVelocity);
      mCurrTick = stream->readRangedU32(0, MaxLivingTicks);

      mInitialDirection = mCurrVelocity;
      mInitialDirection.normalizeSafe();
      
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
         // bounce against dynamic object on the server
         //
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
// Prep render
//--------------------------------------------------------------------------
bool EnergyProjectile::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
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

      state->insertRenderImage(image);
   }

   return false;
}


//--------------------------------------------------------------------------
// Render energy bolt
//--------------------------------------------------------------------------
void EnergyProjectile::renderObject(SceneState* state, SceneRenderImage *sri )
{

   if( mProjectileShape )
   {
      Parent::renderObject( state, sri );
      return;
   }
 
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);

   mMotionBlur.render( state->getCameraPosition() );

   if( !mHidden )
   {
      renderProjectile( state->getCameraPosition() );
      renderCrossSection( state->getCameraPosition() );
   }

   glDepthMask(GL_TRUE);
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   dglSetCanonicalState();

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//--------------------------------------------------------------------------
// Render projectile
//--------------------------------------------------------------------------
void EnergyProjectile::renderProjectile( const Point3F &camPos )
{
   Point3F dir = getVelocity();
   if( dir.isZero() ) return;
   
   Point3F pos = getRenderPosition();
   Point3F dirFromCam = pos - camPos;
   Point3F crossDir;
   mCross( dirFromCam, dir, &crossDir );
   crossDir.normalizeSafe();
   dir.normalize();


   F32 width = mDataBlock->scale.x;
   F32 length = mDataBlock->scale.y / 2.0;

   dir *= width;
   crossDir *= width;
   Point3F start = pos - length * dir;
   Point3F end = pos + length * dir;


   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glEnable(GL_TEXTURE_2D);

   glColor4f( 1.0, 1.0, 1.0, 1.0 );
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[0].getGLName() );

   glBegin(GL_QUADS);

      glTexCoord2f(0, 0);
      glVertex3fv( start + crossDir );

      glTexCoord2f(0, 1);
      glVertex3fv( start - crossDir );

      glTexCoord2f(1, 1);
      glVertex3fv( end - crossDir );

      glTexCoord2f(1, 0);
      glVertex3fv( end + crossDir );

   glEnd();

}

//--------------------------------------------------------------------------
// Render "cross section" so effect doesn't look flat when seen edge-on
//--------------------------------------------------------------------------
void EnergyProjectile::renderCrossSection( const Point3F &camPos )
{
   Point3F dir = getVelocity();
   if( dir.isZero() ) return;

   Point3F pos = getRenderPosition();
   Point3F dirFromCam = pos - camPos;
   dirFromCam.normalizeSafe();
   dir.normalize();

   F32 angle = mDot( dir, dirFromCam );
   if( angle > -mDataBlock->crossViewAng && angle < mDataBlock->crossViewAng ) return;


   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   MatrixF mat = MathUtils::createOrientFromDir( dir );
   mat.setPosition( getRenderPosition() );

   dglMultMatrix( &mat );


   glColor4f( 1.0, 1.0, 1.0, 1.0 );
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[1].getGLName() );

   glBegin(GL_QUADS);

      F32 width = mDataBlock->crossSize / 2.0;

      glTexCoord2f(0, 0);
      glVertex3f( -width, 0, -width );

      glTexCoord2f(0, 1);
      glVertex3f( width, 0, -width );

      glTexCoord2f(1, 1);
      glVertex3f( width, 0, width );

      glTexCoord2f(1, 0);
      glVertex3f( -width, 0, width );

   glEnd();


   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

}


void EnergyProjectile::collisionOn()
{

   if (mSourceIdTimeoutTicks && bool(mSourceObject) )
   {
      mSourceObject->enableCollision();
   }
   if (mSourceIdTimeoutTicks && bool(mVehicleObject))
   {
      mVehicleObject->enableCollision();
   }

}

void EnergyProjectile::collisionOff()
{

   if (mSourceIdTimeoutTicks && bool(mSourceObject))
   {
      mSourceObject->disableCollision();
   }
   if (mSourceIdTimeoutTicks && bool(mVehicleObject))
   {
      mVehicleObject->disableCollision();
   }

}
