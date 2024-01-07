//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBarDisplayCtrl.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"
#include "game/camera.h"

/**
 * This is a health display bar graph
 */
class HudHealthCtrl : public HudBarDisplayCtrl {
   private:
      typedef HudBarDisplayCtrl  Parent;

   public:
      HudHealthCtrl();
      
      F32 getValue();
      ColorI getBarColor();

      DECLARE_CONOBJECT( HudHealthCtrl );
};

IMPLEMENT_CONOBJECT( HudHealthCtrl );

/**
 * Constructor
 */
HudHealthCtrl::HudHealthCtrl() {
   mHorizontalBar = false;
   mDrawFromOrigin = false;
}

/**
 * This method ovrrides the virtual method in hudBarBaseCtrl
 * the bar changes color depending on how much damage is taken
 *
 * @return ColorI that has the bar color stored in it
 */
ColorI HudHealthCtrl::getBarColor() {
   F32 val = getValue();
   
   if( val < 0.6 && val > 0.25)
      return ColorI( 255, 178, 0 );
   else if( val <= 0.25 )
      return ColorI( 255, 0, 0 );
   else  
      return ColorI( 0, 210, 210 );
}

/**
 * This method is overridden from hudBarBaseCtrl, it returns the 
 * damage level of the player as a percentile
 *
 * @return Damage level of the player as a percentile
 */
F32 HudHealthCtrl::getValue() {
   
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
   
   return damage;
}

// hudDamage.cc
