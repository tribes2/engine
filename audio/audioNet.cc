//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platformAudio.h"
#include "audio/audioNet.h"
#include "core/bitStream.h"
#include "game/shapeBase.h"
#include "game/gameConnection.h"
#include "audio/audioCodec.h"

#define SIZE_BITS       5
#define SEQUENCE_BITS   6

// GSM:
//#define SIZE_BITS       6
//#define SEQUENCE_BITS   7

//--------------------------------------------------------------------------

SimVoiceStreamEvent::SimVoiceStreamEvent(U8 streamId, U32 seq, U8 codecId)
{
   mGuaranteeType = Unguaranteed;
   mSize = VOICE_PACKET_DATA_SIZE;
   mData = new U8[VOICE_PACKET_DATA_SIZE+1];

   mCodecId = codecId;
   mStreamId = streamId & STREAM_MASK;
   mSequence = seq;
   mClientId = 0;
   mObjectId = 0;

   // NOTE: the first byte in the data is used as a lock count
   // this will allow us to pass the data from object to object
   // without making multiple copies of it.
   if(mData)
      (*mData) = 1;
}   

SimVoiceStreamEvent::SimVoiceStreamEvent(const SimVoiceStreamEvent *event)
{
   mGuaranteeType = Unguaranteed;
   mData = event->mData;
   mSize = event->mSize;
   mCodecId = event->mCodecId;
   mClientId = event->mClientId;
   mStreamId = event->mStreamId;
   mSequence = event->mSequence;
   mObjectId = event->mObjectId;

   // NOTE: the first byte in the data is used as a lock count
   // this will allow us to pass the data from object to object
   // without making multiple copies of it.
   if (mData)
      (*mData)++;       // increment lock count
}   



//--------------------------------------
SimVoiceStreamEvent::~SimVoiceStreamEvent()
{
   if (mData)
   {
      (*mData)--;       // decrement lock ount
      if (*mData == 0)  // if zero we are responsible for deleting the data
         delete [] mData;
   }
}   


//--------------------------------------
void SimVoiceStreamEvent::pack(NetConnection *con, BitStream *bstream)
{
   AssertFatal((1<<SIZE_BITS) >= VOICE_PACKET_DATA_SIZE, "SimVoiceStreamEvent::pack: insuffecient bits to encode size.");
   AssertFatal((1<<SEQUENCE_BITS) >= mSequence, "SimVoiceStreamEvent::pack: insuffecient bits to encode sequence.");
   AssertFatal((1<<2 >= AUDIO_NUM_CODECS), "SimVoiceStreamEvent::pack: blah");

   bstream->writeInt(mStreamId, 5);
   bstream->writeInt(mSequence, SEQUENCE_BITS);
   bstream->writeInt(mCodecId, 2);

   if(con->isServerConnection())
   {
      // client side
   }
   else
   {
      // server side
      bstream->write(mClientId);
      if(mSequence == 0)
         bstream->writeInt(mObjectId, NetConnection::GhostIdBitSize);
   }
         
   // only the EOS packets are not VOICE_PACKET_DATA_SIZE bytes long.
   if(bstream->writeFlag(mSize != VOICE_PACKET_DATA_SIZE))
      bstream->writeInt(mSize, SIZE_BITS);

   bstream->_write(mSize, mData+1);
}   

//--------------------------------------
void SimVoiceStreamEvent::write(NetConnection *con, BitStream *bstream)
{
   pack(con, bstream);   
}   

//--------------------------------------
void SimVoiceStreamEvent::unpack(NetConnection *con, BitStream *bstream)
{
   mSize = VOICE_PACKET_DATA_SIZE;

   mStreamId = bstream->readInt(5);
   mSequence = bstream->readInt(SEQUENCE_BITS);
   mCodecId = bstream->readInt(2);

   if(con->isServerConnection())
   {  
      // client side 
      bstream->read(&mClientId);
      if (mSequence == 0)
         mObjectId = bstream->readInt(NetConnection::GhostIdBitSize);
   }
   else
   {  
      // server side
      mClientId = con->getId();
      if(mSequence == 0)
      {
         ShapeBase *base = ((GameConnection*)con)->getControlObject();
         mObjectId = base ? con->getGhostIndex(base) : 0;
      }
   }

   // not a full packet?
   if(bstream->readFlag())
      mSize = getMin(bstream->readInt(SIZE_BITS), VOICE_PACKET_DATA_SIZE);

   // read data (skip lock byte)
   bstream->_read(mSize, mData+1);
}   

//--------------------------------------
void SimVoiceStreamEvent::process(NetConnection *con)
{
   if (con->isServerConnection())
      processClient(con);
   else
      processServer(con);
}   


//--------------------------------------
void SimVoiceStreamEvent::processClient(NetConnection *con)
{
   con;
   alxReceiveVoiceStream(this);
}   

//--------------------------------------
void SimVoiceStreamEvent::processServer(NetConnection *con)
{
   if(con->getProtocolVersion() < MIN_PROTOCOL_VERSION)
      return;

   GameConnection *itr = static_cast<GameConnection*>(con->getConnectionList());
   GameConnection *gc = dynamic_cast<GameConnection*>(con);
   if(!gc)
      return;

   if(U32(gc->getVoiceEncodingLevel()) != mCodecId)
      return;

   while (itr != NULL)
   {
      if(!itr->isServerConnection() && (itr->getProtocolVersion() >= MIN_PROTOCOL_VERSION))
      {
         if(itr->canListen(gc) || itr->isListening(gc->getVoiceID()))
         {
            if (mSequence == 0)
               itr->willListen(gc->getVoiceID());

            if ( itr->isListening(gc->getVoiceID()) )
               itr->postNetEvent( new SimVoiceStreamEvent(this) );

            if (mSize < VOICE_PACKET_DATA_SIZE)
               itr->stopListening(gc->getVoiceID());
         }
      }
      itr = static_cast<GameConnection*>(itr->getNext());
   }
}   

IMPLEMENT_CO_NETEVENT_V1(SimVoiceStreamEvent);
