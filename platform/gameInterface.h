//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GAMEINTERFACE_H_
#define _GAMEINTERFACE_H_

class FileStream;

class GameInterface
{
public:
   enum JournalMode {
      JournalOff,
      JournalSave,
      JournalPlay,
      JournalLoad,
   };
private:
   JournalMode mJournalMode;   
   bool mRunning;
public:
   GameInterface();
   
   // calls from the platform into the game:
   virtual int main(int argc, const char **argv);
   virtual void textureKill();
   virtual void textureResurrect();
   virtual void refreshWindow();

   virtual void postEvent(Event &event);

   // event handlers:
   // default event behavior with journaling support
   // default handler forwards events to appropriate routines
   virtual void processEvent(Event *event);

   virtual void processPacketReceiveEvent(PacketReceiveEvent *event);
   virtual void processMouseMoveEvent(MouseMoveEvent *event);
   virtual void processInputEvent(InputEvent *event);
   virtual void processQuitEvent();
   virtual void processTimeEvent(TimeEvent *event);
   virtual void processConsoleEvent(ConsoleEvent *event);
   virtual void processConnectedAcceptEvent(ConnectedAcceptEvent *event);
   virtual void processConnectedReceiveEvent(ConnectedReceiveEvent *event);
   virtual void processConnectedNotifyEvent(ConnectedNotifyEvent *event);

   void setRunning(bool running) { mRunning = running; }
   bool isRunning() { return mRunning; }
   
   void journalProcess();
   void loadJournal(const char *fileName);
   void saveJournal(const char *fileName);
   void playJournal(const char *fileName);
   
   JournalMode getJournalMode() { return mJournalMode; };

   bool isJournalReading() { return mJournalMode == JournalLoad || mJournalMode == JournalPlay; }
   bool isJournalWriting() { return mJournalMode == JournalSave; }

   void journalRead(U32 *val);
   void journalWrite(U32 val);
   void journalRead(U32 size, void *buffer);
   void journalWrite(U32 size, const void *buffer); 

   FileStream *getJournalStream();
};

extern GameInterface *Game;

#endif
