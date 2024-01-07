//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIINPUTCTRL_H_
#define _GUIINPUTCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

class GuiInputCtrl : public GuiControl
{
   private:
      typedef GuiControl Parent;

   public:
   	DECLARE_CONOBJECT(GuiInputCtrl);

		bool onWake();
		void onSleep();

	   bool onInputEvent( const InputEvent &event );
};

#endif // _GUI_INPUTCTRL_H
