//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudObject.h"
#include "hud/hudGLEx.h"
#include "console/consoleTypes.h"

IMPLEMENT_CONOBJECT( HudObject );

/**
 * Constructor 
 */
HudObject::HudObject() {
   mFillColor.set( 0.25, 0.25, 0.25, 0.25 );
   mFrameColor.set( 0, 1, 0, 1 );
   mTextColor.set( 1, 1, 1, 1 );
   mShadowColor.set( 0, 0, 0, 1 ); 
   mOpacity = 1.f;
   mShowFrame = mShowFill = true;
   mUse3dText = true;
}

/**
 * Method called to render the HUD object
 *
 * @param offset           Basically the corner to begin drawing at
 * @param updateRect       The rectangle that this object updates
 * @param firstResponder   ?
 */
void HudObject::onRender( Point2I offset, 
                          const RectI &updateRect, 
                          GuiControl *firstResponder ) {
   
   mFillColor.alpha = mFrameColor.alpha = mTextColor.alpha = mOpacity;

   if( mShowFill )
      dglDrawRectFill( updateRect, mFillColor );
   if( mShowFrame )
      dglDrawRect( updateRect, mFrameColor );   
   
   //renderChildControls( offset, updateRect, firstResponder );
}

/**
 * Draws text on the HUD control, the reason for this is to
 * allow for some text effects, like shadows and such
 *
 * @param offset Where to begin drawing
 * @param buf   The buffer of text to display
 */
void HudObject::drawText( Point2I offset, char *buf ) {
   if( mUse3dText ) {
      dglSetBitmapModulation( mShadowColor );
      dglDrawText( mProfile->mFont, Point2I( offset.x - 2, offset.y + 2 ), buf );
   }
   
   dglSetBitmapModulation( mTextColor );
   dglDrawText( mProfile->mFont, offset, buf );
   dglClearBitmapModulation();
}

/**
 * Registers console-modifyable datamembers
 */
void HudObject::initPersistFields() {
   Parent::initPersistFields();
   addField( "fillColor", TypeColorF, Offset( mFillColor, HudObject ) );
   addField( "frameColor", TypeColorF, Offset( mFrameColor, HudObject ) );
   addField( "textColor", TypeColorF, Offset( mTextColor, HudObject ) );
   addField( "shadowColor", TypeColorF, Offset( mShadowColor, HudObject ) );
   addField( "opacity", TypeF32, Offset( mOpacity, HudObject ) );
   addField( "showFill", TypeBool, Offset( mShowFill, HudObject ) );
   addField( "showFrame", TypeBool, Offset( mShowFrame, HudObject ) );
   addField( "use3dText", TypeBool, Offset( mUse3dText, HudObject ) );
}

// hudObject.cc