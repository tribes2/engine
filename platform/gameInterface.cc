//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "core/fileStream.h"
#ifdef __linux
#include "platform/platformMutex.h"

static void* mutex = 0;
#endif
#include "console/console.h"
GameInterface *Game = NULL;

GameInterface::GameInterface()
{
   AssertFatal(Game == NULL, "ERROR: Multiple games declared.");
   Game = this;
   mJournalMode = JournalOff;
   mRunning = true;
#ifdef __linux
   mutex = Mutex::createMutex( );
   AssertFatal( mutex == 0, "ERROR: can't create mutex" );
#endif
}
   
int GameInterface::main(int, const char**)
{
   return(0);
}

void GameInterface::textureKill()
{

}

void GameInterface::textureResurrect()
{

}

void GameInterface::refreshWindow()
{

}

static U32 sReentrantCount = 0;

void GameInterface::processEvent(Event *event)
{
   if(!mRunning)
      return;

#ifdef __linux
   Mutex::lockMutex( mutex );
#else
   if(PlatformAssert::processingAssert()) // ignore any events if an assert dialog is up.
      return;

#ifdef DEBUG
   sReentrantCount++;
   AssertFatal(sReentrantCount == 1, "Error! ProcessEvent is NOT reentrant.");
#endif
#endif
   switch(event->type)
   {
      case PacketReceiveEventType:
         processPacketReceiveEvent((PacketReceiveEvent *) event);
         break;
      case MouseMoveEventType:
         processMouseMoveEvent((MouseMoveEvent *) event);
         break;
      case InputEventType:
         processInputEvent((InputEvent *) event);
         break;
      case QuitEventType:
         processQuitEvent();
         break;
      case TimeEventType:
         processTimeEvent((TimeEvent *) event);
         break;
      case ConsoleEventType:
         processConsoleEvent((ConsoleEvent *) event);
         break;
      case ConnectedAcceptEventType:
         processConnectedAcceptEvent( (ConnectedAcceptEvent *) event );
         break;
      case ConnectedReceiveEventType:
         processConnectedReceiveEvent( (ConnectedReceiveEvent *) event );
         break;
      case ConnectedNotifyEventType:
         processConnectedNotifyEvent( (ConnectedNotifyEvent *) event );
         break;
   }

#ifdef __linux
   Mutex::unlockMutex( mutex );
#else
#ifdef DEBUG
   sReentrantCount--;
#endif
#endif
}


void GameInterface::processPacketReceiveEvent(PacketReceiveEvent*)
{

}

void GameInterface::processMouseMoveEvent(MouseMoveEvent*)
{

}

void GameInterface::processInputEvent(InputEvent*)
{

}

void GameInterface::processQuitEvent()
{
}

void GameInterface::processTimeEvent(TimeEvent*)
{
}

void GameInterface::processConsoleEvent(ConsoleEvent*)
{

}

void GameInterface::processConnectedAcceptEvent(ConnectedAcceptEvent*)
{

}

void GameInterface::processConnectedReceiveEvent(ConnectedReceiveEvent*)
{

}

void GameInterface::processConnectedNotifyEvent(ConnectedNotifyEvent*)
{

}

struct ReadEvent : public Event
{
   U8 data[3072];
};

FileStream gJournalStream;

void GameInterface::postEvent(Event &event)
{
   if((mJournalMode == JournalLoad || mJournalMode == JournalPlay) && event.type != QuitEventType)
      return;
   if(mJournalMode == JournalSave) {
      gJournalStream.write(event.size, &event);
      gJournalStream.flush();
   }
   processEvent(&event);
}

void GameInterface::journalProcess()
{
   if(mJournalMode == JournalLoad || mJournalMode == JournalPlay)
   {
      ReadEvent journalReadEvent;
      if(gJournalStream.read(&journalReadEvent.type))
      {
         if(gJournalStream.read(&journalReadEvent.size))
         {
            if(gJournalStream.read(journalReadEvent.size - sizeof(Event), &journalReadEvent.data))
            {
               if(gJournalStream.getPosition() == gJournalStream.getStreamSize() && mJournalMode == JournalLoad && !Con::getBoolVariable("NoJournalBreak", false))
                  Platform::debugBreak();
               processEvent(&journalReadEvent);
               return;
            }
         }
      }
      if(mJournalMode == JournalLoad)
         mRunning = false;
      else
         mJournalMode = JournalOff;
   }
}

void GameInterface::loadJournal(const char *fileName)
{
   mJournalMode = JournalLoad;
   gJournalStream.open(fileName, FileStream::Read);
}

void GameInterface::saveJournal(const char *fileName)
{
   mJournalMode = JournalSave;
   gJournalStream.open(fileName, FileStream::Write);
}

void GameInterface::playJournal(const char *fileName)
{
   mJournalMode = JournalPlay;
   gJournalStream.open(fileName, FileStream::Read);
}

FileStream *GameInterface::getJournalStream()
{
   return &gJournalStream;
}

void GameInterface::journalRead(U32 *val)
{
   gJournalStream.read(val);
}

void GameInterface::journalWrite(U32 val)
{
   gJournalStream.write(val);
}

void GameInterface::journalRead(U32 size, void *buffer)
{
   gJournalStream.read(size, buffer);
}

void GameInterface::journalWrite(U32 size, const void *buffer)
{
   gJournalStream.write(size, buffer);
}

