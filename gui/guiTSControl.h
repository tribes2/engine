//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUITSCONTROL_H_
#define _GUITSCONTROL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif
#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif
#ifndef _DGL_H_
#include "dgl/dgl.h"
#endif

struct CameraQuery
{
   SimObject*  object;
   F32         nearPlane;
   F32         farPlane;
   F32         fov;
   MatrixF     cameraMatrix;
   
   //Point3F position;
   //Point3F viewVector;
   //Point3F upVector;
};

class GuiTSCtrl : public GuiControl
{
   typedef GuiControl Parent;

   GLdouble       mSaveModelview[16];
   GLdouble       mSaveProjection[16];
   GLint          mSaveViewport[4];

   static U32     smFrameCount;

public:
   CameraQuery    mLastCameraQuery;
   GuiTSCtrl();
   
   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   virtual bool processCameraQuery(CameraQuery *query);
   virtual void renderWorld(const RectI &updateRect);
   
   static void consoleInit();

   bool project(const Point3F &pt, Point3F *dest); // returns screen space X,Y and Z for world space point
   bool unproject(const Point3F &pt, Point3F *dest); // returns world space point for X, Y and Z
   
   DECLARE_CONOBJECT(GuiTSCtrl);
};

#endif
