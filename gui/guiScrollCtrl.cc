//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "console/console.h"
#include "dgl/gBitmap.h"
#include "dgl/gTexManager.h"
#include "Core/resManager.h"
#include "Platform/event.h"
#include "dgl/dgl.h"
#include "GUI/guiArrayCtrl.h"
#include "GUI/guiScrollCtrl.h"

void GuiScrollContentCtrl::childResized(GuiControl *child)
{
   GuiControl *parent = getParent();
   if (parent)
   {
      ((GuiScrollCtrl *)parent)->computeSizes();
   }

   Parent::childResized(child);
}

void GuiScrollContentCtrl::removeObject(SimObject *object)
{
   Parent::removeObject(object);
   GuiScrollCtrl *sctrl = NULL;
   GuiControl *parent = getParent();
   if (parent)
   {
      sctrl = dynamic_cast<GuiScrollCtrl *>(parent);
   }
   if (sctrl)
   {
      sctrl->computeSizes();
   }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

GuiScrollCtrl::GuiScrollCtrl()
{
   mBounds.extent.set(200,200);
   mChildMargin.set(0,0);
   mBorderThickness = 1;
   mScrollBarThickness = 16;
   mScrollBarArrowBtnLength = 16;
   stateDepressed = false;
   curHitRegion = None;
   
   mBitmapBounds = NULL;

   mLine_V = 0.1f;
   mLine_H = 0.1f;
   mPage_V = 0.2f;
   mPage_H = 0.2f;
   
   mWillFirstRespond = true;
   mThumbAnchorPos = 0.0f;
   mUseConstantHeightThumb = false;

   mForceVScrollBar = ScrollBarAlwaysOn;
   mForceHScrollBar = ScrollBarAlwaysOn;

   mVBarThumbPos = 0;
   mHBarThumbPos = 0;

   mDefaultLineHeight = 15;

   mContentCtrl = NULL;
}

GuiScrollCtrl::~GuiScrollCtrl()
{
	if ( mBitmapBounds != NULL )
	{
		delete [] mBitmapBounds;
		mBitmapBounds = NULL;
	}
}

static EnumTable::Enums scrollBarEnums[] = 
{
	{ GuiScrollCtrl::ScrollBarAlwaysOn,     "alwaysOn"     },
	{ GuiScrollCtrl::ScrollBarAlwaysOff,    "alwaysOff"    },
	{ GuiScrollCtrl::ScrollBarDynamic,      "dynamic"      },
};
static EnumTable gScrollBarTable(3, &scrollBarEnums[0]); 

static void cGuiScrollCtrlScrollToTop( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<GuiScrollCtrl*>( obj ), "Control passed to cGuiScrollCtrlScrollToTop is not a GuiScrollCtrl!" );
	GuiScrollCtrl* control = static_cast<GuiScrollCtrl*>( obj );
	control->scrollTo( 0, 0 );
}

static void cGuiScrollCtrlScrollToBottom( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<GuiScrollCtrl*>( obj ), "Control passed to cGuiScrollCtrlScrollToBottom is not a GuiScrollCtrl!" );
	GuiScrollCtrl* control = static_cast<GuiScrollCtrl*>( obj );
	control->scrollTo( 0, 1 );
}

void GuiScrollCtrl::consoleInit()
{
	Con::addCommand( "GuiScrollCtrl", "scrollToTop",      cGuiScrollCtrlScrollToTop,    "control.scrollToTop();", 2, 2 );
	Con::addCommand( "GuiScrollCtrl", "scrollToBottom",   cGuiScrollCtrlScrollToBottom, "control.scrollToBottom();", 2, 2 );
}

void GuiScrollCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addField("willFirstRespond",     TypeBool,    Offset(mWillFirstRespond, GuiScrollCtrl));
   addField("hScrollBar",           TypeEnum,    Offset(mForceHScrollBar, GuiScrollCtrl), 1, &gScrollBarTable);
   addField("vScrollBar",           TypeEnum,    Offset(mForceVScrollBar, GuiScrollCtrl), 1, &gScrollBarTable);
   addField("constantThumbHeight",  TypeBool,    Offset(mUseConstantHeightThumb, GuiScrollCtrl));
   addField("defaultLineHeight",    TypeS32,     Offset(mDefaultLineHeight, GuiScrollCtrl));
   addField("childMargin",          TypePoint2I, Offset(mChildMargin, GuiScrollCtrl));

}

void GuiScrollCtrl::resize(const Point2I &newPos, const Point2I &newExt)
{
   Parent::resize(newPos, newExt);
   computeSizes();
}

bool GuiScrollCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   GuiScrollContentCtrl *content = new GuiScrollContentCtrl;

   if (!content->registerObject())
      Con::errorf(ConsoleLogEntry::General, "Unhandled execption, could not add content in GuiScrollCtrl::onAdd()");
   addObject(content);

   return true;
}

bool GuiScrollCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   AssertFatal(size(), "Scroll control created with no content control.");
      
   mTextureHandle = mProfile->mTextureHandle;

   mBitmapBounds = new RectI[BmpStates * BmpCount];
   AssertFatal( mBitmapBounds, "Failed to allocate memory for bitmap array!" );

   bool result = createBitmapArray(mTextureHandle.getBitmap(), mBitmapBounds, BmpStates, BmpCount);
   AssertFatal(result, "Failed to create the bitmap array");
   
   for(S32 i = 0; i < BmpStates; i++)
   {
      mBitmapBounds[BmpStates * BmpVThumb + i].inset(0, 1);
      mBitmapBounds[BmpStates * BmpVPage + i].inset(0, 1);
      mBitmapBounds[BmpStates * BmpHThumb + i].inset(1, 0);
      mBitmapBounds[BmpStates * BmpHPage + i].inset(1, 0);
   }
   
   //init
   mBaseThumbSize = mBitmapBounds[BmpStates * BmpVThumbTopCap].extent.y +
                  mBitmapBounds[BmpStates * BmpVThumbBottomCap].extent.y;
   mScrollBarThickness = mBitmapBounds[BmpStates * BmpVPage].extent.x;
   mScrollBarArrowBtnLength = mBitmapBounds[BmpStates * BmpUp].extent.y;
   computeSizes();
   return true;
}

void GuiScrollCtrl::onSleep()
{
	Parent::onSleep();

	if ( mBitmapBounds )
	{
		delete [] mBitmapBounds;
		mBitmapBounds = NULL;
	}

   mTextureHandle = NULL;
}

bool GuiScrollCtrl::calcChildExtents(Point2I *pos, Point2I *ext)
{
   // loop through the children of this scroll view...
   if (! mContentCtrl->size())
      return false;
#if 1      
   GuiControl *ctrl = (GuiControl *) (mContentCtrl->front());
   *ext = ctrl->mBounds.extent;
   *pos = ctrl->mBounds.point;

   return true;

#else
   bool anyVisible = false;
   SimGroup::iterator i;
   for(i = mContentCtrl->begin(); i != mContentCtrl->end();i++)
   {
      GuiControl *ctrl = (GuiControl *) (*i);
      if (ctrl->isVisible())
      {
         anyVisible = true;
         pos->x = getMin(pos->x, ctrl->mBounds.point.x);
         pos->y = getMin(pos->y, ctrl->mBounds.point.y);
         if (ctrl->mBounds.point.x + ctrl->mBounds.extent.x > pos->x + ext->x)
            ext->x = ctrl->mBounds.point.x + ctrl->mBounds.extent.x - pos->x;
         if (ctrl->mBounds.point.y + ctrl->mBounds.extent.y > pos->y + ext->y)
            ext->y = ctrl->mBounds.point.y + ctrl->mBounds.extent.y - pos->y;
      }
   }
   return anyVisible;
#endif
}

void GuiScrollCtrl::addObject(SimObject *object)
{
   GuiScrollContentCtrl *content = dynamic_cast<GuiScrollContentCtrl *>(object);
   if(content)
   {
      if(mContentCtrl)
         mContentCtrl->deleteObject();
      Parent::addObject(object);
      mContentCtrl = content;
      computeSizes();
      return;
   }
   AssertFatal(mContentCtrl, "ERROR - no content control for a scroll control");

   mContentCtrl->addObject(object);
   computeSizes();
}

void GuiScrollCtrl::computeSizes()
{
   Point2I pos(mBorderThickness, mBorderThickness);
   Point2I ext(mBounds.extent.x - (mBorderThickness << 1), mBounds.extent.y - (mBorderThickness << 1));
   Point2I cpos;
   Point2I cext;
   mHBarEnabled = false;
   mVBarEnabled = false;
   mHasVScrollBar = (mForceVScrollBar == ScrollBarAlwaysOn);
   mHasHScrollBar = (mForceHScrollBar == ScrollBarAlwaysOn);

   setUpdate();

   if (calcChildExtents(&cpos, &cext))
   {
      if (mHasVScrollBar)
         ext.x -= mScrollBarThickness;
      if (mHasHScrollBar)
      {
         ext.y -= mScrollBarThickness;
      }
      if (cext.x > ext.x && (mForceHScrollBar == ScrollBarDynamic))
      {
         mHasHScrollBar = true;
         ext.y -= mScrollBarThickness;
      }
      if (cext.y > ext.y && (mForceVScrollBar == ScrollBarDynamic))
      {
         mHasVScrollBar = true;
         ext.x -= mScrollBarThickness;
         // doh! ext.x changed, so check hscrollbar again
         if (cext.x > ext.x && !mHasHScrollBar && (mForceHScrollBar == ScrollBarDynamic))
         {
            mHasHScrollBar = true;
            ext.y -= mScrollBarThickness;
         }
      }

      mContentCtrl->mBounds.point  = pos + mChildMargin;
      mContentCtrl->mBounds.extent = (ext - mChildMargin) - mChildMargin;
      ext = mContentCtrl->mBounds.extent;
      // see if the child controls need to be repositioned (null space in control)
      Point2I delta(0,0);

      if (cpos.x > 0)
         delta.x = -cpos.x;
      else if (ext.x > cpos.x + cext.x)
      {
         S32 diff = ext.x - (cpos.x + cext.x);
         delta.x = getMin(-cpos.x, diff);
      }
      
      //reposition the children if the child extent > the scroll content extent
      if (cpos.y > 0)
         delta.y = -cpos.y;
      else if (ext.y > cpos.y + cext.y)
      {
         S32 diff = ext.y - (cpos.y + cext.y);
         delta.y = getMin(-cpos.y, diff);
      }
      
      // apply the deltas to the children...
      if (delta.x || delta.y)
      {
         SimGroup::iterator i;
         for(i = mContentCtrl->begin(); i != mContentCtrl->end();i++)
         {
            GuiControl *ctrl = (GuiControl *) (*i);
            ctrl->mBounds.point += delta;
         }
         cpos += delta;
      }
      if (cext.x < ext.x)
         cext.x = ext.x;
      if (cext.y < ext.y)
         cext.y = ext.y;

      // enable needed scroll bars
      if (cext.x > ext.x)
         mHBarEnabled = true;
      if (cext.y > ext.y)
         mVBarEnabled = true;

   }
   // build all the rectangles and such...
   calcScrollRects();
   mLine_V = 0.1f;
   mLine_H = 0.1f;
   mPage_V = 0.2f;
   mPage_H = 0.2f;

/*
   SimGroup::iterator i;
   i = mContentCtrl->begin();
   if (i != mContentCtrl->end())
   {
      GuiControl *ctrl = (GuiControl *) (*i);
      RectI clientR(0,0,mContentCtrl->mBounds.extent.x, mContentCtrl->mBounds.extent.y);
      if (ctrl->mBounds.extent.y)
      {
         mLine_V = ctrl->scroll_mLine_V(clientR) / (F32)ctrl->mBounds.extent.y;

         mPage_V = ctrl->scroll_mPage_V(clientR) / (F32)ctrl->mBounds.extent.y;
      }
      if (ctrl->mBounds.extent.x)
      {
         mLine_H = ctrl->scroll_mLine_H(clientR) / (F32)ctrl->mBounds.extent.x; 
         mPage_H = ctrl->scroll_mPage_H(clientR) / (F32)ctrl->mBounds.extent.x; 
      }
   }
*/   

   calcThumbs(cpos, cext);
}

void GuiScrollCtrl::calcScrollRects(void)
{
   if (mHasHScrollBar)
   {
      mLeftArrowRect.set(mBorderThickness,
                        mBounds.extent.y - mBorderThickness - mScrollBarThickness - 1,
                        mScrollBarArrowBtnLength,
                        mScrollBarThickness);

      mRightArrowRect.set(mBounds.extent.x - mBorderThickness - (mHasVScrollBar ? mScrollBarThickness : 0) - mScrollBarArrowBtnLength,
                        mBounds.extent.y - mBorderThickness - mScrollBarThickness - 1,
                        mScrollBarArrowBtnLength,
                        mScrollBarThickness);
      mHTrackRect.set(mLeftArrowRect.point.x + mLeftArrowRect.extent.x,
                        mLeftArrowRect.point.y,
                        mRightArrowRect.point.x - (mLeftArrowRect.point.x + mLeftArrowRect.extent.x),
                        mScrollBarThickness);
   }
   if (mHasVScrollBar)
   {
      mUpArrowRect.set(mBounds.extent.x - mBorderThickness - mScrollBarThickness,
                        mBorderThickness,
                        mScrollBarThickness,
                        mScrollBarArrowBtnLength);
      mDownArrowRect.set(mBounds.extent.x - mBorderThickness - mScrollBarThickness,
                        mBounds.extent.y - mBorderThickness - mScrollBarArrowBtnLength - (mHasHScrollBar ? ( mScrollBarThickness + 1 ) : 0),
                        mScrollBarThickness,
                        mScrollBarArrowBtnLength);
      mVTrackRect.set(mUpArrowRect.point.x,
                        mUpArrowRect.point.y + mUpArrowRect.extent.y,
                        mScrollBarThickness,
                        mDownArrowRect.point.y - (mUpArrowRect.point.y + mUpArrowRect.extent.y) );
   }
}

void GuiScrollCtrl::calcThumbs(Point2I cpos, Point2I cext)
{
   Point2I ext = mContentCtrl->mBounds.extent;
   if (mHBarEnabled)
   {
      mHBarThumbPos = -cpos.x / F32(cext.x - ext.x);
      mHBarThumbWidth = ext.x / F32(cext.x);

      if (mUseConstantHeightThumb)
         mHThumbSize = mBaseThumbSize;
      else
         mHThumbSize = getMax(mBaseThumbSize, S32(mHBarThumbWidth * mHTrackRect.len_x()));
      mHThumbPos = (S32)(mHTrackRect.point.x + mHBarThumbPos * (mHTrackRect.len_x() - mHThumbSize));
   }
   if (mVBarEnabled)
   {
      mVBarThumbPos = -cpos.y / F32(cext.y - ext.y);
      mVBarThumbWidth = ext.y / F32(cext.y);

      if (mUseConstantHeightThumb)
         mVThumbSize = mBaseThumbSize;
      else
         mVThumbSize = getMax(mBaseThumbSize, S32(mVBarThumbWidth * mVTrackRect.len_y()));
      mVThumbPos = (S32)(mVTrackRect.point.y + mVBarThumbPos * (mVTrackRect.len_y() - mVThumbSize));
   }
}

void GuiScrollCtrl::scrollTo(F32 x, F32 y)
{
   setUpdate();
   if (x < 0)
      x = 0;
   else if (x > 1)
      x = 1;

   if (y < 0)
      y = 0;
   else if (y > 1)
      y = 1;

   if (x == mHBarThumbPos && y == mVBarThumbPos)
      return;

   Point2I cpos, cext;

   if (calcChildExtents(&cpos, &cext))
   {
      Point2I ext = mContentCtrl->mBounds.extent;
      Point2I npos;
      if (ext.x < cext.x)
         npos.x = (S32)(-x * (cext.x - ext.x));
      else
         npos.x = 0;
      if (ext.y < cext.y)
         npos.y = (S32)(-y * (cext.y - ext.y));
      else
         npos.y = 0;

      Point2I delta(npos.x - cpos.x, npos.y - cpos.y);
      // get rid of bad (roundoff) deltas
      if (x == mHBarThumbPos)
         delta.x = 0;
      if (y == mVBarThumbPos)
         delta.y = 0;
      SimGroup::iterator i;
      
      for(i = mContentCtrl->begin(); i != mContentCtrl->end();i++)
      {
         GuiControl *ctrl = (GuiControl *) (*i);
         ctrl->mBounds.point += delta;
      }
      cpos += delta;
      calcThumbs(cpos, cext);
   }
}

GuiScrollCtrl::Region GuiScrollCtrl::findHitRegion(const Point2I &pt)
{
   if (mVBarEnabled && mHasVScrollBar)
   {
      if (mUpArrowRect.pointInRect(pt))
         return UpArrow;
      else if (mDownArrowRect.pointInRect(pt))
         return DownArrow;
      else if (mVTrackRect.pointInRect(pt))
      {
         if (pt.y < mVThumbPos)
            return UpPage;
         else if (pt.y < mVThumbPos + mVThumbSize)
            return VertThumb;
         else
            return DownPage;
      }
   }
   if (mHBarEnabled && mHasHScrollBar)
   {
      if (mLeftArrowRect.pointInRect(pt))
         return LeftArrow;
      else if (mRightArrowRect.pointInRect(pt))
         return RightArrow;
      else if (mHTrackRect.pointInRect(pt))
      {
         if (pt.x < mHThumbPos)
            return LeftPage;
         else if (pt.x < mHThumbPos + mHThumbSize)
            return HorizThumb;
         else
            return RightPage;
      }
   }
   return None;
}

bool GuiScrollCtrl::wantsTabListMembership()
{
   return true;
}   

bool GuiScrollCtrl::loseFirstResponder()
{
   setUpdate();
   return true;
}   

bool GuiScrollCtrl::becomeFirstResponder()
{
   setUpdate();
   return mWillFirstRespond;
}

bool GuiScrollCtrl::onKeyDown(const GuiEvent &event)
{
   if (mWillFirstRespond)
   {
      switch (event.keyCode)
      {
         case KEY_RIGHT:
            scrollByRegion(RightArrow);
            return true;
            
         case KEY_LEFT:
            scrollByRegion(LeftArrow);
            return true;
            
         case KEY_DOWN:
            scrollByRegion(DownArrow);
            return true;
            
         case KEY_UP:
            scrollByRegion(UpArrow);
            return true;
            
         case KEY_PAGE_UP:
            scrollByRegion(UpPage);
            return true;
            
         case KEY_PAGE_DOWN:
            scrollByRegion(DownPage);
            return true;
      }
   }
   return Parent::onKeyDown(event);
}   

void GuiScrollCtrl::onMouseDown(const GuiEvent &event)
{
   mouseLock();
   setFirstResponder();

   setUpdate();
   
   Point2I curMousePos = globalToLocalCoord(event.mousePoint);
   curHitRegion = findHitRegion(curMousePos);
   stateDepressed = true;

   scrollByRegion(curHitRegion);

   if (curHitRegion == VertThumb)
   {
      mThumbAnchorPos = mVBarThumbPos;
      mThumbDelta = curMousePos.y - mVThumbPos;
   }
   else if (curHitRegion == HorizThumb)
   {
      mThumbAnchorPos = mHBarThumbPos;
      mThumbDelta = curMousePos.x - mHThumbPos;
   }
}

void GuiScrollCtrl::onMouseUp(const GuiEvent &)
{
   mouseUnlock();

   setUpdate();

   curHitRegion = None;
   stateDepressed = false;
}

void GuiScrollCtrl::onMouseDragged(const GuiEvent &event)
{
   Point2I curMousePos = globalToLocalCoord(event.mousePoint);
   setUpdate();

   if ( (curHitRegion != VertThumb) && (curHitRegion != HorizThumb) )
   {
      Region hit = findHitRegion(curMousePos);
      if (hit != curHitRegion)
         stateDepressed = false;
      else
         stateDepressed = true;   
      return;
   }

   // ok... if the mouse is 'near' the scroll bar, scroll with it
   // otherwise, snap back to the previous position.

   if (curHitRegion == VertThumb)
   {
      if (curMousePos.x >= mVTrackRect.point.x - mScrollBarThickness &&
         curMousePos.x <= mVTrackRect.point.x + mVTrackRect.extent.x - 1 + mScrollBarThickness &&
         curMousePos.y >= mVTrackRect.point.y - mScrollBarThickness &&
         curMousePos.y <= mVTrackRect.point.y + mVTrackRect.extent.y - 1 + mScrollBarThickness)
      {
         scrollTo(mHBarThumbPos, 
            (curMousePos.y - mThumbDelta - mVTrackRect.point.y) /
            F32(mVTrackRect.len_y() - mVThumbSize));
      }
      else
         scrollTo(mHBarThumbPos, mThumbAnchorPos);
   }
   else if (curHitRegion == HorizThumb)
   {
      if (curMousePos.x >= mHTrackRect.point.x - mScrollBarThickness &&
         curMousePos.x <= mHTrackRect.point.x + mHTrackRect.extent.x - 1 + mScrollBarThickness &&
         curMousePos.y >= mHTrackRect.point.y - mScrollBarThickness &&
         curMousePos.y <= mHTrackRect.point.y + mHTrackRect.extent.y - 1 + mScrollBarThickness)
      {
         scrollTo((curMousePos.x - mThumbDelta - mHTrackRect.point.x) /
            F32(mHTrackRect.len_x() - mHThumbSize),
            mVBarThumbPos);
      }
      else
         scrollTo(mThumbAnchorPos, mVBarThumbPos);
   }
}

bool GuiScrollCtrl::onMouseWheelUp(const GuiEvent &event)
{
   if ( !mAwake || !mVisible )
      return( false );

   scrollByRegion((event.modifier & SI_CTRL) ? UpPage : UpArrow);

   // Tell the kids that the mouse moved (relatively):
   iterator itr;
   for ( itr = mContentCtrl->begin(); itr != mContentCtrl->end(); itr++ )
   {
      GuiControl* grandKid = static_cast<GuiControl*>( *itr );
      grandKid->onMouseMove( event );
   }

   return( true );
}

bool GuiScrollCtrl::onMouseWheelDown(const GuiEvent &event)
{
   if ( !mAwake || !mVisible )
      return( false );

   scrollByRegion((event.modifier & SI_CTRL) ? DownPage : DownArrow);

   // Tell the kids that the mouse moved (relatively):
   iterator itr;
   for ( itr = mContentCtrl->begin(); itr != mContentCtrl->end(); itr++ )
   {
      GuiControl* grandKid = static_cast<GuiControl *>( *itr );
      grandKid->onMouseMove( event );
   }

   return( true );
}
  

void GuiScrollCtrl::scrollByRegion(Region reg)
{
   setUpdate();
   if (mVBarEnabled)
   {
      bool alreadyScrolled = false;
      GuiControl* grandKid = NULL;
      GuiArrayCtrl *ac = NULL;
      if (mContentCtrl->begin())
      {
         grandKid = (GuiControl *)(mContentCtrl->front());
         ac = dynamic_cast<GuiArrayCtrl*>(grandKid);
      }

      if ( ac || ( grandKid && mDefaultLineHeight > 0 ) )
      {
         S32 lineHeight, numCells;
         if ( ac )
            ac->getScrollDimensions(lineHeight, numCells);
         else
         {
            lineHeight = mDefaultLineHeight;
            numCells = 47; // OK, we're just fakin'...
         }

         if (lineHeight > 0 && numCells > 0)
         {
            S32 pageHeight = S32(mContentCtrl->mBounds.extent.y / lineHeight) * (lineHeight - 1) - 1;
            switch(reg)
            {
               case UpPage:
                  grandKid->mBounds.point.y = getMin(0, getMax(mContentCtrl->mBounds.extent.y - grandKid->mBounds.extent.y, grandKid->mBounds.point.y + pageHeight));
                  break;
               case DownPage:
                  grandKid->mBounds.point.y = getMin(0, getMax(mContentCtrl->mBounds.extent.y - grandKid->mBounds.extent.y, grandKid->mBounds.point.y - pageHeight));
                  break;
               case UpArrow:
                  grandKid->mBounds.point.y = getMin(0, getMax(mContentCtrl->mBounds.extent.y - grandKid->mBounds.extent.y, grandKid->mBounds.point.y + lineHeight));
                  break;
               case DownArrow:
                  grandKid->mBounds.point.y = getMin(0, getMax(mContentCtrl->mBounds.extent.y - grandKid->mBounds.extent.y, grandKid->mBounds.point.y - lineHeight));
                  break;
            }
            calcThumbs(grandKid->mBounds.point, grandKid->mBounds.extent);
            alreadyScrolled = true;
         }
      }
   
      if (! alreadyScrolled)
      {
         // If all else fails, scroll by percentage:
         mPage_V = 0.1f;
         mLine_V = 0.1f;

         switch(reg)
         {
            case UpPage:
               scrollTo(mHBarThumbPos, mVBarThumbPos - mPage_V);
               break;
            case DownPage:
               scrollTo(mHBarThumbPos, mVBarThumbPos + mPage_V);
               break;
            case UpArrow:
               scrollTo(mHBarThumbPos, mVBarThumbPos - mLine_V);
               break;
            case DownArrow:
               scrollTo(mHBarThumbPos, mVBarThumbPos + mLine_V);
               break;
         }
      }
   }

   if (mHBarEnabled)
   {
      switch(reg)
      {
         case LeftPage:
            scrollTo(mHBarThumbPos - mPage_H, mVBarThumbPos);
            break;
         case RightPage:
            scrollTo(mHBarThumbPos + mPage_H, mVBarThumbPos);
            break;
         case LeftArrow:
            scrollTo(mHBarThumbPos - mLine_H, mVBarThumbPos);
            break;
         case RightArrow:
            scrollTo(mHBarThumbPos + mLine_H, mVBarThumbPos);
            break;
      }
   }
}   


void GuiScrollCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   // draw border
   drawBorder( offset, ( firstResponder == this ) );

   // draw scroll bars
   if (mHasVScrollBar)
      drawVScrollBar(offset);
      
   if (mHasHScrollBar)
      drawHScrollBar(offset);
      
   //draw the scroll corner
   if (mHasVScrollBar && mHasHScrollBar)
      drawScrollCorner(offset);

   // draw content and its children
   renderChildControls(offset, updateRect, firstResponder);
}

void GuiScrollCtrl::drawBorder( const Point2I &offset, bool /*isFirstResponder*/ )
{
   RectI r(offset.x, offset.y, mBounds.extent.x, mBounds.extent.y);
   
   if (mProfile->mOpaque)
      dglDrawRectFill(r, mProfile->mFillColor);

   if (mProfile->mBorder)
      dglDrawRect(r, mProfile->mBorderColor);
}   

void GuiScrollCtrl::drawVScrollBar(const Point2I &offset)
{
   Point2I pos = offset + mUpArrowRect.point;
   S32 bitmap = (mVBarEnabled ? ((curHitRegion == UpArrow && stateDepressed) ?
         BmpStates * BmpUp + BmpHilite : BmpStates * BmpUp) : BmpStates * BmpUp + BmpDisabled);

   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[bitmap]);

   pos.y += mScrollBarArrowBtnLength;
   S32 end;
   if (mVBarEnabled)
      end = mVThumbPos + offset.y;
   else
      end = mDownArrowRect.point.y + offset.y;

   bitmap = (mVBarEnabled ? ((curHitRegion == DownPage && stateDepressed) ?
         BmpStates * BmpVPage + BmpHilite : BmpStates * BmpVPage) : BmpStates * BmpVPage + BmpDisabled);
      
   if (end > pos.y)
   {
      dglClearBitmapModulation();
      dglDrawBitmapStretchSR(mTextureHandle, RectI(pos, Point2I(mBitmapBounds[bitmap].extent.x, end - pos.y)), mBitmapBounds[bitmap]);
   }
   
   pos.y = end;
   if (mVBarEnabled)
   {
      bool thumbSelected = (curHitRegion == VertThumb && stateDepressed);
      S32 ttop = (thumbSelected ? BmpStates * BmpVThumbTopCap + BmpHilite : BmpStates * BmpVThumbTopCap);
      S32 tmid = (thumbSelected ? BmpStates * BmpVThumb + BmpHilite : BmpStates * BmpVThumb);
      S32 tbot = (thumbSelected ? BmpStates * BmpVThumbBottomCap + BmpHilite : BmpStates * BmpVThumbBottomCap);

      // draw the thumb
      dglClearBitmapModulation();
      dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[ttop]);
      pos.y += mBitmapBounds[ttop].extent.y;
      end = mVThumbPos + mVThumbSize - mBitmapBounds[tbot].extent.y + offset.y;

      if (end > pos.y)
      {
         dglClearBitmapModulation();
         dglDrawBitmapStretchSR(mTextureHandle, RectI(pos, Point2I(mBitmapBounds[tmid].extent.x, end - pos.y)), mBitmapBounds[tmid]);
      }
         
      pos.y = end;
      dglClearBitmapModulation();
      dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[tbot]);
      pos.y += mBitmapBounds[tbot].extent.y;
      end = mVTrackRect.point.y + mVTrackRect.extent.y - 1 + offset.y;

      bitmap = (curHitRegion == DownPage && stateDepressed) ? BmpStates * BmpVPage + BmpHilite : BmpStates * BmpVPage;
      if (end > pos.y)
      {
         dglClearBitmapModulation();
         dglDrawBitmapStretchSR(mTextureHandle, RectI(pos, Point2I(mBitmapBounds[bitmap].extent.x, end - pos.y)), mBitmapBounds[bitmap]);
      }
         
      pos.y = end;
   }
   
   bitmap = (mVBarEnabled ? ((curHitRegion == DownArrow && stateDepressed ) ? 
         BmpStates * BmpDown + BmpHilite : BmpStates * BmpDown) : BmpStates * BmpDown + BmpDisabled);

   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[bitmap]);
} 

void GuiScrollCtrl::drawHScrollBar(const Point2I &offset)
{
   S32 bitmap;
   
   //draw the left arrow
   bitmap = (mHBarEnabled ? ((curHitRegion == LeftArrow && stateDepressed) ?
            BmpStates * BmpLeft + BmpHilite : BmpStates * BmpLeft) : BmpStates * BmpLeft + BmpDisabled);
   
   Point2I pos = offset;
   pos += mLeftArrowRect.point;

   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[bitmap]);

   pos.x += mLeftArrowRect.extent.x;
   
   //draw the left page
   S32 end;
   if (mHBarEnabled)
      end = mHThumbPos + offset.x;
   else
      end = mRightArrowRect.point.x + offset.x;

   bitmap = (mHBarEnabled ? ((curHitRegion == LeftPage && stateDepressed) ?
            BmpStates * BmpHPage + BmpHilite : BmpStates * BmpHPage) : BmpStates * BmpHPage + BmpDisabled);

   if (end > pos.x)
   {
      dglClearBitmapModulation();
      dglDrawBitmapStretchSR(mTextureHandle, RectI(pos, Point2I(end - pos.x, mBitmapBounds[bitmap].extent.y)), mBitmapBounds[bitmap]);
   }
   pos.x = end;
   
   //draw the thumb and the rightPage
   if (mHBarEnabled)
   {
      bool thumbSelected = (curHitRegion == HorizThumb && stateDepressed);
      S32 ttop = (thumbSelected ? BmpStates * BmpHThumbLeftCap + BmpHilite : BmpStates * BmpHThumbLeftCap );
      S32 tmid = (thumbSelected ? BmpStates * BmpHThumb + BmpHilite : BmpStates * BmpHThumb);
      S32 tbot = (thumbSelected ? BmpStates * BmpHThumbRightCap + BmpHilite : BmpStates * BmpHThumbRightCap);

      // draw the thumb
      dglClearBitmapModulation();
      dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[ttop]);
      pos.x += mBitmapBounds[ttop].extent.x;
      end = mHThumbPos + mHThumbSize - mBitmapBounds[tbot].extent.x + offset.x;
      if (end > pos.x)
      {
         dglClearBitmapModulation();
         dglDrawBitmapStretchSR(mTextureHandle, RectI(pos, Point2I(end - pos.x, mBitmapBounds[tmid].extent.y)), mBitmapBounds[tmid]);
      }
         
      pos.x = end;
      dglClearBitmapModulation();
      dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[tbot]);
      pos.x += mBitmapBounds[tbot].extent.x;
      end = mHTrackRect.point.x + mHTrackRect.extent.x - 1 + offset.x;

      bitmap = ((curHitRegion == RightPage && stateDepressed) ? BmpStates * BmpHPage + BmpHilite : BmpStates * BmpHPage);

      if (end > pos.x)
      {
         dglClearBitmapModulation();
         dglDrawBitmapStretchSR(mTextureHandle, RectI(pos, Point2I(end - pos.x, mBitmapBounds[bitmap].extent.y)), mBitmapBounds[bitmap]);
      }
         
      pos.x = end;
   }
   bitmap = (mHBarEnabled ? ((curHitRegion == RightArrow && stateDepressed) ?
            BmpStates * BmpRight + BmpHilite : BmpStates * BmpRight) : BmpStates * BmpRight + BmpDisabled);

   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[bitmap]);
}   

void GuiScrollCtrl::drawScrollCorner(const Point2I &offset)
{
   Point2I pos = offset;
   pos.x += mRightArrowRect.point.x + mRightArrowRect.extent.x;
   pos.y += mRightArrowRect.point.y;
   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[BmpStates * BmpResize]);
}   

void GuiScrollCtrl::autoScroll(Region reg)
{
	scrollByRegion(reg);
}	
