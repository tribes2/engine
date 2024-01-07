//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "dgl/dgl.h"
#include "platformWIN32/platformGL.h"
#include "platform/platformAudio.h"
#include "audio/audioDataBlock.h"
#include "scenegraph/sceneGraph.h"
#include "scenegraph/sceneState.h"
#include "core/bitStream.h"
#include "game/particleEngine.h"
#include "game/shockwave.h"
#include "math/mathIO.h"
#include "terrain/terrData.h"
#include "math/mathUtils.h"
#include "sim/netConnection.h"

IMPLEMENT_CO_DATABLOCK_V1(ShockwaveData);
IMPLEMENT_CO_NETOBJECT_V1(Shockwave);

namespace
{

MRandomLCG sgRandom(0xdeadbeef);

} // namespace {}

//----------------------------------------------------------------------------
//--------------------------------------
//

ShockwaveData::ShockwaveData()
{
   soundProfile      = NULL;
   soundProfileId    = 0;

   scale.set(1, 1, 1);

   dMemset( emitterList, 0, sizeof( emitterList ) );
   dMemset( emitterIDList, 0, sizeof( emitterIDList ) );

   delayMS = 0;
   delayVariance = 0;
   lifetimeMS = 1000;
   lifetimeVariance = 0;
   width = 4.0;
   numSegments = 10;
   velocity = 30.0;
   height = 0.0;
   numVertSegments = 1;
   verticalCurve = 1.0;
   acceleration = 0.0;
   texWrap = 1.0;
   is2D = false;
   mapToTerrain = true;
   orientToNormal = false;
   renderBottom = false;
   renderSquare = false;
   
   dMemset( textureName, 0, sizeof( textureName ) );

   U32 i;
   for( i=0; i<NUM_TIME_KEYS; i++ )
   {
      times[i] = 1.0;
   }
   times[0] = 0.0;

   for( i=0; i<NUM_TIME_KEYS; i++ )
   {
      colors[i].set( 1.0, 1.0, 1.0, 1.0 );
   }
}

IMPLEMENT_SETDATATYPE(ShockwaveData)
   IMPLEMENT_GETDATATYPE(ShockwaveData)

   void ShockwaveData::initPersistFields()
{
   Parent::initPersistFields();

   Con::registerType(TypeShockwaveDataPtr, sizeof(ShockwaveData*),
                     REF_GETDATATYPE(ShockwaveData),
                     REF_SETDATATYPE(ShockwaveData));
   
   addField("soundProfile",      TypeAudioProfilePtr,          Offset(soundProfile,       ShockwaveData));
   addField("scale",             TypePoint3F,                  Offset(scale,              ShockwaveData));
   addField("emitter",           TypeParticleEmitterDataPtr,   Offset(emitterList,        ShockwaveData), NUM_EMITTERS);
   addField("delayMS",           TypeS32,                      Offset(delayMS,            ShockwaveData));
   addField("delayVariance",     TypeS32,                      Offset(delayVariance,      ShockwaveData));
   addField("lifetimeMS",        TypeS32,                      Offset(lifetimeMS,         ShockwaveData));
   addField("lifetimeVariance",  TypeS32,                      Offset(lifetimeVariance,   ShockwaveData));
   addField("width",             TypeF32,                      Offset(width,              ShockwaveData));
   addField("numSegments",       TypeS32,                      Offset(numSegments,        ShockwaveData));
   addField("numVertSegments",   TypeS32,                      Offset(numVertSegments,    ShockwaveData));
   addField("velocity",          TypeF32,                      Offset(velocity,           ShockwaveData));
   addField("height",            TypeF32,                      Offset(height,             ShockwaveData));
   addField("verticalCurve",     TypeF32,                      Offset(verticalCurve,      ShockwaveData));
   addField("acceleration",      TypeF32,                      Offset(acceleration,       ShockwaveData));
   addField("times",             TypeF32,                      Offset(times,              ShockwaveData), NUM_TIME_KEYS);
   addField("colors",            TypeColorF,                   Offset(colors,             ShockwaveData), NUM_TIME_KEYS);
   addField("texture",           TypeString,                   Offset(textureName,        ShockwaveData), NUM_TEX);
   addField("texWrap",           TypeF32,                      Offset(texWrap,            ShockwaveData));
   addField("is2D",              TypeBool,                     Offset(is2D,               ShockwaveData));
   addField("mapToTerrain",      TypeBool,                     Offset(mapToTerrain,       ShockwaveData));
   addField("orientToNormal",    TypeBool,                     Offset(orientToNormal,     ShockwaveData));
   addField("renderBottom",      TypeBool,                     Offset(renderBottom,       ShockwaveData));
   addField("renderSquare",      TypeBool,                     Offset(renderSquare,       ShockwaveData));

}

bool ShockwaveData::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   return true;
}

void ShockwaveData::packData(BitStream* stream)
{
   Parent::packData(stream);

   mathWrite(*stream, scale);
   stream->write(delayMS);
   stream->write(delayVariance);
   stream->write(lifetimeMS);
   stream->write(lifetimeVariance);
   stream->write(width);
   stream->write(numSegments);
   stream->write(numVertSegments);
   stream->write(velocity);
   stream->write(height);
   stream->write(verticalCurve);
   stream->write(acceleration);
   stream->write(texWrap);
   stream->write(is2D);
   stream->write(orientToNormal);
   stream->write(mapToTerrain);
   stream->write(renderBottom);
   stream->write(renderSquare);

   S32 i;
   for( i=0; i<NUM_EMITTERS; i++ )
   {
      if( stream->writeFlag( emitterList[i] != NULL ) )
      {
         stream->writeRangedU32( emitterList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }

   for( i=0; i<NUM_TIME_KEYS; i++ )
   {
      stream->write( colors[i] );
   }
  
   for( i=0; i<NUM_TIME_KEYS; i++ )
   {
      stream->write( times[i] );
   }

   for( i=0; i<NUM_TEX; i++ )
   {
      if( textureName[i][0] )
      {
         stream->writeString(textureName[i]);
      }
   }
}

void ShockwaveData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   
   mathRead(*stream, &scale);
   stream->read(&delayMS);
   stream->read(&delayVariance);
   stream->read(&lifetimeMS);
   stream->read(&lifetimeVariance);
   stream->read(&width);
   stream->read(&numSegments);
   stream->read(&numVertSegments);
   stream->read(&velocity);
   stream->read(&height);
   stream->read(&verticalCurve);
   stream->read(&acceleration);
   stream->read(&texWrap);
   stream->read(&is2D);
   stream->read(&orientToNormal);
   stream->read(&mapToTerrain);
   stream->read(&renderBottom);
   stream->read(&renderSquare);

   U32 i;
   for( i=0; i<NUM_EMITTERS; i++ )
   {
      if( stream->readFlag() )
      {
         emitterIDList[i] = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
      }
   }

   for( i=0; i<NUM_TIME_KEYS; i++ )
   {
      stream->read( &colors[i] );
   }
  
   for( i=0; i<NUM_TIME_KEYS; i++ )
   {
      stream->read( &times[i] );
   }

   for( i=0; i<NUM_TEX; i++ )
   {
      textureName[i] = stream->readSTString();
   }
}

bool ShockwaveData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (!server) {
      S32 i;
      for( i=0; i<NUM_EMITTERS; i++ )
      {
         if( !emitterList[i] && emitterIDList[i] != 0 )
         {
            if( Sim::findObject( emitterIDList[i], emitterList[i] ) == false)
            {
               Con::errorf( ConsoleLogEntry::General, "ShockwaveData::onAdd: Invalid packet, bad datablockId(particle emitter): 0x%x", emitterIDList[i] );
            }
         }
      }

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
// Shockwave
//--------------------------------------------------------------------------
Shockwave::Shockwave()
{
   dMemset( mEmitterList, 0, sizeof( mEmitterList ) );

   mDelayMS = 0;
   mCurrMS = 0;
   mEndingMS = 1000;
   mActive = false;
   mRadius = 0.0;
   mVelocity = 1.0;
   mHeight = 0.0;
   mDataBlock = NULL;
   
   mInitialPosition.set( 0.0, 0.0, 0.0 );
   mInitialNormal.set( 0.0, 0.0, 1.0 );
}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
Shockwave::~Shockwave()
{
}

//--------------------------------------------------------------------------
// Set initial state
//--------------------------------------------------------------------------
void Shockwave::setInitialState(const Point3F& point, const Point3F& normal, const F32 fade)
{
   mInitialPosition = point;
   mInitialNormal   = normal;
   mFade            = fade;
   mFog             = 0.0f;
}

//--------------------------------------------------------------------------
// Init persist fields
//--------------------------------------------------------------------------
void Shockwave::initPersistFields()
{
   Parent::initPersistFields();

   addField("pos",    TypePoint3F,   Offset(mInitialPosition,  Shockwave));
   addField("normal",      TypePoint3F,   Offset(mInitialNormal,    Shockwave));

   //
}

//--------------------------------------------------------------------------
// CONSOLE commands
//--------------------------------------------------------------------------
static void cSetInitialState(SimObject *obj, S32, const char** argv)
{

   Shockwave* shock = static_cast<Shockwave*>(obj);
   if( !shock ) return;
   
   Point3F pos;
   dSscanf( argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z );

   Point3F dir;
   dSscanf( argv[3], "%f %f %f", &dir.x, &dir.y, &dir.z );

   shock->setInitialState( pos, dir );
}

//--------------------------------------------------------------------------
// Console Init
//--------------------------------------------------------------------------
void Shockwave::consoleInit()
{
   Con::addCommand("Shockwave", "setInitialState", cSetInitialState, "startFade( pos, normal )", 4, 4);
}

//--------------------------------------------------------------------------
// OnAdd
//--------------------------------------------------------------------------
bool Shockwave::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mDelayMS = mDataBlock->delayMS + sgRandom.randI( -mDataBlock->delayVariance, mDataBlock->delayVariance );
   mEndingMS = mDataBlock->lifetimeMS + sgRandom.randI( -mDataBlock->lifetimeVariance, mDataBlock->lifetimeVariance );

   mVelocity = mDataBlock->velocity;
   mHeight = mDataBlock->height;
   
   mObjBox.min = Point3F( -1, -1, -1 );
   mObjBox.max = Point3F(  1,  1,  1 );
   resetWorldBox();

   gClientContainer.addObject(this);
   gClientSceneGraph->addObjectToScene(this);

   removeFromProcessList();
   gClientProcessList.addObject(this);

   MatrixF trans(true);
   trans.setPosition( mInitialPosition );
   setTransform( trans );

   return true;
}

//--------------------------------------------------------------------------
// OnRemove
//--------------------------------------------------------------------------
void Shockwave::onRemove()
{
   for( int i=0; i<ShockwaveData::NUM_EMITTERS; i++ )
   {
      if( mEmitterList[i] )
      {
         mEmitterList[i]->deleteWhenEmpty();
         mEmitterList[i] = NULL;
      }
   }

   mSceneManager->removeObjectFromScene(this);
   getContainer()->removeObject(this);

   Parent::onRemove();
}


//--------------------------------------------------------------------------
// On New Data Block
//--------------------------------------------------------------------------
bool Shockwave::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<ShockwaveData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
// Prep render image
//--------------------------------------------------------------------------
bool Shockwave::prepRenderImage(SceneState* state, const U32 stateKey,
                                const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      mFog = 0.0;

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
void Shockwave::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   state->setupObjectProjection(this);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   if( mDataBlock->is2D )
   {
      render2DWave();
   }
   else
   {
      if( mDataBlock->renderSquare )
      {
         renderSquare();
      }
      else
      {
         renderWave();
      }
   }

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------------------------------------------
// Process tick
//--------------------------------------------------------------------------
void Shockwave::processTick(const Move*)
{
   mCurrMS += TickMs;

   if( isServerObject() )
   {
      if( mCurrMS >= mEndingMS )
      {
         deleteObject();
         return;
      }
   }

   // set object box
   F32 outerRad = mRadius + mDataBlock->width * 0.5;
   mObjBox.min = Point3F( -outerRad, -outerRad, -outerRad );
   mObjBox.max = Point3F(  outerRad,  outerRad,  outerRad );
   resetWorldBox();

}

//--------------------------------------------------------------------------
// Advance time
//--------------------------------------------------------------------------
void Shockwave::advanceTime(F32 dt)
{
   if (dt == 0.0)
      return;

   updateColor();
   updateWave( dt );
   updateEmitters( dt );
}

//----------------------------------------------------------------------------
// Update emitters
//----------------------------------------------------------------------------
void Shockwave::updateEmitters( F32 dt )
{
   Point3F pos = getPosition();

   for( int i=0; i<ShockwaveData::NUM_EMITTERS; i++ )
   {
      if( mEmitterList[i] )
      {
         mEmitterList[i]->emitParticles( pos, pos, mInitialNormal, Point3F( 0.0, 0.0, 0.0 ), dt * 1000 );
      }
   }

}

//----------------------------------------------------------------------------
// Update wave
//----------------------------------------------------------------------------
void Shockwave::updateWave( F32 dt )
{
   mVelocity += mDataBlock->acceleration * dt;
   mRadius += mVelocity * dt;

}

//----------------------------------------------------------------------------
// Render wave
//----------------------------------------------------------------------------
void Shockwave::renderWave()
{

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   MatrixF orient(true);
   
   if( mDataBlock->orientToNormal )
   {
      MatrixF rotMatrix( EulerF( M_PI/2.0, 0.0, 0.0 ) );
      
      // set up gl modelview matrix
      orient = MathUtils::createOrientFromDir( mInitialNormal );
      orient.mul( rotMatrix );
   }

   orient.setPosition( mInitialPosition );
   dglMultMatrix( &orient );
   

   const F32 minGroundHeight = 0.1;

   F32 innerRad = mRadius - mDataBlock->width * 0.5;
   F32 outerRad = mRadius + mDataBlock->width * 0.5;

   if( innerRad < 0.0 ) innerRad = 0.0;


   glEnable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glEnable(GL_TEXTURE_2D);


   F32 texFactor = mDataBlock->texWrap;
   
   F32 pt = 0.0;
   F32 t = 0.0;

   for( U32 i=1; i<mDataBlock->numSegments+1; i++ )
   {
      glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[0].getGLName());

      pt = t;
      t = F32(i) / F32(mDataBlock->numSegments);

      Point3F vert[2];
      vert[0].x = mSin( pt * M_PI * 2.0 );
      vert[0].y = mCos( pt * M_PI * 2.0 );
      vert[0].z = 0.0;
      vert[1].x = mSin( t * M_PI * 2.0 );
      vert[1].y = mCos( t * M_PI * 2.0 );
      vert[1].z = 0.0;

      Point3F outerPts[2];
      Point3F innerPts[2];

      glBegin( GL_QUADS );

         glColor4fv( mColor );
         glTexCoord2f( t * texFactor, 0.95 );
         Point3F pnt = mInitialPosition + vert[1] * innerRad;
         pnt.z = findHeight( pnt ) + minGroundHeight;
         innerPts[0] = pnt;
         glVertex3fv( pnt - mInitialPosition );

         glColor4fv( mColor );
         glTexCoord2f( pt * texFactor, 0.95 );
         pnt = mInitialPosition + vert[0] * innerRad;
         pnt.z = findHeight( pnt ) + minGroundHeight;
         innerPts[1] = pnt;
         glVertex3fv( pnt - mInitialPosition );

         glColor4fv( mColor );
         glTexCoord2f( pt * texFactor, 0.05 );
         pnt = mInitialPosition + vert[0] * outerRad;
         pnt.z = findHeight( pnt ) + minGroundHeight;
         outerPts[0] = pnt;
         pnt.z += mHeight;
         glVertex3fv( pnt - mInitialPosition );
      
         glColor4fv( mColor );
         glTexCoord2f( t * texFactor, 0.05 );
         pnt = mInitialPosition + vert[1] * outerRad;
         pnt.z = findHeight( pnt ) + minGroundHeight;
         outerPts[1] = pnt;
         pnt.z += mHeight;
         glVertex3fv( pnt - mInitialPosition );

         if( mDataBlock->renderBottom ) 
         {
            glTexCoord2f( t * texFactor, 0.05 );
            glVertex3fv( outerPts[1] - mInitialPosition );
            glTexCoord2f( pt * texFactor, 0.05 );
            glVertex3fv( outerPts[0] - mInitialPosition );
            glTexCoord2f( pt * texFactor, 0.95 );
            glVertex3fv( innerPts[1] - mInitialPosition );
            glTexCoord2f( t * texFactor, 0.95 );
            glVertex3fv( innerPts[0] - mInitialPosition );
         }

      glEnd();

      renderVerticalWall( outerPts[1], outerPts[0] );
      
   }


   glDisable(GL_CULL_FACE);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glDepthMask(GL_TRUE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

}

//----------------------------------------------------------------------------
// Render vertical wall
//----------------------------------------------------------------------------
void Shockwave::renderVerticalWall( Point3F &pnt1, Point3F pnt2 )
{
   Point3F p1 = pnt1;
   Point3F p2 = pnt2;

   p1.z += mHeight * 0.5;
   p2.z += mHeight * 0.5;

   Point3F dir1, dir2;
   dir1 = p1 - mInitialPosition;
   dir1.z = 0.0;
   dir1.normalize();
   dir2 = p2 - mInitialPosition;
   dir2.z = 0.0;
   dir2.normalize();

   F32 vertRadius = mHeight * 0.5;

   glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[1].getGLName());

   glBegin( GL_QUAD_STRIP );

   for( U32 i=0; i<mDataBlock->numVertSegments+1; i++ )
   {
      F32 t = F32(i) / F32(mDataBlock->numVertSegments);

      glColor4f( mColor.red, mColor.green, mColor.blue, mColor.alpha );

      F32 v = t;

      if( mDataBlock->renderBottom )
      {
         v = 0.0;
      }
      else
      {
         if( v < 0.05 ) v = 0.05;
         if( v > .95 ) v = 0.95;
      }

      glTexCoord2f( 0.0, v );

      Point3F pnt = p1;
      pnt.z += mCos( t * M_PI ) * vertRadius;
      pnt += dir1 * mSin( t * M_PI ) * mDataBlock->verticalCurve;
      glVertex3fv( pnt - mInitialPosition );

      glTexCoord2f( 1.0, v );

      pnt = p2;
      pnt.z += mCos( t * M_PI ) * vertRadius;
      pnt += dir2 * mSin( t * M_PI ) * mDataBlock->verticalCurve;
      glVertex3fv( pnt - mInitialPosition );

   }
   
   glEnd();

}

//----------------------------------------------------------------------------
// Find height
//----------------------------------------------------------------------------
F32 Shockwave::findHeight( Point3F &point )
{
   if( mDataBlock->mapToTerrain )
   {
      TerrainBlock* pTerrain = mSceneManager->getCurrentTerrain();
      if( !pTerrain ) return 0.0;

      Point3F terrPt = point;
      pTerrain->getWorldTransform().mulP(terrPt);
      F32 h;
      if (pTerrain->getHeight(Point2F(terrPt.x, terrPt.y), &h))
      {
         return h;
      }
   }
   else
   {
      return point.z;
   }


   return 0.0;
}

//----------------------------------------------------------------------------
// Update color
//----------------------------------------------------------------------------
void Shockwave::updateColor()
{
   F32 t = F32(mCurrMS) / F32(mEndingMS);

   for( U32 i = 1; i < ShockwaveData::NUM_TIME_KEYS; i++ )
   {
      if( mDataBlock->times[i] >= t )
      {
         F32 firstPart =   t - mDataBlock->times[i-1];
         F32 total     =   (mDataBlock->times[i] - 
                            mDataBlock->times[i-1]);

         firstPart /= total;

         mColor.interpolate( mDataBlock->colors[i-1],
                             mDataBlock->colors[i],
                             firstPart);


         return;
      }
   }
}

//----------------------------------------------------------------------------
// Render billboarded shockwave
//----------------------------------------------------------------------------
void Shockwave::render2DWave()
{
   
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   // set up gl modelview matrix
   MatrixF camTrans;
   dglGetModelview(&camTrans);
   Point3F camLookDir;
   camTrans.getRow(1, &camLookDir);
   Point3F negCamDir = -camLookDir;
   MatrixF billBoard = MathUtils::createOrientFromDir( negCamDir );
   billBoard.setPosition( getPosition() );
   dglMultMatrix( &billBoard );


   const F32 minGroundHeight = 0.1;

   F32 innerRad = mRadius - mDataBlock->width * 0.5;
   F32 outerRad = mRadius + mDataBlock->width * 0.5;

   if( innerRad < 0.0 ) innerRad = 0.0;


   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glEnable(GL_TEXTURE_2D);


   F32 texFactor = mDataBlock->texWrap;
   
   F32 pt = 0.0;
   F32 t = 0.0;

   for( U32 i=1; i<mDataBlock->numSegments+1; i++ )
   {
      glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[0].getGLName());

      pt = t;
      t = F32(i) / F32(mDataBlock->numSegments);

      Point3F vert[2];
      vert[0].x = mSin( pt * M_PI * 2.0 );
      vert[0].z = mCos( pt * M_PI * 2.0 );
      vert[0].y = 0.0;
      vert[1].x = mSin( t * M_PI * 2.0 );
      vert[1].z = mCos( t * M_PI * 2.0 );
      vert[1].y = 0.0;

      Point3F outerPts[2];

      glBegin( GL_QUADS );

         glColor4fv( mColor );
         glTexCoord2f( t * texFactor, 0.95 );
         Point3F pnt = vert[1] * innerRad;
         glVertex3fv( pnt );

         glColor4fv( mColor );
         glTexCoord2f( pt * texFactor, 0.95 );
         pnt = vert[0] * innerRad;
         glVertex3fv( pnt );

         glColor4fv( mColor );
         glTexCoord2f( pt * texFactor, 0.05 );
         pnt = vert[0] * outerRad;
         outerPts[0] = pnt;
         glVertex3fv( pnt );
      
         glColor4fv( mColor );
         glTexCoord2f( t * texFactor, 0.05 );
         pnt = vert[1] * outerRad;
         outerPts[1] = pnt;
         glVertex3fv( pnt );

      glEnd();
      
   }


   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glDepthMask(GL_TRUE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

}

//----------------------------------------------------------------------------
// Render wave
//----------------------------------------------------------------------------
void Shockwave::renderSquare()
{

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   MatrixF orient(true);
   
   if( mDataBlock->orientToNormal )
   {
      MatrixF rotMatrix( EulerF( M_PI/2.0, 0.0, 0.0 ) );
      
      // set up gl modelview matrix
      orient = MathUtils::createOrientFromDir( mInitialNormal );
      orient.mul( rotMatrix );
   }

   orient.setPosition( mInitialPosition );
   dglMultMatrix( &orient );


   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[0].getGLName());


   glBegin( GL_QUADS );

      glColor4fv( mColor );
      glTexCoord2f( 0.0, 0.0 );
      glVertex3f( -mRadius, -mRadius, 0.0 );

      glColor4fv( mColor );
      glTexCoord2f( 0.0, 1.0 );
      glVertex3f( -mRadius,  mRadius, 0.0 );

      glColor4fv( mColor );
      glTexCoord2f( 1.0, 1.0 );
      glVertex3f(  mRadius,  mRadius, 0.0 );

      glColor4fv( mColor );
      glTexCoord2f( 1.0, 0.0 );
      glVertex3f(  mRadius, -mRadius, 0.0 );

   glEnd();

   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glDepthMask(GL_TRUE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();


}

//--------------------------------------------------------------------------
// packUpdate
//--------------------------------------------------------------------------
U32 Shockwave::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if( stream->writeFlag(mask & GameBase::InitialUpdateMask) )
   {
      mathWrite(*stream, mInitialPosition);
      mathWrite(*stream, mInitialNormal);
   }

   return retMask;
}

//--------------------------------------------------------------------------
// unpackUpdate
//--------------------------------------------------------------------------
void Shockwave::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if( stream->readFlag() )
   {
      mathRead(*stream, &mInitialPosition);
      mathRead(*stream, &mInitialNormal);
   }
}
