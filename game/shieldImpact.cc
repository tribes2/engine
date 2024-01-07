//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "dgl/dgl.h"
#include "PlatformWin32/platformGL.h"
#include "Platform/platformAudio.h"
#include "audio/audioDataBlock.h"
#include "sceneGraph/sceneGraph.h"
#include "sceneGraph/sceneState.h"
#include "Core/bitStream.h"
#include "game/particleEngine.h"
#include "Math/mathIO.h"
#include "terrain/terrData.h"
#include "game/explosion.h"
#include "game/shieldImpact.h"
#include "game/shapeBase.h"
#include "ts/tsShapeInstance.h"
#include "Math/mathUtils.h"
#include "Sim/netConnection.h"
#include "game/turret.h"

//--------------------------------------------------------------------------
// ShieldImpact
//--------------------------------------------------------------------------
ShieldImpact::ShieldImpact()
{
   mCurScale = 1.0;
   mShieldTexture = NULL;
   mOverrideTex = NULL;
   mShieldStrength = 1.0;
   mLifetime = 0.3;
   mStartScale = 1.0;
   mEndScale = 1.1;
   mElapsedTime = mLifetime;
}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
ShieldImpact::~ShieldImpact()
{
}

//--------------------------------------------------------------------------
// Init
//--------------------------------------------------------------------------
void ShieldImpact::init()
{
   // Ack - hardcode the shield map
   mShieldTexture = TextureHandle("special/shieldenvmap", MeshTexture, false);
   mOverrideTex = TextureHandle("special/whiteAlpha255", MeshTexture, false);
}

//--------------------------------------------------------------------------
// Start effect
//--------------------------------------------------------------------------
void ShieldImpact::start()
{
   mElapsedTime = 0.0;
   mCurScale = 1.0;
}

//--------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------
void ShieldImpact::renderObject(SceneState* state, SceneRenderImage*)
{
   if( !bool( mShieldedObj ) ) return;
   if( mElapsedTime >= mLifetime ) return;
   
   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   F32 percentDone = mElapsedTime / mLifetime;
   F32 alpha = (1.0 - percentDone) * (1.0 + percentDone );
   alpha *= mShieldStrength;
   if( alpha > 1.0 ) alpha = 1.0;

   // render parent object
   renderObj( mShieldedObj, state, alpha );

   // render mounted OBJECTS
   for (ShapeBase* ptr = mShieldedObj->getMountList(); ptr; ptr = ptr->getMountLink())
   {
      renderObj( ptr, state, alpha );
   }

   // render mounted IMAGES
   for( U32 i=0; i<ShapeBase::MaxMountedImages; i++ )
   {
      ShapeBase::MountedImage *image = mShieldedObj->retrieveMountedImage( i );
      if( !image ) continue;

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      
      MatrixF imageTrans;
      mShieldedObj->getRenderImageTransform( i, &imageTrans );

      dglMultMatrix( &imageTrans );

      glScalef(mShieldedObj->getScale().x * mCurScale,
               mShieldedObj->getScale().y * mCurScale,
               mShieldedObj->getScale().z * mCurScale);

      renderInstance( image->shapeInstance, state, alpha );

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
   }
   
   // restore matrices
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   // restore states
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glDepthMask(GL_TRUE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

//--------------------------------------------------------------------------
// Render shapeBase object
//--------------------------------------------------------------------------
void ShieldImpact::renderObj( ShapeBase *obj, SceneState* state, F32 alpha )
{

   // render mounted IMAGES
   for( U32 i=0; i<ShapeBase::MaxMountedImages; i++ )
   {
      ShapeBase::MountedImage *image = obj->retrieveMountedImage( i );
      if( !image ) continue;

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      
      MatrixF imageTrans;
      obj->getRenderImageTransform( i, &imageTrans );

      dglMultMatrix( &imageTrans );

      glScalef(obj->getScale().x * mCurScale,
               obj->getScale().y * mCurScale,
               obj->getScale().z * mCurScale);

      renderInstance( image->shapeInstance, state, alpha );

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
   }



   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   dglMultMatrix(&obj->getRenderTransform());

   glScalef(obj->getScale().x * mCurScale,
            obj->getScale().y * mCurScale,
            obj->getScale().z * mCurScale);

   // RENDER CODE HERE
   TSShapeInstance* pInstance = obj->getShapeInstance();
   renderInstance( pInstance, state, alpha );

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

//--------------------------------------------------------------------------
// Render shape instance
//--------------------------------------------------------------------------
void ShieldImpact::renderInstance( TSShapeInstance *pInstance, SceneState* state, F32 alpha )
{
   
   if (pInstance)
   {
      Point3F cameraOffset = mShieldedObj->getPosition();
      cameraOffset -= state->getCameraPosition();
      F32 fogAmount = state->getHazeAndFog(cameraOffset.len(),cameraOffset.z);
      pInstance->setupFog(0.0, state->getFogColor());
      pInstance->setAlphaAlways(alpha);

      glMatrixMode(GL_TEXTURE);
      glPushMatrix();
      glTranslatef( mElapsedTime, mElapsedTime, 0);
      glMatrixMode(GL_MODELVIEW);

      bool alphaIsReflect = pInstance->queryAlphaIsReflectanceMap();
      pInstance->setAlphaIsReflectanceMap( true );

      pInstance->smRenderData.renderDecals = false;
      pInstance->setOverrideTexture( mOverrideTex );
      pInstance->setEnvironmentMap( mShieldTexture );
      pInstance->setEnvironmentMapOn(true, 1.0);

      pInstance->render(0,1);

      pInstance->setAlphaAlways(1.0);
      pInstance->setEnvironmentMapOn(false, 1.0);
      pInstance->clearOverrideTexture();
      pInstance->smRenderData.renderDecals = true;

      pInstance->setAlphaIsReflectanceMap( alphaIsReflect );

      glMatrixMode(GL_TEXTURE);
      glPopMatrix();
   }


}

//--------------------------------------------------------------------------
// Advance time
//--------------------------------------------------------------------------
void ShieldImpact::update(F32 dt)
{
   if (dt == 0.0)
      return;

   mElapsedTime += dt;

   F32 percentDone = mElapsedTime / mLifetime;

   mCurScale = mStartScale + (mEndScale - mStartScale) * percentDone;

}
