//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBitmapFrameCtrl.h"

IMPLEMENT_CONOBJECT(HudBitmapFrameCtrl);

HudBitmapFrameCtrl::HudBitmapFrameCtrl()
{
   mSubRegion.set(0,0,1,1);
}

void HudBitmapFrameCtrl::onPreRender()
{
   if (mAutoResize && bool(mBitmapHandle))
   {
      Point2I dim(mBitmapHandle.getWidth(), mBitmapHandle.getHeight());
      if (dim != getExtent())
         resize(getPosition(), dim);
   }
}

void HudBitmapFrameCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("subRegion", TypeRectI, Offset(mSubRegion, HudBitmapFrameCtrl));
}
