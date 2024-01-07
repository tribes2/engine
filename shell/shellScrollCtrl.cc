//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/guiCanvas.h"
#include "console/consoleTypes.h"
#include "Shell/shellScrollCtrl.h"
#include "dgl/dgl.h"

IMPLEMENT_CONOBJECT(ShellScrollCtrl);

//------------------------------------------------------------------------------
ShellScrollCtrl::ShellScrollCtrl() : GuiScrollCtrl()
{
	mGlowOffset = 4;
	mBorderThickness = mGlowOffset;

   dMemset( mBarBounds, 0, sizeof( mBarBounds ) );
   mTexVBar = NULL;
   mTexHBar = NULL;

   mTexVButtons = NULL;
   mTexVThumb = NULL;
   mTexHButtons = NULL;
   mTexHThumb = NULL;
   mTexCorner = NULL;

   mFieldBase = StringTable->insert( "gui/shll_field" );
   mTexLeftTop = NULL;
   mTexCenterTop = NULL;
   mTexRightTop = NULL;
   mTexLeftCenter = NULL;
   mTexCenter = NULL;
   mTexRightCenter = NULL;
   mTexLeftBottom = NULL;
   mTexCenterBottom = NULL;
   mTexRightBottom = NULL;

	mMouseOverRegion = None;
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField( "fieldBase", TypeString, Offset( mFieldBase, ShellScrollCtrl ) );
}

//------------------------------------------------------------------------------
bool ShellScrollCtrl::onWake()
{
	// Set the default profile on start-up:
	if ( !mProfile )
	{
      SimObject *obj = Sim::findObject("NewScrollCtrlProfile");
      if ( obj )
         mProfile = dynamic_cast<GuiControlProfile*>( obj );
	}

	// NOTE:  Not using Parent here on purpose.
   if ( !GuiControl::onWake() )
      return false;

	AssertFatal( size(), "ShellScrollCtrl created without content!" );

	mBitmapBounds = new RectI[BtnCount * BtnStateCount];
	AssertFatal( mBitmapBounds, "Failed to allocate memory for bitmap array!" );

   char buf[256];
   if ( mProfile->mBitmapBase[0] )
   {
      bool result;
      dSprintf( buf, sizeof( buf ), "%s_vertfield.png", mProfile->mBitmapBase );
      mTexVBar = TextureHandle( buf, BitmapKeepTexture );
      if ( mTexVBar )
      {
         result = createBitmapArray( mTexVBar.getBitmap(), mBarBounds, BarStateCount, 3 );
         AssertFatal( result, "Failed to create vertical bar bitmap array for ShellScrollCtrl!" );
      }

      dSprintf( buf, sizeof( buf ), "%s_horzfield.png", mProfile->mBitmapBase );
      mTexHBar = TextureHandle( buf, BitmapKeepTexture );
      if ( mTexHBar )
      {
         result = createBitmapArray( mTexHBar.getBitmap(), &mBarBounds[BarHLeft * BarStateCount], BarStateCount, 3 );
         AssertFatal( result, "Failed to create horizontal bar bitmap array for ShellScrollCtrl!" );
      }

      dSprintf( buf, sizeof( buf ), "%s_vertbuttons.png", mProfile->mBitmapBase );
      mTexVButtons = TextureHandle( buf, BitmapKeepTexture );
      if ( mTexVButtons )
      {
         result = createBitmapArray( mTexVButtons.getBitmap(), mBitmapBounds, BtnStateCount, 2 );
         AssertFatal( result, "Failed to create vertical button bitmap array for ShellScrollCtrl!" );
      }

      dSprintf( buf, sizeof( buf ), "%s_vertbar.png", mProfile->mBitmapBase );
      mTexVThumb = TextureHandle( buf, BitmapKeepTexture );
      if ( mTexVThumb )
      {
         result = createBitmapArray( mTexVThumb.getBitmap(), &mBitmapBounds[VThumbTop * BtnStateCount], BtnStateCount, 3 );
         AssertFatal( result, "Failed to create vertical thumb bitmap array for ShellScrollCtrl!" );
      }

      dSprintf( buf, sizeof( buf ), "%s_horzbuttons.png", mProfile->mBitmapBase );
      mTexHButtons = TextureHandle( buf, BitmapKeepTexture );
      if ( mTexHButtons )
      {
         result = createBitmapArray( mTexHButtons.getBitmap(), &mBitmapBounds[LeftBtn * BtnStateCount], BtnStateCount, 2 );
         AssertFatal( result, "Failed to create horizontal button bitmap array for ShellScrollCtrl!" );
      }

      dSprintf( buf, sizeof( buf ), "%s_horzbar.png", mProfile->mBitmapBase );
      mTexHThumb = TextureHandle( buf, BitmapKeepTexture );
      if ( mTexHThumb )
      {
         result = createBitmapArray( mTexHThumb.getBitmap(), &mBitmapBounds[HThumbLeft * BtnStateCount], BtnStateCount, 3 );
         AssertFatal( result, "Failed to create horizontal thumb bitmap array for ShellScrollCtrl!" );
      }

      dSprintf( buf, sizeof( buf ), "%s_scale.png", mProfile->mBitmapBase );
      mTexCorner = TextureHandle( buf, BitmapKeepTexture );
      if ( mTexCorner )
      {
         result = createBitmapArray( mTexCorner.getBitmap(), &mBitmapBounds[Corner * BtnStateCount], BtnStateCount, 1 );
         AssertFatal( result, "Failed to create corner bitmap array for ShellScrollCtrl!" );
      }
   }

   if ( mFieldBase[0] )
   {
      dSprintf( buf, sizeof( buf ), "%s_TL.png", mFieldBase );
      mTexLeftTop = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_TM.png", mFieldBase );
      mTexCenterTop = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_TR.png", mFieldBase );
      mTexRightTop = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_ML.png", mFieldBase );
      mTexLeftCenter = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_MM.png", mFieldBase );
      mTexCenter = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_MR.png", mFieldBase );
      mTexRightCenter = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_BL.png", mFieldBase );
      mTexLeftBottom = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_BM.png", mFieldBase );
      mTexCenterBottom = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_BR.png", mFieldBase );
      mTexRightBottom = TextureHandle( buf, BitmapTexture );
   }

   //init
   if ( mTexVBar && mTexVButtons && mTexVThumb )
   {
      mBaseThumbSize = mBitmapBounds[VThumbTop * BtnStateCount].extent.y +
   		   mBitmapBounds[VThumbBottom * BtnStateCount].extent.y - ( 2 * mGlowOffset );
      mScrollBarThickness = mBarBounds[BarVCenter * BarStateCount].extent.x;
      mScrollBarArrowBtnLength = mBitmapBounds[UpBtn * BtnStateCount].extent.y - ( 2 * mGlowOffset );
   }

	// Ensure minimum extents:
   U32 temp;
   bool needResize = false;
   if ( mTexLeftTop && mTexRightTop )
   {
	   temp = mTexLeftTop.getWidth() + mTexRightTop.getWidth() + ( 2 * mBorderThickness );
      if ( mMinExtent.x < temp )
      {
         mMinExtent.x = temp;
         needResize = true;
      }
   }

   if ( mTexLeftTop && mTexLeftBottom )
   {
	   temp = mTexLeftTop.getHeight() + mTexLeftBottom.getHeight() + ( 2 * mBorderThickness );
      if ( mMinExtent.y < temp )
      {
         mMinExtent.y = temp;
         needResize = true;
      }
   }

   if ( needResize )
	   resize( mBounds.point, mBounds.extent );

   computeSizes();

   return true;
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::onSleep()
{
	Parent::onSleep();

   mTexVBar = NULL;
   mTexHBar = NULL;

   mTexVButtons = NULL;
   mTexVThumb = NULL;
   mTexHButtons = NULL;
   mTexHThumb = NULL;
   mTexCorner = NULL;

   mTexLeftTop = NULL;
   mTexCenterTop = NULL;
   mTexRightTop = NULL;
   mTexLeftCenter = NULL;
   mTexCenter = NULL;
   mTexRightCenter = NULL;
   mTexLeftBottom = NULL;
   mTexCenterBottom = NULL;
   mTexRightBottom = NULL;

	mMouseOverRegion = None;
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::onMouseUp( const GuiEvent &event )
{
   Parent::onMouseUp( event );
   onMouseMove( event );
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::onMouseMove( const GuiEvent &event )
{
	Region oldRegion = mMouseOverRegion;
	Point2I curMousePos = globalToLocalCoord( event.mousePoint );
	mMouseOverRegion = findHitRegion( curMousePos );
	
	// Play button over sound:
	if ( mActive && mMouseOverRegion != oldRegion && mProfile->mSoundButtonOver 
		&& ( mMouseOverRegion == UpArrow || mMouseOverRegion == DownArrow || mMouseOverRegion == LeftArrow || mMouseOverRegion == RightArrow ) )
	{
      F32 pan = (F32(event.mousePoint.x)/ F32(Canvas->mBounds.extent.x)*2.0f-1.0f)*0.8f;
      AUDIOHANDLE handle = alxCreateSource(mProfile->mSoundButtonOver);
      alxSourcef(handle, AL_PAN, pan);
      alxPlay(handle);
	}

	setUpdate();
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::onMouseEnter( const GuiEvent &event )
{
	onMouseMove( event );
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::onMouseLeave( const GuiEvent &/*event*/ )
{
	mMouseOverRegion = None;
	setUpdate();
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::drawBorder( const Point2I &offset, bool /*isFirstResponder*/ )
{
   RectI r( offset, mBounds.extent );
	r.point += Point2I( mBorderThickness, mBorderThickness );
	r.extent -= Point2I( ( 2 * mBorderThickness ), ( 2 * mBorderThickness ) );

   // Draw border:
   if ( mProfile->mBorder )
   {
      dglDrawRect( r, mProfile->mBorderColor );
      r.point += Point2I( 1, 1 );
      r.extent -= Point2I( 2, 2 );
   }

	// Temporary hacky focus effect:
	//if ( isFirstResponder )
	//{
	//	dglDrawRect( r, ColorI( 255, 255, 0 ) );
	//	r.point += Point2I( 1, 1 );
	//	r.extent -= Point2I( 2, 2 );
	//}

   // Draw field background:
   if ( mHasVScrollBar )
      r.extent.x -= mScrollBarThickness;
   if ( mHasHScrollBar )
      r.extent.y -= ( mScrollBarThickness + 1 );

	if ( mTexLeftTop && mTexCenterTop && mTexRightTop && mTexLeftCenter && mTexCenter && mTexRightCenter
	  && mTexLeftBottom && mTexCenterBottom && mTexRightBottom )
   {
      dglClearBitmapModulation();
   
      RectI drawRect = r;
      U32 stretchWidth = r.extent.x - mTexLeftTop.getWidth() - mTexRightTop.getWidth();
		U32 topEdgeHeight = mTexLeftTop.getHeight();
		if ( topEdgeHeight > ( r.extent.y - mTexLeftBottom.getHeight() ) )
			topEdgeHeight = r.extent.y - mTexLeftBottom.getHeight();

      // Draw upper left corner:
		drawRect.extent.x = mTexLeftTop.getWidth();
		drawRect.extent.y = topEdgeHeight;
      dglDrawBitmapStretch( mTexLeftTop, drawRect );

      // Draw upper center edge:
      drawRect.point.x += drawRect.extent.x;
      drawRect.extent.x = stretchWidth;
		if ( drawRect.extent.x > 0 )
      	dglDrawBitmapStretch( mTexCenterTop, drawRect );

      // Draw upper right corner:
      drawRect.point.x += drawRect.extent.x;
		drawRect.extent.x = mTexRightTop.getWidth();
      dglDrawBitmapStretch( mTexRightTop, drawRect );

      drawRect.point.x = r.point.x;
      drawRect.point.y += drawRect.extent.y;
      drawRect.extent.y = r.extent.y - drawRect.extent.y - mTexLeftBottom.getHeight();
		if ( drawRect.extent.y > 0 )
		{
         // Draw center left edge:
      	drawRect.extent.x = mTexLeftCenter.getWidth();
      	dglDrawBitmapStretch( mTexLeftCenter, drawRect );

	      // Draw center:
	      drawRect.point.x += drawRect.extent.x;
	      drawRect.extent.x = stretchWidth;
			if ( drawRect.extent.x > 0 )
	      	dglDrawBitmapStretch( mTexCenter, drawRect );

	      // Draw center right edge:
	      drawRect.point.x += drawRect.extent.x;
	      drawRect.extent.x = mTexRightCenter.getWidth();
	      dglDrawBitmapStretch( mTexRightCenter, drawRect );
		}

      // Draw bottom left corner:
      drawRect.point.x = r.point.x;
      drawRect.point.y += drawRect.extent.y;
      dglDrawBitmap( mTexLeftBottom, drawRect.point );

      // Draw bottom center edge:
      drawRect.point.x += mTexLeftBottom.getWidth();
      drawRect.extent.x = stretchWidth;
      drawRect.extent.y = mTexCenterBottom.getHeight();
		if ( drawRect.extent.x > 0 )
      	dglDrawBitmapStretch( mTexCenterBottom, drawRect );

      // Draw bottom right corner:
      drawRect.point.x += drawRect.extent.x;
      dglDrawBitmap( mTexRightBottom, drawRect.point );
   }
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::drawVScrollBar( const Point2I &offset )
{
   if ( !mTexVBar || !mTexVButtons || !mTexVThumb )
      return;

	RectI drawRect;
	drawRect.point = offset + mUpArrowRect.point - Point2I( mGlowOffset, mGlowOffset );
	dglClearBitmapModulation();

	// Draw the up arrow button:
	U32 state = mVBarEnabled ? ( mMouseOverRegion == UpArrow ? ( stateDepressed ? StatePressed : StateRollover ) : StateNormal ) : StateDisabled;
	U32 bitmap = UpBtn * BtnStateCount + state;
	dglDrawBitmapSR( mTexVButtons, drawRect.point, mBitmapBounds[bitmap] );

	// Draw the scroll bar:
	state = mVBarEnabled ? BarNormal : BarDisabled;

	// Draw top edge:
	drawRect.point += Point2I( mGlowOffset, mGlowOffset );
	drawRect.point.y += mScrollBarArrowBtnLength;
	bitmap = BarVTop * BarStateCount + state;
	dglDrawBitmapSR( mTexVBar, drawRect.point, mBarBounds[bitmap] );

	// Draw the center section:
	drawRect.point.y += mBarBounds[bitmap].extent.y;
	drawRect.extent.x = mScrollBarThickness;
	drawRect.extent.y = mDownArrowRect.point.y - mScrollBarArrowBtnLength - mBarBounds[bitmap].extent.y - mBarBounds[BarStateCount * BarVBottom].extent.y; 
	if ( drawRect.extent.y > 0 )
   {
	   bitmap = BarVCenter * BarStateCount + state;
		dglDrawBitmapStretchSR( mTexVBar, drawRect, mBarBounds[bitmap] );
   }

	// Draw the bottom edge:
	drawRect.point.y += drawRect.extent.y;
	bitmap = BarVBottom * BarStateCount + state;
	dglDrawBitmapSR( mTexVBar, drawRect.point, mBarBounds[bitmap] );

	// Draw the thumb (if enabled):
	if ( mVBarEnabled )
	{
		drawRect.point.y = offset.y + mVThumbPos;
		drawRect.point -= Point2I( mGlowOffset, mGlowOffset );
		state = ( mMouseOverRegion == VertThumb ) ? ( stateDepressed ? StatePressed : StateRollover ) : StateNormal;
	
		// Draw top cap:
		bitmap = VThumbTop * BtnStateCount + state;
		dglDrawBitmapSR( mTexVThumb, drawRect.point, mBitmapBounds[bitmap] );

		// Draw center section:
		drawRect.point.y += mBitmapBounds[bitmap].extent.y;
		drawRect.extent.y = mVThumbSize - mBitmapBounds[bitmap].extent.y - mBitmapBounds[VThumbBottom * BtnStateCount].extent.y + ( 2 * mGlowOffset );
		if ( drawRect.extent.y > 0 )
      {
		   bitmap = VThumb * BtnStateCount + state;
		   drawRect.extent.x = mBitmapBounds[bitmap].extent.x;
			dglDrawBitmapStretchSR( mTexVThumb, drawRect, mBitmapBounds[bitmap] );
      }

		// Draw the bottom cap:
		drawRect.point.y += drawRect.extent.y;
		bitmap = VThumbBottom * BtnStateCount + state;
		dglDrawBitmapSR( mTexVThumb, drawRect.point, mBitmapBounds[bitmap] );
	}

	// Draw the down button:
	drawRect.point = offset + mDownArrowRect.point - Point2I( mGlowOffset, mGlowOffset );
	state = mVBarEnabled ? ( mMouseOverRegion == DownArrow ? ( stateDepressed ? StatePressed : StateRollover ) : StateNormal ) : StateDisabled;
	bitmap = DownBtn * BtnStateCount + state;
	dglDrawBitmapSR( mTexVButtons, drawRect.point, mBitmapBounds[bitmap] );
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::drawHScrollBar( const Point2I &offset )
{
   if ( !mTexHBar || !mTexHButtons || !mTexHThumb )
      return;

	RectI drawRect;
	drawRect.point = offset + mLeftArrowRect.point - Point2I( mGlowOffset, mGlowOffset );
	dglClearBitmapModulation();

	// Draw the left arrow button:
	U32 state = mHBarEnabled ? ( mMouseOverRegion == LeftArrow ? ( stateDepressed ? StatePressed : StateRollover ) : StateNormal ) : StateDisabled;
	U32 bitmap = LeftBtn * BtnStateCount + state;
	dglDrawBitmapSR( mTexHButtons, drawRect.point, mBitmapBounds[bitmap] );

	// Draw the scroll bar:
	state = mHBarEnabled ? BarNormal : BarDisabled;

	// Draw left edge:
	drawRect.point += Point2I( mScrollBarArrowBtnLength + mGlowOffset, mGlowOffset );
	bitmap = BarHLeft * BarStateCount + state;
	dglDrawBitmapSR( mTexHBar, drawRect.point, mBarBounds[bitmap] );

	// Draw the center section:
	drawRect.point.x += mBarBounds[bitmap].extent.x;
	drawRect.extent.x = mRightArrowRect.point.x - mScrollBarArrowBtnLength - mBarBounds[bitmap].extent.x - mBarBounds[BarHRight * BarStateCount].extent.x; 
	if ( drawRect.extent.x > 0 )
   {
	   drawRect.extent.y = mScrollBarThickness;
	   bitmap = BarHCenter * BarStateCount + state;
		dglDrawBitmapStretchSR( mTexHBar, drawRect, mBarBounds[bitmap] );
   }

	// Draw the right edge:
	drawRect.point.x += drawRect.extent.x;
	bitmap = BarHRight * BarStateCount + state;
	dglDrawBitmapSR( mTexHBar, drawRect.point, mBarBounds[bitmap] );

	// Draw the thumb (if enabled):
	if ( mHBarEnabled )
	{
		drawRect.point.x = offset.x + mHThumbPos;
		drawRect.point -= Point2I( mGlowOffset, mGlowOffset );
		state = ( mMouseOverRegion == HorizThumb ) ? ( stateDepressed ? StatePressed : StateRollover ) : StateNormal;
	
		// Draw left cap:
		bitmap = HThumbLeft * BtnStateCount + state;
		dglDrawBitmapSR( mTexHThumb, drawRect.point, mBitmapBounds[bitmap] );

		// Draw center section:
		drawRect.point.x += mBitmapBounds[bitmap].extent.x;
		drawRect.extent.x = mHThumbSize - mBitmapBounds[bitmap].extent.x - mBitmapBounds[HThumbRight * BtnStateCount].extent.x + ( 2 * mGlowOffset );
		if ( drawRect.extent.x > 0 )
      {
		   bitmap = HThumb * BtnStateCount + state;
		   drawRect.extent.y = mBitmapBounds[bitmap].extent.y;
			dglDrawBitmapStretchSR( mTexHThumb, drawRect, mBitmapBounds[bitmap] );
      }

		// Draw the right cap:
		drawRect.point.x += drawRect.extent.x;
		bitmap = HThumbRight * BtnStateCount + state;
		dglDrawBitmapSR( mTexHThumb, drawRect.point, mBitmapBounds[bitmap] );
	}

	// Draw the right button:
	drawRect.point = offset + mRightArrowRect.point - Point2I( mGlowOffset, mGlowOffset );
	state = mHBarEnabled ? ( mMouseOverRegion == RightArrow ? ( stateDepressed ? StatePressed : StateRollover ) : StateNormal ) : StateDisabled;
	bitmap = RightBtn * BtnStateCount + state;
	dglDrawBitmapSR( mTexHButtons, drawRect.point, mBitmapBounds[bitmap] );
}

//------------------------------------------------------------------------------
void ShellScrollCtrl::drawScrollCorner( const Point2I &offset )
{
   if ( mTexCorner )
   {
	   Point2I drawPos = offset + mRightArrowRect.point - Point2I( mGlowOffset, mGlowOffset );
	   drawPos.x += mRightArrowRect.extent.x;
	   dglClearBitmapModulation();
	   dglDrawBitmapSR( mTexCorner, drawPos, mBitmapBounds[Corner * BtnStateCount + StateDisabled] );
   }
}
