//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PCCCONSOLE_H_
#define _PCCCONSOLE_H_

#define MAX_CMDS 10
#ifndef _CONSOLE_H_
#include "console/console.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

class WinConsole
{
   bool winConsoleEnabled;

//   HANDLE stdOut;
//   HANDLE stdIn;
//   HANDLE stdErr;
   ConsoleEvent postEvent;
   char inbuf[512];
   S32  inpos;
   bool lineOutput;
   char curTabComplete[512];
   S32  tabCompleteStart;
   char rgCmds[MAX_CMDS][512];
   S32  iCmdIndex;

   void printf(const char *s, ...);

public:
   WinConsole();
   void process();
   void enable(bool);
   void processConsoleLine(const char *consoleLine);
   static void create();
   static void destroy();
};

extern WinConsole *WindowsConsole;

#endif
