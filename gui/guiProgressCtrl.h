//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIPROGRESSCTRL_H_
#define _GUIPROGRESSCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GuiProgressCtrl : public GuiControl
{
private:
	typedef GuiControl Parent;

	F32 mProgress;

public:
	//creation methods
	DECLARE_CONOBJECT(GuiProgressCtrl);
	GuiProgressCtrl();

	//console related methods
   virtual const char *getScriptValue();
   virtual void setScriptValue(const char *value);

   void onPreRender();
	void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
};

#endif
