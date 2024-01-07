//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIEDITCTRL_H_
#define _GUIEDITCTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GuiEditCtrl : public GuiControl
{
   typedef GuiControl Parent;

   Vector<GuiControl *> mSelectedControls;
   GuiControl*          mCurrentAddSet;
   GuiControl*          mContentControl;
   Point2I              mLastMousePos;
   Point2I              mSelectionAnchor;
   Point2I              mGridSnap;

   enum mouseModes { Selecting, MovingSelection, SizingSelection, DragSelecting };
   enum sizingModes { sizingNone = 0, sizingLeft = 1, sizingRight = 2, sizingTop = 4, sizingBottom = 8 };

   mouseModes				 mMouseDownMode;
   sizingModes				 mSizingMode;

  public:
   GuiEditCtrl();
   DECLARE_CONOBJECT(GuiEditCtrl);
   static void consoleInit();

   bool onWake();

   void select(GuiControl *ctrl);
   void setRoot(GuiControl *ctrl);
   void setEditMode(bool value);
   S32 getSizingHitKnobs(const Point2I &pt, const RectI &box);
   void getDragRect(RectI &b);
   void drawNut(const Point2I &nut, bool);
   void drawNuts(RectI &box, bool);
   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void addNewControl(GuiControl *ctrl);
   bool selectionContains(GuiControl *ctrl);
   void setCurrentAddSet(GuiControl *ctrl);
   void setSelection(GuiControl *ctrl, bool inclusive = false);

   bool onKeyDown(const GuiEvent &event);
   void onMouseDown(const GuiEvent &event);
   void onMouseUp(const GuiEvent &event);
   void onMouseDragged(const GuiEvent &event);
   void onRightMouseDown(const GuiEvent &event);

   enum Justification {
      JUSTIFY_LEFT,
      JUSTIFY_CENTER,
      JUSTIFY_RIGHT,
      JUSTIFY_TOP,
      JUSTIFY_BOTTOM,
      SPACING_VERTICAL,
      SPACING_HORIZONTAL
   };

   void justifySelection( Justification j);
   void moveSelection(const Point2I &delta);
   void saveSelection(const char *filename);
   void loadSelection(const char *filename);
   void deleteSelection();
   void selectAll();
   void bringToFront();
   void pushToBack();
};

#endif //_GUI_EDIT_CTRL_H
