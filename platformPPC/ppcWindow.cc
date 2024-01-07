//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include "PlatformWin32/platformGL.h"
#include "Platform/platform.h"
#include "Platform/platformVideo.h"
#include "PlatformPPC/ppcOGLVideo.h"
#include "Platform/event.h"
#include "console/console.h"
#include "PlatformPPC/ppcConsole.h"

//-------------------------------------- Mac System includes
#include <Fonts.h>
#include <dialogs.h>

//#include <Devices.h>
#include <Displays.h>


#include <stdio.h>

//-------------------------------------- Resource Includes
#include "dgl/gBitmap.h"
#include "dgl/gFont.h"

extern void createFontInit();
extern void createFontShutdown();

PPCPlatState ppcState;

PPCPlatState::PPCPlatState()
{
   hDisplay    = NULL;
	appWindow	= NULL;
	quit			= false;
	
	fmt			= NULL;
	ctx			= NULL;
	
	volRefNum = 0;
	dirID     = 0;
	
	dStrcpy(appWindowTitle, "Darkstar Window");
	
//   log_fp      = NULL;
//   hinstOpenGL = NULL;
//   hinstGLU    = NULL;
//   appWindow   = NULL;
//   appDC       = NULL;
//   appInstance = NULL;
}

static bool windowLocked = false;

static U8 keyboardState[256];
static bool capsLockDown = false;
static S32 modifierKeys = 0;
static bool windowActive = true;

static const char *getKeyName(S32 vkCode);
static Point2I windowSize;


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
   windowTitle, message;
   //messageBox(message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OK);
}

bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
   windowTitle, message;
   return true;
   //return messageBox(message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OKCANCEL) == IDOK;
}

bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
   windowTitle, message;
   return true;
   //return messageBox(message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_RETRYCANCEL) == IDRETRY);
}

//--------------------------------------
static void InitInput()
{
   dMemset( keyboardState, 0, 256 );
}

extern Point MTemp		:	0x828;   // RawMouse and MTemp both contain the current absolute mouse position
extern Point RawMouse	:	0x82C;
extern Rect CrsrPin		:	0x834;
extern Byte CrsrNew		:	0x8CE;   // CrsrNew is a flag that tells the system when the mouse has changed
extern Byte CrsrCouple	:	0x8CF;   // CrsrCouple is what CrsrNew should be set to when the mouse has changed

static Point& ClientToScreen( WindowPtr wnd, Point pt )
{
	static Point screen;
	screen = pt;
	
	GrafPort *savePort;
	GetPort( &savePort );
	SetPort( ppcState.appWindow );
	LocalToGlobal( &screen );
	SetPort( savePort );
	
	return screen;
}


void GetWindowRect( WindowPtr wnd, RectI *pRect )
{
	// note - I should probably add a check for title bar, and borders to simulate Windows completely.
	if ( pRect && wnd ) 
	{
	   GrafPort *port;
	   GetPort( &port );
      SetPort( wnd );
		Rect r = wnd->portRect;
		SetPort( port );
		
		pRect->point.x  = r.left;
		pRect->point.y  = r.top;
		pRect->extent.x = r.right - r.left;
		pRect->extent.y = r.bottom - r.top;

		pRect->point.x  = 0;
		pRect->point.y  = 0;
		pRect->extent.x = 800;
		pRect->extent.y = 600;
		
		
		
	} 
}


static void GetMousePos(Point *pt)
{
	GrafPort *savePort;
	GetPort( &savePort );
	SetPort( ppcState.appWindow );
	GetMouse(pt);
	SetPort( savePort );
}

static void SetCursorPos(const Point2I &pos)
{
	Point	pt;
	Rect deviceRect = (*ppcState.hDisplay)->gdRect;
		
	// window to screen space	
	//RectI r;
   //GetWindowRect(ppcState.appWindow, &r);
   pt.h = pos.x;
   pt.v = pos.y;
   pt = ClientToScreen(ppcState.appWindow, pt);
	
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
         GetWindowRect(ppcState.appWindow, &r);
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

void Platform::setWindowLocked(bool locked)
{
   windowLocked = locked;
//   setMouseClipping();
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

static S32 mouseX = 0xFFFFFFFF;
static S32 mouseY = 0xFFFFFFFF;
//--------------------------------------
static void CheckCursorPos()
{
   if(windowLocked && windowActive)
   {
      Point mousePos;
      GetMousePos(&mousePos);
      RectI r;

      GetWindowRect(ppcState.appWindow, &r);
      r.extent /= 2;   
//      S32 centerX = (r.right + r.left) >> 1;
//      S32 centerY = (r.bottom + r.top) >> 1;
//
      if(mousePos.h != r.extent.x)
      {
         InputEvent event;

         event.deviceInst = 0;
         event.deviceType = MouseDeviceType;
         event.objType = SI_XAXIS;
         event.objInst = 0;
         event.action = SI_MOVE;
         event.modifier = modifierKeys;
         event.ascii = 0;
         event.fValue = mousePos.h - r.extent.x;
         GamePostEvent(event);
      }
      if(mousePos.v != r.extent.y)
      {
         InputEvent event;

         event.deviceInst = 0;
         event.deviceType = MouseDeviceType;
         event.objType = SI_YAXIS;
         event.objInst = 0;
         event.action = SI_MOVE;
         event.modifier = modifierKeys;
         event.ascii = 0;
         event.fValue = mousePos.v - r.extent.y;
         GamePostEvent(event);
      }
      SetCursorPos(r.extent);
   }
}


//--------------------------------------
//static void mouseButtonEvent(S32 action, S32 objInst)
//{
//	action, objInst;
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
//   GamePostEvent(event);
//}


static void ProcessKeyboard( EventRecord &msg )
{
	WindowPtr which;
	FindWindow( msg.where, &which );
	if (which != ppcState.appWindow)
		return;
	
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
	//if (msg.modifiers & cmdKey) 	  event.modifier |= SI_COMMAND;
	if (msg.modifiers & shiftKey)   		 event.modifier |= SI_LSHIFT;
	if (msg.modifiers & rightShiftKey)   event.modifier |= SI_RSHIFT;
	if (msg.modifiers & optionKey)  		 event.modifier |= SI_LALT;
	if (msg.modifiers & rightOptionKey)  event.modifier |= SI_RALT;
	if (msg.modifiers & controlKey) 		 event.modifier |= SI_LCTRL;
	if (msg.modifiers & rightControlKey) event.modifier |= SI_RCTRL;

	// handle command-period exit
	if (event.ascii == '.' && (msg.modifiers & cmdKey))
		Platform::postQuitMessage(0);

	// fake right mose button with command-x for now
	#pragma message("todo: remove")
	if (event.ascii == '`')
	{
   	event.deviceInst = 0;
   	event.deviceType = MouseDeviceType;
   	event.objType 	= SI_BUTTON;
   	event.objInst 	= KEY_BUTTON1;		
   	event.modifier = 0;
   	event.ascii 	= 0;
	}
   GamePostEvent(event);
}


static void ProcessMouse( EventRecord &msg )
{
	WindowPtr which;
	U16 where = FindWindow( msg.where, &which );
	if (which != ppcState.appWindow)
		return;
	
	// handle a little window maintence
	switch (where)
	{
		case inSysWindow:
			SystemClick ( &msg, ppcState.appWindow );
			return;
		
		case inDrag: {
			RgnHandle rgn = GetGrayRgn();
			DragWindow(ppcState.appWindow, ClientToScreen(ppcState.appWindow, msg.where), &((*rgn)->rgnBBox));
			return;
			}		
		default:
			return;
			
		case inContent:
			break;
	}
	
	
   InputEvent event;
   
   event.deviceInst = 0;
   event.deviceType = MouseDeviceType;
   event.objType 	= SI_BUTTON;
   event.objInst 	= KEY_BUTTON0;		// always button 0 for now
   event.modifier = 0;
   event.ascii 	= 0;
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
   GamePostEvent(event);
}


//--------------------------------------
static bool ProcessMessages()
{
	if (ppcState.quit)
		return false;
	
	// the mouse position is polled on the Mac
   if(!windowLocked)
   {
   	Point mouseLoc;
		GetMousePos( &mouseLoc );
		
   	MouseMoveEvent event;
      event.xPos = mouseLoc.h;  	// horizontal position of cursor 
      event.yPos = mouseLoc.v;  	// vertical position of cursor 
      event.modifier = modifierKeys;
      GamePostEvent(event);
   }
   
   SetEventMask(everyEvent);

	EventRecord msg;
	bool done = false;
	while(!done && WaitNextEvent(everyEvent, &msg, 0, NULL))
	{	
		switch(msg.what)
		{
	 		case nullEvent:
	 			done = true;
	 			break;
	 			
			case keyDown:
			case autoKey:
			case keyUp:
				ProcessKeyboard(msg);
				break;
				
	 		case mouseDown:
	 		case mouseUp:
	 			ProcessMouse(msg);
	 			break;
	 			
	 		case updateEvt:
	 			BeginUpdate(ppcState.appWindow);
	 			EndUpdate(ppcState.appWindow);
	 			break;
	 			
			case kHighLevelEvent:{
				// the type of message is stored in the where Point... so cast it
				U32 hlWhat = *((U32*)(&msg.where));	
				if ( hlWhat == kAEQuitApplication )
				{
					return false;
				}
				else 
					AEProcessAppleEvent(&msg);
				}
				break;
				
			//case activateEvt:
			//	SelectWindow(ppcState.appWindow);
			//	break;
				
			//default:
			//   Con::printf("%d %08x", msg.what, msg.what);
		}
	}
   return true;
}


//--------------------------------------
void Platform::process()
{
   CheckCursorPos();
   WindowsConsole->process();
   
   if(!ProcessMessages())
   {
      // generate a quit event
      Event quitEvent;
      quitEvent.type = QuitEventType;
      
      GamePostEvent(quitEvent);
   }
}

//static void InitWindowClass()
//{
//   WNDCLASS wc;
//   memset(&wc, 0, sizeof(wc));
//  
//   wc.style = 0;
//   wc.lpfnWndProc = WindowProc;
//   wc.cbClsExtra    = 0;
//   wc.cbWndExtra    = 0;
//   wc.hInstance     = winState.appInstance;
//   wc.hIcon         = 0;
//   wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
//   wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
//   wc.lpszMenuName  = 0;
//   wc.lpszClassName = windowClassName;
//   RegisterClass( &wc );
//}

enum { MAX_PFDS = 256 };


//--------------------------------------
//static void CreatePixelFormat( PIXELFORMATDESCRIPTOR *pPFD, S32 colorbits, S32 depthbits, S32 stencilbits, bool stereo )
//{
//   PIXELFORMATDESCRIPTOR src = 
//   {
//      sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
//      1,                        // version number
//      PFD_DRAW_TO_WINDOW |         // support window
//      PFD_SUPPORT_OPENGL |         // support OpenGL
//      PFD_DOUBLEBUFFER,            // double buffered
//      PFD_TYPE_RGBA,               // RGBA type
//      24,                        // 24-bit color depth
//      0, 0, 0, 0, 0, 0,            // color bits ignored
//      0,                        // no alpha buffer
//      0,                        // shift bit ignored
//      0,                        // no accumulation buffer
//      0, 0, 0, 0,                // accum bits ignored
//      24,                        // 24-bit z-buffer   
//      8,                        // 8-bit stencil buffer
//      0,                        // no auxiliary buffer
//      PFD_MAIN_PLANE,               // main layer
//      0,                        // reserved
//      0, 0, 0                     // layer masks ignored
//    };
//
//   src.cColorBits = colorbits;
//   src.cDepthBits = depthbits;
//   src.cStencilBits = stencilbits;
//
//   if ( stereo )
//   {
//      //ri.Printf( PRINT_ALL, "...attempting to use stereo\n" );
//      src.dwFlags |= PFD_STEREO;
//      //glConfig.stereoEnabled = true;
//   }
//   else
//   {
//      //glConfig.stereoEnabled = qfalse;
//   }
//   *pPFD = src;
//}


//--------------------------------------
//static S32 ChooseBestPixelFormat(HDC hDC, PIXELFORMATDESCRIPTOR *pPFD)
//{
//   PIXELFORMATDESCRIPTOR pfds[MAX_PFDS+1];
//   S32 i;
//   S32 bestMatch = 0;
//   
//   S32 maxPFD = DescribePixelFormat(hDC, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfds[0]);
//   if(maxPFD > MAX_PFDS)
//      maxPFD = MAX_PFDS;
//   
//   for(i = 1; i < maxPFD; i++)
//   {
//      DescribePixelFormat(hDC, i, sizeof(PIXELFORMATDESCRIPTOR), &pfds[i]);
//
//      // make sure this has hardware acceleration:
//      if ( ( pfds[i].dwFlags & PFD_GENERIC_FORMAT ) != 0 ) 
//         continue;
//
//      // verify pixel type
//      if ( pfds[i].iPixelType != PFD_TYPE_RGBA )
//         continue;
//
//      // verify proper flags
//      if ( ( ( pfds[i].dwFlags & pPFD->dwFlags ) & pPFD->dwFlags ) != pPFD->dwFlags ) 
//         continue;
//
//      //
//      // selection criteria (in order of priority):
//      // 
//      //  PFD_STEREO
//      //  colorBits
//      //  depthBits
//      //  stencilBits
//      //
//      if ( bestMatch )
//      {
//         // check stereo
//         if ( ( pfds[i].dwFlags & PFD_STEREO ) && ( !( pfds[bestMatch].dwFlags & PFD_STEREO ) ) && ( pPFD->dwFlags & PFD_STEREO ) )
//         {
//            bestMatch = i;
//            continue;
//         }
//         
//         if ( !( pfds[i].dwFlags & PFD_STEREO ) && ( pfds[bestMatch].dwFlags & PFD_STEREO ) && ( pPFD->dwFlags & PFD_STEREO ) )
//         {
//            bestMatch = i;
//            continue;
//         }
//
//         // check color
//         if ( pfds[bestMatch].cColorBits != pPFD->cColorBits )
//         {
//            // prefer perfect match
//            if ( pfds[i].cColorBits == pPFD->cColorBits )
//            {
//               bestMatch = i;
//               continue;
//            }
//            // otherwise if this PFD has more bits than our best, use it
//            else if ( pfds[i].cColorBits > pfds[bestMatch].cColorBits )
//            {
//               bestMatch = i;
//               continue;
//            }
//         }
//
//         // check depth
//         if ( pfds[bestMatch].cDepthBits != pPFD->cDepthBits )
//         {
//            // prefer perfect match
//            if ( pfds[i].cDepthBits == pPFD->cDepthBits )
//            {
//               bestMatch = i;
//               continue;
//            }
//            // otherwise if this PFD has more bits than our best, use it
//            else if ( pfds[i].cDepthBits > pfds[bestMatch].cDepthBits )
//            {
//               bestMatch = i;
//               continue;
//            }
//         }
//
//         // check stencil
//         if ( pfds[bestMatch].cStencilBits != pPFD->cStencilBits )
//         {
//            // prefer perfect match
//            if ( pfds[i].cStencilBits == pPFD->cStencilBits )
//            {
//               bestMatch = i;
//               continue;
//            }
//            // otherwise if this PFD has more bits than our best, use it
//            else if ( ( pfds[i].cStencilBits > pfds[bestMatch].cStencilBits ) && 
//                ( pPFD->cStencilBits > 0 ) )
//            {
//               bestMatch = i;
//               continue;
//            }
//         }
//      }
//      else
//      {
//         bestMatch = i;
//      }
//   }
//   
//   if ( !bestMatch )
//      return 0;
//
//   else if ( pfds[bestMatch].dwFlags & PFD_GENERIC_ACCELERATED )
//   {
//      // MCD
//   }
//   else
//   {
//      // ICD
//   }
//
//   *pPFD = pfds[bestMatch];
//
//   return bestMatch;
//}

const Point2I &Platform::getWindowSize()
{
   return windowSize;
}

void Platform::setWindowSize( U32 newWidth, U32 newHeight )
{
   windowSize.set( newWidth, newHeight );
}


//--------------------------------------
static void InitWindow(const Point2I &initialSize)
{
	const S32 offset = 50;  // magic number that can be changed later
	
	// for now just get first screen device, later we should check them all
	ppcState.hDisplay = DMGetFirstScreenDevice(true);
	
	Rect	rect;
	SetRect( &rect, offset, offset, initialSize.x+offset, initialSize.y+offset);
	
	ppcState.appWindow = NewCWindow(NULL, 
											&rect, 					// bounding rect
											str2p(ppcState.appWindowTitle), 	// window title
											false,					// is visible
											documentProc,			// window type
											(WindowPtr) -1L,		// top most window
											true, 					// has a close box
											0L);						// reference constant
	if (ppcState.appWindow == NULL)
		return;
		
	ShowWindow( ppcState.appWindow );
	windowSize = initialSize;	
	
//   RECT r;
//   r.left = 0;
//   r.top = 0;
//   r.right = initialSize.x;
//   r.bottom = initialSize.y;
//
//   windowSize = initialSize;
//
//   S32 windowStyle = (WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE);
//   S32 exWindowStyle = 0;
//
//   AdjustWindowRect (&r, windowStyle, FALSE);
//   
//   S32 x = 50;
//   S32 y = 50;
//   S32 w = r.right - r.left;
//   S32 h = r.bottom - r.top;
//   
//   winState.appWindow = CreateWindowEx(
//      exWindowStyle, 
//      windowClassName,
//      "Tribes II",
//      windowStyle,
//      x, y, w, h,
//      NULL, NULL,
//      winState.appInstance,
//      NULL);
//
//   
//   ShowWindow( winState.appWindow, SW_SHOW );
//   UpdateWindow( winState.appWindow );
//
//   SetForegroundWindow( winState.appWindow );
//   SetFocus( winState.appWindow );
//
//   winState.appDC = GetDC(winState.appWindow);
//   setMouseClipping();
}


//--------------------------------------
static void InitOpenGL()
{
   // Video::setDevice( Con::getVariable( "$pref::Video::displayDevice" ) );
   Video::setDevice( "OpenGL" );

   // Just for kicks.  Seems a relatively central place to put this...
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


//	GLint attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 16, AGL_NONE };  
	
//	// Never unload a code module
//	aglConfigure(AGL_RETAIN_RENDERERS, GL_TRUE);

//	ppcState.fmt = aglChoosePixelFormat(NULL, 0, attrib);
//	if(ppcState.fmt == NULL) return;
	
//	/* Create an AGL context */
//	ppcState.ctx = aglCreateContext(ppcState.fmt, NULL);
//	if(ppcState.ctx == NULL) return;

//	/* Attach the context to the window */
//	if(!aglSetDrawable(ppcState.ctx, (CGrafPtr) ppcState.appWindow)) return;

//	/* Make this context current */
//	aglSetCurrentContext(ppcState.ctx);
	
}

static U32 lastTimeTick;

//--------------------------------------

void Platform::init()
{
   WinConsole::create();
   Input::init();
   InitInput();   // in case our other input falls through
   Video::init();
   Video::installDevice( OpenGLDevice::create() );
}

void Platform::shutdown()
{
   Video::destroy();
   WinConsole::destroy();
   
//	// cleanup openGl
//	aglSetCurrentContext(NULL);
//	if (ppcState.ctx)
//	{
//		aglSetDrawable(ppcState.ctx, NULL);
//		aglDestroyContext(ppcState.ctx);
//	}
//
//	// clean up the app window
//	if (ppcState.appWindow)
//		DisposeWindow(ppcState.appWindow);
}

static S32 run(S32 argc, const char **argv)
{
   createFontInit();
   windowSize.set(0,0);
   
   lastTimeTick = GetMilliseconds();
   
   S32 ret = GameMain(argc, argv);
   createFontShutdown();

   return ret;
}

void Platform::initWindow(const Point2I &initialSize, const char *name)
{
   dSprintf(ppcState.appWindowTitle, sizeof(ppcState.appWindowTitle), name);
   InitWindow(initialSize);
   InitOpenGL();
}

//--------------------------------------
S32 main(S32 argc, const char **argv)
{
	// mac does not support command line arguments
	// it may be possible to get the comment field from the file
	// see DesktopManager and DTGetComment
	
	InitGraf(&qd.thePort);		// init QuickDraw -- 'qd' is a Mac Global
	InitFonts();					// init the Font Manager
	InitWindows();					// init the Window Manager
	InitMenus();
	TEInit();
	InitDialogs( NULL );
	InitCursor();
	
	FlushEvents( everyEvent, 0 );
	SetEventMask(everyEvent);

	ppcGetWorkingDirectory();

    return run(argc, argv);
}


//--------------------------------------
//S32 PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, S32 nCmdShow )
//{
//   hPrevInstance;
//   const char *ptr = lpszCmdLine;
//   nCmdShow;
//   Vector<char *> argv;
//   char moduleName[256];
//   GetModuleFileName(NULL, moduleName, sizeof(moduleName));
//   
//   argv.push_back(moduleName);
//   S32 i = 0;
//   for(;;)
//   {
//      char c = ptr[i];
//      if(c == 0 || c == ' ')
//      {
//         if(i)
//         {
//            char *arg = (char *) dMalloc(i+1);
//            dStrncpy(arg, ptr, i);
//            arg[i] = 0;
//            argv.push_back(arg);
//            ptr += i + 1;
//            i = 0;
//            
//         }
//         if(!c)
//            break;
//      }
//      else
//         i++;
//   }
//   winState.appInstance = hInstance;
//   S32 retVal = run(argv.size(), (const char **) argv.address());
//   
//   for(U32 j = 1; j < argv.size(); j++)
//      dFree(argv[j]);
//
//   return retVal;
//}


//--------------------------------------
//void Video::swapBuffers()
//{
//	aglSwapBuffers(aglGetCurrentContext());
//}


//--------------------------------------
void TimeManager::process()
{
   U32 curTime = GetMilliseconds();
   TimeEvent event;
   event.elapsedTime = curTime - lastTimeTick;
   lastTimeTick = curTime;
   GamePostEvent(event);
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

F32 Platform::getRandom()
{
   //return rand() / F32(RAND_MAX);
   return 0.5;
}

