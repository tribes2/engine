//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _EDITORCHECKBOXCTRL_H_
#define _EDITORCHECKBOXCTRL_H_

#ifndef _GUICHECKBOXCTRL_H_
#include "GUI/guiCheckBoxCtrl.h"
#endif

// JFF - move into gui once finished

class EditorCheckBoxCtrl : public GuiCheckBoxCtrl
{
   private:
      typedef GuiCheckBoxCtrl Parent;

   protected:
      TextureHandle     mTextureHandle;
      StringTableEntry  mBitmapName;
      ColorI            mMouseOverColor;
      F32               mDepressedAlpha;
         
   public:
      DECLARE_CONOBJECT(EditorCheckBoxCtrl);

	   EditorCheckBoxCtrl();
      
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
