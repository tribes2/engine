//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "gui/guiControl.h"
#include "console/consoleTypes.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"

//-----------------------------------------------------------------------------
/**
   Vary basic HUD clock.
   Displays the current simulation time offset from some base. The base time
   is usually synchronized with the server as mission start time.  This hud
   currently only displays minutes:seconds.
*/
class GuiClockHud : public GuiControl
{
   typedef GuiControl Parent;

   bool     mShowFrame;
   bool     mShowFill;

   ColorF   mFillColor;
   ColorF   mFrameColor;
   ColorF   mTextColor;

   S32      mTimeOffset;

public:
   GuiClockHud();

   void setTime(S32 newTime);

   void onRender( Point2I, const RectI &, GuiControl * );
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiClockHud );
};


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiClockHud );

GuiClockHud::GuiClockHud()
{
   mShowFrame = mShowFill = true;
   mFillColor.set(0, 0, 0, 0.5);
   mFrameColor.set(0, 1, 0, 1);
   mTextColor.set( 0, 1, 0, 1 );

   mTimeOffset = 0;
}

void GuiClockHud::initPersistFields()
{
   Parent::initPersistFields();

   addField( "showFill", TypeBool, Offset( mShowFill, GuiClockHud ) );
   addField( "showFrame", TypeBool, Offset( mShowFrame, GuiClockHud ) );
   addField( "fillColor", TypeColorF, Offset( mFillColor, GuiClockHud ) );
   addField( "frameColor", TypeColorF, Offset( mFrameColor, GuiClockHud ) );
   addField( "textColor", TypeColorF, Offset( mTextColor, GuiClockHud ) );
}


//-----------------------------------------------------------------------------

void GuiClockHud::onRender(Point2I offset, const RectI &updateRect,GuiControl *)
{
   // Background first
   if (mShowFill)
      dglDrawRectFill(updateRect, mFillColor);
   
   // Convert ms time into hours, minutes and seconds.
   S32 time = (mTimeOffset + Platform::getVirtualMilliseconds()) / 1000;
   S32 secs = time % 60;
   S32 mins = (time % 3600) / 60;
   S32 hours = time / 3600;

   // Currently only displays min/sec
   char buf[256];
   dSprintf(buf,sizeof(buf), "%02d:%02d",mins,secs);

   // Center the text
   offset.x += (mBounds.extent.x - mProfile->mFont->getStrWidth(buf)) / 2;
   offset.y += (mBounds.extent.y - mProfile->mFont->getHeight()) / 2;
   dglSetBitmapModulation(mTextColor);
   dglDrawText(mProfile->mFont, offset, buf);
   dglClearBitmapModulation();

   // Border last
   if (mShowFrame)
      dglDrawRect(updateRect, mFrameColor);
}


//-----------------------------------------------------------------------------

void GuiClockHud::setTime(S32 time)
{
   mTimeOffset = (time * 1000) - Platform::getVirtualMilliseconds();
}

ConsoleMethod(GuiClockHud,setTime,void,3, 3,"(time in sec)Sets the current base time for the clock")
{
   GuiClockHud *hud = static_cast<GuiClockHud*>(object);
   hud->setTime(dAtoi(argv[2]));
}
