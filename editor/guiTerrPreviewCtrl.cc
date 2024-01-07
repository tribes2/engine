//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"
#include "platformWin32/platformGL.h"
#include "game/game.h"
#include "terrain/terrData.h"
#include "editor/guiTerrPreviewCtrl.h"

IMPLEMENT_CONOBJECT(GuiTerrPreviewCtrl);

GuiTerrPreviewCtrl::GuiTerrPreviewCtrl(void)
{
   mTerrainSize = 2048.0f;
   mRoot.set( 0, 0 ); 
   mOrigin.set( 0, 0 );
   mWorldScreenCenter.set( mTerrainSize*0.5f, mTerrainSize*0.5f );
}

void GuiTerrPreviewCtrl::initPersistFields()
{
   Parent::initPersistFields();
}


static void cTerrPreview_Reset(SimObject *obj, S32, const char **)
{
   static_cast<GuiTerrPreviewCtrl*>(obj)->reset();
}

static void cTerrPreview_SetRoot(SimObject *obj, S32, const char **)
{
   static_cast<GuiTerrPreviewCtrl*>(obj)->setRoot();
}

static const char* cTerrPreview_GetRoot(SimObject *obj, S32, const char **)
{
   GuiTerrPreviewCtrl *ctrl = static_cast<GuiTerrPreviewCtrl*>(obj);
   Point2F p = ctrl->getRoot();

   static char buf[32];
   dSprintf(buf,sizeof(buf),"%f %f", p.x, -p.y);
   return buf;
}

static void cTerrPreview_SetOrigin(SimObject *obj, S32, const char **argv)
{
   GuiTerrPreviewCtrl *ctrl = static_cast<GuiTerrPreviewCtrl*>(obj);
   ctrl->setOrigin( Point2F( dAtof(argv[2]), -dAtof(argv[3]) ) );
}

static const char* cTerrPreview_GetOrigin(SimObject *obj, S32, const char **)
{
   GuiTerrPreviewCtrl *ctrl = static_cast<GuiTerrPreviewCtrl*>(obj);
   Point2F p = ctrl->getOrigin();

   static char buf[32];
   dSprintf(buf,sizeof(buf),"%f %f", p.x, -p.y);
   return buf;
}

static const char* cTerrPreview_GetValue(SimObject *obj, S32, const char **)
{
   GuiTerrPreviewCtrl *ctrl = static_cast<GuiTerrPreviewCtrl*>(obj);
   Point2F r = ctrl->getRoot();
   Point2F o = ctrl->getOrigin();

   static char buf[64];
   dSprintf(buf,sizeof(buf),"%f %f %f %f", r.x, -r.y, o.x, -o.y);
   return buf;
}

static void cTerrPreview_SetValue(SimObject *obj, S32, const char **argv)
{
   Point2F r,o;
   GuiTerrPreviewCtrl *ctrl = static_cast<GuiTerrPreviewCtrl*>(obj);
   dSscanf(argv[1],"%f %f %f %f", &r.x, &r.y, &o.x, &o.y);
   r.y = -r.y;
   o.y = -o.y;
   ctrl->reset();
   ctrl->setRoot(r);
   ctrl->setOrigin(o);
}


void GuiTerrPreviewCtrl::consoleInit()
{
   Con::addCommand("GuiTerrPreviewCtrl", "reset",      cTerrPreview_Reset,       "guiTerrPreviewCtrl.reset()",         2, 2);
   Con::addCommand("GuiTerrPreviewCtrl", "setRoot",    cTerrPreview_SetRoot,     "guiTerrPreviewCtrl.setRoot()",       2, 2);
   Con::addCommand("GuiTerrPreviewCtrl", "getRoot",    cTerrPreview_GetRoot,     "guiTerrPreviewCtrl.getRoot()",       2, 2);
   Con::addCommand("GuiTerrPreviewCtrl", "setOrigin",  cTerrPreview_SetOrigin,   "guiTerrPreviewCtrl.setOrigin(x,y)",  4, 4);
   Con::addCommand("GuiTerrPreviewCtrl", "getOrigin",  cTerrPreview_GetOrigin,   "guiTerrPreviewCtrl.getOrigin()",     2, 2);
   Con::addCommand("GuiTerrPreviewCtrl", "getValue",   cTerrPreview_GetValue,    "guiTerrPreviewCtrl.getValue()",      2, 2);
   Con::addCommand("GuiTerrPreviewCtrl", "setValue",   cTerrPreview_SetValue,    "guiTerrPreviewCtrl.getValue(t)",     3, 3);
}


bool GuiTerrPreviewCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   
   return true;
}

void GuiTerrPreviewCtrl::onSleep()
{
   Parent::onSleep();
}


void GuiTerrPreviewCtrl::setBitmap(const TextureHandle &handle)
{
   mTextureHandle = handle;   
}   

void GuiTerrPreviewCtrl::reset()
{
   mRoot.set(0,0);
   mOrigin.set(0,0);   
}   

void GuiTerrPreviewCtrl::setRoot()
{
   mRoot += mOrigin;
   mOrigin.set(0,0);   
}   

void GuiTerrPreviewCtrl::setRoot(const Point2F &p)
{
   mRoot = p;
}   

void GuiTerrPreviewCtrl::setOrigin(const Point2F &p)
{
   mOrigin = p;
}   


Point2F& GuiTerrPreviewCtrl::wrap(const Point2F &p)
{
   static Point2F result;
   result = p;

   while (result.x < 0.0f)
      result.x += mTerrainSize;
   while (result.x > mTerrainSize)
      result.x -= mTerrainSize;
   while (result.y < 0.0f)
      result.y += mTerrainSize;
   while (result.y > mTerrainSize)
      result.y -= mTerrainSize;

   return result;
}   

Point2F& GuiTerrPreviewCtrl::worldToTexture(const Point2F &p)
{
   static Point2F result;
   result = wrap( p + mRoot ) / mTerrainSize;   
   return result;
}   


Point2F& GuiTerrPreviewCtrl::worldToCtrl(const Point2F &p)
{
   static Point2F result;
   result = wrap( p - mCamera + mOrigin - mWorldScreenCenter );
   result *= mBounds.extent.x / mTerrainSize;
   return result;
}   


void GuiTerrPreviewCtrl::onPreRender()
{
   setUpdate();
}

void GuiTerrPreviewCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   struct CameraQuery query;
	GameProcessCameraQuery(&query);
   Point3F cameraRot;

	MatrixF matrix = query.cameraMatrix;
   matrix.getColumn(3,&cameraRot);           // get Camera translation
   mCamera.set(cameraRot.x, -cameraRot.y);
	matrix.getRow(1,&cameraRot);              // get camera rotation

   mTerrainSize = 8*256;
   TerrainBlock *terrBlock = dynamic_cast<TerrainBlock*>(Sim::findObject("Terrain"));
   if (terrBlock)
      mTerrainSize = terrBlock->getSquareSize()*TerrainBlock::BlockSize;
      

   //----------------------------------------- RENDER the Terrain Bitmap
   if (mTextureHandle)
   {
      TextureObject *texture = (TextureObject*)mTextureHandle;
      if (texture)
      {
         glEnable(GL_TEXTURE_2D);
      
         glBindTexture(GL_TEXTURE_2D, texture->texGLName);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

         Point2F screenP1(offset.x - 0.5f, offset.y + 0.5f);
         Point2F screenP2(offset.x + mBounds.extent.x - 0.5f, offset.y + mBounds.extent.x + 0.5f);
         Point2F textureP1( worldToTexture( mCamera ) );
         Point2F textureP2(textureP1 + Point2F(1.0f, 1.0f));

         // the texture if flipped horz to reflect how the terrain is really drawn
         glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(textureP1.x, textureP2.y);   
            glVertex2f(screenP1.x, screenP2.y);       // left bottom
   
            glTexCoord2f(textureP2.x, textureP2.y);   
            glVertex2f(screenP2.x, screenP2.y);       // right bottom
   
            glTexCoord2f(textureP2.x, textureP1.y);   
            glVertex2f(screenP2.x, screenP1.y);       // right top

            glTexCoord2f(textureP1.x, textureP1.y);   
            glVertex2f(screenP1.x, screenP1.y);       // left top
         glEnd();
          
         glDisable(GL_TEXTURE_2D);
      }
   }
   else
   {
      RectI rect(offset.x, offset.y, mBounds.extent.x, mBounds.extent.y);
      dglDrawRectFill(rect, ColorI(0,0,0));   
   }

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   //----------------------------------------- RENDER the '+' at the center of the Block
   
	glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
   Point2F center( worldToCtrl(Point2F(0,0)) );
   S32 y;
   for (y=-1; y<=1; y++)
   {
      F32 yoffset = offset.y + y*256.0f;
      for (S32 x=-1; x<=1; x++)
      {
         F32 xoffset = offset.x + x*256.0f;
         glBegin(GL_LINES);
            glVertex2f(xoffset + center.x, yoffset + center.y-5);
            glVertex2f(xoffset + center.x, yoffset + center.y+6);
            glVertex2f(xoffset + center.x-5, yoffset + center.y);
            glVertex2f(xoffset + center.x+6, yoffset + center.y);
         glEnd();
      }
   }
   
   //----------------------------------------- RENDER the Block Corners
   Point2F corner( worldToCtrl(Point2F(-mTerrainSize/2.0f, -mTerrainSize/2.0f)) );
   for (y=-1; y<=1; y++)
   {
      S32 yoffset = offset.y + y*256;
      for (S32 x=-1; x<=1; x++)
      {
         S32 xoffset = offset.x + x*256;
         glBegin(GL_LINE_STRIP);
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
            glVertex2i(xoffset + corner.x, yoffset + corner.y-128);
            glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
            glVertex2i(xoffset + corner.x, yoffset + corner.y);
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
            glVertex2i(xoffset + corner.x+128, yoffset + corner.y);
         glEnd();
         glBegin(GL_LINE_STRIP);
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
            glVertex2i(xoffset + corner.x, yoffset + corner.y+128);
            glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
            glVertex2i(xoffset + corner.x, yoffset + corner.y);
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
            glVertex2i(xoffset + corner.x-128, yoffset + corner.y);
         glEnd();
      }
   }


   //----------------------------------------- RENDER the Viewcone
   Point2F pointA(cameraRot.x * -40, cameraRot.y * -40); 
	Point2F pointB(-pointA.y, pointA.x);

	F32 tann = mTan(0.5f);
	Point2F point1( pointA + pointB * tann );
	Point2F point2( pointA - pointB * tann );

	center.set(offset.x + mBounds.extent.x / 2, offset.y + mBounds.extent.y / 2 );
	glBegin(GL_LINE_STRIP);
	   glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
	   glVertex2i(center.x + point1.x, center.y + point1.y);
	   glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	   glVertex2i(center.x, center.y);
	   glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
	   glVertex2i(center.x + point2.x, center.y + point2.y);
	glEnd();

	glDisable(GL_BLEND);

   /* debuging stuff
   Point2I loc(offset.x +5, offset.y+10);
	dglSetBitmapModulation(mProfile->mFontColor);
	dglDrawText(mProfile->mFont, loc, avar("mCamera(%3.2f, %3.2f)", mCamera.x, mCamera.y)); loc.y += 10;
	dglDrawText(mProfile->mFont, loc, avar("mRoot(%3.2f, %3.2f)", mRoot.x, mRoot.y)); loc.y += 10;
	dglDrawText(mProfile->mFont, loc, avar("mOrigin(%3.2f, %3.2f)", mOrigin.x, mOrigin.y)); loc.y += 10;
   */

   renderChildControls(offset, updateRect, firstResponder);
}


