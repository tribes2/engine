//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "Platform/event.h"
#include "GUI/guiScrollCtrl.h"
#include "GUI/guiArrayCtrl.h"


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

GuiArrayCtrl::GuiArrayCtrl()
{
   mActive = true;
   
   mCellSize.set(80, 30);
   mSize = Point2I(5, 30);
   mSelectedCell.set(-1, -1);
   mMouseOverCell.set(-1, -1);
   mHeaderDim.set(0, 0);
}

bool GuiArrayCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
      
   //get the font
   mFont = mProfile->mFont;
      
   return true;
}

void GuiArrayCtrl::onSleep()
{
   Parent::onSleep();
   mFont = NULL;
}
      
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiArrayCtrl::setSize(Point2I newSize)
{
   mSize = newSize;
   Point2I newExtent(newSize.x * mCellSize.x + mHeaderDim.x, newSize.y * mCellSize.y + mHeaderDim.y);

   resize(mBounds.point, newExtent);
}

void GuiArrayCtrl::getScrollDimensions(S32 &cell_size, S32 &num_cells)
{
   cell_size = mCellSize.y;
   num_cells = mSize.y;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiArrayCtrl::cellSelected(Point2I cell)
{
   if (cell.x < 0 || cell.x >= mSize.x || cell.y < 0 || cell.y >= mSize.y)
   {
      mSelectedCell = Point2I(-1,-1);
      return false;
   }
   
   mSelectedCell = cell;
   scrollSelectionVisible();
   onCellSelected(cell);
   return true;
}

void GuiArrayCtrl::onCellSelected(Point2I cell)
{
  	Con::executef(this, 3, "onSelect", Con::getFloatArg(cell.x), Con::getFloatArg(cell.y));
   
   //call the console function
   if (mConsoleCommand[0])
      Con::evaluate(mConsoleCommand, false);
}

void GuiArrayCtrl::setSelectedCell(Point2I cell)
{
   cellSelected(cell);
}

Point2I GuiArrayCtrl::getSelectedCell()
{
   return mSelectedCell;
}

void GuiArrayCtrl::scrollSelectionVisible()
{
   scrollCellVisible(mSelectedCell);
}

void GuiArrayCtrl::scrollCellVisible(Point2I cell)
{
   //make sure we have a parent
   GuiControl *parent = getParent();
   if (! parent) return;
   
   //make sure we have a valid cell selected
   if ((cell.x < 0) || (cell.y < 0)) return;

   //get the parent extent, and the offset of the selected cell
   Point2I parentExtent = parent->getExtent();
   Point2I cellPos(mBounds.point.x + cell.x * mCellSize.x,
                   mBounds.point.y + cell.y * mCellSize.y);

   Point2I delta(0,0);

   //find out how far the selected cell is outside the parent extent horizontally
   if (cellPos.x <= 0)
   {
      delta.x = -cellPos.x;
   }
   else if (cellPos.x + mCellSize.x > parentExtent.x)
   {
      delta.x = parentExtent.x - (cellPos.x + mCellSize.x);
   }

   //find out how far the selected cell is outside the parent extent vertically
   if (cellPos.y <= 0)
   {
      delta.y = -cellPos.y;
   }
   else if (cellPos.y + mCellSize.y > parentExtent.y)
   {
      delta.y = parentExtent.y - (cellPos.y + mCellSize.y);
   }
   
   //if we need to scroll, set the new position
   if (delta.x || delta.y)
   {
      Point2I newPosition = mBounds.point;
      newPosition.x += delta.x;
      newPosition.y += delta.y;
      resize(newPosition, mBounds.extent);
   }
}

void GuiArrayCtrl::scrollSelectionTop()
{
   scrollCellTop( mSelectedCell );
}

void GuiArrayCtrl::scrollSelectionBottom()
{
   scrollCellBottom( mSelectedCell );
}

void GuiArrayCtrl::scrollCellTop( Point2I cell )
{
   // Make sure we have a parent:
   GuiControl* parent = getParent();
   if ( !parent )
      return;

   // Make sure the cell is valid:
   if ( ( cell.x < 0 ) || ( cell.y < 0 ) )
      return;
   
   // This is the >desired< new position--let the scroll control decide
   // whether it is a reasonable position or not...   
   Point2I newPos( -( cell.x * mCellSize.x ), -( cell.y * mCellSize.y ) );
   resize( newPos, mBounds.extent );                    
}

void GuiArrayCtrl::scrollCellBottom( Point2I cell )
{
   // Make sure we have a parent:
   GuiControl* parent = getParent();
   if ( !parent )
      return;

   // Make sure the cell is valid:
   if ( ( cell.x < 0 ) || ( cell.y < 0 ) )
      return;
   
   // This is the >desired< new position--let the scroll control decide
   // whether it is a reasonable position or not...   
   Point2I newPos( parent->mBounds.extent.x - ( ( cell.x + 1 ) * mCellSize.x ), 
         parent->mBounds.extent.y - ( ( cell.y + 1 ) * mCellSize.y ) );
   resize( newPos, mBounds.extent );                    
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiArrayCtrl::onRenderColumnHeaders(Point2I offset, Point2I parentOffset, Point2I headerDim)
{
   if (mProfile->mBorder)
   {
      RectI cellR(offset.x + headerDim.x, parentOffset.y, mBounds.extent.x - headerDim.x, headerDim.y);
      dglDrawRectFill(cellR, mProfile->mBorderColor);
   }
}

void GuiArrayCtrl::onRenderRowHeader(Point2I offset, Point2I parentOffset, Point2I headerDim, Point2I cell)
{
   ColorI color;
   RectI cellR;
   if (cell.x % 2)
      color.set(255, 0, 0, 255);
   else
      color.set(0, 255, 0, 255);
   
   cellR.point.set(parentOffset.x, offset.y);
   cellR.extent.set(headerDim.x, mCellSize.y);
   dglDrawRectFill(cellR, color);
}

void GuiArrayCtrl::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   ColorI color(255 * (cell.x % 2), 255 * (cell.y % 2), 255 * ((cell.x + cell.y) % 2), 255);
   if (selected)
   {
      color.set(255, 0, 0, 255);
   }
   else if (mouseOver)
   {
      color.set(0, 0, 255, 255);
   }
   
   //draw the cell
   RectI cellR(offset.x, offset.y, mCellSize.x, mCellSize.y);
   dglDrawRectFill(cellR, color);
}

void GuiArrayCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   firstResponder;
   
   //make sure we have a parent
   GuiControl *parent = getParent();
   if (! parent)
      return;
   
   S32 i, j;
   RectI headerClip;
   RectI clipRect(updateRect.point, updateRect.extent);
   
   Point2I parentOffset = parent->localToGlobalCoord(Point2I(0, 0));
   
   //if we have column headings
   if (mHeaderDim.y > 0)
   {
      headerClip.point.x =   parentOffset.x + mHeaderDim.x;
      headerClip.point.y =   parentOffset.y;
      headerClip.extent.x =  clipRect.extent.x - headerClip.point.x;
      headerClip.extent.y =  mHeaderDim.y;
      
      if (headerClip.intersect(clipRect))
      {
         dglSetClipRect(headerClip);
               
         //now render the header
         onRenderColumnHeaders(offset, parentOffset, mHeaderDim);
         
         clipRect.point.y = headerClip.point.y + headerClip.extent.y - 1;
      }
      offset.y += mHeaderDim.y;
   }
   
   //if we have row headings 
   if (mHeaderDim.x > 0)
   {
      clipRect.point.x = getMax(clipRect.point.x, parentOffset.x + mHeaderDim.x);
      offset.x += mHeaderDim.x;
   }
   
   //save the original for clipping the row headers
   RectI origClipRect = clipRect;
   
   for (j = 0; j < mSize.y; j++)
   {
      //skip until we get to a visible row
      if ((j + 1) * mCellSize.y + offset.y < updateRect.point.y)
         continue;
      
      //break once we've reached the last visible row
      if(j * mCellSize.y + offset.y >= updateRect.point.y + updateRect.extent.y)
         break;
         
      //render the header
      if (mHeaderDim.x > 0)
      {
         headerClip.point.x = parentOffset.x;
         headerClip.extent.x = mHeaderDim.x;
         headerClip.point.y = offset.y + j * mCellSize.y;
         headerClip.extent.y = mCellSize.y;
         if (headerClip.intersect(origClipRect))
         {
            dglSetClipRect(headerClip);
            
            //render the row header
            onRenderRowHeader(Point2I(0, offset.y + j * mCellSize.y),
                              Point2I(parentOffset.x, offset.y + j * mCellSize.y),
                              mHeaderDim, Point2I(0, j));
         }
      }
      
      //render the cells for the row
      for (i = 0; i < mSize.x; i++)
      {
         //skip past columns off the left edge
         if ((i + 1) * mCellSize.x + offset.x < updateRect.point.x)
            continue;
         
         //break once past the last visible column
         if (i * mCellSize.x + offset.x >= updateRect.point.x + updateRect.extent.x)
            break;
         
         S32 cellx = offset.x + i * mCellSize.x;
         S32 celly = offset.y + j * mCellSize.y;

         RectI cellClip(cellx, celly, mCellSize.x, mCellSize.y);
         
         //make sure the cell is within the update region
         if (cellClip.intersect(clipRect))
         {
            //set the clip rect
            dglSetClipRect(cellClip);
            
            //render the cell
            onRenderCell(Point2I(cellx, celly), Point2I(i, j), 
               i == mSelectedCell.x && j == mSelectedCell.y,
               i == mMouseOverCell.x && j == mMouseOverCell.y);
         }
      }
   }
}

void GuiArrayCtrl::onMouseDown(const GuiEvent &event)
{   
   if ( !mActive || !mAwake || !mVisible ) return;
   
   //let the guiControl method take care of the rest
   Parent::onMouseDown(event);

   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
   
      //store the previously selected cell
      Point2I prevSelected = mSelectedCell;
      
      //select the new cell
      cellSelected(Point2I(cell.x, cell.y));
      
      //if we double clicked on the *same* cell, evaluate the altConsole Command
      if ( ( event.mouseClickCount > 1 ) && ( prevSelected == mSelectedCell ) && mAltConsoleCommand[0] )
         Con::evaluate( mAltConsoleCommand, false );
   }   
}

void GuiArrayCtrl::onMouseEnter(const GuiEvent &event)
{
   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   
   //get the cell
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      mMouseOverCell = cell;
      setUpdateRegion(Point2I(cell.x * mCellSize.x + mHeaderDim.x,
                              cell.y * mCellSize.y + mHeaderDim.y), mCellSize );
   }
}

void GuiArrayCtrl::onMouseLeave(const GuiEvent & /*event*/)
{
   setUpdateRegion(Point2I(mMouseOverCell.x * mCellSize.x + mHeaderDim.x,
                           mMouseOverCell.y * mCellSize.y + mHeaderDim.y), mCellSize);
   mMouseOverCell.set(-1,-1);
}

void GuiArrayCtrl::onMouseMove(const GuiEvent &event)
{
   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x != mMouseOverCell.x || cell.y != mMouseOverCell.y)
   {
      if (mMouseOverCell.x != -1)
      {
         setUpdateRegion(Point2I(mMouseOverCell.x * mCellSize.x + mHeaderDim.x,
                           mMouseOverCell.y * mCellSize.y + mHeaderDim.y), mCellSize);
      }
      
      if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
      {
         setUpdateRegion(Point2I(cell.x * mCellSize.x + mHeaderDim.x,
                           cell.y * mCellSize.y + mHeaderDim.y), mCellSize);
         mMouseOverCell = cell;
      }
      else
         mMouseOverCell.set(-1,-1);
   }
}

bool GuiArrayCtrl::onKeyDown(const GuiEvent &event)
{
   //if this control is a dead end, kill the event
   if ((! mVisible) || (! mActive) || (! mAwake)) return true;
   
   //get the parent
   S32 pageSize = 1;
   GuiControl *parent = getParent();
   if (parent && mCellSize.y > 0)
   {
      pageSize = getMax(1, (parent->mBounds.extent.y / mCellSize.y) - 1);
   }
   
   Point2I delta(0,0);
   switch (event.keyCode)
   {
      case KEY_LEFT:
         delta.set(-1, 0);
         break;
      case KEY_RIGHT:
         delta.set(1, 0);
         break;
      case KEY_UP:
         delta.set(0, -1);
         break;
      case KEY_DOWN:
         delta.set(0, 1);
         break;
      case KEY_PAGE_UP:
         delta.set(0, -pageSize);
         break;
      case KEY_PAGE_DOWN:
         delta.set(0, pageSize);
         break;
      case KEY_HOME:
         cellSelected( Point2I( 0, 0 ) );
         return( true );
      case KEY_END:
         cellSelected( Point2I( 0, mSize.y - 1 ) );
         return( true );
      default:
         return Parent::onKeyDown(event);
   }
   if (mSize.x < 1 || mSize.y < 1)
      return true;

   //select the first cell if no previous cell was selected
   if (mSelectedCell.x == -1 || mSelectedCell.y == -1)
   {
      cellSelected(Point2I(0,0));
      return true;
   }
   
   //select the cell
   Point2I cell = mSelectedCell;
   cell.x = getMax(0, getMin(mSize.x - 1, cell.x + delta.x));
   cell.y = getMax(0, getMin(mSize.y - 1, cell.y + delta.y));
   cellSelected(cell);

   return true;
}

void GuiArrayCtrl::onRightMouseDown(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible ) 
      return;

   Parent::onRightMouseDown( event );

   Point2I pt = globalToLocalCoord( event.mousePoint );
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      char buf[32];
      dSprintf( buf, sizeof( buf ), "%d %d", event.mousePoint.x, event.mousePoint.y );
      // Pass it to the console: 
  	   Con::executef(this, 4, "onRightMouseDown", Con::getIntArg(cell.x), Con::getIntArg(cell.y), buf);
   }   
}
