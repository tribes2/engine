//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _HUDCTRL_H_
#define _HUDCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class HudCtrl : public GuiControl
{
   private:
      typedef GuiControl Parent;
   
   public:
   
      HudCtrl();

      // GuiControl
      virtual void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);

      // field data
      ColorF            mFillColor;
      ColorF            mFrameColor;
      F32               mOpacity;

      static void initPersistFields();

      DECLARE_CONOBJECT(HudCtrl);
};

#endif
