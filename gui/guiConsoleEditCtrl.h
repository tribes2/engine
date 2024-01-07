//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICONSOLEEDITCTRL_H_
#define _GUICONSOLEEDITCTRL_H_

#ifndef _GUITYPES_H_
#include "GUI/guiTypes.h"
#endif
#ifndef _GUITEXTEDITCTRL_H_
#include "GUI/guiTextEditCtrl.h"
#endif

class GuiConsoleEditCtrl : public GuiTextEditCtrl
{
private:
   typedef GuiTextEditCtrl Parent;

   // max string len, must be less then or equal to 255
   SimObjectPtr<SimObject> tabObject;
   S32 baseStart;
   S32 baseLen;
   char tabBuffer[GuiTextCtrl::MAX_STRING_LENGTH + 1];
public:
   GuiConsoleEditCtrl();
   DECLARE_CONOBJECT(GuiConsoleEditCtrl);

   bool onKeyDown(const GuiEvent &event);
   void handleTab(bool forwardTab);
};

#endif //_GUI_TEXTEDIT_CTRL_H
