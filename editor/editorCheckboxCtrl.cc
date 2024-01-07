//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Editor/editorCheckboxCtrl.h"
#include "console/consoleTypes.h"
#include "GUI/guiCanvas.h"
#include "dgl/dgl.h"

IMPLEMENT_CONOBJECT(EditorCheckBoxCtrl);

EditorCheckBoxCtrl::EditorCheckBoxCtrl()
{
   mBitmapName = StringTable->insert("");
   mTextureHandle = 0;
   mMouseOverColor.set(200,0,0);
   mDepressedAlpha = 0.2;
}

//------------------------------------------------------------------------------

void EditorCheckBoxCtrl::onRender(Point2I offset, const RectI & updateRect, 
   GuiControl * firstResponder)
{
   GuiCanvas * root = getRoot();
   bool stateOver = cursorInControl();
   bool stateDepressed = (stateOver && root && root->mouseButtonDown());
   
   if(mTextureHandle)
   {
      dglClearBitmapModulation();
      dglDrawBitmapStretch(mTextureHandle, updateRect);
   }
   else
   {
      glColor4f(0, 0, 0, 1);
      glBegin(GL_LINE_LOOP);
         glVertex2i(updateRect.point.x, updateRect.point.y);
         glVertex2i(updateRect.point.x+updateRect.extent.x-1, updateRect.point.y);
         glVertex2i(updateRect.point.x+updateRect.extent.x-1, updateRect.point.y+updateRect.extent.y-1);
         glVertex2i(updateRect.point.x, updateRect.point.y+updateRect.extent.y-1);
      glEnd();
   }

   //
   if(stateOver)
      Parent::drawBorder(updateRect, mMouseOverColor);
   
   //
   if(!mStateOn)
   {
      glDisable(GL_CULL_FACE);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
      glBegin(GL_QUADS);
      glColor4f(1,1,1,mDepressedAlpha);
      glVertex2i(updateRect.point.x, updateRect.point.y);
      glVertex2i(updateRect.point.x+updateRect.extent.x-1, updateRect.point.y);
      glVertex2i(updateRect.point.x+updateRect.extent.x-1, updateRect.point.y+updateRect.extent.y-1);
      glVertex2i(updateRect.point.x, updateRect.point.y+updateRect.extent.y-1);
      glEnd();
      glDisable(GL_BLEND);
   }
   renderChildControls(offset, updateRect, firstResponder);
}

//------------------------------------------------------------------------------

void EditorCheckBoxCtrl::setBitmap(const char * name)
{
   mBitmapName = StringTable->insert(name);
   if(mBitmapName[0])
      mTextureHandle = TextureHandle(mBitmapName, BitmapTexture);
   else
      mTextureHandle = 0;
   setUpdate();
}

//------------------------------------------------------------------------------

bool EditorCheckBoxCtrl::onWake()
{
   if(!Parent::onWake())
      return false;
   setActive(true);
   setBitmap(mBitmapName);
   return(true);
}

void EditorCheckBoxCtrl::onSleep()
{
   mTextureHandle = 0;
   Parent::onSleep();
}

//------------------------------------------------------------------------------

void EditorCheckBoxCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("bitmap", TypeString, Offset(mBitmapName, EditorCheckBoxCtrl));
   addField("mouseOverColor", TypeColorI, Offset(mMouseOverColor, EditorCheckBoxCtrl));
   addField("depressedAlpha", TypeF32, Offset(mDepressedAlpha, EditorCheckBoxCtrl));
}

static void cSetBitmap(SimObject * obj, S32, const char ** argv)
{
   EditorCheckBoxCtrl * ctrl = static_cast<EditorCheckBoxCtrl*>(obj);
   ctrl->setBitmap(argv[2]);
}

void EditorCheckBoxCtrl::consoleInit()
{
   Con::addCommand("EditorCheckBoxCtrl", "setBitmap", cSetBitmap, "editorButtonCtrl.setBitmap(name);", 3, 3);
}
