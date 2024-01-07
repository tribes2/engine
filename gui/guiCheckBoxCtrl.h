//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICHECKBOXCTRL_H_
#define _GUICHECKBOXCTRL_H_

#ifndef _GUITEXTCTRL_H_
#include "GUI/guiTextCtrl.h"
#endif

class GuiCheckBoxCtrl : public GuiTextCtrl
{
private:
   typedef GuiTextCtrl Parent;
   
protected:
	bool mStateOn;
	bool mKeyPressed;
	
public:   
   DECLARE_CONOBJECT(GuiCheckBoxCtrl);
   GuiCheckBoxCtrl();
	static void consoleInit();

	void AcceleratorKeyPress(void);
	void AcceleratorKeyRelease(void);

	bool onKeyDown(const GuiEvent &event);

   // GuiTextCtrl updates mText with the variable value, which is not desired
   void onPreRender() {}

   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void drawBorder(const RectI &r, const ColorI &color);
	void onAction();
	void setScriptValue(const char *value);
	const char *getScriptValue();
};

#endif //_GUI_CHECKBOX_CTRL_H
