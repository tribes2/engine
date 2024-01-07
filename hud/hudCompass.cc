//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudCtrl.h"
#include "dgl/dgl.h"
#include "gui/guiTSControl.h"
#include "console/consoleTypes.h"
#include "game/gameConnection.h"
#include "game/camera.h"
#include "dgl/gTexManager.h"

//--------------------------------------------------------------------------- 
// Class: HudCompassCtrl
//--------------------------------------------------------------------------- 

class HudCompassCtrl : public HudCtrl
{
   private:

      typedef HudCtrl         Parent;

   public:

      HudCompassCtrl();
      ~HudCompassCtrl();
         
      // SimBase
      bool onAdd();

      // GuiControl
      void onPreRender();
      void onRender(Point2I, const RectI &, GuiControl *);
      bool onWake();
      void onSleep();
      
      ColorI mTextColor;

      static void initPersistFields();

      DECLARE_CONOBJECT(HudCompassCtrl);
};

IMPLEMENT_CONOBJECT(HudCompassCtrl);

HudCompassCtrl::HudCompassCtrl()
{
   mTextColor.set( 0, 255, 0 );
}

HudCompassCtrl::~HudCompassCtrl()
{
}

//--------------------------------------------------------------------------- 

bool HudCompassCtrl::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   return(true);
}

//--------------------------------------------------------------------------- 

bool HudCompassCtrl::onWake()
{
   if(!Parent::onWake())
      return(false);

   return(true);
}

void HudCompassCtrl::onSleep()
{
   Parent::onSleep();

}

//--------------------------------------------------------------------------- 

void HudCompassCtrl::onPreRender()
{
   GuiControl * parent = getParent();
   if(!parent)
      return;

   setUpdate();
}

static float gCompassPoints[] =
{
   //'N'
   4.0f,     -2.0f, -28.0f,     -2.0f, -34.0f,      2.0f, -28.0f,     2.0f, -34.0f,
   
   //'E'
   4.0f,     28.0f,   2.0f,     28.0f,  -2.0f,     34.0f,  -2.0f,    34.0f,   2.0f,
   2.0f,     31.0f,  -2.0f,     31.0f,   2.0f,
   
   //'W'
   5.0f,    -34.0f,   4.0f,    -28.0f,  2.0f,     -34.0f,   0.0f,    -28.0f, -2.0f,   -34.0f, -4.0f,
   
   //'S'
   12.0f,     2.0f,  30.0f,      1.0f,  29.0f,     -1.0f,  29.0f,     -2.0f, 30.0f,    -2.0f, 31.0f,     -1.0f, 32.0f,
              1.0f,  32.0f,      2.0f,  33.0f,      2.0f,  34.0f,      1.0f, 35.0f,    -1.0f, 35.0f,     -2.0f, 34.0f,
   
   //4 marks
   2.0f,     25.0f, -24.0f,     29.0f, -28.0f,
   2.0f,     25.0f,  24.0f,     29.0f,  28.0f,
   2.0f,    -24.0f,  24.0f,    -28.0f,  28.0f,
   2.0f,    -24.0f, -24.0f,    -28.0f, -28.0f,
      
   //EOF
   -1.0f
};

void HudCompassCtrl::onRender(Point2I offset, const RectI &, GuiControl *)
{
   GameConnection * con = GameConnection::getServerConnection();
   if( !con )
      return;

   ShapeBase *obj = con->getControlObject();
   
   if( !obj )
      return;

   Point2F centerPt;
   centerPt.x = 40.5f + F32( offset.x );
   centerPt.y = 38.5f + F32( offset.y );
   
   MatrixF camMat;
   F32 camPos = 0;
   Point3F camVec;
   obj->getCameraTransform(&camPos, &camMat);

   camMat.getColumn( 1, &camVec );
   camVec.z = 0.f;
   camVec.normalize();

   F32 camAngle;
   F32 dot = mDot( camVec, Point3F( 0, 1, 0 ) );
   camAngle = mAcos( dot );
   
   if(camVec.x > 0.f)
      camAngle = M_2PI - camAngle;
      
   //rotate the needle around the compass
   F32 cosTheta, sinTheta;
   mSinCos( camAngle, sinTheta, cosTheta);

   //loop through and rotate the compass markings
   Point2F markPoints[12];
   F32 *points;
   points = &gCompassPoints[0];
   
   while( *points > 0.0f )
   {
      S32 j;
      for( j = 0; j < *points; j++ )
      {
         markPoints[j].x = (points[2 * j + 1] * cosTheta) - (points[2 * j + 2] * sinTheta);
         markPoints[j].y = (points[2 * j + 1] * sinTheta) + (points[2 * j + 2] * cosTheta);
      }
      
      //now draw the lines
      for( j = 0; j < *points - 1; j++ )
      {
         Point2I tempStart;
         tempStart.x = S32( markPoints[j].x + centerPt.x );
         tempStart.y = S32( markPoints[j].y + centerPt.y );
         Point2I tempEnd;
         tempEnd.x = S32( markPoints[j + 1].x + centerPt.x );
         tempEnd.y = S32( markPoints[j + 1].y + centerPt.y );
         dglDrawLine( tempStart, tempEnd, ColorI(mTextColor.red, mTextColor.green, mTextColor.blue ) );
      }
      
      //now increment the points pointer
      points += ( S32( *points ) * 2 ) + 1;
   }
}

//--------------------------------------------------------------------------- 

void HudCompassCtrl::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("textColor", TypeColorI, Offset(mTextColor, HudCompassCtrl));
}
