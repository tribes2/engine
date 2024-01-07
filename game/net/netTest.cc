//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/simBase.h"
#include "platform/event.h"
#include "sim/netConnection.h"
#include "core/bitStream.h"
#include "sim/netObject.h"

//#pragma message "MDF: Make sure this file gets taken out before we go gold"

class SimpleMessageEvent : public NetEvent
{
   char *msg;
public:
   SimpleMessageEvent(const char *message = NULL)
      {
         if(message)
            msg = dStrdup(message);
         else
            msg = NULL;
      }
   ~SimpleMessageEvent()
      { dFree(msg); }
   
   virtual void pack(NetConnection* /*ps*/, BitStream *bstream)
      { bstream->writeString(msg); }
   virtual void write(NetConnection*, BitStream *bstream)
      { bstream->writeString(msg); }
   virtual void unpack(NetConnection* /*ps*/, BitStream *bstream)
      { char buf[256]; bstream->readString(buf); msg = dStrdup(buf); }
   virtual void process(NetConnection *)
      { Con::printf("RMSG %d  %s", mSourceId, msg); }

   DECLARE_CONOBJECT(SimpleMessageEvent);
};

IMPLEMENT_CO_NETEVENT_V1(SimpleMessageEvent);

class SimpleNetObject : public NetObject
{
public:
   char message[256];
   SimpleNetObject()
   {
      mNetFlags.set(ScopeAlways | Ghostable);
      dStrcpy(message, "Hello World!");
   }
   U32 packUpdate(NetConnection *, U32 /*mask*/, BitStream *stream)
   {
      stream->writeString(message);
      return 0;
   }
   void unpackUpdate(NetConnection *, BitStream *stream)
   {
      stream->readString(message);
      Con::printf("Got message: %s", message);
   }
   void setMessage(const char *msg)
   {
      setMaskBits(1);
      dStrcpy(message, msg);
   }
   static void consoleInit();

   DECLARE_CONOBJECT(SimpleNetObject);
};

IMPLEMENT_CO_NETOBJECT_V1(SimpleNetObject);

static void cSNOSetMessage(SimObject *sno, S32, const char **argv)
{
   ((SimpleNetObject *) sno)->setMessage(argv[2]);
}

static void cMsg(SimObject *, S32, const char **argv)
{
   NetConnection *con = (NetConnection *) Sim::findObject(argv[1]);
   if(con)
      con->postNetEvent(new SimpleMessageEvent(argv[2]));
}

void SimpleNetObject::consoleInit()
{
   Con::addCommand("SimpleNetObject", "setMessage", cSNOSetMessage, "obj.setMessage(msg)", 3, 3);
   Con::addCommand("msg", cMsg, "msg(id,msg);", 3, 3);
}

