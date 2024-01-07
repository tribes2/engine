//-----------------------------------------------------------------------------
// Torque Engine
// 
// Copyright (c) 2001 GarageGames.Com
//-----------------------------------------------------------------------------

#ifndef _GUIBUTTONBASECTRL_H_
#define _GUIBUTTONBASECTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/guiControl.h"
#endif

class GuiButtonBaseCtrl : public GuiControl
{
   typedef GuiControl Parent;

protected:
   StringTableEntry mButtonText;
   bool mDepressed;
   bool mMouseOver;
public:
   GuiButtonBaseCtrl();
   static void initPersistFields();

   void setText(const char *text);
   const char *getText();

   void AcceleratorKeyPress();
   void AcceleratorKeyRelease();

   void onMouseDown(const GuiEvent &);
   void onMouseUp(const GuiEvent &);

   void onMouseEnter(const GuiEvent &);
   void onMouseLeave(const GuiEvent &);

   bool onKeyDown(const GuiEvent &event);
   bool onKeyUp(const GuiEvent &event);
};

#endif