//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIOBUFFER_H_
#define _AUDIOBUFFER_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _PLATFORMAL_H_
#include "PlatformWin32/platformAL.h"
#endif
#ifndef _RESMANAGER_H_
#include "Core/resManager.h"
#endif

//--------------------------------------------------------------------------

class AudioBuffer: public ResourceInstance
{
   friend class AudioThread;

private:
   StringTableEntry  mFilename;
   bool              mLoading;
   ALuint            malBuffer;

   bool readRIFFchunk(Stream &s, const char *seekLabel, U32 *size);
   bool readWAV(ResourceObject *obj);

public:
   AudioBuffer(StringTableEntry filename);
   ~AudioBuffer();
   ALuint getALBuffer(bool block = false);
   bool isLoading() {return(mLoading);}

   static Resource<AudioBuffer> find(const char *filename);
   static ResourceInstance* construct(Stream& stream);

};


#endif  // _H_AUDIOBUFFER_
