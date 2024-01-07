//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include "PlatformPPC/ppcConsole.h"
#include "Platform/event.h"

WinConsole *WindowsConsole = NULL;

static void consoleEnableCallback(const char *, const char *enabled)
{
   WindowsConsole->enable(dAtob(enabled));
}

void WinConsole::create()
{
   WindowsConsole = new WinConsole();
}

void WinConsole::destroy()
{
   delete WindowsConsole;
   WindowsConsole = NULL;
}

void WinConsole::enable(bool enabled)
{
   winConsoleEnabled = enabled;
//   if(winConsoleEnabled)
//   {
//      AllocConsole();
//      const char *title = Con::getVariable("Con::WindowTitle");
//      if (title && *title)
//         SetConsoleTitle(title);
//      stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
//      stdIn  = GetStdHandle(STD_INPUT_HANDLE);
//      stdErr = GetStdHandle(STD_ERROR_HANDLE);
//
//      printf("%s", Con::getVariable("Con::Prompt"));
//   }
}

WinConsole::WinConsole()
{
//   for (S32 iIndex = 0; iIndex < MAX_CMDS; iIndex ++)
//      rgCmds[iIndex][0] = '\0';
//
//   iCmdIndex = 0;
//   winConsoleEnabled = false;
//   Con::addConsumer(this);
//   inpos = 0;
//   lineOutput = false;
//   Con::addVariable("WinConsoleEnabled", consoleEnableCallback, "false");
}

void WinConsole::printf(const char *s, ...)
{
   s;
//   static char buffer[512];
//   DWORD bytes;
//   va_list args;
//   va_start(args, s);
//   vsprintf(buffer, s, args);
//   WriteFile(stdOut, buffer, strlen(buffer), &bytes, NULL);
//   FlushFileBuffers( stdOut );
}

void WinConsole::processConsoleLine(const char *consoleLine)
{
   consoleLine;
//   if(winConsoleEnabled)
//   {
//      inbuf[inpos] = 0;
//      if(lineOutput)
//         printf("%s\n", consoleLine);
//      else
//         printf("%c%s\n%s%s", '\r', consoleLine, Con::getVariable("Con::Prompt"), inbuf);
//   }
}

void WinConsole::process()
{
//   if(winConsoleEnabled)
//   {
//      DWORD numEvents;
//      GetNumberOfConsoleInputEvents(stdIn, &numEvents);
//      if(numEvents)
//      {
//         INPUT_RECORD rec[20];
//         char outbuf[256];
//         S32 outpos = 0;
//
//         ReadConsoleInput(stdIn, rec, 20, &numEvents);
//         DWORD i;
//         for(i = 0; i < numEvents; i++)
//         {
//            if(rec[i].EventType == KEY_EVENT)
//            {
//               KEY_EVENT_RECORD *ke = &(rec[i].Event.KeyEvent);
//               if(ke->bKeyDown)
//               {
//                  switch (ke->uChar.AsciiChar)
//                  {
//                     // If no ASCII char, check if it's a handled virtual key
//                     case 0:
//                        switch (ke->wVirtualKeyCode)
//                        {
//                           // UP ARROW
//                           case 0x26 :
//                              // Go to the previous command in the cyclic array
//                              if ((-- iCmdIndex) < 0)
//                                 iCmdIndex = MAX_CMDS - 1;
//
//                              // If this command isn't empty ...
//                              if (rgCmds[iCmdIndex][0] != '\0')
//                              {
//                                 // Obliterate current displayed text
//                                 for (S32 i = outpos = 0; i < inpos; i ++)
//                                 {
//                                    outbuf[outpos ++] = '\b';
//                                    outbuf[outpos ++] = ' ';
//                                    outbuf[outpos ++] = '\b';
//                                 }
//
//                                 // Copy command into command and display buffers
//                                 for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
//                                 {
//                                    outbuf[outpos] = rgCmds[iCmdIndex][inpos];
//                                    inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
//                                 }
//                              }
//                              // If previous is empty, stay on current command
//                              else if ((++ iCmdIndex) >= MAX_CMDS)
//                              {
//                                 iCmdIndex = 0;
//                              }
//                              
//                              break;
//
//                           // DOWN ARROW
//                           case 0x28 : {
//                              // Go to the next command in the command array, if
//                              // it isn't empty
//                              if (rgCmds[iCmdIndex][0] != '\0' && (++ iCmdIndex) >= MAX_CMDS)
//                                  iCmdIndex = 0;
//
//                              // Obliterate current displayed text
//                              for (S32 i = outpos = 0; i < inpos; i ++)
//                              {
//                                 outbuf[outpos ++] = '\b';
//                                 outbuf[outpos ++] = ' ';
//                                 outbuf[outpos ++] = '\b';
//                              }
//
//                              // Copy command into command and display buffers
//                              for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
//                              {
//                                 outbuf[outpos] = rgCmds[iCmdIndex][inpos];
//                                 inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
//                              }
//                              }
//                              break;
//
//                           // LEFT ARROW
//                           case 0x25 :
//                              break;
//
//                           // RIGHT ARROW
//                           case 0x27 :
//                              break;
//
//                           default :
//                              break;
//                        }
//                        break;
//                     case '\b':
//                        if(inpos > 0)
//                        {
//                           outbuf[outpos++] = '\b';
//                           outbuf[outpos++] = ' ';
//                           outbuf[outpos++] = '\b';
//                           inpos--;
//                        }
//                        break;
//                     case '\n':
//                     case '\r':
//                        outbuf[outpos++] = '\r';
//                        outbuf[outpos++] = '\n';
//
//                        inbuf[inpos] = 0;
//                        outbuf[outpos] = 0;
//                        printf("%s", outbuf);
//
//                        S32 eventSize;
//                        eventSize = ConsoleEventHeaderSize;
//                            
//                        dStrcpy(postEvent.data, inbuf);
//                        postEvent.size = eventSize + dStrlen(inbuf) + 1;
//                        GamePostEvent(postEvent);
//
//                        // If we've gone off the end of our array, wrap
//                        // back to the beginning
//                        if (iCmdIndex >= MAX_CMDS)
//                            iCmdIndex %= MAX_CMDS;
//
//                        // Put the new command into the array
//                        strcpy(rgCmds[iCmdIndex ++], inbuf);
//
//                        printf("%s", Con::getVariable("Con::Prompt"));
//                        inpos = outpos = 0;
//                        break;
//                     default:
//                        inbuf[inpos++] = ke->uChar.AsciiChar;
//                        outbuf[outpos++] = ke->uChar.AsciiChar;
//                        break;
//                  }
//               }
//            }
//         }
//         if(outpos)
//         {
//            outbuf[outpos] = 0;
//            printf("%s", outbuf);
//         }
//      }
//   }
}
