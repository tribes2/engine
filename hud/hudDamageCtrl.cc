//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hudBarBaseCtrl.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"
#include "game/camera.h"

//===========================================================================  
// CLASS: HudDamageCtrl
//===========================================================================  

class HudDamageCtrl : public HudBarBaseCtrl {
	private:
		typedef HudBarBaseCtrl  Parent;

	public:

		F32 getValue();
		void getBarColor( ColorI &col );

		DECLARE_CONOBJECT( HudDamageCtrl );
};

//=========================================================================== 
// HudDamageCtrl Implementation
//=========================================================================== 
IMPLEMENT_CONOBJECT( HudDamageCtrl );

//------------------------------------------------------------------
// getBarColor
//
// This method ovrrides the virtual method in hudBarBaseCtrl
// the bar changes color depending on how much damage is taken
//------------------------------------------------------------------
void HudDamageCtrl::getBarColor( ColorI &col ) {
	F32 val = getValue();
   
	if( val < 0.6 && val > 0.25)
		col.set( 255, 178, 0 );
	else if( val <= 0.25 )
		col.set( 255, 0, 0 );
	else  
		col.set( mFillColor.red * 255, mFillColor.green * 255, mFillColor.blue * 255);
}

//------------------------------------------------------------------
// getValue
//
// This method is overridden from hudBarBaseCtrl, it returns the 
// damage level of the player as a percentile
//
// Return: Damage level of the player as a percentile
//------------------------------------------------------------------
F32 HudDamageCtrl::getValue() {
	
	GameConnection *con = GameConnection::getServerConnection();
	
   	// Sanity check connection to game.
	if( !con )
		return( 0.f );

	ShapeBase *obj = con->getControlObject();
	
	if( !obj )
		return (0.f);
   
	mPulse = false;
   
   	// If we're in a floating camera view...
	if( dynamic_cast<Camera*>( con->getControlObject() ) )
		return( 0.f );
   
   	// Nope, we're either on a vehicle or running around
	F32 damage = 0.0f;

	if( mDisplayMounted ) {
		ShapeBase * mount = obj->getObjectMount();
		
		while( mount && mount->isMounted() )
			mount = mount->getObjectMount();
         
		if( mount )
			damage = mClampF( 1.f - mount->getDamageValue(), 0.f, 1.f );
		else
			damage = 0.0f;
	}
	else
		damage = mClampF( 1.f - obj->getDamageValue(), 0.f, 1.f );    
   
   
	if( damage < 0.25 )
		mPulse = true;
   
	return damage ;
}

// hudDamage.cc
