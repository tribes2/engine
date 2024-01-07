//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "gui/guiControl.h"
#include "gui/guiBitmapCtrl.h"
#include "console/consoleTypes.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"

//-----------------------------------------------------------------------------
/**
   Vary basic cross hair hud.
   Use the base bitmap control to render a bitmap, and only decides whether
   to draw or not depending on the current control object and it's state.
   This simple behavior would normally be scripted, but we're going to
   be adding new feature to this class.
*/
class GuiCrossHairHud : public GuiBitmapCtrl
{
   typedef GuiBitmapCtrl Parent;

public:
   GuiCrossHairHud();

   void onRender( Point2I, const RectI &, GuiControl * );
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiCrossHairHud );
};

/// Valid object types for which the cross hair will render, this
/// should really all be script controlled.
static const U32 ObjectMask = PlayerObjectType | VehicleObjectType;


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiCrossHairHud );

GuiCrossHairHud::GuiCrossHairHud()
{
}

void GuiCrossHairHud::initPersistFields()
{
   Parent::initPersistFields();
}


//-----------------------------------------------------------------------------

void GuiCrossHairHud::onRender(Point2I offset, const RectI &updateRect,GuiControl *ctrl)
{
   // Must have a connection and player control object
   GameConnection* conn = GameConnection::getServerConnection();
   if (!conn)
      return;
   ShapeBase* control = conn->getControlObject();
   if (!control || !(control->getType() & ObjectMask) || !conn->isFirstPerson())
      return;

   // Parent render.
   Parent::onRender(offset,updateRect,ctrl);
}


