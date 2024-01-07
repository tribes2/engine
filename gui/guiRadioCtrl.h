//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIRADIOCTRL_H_
#define _GUIRADIOCTRL_H_

#ifndef _GUITEXTCTRL_H_
#include "GUI/guiTextCtrl.h"
#endif

class GuiRadioCtrl : public GuiTextCtrl
{
private:
   typedef GuiTextCtrl Parent;

protected:
	bool mKeyPressed;
	bool mStateOn;
	S32  mGroupNum;	 
	
public:   
   DECLARE_CONOBJECT(GuiRadioCtrl);
   GuiRadioCtrl();
	static void consoleInit();
   static void initPersistFields();

	void AcceleratorKeyPress(void);
	void AcceleratorKeyRelease(void);

	bool onKeyDown(const GuiEvent &event);

   // GuiTextCtrl updates mText with the variable value, which is not desired
   void onPreRender() {}

   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void drawBorder(const RectI &r, const ColorI &color);
   void onAction();
	void onMessage(GuiControl *,S32 msg);
	void setScriptValue(const char *);
	const char *getScriptValue();
};

#endif //_GUI_RADIO_CTRL_H
