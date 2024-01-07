//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMAUDIO_H_
#define _PLATFORMAUDIO_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _PLATFORMAL_H_
#include "PlatformWin32/platformAL.h"
#endif
#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif
#ifndef _BITSET_H_
#include "Core/bitSet.h"
#endif

typedef U32 AUDIOHANDLE;
#define NULL_AUDIOHANDLE 0

//--------------------------------------------------------------------------

namespace Audio
{
   enum AudioTypes {
      DefaultAudioType = 0,
      ChatAudioType,
      GuiAudioType,
      EffectAudioType,
      VoiceAudioType,
      MusicAudioType,

      NumAudioTypes
   };

   //--------------------------------------
   // sound property description
   struct Description
   {
      F32  mVolume;    // 0-1    1=loudest volume
      bool mIsLooping;
      bool mIs3D;

      F32  mMinDistance;
      F32  mMaxDistance;
      U32  mConeInsideAngle;
      U32  mConeOutsideAngle;
      F32  mConeOutsideVolume;
      Point3F mConeVector;

      // environment info
      F32 mEnvironmentLevel;

      // used by 'AudioEmitter' class
      S32  mLoopCount;
      S32  mMinLoopGap;
      S32  mMaxLoopGap;

      // each 'type' can have its own volume
      S32  mType;
   };

   struct DriverInfo
   {
      char *mName;
      char *mVender;
      char *mVersion;
      char *mRenderer;
      char *mExtensions;
   };

   void init();
   void detect();
   void destroy();

   bool setDriver(const char *name);
   const Vector<DriverInfo>* getDriverList();

   const char* getDriverListString();
   const char* getCurrentDriverInfo();
}   

class AudioDescription;
class AudioProfile;
class AudioEnvironment;
class AudioSampleEnvironment;

AUDIOHANDLE alxCreateSource(const Audio::Description *desc, const char *filename, const MatrixF *transform=NULL, AudioSampleEnvironment * sampleEnvironment = 0);
AUDIOHANDLE alxCreateSource(AudioDescription *descObject, const char *filename, const MatrixF *transform=NULL, AudioSampleEnvironment * sampleEnvironment = 0);
AUDIOHANDLE alxCreateSource(const AudioProfile *profile, const MatrixF *transform=NULL);

AUDIOHANDLE alxPlay(AUDIOHANDLE handle);
void alxStop(AUDIOHANDLE handle);
void alxStopAll();

// one-shot helper alxPlay functions, create and play in one call
AUDIOHANDLE alxPlay(const AudioProfile *profile, const MatrixF *transform=NULL, const Point3F *velocity=NULL);

// Source
void alxSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat value);
void alxSourcefv(AUDIOHANDLE handle, ALenum pname, ALfloat *values);
void alxSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3);
void alxSourcei(AUDIOHANDLE handle, ALenum pname, ALint value);
void alxSourceMatrixF(AUDIOHANDLE handle, const MatrixF *transform);

void alxGetSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat *value);
void alxGetSourcefv(AUDIOHANDLE handle, ALenum pname, ALfloat *values);
void alxGetSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3);
void alxGetSourcei(AUDIOHANDLE handle, ALenum pname, ALint *value);

inline void alxSourcePoint3F(AUDIOHANDLE handle, ALenum pname, const Point3F *value)
{
   alxSource3f(handle, pname, value->x, value->y, value->z);   
}

inline void alxSourceGetPoint3F(AUDIOHANDLE handle, ALenum pname, Point3F * value)
{
   alxGetSource3f(handle, pname, &value->x, &value->y, &value->z);
}

// Listener
void alxListenerf(ALenum pname, ALfloat value);
void alxListenerfv(ALenum pname, ALfloat *values);
void alxListener3f(ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3);
void alxListeneri(ALenum pname, ALint value);
void alxListenerMatrixF(const MatrixF *transform);

void alxGetListenerf(ALenum pname, ALfloat *value);
void alxGetListenerfv(ALenum pname, ALfloat *values);
void alxGetListener3f(ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3);
void alxGetListeneri(ALenum pname, ALint *value);

inline void alxListenerPoint3F(ALenum pname, const Point3F *value)
{
   alxListener3f(pname, value->x, value->y, value->z);   
}

inline void alxListenerGetPoint3F(ALenum pname, Point3F * value)
{
   alxGetListener3f(pname, &value->x, &value->y, &value->z);
}

// Environment
void alxEnvironmenti(ALenum pname, ALint value);
void alxEnvironmentf(ALenum pname, ALfloat value);
void alxGetEnvironmenti(ALenum pname, ALint * value);
void alxGetEnvironmentf(ALenum pname, ALfloat * value);

void alxSetEnvironment(const AudioEnvironment * environment);
const AudioEnvironment * alxGetEnvironment();

// voice
struct SimVoiceStreamEvent;
struct SimVoiceEvent;

void alxReceiveVoiceStream(SimVoiceStreamEvent *event);
void alxReceiveVoiceEvent(SimVoiceEvent *event);

// misc
ALuint alxGetWaveLen(ALuint buffer);
bool alxIsValidHandle(AUDIOHANDLE handle);
bool alxIsPlaying(AUDIOHANDLE handle);
void alxUpdate();

#endif  // _H_PLATFORMAUDIO_
