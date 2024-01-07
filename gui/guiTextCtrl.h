//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUITEXTCTRL_H_
#define _GUITEXTCTRL_H_

#ifndef _GFONT_H_
#include "dgl/gFont.h"
#endif
#ifndef _GUITYPES_H_
#include "GUI/guiTypes.h"
#endif
#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GuiTextCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;

public:
	enum Constants { MAX_STRING_LENGTH = 255 };


protected:
	StringTableEntry mInitialText;
   char mText[MAX_STRING_LENGTH + 1];
   S32 mMaxStrLen;   // max string len, must be less then or equal to 255
   Resource<GFont> mFont;
   
public:   

	//creation methods
   DECLARE_CONOBJECT(GuiTextCtrl);
   GuiTextCtrl();
	static void consoleInit();
   static void initPersistFields();

	//Parental methods
   bool onAdd();
   virtual bool onWake();
   virtual void onSleep();

	//text methods
   virtual void setText(const char *txt = NULL);
   const char *getText() { return mText; }

   void inspectPostApply();
	//rendering methods
   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);

	//Console methods
   const char *getScriptValue();
   void setScriptValue(const char *value);
};

#endif //_GUI_TEXT_CONTROL_H_
