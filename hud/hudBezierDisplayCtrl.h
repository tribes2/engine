//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _HUDBEZIERDISPLAYCTRL_H_
#define _HUDBEZIERDISPLAYCTRL_H_

#include "hud/hudObject.h"
#include "math/mBezier2D.h"
#include "console/consoleTypes.h"

class HudBezierDisplayCtrl : public HudObject {
   private:
      typedef HudObject Parent;
      
      Bezier2D *mCurve;
      
   protected:
      virtual F32 getValue();
      virtual ColorI getBarColor();
      virtual ColorI getSecondBarColor();
      
   public:
      HudBezierDisplayCtrl();
      ~HudBezierDisplayCtrl();
      void resize( const Point2I &newPosition, const Point2I &newExtent );   
      void onRender( Point2I, const RectI &, GuiControl * );
      
      S32      mPulseRate;
      F32      mPulseThreshold;
      bool     mPulse;
      bool     mDrawFromOrigin;
      bool     mShowPercentage;
      bool     mDisplayMounted;
      bool     mGradientFill;
      F32      mValue;
      Point2F  mOffset, mTextOffset;
      F32      mScaleValue;
      
      static void initPersistFields();

      DECLARE_CONOBJECT( HudBezierDisplayCtrl );
};

#endif

// hudBezierDisplayCtrl.h