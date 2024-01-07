//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SHELLFANCYTEXTLIST_H_
#define _SHELLFANCYTEXTLIST_H_

#ifndef _SHELLFANCYARRAY_H_
#include "Shell/shellFancyArray.h"
#endif

class ShellFancyTextList : public ShellFancyArray
{
   private:
      typedef ShellFancyArray Parent;

   protected:
		enum 
		{
		   InvalidId = 0xFFFFFFFF
		};

		// Emulate a text list ( to some extent ):
	public:
		struct Entry
		{
			char* text;
			U32 id;
         bool active;
         U32 styleId;
		};

      struct StyleSet
      {
         U32 id;
         StringTableEntry fontType;
         U32 fontSize;
         Resource<GFont> font;
         ColorI fontColor;
         ColorI fontColorHL;
         ColorI fontColorSEL;
         StyleSet* next;

         StyleSet()
         {
            id = 0;
            fontSize = 0;
            font = NULL;
            fontColor.set( 0, 0, 0, 255 );
            fontColorHL.set( 0, 0, 0, 255 );
            fontColorSEL.set( 0, 0, 0, 255 );
            next = NULL;
         };
      };

   protected:
		Vector<Entry> mList;
      DataChunker mResourceChunker;
      StyleSet* mStyleList;

      const StyleSet* getStyleSet( U32 id );

   public:
      DECLARE_CONOBJECT(ShellFancyTextList);
      ShellFancyTextList();
      ~ShellFancyTextList();

		static void consoleInit();
		static void initPersistFields();

      const char* getScriptValue();

		U32   getNumEntries();
		void  clearList();
		void  addEntry( U32 id, const char* text );
      void  insertEntry( U32 id, const char* text, S32 index );
	   void  removeEntry( U32 id );
	   void  removeEntryByIndex( S32 id );
	   void  setEntry( U32 id, const char* text );
      void  setEntryActive( U32 id, bool active );
	   S32   findEntryById( U32 id );
      S32   findEntryByText( const char* text );
      bool  isEntryActive( U32 id );
		void	selectRowById( U32 id );
		U32	getSelectedId();
      U32   getRowId( S32 index );
      const char* getText( S32 index );

      bool  addStyleSet( U32 id, const char* fontName, U32 fontSize, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL );
      void  setEntryStyle( S32 index, U32 styleId );
      U32   getEntryStyle( S32 index );

      void  onCellSelected( S32 row, S32 column );

      bool  onWake();
      void  onSleep();

		void	updateList();
      void  sort();

      void  onRenderCell( Point2I offset, Point2I cell, bool selected, bool mouseOver );
};

#endif // _SHELL_FANCYTEXTLIST_H

