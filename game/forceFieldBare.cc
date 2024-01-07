//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/forceFieldBare.h"
#include "core/bitStream.h"
#include "platformWin32/platformGL.h"
#include "dgl/dgl.h"
#include "sceneGraph/sceneState.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"
#include "collision/boxConvex.h"

IMPLEMENT_CO_DATABLOCK_V1(ForceFieldBareData);
IMPLEMENT_CO_NETOBJECT_V1(ForceFieldBare);

namespace {

static void cOpen(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ForceFieldBare*>(obj) != NULL, "Error, how did a non-forcefield get in here?");
   ForceFieldBare* pBare = static_cast<ForceFieldBare*>(obj);

   if (pBare->isClientObject())
      return;

   pBare->open();
}

static void cClose(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ForceFieldBare*>(obj) != NULL, "Error, how did a non-forcefield get in here?");
   ForceFieldBare* pBare = static_cast<ForceFieldBare*>(obj);

   if (pBare->isClientObject())
      return;

   pBare->close();
}

class ForceConvex : public BoxConvex
{
   typedef BoxConvex Parent;
   friend class ForceFieldBare;

   ForceFieldBare* pField;

  protected:
   void getPolyList(AbstractPolyList* list);
};

struct Face {
   S32 vertex[4];
   S32 axis;
   bool flip;
} sFace[] =
{
   { 0,4,5,1, 1,true  },
   { 0,2,6,4, 0,true  },
   { 3,7,6,2, 1,false },
   { 3,1,5,7, 0,false },
   { 0,1,3,2, 2,true  },
   { 4,6,7,5, 2,false },
};
void ForceConvex::getPolyList(AbstractPolyList* list)
{
   list->setTransform(&getTransform(), getScale());
   list->setObject(getObject());

   U32 base = list->addPoint(Point3F(0, 0, 0));
              list->addPoint(Point3F(1, 0, 0));
              list->addPoint(Point3F(0, 1, 0));
              list->addPoint(Point3F(1, 1, 0));
              list->addPoint(Point3F(0, 0, 1));
              list->addPoint(Point3F(1, 0, 1));
              list->addPoint(Point3F(0, 1, 1));
              list->addPoint(Point3F(1, 1, 1));

   for (U32 i = 0; i < 6; i++) {
      list->begin(0, i);

      list->vertex(base + sFace[i].vertex[0]);
      list->vertex(base + sFace[i].vertex[1]);
      list->vertex(base + sFace[i].vertex[2]);
      list->vertex(base + sFace[i].vertex[3]);

      list->plane(base + sFace[i].vertex[0],
                  base + sFace[i].vertex[1],
                  base + sFace[i].vertex[2]);
      list->end();
   }
}



} // namespace {}

//--------------------------------------------------------------------------
//--------------------------------------
//
ForceFieldBareData::ForceFieldBareData()
{
   fadeMS = 1000;
   baseTranslucency = 0.65;
   powerOffTranslucency = 0.35;
   teamPermiable  = false;
   otherPermiable = false;
   framesPerSec = 10;
   numFrames = NUM_TEX;
   scrollSpeed = 0.5;
   umapping = 1.0;
   vmapping = 1.0;
   
   color.set(0.5, 0.5, 1);
   powerOffColor.set(0.25, 0, 0);

   dMemset( textureName, 0, sizeof( textureName ) );
   dMemset( textureHandle, 0, sizeof( textureHandle ) );
}

ForceFieldBareData::~ForceFieldBareData()
{

}


//--------------------------------------------------------------------------
void ForceFieldBareData::initPersistFields()
{
   Parent::initPersistFields();

   addField("fadeMS",               TypeS32,    Offset(fadeMS,               ForceFieldBareData));
   addField("baseTranslucency",     TypeF32,    Offset(baseTranslucency,     ForceFieldBareData));
   addField("powerOffTranslucency", TypeF32,    Offset(powerOffTranslucency, ForceFieldBareData));
   addField("teamPermiable",        TypeBool,   Offset(teamPermiable,        ForceFieldBareData));
   addField("otherPermiable",       TypeBool,   Offset(otherPermiable,       ForceFieldBareData));
   addField("color",                TypeColorF, Offset(color,                ForceFieldBareData));
   addField("powerOffColor",        TypeColorF, Offset(powerOffColor,        ForceFieldBareData));
   addField("texture",              TypeString, Offset(textureName,          ForceFieldBareData), NUM_TEX);
   addField("framesPerSec",         TypeS32,    Offset(framesPerSec,         ForceFieldBareData));
   addField("numFrames",            TypeS32,    Offset(numFrames,            ForceFieldBareData));
   addField("scrollSpeed",          TypeF32,    Offset(scrollSpeed,          ForceFieldBareData));
   addField("umapping",             TypeF32,    Offset(umapping,             ForceFieldBareData));
   addField("vmapping",             TypeF32,    Offset(vmapping,             ForceFieldBareData));
}


//--------------------------------------------------------------------------
bool ForceFieldBareData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (fadeMS < 0 || fadeMS > 10000) {
      Con::warnf(ConsoleLogEntry::General, "ForceFieldBareData(%s)::onAdd: fadeMS must be in the range [0, 10000]", getName());
      fadeMS = fadeMS < 0 ? 0 : 10000;
   }
   if (baseTranslucency < 0.0f || baseTranslucency > 1.0f) {
      Con::warnf(ConsoleLogEntry::General, "ForceFieldBareData(%s)::onAdd: baseTranslucency must be in the range [0, 1]", getName());
      baseTranslucency = baseTranslucency < 0 ? 0 : 1;
   }

   return true;
}

//--------------------------------------------------------------------------
// Preload data - load resources
//--------------------------------------------------------------------------
bool ForceFieldBareData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (!server)
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
void ForceFieldBareData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(fadeMS);
   stream->write(baseTranslucency);
   stream->write(powerOffTranslucency);
   stream->writeFlag(teamPermiable);
   stream->writeFlag(otherPermiable);
   stream->write(color);
   stream->write(powerOffColor);
   stream->write(framesPerSec);
   stream->write(numFrames);
   stream->write(scrollSpeed);
   stream->write(umapping);
   stream->write(vmapping);

   U32 i;
   for( i=0; i<NUM_TEX; i++ )
   {
      stream->writeString(textureName[i]);
   }
}

void ForceFieldBareData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&fadeMS);
   stream->read(&baseTranslucency);
   stream->read(&powerOffTranslucency);
   teamPermiable = stream->readFlag();
   otherPermiable = stream->readFlag();
   stream->read(&color);
   stream->read(&powerOffColor);
   stream->read(&framesPerSec);
   stream->read(&numFrames);
   stream->read(&scrollSpeed);
   stream->read(&umapping);
   stream->read(&vmapping);

   for( U32 i=0; i<NUM_TEX; i++ )
   {
      textureName[i] = stream->readSTString();
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
ForceFieldBare::ForceFieldBare()
{
   mNetFlags.set(Ghostable);

   mTypeMask |= ForceFieldObjectType;

   mCurrState          = Closed;
   mCurrPosition       = 0;

   mClientLastPosition = 0;
   mCurrBackDelta      = 0.0;
   mCurrFade           = 1.0;

   mDataBlock = NULL;

   mConvexList = new Convex;

   mElapsedTime = 0.0;
}

ForceFieldBare::~ForceFieldBare()
{
   delete mConvexList;
   mConvexList = NULL;
}

//--------------------------------------------------------------------------
void ForceFieldBare::initPersistFields()
{
   Parent::initPersistFields();
}


void ForceFieldBare::consoleInit()
{
   Con::addCommand("ForceFieldBare", "open",  cOpen,  "obj.open()", 2, 2);
   Con::addCommand("ForceFieldBare", "close", cClose, "obj.close()", 2, 2);
}


//--------------------------------------------------------------------------
bool ForceFieldBare::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   mObjBox.min.set(0, 0, 0);
   mObjBox.max.set(1, 1, 1);
   resetWorldBox();

   addToScene();
   scriptOnAdd();

   return true;
}


void ForceFieldBare::onRemove()
{
   scriptOnRemove();
   mConvexList->nukeList();
   removeFromScene();

   Parent::onRemove();
}


bool ForceFieldBare::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<ForceFieldBareData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------
void ForceFieldBare::open()
{
   AssertFatal(isServerObject(), "Client objects not allowed in ForceFieldInstance::open()");

   if (mCurrState != Opening &&
       mCurrState != Open) {
      mCurrState = Opening;
      setMaskBits(StateChangeMask);
   }
}

void ForceFieldBare::close()
{
   AssertFatal(isServerObject(), "Client objects not allowed in ForceFieldInstance::close()");

   if (mCurrState != Closing &&
       mCurrState != Closed) {
      mCurrState = Closing;
      setMaskBits(StateChangeMask);
   }
}

void ForceFieldBare::setClientState(const State newState, const U32 newPosition)
{
   // DMMNOTE: This is not the best way to interpolate, but it'll do for now
   mCurrState    = newState;
   mCurrPosition = newPosition;
}

void ForceFieldBare::setImagePoly(SceneRenderImage* image)
{
   if (mObjScale.x <= mObjScale.y && mObjScale.x <= mObjScale.z)
   {
      // X is smallest (or all are equal)
      image->poly[0].set((mObjBox.min.x + mObjBox.max.x) * 0.5, mObjBox.min.y, mObjBox.min.z);
      image->poly[1].set((mObjBox.min.x + mObjBox.max.x) * 0.5, mObjBox.min.y, mObjBox.max.z);
      image->poly[2].set((mObjBox.min.x + mObjBox.max.x) * 0.5, mObjBox.max.y, mObjBox.max.z);
      image->poly[3].set((mObjBox.min.x + mObjBox.max.x) * 0.5, mObjBox.max.y, mObjBox.min.z);
   }
   else if (mObjScale.y <= mObjScale.x && mObjScale.y <= mObjScale.z)
   {
      // Y is smallest
      image->poly[0].set(mObjBox.min.x, (mObjBox.min.y + mObjBox.max.y) * 0.5, mObjBox.min.z);
      image->poly[1].set(mObjBox.min.x, (mObjBox.min.y + mObjBox.max.y) * 0.5, mObjBox.max.z);
      image->poly[2].set(mObjBox.max.x, (mObjBox.min.y + mObjBox.max.y) * 0.5, mObjBox.max.z);
      image->poly[3].set(mObjBox.max.x, (mObjBox.min.y + mObjBox.max.y) * 0.5, mObjBox.min.z);
   }
   else
   {
      // Z is smallest
      image->poly[0].set(mObjBox.min.x, mObjBox.min.y, (mObjBox.min.z + mObjBox.max.z) * 0.5);
      image->poly[1].set(mObjBox.min.x, mObjBox.max.y, (mObjBox.min.z + mObjBox.max.z) * 0.5);
      image->poly[2].set(mObjBox.max.x, mObjBox.max.y, (mObjBox.min.z + mObjBox.max.z) * 0.5);
      image->poly[3].set(mObjBox.max.x, mObjBox.min.y, (mObjBox.min.z + mObjBox.max.z) * 0.5);
   }
   for (U32 i = 0; i < 4; i++)
   {
      image->poly[i].convolve(mObjScale);
      getTransform().mulP(image->poly[i]);
   }
   image->plane.set(image->poly[0], image->poly[1], image->poly[2]);
   // Calc the area of this poly
   Point3F intermed;
   mCross(image->poly[2] - image->poly[0], image->poly[3] - image->poly[1], &intermed);
   image->polyArea = intermed.len() * 0.5;
}

//--------------------------------------------------------------------------
bool ForceFieldBare::prepRenderImage(SceneState* state, const U32 stateKey,
                                     const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      
      // Since force fields can be fairly large and planar, we need to do a bit more
      //  heavy lifting here to determine what the proper point to calculate the sort key
      //  is.  In essence, it's the closest point on the box to the camera.
      // Transform camera point to object space...
      Point3F camObj = state->getCameraPosition();
      getWorldTransform().mulP(camObj);
      camObj.convolveInverse(getScale());

      image->isTranslucent = true;
      image->sortType = SceneRenderImage::Plane;
      setImagePoly(image);
      state->insertRenderImage(image);
   }

   return false;
}


void ForceFieldBare::renderObject(SceneState* state, SceneRenderImage*)
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

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);


   Point3F cameraOffset;
   getRenderTransform().getColumn(3,&cameraOffset);
   cameraOffset -= state->getCameraPosition();
   F32 dist = cameraOffset.len();
   F32 fogAmount = state->getHazeAndFog(dist,cameraOffset.z);

   ColorF fieldColor;
   fieldColor.red   = (mDataBlock->color.red   * mCurrFade) + (mDataBlock->powerOffColor.red   * (1.0f - mCurrFade));
   fieldColor.green = (mDataBlock->color.green * mCurrFade) + (mDataBlock->powerOffColor.green * (1.0f - mCurrFade));
   fieldColor.blue  = (mDataBlock->color.blue  * mCurrFade) + (mDataBlock->powerOffColor.blue  * (1.0f - mCurrFade));
   fieldColor.alpha = (mDataBlock->baseTranslucency * mCurrFade) + (mDataBlock->powerOffTranslucency * (1.0f - mCurrFade));

   ColorF fogColor = state->getFogColor();
   fogColor.alpha = 0.0;
   fieldColor.interpolate( fieldColor, fogColor, fogAmount );

   glColor4fv( fieldColor );   

   U32 texNum  = mElapsedTime * mDataBlock->framesPerSec;
   texNum      %= mDataBlock->numFrames;
             
   glBindTexture(GL_TEXTURE_2D, mDataBlock->textureHandle[texNum].getGLName());
   glEnable( GL_TEXTURE_2D );
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   F32 umap = mObjScale.x * mDataBlock->umapping;
   F32 vmap = mObjScale.y * mDataBlock->vmapping;
   F32 vScroll = mElapsedTime * mDataBlock->scrollSpeed;

   glBegin(GL_TRIANGLE_FAN);
   {
      glTexCoord2f(0, vScroll);
      glVertex3f(0, 0, 0);

      glTexCoord2f(0, vmap+vScroll);
      glVertex3f(0, 1, 0);

      glTexCoord2f(umap, vmap+vScroll);
      glVertex3f(1, 1, 0);

      glTexCoord2f(umap, vScroll);
      glVertex3f(1, 0, 0);
   }
   glEnd();

   glBegin(GL_TRIANGLE_FAN);
   {
      glTexCoord2f(0, vScroll);
      glVertex3f(0, 0, 1);

      glTexCoord2f(0, vmap+vScroll);
      glVertex3f(0, 1, 1);

      glTexCoord2f(umap, vmap+vScroll);
      glVertex3f(1, 1, 1);

      glTexCoord2f(umap, vScroll);
      glVertex3f(1, 0, 1);
   }
   glEnd();

   umap = mObjScale.y * mDataBlock->umapping;
   vmap = mObjScale.z * mDataBlock->vmapping;

   glBegin(GL_TRIANGLE_FAN);
   {
      glTexCoord2f(0, vScroll);
      glVertex3f(1, 0, 0);

      glTexCoord2f(0, vmap+vScroll);
      glVertex3f(1, 0, 1);

      glTexCoord2f(umap, vmap+vScroll);
      glVertex3f(1, 1, 1);

      glTexCoord2f(umap, vScroll);
      glVertex3f(1, 1, 0);
   }
   glEnd();

   glBegin(GL_TRIANGLE_FAN);
   {
      glTexCoord2f(0, vScroll);
      glVertex3f(0, 0, 0);

      glTexCoord2f(0, vmap+vScroll);
      glVertex3f(0, 0, 1);

      glTexCoord2f(umap, vmap+vScroll);
      glVertex3f(0, 1, 1);

      glTexCoord2f(umap, vScroll);
      glVertex3f(0, 1, 0);
   }
   glEnd();

   umap = mObjScale.x * mDataBlock->umapping;

   glBegin(GL_TRIANGLE_FAN);
   {
      glTexCoord2f(0, vScroll);
      glVertex3f(0, 1, 0);

      glTexCoord2f(0, vmap+vScroll);
      glVertex3f(0, 1, 1);

      glTexCoord2f(umap, vmap+vScroll);
      glVertex3f(1, 1, 1);

      glTexCoord2f(umap, vScroll);
      glVertex3f(1, 1, 0);
   }
   glEnd();

   glBegin(GL_TRIANGLE_FAN);
   {
      glTexCoord2f(0, vScroll);
      glVertex3f(0, 0, 0);

      glTexCoord2f(0, vmap+vScroll);
      glVertex3f(0, 0, 1);

      glTexCoord2f(umap, vmap+vScroll);
      glVertex3f(1, 0, 1);

      glTexCoord2f(umap, vScroll);
      glVertex3f(1, 0, 0);
   }
   glEnd();

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glDisable( GL_TEXTURE_2D );
   glDepthMask(GL_TRUE);
   glDisable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ZERO);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);
   
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


void ForceFieldBare::setTransform(const MatrixF& xform)
{
   Parent::setTransform(xform);

   if (isServerObject())
      setMaskBits(TransformMask);
}

void ForceFieldBare::setScale(const VectorF & scale)
{
   Parent::setScale(scale);

   if (isServerObject())
      setMaskBits(TransformMask);
}


//--------------------------------------------------------------------------
bool ForceFieldBare::buildPolyList(AbstractPolyList* list, const Box3F&, const SphereF&)
{
   list->setTransform(&mObjToWorld, mObjScale);
   list->setObject(this);
   list->addBox(mObjBox);
   return true;
}


bool ForceFieldBare::castRay(const Point3F& s, const Point3F& e, RayInfo* info)
{
   if (mCurrState != Open && mObjBox.collideLine(s, e, &info->t, &info->normal)) {
      PlaneF fakePlane;
      fakePlane.x = info->normal.x;
      fakePlane.y = info->normal.y;
      fakePlane.z = info->normal.z;
      fakePlane.d = 0;

      PlaneF result;
      mTransformPlane(getTransform(), getScale(), fakePlane, &result);
      info->normal = result;

      info->object = this;
      return true;
   } else {
      return false;
   }
}


//--------------------------------------------------------------------------
void ForceFieldBare::processClientTick()
{
   mClientLastPosition = mCurrPosition;
   mCurrBackDelta = 0.0;

   if (mCurrState == Open ||
       mCurrState == Closed) {
      // Nothing to do
      return;
   }

   if (mCurrState == Opening) {
      mCurrPosition += TickMs;
      if (mCurrPosition >= mDataBlock->fadeMS) {
         mCurrState    = Open;
         mCurrPosition = mDataBlock->fadeMS;
      }
   } else if (mCurrState == Closing) {
      mCurrPosition -= TickMs;
      if (mCurrPosition <= 0) {
         mCurrState    = Closed;
         mCurrPosition = 0;
      }
   } else {
      Con::errorf(ConsoleLogEntry::General, "Bad client state on forcefield!  Setting to closed");
      mCurrState     = Closed;
      mCurrPosition  = 0;
   }
}

void ForceFieldBare::processServerTick()
{
   if (mCurrState == Open ||
       mCurrState == Closed) {
      // Nothing to do
      return;
   }

   if (mCurrState == Opening) {
      mCurrPosition += TickMs;
      if (mCurrPosition >= mDataBlock->fadeMS) {
         mCurrState    = Open;
         mCurrPosition = mDataBlock->fadeMS;
      }
   } else if (mCurrState == Closing) {
      mCurrPosition -= TickMs;
      if (mCurrPosition <= 0) {
         mCurrState    = Closed;
         mCurrPosition = 0;
      }
   } else {
      Con::errorf(ConsoleLogEntry::General, "Bad server state on forcefield!  Setting to closed");
      mCurrState     = Closed;
      mCurrPosition  = 0;
      setMaskBits(GameBase::InitialUpdateMask);
   }
}

void ForceFieldBare::processTick(const Move*)
{
   if (isServerObject())
      processServerTick();
   else
      processClientTick();
}

void ForceFieldBare::interpolateTick(F32 backDelta)
{
   mCurrBackDelta = backDelta;

   F32 currPos = F32(mClientLastPosition) + ((mCurrPosition - mClientLastPosition) * backDelta);
   mCurrFade   = 1.0f - (currPos / F32(mDataBlock->fadeMS));
   mCurrFade   = mCurrFade < 0.0 ? 0.0 : mCurrFade;
}


//--------------------------------------------------------------------------
U32 ForceFieldBare::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag((mask & GameBase::InitialUpdateMask) != 0)) {

      stream->writeAffineTransform(mObjToWorld);
      mathWrite(*stream, mObjScale);

   } else {
      if (stream->writeFlag((mask & TransformMask) != 0)) {
         stream->writeAffineTransform(mObjToWorld);
         mathWrite(*stream, mObjScale);
      }
   }

   if (stream->writeFlag((mask & StateChangeMask) != 0)) {
      stream->writeInt(mCurrState, StateBitsRequired);

      // DMMTODO: Optimize based on fadeMS.  Should recover 21 bits or so in general...
      if (mCurrState == Opening || mCurrState == Closing)
         stream->write(mCurrPosition);
   }


   return retMask;
}

void ForceFieldBare::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   MatrixF tempXForm;
   Point3F tempScale;

   if (stream->readFlag()) {
      // Initial update...
      //
      stream->readAffineTransform(&tempXForm);
      mathRead(*stream, &tempScale);

      setScale(tempScale);
      setTransform(tempXForm);

   } else {
      if (stream->readFlag()) {
         // Transform update...
         //
         stream->readAffineTransform(&tempXForm);
         mathRead(*stream, &tempScale);

         setScale(tempScale);
         setTransform(tempXForm);
      }
   }

   if (stream->readFlag()) {
      // State update...

      State newState;
      U32 newPosition;
      newState = (State)stream->readInt(StateBitsRequired);

      // DMMTODO: Optimize based on fadeMS.  Should recover 21 bits or so in general...
      if (newState == Closed)
         newPosition = 0;
      else if (newState == Open)
         newPosition = mDataBlock->fadeMS;
      else
         stream->read(&newPosition);

      setClientState(newState, newPosition);
   }

}


void ForceFieldBare::buildConvex(const Box3F& box, Convex* convex)
{
   // These should really come out of a pool
   mConvexList->collectGarbage();

   if (box.isOverlapped(getWorldBox()) == false)
      return;

   // Just return a box convex for the entire shape...
   Convex* cc = 0;
   CollisionWorkingList& wl = convex->getWorkingList();
   for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
      if (itr->mConvex->getType() == BoxConvexType &&
          itr->mConvex->getObject() == this) {
         cc = itr->mConvex;
         break;
      }
   }
   if (cc)
      return;

   // Create a new convex.
   ForceConvex* cp = new ForceConvex;
   mConvexList->registerObject(cp);
   convex->addToWorkingList(cp);
   cp->init(this);
   cp->pField = this;

   cp->mCenter = Point3F(0.5, 0.5, 0.5);
   cp->mSize = getScale() * 0.5;
}

bool ForceFieldBare::isPermiableTo(GameBase* pass)
{
   // Vehicles never pass through forcefields, even when unpowered.
   if (pass->getType() & VehicleObjectType)
      return false;
   
   if (isOpen() ||
       (mDataBlock->otherPermiable && getSensorGroup() != pass->getSensorGroup()) ||
       (mDataBlock->teamPermiable && getSensorGroup() == pass->getSensorGroup())) {
      return true;
   }

   return false;
}

bool ForceFieldBare::isTeamControlled()
{
   return !mDataBlock->otherPermiable;
}

void ForceFieldBare::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   mElapsedTime += dt;
}
