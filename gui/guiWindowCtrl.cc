//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiWindowCtrl.h"

GuiWindowCtrl::GuiWindowCtrl(void)
{
	mResizeWidth = true;
	mResizeHeight = true;
   mCanMove = true;
   mCanClose = true;
   mCanMinimize = true;
   mCanMaximize = true;
   mTitleHeight = 20;
   mResizeRightWidth = 10;
   mResizeBottomHeight = 10;
   
   mCloseCommand = StringTable->insert("");
   
   mMinimized = false;
   mMaximized = false;
   mMouseMovingWin = false;
   mMouseResizeWidth = false;
	mMouseResizeHeight = false;
   mBounds.extent.set(100, 200);
   mMinSize.set(50, 50);
   mMinimizeIndex = -1;
   mTabIndex = -1;
   
   RectI closeRect(80, 2, 16, 16);
   mCloseButton = closeRect;
   closeRect.point.x -= 18;
   mMaximizeButton = closeRect;
   closeRect.point.x -= 18;
   mMinimizeButton = closeRect;
      
   //other defaults
   mActive = true;
}

void GuiWindowCtrl::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("resizeWidth",       TypeBool,         Offset(mResizeWidth, GuiWindowCtrl));
   addField("resizeHeight",      TypeBool,         Offset(mResizeHeight, GuiWindowCtrl));
   addField("canMove",           TypeBool,         Offset(mCanMove, GuiWindowCtrl));
   addField("canClose",          TypeBool,         Offset(mCanClose, GuiWindowCtrl));
   addField("canMinimize",       TypeBool,         Offset(mCanMinimize, GuiWindowCtrl));
   addField("canMaximize",       TypeBool,         Offset(mCanMaximize, GuiWindowCtrl));
   addField("minSize",           TypePoint2I,      Offset(mMinSize, GuiWindowCtrl));
   addField("closeCommand",      TypeString,       Offset(mCloseCommand, GuiWindowCtrl));
}

void GuiWindowCtrl::consoleInit()
{
}

bool GuiWindowCtrl::isMinimized(S32 &index)
{
   index = mMinimizeIndex;
   return mMinimized && mVisible;
}
       
bool GuiWindowCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   
   //get the texture for the close, minimize, and maximize buttons
   mTextureHandle = mProfile->mTextureHandle;
   bool result = createBitmapArray(mTextureHandle.getBitmap(), mBitmapBounds, BmpStates, BmpCount);
   AssertFatal(result, "Failed to create the bitmap array");
   
   mTitleHeight = mBitmapBounds[BmpStates * BmpClose].extent.y + 6;
   S32 buttonWidth = mBitmapBounds[BmpStates * BmpClose].extent.x;
   mResizeRightWidth = mTitleHeight / 2;
   mResizeBottomHeight = mTitleHeight / 2;
   
   //set the button coords
   RectI closeRect(mBounds.extent.x - buttonWidth - 3, 2, mTitleHeight - 6, buttonWidth);
   mCloseButton = closeRect;
   
   closeRect.point.x -= buttonWidth + 2;
   mMaximizeButton = closeRect;
   
   closeRect.point.x -= buttonWidth + 2;
   mMinimizeButton = closeRect;
   
   //set the tab index
   mTabIndex = -1;
   GuiControl *parent = getParent();
   if (parent && mFirstResponder)
   {
      mTabIndex = 0;
      
      //count the number of windows preceeding this one
      iterator i;
      for (i = parent->begin(); i != parent->end(); i++)
      {
         GuiWindowCtrl *ctrl = dynamic_cast<GuiWindowCtrl *>(*i);
         if (ctrl)
         {
            if (ctrl == this) break;
            else if (ctrl->mFirstResponder) mTabIndex++;
         }
      }
   }
   
   return true;
}

GuiControl* GuiWindowCtrl::findHitControl(const Point2I &pt, S32 initialLayer)
{
   if (! mMinimized)
      return Parent::findHitControl(pt, initialLayer);
   else
      return this;
}

void GuiWindowCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   Parent::resize(newPosition, newExtent);
   
   //set the button coords
   S32 buttonWidth = mBitmapBounds[BmpStates * BmpClose].extent.x;
   RectI closeRect(mBounds.extent.x - buttonWidth - 3, 2, mTitleHeight - 6, buttonWidth);
   mCloseButton = closeRect;
   
   closeRect.point.x -= buttonWidth + 2;
   mMaximizeButton = closeRect;
   
   closeRect.point.x -= buttonWidth + 2;
   mMinimizeButton = closeRect;
}

void GuiWindowCtrl::onMouseDown(const GuiEvent &event)
{
   setUpdate();
   
   mOrigBounds = mBounds;
   mPressClose = false;
   mPressMaximize = false;
   mPressMinimize = false;
   
   mMouseDownPosition = event.mousePoint;
   Point2I localPoint = globalToLocalCoord(event.mousePoint);
   
   //select this window - move it to the front, and set the first responder
   selectWindow();
   
   //if we clicked within the title bar
   if (localPoint.y < mTitleHeight)
   {
      //if we clicked on the close button
      if (mCloseButton.pointInRect(localPoint))
      {
         mPressClose = mCanClose;
      }
      else if (mMaximizeButton.pointInRect(localPoint))
      {
         mPressMaximize = mCanMaximize;
      }
      else if (mMinimizeButton.pointInRect(localPoint))
      {
         mPressMinimize = mCanMinimize;
      }
      
      //else we clicked within the title
      else
      {
         mMouseMovingWin = mCanMove;
         mMouseResizeWidth = false;
   		mMouseResizeHeight = false;
      }
   }
   else
   {
      mMouseMovingWin = false;
      
      //see if we clicked on the right edge
      if (localPoint.x > mBounds.extent.x - mResizeRightWidth)
      {
         mMouseResizeWidth = true;
      }
      
      //see if we clicked on the bottom edge (as well)
      if (localPoint.y > mBounds.extent.y - mResizeBottomHeight)
      {
         mMouseResizeHeight = true;
      }
   }
   if (mMouseMovingWin || mMouseResizeWidth || mMouseResizeHeight ||
         mPressClose || mPressMaximize || mPressMinimize)
   {
      mouseLock(); 
   }
   else
   {
      GuiControl *ctrl = findHitControl(event.mousePoint);
      if (ctrl && ctrl != this)
      {
         ctrl->onMouseDown(event);
      }
   }
}

void GuiWindowCtrl::onMouseDragged(const GuiEvent &event)
{
   GuiControl *parent = getParent();
   GuiCanvas *root = getRoot();
   if (! root) return;
   
   Point2I deltaMousePosition = event.mousePoint - mMouseDownPosition;
   
   Point2I newPosition = mBounds.point;
   Point2I newExtent = mBounds.extent;
   bool update = false;
   if (mMouseMovingWin && parent)
   {
      newPosition.x = getMax(0, getMin(parent->mBounds.extent.x - mBounds.extent.x, mOrigBounds.point.x + deltaMousePosition.x));
      newPosition.y = getMax(0, getMin(parent->mBounds.extent.y - mBounds.extent.y, mOrigBounds.point.y + deltaMousePosition.y));
      update = true;
   }
   else
   {
      if (mMouseResizeWidth && parent)
      {
         newExtent.x = getMax(0, getMax(mMinSize.x, getMin(parent->mBounds.extent.x, mOrigBounds.extent.x + deltaMousePosition.x)));
         update = true;
      }
      if (mMouseResizeHeight && parent)
      {
         newExtent.y = getMax(0, getMax(mMinSize.y, getMin(parent->mBounds.extent.y, mOrigBounds.extent.y + deltaMousePosition.y)));
         update = true;
      }
   }
   if (update)
   {
      Point2I pos = parent->localToGlobalCoord(mBounds.point);
      root->addUpdateRegion(pos, mBounds.extent);
      resize(newPosition, newExtent);
   }
}

void GuiWindowCtrl::onMouseUp(const GuiEvent &event)
{
   event;
   mouseUnlock();
   
   mMouseMovingWin = false;
   mMouseResizeWidth = false;
   mMouseResizeHeight = false;
   
   GuiControl *parent = getParent();
   if (! parent)
      return;
      
   //see if we take an action
   Point2I localPoint = globalToLocalCoord(event.mousePoint);
   if (mPressClose && mCloseButton.pointInRect(localPoint))
   {
      Con::evaluate(mCloseCommand);
   }
   else if (mPressMaximize)
   {
      if (mMaximized)
      {
         //resize to the previous position and extent, bounded by the parent
         resize(Point2I(getMax(0, getMin(parent->mBounds.extent.x - mStandardBounds.extent.x, mStandardBounds.point.x)),
                        getMax(0, getMin(parent->mBounds.extent.y - mStandardBounds.extent.y, mStandardBounds.point.y))),
                        mStandardBounds.extent);
         //set the flag
         mMaximized = false;
      }
      else
      {
         //only save the position if we're not minimized
         if (! mMinimized)
         {
            mStandardBounds = mBounds;
         }
         else
         {
            mMinimized = false;
         }
         
         //resize to fit the parent
         resize(Point2I(0, 0), parent->mBounds.extent);
         
         //set the flag
         mMaximized = true;
      }
   }
   else if (mPressMinimize)
   {
      if (mMinimized)
      {
         //resize to the previous position and extent, bounded by the parent
         resize(Point2I(getMax(0, getMin(parent->mBounds.extent.x - mStandardBounds.extent.x, mStandardBounds.point.x)),
                        getMax(0, getMin(parent->mBounds.extent.y - mStandardBounds.extent.y, mStandardBounds.point.y))),
                        mStandardBounds.extent);
         //set the flag
         mMinimized = false;
      }
      else
      {
         if (parent->mBounds.extent.x < 100 || parent->mBounds.extent.y < mTitleHeight + 3)
            return;
         
         //only save the position if we're not maximized
         if (! mMaximized)
         {
            mStandardBounds = mBounds;
         }
         else
         {
            mMaximized = false;
         }
         
         //first find the lowest unused minimized index up to 32 minimized windows
         U32 indexMask = 0;
         iterator i;
         S32 count = 0;
         for (i = parent->begin(); i != parent->end() && count < 32; i++)
         {
            count++;
            S32 index;
            GuiWindowCtrl *ctrl = dynamic_cast<GuiWindowCtrl *>(*i);
            if (ctrl && ctrl->isMinimized(index))
            {
               indexMask |= (1 << index);
            }
         }
         
         //now find the first unused bit
         for (count = 0; count < 32; count++)
         {
            if (! (indexMask & (1 << count))) break;
         }
         
         //if we have more than 32 minimized windows, use the first position
         count = getMax(0, count);
         
         //this algorithm assumes all window have the same title height, and will minimize to 98 pix
         Point2I newExtent(98, mTitleHeight);
         
         //first, how many can fit across
         S32 numAcross = getMax(1, (parent->mBounds.extent.x / newExtent.x + 2));
         
         //find the new "mini position"
         Point2I newPosition;
         newPosition.x = (count % numAcross) * (newExtent.x + 2) + 2;
         newPosition.y = parent->mBounds.extent.y - (((count / numAcross) + 1) * (newExtent.y + 2)) - 2;
         
         //find the minimized position and extent
         resize(newPosition, newExtent);
         
         //set the index so other windows will not try to minimize to the same location
         mMinimizeIndex = count;
         
         //set the flag
         mMinimized = true;
      }
   }
   
}

GuiControl *GuiWindowCtrl::findNextTabable(GuiControl *curResponder, bool firstCall)
{
   //set the global if this is the first call (directly from the canvas)
   if (firstCall)
   {
      GuiControl::gCurResponder = NULL;
   }
   
   //if the window does not already contain the first responder, return false
   //ie.  Can't tab into or out of a window
   if (! ControlIsChild(curResponder))
   {
      return NULL;
   }
   
   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findNextTabable(curResponder, false);
      if (tabCtrl) break;
   }
   
   //to ensure the tab cycles within the current window...
   if (! tabCtrl)
   {
      tabCtrl = findFirstTabable();
   }
   
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

GuiControl *GuiWindowCtrl::findPrevTabable(GuiControl *curResponder, bool firstCall)
{
   if (firstCall)
   {
      GuiControl::gPrevResponder = NULL;
   }
     
   //if the window does not already contain the first responder, return false
   //ie.  Can't tab into or out of a window
   if (! ControlIsChild(curResponder))
   {
      return NULL;
   }
   
   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findPrevTabable(curResponder, false);
      if (tabCtrl) break;
   }
   
   //to ensure the tab cycles within the current window...
   if (! tabCtrl)
   {
      tabCtrl = findLastTabable();
   }
   
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

bool GuiWindowCtrl::onKeyDown(const GuiEvent &event)
{
   //if this control is a dead end, kill the event
   if ((! mVisible) || (! mActive) || (! mAwake)) return true;

   if ((event.keyCode == KEY_TAB) && (event.modifier & SI_CTRL))
   {
      //find the next sibling window, and select it
      GuiControl *parent = getParent();
      if (parent)
      {
         GuiWindowCtrl *firstWindow = NULL;
         iterator i;
         for (i = parent->begin(); i != parent->end(); i++)
         {
            GuiWindowCtrl *ctrl = dynamic_cast<GuiWindowCtrl *>(*i);
            if (ctrl && ctrl->getTabIndex() == mTabIndex + 1)
            {
               ctrl->selectWindow();
               return true;
            }
            else if (ctrl && ctrl->getTabIndex() == 0)
            {
               firstWindow = ctrl;
            }
         }
         //recycle from the beginning
         if (firstWindow != this)
         {
            firstWindow->selectWindow();
            return true;
         }
      }
   }
   
   return Parent::onKeyDown(event);
}

void GuiWindowCtrl::selectWindow(void)
{
   //first make sure this window is the front most of its siblings
   GuiControl *parent = getParent();
   if (parent)
   {
      parent->bringObjectToFront(this);
   }
   
   //also set the first responder to be the one within this window
   setFirstResponder(mFirstResponder);
}

void GuiWindowCtrl::drawWinRect(const RectI &myRect)
{
   Point2I bl = myRect.point;
   Point2I tr;
   tr.x = myRect.point.x + myRect.extent.x - 1;
   tr.y = myRect.point.y + myRect.extent.y - 1;
   dglDrawRectFill(myRect, mProfile->mFillColor);
   dglDrawLine(Point2I(bl.x + 1, tr.y), Point2I(bl.x + 1, bl.y), ColorI(255, 255, 255));
   dglDrawLine(Point2I(bl.x, tr.y + 1), Point2I(tr.x, tr.y + 1), ColorI(255, 255, 255));
   dglDrawRect(myRect, ColorI(0, 0, 0));
}
      
void GuiWindowCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   //draw the outline
   RectI winRect;
   winRect.point = offset;
   winRect.extent = mBounds.extent; 
   
   if (mProfile->mOpaque)
   {
      drawWinRect(winRect);
      
      //title bar
      ColorI titleColor = (! firstResponder || ControlIsChild(firstResponder)) ?
                                 mProfile->mFillColorHL : mProfile->mFillColorNA;
      RectI titleBarRect = winRect;
      titleBarRect.extent.y = mTitleHeight;
      
      dglDrawRectFill(titleBarRect, titleColor);
   }
   
   //outline the window
   if (mProfile->mBorder)
	   dglDrawRect(winRect, mProfile->mBorderColor);

   //draw the title
   dglSetBitmapModulation(mProfile->mFontColor);
   S32 fontHeight = mFont->getHeight();
   dglDrawText(mFont, Point2I(offset.x + 4, offset.y + ((mTitleHeight - fontHeight) / 2)), mText);
      
   GuiCanvas *root = getRoot();
   AssertFatal(root, "Unable to get the root Canvas.");
   Point2I localPoint = globalToLocalCoord(root->getCursorPos());
   
   //draw the close button
   Point2I tempUL;
   Point2I tempLR;
   S32 bmp = BmpStates * BmpClose;
   if (! mCanClose)
      bmp += BmpDisabled;
   else if (mCloseButton.pointInRect(localPoint) && root->mouseButtonDown())
      bmp += BmpHilite;
   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, offset + mCloseButton.point, mBitmapBounds[bmp]);
                           
   //draw the maximize button
   if (mMaximized)
      bmp = BmpStates * BmpNormal;
   else
      bmp = BmpStates * BmpMaximize;
   if (! mCanMaximize)
      bmp += BmpDisabled;
   else if (mMaximizeButton.pointInRect(localPoint) && root->mouseButtonDown())
      bmp += BmpHilite;
   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, offset + mMaximizeButton.point, mBitmapBounds[bmp]);
   
   //draw the minimize button
   if (mMinimized)
      bmp = BmpStates * BmpNormal;
   else
      bmp = BmpStates * BmpMinimize;
   if (! mCanMinimize)
      bmp += BmpDisabled;
   else if (mMinimizeButton.pointInRect(localPoint) && root->mouseButtonDown())
      bmp += BmpHilite;
   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, offset + mMinimizeButton.point, mBitmapBounds[bmp]);
   
   if (! mMinimized)
   {
      //render the children
      renderChildControls(offset, updateRect, firstResponder);
   }
}
