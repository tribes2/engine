//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICONTROL_H_
#define _GUICONTROL_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MRECT_H_
#include "Math/mRect.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GUITYPES_H_
#include "GUI/guiTypes.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

class GuiCanvas;

class GuiControl : public SimGroup
{

private:
   typedef SimGroup Parent;


public:
	GuiControlProfile *mProfile;

	bool mVisible;
	bool mActive;
	bool mAwake;
	bool mSetFirstResponder;

	S32	mLayer;
	RectI	mBounds;
	Point2I mMinExtent;

	//keyboard input
	GuiControl *mFirstResponder;   
	static GuiControl *gPrevResponder;
	static GuiControl *gCurResponder;

	enum horizSizingOptions
	{
		horizResizeRight = 0, // fixed on the left and width
		horizResizeWidth, // fixed on the left and right
		horizResizeLeft, // fixed on the right and width
	   horizResizeCenter,
	   horizResizeRelative //resize relative
	};
	enum vertSizingOptions
	{
		vertResizeBottom = 0, // fixed on the top and in height
		vertResizeHeight, // fixed on the top and bottom
		vertResizeTop, // fixed in height and on the bottom
	   vertResizeCenter,
	   vertResizeRelative // resize relative
	};

protected:

	S32 mHorizSizing;
	S32 mVertSizing;

   StringTableEntry mConsoleVariable;
   StringTableEntry mConsoleCommand;
   StringTableEntry mAltConsoleCommand;

	StringTableEntry mAcceleratorKey;

	S32 mHelpTag;
	StringTableEntry mHelpText;
   
   // console variable and command methods
   void setVariable(const char *value);
   void setIntVariable(S32 value);
   void setFloatVariable(F32 value);

   const char* getVariable();
   S32 getIntVariable();
   F32 getFloatVariable();

public:
   void setConsoleVariable(const char *variable);
	void setConsoleCommand(const char *newCmd);
	const char * getConsoleCommand();
	void setSizing(S32 horz, S32 vert);
   void inspectPreApply();
   void inspectPostApply();

public:
	static GuiControl* find(const char *name);

   DECLARE_CONOBJECT(GuiControl);
   GuiControl();
   virtual ~GuiControl();
   bool onAdd();
   static void initPersistFields();
	static void consoleInit();

   const Point2I& getPosition() { return mBounds.point; }
   const Point2I& getExtent() { return mBounds.extent; }
   const Point2I& getMinExtent() { return mMinExtent; }

   virtual void setVisible(bool value); 
   inline bool isVisible() { return mVisible; }
   virtual void makeFirstResponder(bool value); 

   void setActive(bool value);
   bool isActive() { return mActive; }

   bool isAwake() 
   {
      return( mAwake );
   }

	//tag, mouseTag and helpTag related calls
   virtual void setHelp(S32 helpTag);
   virtual void setHelp(const char *);
   virtual S32 getHelpTag(F32) { return mHelpTag; }
   virtual const char *getHelpText(F32) { return mHelpText; }
   virtual GuiCursor *getCursor() {return NULL; }

	//methods for organizing the children
	void addObject(SimObject*);
	void removeObject(SimObject*);
	GuiControl *getParent();
	GuiCanvas *getRoot();

	//parents//children coordinate and sizing methods
   Point2I localToGlobalCoord(const Point2I &src);
   Point2I globalToLocalCoord(const Point2I &src);

	virtual void resize(const Point2I &newPosition, const Point2I &newExtent);
	virtual void childResized(GuiControl *child);
   virtual void parentResized(const Point2I &oldParentExtent, const Point2I &newParentExtent);

	//rendering related methods
   virtual void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void renderChildControls(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void setUpdateRegion(Point2I pos, Point2I ext);
   void setUpdate();

	//child hierarchy calls
   void awaken();			// called when this control and it's children have been wired up.
   void sleep();			// called when this control are no longer 
	void preRender();		//prerender it and all it's children

	//parent hierarchy calls - should call Parent::..();
	virtual bool onWake();
	virtual void onSleep();   		//mostly resource stuff
   virtual void onPreRender(); 	// do special pre render processing
	virtual void onRemove();

	//console related methods
   virtual const char *getScriptValue();
   virtual void setScriptValue(const char *value);

	//mouse/keyboard input related methods
   virtual bool pointInControl(const Point2I& parentCoordPoint);
	bool cursorInControl();
   virtual GuiControl* findHitControl(const Point2I &pt, S32 initialLayer = -1);

	void mouseLock(GuiControl *lockingControl);
	void mouseLock();
	void mouseUnlock();

	// General input handler -- for in-GUI control mapping:
	virtual bool onInputEvent(const InputEvent &event);

   virtual void onMouseUp(const GuiEvent &event);
   virtual void onMouseDown(const GuiEvent &event);
   virtual void onMouseMove(const GuiEvent &event);
   virtual void onMouseDragged(const GuiEvent &event);
   virtual void onMouseEnter(const GuiEvent &event);
   virtual void onMouseLeave(const GuiEvent &event);

   virtual bool onMouseWheelUp(const GuiEvent &event);
   virtual bool onMouseWheelDown(const GuiEvent &event);

   virtual void onRightMouseDown(const GuiEvent &event);
   virtual void onRightMouseUp(const GuiEvent &event);
   virtual void onRightMouseDragged(const GuiEvent &event);

	virtual GuiControl* findFirstTabable();
	virtual GuiControl* findLastTabable(bool firstCall = true);
	virtual GuiControl* findPrevTabable(GuiControl *curResponder, bool firstCall = true);
	virtual GuiControl* findNextTabable(GuiControl *curResponder, bool firstCall = true);
	virtual bool ControlIsChild(GuiControl *child);
	virtual void setFirstResponder(GuiControl *firstResponder);
	void setFirstResponder();
   void clearFirstResponder();
	GuiControl *getFirstResponder() { return mFirstResponder; }
	virtual void onLoseFirstResponder();

	void AddAcceleratorKey();
	void BuildAcceleratorMap();
	virtual void AcceleratorKeyPress();
	virtual void AcceleratorKeyRelease();
	virtual bool onKeyDown(const GuiEvent &event);
   virtual bool onKeyUp(const GuiEvent &event);
   virtual bool onKeyRepeat(const GuiEvent &event);

   //misc
   void setControlProfile(GuiControlProfile *prof);

	//the action
	virtual void onAction();

	//peer messaging
   void messageSiblings(S32 message);				//send a message to all siblings
	virtual void onMessage(GuiControl *sender, S32 msg);	//receive a message from another control

	//functions called by the canvas
	virtual void onDialogPush();
	virtual void onDialogPop();

	//this will move into the graphics library at some poing
	bool createBitmapArray(GBitmap *bmp, RectI *boundsArray, S32 numStates, S32 numBitmaps);
};


// default renderer helper functions
void renderRaisedBox(const Point2I &offset, const Point2I &extent, const ColorI &color);
void renderLoweredBox(const Point2I &offset, const Point2I &extent, const ColorI &color);

#endif
