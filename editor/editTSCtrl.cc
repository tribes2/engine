//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "core/dnet.h"
#include "editor/editTSCtrl.h"
#include "sceneGraph/sceneGraph.h"
#include "editor/editor.h"
#include "game/gameConnection.h"
#include "game/gameBase.h"
#include "game/missionArea.h"
#include "console/consoleTypes.h"
#include "terrain/terrData.h"
#include "game/game.h"
#include "game/sphere.h"

IMPLEMENT_CONOBJECT(EditTSCtrl);

//------------------------------------------------------------------------------

Point3F  EditTSCtrl::smCamPos;
EulerF   EditTSCtrl::smCamRot;
MatrixF  EditTSCtrl::smCamMatrix;
F32      EditTSCtrl::smVisibleDistance = 2100.f;

EditTSCtrl::EditTSCtrl() :
   mEditManager(0)
{
   mRenderMissionArea = true;
   mMissionAreaFillColor.set(255,0,0,20);
   mMissionAreaFrameColor.set(255,0,0,128);
   
   mConsoleFrameColor.set(255,0,0,255);
   mConsoleFillColor.set(255,0,0,120);
   mConsoleSphereLevel = 1;
   mConsoleCircleSegments = 32;
   mConsoleLineWidth = 1;

   mConsoleRendering = false;
}

EditTSCtrl::~EditTSCtrl()
{
}

//------------------------------------------------------------------------------

bool EditTSCtrl::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   // give all derived access to the fields
   setModStaticFields(true);
   return true;
}

void EditTSCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   updateGuiInfo();
   Parent::onRender(offset, updateRect, firstResponder);
}

//------------------------------------------------------------------------------

void EditTSCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("renderMissionArea", TypeBool, Offset(mRenderMissionArea, EditTSCtrl));
   addField("missionAreaFillColor", TypeColorI, Offset(mMissionAreaFillColor, EditTSCtrl));
   addField("missionAreaFrameColor", TypeColorI, Offset(mMissionAreaFrameColor, EditTSCtrl));

   addField("consoleFrameColor", TypeColorI, Offset(mConsoleFrameColor, EditTSCtrl));
   addField("consoleFillColor", TypeColorI, Offset(mConsoleFillColor, EditTSCtrl));
   addField("consoleSphereLevel", TypeS32, Offset(mConsoleSphereLevel, EditTSCtrl));
   addField("consoleCircleSegments", TypeS32, Offset(mConsoleCircleSegments, EditTSCtrl));
   addField("consoleLineWidth", TypeS32, Offset(mConsoleLineWidth, EditTSCtrl));

   Con::addVariable("pref::Editor::visibleDistance", TypeF32, &EditTSCtrl::smVisibleDistance);
}

//------------------------------------------------------------------------------

void EditTSCtrl::make3DMouseEvent(Gui3DMouseEvent & gui3DMouseEvent, const GuiEvent & event)
{
   (GuiEvent&)(gui3DMouseEvent) = event;
   
   // get the eye pos and the mouse vec from that...
   Point3F sp(event.mousePoint.x, event.mousePoint.y, 1);

   Point3F wp;
   unproject(sp, &wp);
   
   gui3DMouseEvent.pos = smCamPos;
   gui3DMouseEvent.vec = wp - smCamPos;
   gui3DMouseEvent.vec.normalize();
}

//------------------------------------------------------------------------------

void EditTSCtrl::onMouseUp(const GuiEvent & event)
{
   Gui3DMouseEvent gui3DMouseEvent;
   make3DMouseEvent(gui3DMouseEvent, event);
   on3DMouseUp(gui3DMouseEvent);
}

void EditTSCtrl::onMouseDown(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DMouseDown(gui3DMouseEvent);
}

void EditTSCtrl::onMouseMove(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DMouseMove(gui3DMouseEvent);
}

void EditTSCtrl::onMouseDragged(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DMouseDragged(gui3DMouseEvent);
   
}

void EditTSCtrl::onMouseEnter(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DMouseEnter(gui3DMouseEvent);
}

void EditTSCtrl::onMouseLeave(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DMouseLeave(gui3DMouseEvent);
}

void EditTSCtrl::onRightMouseDown(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DRightMouseDown(gui3DMouseEvent);
}

void EditTSCtrl::onRightMouseUp(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DRightMouseUp(gui3DMouseEvent);
}

void EditTSCtrl::onRightMouseDragged(const GuiEvent & event)
{
	Gui3DMouseEvent gui3DMouseEvent;
	make3DMouseEvent(gui3DMouseEvent, event);
	on3DRightMouseDragged(gui3DMouseEvent);
}

//------------------------------------------------------------------------------

void EditTSCtrl::renderWorld(const RectI & updateRect)
{
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClear(GL_DEPTH_BUFFER_BIT);
   glDisable(GL_CULL_FACE);
   glMatrixMode(GL_MODELVIEW);

   dglSetCanonicalState();
   gClientSceneGraph->renderScene();
   
   // render the mission area...
   if(mRenderMissionArea)
      renderMissionArea();

   glDisable(GL_DEPTH_TEST);
   
   // render through console callbacks
   SimSet * missionGroup = static_cast<SimSet*>(Sim::findObject("MissionGroup"));
   if(missionGroup)
   {
      mConsoleRendering = true;
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);

      for(SimSetIterator itr(missionGroup); *itr; ++itr)
      {  
         char buf[2][16];
         dSprintf(buf[0], 16, (*itr)->isSelected() ? "true" : "false");
         dSprintf(buf[1], 16, (*itr)->isExpanded() ? "true" : "false");
         Con::executef(*itr, 4, "onEditorRender", getIdString(), buf[0], buf[1]);
      }

      glDisable(GL_DEPTH_TEST);
      mConsoleRendering = false;
   }

   // render the editor stuff
   renderScene(updateRect);
   
   dglSetClipRect(updateRect);
}

void EditTSCtrl::renderMissionArea()
{
   MissionArea * obj = dynamic_cast<MissionArea*>(Sim::findObject("MissionArea"));
   TerrainBlock * terrain = dynamic_cast<TerrainBlock*>(Sim::findObject("Terrain"));
   if(!terrain)
      return;

   GridSquare * gs = terrain->findSquare(TerrainBlock::BlockShift, Point2I(0,0));
   F32 height = F32(gs->maxHeight) * 0.03125f + 10.f;
      
   //
   const RectI &area = obj->getArea();
   Point2F min(area.point.x, area.point.y);
   Point2F max(area.point.x + area.extent.x, area.point.y + area.extent.y);

   glDisable(GL_CULL_FACE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   ColorI & a = mMissionAreaFillColor;
   ColorI & b = mMissionAreaFrameColor;
   for(U32 i = 0; i < 2; i++)
   {
      //
      if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
      glVertex3f(min.x, min.y, 0);
      glVertex3f(max.x, min.y, 0);
      glVertex3f(max.x, min.y, height);
      glVertex3f(min.x, min.y, height);
      glEnd();
   
      //
      if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
      glVertex3f(min.x, max.y, 0);
      glVertex3f(max.x, max.y, 0);
      glVertex3f(max.x, max.y, height);
      glVertex3f(min.x, max.y, height);
      glEnd();
   
      //
      if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
      glVertex3f(min.x, min.y, 0);
      glVertex3f(min.x, max.y, 0);
      glVertex3f(min.x, max.y, height);
      glVertex3f(min.x, min.y, height);
      glEnd();
   
      //
      if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
      glVertex3f(max.x, min.y, 0);
      glVertex3f(max.x, max.y, 0);
      glVertex3f(max.x, max.y, height);
      glVertex3f(max.x, min.y, height);
      glEnd();
   }
   
   glDisable(GL_BLEND);
}

//------------------------------------------------------------------------------

bool EditTSCtrl::processCameraQuery(CameraQuery * query)
{
   GameConnection* connection = dynamic_cast<GameConnection *>(NetConnection::getServerConnection());
   if (connection)
   {
      if (connection->getControlCameraTransform(0.032,&query->cameraMatrix)) {
         query->nearPlane = 0.1;
         query->farPlane = getMax(smVisibleDistance, 50.f);
         query->fov = 3.1415 / 2;

         smCamMatrix = query->cameraMatrix;
         smCamMatrix.getColumn(3,&smCamPos);
         smCamRot.set(0,0,0);
         return(true);
      }
   }
   return(false);
}

//------------------------------------------------------------------------------
// sort the surfaces: not correct when camera is inside sphere but not
// inside tesselated representation of sphere....
struct SortInfo {
   U32 idx;
   F32 dot;
};

static int QSORT_CALLBACK alphaSort(const void* p1, const void* p2)
{
   const SortInfo* ip1 = (const SortInfo*)p1;
   const SortInfo* ip2 = (const SortInfo*)p2;

   if(ip1->dot > ip2->dot)
      return(1);
   if(ip1->dot == ip2->dot)
      return(0);
   return(-1);
}

//------------------------------------------------------------------------------
static void cRenderSphere(SimObject * obj, S32 argc, const char ** argv)
{
   EditTSCtrl * ctrl = static_cast<EditTSCtrl*>(obj);
   if(!ctrl->mConsoleRendering)
      return;
   
   static Sphere sphere(Sphere::Icosahedron);

   if(!ctrl->mConsoleFrameColor.alpha && !ctrl->mConsoleFillColor.alpha)
      return;

   S32 sphereLevel = ctrl->mConsoleSphereLevel;
   if(argc == 5)
      sphereLevel = dAtoi(argv[4]);

   const Sphere::TriangleMesh * mesh = sphere.getMesh(sphereLevel);

   Point3F pos;
   dSscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);

   F32 radius = dAtoi(argv[3]);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // sort the surfaces back->front
   Vector<SortInfo> sortInfos;

   Point3F camNormal = ctrl->smCamPos - pos;
   camNormal.normalize();

   sortInfos.setSize(mesh->numPoly);
   for(U32 i = 0; i < mesh->numPoly; i++)
   {
      sortInfos[i].idx = i;
      sortInfos[i].dot = mDot(camNormal, mesh->poly[i].normal);
   }
   dQsort(sortInfos.address(), sortInfos.size(), sizeof(SortInfo), alphaSort);

   // frame
   if(ctrl->mConsoleFrameColor.alpha)
   {
      glColor4ub(ctrl->mConsoleFrameColor.red,
                 ctrl->mConsoleFrameColor.green,
                 ctrl->mConsoleFrameColor.blue,
                 ctrl->mConsoleFrameColor.alpha);

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glLineWidth(ctrl->mConsoleLineWidth);
      glBegin(GL_TRIANGLES);
      for(U32 i = 0; i < mesh->numPoly; i++)
      {
         Sphere::Triangle & tri = mesh->poly[sortInfos[i].idx];
         for(S32 j = 2; j >= 0; j--)
            glVertex3f(tri.pnt[j].x * radius + pos.x,
                       tri.pnt[j].y * radius + pos.y,
                       tri.pnt[j].z * radius + pos.z);
      }
      glEnd();

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glLineWidth(1);
   }

   // fill
   if(ctrl->mConsoleFillColor.alpha)
   {
      glColor4ub(ctrl->mConsoleFillColor.red,
                 ctrl->mConsoleFillColor.green,
                 ctrl->mConsoleFillColor.blue,
                 ctrl->mConsoleFillColor.alpha);

      glBegin(GL_TRIANGLES);
      for(U32 i = 0; i < mesh->numPoly; i++)
      {
         Sphere::Triangle & tri = mesh->poly[sortInfos[i].idx];
         for(S32 j = 2; j >= 0; j--)
            glVertex3f(tri.pnt[j].x * radius + pos.x,
                       tri.pnt[j].y * radius + pos.y,
                       tri.pnt[j].z * radius + pos.z);
      }
      glEnd();
   }
   glDisable(GL_BLEND);
}

static void cRenderCircle(SimObject * obj, S32 argc, const char ** argv)
{
   EditTSCtrl * ctrl = static_cast<EditTSCtrl*>(obj);
   if(!ctrl->mConsoleRendering)
      return;

   if(!ctrl->mConsoleFrameColor.alpha && !ctrl->mConsoleFillColor.alpha)
      return;

   Point3F pos, normal;
   dSscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   dSscanf(argv[3], "%f %f %f", &normal.x, &normal.y, &normal.z);
   
   F32 radius = dAtoi(argv[4]);
   
   S32 segments = ctrl->mConsoleCircleSegments;
   if(argc == 6)
      segments = dAtoi(argv[5]);

   normal.normalize();

   AngAxisF aa;
   mCross(normal, Point3F(0,0,1), &aa.axis);
   aa.angle = mAcos(mClampF(mDot(normal, Point3F(0,0,1)), -1.f, 1.f));

   if(aa.angle == 0.f)
      aa.axis.set(0,0,1);

   MatrixF mat;
   aa.setMatrix(&mat);

   F32 step = M_2PI / segments;
   F32 angle = 0.f;

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   Vector<Point3F> points;
   segments--;
   for(U32 i = 0; i < segments; i++)
   {
      Point3F pnt(mCos(angle), mSin(angle), 0.f);

      mat.mulP(pnt);
      pnt *= radius;
      pnt += pos;

      points.push_back(pnt);
      angle += step;
   }

   // framed
   if(ctrl->mConsoleFrameColor.alpha)
   {
      glColor4ub(ctrl->mConsoleFrameColor.red,
                 ctrl->mConsoleFrameColor.green,
                 ctrl->mConsoleFrameColor.blue,
                 ctrl->mConsoleFrameColor.alpha);
      glLineWidth(ctrl->mConsoleLineWidth);
      glBegin(GL_LINE_LOOP);
      for(U32 i = 0; i < points.size(); i++)
         glVertex3f(points[i].x, points[i].y, points[i].z);
      glEnd();
      glLineWidth(1);
   }

   // filled
   if(ctrl->mConsoleFillColor.alpha)
   {
      glColor4ub(ctrl->mConsoleFillColor.red,
                 ctrl->mConsoleFillColor.green,
                 ctrl->mConsoleFillColor.blue,
                 ctrl->mConsoleFillColor.alpha);

      glBegin(GL_TRIANGLES);

      for(S32 i = 0; i < points.size(); i++)
      {
         S32 j = (i + 1) % points.size();
         glVertex3f(points[i].x, points[i].y, points[i].z);
         glVertex3f(points[j].x, points[j].y, points[j].z);
         glVertex3f(pos.x, pos.y, pos.z);
      }

      glEnd();
   }

   glDisable(GL_BLEND);
}

static void cRenderTriangle(SimObject * obj, S32, const char ** argv)
{
   EditTSCtrl * ctrl = static_cast<EditTSCtrl*>(obj);
   if(!ctrl->mConsoleRendering)
      return;

   if(!ctrl->mConsoleFrameColor.alpha && !ctrl->mConsoleFillColor.alpha)
      return;

   Point3F pnts[3];
   for(U32 i = 0; i < 3; i++)
      dSscanf(argv[i+2], "%f %f %f", &pnts[i].x, &pnts[i].y, &pnts[i].z);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // frame
   if(ctrl->mConsoleFrameColor.alpha)
   {
      glColor4ub(ctrl->mConsoleFrameColor.red,
                 ctrl->mConsoleFrameColor.green,
                 ctrl->mConsoleFrameColor.blue,
                 ctrl->mConsoleFrameColor.alpha);
      glLineWidth(ctrl->mConsoleLineWidth);
      glBegin(GL_LINE_LOOP);
      for(U32 i = 0; i < 3; i++)
         glVertex3f(pnts[i].x, pnts[i].y, pnts[i].z);
      glEnd();
      glLineWidth(1);
   }

   // fill
   if(ctrl->mConsoleFillColor.alpha)
   {
      glColor4ub(ctrl->mConsoleFillColor.red,
                 ctrl->mConsoleFillColor.green,
                 ctrl->mConsoleFillColor.blue,
                 ctrl->mConsoleFillColor.alpha);

      glBegin(GL_TRIANGLES);
      for(U32 i = 0; i < 3; i++)
         glVertex3f(pnts[i].x, pnts[i].y, pnts[i].z);
      glEnd();
   }
   glDisable(GL_BLEND);
}

static void cRenderLine(SimObject * obj, S32 argc, const char ** argv)
{
   EditTSCtrl * ctrl = static_cast<EditTSCtrl*>(obj);
   if(!ctrl->mConsoleRendering)
      return;

   if(!ctrl->mConsoleFrameColor.alpha)
      return;

   Point3F start, end;
   dSscanf(argv[2], "%f %f %f", &start.x, &start.y, &start.z);
   dSscanf(argv[3], "%f %f %f", &end.x, &end.y, &end.z);

   S32 width = ctrl->mConsoleLineWidth;
   if(argc == 5)
      width = dAtoi(argv[4]);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glColor4ub(ctrl->mConsoleFrameColor.red,
              ctrl->mConsoleFrameColor.green,
              ctrl->mConsoleFrameColor.blue,
              ctrl->mConsoleFrameColor.alpha);

   glLineWidth(width);

   glBegin(GL_LINES);
   glVertex3f(start.x, start.y, start.z);
   glVertex3f(end.x, end.y, end.z);
   glEnd();
      
   glLineWidth(1);
   glDisable(GL_BLEND);
}

//------------------------------------------------------------------------------
void EditTSCtrl::consoleInit()
{
   Con::addCommand("EditTSCtrl", "renderSphere", cRenderSphere, "EditTSCtrl.renderSphere(pos, radius, <subdivisions>", 4, 5);
   Con::addCommand("EditTSCtrl", "renderCircle", cRenderCircle, "EditTSCtrl.renderCircle(pos, normal, radius, <segments>", 5, 6);
   Con::addCommand("EditTSCtrl", "renderTriangle", cRenderTriangle, "EditTSCtrl.renderTriangle(pnt, pnt, pnt)", 5, 5);
   Con::addCommand("EditTSCtrl", "renderLine", cRenderLine, "EditTSCtrl.renderLine(start, end, <width>", 4, 5);
}

