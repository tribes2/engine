//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <stdio.h>

extern "C" 
{
#include "inc/gsm.h"
}
#include "audio/audioCodecGSM.h"

// The number of samples encoded at once in the GSM spec
#define GSM_FRAMESIZE		160
#define GSM_FRAMESIZE_BYTES	GSM_FRAMESIZE*sizeof(gsm_signal)

//--------------------------------------------------------------------------
// Class GSMEncoderCodec:
//--------------------------------------------------------------------------
GSMEncoderCodec::GSMEncoderCodec()
{
}

GSMEncoderCodec::~GSMEncoderCodec()
{
   close();
}

VoiceCodec * GSMEncoderCodec::create(U32 codecId)
{
   if(codecId != AUDIO_CODEC_GSM)
      return(0);

   // create and open the codec
   GSMEncoderCodec * codec = new GSMEncoderCodec();
   if(!codec->open())
   {
      delete codec;
      codec = 0;
   }

   return(codec);
}

//--------------------------------------------------------------------------
bool GSMEncoderCodec::open()
{
   return true;
}

void GSMEncoderCodec::close()
{
}

//--------------------------------------------------------------------------
void * GSMEncoderCodec::openStream()
{
   return gsm_create();
}

void GSMEncoderCodec::closeStream(void * stream)
{
   if ( stream )
      gsm_destroy((gsm)stream);
}

U32 GSMEncoderCodec::process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen)
{
   gsm_frame frame;
   gsm_signal gsmdata[  GSM_FRAMESIZE];
   gsm_signal samples[2*GSM_FRAMESIZE];
   U32 encoded;
   U8 *output;
   const U32 framesize = (sizeof frame);

   // Sanity check
   if ( !stream || !queue ) {
      return 0;
   }
   output = const_cast<U8*>(data);
   encoded = 0;
   while ( maxLen >= framesize ) {
      if ( queue->getUsed() < (sizeof samples) ) {
         break;
      }
      queue->dequeue((U8*)samples, (sizeof samples));

      // Convert the samples to 4000 Hz
      for ( int i=0, j=0; i<GSM_FRAMESIZE; i += 1, j += 2 ) {
         gsmdata[i] = ((S32)samples[j]+samples[j+1])/2;
      }
      gsm_encode((gsm)stream, gsmdata, frame);

      dMemcpy(output, frame, framesize);
      encoded += framesize;
      output += framesize;
      maxLen -= framesize;
   }
   return encoded;
}


//--------------------------------------------------------------------------
// Class GSMDecoderCodec:
//--------------------------------------------------------------------------
GSMDecoderCodec::GSMDecoderCodec()
{
}

GSMDecoderCodec::~GSMDecoderCodec()
{
   close();
}

VoiceCodec * GSMDecoderCodec::create(U32 codecId)
{
   if(codecId != AUDIO_CODEC_GSM)
      return(0);

   // create and open the codec
   GSMDecoderCodec * codec = new GSMDecoderCodec();
   if(!codec->open())
   {
      delete codec;
      codec = 0;
   }

   return(codec);
}

//--------------------------------------------------------------------------
bool GSMDecoderCodec::open()
{
   return(true);
}

void GSMDecoderCodec::close()
{
}

//--------------------------------------------------------------------------
void * GSMDecoderCodec::openStream()
{
   return gsm_create();
}

void GSMDecoderCodec::closeStream(void * stream)
{
   if ( stream )
      gsm_destroy((gsm)stream);
}

U32 GSMDecoderCodec::process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen)
{
   gsm_frame frame;
   gsm_signal gsmdata[GSM_FRAMESIZE];
   U32 decoded;
   U8 *output;
   const U32 framesize = 2*GSM_FRAMESIZE_BYTES;

   // Sanity check
   if ( !stream || !queue ) {
      return 0;
   }
   output = const_cast<U8*>(data);
   decoded = 0;
   while ( maxLen >= framesize ) {
      gsm_signal *samples = (gsm_signal *)output;

      // Decode another frame of data
      if ( queue->getUsed() < (sizeof frame) ) {
         break;
      }
      queue->dequeue((U8*)frame, (sizeof frame));

      gsm_decode((gsm)stream, frame, gsmdata);

      // Convert the samples from 4000 Hz
      for ( int i=0, j=0; i<GSM_FRAMESIZE; i += 1, j += 2 ) {
         samples[j] = gsmdata[i];
         if ( i == (GSM_FRAMESIZE-1) ) {
            samples[j+1] = gsmdata[i];
         } else {
            samples[j+1] = ((S32)gsmdata[i]+gsmdata[i+1])/2;
         }
      }
      decoded += framesize;
      output += framesize;
      maxLen -= framesize;
   }
   return decoded;
}
