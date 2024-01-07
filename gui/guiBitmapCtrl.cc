//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"

#include "GUI/guiBitmapCtrl.h"



GuiBitmapCtrl::GuiBitmapCtrl(void)
{
   mBitmapName = StringTable->insert("");
	startPoint.set(0, 0);
	mWrap = false;
}


void GuiBitmapCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("bitmap", TypeString, Offset(mBitmapName, GuiBitmapCtrl));
   addField("wrap",   TypeBool,   Offset(mWrap,       GuiBitmapCtrl));
}

static void cBitmapSetValue(SimObject *obj, S32, const char **argv)
{
   GuiBitmapCtrl *ctrl = static_cast<GuiBitmapCtrl*>(obj);
   ctrl->setValue(dAtoi(argv[2]), dAtoi(argv[3]));
}

static void cBitmapSetBitmap(SimObject *obj, S32, const char **argv)
{
   GuiBitmapCtrl *ctrl = static_cast<GuiBitmapCtrl*>(obj);
   ctrl->setBitmap(argv[2]);
}

void GuiBitmapCtrl::consoleInit()
{
   Con::addCommand("GuiBitmapCtrl", "setBitmap",  cBitmapSetBitmap,   "guiBitmapCtrl.setBitmap(blah)", 3, 3);
   Con::addCommand("GuiBitmapCtrl", "setValue",  cBitmapSetValue,   "guiBitmapCtrl.setValue(xAxis, yAxis)", 4, 4);
}


bool GuiBitmapCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setActive(true);
   setBitmap(mBitmapName);
   return true;
}

void GuiBitmapCtrl::onSleep()
{
   mTextureHandle = NULL;
   Parent::onSleep();
}

void GuiBitmapCtrl::setBitmap(const char *name)
{
   mBitmapName = StringTable->insert(name);
   if (*mBitmapName)
      mTextureHandle = TextureHandle(mBitmapName, BitmapTexture, true);
   else
      mTextureHandle = NULL;
   setUpdate();
}   


void GuiBitmapCtrl::setBitmap(const TextureHandle &handle)
{
   mTextureHandle = handle;   
}   


void GuiBitmapCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   if (mTextureHandle)
   {
      dglClearBitmapModulation();
		if(mWrap)
		{
 			TextureObject* texture = (TextureObject *) mTextureHandle;
			RectI srcRegion;
			RectI dstRegion;
			float xdone = ((float)mBounds.extent.x/(float)texture->bitmapWidth)+1;
			float ydone = ((float)mBounds.extent.y/(float)texture->bitmapHeight)+1;

			int xshift = startPoint.x%texture->bitmapWidth;
			int yshift = startPoint.y%texture->bitmapHeight;
			for(int y = 0; y < ydone; ++y)
				for(int x = 0; x < xdone; ++x)
				{
		 			srcRegion.set(0,0,texture->bitmapWidth,texture->bitmapHeight);
  					dstRegion.set( ((texture->bitmapWidth*x)+offset.x)-xshift,
								      ((texture->bitmapHeight*y)+offset.y)-yshift,
								      texture->bitmapWidth,	
								      texture->bitmapHeight);
   				dglDrawBitmapStretchSR(texture,dstRegion, srcRegion, false);
				}
		}
		else
      {
         RectI rect(offset, mBounds.extent);
//         RectI rect = mBounds;
//         rect.point += offset;
	      dglDrawBitmapStretch(mTextureHandle, rect);
      }
   }
   else
   {
      RectI rect = mBounds;
      rect.point += offset;

      glColor4f(0, 0, 0, 1);
      glBegin(GL_LINE_LOOP);
         glVertex2i(rect.point.x,                     rect.point.y);
         glVertex2i(rect.point.x + rect.extent.x - 1, rect.point.y);
         glVertex2i(rect.point.x + rect.extent.x - 1, rect.point.y + rect.extent.y - 1);
         glVertex2i(rect.point.x,                     rect.point.y + rect.extent.y - 1);
      glEnd();
   }

   renderChildControls(offset, updateRect, firstResponder);
}

void GuiBitmapCtrl::setValue(S32 x, S32 y)
{
   if (mTextureHandle)
   {
		TextureObject* texture = (TextureObject *) mTextureHandle;
		x+=texture->bitmapWidth/2;
		y+=texture->bitmapHeight/2;
  	}
  	while (x < 0)
  		x += 256;
  	startPoint.x = x % 256;
  				   
  	while (y < 0)
  		y += 256;
  	startPoint.y = y % 256;
}
