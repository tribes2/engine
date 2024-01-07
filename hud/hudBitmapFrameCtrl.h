//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _HUDBITMAPFRAMECTRL_H_
#define _HUDBITMAPFRAMECTRL_H_

#ifndef _HUDBITMAPCTRL_H_
#include "hud/hudBitmapCtrl.h"
#endif

class HudBitmapFrameCtrl : public HudBitmapCtrl
{
   private:
      typedef HudBitmapCtrl   Parent;
      
   public:
   
      HudBitmapFrameCtrl();

      // GuiControl
      void onPreRender();

      // field data
      RectI    mSubRegion;

      static void initPersistFields();

      DECLARE_CONOBJECT(HudBitmapFrameCtrl);   
};

#endif
