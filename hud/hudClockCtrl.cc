//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudObject.h"
#include "hud/hudGLEx.h"
#include "console/consoleTypes.h"

/**
 * HudClock is a clock for the HUD.
 */
class HudClockCtrl : public HudObject {
   
   private:
      typedef HudObject Parent;
      S32 mStoredTime;

   public:
      HudClockCtrl();

      void setTime( F32 newTime );
      static void consoleInit();

      void onRender( Point2I offset, const RectI &updateRect, GuiControl *firstResponder );
      
      DECLARE_CONOBJECT( HudClockCtrl ); 
};

//--------------------------------------------------------------------------- 


IMPLEMENT_CONOBJECT( HudClockCtrl );

/**
 * Constructor
 */
HudClockCtrl::HudClockCtrl() {
   mStoredTime = 0;
}

/**
 * This method is used to set the ammount of time this clock should display.
 *
 * @param newTime Time to display in minutes
 */
void HudClockCtrl::setTime( F32 newTime ) {
   F32 time = Platform::getVirtualMilliseconds();   
   mStoredTime = -(S32)( ( newTime * 60 * 1000 )+ time );   
}

/**
 * Method called to render the HUD object
 *
 * @param offset Basically the corner to begin drawing at
 * @param updateRect The rectangle that this object has power to draw in
 * @param firstResponder ?
 */
void HudClockCtrl::onRender( Point2I offset,
                         const RectI &updateRect,
                         GuiControl*  firstResponder ) {

   // Sanity check
   GuiControl *parent = getParent();
   if( !parent )
      return;
   
   Parent::onRender( offset, updateRect, firstResponder );
   
   // Declarations
   char buf[256];
   S32 hours, mins, secs, hundredths;   

   F32 actualTimeMS = (F32)mStoredTime + Platform::getVirtualMilliseconds();   
   F32 time = mAbs( actualTimeMS );

   // Operations
   secs = (S32)time / 1000;
   hundredths = (S32)( ( time - ( secs * 1000 ) ) / 10 );
   hours = secs / 3600;
   secs -= hours * 3600;
   mins = secs / 60; 
   secs -= mins * 60;

   if( mins == 0 && actualTimeMS < 0 )
      dSprintf( buf, sizeof( buf ), "%02d.%02d", secs, hundredths );
   else
      dSprintf( buf, sizeof( buf ), "%02d:%02d", mins, secs );
      
   offset.x += ( mBounds.extent.x / 2 ) - ( mProfile->mFont->getStrWidth( buf ) / 2 );
   offset.y += ( mBounds.extent.y / 2 ) - ( mProfile->mFont->getHeight() / 2 );

   drawText( offset, buf );
}

/**
 * This method is called from the console as a accessor to the setTime method
 *
 * @param obj Clock to set the time of, as this is a static method
 * @param argv Array of string arguments from console
 */
static void cHudClockCtrlSetTime( SimObject *obj, S32, const char **argv ) {
   HudClockCtrl *ctrl = static_cast<HudClockCtrl*>( obj );
   ctrl->setTime( dAtof( argv[2] ) );      
}

/**
 * Adds to the console the methods this object supports
 */
void HudClockCtrl::consoleInit() {
   Con::addCommand( "HudClockCtrl", "setTime", cHudClockCtrlSetTime, "timer.setTime(Time In Min's)", 3, 3 );
}

// hudClockCtrl.cc