//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _HUDOBJECT_H_
#define _HUDOBJECT_H_

#include "gui/guiControl.h"

/**
 * This class is the top level as far as far as HUD controls go.
 * This will allow for inherited behavior such as hud-movers etc.
 */
class HudObject : public GuiControl {
   private:
      typedef GuiControl Parent;
      
   protected:
      void drawText( Point2I offset, char *buf );
   
   public:
   
      HudObject();

      // GuiControl
      virtual void onRender( Point2I offset, 
                             const RectI &updateRect, 
                             GuiControl *firstResponder );

      // field data
      ColorF   mFillColor;
      ColorF   mFrameColor;
      ColorF   mTextColor;
      ColorF   mShadowColor;
      F32      mOpacity;
      bool     mShowFrame;
      bool     mShowFill;
      bool     mUse3dText;
      

      static void initPersistFields();

      DECLARE_CONOBJECT( HudObject );
};

#endif

// hudObject.h
