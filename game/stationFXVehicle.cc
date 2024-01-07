//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/StationFXVehicle.h"
#include "core/bitStream.h"
#include "console/consoleTypes.h"
#include "scenegraph/sceneState.h"
#include "dgl/dgl.h"
#include "sim/netConnection.h"
#include "game/shapeBase.h"
#include "scenegraph/sceneGraph.h"
#include "ts/tsShapeInstance.h"
#include "math/mathUtils.h"
#include "math/mathIO.h"

IMPLEMENT_CO_DATABLOCK_V1(StationFXVehicleData);
IMPLEMENT_CO_NETOBJECT_V1(StationFXVehicle);


//**************************************************************************
// Station FX Personal Data
//**************************************************************************
StationFXVehicleData::StationFXVehicleData()
{
   glowTopHeight = 0.5;
   glowBottomHeight = 0.1;
   glowTopRadius = 8.0;
   glowBottomRadius = 7.5;
   numGlowSegments = 20;
   glowFadeTime = 1.0;

   armLightDelay = 2.0;
   armLightLifetime = 5.0;
   armLightFadeTime = 2.0;

   sphereColor.set( 0.1, 0.1, 0.5, 1.0 );
   spherePhiSegments = 13;
   sphereThetaSegments = 5;
   sphereRadius = 12.0;
   sphereScale.set( 1.0, 1.0, 0.85 );

   lifetime = 6.0;
   numArcSegments = 10.0;

   glowNodeName = NULL;
   dMemset( leftNodeName, 0, sizeof( leftNodeName ) );
   dMemset( rightNodeName, 0, sizeof( rightNodeName ) );

   dMemset( textureName, 0, sizeof( textureName ) );
   dMemset( textureHandle, 0, sizeof( textureHandle ) );
}

//--------------------------------------------------------------------------
void StationFXVehicleData::initPersistFields()
{
   Parent::initPersistFields();
                                                   
   addField("glowTopHeight",     TypeF32,    Offset( glowTopHeight,     StationFXVehicleData ) );
   addField("glowBottomHeight",  TypeF32,    Offset( glowBottomHeight,  StationFXVehicleData ) );
   addField("glowTopRadius",     TypeF32,    Offset( glowTopRadius,     StationFXVehicleData ) );
   addField("glowBottomRadius",  TypeF32,    Offset( glowBottomRadius,  StationFXVehicleData ) );
   addField("numGlowSegments",   TypeS32,    Offset( numGlowSegments,   StationFXVehicleData ) );
   addField("glowFadeTime",      TypeF32,    Offset( glowFadeTime,      StationFXVehicleData ) );

   addField("armLightDelay",     TypeF32,    Offset( armLightDelay,     StationFXVehicleData ) );
   addField("armLightLifetime",  TypeF32,    Offset( armLightLifetime,  StationFXVehicleData ) );
   addField("armLightFadeTime",  TypeF32,    Offset( armLightFadeTime,  StationFXVehicleData ) );

   addField("sphereColor",       TypeColorF, Offset( sphereColor,       StationFXVehicleData ) );
   addField("spherePhiSegments", TypeS32,    Offset( spherePhiSegments, StationFXVehicleData ) );
   addField("sphereThetaSegments", TypeS32,  Offset( sphereThetaSegments, StationFXVehicleData ) );
   addField("sphereRadius",      TypeF32,    Offset( sphereRadius,      StationFXVehicleData ) );
   addField("sphereScale",       TypePoint3F,Offset( sphereScale,       StationFXVehicleData ) );

   addField("lifetime",          TypeF32,    Offset( lifetime,          StationFXVehicleData ) );
   addField("numArcSegments",    TypeS32,    Offset( numArcSegments,    StationFXVehicleData ) );

   addField("glowNodeName",   TypeString, Offset( glowNodeName,   StationFXVehicleData ) );
   addField("leftNodeName",   TypeString, Offset( leftNodeName,   StationFXVehicleData ), SFXC_NUM_NODES );
   addField("rightNodeName",  TypeString, Offset( rightNodeName,  StationFXVehicleData ), SFXC_NUM_NODES );
   addField("texture",        TypeString, Offset( textureName,    StationFXVehicleData ), SFXC_NUM_TEX );

}

//--------------------------------------------------------------------------
bool StationFXVehicleData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}

//--------------------------------------------------------------------------
bool StationFXVehicleData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (server == false)
   {
      for( int i=0; i<SFXC_NUM_TEX; i++ )
      {
         if( textureName[i] && textureName[i][0] )
         {
            textureHandle[i] = TextureHandle(textureName[i], MeshTexture );
         }
      }
   }
   
   return true;
}

//--------------------------------------------------------------------------
void StationFXVehicleData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(glowTopHeight);
   stream->write(glowBottomHeight);
   stream->write(glowTopRadius);
   stream->write(glowBottomRadius);
   stream->write(numGlowSegments);
   stream->write(glowFadeTime);

   stream->write(armLightDelay);
   stream->write(armLightLifetime);
   stream->write(armLightFadeTime);

   stream->write(lifetime);
   stream->write(numArcSegments);

   stream->write(sphereColor);
   stream->write(spherePhiSegments);
   stream->write(sphereThetaSegments);
   stream->write(sphereRadius);
   mathWrite(*stream, sphereScale );

   stream->writeString(glowNodeName);

   U32 i=0;
   for( i=0; i<SFXC_NUM_NODES; i++ )
   {
      stream->writeString(leftNodeName[i]);
      stream->writeString(rightNodeName[i]);
   }

   for( i=0; i<SFXC_NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }
}

//--------------------------------------------------------------------------
void StationFXVehicleData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&glowTopHeight);
   stream->read(&glowBottomHeight);
   stream->read(&glowTopRadius);
   stream->read(&glowBottomRadius);
   stream->read(&numGlowSegments);
   stream->read(&glowFadeTime);

   stream->read(&armLightDelay);
   stream->read(&armLightLifetime);
   stream->read(&armLightFadeTime);

   stream->read(&lifetime);
   stream->read(&numArcSegments);

   stream->read(&sphereColor);
   stream->read(&spherePhiSegments);
   stream->read(&sphereThetaSegments);
   stream->read(&sphereRadius);
   mathRead( *stream, &sphereScale );
   
   glowNodeName = stream->readSTString();

   U32 i=0;
   for( i=0; i<SFXC_NUM_NODES; i++ )
   {
      leftNodeName[i] = stream->readSTString();
      rightNodeName[i] = stream->readSTString();
   }

   for( i=0; i<SFXC_NUM_TEX; i++ )
   {
     textureName[i] = stream->readSTString();
   }
}


//**************************************************************************
// Station FX Personal
//**************************************************************************
StationFXVehicle::StationFXVehicle()
{
   mCurrMS = 0;
   mStationObjectID = -1;
   mDataBlock = NULL;
   mElapsedTime = 0.0;
}

//--------------------------------------------------------------------------
void StationFXVehicle::initPersistFields()
{
   Parent::initPersistFields();

   addField("stationObject",  TypeS32, Offset( mStationObjectID,  StationFXVehicle ));
}

//--------------------------------------------------------------------------
bool StationFXVehicle::onAdd()
{
   
   if(!Parent::onAdd())
      return false;

   if( isServerObject() )
   {
      ShapeBase* obj;
      if( mStationObjectID != -1 )
      {
         Sim::findObject( mStationObjectID, obj );
         mStationObject = obj;
      }
   }

   mObjBox.min = Point3F( -12, -12, -0 );
   mObjBox.max = Point3F(  12,  12,  12 );
   resetWorldBox();

   if( mStationObject )
   {
      setTransform( mStationObject->getTransform() );
   }
     
   addToScene();

   return true;
}

//--------------------------------------------------------------------------
void StationFXVehicle::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

//--------------------------------------------------------------------------
bool StationFXVehicle::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<StationFXVehicleData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}

//--------------------------------------------------------------------------
// Process tick
//--------------------------------------------------------------------------
void StationFXVehicle::processTick(const Move*)
{
   mCurrMS += TickMs;

   if( isServerObject() )
   {
      if( mCurrMS >= (mDataBlock->lifetime * 1000) )
      {
         deleteObject();
         return;
      }
   }
}

//--------------------------------------------------------------------------
// Advance time
//--------------------------------------------------------------------------
void StationFXVehicle::advanceTime(F32 dt)
{
   mElapsedTime += dt;
}

//--------------------------------------------------------------------------
// Prep render image
//--------------------------------------------------------------------------
bool StationFXVehicle::prepRenderImage(SceneState* state, const U32 stateKey,
                                const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;

   setLastState(state, stateKey);

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this))
   {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->isTranslucent = true;
//      image->sortType = SceneRenderImage::Point;
      image->sortType = SceneRenderImage::EndSort;
      state->setImageRefPoint(this, image);
      state->insertRenderImage(image);
      
   }

   return false;
}
   
//--------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------
void StationFXVehicle::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   state->setupObjectProjection(this);

   F32 radius[4];
   
   radius[0] = findRadius( mDataBlock->leftNodeName[0], mDataBlock->rightNodeName[0] );
   radius[1] = findRadius( mDataBlock->leftNodeName[1], mDataBlock->rightNodeName[1] );
   radius[2] = findRadius( mDataBlock->leftNodeName[2], mDataBlock->rightNodeName[2] );
   radius[3] = findRadius( mDataBlock->leftNodeName[3], mDataBlock->rightNodeName[3] );

   renderWall( radius[1], radius[0], 180.0, mDataBlock->leftNodeName[1], mDataBlock->leftNodeName[0] );
   renderWall( radius[1], radius[0], 180.0, mDataBlock->rightNodeName[1], mDataBlock->rightNodeName[0] );

   renderWall( radius[3], radius[2], 180.0, mDataBlock->leftNodeName[3], mDataBlock->leftNodeName[2] );
   renderWall( radius[3], radius[2], 180.0, mDataBlock->rightNodeName[3], mDataBlock->rightNodeName[2] );

   renderGlow();
   renderHemisphere( mDataBlock->spherePhiSegments, mDataBlock->sphereThetaSegments, mDataBlock->sphereRadius );

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");

}

//--------------------------------------------------------------------------
//  Get node transform
//--------------------------------------------------------------------------
MatrixF StationFXVehicle::getNodeTransform( StringTableEntry nodeName )
{
   TSShapeInstance *shapeInstance = mStationObject->getShapeInstance();
   TSShape *shape = shapeInstance->getShape();

   S32 node = shape->findNode( nodeName );
   if( node < 0 )
   {
      return MatrixF(true);
   }
   
   return shapeInstance->mNodeTransforms[node];
}

//--------------------------------------------------------------------------
// Find radius
//--------------------------------------------------------------------------
F32 StationFXVehicle::findRadius( StringTableEntry node1, StringTableEntry node2 )
{
   if( !mStationObject ) return 1.0;

   TSShapeInstance *shapeInstance = mStationObject->getShapeInstance();
   if( !shapeInstance ) return 1.0;
   TSShape *shape = shapeInstance->getShape();
   if( !shape ) return 1.0;

   S32 node = shape->findNode( node1 );
   if( node < 0 ) return 1.0;
   Point3F nodePos1 = shapeInstance->mNodeTransforms[node].getPosition();

   node = shape->findNode( node2 );
   if( node < 0 ) return 1.0;
   Point3F nodePos2 = shapeInstance->mNodeTransforms[node].getPosition();
   
   VectorF diff = nodePos1 - nodePos2;
   return ( diff.magnitudeSafe() * 0.5 );
}

//--------------------------------------------------------------------------
// Render hemisphere
//--------------------------------------------------------------------------
void StationFXVehicle::renderHemisphere( F32 numPhiSegments, F32 numThetaSegments, F32 radius )
{
   if( !mStationObject ) return;
   
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   MatrixF sphereTrans = getNodeTransform( mDataBlock->glowNodeName );

   MatrixF stationTrans;
   stationTrans.mul( mStationObject->getTransform(), sphereTrans );
   dglMultMatrix( &stationTrans );

   glScalef( mDataBlock->sphereScale.x, mDataBlock->sphereScale.y, mDataBlock->sphereScale.z );

   glColor4f( 0.1, 0.1, 0.5, 1.0 );


   F32 armTime = mElapsedTime - mDataBlock->armLightDelay;

   if( armTime < mDataBlock->armLightFadeTime )
   {
      F32 percent = armTime / mDataBlock->armLightFadeTime;
      glColor4f( 0.1, 0.1, 0.5, percent );
   }

   if( (mDataBlock->armLightLifetime - armTime) < mDataBlock->armLightFadeTime )
   {
      F32 timeLeft = mDataBlock->armLightLifetime - armTime;
      F32 percent = timeLeft / mDataBlock->armLightFadeTime;
      glColor4f( 0.1, 0.1, 0.5, percent );
   }


   glEnable( GL_CULL_FACE );
   glDepthMask( GL_FALSE );
   glDisable( GL_TEXTURE_2D );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE );

   for( U32 i=1; i<numThetaSegments; i++ )
   {
      F32 topAngle      = F32(i) / (numThetaSegments-1);
      F32 bottomAngle   = F32(i-1) / (numThetaSegments-1);

      F32 topHeight     = mSin( topAngle * M_PI * 0.5 ) * radius;
      F32 bottomHeight  = mSin( bottomAngle * M_PI * 0.5 ) * radius;
      F32 topRadius     = mCos( topAngle * M_PI * 0.5 ) * radius;
      F32 bottomRadius  = mCos( bottomAngle * M_PI * 0.5 ) * radius;

      glBegin( GL_TRIANGLE_STRIP );

      for( U32 j=0; j<numPhiSegments; j++ )
      {
         F32 percent = F32(j) / (numPhiSegments-1);
         F32 angle = percent * M_2PI;
      
         VectorF topPoint( mSin( angle ) * topRadius, mCos( angle ) * topRadius, topHeight );
         glTexCoord2f( percent, 0.0 );
         glVertex3fv( topPoint );

         VectorF bottomPoint( mSin( angle ) * bottomRadius, mCos( angle ) * bottomRadius, bottomHeight );
         glTexCoord2f( percent, 1.0 );
         glVertex3fv( bottomPoint );
      }

      glEnd();
   }
      
   glDisable( GL_CULL_FACE );

   glDisable( GL_BLEND );
   glDepthMask( GL_TRUE );

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

//--------------------------------------------------------------------------
// Render wall
//--------------------------------------------------------------------------
void StationFXVehicle::renderWall( F32 topRadius, F32 bottomRadius, F32 numDegrees, 
                                   StringTableEntry topNode, StringTableEntry bottomNode )
{
   if( !mStationObject ) return;
   if( mElapsedTime < mDataBlock->armLightDelay ) return;
   
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   TSShapeInstance *shapeInstance = mStationObject->getShapeInstance();
   shapeInstance->animate();
   TSShape *shape = shapeInstance->getShape();

   Point3F topNodePoint = getNodeTransform( topNode ).getPosition();
   Point3F bottomNodePoint = getNodeTransform( bottomNode ).getPosition();

   MatrixF wallTrans = getNodeTransform( mDataBlock->glowNodeName );
   Point3F centerPos = wallTrans.getPosition();

   Point3F dir = topNodePoint - centerPos;
   dir.z = 0.0;
   dir.normalize();
   MatrixF wallOrient = MathUtils::createOrientFromDir( dir );

   centerPos.z = bottomNodePoint.z;
   wallTrans.setPosition( centerPos );
   wallTrans.mul( wallOrient );
   
   MatrixF stationTrans;
   stationTrans.mul( mStationObject->getTransform(), wallTrans );
   dglMultMatrix( &stationTrans );
   
   
   glColor4f( 1.0, 1.0, 1.0, 1.0 );

   F32 armTime = mElapsedTime - mDataBlock->armLightDelay;

   if( armTime < mDataBlock->armLightFadeTime )
   {
      F32 percent = armTime / mDataBlock->armLightFadeTime;
      glColor4f( 1.0, 1.0, 1.0, percent );
      numDegrees *= percent;
   }

   if( (mDataBlock->armLightLifetime - armTime) < mDataBlock->armLightFadeTime )
   {
      F32 timeLeft = mDataBlock->armLightLifetime - armTime;
      F32 percent = timeLeft / mDataBlock->armLightFadeTime;
      glColor4f( 1.0, 1.0, 1.0, percent );
      numDegrees *= percent;
   }


   glDepthMask( GL_FALSE );
   glEnable( GL_TEXTURE_2D );
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[1].getGLName() );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE );
   

   F32 height = topNodePoint.z - bottomNodePoint.z;

   glBegin( GL_TRIANGLE_STRIP );

      for( U32 i=0; i<mDataBlock->numArcSegments; i++ )
      {
         F32 percent = F32(i) / F32(mDataBlock->numArcSegments-1);
         F32 angle = percent * mDegToRad( numDegrees );
         
         if( percent == 1.0 )
         {
            percent = 0.97;
         }
         
         VectorF topPoint( mSin( angle ) * topRadius, mCos( angle ) * topRadius, height );
         glTexCoord2f( percent, 0.025 );
         glVertex3fv( topPoint );

         VectorF bottomPoint( mSin( angle ) * bottomRadius, mCos( angle ) * bottomRadius, 0.0 );
         glTexCoord2f( percent, 1.0 );
         glVertex3fv( bottomPoint );

      }

   glEnd();
   
   glDisable( GL_BLEND );
   glDisable( GL_TEXTURE_2D );
   glDepthMask( GL_TRUE );

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

}

//--------------------------------------------------------------------------
// Render glow on bottom of station
//--------------------------------------------------------------------------
void StationFXVehicle::renderGlow()
{
   if( !mStationObject ) return;
   
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   MatrixF stationTrans;
   TSShapeInstance *shapeInstance = mStationObject->getShapeInstance();

   TSShape *shape = shapeInstance->getShape();

   S32 node = shape->findNode( mDataBlock->glowNodeName );

   MatrixF nodeTrans = shapeInstance->mNodeTransforms[node];

   stationTrans.mul( mStationObject->getTransform(), nodeTrans );
   dglMultMatrix( &stationTrans );

   glColor4f( 1.0, 1.0, 1.0, 1.0 );

   if( mElapsedTime < mDataBlock->glowFadeTime )
   {
      glColor4f( 1.0, 1.0, 1.0, mElapsedTime / mDataBlock->glowFadeTime );
   }
   if( (mDataBlock->lifetime - mElapsedTime) < mDataBlock->glowFadeTime )
   {
      F32 timeLeft = mDataBlock->lifetime - mElapsedTime;
      glColor4f( 1.0, 1.0, 1.0, timeLeft / mDataBlock->glowFadeTime );
   }

   glDepthMask( GL_FALSE );
   glEnable( GL_TEXTURE_2D );
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[0].getGLName() );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE );
   
   glBegin( GL_TRIANGLE_STRIP );

      for( U32 i=0; i<mDataBlock->numGlowSegments; i++ )
      {
         F32 percent = F32(i) / F32(mDataBlock->numGlowSegments-1);
         F32 angle = percent * mDegToRad( 360.0 );
         
         F32 radius = mDataBlock->glowTopRadius;
         VectorF topPoint( mSin( angle ) * radius, mCos( angle ) * radius, mDataBlock->glowTopHeight );
         glTexCoord2f( percent, 0.05 );
         glVertex3fv( topPoint );

         radius = mDataBlock->glowBottomRadius;
         VectorF bottomPoint( mSin( angle ) * radius, mCos( angle ) * radius, mDataBlock->glowBottomHeight );
         glTexCoord2f( percent, 1.0 );
         glVertex3fv( bottomPoint );

      }

   glEnd();
   
   glDisable( GL_BLEND );
   glDisable( GL_TEXTURE_2D );
   glDepthMask( GL_TRUE );

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

//--------------------------------------------------------------------------
// Pack update
//--------------------------------------------------------------------------
U32 StationFXVehicle::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & GameBase::InitialUpdateMask))
   {
      if( bool(mStationObject) )
      {
         S32 ghostIndex = con->getGhostIndex(mStationObject);
         if (stream->writeFlag(ghostIndex != -1)) {
            stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
         }
      }
      else
      {
         stream->writeFlag(false);
      }

   }
      
   return retMask;
}

//--------------------------------------------------------------------------
// Unpack update
//--------------------------------------------------------------------------
void StationFXVehicle::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   // initial update
   if( stream->readFlag() )
   {
      if (stream->readFlag())
      {
         mStationObjectID   = stream->readRangedU32(0, NetConnection::MaxGhostCount);

         NetObject* pObject = con->resolveGhost(mStationObjectID);
         if (pObject != NULL)
         {
            mStationObject = dynamic_cast<ShapeBase*>(pObject);
         }
      }
   }
   
}
