//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hudBarBaseCtrl.h"

//=========================================================================== 
// HudBarBaseCtrl Implementation
//=========================================================================== 

IMPLEMENT_CONOBJECT( HudBarBaseCtrl );

//--------------------------------------------------------------------------- 
// Constructor
//---------------------------------------------------------------------------
HudBarBaseCtrl::HudBarBaseCtrl() {
	mPulseRate = 500;
	mPulseThreshold = 0.3f;
	mVerticalFill = false;
	mPulse = false;
	mDisplayMounted = false;
}

//---------------------------------------------------------------------------
// onWake
// 
// This method is called when this HUD object is revived from a waiting state
//
// Return: True if successfuly awakened
//---------------------------------------------------------------------------
bool HudBarBaseCtrl::onWake() {
	if( !Parent::onWake() )
		return( false );
	return( true );
}

//---------------------------------------------------------------------------
// onSleep
//
// This method is used to put this HUD object in a waiting state
//---------------------------------------------------------------------------
void HudBarBaseCtrl::onSleep() {
	Parent::onSleep();
}

//--------------------------------------------------------------------------- 
// onRender
//
// This method is used to tell the bar to render itself
//
// offset - A offset value for where to render the bar
//---------------------------------------------------------------------------
void HudBarBaseCtrl::onRender( Point2I offset, const RectI & /*updateRect*/, GuiControl * /*firstResponder*/ ) {

	// Why is this code here?
	/*
	GameConnection *con = GameConnection::getServerConnection();
	if(!con)
		return;

	ShapeBase * obj = con->getControlObject();

	if( !obj )
		return;
	*/
	
	F32 val = getValue();

	RectI rect( offset + mSubRegion.point, mSubRegion.extent );
	rect.extent.x = (S32)( rect.extent.x * val );

	mFillColor.alpha = mOpacity;
	ColorI fill;
	getBarColor( fill );
	fill.alpha = mOpacity * 255;
   
	if( mPulse && ( mPulseRate != 0 ) ) {
		S32 time = (S32)Platform::getVirtualMilliseconds();
		F32 alpha = F32( time % mPulseRate ) / F32( mPulseRate / 2 );
   
		if( alpha > 1 )
			alpha = 1.f - ( alpha - 1.f );

		fill.alpha *= alpha;
		fill.alpha *= 255;
	}
   
	dglDrawRectFill( rect, fill );
}

//--------------------------------------------------------------------------- 
// initPersistFields
//
// Registers datamembers of this object with the game
//--------------------------------------------------------------------------- 
void HudBarBaseCtrl::initPersistFields() {
	Parent::initPersistFields();
   
	addField( "displayMounted", TypeBool, Offset( mDisplayMounted, HudBarBaseCtrl ) );
	addField( "pulseRate", TypeS32, Offset( mPulseRate, HudBarBaseCtrl ) );
	addField( "pulseThreshold", TypeF32, Offset( mPulseThreshold, HudBarBaseCtrl ) );
	addField( "verticalFill", TypeBool, Offset( mVerticalFill, HudBarBaseCtrl ) );
	addField( "subRegion", TypeRectI, Offset( mSubRegion, HudBarBaseCtrl ) );
}

// hudBarBaseCtrl.cc