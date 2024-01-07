//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SHELLSCROLLCTRL_H_
#define _SHELLSCROLLCTRL_H_

#ifndef _GUISCROLLCTRL_H_
#include "GUI/guiScrollCtrl.h"
#endif

class ShellScrollCtrl : public GuiScrollCtrl
{
   private:
      typedef GuiScrollCtrl Parent;

   protected:
		enum BarIndices
		{
			BarVTop = 0,
			BarVCenter,
			BarVBottom,
			BarHLeft,
			BarHCenter,
			BarHRight,

			BarCount
		};

		enum BarStates
		{
			BarNormal = 0,
			BarDisabled,

			BarStateCount
		};

      RectI mBarBounds[BarCount * BarStateCount];
      TextureHandle  mTexVBar;
      TextureHandle  mTexHBar;

      enum BtnIndices
      {
			UpBtn = 0,
			DownBtn,
			VThumbTop,
			VThumb,
			VThumbBottom,
			LeftBtn,
			RightBtn,
			HThumbLeft,
			HThumb,
			HThumbRight,
			Corner,

         BtnCount
      };

      enum BtnStates
      {
         StateNormal = 0,
         StatePressed,
         StateRollover,
         StateDisabled,

         BtnStateCount
      };

      TextureHandle  mTexVButtons;
      TextureHandle  mTexVThumb;
      TextureHandle  mTexHButtons;
      TextureHandle  mTexHThumb;
      TextureHandle  mTexCorner;

   public:
      StringTableEntry  mFieldBase;

   protected:
      TextureHandle  mTexLeftTop;
      TextureHandle  mTexCenterTop;
      TextureHandle  mTexRightTop;
      TextureHandle  mTexLeftCenter;
      TextureHandle  mTexCenter;
      TextureHandle  mTexRightCenter;
      TextureHandle  mTexLeftBottom;
      TextureHandle  mTexCenterBottom;
      TextureHandle  mTexRightBottom;

		Region	mMouseOverRegion;
		U32		mGlowOffset;

	   //bool calcChildExtents( Point2I* pos, Point2I* ext );

   public:
      DECLARE_CONOBJECT(ShellScrollCtrl);
      ShellScrollCtrl();

      static void initPersistFields();

      bool onWake();
      void onSleep();

		U32  getGlowOffset()	{ return( mGlowOffset); }
		
		void onMouseUp( const GuiEvent &event );
		void onMouseMove( const GuiEvent &event );
		void onMouseEnter( const GuiEvent &event );
		void onMouseLeave( const GuiEvent &event );
      
      void drawBorder( const Point2I &offset, bool isFirstResponder );
	   void drawVScrollBar( const Point2I &offset );
	   void drawHScrollBar( const Point2I &offset );
	   void drawScrollCorner( const Point2I &offset );
};

#endif // _SHELL_SCROLLCTRL_H
