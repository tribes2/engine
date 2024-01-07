//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "console/simBase.h"
#include "Platform/event.h"
#include "console/consoleInternal.h"
#include "game/tcpObject.h"
#include "Platform/gameInterface.h"
#include "game/tribesGame.h"

TCPObject *TCPObject::table[TCPObject::TableSize] = {0, };

IMPLEMENT_CONOBJECT(TCPObject);

TCPObject *TCPObject::find(NetSocket tag)
{
   for(TCPObject *walk = table[U32(tag) & TableMask]; walk; walk = walk->mNext)
      if(walk->mTag == tag)
         return walk;
   return NULL;
}

void TCPObject::addToTable(NetSocket newTag)
{
   removeFromTable();
   mTag = newTag;
   mNext = table[U32(mTag) & TableMask];
   table[U32(mTag) & TableMask] = this;
}

void TCPObject::removeFromTable()
{
   for(TCPObject **walk = &table[U32(mTag) & TableMask]; *walk; walk = &((*walk)->mNext))
   {
      if(*walk == this)
      {
         *walk = mNext;
         return;
      }
   }      
}

TCPObject::TCPObject()
{
   mBuffer = NULL;
   mBufferSize = 0;
   mPort = 0;
   mTag = InvalidSocket;
   mNext = NULL;
   mState = Disconnected;
}

TCPObject::~TCPObject()
{
   disconnect();
   dFree(mBuffer);
}

bool TCPObject::processArguments(S32 argc, const char **argv)
{
   if(argc == 0)
      return true;
   else if(argc == 1)      
   {
      addToTable(U32(dAtoi(argv[0])));
      return true;
   }
   return false;
}

bool TCPObject::onAdd()
{
   if(!Parent::onAdd())
      return false;
   const char *name = getName();
   if(name && name[0] && getClassRep())
   {
      Namespace *parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);
   
   }
   Sim::getTCPGroup()->addObject(this);
   return true;
}


U32 TCPObject::onReceive(U8 *buffer, U32 bufferLen)
{
   // we got a raw buffer event
   // default action is to split the buffer into lines of text
   // and call processLine on each
   // for any incomplete lines we have mBuffer
   U32 start = 0;
   parseLine(buffer, &start, bufferLen);
   return start;
}

void TCPObject::parseLine(U8 *buffer, U32 *start, U32 bufferLen)
{
   // find the first \n in buffer
   U32 i;
   U8 *line = buffer + *start;

   for(i = *start; i < bufferLen; i++)
      if(buffer[i] == '\n' || buffer[i] == 0)
         break;
   U32 len = i - *start;

   if(i == bufferLen || mBuffer)
   {
      // we've hit the end with no newline
      mBuffer = (U8 *) dRealloc(mBuffer, mBufferSize + len + 1);
      dMemcpy(mBuffer + mBufferSize, line, len);
      mBufferSize += len;
      *start = i;

      // process the line
      if(i != bufferLen)
      {
         mBuffer[mBufferSize] = 0;
         if(mBufferSize && mBuffer[mBufferSize-1] == '\r')
            mBuffer[mBufferSize - 1] = 0;
         U8 *temp = mBuffer;
         mBuffer = 0;
         mBufferSize = 0;
         
         processLine(temp);
         dFree(temp);
      }
   }
   else if(i != bufferLen)
   {
      line[len] = 0;
      if(len && line[len-1] == '\r')
         line[len-1] = 0;
      processLine(line);
   }
   if(i != bufferLen)
      *start = i + 1;
}

void TCPObject::onConnectionRequest(const NetAddress *addr, U32 connectId)
{
   char idBuf[16];
   char addrBuf[256];
   Net::addressToString(addr, addrBuf);
   dSprintf(idBuf, sizeof(idBuf), "%d", connectId);
   Con::executef(this, 3, "onConnectRequest", addrBuf, idBuf);
}

bool TCPObject::processLine(U8 *line)
{
   Con::executef(this, 2, "onLine", line);
   return true;
}

void TCPObject::onDNSResolved()
{
   mState = DNSResolved;
   Con::executef(this, 1, "onDNSResolved");
}

void TCPObject::onDNSFailed()
{
   mState = Disconnected;
   Con::executef(this, 1, "onDNSFailed");
}

void TCPObject::onConnected()
{
   mState = Connected;
   Con::executef(this, 1, "onConnected");
}

void TCPObject::onConnectFailed()
{
   mState = Disconnected;
   Con::executef(this, 1, "onConnectFailed");
}

void TCPObject::finishLastLine()
{
   if(mBufferSize)
   {
      mBuffer[mBufferSize] = 0;
      processLine(mBuffer);
      dFree(mBuffer);
      mBuffer = 0;
      mBufferSize = 0;
   }
}

void TCPObject::onDisconnect()
{
   finishLastLine();
   mState = Disconnected;
   Con::executef(this, 1, "onDisconnect");
}

void TCPObject::listen(U16 port)
{
   mState = Listening;
   U32 newTag = Net::openListenPort(port);
   addToTable(newTag);
}

void TCPObject::connect(const char *address)
{
   NetSocket newTag = Net::openConnectTo(address);
   addToTable(newTag);
}

void TCPObject::disconnect()
{
   if( mTag != InvalidSocket ) {
      Net::closeConnectTo(mTag);
   }
   removeFromTable();
}

void TCPObject::send(const U8 *buffer, U32 len)
{
   Net::sendTo(mTag, buffer, S32(len));
}

static void cTCPObjectSend(SimObject *obj, S32 argc, const char **argv)
{
   TCPObject *tcpo = (TCPObject *) obj;
   for(S32 i = 2; i < argc; i++)
      tcpo->send((const U8 *) argv[i], dStrlen(argv[i]));
}

static void cTCPObjectListen(SimObject *obj, S32, const char **argv)
{
   TCPObject *tcpo = (TCPObject *) obj;
   tcpo->listen(U32(dAtoi(argv[2])));
}

static void cTCPObjectConnect(SimObject *obj, S32, const char **argv)
{
   TCPObject *tcpo = (TCPObject *) obj;
   tcpo->connect(argv[2]);
}

static void cTCPObjectDisconnect(SimObject *obj, S32, const char **)
{
   TCPObject *tcpo = (TCPObject *) obj;
   tcpo->disconnect();
}

void TCPObject::consoleInit()
{
   Con::addCommand("TCPObject", "listen", cTCPObjectListen, "obj.listen(port)", 3, 3);
   Con::addCommand("TCPObject", "send", cTCPObjectSend, "obj.send(string, <string> ...)", 3, 0);
   Con::addCommand("TCPObject", "connect", cTCPObjectConnect, "obj.connect(addr)", 3, 3);
   Con::addCommand("TCPObject", "disconnect", cTCPObjectDisconnect, "obj.disconnect()", 2, 2);
}


void TribesGame::processConnectedReceiveEvent(ConnectedReceiveEvent* event )
{
   TCPObject *tcpo = TCPObject::find(event->tag);
   if(!tcpo)
   {
      Con::printf("Got bad connected receive event.");
      return;
   }
   U32 size = U32(event->size - ConnectedReceiveEventHeaderSize);
   U8 *buffer = event->data;
   
   while(size)
   {
      U32 ret = tcpo->onReceive(buffer, size);
      AssertFatal(ret <= size, "Invalid return size");
      size -= ret;
      buffer += ret;
   }
}

void TribesGame::processConnectedAcceptEvent( ConnectedAcceptEvent* event )
{
   TCPObject *tcpo = TCPObject::find(event->portTag);
   if(!tcpo)
      return;
   tcpo->onConnectionRequest(&event->address, event->connectionTag);
}

void TribesGame::processConnectedNotifyEvent( ConnectedNotifyEvent* event )
{
   TCPObject *tcpo = TCPObject::find(event->tag);
   if(!tcpo)
      return;
   switch(event->state)
   {
      case ConnectedNotifyEvent::DNSResolved:
         tcpo->onDNSResolved();
         break;
      case ConnectedNotifyEvent::DNSFailed:
         tcpo->onDNSFailed();
         break;
      case ConnectedNotifyEvent::Connected:
         tcpo->onConnected();
         break;
      case ConnectedNotifyEvent::ConnectFailed:
         tcpo->onConnectFailed();
         break;
      case ConnectedNotifyEvent::Disconnected:
         tcpo->onDisconnect();
         break;
   }
}
