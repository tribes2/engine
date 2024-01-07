//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/StationFXPersonal.h"
#include "Core/bitStream.h"
#include "console/consoleTypes.h"
#include "sceneGraph/sceneState.h"
#include "dgl/dgl.h"
#include "Sim/netConnection.h"
#include "game/shapeBase.h"
#include "sceneGraph/sceneGraph.h"
#include "ts/tsShapeInstance.h"

IMPLEMENT_CO_DATABLOCK_V1(StationFXPersonalData);
IMPLEMENT_CO_NETOBJECT_V1(StationFXPersonal);


//**************************************************************************
// Station FX Personal Data
//**************************************************************************
StationFXPersonalData::StationFXPersonalData()
{
   delay = 0.0;
   fadeDelay = 1.5;
   lifetime = 2.0;
   height = 2.5;
   numArcSegments = 10.0;
   numDegrees = 180.0;
   trailFadeTime = 0.2;
   leftRadius = 2.0;
   rightRadius = 2.0;
   leftNodeName = rightNodeName = NULL;

   dMemset( textureName, 0, sizeof( textureName ) );
   dMemset( textureHandle, 0, sizeof( textureHandle ) );
}

//--------------------------------------------------------------------------
void StationFXPersonalData::initPersistFields()
{
   Parent::initPersistFields();
                                                   
   addField("delay",          TypeF32,    Offset( delay,       StationFXPersonalData ) );
   addField("fadeDelay",      TypeF32,    Offset( fadeDelay,   StationFXPersonalData ) );
   addField("lifetime",       TypeF32,    Offset( lifetime,    StationFXPersonalData ) );
   addField("height",         TypeF32,    Offset( height,      StationFXPersonalData ) );
   addField("numArcSegments", TypeS32,    Offset( numArcSegments, StationFXPersonalData ) );
   addField("numDegrees",     TypeF32,    Offset( numDegrees,  StationFXPersonalData ) );
   addField("trailFadeTime",  TypeF32,    Offset( trailFadeTime, StationFXPersonalData ) );
   addField("leftRadius",     TypeF32,    Offset( leftRadius,  StationFXPersonalData ) );
   addField("rightRadius",    TypeF32,    Offset( rightRadius, StationFXPersonalData ) );
   addField("leftNodeName",   TypeString, Offset( leftNodeName,StationFXPersonalData ) );
   addField("rightNodeName",  TypeString, Offset( rightNodeName,StationFXPersonalData ) );
   addField("texture",        TypeString, Offset( textureName, StationFXPersonalData ), SFXC_NUM_TEX );

}

//--------------------------------------------------------------------------
bool StationFXPersonalData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}

//--------------------------------------------------------------------------
bool StationFXPersonalData::preload(bool server, char errorBuffer[256])
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
void StationFXPersonalData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(delay);
   stream->write(fadeDelay);
   stream->write(lifetime);
   stream->write(height);
   stream->write(numArcSegments);
   stream->write(numDegrees);
   stream->write(trailFadeTime);
   stream->write(leftRadius);
   stream->write(rightRadius);
   stream->write(numArcSegments);
   stream->writeString(leftNodeName);
   stream->writeString(rightNodeName);

   for( int i=0; i<SFXC_NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }
}

//--------------------------------------------------------------------------
void StationFXPersonalData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&delay);
   stream->read(&fadeDelay);
   stream->read(&lifetime);
   stream->read(&height);
   stream->read(&numArcSegments);
   stream->read(&numDegrees);
   stream->read(&trailFadeTime);
   stream->read(&leftRadius);
   stream->read(&rightRadius);
   stream->read(&numArcSegments);
   leftNodeName = stream->readSTString();
   rightNodeName = stream->readSTString();

   for( int i=0; i<SFXC_NUM_TEX; i++ )
   {
     textureName[i] = stream->readSTString();
   }
}


//**************************************************************************
// Station FX Personal
//**************************************************************************
StationFXPersonal::StationFXPersonal()
{
   mCurrMS = 0;
   mStationObjectID = -1;
   mDataBlock = NULL;
   mElapsedTime = 0.0;
}

//--------------------------------------------------------------------------
void StationFXPersonal::initPersistFields()
{
   Parent::initPersistFields();

   addField("stationObject",  TypeS32, Offset( mStationObjectID,  StationFXPersonal ));
}

//--------------------------------------------------------------------------
bool StationFXPersonal::onAdd()
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

   mObjBox.min = Point3F( -3, -3, -3 );
   mObjBox.max = Point3F(  3,  3,  3 );
   resetWorldBox();

   if( mStationObject )
   {
      setTransform( mStationObject->getTransform() );
   }
     
   addToScene();

   return true;
}

//--------------------------------------------------------------------------
void StationFXPersonal::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

//--------------------------------------------------------------------------
bool StationFXPersonal::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<StationFXPersonalData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}

//--------------------------------------------------------------------------
// Process tick
//--------------------------------------------------------------------------
void StationFXPersonal::processTick(const Move*)
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
void StationFXPersonal::advanceTime(F32 dt)
{
   mElapsedTime += dt;
}

//--------------------------------------------------------------------------
// Prep render image
//--------------------------------------------------------------------------
bool StationFXPersonal::prepRenderImage(SceneState* state, const U32 stateKey,
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
      image->sortType = SceneRenderImage::Point;
      state->setImageRefPoint(this, image);
      state->insertRenderImage(image);
   }

   return false;
}
   
//--------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------
void StationFXPersonal::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   state->setupObjectProjection(this);

   renderWall( mDataBlock->numArcSegments, mDataBlock->leftRadius, mDataBlock->height, 
               mDataBlock->leftNodeName, mDataBlock->numDegrees );

   renderWall( mDataBlock->numArcSegments, mDataBlock->rightRadius, mDataBlock->height, 
               mDataBlock->rightNodeName, mDataBlock->numDegrees );

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");

}

//--------------------------------------------------------------------------
// Render wall
//--------------------------------------------------------------------------
void StationFXPersonal::renderWall( F32 numSegments, F32 radius, F32 height, 
                                    StringTableEntry nodeName, F32 numDegrees )
{
   if( !mStationObject ) return;

   
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   MatrixF stationTrans;
   TSShapeInstance *shapeInstance = mStationObject->getShapeInstance();

   shapeInstance->animate();
   
   if( mElapsedTime < mDataBlock->trailFadeTime )
   {
      F32 percent = mElapsedTime / mDataBlock->trailFadeTime;
      numDegrees *= percent;
   }
   if( (mDataBlock->lifetime - mElapsedTime) < mDataBlock->trailFadeTime )
   {
      F32 timeLeft = mDataBlock->lifetime - mElapsedTime;
      numDegrees *= timeLeft / mDataBlock->trailFadeTime;
   }
   
   TSShape *shape = shapeInstance->getShape();
   S32 node = shape->findNode( nodeName );

   MatrixF nodeTrans = shapeInstance->mNodeTransforms[node];

   stationTrans.mul( mStationObject->getTransform(), nodeTrans );
   dglMultMatrix( &stationTrans );

   if( mElapsedTime > mDataBlock->fadeDelay )
   {
      F32 timeLeft = mDataBlock->lifetime - mElapsedTime;
      F32 fadeTime = mDataBlock->lifetime - mDataBlock->fadeDelay;
      F32 fadePercent = (timeLeft/fadeTime);
      glColor4f( 1.0, 1.0, 1.0, fadePercent );
   }
   else
   {
      glColor4f( 1.0, 1.0, 1.0, 1.0 );
   }

   glDepthMask( GL_FALSE );
   glEnable( GL_TEXTURE_2D );
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[0].getGLName() );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE );
   
   glBegin( GL_TRIANGLE_STRIP );

      for( U32 i=0; i<numSegments; i++ )
      {
         F32 percent = i / (numSegments-1);
         F32 angle = percent * mDegToRad( numDegrees );
         
         VectorF newPoint( mSin( angle ) * radius, (mCos( angle ) * radius) - radius, height * 0.5 );

         glTexCoord2f( percent, 0.0 );
         glVertex3fv( newPoint );
         newPoint.z -= height;
         glTexCoord2f( percent, 1.0 );
         glVertex3fv( newPoint );

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
U32 StationFXPersonal::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
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
void StationFXPersonal::unpackUpdate(NetConnection* con, BitStream* stream)
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
