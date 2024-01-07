//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIBUTTONCTRL_H_
#define _GUIBUTTONCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GuiButtonCtrl : public GuiControl
{
  private:
   typedef GuiControl Parent;

  protected: 
   enum State {
      Normal,
      MouseDown
   };

   bool             mMouseInControl;
   State            mButtonState;
   StringTableEntry mButtonText;

  public:   
   DECLARE_CONOBJECT(GuiButtonCtrl);
   GuiButtonCtrl();
	static void consoleInit();
   static void initPersistFields();

	void AcceleratorKeyPress(void);

	bool onKeyDown(const GuiEvent&);

   void onMouseUp(const GuiEvent&);
   void onMouseDown(const GuiEvent&);
   void onMouseEnter(const GuiEvent&);
   void onMouseLeave(const GuiEvent&);

   const char * getScriptValue();
   void setScriptValue(const char *);

   void onSleep();

   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void drawBorder(const RectI &r, const ColorI &color);
};

#endif //_GUI_BUTTON_CTRL_H
