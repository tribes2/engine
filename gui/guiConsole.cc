//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiTypes.h"
#include "GUI/guiControl.h"
#include "GUI/guiConsole.h"

GuiConsole::GuiConsole()
{
   mBounds.extent.set(1, 1);
   mCellSize.set(1, 1);
   mSize.set(1, 0);
}

bool GuiConsole::onWake()
{
   if (! Parent::onWake())
      return false;
      
   //get the font
   mFont = mProfile->mFont;
   
   return true;
}

S32 GuiConsole::getMaxWidth(S32 startIndex, S32 endIndex)
{
   //sanity check
   U32 size;
   ConsoleLogEntry *log;
   
   Con::getLog(log, size);
      
   if(startIndex < 0 || (U32)endIndex >= size || startIndex > endIndex)
      return 0;
      
   S32 result = 0;
   for(S32 i = startIndex; i <= endIndex; i++)
      result = getMax(result, (S32)(mFont->getStrWidth(log[i].mString)));
      
   return(result + 6);
}

void GuiConsole::onPreRender()
{
   //see if the size has changed
   U32 prevSize = mBounds.extent.y / mCellSize.y;
   U32 size;
   ConsoleLogEntry *log;
   
   Con::getLog(log, size);

   if(size != prevSize)
   {
      //first, find out if the console was scrolled up
      bool scrolled = false;
      GuiControl *parent = getParent();
      if (mBounds.point.y > parent->mBounds.extent.y - mBounds.extent.y)
         scrolled = true;
         
      //find the max cell width for the new entries
      S32 newMax = getMaxWidth(prevSize, size - 1);
      if(newMax > mCellSize.x)
         mCellSize.set(newMax, mFont->getHeight());
         
      //set the array size
      mSize.set(1, size);
      
      //resize the control
      resize(mBounds.point, Point2I(mCellSize.x, mCellSize.y * size));
      
      //if the console was not scrolled, make the last entry visible
      if (! scrolled)
      {
         mSelectedCell.set(0, mSize.y - 1);
         scrollSelectionVisible();
      }
   }
}

void GuiConsole::onRenderCell(Point2I offset, Point2I cell, bool /*selected*/, bool /*mouseOver*/)
{
   U32 size;
   ConsoleLogEntry *log;
   
   Con::getLog(log, size);

   ConsoleLogEntry &entry = log[cell.y];
   switch (entry.mLevel)
   {
      case ConsoleLogEntry::Normal:   dglSetBitmapModulation(mProfile->mFontColor); break;
      case ConsoleLogEntry::Warning:  dglSetBitmapModulation(mProfile->mFontColorHL); break;
      case ConsoleLogEntry::Error:    dglSetBitmapModulation(mProfile->mFontColorNA); break;
   }
   dglDrawText(mFont, Point2I(offset.x + 3, offset.y), entry.mString, mProfile->mFontColors);
}
