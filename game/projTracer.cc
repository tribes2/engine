//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projTracer.h"
#include "Core/bitStream.h"
#include "console/consoleTypes.h"
#include "game/Debris.h"

#include "dgl/dgl.h"
#include "PlatformWin32/platformGL.h"
#include "sceneGraph/sceneState.h"
#include "Math/mathUtils.h"

IMPLEMENT_CO_DATABLOCK_V1(TracerProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(TracerProjectile);

//--------------------------------------------------------------------------
//--------------------------------------
//
TracerProjectileData::TracerProjectileData()
{
   tracerLength    = 10;
   tracerMinPixels = 3;
   tracerAlpha     = false;
   tracerColor.set(1, 1, 1, 1);
   tracerWidth     = 0.5;
   crossViewAng = 0.98;
   crossSize = 0.45;
   renderCross = true;

   dMemset( textureName, 0, sizeof( textureName ) );
   dMemset( textureHandle, 0, sizeof( textureHandle ) );
}

TracerProjectileData::~TracerProjectileData()
{

}


//--------------------------------------------------------------------------
void TracerProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("tracerLength",      TypeF32,            Offset(tracerLength,        TracerProjectileData));
   addField("tracerMinPixels",   TypeF32,            Offset(tracerMinPixels,     TracerProjectileData));
   addField("tracerAlpha",       TypeBool,           Offset(tracerAlpha,         TracerProjectileData));
   addField("tracerColor",       TypeColorF,         Offset(tracerColor,         TracerProjectileData));
   addField("tracerTex",         TypeString,         Offset(textureName,         TracerProjectileData), NUM_TEX );
   addField("tracerWidth",       TypeF32,            Offset(tracerWidth,         TracerProjectileData));
   addField("crossViewAng",      TypeF32,            Offset(crossViewAng,        TracerProjectileData));
   addField("crossSize",         TypeF32,            Offset(crossSize,           TracerProjectileData));
   addField("renderCross",       TypeBool,           Offset(renderCross,         TracerProjectileData));
}


//--------------------------------------------------------------------------
bool TracerProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (tracerLength < 1.0 || tracerLength > 50) {
      Con::warnf(ConsoleLogEntry::General, "TracerProjectileData(%s)::onAdd: tracerLength must be in the range [1, 50]", getName());
      tracerLength = tracerLength < 1.0 ? 1.0 : 50;
   }
   if (tracerMinPixels < 1.0 || tracerMinPixels > 20) {
      Con::warnf(ConsoleLogEntry::General, "TracerProjectileData(%s)::onAdd: tracerMinPixels must be in the range [1, 20]", getName());
      tracerMinPixels = tracerMinPixels < 1.0 ? 1.0 : 20;
   }

   tracerColor.clamp();

   return true;
}


//--------------------------------------------------------------------------
bool TracerProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;


   if( server == false && textureName[0] )
   {
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
void TracerProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(tracerLength);
   stream->write(tracerWidth);
   stream->write(tracerMinPixels);
   stream->write(tracerAlpha);
   stream->write(tracerColor);
   stream->write(crossViewAng);
   stream->write(crossSize);
   stream->write(renderCross);

   U32 i;
   for( i=0; i<NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }
}

void TracerProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&tracerLength);
   stream->read(&tracerWidth);
   stream->read(&tracerMinPixels);
   stream->read(&tracerAlpha);
   stream->read(&tracerColor);
   stream->read(&crossViewAng);
   stream->read(&crossSize);
   stream->read(&renderCross);

   for( U32 i=0; i<NUM_TEX; i++ )
   {
      textureName[i] = stream->readSTString();
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
TracerProjectile::TracerProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);

   //
}

TracerProjectile::~TracerProjectile()
{
   //
}


//--------------------------------------------------------------------------
bool TracerProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (isClientObject()) {
      mCurrLength  = 0;
   }

   Point3F min( -mDataBlock->tracerWidth * 0.5, -mDataBlock->tracerLength * 0.5, -mDataBlock->tracerWidth * 0.5 );
   mObjBox = Box3F( min, -min );

   return true;
}


void TracerProjectile::onRemove()
{
   //

   Parent::onRemove();
}


bool TracerProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<TracerProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   // Todo: Uncomment if this is a "leaf" class
   //scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
void TracerProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   mCurrLength += getVelocity().len() * dt * (2.0 / 3.0);
   if (mCurrLength > mDataBlock->tracerLength)
      mCurrLength = mDataBlock->tracerLength;
}


//--------------------------------------------------------------------------
class TracerRenderImage : public SceneRenderImage {
  public:
   F32 length;
};

bool TracerProjectile::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   if (mHidden == true || mFadeValue <= (1.0/255.0))
      return false;

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      TracerRenderImage* image = new TracerRenderImage;
      image->obj = this;
      image->isTranslucent = true;
      image->sortType = SceneRenderImage::Point;
      state->setImageRefPoint(this, image);

      // Calculate image length...
      image->length = mCurrLength;

      state->insertRenderImage(image);
   }

   return false;
}


//--------------------------------------------------------------------------
void TracerProjectile::renderObject(SceneState* state, SceneRenderImage*)
{
   if( mCurrTick >= mDeleteTick ) return;


   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);

   renderProjectile( state->getCameraPosition() );

   if( mDataBlock->renderCross )
   {
      renderCrossSection( state->getCameraPosition() );
   }

   // restore state
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   glDepthMask(GL_TRUE);

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------------------------------------------
// Render projectile
//--------------------------------------------------------------------------
void TracerProjectile::renderProjectile( const Point3F &camPos )
{
   Point3F vel = getVelocity();
 
   if( vel.isZero() )
   {
      return;
   }

   Point3F dir = mInitialDirection;
   dir.normalize(); 
 
   Point3F pos = getRenderPosition();
   Point3F dirFromCam = pos - camPos;
   Point3F crossDir;
   mCross( dirFromCam, dir, &crossDir );
   crossDir.normalize();


   F32 width = mDataBlock->tracerWidth;
   F32 length = mDataBlock->tracerLength / 2.0;

   crossDir *= width;
   Point3F start = pos - length * dir;
   Point3F end = pos + length * dir;


   // clip start point
   Point3F p1 = start - mInitialPosition;
   Point3F p2 = end - mInitialPosition;
   if( mDot( p1, p2 ) < 0.0 )
   {
      start = mInitialPosition;
   }


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
void TracerProjectile::renderCrossSection( const Point3F &camPos )
{

   Point3F dir = getVelocity();
 
   if( dir.isZero() )
   {
      return;
   }

   dir = mInitialDirection;

   Point3F pos = getRenderPosition();
   Point3F dirFromCam = pos - camPos;
   dirFromCam.normalize();
   dir.normalize();

   F32 angle = mDot( dir, dirFromCam );
   if( angle > -mDataBlock->crossViewAng && angle < mDataBlock->crossViewAng ) return;

   F32 width = mDataBlock->crossSize / 2.0;


   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   MatrixF mat = MathUtils::createOrientFromDir( dir );
   mat.setPosition( getRenderPosition() );
   dglMultMatrix( &mat );

   glColor4f( 1.0, 1.0, 1.0, 1.0 );
   glBindTexture( GL_TEXTURE_2D, mDataBlock->textureHandle[1].getGLName() );

   glBegin(GL_QUADS);

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
