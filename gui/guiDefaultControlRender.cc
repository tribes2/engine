//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "gui/guiDefaultControlRender.h"
#include "core/color.h"
#include "math/mRect.h"

static ColorI colorLightGray(192, 192, 192);
static ColorI colorGray(128, 128, 128);
static ColorI colorDarkGray(64, 64, 64);
static ColorI colorWhite(255,255,255);
static ColorI colorBlack(0,0,0);

void renderRaisedBox(RectI &bounds)
{
   S32 l = bounds.point.x, r = bounds.point.x + bounds.extent.x - 1;
	S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y - 1;

	dglDrawRectFill( bounds, colorLightGray);
   dglDrawLine(l, t, l, b - 1, colorWhite);
   dglDrawLine(l, t, r - 1, t, colorWhite);

   dglDrawLine(l, b, r, b, colorBlack);
   dglDrawLine(r, b - 1, r, t, colorBlack);

	dglDrawLine(l + 1, b - 1, r - 1, b - 1, colorGray);
   dglDrawLine(r - 1, b - 2, r - 1, t + 1, colorGray);
}  

 
void renderLoweredBox(RectI &bounds)
{
   S32 l = bounds.point.x, r = bounds.point.x + bounds.extent.x - 1;
	S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y - 1;

	dglDrawRectFill( bounds, colorLightGray);

	dglDrawLine(l, b, r, b, colorWhite);
   dglDrawLine(r, b - 1, r, t, colorWhite);

   dglDrawLine(l, t, r - 1, t, colorBlack);
   dglDrawLine(l, t + 1, l, b - 1, colorBlack);

	dglDrawLine(l + 1, t + 1, r - 2, t + 1, colorGray);
   dglDrawLine(l + 1, t + 2, l + 1, b - 2, colorGray);
}

