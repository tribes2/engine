//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projRepair.h"
#include "core/bitStream.h"
#include "console/consoleTypes.h"
#include "game/shapeBase.h"
#include "sim/netConnection.h"
#include "dgl/dgl.h"
#include "platformWIN32/platformGL.h"
#include "scenegraph/sceneState.h"
#include "math/mRandom.h"
#include "dgl/splineUtil.h"
#include "game/gameConnection.h"

IMPLEMENT_CO_DATABLOCK_V1(RepairProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(RepairProjectile);

//--------------------------------------------------------------------------
//--------------------------------------
//
RepairProjectileData::RepairProjectileData()
{
   beamRange         = 10.0f;
   beamWidth         = 0.15;
   numSegments       = 20;
   texRepeat         = 1.0/5.0;
   beamSpeed         = 1.0;
   blurFreq          = 10.0;
   blurLifetime      = 1.0;
   cutoffAngle       = 40.0;
   
   textureNames[0]   = "special/redbump2";
   textureNames[1]   = "special/redflare";

}

//--------------------------------------------------------------------------
RepairProjectileData::~RepairProjectileData()
{

}


//--------------------------------------------------------------------------
bool RepairProjectileData::calculateAim(const Point3F& targetPos,
                                        const Point3F& /*targetVel*/,
                                        const Point3F& sourcePos,
                                        const Point3F& /*sourceVel*/,
                                        Point3F*       outputVectorMin,
                                        F32*           outputMinTime,
                                        Point3F*       outputVectorMax,
                                        F32*           outputMaxTime)
{
   //make sure the source and target points are within range
   if ((targetPos - sourcePos).len() >= beamRange)
      return false;
   else
   {
      *outputVectorMin = targetPos - sourcePos;
      *outputMinTime   = 0.001f;
      outputVectorMin->normalize();
      *outputVectorMax = *outputVectorMin;
      *outputMaxTime   = *outputMinTime;

      return true;
   }
}


//--------------------------------------------------------------------------
void RepairProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("beamRange",         TypeF32,    Offset(beamRange,         RepairProjectileData));
   addField("beamWidth",         TypeF32,    Offset(beamWidth,         RepairProjectileData));
   addField("beamSpeed",         TypeF32,    Offset(beamSpeed,         RepairProjectileData));
   addField("texRepeat",         TypeF32,    Offset(texRepeat,         RepairProjectileData));
   addField("textures",          TypeString, Offset(textureNames,      RepairProjectileData), RT_NUM_TEX);
   addField("numSegments",       TypeS32,    Offset(numSegments,       RepairProjectileData));
   addField("blurFreq",          TypeF32,    Offset(blurFreq,          RepairProjectileData));
   addField("blurLifetime",      TypeF32,    Offset(blurLifetime,      RepairProjectileData));
   addField("cutoffAngle",       TypeF32,    Offset(cutoffAngle,       RepairProjectileData));


}


//--------------------------------------------------------------------------
bool RepairProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (beamRange < 2.0) {
      Con::warnf(ConsoleLogEntry::General, "RepairProjectileData(%s)::onAdd: beamRange must be >= 2", getName());
      beamRange = 2.0;
   }

   return true;
}

//--------------------------------------------------------------------------
bool RepairProjectileData::preload( bool server, char errorBuffer[256] )
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if( server == false )
   {
      for( int i=0; i<RT_NUM_TEX; i++ )
      {
         if( textureNames[i][0] )
         {
            textureList[i] = TextureHandle( textureNames[i], MeshTexture );
         }
      }

   }
   else
   {
      for( int i=0; i<RT_NUM_TEX; i++ )
      {
         textureList[i] = TextureHandle();  // set default NULL tex
      }
   }

   return true;
}


//--------------------------------------------------------------------------
void RepairProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(beamRange);
   stream->write(beamWidth);
   stream->write(numSegments);
   stream->write(beamSpeed);
   stream->write(texRepeat);
   stream->write(blurFreq);
   stream->write(blurLifetime);
   stream->write(cutoffAngle);


   for( int i=0; i<RT_NUM_TEX; i++ )
   {
      stream->writeString( textureNames[i] );
   }
}

//--------------------------------------------------------------------------
void RepairProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&beamRange);
   stream->read(&beamWidth);
   stream->read(&numSegments);
   stream->read(&beamSpeed);
   stream->read(&texRepeat);
   stream->read(&blurFreq);
   stream->read(&blurLifetime);
   stream->read(&cutoffAngle);


   for( int i=0; i<RT_NUM_TEX; i++ )
   {
     textureNames[i] = stream->readSTString();
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
RepairProjectile::RepairProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);

   mElapsedTime = 0.0;
   mInitialHit = true;
   mTimeSinceLastBlur = 0.00;
}

//--------------------------------------------------------------------------
RepairProjectile::~RepairProjectile()
{
}

//--------------------------------------------------------------------------
void RepairProjectile::initPersistFields()
{
   Parent::initPersistFields();

   addField("targetObject", TypeS32, Offset(mRepairingObjectId, RepairProjectile));
}


//--------------------------------------------------------------------------
void RepairProjectile::consoleInit()
{
   //
}



//--------------------------------------------------------------------------
bool RepairProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (isServerObject())
   {
      AssertFatal(bool(mSourceObject), "Oh, crap.");

      ShapeBase* ptr;
      if (Sim::findObject(mRepairingObjectId, ptr))
      {
         mRepairingObject = ptr;
      }
      else
      {
         if (mRepairingObjectId != -1)
            Con::errorf(ConsoleLogEntry::General, "Projectile::onAdd: mRepairingObjectId is invalid");
         mRepairingObject = NULL;
      }
      
      mObjBox.min = mSourceObject->getPosition();
      mObjBox.max = mSourceObject->getPosition() + Point3F( 1.0, 1.0, 1.0 );
      resetWorldBox();

   }
   else
   {
      if( bool( mSourceObject ) && bool( mRepairingObject ) )
      {
         mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mCurrStartPoint);
         mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mCurrStartDir);
         mDesiredEndPoint.set(0, 0, 0);

         RayInfo rayInfo;
         if( gClientContainer.castRay(mCurrStartPoint, mCurrStartPoint + (mCurrStartDir*mDataBlock->beamRange), mRepairingObject->getType(), &rayInfo) == true )
         {
            if (rayInfo.object == (ShapeBase*)mRepairingObject)
            {
               mRepairingObject->getWorldBox().getCenter(&mDesiredEndPoint);
               mDesiredEndPoint = rayInfo.point - mDesiredEndPoint;
            }
         }

         mCurrEndPoint = mDesiredEndPoint;

         // resize object box of beam
         mObjBox.min = mCurrEndPoint;
         mObjBox.max = mCurrEndPoint;
         mObjBox.min.setMin(mCurrStartPoint);
         mObjBox.max.setMax(mCurrStartPoint);
         resetWorldBox();
      }
   }

   addToScene();

   return true;
}


//--------------------------------------------------------------------------
void RepairProjectile::onRemove()
{
   removeFromScene();
   //

   Parent::onRemove();
}


//--------------------------------------------------------------------------
bool RepairProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<RepairProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   // Todo: Uncomment if this is a "leaf" class
   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
U32 RepairProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & GameBase::InitialUpdateMask)) {
      // Have to write the source and the dest to the client, let's make sure it has a
      //  ghost on the other side...
      S32 ghostIndexSource = con->getGhostIndex(mSourceObject);
      S32 ghostIndexDest   = con->getGhostIndex(mRepairingObject);
      if (ghostIndexSource == -1 || ghostIndexDest == -1) {
         // One of the two isn't on the client.  Crap.
         stream->writeFlag(false);
      } else {
         stream->writeFlag(true);

         stream->writeRangedU32(U32(ghostIndexSource), 0, NetConnection::MaxGhostCount);
         stream->writeRangedU32(U32(mSourceObjectSlot),
                                0, ShapeBase::MaxMountedImages - 1);
         stream->writeRangedU32(U32(ghostIndexDest), 0, NetConnection::MaxGhostCount);
      }
   } else {

   }

   return retMask;
}

void RepairProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) {
      if (stream->readFlag()) {
         mSourceObjectId    = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         mSourceObjectSlot  = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);
         mRepairingObjectId = stream->readRangedU32(0, NetConnection::MaxGhostCount);

         NetObject* pObject = con->resolveGhost(mSourceObjectId);
         if (pObject != NULL)
            mSourceObject = dynamic_cast<ShapeBase*>(pObject);
         if (bool(mSourceObject) == false)
            Con::errorf(ConsoleLogEntry::General, "RepairProjectile::unpackUpdate: could not resolve source ghost properly on initial update");

         pObject = con->resolveGhost(mRepairingObjectId);
         if (pObject != NULL)
            mRepairingObject = dynamic_cast<ShapeBase*>(pObject);
         if (bool(mRepairingObject) == false)
            Con::errorf(ConsoleLogEntry::General, "RepairProjectile::unpackUpdate: could not resolve dest ghost properly on initial update");
      } else {
         mSourceObjectId    = 0;
         mSourceObjectSlot  = 0;
         mRepairingObjectId = 0;

         mSourceObject    = NULL;
         mRepairingObject = NULL;
      }
   } else {
      // Other
   }
}

//--------------------------------------------------------------------------
void RepairProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if (!bool(mSourceObject) || !bool(mRepairingObject))
      return;

   mElapsedTime += dt;
   mTimeSinceLastBlur += dt;



   // We start out with a good relative end point.
   //  Our goal is to not invalidate that, but to update it if the object
   //  points in a new direction...

   if( bool(mSourceObject) && bool(mRepairingObject) )
   {
      mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mCurrStartPoint);
      mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mCurrStartDir);

      RayInfo rayInfo;
      if (getContainer()->castRay(mCurrStartPoint, mCurrStartPoint + (mCurrStartDir*mDataBlock->beamRange), mRepairingObject->getType(), &rayInfo) == true)
      {
         if (rayInfo.object == (ShapeBase*)mRepairingObject)
         {
            mDesiredEndPoint = rayInfo.point;
            if( mInitialHit )
            {
               mInitialHit = false;
               mCurrEndPoint = mDesiredEndPoint;
            }
         }
      }
      else
      {
         // No hit.  Just use the old endpoint.
      }
      
      // resize object box of beam
      mObjBox.min = mCurrEndPoint;
      mObjBox.max = mCurrEndPoint;
      mObjBox.min.setMin(mCurrStartPoint);
      mObjBox.max.setMax(mCurrStartPoint);
      resetWorldBox();
   }
   else
   {
      mCurrStartPoint.set(0, 0, 0);
      mCurrEndPoint.set(0, 0, 0);
      mObjBox.min.set(0, 0, 0);
      mObjBox.max.set(0, 0, 0);
   }

   MatrixF xform = getTransform();
   setTransform(xform);

   Point3F endMoveVec = mDesiredEndPoint - mCurrEndPoint;
   endMoveVec *= 2.0 * dt;
   mCurrEndPoint += endMoveVec;

   updateBlur( dt );


}

//--------------------------------------------------------------------------
// Update blur
//--------------------------------------------------------------------------
void RepairProjectile::updateBlur( F32 dt )
{
   if( isServerObject() ) return;

   // update blur
   for( int j=0; j<RP_NUM_BLUR; j++ )
   {
      mBlurList[j].elapsedTime += dt;
   }

   // create blur line
   if( mTimeSinceLastBlur > 1.0/mDataBlock->blurFreq )
   {
      mTimeSinceLastBlur -= (1.0 / mDataBlock->blurFreq);

      Point3F cPoints[3];
      cPoints[0] = mCurrStartPoint;
      cPoints[1] = mCurrStartPoint + mCurrStartDir * (mCurrStartPoint - mCurrEndPoint).len();
      cPoints[2] = mCurrEndPoint;

      SplCtrlPts ctrlPoints;
      ctrlPoints.submitPoints( cPoints, 3 );
      
      QuadPatch qPatch;
      qPatch.submitControlPoints( ctrlPoints );


      for( int i=0; i<RP_NUM_BLUR; i++ )
      {
         if( mBlurList[i].elapsedTime > mBlurList[i].lifetime )
         {
            mBlurList[i].qPatch.submitControlPoints( ctrlPoints );
            mBlurList[i].elapsedTime = 0.0;
            mBlurList[i].lifetime = mDataBlock->blurLifetime;
            mBlurList[i].startAlpha = 0.5;;
            break;
         }
      }
      
   }

}

//--------------------------------------------------------------------------
bool RepairProjectile::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   if (!bool(mSourceObject) || !bool(mRepairingObject))
      return false;

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->isTranslucent = true;

      image->sortType = SceneRenderImage::EndSort;
      state->setImageRefPoint(this, image);

      state->insertRenderImage(image);
   }

   return false;
}

//--------------------------------------------------------------------------
// Render the effect
//--------------------------------------------------------------------------
void RepairProjectile::renderObject(SceneState* state, SceneRenderImage* /*sceneImage*/)
{
   if( mCurrStartPoint.equal( mCurrEndPoint ) ) return;

   // make sure beam doesn't twist up in odd fashion if player turns rapidly
   VectorF actionDir = mCurrEndPoint - mCurrStartPoint;
   actionDir.normalize();
   F32 angDiff = 90.0 - mDot( actionDir, mCurrStartDir ) * 90.0;
   if( angDiff > mDataBlock->cutoffAngle ) return;
   
   
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);


   glMatrixMode(GL_MODELVIEW);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);


   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glEnable(GL_TEXTURE_2D);

   glColor4f( 1.0, 1.0, 1.0, 1.0 );

   if( canRenderFlare( state->getCameraPosition() ) )
   {
      renderFlare( 0.6 );
   }

   QuadPatch qPatch;


   Point3F cPoints[3];
   cPoints[0] = mCurrStartPoint;
   cPoints[1] = mCurrStartPoint + mCurrStartDir * (mCurrStartPoint - mCurrEndPoint).len();
   cPoints[2] = mCurrEndPoint;

   SplCtrlPts ctrlPoints;
   ctrlPoints.submitPoints( cPoints, 3 );

   qPatch.submitControlPoints( ctrlPoints );

   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureList[RepairProjectileData::RT_BEAM].getGLName() );
   glColor4f( 1.0, 1.0, 1.0, 0.75 );

   SplineUtil::drawSplineBeam( state->getCameraPosition(), 20, 0.2, qPatch, -mElapsedTime * mDataBlock->beamSpeed, mDataBlock->texRepeat );


   SplineUtil::SplineBeamInfo sbi;
   
   Point3F camPos = state->getCameraPosition();
   sbi.camPos = &camPos;
   sbi.zeroAlphaStart = true;
   sbi.numSegments = mDataBlock->numSegments;
   sbi.uvOffset = -mElapsedTime * mDataBlock->beamSpeed;
   sbi.numTexRep = mDataBlock->texRepeat;

   for( int i=0; i<RP_NUM_BLUR; i++ )
   {
      if( mBlurList[i].elapsedTime < mBlurList[i].lifetime )
      {
         F32 alpha = mBlurList[i].startAlpha * ( mBlurList[i].startAlpha - ( mBlurList[i].elapsedTime / mBlurList[i].lifetime ) );
         F32 width = 0.3 + 0.5 * (mBlurList[i].elapsedTime / mBlurList[i].lifetime);

         sbi.color.set( 1.0, 0.0, 0.0, alpha );
         sbi.width = width;
         sbi.spline = &mBlurList[i].qPatch;

         SplineUtil::drawSplineBeam( sbi );
      }
   }

   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glDepthMask(GL_TRUE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------------------------------------------
// Checks to see if the flare is visible - returns true if it is
//--------------------------------------------------------------------------
bool RepairProjectile::canRenderFlare( const Point3F &camPos )
{

   if( bool(mRepairingObject) == false ) return false;


   // check if in first or third person mode
   bool firstPerson = true;

   GameConnection *gc = mSourceObject->getControllingClient();
   if( gc )
   {
      firstPerson = gc->isFirstPerson();
   }

   VectorF dirFromGun = mCurrEndPoint - mCurrStartPoint;
   dirFromGun.normalizeSafe();

   VectorF dirToFlare = mCurrEndPoint - camPos;
   dirToFlare.normalizeSafe();
   Point3F tweakedCamPos = camPos + dirToFlare * 0.5;

   if( mDot( dirFromGun, dirToFlare ) < -0.75 ) return false;

   bool rayHit = true;

   mRepairingObject->disableCollision();

   if( firstPerson )
   {
      mSourceObject->disableCollision();

      RayInfo rayInfo;
      rayHit = getContainer()->castRay( tweakedCamPos, mCurrEndPoint, mRepairingObject->getType(), &rayInfo );

      mSourceObject->enableCollision();
   }
   else
   {
      RayInfo rayInfo;
      rayHit = getContainer()->castRay( tweakedCamPos, mCurrEndPoint, mRepairingObject->getType(), &rayInfo );

   }

   mRepairingObject->enableCollision();


   return !rayHit;
}


//--------------------------------------------------------------------------
// Render flare
//--------------------------------------------------------------------------
void RepairProjectile::renderFlare( F32 flareSize )
{

   Point3F pos = mCurrEndPoint;

   glDisable(GL_DEPTH_TEST);
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureList[RepairProjectileData::RT_FLARE].getGLName() );

   glColor3f( 0.3333, 0.3333, 0.3333 );

   dglDrawBillboard( pos, flareSize * 2.0, mElapsedTime * .15 * M_PI * 2.0 );
   dglDrawBillboard( pos, flareSize * 1.7, -mElapsedTime * .10 * M_PI * 2.0 );
   dglDrawBillboard( pos, flareSize * 1.82, mElapsedTime * .05 * M_PI * 2.0 );

   glEnable(GL_DEPTH_TEST);
}

