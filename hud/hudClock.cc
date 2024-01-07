//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBitmapFrameCtrl.h"
#include "dgl/dgl.h"

//--------------------------------------------------------------------------- 
// CLASS: HudClockCtrl
//--------------------------------------------------------------------------- 
class HudClockCtrl : public HudBitmapFrameCtrl
{
   private:

      typedef HudBitmapFrameCtrl    Parent;
      S32                           mCurrentSetTime;

   public:

      HudClockCtrl();

      void setTime(F32);
      static void consoleInit();

      void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
      DECLARE_CONOBJECT(HudClockCtrl);
};

IMPLEMENT_CONOBJECT(HudClockCtrl);

//--------------------------------------------------------------------------- 

HudClockCtrl::HudClockCtrl()
{
   mCurrentSetTime = 0;
}

//--------------------------------------------------------------------------- 

void HudClockCtrl::setTime(F32 newTime)
{
   F32 time = Platform::getVirtualMilliseconds();   
   mCurrentSetTime = -(S32)((newTime*60*1000)+time);   
}

//--------------------------------------------------------------------------- 

void HudClockCtrl::onRender(Point2I      /*offset*/,
                        const RectI& /*updateRect*/,
                        GuiControl*  /*firstResponder*/)
{
//   Parent::onRender(offset, updateRect, firstResponder);

   char buf[256];
   Point2I localStart;
   S32 secondsLeft, hours, mins, secs, hundredths;

   F32 actualTimeMS = (F32)mCurrentSetTime + Platform::getVirtualMilliseconds();   
	F32 time = mAbs(actualTimeMS);

   secondsLeft = (S32)time/1000;
   hundredths = (S32)((time - (secondsLeft*1000))/10);
   hours = secondsLeft / 3600;
   secondsLeft -= hours * 3600;
   mins = secondsLeft / 60; 
   secondsLeft -= mins * 60;
   secs = secondsLeft;

//   dSprintf(buf, sizeof(buf), "%02d:%02d:%02d.%1d", hours, mins, secs, tenths);
	if (mins == 0 && actualTimeMS < 0)
	   dSprintf(buf, sizeof(buf), "%02d.%02d", secs, hundredths);
	else
	   dSprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);

   localStart.x = (mSubRegion.extent.x - mProfile->mFont->getStrWidth(buf)) / 2;
  	localStart.y = ((mSubRegion.extent.y - (mProfile->mFont->getHeight() - 2)) / 2);
   localStart += mSubRegion.point;

   //
   if(localStart.x < 0)
      localStart.x = mSubRegion.point.x;
   if(localStart.y < 0)
      localStart.y = mSubRegion.point.y;

   dglSetBitmapModulation(ColorF(1,1,1,mOpacity));
 	dglDrawText(mProfile->mFont, localToGlobalCoord(localStart), buf);
   dglClearBitmapModulation();
}

//--------------------------------------------------------------------------- 

static void cHudClockCtrlSetTime(SimObject *obj, S32, const char **argv)
{
   HudClockCtrl *ctrl = static_cast<HudClockCtrl*>(obj);
   ctrl->setTime(dAtof(argv[2]));      
}

void HudClockCtrl::consoleInit()
{
   Con::addCommand("hudClock", "setTime", cHudClockCtrlSetTime, "timer.setTime(Time In Min's)", 3, 3);
}
