//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "g_surfac.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiHelpCtrl.h"

GuiHelpCtrl::GuiHelpCtrl(void)
{
   //set the id
   id = guiHelpObjectID;
   
   mHelpTag = 0;
   mHelpText = NULL;
}

GuiHelpCtrl::~GuiHelpCtrl()
{
   if (mHelpText)
   {
      delete [] (char*)mHelpText;
      mHelpText = NULL;
   }
}

bool GuiHelpCtrl::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
      
   setGFXFont(mFont, mFontString);
   setGFXFont(mFontHL, mFontStringHL);
   setGFXFont(mFontNA, mFontStringNA);
   
   return TRUE;
}

void GuiHelpCtrl::render(GFXSurface *sfc)
{
   if ((! mHelpText) || (! mHelpText[0]))
      return;
      
   //if opaque, fill the update rect with the fill color
   if (mOpaque)
   {
      sfc->drawRect2d_f(&RectI(mPosition, Point2I(mPosition.x + mExtent.x - 1, mPosition.y + mExtent.y - 1)), mFillColor);
   }
   
   //if there's a boarder, draw the boarder
   if (mBorder)
   {
      sfc->drawRect2d(&RectI(mPosition, Point2I(mPosition.x + mExtent.x - 1, mPosition.y + mExtent.y - 1)), mBorderColor);
   }
   
   //draw the help text
   sfc->drawText_p(mFont, &Point2I(mPosition.x + 2, mPosition.y), mHelpText);
}

void GuiHelpCtrl::setHelpText(guiCanvas *root, const char *text, F32 /* rElapsedTime */, bool /*mouseClicked*/)
{
   Point2I newPos, newExt;
   if (mHelpText)
   {
      delete [] (char*)mHelpText;
      mHelpText = NULL;
      newPos.set(0, 0);
      newExt.set(1, 1);
   }

   if (text && text[0])
   {
      mHelpText = strnew(text);
      mHelpTag = 0;
      
      if (bool(mFont) && root)
      {
         newExt.set(mFont->getStrWidth(text) + 4, mFont->getHeight() + 4);
         newPos = root->getCursorPos();
         newPos.x = MAX(0, MIN(newPos.x, root->mExtent.x - newExt.x));
         newPos.y = MAX(0, MIN(newPos.y, root->mExtent.y - newExt.y));
      }
   }
   resize(newPos, newExt);
}

void GuiHelpCtrl::setHelpTag(guiCanvas *root, S32 helpTag, F32 /* rElapsedTime */, bool /*mouseClicked*/)
{
   if (mHelpText)
   {
      delete [] (char*)mHelpText;
      mHelpText = NULL;
   }

   mHelpTag = helpTag;
}
