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
// CLASS: HudHeat
//===========================================================================  

class HudHeat : public HudBarBaseCtrl {
	private:
		typedef HudBarBaseCtrl Parent;

	public:

		F32 getValue();

		DECLARE_CONOBJECT( HudHeat );
      
		static void initPersistFields();
		HudHeat();
		F32 mHeatWarning;    
};

//=========================================================================== 
// HudHeatImplementation
//=========================================================================== 
IMPLEMENT_CONOBJECT( HudHeat );

//------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------
HudHeat::HudHeat() {
	mHeatWarning = 0.5;
}

//------------------------------------------------------------------
// getValue
//
// This method is overridden from hudBarBaseCtrl, it returns the 
// heat level of the player as a percentile
//
// Return: Heat level of the player as a percentile
//------------------------------------------------------------------
F32 HudHeat::getValue() {
	GameConnection *con = GameConnection::getServerConnection();
	
	if(!con)
		return 0.f;

	ShapeBase *obj = con->getControlObject();
	
	if( !obj )
		return 0.f;
	
    // Make sure this isn't a free camera
	if( dynamic_cast<Camera*>(obj) )
		return 0.f;
   
	F32 heat = obj->getHeat();
   
	if( heat > mHeatWarning )
		mPulse = true;
	else  
		mPulse = false;   
   
   return mClampF( heat, 0.f, 1.f );
}

//------------------------------------------------------------------
// initPersistFields
//
// Registers datamembers of this object with the game
//------------------------------------------------------------------
void HudHeat::initPersistFields() {
	Parent::initPersistFields();
   
	addField( "heatWarning", TypeF32, Offset( mHeatWarning, HudHeat ) );
}