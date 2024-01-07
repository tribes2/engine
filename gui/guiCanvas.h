//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICANVAS_H_
#define _GUICANVAS_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif
#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GuiCanvas : public GuiControl
{

private:
   typedef GuiControl Parent;
   typedef SimObject Grandparent;

	//Rendering members
   RectI		mOldUpdateRects[2];
   RectI		mCurUpdateRect;
   F32		rLastFrameTime;

public:

private:
	//Mouse members
	F32		mPixelsPerMickey;
   bool		cursorON;
   bool     mShowCursor;
   bool     mRenderFront;
   Point2F	cursorPt;
   Point2I	lastCursorPt;
   GuiCursor   *defaultCursor;
   GuiCursor   *lastCursor;
   bool     lastCursorON;
   
	//Input Members - mouse
   SimObjectPtr<GuiControl>   mMouseCapturedControl;  // All mouse events will go to this ctrl only
   SimObjectPtr<GuiControl>   mMouseControl;          // the control the mouse was last seen in unless some other on captured it
	bool			mMouseControlClicked;	// whether the current ctrl has been clicked - used by helpctrl
	U32		mPrevMouseTime;			// this determines how long the mouse has been in the same control
   U32		mNextMouseTime;			// used for onMouseRepeat()
	U32		mInitialMouseDelay;		// also used for onMouseRepeat()
	bool		mMouseButtonDown;			// bool to determine if the button is depressed
	bool		mMouseRightButtonDown;	// bool to determine if the right button is depressed
   GuiEvent	mLastMouseEvent;

	//Input Members - keyboard
	GuiControl	*keyboardControl;			//  All keyboard events will go to this ctrl first
   U32		nextKeyTime;
   //GuiEvent			lastKeyEvent;

	U8	mLastMouseClickCount;
	S32	mLastMouseDownTime;
	bool	mLeftMouseLast;

	//accelerator key map
	struct AccKeyMap
	{
		GuiControl *ctrl;
		U32 keyCode;
		U32 modifier;
	};
	Vector <AccKeyMap> mAcceleratorMap;
   void findMouseControl(const GuiEvent &event);
   void refreshMouseControl();

public:
   DECLARE_CONOBJECT(GuiCanvas);
   GuiCanvas();
   ~GuiCanvas();
	static void consoleInit();

	//Rendering methods
   void renderFrame(bool preRenderOnly);			//called ever cycle to repaint the updateRects
   void paint();			//causes the entire canvas to be repainted
   void addUpdateRegion(Point2I pos, Point2I ext);
   void resetUpdateRegions();
   void buildUpdateUnion(RectI *updateUnion);

	//Control content methods
   void setContentControl(GuiControl *gui);
   GuiControl *getContentControl();
   
   void pushDialogControl(GuiControl *gui, S32 layer = 0);
	void popDialogControl(S32 layer = 0);
	void popDialogControl(GuiControl *gui);
   
   // general cursor commands
   void setCursor(GuiCursor *curs) { defaultCursor = curs; }
   void setRenderFront(bool front) { mRenderFront = front; }
   bool isCursorON() {return cursorON; }
   void setCursorON(bool onOff);
   void setCursorPos(const Point2I &pt)   { cursorPt.x = F32(pt.x); cursorPt.y = F32(pt.y); }
   Point2I getCursorPos()                 { return Point2I(S32(cursorPt.x), S32(cursorPt.y)); }
   void showCursor(bool state)            { mShowCursor = state; }
   bool isCursorShown()                   { return(mShowCursor); }

	//all input events come through the canvas	
   bool processInputEvent(const InputEvent *event);
   void processMouseMoveEvent(const MouseMoveEvent *event);

	//mouse methods
   void mouseLock(GuiControl *lockingControl);
   void mouseUnlock(GuiControl *lockingControl);
   GuiControl* getMouseControl()       { return mMouseControl; }
   GuiControl* getMouseLockedControl() { return mMouseCapturedControl; }
	bool mouseButtonDown(void) { return mMouseButtonDown; }
	bool mouseRightButtonDown(void) { return mMouseRightButtonDown; }

	//Mouse input methods
   void rootMouseUp(const GuiEvent &event);
   void rootMouseDown(const GuiEvent &event);
   void rootMouseMove(const GuiEvent &event);
   void rootMouseDragged(const GuiEvent &event);

   void rootRightMouseDown(const GuiEvent &event);
   void rootRightMouseUp(const GuiEvent &event);
   void rootRightMouseDragged(const GuiEvent &event);

   void rootMouseWheelUp(const GuiEvent &event);
   void rootMouseWheelDown(const GuiEvent &event);

	//Keyboard input methods
	void tabNext(void);
	void tabPrev(void);
	void AddAcceleratorKey(GuiControl *ctrl, U32 keyCode, U32 modifier);

	void setFirstResponder(GuiControl *firstResponder);

	//deleted?  possibly
   //DWORD onMessage(SimObject *sender, DWORD msg); // root handles IDGUI and IDCMD messages
};

extern GuiCanvas *Canvas;

#endif
