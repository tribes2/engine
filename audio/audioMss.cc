//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "audio/audioMss.h"

namespace {
   const char * cVoiceCodecs [] = { ".v12", ".v24", ".v29" };
   const U32 cNumVoiceCodecs = sizeof(cVoiceCodecs) / sizeof(cVoiceCodecs[0]);
};

HPROVIDER  EncoderStream::hProvider = 0;    // ASI provider used to encode data
U32 EncoderStream::mCodecLevel = 0;

ASI_STREAM_OPEN     EncoderStream::streamOpen;
ASI_STREAM_PROCESS  EncoderStream::streamProcess;
ASI_STREAM_CLOSE    EncoderStream::streamClose;

//-------------------------------------- 
// - can encode using just one method (hence the statics)
EncoderStream::EncoderStream()
{
   hStream   = NULL;
   mStreamId = 0;     
   mConnection = NULL;
}

//-------------------------------------- 
EncoderStream::~EncoderStream()
{
   closeStream();
}

//--------------------------------------
bool EncoderStream::open(U32 codecLevel)
{
   if(codecLevel >= cNumVoiceCodecs)
      return(false);

   mCodecLevel = codecLevel;

   // there can be only one
   if(hProvider)
      return true;

   RIB_INTERFACE_ENTRY ENCODER_REQUEST[] =  {
      { RIB_FUNCTION,   "ASI_stream_open",      (U32) &streamOpen,      RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_close",     (U32) &streamClose,     RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_process",   (U32) &streamProcess,   RIB_NONE }  };

   hProvider = RIB_find_file_provider("ASI codec", "Output file types", const_cast<char*>(cVoiceCodecs[codecLevel]));
   if (hProvider != NULL)
      if (RIB_request(hProvider, "ASI stream", ENCODER_REQUEST) == RIB_NOERR)
      {
         return true;
      }

   Con::printf("EncoderStream::open FAILED");
   close();
   return false;
}

//--------------------------------------
void EncoderStream::close()
{
   if (hProvider)
   {
      RIB_free_provider_handle(hProvider);
      hProvider = NULL;  
   }
}


//--------------------------------------
bool EncoderStream::openStream()
{
   if (!hProvider)
      return false;

   closeStream();
   mConnection = NULL;
   hStream = streamOpen((U32)this, callback_router, 0);
   mQueue.setSize(8000*2*5);
   
   if (hStream)
      Con::printf("EncoderStream::open %d", mStreamId);
   else
      Con::printf("EncoderStream::open FAILED");

   return (hStream != NULL);
}


//--------------------------------------
void EncoderStream::closeStream()
{
   if (hStream)
   {
      streamClose(hStream);
      hStream = NULL;   
   }

   mSequence = 0;
}


bool EncoderStream::setConnection(GameConnection *con)
{
   mStreamId++;
   mSequence = 0;
   mConnection = con;
   mQueue.clear();   
   return true;
}   


//--------------------------------------
S32 AILCALLBACK EncoderStream::callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset)
{
   return ((EncoderStream*)user)->callback(dest, bytesRequested, offset);
}

//--------------------------------------------------------------------------
S32 EncoderStream::callback(void *dest, S32 bytesRequested, S32 offset)
{
   offset; // unused
   return mQueue.dequeue((U8*)dest, bytesRequested);
}

//--------------------------------------
void EncoderStream::setBuffer(const U8 *buffer, U32 size)
{
   if (size > mQueue.getFree())   
   {
      AssertWarn(0, "EncoderStream: queue full");
      return;   
   }

   mQueue.enqueue(buffer, size);
}   


//-------------------------------------- 
void EncoderStream::process(bool flush)
{
   if (!hStream)
      return;

   while ( flush || (mQueue.getUsed() >= 1800) )
   {
      SimVoiceStreamEvent *mEvent = new SimVoiceStreamEvent(mStreamId, mSequence++, mCodecLevel);
      S32 amount = streamProcess(hStream, mEvent->getData(), SimVoiceStreamEvent::VOICE_PACKET_DATA_SIZE);
      mEvent->setDataSize(amount);

      mConnection->postNetEvent(mEvent);
            
      if (flush && amount < SimVoiceStreamEvent::VOICE_PACKET_DATA_SIZE)
         break;
   }
}


//--------------------------------------
void EncoderStream::flush()
{
   process(true);
   mQueue.clear();
}   


//--------------------------------------------------------------------------
// Class VoiceDecoder:
//--------------------------------------------------------------------------
VoiceDecoder::VoiceDecoder()
{
   hProvider = 0;
   mCodecLevel = U32(-1);
}

VoiceDecoder::~VoiceDecoder()
{
   close();
}

bool VoiceDecoder::open(U32 codecLevel)
{
   if(codecLevel >= cNumVoiceCodecs)
      return(false);

   mCodecLevel = codecLevel;
   if(hProvider)
      return true;

   RIB_INTERFACE_ENTRY DECODER_REQUEST[] =  {
      { RIB_FUNCTION,   "ASI_stream_attribute",      (U32) &streamAttribute,      RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_open",           (U32) &streamOpen,           RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_seek",           (U32) &streamSeek,           RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_close",          (U32) &streamClose,          RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_process",        (U32) &streamProcess,        RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_set_preference", (U32) &streamSetPreference,  RIB_NONE },
      { RIB_ATTRIBUTE,  "Output sample rate",        (U32) &OUTPUT_SAMPLE_RATE,   RIB_NONE },
      { RIB_ATTRIBUTE,  "Output sample width",       (U32) &OUTPUT_BITS,          RIB_NONE },
      { RIB_ATTRIBUTE,  "Output channels",           (U32) &OUTPUT_CHANNELS,      RIB_NONE },
      { RIB_PREFERENCE, "Requested sample rate",     (U32) &REQUESTED_RATE,       RIB_NONE } };

   hProvider = RIB_find_file_provider("ASI codec", "Input file types", cVoiceCodecs[codecLevel]);
   if (hProvider != NULL)
      if (RIB_request(hProvider, "ASI stream", DECODER_REQUEST) == RIB_NOERR)
         return true;

   Con::printf("VoiceDecoder::open FAILED");
   close();
   return false;
}

void VoiceDecoder::close()
{
   if (hProvider)
   {
      AssertWarn(hProvider, "No Provider?")
      RIB_free_provider_handle(hProvider);
      hProvider = NULL;  
   }
}

//--------------------------------------------------------------------------
DecoderStream::DecoderStream()
{
   hStream   = NULL;
   mDecoder = 0;
}

//-------------------------------------- 
DecoderStream::~DecoderStream()
{
   close();
}

//--------------------------------------
bool DecoderStream::open()
{
   if(!mDecoder)
      return false;

   close();
   hStream = mDecoder->streamOpen((U32)this, callback_router, 0);

   if (hStream)
      Con::printf("DecoderStream::open");
   else
   {
      Con::printf("DecoderStream::open FAILED");
      return false;
   }

   mInQueue.setSize(8000*5);
   mOutQueue.setSize(8000*2*5);
   
   // request an output rate
   U32 request = OUTPUT_RATE;
   mDecoder->streamSetPreference(hStream, mDecoder->REQUESTED_RATE, &request);

   U32 nch  = mDecoder->streamAttribute(hStream, mDecoder->OUTPUT_CHANNELS);
   U32 rate = mDecoder->streamAttribute(hStream, mDecoder->OUTPUT_SAMPLE_RATE);
   U32 bits = mDecoder->streamAttribute(hStream, mDecoder->OUTPUT_BITS);

   // make a few assumptions 
   if (nch != 1 || rate != OUTPUT_RATE || bits != 16)
   {
      close();
      return false;
   }

   return true;
}


//--------------------------------------
void DecoderStream::close()
{
   if(hStream && mDecoder)
      mDecoder->streamClose(hStream);
   hStream = 0;
}

void DecoderStream::setDecoder(VoiceDecoder * decoder)
{
   if(mDecoder && mDecoder != decoder)
      close();
   mDecoder = decoder;
}

//--------------------------------------
S32 AILCALLBACK DecoderStream::callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset)
{
   return ((DecoderStream*)user)->callback(dest, bytesRequested, offset);
}


//--------------------------------------------------------------------------
S32 DecoderStream::callback(void *dest, S32 bytesRequested, S32 offset)
{
   offset; // unused
   return mInQueue.dequeue((U8*)dest, bytesRequested);
}


//--------------------------------------
U32 DecoderStream::getBuffer(U8 **data, U32 *size)
{
   *data = mOutQueue.getHead();
   *size = mOutQueue.getContiguousUsed();
   mOutQueue.dequeue(*size);
   return *size;      
}


//--------------------------------------
void DecoderStream::setBuffer(U8 *data, U32 size)
{
   if (size > mInQueue.getFree())   
   {
      AssertWarn(0, "DecoderStream: queue full");
      return;   
   }

   mInQueue.enqueue(data, size);
}   


//-------------------------------------- 
void DecoderStream::process(bool flush)
{
   if (!hStream)
      return;

   while ( flush || (mInQueue.getUsed() && !mOutQueue.isFull()) )
   {

      S32 amount = mDecoder->streamProcess(hStream, mOutQueue.getTail(), mOutQueue.getContiguousFree());
      mOutQueue.enqueue(amount);
      
      if (flush && amount == 0)
         break;
   }
}


