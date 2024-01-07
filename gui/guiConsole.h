//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICONSOLE_H_
#define _GUICONSOLE_H_

#ifndef _GUIARRAYCTRL_H_
#include "GUI/guiArrayCtrl.h"
#endif

class GuiConsole : public GuiArrayCtrl
{
	private:
		typedef GuiArrayCtrl Parent;

		Resource<GFont> mFont;

		S32 getMaxWidth(S32 startIndex, S32 endIndex);

	public:
		GuiConsole();
		DECLARE_CONOBJECT(GuiConsole);

		bool onWake();

		void onPreRender();
		void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);
};

#endif
