//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICHATMENUTREECTRL_H_
#define _GUICHATMENUTREECTRL_H_

#ifndef _GUITREEVIEWCTRL_H_
#include "GUI/guiTreeViewCtrl.h"
#endif

class GuiChatMenuTreeCtrl : public GuiTreeViewCtrl
{
   private:
      typedef GuiTreeViewCtrl Parent;

	protected:
      enum BmpIndices
      {
         BmpDunno,
         BmpLastChild,
         BmpChild,
         BmpExp,
         BmpExpN,
         BmpExpP,
         BmpExpPN,
         BmpCon,
         BmpConN,
         BmpConP,
         BmpConPN,
         BmpLine,
         BmpGlow,
      };

		TextureHandle	mTexRollover;
		TextureHandle	mTexSelected;

      ColorI   mAltFontColor;
      ColorI   mAltFontColorHL;
      ColorI   mAltFontColorSE;

   public:
      DECLARE_CONOBJECT(GuiChatMenuTreeCtrl);
		GuiChatMenuTreeCtrl();

      static void consoleInit();
      static void initPersistFields();

		bool onWake();
		void onSleep();

      bool onKeyDown( const GuiEvent& event );

      void onRenderCell( Point2I offset, Point2I cell, bool, bool );
};

#endif // _GUI_CHATMENUTREECTRL_H
