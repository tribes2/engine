//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBarDisplayCtrl.h"
#include "hud/hudGLEx.h"

IMPLEMENT_CONOBJECT( HudBarDisplayCtrl );

/**
 * Constructor
 */
HudBarDisplayCtrl::HudBarDisplayCtrl() {
   mPulseRate = 500;
   mPulseThreshold = 0.3f;
   mPulse = false;
   mHorizontalBar = true;
   mDrawFromOrigin = true;
   mShowPercentage = false;
   mDisplayMounted = false;
   mGradientFill = true;
   mValue = 0.2f;
}

/**
 * Default method that is intended to be over-ridden, returns
 * the current value of the bar in a percentile format
 *
 * @return Value of the bar as a percentile
 */
F32 HudBarDisplayCtrl::getValue() { 
   return mValue; 
}
      
/**
 * This method returns the color of the bar, this allows for
 * the changing of the bar color depending upon it's value
 * and things like that.
 *
 * @return Bar color
 */
ColorI HudBarDisplayCtrl::getBarColor() {
   return ColorI( 0, 255, 0 );
}

/**
 * This method returns the secondary color of the bar, this allows for
 * the changing of the bar color depending upon it's value
 * and things like that.
 *
 * @return Secondary bar color
 */
ColorI HudBarDisplayCtrl::getSecondBarColor() {
   return ColorI( 255, 0, 0 );
}

/**
 * Method called to render the HUD object
 *
 * @param offset           Basically the corner to begin drawing at
 * @param updateRect       The rectangle that this object updates
 * @param firstResponder   ?
 */
void HudBarDisplayCtrl::onRender( Point2I offset, 
                                  const RectI &updateRect, 
                                  GuiControl * firstResponder ) {
   mFillColor.alpha = mOpacity;
   
   F32 val = getValue();
   if( val > 1.0f )
      val = 1.0f;
   else if( val < 0.0f )
      val = 0.0f; 
   
   ColorI fill, fill2;
   fill = getBarColor();
   fill2 = getBarColor();
   
   fill.alpha = mOpacity * 255;
   fill2.alpha = mOpacity * 255;
   
   RectI rect( updateRect );

   S32 modVal = ( mDrawFromOrigin ? -1 : 1 );   
   
   if( mHorizontalBar )
      if( mDrawFromOrigin )
         rect.extent.x *= val;
      else {
         rect.point.x += rect.extent.x * ( 1.0f - val );
         rect.extent.x *= val;
      }
   else
      if( mDrawFromOrigin )
         rect.extent.y *= val;
      else {
         rect.point.y += rect.extent.y * ( 1.0f - val );
         rect.extent.y *= val;
         rect.extent.y++;        // Wierd, yeah, this is just to fix it being off by one pix from bottom
      }
      
   if( mPulse && ( mPulseRate != 0 ) ) {
      S32 time = (S32)Platform::getVirtualMilliseconds();
      F32 alpha = F32( time % mPulseRate ) / F32( mPulseRate / 2 );
   
      if( alpha > 1 )
         alpha = 1.f - ( alpha - 1.f );

      fill.alpha *= alpha;
      fill.alpha *= 255;
      fill2.alpha *= alpha;
      fill2.alpha *= 255;
   }
   
   if( mShowFill )
      dglDrawRectFill( updateRect, mFillColor );
   
   if( mGradientFill ) {
      fill2 = getSecondBarColor();
      fill2.alpha = mOpacity * 255;
   }
     
   if( !mHorizontalBar )
      if( !mDrawFromOrigin )
         dglDrawRectFill( rect, fill, fill2, fill, fill2 );
      else
         dglDrawRectFill( rect, fill2, fill, fill2, fill );
   else
      if( !mDrawFromOrigin )
         dglDrawRectFill( rect, fill, fill, fill2, fill2 );
      else
         dglDrawRectFill( rect, fill2, fill2, fill, fill );
   
   if( mShowPercentage ) {
      char buf[256];
      S32 per = (S32)mFloor( val * 100.0f );
        
      dSprintf( buf, sizeof( buf ), "%d%%", per );
      
      offset.x += ( updateRect.extent.x / 2 ) - ( mProfile->mFont->getStrWidth( buf ) / 2 );
      offset.y += ( updateRect.extent.y / 2 ) - ( mProfile->mFont->getHeight() / 2 );
   
      drawText( offset, buf );
   }
   
   if( mShowFrame )
      dglDrawRect( updateRect, mFrameColor );
}

/**
 * Registers console-modifyable datamembers
 */
void HudBarDisplayCtrl::initPersistFields() {
   Parent::initPersistFields();
   
   addField( "pulse", TypeBool, Offset( mPulse, HudBarDisplayCtrl ) );
   addField( "horizontalBar", TypeBool, Offset( mHorizontalBar, HudBarDisplayCtrl ) );
   addField( "drawFromOrigin", TypeBool, Offset( mDrawFromOrigin, HudBarDisplayCtrl ) );
   addField( "showPercentage", TypeBool, Offset( mShowPercentage, HudBarDisplayCtrl ) );
   addField( "displayMounted", TypeBool, Offset( mDisplayMounted, HudBarDisplayCtrl ) );
   addField( "gradientFill", TypeBool, Offset( mGradientFill, HudBarDisplayCtrl ) );
   addField( "pulseRate", TypeS32, Offset( mPulseRate, HudBarDisplayCtrl ) );
   addField( "pulseThreshold", TypeF32, Offset( mPulseThreshold, HudBarDisplayCtrl ) );
   addField( "value", TypeF32, Offset( mValue, HudBarDisplayCtrl ) );
}

// HudBarDisplayCtrl.cc