//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUITERRPREVIEWCTRL_H_
#define _GUITERRPREVIEWCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif
#ifndef _GUITSCONTROL_H_
#include "GUI/guiTSControl.h"
#endif

class GuiTerrPreviewCtrl : public GuiControl
{
private:
	typedef GuiControl Parent;
   TextureHandle mTextureHandle;
   Point2F mRoot;
   Point2F mOrigin;
   Point2F mWorldScreenCenter;
   Point2F mCamera;
   F32     mTerrainSize;
		   
   Point2F& wrap(const Point2F &p);
   Point2F& worldToTexture(const Point2F &p);
   Point2F& worldToCtrl(const Point2F &p);


public:
	//creation methods
	DECLARE_CONOBJECT(GuiTerrPreviewCtrl);
	GuiTerrPreviewCtrl();
   static void initPersistFields();
   static void consoleInit();

   //Parental methods
   bool onWake();
   void onSleep();

   void setBitmap(const TextureHandle &handle);

   void reset();
   void setRoot();
   void setRoot(const Point2F &root);
   void setOrigin(const Point2F &origin);
   const Point2F& getRoot() { return mRoot; }
   const Point2F& getOrigin() { return mOrigin; }

   //void setValue(const Point2F *center, const Point2F *camera);
	//const char *getScriptValue();
   void onPreRender();
	void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
};


#endif
