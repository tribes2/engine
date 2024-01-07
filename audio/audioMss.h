//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIOMSS_H_
#define _AUDIOMSS_H_

#ifndef _AUDIONET_H_
#include "audio/audioNet.h"
#endif
#ifndef _GAMECONNECTION_H_
#include "game/gameConnection.h"
#endif
#ifndef _MSS_H_
#include "mss.h"
#endif
#ifndef _FILESTREAM_H_
#include "core/fileStream.h"
#endif
#ifndef _BUFFERQUEUE_H_
#include "audio/bufferQueue.h"
#endif

//--------------------------------------------------------------------------
// we can only have one encoder... but multiple decoders can be installed
class EncoderStream
{
private:
   static HPROVIDER  hProvider;    // ASI provider used to encode data
   static U32                       mCodecLevel;

   static ASI_STREAM_OPEN           streamOpen;
   static ASI_STREAM_PROCESS        streamProcess;
   static ASI_STREAM_CLOSE          streamClose;

   HASISTREAM hStream;
   BufferQueue mQueue;

   GameConnection *mConnection;
   U8 mStreamId;
   U8 mSequence;

   S32 callback(void *dest, S32 bytesRequested, S32 offset);
   static S32 AILCALLBACK callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset);
   void sendPacket(bool flush=false);

public:

   EncoderStream();
   ~EncoderStream();

   static bool open(U32 codecLevel);
   static void close();

   bool openStream();
   void closeStream();

   bool setConnection(GameConnection *con);
   void setBuffer(const U8 *buffer, U32 size);
   void process(bool flush=false);
   void flush();
};

//--------------------------------------------------------------------------
class VoiceDecoder
{
   friend class DecoderStream;

private:
   HPROVIDER  hProvider;    // ASI provider used to decode data

   ASI_STREAM_OPEN           streamOpen;
   ASI_STREAM_PROCESS        streamProcess;
   ASI_STREAM_SEEK           streamSeek;
   ASI_STREAM_CLOSE          streamClose;
   ASI_STREAM_ATTRIBUTE      streamAttribute;
   ASI_STREAM_SET_PREFERENCE streamSetPreference;

   HATTRIB OUTPUT_SAMPLE_RATE;
   HATTRIB OUTPUT_BITS;
   HATTRIB OUTPUT_CHANNELS;
   HATTRIB REQUESTED_RATE;

   U32                     mCodecLevel;
   
public:
   VoiceDecoder();
   ~VoiceDecoder();
   
   U32 getCodecLevel()  {return(mCodecLevel);}
   bool open(U32 codecLevel);
   void close();
};

class DecoderStream
{
   HASISTREAM hStream;

   GameConnection *mConnection;
   BufferQueue mInQueue;
   BufferQueue mOutQueue;

   enum { OUTPUT_RATE = 8000 };

   VoiceDecoder * mDecoder;
   
   //--------------------------------------
   S32 callback(void *dest, S32 bytesRequested, S32 offset);
   static S32 AILCALLBACK callback_router(U32 user, void FAR *dest, S32 bytesRequested, S32 offset);

public:
   DecoderStream();
   ~DecoderStream();

   void setDecoder(VoiceDecoder * decoder);
   bool open();
   void close();

   void setBuffer(U8 *data, U32 size);
   U32  getBuffer(U8 **data, U32 *size);
   void process(bool flush=false);
};


#endif  // _H_AUDIOMSS_
