//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXStdConsole.h"
#include "platform/event.h"
#include "platform/gameInterface.h"

#include <unistd.h>
#include <stdarg.h>
#include <termios.h>
#include <ctype.h>
#include <fcntl.h>

StdConsole *stdConsole = NULL;

ConsoleFunction(enableWinConsole, void, 2, 2, "enableWinConsole(bool);")
{
   argc;
   stdConsole->enable(dAtob(argv[1]));
}

void StdConsole::create()
{
   stdConsole = new StdConsole();
}

void StdConsole::destroy()
{
   delete stdConsole;
   stdConsole = NULL;
}

void StdConsole::enable(bool enabled)
{
   stdConsoleEnabled = enabled;
   if (stdConsoleEnabled)
   {
      stdOut = dup(1);
      stdIn  = dup(0);
      stdErr = dup(2);
      /* setup the proper terminal modes */
      struct termios termModes;
      tcgetattr(fileno(stdin), &termModes);
      termModes.c_lflag &= ~ICANON; // enable non-canonical mode
      termModes.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE);
      termModes.c_cc[VMIN] = 0;
      termModes.c_cc[VTIME] = 5;
      tcsetattr(stdIn, TCSAFLUSH, &termModes);
      /* turn on non-blocking mode so that if no data is available at read(), then it returns */
      fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);

      printf("%s", Con::getVariable("Con::Prompt"));
   }
}

bool StdConsole::isEnabled()
{
   if ( stdConsole )
      return stdConsole->stdConsoleEnabled;

   return false;
}

static void stdConsoleConsumer(ConsoleLogEntry::Level, const char *line)
{
   stdConsole->processConsoleLine(line);
}

StdConsole::StdConsole()
{
   for (S32 iIndex = 0; iIndex < MAX_CMDS; iIndex ++)
      rgCmds[iIndex][0] = '\0';

   iCmdIndex = 0;
   stdConsoleEnabled = false;
   Con::addConsumer(stdConsoleConsumer);
   inpos = 0;
   lineOutput = false;
}

void StdConsole::printf(const char *s, ...)
{
   static char buffer[512];
   long bytes;
   va_list args;

   va_start(args, s);
   vsprintf(buffer, s, args);

   write(stdOut, buffer, strlen(buffer));
   fflush(stdout);
}

void StdConsole::processConsoleLine(const char *consoleLine)
{
   if(stdConsoleEnabled)
   {
      inbuf[inpos] = 0;
      if(lineOutput)
         printf("%s\n", consoleLine);
      else
         printf("%c%s\n%s%s", '\r', consoleLine, Con::getVariable("Con::Prompt"), inbuf);
   }
}

void StdConsole::process()
{
   if(stdConsoleEnabled)
   {
      U16 key;
      char typedData[64];  // damn, if you can type this fast... :-D
      int numEvents = read(stdIn, typedData, 64);
      if (numEvents == -1) return;
      typedData[numEvents] = '\0';
//if (typedData[0] == 27) {
//      fprintf(stdout, "1 = %c, 2 = %c\n", typedData[1], typedData[2]);
//}
//      fprintf(stdout, "read %d elements\n",numEvents);fflush(NULL);
//      
//      int numEvents = read(stdIn, &key, sizeof(char));
      if (numEvents > 0)
      {
        char outbuf[256];
        S32 outpos = 0;

        for (int i = 0; i < numEvents; i++)
        {
         switch(typedData[i])
         {
            case 8:
            case 127:
            /* backspace */
               if (inpos > 0)
               {
                  outbuf[outpos++] = '\b';
                  outbuf[outpos++] = ' ';
                  outbuf[outpos++] = '\b';
                  inpos--;
               }
               break;
            case '\n':
            /* new line */
                  outbuf[outpos++] = '\n';

                  inbuf[inpos] = 0;
                  outbuf[outpos] = 0;
                  printf("%s", outbuf);

                  S32 eventSize;
                  eventSize = 1;

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
               case 27:
                  if (typedData[++i] == 91) {
                     // an arrow key was pressed.
                     switch(typedData[++i])
                     {
                        case 'A':
                           /* up arrow */
                           // Go to the previous command in the cyclic array
                           if ((-- iCmdIndex) < 0)
                           iCmdIndex = MAX_CMDS - 1;

                           // If this command isn't empty ...
                           if (rgCmds[iCmdIndex][0] != '\0')
                           {
                              // Obliterate current displayed text
                              for (S32 i = outpos = 0; i < inpos; i ++)
                              {
                                 outbuf[outpos++] = '\b';
                                 outbuf[outpos++] = ' ';
                                 outbuf[outpos++] = '\b';
                              }

                              // Copy command into command and display buffers
                              for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos++, outpos++)
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
                        case 'B':
                           /* down arrow */
                           // Go to the next command in the command array, if
                           // it isn't empty
                           if (rgCmds[iCmdIndex][0] != '\0' && (++ iCmdIndex) >= MAX_CMDS)
                              iCmdIndex = 0;

                           // Obliterate current displayed text
                           for (S32 i = outpos = 0; i < inpos; i ++)
                           {
                              outbuf[outpos++] = '\b';
                              outbuf[outpos++] = ' ';
                              outbuf[outpos++] = '\b';
                           }

                           // Copy command into command and display buffers
                           for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos++, outpos++)
                           {
                              outbuf[outpos] = rgCmds[iCmdIndex][inpos];
                              inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
                           }
                           break;
                        case 'C':
                           /* right arrow */
                           break;
                        case 'D':
                           /* left arrow */
                           break;
                     }
                     // read again to get rid of a bad char.
                     //read(stdIn, &key, sizeof(char));
                     break;
                  } else {
                     inbuf[inpos++] = typedData[i];
                     outbuf[outpos++] = typedData[i];
                     break;
                  }
                  break;
               default:
                  inbuf[inpos++] = typedData[i];
                  outbuf[outpos++] = typedData[i];
                  break;
         }
         if (outpos)
         {
            outbuf[outpos] = 0;
            printf("%s", outbuf);
         }
        }
      }
   }
}
