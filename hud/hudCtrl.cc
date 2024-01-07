//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudCtrl.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"

IMPLEMENT_CONOBJECT(HudCtrl);

HudCtrl::HudCtrl()
{
   mFillColor.set(0.25, 0.25, 0.25, 0.25);
   mFrameColor.set(0, 1, 0, 1);
   mOpacity = 1.f;
}

//--------------------------------------------------------------------------

void HudCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl * firstResponder)
{
   RectI ctrlRect(offset, mBounds.extent);
   
   //
   mFillColor.alpha = mFrameColor.alpha = mOpacity;

   dglDrawRectFill(ctrlRect, mFillColor);
   dglDrawRect(ctrlRect, mFrameColor);   
   
   renderChildControls(offset, updateRect, firstResponder);
}

void HudCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("fillColor", TypeColorF, Offset(mFillColor, HudCtrl));
   addField("frameColor", TypeColorF, Offset(mFrameColor, HudCtrl));
   addField("opacity", TypeF32, Offset(mOpacity, HudCtrl));
}
