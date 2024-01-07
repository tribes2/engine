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
   A basic health bar control.
   This gui displays the damage value of the current PlayerObjectType
   control object.  The gui can be set to pulse if the health value
   drops below a set value. This control only works if a server
   connection exists and it's control object is a PlayerObjectType. If
   either of these requirements is false, the control is not rendered.
*/
class GuiHealthBarHud : public GuiControl
{
   typedef GuiControl Parent;

   bool     mShowFrame;
   bool     mShowFill;

   ColorF   mFillColor;
   ColorF   mFrameColor;
   ColorF   mDamageFillColor;

   S32      mPulseRate;
   F32      mPulseThreshold;

   F32      mValue;

public:
   GuiHealthBarHud();

   void onRender( Point2I, const RectI &, GuiControl * );
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiHealthBarHud );
};


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiHealthBarHud );

GuiHealthBarHud::GuiHealthBarHud()
{
   mShowFrame = mShowFill = true;
   mFillColor.set(0, 0, 0, 0.5);
   mFrameColor.set(0, 1, 0, 1);
   mDamageFillColor.set(0, 1, 0, 1);

   mPulseRate = 0;
   mPulseThreshold = 0.3f;
   mValue = 0.2f;
}

void GuiHealthBarHud::initPersistFields()
{
   Parent::initPersistFields();

   addField( "showFill", TypeBool, Offset( mShowFill, GuiHealthBarHud ) );
   addField( "showFrame", TypeBool, Offset( mShowFrame, GuiHealthBarHud ) );
   addField( "fillColor", TypeColorF, Offset( mFillColor, GuiHealthBarHud ) );
   addField( "frameColor", TypeColorF, Offset( mFrameColor, GuiHealthBarHud ) );
   addField( "damageFillColor", TypeColorF, Offset( mDamageFillColor, GuiHealthBarHud ) );
   addField( "pulseRate", TypeS32, Offset( mPulseRate, GuiHealthBarHud ) );
   addField( "pulseThreshold", TypeF32, Offset( mPulseThreshold, GuiHealthBarHud ) );
}


//-----------------------------------------------------------------------------
/**
   Gui onRender method.
   Renders a health bar with filled background and border.
*/
void GuiHealthBarHud::onRender(Point2I offset, const RectI &updateRect,GuiControl *)
{
   // Must have a connection and player control object
   GameConnection* conn = GameConnection::getServerConnection();
   if (!conn)
      return;
   ShapeBase* control = conn->getControlObject();
   if (!control || !(control->getType() & PlayerObjectType))
      return;

   // We'll just grab the damage right off the control object.
   // Damage value 0 = no damage.
   mValue = 1 - control->getDamageValue();

   // Background first
   if (mShowFill)
      dglDrawRectFill(updateRect, mFillColor);
   
   // Pulse the damage fill if it's below the threshold
   if (mPulseRate != 0)
      if (mValue < mPulseThreshold) {
         F32 time = Platform::getVirtualMilliseconds();
         F32 alpha = mFmod(time,mPulseRate) / (mPulseRate / 2.0);
         mDamageFillColor.alpha = (alpha > 1.0)? 2.0 - alpha: alpha;
      }
      else
         mDamageFillColor.alpha = 1;

   // Render damage fill %
   RectI rect(updateRect);
   rect.extent.x *= mValue;
   dglDrawRectFill(rect, mDamageFillColor);
   
   // Border last
   if (mShowFrame)
      dglDrawRect(updateRect, mFrameColor);
}
