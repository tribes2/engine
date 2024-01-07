//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICONSOLETEXTCTRL_H_
#define _GUICONSOLETEXTCTRL_H_

#ifndef _GFONT_H_
#include "dgl/gFont.h"
#endif
#ifndef _GUITYPES_H_
#include "GUI/guiTypes.h"
#endif
#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GuiConsoleTextCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;

public:
	enum Constants { MAX_STRING_LENGTH = 255 };


protected:
   const char *mConsoleExpression;
   const char *mResult;
   Resource<GFont> mFont;
   
public:   

	//creation methods
   DECLARE_CONOBJECT(GuiConsoleTextCtrl);
   GuiConsoleTextCtrl();
	static void consoleInit();
   static void initPersistFields();

	//Parental methods
   bool onWake();
   void onSleep();

	//text methods
   virtual void setText(const char *txt = NULL);
   const char *getText() { return mConsoleExpression; }

	//rendering methods
   void calcResize();
   void onPreRender(); 	// do special pre render processing
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);

	//Console methods
   const char *getScriptValue();
   void setScriptValue(const char *value);
};

#endif //_GUI_TEXT_CONTROL_H_
