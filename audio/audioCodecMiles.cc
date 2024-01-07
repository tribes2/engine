//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "audio/audioCodecMiles.h"

//--------------------------------------------------------------------------
// Class MilesEncoderCodec:
//--------------------------------------------------------------------------
MilesEncoderCodec::MilesEncoderCodec(const char * ext) : VoiceEncoderCodec()
{
   mProvider = 0;
   mProcStreamOpen = 0;
   mProcStreamProcess = 0;
   mProcStreamClose = 0;
   mQueue = 0;

   mExtension = dStrdup(ext);
}

MilesEncoderCodec::~MilesEncoderCodec()
{
   close();
   dFree(mExtension);
}

VoiceCodec * MilesEncoderCodec::create(U32 codecId)
{
   const char * ext = 0;
   switch(codecId)
   {
      case AUDIO_CODEC_V12:   ext = ".v12";  break;
      case AUDIO_CODEC_V24:   ext = ".v24";  break;
      case AUDIO_CODEC_V29:   ext = ".v29";  break;

      default:
         return(0);
   }

   // create and open the codec
   MilesEncoderCodec * codec = new MilesEncoderCodec(ext);
   if(!codec->open())
   {
      delete codec;
      codec = 0;
   }

   return(codec);
}

//--------------------------------------------------------------------------
bool MilesEncoderCodec::open()
{
   if(mProvider)
      return(true);
   
   RIB_INTERFACE_ENTRY  ENCODER_REQUEST[] = {
      { RIB_FUNCTION, "ASI_stream_open",     (U32)&mProcStreamOpen,    RIB_NONE },
      { RIB_FUNCTION, "ASI_stream_close",    (U32)&mProcStreamClose,   RIB_NONE },
      { RIB_FUNCTION, "ASI_stream_process",  (U32)&mProcStreamProcess, RIB_NONE } };

   mProvider = RIB_find_file_provider("ASI codec", "Output file types", mExtension);
   if(mProvider && (RIB_request(mProvider, "ASI stream", ENCODER_REQUEST) == RIB_NOERR))
      return(true);

   return(false);
}

void MilesEncoderCodec::close()
{
   if(mProvider)
   {
      RIB_free_provider_handle(mProvider);
      mProvider = 0;
   }
}

//--------------------------------------------------------------------------
void * MilesEncoderCodec::openStream()
{
   HASISTREAM handle = mProcStreamOpen((U32)this, callback_router, 0);
   if(!handle)
      return(0);
   
   HASISTREAM * stream = new HASISTREAM;
   *stream = handle;
   return(static_cast<void *>(stream));
}

void MilesEncoderCodec::closeStream(void * stream)
{
   AssertFatal(stream, "MilesEncoderCodec::closeStream: invalid stream ptr");
   HASISTREAM * pHandle = static_cast<HASISTREAM*>(stream);

   if(mProvider)
      mProcStreamClose(*pHandle);
   delete pHandle;
}

//--------------------------------------------------------------------------
S32 AILCALLBACK MilesEncoderCodec::callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset)
{
   return(((MilesEncoderCodec*)user)->callback(dest, bytesRequested, offset));
}

S32 MilesEncoderCodec::callback(void * dest, S32 bytesRequested, S32 offset)
{
   offset; 
   return(mQueue->dequeue((U8*)dest, bytesRequested));
}

U32 MilesEncoderCodec::process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen)
{
   if(!stream || !queue)
      return(0);

   mQueue = queue;
   U32 amount = U32(mProcStreamProcess(*static_cast<HASISTREAM * >(stream), static_cast<void*>(const_cast<U8*>(data)), S32(maxLen)));
   mQueue = 0;

   return(amount);
}


//--------------------------------------------------------------------------
// Class MilesDecoderCodec:
//--------------------------------------------------------------------------
MilesDecoderCodec::MilesDecoderCodec(const char * ext) : VoiceDecoderCodec()
{
   mProvider = 0;

   mProcStreamOpen = 0;
   mProcStreamProcess = 0;
   mProcStreamSeek = 0;
   mProcStreamClose = 0;
   mProcStreamAttribute = 0;
   mProcStreamSetPreference = 0;

   mAttribOutputSampleRate = 0;
   mAttribOutputBits = 0;
   mAttribOutputChannels = 0;
   mAttribRequestedRate = 0;

   mQueue = 0;
   mExtension = dStrdup(ext);
}

MilesDecoderCodec::~MilesDecoderCodec()
{
   close();
   dFree(mExtension);
}

VoiceCodec * MilesDecoderCodec::create(U32 codecId)
{
   const char * ext = 0;
   switch(codecId)
   {
      case AUDIO_CODEC_V12:   ext = ".v12";  break;
      case AUDIO_CODEC_V24:   ext = ".v24";  break;
      case AUDIO_CODEC_V29:   ext = ".v29";  break;

      default:
         return(0);
   }

   // create and open the codec
   MilesDecoderCodec * codec = new MilesDecoderCodec(ext);
   if(!codec->open())
   {
      delete codec;
      codec = 0;
   }

   return(codec);
}

//--------------------------------------------------------------------------
bool MilesDecoderCodec::open()
{
   if(mProvider)
      return(true);
   
   RIB_INTERFACE_ENTRY DECODER_REQUEST[] =  {
      { RIB_FUNCTION,   "ASI_stream_attribute",      (U32) &mProcStreamAttribute,      RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_open",           (U32) &mProcStreamOpen,           RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_seek",           (U32) &mProcStreamSeek,           RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_close",          (U32) &mProcStreamClose,          RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_process",        (U32) &mProcStreamProcess,        RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_set_preference", (U32) &mProcStreamSetPreference,  RIB_NONE },
      { RIB_ATTRIBUTE,  "Output sample rate",        (U32) &mAttribOutputSampleRate,   RIB_NONE },
      { RIB_ATTRIBUTE,  "Output sample width",       (U32) &mAttribOutputBits,         RIB_NONE },
      { RIB_ATTRIBUTE,  "Output channels",           (U32) &mAttribOutputChannels,     RIB_NONE },
      { RIB_PREFERENCE, "Requested sample rate",     (U32) &mAttribRequestedRate,      RIB_NONE } };

   mProvider = RIB_find_file_provider("ASI codec", "Input file types", mExtension);
   if(mProvider && (RIB_request(mProvider, "ASI stream", DECODER_REQUEST) == RIB_NOERR))
      return(true);

   return(false);
}

void MilesDecoderCodec::close()
{
   if(mProvider)
   {
      RIB_free_provider_handle(mProvider);
      mProvider = 0;
   }
}

//--------------------------------------------------------------------------
void * MilesDecoderCodec::openStream()
{
   if(!mProvider)
      return(0);

   HASISTREAM handle = mProcStreamOpen((U32)this, callback_router, 0);
   if(!handle)
      return(0);

   // properties...
   U32 requestedRate = VOICE_FREQUENCY;
   mProcStreamSetPreference(handle, mAttribRequestedRate, &requestedRate);

   U32 chan = mProcStreamAttribute(handle, mAttribOutputChannels);
   U32 rate = mProcStreamAttribute(handle, mAttribOutputSampleRate);
   U32 bits = mProcStreamAttribute(handle, mAttribOutputBits);

   if((chan != VOICE_CHANNELS) || (rate != VOICE_FREQUENCY) || (bits != VOICE_BITS))
   {
      mProcStreamClose(handle);
      return(0);
   }

   HASISTREAM * stream = new HASISTREAM;
   *stream = handle;
   return(static_cast<void *>(stream));
}

void MilesDecoderCodec::closeStream(void * stream)
{
   AssertFatal(stream, "MilesDecoderCodec::closeStream: invalid stream ptr");
   HASISTREAM * pHandle = static_cast<HASISTREAM*>(stream);

   if(mProvider)
      mProcStreamClose(*pHandle);
   delete pHandle;
}

//--------------------------------------------------------------------------
S32 AILCALLBACK MilesDecoderCodec::callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset)
{
   return(((MilesDecoderCodec*)user)->callback(dest, bytesRequested, offset));
}

S32 MilesDecoderCodec::callback(void * dest, S32 bytesRequested, S32 offset)
{
   offset; 
   return(mQueue->dequeue((U8*)dest, bytesRequested));
}

U32 MilesDecoderCodec::process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen)
{
   if(!stream || !queue)
      return(0);

   mQueue = queue;
   U32 amount = U32(mProcStreamProcess(*static_cast<HASISTREAM * >(stream), static_cast<void*>(const_cast<U8*>(data)), S32(maxLen)));
   mQueue = 0;

   return(amount);
}
