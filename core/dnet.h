//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _DNET_H_
#define _DNET_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

class BitStream;
class ResizeBitStream;

class ConnectionProtocol
{
protected:
   U32 mLastSeqRecvdAtSend[32];
   U32 mLastSeqRecvd;
   U32 mHighestAckedSeq;
   U32 mLastSendSeq;
   U32 mAckMask;
   U32 mConnectSequence;
   U32 mLastRecvAckAck;
   bool mConnectionEstablished;
public:
   ConnectionProtocol();

   void buildSendPacketHeader(BitStream *bstream, S32 packetType = 0);
      
   void sendPingPacket();
   void sendAckPacket();
   void setConnectionEstablished() { mConnectionEstablished = true; }
   
   bool windowFull();
   bool connectionEstablished();
   void setConnectSequence(U32 connectSeq) { mConnectSequence = connectSeq; }

   virtual void writeDemoStartBlock(ResizeBitStream *stream);
   virtual void readDemoStartBlock(BitStream *stream);

   virtual void processRawPacket(BitStream *bstream);
   virtual Net::Error sendPacket(BitStream *bstream) = 0;
   virtual void keepAlive() = 0;
   virtual void handleConnectionEstablished() = 0;
   virtual void handleNotify(bool recvd) = 0;
   virtual void handlePacket(BitStream *bstream) = 0;
};

#endif
