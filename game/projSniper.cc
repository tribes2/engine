//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projSniper.h"
#include "Core/bitStream.h"
#include "console/consoleTypes.h"
#include "Math/mathIO.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"
#include "game/explosion.h"
#include "terrain/waterBlock.h"

#include "sceneGraph/sceneState.h"
#include "sceneGraph/sceneGraph.h"
#include "dgl/dgl.h"
#include "PlatformWin32/platformGL.h"
#include "game/splash.h"


#define COUPLE_BEAM 1

IMPLEMENT_CO_DATABLOCK_V1(SniperProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(SniperProjectile);


//**************************************************************************
// Sniper Projectile Data
//**************************************************************************
SniperProjectileData::SniperProjectileData()
{
   maxRifleRange       = 1000;
   rifleHeadMultiplier = 1.2;
   beamColor.set(1, 0.25, 0.25);
   fadeTime = 1;

   textureName[ST_FLARE] = "special/flare";
   textureName[ST_BEAM] = "special/nonlingradient";
   textureName[ST_RIP1] = "special/laserrip01";
   textureName[ST_RIP2] = "special/laserrip02";
   textureName[ST_RIP3] = "special/laserrip03";
   textureName[ST_RIP4] = "special/laserrip04";
   textureName[ST_RIP5] = "special/laserrip05";
   textureName[ST_RIP6] = "special/laserrip06";
   textureName[ST_RIP7] = "special/laserrip07";
   textureName[ST_RIP8] = "special/laserrip08";
   textureName[ST_RIP9] = "special/laserrip09";

   textureName[ST_BEAM2] = "special/sniper00";

   startBeamWidth = 0.25;
   endBeamWidth = 0.5;
   pulseBeamWidth = 0.5;
   beamFlareAngle = 3.0;
   minFlareSize = 0.0;
   maxFlareSize = 400.0;
   pulseSpeed = 6.0;
   pulseLength = 0.150;
   lightRadius = 1.0;
   lightColor.set( 0.4, 0.0, 0.0 );
}

//--------------------------------------------------------------------------
SniperProjectileData::~SniperProjectileData()
{

}


//--------------------------------------------------------------------------
void SniperProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("maxRifleRange",       TypeF32,    Offset(maxRifleRange,       SniperProjectileData));
   addField("rifleHeadMultiplier", TypeF32,    Offset(rifleHeadMultiplier, SniperProjectileData));
   addField("beamColor",           TypeColorF, Offset(beamColor,           SniperProjectileData));
   addField("fadeTime",            TypeF32,    Offset(fadeTime,            SniperProjectileData));
   addField("textureName",         TypeString, Offset(textureName,         SniperProjectileData), ST_NUM_TEX);
   addField("startBeamWidth",      TypeF32,    Offset(startBeamWidth,      SniperProjectileData));
   addField("endBeamWidth",        TypeF32,    Offset(endBeamWidth,        SniperProjectileData));
   addField("pulseBeamWidth",      TypeF32,    Offset(pulseBeamWidth,      SniperProjectileData));
   addField("beamFlareAngle",      TypeF32,    Offset(beamFlareAngle,      SniperProjectileData));
   addField("minFlareSize",        TypeF32,    Offset(minFlareSize,        SniperProjectileData));
   addField("maxFlareSize",        TypeF32,    Offset(maxFlareSize,        SniperProjectileData));
   addField("pulseSpeed",          TypeF32,    Offset(pulseSpeed,          SniperProjectileData));
   addField("pulseLength",         TypeF32,    Offset(pulseLength,         SniperProjectileData));
   addField("lightColor",          TypeColorF, Offset(lightColor,          SniperProjectileData));
   addField("lightRadius",         TypeF32,    Offset(lightRadius,         SniperProjectileData));

}


//--------------------------------------------------------------------------
bool SniperProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (maxRifleRange < 10.0 || maxRifleRange > 2000.0f) {
      Con::warnf(ConsoleLogEntry::General, "SniperProjectileData(%s)::onAdd: sniper rifle maxRange must be have range [10, 2000]", getName());
      maxRifleRange = maxRifleRange < 10.0 ? 10.0 : 2000;
   }
   if (rifleHeadMultiplier < 1.0) {
      Con::warnf(ConsoleLogEntry::General, "SniperProjectileData(%s)::onAdd: sniper rifle head multiplier must be >= 1", getName());
      rifleHeadMultiplier = 1.0;
   }
   if (fadeTime < 0.25) {
      Con::warnf(ConsoleLogEntry::General, "SniperProjectileData(%s)::onAdd: sniper rifle fade time must be >= 0.25", getName());
      fadeTime = 0.25;
   }
   beamColor.clamp();

   return true;
}

//--------------------------------------------------------------------------
bool SniperProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (server == false)
   {
      for( int i=0; i<ST_NUM_TEX; i++ )
      {
         if (textureName[i] && textureName[i][0])
         {
            textureHandle[i] = TextureHandle(textureName[i], MeshTexture );
         }
      }
   }
   else
   {
      for( int i=0; i<ST_NUM_TEX; i++ )
      {
         textureHandle[i] = TextureHandle();  // set default NULL tex
      }
   }
   return true;
}


//--------------------------------------------------------------------------
bool SniperProjectileData::calculateAim(const Point3F& targetPos,
                                        const Point3F& /*targetVel*/,
                                        const Point3F& sourcePos,
                                        const Point3F& /*sourceVel*/,
                                        Point3F*       outputVectorMin,
                                        F32*           outputMinTime,
                                        Point3F*       outputVectorMax,
                                        F32*           outputMaxTime)
{
   if ((targetPos - sourcePos).magnitudeSafe() > maxRifleRange) {
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
void SniperProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(maxRifleRange);
   stream->write(rifleHeadMultiplier);
   stream->write(beamColor);
   stream->write(fadeTime);
   stream->write(startBeamWidth);
   stream->write(endBeamWidth);
   stream->write(pulseBeamWidth);
   stream->write(beamFlareAngle);
   stream->write(minFlareSize);
   stream->write(maxFlareSize);
   stream->write(pulseSpeed);
   stream->write(pulseLength);
   stream->write(lightColor);
   stream->write(lightRadius);

   for( int i=0; i<ST_NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }
}

//--------------------------------------------------------------------------
void SniperProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&maxRifleRange);
   stream->read(&rifleHeadMultiplier);
   stream->read(&beamColor);
   stream->read(&fadeTime);
   stream->read(&startBeamWidth);
   stream->read(&endBeamWidth);
   stream->read(&pulseBeamWidth);
   stream->read(&beamFlareAngle);
   stream->read(&minFlareSize);
   stream->read(&maxFlareSize);
   stream->read(&pulseSpeed);
   stream->read(&pulseLength);
   stream->read(&lightColor);
   stream->read(&lightRadius);

   for( int i=0; i<ST_NUM_TEX; i++ )
   {
     textureName[i] = stream->readSTString();
   }

}


//**************************************************************************
// Sniper Projectile
//**************************************************************************
SniperProjectile::SniperProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable|ScopeAlways);

   mClientTotalWarpTicks = 0;
   mClientWarpTicks      = 0;
   mHitWater = false;
   mEnergyPercentage = 0;
}

//--------------------------------------------------------------------------
SniperProjectile::~SniperProjectile()
{
   //
}

//--------------------------------------------------------------------------

void SniperProjectile::setEnergyPercentage(F32 val)
{
   mEnergyPercentage = val;
}

ConsoleMethod(SniperProjectile,setEnergyPercentage,void,3,3,"proj.setEnergyPercentage(pct)")
{
   argc;
   ((SniperProjectile *)object)->setEnergyPercentage(dAtof(argv[2]));
}

//--------------------------------------------------------------------------
bool SniperProjectile::calculateImpact(float    /*simTime*/,
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
bool SniperProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (isServerObject()) {
      MatrixF xform(true);
      xform.setColumn(3, mInitialPosition);

      mBeam.mData.endPos = mInitialPosition + mInitialDirection * mDataBlock->maxRifleRange;
      RayInfo rayInfo;

      if( mSourceObject )
      {
         mSourceObject->disableCollision();
      }
         
      bool rayHit = gServerContainer.castRay(mInitialPosition, mBeam.mData.endPos,
                                             (csmStaticCollisionMask | csmDynamicCollisionMask | WaterObjectType),
                                             &rayInfo);
      if( mSourceObject )
      {
         mSourceObject->enableCollision();
      }

      if( rayHit )
      {
         // Damage the object
         if (rayInfo.object->getType() & csmDamageableMask)
            onCollision(rayInfo.point, rayInfo.normal, rayInfo.object);

         // shoot ray again to see if it hit water
         RayInfo waterRayInfo;

         if( mSourceObject )
         {
            mSourceObject->disableCollision();
         }
         mHitWater = gClientContainer.castRay(mInitialPosition, mBeam.mData.endPos, WaterObjectType, &waterRayInfo);

         if( mSourceObject )
         {
            mSourceObject->enableCollision();
         }


         mBeam.mData.endPos = rayInfo.point;
         mTruncated   = true;
      } else {
         mTruncated = false;
      }

      if( mSourceObject )
      {
         mSourceObject->enableCollision();
      }

      mDeleteWaitTicks = (S32)((mDataBlock->fadeTime * (1000.0 / TickMs)) * 2);
   } 
   else {
      mClientTime = mDataBlock->fadeTime * (1.0 - mEnergyPercentage);

      if (mTruncated) {

         if (mDataBlock->explosion && !mHitWater) {
            Point3F n = mBeam.mData.endPos - mInitialPosition;
            n.normalizeSafe();

            Explosion* pExplosion = new Explosion;
            pExplosion->onNewDataBlock(mDataBlock->explosion);

            MatrixF xform(true);
            xform.setColumn(3, mBeam.mData.endPos);
            pExplosion->setTransform(xform);
            pExplosion->setInitialState(mBeam.mData.endPos, n, mFadeValue);
            if (pExplosion->registerObject() == false) {
               Con::errorf(ConsoleLogEntry::General, "SniperProjectile(%s)::explode: couldn't register explosion(%s)",
                           mDataBlock->getName(), mDataBlock->explosion->getName());
               delete pExplosion;
               pExplosion = NULL;
            }
         }

         if( mHitWater && mDataBlock->splash )
         {
            MatrixF trans = getTransform();
            trans.setPosition( mBeam.mData.endPos );
            Splash *splash = new Splash;
            splash->onNewDataBlock( mDataBlock->splash );
            splash->setTransform( trans );
            splash->setInitialState( trans.getPosition(), Point3F( 0.0, 0.0, 1.0 ) );
            if (!splash->registerObject())
               delete splash;
         }

         Sim::getLightSet()->addObject(this);
      }
   }

   mObjBox.min.set(-1e7, -1e7, -1e7);
   mObjBox.max.set( 1e7,  1e7,  1e7);
   resetWorldBox();
   addToScene();


   return true;
}

F32 SniperProjectile::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
   if(updateMask & InitialUpdateMask)
      return 9.9;
   return Parent::getUpdatePriority(camInfo, updateMask, updateSkips);
}

//--------------------------------------------------------------------------
void SniperProjectile::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}


//--------------------------------------------------------------------------
bool SniperProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<SniperProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
bool SniperProjectile::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   if (mClientTime > mDataBlock->fadeTime)
      return false;

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   SceneRenderImage* image = new SceneRenderImage;
   image->obj = this;
   image->isTranslucent = true;
   image->sortType = SceneRenderImage::EndSort;
   state->insertRenderImage(image);

   return false;
}


//--------------------------------------------------------------------------
void SniperProjectile::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   state->setupBaseProjection();

   F32 percentDone = mClientTime / mDataBlock->fadeTime;
   F32 invPercentDone = 1.0 - percentDone;


   updateBeamData( mBeam.mData, state->getCameraPosition() );

   // tweak end position so it doesn't intersect camera
   mBeam.adjustEdge( mBeam.mData );

   // render the flare
   F32 flareScale = 0.0;
   bool result = checkForFlare( flareScale, state->getCameraPosition() );

   if( result )
   {
      ColorF flareColor = mDataBlock->beamColor;
      flareColor.alpha = invPercentDone;
      renderFlare( flareColor, flareScale );
   }

   renderBeam( mBeam, percentDone );


   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------------------------------------------
// Returns true if need to render flare.
//--------------------------------------------------------------------------
bool SniperProjectile::checkForFlare( F32 &flareScale, const Point3F &camPos )
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
void SniperProjectile::updateBeamData( BeamData &beamData, const Point3F &camPos )
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
void SniperProjectile::renderBeam( WeaponBeam &beam, F32 percentDone )
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


      // render main beam
      F32 width = mDataBlock->startBeamWidth;
      width += (mDataBlock->endBeamWidth - width) * percentDone;
      

      glColor4f( 1.0, 1.0, 1.0, 1.0 - percentDone );
      glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[SniperProjectileData::ST_BEAM2].getGLName());

      beam.render( width );



      // render pulse

      ColorF beamColor = mDataBlock->beamColor;
      beamColor.alpha  = 1.0 - percentDone;
      glColor4fv( beamColor );

      S32 texNum = (S32)(F32)( ((F32)(SniperProjectileData::ST_BEAM)) + 10.0 * percentDone);
      F32 pulseOffset = mClientTime * -mDataBlock->pulseSpeed * 0.5;
      F32 numWrap = beam.mData.length * mDataBlock->pulseLength;
      glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[texNum].getGLName());
      beam.render( width * 1.25, pulseOffset, numWrap );



      // restore state
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glDepthMask(GL_TRUE);
      glEnable(GL_DEPTH_TEST);

   }

}

//--------------------------------------------------------------------------
void SniperProjectile::renderFlare( ColorF flareColor, F32 flareScale )
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
   glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[SniperProjectileData::ST_FLARE].getGLName());

   F32 ang = mClientTime;

   dglDraw2DSquare( Point2F( screenPoint.x, screenPoint.y ), flareSize, ang );
   dglDraw2DSquare( Point2F( screenPoint.x, screenPoint.y ), flareSize * .75, -ang );


   glEnable(GL_DEPTH_TEST);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();

}

//--------------------------------------------------------------------------
void SniperProjectile::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isClientObject() && mClientWarpTicks != 0)
   {
      mClientWarpTicks--;
   }
   else
   {
      if (isServerObject())
      {
         if (--mDeleteWaitTicks <= 0)
         {
            deleteObject();
         }
      }
   }
}


//--------------------------------------------------------------------------
void SniperProjectile::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);

   if (mClientWarpTicks != 0 && mClientOwned == false)
   {
      // Warp the end point
      F32 factor = 1.0 - (F32(mClientWarpTicks) / F32(mClientTotalWarpTicks));

      #if COUPLE_BEAM
         mBeam.mData.endPos.interpolate(mClientWarpFrom, mClientWarpTo, factor);
      #endif

   } 
   else
   {
      if (mClientOwned && bool(mSourceObject))
      {
         #if COUPLE_BEAM
            // We're the shooter recast the end point...
            mSourceObject->getRenderMuzzlePoint(mSourceObjectSlot, &mInitialPosition);
            mSourceObject->getRenderMuzzleVector(mSourceObjectSlot, &mInitialDirection);
            mBeam.mData.endPos = mInitialPosition + mInitialDirection * mDataBlock->maxRifleRange;

         #endif

         mSourceObject->disableCollision();

         RayInfo rayInfo;
         bool rayHit = gClientContainer.castRay(mInitialPosition, mBeam.mData.endPos,
                                      (csmStaticCollisionMask | csmDynamicCollisionMask | WaterObjectType),
                                      &rayInfo);

         mSourceObject->enableCollision();

         if( rayHit )
         {
            #if COUPLE_BEAM
               mBeam.mData.endPos = rayInfo.point;
            #endif

            mTruncated   = true;
         }
         else
         {
            mTruncated = false;
         }
      }
      else
      {

         #if COUPLE_BEAM
            // Just set the last position...
            mBeam.mData.endPos = mClientWarpTo;
         #endif
      }
   }

   mBeam.mData.direction = mBeam.mData.endPos - mInitialPosition;
   mBeam.mData.direction.normalizeSafe();
}


//--------------------------------------------------------------------------
void SniperProjectile::advanceTime(F32 dt)
{
   mClientTime += dt;
}


//--------------------------------------------------------------------------
void SniperProjectile::setClientWarp(const Point3F& newEndPoint)
{
   mClientWarpFrom       = mBeam.mData.endPos;
   mClientWarpTo         = newEndPoint;
   mClientTotalWarpTicks = 10;
   mClientWarpTicks      = 10;
}


//--------------------------------------------------------------------------
U32 SniperProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{

   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag((mask & GameBase::InitialUpdateMask) != 0)) {

      stream->writeFloat( mEnergyPercentage, 7 );

      mathWrite(*stream, mInitialPosition);
      mathWrite(*stream, mBeam.mData.endPos);
      stream->writeFlag(mTruncated);
      stream->writeFlag(mHitWater);

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
void SniperProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);


   if (stream->readFlag()) {
      
      mEnergyPercentage = stream->readFloat( 7 );

      mathRead(*stream, &mInitialPosition);
      mathRead(*stream, &mBeam.mData.endPos);

      mClientWarpFrom       = mBeam.mData.endPos;
      mClientWarpTo         = mBeam.mData.endPos;
      mClientTotalWarpTicks = 0;
      mClientWarpTicks      = 0;

      mTruncated = stream->readFlag();
      mHitWater = stream->readFlag();

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
   } else {
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

//--------------------------------------------------------------------------
// For light at end of sniper beam
//--------------------------------------------------------------------------
void SniperProjectile::registerLights(LightManager * lightManager, bool lightingScene)
{
   if(lightingScene)
      return;
      
   if (mHidden == false) {

      F32 percentDone = mClientTime / mDataBlock->fadeTime;

      mLight.mColor = mDataBlock->lightColor;
      mLight.mColor *= 1.0 - percentDone;
      mLight.mType = LightInfo::Point;
      mLight.mPos = mBeam.mData.endPos;
      mLight.mRadius = mDataBlock->lightRadius;
      lightManager->addLight(&mLight);
   }
}
