//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _X86UNIXMESSAGEBOX_H_
#define _X86UNIXMESSAGEBOX_H_

#include "core/tVector.h"
#include <X11/Intrinsic.h>

class XMessageBox;

//------------------------------------------------------------------------------
class XMessageBoxButton
{
   public:
      XMessageBoxButton();
      XMessageBoxButton(const char* label, int clickVal, XMessageBox* mb);

      const char *getLabel() { return static_cast<const char*>(mLabel); }
      int getClickVal() { return mClickVal; }

      void clicked(); 

      static void dialogCallback(Widget w,
         XtPointer client_data,
         XtPointer call_data);

   private:
      static const int LabelSize = 100;
      char mLabel[LabelSize];
      int mClickVal;
      XMessageBox *mMB;
};

//------------------------------------------------------------------------------
class XMessageBox
{
   public:
      static const int OK = 4;
      static const int Cancel = 5;
      static const int Retry = 6;     

      XMessageBox();
      ~XMessageBox();

      int alertOK(const char *windowTitle, const char *message);
      int alertOKCancel(const char *windowTitle, const char *message);
      int alertRetryCancel(const char *windowTitle, const char *message);
      static void onWMProtocols();

      void setClickedButton(XMessageBoxButton* clickedButton)
      {
         mClickedButton = clickedButton;
      }

   private:
      int show();
      void splitMessage();
      void clearWrappedMessage();

      XMessageBoxButton* mClickedButton;
      const char* mMessage;
      const char* mWindowTitle;
      char* mWrappedMessage;
      int mWrappedMessageSize;
      Vector<XMessageBoxButton> mButtons;

      Display* mDisplay;
      XFontStruct* mFS;

      int mScreenWidth, mScreenHeight, mMaxWindowWidth, mMaxWindowHeight;
};

#endif // #define _X86UNIXMESSAGEBOX_H_
