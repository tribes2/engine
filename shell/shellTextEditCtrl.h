//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SHELLTEXTEDITCTRL_H_
#define _SHELLTEXTEDITCTRL_H_

#ifndef _GUITEXTEDITCTRL_H_
#include "GUI/guiTextEditCtrl.h"
#endif

class ShellTextEditCtrl : public GuiTextEditCtrl
{
   private:
      typedef GuiTextEditCtrl Parent;

   protected:
      enum BitmapIndices
      {
         BmpLeft,
         BmpCenter,
         BmpRight,

         BmpCount
      };

      enum BitmapStates
      {
         StateNormal,
         StateActive,

         StateCount
      };
      
      RectI mBitmapBounds[BmpCount * StateCount];
      TextureHandle  mTexField;
		Point2I	mGlowOffset;

   public:
      DECLARE_CONOBJECT(ShellTextEditCtrl);
      ShellTextEditCtrl();

		static void initPersistFields();

      bool onWake();
      void onSleep();
      
      void onRender( Point2I offset, const RectI &updateRect, GuiControl *firstResponder );
};

#endif // _SHELL_TEXTEDITCTRL_H
