//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _DEBUGVIEW_H_
#define _DEBUGVIEW_H_

#ifndef _GUITEXTCTRL_H_
#include "gui/guiTextCtrl.h"
#endif

class DebugView : public GuiTextCtrl
{
	private:
		typedef GuiTextCtrl Parent;

		enum
		{
			MaxTextLines = 64,
			MaxTextLineLength = 255
		};

		//text members
		char mTextLines[MaxTextLines][MaxTextLineLength + 1];
		ColorF mTextColors[MaxTextLines];

		struct DebugLine
		{
			Point3F start;
			Point3F end;
			ColorF color;
			DebugLine(const Point3F &inStart, const Point3F &inEnd, const ColorI &inColor)
			{
				color = inColor;
				start = inStart;
				end = inEnd;
			}
		};
		Vector<DebugLine> mLines;

	public:
	   DECLARE_CONOBJECT(DebugView);
	   DebugView();
		static void consoleInit();

		void addLine(const Point3F &start, const Point3F &end, const ColorF &color);
		void clearLines();

		void setTextLine(int line, const char *text, ColorF *color);
		void clearTextLine(int line = -1);

		void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);

};

#endif
