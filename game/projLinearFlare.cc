//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/projLinearFlare.h"
#include "core/bitStream.h"
#include "console/consoleTypes.h"

#include "dgl/dgl.h"
#include "platformWIN32/platformGL.h"
#include "scenegraph/sceneState.h"

IMPLEMENT_CO_DATABLOCK_V1(LinearFlareProjectileData);
IMPLEMENT_CO_NETOBJECT_V1(LinearFlareProjectile);

//--------------------------------------------------------------------------
//--------------------------------------
//
LinearFlareProjectileData::LinearFlareProjectileData()
{
   numFlares  = 25;
   size[0]   = .2;
   size[1]   = .25;
   size[2]   = 3.0;
   flareColor = ColorF(0.25, 0.25, 1);

   flareModTexture = "";
   flareBaseTexture = "";
}

LinearFlareProjectileData::~LinearFlareProjectileData()
{
   
}


//--------------------------------------------------------------------------
void LinearFlareProjectileData::initPersistFields()
{
   Parent::initPersistFields();

   addField("numFlares",  TypeS32,    Offset(numFlares,  LinearFlareProjectileData));
   addField("size",       TypeF32,    Offset(size,       LinearFlareProjectileData), NUM_SIZES);
   addField("flareColor", TypeColorF, Offset(flareColor, LinearFlareProjectileData));

   addField("flareModTexture",  TypeString, Offset(flareModTexture,  LinearFlareProjectileData));
   addField("flareBaseTexture", TypeString, Offset(flareBaseTexture, LinearFlareProjectileData));
}


//--------------------------------------------------------------------------
bool LinearFlareProjectileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (numFlares < 0) {
      Con::warnf(ConsoleLogEntry::General, "LinearFlareProjectile(%s)::onAdd: must have at least no flares", getName());
      numFlares = 0;
   }
   flareColor.clamp();

   if (!flareModTexture || !flareModTexture[0] || !flareBaseTexture || !flareBaseTexture[0]) {
      Con::warnf(ConsoleLogEntry::General, "LinearFlareProjectile(%s)::onAdd: must have mod and base textures", getName());
      return false;
   }
   if (dStrlen(flareModTexture) >= 255 || dStrlen(flareBaseTexture) >= 255) {
      Con::warnf(ConsoleLogEntry::General, "LinearFlareProjectile(%s)::onAdd: must have mod and base textures with strlen < 255", getName());
      return false;
   }

   return true;
}


bool LinearFlareProjectileData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if(!server)
   {
      baseHandle = TextureHandle(flareBaseTexture, MeshTexture);
      modHandle  = TextureHandle(flareModTexture, MeshTexture);
      if((TextureObject*)baseHandle == NULL)
      {
         dSprintf(errorBuffer, sizeof(errorBuffer), "Unable to load texture: %s", flareBaseTexture);
         return false;
      }
      if((TextureObject*)modHandle == NULL)
      {
         dSprintf(errorBuffer, sizeof(errorBuffer), "Unable to load texture: %s", flareModTexture);
         return false;
      }
   }
   return true;
}


//--------------------------------------------------------------------------
void LinearFlareProjectileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(numFlares);
   stream->write(flareColor);
   stream->writeString(flareModTexture);
   stream->writeString(flareBaseTexture);

   for( U32 i=0; i<NUM_SIZES; i++ )
   {
      stream->write( size[i] );
   }
}

void LinearFlareProjectileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&numFlares);
   stream->read(&flareColor);
   flareModTexture  = stream->readSTString();
   flareBaseTexture = stream->readSTString();

   for( U32 i=0; i<NUM_SIZES; i++ )
   {
      stream->read( &size[i] );
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
LinearFlareProjectile::LinearFlareProjectile()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);

   mFlares = NULL;
}

LinearFlareProjectile::~LinearFlareProjectile()
{
   delete [] mFlares;
   mFlares = NULL;
}

//--------------------------------------------------------------------------
void LinearFlareProjectile::initPersistFields()
{
   Parent::initPersistFields();

   //
}


//--------------------------------------------------------------------------
bool LinearFlareProjectile::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   U32 numSizes = LinearFlareProjectileData::NUM_SIZES;

   mObjBox.min.set(mDataBlock->size[numSizes], mDataBlock->size[numSizes], mDataBlock->size[numSizes]);
   mObjBox.max.set(mDataBlock->size[numSizes], mDataBlock->size[numSizes], mDataBlock->size[numSizes]);
   mObjBox.min.neg();
   resetWorldBox();

   if (isClientObject()) {
      if (mDataBlock->numFlares > 0)
         mFlares = new Flare[mDataBlock->numFlares];
      else
         mFlares = NULL;

      for (U32 i = 0; i < mDataBlock->numFlares; i++)
         mFlares[i].active = false;
   }

   return true;
}


void LinearFlareProjectile::onRemove()
{
   Parent::onRemove();
}


bool LinearFlareProjectile::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<LinearFlareProjectileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
void LinearFlareProjectile::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if (mDataBlock->activateDelayMS != -1 &&
       mCurrTick * TickMs < mDataBlock->activateDelayMS)
      return;

   static MRandomLCG sRand(50817);
   for (U32 i = 0; i < mDataBlock->numFlares; i++) {
      if (mFlares[i].active) {
         mFlares[i].currT += dt;
         if (mFlares[i].currT > mFlares[i].t2) {
            mFlares[i].active = false;
         }
      }

      if (mFlares[i].active == false) {
         mFlares[i].minTheta = mDegToRad(87.0f);
         mFlares[i].maxTheta = mDegToRad(65.0f + 20 * sRand.randF());
         mFlares[i].h0       = mDataBlock->size[0];
         mFlares[i].h1       = mDataBlock->size[1] + 0.25 * sRand.randF();
         mFlares[i].h2       = mDataBlock->size[2] + sRand.randF();

         mFlares[i].a0       = 0.0;
         mFlares[i].a1       = 1.0;
         mFlares[i].a2       = 0.0;
         mFlares[i].currT    = 0.0;
         mFlares[i].active   = true;
         mFlares[i].normal   = Point3F(1.0 - 2*sRand.randF(), 1.0 - 2*sRand.randF(), 1.0 - 2*sRand.randF());
         mFlares[i].normal.normalize();

         mFlares[i].t0       = 0.0;
         mFlares[i].t1       = 0.15 + 0.225 * sRand.randF();
         mFlares[i].t2       = 0.4  + 0.595 * sRand.randF();
      }

      mFlares[i].generatePoints();
   }
}


//--------------------------------------------------------------------------
void LinearFlareProjectile::renderObject(SceneState* state, SceneRenderImage *sri /*sceneImage*/)
{
   if( mHitWater && mPlayedSplash ) return;

   if( mProjectileShape )
   {
      Parent::renderObject( state, sri );
   }

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   // Uncomment this if this is a "simple" (non-zone managing) object
   state->setupObjectProjection(this);

   Point3F cameraOffset;
   getRenderTransform().getColumn(3,&cameraOffset);
   cameraOffset -= state->getCameraPosition();
   F32 fogAmount = state->getHazeAndFog(cameraOffset.len(),cameraOffset.z);

   MatrixF mv;
   dglGetModelview(&mv);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   dglMultMatrix(&getRenderTransform());

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, mDataBlock->modHandle.getGLName());
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   glEnable(GL_CULL_FACE);
   glBlendFunc(GL_ONE, GL_ONE);
   glEnable(GL_BLEND);
   glDepthMask(GL_FALSE);


   for (U32 i = 0; i < mDataBlock->numFlares; i++) {
      if (mFlares[i].active)
         mFlares[i].render(mDataBlock->flareColor, mFadeValue * (1.0 - fogAmount));
   }

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();


   if( !mProjectileShape )
   {
      Point3F ax0;
      Point3F ax1;
      Point3F center;
      mv.getRow(0, &ax0);
      mv.getRow(2, &ax1);
      getRenderTransform().getColumn(3, &center);

      glDisable(GL_CULL_FACE);
      glBindTexture(GL_TEXTURE_2D, mDataBlock->baseHandle.getGLName());
      glColor3f(mSqrt(mDataBlock->flareColor.red)   * mFadeValue * (1.0 - fogAmount),
                mSqrt(mDataBlock->flareColor.green) * mFadeValue * (1.0 - fogAmount),
                mSqrt(mDataBlock->flareColor.blue)  * mFadeValue * (1.0 - fogAmount));
      glBegin(GL_TRIANGLE_FAN);
         glTexCoord2f(0, 0);
         glVertex3fv(center + ((ax0 * -0.6) + (ax1 * -0.6)) * mDataBlock->scale.x);
         glTexCoord2f(0, 1);
         glVertex3fv(center + ((ax0 * -0.6) + (ax1 *  0.6)) * mDataBlock->scale.x);
         glTexCoord2f(1, 1);
         glVertex3fv(center + ((ax0 *  0.6) + (ax1 *  0.6)) * mDataBlock->scale.x);
         glTexCoord2f(1, 0);
         glVertex3fv(center + ((ax0 *  0.6) + (ax1 * -0.6)) * mDataBlock->scale.x);
      glEnd();
      glColor3f(mFadeValue * (1.0 - fogAmount),
                mFadeValue * (1.0 - fogAmount),
                mFadeValue * (1.0 - fogAmount));
      glBegin(GL_TRIANGLE_FAN);
         glTexCoord2f(0, 0);
         glVertex3fv(center + ((ax0 * -0.3) + (ax1 * -0.3)) * mDataBlock->scale.x);
         glTexCoord2f(0, 1);
         glVertex3fv(center + ((ax0 * -0.3) + (ax1 *  0.3)) * mDataBlock->scale.x);
         glTexCoord2f(1, 1);
         glVertex3fv(center + ((ax0 *  0.3) + (ax1 *  0.3)) * mDataBlock->scale.x);
         glTexCoord2f(1, 0);
         glVertex3fv(center + ((ax0 *  0.3) + (ax1 * -0.3)) * mDataBlock->scale.x);
      glEnd();
   }



   glDisable(GL_CULL_FACE);
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
U32 LinearFlareProjectile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   //

   return retMask;
}

void LinearFlareProjectile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   //
}


//--------------------------------------------------------------------------
void LinearFlareProjectile::Flare::generatePoints()
{
   Point3F axisz = normal;
   Point3F axisx;
   Point3F axisy;

   if (mFabs(axisz.x) < 0.9) {
      mCross(axisz, Point3F(1, 0, 0), &axisy);
      axisy.normalize();
      mCross(axisz, axisy, &axisx);
      axisx.normalize();
      axisx.neg();
   } else {
      mCross(axisz, Point3F(0, 1, 0), &axisy);
      axisy.normalize();
      mCross(axisz, axisy, &axisx);
      axisx.normalize();
      axisx.neg();
   }

   F32 currTheta;
   F32 currH1;
   if (currT < t1) {
      currTheta = minTheta;
      currH1 = h1 + (h2 - h1) * (currT / t1);
   } else {
      currTheta = minTheta + ((maxTheta - minTheta) * ((currT - t1) / (t2 - t1)));
      currH1 = h2;
   }
   F32 zmul = 1 - mSin(currTheta);

   axisx *= mCos(currTheta);
   axisy *= mCos(currTheta);

   points[0] = (axisz - axisx - axisy) * h0;
   points[1] = (axisz - axisy        ) * h0;
   points[2] = (axisz + axisx - axisy) * h0;
   points[3] = (axisz + axisx        ) * h0;
   points[4] = (axisz + axisx + axisy) * h0;
   points[5] = (axisz + axisy        ) * h0;
   points[6] = (axisz - axisx + axisy) * h0;
   points[7] = (axisz - axisx        ) * h0;

   points[8]  = ((axisz - axisx - axisy) * currH1) - axisz * zmul;
   points[9]  = ((axisz - axisy        ) * currH1) - axisz * zmul;
   points[10] = ((axisz + axisx - axisy) * currH1) - axisz * zmul;
   points[11] = ((axisz + axisx        ) * currH1) - axisz * zmul;
   points[12] = ((axisz + axisx + axisy) * currH1) - axisz * zmul;
   points[13] = ((axisz + axisy        ) * currH1) - axisz * zmul;
   points[14] = ((axisz - axisx + axisy) * currH1) - axisz * zmul;
   points[15] = ((axisz - axisx        ) * currH1) - axisz * zmul;
}

void LinearFlareProjectile::Flare::render(const ColorF& color, const F32 currFade)
{
   F32 currA;
   if (currT < t1) {
      currA = a0 + (a1 - a0) * (currT / t1);
   } else {
      currA = a1 + (a2 - a1) * ((currT - t1) / (t2 - t1));
   }
   currA *= currFade;

   ColorF base   = color * currA * currFade;
   ColorF bright = color * currA * 0.5;
   ColorF dim    = color * (0.25 * currA);

   for (U32 i = 0; i <= 6; i+=2) {
      glBegin(GL_TRIANGLE_FAN);
         glTexCoord2f(0.5, 0.9);
         glColor3f(base.red, base.green, base.blue);
         glVertex3fv(points[i + 1]);

         glTexCoord2f(0, 0.9);
         glColor3f(base.red, base.green, base.blue);
         glVertex3fv(points[i + 0]);

         glTexCoord2f(0, 0.1);
         glColor3f(bright.red, bright.green, bright.blue);
         glVertex3fv(points[i + 8 + 0]);

         glTexCoord2f(0.5, 0.1);
         glColor3f(dim.red, dim.green, dim.blue);
         glVertex3fv(points[i + 8 + 1]);

         glTexCoord2f(1, 0.1);
         glColor3f(bright.red, bright.green, bright.blue);
         glVertex3fv(points[8 + ((i + 2) & 0x7)]);

         glTexCoord2f(1, 0.9);
         glColor3f(base.red, base.green, base.blue);
         glVertex3fv(points[(i + 2) & 0x7]);
      glEnd();

   }
}
