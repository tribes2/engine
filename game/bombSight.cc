//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/bombSight.h"
#include "platformWin32/platformGL.h"
#include "dgl/dgl.h"
#include "sceneGraph/sceneGraph.h"
#include "sceneGraph/sceneState.h"
#include "ts/tsShapeInstance.h"

IMPLEMENT_CONOBJECT(BombSight);

//--------------------------------------------------------------------------
BombSight::BombSight()
{
   mTypeMask |= ProjectileObjectType;
   mMoveShape = NULL;
   mDtsFileName = NULL;
   mRenderObject = false;
   mAlpha = 1.0f;
}

//--------------------------------------------------------------------------
BombSight::~BombSight()
{
}

//--------------------------------------------------------------------------
bool BombSight::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   char fullName[256];
   dSprintf(fullName,sizeof(fullName),"shapes/%s", mDtsFileName);
   
   mMoveShape = ResourceManager->load(fullName);
   mShapeInstance = new TSShapeInstance(mMoveShape, true);
   mObjBox = mMoveShape->bounds;
   resetWorldBox();

   gClientContainer.addObject(this);
   gClientSceneGraph->addObjectToScene(this);

   return true;
}

//--------------------------------------------------------------------------
void BombSight::onRemove()
{
   gClientContainer.removeObject(this);
   mSceneManager->removeObjectFromScene(this);

   Parent::onRemove();
}

//--------------------------------------------------------------------------
bool BombSight::prepRenderImage(SceneState* state, const U32 stateKey,
                                       const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey) || !mRenderObject)
      return false;
   setLastState(state, stateKey);

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->isTranslucent = true;
      image->sortType = SceneRenderImage::EndSort;
      state->insertRenderImage(image);
   }

   return false;
}

//--------------------------------------------------------------------------
void BombSight::renderObject(SceneState* state, SceneRenderImage *)
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
   dglMultMatrix(&mObjToWorld);
   glScalef(mObjScale.x, mObjScale.y, mObjScale.z);

   // RENDER CODE HERE
   glDisable(GL_DEPTH_TEST);
   mShapeInstance->setAlphaAlways(mAlpha);
   mShapeInstance->render();
   glEnable(GL_DEPTH_TEST);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   dglSetCanonicalState();
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

void BombSight::setDtsShape(StringTableEntry dtsName)
{
   mDtsFileName = dtsName; 
}

void BombSight::setRender(const bool renderObj, const F32 alpha)
{
   mRenderObject = renderObj; 
   mAlpha = alpha;
}
