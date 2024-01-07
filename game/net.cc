//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "core/dnet.h"
#include "console/simBase.h"
#include "sim/netConnection.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/bitStream.h"
#include "sim/netObject.h"
#include "game/gameConnection.h"
#include "core/idGenerator.h"
#include "game/serverQuery.h"
#include "game/targetManager.h"

static char gMasterAddress[256];

//----------------------------------------------------------------
// remote procedure call console functions
//----------------------------------------------------------------

class RemoteCommandEvent : public NetEvent
{
public:
   enum {
      MaxRemoteCommandArgs = 20,
      CommandArgsBits = 5
   };
   
private:
   S32 mArgc;
   char *mArgv[MaxRemoteCommandArgs + 1];
   U32 mTagv[MaxRemoteCommandArgs + 1];
   static char mBuf[1024];
public:
   RemoteCommandEvent(S32 argc=0, const char **argv=NULL)
   {
      mArgc = argc;
      for(S32 i = 0; i < argc; i++)
      {
         mArgv[i+1] = dStrdup(argv[i]);
         if(argv[i][0] == StringTagPrefixByte)
         {
            mTagv[i+1] = dAtoi(argv[i]+1);
            gNetStringTable->incStringRef(mTagv[i+1]);
         }
         else
            mTagv[i+1] = 0;
      }
   }
#ifdef DEBUG_NET
   const char *getDebugName()
   {
      static char buffer[256];
      dSprintf(buffer, sizeof(buffer), "%s [%s]", getClassName(), gNetStringTable->lookupString(dAtoi(mArgv[1] + 1)) );
      return buffer;
   }
#endif
   void notifyDelivered(NetConnection* , bool)
   {
      // Moved this to the destructor:
//       for(S32 i = 0; i < mArgc; i++)
//          if(mTagv[i+1])
//             gNetStringTable->removeString(mTagv[i+1]);
   }
   ~RemoteCommandEvent()
   {
      for(S32 i = 0; i < mArgc; i++)
      {
         if(mTagv[i+1])
            gNetStringTable->removeString(mTagv[i+1]);
         dFree(mArgv[i+1]);
      }
   }
   
   virtual void pack(NetConnection* conn, BitStream *bstream)
   {
      bstream->writeInt(mArgc, CommandArgsBits);
      // write it out reversed... why?
      // automatic string substitution with later arguments -
      // handled automatically by the system.
      
      for(S32 i = 0; i < mArgc; i++)
         conn->packString(bstream, mArgv[i+1]);
   }

   virtual void write(NetConnection* conn, BitStream *bstream)
   {
      pack(conn, bstream);
   }
   
   virtual void unpack(NetConnection* conn, BitStream *bstream)
   {
      
      mArgc = bstream->readInt(CommandArgsBits);
      // read it out backwards
      for(S32 i = 0; i < mArgc; i++)
      {
         conn->unpackString(bstream, mBuf);
         mArgv[i+1] = dStrdup(mBuf);
         mTagv[i+1] =0;
      }
   }
   
   virtual void process(NetConnection *conn)
   {
      static char idBuf[10];

      // de-tag the command name
      
      for(S32 i = mArgc - 1; i >= 0; i--)
      {
         char *arg = mArgv[i+1];
         if(*arg == StringTagPrefixByte)
         {
            // it's a tag:
            U32 tag = conn->translateRemoteStringId(dAtoi(arg+1));
            NetStringTable::expandString( tag, 
                                          mBuf, 
                                          sizeof(mBuf), 
                                          (mArgc - 1) - i, 
                                          (const char**)(mArgv + i + 2) );
            dFree(mArgv[i+1]);
            mArgv[i+1] = dStrdup(mBuf);
         }
      }
      const char *rmtCommandName = dStrchr(mArgv[1], ' ') + 1;
      if(conn->isServerConnection())
      {
         dStrcpy(mBuf, "clientCmd");
         dStrcat(mBuf, rmtCommandName);
         
         char *temp = mArgv[1];
         mArgv[1] = mBuf;
         
         Con::execute(mArgc, (const char **) mArgv+1);
         mArgv[1] = temp;
      }
      else
      {
         dStrcpy(mBuf, "serverCmd");
         dStrcat(mBuf, rmtCommandName);
         char *temp = mArgv[1];
         
         dSprintf(idBuf, sizeof(idBuf), "%d", conn->getId());
         mArgv[0] = mBuf;
         mArgv[1] = idBuf;
         
         Con::execute(mArgc+1, (const char **) mArgv);
         mArgv[1] = temp;
      }
   }

   DECLARE_CONOBJECT(RemoteCommandEvent);
};
char RemoteCommandEvent::mBuf[1024];

IMPLEMENT_CO_NETEVENT_V1(RemoteCommandEvent);

static void sendRemoteCommand(NetConnection *conn, S32 argc, const char **argv)
{
   if(U8(argv[0][0]) != StringTagPrefixByte)
   {
      Con::errorf(ConsoleLogEntry::Script, "Remote Command Error - command must be a tag.");
      return;
   }
   S32 i;
   for(i = argc - 1; i >= 0; i--)
   {
      if(argv[i][0] != 0)
         break;
      argc = i;
   }
   for(i = 0; i < argc; i++)
      conn->validateSendString(argv[i]);
   RemoteCommandEvent *cevt = new RemoteCommandEvent(argc, argv);
   conn->postNetEvent(cevt);
}

static void cCommandToServer(SimObject *, S32 argc, const char **argv)
{
   NetConnection *conn = NetConnection::getServerConnection();
   if(!conn)
      return;
   sendRemoteCommand(conn, argc - 1, argv + 1);
}

static void cCommandToClient(SimObject *, S32 argc, const char **argv)
{
   NetConnection *conn;
   if(!Sim::findObject(argv[1], conn))
      return;
   sendRemoteCommand(conn, argc - 2, argv + 2);
}

//----------------------------------------------------------------
// Console function registration / processing
//----------------------------------------------------------------

void cRemoveTaggedString(SimObject *, S32, const char **argv)
{
   gNetStringTable->removeString(dAtoi(argv[1]+1));
}

const char *cAddTaggedString(SimObject *, S32, const char **argv)
{
   U32 id = gNetStringTable->addString(argv[1]);
   char *ret = Con::getReturnBuffer(10);
   ret[0] = StringTagPrefixByte;
   dSprintf(ret + 1, 9, "%d", id);
   return ret;
}

const char *cGetTaggedString(SimObject *, S32, const char **argv)
{
   const char *indexPtr = argv[1];
   if (*indexPtr == StringTagPrefixByte)
      indexPtr++;
   return gNetStringTable->lookupString(dAtoi(indexPtr));
}

const char *cBuildTaggedString(SimObject *, S32 argc, const char **argv)
{
   const char *indexPtr = argv[1];
   if (*indexPtr == StringTagPrefixByte)
      indexPtr++;
   const char *fmtString = gNetStringTable->lookupString(dAtoi(indexPtr));
   char *strBuffer = Con::getReturnBuffer(512);
   const char *fmtStrPtr = fmtString;
   char *strBufPtr = strBuffer;
   S32 strMaxLength = 511;
   if (!fmtString)
      goto done;

   //build the string
   while (*fmtStrPtr)
   {
      //look for an argument tag
      if (*fmtStrPtr == '%')
      {
         if (fmtStrPtr[1] >= '1' && fmtStrPtr[1] <= '9')
         {
            S32 argIndex = S32(fmtStrPtr[1] - '0') + 1;
            if (argIndex >= argc)
               goto done;
            const char *argStr = argv[argIndex];
            if (!argStr)
               goto done;
            S32 strLength = dStrlen(argStr);
            if (strLength > strMaxLength)
               goto done;
            dStrcpy(strBufPtr, argStr);
            strBufPtr += strLength;
            strMaxLength -= strLength;
            fmtStrPtr += 2;
            continue;
         }
      }

      //if we don't continue, just copy the character
      if (strMaxLength <= 0)
         goto done;
      *strBufPtr++ = *fmtStrPtr++;
      strMaxLength--;
   }

done:
   *strBufPtr = '\0';
   return strBuffer;
}

void netInit()
{
   Con::addCommand("addTaggedString", cAddTaggedString, "addTaggedString(string)", 2, 2);
   Con::addCommand("removeTaggedString", cRemoveTaggedString, "removeTaggedString(tag)", 2, 2);
   Con::addCommand("getTaggedString", cGetTaggedString, "getTaggedString(tag)", 2, 2);

   Con::addCommand("buildTaggedString", cBuildTaggedString, "buildTaggedString(fmtTag, <arg1, ...arg9>);", 2, 11);

   Con::addCommand("commandToServer", cCommandToServer, "commandToServer(func, <arg1,...argn>);", 2, RemoteCommandEvent::MaxRemoteCommandArgs + 1);
   Con::addCommand("commandToClient", cCommandToClient, "commandToClient(client, func, <arg1,...argn>);", 3, RemoteCommandEvent::MaxRemoteCommandArgs + 2);

   Con::addVariable( "MasterServerAddress", TypeString, &gMasterAddress );
}
