//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUISCROLLCTRL_H_
#define _GUISCROLLCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

// the function of the scroll content control class
// is to notify the parent class that children have resized.
// basically it just calls it's parent (enclosing) control's
// childResized method which turns around and computes new sizes
// for the scroll bars

class GuiScrollContentCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;
   
protected:
   void childResized(GuiControl *child);
   void removeObject(SimObject *object);

public:
   DECLARE_CONOBJECT(GuiScrollContentCtrl);
};

class GuiScrollCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;

protected:
   // the scroll control uses a bitmap array to draw all its
   // stuff... these are the bitmaps it needs:

   enum BitmapIndices
   {
      BmpUp,
      BmpDown,
      BmpVThumbTopCap,
      BmpVThumb,
      BmpVThumbBottomCap,
      BmpVPage,
      BmpLeft,
      BmpRight,
      BmpHThumbLeftCap,
      BmpHThumb,
      BmpHThumbRightCap,
      BmpHPage,
      BmpResize,

		BmpCount
   };

	enum BitmapStates
	{
		BmpDefault = 0,
		BmpHilite,
		BmpDisabled,

		BmpStates
	};
	RectI* mBitmapBounds;  //bmp is [3*n], bmpHL is [3*n + 1], bmpNA is [3*n + 2]
	TextureHandle mTextureHandle;

   GuiScrollContentCtrl *mContentCtrl;  // always have a pointer to the content control

   S32     mBorderThickness;           // this gets set per class in the constructor
   Point2I mChildMargin;               // the thickeness of the margin around the child controls
   // note - it is implicit in the scroll view that the buttons all have the same
   // arrow length and that horizontal and vertical scroll bars have the
   // same thickness

   S32 mScrollBarThickness;        // determined by the width of the vertical page bmp
   S32 mScrollBarArrowBtnLength;   // determined by the height of the up arrow

   bool mHBarEnabled;
   bool mVBarEnabled;
   bool mHasHScrollBar;
   bool mHasVScrollBar;

   F32 mHBarThumbPos;
   F32 mHBarThumbWidth;
   F32 mVBarThumbPos;
   F32 mVBarThumbWidth;

   S32 mHThumbSize;
   S32 mHThumbPos;
   
   S32 mVThumbSize;
   S32 mVThumbPos;

   S32 mBaseThumbSize;

   RectI mUpArrowRect;
   RectI mDownArrowRect;
   RectI mLeftArrowRect;
   RectI mRightArrowRect;
   RectI mHTrackRect;
   RectI mVTrackRect;

   S32 mDefaultLineHeight; // line height for scroll controls that DO NOT have array controls in them
                           // ignored if left at zero 

   F32 mLine_V;     // percentage to scroll line Vertically
   F32 mLine_H;
   F32 mPage_V;     // percentage to scroll page Vertically
   F32 mPage_H;


   //--------------------------------------
   // for determing hit area   
public:		//called by the ComboPopUp class
   enum Region 
   { 
      UpArrow, 
      DownArrow,
      LeftArrow,
      RightArrow, 
      UpPage, 
      DownPage, 
      LeftPage,
      RightPage,
      VertThumb,
      HorizThumb,
      None
   };
   Region findHitRegion(const Point2I &);

protected:
   bool stateDepressed;
   Region curHitRegion;

   virtual bool calcChildExtents(Point2I *pos, Point2I *ext);
	virtual void calcScrollRects(void);
   void calcThumbs(Point2I cpos, Point2I cext);
   void scrollByRegion(Region reg);
   
   //--------------------------------------
   
   //--------------------------------------
   // for mouse dragging the thumb
   F32 mThumbAnchorPos;
   S32 mThumbDelta;
   //--------------------------------------

public:
   GuiScrollCtrl();
   ~GuiScrollCtrl();
   DECLARE_CONOBJECT(GuiScrollCtrl);
	static void consoleInit();
   static void initPersistFields();
	void autoScroll(Region reg);
	bool disabled;

   F32 getCurrVPos() const;
   F32 getCurrHPos() const;

   void scrollTo(F32 x, F32 y);
   void computeSizes();

   enum {
      ScrollBarAlwaysOn = 0,
      ScrollBarAlwaysOff = 1,
      ScrollBarDynamic = 2
   };
   // you can change the bitmap array dynamically.
   void loadBitmapArray();

   S32 mForceHScrollBar;
   S32 mForceVScrollBar;

   bool mUseConstantHeightThumb;
   bool mWillFirstRespond;     // for automatically handling arrow keys

   GuiScrollContentCtrl *getScrollContentCtrl() { return mContentCtrl; }

   void addObject(SimObject *object);
   void resize(const Point2I &newPos, const Point2I &newExt);
	S32 getBorderThickness(void) { return mBorderThickness; }
   S32 scrollBarThickness() const                        { return(mScrollBarThickness); }
   S32 scrollBarArrowBtnLength() const                   { return(mScrollBarArrowBtnLength); }
   bool hasHScrollBar() const                            { return(mHasHScrollBar); }
   bool hasVScrollBar() const                            { return(mHasVScrollBar); }
   bool enabledHScrollBar() const                        { return(mHBarEnabled); }
   bool enabledVScrollBar() const                        { return(mVBarEnabled); }

   bool wantsTabListMembership();
   bool becomeFirstResponder();
   bool loseFirstResponder();

   Region getCurHitRegion(void) { return curHitRegion; }

   bool onKeyDown(const GuiEvent &event);
   void onMouseDown(const GuiEvent &event);
   void onMouseRepeat(const GuiEvent &event);
   void onMouseUp(const GuiEvent &event);
   void onMouseDragged(const GuiEvent &event);
   bool onMouseWheelUp(const GuiEvent &event);
   bool onMouseWheelDown(const GuiEvent &event);

   bool onAdd();
   bool onWake();
   void onSleep();

   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   virtual void drawBorder(const Point2I &offset, bool isFirstResponder);
   virtual void drawVScrollBar(const Point2I &offset);
   virtual void drawHScrollBar(const Point2I &offset);
   virtual void drawScrollCorner(const Point2I &offset);
};   


//--------------------------------------------------------------------------
inline F32 GuiScrollCtrl::getCurrVPos() const
{
   if (mHasVScrollBar)
      return mVBarThumbPos;
   else
      return 1.0;
}

inline F32 GuiScrollCtrl::getCurrHPos() const
{
   if (mHasVScrollBar)
      return mVBarThumbPos;
   else
      return 1.0;
}

#endif //_GUI_SCROLL_CTRL_H
