//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBezierDisplayCtrl.h"
#include "hud/hudGLEx.h"

IMPLEMENT_CONOBJECT( HudBezierDisplayCtrl );

HudBezierDisplayCtrl::HudBezierDisplayCtrl() {
   mCurve = NULL;
   
   mPulseRate = 500;
   mPulseThreshold = 0.3f;
   mPulse = false;
   mDrawFromOrigin = true;
   mShowPercentage = false;
   mDisplayMounted = false;
   mGradientFill = true;
   mValue = 0.2f;
   
   mOffset.set( 0, 0 );
   mTextOffset.set( 0, 0 );
   mScaleValue = 0.4f;
}

/**
 * Destructor
 */
HudBezierDisplayCtrl::~HudBezierDisplayCtrl() { 
   if( mCurve != NULL ) 
      delete mCurve; 
}

/**
 * Default method that is intended to be over-ridden, returns
 * the current value of the bar in a percentile format
 *
 * @return Value of the bar as a percentile
 */
F32 HudBezierDisplayCtrl::getValue() { 
   return mValue; 
}
      
/**
 * This method returns the color of the bar, this allows for
 * the changing of the bar color depending upon it's value
 * and things like that.
 *
 * @return Bar color
 */
ColorI HudBezierDisplayCtrl::getBarColor() {
   return ColorI( 0, 255, 0 );
}

/**
 * This method returns the secondary color of the bar, this allows for
 * the changing of the bar color depending upon it's value
 * and things like that.
 *
 * @return Secondary bar color
 */
ColorI HudBezierDisplayCtrl::getSecondBarColor() {
   return ColorI( 255, 0, 0 );
}

/**
 * This method gets executed whenever the control is resized
 *
 * @param newPosition The new position of the top left corner of the control
 * @param newExtent   The new extent of the control
 */
void HudBezierDisplayCtrl::resize( const Point2I &newPosition, const Point2I &newExtent ) {
   Parent::resize( newPosition, newExtent );
   
   Point2F ctrlPoints[4];
   Point2I upperL, lowerR;
   
   upperL.set( mBounds.point.x, mBounds.point.y );
   lowerR.set( mBounds.point.x + mBounds.extent.x - 1, mBounds.point.y + mBounds.extent.y - 1 );
   
   ctrlPoints[0].set( upperL.x, lowerR.y );
   ctrlPoints[1].set( upperL.x, upperL.y );
   ctrlPoints[2].set( lowerR.x, upperL.y );
   ctrlPoints[3].set( lowerR.x, lowerR.y + 10 );

   if( mCurve == NULL )
      mCurve = new Bezier2D( ctrlPoints, 4, 49 );
   else
      mCurve->setControlPoints( ctrlPoints, 4 );
}

/**
 * Method called to render the HUD object
 *
 * @param offset           Basically the corner to begin drawing at
 * @param updateRect       The rectangle that this object updates
 * @param firstResponder   ?
 */
void HudBezierDisplayCtrl::onRender( Point2I offset, const RectI &updateRect, GuiControl *firstResponder ) {
   if( mCurve != NULL ) {
      mFillColor.alpha = mOpacity;
      
      ColorI fill = getBarColor();
      ColorI fill2 = getSecondBarColor();
      
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
         dglDrawBezierBand( mCurve, mOffset, mScaleValue, mFillColor );
      if( mGradientFill )
         dglDrawBezierBand( mCurve, mOffset, mScaleValue, fill, fill2, getValue(), !mDrawFromOrigin );
      else
         dglDrawBezierBand( mCurve, mOffset, mScaleValue, fill, getValue(), !mDrawFromOrigin );
      
      if( mShowFrame ) {
         dglDrawBezier( mCurve, mFrameColor );
         dglDrawBezier( mCurve->getScaledCurve( mScaleValue ), mFrameColor, mOffset );
         
         Point2I pt1( mCurve->getScaledCurve( mScaleValue )->getCurvePoints()[0].x + mOffset.x, 
                  mCurve->getScaledCurve( mScaleValue )->getCurvePoints()[0].y - mOffset.y );
         Point2I pt2( mCurve->getCurvePoints()[0].x, mCurve->getCurvePoints()[0].y );
         dglDrawLine( pt1, pt2, mFrameColor );

         pt1.set( mCurve->getScaledCurve( mScaleValue )->getCurvePoints()[mCurve->getNumCurvePoints() - 1].x + mOffset.x,
                  mCurve->getScaledCurve( mScaleValue )->getCurvePoints()[mCurve->getNumCurvePoints() - 1].y - mOffset.y );
         pt2.set( mCurve->getCurvePoints()[mCurve->getNumCurvePoints() - 1].x,
                      mCurve->getCurvePoints()[mCurve->getNumCurvePoints() - 1].y );
         dglDrawLine( pt1, pt2, mFrameColor );
      }
      
      if( mShowPercentage ) {
         char buf[256];
         S32 per = (S32)mFloor( getValue() * 100.0f );
           
         dSprintf( buf, sizeof( buf ), "%d%%", per );
      
         drawText( Point2I( offset.x + mTextOffset.x - ( mProfile->mFont->getStrWidth( buf ) / 2 ),
                            offset.y + mTextOffset.y - ( mProfile->mFont->getHeight() / 2 ) ), buf );
      }
   }
}

/**
 * Registers console-modifyable datamembers
 */
void HudBezierDisplayCtrl::initPersistFields() {
   Parent::initPersistFields();
   
   addField( "pulse", TypeBool, Offset( mPulse, HudBezierDisplayCtrl ) );
   addField( "drawFromOrigin", TypeBool, Offset( mDrawFromOrigin, HudBezierDisplayCtrl ) );
   addField( "showPercentage", TypeBool, Offset( mShowPercentage, HudBezierDisplayCtrl ) );
   addField( "displayMounted", TypeBool, Offset( mDisplayMounted, HudBezierDisplayCtrl ) );
   addField( "gradientFill", TypeBool, Offset( mGradientFill, HudBezierDisplayCtrl ) );
   addField( "pulseRate", TypeS32, Offset( mPulseRate, HudBezierDisplayCtrl ) );
   addField( "pulseThreshold", TypeF32, Offset( mPulseThreshold, HudBezierDisplayCtrl ) );
   addField( "value", TypeF32, Offset( mValue, HudBezierDisplayCtrl ) );
   addField( "offsetX", TypeF32, Offset( mOffset.x, HudBezierDisplayCtrl ) );
   addField( "offsetY", TypeF32, Offset( mOffset.y, HudBezierDisplayCtrl ) );
   addField( "textOffsetX", TypeF32, Offset( mTextOffset.x, HudBezierDisplayCtrl ) );
   addField( "textOffsetY", TypeF32, Offset( mTextOffset.y, HudBezierDisplayCtrl ) );
   addField( "scaleValue", TypeF32, Offset( mScaleValue, HudBezierDisplayCtrl ) );
}

// hudBezierDisplayCtrl.cc