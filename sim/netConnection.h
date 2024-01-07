//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _NETCONNECTION_H_
#define _NETCONNECTION_H_

#ifndef _MPOINT_H_
#include "math/mPoint.h"
#endif
#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif
#ifndef _NETSTRINGTABLE_H_
#include "sim/netStringTable.h"
#endif
#ifndef _EVENT_H_
#include "platform/event.h"
#endif
#ifndef _DNET_H_
#include "core/dnet.h"
#endif

//----------------------------------------------------------------------------
// the sim connection encapsulates the packet stream,
// ghost manager, event manager and playerPSC of the old tribes net stuff

class NetConnection;
class NetObject;
class BitStream;
class ResizeBitStream;
class Stream;
class Point3F;

struct GhostInfo;
struct SubPacketRef; // defined in NetConnection subclass

//#define DEBUG_NET

#ifdef DEBUG_NET
#define DEBUG_LOG(x) if(mLogging){Con::printf x;}
#else
#define DEBUG_LOG(x)
#endif

//----------------------------------------------------------------------------

class NetEvent : public ConsoleObject
{
   // event manager variables
public:
   typedef ConsoleObject Parent;
   S32 mSeqCount;
   NetEvent *mNextEvent;
   bool mIsPosted;
   enum {
      GuaranteedOrdered = 0,
      Guaranteed = 1,
      Unguaranteed = 2
   } mGuaranteeType;
   
   NetEvent() { mGuaranteeType = GuaranteedOrdered; mIsPosted = false; }
   virtual ~NetEvent();
   
   NetConnectionId mSourceId;
#ifdef DEBUG_NET
   virtual const char *getDebugName();
#endif
   virtual void write(NetConnection *ps, BitStream *bstream) = 0;
   virtual void pack(NetConnection *ps, BitStream *bstream) = 0;
   virtual void unpack(NetConnection *ps, BitStream *bstream) = 0;
   virtual void process(NetConnection *ps) = 0;
   virtual void notifySent(NetConnection *ps);
   virtual void notifyDelivered(NetConnection *ps, bool madeit);
};


//----------------------------------------------------------------------------

class NetConnection : public ConnectionProtocol, public SimGroup
{
   typedef SimGroup Parent;
   
public:
   struct GhostRef
   {
      U32 mask;
      U32 ghostInfoFlags;
      GhostInfo *ghost;
      GhostRef *nextRef;
      GhostRef *nextUpdateChain;
   };
   enum {
      HashTableSize = 127,
   };
public:
   struct PacketNotify
   {
      // packet stream notify stuff:
      bool rateChanged;
      bool maxRateChanged;
      U32  sendTime;

      NetEvent *eventList;
      GhostRef *ghostList;
      SubPacketRef *subList; // defined by subclass - used as desired
      
      PacketNotify *nextPacket;
      PacketNotify();
   };
   virtual PacketNotify *allocNotify();
//----------------------------------------------------------------
// Connection functions
//----------------------------------------------------------------

private:
   U32 mLastUpdateTime;
   F32 mRoundTripTime;
   F32 mPacketLoss;
   U32 mProtocolVersion;

   U32 mSimulatedPing;
   F32 mSimulatedPacketLoss;

   U32 mClientConnectSequence;
   U32 mServerConnectSequence;

   struct NetRate
   {
      U32 updateDelay;
      S32 packetSize;
      bool changed;
   };

   NetRate mCurRate;
   NetRate mMaxRate;

   PacketNotify *mNotifyQueueHead;
   PacketNotify *mNotifyQueueTail;
   
   SimObjectId mConnectionObjectId;

   bool mMissionPathsSent;
   
   NetAddress mNetAddress;

   // timeout management stuff:
   U32 mPingSendCount;
   U32 mPingRetryCount;
   U32 mLastPingSendTime;

   NetConnection *mNextTableHash;
   static NetConnection *mHashTable[HashTableSize];
   bool mIsNetworkConnection;
protected:
   static NetConnection *mServerConnection;
   static NetConnection *mLocalClientConnection;
   static char mErrorBuffer[256];

public:
   static char *getErrorBuffer() { return mErrorBuffer; }
#ifdef DEBUG_NET
   bool mLogging;
   void setLogging(bool logging) { mLogging = logging; }
#endif
   void setSimulatedNetParams(F32 packetLoss, U32 ping) 
      { mSimulatedPacketLoss = packetLoss; mSimulatedPing = ping; }
   bool isServerConnection()  { return this == mServerConnection; }
   bool isLocalConnection() { return mConnectionObjectId != 0; }
   bool isNetworkConnection() { return mIsNetworkConnection; }
   void setNetworkConnection(bool net) { mIsNetworkConnection = net; }
   // call this if the "connection" is local to this app
   // short-circuits protocol layer
   
   void setRemoteConnectionObjectId(SimObjectId objectId);
   SimObjectId getRemoteConnectionObjectId() { return mConnectionObjectId; }
   void setSequences(U32 ccs, U32 css);
   void getSequences(U32 *ccs, U32 *css);

   void setProtocolVersion(U32 protocol)
      { mProtocolVersion = protocol; }
   U32 getProtocolVersion()
      { return mProtocolVersion; }
   F32 getRoundTripTime()
      { return mRoundTripTime; }
   F32 getPacketLoss()
      { return( mPacketLoss ); }

   static void setLastError(const char *fmt,...);
   
   static void setServerConnection(NetConnection *conn);
   static void setLocalClientConnection(NetConnection *conn);

   static NetConnection *getServerConnection() { return mServerConnection; }
   static NetConnection *getLocalClientConnection() { return mLocalClientConnection; }

   void checkMaxRate();

   void handlePacket(BitStream *stream);
   void processRawPacket(BitStream *stream);
   void handleNotify(bool recvd);
   void handleConnectionEstablished();
   void keepAlive();
   
   const NetAddress *getNetAddress();
   void setNetAddress(const NetAddress *address);
   Net::Error sendPacket(BitStream *stream);   
private:
   void netAddressTableInsert();
   void netAddressTableRemove();
public:
   static NetConnection *lookup(const NetAddress *remoteAddress);   

   bool checkTimeout(U32 time); // returns true if the connection timed out
   
   void checkPacketSend();

   bool missionPathsSent() const          { return mMissionPathsSent; }
   void setMissionPathsSent(const bool s) { mMissionPathsSent = s; }
      
   static void consoleInit();

   bool onAdd();
   void onRemove();

   NetConnection(bool ghostFrom = false, bool ghostTo = false, bool sendEvents = false);
   ~NetConnection();

   DECLARE_CONOBJECT(NetConnection);
protected:
   virtual void readPacket(BitStream *bstream);
   virtual void writePacket(BitStream *bstream, PacketNotify *note);
   virtual void packetReceived(PacketNotify *note);
   virtual void packetDropped(PacketNotify *note);
   virtual void connectionError(const char *errorString);
   
//----------------------------------------------------------------
// event manager functions/code:
//----------------------------------------------------------------

private:
   NetEvent *mSendEventQueueHead;
   NetEvent *mSendEventQueueTail;
   NetEvent *mUnorderedSendEventQueueHead;
   NetEvent *mUnorderedSendEventQueueTail;
   NetEvent *mWaitSeqEvents;
   NetEvent *mNotifyEventList;
   bool mSendingEvents;
   
   S32 mNextSendEventSeq;
   S32 mNextRecvEventSeq;
   S32 mLastAckedEventSeq;

   enum {
      InvalidSendEventSeq = -1,
      FirstValidSendEventSeq = 0
   };
   
   void eventPacketDropped(PacketNotify *notify);
   void eventPacketReceived(PacketNotify *notify);

   void eventWritePacket(BitStream *bstream, PacketNotify *notify);
   void eventReadPacket(BitStream *bstream);

   void eventWriteStartBlock(ResizeBitStream *stream);
   void eventReadStartBlock(BitStream *stream);
public:
   bool postNetEvent(NetEvent *event);

//----------------------------------------------------------------
// String table functions
//----------------------------------------------------------------

private:

   U16 mStringXLTable[NetStringTable::MaxStrings];
   U8 mStringSentBitArray[NetStringTable::MaxStrings >> 3];
   NetConnection *mNextConnection;
   NetConnection *mPrevConnection;
   static NetConnection *mConnectionList;
public:   
   static NetConnection *getConnectionList() { return mConnectionList; }
   NetConnection *getNext() { return mNextConnection; }
   
   void mapString(U32 remoteId, U32 localId);
   void clearString(U32 id);
   void checkString(U32 id);
   U32 translateRemoteStringId(U32 id) { return mStringXLTable[id]; }
   void validateSendString(const char *str);
   void packString(BitStream *stream, const char *str);
   void unpackString(BitStream *stream, char readBuffer[1024]);

//----------------------------------------------------------------
// ghost manager functions/code:
//----------------------------------------------------------------

protected:
   enum
   {
      GhostAlwaysDone,
      ReadyForNormalGhosts,
      EndGhosting,
      GhostAlwaysStarting,
   };
   GhostInfo **mGhostArray;     // linked list of ghostInfos ghosted by this side of the connection

   U32 mGhostZeroUpdateIndex; // index in mGhostArray of first ghost with 0 update mask
   U32 mGhostFreeIndex;    // index in mGhostArray of first free ghost

   bool mGhostFrom;        // can this side ghost objects over to the other?
   bool mGhostTo;          // can this side accept remote ghosts?
   bool mGhosting;         // am I currently ghosting objects over?
   bool mScoping;          // am I currently allowing objects to be scoped?
   U32  mGhostingSequence; // sequence number describing this ghosting session

   // mLocalGhosts pointer is NULL if mGhostTo is false
   NetObject **mLocalGhosts;  // local ghost for remote object

   // these pointers are NULL if mGhostFrom is false
   GhostInfo *mGhostRefs;     // allocated array of ghostInfos
   GhostInfo **mGhostLookupTable; // table indexed by object id->GhostInfo
   
   SimObjectPtr<NetObject> mScopeObject;
   
   void clearGhostInfo();
   bool validateGhostArray();

   void ghostPacketDropped(PacketNotify *notify);
   void ghostPacketReceived(PacketNotify *notify);
   
   void ghostWritePacket(BitStream *bstream, PacketNotify *notify);
   void ghostReadPacket(BitStream *bstream);
   void freeGhostInfo(GhostInfo *);
   
   void ghostWriteStartBlock(ResizeBitStream *stream);
   void ghostReadStartBlock(BitStream *stream);

public:
   enum { MaxGhostCount = 1024, GhostIdBitSize = 10, GhostLookupTableSize = 1024 };
   
   virtual void doneScopingScene() {}
   void setScopeObject(NetObject *object);
   NetObject *getScopeObject();
   
   void objectInScope(NetObject *object);
   void objectLocalScopeAlways(NetObject *object);
   void objectLocalClearAlways(NetObject *object);

   NetObject *resolveGhost(S32 id);
   NetObject *resolveGhostParent(S32 id);
   void ghostPushNonZero(GhostInfo *gi);
   void ghostPushToZero(GhostInfo *gi);
   void ghostPushZeroToFree(GhostInfo *gi);
   inline void ghostPushFreeToZero(GhostInfo *info);

   S32 getGhostIndex(NetObject *object);

   void resetGhosting();
   void activateGhosting();
   bool isGhosting() { return mGhosting; }
   
   void detachObject(GhostInfo *info);
   virtual void handleGhostMessage(S32 message, U32 sequence, U32 ghostCount);
   void setGhostAlwaysObject(NetObject *object, U32 index);

//----------------------------------------------------------------
// Demo recording functions
//----------------------------------------------------------------

private:
   Stream *mDemoWriteStream;
   Stream *mDemoReadStream;
   U32 mDemoWriteStartTime;
   U32 mDemoReadStartTime;
   U32 mDemoLastWriteTime;

   U32 mDemoRealStartTime;

public:
   enum {
      BlockTypePacket,
      NetConnectionBlockTypeCount
   };
   bool isRecording()
      { return mDemoWriteStream != NULL; }
   bool isPlayingBack()
      { return mDemoReadStream != NULL; }
   
   void recordBlock(U32 time, U32 type, U32 size, void *data);
   virtual void handleRecordedBlock(U32 type, U32 size, void *data);
   void readNextDemoBlock();
   
   bool startDemoRecord(const char *fileName);
   bool replayDemoRecord(const char *fileName);
   void startDemoRead();   
   void stopRecording();
   
   virtual void writeDemoStartBlock(ResizeBitStream *stream);
   virtual void readDemoStartBlock(BitStream *stream);
   virtual U32 getDemoTickSize();
   virtual void demoPlaybackComplete();


//----------------------------------------------------------------
// Connection relative compression
//----------------------------------------------------------------

private:
   bool mCompressRelative;
   Point3F mCompressPoint;

public:
   void clearCompression();
   void setCompressionPoint(const Point3F& p);

   // Matching calls to these compression methods must, of course,
   // have matching scale values.
   virtual void writeCompressed(BitStream* stream,const Point3F& p,F32 scale = 0.01f);
   virtual void readCompressed(BitStream* stream,Point3F* p,F32 scale = 0.01f);
};


//----------------------------------------------------------------------------

struct GhostInfo
{
   public:  // required for MSVC

   // NOTE:
   // if the size of this structure changes, the
   // NetConnection::getGhostIndex function MUST be changed 
   // to reflect.
   
   NetObject *obj;            // the real object
   U32 updateMask;         // 32 bits of object info
   NetConnection::GhostRef *updateChain;     // chain of updates for this object in packets
   GhostInfo *nextObjectRef;  // next ghost ref for this object (another connection)

   GhostInfo *prevObjectRef;  // prev ghost ref for this object
   NetConnection *connection;
   GhostInfo *nextLookupInfo;
   U32 updateSkipCount;
   
   U32 flags;
   F32 priority;
   U32 index;
   U32 arrayIndex;

	enum Flags
	{
		Valid = BIT(0),
		InScope = BIT(1),
      ScopeAlways = BIT(2),
      NotYetGhosted = BIT(3),
      Ghosting = BIT(4),
      KillGhost = BIT(5),
      KillingGhost = BIT(6),
      ScopedEvent = BIT(7),
      ScopeLocalAlways = BIT(8),
	};
};

inline void NetConnection::ghostPushNonZero(GhostInfo *info)
{
   AssertFatal(info->arrayIndex >= mGhostZeroUpdateIndex && info->arrayIndex < mGhostFreeIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   if(info->arrayIndex != mGhostZeroUpdateIndex)
   {
      mGhostArray[mGhostZeroUpdateIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostZeroUpdateIndex];
      mGhostArray[mGhostZeroUpdateIndex] = info;
      info->arrayIndex = mGhostZeroUpdateIndex;
   }
   mGhostZeroUpdateIndex++;
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

inline void NetConnection::ghostPushToZero(GhostInfo *info)
{
   AssertFatal(info->arrayIndex < mGhostZeroUpdateIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   mGhostZeroUpdateIndex--;
   if(info->arrayIndex != mGhostZeroUpdateIndex)
   {
      mGhostArray[mGhostZeroUpdateIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostZeroUpdateIndex];
      mGhostArray[mGhostZeroUpdateIndex] = info;
      info->arrayIndex = mGhostZeroUpdateIndex;
   }
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

inline void NetConnection::ghostPushZeroToFree(GhostInfo *info)
{
   AssertFatal(info->arrayIndex >= mGhostZeroUpdateIndex && info->arrayIndex < mGhostFreeIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   mGhostFreeIndex--;
   if(info->arrayIndex != mGhostFreeIndex)
   {
      mGhostArray[mGhostFreeIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostFreeIndex];
      mGhostArray[mGhostFreeIndex] = info;
      info->arrayIndex = mGhostFreeIndex;
   }
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

inline void NetConnection::ghostPushFreeToZero(GhostInfo *info)
{
   AssertFatal(info->arrayIndex >= mGhostFreeIndex, "Out of range arrayIndex.");
   AssertFatal(mGhostArray[info->arrayIndex] == info, "Invalid array object.");
   if(info->arrayIndex != mGhostFreeIndex)
   {
      mGhostArray[mGhostFreeIndex]->arrayIndex = info->arrayIndex;
      mGhostArray[info->arrayIndex] = mGhostArray[mGhostFreeIndex];
      mGhostArray[mGhostFreeIndex] = info;
      info->arrayIndex = mGhostFreeIndex;
   }
   mGhostFreeIndex++;
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

#endif
