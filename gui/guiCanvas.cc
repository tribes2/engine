//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "platform/event.h"
#include "platform/platform.h"
#include "platform/platformVideo.h"
#include "gui/guiTypes.h"
#include "gui/guiTextCtrl.h"
#include "gui/guiTextEditCtrl.h"
#include "gui/guiTextListCtrl.h"
#include "gui/guiBitmapCtrl.h"
#include "gui/guiButtonCtrl.h"
#include "gui/guiRadioCtrl.h"
#include "gui/guiCheckBoxCtrl.h"
#include "gui/guiArrayCtrl.h"
#include "gui/guiScrollCtrl.h"
#include "gui/guiSliderCtrl.h"
#include "gui/guiWindowCtrl.h"
#include "gui/guiConsole.h"
#include "gui/guiCanvas.h"
#include "gui/guiInspector.h"
#include "gui/guiTreeViewCtrl.h"
#include "gui/guiEditCtrl.h"
#include "gui/guiFilterCtrl.h"
#include "gui/guiConsoleTextCtrl.h"
#include "gui/guiPopUpCtrl.h"
#include "gui/guiDebugger.h"
#include "editor/guiTerrPreviewCtrl.h"
#include "gui/guiBackgroundCtrl.h"
#include "gui/guiTextEditSliderCtrl.h"
#include "gui/guiProgressCtrl.h"
#include "gui/guiVoteCtrl.h"
#include "platform/profiler.h"

extern bool gDGLRender;


//all the ConObject implements go here
IMPLEMENT_CONOBJECT(GuiControlProfile);
IMPLEMENT_CONOBJECT(GuiCursor);
IMPLEMENT_CONOBJECT(GuiControl);
IMPLEMENT_CONOBJECT(GuiCanvas);
IMPLEMENT_CONOBJECT(GuiTextCtrl);
IMPLEMENT_CONOBJECT(GuiTextEditCtrl);
IMPLEMENT_CONOBJECT(GuiTextListCtrl);
IMPLEMENT_CONOBJECT(GuiBitmapCtrl);
IMPLEMENT_CONOBJECT(GuiButtonCtrl);
IMPLEMENT_CONOBJECT(GuiRadioCtrl);
IMPLEMENT_CONOBJECT(GuiCheckBoxCtrl);
IMPLEMENT_CONOBJECT(GuiArrayCtrl);
IMPLEMENT_CONOBJECT(GuiScrollContentCtrl);
IMPLEMENT_CONOBJECT(GuiScrollCtrl);
IMPLEMENT_CONOBJECT(GuiSliderCtrl);
IMPLEMENT_CONOBJECT(GuiWindowCtrl);
IMPLEMENT_CONOBJECT(GuiPopUpMenuCtrl);
IMPLEMENT_CONOBJECT(GuiFilterCtrl);
IMPLEMENT_CONOBJECT(GuiBackgroundCtrl);
IMPLEMENT_CONOBJECT(GuiTextEditSliderCtrl);
IMPLEMENT_CONOBJECT(GuiProgressCtrl);
IMPLEMENT_CONOBJECT(GuiVoteCtrl);

IMPLEMENT_CONOBJECT(GuiConsole);
IMPLEMENT_CONOBJECT(GuiInspector);
IMPLEMENT_CONOBJECT(GuiTreeView);
IMPLEMENT_CONOBJECT(GuiEditCtrl);

// editor/debugging controls
IMPLEMENT_CONOBJECT(GuiConsoleTextCtrl);
IMPLEMENT_CONOBJECT(DbgFileView);
             
GuiCanvas *Canvas = NULL;

static S32 cGuiCanvasGetContent(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = ((GuiCanvas *) obj)->getContentControl();
   if(ctrl)
      return ctrl->getId();
   return -1;
}

static void cGuiCanvasSetContent(SimObject *obj, S32 argc, const char **argv)
{
   obj;
   argc;
   
   GuiControl *gui = NULL;
   if(argv[2][0])
   {
	   SimObject* ctrl = Sim::findObject(argv[2]);
      if (ctrl)
         gui = dynamic_cast<GuiControl*>(ctrl);
      if (! gui)
      {
         Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
         return;
      }
   }
   //set the new content control
   Canvas->setContentControl(gui);
}

static void cGuiCanvasPushDialog(SimObject *obj, S32 argc, const char **argv)
{
   obj;
   
   GuiControl *gui = NULL;
	SimObject* ctrl = Sim::findObject(argv[2]);
   if (ctrl)
      gui = dynamic_cast<GuiControl*>(ctrl);
   if (! gui)
   {
      Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
      return;
   }
   
   //find the layer
   S32 layer = 0;
   if (argc == 4)
      layer = dAtoi(argv[3]);
      
   //set the new content control
   Canvas->pushDialogControl(gui, layer);
}

static void cGuiCanvasPopDialog(SimObject *obj, S32 argc, const char **argv)
{
   obj;
   
   GuiControl *gui = NULL;
   if (argc == 3)
   {
   	SimObject* ctrl = Sim::findObject(argv[2]);
      if (ctrl)
         gui = dynamic_cast<GuiControl*>(ctrl);
      if (! gui)
      {
         Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
         return;
      }
   }
   
   if (gui)
      Canvas->popDialogControl(gui);
   else
      Canvas->popDialogControl();
}

static void cGuiCanvasPopLayer(SimObject *obj, S32 argc, const char **argv)
{
   obj;
   
   S32 layer = 0;
   if (argc == 3)
      layer = dAtoi(argv[2]);
   Canvas->popDialogControl(layer);
}

static void cGuiCanvasCursorOn(SimObject *obj, S32 argc, const char **argv)
{
   obj;
   argc;
   argv;
   Canvas->setCursorON(true);
}

static void cGuiCanvasCursorOff(SimObject *obj, S32 argc, const char **argv)
{
   obj;
   argc;
   argv;
   Canvas->setCursorON(false);
}

static void cGuiCanvasSetCursor(SimObject *obj, S32 argc, const char **argv)
{
   obj;
   argc;
   
   GuiCursor *curs = NULL;
   if(argv[2][0])
   {
      if(!Sim::findObject(argv[2], curs))
      {
         Con::printf("%s is not a valid cursor.", argv[2]);
         return;
      }
   }
   Canvas->setCursor(curs);
}

static void cGuiCanvasRenderFront(SimObject*, S32, const char **argv)
{
   Canvas->setRenderFront(dAtob(argv[2]));
}

static void cGuiCanvasShowCursor(SimObject *, S32, const char **)
{
   Canvas->showCursor(true);
}

static void cGuiCanvasHideCursor(SimObject *, S32, const char **)
{
   Canvas->showCursor(false);
}

static bool cGuiCanvasIsCursorOn(SimObject *, S32, const char **)
{
   return Canvas->isCursorON();
}

static void cGuiCanvasRepaint(SimObject *, S32, const char **)
{
   Canvas->paint();
}

static void cGuiCanvasReset(SimObject *, S32, const char **)
{
   Canvas->resetUpdateRegions();
}

static const char * cGuiCanvasGetCursorPos(SimObject *, S32, const char **)
{
   Point2I pos = Canvas->getCursorPos();
   char * ret = Con::getReturnBuffer(32);
   dSprintf(ret, 32, "%d %d", pos.x, pos.y);
   return(ret);
}

static void cGuiCanvasSetCursorPos(SimObject *, S32 argc, const char ** argv)
{
   Point2I pos(0,0);
   if(argc == 4)
      pos.set(dAtoi(argv[2]), dAtoi(argv[3]));
   else
      dSscanf(argv[3], "%d %d", &pos.x, &pos.y);
   Canvas->setCursorPos(pos);
}

void GuiCanvas::consoleInit(void)
{
   Con::addCommand("GuiCanvas", "renderFront",  cGuiCanvasRenderFront,   "canvas.renderFront(bool)",   3, 3);
   Con::addCommand("GuiCanvas", "setContent",   cGuiCanvasSetContent,    "canvas.setContent(ctrl)",    3, 3);
   Con::addCommand("GuiCanvas", "getContent",   cGuiCanvasGetContent,    "canvas.getContent()",        2, 2);
   Con::addCommand("GuiCanvas", "pushDialog",   cGuiCanvasPushDialog,    "canvas.pushDialog(ctrl)",    3, 4);
   Con::addCommand("GuiCanvas", "popDialog",    cGuiCanvasPopDialog,     "canvas.popDialog(<ctrl>)",   2, 3);
   Con::addCommand("GuiCanvas", "popLayer",     cGuiCanvasPopLayer,      "canvas.popLayer(<S32>)",     2, 3);
   Con::addCommand("GuiCanvas", "cursorOn",     cGuiCanvasCursorOn,      "canvas.cursorOn()",          2, 2);
   Con::addCommand("GuiCanvas", "cursorOff",    cGuiCanvasCursorOff,     "canvas.cursorOff()",         2, 2);
   Con::addCommand("GuiCanvas", "setCursor",    cGuiCanvasSetCursor,     "canvas.setCursor(cursor)",   3, 3);
   Con::addCommand("GuiCanvas", "hideCursor",   cGuiCanvasHideCursor,    "canvas.hideCursor()",        2, 2);
   Con::addCommand("GuiCanvas", "showCursor",   cGuiCanvasShowCursor,    "canvas.showCursor()",        2, 2);
   Con::addCommand("GuiCanvas", "repaint",      cGuiCanvasRepaint,       "canvas.repaint()",           2, 2);
	Con::addCommand("GuiCanvas", "reset",			cGuiCanvasReset,			 "canvas.reset()",				 2, 2);
   Con::addCommand("GuiCanvas", "isCursorOn",   cGuiCanvasIsCursorOn,    "canvas.isCursorOn()",        2, 2);
   Con::addCommand("GuiCanvas", "getCursorPos", cGuiCanvasGetCursorPos,  "canvas.getCursorPos()",      2, 2);
   Con::addCommand("GuiCanvas", "setCursorPos", cGuiCanvasSetCursorPos,  "canvas.setCursorPos(pos)",   3, 4);
}

GuiCanvas::GuiCanvas()
{
   mBounds.set(0, 0, 640, 480);
   mAwake = true;
   mPixelsPerMickey = 1.0f;
   lastCursorON = false;
   cursorON    = true; 
   mShowCursor = true;
   rLastFrameTime = 0.0f;

   mMouseCapturedControl = NULL;
   mMouseControl = NULL;
   mMouseControlClicked = false;
   mMouseButtonDown = false;
   mMouseRightButtonDown = false;

   lastCursor = NULL;
   lastCursorPt.set(0,0);
   cursorPt.set(0,0);
   
	mLastMouseClickCount = 0;
	mLastMouseDownTime = 0;
	mPrevMouseTime = 0;
	defaultCursor = NULL;

   mRenderFront = false;
}

GuiCanvas::~GuiCanvas()
{
   if(Canvas == this)
      Canvas = 0;
}

//------------------------------------------------------------------------------
void GuiCanvas::setCursorON(bool onOff)               
{ 
   cursorON = onOff;
   if(!cursorON)
      mMouseControl = NULL;
}

void GuiCanvas::AddAcceleratorKey(GuiControl *ctrl, U32 keyCode, U32 modifier)
{
   if (keyCode > 0 && ctrl)
   {
      AccKeyMap newMap;
      newMap.ctrl = ctrl;
      newMap.keyCode = keyCode;
      newMap.modifier = modifier;
      mAcceleratorMap.push_back(newMap);
   }
}

void GuiCanvas::tabNext(void)
{
   GuiControl *ctrl = static_cast<GuiControl *>(last());
   if (ctrl)
   {
      //save the old
      GuiControl *oldResponder = mFirstResponder;
      
		GuiControl* newResponder = ctrl->findNextTabable(mFirstResponder);
      if ( !newResponder )
         newResponder = ctrl->findFirstTabable();

		if ( newResponder && newResponder != oldResponder )
		{
			newResponder->setFirstResponder();
      
      	if ( oldResponder )
         	oldResponder->onLoseFirstResponder();
		}
   }
}

void GuiCanvas::tabPrev(void)
{
   GuiControl *ctrl = static_cast<GuiControl *>(last());
   if (ctrl)
   {
      //save the old
      GuiControl *oldResponder = mFirstResponder;
      
		GuiControl* newResponder = ctrl->findPrevTabable(mFirstResponder);
		if ( !newResponder )
         newResponder = ctrl->findLastTabable();

		if ( newResponder && newResponder != oldResponder )
		{
			newResponder->setFirstResponder();
	      
	      if ( oldResponder )
	         oldResponder->onLoseFirstResponder();
		}
   }
}

void GuiCanvas::processMouseMoveEvent(const MouseMoveEvent *event)
{
   InputEvent iEvent;
   iEvent.deviceType = MouseDeviceType;
   iEvent.objType = SI_XAXIS;
   iEvent.fValue = event->xPos - cursorPt.x;
   iEvent.modifier = event->modifier;
   processInputEvent(&iEvent);
   iEvent.objType = SI_YAXIS;
   iEvent.fValue = event->yPos - cursorPt.y;
   processInputEvent(&iEvent);
}
   
bool GuiCanvas::processInputEvent(const InputEvent *event)
{
   GuiEvent evt;

	// First call the general input handler (on the extremely off-chance that it will be handled):
	if ( mFirstResponder )
   {
      if ( mFirstResponder->onInputEvent( *event ) )
		   return( true );
   }

   if(event->deviceType == KeyboardDeviceType)
   {
      evt.ascii = event->ascii;
      evt.modifier = event->modifier;
      evt.keyCode = event->objInst;
            
      if (event->action == SI_MAKE)
      {
         //see if we should tab next/prev
         
         //see if we should now pass the event to the first responder
         if (mFirstResponder)
         {
            if(mFirstResponder->onKeyDown(evt))
               return true;
         }

         if ( isCursorON() && ( event->objInst == KEY_TAB ) )
         {
            if (size() > 0)
            {
               if (event->modifier & SI_SHIFT)
               {
                  tabPrev();
                  return true;
               }
               else if (event->modifier == 0)
               {
                  tabNext();
                  return true;
               }
            }
         }
         
         //if not handled, search for an accelerator
         for (U32 i = 0; i < mAcceleratorMap.size(); i++)
         {
            if ((U32)mAcceleratorMap[i].keyCode == (U32)event->objInst && (U32)mAcceleratorMap[i].modifier == (U32)event->modifier)
            {
               mAcceleratorMap[i].ctrl->AcceleratorKeyPress();
               return true;
            }
         }
      }
      else if(event->action == SI_BREAK)
      {
         if(mFirstResponder)
            if(mFirstResponder->onKeyUp(evt))
               return true;
            
         //see if there's an accelerator
         for (U32 i = 0; i < mAcceleratorMap.size(); i++)
         {
            if ((U32)mAcceleratorMap[i].keyCode == (U32)event->objInst && (U32)mAcceleratorMap[i].modifier == (U32)event->modifier)
            {
               mAcceleratorMap[i].ctrl->AcceleratorKeyRelease();
               return true;
            }
         }
      }
      else if(event->action == SI_REPEAT)
      {
         if(mFirstResponder)
            mFirstResponder->onKeyRepeat(evt);
         return true;
      }
   }
   else if(event->deviceType == MouseDeviceType && cursorON)
   {      
      //copy the modifier into the new event
      evt.modifier = event->modifier;
      
      if(event->objType == SI_XAXIS || event->objType == SI_YAXIS)
      {
         bool moved = false;
         Point2I oldpt(cursorPt.x, cursorPt.y);
         Point2F pt(cursorPt.x, cursorPt.y);

         if (event->objType == SI_XAXIS)
         {
            pt.x += (event->fValue * mPixelsPerMickey);
            cursorPt.x = getMax(0, getMin((S32)pt.x, mBounds.extent.x - 1));
            if (oldpt.x != S32(cursorPt.x))
               moved = true;
         }
         else
         {
            pt.y += (event->fValue * mPixelsPerMickey);
            cursorPt.y = getMax(0, getMin((S32)pt.y, mBounds.extent.y - 1));
            if (oldpt.y != S32(cursorPt.y))
               moved = true;
         }
         if (moved)
         {
            evt.mousePoint.x = S32(cursorPt.x);
            evt.mousePoint.y = S32(cursorPt.y);
            
            if (mMouseButtonDown)
               rootMouseDragged(evt);
            else if (mMouseRightButtonDown)
               rootRightMouseDragged(evt);
            else
               rootMouseMove(evt);
         }
         return true;
      }
		else if ( event->objType == SI_ZAXIS )
		{
         evt.mousePoint.x = S32( cursorPt.x );
         evt.mousePoint.y = S32( cursorPt.y );

			if ( event->fValue < 0.0f )
            rootMouseWheelDown( evt );
			else 
            rootMouseWheelUp( evt );
      }
      else if(event->objType == SI_BUTTON)
      {
         //copy the cursor point into the event
         evt.mousePoint.x = S32(cursorPt.x);
         evt.mousePoint.y = S32(cursorPt.y);
               
         if(event->objInst == KEY_BUTTON0) // left button
         {
            //see if button was pressed
            if (event->action == SI_MAKE)
            {
               U32 curTime = Platform::getVirtualMilliseconds();
               mNextMouseTime = curTime + mInitialMouseDelay;
               
               //if the last button pressed was the left...
               if (mLeftMouseLast)
               {
                  //if it was within the double click time count the clicks
                  if (curTime - mLastMouseDownTime <= 500)
                     mLastMouseClickCount++;
                  else
                     mLastMouseClickCount = 1;
               }
               else
               {
                  mLeftMouseLast = true;
                  mLastMouseClickCount = 1;
               }
               
               mLastMouseDownTime = curTime;
               evt.mouseClickCount = mLastMouseClickCount;
               
               mLastMouseEvent = evt;
               rootMouseDown(evt);
            }
            //else button was released
            else
            {
               mNextMouseTime = 0xFFFFFFFF;
               rootMouseUp(evt);
            }
            return true;
         }
         else if(event->objInst == KEY_BUTTON1) // right button
         {
            if(event->action == SI_MAKE)
            {
               U32 curTime = Platform::getVirtualMilliseconds();
               
               //if the last button pressed was the right...
               if (! mLeftMouseLast)
               {
                  //if it was within the double click time count the clicks
                  if (curTime - mLastMouseDownTime <= 50)
                     mLastMouseClickCount++;
                  else
                     mLastMouseClickCount = 1;
               }
               else
               {
                  mLeftMouseLast = false;
                  mLastMouseClickCount = 1;
               }
               
               mLastMouseDownTime = curTime;
               evt.mouseClickCount = mLastMouseClickCount;
               
               rootRightMouseDown(evt);
            }
            else // it was a mouse up
               rootRightMouseUp(evt);
            return true;
         }
      }
   }
   return false;
}

void GuiCanvas::rootMouseDown(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseButtonDown = true;
   
   //pass the event to the mouse locked control
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseDown(event);
   
   //else pass it to whoever is underneath the cursor
   else
   {
      iterator i;
      i = end();
      while (i != begin())
      {
         i--;
         GuiControl *ctrl = static_cast<GuiControl *>(*i);
         GuiControl *controlHit = ctrl->findHitControl(event.mousePoint);
         
         //see if the controlHit is a modeless dialog...
         if ((! controlHit->mActive) && (! controlHit->mProfile->mModal))
            continue;
         else
         {
            GuiControl * firstResponder = getFirstResponder();
            if(firstResponder && firstResponder != controlHit)
            {
               firstResponder->clearFirstResponder();
               firstResponder->onLoseFirstResponder();
            }
            controlHit->onMouseDown(event);
            break;
         }
      }
   }
   if (bool(mMouseControl))
      mMouseControlClicked = true;
}

void GuiCanvas::findMouseControl(const GuiEvent &event)
{
   if(size() == 0)
   {
      mMouseControl = NULL;
      return;
   }
   GuiControl *controlHit = NULL;
   for(S32 i = size() - 1; i >= 0; i--)
   {
      GuiControl *ctrl = static_cast<GuiControl *>((*this)[i]);
      Point2I pt = ctrl->globalToLocalCoord(Point2I(cursorPt.x, cursorPt.y));
      controlHit = ctrl->findHitControl(pt);
      if(controlHit->mProfile->mModal)
         break;
   }
   if(controlHit != static_cast<GuiControl*>(mMouseControl))
   {
      if(bool(mMouseControl))
         mMouseControl->onMouseLeave(event);
      mMouseControl = controlHit;
      mMouseControl->onMouseEnter(event);
   }
}

void GuiCanvas::refreshMouseControl()
{
   GuiEvent evt;
   evt.mousePoint.x = S32(cursorPt.x);
   evt.mousePoint.y = S32(cursorPt.y);
   findMouseControl(evt);
}

void GuiCanvas::rootMouseUp(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseButtonDown = false;
   
   //pass the event to the mouse locked control
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseUp(event);
   else
   {
      findMouseControl(event);
      if(bool(mMouseControl))
         mMouseControl->onMouseUp(event);
   }
}

void GuiCanvas::rootMouseDragged(const GuiEvent &event)
{
   //pass the event to the mouse locked control
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseDragged(event);
   else
   {
      findMouseControl(event);
      if(bool(mMouseControl))
         mMouseControl->onMouseDragged(event);
   }
}

void GuiCanvas::rootMouseMove(const GuiEvent &event)
{
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseMove(event);
   else
   {
      findMouseControl(event);
      if(bool(mMouseControl))
         mMouseControl->onMouseMove(event);
   }
}

void GuiCanvas::rootRightMouseDown(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseRightButtonDown = true;
   
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onRightMouseDown(event);
   else
   {
      findMouseControl(event);
         
      if(bool(mMouseControl))
      {
         if(mMouseControl->mActive && mMouseControl->mProfile->mModal)
         {
            GuiControl * firstResponder = getFirstResponder();
            if(firstResponder && firstResponder != static_cast<GuiControl*>(mMouseControl))
            {
               firstResponder->clearFirstResponder();
               firstResponder->onLoseFirstResponder();
            }
         }
         mMouseControl->onRightMouseDown(event);
      }
   }
}

void GuiCanvas::rootRightMouseUp(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseRightButtonDown = false;
   
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onRightMouseUp(event);
   else 
   {
      findMouseControl(event);
         
      if(bool(mMouseControl))
         mMouseControl->onRightMouseUp(event);
   }
}

void GuiCanvas::rootRightMouseDragged(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
      
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onRightMouseDragged(event);
   else
   {
      findMouseControl(event);
         
      if(bool(mMouseControl))
         mMouseControl->onRightMouseDragged(event);
   }
}

void GuiCanvas::rootMouseWheelUp(const GuiEvent &event)
{
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseWheelUp(event);
   else
   {
      findMouseControl(event);

      if (bool(mMouseControl))
         mMouseControl->onMouseWheelUp(event);
   }
}

void GuiCanvas::rootMouseWheelDown(const GuiEvent &event)
{
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseWheelDown(event);
   else
   {
      findMouseControl(event);

      if (bool(mMouseControl))
         mMouseControl->onMouseWheelDown(event);
   }
}

void GuiCanvas::setContentControl(GuiControl *gui)
{
   //remove all dialogs on layer 0
   
   // awaken the new control:
   U32 index = 0;
   bool wakedGui = false;
   
   if(gui)
   {
      //add the gui to the front
      if(!size() || gui != (*this)[0])
      {
         // automatically wakes objects in GuiControl::onAdd
         wakedGui = !gui->isAwake();
         addObject(gui);
         if (size() >= 2)
            reOrder(gui, *begin());
      }
      index = 1;
   }
   while (size() > index)
   {
      GuiControl *ctrl = static_cast<GuiControl*>((*this)[index]);
      if (ctrl->mLayer != 0)
         break;
      
      bool didSleep = ctrl->isAwake();

      // GuiControl::removeObject removes awoke controls
      removeObject(ctrl);
      if(didSleep)
         Con::executef(ctrl, 1, "onSleep");

      Sim::getGuiGroup()->addObject(ctrl);
   }
   
   if(gui)
   {
      //wake the object up in script
      if(wakedGui)
         Con::executef(gui, 1, "onWake");
   
      GuiControl *oldResponder = mFirstResponder;
      mFirstResponder = gui->findFirstTabable();
      if(oldResponder && oldResponder != mFirstResponder)
         oldResponder->onLoseFirstResponder();
   }
   //refresh the entire gui
   resetUpdateRegions();
   
   //rebuild the accelerator map
   mAcceleratorMap.clear();

   for(iterator i = end(); i != begin() ; )
   {
      i--;
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->BuildAcceleratorMap();

      if (ctrl->mProfile->mModal)
         break;
   }
   refreshMouseControl();
}

GuiControl *GuiCanvas::getContentControl()
{
   if(size() > 0)
      return (GuiControl *) first();
   return NULL;
}

void GuiCanvas::pushDialogControl(GuiControl *gui, S32 layer)
{
   //add the gui
   gui->mLayer = layer;

   // GuiControl::addObject wakes the object
   bool wakedGui = !gui->isAwake();
   addObject(gui);
   
   //reorder it to the correct layer
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl*>(*i);
      if (ctrl->mLayer > gui->mLayer)
      {
         reOrder(gui, ctrl);
         break;
      }
   }
   
   // call the 'onWake' method?
   if(wakedGui)
      Con::executef(gui, 1, "onWake");
   
   //call the dialog push method
   gui->onDialogPush();
   
   //find the top most dialog
   GuiControl *topCtrl = static_cast<GuiControl*>(last());
   
   //save the old
   GuiControl *oldResponder = mFirstResponder;
   
   //find the first responder
   mFirstResponder = gui->findFirstTabable();
   
   if (oldResponder && oldResponder != mFirstResponder)
      oldResponder->onLoseFirstResponder();
      
   //refresh the entire gui
   resetUpdateRegions();
   
   //rebuild the accelerator map
   mAcceleratorMap.clear();
   if (size() > 0)
   {
      GuiControl *ctrl = static_cast<GuiControl*>(last());
      ctrl->BuildAcceleratorMap();
   }
   refreshMouseControl();
}

void GuiCanvas::popDialogControl(GuiControl *gui)
{
   if (size() < 1)
      return;
   
   //first, find the dialog, and call the "onDialogPop()" method
   GuiControl *ctrl = NULL;
   if (gui)
   {
      //make sure the gui really exists on the stack
      iterator i;
      bool found = false;
      for(i = begin(); i != end(); i++)
      {
         GuiControl *check = static_cast<GuiControl *>(*i);
         if (check == gui)
         {
            ctrl = check;
            found = true;
         }
      }
      if (! found)
         return;
   }
   else
      ctrl = static_cast<GuiControl*>(last());
   
   //call the "on pop" function   
   ctrl->onDialogPop();
   
   // sleep the object
   bool didSleep = ctrl->isAwake();
   
   //now pop the last child (will sleep if awake)
   removeObject(ctrl);
   
   // Save the old responder:
   GuiControl *oldResponder = mFirstResponder;

   if(didSleep)
      Con::executef(ctrl, 1, "onSleep");

   Sim::getGuiGroup()->addObject(ctrl);
   
   if (size() > 0)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(last());
      mFirstResponder = ctrl->mFirstResponder;
   }
   else
   {
      mFirstResponder = NULL;
   }
   
   if (oldResponder && oldResponder != mFirstResponder)
      oldResponder->onLoseFirstResponder();
         
   //refresh the entire gui
   resetUpdateRegions();
   
   //rebuild the accelerator map
   mAcceleratorMap.clear();
   if (size() > 0)
   {
      GuiControl *ctrl = static_cast<GuiControl*>(last());
      ctrl->BuildAcceleratorMap();
   }
   refreshMouseControl();
}

void GuiCanvas::popDialogControl(S32 layer)
{
   if (size() < 1)
      return;
      
   GuiControl *ctrl = NULL;
   iterator i = end(); // find in z order (last to first)
   while (i != begin())
   {
      i--;
      ctrl = static_cast<GuiControl*>(*i);
      if (ctrl->mLayer == layer)
         break;
   }
   if (ctrl)
      popDialogControl(ctrl);
}

void GuiCanvas::mouseLock(GuiControl *lockingControl)
{
   if (bool(mMouseCapturedControl))
      return;
   mMouseCapturedControl = lockingControl;
}

void GuiCanvas::mouseUnlock(GuiControl *lockingControl)
{
   if (static_cast<GuiControl*>(mMouseCapturedControl) != lockingControl)
      return;
   
   GuiEvent evt;
   evt.mousePoint.x = S32(cursorPt.x);
   evt.mousePoint.y = S32(cursorPt.y);
   
   GuiControl * controlHit = NULL;
   for(S32 i = size() - 1; i >= 0; i--)
   {
      GuiControl *ctrl = static_cast<GuiControl *>((*this)[i]);
      Point2I pt = ctrl->globalToLocalCoord(Point2I(cursorPt.x, cursorPt.y));
      controlHit = ctrl->findHitControl(pt);
      if(controlHit->mProfile->mModal)
         break;
   }
   
   if (controlHit != static_cast<GuiControl*>(mMouseCapturedControl))
   {
      if (bool(mMouseCapturedControl))
         mMouseCapturedControl->onMouseLeave(evt);

      mMouseControl = controlHit;
      mMouseControlClicked = false;
      if (bool(mMouseControl))
         mMouseControl->onMouseEnter(evt);
   }
   mMouseCapturedControl = NULL;
}

void GuiCanvas::paint()
{
   resetUpdateRegions();

   // inhibit explicit refreshes in the case we're swapped out
   if (gDGLRender)
      renderFrame(false);
}

void GuiCanvas::renderFrame(bool preRenderOnly)
{
   PROFILE_START(CanvasPreRender);
   if(mRenderFront)
      glDrawBuffer(GL_FRONT);
   else
      glDrawBuffer(GL_BACK);
   
   Point2I size = Platform::getWindowSize();

   if(size.x == 0 || size.y == 0)
      return;
      
   RectI screenRect(0, 0, size.x, size.y);
   mBounds = screenRect;
   
   //all bottom level controls should be the same dimensions as the canvas
   //this is necessary for passing mouse events accurately
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      AssertFatal(static_cast<GuiControl*>((*i))->isAwake(), "GuiCanvas::renderFrame: ctrl is not awake");
      GuiControl *ctrl = static_cast<GuiControl*>(*i);
      Point2I ext = ctrl->getExtent();
      Point2I pos = ctrl->getPosition();
      
      if(pos != screenRect.point || ext != screenRect.extent)
      {
         ctrl->resize(screenRect.point, screenRect.extent);
         resetUpdateRegions();
      }
   }
   
   //preRender (recursive) all controls
   preRender();
   PROFILE_END();
   if(preRenderOnly)
      return;
   
   //resetUpdateRegions();
   
   // finish the gl render so we don't get too far ahead of ourselves

   PROFILE_START(glFinish);
   glFinish();
   PROFILE_END();
   
   //draw the mouse, but not using tags...
   PROFILE_START(CanvasRenderControls);
   
   GuiCursor *mouseCursor = NULL;
   if(bool(mMouseCapturedControl))
      mouseCursor = mMouseCapturedControl->getCursor();
   else if(bool(mMouseControl))
      mouseCursor = mMouseControl->getCursor();

   //determine if a help ctrl should be drawn
   //get the time
   //U32 time = Platform::getVirtualMilliseconds();

   //static SimManager* simMngr = SimGame::get()->getManager();
   //AssertFatal(simMngr, "Unable to get the sim manager");
   //GuiHelpCtrl *helpCtrl = static_cast<GuiHelpCtrl*>(simMngr->findObject(guiHelpObjectID));
   
   //S32 helpTag = 0;
   //const char *helpText = NULL;
   //if (mMouseControl && cursorON && (! mMouseCapturedControl) && (! mMouseButtonDown) && (! mMouseRightButtonDown))
   //{
   //   helpTag = mMouseControl->getHelpTag(time - mPrevMouseTime);
   //   helpText = mMouseControl->getHelpText(time - mPrevMouseTime);
   //}
   //if (helpCtrl) 
   //{
   //   helpCtrl->setHelpTag(this, helpTag,  time - mPrevMouseTime, mMouseControlClicked);
   //   helpCtrl->setHelpText(this, helpText, time - mPrevMouseTime, mMouseControlClicked);
   //}

   Point2I cursorPos(cursorPt.x, cursorPt.y);
   if(!mouseCursor)
      mouseCursor = defaultCursor;

   if(lastCursorON && lastCursor)
   {
      Point2I spot = lastCursor->getHotSpot();
      Point2I cext = lastCursor->getExtent();
      Point2I pos = lastCursorPt - spot;
      addUpdateRegion(pos - Point2I(2, 2), Point2I(cext.x + 4, cext.y + 4));
   }
   if(cursorON && mouseCursor)
   {
      Point2I spot = mouseCursor->getHotSpot();
      Point2I cext = mouseCursor->getExtent();
      Point2I pos = cursorPos - spot;

      addUpdateRegion(pos - Point2I(2, 2), Point2I(cext.x + 4, cext.y + 4));
   }

	lastCursorON = cursorON;
	lastCursor = mouseCursor;
	lastCursorPt = cursorPos;

   RectI updateUnion;
   buildUpdateUnion(&updateUnion);
   if (updateUnion.intersect(screenRect))
   {
      //fill in with black first
      //glClearColor(0, 0, 0, 0);
      //glClear(GL_COLOR_BUFFER_BIT);

      //render the dialogs
      iterator i;
      for(i = begin(); i != end(); i++)
      {
         GuiControl *contentCtrl = static_cast<GuiControl*>(*i);
         dglSetClipRect(updateUnion);
         glDisable( GL_CULL_FACE );
         contentCtrl->onRender(contentCtrl->getPosition(), updateUnion, mFirstResponder);
      }
      
      dglSetClipRect(updateUnion);
      
      //temp draw the mouse
      if (cursorON && mShowCursor && !mouseCursor)
      {
         glColor4ub(255, 0, 0, 255);
         glRecti((S32)cursorPt.x, (S32)cursorPt.y, (S32)(cursorPt.x + 2), (S32)(cursorPt.y + 2));
      }
      
      //DEBUG
      //draw the help ctrl
      //if (helpCtrl)
      //{
      //   helpCtrl->render(srf);
      //}

      if (cursorON && mouseCursor && mShowCursor)
      {
         Point2I pos(cursorPt.x, cursorPt.y);
         Point2I spot = mouseCursor->getHotSpot();
         
         pos -= spot;
         dglClearBitmapModulation();
         dglDrawBitmap(mouseCursor->getTextureHandle(), pos);
      }
   }
   PROFILE_END();
   PROFILE_START(SwapBuffers);
   //flip the surface
   if(!mRenderFront)
      Video::swapBuffers();
   PROFILE_END();
}

void GuiCanvas::buildUpdateUnion(RectI *updateUnion)
{
   *updateUnion = mOldUpdateRects[0];

   //the update region should encompass the oldUpdateRects, and the curUpdateRect
   Point2I upperL;
   Point2I lowerR;
   
   upperL.x = getMin(mOldUpdateRects[0].point.x, mOldUpdateRects[1].point.x);
   upperL.x = getMin(upperL.x, mCurUpdateRect.point.x);
   
   upperL.y = getMin(mOldUpdateRects[0].point.y, mOldUpdateRects[1].point.y);
   upperL.y = getMin(upperL.y, mCurUpdateRect.point.y);
   
   lowerR.x = getMax(mOldUpdateRects[0].point.x + mOldUpdateRects[0].extent.x, mOldUpdateRects[1].point.x + mOldUpdateRects[1].extent.x);
   lowerR.x = getMax(lowerR.x, mCurUpdateRect.point.x + mCurUpdateRect.extent.x);
   
   lowerR.y = getMax(mOldUpdateRects[0].point.y + mOldUpdateRects[0].extent.y, mOldUpdateRects[1].point.y + mOldUpdateRects[1].extent.y);
   lowerR.y = getMax(lowerR.y, mCurUpdateRect.point.y + mCurUpdateRect.extent.y);
   
   updateUnion->point = upperL;
   updateUnion->extent = lowerR - upperL;
   
   //shift the oldUpdateRects
   mOldUpdateRects[0] = mOldUpdateRects[1];
   mOldUpdateRects[1] = mCurUpdateRect;
   
   mCurUpdateRect.point.set(0,0);
   mCurUpdateRect.extent.set(0,0);
}

void GuiCanvas::addUpdateRegion(Point2I pos, Point2I ext)
{
   if(mCurUpdateRect.extent.x == 0)
   {
      mCurUpdateRect.point = pos;
      mCurUpdateRect.extent = ext;
   }
   else
   {
      Point2I upperL;
      upperL.x = getMin(mCurUpdateRect.point.x, pos.x);
      upperL.y = getMin(mCurUpdateRect.point.y, pos.y);
      Point2I lowerR;
      lowerR.x = getMax(mCurUpdateRect.point.x + mCurUpdateRect.extent.x, pos.x + ext.x);
      lowerR.y = getMax(mCurUpdateRect.point.y + mCurUpdateRect.extent.y, pos.y + ext.y);
      mCurUpdateRect.point = upperL;
      mCurUpdateRect.extent = lowerR - upperL;
   }
}

void GuiCanvas::resetUpdateRegions()
{
   //DEBUG - get surface width and height
   mOldUpdateRects[0].set(mBounds.point, mBounds.extent);
   mOldUpdateRects[1] = mOldUpdateRects[0];
   mCurUpdateRect = mOldUpdateRects[0];
}

void GuiCanvas::setFirstResponder( GuiControl* newResponder )
{
	GuiControl* oldResponder = mFirstResponder;
	Parent::setFirstResponder( newResponder );

	if ( oldResponder && ( oldResponder != mFirstResponder ) )
		oldResponder->onLoseFirstResponder();
}
