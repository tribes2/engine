//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

// TODO

//#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXMessageBox.h"

#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Dialog.h>

#define MaxWinWidth 800
#define MaxWinHeight 600

//------------------------------------------------------------------------------
// XMessageBoxButton
//------------------------------------------------------------------------------
XMessageBoxButton::XMessageBoxButton()
{
   dStrcpy(mLabel, "");
   mClickVal = -1;
   mMB = NULL;
}

//------------------------------------------------------------------------------
XMessageBoxButton::XMessageBoxButton(
   const char* label, int clickVal, XMessageBox* mb)
{
   dStrncpy(mLabel, label, LabelSize);
   mClickVal = clickVal;
   mMB = mb;
}

//------------------------------------------------------------------------------
void XMessageBoxButton::dialogCallback(Widget w,
         XtPointer client_data,
         XtPointer call_data)
{
   XMessageBoxButton* button = 
      reinterpret_cast<XMessageBoxButton*>(client_data);
   if (button != NULL)
      button->clicked();
   XtAppSetExitFlag(XtWidgetToApplicationContext(w));
}

//------------------------------------------------------------------------------
void XMessageBoxButton::clicked()
{
   if (mMB) 
      mMB->setClickedButton(this);
}

//------------------------------------------------------------------------------
// XMessageBox
//------------------------------------------------------------------------------
XMessageBox::XMessageBox()
{
   mWindowTitle = "";
   mMessage = "";
   mWrappedMessage = NULL;
   mWrappedMessageSize = 0;
   mDisplay = NULL;
   mFS = NULL;
   mClickedButton = NULL;
}

//------------------------------------------------------------------------------
XMessageBox::~XMessageBox()
{
   clearWrappedMessage();
}

//------------------------------------------------------------------------------
int XMessageBox::alertOK(const char *windowTitle, const char *message)
{
   mWindowTitle = windowTitle;
   mMessage = message;
   mButtons.clear();
   mButtons.push_back(XMessageBoxButton("OK", OK, this));
   int val = show();
   if (val == -1)
      return OK;
   return val;
}

//------------------------------------------------------------------------------
int XMessageBox::alertOKCancel(const char *windowTitle, const char *message)
{
   mWindowTitle = windowTitle;
   mMessage = message;
   mButtons.clear();
   mButtons.push_back(XMessageBoxButton("OK", OK, this));
   mButtons.push_back(XMessageBoxButton("Cancel", Cancel, this));
   int val = show();
   if (val == -1)
      return Cancel;
   return val;
}

//------------------------------------------------------------------------------
int XMessageBox::alertRetryCancel(const char *windowTitle, const char *message)
{
   mWindowTitle = windowTitle;
   mMessage = message;
   mButtons.clear();
   mButtons.push_back(XMessageBoxButton("Retry", Retry, this));
   mButtons.push_back(XMessageBoxButton("Cancel", Cancel, this));
   int val = show();
   if (val == -1)
      return Cancel;
   return val;
}

//------------------------------------------------------------------------------
template <class Type>
static inline Type min(Type v1, Type v2)
{
   if (v1 > v2)
      return v2;
   else
      return v1;
}

//------------------------------------------------------------------------------
void XMessageBox::clearWrappedMessage()
{
   if (mWrappedMessage != NULL)
   {
      delete [] mWrappedMessage;
      mWrappedMessageSize = 0;
   }
}

//------------------------------------------------------------------------------
void XMessageBox::splitMessage()
{
   clearWrappedMessage();
   if (mMessage == NULL || dStrlen(mMessage)==0)
      return;

   int numChars = dStrlen(mMessage);
   mWrappedMessageSize = (numChars * 2) + 1;
   mWrappedMessage = new char[mWrappedMessageSize];
   dMemset(mWrappedMessage, 0, mWrappedMessageSize);

   int fontDirection, fontAscent, fontDescent;
   XCharStruct strInfo;

   char *curChar = const_cast<char*>(mMessage);
   char *endChar;
   char *curWrapped = mWrappedMessage;
   int curWidth = 0;

   while ( // while pointers are in range...
      (curChar - mMessage) < numChars &&
      (curWrapped - mWrappedMessage) < mWrappedMessageSize)
   {
      // look for next space in remaining string
      endChar = index(curChar, ' ');
      if (endChar == NULL)
         endChar = index(curChar, '\0');

      if (endChar != NULL)
         // increment one char past the space to include it
         endChar++;
      else
         // otherwise, set the endchar to one char ahead
         endChar = curChar + 1;

      // compute length of substr
      int len = endChar - curChar;
      XTextExtents(mFS, curChar, len, 
         &fontDirection, &fontAscent, &fontDescent,
         &strInfo);
      // if its too big, insert a newline and reset curWidth
      if ((curWidth + strInfo.width) > MaxWinWidth)
      {
         *curWrapped = '\n';
         curWrapped++;
         curWidth = 0;
      }
      // copy the current string into curWrapped if we have enough room
      int bytesRemaining = 
         mWrappedMessageSize - (curWrapped - mWrappedMessage);
      if (bytesRemaining >= len)
         dStrncpy(curWrapped, curChar, len);

      curWrapped += len;
      curWidth += strInfo.width;
      curChar = endChar;
   }

   // shouldn't be necessary due to the dMemset, but just in case
   mWrappedMessage[mWrappedMessageSize-1] = '\0';
}

//------------------------------------------------------------------------------
int XMessageBox::show()
{
   Widget toplevel;
   int argc = 3;
   // this is a hack to specify a target font
   char *argv[] = { const_cast<char*>(mWindowTitle), 
                    "-fn", "-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*" };

   // init the top level widget
   XtAppContext appContext;

   toplevel = XtOpenApplication(&appContext, "", NULL, 0, &argc, argv, NULL,
      applicationShellWidgetClass, NULL, 0);

//this is supposed to initialize the toolkit from a pre existing display,
//but I couldn't get it to work.
//    XtToolkitInitialize();
//    XtAppContext xtAppContext = XtCreateApplicationContext();
//    XtAppSetFallbackResources(xtAppContext, fallback) ;
//    XtDisplayInitialize(xtAppContext, mDisplay, "", "", NULL, 0,
//       &argc, argv);
// //    toplevel = XtAppCreateShell("Torque Alert", "", applicationShellWidgetClass,
// //       mDisplay, NULL, 0);
//    toplevel = XtVaAppCreateShell("", "",
//       applicationShellWidgetClass, mDisplay, 0) ;

   // set the display member
   mDisplay = XtDisplay(toplevel);

   // don't map it when managed
   XtVaSetValues(toplevel, 
      XtNmappedWhenManaged, False, 
      0);

   // read the font resource to see what font we got
   struct _appRes
   {
         XFontStruct* font;
   } appRes;
   
   XtResource resources[] = {
      { 
         "font", "Font", 
         XtRFontStruct, 
         sizeof(XFontStruct*),
         XtOffsetOf( struct _appRes, font ),
         XtRImmediate, NULL 
      },
   };

   XtVaGetApplicationResources( toplevel, (XtPointer)&appRes,
      resources, XtNumber(resources), 0 );

   mFS = appRes.font;

   // set the maximum window dimensions
   mScreenWidth = DisplayWidth(mDisplay, DefaultScreen(mDisplay));
   mScreenHeight = DisplayHeight(mDisplay, DefaultScreen(mDisplay));
   mMaxWindowWidth = min(mScreenWidth, MaxWinWidth);
   mMaxWindowHeight = min(mScreenHeight, MaxWinHeight);

   // split the message into a vector of lines
   splitMessage();

   // create the dialog
   Widget dialog = XtVaCreateManagedWidget("ok_dialog", dialogWidgetClass,
      toplevel, XtNlabel, 
      mWrappedMessageSize != 0 ? mWrappedMessage : "", 
      0);
   Vector<XMessageBoxButton>::iterator iter;
   for (iter = mButtons.begin(); iter != mButtons.end(); ++iter)
   {
      XawDialogAddButton(dialog, iter->getLabel(), 
         iter->dialogCallback, iter);
   }

   XtRealizeWidget(toplevel);

//    XChangeProperty(mDisplay, XtWindow(toplevel),
//       XInternAtom(mDisplay, "WM_TRANSIENT_FOR", False),
//       XA_ATOM, 32, PropModeAppend,
//       NULL, 0);

   // set the x, y position to center of display
   Position x, y;
   Dimension width, height;
   XtVaGetValues(toplevel, XtNx, &x, XtNy, &y, 
      XtNwidth, &width, XtNheight, &height, 0);
   x = (mScreenWidth - width) / 2;
   y = (mScreenHeight - height) / 2;
   XtVaSetValues(toplevel, XtNx, x, XtNy, y, 0);

   // initialize wm_protocols, so that we can respond to window destroyed
   // message
   Atom wm_delete_window = 
      XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
   Atom wm_protocols = 
      XInternAtom(mDisplay, "WM_PROTOCOLS", False);
   XSetWMProtocols (mDisplay, XtWindow(toplevel),
      &wm_delete_window, 1);

   XSetWindowAttributes xattr;
   xattr.override_redirect = True;

//    XChangeWindowAttributes(mDisplay, XtWindow(toplevel),
//       CWOverrideRedirect, &xattr);

   // show the widget
   XtMapWidget(toplevel);
   XRaiseWindow(mDisplay, XtWindow(toplevel));

   // do event loop
   XEvent event;

   while (!XtAppGetExitFlag(appContext))
   {
      XtAppNextEvent(appContext, &event);
      if (event.type == ClientMessage &&
         event.xclient.message_type == wm_protocols &&
         event.xclient.data.l[0] == static_cast<long>(wm_delete_window))
         XtAppSetExitFlag(appContext);
      XtDispatchEvent(&event);
   }

   XtUnrealizeWidget(toplevel);
   XtDestroyWidget(dialog);
   XtDestroyWidget(toplevel);
   XtDestroyApplicationContext(appContext);

   // return the clicked val if we have one
   if (mClickedButton != NULL)
      return mClickedButton->getClickVal();
   else
      return -1;
}
