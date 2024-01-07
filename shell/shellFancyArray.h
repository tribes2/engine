//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SHELLFANCYARRAY_H_
#define _SHELLFANCYARRAY_H_

#ifndef _GUITYPES_H_
#include "gui/guiTypes.h"
#endif
#ifndef _SHELLSCROLLCTRL_H_
#include "shell/shellScrollCtrl.h"
#endif

//------------------------------------------------------------------------------
class VirtualScrollContentCtrl : public GuiScrollContentCtrl
{
	private:
		typedef GuiScrollContentCtrl Parent;

	public:
		GuiControl*	mVirtualContent;

      DECLARE_CONOBJECT(VirtualScrollContentCtrl);
		VirtualScrollContentCtrl();

		bool onAdd(); 
		void addObject( SimObject* obj );
		void removeObject( SimObject* obj );
};


//------------------------------------------------------------------------------
class VirtualScrollCtrl : public ShellScrollCtrl
{
	private:
		typedef ShellScrollCtrl Parent;

	public:
      DECLARE_CONOBJECT(VirtualScrollCtrl);
      VirtualScrollCtrl();

		void setVirtualContent( GuiControl* control );
		GuiControl* getVirtualContent();

		bool onAdd(); 
		void addObject( SimObject* obj );
};


//------------------------------------------------------------------------------
class ShellFancyArray : public GuiControl
{
   private:
      typedef GuiControl Parent;

   protected:
		// Bitmap defines for the header:
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
			StatePressed,
			StateRollover,

			StateCount
		};

		RectI	mBmpBounds[BmpCount * StateCount];
		StringTableEntry	mHeaderBitmap;
		TextureHandle	   mTexHeader;
      StringTableEntry  mSortArrowBitmap;
      TextureHandle     mTexSortArrow;

		StringTableEntry	mBarBase;
		TextureHandle	   mTexCellSelected;
		TextureHandle	   mTexCellRollover;

		// Field stuff:
		StringTableEntry	mFieldBase;
      TextureHandle     mTexLeftTop;
      TextureHandle     mTexCenterTop;
      TextureHandle     mTexRightTop;
      TextureHandle     mTexLeftCenter;
      TextureHandle     mTexCenter;
      TextureHandle     mTexRightCenter;
      TextureHandle     mTexLeftBottom;
      TextureHandle     mTexCenterBottom;
      TextureHandle     mTexRightBottom;

		// Do not change after construction:
      enum ColumnFlags
      {
         Column_Numeric = BIT(0),
         Column_Center  = BIT(1),
         Column_Right   = BIT(2),
         Column_Icon    = BIT(3),
      };

		struct ColumnInfo
		{
			StringTableEntry	name;
			S32					width;
			S32					minWidth;
			S32					maxWidth;
			S32					key;
         BitSet32          flags;
		};

		bool		mFixedHorizontal;
		Vector<ColumnInfo> mColumnInfoList;
		S32		mNumColumns;
		S32		mRowHeight;
		S32		mHeaderHeight;
		S32		mGlowOffset;
		S32		mMinColumnWidth;
		Point2I	mStartScrollRgn;

		Resource<GFont>	mFont;
      Resource<GFont>   mHeaderFont;

      StringTableEntry  mHeaderFontType;
      S32               mHeaderFontSize;

		GuiCursor*	mDefaultCursor;
      GuiCursor*  mResizeCursor;
      GuiCursor*  mRepositionCursor;

		ColorI	mHeaderFontColor;
		ColorI	mHeaderFontColorHL;
		ColorI	mSeparatorColor;

		bool	mDrawCellSeparators;
      bool  mHeaderSort;
      bool  mAllowReposition;
      bool  mNoSelect;

		// State variables:
      S32		mNumRows;
      S32		mSelectedRow;
      S32		mMouseOverRow;
      S32		mMouseOverColumn;
      Point2I	mScrollPanePos;

		enum ColumnState
		{
			None,
			Resizing,
			Repositioning,
			Sorting,
         SecondarySorting, // For derived classes
		};

		ColumnState	mColumnState;
		S32			mActiveColumn;
		Point2I		mDragAnchor;
		
		// Sorting variables:
		S32	mSortColumnKey;
      S32   mSecondarySortColumnKey;
		S32	mSortInc;
      S32   mSecondarySortInc;

		// Column resizing work variables:
		S32	mAbsResizeLeftMargin;
		S32	mAbsResizeRightMargin;
		bool	mResizeFixedColumn;
		S32	mResizeColumnOrigSize;

		// Column reposition work variables:
		S32		mRepositionColumnTo;
		Point2I	mRepositionCursorPos;

		void	determineRepositionColumn( Point2I pt );

      bool  getColumnScrollViewRect( RectI &rect );
      bool  getRowScrollViewRect( RectI &rect );

      void  setMouseOverRow( S32 row );
      void  setMouseOverColumn( S32 column, S32 xPos = 0 );

      S32   findColumn( S32 key );
      void  drawColumn( Point2I offset, RectI clipRect, bool hasRowScrollRect, RectI rowScrollRect, S32 column );

	public:
      DECLARE_CONOBJECT(ShellFancyArray);
      ShellFancyArray();

      bool  onAdd();
      void  onRemove();

      static void initPersistFields();
      static void consoleInit();

      bool  onWake();
      void  onSleep();
		
		void	setFixedHorizontal()	{ mFixedHorizontal = true; }
		void	clearColumns();
		void	addColumn( S32 key, const char* name, S32 defaultWidth, S32 minWidth, S32 maxWidth, const char* flags = NULL );
		S32	getNumColumns()	{ return( mNumColumns ); }
		
      virtual void clearList();
		virtual void updateList();

      bool  pointInColumn( bool inHeader, Point2I pt, S32 &column, bool &inResizeRgn, bool &resizeLeft );
      bool  getScrollRect( RectI &rect );

      void  setSortColumnKey( S32 newKey );
      void  setSortInc( bool newSortInc ) {  mSortInc = newSortInc; sort(); }
      void  setSecondarySortColumnKey( S32 newKey );
      void  setSecondarySortInc( bool newSortInc ) { mSecondarySortInc = newSortInc; sort(); }
      virtual void sort();

      void  selectCell( S32 row, S32 column );
      S32   getSelectedRow()  { return( mSelectedRow ); }
      void  setNumRows( S32 numRows );
      U32   getNumRows()      { return( mNumRows ); }

		S32	getGlowOffset()	{ return( mGlowOffset ); }
      S32   getHeaderHeight() { return( mHeaderHeight ); }
      S32   getRowHeight()    { return( mRowHeight ); }

      S32   getNoScrollWidth();
      S32   getNoScrollHeight()  { return( mHeaderHeight + ( mRowHeight * mStartScrollRgn.y ) ); }

      S32   getColumnKey( S32 index );
      S32   getColumnWidth( S32 index );
      S32   getSortColumnKey()   { return( mSortColumnKey ); }
      bool  getSortIncreasing()  { return( mSortInc ); } 
      S32   getSecondarySortColumnKey()   { return( mSecondarySortColumnKey ); }
      bool  getSecondarySortIncreasing()  { return( mSecondarySortInc ); } 

      virtual void onHeaderAction( S32 column );
      virtual void onSecondaryHeaderAction( S32 column );
      virtual void onCellSelected( S32 row, S32 column );

      void  forceFillScrollRegion();
      void  scrollSelectedRowVisible();

      void  computeFixedResizingVals();
      void  resizeFixedColumn( const GuiEvent &event );
      void  resizeScrollColumn( const GuiEvent &event );

      Point2I  getScrollPos() { return mScrollPanePos; }
      void     setScrollPos( Point2I newPos )   { mScrollPanePos = newPos; }
      Point2I  getScrollExtent();

      void  onMouseDown( const GuiEvent& event );
      void  onMouseUp( const GuiEvent& event );
      void  onMouseMove( const GuiEvent& event );
      void  onMouseDragged( const GuiEvent& event );
      void  onMouseEnter( const GuiEvent &event );
      void  onMouseLeave( const GuiEvent &event );
      void  onRightMouseDown( const GuiEvent &event );
      void  onRightMouseUp( const GuiEvent &event );
      bool  onMouseWheelUp( const GuiEvent &event );
      bool  onMouseWheelDown( const GuiEvent &event );
      bool  onKeyDown( const GuiEvent& event );

		void  resize( const Point2I &newPos, const Point2I &newExtent );

	   void  onRenderColumnHeader( Point2I offset, RectI clipRect, S32 column, bool mouseOver );
      virtual void onRenderCell( Point2I offset, Point2I cell, bool selected, bool mouseOver );
      void  onRender( Point2I offset, const RectI &updateRect, GuiControl* firstResponder );
};


//------------------------------------------------------------------------------
class ShellFancyArrayScrollCtrl : public GuiControl
{
	private:
		typedef GuiControl Parent;

	protected:
		ShellFancyArray*		mArray;
		VirtualScrollCtrl*	mScrollView;
		GuiControl*				mVirtualContent;

		Point2I	mPrevArrayPos;
		Point2I	mPrevArrayExtent;
		Point2I	mPrevContentPos;

		bool	mFixedHorizontal;

      StringTableEntry  mVSpacerBitmap;
      StringTableEntry  mHSpacerBitmap;
      TextureHandle     mTexVSpacer;
      TextureHandle     mTexHSpacer;

	public:
      DECLARE_CONOBJECT(ShellFancyArrayScrollCtrl);
      ShellFancyArrayScrollCtrl();

		static void initPersistFields();

		bool onAdd();

		void addObject( SimObject* obj );
		void removeObject( SimObject* obj );

      bool onWake();
      void onSleep();

		void setVirtualContent( GuiControl* control )	{ mVirtualContent = control; }
		void positionChildren();
		void resize( const Point2I &newPos, const Point2I &newExtent );

		void onPreRender();
		void onRender( Point2I offset, const RectI &updateRect, GuiControl* firstResponder );
};

#endif // _SHELL_FANCYARRAY_H
