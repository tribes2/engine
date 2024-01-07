//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIOCODEC_H_
#define _AUDIOCODEC_H_

#ifndef _AUDIONET_H_
#include "audio/audioNet.h"
#endif
#ifndef _GAMECONNECTION_H_
#include "game/gameConnection.h"
#endif
#ifndef _FILESTREAM_H_
#include "core/fileStream.h"
#endif
#ifndef _BUFFERQUEUE_H_
#include "audio/bufferQueue.h"
#endif

//--------------------------------------------------------------------------
#define  VOICE_FREQUENCY       8000
#define  VOICE_BITS            16
#define  VOICE_CHANNELS        1
#define  VOICE_LENGTH          5       // in seconds

enum {
   AUDIO_CODEC_V12 = 0,
   AUDIO_CODEC_V24,
   AUDIO_CODEC_V29,
   AUDIO_CODEC_GSM,

   AUDIO_NUM_CODECS,
   AUDIO_CODEC_NONE = -1
};

//--------------------------------------------------------------------------
// Class VoiceCodec:
//--------------------------------------------------------------------------
class VoiceCodec
{
   public:
      VoiceCodec()               {};
      virtual ~VoiceCodec()      {};

      virtual bool open() = 0;
      virtual void close() = 0;

      virtual void * openStream() = 0;
      virtual void closeStream(void * stream) = 0;

      virtual U32 process(void * stream, BufferQueue * queue, const U8 * data, U32 maxLen) = 0;
};

class VoiceDecoderCodec : public VoiceCodec {};
class VoiceEncoderCodec : public VoiceCodec {};

typedef VoiceCodec * (*CODEC_CREATE_PROC)(U32 codecId);

//-------------------------------------------------------------------------
// Struct AudioCodecManager:
//-------------------------------------------------------------------------
struct AudioCodecManager
{
   struct CodecInfo
   {
      S32                  mId;
      CODEC_CREATE_PROC    mCreateEncoder;
      CODEC_CREATE_PROC    mCreateDecoder;
      
      VoiceEncoderCodec *  mEncoder;
      VoiceDecoderCodec *  mDecoder;
   };
   static CodecInfo        smCodecTable[];
   
   static VoiceEncoderCodec * createEncoderCodec(S32 codecId);
   static VoiceDecoderCodec * createDecoderCodec(S32 codecId);

   static void destroy();
};

//-------------------------------------------------------------------------
// Class VoiceEncoderStream:
//-------------------------------------------------------------------------
class VoiceEncoderStream
{
   private:
      BufferQueue          mQueue;
      VoiceEncoderCodec *  mEncoder;
      S32                  mEncoderId;

      GameConnection *     mConnection;
      U8                   mStreamId;
      U8                   mSequence;

      void *               mStream;

   public:
      VoiceEncoderStream();
      ~VoiceEncoderStream();

      S32 getCodec()                { return(mEncoderId); }
      bool setCodec(S32 codecId);

      bool open();
      void close();

      bool setConnection(GameConnection *);
      bool setBuffer(const U8 * data, U32 size);

      void process(bool flush=false);
      void flush();
};

//-------------------------------------------------------------------------
// Class VoiceDecoderStream:
//-------------------------------------------------------------------------
class VoiceDecoderStream
{
   private:
      VoiceDecoderCodec *  mDecoder;
      S32                  mDecoderId;

      BufferQueue          mInQueue;
      BufferQueue          mOutQueue;

      void *               mStream;
      
   public:
      VoiceDecoderStream();
      ~VoiceDecoderStream();
      
      S32 getCodec()                { return(mDecoderId); }
      bool setCodec(S32 codecId);
      
      bool open();
      void close();

      bool setBuffer(U8 * data, U32 size);
      U32  getBuffer(U8 ** data, U32 * size);
      void process(bool flush=false);
};

#endif   // _INC_AUDIOCODEC
