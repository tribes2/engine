//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIONET_H_
#define _AUDIONET_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _NETCONNECTION_H_
#include "sim/netConnection.h"
#endif

class GameConnection;

//--------------------------------------
struct SimVoiceStreamEvent: public NetEvent
{
private:
   void processClient(NetConnection *);
   void processServer(NetConnection *);

   enum { MIN_PROTOCOL_VERSION = 34 };
   enum { STREAM_MASK = 0x1f };     // 5 bits

public:
   U8 *mData;
   U8  mSize;
   U32 mClientId;
   U8  mStreamId;
   U8  mSequence;
   U8  mCodecId;
   SimObjectId mObjectId;

   enum { VOICE_PACKET_DATA_SIZE = 30 };

//   // GSM:
//   enum { VOICE_PACKET_DATA_SIZE = 33 };

   SimVoiceStreamEvent(U8 streamId=0, U32 seq=0, U8 codecId=0);
   SimVoiceStreamEvent(const SimVoiceStreamEvent *event);
   ~SimVoiceStreamEvent();
   void pack(NetConnection *, BitStream *bstream);
   void write(NetConnection *, BitStream *bstream);
   void unpack(NetConnection *, BitStream *bstream);
   void process(NetConnection *);
   DECLARE_CONOBJECT(SimVoiceStreamEvent);

   U8* getData() { return mData+1; }
   U32 getSize() { return mSize; }
   void setDataSize(U32 size) 
   { 
      mSize = size; 
      if (mSize < VOICE_PACKET_DATA_SIZE) // if end of stream make sure we notify 
         mGuaranteeType = Guaranteed;   
   }
};



#endif  // _H_AUDIONET_
