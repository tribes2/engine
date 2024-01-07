//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projFlareGrenade.h"
#include "core/bitStream.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"
#include "scenegraph/sceneState.h"
#include "game/shapeBase.h"
#include "math/mathUtils.h"

IMPLEMENT_CO_DATABLOCK_V1(FlareProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(FlareProjectile);

//--------------------------------------------------------------------------
//--------------------------------------
//
FlareProjectileData::FlareProjectileData()
{
   dMemset( textureNameList, 0, sizeof( textureNameList ) );
   size = 50.0;
   useLensFlare = true;
}

FlareProjectileData::~FlareProjectileData()
{

}


//--------------------------------------------------------------------------
void FlareProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("texture",        TypeString, Offset(textureNameList,    FlareProjectileData), FPD_NUM_TEX);
   addField("size",           TypeF32,    Offset(size,               FlareProjectileData));
   addField("useLensFlare",   TypeBool,   Offset(useLensFlare,       FlareProjectileData));

}


//--------------------------------------------------------------------------
bool FlareProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}


//--------------------------------------------------------------------------
bool FlareProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (server == false)
   {
      for( int i=0; i<FPD_NUM_TEX; i++ )
      {
         if (textureNameList[i] && textureNameList[i][0])
         {
            textureList[i] = TextureHandle(textureNameList[i], MeshTexture );
         }
      }

   }
   else
   {
      for( int i=0; i<FPD_NUM_TEX; i++ )
      {
         textureList[i] = TextureHandle();  // set default NULL tex
      }
   }

   return true;
}


//--------------------------------------------------------------------------
void FlareProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(size);
   stream->write(useLensFlare);

   for( int i=0; i<FPD_NUM_TEX; i++ )
   {
      stream->writeString(textureNameList[i]);
   }
}

void FlareProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&size);
   stream->read(&useLensFlare);
   
   for( int i=0; i<FPD_NUM_TEX; i++ )
   {
      textureNameList[i] = stream->readSTString();
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
FlareProjectile::FlareProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);

   mLifetime = 10.0;
   mElapsedTime = 0.0;
}

FlareProjectile::~FlareProjectile()
{
   //
}

//--------------------------------------------------------------------------
void FlareProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


void FlareProjectile::consoleInit()
{
   //
}


//--------------------------------------------------------------------------
bool FlareProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   setupLensFlare();

   return true;
}


void FlareProjectile::onRemove()
{
   //

   Parent::onRemove();
}


bool FlareProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<FlareProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   // Todo: Uncomment if this is a "leaf" class
   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
U32 FlareProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   //

   return retMask;
}

void FlareProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   //
}


//--------------------------------------------------------------------------
void FlareProjectile::processTick(const Move* move)
{
   Parent::processTick(move);

   if (mLifetime <= 0.0)
   {
//      deleteObject();
   }
}

//--------------------------------------------------------------------------
void FlareProjectile::advanceTime( float dt )
{
   mElapsedTime += dt;

   mLifetime -= dt;
   if( mLifetime <= 0.0 )
   {
      mLifetime = 0.0;
      return;
   }

   updateBlur();
}



//----------------------------------------------------------------------------
// Render Object
//----------------------------------------------------------------------------
void FlareProjectile::renderObject(SceneState* state, SceneRenderImage* )
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   F32 ang = mElapsedTime * 0.5;

   renderFlare( state->getCameraPosition(), ang );

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glPushMatrix();

   renderFlare( state->getCameraPosition(), -ang * 0.5 );

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();

   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   dglSetCanonicalState();
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//----------------------------------------------------------------------------
// Render flare
//----------------------------------------------------------------------------
void FlareProjectile::renderFlare( const Point3F &camPos, F32 ang )
{
   Point3F pos = getRenderPosition();

   MatrixF camTrans;
   dglGetModelview(&camTrans);
   camTrans.setPosition( camPos );
   Point3F camLookDir;
   camTrans.getRow(1, &camLookDir);

   // find 2D screen points for flares
   Point3F screenPoint;
   dglPointToScreen( pos, screenPoint );

   for( int j=0; j<FP_NUM_BLUR_FLARES; j++ )
   {
      dglPointToScreen( mBlurList[j].pos, mBlurList[j].screenPoint );
   }


   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glDepthMask(GL_FALSE);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   
   glColor4f( 1.0, 1.0, 1.0, 1.0 );
   glBindTexture(GL_TEXTURE_2D, mDataBlock->textureList[FlareProjectileData::FPD_FLARE_TEX].getGLName());

   MatrixF rotMatrix( true );
   rotMatrix.set( EulerF( 0.0, ang, 0.0 ) );
   
   Point3F negCamDir = -camLookDir;
   MatrixF billBoard = MathUtils::createOrientFromDir( negCamDir );
   billBoard.mul( rotMatrix );
   billBoard.setPosition( getRenderPosition() );

   dglMultMatrix( &billBoard );

   
   glBegin(GL_QUADS);

      F32 width = mDataBlock->size * 0.5;

      glTexCoord2f(0, 0);
      glVertex3f( -width, 0, -width );

      glTexCoord2f(0, 1);
      glVertex3f( width, 0, -width );

      glTexCoord2f(1, 1);
      glVertex3f( width, 0, width );

      glTexCoord2f(1, 0);
      glVertex3f( -width, 0, width );

   glEnd();

   if( mDataBlock->useLensFlare )
   {
      // render lens flare
      mLensFlare.render( camTrans, pos );
   }

   glDepthMask(GL_TRUE);

   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}

//----------------------------------------------------------------------------
// Update blur
//----------------------------------------------------------------------------
void FlareProjectile::updateBlur()
{
   Point3F blurDir = -getVelocity();
   F32 velMag = blurDir.magnitudeSafe() * 0.5;
   if( velMag > 3.0 )
      velMag = 3.0;

   blurDir.normalizeSafe();
   Point3F pos = getRenderPosition();

   for( int i=0; i<FP_NUM_BLUR_FLARES; i++ )
   {

      F32 t = (F32)(i+1) / (F32)(FP_NUM_BLUR_FLARES);

      mBlurList[i].pos = pos + blurDir * velMag * t;
      mBlurList[i].alpha = mBlurList[i].startAlpha - (mBlurList[i].startAlpha * t);
   }

}

//----------------------------------------------------------------------------
// Setup for lens flare
//----------------------------------------------------------------------------
void FlareProjectile::setupLensFlare()
{

   LFlare flare;

   flare.offset = 0.1;
   flare.size = 150.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 0.5 );
   mLensFlare.addFlare( flare );

   flare.offset = 0.3;
   flare.size = 100.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 0.7 );
   mLensFlare.addFlare( flare );

   flare.offset = 0.45;
   flare.size = 10.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 1.0 );
   mLensFlare.addFlare( flare );

   flare.offset = 0.75;
   flare.size = 20.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 0.8 );
   mLensFlare.addFlare( flare );

   flare.offset = 1.0;
   flare.size = 150.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 1.0 );
   mLensFlare.addFlare( flare );

   flare.offset = 1.5;
   flare.size = 100.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 0.4 );
   mLensFlare.addFlare( flare );

   flare.offset = 0.17;
   flare.size = 150.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 0.4 );
   mLensFlare.addFlare( flare );

   flare.offset = 0.23;
   flare.size = 80.0;
   flare.tex = mDataBlock->textureList[FlareProjectileData::FPD_LENS0_TEX];
   flare.color.set( 1.0, 0.0, 0.0, 1.0 );
   mLensFlare.addFlare( flare );


}

//----------------------------------------------------------------------------
// Got Heat?
//----------------------------------------------------------------------------
F32 FlareProjectile::getHeat() const
{
   return 1.0f;
}

