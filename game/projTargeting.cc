//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projTargeting.h"
#include "core/bitStream.h"
#include "console/consoleTypes.h"
#include "platformWIN32/platformGL.h"
#include "dgl/dgl.h"
#include "scenegraph/sceneState.h"
#include "math/mathIO.h"
#include "game/shapeBase.h"
#include "game/gameConnection.h"

IMPLEMENT_CO_DATABLOCK_V1(TargetProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(TargetProjectile);

//--------------------------------------------------------------------------
//--------------------------------------
//
TargetProjectileData::TargetProjectileData()
{
   maxRifleRange = 1000;
   beamColor.set( 0.1, 1.0, 0.1 );
   coupleBeam = true;

   startBeamWidth = 1.0;
   beamFlareAngle = 3.0;
   minFlareSize = 0.0;
   maxFlareSize = 400.0;
   pulseBeamWidth = 0.5;
   pulseSpeed = 6.0;
   pulseLength = 0.150;

   dMemset( textureName, 0, sizeof( textureName ) );
}

TargetProjectileData::~TargetProjectileData()
{

}


//--------------------------------------------------------------------------
void TargetProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("maxRifleRange",       TypeF32,    Offset(maxRifleRange,       TargetProjectileData));
   addField("beamColor",           TypeColorF, Offset(beamColor,           TargetProjectileData));
   addField("textureName",         TypeString, Offset(textureName,         TargetProjectileData), TT_NUM_TEX);
   addField("startBeamWidth",      TypeF32,    Offset(startBeamWidth,      TargetProjectileData));
   addField("pulseBeamWidth",      TypeF32,    Offset(pulseBeamWidth,      TargetProjectileData));
   addField("beamFlareAngle",      TypeF32,    Offset(beamFlareAngle,      TargetProjectileData));
   addField("minFlareSize",        TypeF32,    Offset(minFlareSize,        TargetProjectileData));
   addField("maxFlareSize",        TypeF32,    Offset(maxFlareSize,        TargetProjectileData));
   addField("pulseSpeed",          TypeF32,    Offset(pulseSpeed,          TargetProjectileData));
   addField("pulseLength",         TypeF32,    Offset(pulseLength,         TargetProjectileData));
   addField("coupleBeam",          TypeBool,   Offset(coupleBeam,          TargetProjectileData));
}


//--------------------------------------------------------------------------
bool TargetProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (maxRifleRange < 10.0 || maxRifleRange > 2000.0f) {
      Con::warnf(ConsoleLogEntry::General, "TargetProjectileData(%s)::onAdd: sniper rifle maxRange must be have range [10, 2000]", getName());
      maxRifleRange = maxRifleRange < 10.0 ? 10.0 : 2000;
   }
   beamColor.clamp();

   return true;
}


//--------------------------------------------------------------------------
bool TargetProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (server == false)
   {
      for( int i=0; i<TT_NUM_TEX; i++ )
      {
         if (textureName[i] && textureName[i][0])
         {
            textureHandle[i] = TextureHandle(textureName[i], MeshTexture );
         }
      }

   }
   else
   {
      for( int i=0; i<TT_NUM_TEX; i++ )
      {
         textureHandle[i] = TextureHandle();  // set default NULL tex
      }
   }

   return true;
}


//--------------------------------------------------------------------------
bool TargetProjectileData::calculateAim(const Point3F& targetPos,
                                        const Point3F& /*targetVel*/,
                                        const Point3F& sourcePos,
                                        const Point3F& /*sourceVel*/,
                                        Point3F*       outputVectorMin,
                                        F32*           outputMinTime,
                                        Point3F*       outputVectorMax,
                                        F32*           outputMaxTime)
{
   if ((targetPos - sourcePos).len() > maxRifleRange) {
      outputVectorMin->set(0, 0, 1);
      outputVectorMax->set(0, 0, 1);
      *outputMinTime = -1;
      *outputMaxTime = -1;

      return false;
   }

   *outputVectorMin = targetPos - sourcePos;
   *outputMinTime   = 0.0;
   outputVectorMin->normalizeSafe();
   *outputVectorMax = *outputVectorMin;
   *outputMaxTime   = *outputMinTime;

   return true;
}


//--------------------------------------------------------------------------
void TargetProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(maxRifleRange);
   stream->write(beamColor);
   stream->write(startBeamWidth);
   stream->write(pulseBeamWidth);
   stream->write(beamFlareAngle);
   stream->write(minFlareSize);
   stream->write(maxFlareSize);
   stream->write(pulseSpeed);
   stream->write(pulseLength);

   for( int i=0; i<TT_NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }

}

//--------------------------------------------------------------------------
void TargetProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   
   stream->read(&maxRifleRange);
   stream->read(&beamColor);
   stream->read(&startBeamWidth);
   stream->read(&pulseBeamWidth);
   stream->read(&beamFlareAngle);
   stream->read(&minFlareSize);
   stream->read(&maxFlareSize);
   stream->read(&pulseSpeed);
   stream->read(&pulseLength);

   for( int i=0; i<TT_NUM_TEX; i++ )
   {
      textureName[i] = stream->readSTString();
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
TargetProjectile::TargetProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable|ScopeAlways);

   mClientTotalWarpTicks = 0;
   mClientWarpTicks      = 0;
   mElapsedTime = 0.0;
}

TargetProjectile::~TargetProjectile()
{
   //
}

//--------------------------------------------------------------------------
void TargetProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


static const char* cGetTargetPoint(SimObject *obj, S32, const char **)
{
   TargetProjectile *targ = static_cast<TargetProjectile*>(obj);
   
   //find the target
   Point3F targPoint;
   U32 team;

   char* returnBuffer = Con::getReturnBuffer(256);
   if (targ->getTarget(&targPoint, &team))
      dSprintf(returnBuffer, 256, "%f %f %f %d", targPoint.x, targPoint.y, targPoint.z, team);
   else
      dSprintf(returnBuffer, 256, "0 0 0 -1");
   return returnBuffer;
}   

void TargetProjectile::consoleInit()
{
   Con::addCommand("TargetProjectile", "getTargetPoint", cGetTargetPoint, "targ.getTargetPoint()", 2, 2);
}


//--------------------------------------------------------------------------
bool TargetProjectile::calculateImpact(float    /*simTime*/,
                                       Point3F& pointOfImpact,
                                       float&   impactTime)
{
   impactTime    = 0;

   if (mTruncated) {
      pointOfImpact = mBeam.mData.endPos;
      return true;
   } else {
      pointOfImpact.set(0, 0, 0);
      return false;
   }
}


//--------------------------------------------------------------------------
bool TargetProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (isServerObject()) {
      if (bool(mSourceObject))
         mSourceObject->disableCollision();

      mBeam.mData.endPos = mInitialPosition + mInitialDirection * mDataBlock->maxRifleRange;
      RayInfo rayInfo;
      if (gServerContainer.castRay(mInitialPosition, mBeam.mData.endPos,
                                   (csmStaticCollisionMask | csmDynamicCollisionMask | WaterObjectType),
                                   &rayInfo) == true) {
         mBeam.mData.endPos = rayInfo.point;
         mTruncated   = true;
      } else {
         mTruncated = false;
      }

      if (bool(mSourceObject))
         mSourceObject->enableCollision();

      addToSet("ServerTargetSet");
   } else {
      addToSet("ClientTargetSet");
   }

   mObjBox.min.set(-1e7, -1e7, -1e7);
   mObjBox.max.set( 1e7,  1e7,  1e7);

   MatrixF temp(true);
   setTransform(temp);
   setRenderTransform(temp);

   addToScene();

   return true;
}


void TargetProjectile::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}


bool TargetProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<TargetProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   // Todo: Uncomment if this is a "leaf" class
   //scriptOnNewDataBlock();
   return true;
}

//--------------------------------------------------------------------------
bool TargetProjectile::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   SceneRenderImage* image = new SceneRenderImage;
   image->obj = this;
   image->isTranslucent = true;
   image->sortType = SceneRenderImage::EndSort;
   state->insertRenderImage(image);

   return false;
}


//--------------------------------------------------------------------------
void TargetProjectile::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   state->setupBaseProjection();
   updateBeamData( mBeam.mData, state->getCameraPosition() );

   // tweak end position so it doesn't intersect camera
   mBeam.adjustEdge( mBeam.mData );

   // render the flare
   F32 flareScale = 0.0;
   bool result = checkForFlare( flareScale, state->getCameraPosition() );


   if( result )
   {
      ColorF flareColor = mDataBlock->beamColor;
      flareColor.alpha = 1.0;
      renderFlare( flareColor, flareScale );
   }

   if( canRenderEndFlare( state->getCameraPosition() ) )
   {
      renderEndFlare( 0.3 );
   }
   

   renderBeam( mBeam, 0.0 );


   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------------------------------------------
// Returns true if need to render flare.
//--------------------------------------------------------------------------
bool TargetProjectile::checkForFlare( F32 &flareScale, const Point3F &camPos )
{

   if( ( mBeam.mData.angToCamPos < -0.5 ) && ( mBeam.mData.angToCamDir < -0.5 ) )
   {

      if( mSourceObject )
      {
         mSourceObject->disableCollision();
      }

      // cast ray to make sure beam source is visible
      RayInfo rayInfo;
      bool rayHit = gClientContainer.castRay( camPos, mInitialPosition,
                                              (csmStaticCollisionMask | csmDynamicCollisionMask | WaterObjectType),
                                              &rayInfo);

      if( mSourceObject )
      {
         mSourceObject->enableCollision();
      }

      if( !rayHit )
      {

         flareScale = 0.0;
         F32 flareAngle = -mCos( mDegToRad( mDataBlock->beamFlareAngle ) );

         if( mBeam.mData.angToCamPos >= flareAngle )
         {
            flareScale = mDataBlock->minFlareSize / mDataBlock->maxFlareSize;
         }
         else
         {

            F32 angScale = 1.0 / (-1.0 - flareAngle);
            F32 angDiff = mBeam.mData.angToCamPos - flareAngle;
            F32 camPosToBeamColScale = 1.0 - (angDiff * angScale);  // 0.0 - 1.0, 1.0 if beam is directly at camera, 0.0 if at beamFlareAngle

            flareScale = 1.0 - camPosToBeamColScale;

         }
         return true;
      }

      return false;
   }

   return false;
}


//--------------------------------------------------------------------------
// Checks to see if the end flare is visible - returns true if it is
//--------------------------------------------------------------------------
bool TargetProjectile::canRenderEndFlare( const Point3F &camPos )
{
   // check if in first or third person mode
   bool firstPerson = true;

   GameConnection * gc = mSourceObject ? mSourceObject->getControllingClient() : NULL;
   if( gc )
   {
      firstPerson = gc->isFirstPerson();
   }

   bool rayHit = true;

   Point3F endPos = mBeam.mData.endPos + mBeam.mData.direction * 0.2;

   VectorF dirToFlare = endPos - camPos;
   dirToFlare.normalizeSafe();
   
   Point3F tweakedCamPos = camPos + dirToFlare * 0.5;

   
   if( firstPerson && mSourceObject )
   {
      mSourceObject->disableCollision();
   }

   RayInfo rayInfo;
   rayHit = getContainer()->castRay( mBeam.mData.startPos, endPos, -1, &rayInfo );

   if( rayHit )
   {
      SceneObject *obj = rayInfo.object;

      obj->disableCollision();

      rayHit = getContainer()->castRay( tweakedCamPos, endPos, -1, &rayInfo );

      obj->enableCollision();
   }


   if( firstPerson && mSourceObject )
   {
      mSourceObject->enableCollision();
   }


   return !rayHit;
}


//--------------------------------------------------------------------------
void TargetProjectile::updateBeamData( BeamData &beamData, const Point3F &camPos )
{
   beamData.startPos = mInitialPosition;
   beamData.endRenderPos = beamData.endPos;
   beamData.dirFromCam = beamData.startPos - camPos;
   beamData.dirFromCam.normalizeSafe();

   MatrixF mv;
   dglGetModelview(&mv);
   Point3F camLookDir;
   mv.getRow(1, &camLookDir);


   mCross( beamData.dirFromCam, beamData.direction, &beamData.axis);

   if( beamData.axis.isZero() )
   {
      Point3D dirFromCam( beamData.dirFromCam.x, beamData.dirFromCam.y, beamData.dirFromCam.z );
      Point3D direction( beamData.direction.x, beamData.direction.y, beamData.direction.z );
      
      Point3D newAxis;
      mCross( dirFromCam, direction, &newAxis);

      if( newAxis.isZero() )
      {
         beamData.axis.set( 0.0, 0.0, 1.0 );
      }
      else
      {
         newAxis.normalize();

         beamData.axis.x = newAxis.x;
         beamData.axis.y = newAxis.y;
         beamData.axis.z = newAxis.z;
      }
   }
   else
   {
      beamData.axis.normalize();
   }
   
   beamData.angToCamPos = mDot( beamData.dirFromCam, beamData.direction );
   beamData.angToCamDir = mDot( camLookDir, beamData.direction );
   beamData.length = (beamData.endPos - beamData.startPos).magnitudeSafe();
}

//--------------------------------------------------------------------------
void TargetProjectile::renderBeam( WeaponBeam &beam, F32 percentDone )
{

   if( !beam.mData.onEdge )
   {

      // set render state
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);
      glDisable(GL_CULL_FACE);
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      // set color
      ColorF beamColor = mDataBlock->beamColor;
      beamColor.alpha  = 1.0 - percentDone;
      glColor4fv( beamColor );

      // render pulse beam
      glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[TargetProjectileData::TT_PULSE].getGLName());

      F32 width = mDataBlock->pulseBeamWidth;
      F32 pulseOffset = mElapsedTime * -mDataBlock->pulseSpeed;

      F32 beamLength = (beam.mData.endRenderPos - mInitialPosition).magnitudeSafe();
      F32 numWrap = beamLength * mDataBlock->pulseLength;

      beam.render( width, pulseOffset, numWrap );


      // render main beam
      width = mDataBlock->startBeamWidth;
      width += (1.0 - width) * percentDone;

      glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[TargetProjectileData::TT_BEAM].getGLName());

      beam.render( width );

      
      // restore state
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glDepthMask(GL_TRUE);
      glEnable(GL_DEPTH_TEST);

   }

}

//--------------------------------------------------------------------------
void TargetProjectile::renderFlare( ColorF flareColor, F32 flareScale )
{

   F32 flareSize = mDataBlock->maxFlareSize;
   flareSize *= flareScale;
   if( flareSize == 0.0 ) return;

   Point3F screenPoint;
   dglPointToScreen( mInitialPosition, screenPoint );


   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   RectI viewport;
   dglGetViewport(&viewport);
   dglSetClipRect( viewport );


   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDepthMask(GL_TRUE);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   glColor4fv(flareColor);
   glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[TargetProjectileData::TT_FLARE].getGLName());

   F32 ang = mElapsedTime;

   dglDraw2DSquare( Point2F( screenPoint.x, screenPoint.y ), flareSize, ang );
   dglDraw2DSquare( Point2F( screenPoint.x, screenPoint.y ), flareSize * .75, -ang );


   glEnable(GL_DEPTH_TEST);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();

}

//--------------------------------------------------------------------------
// Render flare
//--------------------------------------------------------------------------
void TargetProjectile::renderEndFlare( F32 flareSize )
{
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);

   Point3F pos = mBeam.mData.endPos;


   glDisable(GL_DEPTH_TEST);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[TargetProjectileData::TT_END_FLARE].getGLName() );

   glColor4f( 0.0, 1.0, 0.0, 1.0 );

   dglDrawBillboard( pos, flareSize * 2.0, 0.0 );

   glEnable(GL_DEPTH_TEST);
}

//--------------------------------------------------------------------------
void TargetProjectile::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isClientObject() && mClientWarpTicks != 0)
   {
      mClientWarpTicks--;
      if( mSourceObject )
      {
         mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mInitialPosition);
      }
   }
   else
   {
      if (isServerObject())
      {
         if (bool(mSourceObject))
         {
            mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mInitialPosition);
            mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mInitialDirection);
            mSourceObject->disableCollision();

            mBeam.mData.endPos = mInitialPosition + mInitialDirection * mDataBlock->maxRifleRange;
            RayInfo rayInfo;
            if (gServerContainer.castRay(mInitialPosition, mBeam.mData.endPos,
                                         (csmStaticCollisionMask | csmDynamicCollisionMask | WaterObjectType),
                                         &rayInfo) == true) {
               mBeam.mData.endPos = rayInfo.point;
               mTruncated   = true;
            } else {
               mTruncated = false;
            }
            mSourceObject->enableCollision();

            if ((mCurrTick % 8) == 0)
               setMaskBits(ResetEndPointMask);

            MatrixF temp(true);
            temp.setPosition( mBeam.mData.endPos );
            setTransform(temp);
         }
      }
   }
}


//--------------------------------------------------------------------------
void TargetProjectile::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);

   if (mClientWarpTicks != 0 && mClientOwned == false)
   {
      // Warp the end point
      F32 factor = 1.0 - (F32(mClientWarpTicks) / F32(mClientTotalWarpTicks));

      if( mDataBlock->coupleBeam )
      {
         mBeam.mData.endPos.interpolate(mClientWarpFrom, mClientWarpTo, factor);
      }

   }
   else
   {
      if (mClientOwned && bool(mSourceObject))
      {

         mSourceObject->disableCollision();

         if( mDataBlock->coupleBeam )
         {
            // We're the shooter recast the end point...
            mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mInitialPosition);
            mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mInitialDirection);

            mBeam.mData.endPos = mInitialPosition + mInitialDirection * mDataBlock->maxRifleRange;
         }

         RayInfo rayInfo;
         bool rayHit = gClientContainer.castRay(mInitialPosition, mBeam.mData.endPos,
                                                (csmStaticCollisionMask | csmDynamicCollisionMask | WaterObjectType),
                                                &rayInfo);
         if( rayHit )
         {
            if( mDataBlock->coupleBeam )
            {
               mBeam.mData.endPos = rayInfo.point;
            }
            mTruncated   = true;
         } 
         else 
         {
            mTruncated = false;
         }
         mSourceObject->enableCollision();
      }
      else
      {
         // Just set the last position...
         if( mDataBlock->coupleBeam )
         {
            mBeam.mData.endPos = mClientWarpTo;
         }
      }
   }

   MatrixF temp(true);
   temp.setPosition( mBeam.mData.endPos );
   setTransform(temp);

   mBeam.mData.direction = mBeam.mData.endPos - mInitialPosition;
   mBeam.mData.direction.normalizeSafe();
}

//--------------------------------------------------------------------------
void TargetProjectile::advanceTime(F32 dt)
{
   mElapsedTime += dt;
}


//--------------------------------------------------------------------------
void TargetProjectile::setClientWarp(const Point3F& newEndPoint)
{
   mClientWarpFrom       = mBeam.mData.endPos;
   mClientWarpTo         = newEndPoint;
   mClientTotalWarpTicks = 10;
   mClientWarpTicks      = 10;
}


//--------------------------------------------------------------------------
bool TargetProjectile::getTarget(Point3F* pTarget, U32* pTeamId)
{
   if (mTruncated == true) {
      *pTarget = mBeam.mData.endPos;
      *pTeamId = 0;
      return true;
   } else {
      return false;
   }
}


//--------------------------------------------------------------------------
U32 TargetProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag((mask & GameBase::InitialUpdateMask) != 0)) {
      mathWrite(*stream, mInitialPosition);
      mathWrite(*stream, mBeam.mData.endPos);
      stream->writeFlag(mTruncated);

      if (bool(mSourceObject)) {
         // Potentially have to write this to the client, let's make sure it has a
         //  ghost on the other side...
         bool isFiredOnClient = mSourceObject->getControllingClient() == con;

         S32 ghostIndex = con->getGhostIndex(mSourceObject);
         if (stream->writeFlag(ghostIndex != -1)) {
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
            stream->writeRangedU32(U32(mSourceObjectSlot),
                                   0, ShapeBase::MaxMountedImages - 1);
            stream->writeFlag(isFiredOnClient);
         }
      } else {
         stream->writeFlag(false);
      }
   } else {
      // Swinging around...
      if (bool(mSourceObject)) {
         // Potentially have to write this to the client, let's make sure it has a
         //  ghost on the other side...
         bool isFiredOnClient = mSourceObject->getControllingClient() == con;
         S32 ghostIndex = con->getGhostIndex(mSourceObject);
         if (ghostIndex != -1) {
            stream->writeFlag(true);
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
            stream->writeRangedU32(U32(mSourceObjectSlot),
                                   0, ShapeBase::MaxMountedImages - 1);
            stream->writeFlag(isFiredOnClient);
         } else {
            stream->writeFlag(false);
            mathWrite(*stream, mInitialPosition);
         }
      } else {
         stream->writeFlag(false);
         mathWrite(*stream, mInitialPosition);
      }

      mathWrite(*stream, mBeam.mData.endPos);
      stream->writeFlag(mTruncated);
   }

   return retMask;
}

//--------------------------------------------------------------------------
void TargetProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) {
      mathRead(*stream, &mInitialPosition);
      mathRead(*stream, &mBeam.mData.endPos);
      mClientWarpFrom       = mBeam.mData.endPos;
      mClientWarpTo         = mBeam.mData.endPos;
      mClientTotalWarpTicks = 0;
      mClientWarpTicks      = 0;


      mTruncated = stream->readFlag();

      mClientOwned = false;
      if (stream->readFlag()) {
         mSourceObjectId   = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         mSourceObjectSlot = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);
         mClientOwned      = stream->readFlag();

         NetObject* pObject = con->resolveGhost(mSourceObjectId);
         if (pObject != NULL)
            mSourceObject = dynamic_cast<ShapeBase*>(pObject);
      } else {
         mSourceObjectId   = -1;
         mSourceObjectSlot = -1;
         mSourceObject     = NULL;
      }
   } 
   else
   {
      // Swinging around...
      mClientOwned = false;
      if (stream->readFlag()) {
         mSourceObjectId   = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         mSourceObjectSlot = stream->readRangedU32(0, ShapeBase::MaxMountedImages - 1);
         mClientOwned      = stream->readFlag();

         NetObject* pObject = con->resolveGhost(mSourceObjectId);
         if (pObject != NULL)
            mSourceObject = dynamic_cast<ShapeBase*>(pObject);
      } else {
         mSourceObjectId   = -1;
         mSourceObjectSlot = -1;
         mSourceObject     = NULL;

         mathRead(*stream, &mInitialPosition);
      }

      Point3F newEndPoint;
      mathRead(*stream, &newEndPoint);
      mTruncated = stream->readFlag();

      if (mClientOwned == false)
         setClientWarp(newEndPoint);
   }

   mBeam.mData.direction = mBeam.mData.endPos - mInitialPosition;
   mBeam.mData.direction.normalizeSafe();
}

