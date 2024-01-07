//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIOCODECGSM_H_
#define _AUDIOCODECGSM_H_

#ifndef _AUDIOCODEC_H_
#include "audio/audioCodec.h"
#endif

class GSMEncoderCodec : public VoiceEncoderCodec
{
   private:
      GSMEncoderCodec();
      ~GSMEncoderCodec();

   public:
      static VoiceCodec *     create(U32 codecId);

      bool open();
      void close();
      
      void * openStream();
      void closeStream(void * stream);

      U32 process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen);
};

class GSMDecoderCodec : public VoiceDecoderCodec
{
   private:

      GSMDecoderCodec();
      ~GSMDecoderCodec();

   public:
      static VoiceCodec *        create(U32 codecId);

      bool open();
      void close();

      void * openStream();
      void closeStream(void * stream);

      U32 process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen);
};

#endif   // _INC_AUDIOCODECGSM
