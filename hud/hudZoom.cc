//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudBitmapCtrl.h"
#include "dgl/dgl.h"

class HudZoom : public HudBitmapCtrl
{
   typedef HudBitmapCtrl Parent;
public:
   void getPoints(Point2I *point, Point2I offset);
   HudZoom();
   ~HudZoom();
   DECLARE_CONOBJECT(HudZoom);
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void findLastColor(char *line, Vector<U32> lineStarts, ColorI *useColor);
};

IMPLEMENT_CONOBJECT(HudZoom);

HudZoom::HudZoom()
{
}

HudZoom::~HudZoom()
{
}

void HudZoom::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   if(mBitmapHandle)
      Parent::onRender(offset, updateRect, firstResponder);
   else
   {
      S32 x;
      Point2I point[12];
      getPoints(point, offset);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);

      glColor4ub(mProfile->mBorderColor.red, mProfile->mBorderColor.green, mProfile->mBorderColor.blue, mProfile->mBorderColor.alpha);

      for(x=0;x<4; ++x)
      {
         glBegin(GL_LINE_STRIP);
               glVertex2i(point[x*3  ].x, point[x*3  ].y);
               glVertex2i(point[x*3+1].x, point[x*3+1].y);
               glVertex2i(point[x*3+2].x, point[x*3+2].y);
         glEnd();               
      }
      glDisable(GL_BLEND);
   }
}

void HudZoom::getPoints(Point2I *point, Point2I offset)
{
   Point2I cPoint[4];
   cPoint[0] = Point2I(offset.x+1, offset.y+1);
   cPoint[1] = Point2I(offset.x-1 + mBounds.extent.x, offset.y+1);
   cPoint[2] = Point2I(offset.x-1 + mBounds.extent.x, offset.y-1 + mBounds.extent.y);
   cPoint[3] = Point2I(offset.x+1, offset.y-1 + mBounds.extent.y);

   point[0]  = Point2I(cPoint[0].x, cPoint[0].y + mBounds.extent.y/4);
   point[1]  = Point2I(cPoint[0].x, cPoint[0].y);
   point[2]  = Point2I(cPoint[0].x + mBounds.extent.x/4, cPoint[0].y);

   point[3]  = Point2I(cPoint[1].x - mBounds.extent.x/4, cPoint[1].y);
   point[4]  = Point2I(cPoint[1].x, cPoint[1].y);
   point[5]  = Point2I(cPoint[1].x, cPoint[1].y + mBounds.extent.y/4);

   point[6]  = Point2I(cPoint[2].x, cPoint[2].y - mBounds.extent.y/4);
   point[7]  = Point2I(cPoint[2].x, cPoint[2].y);
   point[8]  = Point2I(cPoint[2].x - mBounds.extent.x/4, cPoint[2].y);

   point[9]  = Point2I(cPoint[3].x +mBounds.extent.x/4, cPoint[3].y);
   point[10] = Point2I(cPoint[3].x, cPoint[3].y);
   point[11] = Point2I(cPoint[3].x, cPoint[3].y - mBounds.extent.y/4);
}