//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBitmapCtrl.h"

IMPLEMENT_CONOBJECT(HudBitmapCtrl);

HudBitmapCtrl::HudBitmapCtrl()
{
   mBitmapHandle = 0;

   //
   mBitmap = 0;
   mAutoResize = false;
   mAutoCenter = false;
   mFlipVert = false;
   mFlipHorz = false;
}

void HudBitmapCtrl::setBitmap(const char * bitmap)
{
   mBitmap = StringTable->insert(bitmap);
   mBitmapHandle = TextureHandle(mBitmap, BitmapTexture);
}

//--------------------------------------------------------------------------- 

void HudBitmapCtrl::onPreRender()
{
   GuiControl * parent = getParent();
   if(!parent)
      return;

   // set extent to that of bitmap
   if(mAutoResize && bool(mBitmapHandle))
   {
      Point2I dim(mBitmapHandle.getWidth(), mBitmapHandle.getHeight());
      if (dim != getExtent())
         resize(getPosition(), dim);
   }

   // resize to parent
   if(mAutoCenter)
   {
      Point2I pos = parent->getExtent();
      pos = pos / 2 - (getExtent() + Point2I(1,1)) / 2;
      resize(pos, getExtent());
   }

   Parent::onPreRender();
}

//--------------------------------------------------------------------------- 

void HudBitmapCtrl::onRender(Point2I offset, const RectI & updateRect, GuiControl * firstResponder)
{
   if(!bool(mBitmapHandle))
   {
      Parent::onRender(offset, updateRect, firstResponder);
      return;
   }

   U32 flip = ( mFlipVert ? GFlip_Y : GFlip_None ) | ( mFlipHorz ? GFlip_X : GFlip_None );

   mOpacity = mClampF(mOpacity, 0.f, 1.f);

   //
   dglSetBitmapModulation(ColorF(1,1,1,mOpacity));
   dglDrawBitmapStretch(mBitmapHandle, updateRect, flip);
   dglClearBitmapModulation();

   renderChildControls(offset, updateRect, firstResponder);
}

//--------------------------------------------------------------------------- 

void HudBitmapCtrl::inspectPostApply()
{
   mBitmapHandle = TextureHandle(mBitmap, BitmapTexture);
   Parent::inspectPostApply();
}

//--------------------------------------------------------------------------- 

bool HudBitmapCtrl::onWake()
{
   if(!Parent::onWake())
      return(false);
   mBitmapHandle = TextureHandle(mBitmap, BitmapTexture);
   return(true);
}

void HudBitmapCtrl::onSleep()
{
   mBitmapHandle = 0;
   Parent::onSleep();
}

//--------------------------------------------------------------------------- 

void HudBitmapCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("bitmap", TypeString, Offset(mBitmap, HudBitmapCtrl));
   addField("autoCenter", TypeBool, Offset(mAutoCenter, HudBitmapCtrl));
   addField("autoResize", TypeBool, Offset(mAutoResize, HudBitmapCtrl));
   addField("flipVertical", TypeBool, Offset(mFlipVert, HudBitmapCtrl));
   addField("flipHorizontal", TypeBool, Offset(mFlipHorz, HudBitmapCtrl));
}

//--------------------------------------------------------------------------- 

ConsoleMethod( HudBitmapCtrl, setBitmap, void, 3, 3, "hudBitmapCtrl.setBitmap( bitmap )" )
{
   argc;
   static_cast<HudBitmapCtrl*>( object )->setBitmap( argv[2] );
}
