//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "gui/guiCanvas.h"
#include "hud/hudBitmapCtrl.h"
#include "game/gameConnection.h"
#include "game/camera.h"

//--------------------------------------------------------------------------- 
// Class: HudCrosshairCtrl
//--------------------------------------------------------------------------- 
class HudCrosshairCtrl : public HudBitmapCtrl
{
   private:
      typedef HudBitmapCtrl   Parent;
      
   public:   
      HudCrosshairCtrl();

      // GuiControl
      void onRender(Point2I, const RectI &, GuiControl *);

      DECLARE_CONOBJECT(HudCrosshairCtrl);
};

IMPLEMENT_CONOBJECT(HudCrosshairCtrl);

//--------------------------------------------------------------------------- 

HudCrosshairCtrl::HudCrosshairCtrl()
{
   mAutoResize = true;
   mAutoCenter = true;
}

//--------------------------------------------------------------------------- 

void HudCrosshairCtrl::onRender(Point2I offset, const RectI & updateRect, GuiControl * firstResponder)
{
   if(!bool(mBitmapHandle))
      return;

   // renders in center of parent control
   GuiControl * parent = getParent();
   if(!parent)
      return;

   GameConnection * con = GameConnection::getServerConnection();
   if(!con)
      return;

   if(!con->isFirstPerson())
      return;

   if(dynamic_cast<Camera*>(con->getControlObject()))
      return;
      
   Parent::onRender(offset, updateRect, firstResponder);
}
