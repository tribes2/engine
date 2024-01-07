//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projBomb.h"
#include "Core/bitStream.h"
#include "Sim/netConnection.h"
#include "game/shapeBase.h"
#include "Math/mathIO.h"
#include "console/consoleTypes.h"
#include "game/explosion.h"
#include "terrain/waterBlock.h"
#include "game/splash.h"
#include "game/particleEngine.h"
#include "dgl/dgl.h"
#include "sceneGraph/sceneState.h"
#include "Math/mathUtils.h"

IMPLEMENT_CO_NETOBJECT_V1(BombProjectile);
IMPLEMENT_CO_DATABLOCK_V1(BombProjectileData);

//--------------------------------------------------------------------------
//--------------------------------------
//
BombProjectileData::BombProjectileData()
{
   dMemset( textureName, 0, sizeof( textureName ) );
   dMemset( textureHandle, 0, sizeof( textureHandle ) );

   minRotSpeed.set( 0.0, 0.0, 0.0 );
   maxRotSpeed.set( 0.0, 0.0, 0.0 );
}

//--------------------------------------------------------------------------
BombProjectileData::~BombProjectileData()
{

}


//--------------------------------------------------------------------------
void BombProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("texture",        TypeString,    Offset( textureName,    BombProjectileData), NUM_TEX);
   addField("minRotSpeed",    TypePoint3F,   Offset( minRotSpeed,    BombProjectileData));
   addField("maxRotSpeed",    TypePoint3F,   Offset( maxRotSpeed,    BombProjectileData));
}


//--------------------------------------------------------------------------
// Preload data - load resources
//--------------------------------------------------------------------------
bool BombProjectileData::preload(bool server, char errorBuffer[256])
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
bool BombProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}

//--------------------------------------------------------------------------
void BombProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(minRotSpeed.x);
   stream->write(minRotSpeed.y);
   stream->write(minRotSpeed.z);
   stream->write(maxRotSpeed.x);
   stream->write(maxRotSpeed.y);
   stream->write(maxRotSpeed.z);

   U32 i;
   for( i=0; i<NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }

}

//--------------------------------------------------------------------------
void BombProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&minRotSpeed.x);
   stream->read(&minRotSpeed.y);
   stream->read(&minRotSpeed.z);
   stream->read(&maxRotSpeed.x);
   stream->read(&maxRotSpeed.y);
   stream->read(&maxRotSpeed.z);
   
   for( U32 i=0; i<NUM_TEX; i++ )
   {
      textureName[i] = stream->readSTString();
   }

}


//--------------------------------------------------------------------------
//--------------------------------------
//
BombProjectile::BombProjectile()
{

}

BombProjectile::~BombProjectile()
{
   //
}

//--------------------------------------------------------------------------
void BombProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


void BombProjectile::consoleInit()
{
   //
}


//--------------------------------------------------------------------------
bool BombProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mSourceIdTimeoutTicks = 1000;
   
   if( isClientObject() )
   {
      mRotSpeed.x = gRandGen.randF( mDataBlock->minRotSpeed.x, mDataBlock->maxRotSpeed.x );
      mRotSpeed.y = gRandGen.randF( mDataBlock->minRotSpeed.y, mDataBlock->maxRotSpeed.y );
      mRotSpeed.z = gRandGen.randF( mDataBlock->minRotSpeed.z, mDataBlock->maxRotSpeed.z );

      if( bool( mSourceObject ) )
      {
         setTransform( mSourceObject->getTransform() );
      }

   }

   mObjBox = Box3F( Point3F( -1, -1, -1 ), Point3F( 1.0, 1.0, 1.0 ) );
   resetWorldBox();

   return true;
}


//--------------------------------------------------------------------------
void BombProjectile::onRemove()
{
   Parent::onRemove();
}


//--------------------------------------------------------------------------
bool BombProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<BombProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
// Process Tick
//--------------------------------------------------------------------------
void BombProjectile::processTick(const Move* move)
{
   F32 timeLeft;
   RayInfo rInfo;
   Point3F oldPosition;
   Point3F newPosition;

   Parent::Parent::processTick(move);

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

      if (getContainer()->castRay(oldPosition, newPosition,
                                  csmDynamicCollisionMask | csmStaticCollisionMask,
                                  &rInfo) == true)
      {
         // explode
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

      }
      else
      {
         // No problems, just set the end position, and we're golden.
         mCurrPosition = newPosition;
         MatrixF xform(true);
         xform.setColumn(3, mCurrPosition);
         setTransform(xform);
      }

      if( !mSentInitialUpdate && determineSplash( oldPosition, newPosition ) )
      {
         mQuickSplash = true;
      }
      
      if (mSourceIdTimeoutTicks && bool(mSourceObject))
         mSourceObject->enableCollision();
      if (mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->enableCollision();
   }
   else
   {
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

      if (getContainer()->castRay(oldPosition, newPosition,
                                  csmDynamicCollisionMask | csmStaticCollisionMask,
                                  &rInfo) == true)
      {
         // explode
         MatrixF xform(true);
         xform.setColumn(3, rInfo.point);
         setTransform(xform);
         mCurrPosition    = rInfo.point;
         mCurrVelocity    = Point3F(0, 0, 0);

         explode(rInfo.point, rInfo.normal, rInfo.object->getType() );

      }
      else
      {
         // No problems, just set the end position, and we're golden.
         mCurrDeltaBase  = newPosition;
         mCurrBackVector = mCurrPosition - newPosition;

         if( !mInWater )
         {
            emitParticles(mCurrPosition, newPosition, mCurrVelocity, TickMs);
         }

         mCurrPosition   = newPosition;
      }

      if( determineSplash( oldPosition, newPosition ) )
      {
         createSplash( mSplashPos );
      }

      if (bool(mSourceObject))
         mSourceObject->enableCollision();
      if (mSourceIdTimeoutTicks && bool(mVehicleObject))
         mVehicleObject->enableCollision();
   }
}


//--------------------------------------------------------------------------
// Advance time
//--------------------------------------------------------------------------
void BombProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   rotate( dt );
}

//----------------------------------------------------------------------------
// Rotate bomb
//----------------------------------------------------------------------------
void BombProjectile::rotate( F32 dt )
{
   MatrixF curTrans = getRenderTransform();
   curTrans.setPosition( Point3F(0.0, 0.0, 0.0) );
   
   Point3F curAngles = mRotSpeed * dt * M_PI/180.0;
   MatrixF rotMatrix( EulerF( curAngles.x, curAngles.y, curAngles.z ) );
   
   curTrans.mul( rotMatrix );
   curTrans.setPosition( getRenderPosition() );
   setRenderTransform( curTrans );
}

//--------------------------------------------------------------------------
// Prep render
//--------------------------------------------------------------------------
bool BombProjectile::prepRenderImage(SceneState* state, const U32 stateKey,
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
void BombProjectile::renderObject(SceneState* state, SceneRenderImage *sri )
{
   if( mHidden )
      return;

   if( mProjectileShape )
   {
      Parent::renderObject( state, sri );
      return;
   }
}


