//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "audio/audioBuffer.h"
#include "Core/stream.h"
#include "console/console.h"
#include "Core/fileStream.h"
#include "audio/audioThread.h"

//--------------------------------------
AudioBuffer::AudioBuffer(StringTableEntry filename)
{
   AssertFatal(StringTable->lookup(filename), "AudioBuffer:: filename is not a string table entry");
   
   mFilename = filename;
   mLoading = false;
   malBuffer = AL_INVALID;
}

AudioBuffer::~AudioBuffer()
{
   if( malBuffer != AL_INVALID ) {
      alDeleteBuffers( 1, &malBuffer );
   }
}   

//--------------------------------------
Resource<AudioBuffer> AudioBuffer::find(const char *filename)
{
   Resource<AudioBuffer> buffer = ResourceManager->load(filename);
   if (bool(buffer) == false)
   {
      char buf[512];
      dSprintf(buf, sizeof(buf), "audio/%s", filename);

      // see if the file exists
      if (ResourceManager->getPathOf(buf))             
      {
         AudioBuffer *temp = new AudioBuffer(StringTable->insert(filename));
         ResourceManager->add(filename, temp);
         buffer = ResourceManager->load(filename);
      }
   }
   return buffer;
}   

ResourceInstance* AudioBuffer::construct(Stream &)
{
   return NULL;
}   

//-----------------------------------------------------------------
ALuint AudioBuffer::getALBuffer(bool block)
{
// jff: block until thread is completely stable (resourcemanager is not
// thread safe yet (though it never seems to crash there...) )
   block = true;
// sol: if the above "block = true" is removed, uncomment the following
// error so that the Linux build maintainer knows to re-enable the audio
// thread.  It's disabled at the moment as an optimization.
//#ifdef __linux
//#error Linux version needs to re-enable audio thread in platformLinux/audio.cc
//#endif

   // clear the error state
   alGetError();

   if (alIsBuffer(malBuffer))
      return malBuffer;

   alGenBuffers(1, &malBuffer);
   if(alGetError() != AL_NO_ERROR)
      return(AL_INVALID);

   char buffer[512];
   dSprintf(buffer, sizeof(buffer), "audio/%s", mFilename);

   ResourceObject * obj = ResourceManager->find(buffer);
   if(obj)
   {
      if(block)
      {
         bool readWavSuccess = readWAV(obj);
         if(readWavSuccess)
            return(malBuffer);
      }
      else if(gAudioThread)
      {
         gAudioThread->loadResource(obj, this);
         return(malBuffer);
      }
   }

   alDeleteBuffers(1, &malBuffer);
   return(AL_INVALID);
}

bool AudioBuffer::readWAV(ResourceObject *obj)
{
   if(Audio::doesSupportDynamix())
   {
      U32 size = obj->fileSize;
      Stream *str = ResourceManager->openStream(obj);
      void * data = dMalloc(obj->fileSize);
      str->read(obj->fileSize, data);
      ResourceManager->closeStream(str);

      if(alBufferSyncData_EXT(malBuffer, AL_FORMAT_WAVE_EXT, data, obj->fileSize, 0))
         return(true);

      dFree(data);
   }
   return(false);
}

