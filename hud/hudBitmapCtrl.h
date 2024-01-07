//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _HUDBITMAPCTRL_H_
#define _HUDBITMAPCTRL_H_

#ifndef _HUDCTRL_H_
#include "hud/hudCtrl.h"
#endif
#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif
#ifndef _DGL_H_
#include "dgl/dgl.h"
#endif

class HudBitmapCtrl : public HudCtrl
{
   private:
      typedef HudCtrl   Parent;

   protected:

      TextureHandle     mBitmapHandle;
      
   public:
      
      HudBitmapCtrl();
      
      void setBitmap(const char * bitmap);
      
      // SimObject
      void inspectPostApply();
      
      // GuiControl
      void onPreRender();
      void onRender(Point2I offset, const RectI & updateRect, GuiControl * firstResponder);
      
      bool onWake();
      void onSleep();
      
      // field data
      StringTableEntry     mBitmap;
      bool                 mAutoResize;
      bool                 mAutoCenter;
      bool                 mFlipVert;
      bool                 mFlipHorz;   
      
      static void initPersistFields();
      
      DECLARE_CONOBJECT(HudBitmapCtrl);
};

#endif
