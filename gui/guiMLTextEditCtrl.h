//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIMLTEXTEDITCTRL_H_
#define _GUIMLTEXTEDITCTRL_H_

#ifndef _GUIMLTEXTCTRL_H_
#include "GUI/guiMLTextCtrl.h"
#endif

class GuiMLTextEditCtrl : public GuiMLTextCtrl
{
   typedef GuiMLTextCtrl Parent;

   //-------------------------------------- Overrides
  protected:
   StringTableEntry mEscapeCommand;

   // Events
   bool onKeyDown(const GuiEvent&);

   // Event forwards
   void handleMoveKeys(const GuiEvent&);
   void handleDeleteKeys(const GuiEvent&);

   // rendering
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);

  public:
   GuiMLTextEditCtrl();
   ~GuiMLTextEditCtrl();

	void resize(const Point2I &newPosition, const Point2I &newExtent);

   DECLARE_CONOBJECT(GuiMLTextEditCtrl);
	static void initPersistFields();
	static void consoleInit();
};

#endif  // _H_GUIMLTEXTEDITCTRL_
