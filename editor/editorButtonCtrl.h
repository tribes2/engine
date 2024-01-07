//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _EDITORBUTTONCTRL_H_
#define _EDITORBUTTONCTRL_H_

#ifndef _GUIBUTTONCTRL_H_
#include "GUI/guiButtonCtrl.h"
#endif

// JFF - move into gui once finished

class EditorButtonCtrl : public GuiButtonCtrl
{
   private:
      typedef GuiButtonCtrl Parent;

   protected:      
      TextureHandle     mTextureHandle;
      StringTableEntry  mBitmapName;
      ColorI            mMouseOverColor;
      F32               mDepressedAlpha;
      
   public:
      DECLARE_CONOBJECT(EditorButtonCtrl);

	   EditorButtonCtrl();
      
      // SimObject
      static void initPersistFields();
      static void consoleInit();

      // GuiControl
      bool onWake();
      void onSleep();

      void setBitmap(const char *name);
      void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
};

#endif
