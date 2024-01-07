//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBitmapFrameCtrl.h"
#include "core/dnet.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"
#include "game/camera.h"

//--------------------------------------------------------------------------- 
// Class: HudBarBaseCtrl
//--------------------------------------------------------------------------- 

class HudBarBaseCtrl : public HudBitmapFrameCtrl
{
   private:
      typedef HudBitmapFrameCtrl       Parent;

   protected:
      
      virtual F32 getValue() { return( 0.0f ); }
      virtual void getBarColor(ColorI &col){ col.set( mFillColor.red * 255, mFillColor.green * 255, mFillColor.blue * 255); }
   public:
      
      HudBarBaseCtrl();
         
      // GuiControl
      bool onWake();
      void onSleep();
      void onRender(Point2I, const RectI &, GuiControl *);
      
      S32                              mPulseRate;
      F32                              mPulseThreshold;
      bool                             mVerticalFill;
      bool                             mPulse;
      bool                             mDisplayMounted;
      
      static void initPersistFields();

      DECLARE_CONOBJECT(HudBarBaseCtrl);
};

IMPLEMENT_CONOBJECT(HudBarBaseCtrl);

//--------------------------------------------------------------------------- 

HudBarBaseCtrl::HudBarBaseCtrl()
{
   mPulseRate = 500;
   mPulseThreshold = 0.3f;
   mVerticalFill = false;
   mPulse = false;
   mDisplayMounted = false;
}

bool HudBarBaseCtrl::onWake()
{
   if(!Parent::onWake())
      return(false);
   return(true);
}

void HudBarBaseCtrl::onSleep()
{
   Parent::onSleep();
}

//--------------------------------------------------------------------------- 

void HudBarBaseCtrl::onRender(Point2I offset, const RectI & /*updateRect*/, GuiControl * /*firstResponder*/)
{
   GameConnection * con = GameConnection::getServerConnection();
   if(!con)
      return;

   ShapeBase * obj = con->getControlObject();
   if(!obj)
      return;

   F32 val = getValue();

   RectI rect(offset + mSubRegion.point, mSubRegion.extent);
   rect.extent.x = (S32)(rect.extent.x * val);

   mFillColor.alpha = mOpacity;
   ColorI fill;
   getBarColor( fill );
   fill.alpha = mOpacity * 255;
   
   if(mPulse && (mPulseRate != 0))
   {
      S32 time = (S32)Platform::getVirtualMilliseconds();
      F32 alpha = F32(time % mPulseRate) / F32(mPulseRate/2);
   
      if(alpha > 1)
         alpha = 1.f - (alpha - 1.f);

      fill.alpha *= alpha;
      fill.alpha *= 255;
   }
   
   dglDrawRectFill(rect, fill);
}

//--------------------------------------------------------------------------- 

void HudBarBaseCtrl::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("displayMounted", TypeBool, Offset(mDisplayMounted, HudBarBaseCtrl));
   addField("pulseRate", TypeS32, Offset(mPulseRate, HudBarBaseCtrl));
   addField("pulseThreshold", TypeF32, Offset(mPulseThreshold, HudBarBaseCtrl));
   addField("verticalFill", TypeBool, Offset(mVerticalFill, HudBarBaseCtrl));
}

//--------------------------------------------------------------------------- 
// Class: HudEnergy
//--------------------------------------------------------------------------- 

class HudEnergyCtrl : public HudBarBaseCtrl
{
   private:
      typedef HudBarBaseCtrl  Parent;

   public:
      
      F32 getValue();

      DECLARE_CONOBJECT(HudEnergyCtrl);
};

IMPLEMENT_CONOBJECT(HudEnergyCtrl);

F32 HudEnergyCtrl::getValue()
{
   GameConnection * con = GameConnection::getServerConnection();
   if(!con)
      return(0.f);

   ShapeBase * obj = con->getControlObject();
   if(!obj)
      return(0.f);

   if(mDisplayMounted)
   {
      ShapeBase * mount = obj->getObjectMount();
      while(mount && mount->isMounted())
         mount = mount->getObjectMount();

      if(mount)
         return(mClampF(mount->getEnergyValue(), 0.f, 1.f));
      else
         return(0.f);
   }
   else
      return(mClampF(obj->getEnergyValue(), 0.f, 1.f));
}

//--------------------------------------------------------------------------- 
// Class: HudDamage
//--------------------------------------------------------------------------- 

class HudDamageCtrl : public HudBarBaseCtrl
{
   private:
      typedef HudBarBaseCtrl  Parent;

   public:

      F32 getValue();
      void getBarColor( ColorI &col );

      DECLARE_CONOBJECT(HudDamageCtrl);
};

IMPLEMENT_CONOBJECT(HudDamageCtrl);

void HudDamageCtrl::getBarColor( ColorI &col )
{
   F32 val = getValue();
   
   if( val < 0.6 && val > 0.25)
      col.set( 255, 178, 0 );
   else if( val <= 0.25 )
      col.set( 255, 0, 0 );
   else  
      col.set( mFillColor.red * 255, mFillColor.green * 255, mFillColor.blue * 255);}

F32 HudDamageCtrl::getValue()
{
   GameConnection * con = GameConnection::getServerConnection();
   
   if(!con)
      return(0.f);

   ShapeBase * obj = con->getControlObject();
   if( !obj )
      return (0.f);
   
   mPulse = false;
   
   if(dynamic_cast<Camera*>(con->getControlObject()))
      return(0.f);
   
   F32 damage = 0.0f;

   if(mDisplayMounted)
   {
      ShapeBase * mount = obj->getObjectMount();
      while(mount && mount->isMounted())
         mount = mount->getObjectMount();
         
      if(mount)
         damage = mClampF(1.f - mount->getDamageValue(), 0.f, 1.f);
      else
         damage = 0.0f;
   }
   else
      damage = mClampF(1.f - obj->getDamageValue(), 0.f, 1.f);    
   
   
   if( damage < 0.25 )
      mPulse = true;
   
   return(damage);
}

//--------------------------------------------------------------------------- 
// Class: HudHeat
//--------------------------------------------------------------------------- 

class HudHeat : public HudBarBaseCtrl
{
   private:
      typedef HudBarBaseCtrl  Parent;

   public:

      F32 getValue();
      void onRender(Point2I, const RectI &, GuiControl *);

      DECLARE_CONOBJECT(HudHeat);
      
      static void initPersistFields();
      HudHeat();
      F32 mHeatWarning;    
};

IMPLEMENT_CONOBJECT(HudHeat);

HudHeat::HudHeat()
{
   mHeatWarning = 0.5;
}

F32 HudHeat::getValue()
{
   GameConnection * con = GameConnection::getServerConnection();
   if(!con)
      return 0.f;

   ShapeBase *obj = con->getControlObject();
   if(!obj)
      return 0.f;
      
   if( dynamic_cast<Camera*>(obj) )
      return(0.f);
   
   F32 heat = obj->getHeat();
   
   if( heat > mHeatWarning )
      mPulse = true;
   else  
      mPulse = false;   
   
   return(mClampF( heat, 0.f, 1.f));
}

void HudHeat::onRender( Point2I offset, const RectI &rect, GuiControl *ctrl )
{
   Parent::onRender( offset, rect, ctrl );
   
   // need to draw a little hash here
}

void HudHeat::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("heatWarning", TypeF32, Offset(mHeatWarning, HudHeat));
}
