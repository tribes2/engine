//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "shell/shellTextEditCtrl.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"

IMPLEMENT_CONOBJECT(ShellTextEditCtrl);

//------------------------------------------------------------------------------
ShellTextEditCtrl::ShellTextEditCtrl() : GuiTextEditCtrl()
{
	dMemset( mBitmapBounds, 0, sizeof( mBitmapBounds ) );
   mTexField = NULL;
	mGlowOffset.set( 9, 9 );
}


//------------------------------------------------------------------------------
void ShellTextEditCtrl::initPersistFields()
{
	Parent::initPersistFields();
	addField( "glowOffset",	TypePoint2I, Offset( mGlowOffset, ShellTextEditCtrl ) );
}


//------------------------------------------------------------------------------
bool ShellTextEditCtrl::onWake()
{
	// Set the default profile on start-up:
	if ( !mProfile )
	{
      SimObject *obj = Sim::findObject("NewTextEditProfile");
      if ( obj )
         mProfile = dynamic_cast<GuiControlProfile*>( obj );
	}

   if ( !Parent::onWake() )
      return false;

   mTexField = mProfile->mTextureHandle;

	if ( mTexField )
	{
	   bool result = createBitmapArray( mTexField.getBitmap(), mBitmapBounds, StateCount, BmpCount );
	   AssertFatal( result, "ShellTextEditCtrl failed to create bitmap array!" );

		// Set minimum extents:
		mMinExtent.x = mBitmapBounds[StateCount * BmpLeft].extent.x + mBitmapBounds[StateCount * BmpRight].extent.x;
		mMinExtent.y = mBitmapBounds[StateCount * BmpLeft].extent.y;
		resize( mBounds.point, mBounds.extent );
	}

   return true;
}


//------------------------------------------------------------------------------
void ShellTextEditCtrl::onSleep()
{
	Parent::onSleep();
   mTexField = NULL;
}


//------------------------------------------------------------------------------
void ShellTextEditCtrl::onRender( Point2I offset, const RectI &updateRect, GuiControl* firstResponder )
{  
   // Draw the bitmap background:
	RectI drawRect( offset, mBounds.extent );

	if ( mTexField )
	{
	   // Differentiate between normal and active:
		U32 state; 
      if ( mActive )
      {
         state = ( firstResponder == this ) ? StateActive : StateNormal;
   	   dglClearBitmapModulation();
      }
      else
      {
         state = StateNormal;
         dglSetBitmapModulation( ColorI( 128, 128, 128 ) );
      }

	   // Draw the left edge:
	   U32 bitmap = StateCount * BmpLeft + state;
	   dglDrawBitmapSR( mTexField, drawRect.point, mBitmapBounds[bitmap] );

	   // Draw the center section:
	   bitmap = StateCount * BmpCenter + state;
	   drawRect.point.x += mBitmapBounds[bitmap].extent.x;
	   drawRect.extent.x = mBounds.extent.x - mBitmapBounds[StateCount * BmpLeft].extent.x - mBitmapBounds[StateCount * BmpRight].extent.x;
	   drawRect.extent.y = mBitmapBounds[bitmap].extent.y;
		if ( drawRect.extent.x > 0 )
	   	dglDrawBitmapStretchSR( mTexField, drawRect, mBitmapBounds[bitmap] );

	   // Draw the right edge:
	   bitmap = StateCount * BmpRight + state;
	   drawRect.point.x += drawRect.extent.x;
	   dglDrawBitmapSR( mTexField, drawRect.point, mBitmapBounds[bitmap] );
	}
 
   // Draw the text:
	drawRect.point.y = offset.y + mGlowOffset.y;
	drawRect.extent.y -= ( 2 * mGlowOffset.y );
	drawRect.point.x = offset.x + mBitmapBounds[BmpLeft].extent.x;
	RectI clipRect( drawRect.point, drawRect.extent );			   
	if ( clipRect.intersect( updateRect ) )
	{
		dglSetClipRect( clipRect );
		DrawText( drawRect, ( firstResponder == this ) );
	}


   // Draw border (if any):
   if ( mProfile->mBorder )
      dglDrawRect( RectI( offset, mBounds.extent ), mProfile->mBorderColor );
}
