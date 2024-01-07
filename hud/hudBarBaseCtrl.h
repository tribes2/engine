//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudCtrl.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"

//=========================================================================== 
// CLASS: HudBarBaseCtrl
//
// This HUD object is a horizontal percentile bar
//=========================================================================== 

class HudBarBaseCtrl : public HudCtrl {
	private:
		typedef HudCtrl Parent;
		
		RectI	mSubRegion;

	protected:
      	//------------------------------------------------------------------
      	// getValue
      	//
      	// Default method that is intended to be over-ridden, returns
      	// the current value of the bar in a percentile format
      	//
      	// Return: Value of the bar as a percentile
      	//------------------------------------------------------------------
		virtual F32 getValue() { 
			return 0.0f; 
		}
		
		//------------------------------------------------------------------
		// getBarColor
		// 
		// This method stores the fill color of the bar in a provided variable
		//
		// col - This color variable will be set to the current fill color of the bar
		//------------------------------------------------------------------
		virtual void getBarColor( ColorI &col ) { 
			col.set( mFillColor.red * 255, mFillColor.green * 255, mFillColor.blue * 255 ); 
		}
		
	public:
      
		HudBarBaseCtrl();
         
		// GuiControl
		bool onWake();
		void onSleep();
		void onRender( Point2I, const RectI &, GuiControl * );
      
		S32                              mPulseRate;
		F32                              mPulseThreshold;
		bool                             mVerticalFill;
		bool                             mPulse;
		bool                             mDisplayMounted;
      
		static void initPersistFields();

		DECLARE_CONOBJECT(HudBarBaseCtrl);
};
