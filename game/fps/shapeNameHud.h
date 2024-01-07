//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
//-----------------------------------------------------------------------------

#ifndef _SHAPENAMEHUD_H_
#define _SHAPENAMEHUD_H_

#include "gui/guiControl.h"

class ShapeNameHud : public GuiControl {
   typedef GuiControl Parent;

   // field data
   ColorF   mFillColor;
   ColorF   mFrameColor;
   ColorF   mTextColor;
   ColorF   mDamageFillColor;
   ColorF   mDamageFrameColor;
   Point2I  mDamageRectSize;

   F32      mOpacity;
   F32      mVerticalOffset;
   F32      mDistanceFade;
   bool     mShowFrame;
   bool     mShowFill;
   
protected:
   void drawName( Point2I offset, const char *buf, F32 opacity);
   void drawDamage(Point2I offset, F32 damage, F32 opacity);

public:
   ShapeNameHud();

   // GuiControl
   virtual void onRender(Point2I offset, 
      const RectI &updateRect, GuiControl *firstResponder );
   
   static void initPersistFields();
   DECLARE_CONOBJECT( ShapeNameHud );
};

#endif

