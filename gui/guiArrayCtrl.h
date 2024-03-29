//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIARRAYCTRL_H_
#define _GUIARRAYCTRL_H_

#ifndef _GUITYPES_H_
#include "GUI/guiTypes.h"
#endif
#ifndef _GUITEXTCTRL_H_
#include "GUI/guiTextCtrl.h"
#endif

class GuiArrayCtrl : public GuiControl
{
   typedef GuiControl Parent;

protected:

	Point2I mHeaderDim;
   Point2I mSize;
   Point2I mCellSize;
   Point2I mSelectedCell;
   Point2I mMouseOverCell;

   Resource<GFont> mFont;

   bool cellSelected(Point2I cell);
   virtual void onCellSelected(Point2I cell);
public:

   GuiArrayCtrl();
   DECLARE_CONOBJECT(GuiArrayCtrl);

	bool onWake();
	void onSleep();

	//array attribute methods
   Point2I getSize() { return mSize; }
   virtual void setSize(Point2I size);
	void setHeaderDim(const Point2I &dim) { mHeaderDim = dim; }
	void getScrollDimensions(S32 &cell_size, S32 &num_cells);
   
	//selected cell methods
   void setSelectedCell(Point2I cell);
   void deselectCells() { mSelectedCell.set(-1,-1); }
   Point2I getSelectedCell();
   void scrollSelectionVisible();
   void scrollCellVisible(Point2I cell);
   void scrollSelectionTop();
   void scrollSelectionBottom();
   void scrollCellTop(Point2I cell);
   void scrollCellBottom(Point2I cell);

	//rendering methods
	virtual void onRenderColumnHeaders(Point2I offset, Point2I parentOffset, Point2I headerDim);
	virtual void onRenderRowHeader(Point2I offset, Point2I parentOffset, Point2I headerDim, Point2I cell);
   virtual void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);
	//void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);

	//mouse input methods
   void onMouseDown(const GuiEvent &event);
   void onMouseMove(const GuiEvent &event);
   void onMouseEnter(const GuiEvent &event);
   void onMouseLeave(const GuiEvent &event);
   bool onKeyDown(const GuiEvent &event);
   void onRightMouseDown(const GuiEvent &event);

};

#endif //_GUI_ARRAY_CTRL_H
