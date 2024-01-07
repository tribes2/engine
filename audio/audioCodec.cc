//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "audio/audioCodec.h"
//#include "audio/audioCodecGSM.h"

#ifndef __linux
#include "audio/audioCodecMiles.h"
#endif

//-------------------------------------------------------------------------
// Class AudioCodecManager:
//-------------------------------------------------------------------------
AudioCodecManager::CodecInfo AudioCodecManager::smCodecTable[] = 
{
   #ifndef __linux   // miles only exists on Win32!
      {  AUDIO_CODEC_V12,  MilesEncoderCodec::create, MilesDecoderCodec::create, 0, 0},
      {  AUDIO_CODEC_V24,  MilesEncoderCodec::create, MilesDecoderCodec::create, 0, 0},
      {  AUDIO_CODEC_V29,  MilesEncoderCodec::create, MilesDecoderCodec::create, 0, 0},
   #else
      {  AUDIO_CODEC_V12,  0, 0, 0, 0 },
      {  AUDIO_CODEC_V24,  0, 0, 0, 0 },
      {  AUDIO_CODEC_V29,  0, 0, 0, 0 },
   #endif
//      {  AUDIO_CODEC_GSM,  GSMEncoderCodec::create, GSMDecoderCodec::create, 0, 0 },
};


//-------------------------------------------------------------------------
void AudioCodecManager::destroy()
{
   for(U32 i = 0; i < sizeof(smCodecTable) / sizeof(smCodecTable[0]); i++)
   {
      delete smCodecTable[i].mEncoder;
      delete smCodecTable[i].mDecoder;
      smCodecTable[i].mEncoder = 0;
      smCodecTable[i].mDecoder = 0;
   }
}

VoiceEncoderCodec * AudioCodecManager::createEncoderCodec(S32 codecId)
{
   for(U32 i = 0; i < sizeof(smCodecTable) / sizeof(smCodecTable[0]); i++)
      if((smCodecTable[i].mId == codecId) && smCodecTable[i].mCreateEncoder)
      {
         // already created?
         if(smCodecTable[i].mEncoder)
            return(smCodecTable[i].mEncoder);

         VoiceCodec * codec = smCodecTable[i].mCreateEncoder(codecId);
         if(!dynamic_cast<VoiceEncoderCodec*>(codec))
         {
            delete codec;
            return(0);
         }
         else
         {
            smCodecTable[i].mEncoder = static_cast<VoiceEncoderCodec*>(codec);
            return(smCodecTable[i].mEncoder);
         }
      }
   return(0);
}

VoiceDecoderCodec * AudioCodecManager::createDecoderCodec(S32 codecId)
{
   for(U32 i = 0; i < sizeof(smCodecTable) / sizeof(smCodecTable[0]); i++)
      if((smCodecTable[i].mId == codecId) && smCodecTable[i].mCreateDecoder)
      {
         // already created?
         if(smCodecTable[i].mDecoder)
            return(smCodecTable[i].mDecoder);

         VoiceCodec * codec = smCodecTable[i].mCreateDecoder(codecId);
         if(!dynamic_cast<VoiceDecoderCodec*>(codec))
         {
            delete codec;
            return(0);
         }
         else
         {
            smCodecTable[i].mDecoder = static_cast<VoiceDecoderCodec*>(codec);
            return(smCodecTable[i].mDecoder);
         }
      }
   return(0);
}

//-------------------------------------------------------------------------
// Class VoiceEncoderStream:
//-------------------------------------------------------------------------
VoiceEncoderStream::VoiceEncoderStream()
{
   mEncoder = 0;
   mEncoderId = AUDIO_CODEC_NONE;
   mConnection = 0;
   mStreamId = 0;
   mSequence = 0;
   mStream = 0;

   mQueue.setSize(VOICE_CHANNELS * VOICE_FREQUENCY * (VOICE_BITS >> 3) * VOICE_LENGTH);
}

VoiceEncoderStream::~VoiceEncoderStream()
{
   close();
}

//-------------------------------------------------------------------------
bool VoiceEncoderStream::setConnection(GameConnection * con)
{
   mStreamId++;
   mSequence = 0;
   mConnection = con;
   mQueue.clear();   
   return(true);
}   

//-------------------------------------------------------------------------
bool VoiceEncoderStream::setCodec(S32 codecId)
{
   close();

   mEncoder = 0;
   mEncoderId = AUDIO_CODEC_NONE;

   if(codecId == AUDIO_CODEC_NONE)
      return(true);

   mEncoder = AudioCodecManager::createEncoderCodec(codecId);
   if(mEncoder)
   {
      if(dynamic_cast<VoiceEncoderCodec*>(mEncoder))
      {
         mEncoderId = codecId;
         return(true);
      }
      else
      {
         delete mEncoder;
         mEncoder = 0;
      }
   }
   return(false);
}

//-------------------------------------------------------------------------
bool VoiceEncoderStream::open()
{
   close();

   if(mEncoder)
      mStream = mEncoder->openStream();
   return(bool(mStream));
}

void VoiceEncoderStream::close()
{
   if(mEncoder && mStream)
   {
      mEncoder->closeStream(mStream);
      mStream = 0;
   }
}

//-------------------------------------------------------------------------
bool VoiceEncoderStream::setBuffer(const U8 * data, U32 size)
{
   AssertFatal(data, "VoiceEncoderStream::setBuffer: invalid buffer ptr");

   if(size > mQueue.getFree())
      return(false);

   mQueue.enqueue(data, size);
   return(true);
}

//-------------------------------------------------------------------------
void VoiceEncoderStream::process(bool flush)
{
   if(!mEncoder || !mStream)
      return;

   while(flush || (mQueue.getUsed() >= 1800))
   {
      SimVoiceStreamEvent * mEvent = new SimVoiceStreamEvent(mStreamId, mSequence++, mEncoderId);
      U32 amount = mEncoder->process(mStream, &mQueue, mEvent->getData(), SimVoiceStreamEvent::VOICE_PACKET_DATA_SIZE);

      mEvent->setDataSize(amount);
      mConnection->postNetEvent(mEvent);

      if(flush && (amount < SimVoiceStreamEvent::VOICE_PACKET_DATA_SIZE))
         break;
   }
}

void VoiceEncoderStream::flush()
{
   process(true);
   mQueue.clear();
}


//-------------------------------------------------------------------------
// Class VoiceEncoder:
//-------------------------------------------------------------------------
VoiceDecoderStream::VoiceDecoderStream()
{
   mDecoder = 0;
   mDecoderId = AUDIO_CODEC_NONE;
   
   mInQueue.setSize(VOICE_CHANNELS * VOICE_FREQUENCY * (VOICE_BITS >> 3) * VOICE_LENGTH);
   mOutQueue.setSize(VOICE_CHANNELS * VOICE_FREQUENCY * (VOICE_BITS >> 3) * VOICE_LENGTH);

   mStream = 0;
}

VoiceDecoderStream::~VoiceDecoderStream()
{
   close();
}

//-------------------------------------------------------------------------
bool VoiceDecoderStream::setCodec(S32 codecId)
{
   close();

   mDecoder = 0;
   mDecoderId = AUDIO_CODEC_NONE;

   if(codecId == AUDIO_CODEC_NONE)
      return(true);

   mDecoder = AudioCodecManager::createDecoderCodec(codecId);
   if(mDecoder)
   {
      if(dynamic_cast<VoiceDecoderCodec*>(mDecoder))
      {
         mDecoderId = codecId;
         return(true);
      }
      else
      {
         delete mDecoder;
         mDecoder = 0;
      }
   }
   return(false);
}

//-------------------------------------------------------------------------
bool VoiceDecoderStream::open()
{
   if(!mDecoder)
      return(false);

   close();

   if(mDecoder)
      mStream = mDecoder->openStream();
   return(bool(mStream));
}

void VoiceDecoderStream::close()
{
   if(mDecoder && mStream)
   {
      mDecoder->closeStream(mStream);
      mStream = 0;
   }
}

//-------------------------------------------------------------------------
bool VoiceDecoderStream::setBuffer(U8 * data, U32 size)
{
   AssertFatal(data, "VoiceDecoderStream::setBuffer: invalid data ptr");

   if(size > mInQueue.getFree())
      return(false);

   mInQueue.enqueue(data, size);
   return(true);
}

U32 VoiceDecoderStream::getBuffer(U8 ** data, U32 * size)
{
   *data = mOutQueue.getHead();
   *size = mOutQueue.getContiguousUsed();

   mOutQueue.dequeue(*size);
   return(*size);
}

void VoiceDecoderStream::process(bool flush)
{
   if(!mDecoder || !mStream)
      return;

   while( flush || (mInQueue.getUsed() && !mOutQueue.isFull()) )
   {
      U32 amount = mDecoder->process(mStream, &mInQueue, mOutQueue.getTail(), mOutQueue.getContiguousFree());
      mOutQueue.enqueue(amount);

      if(flush && (amount == 0))
         break;
   }
}
