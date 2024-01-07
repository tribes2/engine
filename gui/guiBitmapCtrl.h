//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIBITMAPCTRL_H_
#define _GUIBITMAPCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif

class GuiBitmapCtrl : public GuiControl
{
private:
	typedef GuiControl Parent;

protected:
   StringTableEntry mBitmapName;
   TextureHandle mTextureHandle;
	Point2I startPoint;
	bool mWrap;
   
public:
	//creation methods
	DECLARE_CONOBJECT(GuiBitmapCtrl);
	GuiBitmapCtrl();
   static void initPersistFields();
   static void consoleInit();

   //Parental methods
   bool onWake();
   void onSleep();

   void setBitmap(const char *name);
   void setBitmap(const TextureHandle &handle);

   S32 getWidth() const       { return(mTextureHandle.getWidth()); }
   S32 getHeight() const      { return(mTextureHandle.getHeight()); }

	void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
	void setValue(S32 x, S32 y);
};

#endif
