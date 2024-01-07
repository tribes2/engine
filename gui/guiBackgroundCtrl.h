//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIBACKGROUNDCTRL_H_
#define _GUIBACKGROUNDCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/guiControl.h"
#endif

class GuiBackgroundCtrl : public GuiControl
{
private:
	typedef GuiControl Parent;

public:
   bool  mDraw;

	//creation methods
	DECLARE_CONOBJECT(GuiBackgroundCtrl);
   GuiBackgroundCtrl();

	void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
};

#endif
