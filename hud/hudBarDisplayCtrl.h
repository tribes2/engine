//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _HUDBARDISPLAYCTRL_H_
#define _HUDBARDISPLAYCTRL_H_

#include "hud/hudObject.h"
#include "console/consoleTypes.h"

/**
 * This is a HUD object that displays data in horizontal or vertical
 * percentage bars
 */
class HudBarDisplayCtrl : public HudObject {
   private:
      typedef HudObject Parent;

   protected:
      virtual F32 getValue();
      virtual ColorI getBarColor();
      virtual ColorI getSecondBarColor();
      
   public:
      
      HudBarDisplayCtrl();
         
      void onRender( Point2I, const RectI &, GuiControl * );
      
      S32      mPulseRate;
      F32      mPulseThreshold;
      bool     mPulse;
      bool     mHorizontalBar;
      bool     mDrawFromOrigin;
      bool     mShowPercentage;
      bool     mDisplayMounted;
      bool     mGradientFill;
      F32      mValue;
      
      static void initPersistFields();

      DECLARE_CONOBJECT( HudBarDisplayCtrl );
};

#endif

// HudBarDisplayCtrl.h