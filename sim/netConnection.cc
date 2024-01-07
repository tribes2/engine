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
#include "core/bitStream.h"
#include "sim/pathManager.h"
#include "core/fileStream.h"
#include "core/resManager.h"
#include "sim/pathManager.h"
#include "console/consoleTypes.h"
#include <stdarg.h>
#include "game/badWordFilter.h"

enum {
   PingTimeout = 4500, // milliseconds
   DefaultPingRetryCount = 15,
};

NetEvent::~NetEvent()
{
}

//--------------------------------------------------------------------

class NetStringEvent : public NetEvent
{
   char *mString;
   U32 mId;
public:
   NetStringEvent(U32 id = 0)
   {
      mId = id;
      mString = NULL;
   }
   ~NetStringEvent()
   {
      dFree(mString);
   }
   
   virtual void pack(NetConnection* /*ps*/, BitStream *bstream)
   {
      const char *str = gNetStringTable->lookupString(mId);
      bstream->writeInt(mId, NetStringTable::StringIdBitSize);
      bstream->writeString(str);
   }
   virtual void write(NetConnection* /*ps*/, BitStream *bstream)
   {
      bstream->writeInt(mId, NetStringTable::StringIdBitSize);
      bstream->writeString(mString);
   }
   virtual void unpack(NetConnection* con, BitStream *bstream)
   {
      char buf[256];
      mId = bstream->readInt(NetStringTable::StringIdBitSize);
      bstream->readString(buf);
      if(gBadWordFilter->isEnabled()  && con->isServerConnection())
         gBadWordFilter->filterString(buf);
      mString = dStrdup(buf);
   }
   virtual void process(NetConnection *connection)
   {
      if(!mString[0])
         return;
      U32 localId;
      localId = gNetStringTable->addString(mString);
      connection->mapString(mId, localId);
   }
#ifdef DEBUG_NET
   const char *getDebugName()
   {
      static char buffer[512];
      dSprintf(buffer, sizeof(buffer), "%s - \"", getClassName());
      expandEscape(buffer + dStrlen(buffer), gNetStringTable->lookupString(mId));
      dStrcat(buffer, "\"");
      return buffer;
   }
#endif
   DECLARE_CONOBJECT(NetStringEvent);
};

IMPLEMENT_CO_NETEVENT_V1(NetStringEvent);

//--------------------------------------------------------------------
IMPLEMENT_CONOBJECT(NetConnection);
NetConnection* NetConnection::mConnectionList = NULL;
NetConnection* NetConnection::mHashTable[NetConnection::HashTableSize] = { NULL, };
NetConnection* NetConnection::mServerConnection = NULL;
NetConnection* NetConnection::mLocalClientConnection = NULL;

static inline U32 HashNetAddress(const NetAddress *addr)
{
   return *((U32 *)addr->netNum) % NetConnection::HashTableSize;
}

NetConnection *NetConnection::lookup(const NetAddress *addr)
{
   U32 hashIndex = HashNetAddress(addr);
   for(NetConnection *walk = mHashTable[hashIndex]; walk; walk = walk->mNextTableHash)
      if(Net::compareAddresses(addr, walk->getNetAddress()))
         return walk;
   return NULL;
}

void NetConnection::netAddressTableInsert()
{
   U32 hashIndex = HashNetAddress(&mNetAddress);
   mNextTableHash = mHashTable[hashIndex];
   mHashTable[hashIndex] = this;
}

void NetConnection::netAddressTableRemove()
{
   U32 hashIndex = HashNetAddress(&mNetAddress);
   NetConnection **walk = &mHashTable[hashIndex];
   while(*walk)
   {
      if(*walk == this)
      {
         *walk = mNextTableHash;
         mNextTableHash = NULL;
         return;
      }
      walk = &((*walk)->mNextTableHash);
   }
}

void NetConnection::setNetAddress(const NetAddress *addr)
{
   netAddressTableRemove();
   mNetAddress = *addr;
   netAddressTableInsert();
}

const NetAddress *NetConnection::getNetAddress()
{
   return &mNetAddress;
}

void NetConnection::setSequences(U32 ccs, U32 css)
{
   mClientConnectSequence = ccs;
   mServerConnectSequence = css;
}

void NetConnection::getSequences(U32 *ccs, U32 *css)
{
   *ccs = mClientConnectSequence;
   *css = mServerConnectSequence;
}

void NetConnection::setServerConnection(NetConnection *conn)
{
   mServerConnection = conn;
}

void NetConnection::setLocalClientConnection(NetConnection *conn)
{
   mLocalClientConnection = conn;
}


static U32 gPacketRateToServer = 32;
static U32 gPacketUpdateDelayToServer = 32;
static U32 gPacketRateToClient = 10;
static U32 gPacketSize = 200;

void NetConnection::consoleInit()
{
   Con::addVariable("pref::Net::PacketRateToServer",  TypeS32, &gPacketRateToServer);
   Con::addVariable("pref::Net::PacketRateToClient",  TypeS32, &gPacketRateToClient);
   Con::addVariable("pref::Net::PacketSize",          TypeS32, &gPacketSize);
}

void NetConnection::checkMaxRate()
{
   if(gPacketRateToServer > 32)
      gPacketRateToServer = 32;
   if(gPacketRateToServer < 8)
      gPacketRateToServer = 8;
   if(gPacketRateToClient > 32)
      gPacketRateToClient = 32;
   if(gPacketRateToClient < 1)
      gPacketRateToClient = 1;
   if(gPacketSize > 450)
      gPacketSize = 450;
   if(gPacketSize < 100)
      gPacketSize = 100;

   gPacketUpdateDelayToServer = 1024 / gPacketRateToServer;
   U32 toClientUpdateDelay = 1024 / gPacketRateToClient;

   if(mMaxRate.updateDelay != toClientUpdateDelay || mMaxRate.packetSize != gPacketSize)
   {
      mMaxRate.updateDelay = toClientUpdateDelay;
      mMaxRate.packetSize = gPacketSize;
      mMaxRate.changed = true;
   }
}

NetConnection::NetConnection(bool ghostFrom, bool ghostTo, bool sendEvents)
{
   mClientConnectSequence = 0;
   mServerConnectSequence = 0;
   
   mSendingEvents = sendEvents;
   
   mSimulatedPing = 0;
   mSimulatedPacketLoss = 0;
#ifdef DEBUG_NET
   mLogging = false;
#endif   
   mIsNetworkConnection = false;
   mConnectionObjectId = 0;
   mLastUpdateTime = 0;
   mRoundTripTime = 0;
   mPacketLoss = 0;
   mNextTableHash = NULL;
   
   mNextConnection = NULL;
   mPrevConnection = NULL;
   
   mNotifyQueueHead = NULL;
   mNotifyQueueTail = NULL;
   
   mCurRate.updateDelay = 102;
   mCurRate.packetSize = 200;
   mCurRate.changed = false;
   mMaxRate.updateDelay = 102;
   mMaxRate.packetSize = 200;
   mMaxRate.changed = false;
   checkMaxRate();

   // event management data:

   mNotifyEventList = NULL;
   mSendEventQueueHead = NULL;
   mSendEventQueueTail = NULL;
   mUnorderedSendEventQueueHead = NULL;
   mUnorderedSendEventQueueTail = NULL;
   mWaitSeqEvents = NULL;

   mNextSendEventSeq = FirstValidSendEventSeq;
   mNextRecvEventSeq = FirstValidSendEventSeq;
   mLastAckedEventSeq = -1;
   
   // ghost management data:

   mGhostFrom = ghostFrom;
   mGhostTo = ghostTo;

   mScopeObject = NULL;
   mGhostingSequence = 0;
   mGhosting = false;
   mScoping = false;
   
   if(mGhostFrom)
   {
      mGhostFreeIndex = mGhostZeroUpdateIndex = 0;
      mGhostArray = new GhostInfo *[MaxGhostCount];
      mGhostRefs = new GhostInfo[MaxGhostCount];
      S32 i;
      for(i = 0; i < MaxGhostCount; i++)
      {
         mGhostRefs[i].obj = NULL;
         mGhostRefs[i].index = i;
         mGhostRefs[i].updateMask = 0;
      }
      mGhostLookupTable = new GhostInfo *[GhostLookupTableSize];
      for(i = 0; i < GhostLookupTableSize; i++)
         mGhostLookupTable[i] = 0;
   }
   else
   {
      mGhostArray = NULL;
      mGhostRefs = NULL;
      mGhostLookupTable = NULL;
   }
   
   if(mGhostTo)
   {
      mLocalGhosts = new NetObject *[MaxGhostCount];
      for(S32 i = 0; i < MaxGhostCount; i++)
         mLocalGhosts[i] = NULL;
   }
   else
      mLocalGhosts = NULL;
      
   U32 i;
   for(i = 0; i < NetStringTable::MaxStrings; i++)
      mStringXLTable[i] = 0;
   for(i = 0; i < NetStringTable::MaxStrings >> 3; i++)
      mStringSentBitArray[i] = 0;

   mMissionPathsSent = false;
   mDemoWriteStream = NULL;
   mDemoReadStream = NULL;
   
   mPingSendCount = 0;
   mPingRetryCount = DefaultPingRetryCount;
   mLastPingSendTime = Platform::getVirtualMilliseconds();
}

NetConnection::~NetConnection()
{
   AssertFatal(mNotifyQueueHead == NULL, "Uncleared notifies remain.");
   netAddressTableRemove();
   
   delete[] mLocalGhosts;
   delete[] mGhostLookupTable;
   delete[] mGhostRefs;
   delete[] mGhostArray;
   if(mDemoWriteStream)
      delete mDemoWriteStream;
   if(mDemoReadStream)
      ResourceManager->closeStream(mDemoReadStream);
}

NetConnection::PacketNotify::PacketNotify()
{
   rateChanged = false;
   maxRateChanged = false;
   sendTime = 0;
   eventList = 0;
   ghostList = 0;
}

bool NetConnection::checkTimeout(U32 time)
{
   if(!isNetworkConnection())
      return false;
      
   if(time > mLastPingSendTime + PingTimeout)
   {
      if(mPingSendCount >= mPingRetryCount)
         return true;
      mLastPingSendTime = time;
      mPingSendCount++;
      sendPingPacket();
   }
   return false;
}

void NetConnection::keepAlive()
{
   mLastPingSendTime = Platform::getVirtualMilliseconds();
   mPingSendCount = 0;
}

void NetConnection::handleConnectionEstablished()
{
}

//--------------------------------------------------------------------------

ConsoleMethod(NetConnection,transmitPaths,void,2,2,"conn.transmitPaths();")
{
   argc; argv;
   NetConnection *cptr = (NetConnection *) object;

   gServerPathManager->transmitPaths(cptr);
   cptr->setMissionPathsSent(true);
}

ConsoleMethod(NetConnection,clearPaths,void,2,2,"conn.clearPaths();")
{
   argc; argv;
   NetConnection *cptr = (NetConnection *) object;
   cptr->setMissionPathsSent(false);
}

ConsoleMethod(NetConnection,getAddress,const char *,2,2,"conn.getAddress();")
{
   argc; argv;
   NetConnection *con = (NetConnection *) object;
   if(con->isLocalConnection())
      return "local";
   char *buffer = Con::getReturnBuffer(256);
   Net::addressToString(con->getNetAddress(), buffer);
   return buffer;
}

ConsoleMethod(NetConnection,setSimulatedNetParams,void,4, 4,"conn.setSimulatedNetParams(packetLoss,delay);")
{
   argc;
   NetConnection *con = (NetConnection *) object;
   con->setSimulatedNetParams(dAtof(argv[2]), dAtoi(argv[3]));
}

ConsoleMethod( NetConnection, getPing, S32, 2, 2, "conn.getPing()" )
{
   argc; argv;
   NetConnection* conn = (NetConnection*) object;
   return( S32( conn->getRoundTripTime() ) );
}

ConsoleMethod( NetConnection, getPacketLoss, S32, 2, 2, "conn.getPacketLoss()" )
{
   argc; argv;
   NetConnection* conn = (NetConnection*) object;
   return( S32( 100 * conn->getPacketLoss() ) );
}

ConsoleMethod( NetConnection, checkMaxRate, void, 2, 2, "conn.checkMaxRate()")
{
   argc; argv;
   NetConnection* conn = (NetConnection*) object;
   conn->checkMaxRate();
}

#ifdef DEBUG_NET

ConsoleMethod( NetConnection, setLogging, void, 3, 3, "conn.setLogging(bool)")
{
   argc;
   ((NetConnection *) object)->setLogging(dAtob(argv[2]));
}

#endif 

void NetConnection::mapString(U32 remoteId, U32 localId)
{
   if(mStringXLTable[remoteId])
      gNetStringTable->removeString(mStringXLTable[remoteId]);
   mStringXLTable[remoteId] = localId;
}

void NetConnection::clearString(U32 id)
{
   mStringSentBitArray[id >> 3] &= ~(1 << (id & 0x7));
}

void NetConnection::checkString(U32 id)
{
   if(!id)  
      return;
   if(!(mStringSentBitArray[id >> 3] & (1 << (id & 0x7))))
   {
      mStringSentBitArray[id >> 3] |= (1 << (id & 0x7));
      postNetEvent(new NetStringEvent(id));
   }
}

//--------------------------------------------------------------------

bool NetConnection::onAdd()
{
   if(!Parent::onAdd())
      return false;
   mNextConnection = mConnectionList;
   if(mConnectionList)
      mConnectionList->mPrevConnection = this;
   mConnectionList = this;
   return true;
}

void NetConnection::onRemove()
{
   if(mLocalClientConnection == this)
      mLocalClientConnection = NULL;
   if(mServerConnection == this)
      mServerConnection = NULL;

   if(mNextConnection)
      mNextConnection->mPrevConnection = mPrevConnection;
   if(mPrevConnection)
      mPrevConnection->mNextConnection = mNextConnection;
   else
      mConnectionList = mNextConnection;
   while(mNotifyQueueHead)
      handleNotify(false);
   
   if(mGhostFrom)
      clearGhostInfo();   
   for(U32 i = 0; i < NetStringTable::MaxStrings; i++)
      if(mStringXLTable[i])
         gNetStringTable->removeString(mStringXLTable[i]);
   
   while(mNotifyEventList)
   {
      NetEvent *temp = mNotifyEventList;
      mNotifyEventList = temp->mNextEvent;
      
      temp->notifyDelivered(this, true);
      delete temp;
   }
   while(mUnorderedSendEventQueueHead)
   {
      NetEvent *temp = mUnorderedSendEventQueueHead;
      mUnorderedSendEventQueueHead = temp->mNextEvent;
      
      temp->notifyDelivered(this, true);
      delete temp;
   }
   while(mSendEventQueueHead)
   {
      NetEvent *temp = mSendEventQueueHead;
      mSendEventQueueHead = temp->mNextEvent;
      
      temp->notifyDelivered(this, true);
      delete temp;
   }
   Parent::onRemove();
}

char NetConnection::mErrorBuffer[256];

void NetConnection::setLastError(const char *fmt, ...)
{
   va_list argptr;
   va_start(argptr, fmt);
   dVsprintf(mErrorBuffer, sizeof(mErrorBuffer), fmt, argptr);
   // setLastErrors assert in debug builds
   AssertFatal(0, mErrorBuffer);
   va_end(argptr);
}

//--------------------------------------------------------------------

void NetConnection::handleNotify(bool recvd)
{
//   Con::printf("NET  %d: NOTIFY - %d %s", getId(), gPacketId, recvd ? "RECVD" : "DROPPED");

   PacketNotify *note = mNotifyQueueHead;
   AssertFatal(note != NULL, "Error: got a notify with a null notify head.");
   mNotifyQueueHead = mNotifyQueueHead->nextPacket;

   if(note->rateChanged && !recvd)
      mCurRate.changed = true;
   if(note->maxRateChanged && !recvd)
      mMaxRate.changed = true;

   if(recvd) {
      // Running average of roundTrip time
      U32 curTime = Platform::getVirtualMilliseconds();
      mRoundTripTime = (mRoundTripTime + (curTime - note->sendTime)) * 0.5;
      packetReceived(note);
   }
   else
      packetDropped(note);
   delete note;
}

void NetConnection::processRawPacket(BitStream *bstream)
{
   if(mDemoWriteStream)
      recordBlock(Sim::getCurrentTime(), BlockTypePacket, bstream->getReadByteSize(), bstream->getBuffer());
   ConnectionProtocol::processRawPacket(bstream);
}

void NetConnection::handlePacket(BitStream *bstream)
{
//   Con::printf("NET  %d: RECV - %d", getId(), mLastSeqRecvd);
   // clear out any errors
   mErrorBuffer[0] = 0;
   if(bstream->readFlag())
   {
      mCurRate.updateDelay = bstream->readInt(10);
      mCurRate.packetSize = bstream->readInt(10);
   }
   if(bstream->readFlag())
   {
      U32 omaxDelay = bstream->readInt(10);
      S32 omaxSize = bstream->readInt(10);
      if(omaxDelay < mMaxRate.updateDelay)
         omaxDelay = mMaxRate.updateDelay;
      if(omaxSize > mMaxRate.packetSize)
         omaxSize = mMaxRate.packetSize;
      if(omaxDelay != mCurRate.updateDelay || omaxSize != mCurRate.packetSize)
      {
         mCurRate.updateDelay = omaxDelay;
         mCurRate.packetSize = omaxSize;
         mCurRate.changed = true;
      }
   }
   readPacket(bstream);
   if(mErrorBuffer[0])
      connectionError(mErrorBuffer);
}

void NetConnection::connectionError(const char *errorString)
{
   errorString;
}

//--------------------------------------------------------------------

void NetConnection::setRemoteConnectionObjectId(SimObjectId id)
{
   mConnectionObjectId = id;
}

NetConnection::PacketNotify *NetConnection::allocNotify()
{
   return new PacketNotify;
}

class NetDelayEvent : public SimEvent
{
   U8 buffer[MaxPacketDataSize];
   BitStream stream;
public:
   NetDelayEvent(BitStream *inStream) : stream(NULL, 0)
   {
      dMemcpy(buffer, inStream->getBuffer(), inStream->getPosition());
      stream.setBuffer(buffer, inStream->getPosition());
      stream.setPosition(inStream->getPosition());
   }
   void process(SimObject *destObject)
   {
      ((NetConnection *) destObject)->sendPacket(&stream);
   }
};

void NetConnection::checkPacketSend()
{
   U32 curTime = Platform::getVirtualMilliseconds();
   U32 delay = isServerConnection() ? gPacketUpdateDelayToServer : mCurRate.updateDelay;

   if(curTime < mLastUpdateTime + delay )
      return;
   
   if(windowFull())
      return;

   BitStream *stream = BitStream::getPacketStream(mCurRate.packetSize);
   buildSendPacketHeader(stream);
   
   mLastUpdateTime = curTime;

   PacketNotify *note = allocNotify();
   if(!mNotifyQueueHead)
      mNotifyQueueHead = note;
   else
      mNotifyQueueTail->nextPacket = note;
   mNotifyQueueTail = note;
   note->nextPacket = NULL;
   note->sendTime = curTime;
   
   note->rateChanged = mCurRate.changed;
   note->maxRateChanged = mMaxRate.changed;
   
   if(stream->writeFlag(mCurRate.changed))
   {
      stream->writeInt(mCurRate.updateDelay, 10);
      stream->writeInt(mCurRate.packetSize, 10);
      mCurRate.changed = false;
   }
   if(stream->writeFlag(mMaxRate.changed))
   {
      stream->writeInt(mMaxRate.updateDelay, 10);
      stream->writeInt(mMaxRate.packetSize, 10);
      mMaxRate.changed = false;
   }
   U32 start = stream->getCurPos();
   DEBUG_LOG(("PKLOG %d START", getId()) );
   writePacket(stream, note);
   DEBUG_LOG(("PKLOG %d END - %d", getId(), stream->getCurPos() - start) );
   if(mSimulatedPacketLoss && Platform::getRandom() < mSimulatedPacketLoss)
   {
      //Con::printf("NET  %d: SENDDROP - %d", getId(), mLastSendSeq);
      return;
   }
   if(mSimulatedPing)
   {
      Sim::postEvent(getId(), new NetDelayEvent(stream), Sim::getCurrentTime() + mSimulatedPing);
      return;
   }
   sendPacket(stream);
}

Net::Error NetConnection::sendPacket(BitStream *stream)
{
   //Con::printf("NET  %d: SEND - %d", getId(), mLastSendSeq);
   // do nothing on send if this is a demo replay.
   if(mDemoReadStream)
      return Net::NoError;

   if(mConnectionObjectId)
   {
      // short circuit connection to the other side.
      // handle the packet, then force a notify.
      stream->setBuffer(stream->getBuffer(), stream->getPosition(), stream->getPosition());
      NetConnection *remoteConnection = (NetConnection *) Sim::findObject(mConnectionObjectId);
      AssertFatal(remoteConnection, "Invalid connection objectId");
      
      remoteConnection->processRawPacket(stream);
      return Net::NoError;
   }
   else
   {
      return Net::sendto(getNetAddress(), stream->getBuffer(), stream->getPosition());
   }
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// these are the virtual function defs for Connection -
// if your subclass has additional data to read / write / notify, add it in these functions.

void NetConnection::readPacket(BitStream *bstream)
{
   eventReadPacket(bstream);
   ghostReadPacket(bstream);
}

void NetConnection::writePacket(BitStream *bstream, PacketNotify *note)
{
   eventWritePacket(bstream, note);
   ghostWritePacket(bstream, note);
}

void NetConnection::packetReceived(PacketNotify *note)
{
   eventPacketReceived(note);
   ghostPacketReceived(note);
}

void NetConnection::packetDropped(PacketNotify *note)
{
   eventPacketDropped(note);
   ghostPacketDropped(note);
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

void NetConnection::writeDemoStartBlock(ResizeBitStream* stream)
{
   ConnectionProtocol::writeDemoStartBlock(stream);

   stream->write(mRoundTripTime);
   stream->write(mPacketLoss);
   
   // Write all the current paths to the stream...
   gClientPathManager->dumpState(stream);
   stream->validate();
   U32 start = 0;
   for(U32 i = 0; i < NetStringTable::MaxStrings;)
   {
      for(;mStringXLTable[i] && i < NetStringTable::MaxStrings;i++)
         ;
      stream->write(i - start);
      for(U32 j = start; j < i; j++)
         stream->writeString(gNetStringTable->lookupString(mStringXLTable[j]));
      start = i;
      for(;!mStringXLTable[i] && i < NetStringTable::MaxStrings;i++)
         ;
      stream->write(i - start);
      stream->validate();
      start = i;
   }
   start = 0;
   PacketNotify *note = mNotifyQueueHead;
   while(note)
   {
      start++;
      note = note->nextPacket;
   }
   stream->write(start);

   eventWriteStartBlock(stream);
   ghostWriteStartBlock(stream);
}

void NetConnection::readDemoStartBlock(BitStream* stream)
{
   ConnectionProtocol::readDemoStartBlock(stream);
   
   stream->read(&mRoundTripTime);
   stream->read(&mPacketLoss);

   // Read
   gClientPathManager->readState(stream);
   U32 pos = 0;
   while(pos < NetStringTable::MaxStrings)
   {
      U32 count;
      stream->read(&count);
      for(;count;count--)
      {
         char buf[256];
         stream->readString(buf);
         U32 localId = gNetStringTable->addString(buf);
         mapString(pos++, localId);
      }
      stream->read(&count);
      pos += count;
   }
   stream->read(&pos); // notify count
   for(U32 i = 0; i < pos; i++)
   {
      PacketNotify *note = allocNotify();
      note->nextPacket = NULL;
      if(!mNotifyQueueHead)
         mNotifyQueueHead = note;
      else
         mNotifyQueueTail->nextPacket = note;
      mNotifyQueueTail = note;
   }
   eventReadStartBlock(stream);
   ghostReadStartBlock(stream);
}

bool NetConnection::startDemoRecord(const char *fileName)
{
   FileStream *fs = new FileStream;

   if(!ResourceManager->openFileForWrite(*fs, ResourceManager->getBasePath(), fileName))
   {
      delete fs;
      return false;
   }
   
   mDemoWriteStream = fs;
   mDemoWriteStream->write(mProtocolVersion);
   ResizeBitStream bs;
   // first write out all the strings we have from the server:
   for(U32 i = 0; i < NetStringTable::MaxStrings; i++)
   {
      U32 strId = mStringXLTable[i];
      if(strId)
      {
         bs.writeFlag(true);
         bs.writeInt(i, NetStringTable::StringIdBitSize);
         bs.writeString(gNetStringTable->lookupString(strId));
         bs.validate();
      }
   }
   bs.writeFlag(false);

   // then write out the start block
   writeDemoStartBlock(&bs);
   U32 size = bs.getPosition() + 1;
   mDemoWriteStream->write(size);
   mDemoWriteStream->write(size, bs.getBuffer());
   mDemoWriteStartTime = Sim::getCurrentTime() + 
      getDemoTickSize() - (Sim::getCurrentTime() % getDemoTickSize());
   mDemoLastWriteTime = mDemoWriteStartTime;
   return true;
}

class DemoProcessEvent : public SimEvent
{
public:
   virtual void process(SimObject *obj)
      {
         ((NetConnection *) obj)->readNextDemoBlock();
      }
};

class DemoStartReadEvent : public SimEvent
{
public:
   virtual void process(SimObject *obj)
      {
         ((NetConnection *) obj)->startDemoRead();
      }
};

bool NetConnection::replayDemoRecord(const char *fileName)
{
   Stream *fs = ResourceManager->openStream(fileName);
   if(!fs)
      return false;
      
   mDemoReadStream = fs;
   mDemoReadStream->read(&mProtocolVersion);
   U32 size;
   mDemoReadStream->read(&size);
   U8 *block = new U8[size];
   mDemoReadStream->read(size, block);
   BitStream bs(block, size);

   while(bs.readFlag())
   {
      U32 id = bs.readInt(NetStringTable::StringIdBitSize);
      char buf[256];
      bs.readString(buf);
      U32 localId;
      localId = gNetStringTable->addString(buf);
      mapString(id, localId);
   }

   readDemoStartBlock(&bs);
   delete[] block;
   if(mDemoReadStream->getStatus() != Stream::Ok)
      return false;

   Sim::postEvent(this, new DemoStartReadEvent, Sim::getTargetTime() + 1);
   return true;
}

void NetConnection::startDemoRead()
{
   mDemoRealStartTime = Platform::getRealMilliseconds();

   U32 time;
   mDemoReadStream->read(&time);
   if(mDemoReadStream->getStatus() != Stream::Ok)
   {
      demoPlaybackComplete();
      deleteObject();
      return;
   }      
   mDemoReadStartTime = Sim::getTargetTime();
   mDemoReadStartTime += getDemoTickSize() - (mDemoReadStartTime % getDemoTickSize());
   
   Sim::postEvent(this, new DemoProcessEvent, mDemoReadStartTime + time);
}

U32 NetConnection::getDemoTickSize()
{
   return 32; // default tick size
}

void NetConnection::stopRecording()
{
   if(mDemoWriteStream)
   {
      delete mDemoWriteStream;
      mDemoWriteStream = NULL;
   }
}

void NetConnection::recordBlock(U32 time, U32 type, U32 size, void *data)
{
   if(mDemoWriteStream)
   {
      if(time < mDemoWriteStartTime)
         time = mDemoWriteStartTime;
      AssertFatal(time >= mDemoLastWriteTime, "Cannot write block back in time.");
      
      mDemoWriteStream->write(time - mDemoWriteStartTime);
      mDemoWriteStream->write(type);
      mDemoWriteStream->write(size);
      if(size)
         mDemoWriteStream->write(size, data);
      mDemoLastWriteTime = time;
   }
}

void NetConnection::handleRecordedBlock(U32 type, U32 size, void *data)
{
   switch(type)
   {
      case BlockTypePacket: {
         BitStream bs(data, size);
         processRawPacket(&bs);
         break;
      }
   }
}

void NetConnection::demoPlaybackComplete()
{
   mDemoRealStartTime = Platform::getRealMilliseconds() - mDemoRealStartTime;
   Con::setIntVariable("$Demo::playbackTime", mDemoRealStartTime);
}

void NetConnection::readNextDemoBlock()
{
   U8 buffer[MaxPacketDataSize];
   U32 type;
   U32 size;
   mDemoReadStream->read(&type);
   mDemoReadStream->read(&size);
   mDemoReadStream->read(size, buffer);
   handleRecordedBlock(type, size, buffer);
   U32 time;
   mDemoReadStream->read(&time);
   if(mDemoReadStream->getStatus() != Stream::Ok)
   {
      demoPlaybackComplete();
      deleteObject();
   }
   else   
      Sim::postEvent(this, new DemoProcessEvent, time + mDemoReadStartTime);
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// some handy string functions for compressing strings over a connection:
enum
{
   NullString = 0,
   CString,
   TagString,
   Integer
};

void NetConnection::validateSendString(const char *str)
{
   if(U8(*str) == StringTagPrefixByte)
      checkString(dAtoi(str + 1));
}

void NetConnection::packString(BitStream *stream, const char *str)
{
   char buf[16];
   if(!*str)
   {
      stream->writeInt(NullString, 2);
      return;
   }
   if(U8(str[0]) == StringTagPrefixByte)
   {
      stream->writeInt(TagString, 2);
      stream->writeInt(dAtoi(str + 1), NetStringTable::StringIdBitSize);
      return;
   }
   if(str[0] == '-' || (str[0] >= '0' && str[0] <= '9'))
   {
      S32 num = dAtoi(str);
      dSprintf(buf, sizeof(buf), "%d", num);
      if(!dStrcmp(buf, str))
      {
         stream->writeInt(Integer, 2);
         if(stream->writeFlag(num < 0))
            num = -num;
         if(stream->writeFlag(num < 128))
         {
            stream->writeInt(num, 7);
            return;
         }
         if(stream->writeFlag(num < 32768))
         {
            stream->writeInt(num, 15);
            return;
         }
         else
         {
            stream->writeInt(num, 31);
            return;
         }
      }
   }
   stream->writeInt(CString, 2);
   stream->writeString(str);
}

void NetConnection::unpackString(BitStream *stream, char readBuffer[1024])
{
   U32 code = stream->readInt(2);
   switch(code)
   {
      case NullString:
         readBuffer[0] = 0;
         return;
      case CString:
         stream->readString(readBuffer);
         if(gBadWordFilter->isEnabled() && isServerConnection())
            gBadWordFilter->filterString(readBuffer);
         return;
      case TagString:
         U32 tag;
         tag = stream->readInt(NetStringTable::StringIdBitSize);
         readBuffer[0] = StringTagPrefixByte;
         dSprintf(readBuffer+1, 1023, "%d", tag);
         return;
      case Integer:
         bool neg;
         neg = stream->readFlag();
         U32 num;
         if(stream->readFlag())
            num = stream->readInt(7);
         else if(stream->readFlag())
            num = stream->readInt(15);
         else
            num = stream->readInt(31);
         if(neg)
            num = -num;
         dSprintf(readBuffer, 1024, "%d", num);
   }
}


//----------------------------------------------------------------------------

void NetConnection::clearCompression()
{
   mCompressRelative = false;
}

void NetConnection::setCompressionPoint(const Point3F& p)
{
   mCompressRelative = true;
   mCompressPoint = p;
}

static U32 gBitCounts[4] = {
   16, 18, 20, 32
};

void NetConnection::writeCompressed(BitStream* stream,const Point3F& p,F32 scale)
{
   // Same # of bits for all axis
   VectorF vec;
   F32 invScale = 1 / scale;
   U32 type;
   if(mCompressRelative)
   {
      vec = p - mCompressPoint;
      F32 dist = vec.len() * invScale;
      if(dist < (1 << 15))
         type = 0;
      else if(dist < (1 << 17))
         type = 1;
      else if(dist < (1 << 19))
         type = 2;
      else
         type = 3;
   }
   else
      type = 3;

   stream->writeInt(type, 2);

   if (type != 3)
   {
      type = gBitCounts[type];
      stream->writeSignedInt(S32(vec.x * invScale),type);
      stream->writeSignedInt(S32(vec.y * invScale),type);
      stream->writeSignedInt(S32(vec.z * invScale),type);
   }
   else
   {
      stream->write(p.x);
      stream->write(p.y);
      stream->write(p.z);
   }
}

void NetConnection::readCompressed(BitStream* stream,Point3F* p,F32 scale)
{
   // Same # of bits for all axis
   U32 type = stream->readInt(2);

   if(type == 3)
   {
      stream->read(&p->x);
      stream->read(&p->y);
      stream->read(&p->z);
   }
   else
   {
      type = gBitCounts[type];
      p->x = stream->readSignedInt(type);
      p->y = stream->readSignedInt(type);
      p->z = stream->readSignedInt(type);

      p->x = mCompressPoint.x + p->x * scale;
      p->y = mCompressPoint.y + p->y * scale;
      p->z = mCompressPoint.z + p->z * scale;
   }
}
