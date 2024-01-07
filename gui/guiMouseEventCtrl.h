//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIMOUSEEVENTCTRL_H_
#define _GUIMOUSEEVENTCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif


class GuiMouseEventCtrl : public GuiControl
{
   private:
      typedef  GuiControl     Parent;
      void sendMouseEvent(const char * name, const GuiEvent &);
      
      // field info
      bool        mLockMouse;

   public:

      GuiMouseEventCtrl();

      // GuiControl
      void onMouseDown(const GuiEvent & event);
      void onMouseUp(const GuiEvent & event);
      void onMouseMove(const GuiEvent & event);
      void onMouseDragged(const GuiEvent & event);
      void onMouseEnter(const GuiEvent & event);
      void onMouseLeave(const GuiEvent & event);
      void onRightMouseDown(const GuiEvent & event);
      void onRightMouseUp(const GuiEvent & event);
      void onRightMouseDragged(const GuiEvent & event);

      static void consoleInit();
      static void initPersistFields();

      DECLARE_CONOBJECT(GuiMouseEventCtrl);
};

#endif
