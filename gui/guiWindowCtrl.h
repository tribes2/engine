//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIWINDOWCTRL_H_
#define _GUIWINDOWCTRL_H_

#ifndef _GUITEXTCTRL_H_
#include "GUI/guiTextCtrl.h"
#endif

class GuiWindowCtrl : public GuiTextCtrl
{
	private:
		typedef GuiTextCtrl Parent;

		bool mResizeWidth;
		bool mResizeHeight;
		bool mCanMove;
      bool mCanClose;
      bool mCanMinimize;
      bool mCanMaximize;
		bool mPressClose;
		bool mPressMinimize;
		bool mPressMaximize;
		Point2I mMinSize;

		StringTableEntry mCloseCommand;

      S32 mTitleHeight;
      S32 mResizeRightWidth;
      S32 mResizeBottomHeight;

		bool mMouseMovingWin;
		bool mMouseResizeWidth;
		bool mMouseResizeHeight;
		bool mMinimized;
		bool mMaximized;

		Point2I mMouseDownPosition;
		RectI mOrigBounds;
		RectI mStandardBounds;

		RectI mCloseButton;
		RectI mMinimizeButton;
		RectI mMaximizeButton;
		S32 mMinimizeIndex;
		S32 mTabIndex;

	protected:
	   enum BitmapIndices
	   {
	      BmpClose,
	      BmpMaximize,
	      BmpNormal,
	      BmpMinimize,

			BmpCount
	   };

		enum BitmapStates
		{
			BmpDefault = 0,
			BmpHilite,
			BmpDisabled,

			BmpStates
		};
		RectI mBitmapBounds[BmpStates * BmpCount];  //bmp is [3*n], bmpHL is [3*n + 1], bmpNA is [3*n + 2]
		TextureHandle mTextureHandle;


		void drawWinRect(const RectI &myRect);

	public:
		GuiWindowCtrl();
	   DECLARE_CONOBJECT(GuiWindowCtrl);
	   static void initPersistFields();
		static void consoleInit();

      bool onWake();

		bool isMinimized(S32 &index);
      
      void setFont(S32 fntTag);

		GuiControl* findHitControl(const Point2I &pt, S32 initialLayer = -1);
		void resize(const Point2I &newPosition, const Point2I &newExtent);

	   void onMouseDown(const GuiEvent &event);
	   void onMouseDragged(const GuiEvent &event);
	   void onMouseUp(const GuiEvent &event);

		//only cycle tabs through the current window, so overwrite the method
		GuiControl* findNextTabable(GuiControl *curResponder, bool firstCall = true);
		GuiControl* findPrevTabable(GuiControl *curResponder, bool firstCall = true);

	   bool onKeyDown(const GuiEvent &event);

		S32 getTabIndex(void) { return mTabIndex; }
		void selectWindow(void);

	   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
};

#endif //_GUI_WINDOW_CTRL_H
