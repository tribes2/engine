//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "interior/mirrorSubObject.h"
#include "interior/interiorInstance.h"
#include "interior/interior.h"
#include "dgl/materialList.h"
#include "Core/stream.h"
#include "PlatformWin32/platformGL.h"
#include "dgl/dgl.h"
#include "sceneGraph/sgUtil.h"

IMPLEMENT_CONOBJECT(MirrorSubObject);

//--------------------------------------------------------------------------
MirrorSubObject::MirrorSubObject()
{
   mTypeMask = StaticObjectType;

   mInitialized = false;
   mWhite       = NULL;
}

MirrorSubObject::~MirrorSubObject()
{
   delete mWhite;
   mWhite = NULL;
}

//--------------------------------------------------------------------------
void MirrorSubObject::initPersistFields()
{
   Parent::initPersistFields();

   //
}

//--------------------------------------------------------------------------
void MirrorSubObject::renderObject(SceneState* state, SceneRenderImage* image)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   SubObjectRenderImage* sori = static_cast<SubObjectRenderImage*>(image);

   if (mZone == 0) {
      state->setupZoneProjection(getInstance()->getCurrZone(0));
   } else {
      state->setupZoneProjection(mZone + getInstance()->getZoneRangeStart() - 1);
   }

   RectI viewport;
   dglGetViewport(&viewport);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   dglMultMatrix(&getSOTransform());
   glScalef(getSOScale().x, getSOScale().y, getSOScale().z);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_TEXTURE_2D);
   glEnable(GL_TEXTURE_GEN_S); glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
   glEnable(GL_TEXTURE_GEN_T); glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
   glEnableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glColor4f(1, 1, 1, mAlphaLevel);

   Interior* interior = getInstance()->getDetailLevel(sori->mDetailLevel);
   glVertexPointer(3, GL_FLOAT, sizeof(ItrPaddedPoint), interior->mPoints.address());
   
   glBindTexture(GL_TEXTURE_2D, interior->mMaterialList->getMaterial(interior->mSurfaces[surfaceStart].textureIndex).getGLName());
   glTexGenfv(GL_S, GL_OBJECT_PLANE, (GLfloat*)interior->mTexGenEQs[interior->mSurfaces[surfaceStart].texGenIndex].planeX);
   glTexGenfv(GL_T, GL_OBJECT_PLANE, (GLfloat*)interior->mTexGenEQs[interior->mSurfaces[surfaceStart].texGenIndex].planeY);

   for (U32 i = 0; i < surfaceCount; i++) {
      glDrawElements(GL_TRIANGLE_STRIP,
                     interior->mSurfaces[surfaceStart+i].windingCount,
                     GL_UNSIGNED_INT,
                     &interior->mWindings[interior->mSurfaces[surfaceStart+i].windingStart]);
   }

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisable(GL_TEXTURE_GEN_S);
   glDisable(GL_TEXTURE_GEN_T);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ZERO);

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   dglSetViewport(viewport);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   dglSetCanonicalState();
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//--------------------------------------------------------------------------
void MirrorSubObject::transformModelview(const U32 portalIndex, const MatrixF& oldMV, MatrixF* pNewMV)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, we only have one portal!");

   *pNewMV = oldMV;
   pNewMV->mul(mReflectionMatrix);
}


//--------------------------------------------------------------------------
void MirrorSubObject::transformPosition(const U32 portalIndex, Point3F& ioPosition)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, we only have one portal!");

   mReflectionMatrix.mulP(ioPosition);
}


//--------------------------------------------------------------------------
bool MirrorSubObject::computeNewFrustum(const U32      portalIndex,
                                        const F64*     oldFrustum,
                                        const F64      nearPlane,
                                        const F64      farPlane,
                                        const RectI&   oldViewport,
                                        F64*           newFrustum,
                                        RectI&         newViewport,
                                        const bool     flippedMatrix)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, mirrortests only have one portal!");

   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   
   static Vector<SGWinding> mirrorWindings;
   mirrorWindings.setSize(surfaceCount);

   for (U32 i = 0; i < surfaceCount; i++) {
      SGWinding& rSGWinding             = mirrorWindings[i];
      const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart + i];

      U32 fanIndices[32];
      U32 numFanIndices = 0;
      interior->collisionFanFromSurface(rSurface, fanIndices, &numFanIndices);

      for (U32 j = 0; j < numFanIndices; j++)
         rSGWinding.points[j] = interior->mPoints[fanIndices[j]].point;
      rSGWinding.numPoints = numFanIndices;
   }

   MatrixF finalModelView;
   dglGetModelview(&finalModelView);
   finalModelView.mul(getSOTransform());
   finalModelView.scale(getSOScale());

   return sgComputeNewFrustum(oldFrustum, nearPlane, farPlane,
                              oldViewport,
                              mirrorWindings.address(), mirrorWindings.size(),
                              finalModelView,
                              newFrustum, newViewport,
                              flippedMatrix);
}


//--------------------------------------------------------------------------
void MirrorSubObject::openPortal(const U32   portalIndex,
                                 SceneState* pCurrState,
                                 SceneState* pParentState)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, mirrortests only have one portal!");

   RectI viewport;
   dglGetViewport(&viewport);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   const SceneState::ZoneState& baseState = pCurrState->getBaseZoneState();
   dglSetViewport(baseState.viewport);

   glDisable(GL_TEXTURE_2D);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ZERO);
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_ALWAYS);

   glColor3f(1, 0, 0);
   glBegin(GL_TRIANGLE_FAN);
      glVertex3f(-1, -1, 0);
      glVertex3f(-1,  1, 0);
      glVertex3f( 1,  1, 0);
      glVertex3f( 1, -1, 0);
   glEnd();

   // This is fairly tricky-tricky, so here's what's going on.  We need this poly
   //  to be at the far plane, but have it's rasterization coords be exactly that of
   //  the final poly.  So we copy the w-coord post projection into the z coord.  this
   //  ensures that after the w divide, we have the z coord == 1.  This would screw
   //  up texturing, but we don't really care, do we?

   if (mZone == 0)
      pParentState->setupZoneProjection(getInstance()->getCurrZone(0));
   else
      pParentState->setupZoneProjection(mZone + getInstance()->getZoneRangeStart() - 1);

   MatrixF finalProj(true);
   MatrixF currProj;
   dglGetProjection(&currProj);
   ((F32*)finalProj)[10] = 0.0f;
   ((F32*)finalProj)[11] = 0.9999f;
   finalProj.mul(currProj);
   glMatrixMode(GL_PROJECTION);
   dglLoadMatrix(&finalProj);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glPushMatrix();
   dglMultMatrix(&getSOTransform());
   glScalef(getSOScale().x, getSOScale().y, getSOScale().z);

   glColor3f(0, 1, 0);
   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(ItrPaddedPoint), interior->mPoints.address());

   for (U32 i = 0; i < surfaceCount; i++) {
      const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart + i];

      glBegin(GL_TRIANGLE_STRIP);
      for (U32 j = 0; j < rSurface.windingCount; j++) {
         const Point3F& rPoint = interior->mPoints[interior->mWindings[rSurface.windingStart + j]].point;
         glVertex3fv(rPoint);
      }
      glEnd();
   }

   glDisableClientState(GL_VERTEX_ARRAY);

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   dglSetViewport(viewport);

   glDepthFunc(GL_LEQUAL);
   dglSetCanonicalState();
}


//--------------------------------------------------------------------------
void MirrorSubObject::closePortal(const U32   portalIndex,
                                  SceneState* pCurrState,
                                  SceneState* pParentState)
{
   AssertFatal(isInitialized() == true, "Error, we should have been initialized by this point!");
   AssertFatal(portalIndex == 0, "Error, mirrortests only have one portal!");

   RectI viewport;
   dglGetViewport(&viewport);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   const SceneState::ZoneState& baseState = pCurrState->getBaseZoneState();
   dglSetViewport(baseState.viewport);

   glDisable(GL_TEXTURE_2D);
   glEnable(GL_BLEND);
   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_ALWAYS);

   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
   glColor3f(0, 0, 1);
   glBegin(GL_TRIANGLE_FAN);
      glVertex3f(-1, -1, 1);
      glVertex3f(-1,  1, 1);
      glVertex3f( 1,  1, 1);
      glVertex3f( 1, -1, 1);
   glEnd();
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

   if (mZone == 0) {
      pParentState->setupZoneProjection(getInstance()->getCurrZone(0));
   } else {
      pParentState->setupZoneProjection(mZone + getInstance()->getZoneRangeStart() - 1);
   }

   glMatrixMode(GL_MODELVIEW);
   dglLoadMatrix(&pParentState->mModelview);
   dglMultMatrix(&getSOTransform());
   glScalef(getSOScale().x, getSOScale().y, getSOScale().z);

   // Need to have texturing turned on because of lame LSB z buffer errors
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, mWhite->getGLName());
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_CULL_FACE);

   glColor3f(1, 0, 1);
   glEnableClientState(GL_VERTEX_ARRAY);

   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   glVertexPointer(3, GL_FLOAT, sizeof(ItrPaddedPoint), interior->mPoints.address());
   for (U32 i = 0; i < surfaceCount; i++) {
      glDrawElements(GL_TRIANGLE_STRIP,
                     interior->mSurfaces[surfaceStart+i].windingCount,
                     GL_UNSIGNED_INT,
                     &interior->mWindings[interior->mSurfaces[surfaceStart+i].windingStart]);
   }
   glDisableClientState(GL_VERTEX_ARRAY);

   glDisable(GL_TEXTURE_2D);
   glBlendFunc(GL_ONE, GL_ZERO);

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   dglSetViewport(viewport);

   glDepthFunc(GL_LEQUAL);
   dglSetCanonicalState();
}


//--------------------------------------------------------------------------
void MirrorSubObject::getWSPortalPlane(const U32 portalIndex, PlaneF* pPlane)
{
   AssertFatal(portalIndex == 0, "Error, mirrortests only have one portal!");

   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart];

   PlaneF temp = interior->getPlane(rSurface.planeIndex);
   if (Interior::planeIsFlipped(rSurface.planeIndex))
      temp.neg();

   mTransformPlane(getSOTransform(), getSOScale(), temp, pPlane);
}


//--------------------------------------------------------------------------
U32 MirrorSubObject::getSubObjectKey() const
{
   return InteriorSubObject::MirrorSubObjectKey;
}


bool MirrorSubObject::_readISO(Stream& stream)
{
   AssertFatal(isInitialized() == false, "Error, should not be initialized here!");

   if (Parent::_readISO(stream) == false)
      return false;

   stream.read(&mDetailLevel);
   stream.read(&mZone);
   stream.read(&mAlphaLevel);
   stream.read(&surfaceCount);
   stream.read(&surfaceStart);

   stream.read(&mCentroid.x);
   stream.read(&mCentroid.y);
   stream.read(&mCentroid.z);

   return true;
}


bool MirrorSubObject::_writeISO(Stream& stream) const
{
   if (Parent::_writeISO(stream) == false)
      return false;

   stream.write(mDetailLevel);
   stream.write(mZone);
   stream.write(mAlphaLevel);
   stream.write(surfaceCount);
   stream.write(surfaceStart);

   stream.write(mCentroid.x);
   stream.write(mCentroid.y);
   stream.write(mCentroid.z);

   return true;
}


SubObjectRenderImage* MirrorSubObject::getRenderImage(SceneState*    state,
                                                      const Point3F& osPoint)
{
   if (isInitialized() == false)
      setupTransforms();


   // Check to make sure that we're on the right side of the plane...
   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart];

   PlaneF plane = interior->getPlane(rSurface.planeIndex);
   if (Interior::planeIsFlipped(rSurface.planeIndex))
      plane.neg();

   if (plane.whichSide(osPoint) != PlaneF::Front)
      return NULL;

   // On the right side, guess we have to return an image and a portal...
   //
   SubObjectRenderImage* ri = new SubObjectRenderImage;

   ri->obj           = this;
   ri->isTranslucent = false;

   U32 realZone;
   if (getInstance()->getZoneRangeStart() == 0xFFFFFFFF || mZone == 0) {
      realZone = getInstance()->getCurrZone(0);
   } else {
      realZone = getInstance()->getZoneRangeStart() + mZone - 1;
   }

   // Create the WS start point.  this will be the centroid of the first poly in os space,
   //  transformed out for the sceneGraph, with a smidge of our normal added in to pull
   //  it off the surface plane...

   Point3F startPoint = mCentroid;
   PlaneF temp = interior->getPlane(rSurface.planeIndex);
   if (Interior::planeIsFlipped(rSurface.planeIndex))
      temp.neg();
   startPoint += Point3F(temp.x, temp.y, temp.z) * 0.01f;
   getSOTransform().mulP(mCentroid);
   startPoint.convolve(getSOScale());

   state->insertTransformPortal(this, 0, realZone, startPoint, true);

   return ri;
}


bool MirrorSubObject::renderDetailDependant() const
{
   return true;
}


U32 MirrorSubObject::getZone() const
{
   return mZone;
}


void MirrorSubObject::setupTransforms()
{
   mInitialized = true;

   // This is really bad, but it's just about the only good place for this...
   if (getInstance()->isClientObject() && mWhite == NULL)
      mWhite = new TextureHandle("special/whiteAlpha0", MeshTexture);

   Interior* interior = getInstance()->getDetailLevel(mDetailLevel);
   const Interior::Surface& rSurface = interior->mSurfaces[surfaceStart];

   PlaneF plane = interior->getPlane(rSurface.planeIndex);
   if (Interior::planeIsFlipped(rSurface.planeIndex))
      plane.neg();

   Point3F n(plane.x, plane.y, plane.z);
   Point3F q = n;
   q *= -plane.d;

   MatrixF t(true);
   t.scale(getSOScale());
   t.mul(getSOTransform());

   t.mulV(n);
   t.mulP(q);

   F32* ra = mReflectionMatrix;

   ra[0]  = 1.0f - 2.0f*(n.x*n.x); ra[1]  = 0.0f - 2.0f*(n.x*n.y); ra[2]  = 0.0f - 2.0f*(n.x*n.z); ra[3]  = 0.0f;
   ra[4]  = 0.0f - 2.0f*(n.y*n.x); ra[5]  = 1.0f - 2.0f*(n.y*n.y); ra[6]  = 0.0f - 2.0f*(n.y*n.z); ra[7]  = 0.0f;
   ra[8]  = 0.0f - 2.0f*(n.z*n.x); ra[9]  = 0.0f - 2.0f*(n.z*n.y); ra[10] = 1.0f - 2.0f*(n.z*n.z); ra[11] = 0.0f;

   Point3F qnn = n * mDot(n, q);

   ra[12] = qnn.x * 2.0f;
   ra[13] = qnn.y * 2.0f;
   ra[14] = qnn.z * 2.0f;
   ra[15] = 1.0f;

   // Now, the GGems series (as of v1) uses row vectors (arg)
   mReflectionMatrix.transpose();
}

void MirrorSubObject::noteTransformChange()
{
   setupTransforms();
   Parent::noteTransformChange();
}

InteriorSubObject* MirrorSubObject::clone(InteriorInstance* instance) const
{
   MirrorSubObject* pClone = new MirrorSubObject;

   pClone->mDetailLevel = mDetailLevel;
   pClone->mZone        = mZone;
   pClone->mAlphaLevel  = mAlphaLevel;
   pClone->mCentroid    = mCentroid;
   pClone->surfaceCount = surfaceCount;
   pClone->surfaceStart = surfaceStart;

   pClone->mInteriorInstance = instance;

   return pClone;
}
