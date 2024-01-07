//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"
#include "dgl/gBitmap.h"
#include "GUI/guiControl.h"
#include "dgl/gTexManager.h"
#include "dgl/gChunkedTexManager.h"

class GuiChunkedBitmapCtrl : public GuiControl
{
private:
	typedef GuiControl Parent;

protected:
   StringTableEntry mBitmapName;
   ChunkedTextureHandle mTexHandle;
   bool  mUseVariable;
   
public:
	//creation methods
	DECLARE_CONOBJECT(GuiChunkedBitmapCtrl);
	GuiChunkedBitmapCtrl();
   static void initPersistFields();
   static void consoleInit();

   //Parental methods
   bool onWake();
   void onSleep();

   void setBitmap(const char *name);

	void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
};

IMPLEMENT_CONOBJECT(GuiChunkedBitmapCtrl);

void GuiChunkedBitmapCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField( "bitmap",        TypeString, Offset( mBitmapName, GuiChunkedBitmapCtrl ) );
   addField( "useVariable",   TypeBool,   Offset( mUseVariable, GuiChunkedBitmapCtrl ) );
}

static void cChunkBmpSetBitmap( SimObject* obj, S32, const char** argv )
{
   AssertFatal( dynamic_cast<GuiChunkedBitmapCtrl*>( obj ), "Object passed to cChunkBmpSetBitmap is not a GuiChunkedBitmapCtrl!" );
   GuiChunkedBitmapCtrl* ctrl = static_cast<GuiChunkedBitmapCtrl*>( obj );
   ctrl->setBitmap( argv[2] );
}

void GuiChunkedBitmapCtrl::consoleInit()
{
   Con::addCommand( "GuiChunkedBitmapCtrl", "setBitmap", cChunkBmpSetBitmap, "ctrl.setBitmap( name );", 3, 3 );
}

GuiChunkedBitmapCtrl::GuiChunkedBitmapCtrl()
{
   mBitmapName = StringTable->insert("");
   mUseVariable = false;
}

void GuiChunkedBitmapCtrl::setBitmap(const char *name)
{
   bool awake = mAwake;
   if(awake)
      onSleep();
   
   mBitmapName = StringTable->insert(name);
   if(awake)
      onWake();
   setUpdate();
}

bool GuiChunkedBitmapCtrl::onWake()
{
   if(!Parent::onWake())
      return false;
   if ( mUseVariable )
      mTexHandle = ChunkedTextureHandle( Con::getVariable( mConsoleVariable ) );
   else
      mTexHandle = ChunkedTextureHandle( mBitmapName );
   return true;
}

void GuiChunkedBitmapCtrl::onSleep()
{
   mTexHandle = NULL;
   Parent::onSleep();
}

void GuiChunkedBitmapCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   if(mTexHandle)
   {
      U32 widthCount = mTexHandle.getTextureCountWidth();
      U32 heightCount = mTexHandle.getTextureCountHeight();
      if(!widthCount || !heightCount)
         return;
      
      F32 widthScale = F32(mBounds.extent.x) / F32(mTexHandle.getWidth());
      F32 heightScale = F32(mBounds.extent.y) / F32(mTexHandle.getHeight());
      dglSetBitmapModulation(ColorF(1,1,1));
      for(U32 i = 0; i < widthCount; i++)
      {
         for(U32 j = 0; j < heightCount; j++)
         {
            TextureHandle t = mTexHandle.getSubTexture(i, j);
            RectI stretchRegion;
            stretchRegion.point.x = (S32)(i * 256 * widthScale  + offset.x);
            stretchRegion.point.y = (S32)(j * 256 * heightScale + offset.y);
            stretchRegion.extent.x = (S32)((i * 256 + t.getWidth() ) * widthScale  + offset.x - stretchRegion.point.x);
            stretchRegion.extent.y = (S32)((j * 256 + t.getHeight()) * heightScale + offset.y - stretchRegion.point.y);
            dglDrawBitmapStretch(t, stretchRegion);
         }
      }
      renderChildControls(offset, updateRect, firstResponder);
   }
   else
      Parent::onRender(offset, updateRect, firstResponder);
}
