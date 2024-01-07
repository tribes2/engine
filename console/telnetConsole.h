//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TELNETCONSOLE_H_
#define _TELNETCONSOLE_H_

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

class TelnetConsole
{
   NetSocket mAcceptSocket;
   S32 mAcceptPort;

   enum {
      PasswordMaxLength = 32
   };

   char mTelnetPassword[PasswordMaxLength+1];
   char mListenPassword[PasswordMaxLength+1];
   ConsoleEvent mPostEvent;

   enum State
   {
      PasswordTryOne,
      PasswordTryTwo,
      PasswordTryThree,
      DisconnectThisDude,
      FullAccessConnected,
      ReadOnlyConnected
   };
   
   struct TelnetClient
   {
      NetSocket socket;
      char curLine[Con::MaxLineLength];
      S32 curPos;
      S32 state;
      TelnetClient *nextClient;
   };
   TelnetClient *mClientList;
   TelnetConsole();
   ~TelnetConsole();
public:
   static void create();
   static void destroy();
   void process();
   void setTelnetParameters(S32 port, const char *telnetPassword, const char *listenPassword);
   void processConsoleLine(const char *line);
};

extern TelnetConsole *TelConsole;

#endif

