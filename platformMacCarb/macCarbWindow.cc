//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformMacCarb/platformMacCarb.h"
#include "platformMacCarb/platformGL.h"
#include "platform/platform.h"
#include "platform/platformVideo.h"
#include "platformMacCarb/maccarbOGLVideo.h"
#include "platform/event.h"
#include "console/console.h"
#include "platformMacCarb/maccarbConsole.h"
#include "platform/platformInput.h"
//#include "platformMacCarb/maccarbInput.h"
#include "platform/gameInterface.h"
#include "math/mRandom.h"
#include "core/fileStream.h"
#include "game/resource.h"
#include "platformMacCarb/maccarbFileio.h"

//-------------------------------------- Mac System includes
#if defined(TARG_MACCARB) && (TARG_MACCARB>=0x0120)
#include <Carbon.h>
#else
#include <Quickdraw.h>
#include <Dialogs.h>
#endif

#include <stdio.h>

#if !__APPLE__
#include <SIOUX.h>
#endif

//-------------------------------------- Resource Includes
#include "dgl/gBitmap.h"
#include "dgl/gFont.h"

extern void createFontInit();
extern void createFontShutdown();

bool gWindowCreated = false;

#pragma message("todo: better timing abstraction")
#define GetTickCount()   GetMilliseconds()


MacCarbPlatState platState;

MacCarbPlatState::MacCarbPlatState()
{
   hDisplay    = NULL;
   appWindow   = NULL;
   drawable      = NULL;
   
   quit      = false;
   
   fmt         = NULL;
   ctx         = NULL;

	// start with something reasonable.
   desktopBitsPixel = 16;
   desktopWidth = 1024;
   desktopHeight = 768;

   volRefNum = 0;
   dirID     = 0;
   
   dStrcpy(appWindowTitle, "V12 Window");
}

static bool windowLocked = false;

static U8 keyboardState[256];
static bool mouseButtonState[3];
static bool capsLockDown = false;
static S32 modifierKeys = 0;
static bool windowActive = true;

static Point2I lastCursorPos(0,0);
static Point2I windowSize;
static bool sgDoubleByteEnabled = false;

static const char *getKeyName(S32 vkCode);

#define TICKS_IN_FRONT      0L
#define TICKS_IN_BACK      60L
static bool gBackgrounded = false;
static long gSleepTicks = TICKS_IN_FRONT;

//--------------------------------------
static const char *getMessageName(S32 msg)
{
   switch(msg)
   {
//      case WM_KEYDOWN:
//         return "WM_KEYDOWN";
//      case WM_KEYUP:
//         return "WM_KEYUP";
//      case WM_SYSKEYUP:
//         return "WM_SYSKEYUP";
//      case WM_SYSKEYDOWN:
//         return "WM_SYSKEYDOWN";
      default:
         return "Unknown!!";
   }
}


void Platform::AlertOK(const char *windowTitle, const char *message)
{
   S16 hit;
   Str255 title, desc;
   str2p(windowTitle, title);
   str2p(message, desc);

   Input::deactivate();

   StandardAlert(0, title, desc, NULL, &hit);
}

bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
   S16 hit;
   AlertStdAlertParamRec param;
   Str255 title, desc;
   str2p(windowTitle, title);
   str2p(message, desc);
   param.movable = false;
   param.helpButton = false;
   param.filterProc = NULL;
   param.defaultText = (StringPtr)kAlertDefaultOKText;
   param.cancelText = (StringPtr)kAlertDefaultCancelText;
   param.otherText = NULL;
   param.defaultButton = kAlertStdAlertOKButton;
   param.cancelButton = kAlertStdAlertCancelButton;
   param.position = kWindowDefaultPosition;
   StandardAlert(0, title, desc, &param, &hit);
   return (hit==kAlertStdAlertOKButton);
}

bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
   S16 hit;
   AlertStdAlertParamRec param;
   Str255 title, desc;
   Str255 retryStr;
   str2p(windowTitle, title);
   str2p(message, desc);
   str2p("Retry", retryStr);
   param.movable = false;
   param.helpButton = false;
   param.filterProc = NULL;
   param.defaultText = retryStr;
   param.cancelText = (StringPtr)kAlertDefaultCancelText;
   param.otherText = NULL;
   param.defaultButton = kAlertStdAlertOKButton;
   param.cancelButton = kAlertStdAlertCancelButton;
   param.position = kWindowDefaultPosition;
   StandardAlert(0, title, desc, &param, &hit);
   return (hit==kAlertStdAlertOKButton);
}

//--------------------------------------
static void InitInput()
{
   dMemset( keyboardState, 0, 256 );
   dMemset( mouseButtonState, 0, sizeof( mouseButtonState ) );
}

/*
extern Point MTemp      :   0x828;   // RawMouse and MTemp both contain the current absolute mouse position
extern Point RawMouse   :   0x82C;
extern Rect CrsrPin      :   0x834;
extern Byte CrsrNew      :   0x8CE;   // CrsrNew is a flag that tells the system when the mouse has changed
extern Byte CrsrCouple   :   0x8CF;   // CrsrCouple is what CrsrNew should be set to when the mouse has changed
*/

static Point& ClientToScreen( WindowPtr wnd, Point pt )
{
   static Point screen;
   screen = pt;
   
   GrafPtr savePort;
   GetPort( &savePort );
   SetPortWindowPort(platState.appWindow);
   LocalToGlobal( &screen );
   SetPort( savePort );
   
   return screen;
}


void GetWindowRect( WindowPtr wnd, RectI *pRect )
{
   // note - I should probably add a check for title bar, and borders to simulate Windows completely.
   if ( pRect && wnd ) 
   {
      GrafPtr port;
      Rect r;
      GetPort( &port );
      SetPortWindowPort(wnd);
      GetWindowPortBounds(wnd, &r);
      SetPort( port );
      
      pRect->point.x  = r.left;
      pRect->point.y  = r.top;
      pRect->extent.x = r.right - r.left;
      pRect->extent.y = r.bottom - r.top;
   } 
}


static void GetMousePos(Point *pt)
{
   GrafPtr savePort;
   GetPort( &savePort );
   SetPortWindowPort( platState.appWindow );
   GetMouse(pt);
   SetPort( savePort );
}

/*
static void SetCursorPos(const Point2I &pos)
{
   Point   pt;
   Rect deviceRect = (*platState.hDisplay)->gdRect;
      
   // window to screen space   
   //RectI r;
   //GetWindowRect(platState.appWindow, &r);
   pt.h = pos.x;
   pt.v = pos.y;
   pt = ClientToScreen(platState.appWindow, pt);
   
   // display to desktop ?
   //pt.h = deviceRect.left + pos.x;
   //pt.v = deviceRect.top + pos.y;

   MTemp = RawMouse = pt;
   CrsrNew = CrsrCouple;
}



//--------------------------------------
static void setMouseClipping()
{
//   ClipCursor(NULL);
   if(windowActive)
   {
//      ShowCursor(false);
      if(windowLocked)
      {
//         RECT r;
//         GetWindowRect(winState.appWindow, &r);
//         ClipCursor(&r);
//
         RectI r;
         GetWindowRect(platState.appWindow, &r);
         //r.point += r.extent / 2;
         r.extent /= 2;
         //S32 centerX = (r.right + r.left) >> 1;
         //S32 centerY = (r.bottom + r.top) >> 1;
         SetCursorPos(r.extent);
      }
   }
//   else
//      ShowCursor(true);
}
*/

//--------------------------------------
void Platform::enableKeyboardTranslation(void)
{
}

//--------------------------------------
void Platform::disableKeyboardTranslation(void)
{
}


//--------------------------------------
void Platform::setWindowLocked(bool locked)
{
   windowLocked = locked;
//   setMouseClipping();
}


//--------------------------------------
static bool HandleUpdate(WindowPtr w)
{
   if (w && w == platState.appWindow)
   {
      BeginUpdate(w);
      Game->refreshWindow();
      EndUpdate(w);
      return(true);
   }
   
   return(false);
}


//--------------------------------------
static void HandleActivate(WindowPtr w, bool winActive, bool appActive)
{
   if (w && w==platState.appWindow)
   {
      if (appActive)
      {
         Video::reactivate();
/*
         ShowCursor(false);
         if ( Video::isFullScreen() )
            hideTheTaskbar();
*/
      }
      else
      {
         Video::deactivate();
//         restoreTheTaskbar();
      }
      
      if (winActive)
      {
         Input::activate();
      }
      else
      {
         #pragma message("need to release input state on deactivate")
         // if any input captured -- release all input states so we don't errantly keep them locked... 
         Input::deactivate();
      }

      HandleUpdate(w);
   }
}


//--------------------------------------
//static void processKeyMessage(UINT message, WPARAM wParam, LPARAM lParam)
//{
//   S32 nVirtkey = wParam;
//   S32 scanCode = (lParam >> 16) & 0xff;
//   bool extended = (lParam >> 24) & 1;
//   bool repeat = (lParam >> 30) & 1;
//   bool make = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
//   
//   S32 newVirtKey = nVirtkey;
//   switch(nVirtkey)
//   {
//      case KEY_MENU:
//         if(extended)
//            newVirtKey = KEY_RMENU;
//         else
//            newVirtKey = KEY_LMENU;
//         break;
//      case KEY_CONTROL:
//         if(extended)
//            newVirtKey = KEY_RCONTROL;
//         else
//            newVirtKey = KEY_LCONTROL;
//         break;
//      case KEY_SHIFT:
//         if(scanCode == 54)
//            newVirtKey = KEY_RSHIFT;
//         else
//            newVirtKey = KEY_LSHIFT;
//         break;
//   }
//   
//   S32 modKey = modifierKeys;
//   
//   if(make)
//   {
//      switch (newVirtKey)
//      {
//         case KEY_LSHIFT:     modifierKeys |= SI_LSHIFT; modKey = 0; break; 
//         case KEY_RSHIFT:     modifierKeys |= SI_RSHIFT; modKey = 0; break;
//         case KEY_LCONTROL:   modifierKeys |= SI_LCTRL; modKey = 0; break;
//         case KEY_RCONTROL:   modifierKeys |= SI_RCTRL; modKey = 0; break;
//         case KEY_LMENU:      modifierKeys |= SI_LALT; modKey = 0; break;
//         case KEY_RMENU:      modifierKeys |= SI_RALT; modKey = 0; break;
//      }
//      if(nVirtkey == VK_CAPITAL)
//      {
//         capsLockDown = !capsLockDown;
//         if(capsLockDown)
//            keyboardState[nVirtkey] |= 0x01;
//         else
//            keyboardState[nVirtkey] &= 0xFE;
//      }
//      keyboardState[nVirtkey] |= 0x80;
//   }
//   else
//   {
//      switch (newVirtKey)
//      {
//         case KEY_LSHIFT:     modifierKeys &= ~SI_LSHIFT; modKey = 0; break; 
//         case KEY_RSHIFT:     modifierKeys &= ~SI_RSHIFT; modKey = 0; break;
//         case KEY_LCONTROL:   modifierKeys &= ~SI_LCTRL; modKey = 0; break;
//         case KEY_RCONTROL:   modifierKeys &= ~SI_RCTRL; modKey = 0; break;
//         case KEY_LMENU:      modifierKeys &= ~SI_LALT; modKey = 0; break;
//         case KEY_RMENU:      modifierKeys &= ~SI_RALT; modKey = 0; break;
//      }
//      keyboardState[nVirtkey] &= 0x7f;
//   }
//
//   WORD ascii = 0;
//   S32 res = ToAscii(nVirtkey, (lParam >> 16) & 0xFF, keyboardState, &ascii, 0);
//   if(res < 1)
//      ascii = 0;
//
//   InputEvent event;
//   
//   event.deviceInst = 0;
//   event.deviceType = KeyboardDeviceType;
//   event.objType = SI_KEY;
//   event.objInst = newVirtKey;
//   event.action = make ? (repeat ? SI_REPEAT : SI_MAKE ) : SI_BREAK;
//   event.modifier = modKey;
//   event.ascii = ascii;
//   event.fValue = make ? 1.0 : 0.0;
//   
//   //   printf("%s - %s (%d) count: %d ext: %s char: %c\n",
//   //         getMessageName(message), getKeyName(nVirtkey), scanCode,
//   //         repeat, extended ? "True" : "False", ascii);
//   GamePostEvent(event);
//}

//--------------------------------------
static Point lastPos = {-11,-11}; // something unlikely...
static void CheckCursorPos()
{
   static bool reset = true;

   if (!windowActive || gBackgrounded) //!!!!TBD windowActive/Locked should go false if bg.
   {
      reset = true;
      return;
   }
   
   Point mousePos;
   GetMousePos(&mousePos);

   if (!windowLocked)
   {
      // since we're in Poll mode, we need to post a mousemove event for the cursor to be updated.
      MouseMoveEvent event;
      event.xPos = mousePos.h;     // horizontal position of cursor 
      event.yPos = mousePos.v;     // vertical position of cursor 
      event.modifier = modifierKeys;
      Game->postEvent(event);
   }
   else
   {
      if (reset)
         reset = false;
      else
      if (mousePos.h!=lastPos.h
      &&  mousePos.v!=lastPos.v) // mouse moved.
      {
         if(mousePos.h != lastPos.h)
         {
            InputEvent event;

            event.deviceInst = 0;
            event.deviceType = MouseDeviceType;
            event.objType = SI_XAXIS;
            event.objInst = 0;
            event.action = SI_MOVE;
            event.modifier = modifierKeys;
            event.ascii = 0;
            event.fValue = F32(mousePos.h - lastPos.h);
            Game->postEvent(event);
//            Con::printf( "EVENT: Mouse move (%.1f, 0.0).\n", event.fValue );
         }

         if(mousePos.v != lastPos.v)
         {
            InputEvent event;

            event.deviceInst = 0;
            event.deviceType = MouseDeviceType;
            event.objType = SI_YAXIS;
            event.objInst = 0;
            event.action = SI_MOVE;
            event.modifier = modifierKeys;
            event.ascii = 0;
            event.fValue = F32(mousePos.v - lastPos.v);
            Game->postEvent(event);
//           Con::printf( "EVENT: Mouse move (0.0, %.1f).\n", event.fValue );
         }

      }

      lastPos = mousePos; // copy over last position.
   }
}


//--------------------------------------
//static void mouseButtonEvent(S32 action, S32 objInst)
//{
//   action, objInst;
//   CheckCursorPos();
//   if(!windowLocked)
//   {
//      if(action == SI_MAKE)
//         SetCapture(winState.appWindow);
//      else
//         ReleaseCapture();
//   }
//
//   InputEvent event;
//
//   event.deviceInst = 0;
//   event.deviceType = MouseDeviceType;
//   event.objType = SI_BUTTON;
//   event.objInst = objInst;
//   event.action = action;
//   event.modifier = modifierKeys;
//   event.ascii = 0;
//   event.fValue = action == SI_MAKE ? 1.0 : 0.0;
//   
//   Game->postEvent(event);
//}


#define WINDOW_RESTRICT_EVENTS      0

static void ProcessKeyboard( EventRecord &msg )
{
#if WINDOW_RESTRICT_EVENTS
   WindowPtr which;
   FindWindow( msg.where, &which );
   if (which != platState.appWindow)
      return;
#endif

#if ALLOW_MENU_PROCESSING
   unsigned char c;
   c = (msg.message & charCodeMask);
   if ((msg.modifiers & cmdKey) != 0)
   {
      AdjustMenus();
      HandleMenuKey(c, msg.modifiers);
   }
#endif
   
   InputEvent event;
   event.deviceInst = 0;
   event.deviceType = KeyboardDeviceType;
   event.objType = SI_KEY;
   event.objInst = TranslateOSKeyCode( (msg.message & keyCodeMask) >> 8 );
   event.ascii   = msg.message & charCodeMask;
   
   switch(msg.what)
   {
      case keyDown:
         event.action = SI_MAKE;
         event.fValue = 1.0f;
         //Con::printf("keyDN ( %02x )", (msg.message & keyCodeMask) >> 8);
         break;
         
      case autoKey:
         event.action = SI_REPEAT;
         event.fValue = 1.0f;
         break;
         
      case keyUp:
         event.action = SI_BREAK;
         event.fValue = 0.0f;
         //Con::printf("keyUP ( %02x )", (msg.message & keyCodeMask) >> 8);   
         break;
   }   
   
   event.modifier = 0;
   if (msg.modifiers & shiftKey)          event.modifier |= SI_LSHIFT;
   if (msg.modifiers & rightShiftKey)     event.modifier |= SI_RSHIFT;
   if (msg.modifiers & cmdKey)            event.modifier |= SI_LALT;
   if (msg.modifiers & optionKey)         event.modifier |= SI_MAC_LOPT;
   if (msg.modifiers & rightOptionKey)    event.modifier |= SI_MAC_ROPT;
   if (msg.modifiers & controlKey)        event.modifier |= SI_LCTRL;
   if (msg.modifiers & rightControlKey)   event.modifier |= SI_RCTRL;

   // handle command-Q exit
   if ((event.objInst == KEY_Q) && (msg.modifiers & cmdKey))
      Platform::postQuitMessage(0);

   Game->postEvent(event);
}


static void ProcessMouse( EventRecord &msg )
{
   WindowPtr which;
   U16 where = FindWindow( msg.where, &which );
   if (which != platState.appWindow)
      return;

   SelectWindow(platState.appWindow);
   
   // handle a little window maintence
   switch (where)
   {
#if !TARGET_API_MAC_CARBON
      case inSysWindow:
         SystemClick ( &msg, platState.appWindow );
         return;
#endif
      
      case inDrag:
      {
         RgnHandle rgn = GetGrayRgn();
         Rect r;
         GetRegionBounds(rgn, &r);
         SetPortWindowPort(platState.appWindow);
         DragWindow(platState.appWindow, msg.where, &r);
         if (platState.ctx)
            aglUpdateContext(platState.ctx);
         HandleUpdate(platState.appWindow);
         return;
      }   
         
      case inContent:
         // will handle below.
         break;

      default:
         return;
   }
   
   
   InputEvent event;
   
   event.deviceInst = 0;
   event.deviceType = MouseDeviceType;
   event.objType    = SI_BUTTON;
   event.objInst    = KEY_BUTTON0;      // always button 0 for now
   event.modifier = 0;
   event.ascii    = 0;
   if (msg.what == mouseDown)
   {
      event.action = SI_MAKE;
      event.fValue = 1.0;
   }
   else
   {
      event.action = SI_BREAK;
      event.fValue = 1.0;
   }
   Game->postEvent(event);
}


//--------------------------------------
static bool ProcessMessages()
{
   if (platState.quit)
      return false;
   
   SetEventMask(everyEvent);

   EventRecord msg;
   bool done = false;
   while(!done && WaitNextEvent(everyEvent, &msg, 0, NULL))
   {
      bool handled = (gConsole && gConsole->handleEvent(&msg));
      if (!handled)
      switch(msg.what)
      {
          case nullEvent:
            //AdjustCursor(msg.where, ((msg.modifiers&optionKey)!=0), cursorRgn);
             done = true;
             break;
             
         case keyDown:
//         case autoKey:
         case keyUp:
            ProcessKeyboard(msg);
            break;
            
          case mouseDown:
          case mouseUp:
             ProcessMouse(msg);
             break;
             
         case activateEvt:
            HandleActivate((WindowPtr)(msg.message), (msg.modifiers & activeFlag) != 0, !gBackgrounded);
            break;

          case updateEvt:
            HandleUpdate((WindowPtr)(msg.message));
             break;
          
          case osEvt:
          {
            switch ((unsigned long)(msg.message >> 24)) // osEvt msg is in the high byte.
            {
               case mouseMovedMessage: // this is it moved from the region passed to WNE
               {
//                  AdjustCursor(msg.where, ((msg.modifiers&optionKey)!=0), cursorRgn);
                  break;
               }
               
               case suspendResumeMessage:
               {
                  Cursor arrow; 
                  GetQDGlobalsArrow(&arrow);
                  
                  gBackgrounded = (msg.message & resumeFlag) == 0;
                  HandleActivate(platState.appWindow, !gBackgrounded, !gBackgrounded);
                  gSleepTicks = (gBackgrounded?TICKS_IN_BACK:TICKS_IN_FRONT);
                  
                  if (gBackgrounded)   /* just suspended */
                     SetCursor(&arrow);
                  else                  /* just resumed */
                     SetCursor(&arrow);
                  break;
               }/* end case suspend/resume evt */
            }
            
             break;
          }
          
         case kHighLevelEvent:
         {
            // the type of message is stored in the where Point... so cast it
            U32 hlWhat = *((U32*)(&msg.where));   
            if ( hlWhat == kAEQuitApplication )
            {
               Platform::postQuitMessage(0);
               return false;
            }
            else 
               AEProcessAppleEvent(&msg);
            break;
         }
            
         default:
         //   Con::printf("%d %08x", msg.what, msg.what);
            break;
      }
   }
   return true;
}


//--------------------------------------
void Platform::process()
{
   extern bool gMouseActive;
   if (!Input::isActive() || !gMouseActive)
      CheckCursorPos();

   gConsole->process();
   
   if(!ProcessMessages())
   {
      // generate a quit event
      Event quitEvent;
      quitEvent.type = QuitEventType;
      
      Game->postEvent(quitEvent);
   }

/*
   // if there's no window, we sleep 1, otherwise we sleep 0
   if(!Game->isJournalReading())
      Sleep(gWindowCreated ? 0 : 1); // give others some process time if necessary...
   HWND window = GetForegroundWindow();
   if (window && gWindowCreated) 
   {
      // check to see if we are in the foreground or not
      // if not Sleep for 100ms or until a Win32 message/input is recieved
      DWORD foregroundProcessId;
      GetWindowThreadProcessId(window, &foregroundProcessId);
      if (foregroundProcessId != winState.processId)
         MsgWaitForMultipleObjects(0, NULL, false, 100, QS_ALLINPUT);
   }

*/

   Input::process();
}

extern U32 calculateCRC(void * buffer, S32 len, U32 crcVal );

#if defined(DEBUG) || defined(INTERNAL_RELEASE)
static U32 stubCRC = 0;
#else
static U32 stubCRC = 0xEA63F56C;
#endif

//--------------------------------------
/*
static void InitWindowClass()
{
   WNDCLASS wc;
   dMemset(&wc, 0, sizeof(wc));
   
   wc.style         = CS_OWNDC;
   wc.lpfnWndProc   = WindowProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = winState.appInstance;
   wc.hIcon         = LoadIcon(winState.appInstance, MAKEINTRESOURCE(IDI_ICON2));
   wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
   wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
   wc.lpszMenuName  = 0;
   wc.lpszClassName = windowClassName;
   RegisterClass( &wc );
   
   // Curtain window class:
   wc.lpfnWndProc   = DefWindowProc;
   wc.hCursor       = NULL;
   wc.hbrBackground = (HBRUSH) GetStockObject(GRAY_BRUSH);
   wc.lpszClassName = "Curtain";
   RegisterClass( &wc );
}
*/

//--------------------------------------
static void GetDesktopState()
{
   platState.hDisplay = GetMainDevice();
   platState.desktopBitsPixel = (*(*(platState.hDisplay))->gdPMap)->pixelSize;
   Rect r = (*(platState.hDisplay))->gdRect;
   platState.desktopWidth = r.right - r.left;
   platState.desktopHeight = r.bottom - r.top;
}



//--------------------------------------
WindowPtr CreateOpenGLWindow( U32 width, U32 height, bool fullScreen )
{
   int offset = 144; // some nice starting offset.
   
   // for now just get first screen device.
   //!!!!!tbd - this needs to be in sync with what setupgl picks!!!!
   platState.hDisplay = GetMainDevice();

   // this rect should really be based off the device coords... !!!!!TBD   
   Rect   rect;
   SetRect( &rect, offset+48, offset, width+offset+48, height+offset);
   
   CWindowPtr w;
   w = NewCWindow(NULL, 
                  &rect,                                                    // bounding rect
                  str2p(platState.appWindowTitle),                         // window title
                  false,                                                   // is visible
                  fullScreen?kWindowPlainDialogProc:kWindowDocumentProc,   // window type
                  (WindowPtr) -1L,                                          // top most window
                  false,                                                    // has a close box
                  0L);                                                      // reference constant

   if (w != NULL)
      ShowWindow(w);

   return(w);
}

//--------------------------------------
WindowPtr CreateCurtain( U32 width, U32 height )
{
   WindowPtr w=NULL;
/*   
   w =  CreateWindow( 
      "Curtain",
      "",
      ( WS_POPUP | WS_MAXIMIZE | WS_VISIBLE ),
      0, 0,
      width, height,
      NULL, NULL,
      winState.appInstance,
      NULL );
*/
   return(w);
}


/*
//--------------------------------------
void CreatePixelFormat( PIXELFORMATDESCRIPTOR *pPFD, S32 colorBits, S32 depthBits, S32 stencilBits, bool stereo )
{
   PIXELFORMATDESCRIPTOR src = 
   {
      sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
      1,                      // version number
      PFD_DRAW_TO_WINDOW |    // support window
      PFD_SUPPORT_OPENGL |    // support OpenGL
      PFD_DOUBLEBUFFER,       // double buffered
      PFD_TYPE_RGBA,          // RGBA type
      colorBits,              // color depth
      0, 0, 0, 0, 0, 0,       // color bits ignored
      0,                      // no alpha buffer
      0,                      // shift bit ignored
      0,                      // no accumulation buffer
      0, 0, 0, 0,             // accum bits ignored
      depthBits,              // z-buffer   
      stencilBits,            // stencil buffer
      0,                      // no auxiliary buffer
      PFD_MAIN_PLANE,         // main layer
      0,                      // reserved
      0, 0, 0                 // layer masks ignored
    };

   if ( stereo )
   {
      //ri.Printf( PRINT_ALL, "...attempting to use stereo\n" );
      src.dwFlags |= PFD_STEREO;
      //glConfig.stereoEnabled = true;
   }
   else
   {
      //glConfig.stereoEnabled = qfalse;
   }
   *pPFD = src;
}

//--------------------------------------
enum { MAX_PFDS = 256 };

S32 ChooseBestPixelFormat(HDC hDC, PIXELFORMATDESCRIPTOR *pPFD)
{
   PIXELFORMATDESCRIPTOR pfds[MAX_PFDS+1];
   S32 i;
   S32 bestMatch = 0;
   
   S32 maxPFD = qwglDescribePixelFormat(hDC, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfds[0]);
   if(maxPFD > MAX_PFDS)
      maxPFD = MAX_PFDS;

   bool accelerated = false;
   
   for(i = 1; i <= maxPFD; i++)
   {
      qwglDescribePixelFormat(hDC, i, sizeof(PIXELFORMATDESCRIPTOR), &pfds[i]);

      // make sure this has hardware acceleration:
      if ( ( pfds[i].dwFlags & PFD_GENERIC_FORMAT ) != 0 ) 
         continue;

      // verify pixel type
      if ( pfds[i].iPixelType != PFD_TYPE_RGBA )
         continue;

      // verify proper flags
      if ( ( ( pfds[i].dwFlags & pPFD->dwFlags ) & pPFD->dwFlags ) != pPFD->dwFlags ) 
         continue;

      accelerated = !(pfds[i].dwFlags & PFD_GENERIC_FORMAT);

      //
      // selection criteria (in order of priority):
      // 
      //  PFD_STEREO
      //  colorBits
      //  depthBits
      //  stencilBits
      //
      if ( bestMatch )
      {
         // check stereo
         if ( ( pfds[i].dwFlags & PFD_STEREO ) && ( !( pfds[bestMatch].dwFlags & PFD_STEREO ) ) && ( pPFD->dwFlags & PFD_STEREO ) )
         {
            bestMatch = i;
            continue;
         }
         
         if ( !( pfds[i].dwFlags & PFD_STEREO ) && ( pfds[bestMatch].dwFlags & PFD_STEREO ) && ( pPFD->dwFlags & PFD_STEREO ) )
         {
            bestMatch = i;
            continue;
         }

         // check color
         if ( pfds[bestMatch].cColorBits != pPFD->cColorBits )
         {
            // prefer perfect match
            if ( pfds[i].cColorBits == pPFD->cColorBits )
            {
               bestMatch = i;
               continue;
            }
            // otherwise if this PFD has more bits than our best, use it
            else if ( pfds[i].cColorBits > pfds[bestMatch].cColorBits )
            {
               bestMatch = i;
               continue;
            }
         }

         // check depth
         if ( pfds[bestMatch].cDepthBits != pPFD->cDepthBits )
         {
            // prefer perfect match
            if ( pfds[i].cDepthBits == pPFD->cDepthBits )
            {
               bestMatch = i;
               continue;
            }
            // otherwise if this PFD has more bits than our best, use it
            else if ( pfds[i].cDepthBits > pfds[bestMatch].cDepthBits )
            {
               bestMatch = i;
               continue;
            }
         }

         // check stencil
         if ( pfds[bestMatch].cStencilBits != pPFD->cStencilBits )
         {
            // prefer perfect match
            if ( pfds[i].cStencilBits == pPFD->cStencilBits )
            {
               bestMatch = i;
               continue;
            }
            // otherwise if this PFD has more bits than our best, use it
            else if ( ( pfds[i].cStencilBits > pfds[bestMatch].cStencilBits ) && 
                ( pPFD->cStencilBits > 0 ) )
            {
               bestMatch = i;
               continue;
            }
         }
      }
      else
      {
         bestMatch = i;
      }
   }
   
   if ( !bestMatch )
      return 0;

   else if ( pfds[bestMatch].dwFlags & PFD_GENERIC_ACCELERATED )
   {
      // MCD
   }
   else
   {
      // ICD
   }

   *pPFD = pfds[bestMatch];

   return bestMatch;
}
*/


//--------------------------------------
const Point2I &Platform::getWindowSize()
{
   return windowSize;
}


//--------------------------------------
void Platform::setWindowSize( U32 newWidth, U32 newHeight )
{
   windowSize.set( newWidth, newHeight );
}


//--------------------------------------
// !!!!!TBD!!!!! what should this do on the Mac.
void Platform::minimizeWindow()
{
}


//--------------------------------------
static void InitWindow(const Point2I &initialSize)
{
   windowSize = initialSize;   
}


//--------------------------------------
static void InitOpenGL()
{
   // Just for kicks.  Seems a relatively central place to put this...
#if !__APPLE__
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif

   DisplayDevice::init();

   // Get the video settings from the prefs:
   const char* resString = Con::getVariable( "$pref::Video::resolution" );
   char* tempBuf = new char[dStrlen( resString ) + 1];
   dStrcpy( tempBuf, resString );
   char* temp = dStrtok( tempBuf, " x\0" );
   U32 width = ( temp ? dAtoi( temp ) : 800 );
   temp = dStrtok( NULL, " x\0" );
   U32 height = ( temp ? dAtoi( temp ) : 600 );
   temp = dStrtok( NULL, "\0" );
   U32 bpp = ( temp ? dAtoi( temp ) : 16 );
   delete [] tempBuf;

   bool fullScreen = Con::getBoolVariable( "$pref::Video::fullScreen" );

   if ( !Video::setDevice( Con::getVariable( "$pref::Video::displayDevice" ), width, height, bpp, fullScreen ) )
   {
      // Next, try the default OpenGL device:
      if ( !Video::setDevice( "OpenGL", width, height, bpp, fullScreen ) )
      {
           AssertFatal( false, "Could not find a compatible display device!" );
           return;
      }
   }
}


//--------------------------------------
ConsoleFunction( getDesktopResolution, const char*, 1, 1, "getDesktopResolution()" )
{
   argc; argv;
   char buffer[256];
   dSprintf( buffer, sizeof( buffer ), "%d %d %d", platState.desktopWidth, platState.desktopHeight, platState.desktopBitsPixel );
   char* returnString = Con::getReturnBuffer( dStrlen( buffer ) + 1 );
   dStrcpy( returnString, buffer );
   return( returnString ); 
}


//--------------------------------------
void Platform::init()
{
   // Set the platform variable for the scripts
   Con::setVariable( "$platform", "maccarb" );

   MacConsole::create();
   if ( !MacConsole::isEnabled() )
      Input::init();
   InitInput();   // in case Input setup falls through.

   GetDesktopState();
//   installRedBookDevices();

// not sure why this is here on mac.
   Video::init();
   Video::installDevice( OpenGLDevice::create() );

   sgDoubleByteEnabled = true; // !!!!! this right?
//   sgQueueEvents = true;
}

//--------------------------------------
void Platform::shutdown()
{
//   sgQueueEvents = false;

//   if(gMutexHandle)
//      CloseHandle(gMutexHandle);
   setWindowLocked( false );
   Video::destroy();
   Input::destroy();
   MacConsole::destroy();
}


//--------------------------------------

static U32 lastTimeTick;

//--------------------------------------
static S32 run(S32 argc, const char **argv)
{
   createFontInit();
   windowSize.set(0,0);
   
   lastTimeTick = GetTickCount();
   
//   TribesGame *tmpgame = new TribesGame;
   int ret = Game->main(argc, argv);
   createFontShutdown();
   return ret;
}

void Platform::initWindow(const Point2I &initialSize, const char *name)
{
   Con::printf( "Video Init:" );
   Video::init();
   if ( Video::installDevice( OpenGLDevice::create() ) )
      Con::printf( "   Accelerated OpenGL display device detected." );
   else
      Con::printf( "   Accelerated OpenGL display device not detected." );
   Con::printf( "" );

   dSprintf(platState.appWindowTitle, sizeof(platState.appWindowTitle), name);
   InitWindow(initialSize);
   InitOpenGL();
   gWindowCreated = true;
}

//--------------------------------------
S32 main(S32 argc, const char **argv)
{
   // mac does not support command line arguments
   // it may be possible to get the comment field from the file
   // see DesktopManager and DTGetComment

#if !TARGET_API_MAC_CARBON   
   InitGraf(&qd.thePort);      // init QuickDraw -- 'qd' is a Mac Global
   InitFonts();               // init the Font Manager
   InitWindows();               // init the Window Manager
   InitMenus();
   TEInit();
   InitDialogs( NULL );
#endif
   InitCursor();
   
   FlushEvents( everyEvent, 0 );
   SetEventMask(everyEvent);

   // save away home directory info into platState.
   macGetHomeDirectory();

   return run(argc, argv);
}

//--------------------------------------
void TimeManager::process()
{
   U32 curTime = GetTickCount();
   TimeEvent event;
   event.elapsedTime = curTime - lastTimeTick;
   if(event.elapsedTime > 5)
   {
      lastTimeTick = curTime;
      Game->postEvent(event);
   }
}

/*
GLimp_Init
   GLW_LoadOpenGL
      QGL_Init(driver);
      GLW_StartDriverAndSetMode
         GLW_SetMode
            ChangeDisplaySettings
            GLW_CreateWindow
               GLW_InitDriver
                  GLW_CreatePFD
                  GLW_MakeContext
                     GLW_ChoosePFD
                     DescribePixelFormat
                     SetPixelFormat
                  
   GLW_InitExtensions
   WG_CheckHardwareGamma
*/

//--------------------------------------
#pragma message("todo: implement random")
F32 Platform::getRandom()
{
   //return rand() / F32(RAND_MAX);
   return 0.5;
}


//--------------------------------------
// Web browser function:
//--------------------------------------
bool Platform::openWebBrowser( const char* webAddress )
{//!!!!!!! TBD
   return(false);
}