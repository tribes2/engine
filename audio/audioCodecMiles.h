//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIOCODECMILES_H_
#define _AUDIOCODECMILES_H_

#ifndef _AUDIOCODEC_H_
#include "audio/audioCodec.h"
#endif
#ifndef _MSS_H_
#include "mss.h"
#endif

class MilesEncoderCodec : public VoiceEncoderCodec
{
   private:
      MilesEncoderCodec(const char * ext);
      ~MilesEncoderCodec();

      char *               mExtension;
      HPROVIDER            mProvider;

      ASI_STREAM_OPEN      mProcStreamOpen;
      ASI_STREAM_PROCESS   mProcStreamProcess;
      ASI_STREAM_CLOSE     mProcStreamClose;

      BufferQueue *        mQueue;

      static S32 AILCALLBACK callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset);
      S32 callback(void * dest, S32 bytesRequested, S32 offset);

   public:
      static VoiceCodec *     create(U32 codecId);

      bool open();
      void close();
      
      void * openStream();
      void closeStream(void * stream);

      U32 process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen);
};

class MilesDecoderCodec : public VoiceDecoderCodec
{
   private:

      MilesDecoderCodec(const char * ext);
      ~MilesDecoderCodec();

      char *                     mExtension;
      HPROVIDER                  mProvider;

      ASI_STREAM_OPEN            mProcStreamOpen;
      ASI_STREAM_PROCESS         mProcStreamProcess;
      ASI_STREAM_SEEK            mProcStreamSeek;
      ASI_STREAM_CLOSE           mProcStreamClose;
      ASI_STREAM_ATTRIBUTE       mProcStreamAttribute;
      ASI_STREAM_SET_PREFERENCE  mProcStreamSetPreference;

      HATTRIB                    mAttribOutputSampleRate;
      HATTRIB                    mAttribOutputBits;
      HATTRIB                    mAttribOutputChannels;
      HATTRIB                    mAttribRequestedRate;

      BufferQueue *              mQueue;

      static S32 AILCALLBACK callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset);
      S32 callback(void * dest, S32 bytesRequested, S32 offset);

   public:
      static VoiceCodec *        create(U32 codecId);

      bool open();
      void close();

      void * openStream();
      void closeStream(void * stream);

      U32 process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen);
};

#endif   // _INC_AUDIOCODEMILES
