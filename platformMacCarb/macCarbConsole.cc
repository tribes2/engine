//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformMacCarb/platformMacCarb.h"
#include "PlatformMacCarb/maccarbConsole.h"
#include "Platform/event.h"
#include "Platform/gameInterface.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if !__APPLE__
#include <SIOUX.h>
#endif

MacConsole *gConsole = NULL;

ConsoleFunction(enableWinConsole, void, 2, 2, "enableWinConsole(bool);")
{
   argc;
   gConsole->enable(dAtob(argv[1]));
}

void MacConsole::create()
{
   gConsole = new MacConsole();
}

void MacConsole::destroy()
{
   delete gConsole;
   gConsole = NULL;
}

void MacConsole::enable(bool enabled)
{
   consoleEnabled = enabled;
   if(consoleEnabled)
   {
//      AllocConsole();
      printf("Console Initialized.\n");
      const char *title = Con::getVariable("Con::WindowTitle");
      if (title && *title)
      {
         unsigned char t2[256];
         str2p(title, t2);
#if !__APPLE__
         SIOUXSetTitle(t2);
#endif
      }
//      stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
//      stdIn  = GetStdHandle(STD_INPUT_HANDLE);
//      stdErr = GetStdHandle(STD_ERROR_HANDLE);
//
      printf("%s", Con::getVariable("Con::Prompt"));
   }
}

bool MacConsole::isEnabled()
{
   if ( gConsole )
      return gConsole->consoleEnabled;

   return false;
}

static void macConsoleConsumer(ConsoleLogEntry::Level, const char *line)
{
   gConsole->processConsoleLine(line);
}

MacConsole::MacConsole()
{
   for (S32 iIndex = 0; iIndex < MAX_CMDS; iIndex ++)
      rgCmds[iIndex][0] = '\0';

   iCmdIndex = 0;

   consoleEnabled = false;
   currMsg = NULL;
   
   Con::addConsumer(macConsoleConsumer);

   inpos = 0;
   lineOutput = false;

//   Con::addVariable("MacConsoleEnabled", consoleEnableCallback, "false");

#if !__APPLE__
   // set up the SIOUX stuff here for now.
   SIOUXSettings.standalone = false;
   SIOUXSettings.setupmenus = false;
   SIOUXSettings.initializeTB = false;
   SIOUXSettings.asktosaveonclose = false;
   SIOUXSettings.toppixel = 4 + GetMBarHeight();
   SIOUXSettings.leftpixel = 4;
   SIOUXSettings.columns = 120;
   SIOUXSettings.rows = 40;
#endif
}

void MacConsole::printf(const char *s, ...)
{
//   static char buffer[512];
   int bytes;
   va_list args;
   va_start(args, s);

   vprintf(s, args);
   
//   vsprintf(buffer, s, args);
//   WriteFile(stdOut, buffer, strlen(buffer), &bytes, NULL);
//   FlushFileBuffers( stdOut );
}

void MacConsole::processConsoleLine(const char *consoleLine)
{
   if(consoleEnabled)
   {
      inbuf[inpos] = 0;
      if(lineOutput)
         printf("%s\n", consoleLine);
      else
         printf("%c%s\n%s%s", '\r', consoleLine, Con::getVariable("Con::Prompt"), inbuf);
   }
}


//--------------------------------------
bool MacConsole::handleEvent(const EventRecord *msg)
{
#if !__APPLE__
//   if (MacConsole::isEnabled())
   {
      if (SIOUXHandleOneEvent((EventRecord*)msg))
      {
         currMsg = msg;
         process();
         currMsg = NULL;

         return(true);
      }
   }
#endif
   
   return(false);
}


//--------------------------------------
// on the mac, we call this after the console has consumed a keyboard event.
void MacConsole::process()
{
   // for now, minimal processing.
   
   if(consoleEnabled && currMsg)
   {
/*
      DWORD numEvents;
      GetNumberOfConsoleInputEvents(stdIn, &numEvents);
      if(numEvents)
      {
         INPUT_RECORD rec[20];
*/
         char outbuf[256];
         S32 outpos = 0;

//         ReadConsoleInput(stdIn, rec, 20, &numEvents);
//         DWORD i;
//         for(i = 0; i < numEvents; i++)
//         {
//            if(rec[i].EventType == KEY_EVENT)
//            {
//               KEY_EVENT_RECORD *ke = &(rec[i].Event.KeyEvent);
               if(currMsg->what==keyDown || currMsg->what==autoKey)
               {
                  U8 ascii = currMsg->message & charCodeMask;
                  U8 keycode = TranslateOSKeyCode( (currMsg->message & keyCodeMask) >> 8 );
                 
                  switch (ascii)
                  {
                     // If no ASCII char, check if it's a handled virtual key
                     case 28:
                     case 29:
                     case 30:
                     case 31:
                        switch (keycode)
                        {
                           // UP ARROW
                           case KEY_UP:
                           {
                              // Go to the previous command in the cyclic array
                              if ((-- iCmdIndex) < 0)
                                 iCmdIndex = MAX_CMDS - 1;

                              // If this command isn't empty ...
                              if (rgCmds[iCmdIndex][0] != '\0')
                              {
                                 // Obliterate current displayed text
                                 for (S32 i = outpos = 0; i < inpos; i ++)
                                 {
                                    outbuf[outpos ++] = '\b';
                                    outbuf[outpos ++] = ' ';
                                    outbuf[outpos ++] = '\b';
                                 }

                                 // Copy command into command and display buffers
                                 for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
                                 {
                                    outbuf[outpos] = rgCmds[iCmdIndex][inpos];
                                    inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
                                 }
                              }
                              // If previous is empty, stay on current command
                              else if ((++ iCmdIndex) >= MAX_CMDS)
                              {
                                 iCmdIndex = 0;
                              }
                              
                              break;
                           }

                           // DOWN ARROW
                           case KEY_DOWN:
                           {
                              // Go to the next command in the command array, if
                              // it isn't empty
                              if (rgCmds[iCmdIndex][0] != '\0' && (++ iCmdIndex) >= MAX_CMDS)
                                  iCmdIndex = 0;

                              // Obliterate current displayed text
                              for (S32 i = outpos = 0; i < inpos; i ++)
                              {
                                 outbuf[outpos ++] = '\b';
                                 outbuf[outpos ++] = ' ';
                                 outbuf[outpos ++] = '\b';
                              }

                              // Copy command into command and display buffers
                              for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
                              {
                                 outbuf[outpos] = rgCmds[iCmdIndex][inpos];
                                 inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
                              }
                              break;
                           }

                           // LEFT ARROW
                           case KEY_LEFT:
                              break;

                           // RIGHT ARROW
                           case KEY_RIGHT:
                              break;

                           default :
                              break;
                        }
                        break;

                     // backspace???
                     case '\b':
                        if(inpos > 0)
                        {
                           outbuf[outpos++] = '\b';
                           outbuf[outpos++] = ' ';
                           outbuf[outpos++] = '\b';
                           inpos--;
                        }
                        break;

                     // some kind of CR/LF
                     case '\n':
                     case '\r':
//                        outbuf[outpos++] = '\r';
                        outbuf[outpos++] = '\n';

                        inbuf[inpos] = 0;
                        outbuf[outpos] = 0;
                        printf("%s", outbuf);

                        S32 eventSize;
                        eventSize = ConsoleEventHeaderSize;
                            
                        dStrcpy(postEvent.data, inbuf);
                        postEvent.size = eventSize + dStrlen(inbuf) + 1;
                        Game->postEvent(postEvent);

                        // If we've gone off the end of our array, wrap
                        // back to the beginning
                        if (iCmdIndex >= MAX_CMDS)
                            iCmdIndex %= MAX_CMDS;

                        // Put the new command into the array
                        strcpy(rgCmds[iCmdIndex ++], inbuf);

                        printf("%s", Con::getVariable("Con::Prompt"));
                        inpos = outpos = 0;
                        break;
  
                     default:
                        inbuf[inpos++] = ascii;
                        outbuf[outpos++] = ascii;
                        break;
                  }
               }
            //}
         //}

         if(outpos)
         {
            outbuf[outpos] = 0;
            printf("%s", outbuf);
         }
      //}
   }
}
