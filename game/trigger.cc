//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "sceneGraph/sceneState.h"
#include "PlatformWin32/platformGL.h"
#include "dgl/dgl.h"
#include "dgl/gTexManager.h"
#include "console/consoleTypes.h"
#include "Collision/boxConvex.h"

#include "Core/bitStream.h"
#include "Math/mathIO.h"

#include "game/trigger.h"

namespace {

void cTriggerEnterTrigger(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<TriggerData*>(obj) != NULL, "Error, how did a non-trigger get here?");
   TriggerData* triggerData = static_cast<TriggerData*>(obj);

   Trigger* trigger = NULL;
   if (Sim::findObject(argv[2], trigger) == false)
      return;

   // Do nothing with the trigger object id by default...
   SimGroup* pGroup = trigger->getGroup();
   for (SimGroup::iterator itr = pGroup->begin(); itr != pGroup->end(); itr++)
      Con::executef(*itr, 3, "onTrigger", Con::getIntArg(trigger->getId()), "1");
}

void cTriggerLeaveTrigger(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<TriggerData*>(obj) != NULL, "Error, how did a non-triggerdata get here?");
   TriggerData* triggerData = static_cast<TriggerData*>(obj);

   Trigger* trigger = NULL;
   if (Sim::findObject(argv[2], trigger) == false)
      return;

   if (trigger->getNumTriggeringObjects() == 0) {
      SimGroup* pGroup = trigger->getGroup();
      for (SimGroup::iterator itr = pGroup->begin(); itr != pGroup->end(); itr++)
         Con::executef(*itr, 3, "onTrigger", Con::getIntArg(trigger->getId()), "0");
   }
}

void cTriggerTickTrigger(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<TriggerData*>(obj) != NULL, "Error, how did a non-trigger get here?");
   TriggerData* triggerData = static_cast<TriggerData*>(obj);

   Trigger* trigger = NULL;
   if (Sim::findObject(argv[2], trigger) == false)
      return;
   
   // Do nothing with the trigger object id by default...
   SimGroup* pGroup = trigger->getGroup();
   for (SimGroup::iterator itr = pGroup->begin(); itr != pGroup->end(); itr++)
      Con::executef(*itr, 2, "onTriggerTick", Con::getIntArg(trigger->getId()));
}

S32 cTriggerGetNumObjects(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<Trigger*>(obj) != NULL, "Error, how did a non-trigger get here?");
   Trigger* trigger = static_cast<Trigger*>(obj);

   return trigger->getNumTriggeringObjects();
}

S32 cTriggerGetObject(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<Trigger*>(obj) != NULL, "Error, how did a non-trigger get here?");
   Trigger* trigger = static_cast<Trigger*>(obj);

   S32 index = dAtoi(argv[1]);

   if (index >= trigger->getNumTriggeringObjects() || index < 0)
      return -1;
   else
      return trigger->getObject(U32(index))->getId();
}


} // namespace {}

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(TriggerData);

TriggerData::TriggerData()
{
   tickPeriodMS = 100;
}

bool TriggerData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void TriggerData::consoleInit()
{
   // Triggering and onTick commands
   Con::addCommand("TriggerData", "onEnterTrigger", cTriggerEnterTrigger, "[TriggerData].enterTrigger(Trigger, ObjectId)", 4, 4);
   Con::addCommand("TriggerData", "onLeaveTrigger", cTriggerLeaveTrigger, "[TriggerData].leaveTrigger(Trigger, ObjectId)", 4, 4);
   Con::addCommand("TriggerData", "onTickTrigger",  cTriggerTickTrigger,  "[TriggerData].tickTrigger(Trigger)",            3, 3);
}


void TriggerData::initPersistFields()
{
   Parent::initPersistFields();

   addField("tickPeriodMS", TypeS32,  Offset(tickPeriodMS, TriggerData));
}


//--------------------------------------------------------------------------
void TriggerData::packData(BitStream* stream)
{
   Parent::packData(stream);
   stream->write(tickPeriodMS);
}

void TriggerData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   stream->read(&tickPeriodMS);
}


//--------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(Trigger);
Trigger::Trigger()
{
   mNetFlags.clear(Ghostable);

   mTypeMask |= TriggerObjectType;

   mObjScale.set(1, 1, 1);
   mObjToWorld.identity();
   mWorldToObj.identity();

   mDataBlock = NULL;

   mLastThink = 0;
   mCurrTick  = 0;

   mConvexList = new Convex;
}

Trigger::~Trigger()
{
   delete mConvexList;
   mConvexList = NULL;
}


//--------------------------------------------------------------------------
//-------------------------------------- Polyhedron type and persist field init
//
static const char *getDataTypeTriggerPolyhedron(void *dptr, EnumTable *, BitSet32)
{
   U32 i;
   Polyhedron* pPoly = reinterpret_cast<Polyhedron*>(dptr);

   Point3F origin = pPoly->pointList[0];
   U32 currVec = 0;
   Point3F vecs[3];
   for (i = 0; i < pPoly->edgeList.size(); i++) {
      if (pPoly->edgeList[i].vertex[0] == 0)
         vecs[currVec++] = pPoly->pointList[pPoly->edgeList[i].vertex[1]] - pPoly->pointList[0];
      else if (pPoly->edgeList[i].vertex[1] == 0)
         vecs[currVec++] = -(pPoly->pointList[0] - pPoly->pointList[pPoly->edgeList[i].vertex[0]]);
   }
   AssertFatal(currVec == 3, "Error, bad vectors!");

   char* retBuf = Con::getReturnBuffer(1024);

   dSprintf(retBuf, 1023, "%7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f",
            origin.x, origin.y, origin.z,
            vecs[0].x, vecs[0].y, vecs[0].z,
            vecs[1].x, vecs[1].y, vecs[1].z,
            vecs[2].x, vecs[2].y, vecs[2].z);

   return retBuf;
}

static void setDataTypeTriggerPolyhedron(void *dptr, S32 argc, const char **argv, EnumTable *, BitSet32)
{
   if (argc != 1) {
      Con::printf("(TypeTriggerPolyhedron) multiple args not supported for polyhedra");
      return;
   }

   Point3F origin;
   Point3F vecs[3];

   U32 numArgs = dSscanf(argv[0], "%f %f %f %f %f %f %f %f %f %f %f %f",
                         &origin.x, &origin.y, &origin.z,
                         &vecs[0].x, &vecs[0].y, &vecs[0].z,
                         &vecs[1].x, &vecs[1].y, &vecs[1].z,
                         &vecs[2].x, &vecs[2].y, &vecs[2].z);
   if (numArgs != 12) {
      Con::printf("Bad polyhedron!");
      return;
   }

   Polyhedron* pPoly = reinterpret_cast<Polyhedron*>(dptr);

   pPoly->pointList.setSize(8);
   pPoly->pointList[0] = origin;
   pPoly->pointList[1] = origin + vecs[0];
   pPoly->pointList[2] = origin + vecs[1];
   pPoly->pointList[3] = origin + vecs[2];
   pPoly->pointList[4] = origin + vecs[0] + vecs[1];
   pPoly->pointList[5] = origin + vecs[0] + vecs[2];
   pPoly->pointList[6] = origin + vecs[1] + vecs[2];
   pPoly->pointList[7] = origin + vecs[0] + vecs[1] + vecs[2];

   Point3F normal;
   pPoly->planeList.setSize(6);
   
   mCross(vecs[2], vecs[0], &normal);
   pPoly->planeList[0].set(origin, normal);
   mCross(vecs[0], vecs[1], &normal);
   pPoly->planeList[1].set(origin, normal);
   mCross(vecs[1], vecs[2], &normal);
   pPoly->planeList[2].set(origin, normal); 
   mCross(vecs[1], vecs[0], &normal);
   pPoly->planeList[3].set(pPoly->pointList[7], normal); 
   mCross(vecs[2], vecs[1], &normal);
   pPoly->planeList[4].set(pPoly->pointList[7], normal); 
   mCross(vecs[0], vecs[2], &normal);
   pPoly->planeList[5].set(pPoly->pointList[7], normal); 

   pPoly->edgeList.setSize(12);
   pPoly->edgeList[0].vertex[0]  = 0; pPoly->edgeList[0].vertex[1]  = 1; pPoly->edgeList[0].face[0]  = 0; pPoly->edgeList[0].face[1]  = 1;
   pPoly->edgeList[1].vertex[0]  = 1; pPoly->edgeList[1].vertex[1]  = 5; pPoly->edgeList[1].face[0]  = 0; pPoly->edgeList[1].face[1]  = 4;
   pPoly->edgeList[2].vertex[0]  = 5; pPoly->edgeList[2].vertex[1]  = 3; pPoly->edgeList[2].face[0]  = 0; pPoly->edgeList[2].face[1]  = 3;
   pPoly->edgeList[3].vertex[0]  = 3; pPoly->edgeList[3].vertex[1]  = 0; pPoly->edgeList[3].face[0]  = 0; pPoly->edgeList[3].face[1]  = 2;
   pPoly->edgeList[4].vertex[0]  = 3; pPoly->edgeList[4].vertex[1]  = 6; pPoly->edgeList[4].face[0]  = 3; pPoly->edgeList[4].face[1]  = 2;
   pPoly->edgeList[5].vertex[0]  = 6; pPoly->edgeList[5].vertex[1]  = 2; pPoly->edgeList[5].face[0]  = 2; pPoly->edgeList[5].face[1]  = 5;
   pPoly->edgeList[6].vertex[0]  = 2; pPoly->edgeList[6].vertex[1]  = 0; pPoly->edgeList[6].face[0]  = 2; pPoly->edgeList[6].face[1]  = 1;
   pPoly->edgeList[7].vertex[0]  = 1; pPoly->edgeList[7].vertex[1]  = 4; pPoly->edgeList[7].face[0]  = 4; pPoly->edgeList[7].face[1]  = 1;
   pPoly->edgeList[8].vertex[0]  = 4; pPoly->edgeList[8].vertex[1]  = 2; pPoly->edgeList[8].face[0]  = 1; pPoly->edgeList[8].face[1]  = 5;
   pPoly->edgeList[9].vertex[0]  = 4; pPoly->edgeList[9].vertex[1]  = 7; pPoly->edgeList[9].face[0]  = 4; pPoly->edgeList[9].face[1]  = 5;
   pPoly->edgeList[10].vertex[0] = 5; pPoly->edgeList[10].vertex[1] = 7; pPoly->edgeList[10].face[0] = 3; pPoly->edgeList[10].face[1] = 4;
   pPoly->edgeList[11].vertex[0] = 7; pPoly->edgeList[11].vertex[1] = 6; pPoly->edgeList[11].face[0] = 3; pPoly->edgeList[11].face[1] = 5;
}


void Trigger::initPersistFields()
{
   Parent::initPersistFields();

   Con::registerType(TypeTriggerPolyhedron, sizeof(Polyhedron), getDataTypeTriggerPolyhedron, setDataTypeTriggerPolyhedron);
   addField("polyhedron", TypeTriggerPolyhedron, Offset(mTriggerPolyhedron, Trigger));
}


void Trigger::consoleInit()
{
   //-------------------------------------- Object level commands

   // Manipulation commands
   Con::addCommand("Trigger", "getNumObjects",  cTriggerGetNumObjects, "[TriggerObject].getNumObjects()", 2, 2);
   Con::addCommand("Trigger", "getObject",      cTriggerGetObject,     "[TriggerObject].getNumObjects(Object Index)", 3, 3);
}


//--------------------------------------------------------------------------
bool Trigger::onAdd()
{
   if(!Parent::onAdd())
      return false;

   Polyhedron temp = mTriggerPolyhedron;
   setTriggerPolyhedron(temp);

   addToScene();

   return true;
}


void Trigger::onRemove()
{
   mConvexList->nukeList();

   removeFromScene();
   Parent::onRemove();
}

bool Trigger::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<TriggerData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}

void Trigger::onDeleteNotify(SimObject* obj)
{
   GameBase* pScene = dynamic_cast<GameBase*>(obj);
   if (pScene != NULL) {
      for (U32 i = 0; i < mObjects.size(); i++) {
         if (pScene == mObjects[i]) {
            mObjects.erase(i);
            Con::executef(mDataBlock, 3, "onLeaveTrigger", scriptThis(), Con::getIntArg(pScene->getId()));
            break;
         }
      }
   }

   Parent::onDeleteNotify(obj);
}


//--------------------------------------------------------------------------
void Trigger::buildConvex(const Box3F& box, Convex* convex)
{
   // These should really come out of a pool
   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.min.convolveInverse(mObjScale);
   realBox.max.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
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
   BoxConvex* cp = new BoxConvex;
   mConvexList->registerObject(cp);
   convex->addToWorkingList(cp);
   cp->init(this);

   mObjBox.getCenter(&cp->mCenter);
   cp->mSize.x = mObjBox.len_x() / 2.0f;
   cp->mSize.y = mObjBox.len_y() / 2.0f;
   cp->mSize.z = mObjBox.len_z() / 2.0f;
}


//------------------------------------------------------------------------------
void Trigger::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

   if (isServerObject()) {
      MatrixF base(true);
      base.scale(Point3F(1.0/mObjScale.x,
                         1.0/mObjScale.y,
                         1.0/mObjScale.z));
      base.mul(mWorldToObj);
      mClippedList.setBaseTransform(base);

      setMaskBits(GameBase::InitialUpdateMask);
   }
}

void Trigger::setTriggerPolyhedron(const Polyhedron& rPolyhedron)
{
   mTriggerPolyhedron = rPolyhedron;

   if (mTriggerPolyhedron.pointList.size() != 0) {
      mObjBox.min.set(1e10, 1e10, 1e10);
      mObjBox.max.set(-1e10, -1e10, -1e10);
      for (U32 i = 0; i < mTriggerPolyhedron.pointList.size(); i++) {
         mObjBox.min.setMin(mTriggerPolyhedron.pointList[i]);
         mObjBox.max.setMax(mTriggerPolyhedron.pointList[i]);
      }
   } else {
      mObjBox.min.set(-0.5, -0.5, -0.5);
      mObjBox.max.set( 0.5,  0.5,  0.5);
   }

   MatrixF xform = getTransform();
   setTransform(xform);

   mClippedList.clear();
   mClippedList.mPlaneList = mTriggerPolyhedron.planeList;
//   for (U32 i = 0; i < mClippedList.mPlaneList.size(); i++)
//      mClippedList.mPlaneList[i].neg();

   MatrixF base(true);
   base.scale(Point3F(1.0/mObjScale.x,
                      1.0/mObjScale.y,
                      1.0/mObjScale.z));
   base.mul(mWorldToObj);

   mClippedList.setBaseTransform(base);
}


//--------------------------------------------------------------------------
bool Trigger::testObject(GameBase* enter)
{
   if (mTriggerPolyhedron.pointList.size() == 0)
      return false;

   mClippedList.clear();

   SphereF sphere;
   sphere.center = (mWorldBox.min + mWorldBox.max) * 0.5;
   VectorF bv = mWorldBox.max - sphere.center;
   sphere.radius = bv.len();

   enter->buildPolyList(&mClippedList, mWorldBox, sphere);
   return mClippedList.isEmpty() == false;
}


void Trigger::potentialEnterObject(GameBase* enter)
{
   AssertFatal(isServerObject(), "Error, should never be called on the client!");

   for (U32 i = 0; i < mObjects.size(); i++) {
      if (mObjects[i] == enter)
         return;
   }

   if (testObject(enter) == true) {
      mObjects.push_back(enter);
      deleteNotify(enter);

      Con::executef(mDataBlock, 3, "onEnterTrigger", scriptThis(), Con::getIntArg(enter->getId()));
   }
}


void Trigger::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isClientObject())
      return;

   if (mObjects.size() == 0)
      return;

   if (mLastThink + mDataBlock->tickPeriodMS < mCurrTick) {
      mCurrTick  = 0;
      mLastThink = 0;

      for (S32 i = S32(mObjects.size() - 1); i >= 0; i--) {
         if (testObject(mObjects[i]) == false) {
            GameBase* remove = mObjects[i];
            mObjects.erase(i);
            clearNotify(remove);
            Con::executef(mDataBlock, 3, "onLeaveTrigger", scriptThis(), remove->scriptThis());
         }
      }

      if (mObjects.size() != 0)
         Con::executef(mDataBlock, 2, "onTickTrigger", scriptThis());
   } else {
      mCurrTick += TickMs;
   }
}


//--------------------------------------------------------------------------
bool Trigger::prepRenderImage(SceneState* state, const U32 stateKey,
                              const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

//    // This should be sufficient for most objects that don't manage zones, and
//    //  don't need to return a specialized RenderImage...
//    if (state->isObjectRendered(this)) {
//       SceneRenderImage* image = new SceneRenderImage;
//       image->obj = this;
//       image->isTranslucent = true;
//       image->sortKey = 0.0f;
//       state->insertRenderImage(image);
//    }

   return false;
}


void Trigger::renderObject(SceneState* state, SceneRenderImage*)
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

   mTriggerPolyhedron.render();

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//--------------------------------------------------------------------------
U32 Trigger::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 i;
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // Note that we don't really care about efficiency here, since this is an
   //  edit-only ghost...
   stream->writeAffineTransform(mObjToWorld);
   mathWrite(*stream, mObjScale);

   // Write the polyhedron
   stream->write(mTriggerPolyhedron.pointList.size());
   for (i = 0; i < mTriggerPolyhedron.pointList.size(); i++)
      mathWrite(*stream, mTriggerPolyhedron.pointList[i]);

   stream->write(mTriggerPolyhedron.planeList.size());
   for (i = 0; i < mTriggerPolyhedron.planeList.size(); i++)
      mathWrite(*stream, mTriggerPolyhedron.planeList[i]);

   stream->write(mTriggerPolyhedron.edgeList.size());
   for (i = 0; i < mTriggerPolyhedron.edgeList.size(); i++) {
      const Polyhedron::Edge& rEdge = mTriggerPolyhedron.edgeList[i];

      stream->write(rEdge.face[0]);
      stream->write(rEdge.face[1]);
      stream->write(rEdge.vertex[0]);
      stream->write(rEdge.vertex[1]);
   }

   return retMask;
}

void Trigger::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   U32 i, size;
   MatrixF temp;
   Point3F tempScale;
   Polyhedron tempPH;

   // Transform
   stream->readAffineTransform(&temp);
   mathRead(*stream, &tempScale);

   // Read the polyhedron
   stream->read(&size);
   tempPH.pointList.setSize(size);
   for (i = 0; i < tempPH.pointList.size(); i++)
      mathRead(*stream, &tempPH.pointList[i]);

   stream->read(&size);
   tempPH.planeList.setSize(size);
   for (i = 0; i < tempPH.planeList.size(); i++)
      mathRead(*stream, &tempPH.planeList[i]);

   stream->read(&size);
   tempPH.edgeList.setSize(size);
   for (i = 0; i < tempPH.edgeList.size(); i++) {
      Polyhedron::Edge& rEdge = tempPH.edgeList[i];

      stream->read(&rEdge.face[0]);
      stream->read(&rEdge.face[1]);
      stream->read(&rEdge.vertex[0]);
      stream->read(&rEdge.vertex[1]);
   }

   setTriggerPolyhedron(tempPH);
   setScale(tempScale);
   setTransform(temp);
}


