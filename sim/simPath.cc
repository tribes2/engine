//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Sim/simPath.h"
#include "console/consoleTypes.h"
#include "Sim/pathManager.h"
#include "PlatformWin32/platformGL.h"
#include "dgl/dgl.h"
#include "sceneGraph/sceneState.h"
#include "Math/mathIO.h"
#include "Core/bitStream.h"

//--------------------------------------------------------------------------
//-------------------------------------- Console functions and cmp funcs
//
namespace {

static ColorF cubeColors[8] = {
   ColorF(0, 0, 0),
   ColorF(1, 0, 0),
   ColorF(0, 1, 0),
   ColorF(0, 0, 1),
   ColorF(1, 1, 0),
   ColorF(1, 0, 1),
   ColorF(0, 1, 1),
   ColorF(1, 1, 1)
};

static Point3F cubePoints[8] = {
   Point3F(-1, -1, -1),
   Point3F(-1, -1,  1),
   Point3F(-1,  1, -1),
   Point3F(-1,  1,  1),
   Point3F( 1, -1, -1),
   Point3F( 1, -1,  1),
   Point3F( 1,  1, -1),
   Point3F( 1,  1,  1)
};

static U32 cubeFaces[6][4] = {
   { 0, 2, 6, 4 },
   { 0, 2, 3, 1 },
   { 0, 1, 5, 4 },
   { 3, 2, 6, 7 },
   { 7, 6, 4, 5 },
   { 3, 7, 5, 1 }
};

void wireCube(F32 size, Point3F pos)
{
   glDisable(GL_CULL_FACE);

   for(int i = 0; i < 6; i++)
   {
      glBegin(GL_LINE_LOOP);
      for(int vert = 0; vert < 4; vert++)
      {
         int idx = cubeFaces[i][vert];
         glColor3f(cubeColors[idx].red, cubeColors[idx].green, cubeColors[idx].blue);
         glVertex3f(cubePoints[idx].x * size + pos.x, cubePoints[idx].y * size + pos.y, cubePoints[idx].z * size + pos.z);
      }
      glEnd();
   }
}


void cPathMissionLoadDone(SimObject*, S32, const char**)
{
   // Need to load subobjects for all loaded interiors...
   SimGroup* pMissionGroup = dynamic_cast<SimGroup*>(Sim::findObject("MissionGroup"));
   AssertFatal(pMissionGroup != NULL, "Error, mission done loading and no mission group?");

   U32 currStart = 0;
   U32 currEnd   = 1;
   Vector<SimGroup*> groups;
   groups.push_back(pMissionGroup);

   while (true) {
      for (U32 i = currStart; i < currEnd; i++) {
         for (SimGroup::iterator itr = groups[i]->begin(); itr != groups[i]->end(); itr++) {
            if (dynamic_cast<SimGroup*>(*itr) != NULL)
               groups.push_back(static_cast<SimGroup*>(*itr));
         }
      }

      if (groups.size() == currEnd) {
         break;
      } else {
         currStart = currEnd;
         currEnd   = groups.size();
      }
   }

   for (U32 i = 0; i < groups.size(); i++) {
      Path* pPath = dynamic_cast<Path*>(groups[i]);
      if (pPath)
         pPath->finishPath();
   }
}

S32 FN_CDECL cmpPathObject(const void* p1, const void* p2)
{
   SimObject* o1 = *((SimObject**)p1);
   SimObject* o2 = *((SimObject**)p2);

   Marker* m1 = dynamic_cast<Marker*>(o1);
   Marker* m2 = dynamic_cast<Marker*>(o2);

   if (m1 == NULL && m2 == NULL)
      return 0;
   else if (m1 != NULL && m2 == NULL)
      return 1;
   else if (m1 == NULL && m2 != NULL)
      return -1;
   else {
      // Both markers...
      return S32(m1->mSeqNum) - S32(m2->mSeqNum);
   }
}

} // namespace {}


//--------------------------------------------------------------------------
//-------------------------------------- Implementation
//
IMPLEMENT_CONOBJECT(Path);

Path::Path()
{
   mPathIndex = NoPathIndex;
}

Path::~Path()
{
   //
}

//--------------------------------------------------------------------------
void Path::initPersistFields()
{
   Parent::initPersistFields();

   //
}


void Path::consoleInit()
{
   Con::addCommand("pathOnMissionLoadDone", cPathMissionLoadDone, "pathOnMissionLoadDone()", 1, 1);
}


//--------------------------------------------------------------------------
bool Path::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   return true;
}


void Path::onRemove()
{
   //

   Parent::onRemove();
}


//--------------------------------------------------------------------------
void Path::finishPath()
{
   // If we need to, allocate a path index from the manager
   if (mPathIndex == NoPathIndex)
      mPathIndex = gServerPathManager->allocatePathId();

   // Sort the markers objects into sequence order
   dQsort(objectList.address(), objectList.size(), sizeof(SimObject*), cmpPathObject);

   Vector<Point3F> positions;
   Vector<U32>     times;

   for (iterator itr = begin(); itr != end(); itr++) {
      Marker* pMarker = dynamic_cast<Marker*>(*itr);
      if (pMarker != NULL) {
         Point3F pos;
         pMarker->getTransform().getColumn(3, &pos);

         positions.push_back(pos);
         times.push_back(pMarker->mMSToNext);
      }
   }

   // DMMTODO: Looping paths.
   gServerPathManager->updatePath(mPathIndex, positions, times);
}

void Path::addObject(SimObject* obj)
{
   Parent::addObject(obj);

   if (mPathIndex != NoPathIndex) {
      // If we're already finished, and this object is a marker, then we need to
      //  update our path information...
      if (dynamic_cast<Marker*>(obj) != NULL)
         finishPath();
   }
}

void Path::removeObject(SimObject* obj)
{
   bool recalc = dynamic_cast<Marker*>(obj) != NULL;

   Parent::removeObject(obj);

   if (mPathIndex != NoPathIndex && recalc == true)
      finishPath();
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(Marker);
Marker::Marker()
{
   // Not ghostable unless we're editing...
   //mNetFlags.set(Ghostable);

   mTypeMask = MarkerObjectType;

   mSeqNum   = 0;
   mMSToNext = 1000;
}

Marker::~Marker()
{
   //
}

//--------------------------------------------------------------------------
void Marker::initPersistFields()
{
   Parent::initPersistFields();

   addField("seqNum",   TypeS32, Offset(mSeqNum,   Marker));
   addField("msToNext", TypeS32, Offset(mMSToNext, Marker));
}


void Marker::consoleInit()
{
   //
}


//--------------------------------------------------------------------------
bool Marker::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   mObjBox = Box3F(Point3F(-.25, -.25, -.25), Point3F(.25, .25, .25));
   resetWorldBox();

   addToScene();

   return true;
}


void Marker::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}



//--------------------------------------------------------------------------
bool Marker::prepRenderImage(SceneState* state, const U32 stateKey,
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
      state->insertRenderImage(image);
   }

   return false;
}


void Marker::renderObject(SceneState* state, SceneRenderImage*)
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
   wireCube(0.5, Point3F(0, 0, 0));

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//--------------------------------------------------------------------------
U32 Marker::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   Point3F pos;
   getTransform().getColumn(3, &pos);
   mathWrite(*stream, pos);

   //

   return retMask;
}

void Marker::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   Point3F pos;
   mathRead(*stream, &pos);

   MatrixF temp(true);
   temp.setColumn(3, pos);
   setTransform(temp);
}
