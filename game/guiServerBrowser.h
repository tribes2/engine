//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUISERVERBROWSER_H_
#define _GUISERVERBROWSER_H_

#ifndef _SHELLFANCYARRAY_H_
#include "Shell/shellFancyArray.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

class GuiServerBrowser : public ShellFancyArray
{
   private:
      typedef ShellFancyArray Parent;

   public:
      enum ColumnKey
      {
         Name_Column = 0,		// 0
         Status_Column,			// 1
         Favorite_Column,		// 2
         Ping_Column,			// 3
         MissionType_Column,	// 4
         Map_Column,				// 5
         GameType_Column,		// 6
         Players_Column,		// 7
         CPU_Column,				// 8
         IP_Column,				// 9
         Version_Column,      // 10

         ColumnCount
      };
      
   protected:
		StringTableEntry	mIconBase;

      TextureHandle  mTexFavorite;
      TextureHandle  mTexFavoriteHI;
      TextureHandle  mTexNotQueried;
      TextureHandle  mTexNotQueriedHI;
      TextureHandle  mTexQuerying;
      TextureHandle  mTexQueryingHI;
      TextureHandle  mTexTimedOut;
      TextureHandle  mTexDedicated;
      TextureHandle  mTexDedicatedHI;
      TextureHandle  mTexPassworded;
      TextureHandle  mTexPasswordedHI;
      TextureHandle  mTexTournament;
      TextureHandle  mTexTournamentHI;

      NetAddress  mSelectedAddress;

   public:
      DECLARE_CONOBJECT(GuiServerBrowser);
      GuiServerBrowser();

      // GuiControl overrides:
		static void initPersistFields();

      const char* getScriptValue();

      bool  onWake();
      void  onSleep();

      void  onPreRender();

      // ShellFancyArray overrides:
		void	updateList();
      void  sort();
      void  onCellSelected( S32 row, S32 column );
      void  onRenderCell( Point2I offset, Point2I cell, bool selected, bool mouseOver );

      // Class-specific functions:
      void  selectRowByAddress();

      const char* getSelectedServerStatus();
      const char* getSelectedServerInfoString();
      const char* getSelectedServerContentString();
};

#endif // _GUI_SERVERBROWSER_H

