//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hudBarBaseCtrl.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"

//===========================================================================  
// CLASS: HudEnergyCtrl
//
// This object displays the players energy level in a bar format
//=========================================================================== 

class HudEnergyCtrl : public HudBarBaseCtrl {
	private:
		typedef HudBarBaseCtrl  Parent;

	public:
      
		F32 getValue();

	DECLARE_CONOBJECT( HudEnergyCtrl );
};

//=========================================================================== 
// HudEnergyCtrl Implementation
//=========================================================================== 
IMPLEMENT_CONOBJECT( HudEnergyCtrl );

//------------------------------------------------------------------
// getValue
//
// This method is overridden from hudBarBaseCtrl, it returns the 
// energy level of the player as a percentile
//
// Return: Energy level of the player as a percentile
//------------------------------------------------------------------
F32 HudEnergyCtrl::getValue() {
	GameConnection *con = GameConnection::getServerConnection();
	
	if( !con )
		return(0.f);

	ShapeBase *obj = con->getControlObject();
	if( !obj )
		return(0.f);

	if( mDisplayMounted ) {
		ShapeBase *mount = obj->getObjectMount();
		
		while( mount && mount->isMounted() )
			mount = mount->getObjectMount();

		if( mount )
			return( mClampF( mount->getEnergyValue(), 0.f, 1.f ) );
		else
			return( 0.f );
	}
	else
		return mClampF( obj->getEnergyValue(), 0.f, 1.f );
}

// hudEnergy.cc