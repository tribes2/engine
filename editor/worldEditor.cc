//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "editor/worldEditor.h"
#include "sceneGraph/sceneGraph.h"
#include "sceneGraph/sceneState.h"
#include "sim/sceneObject.h"
#include "platform/event.h"
#include "gui/guiCanvas.h"
#include "game/gameConnection.h"
#include "core/memstream.h"
#include "collision/clippedPolyList.h"
#include "game/shapeBase.h"
#include "console/consoleInternal.h"
#include "game/sphere.h"
#include "ai/graph.h"

IMPLEMENT_CONOBJECT(WorldEditor);

// unnamed namespace for static data
namespace {

   static Point3F BoxPnts[] = {
      Point3F(0,0,0),
      Point3F(0,0,1),
      Point3F(0,1,0),
      Point3F(0,1,1),
      Point3F(1,0,0),
      Point3F(1,0,1),
      Point3F(1,1,0),
      Point3F(1,1,1)
   };

   static U32 BoxVerts[][4] = {
      {0,2,3,1},     // -x
      {7,6,4,5},     // +x
      {0,1,5,4},     // -y
      {3,2,6,7},     // +y
      {0,4,6,2},     // -z
      {3,7,5,1}      // +z   
   };
   
   static Point3F BoxNormals[] = {
      Point3F(-1, 0, 0),
      Point3F( 1, 0, 0),
      Point3F( 0,-1, 0),
      Point3F( 0, 1, 0),
      Point3F( 0, 0,-1),
      Point3F( 0, 0, 1)
   };

   //
   U32 getBoxNormalIndex(const VectorF & normal)
   {
      const F32 * pNormal = ((const F32 *)normal);
      
      F32 max = 0;
      S32 index = -1;
      
      for(U32 i = 0; i < 3; i++)
         if(mFabs(pNormal[i]) >= mFabs(max))
         {
            max = pNormal[i];
            index = i*2;
         }
      
      AssertFatal(index >= 0, "Failed to get best normal");
      if(max > 0.f)
         index++;
      
      return(index);
   }

   //
   Point3F getBoundingBoxCenter(SceneObject * obj)
   {
      Box3F box = obj->getObjBox();
      MatrixF mat = obj->getTransform();
      VectorF scale = obj->getScale();
   
      Point3F center(0,0,0);
      Point3F projPnts[8];
   
      for(U32 i = 0; i < 8; i++)
      {  
         Point3F pnt;
         pnt.set(BoxPnts[i].x ? box.max.x : box.min.x, 
                 BoxPnts[i].y ? box.max.y : box.min.y,
                 BoxPnts[i].z ? box.max.z : box.min.z);
   
         // scale it
         pnt.convolve(scale);
         mat.mulP(pnt, &projPnts[i]);
         center += projPnts[i];
      }
   
      center /= 8;
      return(center);
   }

   //
   const char * parseObjectFormat(SimObject * obj, const char * format)
   {
      static char buf[1024];

      U32 curPos = 0;
      U32 len = dStrlen(format);

      for(U32 i = 0; i < len; i++)
      {  
         if(format[i] == '$')
         {
            U32 j;
            for(j = i+1; j < len; j++)
               if(format[j] == '$')
                  break;

            if(j == len)
               break;

            char token[80];

            AssertFatal((j - i) < (sizeof(token) - 1), "token too long");
            dStrncpy(token, &format[i+1], (j - i - 1));
            token[j-i-1] = 0;
         
            U32 remaining = sizeof(buf) - curPos - 1;
            
            // look at the token
            if(!dStricmp(token, "id"))
               curPos += dSprintf(buf + curPos, remaining, "%d", obj->getId());
            else if(!dStricmp(token, "name"))
               curPos += dSprintf(buf + curPos, remaining, "%s", obj->getName());
            else if(!dStricmp(token, "class"))
               curPos += dSprintf(buf + curPos, remaining, "%s", obj->getClassName());
            else if(!dStricmp(token, "namespace") && obj->getNamespace())
               curPos += dSprintf(buf + curPos, remaining, "%s", obj->getNamespace()->mName);

            // 
            i = j;
         }
         else
            buf[curPos++] = format[i];
      }   

      buf[curPos] = 0;
      return(buf);
   }

   //
   F32 snapFloat(F32 val, F32 snap)
   {
      if(snap == 0.f)
         return(val);

      F32 a = mFmod(val, snap);

      if(mFabs(a) > (snap / 2))
         val < 0.f ? val -= snap : val += snap;

      return(val - a);
   }

   //
   EulerF extractEuler(const MatrixF & matrix)
   {
      const F32 * mat = (const F32*)matrix;
      
      EulerF r;   
      r.x = mAsin(mat[MatrixF::idx(2,1)]);

      if(mCos(r.x) != 0.f)
      {
         r.y = mAtan(-mat[MatrixF::idx(2,0)], mat[MatrixF::idx(2,2)]);
         r.z = mAtan(-mat[MatrixF::idx(0,1)], mat[MatrixF::idx(1,1)]);
      }
      else
      {
         r.y = 0.f;
         r.z = mAtan(mat[MatrixF::idx(1,0)], mat[MatrixF::idx(0,0)]);
      }

      return(r);
   }
}

//------------------------------------------------------------------------------
// Class WorldEditor::Selection
//------------------------------------------------------------------------------

WorldEditor::Selection::Selection() :
   mCentroidValid(false),
   mAutoSelect(false)
{
   registerObject();
}

WorldEditor::Selection::~Selection()
{
   unregisterObject();
}

bool WorldEditor::Selection::objInSet(SceneObject * obj)
{
   for(U32 i = 0; i < mObjectList.size(); i++)
      if(mObjectList[i] == (SimObject*)obj)
         return(true);
   return(false);
}

bool WorldEditor::Selection::addObject(SceneObject * obj)
{
   if(objInSet(obj))
      return(false);

   mCentroidValid = false;

   mObjectList.pushBack(obj);
   deleteNotify(obj);

   if(mAutoSelect)
   {
      obj->setSelected(true);
      SceneObject * clientObj = WorldEditor::getClientObj(obj);
      if(clientObj)
         clientObj->setSelected(true);
   }

   return(true);
}

bool WorldEditor::Selection::removeObject(SceneObject * obj)
{
   if(!objInSet(obj))
      return(false);

   mCentroidValid = false;

   mObjectList.remove(obj);
   clearNotify(obj);

   if(mAutoSelect)
   {
      obj->setSelected(false);

      SceneObject * clientObj = WorldEditor::getClientObj(obj);
      if(clientObj)
         clientObj->setSelected(false);
   }

   return(true);
}

void WorldEditor::Selection::clear()
{
   while(mObjectList.size())
      removeObject((SceneObject*)mObjectList[0]);
}

void WorldEditor::Selection::onDeleteNotify(SimObject * obj)
{
   removeObject((SceneObject*)obj);
}

void WorldEditor::Selection::updateCentroid()
{
   if(mCentroidValid)
      return;

   mCentroidValid = true;

   //
   mCentroid.set(0,0,0);
   mBoxCentroid = mCentroid;

   if(!mObjectList.size())
      return;

   //
   for(U32 i = 0; i < mObjectList.size(); i++)
   {
      const MatrixF & mat = ((SceneObject*)mObjectList[i])->getTransform();
      Point3F wPos;
      mat.getColumn(3, &wPos);
      
      //   
      mBoxCentroid += getBoundingBoxCenter((SceneObject*)mObjectList[i]);      
      mCentroid += wPos;
   }

   mCentroid /= mObjectList.size();
   mBoxCentroid /= mObjectList.size();
}

const Point3F & WorldEditor::Selection::getCentroid()
{
   updateCentroid();
   return(mCentroid);
}

const Point3F & WorldEditor::Selection::getBoxCentroid()
{
   updateCentroid();
   return(mBoxCentroid);
}

void WorldEditor::Selection::enableCollision()
{
   for(U32 i = 0; i < mObjectList.size(); i++)
      ((SceneObject*)mObjectList[i])->enableCollision();
}

void WorldEditor::Selection::disableCollision()
{
   for(U32 i = 0; i < mObjectList.size(); i++)
      ((SceneObject*)mObjectList[i])->disableCollision();
}

//------------------------------------------------------------------------------

void WorldEditor::Selection::offset(const Point3F & offset)
{
   for(U32 i = 0; i < mObjectList.size(); i++)
   {
      MatrixF mat = ((SceneObject*)mObjectList[i])->getTransform();
      Point3F wPos;
      mat.getColumn(3, &wPos);
      
      // adjust
      wPos += offset;
      mat.setColumn(3, wPos);
      ((SceneObject*)mObjectList[i])->setTransform(mat);
   }

   mCentroidValid = false;
}

void WorldEditor::Selection::rotate(const EulerF & rot, const Point3F & center)
{
   // single selections will rotate around own axis, multiple about world
   if(mObjectList.size() == 1)
   {
      MatrixF mat = ((SceneObject*)mObjectList[0])->getTransform();

      Point3F pos;
      mat.getColumn(3, &pos);

      // get offset in obj space
      Point3F offset = pos - center;
      MatrixF wMat = ((SceneObject*)mObjectList[0])->getWorldTransform();
      wMat.mulV(offset);

      // 
      MatrixF transform(EulerF(0,0,0), -offset);
      transform.mul(MatrixF(rot));
      transform.mul(MatrixF(EulerF(0,0,0), offset));
      mat.mul(transform);

      ((SceneObject*)mObjectList[0])->setTransform(mat);
   }
   else
   {
      for(U32 i = 0; i < size(); i++)
      {
         MatrixF mat = ((SceneObject*)mObjectList[i])->getTransform();

         Point3F pos;
         mat.getColumn(3, &pos);

         // get offset in obj space
         Point3F offset = pos - center;
      
         MatrixF transform(rot);
         Point3F wOffset;   
         transform.mulV(offset, &wOffset);

         MatrixF wMat = ((SceneObject*)mObjectList[i])->getWorldTransform();
         wMat.mulV(offset);

         //
         transform.set(EulerF(0,0,0), -offset);
         
         mat.setColumn(3, Point3F(0,0,0));
         wMat.setColumn(3, Point3F(0,0,0));

         transform.mul(wMat);
         transform.mul(MatrixF(rot));
         transform.mul(mat);
         mat.mul(transform);

         mat.normalize();
         mat.setColumn(3, wOffset + center);

         ((SceneObject*)mObjectList[i])->setTransform(mat);
      }
   }

   mCentroidValid = false;
}

void WorldEditor::Selection::scale(const VectorF & scale)
{
   for(U32 i = 0; i < mObjectList.size(); i++)
   {
      VectorF current = ((SceneObject*)mObjectList[i])->getScale();
      current.convolve(scale);
      ((SceneObject*)mObjectList[i])->setScale(current);
   }

   mCentroidValid = false;
}

//------------------------------------------------------------------------------

SceneObject * WorldEditor::getClientObj(SceneObject * obj)
{
   AssertFatal(obj->isServerObject(), "WorldEditor::getClientObj: not a server object!");

   NetConnection * toServer = NetConnection::getServerConnection();
   NetConnection * toClient = NetConnection::getLocalClientConnection();

   S32 index = toClient->getGhostIndex(obj);
   if(index == -1)
      return(0);
   
   return(dynamic_cast<SceneObject*>(toServer->resolveGhost(index)));
}

void WorldEditor::setClientObjInfo(SceneObject * obj, const MatrixF & mat, const VectorF & scale)
{
   SceneObject * clientObj = getClientObj(obj);
   if(!clientObj)
      return;
   
   clientObj->setTransform(mat);
   clientObj->setScale(scale);
}

void WorldEditor::updateClientTransforms(Selection & sel)
{
   for(U32 i = 0; i < sel.size(); i++)
   {
      SceneObject * clientObj = getClientObj(sel[i]);
      if(!clientObj)
         continue;

      //
      clientObj->setTransform(sel[i]->getTransform());
      clientObj->setScale(sel[i]->getScale());
   }
}

//------------------------------------------------------------------------------
// simple undo mechanism: bascially maintains stacks of state information
// from the selections (transform info only)
WorldEditor::SelectionState * WorldEditor::createUndo(Selection & sel)
{
   SelectionState * sState = new SelectionState;
   for(U32 i = 0; i < sel.size(); i++)
   {
      SelectionState::Entry entry;

      entry.mMatrix = sel[i]->getTransform();
      entry.mScale = sel[i]->getScale();
      entry.mObjId = sel[i]->getId();
      sState->mEntries.push_back(entry);
   }
   return(sState);
}
   
void WorldEditor::addUndo(Vector<SelectionState *> & list, SelectionState * sel)
{
   AssertFatal(sel, "WorldEditor::addUndo - invalid selection");
   list.push_front(sel);
   if(list.size() == mUndoLimit)
   {
      SelectionState * ss = list[list.size()-1];
      delete ss;
      list.pop_back();
   }
}

bool WorldEditor::processUndo(Vector<SelectionState*> & src, Vector<SelectionState*> & dest)
{
   if(!src.size())
      return(false);
      
   SelectionState * task = src.front();
   src.pop_front();
   
   for(U32 i = 0; i < task->mEntries.size(); i++)
   {
      SceneObject * obj = static_cast<SceneObject*>(Sim::findObject(task->mEntries[i].mObjId));
      if(obj)
      {
         setClientObjInfo(obj, task->mEntries[i].mMatrix, task->mEntries[i].mScale);
         obj->setTransform(task->mEntries[i].mMatrix);
         obj->setScale(task->mEntries[i].mScale);
      }
   }
   
   addUndo(dest, task);
   mSelected.invalidateCentroid();

   return(true);
}

void WorldEditor::clearUndo(Vector<SelectionState *> & list)
{
   for(U32 i = 0; i < list.size(); i++)
      delete list[i];
   list.clear();
}

//------------------------------------------------------------------------------
// edit stuff
bool WorldEditor::deleteSelection(Selection & sel)
{
   while(sel.size())
      sel[0]->deleteObject();
   return(true);
}

bool WorldEditor::copySelection(Selection & sel)
{
   mStreamBufs.clear();
   
   for(U32 i = 0; i < sel.size(); i++)
   {
      // um.. can we say lame? 
      mStreamBufs.increment();
      MemStream stream(2048, mStreamBufs.last(), false, true);
      stream.write(7, (void*)"return ");
      sel[i]->write(stream, 0);
      
      if(stream.getStatus() != Stream::Ok)
      {
         mStreamBufs.clear();
         return(false);
      }
   }
   return(true);
}

bool WorldEditor::pasteSelection()
{
   if(!mSelectionLocked)   
      mSelected.clear();
   for(U32 i = 0; i < mStreamBufs.size(); i++)
   {
      const char * eval = Con::evaluate((const char *)mStreamBufs[i]);
      SceneObject * obj = dynamic_cast<SceneObject*>(Sim::findObject(eval));
      if(obj && !mSelectionLocked)
         mSelected.addObject(obj);
   }
   
   // drop it ... 
   dropSelection(mSelected);
   return(true);
}

//------------------------------------------------------------------------------

void WorldEditor::hideSelection(bool hide)
{
   // set server/client objects hide field
   for(U32 i = 0; i < mSelected.size(); i++)
   {
      // client
      SceneObject * clientObj = getClientObj(mSelected[i]);
      if(!clientObj)
         continue;

      clientObj->setHidden(hide);

      // server
      mSelected[i]->setHidden(hide);
   }
}

void WorldEditor::lockSelection(bool lock)
{
   //
   for(U32 i = 0; i < mSelected.size(); i++)
      mSelected[i]->setLocked(lock);
}

//------------------------------------------------------------------------------
// the centroid get's moved to the drop point...
void WorldEditor::dropSelection(Selection & sel)
{
   if(!sel.size())
      return;
      
   Point3F centroid = mObjectsUseBoxCenter ? sel.getBoxCentroid() : sel.getCentroid();
   
   switch(mDropType)
   {
      case DropAtCentroid:
         // already there
         break;
         
      case DropAtOrigin:
      {
         sel.offset(Point3F(-centroid));
         break;
      }
      
      case DropAtCameraWithRot:
      {
         sel.offset(Point3F(smCamPos - centroid));
         sel.rotate(smCamRot, centroid);
         break;
      }
     
      case DropAtCamera:
      {
         sel.offset(Point3F(smCamPos - centroid));
         break;
      }
      
      case DropBelowCamera:
      {
         Point3F offset = smCamPos - centroid;
         offset.z -= 15.f;
         sel.offset(offset);
         break;
      }
      
      case DropAtScreenCenter:
      {
         Gui3DMouseEvent event;
         event.pos = smCamPos;
         
         Point2I offset = localToGlobalCoord(Point2I(0,0));

         Point3F sp(offset.x + (getExtent().x / 2), offset.y + (getExtent().y / 2), 1);

         Point3F wp;
         unproject(sp, &wp);
         event.vec = wp - smCamPos;
         event.vec.normalizeSafe();
         
         // if fails to hit screen, then place at camera
         CollisionInfo info;
         if(collide(event, info))
            sel.offset(Point3F(info.pos - centroid));
         else
            sel.offset(Point3F(event.pos - centroid));

         break;
      }
      
      case DropToGround:
      {
         for(U32 i = 0; i < sel.size(); i++)
         {
            Point3F start;
            MatrixF mat = sel[i]->getTransform();
            mat.getColumn(3, &start);
            Point3F end = start;
            start.z = -2000.f;
            end.z = 2000.f;
            
            RayInfo ri;
            bool hit;
            if(mBoundingBoxCollision)
               hit = gServerContainer.collideBox(start, end, TerrainObjectType, &ri);
            else
               hit = gServerContainer.castRay(start, end, TerrainObjectType, &ri);
               
            if(hit)
            {
               mat.setColumn(3, ri.point);
               sel[i]->setTransform(mat);
            }
            else
               sel.offset(Point3F(smCamPos - centroid));
         }
         break;
      }
   }
   
   //
   updateClientTransforms(sel);
}

//------------------------------------------------------------------------------

SceneObject * WorldEditor::getControlObject()
{
   GameConnection * connection = GameConnection::getLocalClientConnection();
   if(connection)
      return(dynamic_cast<SceneObject*>(connection->getControlObject()));
   return(0);
}

bool WorldEditor::collide(const Gui3DMouseEvent & event, CollisionInfo & info)
{
   // turn off the collsion with the control object
   SceneObject * controlObj = getControlObject();
   if(controlObj)
      controlObj->disableCollision();

   //
   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * mProjectDistance;

   //
   RayInfo ri;
   bool hit;
   if(mBoundingBoxCollision)
      hit = gServerContainer.collideBox(startPnt, endPnt, -1, &ri);
   else
      hit = gServerContainer.castRay(startPnt, endPnt, -1, &ri);
   
   //
   if(hit)
   {
      info.pos = ri.point;
      info.obj = ri.object;
      info.normal = ri.normal;
      AssertFatal(info.obj, "WorldEditor::collide - client container returned non SceneObject");
   }
   
   if(controlObj)
      controlObj->enableCollision();
   return(hit);
}

//------------------------------------------------------------------------------
// main render functions

void WorldEditor::renderSelectionWorldBox(Selection & sel)
{
   if(!mRenderSelectionBox)
      return;

   //
   if(!sel.size())
      return;

   // build the world bounds
   Box3F selBox = sel[0]->getWorldBox();

   U32 i;
   for(i = 1; i < sel.size(); i++)
   {
      const Box3F & wBox = sel[i]->getWorldBox();
      selBox.min.setMin(wBox.min);
      selBox.max.setMax(wBox.max);
   }

   //
   glDisable(GL_CULL_FACE);
   glColor4ub(mSelectionBoxColor.red, mSelectionBoxColor.green, 
      mSelectionBoxColor.blue, mSelectionBoxColor.alpha);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // create the box points   
   Point3F projPnts[8];
   for(i = 0; i < 8; i++)
   {  
      Point3F pnt;
      pnt.set(BoxPnts[i].x ? selBox.max.x : selBox.min.x, 
              BoxPnts[i].y ? selBox.max.y : selBox.min.y,
              BoxPnts[i].z ? selBox.max.z : selBox.min.z);
      projPnts[i] = pnt;
   }

   // do the box
   for(U32 j = 0; j < 6; j++)
   {
      glBegin(GL_LINE_LOOP);
      for(U32 k = 0; k < 4; k++)
         glVertex3f(projPnts[BoxVerts[j][k]].x,
                    projPnts[BoxVerts[j][k]].y,
                    projPnts[BoxVerts[j][k]].z);
      glEnd();
   }
}

void WorldEditor::renderObjectBox(SceneObject * obj, const ColorI & col)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   glDisable(GL_CULL_FACE);
   glColor4ub(col.red, col.green, col.blue, col.alpha);
   
   // project the points...
   Box3F box = obj->getObjBox();
   MatrixF mat = obj->getTransform();
   VectorF scale = obj->getScale();
   
   Point3F projPnts[8];
   for(U32 i = 0; i < 8; i++)
   {  
      Point3F pnt;
      pnt.set(BoxPnts[i].x ? box.max.x : box.min.x, 
              BoxPnts[i].y ? box.max.y : box.min.y,
              BoxPnts[i].z ? box.max.z : box.min.z);

      // scale it
      pnt.convolve(scale);
      mat.mulP(pnt, &projPnts[i]);
   }
   
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
   // do the box
   for(U32 j = 0; j < 6; j++)
   {
      glBegin(GL_LINE_LOOP);
      for(U32 k = 0; k < 4; k++)
         glVertex3f(projPnts[BoxVerts[j][k]].x,
                    projPnts[BoxVerts[j][k]].y,
                    projPnts[BoxVerts[j][k]].z);
      glEnd();
   }

//   // render the collision polys?
//   // jff - this is using the worldbox... use obj box? which sides?
//   if(mRenderBoxIntersect)
//   {
//      obj->disableCollision();
//      Box3F bBox = obj->getWorldBox();
//   
//      bBox.min.z -= mProjectDistance;
//      bBox.max.z += mProjectDistance;
//
//      ClippedPolyList polyList;
//      polyList.mPlaneList.clear();
//      polyList.mNormal.set(0,0,0);
//      polyList.mPlaneList.setSize(4);
//      polyList.mPlaneList[0].set(bBox.min, VectorF(-1,0,0));
//      polyList.mPlaneList[1].set(bBox.max, VectorF(0,1,0));
//      polyList.mPlaneList[2].set(bBox.max, VectorF(1,0,0));
//      polyList.mPlaneList[3].set(bBox.min, VectorF(0,-1,0));
//      
//      // build the poly list
//      if(gServerContainer.buildPolyList(bBox, -1, &polyList))
//      {
//         glEnable(GL_CULL_FACE);
//         glDisable(GL_DEPTH_TEST);
//         glVertexPointer(3,GL_FLOAT,sizeof(ClippedPolyList::Vertex), polyList.mVertexList.address());
//         glEnableClientState(GL_VERTEX_ARRAY);
//
//         glColor4ub(mBoxIntersectColor.red, mBoxIntersectColor.green, mBoxIntersectColor.blue, mBoxIntersectColor.alpha);
//
//         glEnable(GL_BLEND);
//         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//         // render em...
//         ClippedPolyList::Poly * p;
//         for (p = polyList.mPolyList.begin(); p < polyList.mPolyList.end(); p++) {
//            glDrawElements(GL_POLYGON,p->vertexCount,
//               GL_UNSIGNED_INT,&polyList.mIndexList[p->vertexStart]);
//         }
//
//         glDisableClientState(GL_VERTEX_ARRAY);
//         glEnable(GL_DEPTH_TEST);
//         glDisable(GL_CULL_FACE);
//      }
//      obj->enableCollision();
//   }

   dglSetCanonicalState();
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//------------------------------------------------------------------------------

void WorldEditor::renderObjectFace(SceneObject * obj, const VectorF & normal, const ColorI & col)
{
   // get the normal index
   VectorF objNorm;
   obj->getWorldTransform().mulV(normal, &objNorm);

   U32 normI = getBoxNormalIndex(objNorm);   

   //
   Box3F box = obj->getObjBox();
   MatrixF mat = obj->getTransform();
   VectorF scale = obj->getScale();
      
   Point3F projPnts[4];
   for(U32 i = 0; i < 4; i++)
   {  
      Point3F pnt;
      pnt.set(BoxPnts[BoxVerts[normI][i]].x ? box.max.x : box.min.x,
              BoxPnts[BoxVerts[normI][i]].y ? box.max.y : box.min.y,
              BoxPnts[BoxVerts[normI][i]].z ? box.max.z : box.min.z);

      // scale it
      pnt.convolve(scale);
      mat.mulP(pnt, &projPnts[i]);
   }
   
   glDisable(GL_CULL_FACE);
   glColor4ub(col.red, col.green, col.blue, col.alpha);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_DEPTH_TEST);

   //
   glBegin(GL_QUADS);
   for(U32 k = 0; k < 4; k++)
      glVertex3f(projPnts[k].x, projPnts[k].y, projPnts[k].z);
   glEnd();
   glDisable(GL_BLEND);
   glEnable(GL_DEPTH_TEST);
}

//------------------------------------------------------------------------------

void WorldEditor::renderPlane(const Point3F & origin)
{
   if(!(mRenderPlane || mRenderPlaneHashes))
      return;

   glDisable(GL_CULL_FACE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
   glColor4ub(mGridColor.red, mGridColor.green, mGridColor.blue, mGridColor.alpha);
   Point2F start(origin.x - mPlaneDim / 2, origin.y - mPlaneDim / 2);

   //   
   if(mRenderPlane)
   {
      // draw the plane
      glBegin(GL_QUADS);
         glVertex3f(start.x, start.y, origin.z);
         glVertex3f(start.x, start.y + mPlaneDim, origin.z);
         glVertex3f(start.x + mPlaneDim, start.y + mPlaneDim, origin.z);
         glVertex3f(start.x + mPlaneDim, start.y, origin.z);
      glEnd();
   }
   
   //
   if(mRenderPlaneHashes)
   {
      if(mGridSize.x > 0)
      {
         U32 xSteps = (U32)(mPlaneDim / mGridSize.x);
         F32 hashStart = mCeil(start.x / mGridSize.x) * mGridSize.x;
         for(U32 i = 0; i < xSteps; i++)
         {
            glBegin(GL_LINE_LOOP);
            glVertex3f(hashStart + mGridSize.x * i, start.y, origin.z + 0.001f);
            glVertex3f(hashStart + mGridSize.x * i, start.y + mPlaneDim, origin.z + 0.001f);
            glEnd();
         }
      }
      
      if(mGridSize.y > 0)
      {
         U32 ySteps = (U32)(mPlaneDim / mGridSize.y);
         F32 hashStart = mCeil(start.y / mGridSize.y) * mGridSize.y;
         for(U32 i = 0; i < ySteps; i++)
         {
            glBegin(GL_LINE_LOOP);
            glVertex3f(start.x, hashStart + mGridSize.y * i, origin.z + 0.001f);
            glVertex3f(start.x + mPlaneDim, hashStart + mGridSize.y * i, origin.z + 0.001f);
            glEnd();
         }
      }
   }
   
   glDisable(GL_BLEND);
}

//------------------------------------------------------------------------------

void WorldEditor::renderMousePopupInfo()
{
   Point2I pos(mLastMouseEvent.mousePoint.x,
      mLastMouseEvent.mousePoint.y + getCursor()->getExtent().y - getCursor()->getHotSpot().y);
         
   if(mCurrentMode == mDefaultMode && !mMouseDragged)
      return;
      
   char buf[256];
   switch(mCurrentMode)
   {
      case Move:
      {
         if(!mSelected.size())
            return;
            
         Point3F pos = mObjectsUseBoxCenter ? mSelected.getBoxCentroid() : mSelected.getCentroid();
         dSprintf(buf, sizeof(buf), "x: %0.3f, y: %0.3f, z: %0.3f", pos.x, pos.y, pos.z);
         break;
      }
      
      case Rotate: {
         
         if(!bool(mHitObject) || (mSelected.size() != 1))
            return;
   
         // print out the angle-axis 'fo
         AngAxisF aa(mHitObject->getTransform());

         dSprintf(buf, sizeof(buf), "x: %0.3f, y: %0.3f, z: %0.3f, a: %0.3f",
            aa.axis.x, aa.axis.y, aa.axis.z, mRadToDeg(aa.angle));

         break;
      }

      case Scale: {
      
         if(!bool(mHitObject) || (mSelected.size() != 1))
            return;
            
         VectorF scale = mHitObject->getScale();
         
         Box3F box = mHitObject->getObjBox();
         box.min.convolve(scale);
         box.max.convolve(scale);

         box.max -= box.min;
         dSprintf(buf, sizeof(buf), "w: %0.3f, h: %0.3f, d: %0.3f", box.max.x, box.max.y, box.max.z);
         break;
      }

      default:
         return;
   }

   U32 width = mProfile->mFont->getStrWidth(buf);

   if(mRenderPopupBackground)
   {
      Point2I min(pos.x - width / 2 - 2, pos.y - 1);
      Point2I max(pos.x + width / 2 + 2, pos.y + mProfile->mFont->getHeight() + 1);
      
      dglDrawRectFill(min, max, mPopupBackgroundColor);
   }
   
	dglSetBitmapModulation(mPopupTextColor);
   dglDrawText(mProfile->mFont, Point2I(pos.x - width / 2, pos.y), buf);
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// render the handle/text/...
void WorldEditor::renderScreenObj(SceneObject * obj, Point2I sPos)
{
   // do not render control object stuff
   if(obj == getControlObject() || obj->isHidden())
      return;
      
   ClassInfo::Entry * entry = getClassEntry(obj);
   if(!entry)
      entry = &mDefaultClassEntry;
      
   TextureObject * bitmap;
   if(mRenderObjHandle)
   {      
      // offset
      if(obj->isLocked())
         bitmap = entry->mLockedHandle ? entry->mLockedHandle : mDefaultClassEntry.mLockedHandle;
      else
      {
         if(mSelected.objInSet(obj))
            bitmap = entry->mSelectHandle ? entry->mSelectHandle : mDefaultClassEntry.mSelectHandle;
         else
            bitmap = entry->mDefaultHandle ? entry->mDefaultHandle : mDefaultClassEntry.mDefaultHandle;
      }

      sPos.x -= (bitmap->bitmapWidth / 2);
      sPos.y -= (bitmap->bitmapHeight / 2);
      dglClearBitmapModulation();
      dglDrawBitmap(bitmap, sPos);
   }
   
   //
   if(mRenderObjText)
   {

      const char * str = parseObjectFormat(obj, mObjTextFormat);

      Point2I extent(mProfile->mFont->getStrWidth(str), mProfile->mFont->getHeight());

      Point2I pos(sPos);

      if(mRenderObjHandle)
      {
         pos.x += (bitmap->bitmapWidth / 2) - (extent.x / 2);
         pos.y += (bitmap->bitmapHeight / 2) + 3;
      }
		dglSetBitmapModulation(mObjectTextColor);
      dglDrawText(mProfile->mFont, pos, str);
   }
}

//------------------------------------------------------------------------------
// axis gizmo stuff

void WorldEditor::calcAxisInfo()
{
   if(!mSelected.size())
      return;

   // get the centroid..
   mSelected.invalidateCentroid();
   Point3F centroid = mObjectsUseBoxCenter ? mSelected.getBoxCentroid() : mSelected.getCentroid();
   mAxisGizmoCenter = centroid;
   
   VectorF axisVector[3] = {
      VectorF(1,0,0),
      VectorF(0,1,0),
      VectorF(0,0,1)
   };

   // adjust to object space if just one object...
   if((mSelected.size() == 1) && !((mLastMouseEvent.modifier & SI_SHIFT) && (mCurrentMode == Move)))
   {
      const MatrixF & mat = mSelected[0]->getTransform();
      for(U32 i = 0; i < 3; i++)
      {
         VectorF tmp;
         mat.mulV(axisVector[i], &tmp);
         mAxisGizmoVector[i] = tmp;
         mAxisGizmoVector[i].normalizeSafe();
      }
   }
   else
      for(U32 i = 0; i < 3; i++)
         mAxisGizmoVector[i] = axisVector[i];

   // get the projected size...
   SceneObject * obj = getControlObject();
   if(!obj)
      return;

   //
   Point3F camPos;
   obj->getTransform().getColumn(3, &camPos);

   // assumes a 90deg FOV
   Point3F dir = mAxisGizmoCenter - camPos;
   mAxisGizmoProjLen = (F32(mAxisGizmoMaxScreenLen) / F32(getExtent().x)) * dir.magnitudeSafe() * mTan(mDegToRad(45.0));
}         

bool WorldEditor::collideAxisGizmo(const Gui3DMouseEvent & event)
{
   if(!mAxisGizmoActive || !mSelected.size())
      return(false);

   // get the projected size...
   SceneObject * obj = getControlObject();
   if(!obj)
      return(false);

   //
   Point3F camPos;
   obj->getTransform().getColumn(3, &camPos);

   // assumes a 90deg FOV
   Point3F dir = mAxisGizmoCenter - camPos;
   mAxisGizmoProjLen = (F32(mAxisGizmoMaxScreenLen) / F32(getExtent().x)) * dir.magnitudeSafe() * mTan(mDegToRad(45.0));

   dir.normalizeSafe();

   mAxisGizmoSelAxis = -1;

   // find axis to use...
   for(U32 i = 0; i < 3; i++)
   {
      VectorF up, normal;
      mCross(dir, mAxisGizmoVector[i], &up);
      mCross(up, mAxisGizmoVector[i], &normal);

      if(normal.isZero())
         break;

      PlaneF plane(mAxisGizmoCenter, normal);

      // width of the axis poly is 1/10 the run   
      Point3F a = up * mAxisGizmoProjLen / 10;
      Point3F b = mAxisGizmoVector[i] * mAxisGizmoProjLen;

      Point3F poly [] = {
         Point3F(mAxisGizmoCenter + a),
         Point3F(mAxisGizmoCenter + a + b),
         Point3F(mAxisGizmoCenter - a + b),
         Point3F(mAxisGizmoCenter - a)
      };

      Point3F end = camPos + event.vec * mProjectDistance;
   	F32 t = plane.intersect(camPos, end);
      if(t >= 0 && t <= 1)
      {
         Point3F pos;
         pos.interpolate(camPos, end, t);

         // check if inside our 'poly' of this axis vector...
         bool inside = true;
         for(U32 j = 0; inside && (j < 4); j++)
         {
            U32 k = (j+1) % 4;
            VectorF vec1 = poly[k] - poly[j];
            VectorF vec2 = pos - poly[k];

            if(mDot(vec1, vec2) > 0.f)
               inside = false;   
         }

         //
         if(inside)
         {
            mAxisGizmoSelAxis = i;
            return(true);
         }
      }
   }      

   // default to Z
   mAxisGizmoSelAxis = 2;
   return(false);
}

//------------------------------------------------------------------------------

void WorldEditor::renderAxisGizmo()
{
   calcAxisInfo();

   ColorI axisColors[3] = {
      ColorI(0xff, 0x00, 0x00),
      ColorI(0x00, 0xff, 0x00),
      ColorI(0x00, 0x00, 0xff)
   };

   ColorI selectColor(0xff, 0xff, 0x00);

   glDisable(GL_CULL_FACE);
   glDisable(GL_DEPTH_TEST);
   glBegin(GL_LINES);

   // render each of them...
   for(U32 i = 0; i < 3; i++)
   {
      Point3F & centroid = mAxisGizmoCenter;
      if(i == mAxisGizmoSelAxis)
         glColor3ub(selectColor.red, selectColor.green, selectColor.blue);
      else
         glColor3ub(axisColors[i].red, axisColors[i].green, axisColors[i].blue);
      glVertex3f(centroid.x, centroid.y, centroid.z);
      glVertex3f(centroid.x + mAxisGizmoVector[i].x * mAxisGizmoProjLen,
                 centroid.y + mAxisGizmoVector[i].y * mAxisGizmoProjLen,
                 centroid.z + mAxisGizmoVector[i].z * mAxisGizmoProjLen);
   }

   glEnd();
   glEnable(GL_DEPTH_TEST);
}

void WorldEditor::renderAxisGizmoText()
{
   char axisText[] = "xyz";

   for(U32 i = 0; i < 3; i++)
   {
      const Point3F & centroid = mAxisGizmoCenter;
      Point3F pos(centroid.x + mAxisGizmoVector[i].x * mAxisGizmoProjLen,
                  centroid.y + mAxisGizmoVector[i].y * mAxisGizmoProjLen,
                  centroid.z + mAxisGizmoVector[i].z * mAxisGizmoProjLen);

      Point3F sPos;
      if(project(pos, &sPos))
      {
         char buf[2];
         buf[0] = axisText[i]; buf[1] = '\0';
         dglSetBitmapModulation(mObjectTextColor);
         dglDrawText(mProfile->mFont, Point2I(sPos.x, sPos.y), buf);
      }
   }
}

//------------------------------------------------------------------------------

Point3F WorldEditor::snapPoint(const Point3F & pnt)
{
   if(!mSnapToGrid)
      return(pnt);
      
         
   Point3F snap;
   snap.x = snapFloat(pnt.x, mGridSize.x);
   snap.y = snapFloat(pnt.y, mGridSize.y);
   snap.z = snapFloat(pnt.z, mGridSize.z);

   return(snap);
}

//------------------------------------------------------------------------------
// ClassInfo stuff

WorldEditor::ClassInfo::~ClassInfo()
{
   for(U32 i = 0; i < mEntries.size(); i++)
      delete mEntries[i];
}

bool WorldEditor::objClassIgnored(const SceneObject * obj)
{
   ClassInfo::Entry * entry = getClassEntry(obj);
   if(mToggleIgnoreList)
      return(!(entry ? entry->mIgnoreCollision : false));
   else
      return(entry ? entry->mIgnoreCollision : false);
}

WorldEditor::ClassInfo::Entry * WorldEditor::getClassEntry(StringTableEntry name)
{
   AssertFatal(name, "WorldEditor::getClassEntry - invalid args");
   for(U32 i = 0; i < mClassInfo.mEntries.size(); i++)
      if(!dStricmp(name, mClassInfo.mEntries[i]->mName))
         return(mClassInfo.mEntries[i]);
   return(0);
}

WorldEditor::ClassInfo::Entry * WorldEditor::getClassEntry(const SceneObject * obj)
{
   AssertFatal(obj, "WorldEditor::getClassEntry - invalid args");
   return(getClassEntry(obj->getClassName()));
}

bool WorldEditor::addClassEntry(ClassInfo::Entry * entry)
{
   AssertFatal(entry, "WorldEditor::addClassEntry - invalid args");
   if(getClassEntry(entry->mName))
      return(false);

   mClassInfo.mEntries.push_back(entry);
   return(true);
}

//------------------------------------------------------------------------------
// Mouse cursor stuff

bool WorldEditor::grabCursors()
{
   struct _cursorInfo {
      U32 index;
      const char * name;
   } infos[] = {
      {HandCursor,         "Editor_HandCursor"},
      {RotateCursor,       "Editor_RotateCursor"},
      {ScaleCursor,        "Editor_RotateCursor"},
      {MoveCursor,         "Editor_MoveCursor"},
      {ArrowCursor,        "Editor_ArrowCursor"},
      {DefaultCursor,      "DefaultCursor"},
   };
   
   //
   for(U32 i = 0; i < (sizeof(infos) / sizeof(infos[0])); i++)
   {
      SimObject * obj = Sim::findObject(infos[i].name);
      if(!obj)
      {
         Con::errorf(ConsoleLogEntry::Script, "WorldEditor::grabCursors: failed to find cursor '%s'.", infos[i].name);
         return(false);
      }
      
      GuiCursor *cursor = dynamic_cast<GuiCursor*>(obj);
      if(!cursor)
      {
         Con::errorf(ConsoleLogEntry::Script, "WorldEditor::grabCursors: object is not a cursor '%s'.", infos[i].name);
         return(false);
      }
      
      //   
      mCursors[infos[i].index] = cursor;
   }
   
   //
   mCurrentCursor = mCursors[DefaultCursor];
   return(true);
}

void WorldEditor::setCursor(U32 cursor)
{
   AssertFatal(cursor < NumCursors, "WorldEditor::setCursor: invalid cursor");

   mCurrentCursor = mCursors[cursor];
   Canvas->setCursor(mCurrentCursor);
}

GuiCursor * WorldEditor::getCursor()
{
   AssertFatal(mCurrentCursor, "WorldEditor::getCursor: no cursor to get!");
   return(mCurrentCursor);
}

//------------------------------------------------------------------------------

WorldEditor::WorldEditor()
{
   // init the field data
   mPlanarMovement = true;
   mUndoLimit = 40;
   mDropType = DropAtScreenCenter;
   mProjectDistance = 2000.f;
   mBoundingBoxCollision = true;
   mRenderPlane = true;
   mRenderPlaneHashes = true;
   mGridColor.set(255,255,255,20);
   mPlaneDim = 500;
   mGridSize.set(10,10,10);
   mRenderPopupBackground = true;
   mPopupBackgroundColor.set(100,100,100);
   mPopupTextColor.set(255,255,0);
   mSelectHandle = StringTable->insert("gui/Editor_SelectHandle.png");
   mDefaultHandle = StringTable->insert("gui/Editor_DefaultHandle.png");
   mLockedHandle = StringTable->insert("gui/Editor_LockedHandle.png");
   mObjectTextColor.set(255,255,255);
   mObjectsUseBoxCenter = true;
   mAxisGizmoMaxScreenLen = 200;
   mAxisGizmoActive = true;
   mMouseMoveScale = 0.2f;
   mMouseRotateScale = 0.01f;
   mMouseScaleScale = 0.01f;
   mMinScaleFactor = 0.1f;
   mMaxScaleFactor = 4000.f;
   mObjSelectColor.set(255,0,0);
   mObjMouseOverSelectColor.set(0,0,255);
   mObjMouseOverColor.set(0,255,0);
   mShowMousePopupInfo = true;
   mDragRectColor.set(255,255,0);
   mRenderObjText = true;
   mRenderObjHandle = true;
   mObjTextFormat = StringTable->insert("$id$: $name$");
   mFaceSelectColor.set(0,0,100,100);
   mRenderSelectionBox = true;
   mSelectionBoxColor.set(255,255,0);
   mSelectionLocked = false;
   mSnapToGrid = false;
   mSnapRotations = false;
   mRotationSnap = 15.f;
   mToggleIgnoreList = false;
   mRenderNav = false;

   mRedirectID = 0;

   //
   mHitInfo.obj = 0;
   mHitObject = mHitInfo.obj;

   //
   mDefaultMode = mCurrentMode = Move;
   mCurrentCursor = 0;
   mMouseDown = false;
   mDragSelect = false;

   //
   mSelected.autoSelect(true);
   mDragSelected.autoSelect(false);
}

WorldEditor::~WorldEditor()
{
   clearUndo(mUndoList);
   clearUndo(mRedoList);
}

//------------------------------------------------------------------------------

bool WorldEditor::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   // grab all the cursors
    if(!grabCursors())
      return(false);
     
   // create the default class entry
   mDefaultClassEntry.mName = 0;
   mDefaultClassEntry.mIgnoreCollision = false;
   mDefaultClassEntry.mDefaultHandle = TextureHandle(mDefaultHandle);
   mDefaultClassEntry.mSelectHandle = TextureHandle(mSelectHandle);
   mDefaultClassEntry.mLockedHandle = TextureHandle(mLockedHandle);
   
   if(!(mDefaultClassEntry.mDefaultHandle && mDefaultClassEntry.mSelectHandle && mDefaultClassEntry.mLockedHandle))
      return(false);
    
   return(true);
}

//------------------------------------------------------------------------------

void WorldEditor::onEditorEnable()
{
   // go through and copy the hidden field to the client objects...
   for(SimSetIterator itr(Sim::getRootGroup());  *itr; ++itr)
   {
      SceneObject * obj = dynamic_cast<SceneObject *>(*itr);
      if(!obj)
         continue;

      // only work with a server obj...
      if(obj->isClientObject())
         continue;

      // grab the client object
      SceneObject * clientObj = getClientObj(obj);
      if(!clientObj)
         continue;

      //
      clientObj->setHidden(obj->isHidden());
   }
}

//------------------------------------------------------------------------------

void WorldEditor::on3DMouseMove(const Gui3DMouseEvent & event)
{
   setCursor(ArrowCursor);
   mHitInfo.obj = 0;

   //
   if(collideAxisGizmo(event))
      setCursor(HandCursor);
   else
   {
      CollisionInfo info;
      if(collide(event, info) && !objClassIgnored(info.obj))
      {
         setCursor(HandCursor);
         mHitInfo = info;
      }
   }

   //
   mHitObject = mHitInfo.obj;

   mLastMouseEvent = event;
}

void WorldEditor::on3DMouseDown(const Gui3DMouseEvent & event)
{
   mMouseDown = true;
   mMouseDragged = false;
   mLastRotation = 0.f;

   mouseLock();

   // use ctrl to toggle vertical movement
   mUseVertMove = (event.modifier & SI_CTRL);

   // check gizmo first
   mUsingAxisGizmo = false;
   if(collideAxisGizmo(event))
   {
      mUsingAxisGizmo = true;
      mHitMousePos = event.mousePoint;
      mHitCentroid = mSelected.getCentroid();
      mHitRotation = extractEuler(mSelected[0]->getTransform());
      mHitObject = mSelected[0];
   }
   else
   {
      CollisionInfo info;
      if(collide(event, info) && !objClassIgnored(info.obj))
      {
         if(!mSelectionLocked)
         {
            if(!(event.modifier & SI_CTRL))
            {
               if(!(event.modifier & SI_SHIFT) && !mSelected.objInSet(info.obj))
                  mSelected.clear();
               mSelected.addObject(info.obj);
            }
         }

         if(event.mouseClickCount > 1)
         {
            //
            char buf[16];
            dSprintf(buf, sizeof(buf), "%d", info.obj->getId());

            SimObject * obj = 0;
            if(mRedirectID)
               obj = Sim::findObject(mRedirectID);
            Con::executef(obj ? obj : this, 2, "onDblClick", buf);
         }

         mHitInfo = info;
         mHitObject = mHitInfo.obj;
         mHitOffset = info.pos - mSelected.getCentroid();
         mHitRotation = extractEuler(mHitObject->getTransform());
         mHitMousePos = event.mousePoint;
         mHitCentroid = mSelected.getCentroid();
      }
      else if(!mSelectionLocked)
      {
         if(!(event.modifier & SI_SHIFT))
            mSelected.clear();

         mDragSelect = true;
         mDragSelected.clear();
         mDragRect.set(Point2I(event.mousePoint), Point2I(0,0));
         mDragStart = event.mousePoint;
      }
   }

   mLastMouseEvent = event;
}

void WorldEditor::on3DMouseUp(const Gui3DMouseEvent & event)
{
   mMouseDown = false;
   mUsingAxisGizmo = false;
   
   // check if selecting objects....
   if(mDragSelect)
   {
      mDragSelect = false;
      
      // add all the objects from the drag selection into the normal selection
      for(U32 i = 0; i < mDragSelected.size(); i++)
         mSelected.addObject(mDragSelected[i]);
      mDragSelected.clear();

      mouseUnlock();
      return;
   }
   
   mHitInfo.obj = 0;

   //   
   if(collideAxisGizmo(event))
      setCursor(HandCursor);
   else
   {
      CollisionInfo info;
      if(collide(event, info) && !objClassIgnored(info.obj))
      {
         setCursor(HandCursor);

         if(!mMouseDragged && !mSelectionLocked)
         {
            if(event.modifier & SI_CTRL)
            {
               if(mSelected.objInSet(info.obj))
                  mSelected.removeObject(info.obj);
               else
                  mSelected.addObject(info.obj);
            }
            else if(!(event.modifier & SI_SHIFT))
            {
               mSelected.clear();
               mSelected.addObject(info.obj);
            }
         }

         mHitInfo = info;
      }
      else
         setCursor(ArrowCursor);
   }

   //
   mHitObject = mHitInfo.obj;
   mouseUnlock();
}

void WorldEditor::on3DMouseDragged(const Gui3DMouseEvent & event)
{
   if(!mMouseDown)
      return;
      
   //   
   if(!mMouseDragged)
   {
      if(!mUsingAxisGizmo)
      {
         // vert drag on new object.. reset hit offset
         if(bool(mHitObject) && !mSelected.objInSet(mHitObject))
         {
            if(!mSelectionLocked)
               mSelected.addObject(mHitObject);
               
            mHitOffset = mHitInfo.pos - mSelected.getCentroid();
         }
      }

      // create and add an undo state
      if(!mDragSelect)
      {
         addUndo(mUndoList, createUndo(mSelected));
         clearUndo(mRedoList);
      }
      
      mMouseDragged = true;
   }
   
   // update the drag selection
   if(mDragSelect)
   {
      // build the drag selection on the renderScene method - make sure no neg extent!
      mDragRect.point.x = (event.mousePoint.x < mDragStart.x) ? event.mousePoint.x : mDragStart.x;
      mDragRect.extent.x = (event.mousePoint.x > mDragStart.x) ? event.mousePoint.x - mDragStart.x : mDragStart.x - event.mousePoint.x;
      mDragRect.point.y = (event.mousePoint.y < mDragStart.y) ? event.mousePoint.y : mDragStart.y;
      mDragRect.extent.y = (event.mousePoint.y > mDragStart.y) ? event.mousePoint.y - mDragStart.y : mDragStart.y - event.mousePoint.y;
      return;
   }

   if(!mUsingAxisGizmo && (!bool(mHitObject) || !mSelected.objInSet(mHitObject)))
      return;

   // anything locked?
   for(U32 i = 0; i < mSelected.size(); i++)
      if(mSelected[i]->isLocked())
         return;

   // do stuff
   switch(mCurrentMode)
   {
      case Move:
      {
         setCursor(MoveCursor);
         
         // grabbed axis gizmo?
         if(mUsingAxisGizmo)
         {
            F32 offset = (event.mousePoint.x - mHitMousePos.x) * mMouseMoveScale;
            Point3F projPnt = mHitCentroid;

            for(S32 i = 0; i < 3; i++)
               if(i == mAxisGizmoSelAxis)
                  ((F32*)projPnt)[i] += offset;

            // 
            if((mSelected.size() == 1) && !(event.modifier & SI_SHIFT))
            {
               MatrixF mat = mSelected[0]->getTransform();
               Point3F offset;
               mat.mulV(projPnt - mHitCentroid, &offset);

               mSelected.offset(offset + mHitCentroid - mSelected.getCentroid());
            }
            else 
            {
               // snap to the selected axis
               Point3F snap = snapPoint(projPnt);
               ((F32*)projPnt)[mAxisGizmoSelAxis] = ((F32*)snap)[mAxisGizmoSelAxis];
               mSelected.offset(projPnt - mSelected.getCentroid());
            }

            updateClientTransforms(mSelected);
         }
         else
         {
            // ctrl modifier movement?
            if(mUseVertMove)
            {
               if(mPlanarMovement)
               {
                  // do a projection onto the z axis
                  F64 pDist = mSqrt((event.pos.x - mHitInfo.pos.x) * (event.pos.x - mHitInfo.pos.x) +
                     (event.pos.y - mHitInfo.pos.y) * (event.pos.y - mHitInfo.pos.y));

                  Point3F vec(mHitInfo.pos.x - event.pos.x, mHitInfo.pos.y - event.pos.y, 0.f);
                  vec.normalizeSafe();
   
                  F64 projDist = mDot(event.vec, vec);
                  if(projDist == 0.f)
                     return;
      
                  F64 scale = pDist / projDist;
                  vec = event.pos + (event.vec * scale);
   
                  vec.x = mHitInfo.pos.x;
                  vec.y = mHitInfo.pos.y;
                  mSelected.offset(vec - mSelected.getCentroid() - mHitOffset);
                  updateClientTransforms(mSelected);
               }
               else
               {
                  // do a move on the z axis
                  F32 diff = mLastMouseEvent.mousePoint.x - event.mousePoint.x;
                  F32 offset = diff * mMouseMoveScale;
                  Point3F projPnt = mSelected.getCentroid();
                  projPnt.z += offset;

                  // snap just to z axis
                  Point3F snapped = snapPoint(projPnt);
                  projPnt.z = snapped.z;

                  mSelected.offset(projPnt - mSelected.getCentroid());
                  updateClientTransforms(mSelected);
               }
            }
            else
            {
               // move on XY plane
               if(mPlanarMovement)
               {
                  // on z
                  F32 cos = mDot(event.vec, Point3F(0,0,-1));
                  F32 a = event.pos.z - mHitInfo.pos.z;
                  if(cos != 0.f)
                  {
                     F32 c = a / cos;
      
                     Point3F projPnt = event.vec * c;
                     projPnt += event.pos;
                     projPnt -= mHitOffset;

                     //
                     F32 z = projPnt.z;            
                     projPnt = snapPoint(projPnt);
                     projPnt.z = z;
      
                     mSelected.offset(projPnt - mSelected.getCentroid());
                     updateClientTransforms(mSelected);
                  }
               }
               else
               {  
                  // offset the pnt of collision - no snapping involved
                  mSelected.disableCollision();
      
                  CollisionInfo info;
                  if(collide(event, info))
                  {
                     mSelected.offset(info.pos - mSelected.getCentroid() - mHitOffset);
                     updateClientTransforms(mSelected);
                  }
         
                  mSelected.enableCollision();
               }
            }
         }
         break;
      }
      
      case Scale:
      {
         setCursor(ScaleCursor);

         // can scale only single selections
         if(mSelected.size() > 1)
            break;

         if(mUsingAxisGizmo)
         {
            // 
            F32 diff = mLastMouseEvent.mousePoint.x - event.mousePoint.x;

            // offset the correct axis
            EulerF curScale = ((SceneObject*)mSelected[0])->getScale();
            EulerF scale;

            F32 * pCurScale = ((F32*)curScale);
            F32 * pScale = ((F32*)scale);

            for(S32 i = 0; i < 3; i++)
            {
               if(i == mAxisGizmoSelAxis)
                  pScale[i] = 1.f + (diff * mMouseScaleScale);
               else
                  pScale[i] = 1.f;

               // clamp
               if(pCurScale[i] * pScale[i] < mMinScaleFactor)
                  pScale[i] = mMinScaleFactor / pCurScale[i];
               if(pCurScale[i] * pScale[i] > mMaxScaleFactor)
                  pScale[i] = mMaxScaleFactor / pCurScale[i];
            }

            mSelected.scale(scale);
            updateClientTransforms(mSelected);
         }
         else
         {
            if(!mBoundingBoxCollision)
            {
               // the hit normal is not useful, enable bounding box
               // collision and get the collided face/normal
               CollisionInfo info;
               
               mBoundingBoxCollision = true;
               bool hit = collide(event, info);
               mBoundingBoxCollision = false;

               // hit and the hit object?
               if(!hit || (info.obj != (SceneObject*)mHitObject))
                  break;

               mHitInfo = info;
            }

            VectorF normal;   
            mCross(mHitInfo.normal, event.vec, &normal);
         
            VectorF planeNormal;
            mCross(mHitInfo.normal, normal, &planeNormal);
         
            PlaneF plane;
            plane.set(mHitInfo.pos, planeNormal);
         
            Point3F vecProj = event.pos + event.vec * mProjectDistance;
            F32 t = plane.intersect(event.pos, vecProj);
            if(t < 0.f || t > 1.f)
               break;
            
            Point3F pnt;
            pnt.interpolate(event.pos, vecProj, t);
         
            // figure out which axis we are working with, then
            // find the distance to the correct face on the bbox
            VectorF objNorm;
            const MatrixF & worldMat = mHitObject->getWorldTransform();
            worldMat.mulV(mHitInfo.normal, &objNorm);

            U32 normIndex = getBoxNormalIndex(objNorm);

            Box3F box = mHitObject->getObjBox();
            VectorF curScale = mHitObject->getScale();

            // scale and transform the bbox
            Point3F size = box.max - box.min;
         
            box.min.convolve(curScale);
            box.max.convolve(curScale);
         
            Box3F projBox;
            const MatrixF & objMat = mHitObject->getTransform();
            objMat.mulP(box.min, &projBox.min);
            objMat.mulP(box.max, &projBox.max);
         
            // if positive normal then grab max, else min
            Point3F boxPnt;
            Point3F offset;
            if(normIndex & 0x01)
            {
               boxPnt = projBox.max;
               worldMat.mulV(mSelected.getCentroid() - projBox.min, &offset);
            }
            else
            {
               boxPnt = projBox.min;
               worldMat.mulV(mSelected.getCentroid() - projBox.max, &offset);
            }

            plane.set(boxPnt, mHitInfo.normal);
            F32 dist = plane.distToPlane(pnt);
         
            // set the scale for the correct axis
            Point3F scale;
            F32 * pScale = ((F32*)scale);
            F32 * pSize = ((F32*)size);
            F32 * pCurScale = ((F32*)curScale);
            F32 * pOffset = ((F32*)offset);

            for(U32 i = 0; i < 3; i++)
            {
               if((normIndex >> 1) == i)
               {  
                  // get the new scale
                  pScale[i] = (pSize[i] * pCurScale[i] + dist) / pSize[i];
            
                  // clamp
                  if(pScale[i] < mMinScaleFactor)
                     pScale[i] = mMinScaleFactor;
                  if(pScale[i] > mMaxScaleFactor)
                     pScale[i] = mMaxScaleFactor;

                  pOffset[i] = pScale[i] / pCurScale[i] * pOffset[i] - pOffset[i];
               }
               else
               {
                  pScale[i] = pCurScale[i];
                  pOffset[i] = 0;
               }
            }
         
            objMat.mulV(offset, &pnt);
            mSelected.offset(pnt);
            mHitObject->setScale(scale);
            updateClientTransforms(mSelected);
         }
         
         break;
      }
      
      case Rotate:
      {
         setCursor(RotateCursor);

         // default to z axis
         S32 axis = 2;
         if(mUsingAxisGizmo)
            axis = mAxisGizmoSelAxis;

         F32 angle = (event.mousePoint.x - mHitMousePos.x) * mMouseRotateScale;   
         
         //
         if(mSnapRotations)
         {
            angle = mDegToRad(snapFloat(mRadToDeg(angle), mRotationSnap));
            if(mSelected.size() == 1)
               angle -= ((F32*)mHitRotation)[axis];
         }

         EulerF rot(0.f, 0.f, 0.f);
         ((F32*)rot)[axis] = (angle - mLastRotation);

         Point3F centroid = mObjectsUseBoxCenter ? mSelected.getBoxCentroid() : mSelected.getCentroid();
         
         mSelected.rotate(rot, centroid);
         updateClientTransforms(mSelected);

         mLastRotation = angle;

         break;
      }
   }
   mLastMouseEvent = event;
}

void WorldEditor::on3DMouseEnter(const Gui3DMouseEvent &)
{
   mMouseDown = false;
}

void WorldEditor::on3DMouseLeave(const Gui3DMouseEvent &)
{
   mMouseDown = false;
   mDragSelect = false;
   mDragSelected.clear();
   mMouseDragged = false;
   mUsingAxisGizmo = false;

   mHitInfo.obj = 0;
   mHitObject = mHitInfo.obj;

   setCursor(DefaultCursor);
}

void WorldEditor::on3DRightMouseDown(const Gui3DMouseEvent & event)
{
   CollisionInfo info;
   if(collide(event, info) && !objClassIgnored(info.obj))
   {
      if(!mSelected.objInSet(info.obj) && !mSelectionLocked)
      {
         mSelected.clear();
         mSelected.addObject(info.obj);
      }

      mHitInfo = info;
      mHitObject = mHitInfo.obj;
   }
   else
   {
      if(!mSelectionLocked)
         mSelected.clear();

      mHitInfo.obj = 0;
      mHitObject = mHitInfo.obj;
   }
}

void WorldEditor::on3DRightMouseUp(const Gui3DMouseEvent & event)
{
   char buf[32];
   dSprintf(buf, sizeof(buf), "%d %d", event.mousePoint.x, event.mousePoint.y);
   
   SimObject * obj = 0;
   if(mRedirectID)
      obj = Sim::findObject(mRedirectID);
   Con::executef(obj ? obj : this, 2, "onContextMenu", buf);
}

//------------------------------------------------------------------------------

void WorldEditor::updateGuiInfo()
{
   SimObject * obj = 0;
   if(mRedirectID)
      obj = Sim::findObject(mRedirectID);

   char buf[] = "";
   Con::executef(obj ? obj : this, 2, "onGuiUpdate", buf);
}

//------------------------------------------------------------------------------

static void findObjectsCallback(SceneObject* obj, S32 val)
{  
   Vector<SceneObject*> * list = (Vector<SceneObject*>*)val;
   list->push_back(obj);
}

void WorldEditor::renderScene(const RectI & updateRect)
{
   glEnable(GL_DEPTH_TEST);
   
   // navGraph 
   if(mRenderNav)
   {
      NavigationGraph *ng = dynamic_cast<NavigationGraph*>(Sim::findObject("NavGraph"));
      if(ng)
         ng->render(smCamPos, true);
   }   
   
   glDepthFunc(GL_LEQUAL);

   // just walk the selected   
   U32 i;
   for(i = 0; i < mSelected.size(); i++)
   {
      if((const SceneObject *)mHitObject == mSelected[i])
         continue;
      renderObjectBox(mSelected[i], mObjSelectColor);
   }

   // do the drag selection
   for(i = 0; i < mDragSelected.size(); i++)
      renderObjectBox(mDragSelected[i], mObjSelectColor);
   
   // draw the mouse over obj
   if(bool(mHitObject))
   {
      ColorI & col = mSelected.objInSet(mHitObject) ? mObjMouseOverSelectColor : mObjMouseOverColor;
      renderObjectBox(mHitObject, col);

      if(mCurrentMode == Scale && !mUsingAxisGizmo && mSelected.size() == 1)
         renderObjectFace(mHitObject, mHitInfo.normal, mFaceSelectColor);
   }

   // stuff to do if there is a selection
   if(mSelected.size())
   {
      if(mAxisGizmoActive)
         renderAxisGizmo();

      renderSelectionWorldBox(mSelected);
      renderPlane(mObjectsUseBoxCenter ? mSelected.getBoxCentroid() : mSelected.getCentroid());
   }
   
   // draw the handles and text's now...
   dglSetClipRect(updateRect);

   if(mSelected.size() && mAxisGizmoActive)
      renderAxisGizmoText();
      
   // update what is in the selction      
   if(mDragSelect)
      mDragSelected.clear();

   //
   Vector<SceneObject *> objects;
   gServerContainer.findObjects(-1, findObjectsCallback, (S32)&objects);
   for(i = 0; i < objects.size(); i++)
   {
      SceneObject * obj = objects[i];
      if(objClassIgnored(obj))
         continue;
         
      Point3F wPos;
      if(mObjectsUseBoxCenter)
         wPos = getBoundingBoxCenter(obj);
      else
         obj->getTransform().getColumn(3, &wPos);
   
      Point3F sPos;
      if(project(wPos, &sPos))
      {
         // check if object needs to be added into the regions select
         if(mDragSelect)
            if(mDragRect.pointInRect(Point2I(sPos.x, sPos.y)) && !mSelected.objInSet(obj))
               mDragSelected.addObject(obj);

         //
         renderScreenObj(obj, Point2I(sPos.x, sPos.y));
      }
   }
   
   //
   if(mShowMousePopupInfo && mMouseDown)
      renderMousePopupInfo();

   // seletion box
   if(mDragSelect)
      dglDrawRect(mDragRect, mDragRectColor);
      
}

//------------------------------------------------------------------------------
// Console stuff

static EnumTable::Enums dropEnums[] =
{
	{ WorldEditor::DropAtOrigin,           "atOrigin"        },
   { WorldEditor::DropAtCamera,           "atCamera"        },
   { WorldEditor::DropAtCameraWithRot,    "atCameraRot"     },
   { WorldEditor::DropBelowCamera,        "belowCamera"     },
   { WorldEditor::DropAtScreenCenter,     "screenCenter"    },
   { WorldEditor::DropAtCentroid,         "atCentroid"      },
   { WorldEditor::DropToGround,           "toGround"        }
};
static EnumTable gEditorDropTable(7, &dropEnums[0]);

void WorldEditor::initPersistFields()
{
   Parent::initPersistFields();
   addField("planarMovement", TypeBool, Offset(mPlanarMovement, WorldEditor));
   addField("undoLimit", TypeS32, Offset(mUndoLimit, WorldEditor));
   addField("dropType", TypeEnum, Offset(mDropType, WorldEditor), 1, &gEditorDropTable);
   addField("projectDistance", TypeF32, Offset(mProjectDistance, WorldEditor));
   addField("boundingBoxCollision", TypeBool, Offset(mBoundingBoxCollision, WorldEditor));
   addField("renderPlane", TypeBool, Offset(mRenderPlane, WorldEditor));
   addField("renderPlaneHashes", TypeBool, Offset(mRenderPlaneHashes, WorldEditor));
   addField("gridColor", TypeColorI, Offset(mGridColor, WorldEditor)); 
   addField("planeDim", TypeF32, Offset(mPlaneDim, WorldEditor));
   addField("gridSize", TypePoint3F, Offset(mGridSize, WorldEditor));
   addField("renderPopupBackground", TypeBool, Offset(mRenderPopupBackground, WorldEditor));
   addField("popupBackgroundColor", TypeColorI, Offset(mPopupBackgroundColor, WorldEditor));
   addField("popupTextColor", TypeColorI, Offset(mPopupTextColor, WorldEditor));
   addField("selectHandle", TypeString, Offset(mSelectHandle, WorldEditor));
   addField("defaultHandle", TypeString, Offset(mDefaultHandle, WorldEditor));
   addField("lockedHandle", TypeString, Offset(mLockedHandle, WorldEditor));
   addField("objectTextColor", TypeColorI, Offset(mObjectTextColor, WorldEditor));
   addField("objectsUseBoxCenter", TypeBool, Offset(mObjectsUseBoxCenter, WorldEditor));
   addField("axisGizmoMaxScreenLen", TypeS32, Offset(mAxisGizmoMaxScreenLen, WorldEditor));
   addField("axisGizmoActive", TypeBool, Offset(mAxisGizmoActive, WorldEditor));
   addField("mouseMoveScale", TypeF32, Offset(mMouseMoveScale, WorldEditor));
   addField("mouseRotateScale", TypeF32, Offset(mMouseRotateScale, WorldEditor));
   addField("mouseScaleScale", TypeF32, Offset(mMouseScaleScale, WorldEditor));
   addField("minScaleFactor", TypeF32, Offset(mMinScaleFactor, WorldEditor));
   addField("maxScaleFactor", TypeF32, Offset(mMaxScaleFactor, WorldEditor));
   addField("objSelectColor", TypeColorI, Offset(mObjSelectColor, WorldEditor));
   addField("objMouseOverSelectColor", TypeColorI, Offset(mObjMouseOverSelectColor, WorldEditor));
   addField("objMouseOverColor", TypeColorI, Offset(mObjMouseOverColor, WorldEditor));
   addField("showMousePopupInfo", TypeBool, Offset(mShowMousePopupInfo, WorldEditor));
   addField("dragRectColor", TypeColorI, Offset(mDragRectColor, WorldEditor));
   addField("renderObjText", TypeBool, Offset(mRenderObjText, WorldEditor));
   addField("renderObjHandle", TypeBool, Offset(mRenderObjHandle, WorldEditor));
   addField("objTextFormat", TypeString, Offset(mObjTextFormat, WorldEditor));
   addField("faceSelectColor", TypeColorI, Offset(mFaceSelectColor, WorldEditor));
   addField("renderSelectionBox", TypeBool, Offset(mRenderSelectionBox, WorldEditor));
   addField("selectionBoxColor", TypeColorI, Offset(mSelectionBoxColor, WorldEditor));
   addField("selectionLocked", TypeBool, Offset(mSelectionLocked, WorldEditor));
   addField("snapToGrid", TypeBool, Offset(mSnapToGrid, WorldEditor));
   addField("snapRotations", TypeBool, Offset(mSnapRotations, WorldEditor));
   addField("rotationSnap", TypeF32, Offset(mRotationSnap, WorldEditor));
   addField("toggleIgnoreList", TypeBool, Offset(mToggleIgnoreList, WorldEditor));
   addField("renderNav", TypeBool, Offset(mRenderNav, WorldEditor));
}

//------------------------------------------------------------------------------

void WorldEditor::consoleInit()
{
   Con::addCommand("WorldEditor", "ignoreObjClass", cIgnoreObjClass, "worldEditor.ignoreObjectClass(class_name, ...);", 3, 0);
   Con::addCommand("WorldEditor", "clearIgnoreList", cClearIgnoreList, "worldEditor.clearIgnoreList();", 2, 2);
   Con::addCommand("WorldEditor", "undo", cUndoAction, "worldEditor.undo();", 2, 2);
   Con::addCommand("WorldEditor", "redo", cRedoAction, "worldEditor.redo();", 2, 2);
   Con::addCommand("WorldEditor", "clearSelection", cClearSelection, "worldEditor.clearSelection();", 2, 2);
   Con::addCommand("WorldEditor", "selectObject", cSelectObject, "worldEditor.selectObject(object);", 3, 3);
   Con::addCommand("WorldEditor", "unselectObject", cUnselectObject, "worldEditor.unselectObject(object);", 3, 3);
   Con::addCommand("WorldEditor", "getSelectionSize", cGetSelectionSize, "worldEditor.getSelectionSize();", 2, 2);
   Con::addCommand("WorldEditor", "getSelectedObject", cGetSelectedObject, "worldEditor.getSelectedObject(index);", 3, 3);
   Con::addCommand("WorldEditor", "getSelectionCentroid", cGetSelectionCentroid, "worldEditor.getSelectionCentroid();", 2, 2);
   Con::addCommand("WorldEditor", "dropSelection", cDropSelection, "worldEditor.dropSelection();", 2, 2);
   Con::addCommand("WorldEditor", "deleteSelection", cDeleteSelection, "worldEditor.deleteSelection();", 2, 2);
   Con::addCommand("WorldEditor", "copySelection", cCopySelection, "worldEditor.copySelection();", 2, 2);
   Con::addCommand("WorldEditor", "pasteSelection", cPasteSelection, "worldEditor.pasteSelection();", 2, 2);
   Con::addCommand("WorldEditor", "canPasteSelection", cCanPasteSelection, "worldEditor.canPasteSelection();", 2, 2);
   Con::addCommand("WorldEditor", "getMode", cGetMode, "worldEditor.getMode();", 2, 2);
   Con::addCommand("WorldEditor", "setMode", cSetMode, "worldEditor.setMode(move|rotate|scale);", 3, 3);
   Con::addCommand("WorldEditor", "hideSelection", cHideSelection, "worldEditor.hideSelection(bool);", 3, 3);
   Con::addCommand("WorldEditor", "lockSelection", cLockSelection, "worldEditor.lockSelection(bool);", 3, 3);
   Con::addCommand("WorldEditor", "addUndoState", cAddUndoState, "worldEditor.addUndoState();", 2, 2);
   Con::addCommand("WorldEditor", "redirectConsole", cRedirectConsole, "worldEditor.redirectConsole(objID)", 3, 3);
}

//------------------------------------------------------------------------------

static void cIgnoreObjClass(SimObject * obj, S32 argc, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor *>(obj);
   
   for(S32 i = 2; i < argc; i++)
   {
      WorldEditor::ClassInfo::Entry * entry = wEditor->getClassEntry(argv[i]);
      if(entry)
         entry->mIgnoreCollision = true;
      else
      {
         entry = new WorldEditor::ClassInfo::Entry;
         entry->mName = StringTable->insert(argv[i]);
         entry->mIgnoreCollision = true;
         if(!wEditor->addClassEntry(entry))
            delete entry;
      }
   }
}

static void cClearIgnoreList(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor *>(obj);
   for(U32 i = 0; i < wEditor->mClassInfo.mEntries.size(); i++)
      wEditor->mClassInfo.mEntries[i]->mIgnoreCollision = false;
}

static void cUndoAction(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->processUndo(wEditor->mUndoList, wEditor->mRedoList);
}

static void cRedoAction(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->processUndo(wEditor->mRedoList, wEditor->mUndoList);
}

static void cClearSelection(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   if(wEditor->mSelectionLocked)
      return;

   wEditor->mSelected.clear();
}

static void cSelectObject(SimObject * obj, S32, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   if(wEditor->mSelectionLocked)
      return;

   SceneObject * select = dynamic_cast<SceneObject*>(Sim::findObject(argv[2]));
   if(select && !wEditor->objClassIgnored(select))
      wEditor->mSelected.addObject(select);
}

static void cUnselectObject(SimObject * obj, S32, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   if(wEditor->mSelectionLocked)
      return;

   SceneObject * select = dynamic_cast<SceneObject*>(Sim::findObject(argv[2]));
   if(select && !wEditor->objClassIgnored(select))
      wEditor->mSelected.removeObject(select);
}

static S32 cGetSelectionSize(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   return(wEditor->mSelected.size());
}

static S32 cGetSelectedObject(SimObject * obj, S32, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);

   S32 index = dAtoi(argv[2]);
   if(index < 0 || index >= wEditor->mSelected.size())
   {
      Con::errorf(ConsoleLogEntry::General, "WorldEditor::getSelectedObject: invalid object index");
      return(-1);
   }

   return(wEditor->mSelected[index]->getId());
}

static const char * cGetSelectionCentroid(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);

   const Point3F & centroid = wEditor->mSelected.getCentroid();
   char * ret = Con::getReturnBuffer(100);
   dSprintf(ret, 100, "%f %f %f", centroid.x, centroid.y, centroid.z);
   return(ret);
}

static void cDropSelection(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->dropSelection(wEditor->mSelected);
}

static void cDeleteSelection(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->deleteSelection(wEditor->mSelected);
}

static void cCopySelection(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->copySelection(wEditor->mSelected);
}

static void cPasteSelection(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->pasteSelection();
}
static bool cCanPasteSelection(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   return(wEditor->mStreamBufs.size() != 0);
}

static void cHideSelection(SimObject * obj, S32, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->hideSelection(dAtob(argv[2]));
}

static void cLockSelection(SimObject * obj, S32, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->lockSelection(dAtob(argv[2]));
}

static void cRedirectConsole(SimObject * obj, S32, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->mRedirectID = dAtoi(argv[2]);
}

static const char * cGetMode(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);

   if(wEditor->mCurrentMode == WorldEditor::Move)
      return("move");
   else if(wEditor->mCurrentMode == WorldEditor::Rotate)
      return("rotate");
   else if(wEditor->mCurrentMode == WorldEditor::Scale)
      return("scale");
   else
   {
      Con::warnf(ConsoleLogEntry::General, avar("worldEditor.getMode: unknown mode"));
      return("");
   }
}

static void cSetMode(SimObject * obj, S32, const char ** argv)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);

   if(!dStricmp(argv[2], "move"))
      wEditor->mCurrentMode = WorldEditor::Move;
   else if(!dStricmp(argv[2], "rotate"))
      wEditor->mCurrentMode = WorldEditor::Rotate;
   else if(!dStricmp(argv[2], "scale"))
      wEditor->mCurrentMode = WorldEditor::Scale;
   else
      Con::warnf(ConsoleLogEntry::General, avar("worldEditor.setMode: invalid mode '%s'", argv[2]));
}

static void cAddUndoState(SimObject * obj, S32, const char **)
{
   WorldEditor * wEditor = static_cast<WorldEditor*>(obj);
   wEditor->addUndo(wEditor->mUndoList, wEditor->createUndo(wEditor->mSelected));
   wEditor->clearUndo(wEditor->mRedoList);
}

