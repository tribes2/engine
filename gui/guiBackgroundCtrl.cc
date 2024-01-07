//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "gui/guiBackgroundCtrl.h"

//--------------------------------------------------------------------------
GuiBackgroundCtrl::GuiBackgroundCtrl() : GuiControl()
{
   mDraw = false;
}

//--------------------------------------------------------------------------
void GuiBackgroundCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   if ( mDraw )
      Parent::onRender( offset, updateRect, firstResponder );

   renderChildControls(offset, updateRect, firstResponder);
}


