//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "audio/audio.h"
#include "audio/audioNet.h"
#include "core/tVector.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "game/gameConnection.h"
#include "audio/audioCodec.h"
#include "core/fileStream.h"
#include "audio/audioThread.h"

// Comment the following line to enable the audio thread again
#ifdef __linux
#define DISABLE_AUDIO_THREAD
#endif

// miles openAL implementation does not handle 3dsource volumes through listener for all providers, 
// so wrapper takes care of all this (undef if not needed)
// - scores for sources are attenuated by channel gain but not by listener gain
#define HANDLE_LISTENER_GAIN

//-------------------------------------------------------------------------
namespace {

#define RECORD_SPEED          8000              // default rate for capturing
#define RECORD_FORMAT         AL_FORMAT_MONO16  // capture format
#define RECORD_BUFFERSIZE     1024              // 1k capture buffer size
#define RECORD_LEN            (4 * 1000)        // 4seconds max capture time
#define MAX_AUDIOSOURCES      16                // maximum number of concurrent sources
#define MIN_GAIN              0.05f             // anything with lower gain will not be started
#define MIN_CAPTURE_SCALE     0.1f              // for scaling captured buffers
#define MAX_CAPTURE_SCALE     5.f
#define MIN_UNCULL_PERIOD     500               // time before buffer is checked to be unculled
#define MIN_UNCULL_GAIN       0.1f              // min gain of source to be unculled

#define ALX_DEF_SAMPLE_RATE      44100          // default values for mixer
#define ALX_DEF_SAMPLE_BITS      16
#define ALX_DEF_CHANNELS         2

#define FORCED_OUTER_FALLOFF  10000.f           // forced falloff distance
static bool mDisableOuterFalloffs = false;      // forced max falloff?
static F32 mInnerFalloffScale = 1.f;            // amount to scale inner falloffs

static Vector<Audio::DriverInfo> mDriverInfoList(__FILE__, __LINE__);
static bool mInitialized = false;
static Audio::DriverInfo *mActiveDriver = NULL;          // the current audio driver (miles, none, ...)
static F32 mCaptureGainScale = 1.f;                      // amount to scale captured buffers before sending to server

static F32 mAudioTypeVolume[Audio::NumAudioTypes];       // the attenuation for each of the channel types

//-------------------------------------------------------------------------
struct LoopingImage
{
   AUDIOHANDLE                   mHandle;
   Resource<AudioBuffer>         mBuffer;
   Audio::Description            mDescription;
   AudioSampleEnvironment *      mEnvironment;
      
   Point3F                       mPosition;
   Point3F                       mDirection;
   F32                           mPan;
   F32                           mPitch;
   F32                           mScore;
   S32                           mCullTime;

   LoopingImage()  { clear(); }

   void clear()
   {
      mHandle           = NULL_AUDIOHANDLE;
      mBuffer           = NULL;
      dMemset(&mDescription, 0, sizeof(Audio::Description));
      mEnvironment = 0;
      mPosition.set(0.f,0.f,0.f);
      mDirection.set(0.f,1.f,0.f);
      mPan = 0.f;
      mPitch = 1.f;
      mScore = 0.f;
      mCullTime = 0;
   }
};   

//-------------------------------------------------------------------------
struct VoiceStream
{
   U32                           mClientId;
   U8                            mCodecId;
   U8                            mStreamId;
   VoiceDecoderStream            mDecoderStream;
   ALuint                        mBuffer;
   AUDIOHANDLE                   mHandle;
};

static F32 mMasterVolume = 1.f;           // traped from AL_LISTENER gain (miles has difficulties with 3d sources)

static ALuint                 mSource[MAX_AUDIOSOURCES];                   // ALSources
static AUDIOHANDLE            mHandle[MAX_AUDIOSOURCES];                   // unique handles
static Resource<AudioBuffer>  mBuffer[MAX_AUDIOSOURCES];                   // each of the playing buffers (needed for AudioThread)
static F32                    mScore[MAX_AUDIOSOURCES];                    // for figuring out which sources to cull/uncull
static F32                    mSourceVolume[MAX_AUDIOSOURCES];             // the samples current un-attenuated gain (not scaled by master/channel gains)
static U32                    mType[MAX_AUDIOSOURCES];                     // the channel which this source belongs
static bool                   mRecording = false;                          // currently recording?
static bool                   mLocalCapture = false;                       // recording to local buffer?
static ALboolean              mCaptureInitialized = AL_FALSE;              // capture initialized?

static ALuint                 mStreamBuffer;                               // the music strean buffer (only 1 music stream currently supported)
static AUDIOHANDLE            mStreamHandle;                               // handle to the music stream
static bool                   mFinishedMusicStream = false;                // set in callback (al thread) to process end of music stream 
static bool                   mFinishedMusicStreamStopState = false;       // set in callback (passed along to console)   

static AudioSampleEnvironment *        mSampleEnvironment[MAX_AUDIOSOURCES];           // currently playing sample environments
static bool                            mEnvironmentEnabled = false;                    // environment enabled?
static SimObjectPtr<AudioEnvironment>  mCurrentEnvironment;                            // the last environment set

#define LOCAL_CAPTURE_SIZE       (RECORD_LEN * RECORD_SPEED)
static U8 *           mLocalCaptureBuffer = 0;                 // buffer to contain captured data
static U32            mLocalCaptureBufferPos = 0;              // current capture buffer pos (circular)
static bool           mLocalCaptureFinished = false;           // finished capturing? (set in callback)
static ALuint         mALCaptureBuffer = 0;                    // the buffer used to playback local capture data
static ALuint         mEnvironment = 0;                        // al environment handle

struct LoopingList : VectorPtr<LoopingImage*>
{
   LoopingList() : VectorPtr<LoopingImage*>(__FILE__, __LINE__) { }

   LoopingList::iterator findImage(AUDIOHANDLE handle);
   void sort();
};

// LoopingList and LoopingFreeList own the images
static LoopingList               mLoopingList;                 // all the looping sources
static LoopingList               mLoopingFreeList;             // free store
static LoopingList               mLoopingInactiveList;         // sources which have not been played yet
static LoopingList               mLoopingCulledList;           // sources which have been culled (alxPlay called)

static VectorPtr<VoiceStream*>   sVoiceStreams(__FILE__, __LINE__);
static VectorPtr<VoiceStream*>   sVoiceStreams_free(__FILE__, __LINE__);

static VoiceEncoderStream        mVoiceEncoderStream;

#define AUDIOHANDLE_LOOPING_BIT     (0x80000000)
#define AUDIOHANDLE_VOICE_BIT       (0x40000000)
#define AUDIOHANDLE_INACTIVE_BIT    (0x20000000)   
#define AUDIOHANDLE_LOADING_BIT     (0x10000000)
#define HANDLE_MASK           ~(AUDIOHANDLE_LOOPING_BIT | AUDIOHANDLE_VOICE_BIT | AUDIOHANDLE_INACTIVE_BIT | AUDIOHANDLE_LOADING_BIT)

// keep the 'AUDIOHANDLE_LOOPING_BIT' on the handle returned to the caller so that
// the handle can quickly be rejected from looping list queries
#define RETURN_MASK           ~(AUDIOHANDLE_VOICE_BIT | AUDIOHANDLE_INACTIVE_BIT | AUDIOHANDLE_LOADING_BIT)
static AUDIOHANDLE mLastHandle = NULL_AUDIOHANDLE;

static bool mForceMaxDistanceUpdate = false;             // force gain setting for 3d distances
static U32 mNumSources = 0;                              // total number of sources to work with
static U32 mRequestSources = MAX_AUDIOSOURCES;           // number of sources to request from openAL

#define INVALID_SOURCE        0xffffffff
#define CAPTURE_BUFFER_SIZE   (300)

static U32 sCaptureTimeout = 0;

inline bool areEqualHandles(AUDIOHANDLE a, AUDIOHANDLE b)
{
   return((a & HANDLE_MASK) == (b & HANDLE_MASK));
}

//-------------------------------------------------------------------------
// Looping image
//-------------------------------------------------------------------------
inline LoopingList::iterator LoopingList::findImage(AUDIOHANDLE handle)
{
   if(handle & AUDIOHANDLE_LOOPING_BIT)
   {
      LoopingList::iterator itr = begin();
      while(itr != end())
      {
         if(areEqualHandles((*itr)->mHandle, handle))
            return(itr);
         itr++;
      }
   }
   return(0);
}

inline int QSORT_CALLBACK loopingImageSort(const void * p1, const void * p2)
{
   const LoopingImage * ip1 = *(const LoopingImage**)p1;
   const LoopingImage * ip2 = *(const LoopingImage**)p2;

   // min->max
   return ip2->mScore - ip1->mScore;
}

void LoopingList::sort()
{
   dQsort(address(), size(), sizeof(LoopingImage*), loopingImageSort);
}

//-------------------------------------------------------------------------
LoopingImage * createLoopingImage()
{
   LoopingImage *image;
   if (mLoopingFreeList.size())
   {
      image = mLoopingFreeList.last();
      mLoopingFreeList.pop_back();
   }
   else
      image = new LoopingImage;
   return(image);
}

//-------------------------------------------------------------------------
static AUDIOHANDLE getNewHandle()
{
   mLastHandle++;
   mLastHandle &= HANDLE_MASK;
   if (mLastHandle == NULL_AUDIOHANDLE)
      mLastHandle++;
   return mLastHandle;
}
} // namespace {}

//-------------------------------------------------------------------------
// function declarations
void alxLoopingUpdate();
void alxUpdateScores(bool);

//-------------------------------------------------------------------------
static void alxFreeVoiceStream(VectorPtr<VoiceStream*>::iterator itr)
{
   VoiceStream *vs = *itr;
   sVoiceStreams.erase_fast(itr);
   sVoiceStreams_free.push_back(vs);

   vs->mHandle = NULL_AUDIOHANDLE;
   vs->mDecoderStream.close();

   Con::evaluatef("clientCmdPlayerStoppedTalking(%d, 1);", vs->mClientId);
}   

static void alxFreeVoiceStream(AUDIOHANDLE handle)
{
   VectorPtr<VoiceStream*>::iterator itr = sVoiceStreams.begin();
   for (; itr != sVoiceStreams.end(); itr++)
      if ((*itr)->mHandle == handle)
      {
         alxFreeVoiceStream(itr);
         return;
      }
}   

inline VectorPtr<VoiceStream*>::iterator alxFindVoiceStream(AUDIOHANDLE handle)
{
   for(VectorPtr<VoiceStream*>::iterator itr = sVoiceStreams.begin(); itr != sVoiceStreams.end(); itr++)
      if(areEqualHandles((*itr)->mHandle, handle))
         return(itr);
   return(0);
}

static bool findFreeSource(U32 *index)
{
   for(U32 i = 0; i < mNumSources; i++)
      if(mHandle[i] == NULL_AUDIOHANDLE)
      {
         *index = i;
         return(true);
      }
   return(false);
}   

//--------------------------------------------------------------------------
// - cull out the min source that is below volume
// - streams/voice/loading streams are all scored > 2
// - volumes are attenuated by channel only
static bool cullSource(U32 *index, F32 volume)
{
   alGetError();

   F32 minVolume = volume;
   S32 best = -1;
   for(S32 i = 0; i < mNumSources; i++)
   {
      if(mScore[i] < minVolume)
      {
         minVolume = mScore[i];
         best = i;
      }
   }
   
   if(best == -1)
      return(false);

   // check if culling a looper
   LoopingList::iterator itr = mLoopingList.findImage(mHandle[best]);
   if(itr)
   {
      // check if culling an inactive looper
      if(mHandle[best] & AUDIOHANDLE_INACTIVE_BIT)
      {
         AssertFatal(!mLoopingInactiveList.findImage(mHandle[best]), "cullSource: image already in inactive list");
         AssertFatal(!mLoopingCulledList.findImage(mHandle[best]), "cullSource: image should not be in culled list");
         mLoopingInactiveList.push_back(*itr);
      }
      else
      {
         (*itr)->mHandle |= AUDIOHANDLE_INACTIVE_BIT;
         AssertFatal(!mLoopingCulledList.findImage(mHandle[best]), "cullSource: image already in culled list");
         AssertFatal(!mLoopingInactiveList.findImage(mHandle[best]), "cullSource: image should no be in inactive list");
         (*itr)->mCullTime = Platform::getRealMilliseconds();
         mLoopingCulledList.push_back(*itr);
      }
   }

   alSourceStop(mSource[best]);
   mHandle[best] = NULL_AUDIOHANDLE;
   mBuffer[best] = 0;
   *index = best;

   return(true);
}

//--------------------------------------------------------------------------
// we want to compute approximate max volume at a particular distance
// ignore cone volume influnces
static F32 approximate3DVolume(const Audio::Description *desc, const Point3F &position)
{
   Point3F p1;
   alxGetListener3f(AL_POSITION, &p1.x, &p1.y, &p1.z);

   p1 -= position;
   F32 distance = p1.magnitudeSafe();

   if(distance >= desc->mMaxDistance)
      return(0.f);
   else if(distance > desc->mMinDistance)
      return(desc->mMinDistance / distance);
   else
      return 1.0f;
}   
   
//--------------------------------------------------------------------------
inline U32 alxFindIndex(AUDIOHANDLE handle)
{
   for (U32 i=0; i<MAX_AUDIOSOURCES; i++)
      if(mHandle[i] && areEqualHandles(mHandle[i], handle))
         return i;
   return MAX_AUDIOSOURCES;
}   

//--------------------------------------------------------------------------
ALuint alxFindSource(AUDIOHANDLE handle)
{
   for (U32 i=0; i<MAX_AUDIOSOURCES; i++)
      if(mHandle[i] && areEqualHandles(mHandle[i], handle))
         return mSource[i];
   return(INVALID_SOURCE);
} 


//--------------------------------------------------------------------------
// * a handle is valid if it is a currently playing source, inactive source,
//   or a looping source (basically anything where a alxSource??? call will succeed)
bool alxIsValidHandle(AUDIOHANDLE handle)
{
   if(handle == NULL_AUDIOHANDLE)
      return(false);

   // inactive sources are valid
   U32 idx = alxFindIndex(handle);
   if(idx != MAX_AUDIOSOURCES)
   {
      if(mHandle[idx] & AUDIOHANDLE_INACTIVE_BIT)
         return(true);
      
      // if it is active but not playing then it has stopped...
      ALint state = AL_STOPPED;
      alGetSourcei(mSource[idx], AL_SOURCE_STATE, &state);
      return(state == AL_PLAYING);
   }

   if(mLoopingList.findImage(handle))
      return(true);

   return(false);      
}

bool alxIsPlaying(AUDIOHANDLE handle)
{
   if(handle == NULL_AUDIOHANDLE)
      return(false);
      
   U32 idx = alxFindIndex(handle);
   if(idx == MAX_AUDIOSOURCES)
      return(false);

   ALint state = 0;
   alGetSourcei(mSource[idx], AL_SOURCE_STATE, &state);
   return(state == AL_PLAYING);
}

//--------------------------------------------------------------------------
void alxEnvironmentDestroy()
{
   if(mEnvironment)
   {
      alDeleteEnvironmentIASIG(1, &mEnvironment);
      mEnvironment = 0;
   }
}

void alxEnvironmentInit()
{
   if(!mInitialized)
      return;

   alxEnvironmentDestroy();
   if(alIsExtensionPresent((const ALubyte *)"AL_EXT_IASIG"))
   {
      alGenEnvironmentIASIG(1, &mEnvironment);
      if(alGetError() != AL_NO_ERROR)
         mEnvironment = 0;
   }
}

//--------------------------------------------------------------------------
// - setup a sources environmental effect settings
static void alxSourceEnvironment(ALuint source, F32 environmentLevel, AudioSampleEnvironment * env)
{
   // environment level is on the AudioDatablock
   alSourcef(source, AL_ENV_SAMPLE_REVERB_MIX_EXT, environmentLevel);

   if(!env)
      return;

   alSourcei(source, AL_ENV_SAMPLE_DIRECT_EXT,                 env->mDirect);
   alSourcei(source, AL_ENV_SAMPLE_DIRECT_HF_EXT,              env->mDirectHF);
   alSourcei(source, AL_ENV_SAMPLE_ROOM_EXT,                   env->mRoom);
   alSourcei(source, AL_ENV_SAMPLE_ROOM_HF_EXT,                env->mRoomHF);
   alSourcei(source, AL_ENV_SAMPLE_OUTSIDE_VOLUME_HF_EXT,      env->mOutsideVolumeHF);
   alSourcei(source, AL_ENV_SAMPLE_FLAGS_EXT,                  env->mFlags);

   alSourcef(source, AL_ENV_SAMPLE_OBSTRUCTION_EXT,            env->mObstruction);
   alSourcef(source, AL_ENV_SAMPLE_OBSTRUCTION_LF_RATIO_EXT,   env->mObstructionLFRatio);
   alSourcef(source, AL_ENV_SAMPLE_OCCLUSION_EXT,              env->mOcclusion);
   alSourcef(source, AL_ENV_SAMPLE_OCCLUSION_LF_RATIO_EXT,     env->mOcclusionLFRatio);
   alSourcef(source, AL_ENV_SAMPLE_OCCLUSION_ROOM_RATIO_EXT,   env->mOcclusionRoomRatio);
   alSourcef(source, AL_ENV_SAMPLE_ROOM_ROLLOFF_EXT,           env->mRoomRolloff);
   alSourcef(source, AL_ENV_SAMPLE_AIR_ABSORPTION_EXT,         env->mAirAbsorption);
}

static void alxSourceEnvironment(ALuint source, LoopingImage * image)
{
   AssertFatal(image, "alxSourceEnvironment: invalid looping image");
   if(image->mDescription.mIs3D)
      alxSourceEnvironment(source, image->mDescription.mEnvironmentLevel, image->mEnvironment);
}

//--------------------------------------------------------------------------
// setup a source to play... loopers have pitch/pan cached
// - by default, pitch is 1x and pan is center (settings not defined in description)
// - all the settings are cached by openAL (miles version), so no worries setting them here
static void alxSourcePlay(ALuint source, Resource<AudioBuffer> buffer, const Audio::Description *desc, const MatrixF *transform)
{
   alSourcei(source, AL_BUFFER, buffer->getALBuffer());
   alSourcef(source, AL_GAIN_LINEAR, desc->mVolume * mAudioTypeVolume[desc->mType] * mMasterVolume);
   alSourcei(source, AL_SOURCE_LOOPING, desc->mIsLooping ? AL_TRUE : AL_FALSE);
   alSourcef(source, AL_PITCH, 1.f);

   // stream sources are only setup for music streams (below...)
   alSourcei(source, AL_STREAMING, AL_FALSE);

   // 3d?
   if(transform != NULL)
   {
      alSourcei(source, AL_SOURCE_AMBIENT, AL_FALSE);
      alSourcei(source, AL_CONE_INNER_ANGLE, desc->mConeInsideAngle);
      alSourcei(source, AL_CONE_OUTER_ANGLE, desc->mConeOutsideAngle);
      alSourcef(source, AL_CONE_OUTER_GAIN, desc->mConeOutsideVolume);

      Point3F p;
      transform->getColumn(3, &p);
      alSource3f(source, AL_POSITION, p.x, p.y, p.z);

      transform->getRow(1, &p);  
      alSource3f(source, AL_DIRECTION, p.x, p.y, p.z);

      // forcing different falloffs?
      if(mDisableOuterFalloffs)
      {
         alSourcef(source, AL_MIN_DISTANCE, mInnerFalloffScale * desc->mMinDistance);
         alSourcef(source, AL_MAX_DISTANCE, FORCED_OUTER_FALLOFF);
      }
      else
      {
         alSourcef(source, AL_MIN_DISTANCE, desc->mMinDistance);
         alSourcef(source, AL_MAX_DISTANCE, desc->mMaxDistance);
      }

      // environmental audio stuff:
      alSourcef(source, AL_ENV_SAMPLE_REVERB_MIX_EXT, desc->mEnvironmentLevel);
      if(desc->mEnvironmentLevel != 0.f)
         alSourceResetEnvironment_EXT(source);
   }
   else  // 2d source (default the pan)
   {
      alSourcei(source, AL_SOURCE_AMBIENT, AL_TRUE);
      alSourcef(source, AL_PAN, 0.f);
   }
}   

// helper for looping images
static void alxSourcePlay(ALuint source, LoopingImage * image)
{
   AssertFatal(image, "alxSourcePlay: invalid looping image");

   // 3d source? need position/direction
   if(image->mDescription.mIs3D)
   {
      MatrixF transform(true);

      transform.setColumn(3, image->mPosition);
      transform.setRow(1, image->mDirection);

      alxSourcePlay(source, image->mBuffer, &image->mDescription, &transform);
   }
   else  // 2d source? 
   {
      alxSourcePlay(source, image->mBuffer, &image->mDescription, 0);

      // set the pan/pitch...
      alxSourcef(source, AL_PITCH, image->mPitch);   
      alxSourcef(source, AL_PAN, image->mPan);
   }
}

//--------------------------------------------------------------------------
AUDIOHANDLE alxCreateSource(const Audio::Description *desc, 
                            const char *filename, 
                            const MatrixF *transform, 
                            AudioSampleEnvironment *sampleEnvironment)
{
   if(desc == NULL || filename == NULL || *filename == '\0')
      return NULL_AUDIOHANDLE;

   F32 volume = desc->mVolume;

   // calculate an approximate attenuation for 3d sounds
   if(transform && desc->mIs3D)
   {
      Point3F position;
      transform->getColumn(3, &position);
      volume *= approximate3DVolume(desc, position);
   }

   // check the type specific volume
   AssertFatal(desc->mType < Audio::NumAudioTypes, "alxCreateSource: invalid type for source");
   if(desc->mType >= Audio::NumAudioTypes)
      return(NULL_AUDIOHANDLE);

   // done if channel is muted (and not a looper)
   if(!desc->mIsLooping && (mAudioTypeVolume[desc->mType] == 0.f))
      return(NULL_AUDIOHANDLE);

   // scale volume by channel attenuation
   volume *= mAudioTypeVolume[desc->mType];

   // non-loopers don't add if < minvolume
   if(!desc->mIsLooping && (volume <= MIN_GAIN))
      return(NULL_AUDIOHANDLE);

   U32 index = MAX_AUDIOSOURCES;

   // try and find an available source: 0 volume loopers get added to inactive list
   if(volume > MIN_GAIN)
   {
      if(!findFreeSource(&index))
      {
         alxUpdateScores(true);
         
         // scores do not include master volume
         if(!cullSource(&index, volume))
            index = MAX_AUDIOSOURCES;
      }
   }

   // make sure that loopers are added
   if(index == MAX_AUDIOSOURCES)
   {
      if(desc->mIsLooping)
      {
         Resource<AudioBuffer> buffer = AudioBuffer::find(filename);
         if(!(bool)buffer)
            return(NULL_AUDIOHANDLE);

         // create the inactive looping image
         LoopingImage * image = createLoopingImage();

         image->mHandle = getNewHandle() | AUDIOHANDLE_LOOPING_BIT | AUDIOHANDLE_INACTIVE_BIT;
         image->mBuffer = buffer;
         image->mDescription = *desc;
         image->mScore = volume;
         image->mEnvironment = sampleEnvironment;
         
         // grab position/direction if 3d source
         if(transform)
         {
            transform->getColumn(3, &image->mPosition);
            transform->getRow(1, &image->mDirection);
         }

         AssertFatal(!mLoopingInactiveList.findImage(image->mHandle), "alxCreateSource: handle in inactive list");
         AssertFatal(!mLoopingCulledList.findImage(image->mHandle), "alxCreateSource: handle in culled list");

         // add to the looping and inactive lists
         mLoopingList.push_back(image);
         mLoopingInactiveList.push_back(image);
         return(image->mHandle & RETURN_MASK);
      }
      else
         return(NULL_AUDIOHANDLE);
   }

   // clear the error state
   alGetError();     

   // grab the buffer
   Resource<AudioBuffer> buffer = AudioBuffer::find(filename);
   if((bool)buffer == false)
      return NULL_AUDIOHANDLE;

   // init the source (created inactive) and store needed values
   mHandle[index] = getNewHandle() | AUDIOHANDLE_INACTIVE_BIT;
   mType[index] = desc->mType;
   mBuffer[index] = buffer;
   mScore[index] = volume;
   mSourceVolume[index] = desc->mVolume;
   mSampleEnvironment[index] = sampleEnvironment;

   ALuint source = mSource[index];

   // setup play info
   alxSourcePlay(source, buffer, desc, desc->mIs3D ? transform : 0);
   if(mEnvironmentEnabled)
      alxSourceEnvironment(source, desc->mEnvironmentLevel, sampleEnvironment);

   // setup a LoopingImage ONLY if the sound is a looper: 
   if(desc->mIsLooping)
   {
      mHandle[index] |= AUDIOHANDLE_LOOPING_BIT;

      LoopingImage * image = createLoopingImage();
      image->mHandle = mHandle[index];
      image->mBuffer = buffer;
      image->mDescription = *desc;
      image->mScore = volume;
      image->mEnvironment = sampleEnvironment;

      // grab position/direction
      if(transform)
      {
         transform->getColumn(3, &image->mPosition);
         transform->getRow(1, &image->mDirection);
      }

      AssertFatal(!mLoopingInactiveList.findImage(image->mHandle), "alxCreateSource: handle in inactive list");
      AssertFatal(!mLoopingCulledList.findImage(image->mHandle), "alxCreateSource: handle in culled list");
      
      // add to the looping list
      mLoopingList.push_back(image);
   }

   // clear off all but looping bit
   return(mHandle[index] & RETURN_MASK);
}

//------------------------------------------------------------------------------
AUDIOHANDLE alxCreateSource(AudioDescription *descObject,
                            const char *filename,
                            const MatrixF *transform,
                            AudioSampleEnvironment * sampleEnvironment )
{
   if(!descObject || !descObject->getDescription())
      return(NULL_AUDIOHANDLE);
   return (alxCreateSource(descObject->getDescription(), filename, transform, sampleEnvironment));
}

AUDIOHANDLE alxCreateSource(const AudioProfile *profile, const MatrixF *transform)
{
   if (profile == NULL)
      return NULL_AUDIOHANDLE;

   return alxCreateSource(profile->mDescriptionObject, profile->mFilename, transform, profile->mSampleEnvironment);
}   

//--------------------------------------------------------------------------
extern void threadPlay(AudioBuffer * buffer, AUDIOHANDLE handle);

AUDIOHANDLE alxPlay(AUDIOHANDLE handle)
{
   U32 index = alxFindIndex(handle);

   if(index != MAX_AUDIOSOURCES)
   {
      // play if not already playing
      if(mHandle[index] & AUDIOHANDLE_INACTIVE_BIT)
      {
      // jff: thread stuff
//         // have the thread play this once the buffer is loaded
//         if(bool(mBuffer[index]) && mBuffer[index]->isLoading())
//         {
//            // move loopers into the inactive list
//            LoopingList::iterator itr = mLoopingList.findImage(handle);
//            if(itr)
//            {
//               mHandle[index] = NULL_AUDIOHANDLE;
//               mBuffer[index] = 0;
//
//               (*itr)->mHandle |= AUDIOHANDLE_LOADING_BIT;
//               mLoopingInactiveList.push_back(*itr);
//            }
//
//            mHandle[index] |= AUDIOHANDLE_LOADING_BIT;
//
//            if(gAudioThread)
//               gAudioThread->setBufferPlayHandle(mBuffer[index], handle);
//            return(handle);
//         }

         mHandle[index] &= ~(AUDIOHANDLE_INACTIVE_BIT | AUDIOHANDLE_LOADING_BIT);

         // make sure the looping image also clears it's inactive bit
         LoopingList::iterator itr = mLoopingList.findImage(handle);
         if(itr)
            (*itr)->mHandle &= ~(AUDIOHANDLE_INACTIVE_BIT | AUDIOHANDLE_LOADING_BIT);

         alSourcePlay(mSource[index]);

         return(handle);
      }
   }
   else
   {
      // move inactive loopers to the culled list, try to start the sound
      LoopingList::iterator itr = mLoopingInactiveList.findImage(handle);
      if(itr)
      {
         AssertFatal(!mLoopingCulledList.findImage(handle), "alxPlay: image already in culled list");
         mLoopingCulledList.push_back(*itr);
         mLoopingInactiveList.erase_fast(itr);
         alxLoopingUpdate();
      }
      else if(mLoopingCulledList.findImage(handle))
      {
         alxLoopingUpdate();
      }
      else
         return(NULL_AUDIOHANDLE);   
   }

   return(handle);
}   

//--------------------------------------------------------------------------
// helper function.. create a source and play it
AUDIOHANDLE alxPlay(const AudioProfile *profile, const MatrixF *transform, const Point3F* /*velocity*/)
{
   if(profile == NULL)
      return NULL_AUDIOHANDLE;

   AUDIOHANDLE handle = alxCreateSource(profile->mDescriptionObject, profile->mFilename, transform, profile->mSampleEnvironment);
   if(handle != NULL_AUDIOHANDLE)
      return(alxPlay(handle));
   return(handle);
}   

//--------------------------------------------------------------------------
void alxStop(AUDIOHANDLE handle)
{
   U32 index = alxFindIndex(handle);

   // stop it
   if(index != MAX_AUDIOSOURCES)
   {
// jff: will have problems if buffer still loading
      if(!(mHandle[index] & AUDIOHANDLE_INACTIVE_BIT))
      {
         alSourceStop(mSource[index]);
      }

      mSampleEnvironment[index] = 0;
      mHandle[index] = NULL_AUDIOHANDLE;
      mBuffer[index] = 0;
   }

   // remove loopingImage and add it to the free list
   LoopingList::iterator itr = mLoopingList.findImage(handle);
   if(itr)
   {
      // remove from inactive/culled list
      if((*itr)->mHandle & AUDIOHANDLE_INACTIVE_BIT)
      {
         LoopingList::iterator tmp = mLoopingInactiveList.findImage(handle);
         
         // inactive?
         if(tmp)
            mLoopingInactiveList.erase_fast(tmp);
         else
         {
            //culled?
            tmp = mLoopingCulledList.findImage(handle);
            AssertFatal(tmp, "alxStop: failed to find inactive looping source");
            mLoopingCulledList.erase_fast(tmp);
         }
      }

      AssertFatal(!mLoopingInactiveList.findImage((*itr)->mHandle), "alxStop: handle in inactive list");
      AssertFatal(!mLoopingCulledList.findImage((*itr)->mHandle), "alxStop: handle in culled list");
      
      // remove it
      (*itr)->clear();
      mLoopingFreeList.push_back(*itr);
      mLoopingList.erase_fast(itr);
   }
}   
                           
//--------------------------------------------------------------------------
void alxStopAll()
{
   // stop all open sources
   for(S32 i = mNumSources - 1; i >= 0; i--)
      if(mHandle[i] != NULL_AUDIOHANDLE)
         alxStop(mHandle[i]);

   // stop all looping sources
   while(mLoopingList.size())
      alxStop(mLoopingList.last()->mHandle);
}

void alxLoopSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat value) 
{
   LoopingList::iterator itr = mLoopingList.findImage(handle);
   if(itr)
   {
      switch(pname)
      {
         case AL_PAN:
            (*itr)->mPan = value;
            break;
         case AL_GAIN:
            (*itr)->mDescription.mVolume = Audio::DBToLinear(value);
            break;
         case AL_GAIN_LINEAR:
            (*itr)->mDescription.mVolume = value;
            break;
         case AL_PITCH:
            (*itr)->mPitch = value;
            break;
         case AL_MIN_DISTANCE:
            (*itr)->mDescription.mMinDistance = value;
            break;
         case AL_CONE_OUTER_GAIN:
            (*itr)->mDescription.mMaxDistance = value;
            break;
      }
   }
}
                            
void alxLoopSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3) 
{
   LoopingList::iterator itr = mLoopingList.findImage(handle);
   if(itr)
   { 
      switch(pname)
      {
         case AL_POSITION:
            (*itr)->mPosition.x = value1;
            (*itr)->mPosition.y = value2;
            (*itr)->mPosition.z = value3;
            break;

         case AL_DIRECTION:
            (*itr)->mDirection.x = value1;
            (*itr)->mDirection.y = value2;
            (*itr)->mDirection.z = value3;
            break;
      }
   }
}

void alxLoopSourcei(AUDIOHANDLE handle, ALenum pname, ALint value) 
{
   LoopingList::iterator itr = mLoopingList.findImage(handle);
   if(itr)
   { 
      switch(pname)
      {
         case AL_SOURCE_AMBIENT:
            (*itr)->mDescription.mIs3D = value;
            break;
         case AL_CONE_INNER_ANGLE:
            (*itr)->mDescription.mConeInsideAngle = value;
            break;
         case AL_CONE_OUTER_ANGLE:
            (*itr)->mDescription.mConeOutsideAngle = value;
            break;
      }
   }
}

void alxLoopGetSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat *value) 
{
   LoopingList::iterator itr = mLoopingList.findImage(handle);
   if(itr)
   { 
      switch(pname)
      {
         case AL_PAN:
            *value = (*itr)->mPan;
            break;
         case AL_GAIN:
            *value = Audio::linearToDB((*itr)->mDescription.mVolume);
            break;
         case AL_GAIN_LINEAR:
            *value = (*itr)->mDescription.mVolume;
            break;
         case AL_PITCH:
            *value = (*itr)->mPitch;
            break;
         case AL_MIN_DISTANCE:
            *value = (*itr)->mDescription.mMinDistance;
            break;
         case AL_CONE_OUTER_GAIN:
            *value = (*itr)->mDescription.mMaxDistance;
            break;
      }
   }
}

void alxLoopGetSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3) 
{
   LoopingList::iterator itr = mLoopingList.findImage(handle);
   if(itr)
   { 
      switch(pname)
      {
         case AL_POSITION:
            *value1 = (*itr)->mPosition.x;
            *value2 = (*itr)->mPosition.y;
            *value3 = (*itr)->mPosition.z;
            break;

         case AL_DIRECTION:
            *value1 = (*itr)->mDirection.x;
            *value2 = (*itr)->mDirection.y;
            *value3 = (*itr)->mDirection.z;
            break;
      }
   }
}

void alxLoopGetSourcei(AUDIOHANDLE handle, ALenum pname, ALint *value) 
{
   LoopingList::iterator itr = mLoopingList.findImage(handle);
   if(itr)
   { 
      switch(pname)
      {
         case AL_SOURCE_AMBIENT:
            *value = (*itr)->mDescription.mIs3D;
            break;
         case AL_SOURCE_LOOPING:
            *value = true;
            break;
         case AL_CONE_INNER_ANGLE:
            *value = (*itr)->mDescription.mConeInsideAngle;
            break;
         case AL_CONE_OUTER_ANGLE:
            *value = (*itr)->mDescription.mConeOutsideAngle;
            break;
      }
   }
}

//--------------------------------------------------------------------------
// AL get/set methods: Source
//--------------------------------------------------------------------------
// - only need to worry about playing sources.. proper volume gets set on
//   create source (so, could get out of sync if someone changes volume between
//   a createSource and playSource call...)
void alxUpdateTypeGain(U32 typeMask)
{
   for(U32 i = 0; i < MAX_AUDIOSOURCES; i++)
   {
      if(mHandle[i] == NULL_AUDIOHANDLE)
         continue;
      
      if(!(typeMask & (1 << mType[i])))
         continue;

      ALint state = AL_STOPPED;
      alGetSourcei(mSource[i], AL_SOURCE_STATE, &state);

      if(state == AL_PLAYING)
      {
         // volume = SourceVolume * ChannelVolume * MasterVolume
         F32 vol = mSourceVolume[i] * mAudioTypeVolume[mType[i]] * mMasterVolume;
         alSourcef(mSource[i], AL_GAIN_LINEAR, mClampF(vol, 0.f, 1.f));
      }
   }
}

void alxSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat value)
{
   ALuint source = alxFindSource(handle);

   if(source != INVALID_SOURCE)
   {
      // ensure gain_linear
      if(pname == AL_GAIN)
      {
         value = Audio::DBToLinear(value);
         pname = AL_GAIN_LINEAR;
      }
   
      // need to process gain settings (so source can be affected by channel/master gains)
      if(pname == AL_GAIN_LINEAR)
      {
         U32 idx = alxFindIndex(handle);
         AssertFatal(idx != MAX_AUDIOSOURCES, "alxSourcef: handle not located for found source");
         if(idx == MAX_AUDIOSOURCES)
            return;

         // update the stored value         
         mSourceVolume[idx] = value;

         // volume = SourceVolume * ChannelVolume * MasterVolume
         F32 vol = mSourceVolume[idx] * mAudioTypeVolume[mType[idx]] * mMasterVolume;
         alSourcef(source, AL_GAIN_LINEAR, mClampF(vol, 0.f, 1.f));
      }
      else
         alSourcef(source, pname, value);
   }
   alxLoopSourcef(handle, pname, value);
}

void alxSourcefv(AUDIOHANDLE handle, ALenum pname, ALfloat *values)
{
   ALuint source = alxFindSource(handle);
   if(source != INVALID_SOURCE) 
      alSourcefv(source, pname, values);
   
   if((pname == AL_POSITION) || (pname == AL_DIRECTION) || (pname == AL_VELOCITY))
      alxLoopSource3f(handle, pname, values[0], values[1], values[2]);
}

void alxSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3)
{
   ALuint source = alxFindSource(handle);
   if(source != INVALID_SOURCE)
   {
      ALfloat values[3];
      values[0] = value1;
      values[1] = value2;
      values[2] = value3;
      alSourcefv(source, pname, values);
   }
   alxLoopSource3f(handle, pname, value1, value2, value3);
}

void alxSourcei(AUDIOHANDLE handle, ALenum pname, ALint value)
{
   ALuint source = alxFindSource(handle);
   if(source != INVALID_SOURCE)
      alSourcei(source, pname, value);
   alxLoopSourcei(handle, pname, value);
}   

// sets the position and direction of the source
void alxSourceMatrixF(AUDIOHANDLE handle, const MatrixF *transform)
{
   ALuint source = alxFindSource(handle);

   Point3F pos;
   transform->getColumn(3, &pos);

   Point3F dir;
   transform->getRow(1, &dir);

   if(source != INVALID_SOURCE)
   {
      alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
      alSource3f(source, AL_DIRECTION, dir.x, dir.y, dir.z);
   }
   
   alxLoopSource3f(handle, AL_POSITION, pos.x, pos.y, pos.z);
   alxLoopSource3f(handle, AL_DIRECTION, dir.x, dir.y, dir.z);
}

//--------------------------------------------------------------------------
void alxGetSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat *value)
{
   ALuint source = alxFindSource(handle);
   if(source != INVALID_SOURCE)
   {
      // gain queries return unattenuated values
      if((pname == AL_GAIN) || (pname == AL_GAIN_LINEAR))
      {
         U32 idx = alxFindIndex(handle);
         AssertFatal(idx != MAX_AUDIOSOURCES, "alxGetSourcef: found source but handle is invalid");
         if(idx == MAX_AUDIOSOURCES)
         {
            *value = 0.f;
            return;
         }

         if(pname == AL_GAIN)
            *value = Audio::linearToDB(mSourceVolume[idx]);
         else
            *value = mSourceVolume[idx];
      }
      else
         alGetSourcef(source, pname, value);
   }
   else
      alxLoopGetSourcef(handle, pname, value);
}   

void alxGetSourcefv(AUDIOHANDLE handle, ALenum pname, ALfloat *values)
{
   if((pname == AL_POSITION) || (pname == AL_DIRECTION) || (pname == AL_VELOCITY))
      alxGetSource3f(handle, pname, &values[0], &values[1], &values[2]);
}

void alxGetSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
   ALuint source = alxFindSource(handle);
   if(source != INVALID_SOURCE)
   {
      ALfloat values[3];
      alGetSourcefv(source, pname, values);
      *value1 = values[0];
      *value2 = values[1];
      *value3 = values[2];
   }
   else
      alxLoopGetSource3f(handle, pname, value1, value2, value3);
}   

void alxGetSourcei(AUDIOHANDLE handle, ALenum pname, ALint *value)
{
   ALuint source = alxFindSource(handle);
   if(source != INVALID_SOURCE)
      alGetSourcei(source, pname, value);
   else
      alxLoopGetSourcei(handle, pname, value);
}   

//--------------------------------------------------------------------------
// AL get/set methods: Listener
//--------------------------------------------------------------------------
void alxListenerf(ALenum pname, ALfloat value)
{
#ifdef HANDLE_LISTENER_GAIN
   // listener gain is handled through wrapper
   if((pname == AL_GAIN) || (pname == AL_GAIN_LINEAR))
   {
      // just handles al_gain_linear
      if(pname == AL_GAIN)
         value = Audio::DBToLinear(value);

      mMasterVolume = mClampF(value, 0.f, 1.f);

      // update all the sources
      alxUpdateTypeGain(0xffffffff);
   }
   else
      alListenerf(pname, value);
#else
   alListenerf(pname, value);
#endif
}   

void alxListenerfv(ALenum pname, ALfloat * values)
{
   alListenerfv(pname, values);
}

void alxListener3f(ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3)
{
   alListener3f(pname, value1, value2, value3);
}

//--------------------------------------------------------------------------
// set the listener's position and orientation
void alxListenerMatrixF(const MatrixF *transform)
{
   Point3F p1, p2;
   transform->getColumn(3, &p1);
   alListener3f(AL_POSITION, p1.x, p1.y, p1.z);
   
   transform->getColumn(2, &p1);  // Up Vector
   transform->getColumn(1, &p2);  // Forward Vector
   F32 orientation[6];
   orientation[0] = p1.x;
   orientation[1] = p1.y;
   orientation[2] = p1.z;
   orientation[3] = p2.x;
   orientation[4] = p2.y;
   orientation[5] = p2.z;
   alListenerfv(AL_ORIENTATION, orientation);
}   

//--------------------------------------------------------------------------
void alxGetListenerf(ALenum pname, ALfloat *value)
{
#ifdef HANDLE_LISTENER_GAIN
   // listener gain is handled through wrapper
   if(pname == AL_GAIN)
      *value = Audio::linearToDB(mMasterVolume);
   else if(pname == AL_GAIN_LINEAR)
      *value = mMasterVolume;
   else
      alGetListenerf(pname, value);
#else
   alGetListenerf(pname, value);
#endif
}

void alxGetListenerfv(ALenum pname, ALfloat *values)
{
   alGetListenerfv(pname, values);
}

void alxGetListener3f(ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
   ALfloat val[3];
   alGetListenerfv(pname, val);
   *value1 = val[0];
   *value2 = val[1];
   *value3 = val[2];
}

void alxGetListeneri(ALenum pname, ALint *value)
{
   alGetListeneri(pname, value);
}

//--------------------------------------------------------------------------
// Capture methods
//--------------------------------------------------------------------------
void alxCaptureDestroy()
{
   if(mCaptureInitialized)
      alCaptureDestroy_EXT();

   mVoiceEncoderStream.setCodec(AUDIO_CODEC_NONE);
   AudioCodecManager::destroy();

   mCaptureInitialized = false;
}   

bool alxCaptureInit()
{
   alxCaptureDestroy();

   mCaptureInitialized = alCaptureInit_EXT(RECORD_FORMAT, RECORD_SPEED, RECORD_BUFFERSIZE);
   if(!mCaptureInitialized)
      return(false);

   // 0: <.v12>  1: <.v24>  2: <.v29>  3: <gsm>
   #ifdef USE_GSM_CODEC
      S32 encodingId = Con::getIntVariable("$pref::Audio::encodingLevel", AUDIO_CODEC_GSM);
      S32 decodingMask = Con::getIntVariable("$pref::Audio::decodingMask", 1 << AUDIO_CODEC_GSM)
   #else
      S32 encodingId = Con::getIntVariable("$pref::Audio::encodingLevel", AUDIO_CODEC_V12);
      S32 decodingMask = Con::getIntVariable("$pref::Audio::decodingMask", 1 << AUDIO_CODEC_V12);
   #endif
   
   // bring up the encoder (decoders instantiated when needed..)
   if(!mVoiceEncoderStream.setCodec(encodingId) || !mVoiceEncoderStream.open())
      return(false);

   return(true);
}       

//--------------------------------------------------------------------------
void alxCaptureStart(bool local)
{
   if(!mCaptureInitialized || mRecording)
      return;

   mLocalCapture = local;
   if(mLocalCapture)
   {
      mLocalCaptureBuffer = (U8 *)dMalloc(LOCAL_CAPTURE_SIZE);
      mLocalCaptureBufferPos = 0;
      Con::executef(2, "localCaptureStart", "record");
   }
   else
   {
      GameConnection* connection = GameConnection::getServerConnection();
      if(!connection)
      {
         Con::errorf(ConsoleLogEntry::General, "alxCaptureStart: no server connection");      
         return;
      }
      mVoiceEncoderStream.setConnection(connection);
   }
   sCaptureTimeout = Platform::getRealMilliseconds();

   alCaptureStart_EXT();
   mRecording = true;
}   

void scaleSamples(void * data, U32 size, U32 format, F32 scale)
{
   if((format != AL_FORMAT_MONO16) || (scale == 1.f))
      return;
      
   S16 * pData = (S16 *)data;
   U32 samples = size >> 1;
   
   for(U32 i = 0; i < samples; i++)
   {
      S32 samp = pData[i];
      samp *= scale;
      pData[i] = mClamp(samp, S16_MIN, S16_MAX);
   }
}

static void alxCaptureSend(bool flush = false)
{
   U32 size;
   U8 buffer[CAPTURE_BUFFER_SIZE];
   do {
      size = alCaptureGetData_EXT(buffer, CAPTURE_BUFFER_SIZE, RECORD_FORMAT, RECORD_SPEED);
      scaleSamples(buffer, size, RECORD_FORMAT, mCaptureGainScale);
      mVoiceEncoderStream.setBuffer(buffer, size);
      mVoiceEncoderStream.process();
   } while ((flush && size) || (size == CAPTURE_BUFFER_SIZE));
}   

void alxBufferCapture()
{
   S32 len = LOCAL_CAPTURE_SIZE - mLocalCaptureBufferPos;
   if(len > 0)
   {
      U32 size = alCaptureGetData_EXT(mLocalCaptureBuffer + mLocalCaptureBufferPos, len, RECORD_FORMAT, RECORD_SPEED);
      scaleSamples(mLocalCaptureBuffer + mLocalCaptureBufferPos, size, RECORD_FORMAT, mCaptureGainScale);
      mLocalCaptureBufferPos += size;
   }
}

// called from Miles thread... must process in main thread
void localCaptureFinishedCallback(U32, bool)
{
   mLocalCaptureFinished = true;
}

void alxCaptureStop()
{
   if(!mCaptureInitialized || !mRecording)
      return;

   mRecording = false;
   sCaptureTimeout = 0;
   alCaptureStop_EXT();

   if(mLocalCapture && mLocalCaptureBuffer)
   {
      Con::executef(2, "localCaptureStop", "record");

      alGenBuffers(1, &mALCaptureBuffer);
      if(alGetError() != AL_NO_ERROR)
         return;

      alBufferData(mALCaptureBuffer, RECORD_FORMAT, mLocalCaptureBuffer, mLocalCaptureBufferPos, RECORD_SPEED);
      dFree(mLocalCaptureBuffer);
      mLocalCaptureBuffer = 0;

      U32 index;
      if(findFreeSource(&index))
      {
         mHandle[index] = getNewHandle() | AUDIOHANDLE_INACTIVE_BIT;
         mType[index] = Audio::DefaultAudioType;
         ALuint source = mSource[index];

         alSourcei(source, AL_SOURCE_AMBIENT, AL_TRUE);
         alSourcei(source, AL_SOURCE_LOOPING, AL_FALSE);
         alSourcei(source, AL_BUFFER, mALCaptureBuffer);
         alSourcef(source, AL_GAIN_LINEAR, mAudioTypeVolume[Audio::DefaultAudioType]);
         
         alSourceCallback_EXT(source, localCaptureFinishedCallback);
         alxPlay(mHandle[index]);
         Con::executef(2, "localCaptureStart", "play");
      }
      else
         alDeleteBuffers(1, &mALCaptureBuffer);
   }
   else
   {
      alxCaptureSend(true);  
      mVoiceEncoderStream.flush();
   }
}   

bool alxIsCapturing()
{
   return(mRecording);
}

void alxCaptureUpdate()
{
   // check if should destroy local capture buffer
   if(mLocalCaptureFinished)
   {
      alDeleteBuffers(1, &mALCaptureBuffer);
      Con::executef(2, "localCaptureStop", "play");
      mLocalCaptureFinished = false;
   }

   // check if need to stop capturing
   if(mCaptureInitialized && sCaptureTimeout != 0)
   {
      if((Platform::getRealMilliseconds()-sCaptureTimeout) > RECORD_LEN)
         alxCaptureStop();
      else
      {
         if(mLocalCapture)
            alxBufferCapture();
         else
            alxCaptureSend();
      }
   }
}   

//--------------------------------------------------------------------------
// Voice streams
//--------------------------------------------------------------------------
Vector<VoiceStream*>::iterator alxFindStream(U8 codecId, U32 clientId, U8 streamId)
{
   Vector<VoiceStream*>::iterator itr = sVoiceStreams.begin();
   for (; itr != sVoiceStreams.end(); itr++)
   {
      VoiceStream *vs = *itr;
      if (vs->mCodecId == codecId && vs->mClientId == clientId && vs->mStreamId == streamId)
         return itr;
   }
   return NULL;
}   

Vector<VoiceStream*>::iterator alxNewStream(U8 codecId, U32 clientId, U8 streamId)
{
   if(codecId >= AUDIO_NUM_CODECS)
      return(0);

   // need to get a new stream
   VoiceStream *vs;
   if (sVoiceStreams_free.size())
   {
      vs = sVoiceStreams_free.last();
      sVoiceStreams_free.pop_back();
   }
   else
      vs = new VoiceStream;

   vs->mCodecId = codecId;
   vs->mClientId = clientId;
   vs->mStreamId = streamId;

   // open the codec
   if(!vs->mDecoderStream.setCodec(codecId) || !vs->mDecoderStream.open())
   {
      delete vs;
      return(0);
   }

   sVoiceStreams.push_back(vs);
   return &sVoiceStreams.last();   
}   

void alxPlayStream(U8 codecId, U32 clientId, U8 streamId)
{
   Vector<VoiceStream*>::iterator itr = alxFindStream(codecId, clientId, streamId);
   if (itr == NULL)
      return;

   Con::evaluatef("clientCmdPlayerStartTalking(%d, 1);", clientId);
   VoiceStream *vs = *itr;
      
   alGenBuffers(1, &vs->mBuffer);
   vs->mDecoderStream.process();
   U8 *data;
   U32 size;
   vs->mDecoderStream.getBuffer(&data, &size);
   if(size)   
   {
      alBufferData(vs->mBuffer, RECORD_FORMAT, data, size, RECORD_SPEED);
   
      // find an available source, if none then (hopefully) force a cull
      U32 index;
      if(!findFreeSource(&index))
         if(!cullSource(&index, 2.0f))
            return;

      // clear the error state
      alGetError();     

      // init and play the source
      mHandle[index] = getNewHandle() | AUDIOHANDLE_VOICE_BIT;
      mSourceVolume[index] = mAudioTypeVolume[Audio::VoiceAudioType];
      vs->mHandle    = mHandle[index];
      ALuint source  = mSource[index];

      alSourcei(source, AL_BUFFER, vs->mBuffer);    
      alSourcei(source, AL_SOURCE_LOOPING, AL_FALSE);
      alSourcei(source, AL_SOURCE_AMBIENT, AL_TRUE);
      alSourcef(source, AL_GAIN_LINEAR, mSourceVolume[index] * mMasterVolume);
      alSourcePlay(source);
   }
   else
      Con::printf("Decode size is ZERO!  client %d", clientId);
   
   vs->mDecoderStream.close();
}   

//--------------------------------------------------------------------------
void alxReceiveVoiceStream(SimVoiceStreamEvent *event)
{
   Vector<VoiceStream*>::iterator itr;
   if (event->mSequence == 0)
      itr = alxNewStream(event->mCodecId, event->mClientId, event->mStreamId);
   else
      itr = alxFindStream(event->mCodecId, event->mClientId, event->mStreamId);

   if (itr) 
   {
      if (event->getSize())  
         (*itr)->mDecoderStream.setBuffer(event->getData(), event->getSize());

      if (event->getSize() < SimVoiceStreamEvent::VOICE_PACKET_DATA_SIZE)
         alxPlayStream(event->mCodecId, event->mClientId, event->mStreamId);
   }
}   

// Music stream: -----------------------------------------------------------
// called from Miles thread... must process in main thread
void streamCallback(U32, bool stopped)
{
   mFinishedMusicStream = true;
   mFinishedMusicStreamStopState = stopped;
}

void alxPlayMusicStream(const char * filename)
{
   if(!alIsBuffer(mStreamBuffer))
   {
      alGenBuffers(1, &mStreamBuffer);
      if(alGetError() != AL_NO_ERROR)
         return;

      // streamed buffers should not be managed
      alBufferi_EXT(mStreamBuffer, AL_BUFFER_KEEP_RESIDENT, AL_TRUE);
   }

   alxStop(mStreamHandle);

   if(!alBufferStreamFile_EXT(mStreamBuffer, (const ALubyte *)filename))
   {
      const char * error = (const char *)alGetString(alGetError());
      Con::errorf(ConsoleLogEntry::General, "alxStartStream: %s", error);
   }

   U32 index;
   if(findFreeSource(&index))
   {
      mHandle[index] = getNewHandle() | AUDIOHANDLE_INACTIVE_BIT;
      mType[index] = Audio::MusicAudioType;
      mSourceVolume[index] = mAudioTypeVolume[Audio::MusicAudioType];

      ALuint source = mSource[index];
      alSourcei(source, AL_SOURCE_AMBIENT, AL_TRUE);
      alSourcei(source, AL_STREAMING, AL_TRUE);
      alSourcei(source, AL_BUFFER, mStreamBuffer);
      alSourcef(source, AL_GAIN_LINEAR, mSourceVolume[index] * mMasterVolume);

      alSourceCallback_EXT(source, streamCallback);
      alxPlay(mHandle[index]);
      mStreamHandle = mHandle[index];
   }
   else
   {
      Con::errorf(ConsoleLogEntry::General, "alxStartStream: no free sources");
      alDeleteBuffers(1, &mStreamBuffer);
   }
}

void alxStopMusicStream()
{
   alxStop(mStreamHandle);
   if(alIsBuffer(mStreamBuffer))
      alDeleteBuffers(1, &mStreamBuffer);
}

// checks if the music stream is done and calls into console..
void alxStreamUpdate()
{
   if(mFinishedMusicStream)
   {
      Con::executef(2, "finishedMusicStream", mFinishedMusicStreamStopState ? "true" : "false");
      mFinishedMusicStream = false;
   }
}
   
//--------------------------------------------------------------------------
// This is a somewhat crappy way of doing this, but under win32 the outer falloff
// is not always the distance at which the gain is clamped to 0 (some drivers
// ignore this setting on the dsound 3dbuffer). So, if this is enabled then
// the outer falloffs of all sources is pushed out to MAX_FALLOFF and the inner ones
// are scaled by the inner falloff scale (some hardware drivers attenuate differently)
// - sources are scaled on the play call as well
// - will trash any non-default falloffs
void alxDisableOuterFalloffs(bool disable)
{
   if(disable == mDisableOuterFalloffs)
      return;

   // update all the playing sources (non-loopers cannot be reset after this)
   for(U32 i = 0; i < MAX_AUDIOSOURCES; i++)
   {
      if(alxIsValidHandle(mHandle[i]))
      {
         // only care about 3d sources...
         ALint ambient = AL_FALSE;
         alxGetSourcei(mHandle[i], AL_SOURCE_AMBIENT, &ambient);
         if(ambient == AL_FALSE)
            continue;
            
         if(disable)
         {
            ALfloat min = 1.f;
            alGetSourcef(mSource[i], AL_MIN_DISTANCE, &min);
            alSourcef(mSource[i], AL_MIN_DISTANCE, min * mInnerFalloffScale);
            alSourcef(mSource[i], AL_MAX_DISTANCE, FORCED_OUTER_FALLOFF);
         }
         else
         {
            LoopingList::iterator itr = mLoopingList.findImage(mHandle[i]);
            if(itr)
            {
               alSourcef(mSource[i], AL_MIN_DISTANCE, (*itr)->mDescription.mMinDistance);
               alSourcef(mSource[i], AL_MAX_DISTANCE, (*itr)->mDescription.mMaxDistance);
            }
         }
      }
   }

   mDisableOuterFalloffs = disable;
}

void alxSetInnerFalloffScale(F32 scale)
{
   mInnerFalloffScale = mClampF(scale, 0.1f, 10.f);
}

F32 alxGetInnerFalloffScale()
{
   return(mInnerFalloffScale);
}

//--------------------------------------------------------------------------
// Simple metrics
//--------------------------------------------------------------------------

#ifdef GATHER_METRICS
static void alxGatherMetrics()
{
   S32 mNumOpenHandles              = 0;
   S32 mNumOpenLoopingHandles       = 0;
   S32 mNumOpenVoiceHandles         = 0;

   S32 mNumActiveStreams            = 0;
   S32 mNumNullActiveStreams        = 0;
   S32 mNumActiveLoopingStreams     = 0;
   S32 mNumActiveVoiceStreams       = 0;

   S32 mNumLoopingStreams           = 0;
   S32 mNumInactiveLoopingStreams   = 0;
   S32 mNumCulledLoopingStreams     = 0;

   S32 mDynamicMemUsage             = 0;
   S32 mDynamicMemSize              = 0;
   S32 mMemUsage                    = 0;
   S32 mBufferCount                 = 0;
   S32 mDynamicBufferCount          = 0;

   // count installed streams and open handles
   for(U32 i = 0; i < mNumSources; i++)
   {
      if(mHandle[i] != NULL_AUDIOHANDLE)
      {
         mNumOpenHandles++;
         if(mHandle[i] & AUDIOHANDLE_LOOPING_BIT)
            mNumOpenLoopingHandles++;
         if(mHandle[i] & AUDIOHANDLE_VOICE_BIT)
            mNumOpenVoiceHandles++;
      }

      ALint state = AL_STOPPED;
      alGetSourcei(mSource[i], AL_SOURCE_STATE, &state);
      if(state == AL_PLAYING)
      {
         mNumActiveStreams++;
         if(mHandle[i] == NULL_AUDIOHANDLE)
            mNumNullActiveStreams++;
         if(mHandle[i] & AUDIOHANDLE_LOOPING_BIT)
            mNumActiveLoopingStreams++;
         if(mHandle[i] & AUDIOHANDLE_VOICE_BIT)
            mNumActiveVoiceStreams++;   
      }
   }

   for(LoopingList::iterator itr = mLoopingList.begin(); itr != mLoopingList.end(); itr++)
      mNumLoopingStreams++;
   for(LoopingList::iterator itr = mLoopingInactiveList.begin(); itr != mLoopingInactiveList.end(); itr++)
      mNumInactiveLoopingStreams++;
   for(LoopingList::iterator itr = mLoopingCulledList.begin(); itr != mLoopingCulledList.end(); itr++)
      mNumCulledLoopingStreams++;

   alGetContexti_EXT(ALC_BUFFER_MEMORY_USAGE, &mMemUsage);
   alGetContexti_EXT(ALC_BUFFER_DYNAMIC_MEMORY_SIZE, &mDynamicMemSize);
   alGetContexti_EXT(ALC_BUFFER_DYNAMIC_MEMORY_USAGE, &mDynamicMemUsage);
   alGetContexti_EXT(ALC_BUFFER_COUNT, &mBufferCount);
   alGetContexti_EXT(ALC_BUFFER_DYNAMIC_COUNT, &mDynamicBufferCount);
   
   Con::setIntVariable("Audio::numOpenHandles",             mNumOpenHandles);
   Con::setIntVariable("Audio::numOpenLoopingHandles",      mNumOpenLoopingHandles);
   Con::setIntVariable("Audio::numOpenVoiceHandles",        mNumOpenVoiceHandles);

   Con::setIntVariable("Audio::numActiveStreams",           mNumActiveStreams);
   Con::setIntVariable("Audio::numNullActiveStreams",       mNumNullActiveStreams);
   Con::setIntVariable("Audio::numActiveLoopingStreams",    mNumActiveLoopingStreams);
   Con::setIntVariable("Audio::numActiveVoiceStreams",      mNumActiveVoiceStreams);

   Con::setIntVariable("Audio::numLoopingStreams",          mNumLoopingStreams);
   Con::setIntVariable("Audio::numInactiveLoopingStreams",  mNumInactiveLoopingStreams);
   Con::setIntVariable("Audio::numCulledLoopingStreams",    mNumCulledLoopingStreams);
   
   Con::setIntVariable("Audio::memUsage",                   mMemUsage >> 10);
   Con::setIntVariable("Audio::dynamicMemSize",             mDynamicMemSize >> 10);
   Con::setIntVariable("Audio::dynamicMemUsage",            mDynamicMemUsage >> 10);
   Con::setIntVariable("Audio::bufferCount",                mBufferCount);
   Con::setIntVariable("Audio::dynamicBufferCount",         mDynamicBufferCount);
}
#endif

//--------------------------------------------------------------------------
// Audio Update... 
//--------------------------------------------------------------------------
void alxLoopingUpdate()
{
   static LoopingList culledList;

   S32 updateTime = Platform::getRealMilliseconds();

   // check if can wakeup the inactive loopers
   if(mLoopingCulledList.size())
   {
      Point3F listener;
      alxListenerGetPoint3F(AL_POSITION, &listener);
      
      // get the 'sort' value for this sound (could be based on time played...),
      // and add to the culled list
      LoopingList::iterator itr;
      culledList.clear();

      for(itr = mLoopingCulledList.begin(); itr != mLoopingCulledList.end(); itr++)
      {
         if((*itr)->mScore <= MIN_UNCULL_GAIN)
            continue;

         if((updateTime - (*itr)->mCullTime) < MIN_UNCULL_PERIOD)
            continue;
         
         culledList.push_back(*itr);
      }

      if(!culledList.size())
         return;

      U32 index = MAX_AUDIOSOURCES;   

      if(culledList.size() > 1)
         culledList.sort();

      for(itr = culledList.begin(); itr != culledList.end(); itr++)
      {
         if(!findFreeSource(&index))
         {  
            // score does not include master volume
            if(!cullSource(&index, (*itr)->mScore))
               break;

             // check buffer        
            if(!bool((*itr)->mBuffer))
            {
               // remove from culled list
               LoopingList::iterator tmp;
               tmp = mLoopingCulledList.findImage((*itr)->mHandle);
               AssertFatal(tmp, "alxLoopingUpdate: failed to find culled source");
               mLoopingCulledList.erase_fast(tmp);

               // remove from looping list (and free)
               tmp = mLoopingList.findImage((*itr)->mHandle);
               if(tmp)
               {
                  (*tmp)->clear();
                  mLoopingFreeList.push_back(*tmp);
                  mLoopingList.erase_fast(tmp);
               }
               
               continue;
            }
         }
            
         // remove from culled list
         LoopingList::iterator tmp = mLoopingCulledList.findImage((*itr)->mHandle);
         AssertFatal(tmp, "alxLoopingUpdate: failed to find culled source");
         mLoopingCulledList.erase_fast(tmp);

         // restore all state data
         mHandle[index] = (*itr)->mHandle;
         mBuffer[index] = (*itr)->mBuffer;
         mScore[index] = (*itr)->mScore;
         mSourceVolume[index] = (*itr)->mDescription.mVolume;
         mType[index] = (*itr)->mDescription.mType;
         mSampleEnvironment[index] = (*itr)->mEnvironment;

         ALuint source = mSource[index];

         // setup play info
         alGetError();     

         alxSourcePlay(source, *itr);
         if(mEnvironmentEnabled)
            alxSourceEnvironment(source, *itr);

         alxPlay(mHandle[index]);
      }
   }
}

//--------------------------------------------------------------------------
void alxCloseHandles()
{
   for(U32 i = 0; i < mNumSources; i++)
   {
      if(mHandle[i] & AUDIOHANDLE_LOADING_BIT)
         continue;

      if(mHandle[i] == NULL_AUDIOHANDLE)
         continue;

      ALint state = 0;
      alGetSourcei(mSource[i], AL_SOURCE_STATE, &state);
      if(state == AL_PLAYING)
         continue;
         
      // voice
      if(mHandle[i] & AUDIOHANDLE_VOICE_BIT)
      {
         VectorPtr<VoiceStream*>::iterator itr = alxFindVoiceStream(mHandle[i]);
         if(itr)
            alxFreeVoiceStream(itr);
      }
      else if(!(mHandle[i] & AUDIOHANDLE_INACTIVE_BIT))
      {
         // should be playing? must have encounted an error.. remove
         LoopingList::iterator itr = mLoopingList.findImage(mHandle[i]);
         if(itr && !((*itr)->mHandle & AUDIOHANDLE_INACTIVE_BIT))
         {
            AssertFatal(!mLoopingInactiveList.findImage((*itr)->mHandle), "alxCloseHandles: image incorrectly in inactive list");
            AssertFatal(!mLoopingCulledList.findImage((*itr)->mHandle), "alxCloseHandles: image already in culled list");
            mLoopingCulledList.push_back(*itr);
            (*itr)->mHandle |= AUDIOHANDLE_INACTIVE_BIT;

            mHandle[i] = NULL_AUDIOHANDLE;
            mBuffer[i] = 0;
         }
      }

      mHandle[i] = NULL_AUDIOHANDLE;
      mBuffer[i] = 0;
   }
}

//----------------------------------------------------------------------------------
// - update the score for each audio source.  this is used for culing sources.
//   normal ranges are between 0.f->1.f, voice/loading/music streams are scored
//   outside this range so that they will not be culled
// - does not scale by attenuated volumes
void alxUpdateScores(bool sourcesOnly)
{
   Point3F listener;
   alxGetListener3f(AL_POSITION, &listener.x, &listener.y, &listener.z);

   // do the base sources
   for(U32 i = 0; i < mNumSources; i++)
   {
      if(mHandle[i] == NULL_AUDIOHANDLE)
      {
         mScore[i] = 0.f;
         continue;
      }

      // thread loading buffer or voice buffer?
      if(mHandle[i] & (AUDIOHANDLE_LOADING_BIT|AUDIOHANDLE_VOICE_BIT))
      {
         mScore[i] = 3.f;
         continue;
      }

      // streaming?
      ALint val = AL_FALSE;
      alGetSourcei(mSource[i], AL_STREAMING, &val);
      if(val == AL_TRUE)
      {
         mScore[i] = 3.f;
         continue;
      }

      // grab the volume.. (not attenuated by master for score)
      F32 volume = mSourceVolume[i] * mAudioTypeVolume[mType[i]];

      // 3d?
      val = AL_FALSE;
      alGetSourcei(mSource[i], AL_SOURCE_AMBIENT, &val);

      if(val == AL_FALSE)
      {
         // approximate 3d volume
         Point3F pos;
         alGetSourcefv(mSource[i], AL_POSITION, (ALfloat * )((F32*)pos));

         ALfloat min, max;
         alGetSourcef(mSource[i], AL_MIN_DISTANCE, &min);
         alGetSourcef(mSource[i], AL_MAX_DISTANCE, &max);

         pos -= listener;
         F32 dist = pos.magnitudeSafe();

         if(dist >= max)
            mScore[i] = 0.f;
         else if(dist > min)
            mScore[i] = min / dist;
         else
            mScore[i] = volume;
      }
      else
         mScore[i] = volume;
   }

   if(sourcesOnly)
      return;

   S32 updateTime = Platform::getRealMilliseconds();

   // update the loopers
   for(LoopingList::iterator itr = mLoopingList.begin(); itr != mLoopingList.end(); itr++)
   {
      if(!((*itr)->mHandle & AUDIOHANDLE_INACTIVE_BIT))
         continue;

      if((updateTime - (*itr)->mCullTime) < MIN_UNCULL_PERIOD)
         continue;
      
      if((*itr)->mDescription.mIs3D)
      {
         Point3F pos = (*itr)->mPosition - listener;
         F32 dist = pos.magnitudeSafe();

         F32 min = (*itr)->mDescription.mMinDistance;
         F32 max = (*itr)->mDescription.mMaxDistance;

         if(dist >= max)
            (*itr)->mScore = 0.f;
         else if(dist > min)
            (*itr)->mScore = (*itr)->mDescription.mVolume * (min / dist);
         else
            (*itr)->mScore = (*itr)->mDescription.mVolume;
      }
      else
         (*itr)->mScore = (*itr)->mDescription.mVolume;

      // attenuate by the channel gain
      (*itr)->mScore *= mAudioTypeVolume[(*itr)->mDescription.mType];
   }
}

// the directx buffers are set to mute at max distance, but many of the providers seem to 
// ignore this flag... that is why this is here
void alxUpdateMaxDistance()
{
#ifndef __linux
   Point3F listener;
   alxGetListener3f(AL_POSITION, &listener.x, &listener.y, &listener.z);

   for(U32 i = 0; i < mNumSources; i++)
   {
      if(mHandle[i] == NULL_AUDIOHANDLE)
         continue;
      
      ALint val = AL_FALSE;
      alGetSourcei(mSource[i], AL_SOURCE_AMBIENT, &val);
      if(val == AL_TRUE)
         continue;
         
      val = AL_FALSE;   
      alGetSourcei(mSource[i], AL_STREAMING, &val);
      if(val == AL_TRUE)
         continue;
      
      Point3F pos;
      alGetSourcefv(mSource[i], AL_POSITION, (F32*)pos);

      F32 dist = 0.f;
      alGetSourcef(mSource[i], AL_MAX_DISTANCE, &dist);
      
      pos -= listener;
      dist -= pos.len();

      alSourcef(mSource[i], AL_GAIN_LINEAR, (dist < 0.f) ? 0.f : mSourceVolume[i] * mAudioTypeVolume[mType[i]] * mMasterVolume);
   }
#endif
}

//--------------------------------------------------------------------------
// Called to update alx system
//--------------------------------------------------------------------------
void alxUpdate()
{
   if(mForceMaxDistanceUpdate)
      alxUpdateMaxDistance();

   alxCloseHandles();
   alxUpdateScores(false);
   alxLoopingUpdate();
   alxCaptureUpdate();
   alxStreamUpdate();
#ifndef DISABLE_AUDIO_THREAD
   AudioThread::process();
#endif

#ifdef GATHER_METRICS
   alxGatherMetrics();
#endif

#if defined(__linux) && !defined(DEDICATED)
   alxFakeCallbackUpdate();
#endif
}   

//--------------------------------------------------------------------------
// Misc
//--------------------------------------------------------------------------
// client-side function only
ALuint alxGetWaveLen(ALuint buffer)
{
   if(buffer == AL_INVALID)
      return(0);

   ALint frequency = 0;
   ALint bits = 0;
   ALint channels = 0;
   ALint size;

   alGetBufferi(buffer, AL_FREQUENCY, &frequency);
   alGetBufferi(buffer, AL_BITS, &bits);
   alGetBufferi(buffer, AL_CHANNELS, &channels);
   alGetBufferi(buffer, AL_SIZE, &size);

   if(!frequency || !bits || !channels)
   {
      Con::errorf(ConsoleLogEntry::General, "alxGetWaveLen: invalid buffer");
      return(0);
   }

   F64 len = (F64(size) * 8000.f) / F64(frequency * bits * channels);
   return(len);
}

//--------------------------------------------------------------------------
// Console functions
//--------------------------------------------------------------------------
static void cAudio_detect(SimObject *, S32, const char **)
{
   Audio::detect();
}   

static void cAudio_destroy(SimObject *, S32, const char **)
{
   Audio::destroy();
}   

static bool cAudio_setDriver(SimObject *, S32, const char *argv[])
{
   return(Audio::setDriver(argv[1]));
}   

//--------------------------------------------------------------------------
static S32 cAudio_alxCreateSource(SimObject *, S32 argc, const char *argv[])
{
   AudioDescription *description = NULL;
   AudioProfile *profile = dynamic_cast<AudioProfile*>( Sim::findObject( argv[1] ) );
   if (profile == NULL)
   {
      description = dynamic_cast<AudioDescription*>( Sim::findObject( argv[1] ) );
      if (description == NULL)
      {
         Con::printf("Unable to locate audio profile/description '%s'", argv[1]);
         return NULL_AUDIOHANDLE;
      }
   }
   
   if (profile)
   {
      if (argc == 2)
         return alxCreateSource(profile);

      MatrixF transform;
      transform.set(EulerF(0,0,0), Point3F( dAtof(argv[2]),dAtof(argv[3]),dAtof(argv[4]) ));
      return alxCreateSource(profile, &transform);
   }

   if (description)
   {
      if (argc == 3)
         return alxCreateSource(description, argv[2]);
      
      MatrixF transform;
      transform.set(EulerF(0,0,0), Point3F( dAtof(argv[3]),dAtof(argv[4]),dAtof(argv[5]) ));
      return alxCreateSource(description, argv[2], &transform);
   }

   return NULL_AUDIOHANDLE;
}   

//--------------------------------------------------------------------------
// Expose all al get/set methods...
//--------------------------------------------------------------------------
enum {
   Source      =  BIT(0),
   Listener    =  BIT(1),
   Context     =  BIT(2),
   Environment =  BIT(3),
   Get         =  BIT(4),
   Set         =  BIT(5),
   Int         =  BIT(6),
   Float       =  BIT(7),
   Float3      =  BIT(8),
   Float6      =  BIT(9),
   DebugGet    =  BIT(10),
   DebugSet    =  BIT(11),
   Debug       =  (DebugGet|DebugSet),
};

static ALenum getEnum(const char * name, U32 flags)
{
   AssertFatal(name, "getEnum: bad param");

   static struct { 
      char *   mName;
      ALenum   mAlenum;
      U32      mFlags;
   } table[] = {
      //-----------------------------------------------------------------------------------------------------------------
      // "name"                           ENUM                             Flags
      //-----------------------------------------------------------------------------------------------------------------
      { "AL_PAN",                         AL_PAN,                          (Source|Get|Set|Float) },
      { "AL_GAIN",                        AL_GAIN,                         (Source|Listener|Get|Set|Float) },
      { "AL_GAIN_LINEAR",                 AL_GAIN_LINEAR,                  (Source|Listener|Get|Set|Float) },
      { "AL_PITCH",                       AL_PITCH,                        (Source|Get|Set|Float) },
      { "AL_MIN_DISTANCE",                AL_MIN_DISTANCE,                 (Source|Get|Set|Float) },
      { "AL_MAX_DISTANCE",                AL_MAX_DISTANCE,                 (Source|Get|Set|Float) },
      { "AL_CONE_OUTER_GAIN",             AL_CONE_OUTER_GAIN,              (Source|Get|Set|Float) },
      { "AL_POSITION",                    AL_POSITION,                     (Source|Listener|Get|Set|Float3) },
      { "AL_DIRECTION",                   AL_DIRECTION,                    (Source|Get|Set|Float3) },
      { "AL_VELOCITY",                    AL_VELOCITY,                     (Source|Listener|Get|Set|Float3) },
      { "AL_ORIENTATION",                 AL_ORIENTATION,                  (Listener|Set|Float6) },
      { "AL_CONE_INNER_ANGLE",            AL_CONE_INNER_ANGLE,             (Source|Get|Set|Int) },      
      { "AL_CONE_OUTER_ANGLE",            AL_CONE_OUTER_ANGLE,             (Source|Get|Set|Int) },
      { "AL_SOURCE_LOOPING",              AL_SOURCE_LOOPING,               (Source|Get|Set|Int) },
      { "AL_STREAMING",                   AL_STREAMING,                    (Source|Get|Set|Int) },
      { "AL_BUFFER",                      AL_BUFFER,                       (Source|Get|Set|Int) },
      { "AL_SOURCE_AMBIENT",              AL_SOURCE_AMBIENT,               (Source|Get|Set|Int) },

      { "ALC_PROVIDER",                   ALC_PROVIDER,                    (Context|Get|Set|Int) },
      { "ALC_PROVIDER_COUNT",             ALC_PROVIDER_COUNT,              (Context|Get|Int) },
      { "ALC_PROVIDER_NAME",              ALC_PROVIDER_NAME,               (Context|Get|Int) },
      { "ALC_SPEAKER",                    ALC_SPEAKER,                     (Context|Get|Set|Int) },
      { "ALC_SPEAKER_COUNT",              ALC_SPEAKER_COUNT,               (Context|Get|Int) },
      { "ALC_SPEAKER_NAME",               ALC_SPEAKER_NAME,                (Context|Get|Int) },
      { "ALC_BUFFER_DYNAMIC_MEMORY_SIZE", ALC_BUFFER_DYNAMIC_MEMORY_SIZE,  (Context|Get|Set|Int) },
      { "ALC_BUFFER_DYNAMIC_MEMORY_USAGE",ALC_BUFFER_DYNAMIC_MEMORY_USAGE, (Context|Get|Int) },
      { "ALC_BUFFER_DYNAMIC_COUNT",       ALC_BUFFER_DYNAMIC_COUNT,        (Context|Get|Int) },
      { "ALC_BUFFER_MEMORY_USAGE",        ALC_BUFFER_MEMORY_USAGE,         (Context|Get|Int) },
      { "ALC_BUFFER_COUNT",               ALC_BUFFER_COUNT,                (Context|Get|Int) },
      { "ALC_BUFFER_LATENCY",             ALC_BUFFER_LATENCY,              (Context|Get|Int) },

      // environment
      { "AL_ENV_ROOM_IASIG",                       AL_ENV_ROOM_IASIG,                        (Environment|Get|Set|Int) },
      { "AL_ENV_ROOM_HIGH_FREQUENCY_IASIG",        AL_ENV_ROOM_HIGH_FREQUENCY_IASIG,         (Environment|Get|Set|Int) },
      { "AL_ENV_REFLECTIONS_IASIG",                AL_ENV_REFLECTIONS_IASIG,                 (Environment|Get|Set|Int) },
      { "AL_ENV_REVERB_IASIG",                     AL_ENV_REVERB_IASIG,                      (Environment|Get|Set|Int) },
      { "AL_ENV_ROOM_ROLLOFF_FACTOR_IASIG",        AL_ENV_ROOM_ROLLOFF_FACTOR_IASIG,         (Environment|Get|Set|Float) },
      { "AL_ENV_DECAY_TIME_IASIG",                 AL_ENV_DECAY_TIME_IASIG,                  (Environment|Get|Set|Float) },
      { "AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG", AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG,  (Environment|Get|Set|Float) },
      { "AL_ENV_REFLECTIONS_DELAY_IASIG",          AL_ENV_REFLECTIONS_DELAY_IASIG,           (Environment|Get|Set|Float) },
      { "AL_ENV_REVERB_DELAY_IASIG",               AL_ENV_REVERB_DELAY_IASIG,                (Environment|Get|Set|Float) },
      { "AL_ENV_DIFFUSION_IASIG",                  AL_ENV_DIFFUSION_IASIG,                   (Environment|Get|Set|Float) },
      { "AL_ENV_DENSITY_IASIG",                    AL_ENV_DENSITY_IASIG,                     (Environment|Get|Set|Float) },
      { "AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG",   AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG,    (Environment|Get|Set|Float) },

      { "AL_ENV_ROOM_VOLUME_EXT",                  AL_ENV_ROOM_VOLUME_EXT,                   (Environment|Get|Set|Int) },
      { "AL_ENV_FLAGS_EXT",                        AL_ENV_FLAGS_EXT,                         (Environment|Get|Set|Int) },
      { "AL_ENV_EFFECT_VOLUME_EXT",                AL_ENV_EFFECT_VOLUME_EXT,                 (Environment|Get|Set|Float) },
      { "AL_ENV_DAMPING_EXT",                      AL_ENV_DAMPING_EXT,                       (Environment|Get|Set|Float) },
      { "AL_ENV_ENVIRONMENT_SIZE_EXT",             AL_ENV_ENVIRONMENT_SIZE_EXT,              (Environment|Get|Set|Float) },
      
      // sample environment
      { "AL_ENV_SAMPLE_DIRECT_EXT",                AL_ENV_SAMPLE_DIRECT_EXT,                 (Source|Get|Set|Int) },
      { "AL_ENV_SAMPLE_DIRECT_HF_EXT",             AL_ENV_SAMPLE_DIRECT_HF_EXT,              (Source|Get|Set|Int) },
      { "AL_ENV_SAMPLE_ROOM_EXT",                  AL_ENV_SAMPLE_ROOM_EXT,                   (Source|Get|Set|Int) },
      { "AL_ENV_SAMPLE_ROOM_HF_EXT",               AL_ENV_SAMPLE_ROOM_HF_EXT,                (Source|Get|Set|Int) },
      { "AL_ENV_SAMPLE_OUTSIDE_VOLUME_HF_EXT",     AL_ENV_SAMPLE_OUTSIDE_VOLUME_HF_EXT,      (Source|Get|Set|Int) },
      { "AL_ENV_SAMPLE_FLAGS_EXT",                 AL_ENV_SAMPLE_FLAGS_EXT,                  (Source|Get|Set|Int) },

      { "AL_ENV_SAMPLE_REVERB_MIX_EXT",            AL_ENV_SAMPLE_REVERB_MIX_EXT,             (Source|Get|Set|Float) },
      { "AL_ENV_SAMPLE_OBSTRUCTION_EXT",           AL_ENV_SAMPLE_OBSTRUCTION_EXT,            (Source|Get|Set|Float) },
      { "AL_ENV_SAMPLE_OBSTRUCTION_LF_RATIO_EXT",  AL_ENV_SAMPLE_OBSTRUCTION_LF_RATIO_EXT,   (Source|Get|Set|Float) },
      { "AL_ENV_SAMPLE_OCCLUSION_EXT",             AL_ENV_SAMPLE_OCCLUSION_EXT,              (Source|Get|Set|Float) },
      { "AL_ENV_SAMPLE_OCCLUSION_LF_RATIO_EXT",    AL_ENV_SAMPLE_OCCLUSION_LF_RATIO_EXT,     (Source|Get|Set|Float) },
      { "AL_ENV_SAMPLE_OCCLUSION_ROOM_RATIO_EXT",  AL_ENV_SAMPLE_OCCLUSION_ROOM_RATIO_EXT,   (Source|Get|Set|Float) },
      { "AL_ENV_SAMPLE_ROOM_ROLLOFF_EXT",          AL_ENV_SAMPLE_ROOM_ROLLOFF_EXT,           (Source|Get|Set|Float) },
      { "AL_ENV_SAMPLE_AIR_ABSORPTION_EXT",        AL_ENV_SAMPLE_AIR_ABSORPTION_EXT,         (Source|Get|Set|Float) },
   };

   for(U32 i = 0; i < (sizeof(table) / sizeof(table[0])); i++)
   {
#ifdef DEBUG
      if(((table[i].mFlags & ~Debug) & flags) != flags)
         continue;
#else
      if((table[i].mFlags & flags) != flags)
         continue;
#endif
      if(!dStricmp(table[i].mName, name))
         return(table[i].mAlenum);
   }

   return(AL_INVALID);
}

//--------------------------------------------------------------------------
// Source
//--------------------------------------------------------------------------
static void cAudio_alxSourcef(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[2], (Source|Set|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxSourcef: invalid enum name '%s'", argv[2]);
      return;
   }

   alxSourcef(dAtoi(argv[1]), e, dAtof(argv[3]));
}   

static void cAudio_alxSource3f(SimObject *, S32 argc, const char ** argv)
{
   ALenum e = getEnum(argv[2], (Source|Set|Float3));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxSource3f: invalid enum name '%s'", argv[2]);
      return;
   }

   if(argc != 3 || argc != 6)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxSource3f: wrong number of args");
      return;
   }

   Point3F pos;
   if(argc == 3)
      dSscanf(argv[1], "%f %f %f", &pos.x, &pos.y, &pos.z);
   else
   {
      pos.x = dAtof(argv[1]);
      pos.y = dAtof(argv[2]);
      pos.z = dAtof(argv[3]);
   }

   alxSource3f(dAtoi(argv[1]), e, pos.x, pos.y, pos.z);
}

static void cAudio_alxSourcei(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[2], (Source|Set|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxSourcei: invalid enum name '%s'", argv[2]);
      return;
   }

   alxSourcei(dAtoi(argv[1]), e, dAtoi(argv[3]));
}

//--------------------------------------------------------------------------

static F32 cAudio_alxGetSourcef(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[2], (Source|Get|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetSourcef: invalid enum name '%s'", argv[2]);
      return(0.f);
   }

   F32 value;
   alxGetSourcef(dAtoi(argv[1]), e, &value);
   return(value);
}  

static const char * cAudio_alxGetSource3f(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[2], (Source|Get|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetSource3f: invalid enum name '%s'", argv[2]);
      return("0 0 0");
   }

   F32 value1, value2, value3;
   alxGetSource3f(dAtoi(argv[1]), e, &value1, &value2, &value3);

   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%7.3f %7.3 %7.3", value1, value2, value3);
   return(ret);
}

static S32 cAudio_alxGetSourcei(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[2], (Source|Get|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetSourcei: invalid enum name '%s'", argv[2]);
      return(0);
   }

   S32 value;
   alxGetSourcei(dAtoi(argv[1]), e, &value);
   return(value);
}

//--------------------------------------------------------------------------
// Listener
//--------------------------------------------------------------------------
static void cAudio_alxListenerf(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Listener|Set|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxListenerf: invalid enum name '%s'", argv[1]);
      return;
   }

   alxListenerf(e, dAtof(argv[2]));
}   

static void cAudio_alxListener3f(SimObject *, S32 argc, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Listener|Set|Float3));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxListener3f: invalid enum name '%s'", argv[1]);
      return;
   }

   if(argc != 3 || argc != 5)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxListener3f: wrong number of args");
      return;
   }

   Point3F pos;
   if(argc == 3)
      dSscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   else
   {
      pos.x = dAtof(argv[2]);
      pos.y = dAtof(argv[3]);
      pos.z = dAtof(argv[4]);
   }

   alxListener3f(e, pos.x, pos.y, pos.z);
}

//--------------------------------------------------------------------------
static F32 cAudio_alxGetListenerf(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Source|Get|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetListenerf: invalid enum name '%s'", argv[1]);
      return(0.f);
   }

   F32 value;
   alxGetListenerf(e, &value);
   return(value);
}

static const char * cAudio_alxGetListener3f(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[2], (Source|Get|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetListener3f: invalid enum name '%s'", argv[1]);
      return("0 0 0");
   }

   F32 value1, value2, value3;
   alxGetListener3f(e, &value1, &value2, &value3);

   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%7.3f %7.3 %7.3", value1, value2, value3);
   return(ret);
}

static S32 cAudio_alxGetListeneri(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Source|Get|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetListeneri: invalid enum name '%s'", argv[1]);
      return(0);
   }

   S32 value;
   alxGetListeneri(e, &value);
   return(value);
}

//--------------------------------------------------------------------------
// Play/Stop
//--------------------------------------------------------------------------
static S32 cAudio_alxPlay(SimObject *, S32 argc, const char *argv[])
{
   if (argc == 2)
   {
      AUDIOHANDLE handle = dAtoi(argv[1]);
      return alxPlay(handle);
   }

   AudioProfile *profile = dynamic_cast<AudioProfile*>( Sim::findObject( argv[1] ) );
   if (profile == NULL)
   {
      Con::printf("Unable to locate audio profile '%s'", argv[1]);
      return NULL_AUDIOHANDLE;
   }

   Point3F pos(0.f, 0.f, 0.f);
   if(argc == 3)
      dSscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   else if(argc == 5)
      pos.set(dAtof(argv[2]), dAtof(argv[3]), dAtof(argv[4]));

   MatrixF transform;
   transform.set(EulerF(0,0,0), pos);

   return alxPlay(profile, &transform, NULL);
}   

static void cAudio_alxStop(SimObject *, S32, const char ** argv)
{
   AUDIOHANDLE handle = dAtoi(argv[1]);
   if(handle == NULL_AUDIOHANDLE)
      return;
   alxStop(handle);
}

static void cAudio_alxStopAll(SimObject *, S32, const char **)
{
   alxStopAll();
}

//--------------------------------------------------------------------------
// Capture
//--------------------------------------------------------------------------
static bool cAudio_alxCaptureInit(SimObject *, S32, const char **)
{
   return(alxCaptureInit());
}   

static void cAudio_alxCaptureDestroy(SimObject *, S32, const char **)
{
   alxCaptureDestroy();
}   

static void cAudio_alxCaptureStart(SimObject *, S32 argc, const char ** argv)
{
   alxCaptureStart((argc == 2) ? dAtob(argv[1]) : false);
}   

static void cAudio_alxCaptureStop(SimObject *, S32, const char **)
{
   alxCaptureStop();
}   

static bool cAudio_alxIsCapturing(SimObject *, S32, const char **)
{
   return(alxIsCapturing());
}

//--------------------------------------------------------------------------
// Environment:
//--------------------------------------------------------------------------
void alxEnvironmenti(ALenum pname, ALint value)
{
   alEnvironmentiIASIG(mEnvironment, pname, value);
}

void alxEnvironmentf(ALenum pname, ALfloat value)
{
   alEnvironmentfIASIG(mEnvironment, pname, value);
}

void alxGetEnvironmenti(ALenum pname, ALint * value)
{
   alGetEnvironmentiIASIG_EXT(mEnvironment, pname, value);
}

void alxGetEnvironmentf(ALenum pname, ALfloat * value)
{
   alGetEnvironmentfIASIG_EXT(mEnvironment, pname, value);
}

void alxEnableEnvironmental(bool enable)
{
   if(mEnvironmentEnabled == enable)
      return;

   // go through the playing sources and update their reverb mix
   // - only 3d samples get environmental fx
   // - only loopers can reenable fx
   for(U32 i = 0; i < MAX_AUDIOSOURCES; i++)
   {
      if(mHandle[i] == NULL_AUDIOHANDLE)
         continue;

      ALint val = AL_FALSE;

      // 3d?
      alGetSourcei(mSource[i], AL_SOURCE_AMBIENT, &val);
      if(val == AL_TRUE)
         continue;

      // stopped?
      val = AL_STOPPED;
      alGetSourcei(mSource[i], AL_SOURCE_STATE, &val);

      // only looping sources can reenable environmental effects (no description around
      // for the non-loopers)
      if(enable)
      {
         LoopingList::iterator itr = mLoopingList.findImage(mHandle[i]);
         if(!itr)
            continue;

         alSourcef(mSource[i], AL_ENV_SAMPLE_REVERB_MIX_EXT, (*itr)->mDescription.mEnvironmentLevel);
      }
      else
         alSourcef(mSource[i], AL_ENV_SAMPLE_REVERB_MIX_EXT, 0.f);
   }

   mEnvironmentEnabled = enable;
}

void alxSetEnvironment(const AudioEnvironment * env)
{
   mCurrentEnvironment = const_cast<AudioEnvironment*>(env);

   // reset environmental audio?
   if(!env)
   {
      alxEnvironmenti(AL_ENV_ROOM_IASIG, AL_ENVIRONMENT_GENERIC);
      return;
   }

   // room trashes all the values
   if(env->mUseRoom)
   {
      alxEnvironmenti(AL_ENV_ROOM_IASIG,                       env->mRoom);
      return;
   }

   // set all the params
   alxEnvironmenti(AL_ENV_ROOM_HIGH_FREQUENCY_IASIG,           env->mRoomHF);
   alxEnvironmenti(AL_ENV_REFLECTIONS_IASIG,                   env->mReflections);
   alxEnvironmenti(AL_ENV_REVERB_IASIG,                        env->mReverb);

   alxEnvironmentf(AL_ENV_ROOM_ROLLOFF_FACTOR_IASIG,           env->mRoomRolloffFactor);
   alxEnvironmentf(AL_ENV_DECAY_TIME_IASIG,                    env->mDecayTime);
   alxEnvironmentf(AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG,    env->mDecayTime);
   alxEnvironmentf(AL_ENV_REFLECTIONS_DELAY_IASIG,             env->mReflectionsDelay);
   alxEnvironmentf(AL_ENV_REVERB_DELAY_IASIG,                  env->mReverbDelay);
   alxEnvironmentf(AL_ENV_DENSITY_IASIG,                       env->mAirAbsorption);
   alxEnvironmentf(AL_ENV_DIFFUSION_IASIG,                     env->mEnvironmentDiffusion);

   // ext:
   alxEnvironmenti(AL_ENV_ROOM_VOLUME_EXT,                     env->mRoomVolume);
   alxEnvironmenti(AL_ENV_FLAGS_EXT,                           env->mFlags);

   alxEnvironmentf(AL_ENV_EFFECT_VOLUME_EXT,                   env->mEffectVolume);
   alxEnvironmentf(AL_ENV_DAMPING_EXT,                         env->mDamping);
   alxEnvironmentf(AL_ENV_ENVIRONMENT_SIZE_EXT,                env->mEnvironmentSize);
}

const AudioEnvironment * alxGetEnvironment()
{
   return(mCurrentEnvironment);
}

//--------------------------------------------------------------------------
static void cAudio_alxEnvironmenti(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Environment|Set|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxEnvironmenti: invalid enum name '%s'", argv[1]);
      return;
   }

   alxEnvironmenti(e, dAtoi(argv[2]));
}

static void cAudio_alxEnvironmentf(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Environment|Set|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxEnvironmentf: invalid enum name '%s'", argv[1]);
      return;
   }

   alxEnvironmenti(e, dAtof(argv[2]));
}

static S32 cAudio_alxGetEnvironmenti(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Environment|Get|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetEnvironmenti: invalid enum name '%s'", argv[1]);
      return(0);
   }

   S32 value;
   alxGetEnvironmenti(e, &value);
   return(value);
}

static F32 cAudio_alxGetEnvironmentf(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Environment|Get|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetEnvironmentf: invalid enum name '%s'", argv[1]);
      return(0.f);
   }

   F32 value;
   alxGetEnvironmentf(e, &value);
   return(value);
}

static void cAudio_alxSetEnvironment(SimObject *, S32, const char ** argv)
{
   AudioEnvironment * environment = dynamic_cast<AudioEnvironment*>(Sim::findObject(argv[1]));
   alxSetEnvironment(environment);
}

static void cAudio_alxEnableEnvironmental(SimObject *, S32, const char ** argv)
{
   alxEnableEnvironmental(dAtob(argv[1]));
}

//--------------------------------------------------------------------------
// Misc
//--------------------------------------------------------------------------
static S32 cAudio_alxGetWaveLen(SimObject *, S32, const char ** argv)
{
   // filename or profile?
   AudioProfile * profile = 0;
   const char * fileName = 0;

   if(!dStrrchr(argv[1], '.'))
   {
      profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));
      if(!profile)
      {
         Con::errorf(ConsoleLogEntry::General, "Unable to locate audio profile: '%s'", argv[1]);
         return(0);
      }

      fileName = profile->mFilename;
   }
   else
      fileName = argv[1];

   Resource<AudioBuffer> buffer = AudioBuffer::find(fileName);
   if(!bool(buffer))
   {
      Con::errorf(ConsoleLogEntry::General, "Failed to find wave file: '%s'", fileName);
      return(0);
   }
   
   ALuint alBuffer = buffer->getALBuffer(true);
   if(alBuffer == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetWaveLen: invalid buffer");
      return(0);
   }
   return(alxGetWaveLen(alBuffer));
}

static const char* cGetAudioDriverList( SimObject*, S32, const char** )
{
   return( Audio::getDriverListString() );   
}

static const char* cGetAudioDriverInfo( SimObject*, S32, const char** )
{
   return( Audio::getCurrentDriverInfo() );
}

static F32 cAudio_alxGetChannelVolume(SimObject *, S32, const char ** argv)
{
   U32 type = dAtoi(argv[1]);
   if(type >= Audio::NumAudioTypes)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetChannelVolume: invalid channel '%d'", dAtoi(argv[1]));
      return(0.f);
   }

   return(mAudioTypeVolume[type]);
}

static void cAudio_alxSetChannelVolume(SimObject *, S32, const char ** argv)
{
   U32 type = dAtoi(argv[1]);
   F32 volume = mClampF(dAtof(argv[2]), 0.f, 1.f);

   if(type >= Audio::NumAudioTypes)
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxSetChannelVolume: invalid channel '%d'", dAtoi(argv[1]));
   else
   {
      mAudioTypeVolume[type] = volume;
      alxUpdateTypeGain(1 << type);
   }
}

static void cAudio_alxSetCaptureGainScale(SimObject *, S32, const char ** argv)
{
   mCaptureGainScale = mClampF(dAtof(argv[1]), MIN_CAPTURE_SCALE, MAX_CAPTURE_SCALE);
}

static F32 cAudio_alxGetCaptureGainScale(SimObject *, S32, const char **)
{
   return(mCaptureGainScale);
}

static void cAudio_alxForceMaxDistanceUpdate(SimObject *, S32, const char ** argv)
{
   mForceMaxDistanceUpdate = dAtob(argv[1]);
}

static void cAudio_alxDisableOuterFalloffs(SimObject *, S32, const char ** argv)
{
   alxDisableOuterFalloffs(dAtob(argv[1]));
}

static void cAudio_alxSetInnerFalloffScale(SimObject *, S32, const char ** argv)
{
   alxSetInnerFalloffScale(dAtof(argv[1]));
}

static F32 cAudio_alxGetInnerFalloffScale(SimObject *, S32, const char **)
{
   return(alxGetInnerFalloffScale());
}

// Music: ----------------------------------------------------------------
static void cAudio_alxPlayMusic(SimObject *, S32, const char ** argv)
{
   alxPlayMusicStream(StringTable->insert(argv[1]));
}

static void cAudio_alxStopMusic(SimObject *, S32, const char **)
{
   alxStopMusicStream();
}

// Context: ----------------------------------------------------------------
void alxContexti(ALenum pname, ALint value)
{
   alContexti_EXT(pname, value);
}

void alxGetContexti(ALenum pname, ALint * value)
{
   alGetContexti_EXT(pname, value);
}

void alxGetContextstr(ALenum pname, ALuint idx, ALubyte ** value)
{
   alGetContextstr_EXT(pname, idx, value);
}

static void cAudio_alxContexti(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Context|Set|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxContexti: invalid enum name '%s'", argv[1]);
      return;
   }

   alxContexti(e, dAtoi(argv[2]));
}

static S32 cAudio_alxGetContexti(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Context|Get|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetContexti: invalid enum name '%s'", argv[2]);
      return(0);
   }

   ALint value = 0;
   alxGetContexti(e, &value);
   return(value);
}

static const char * cAudio_alxGetContextstr(SimObject *, S32, const char ** argv)
{
   ALenum e = getEnum(argv[1], (Context|Get|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxGetContextstr: invalid enum name '%s'", argv[2]);
      return("");
   }

   ALubyte * str = (ALubyte *)"";
   alxGetContextstr(e, dAtoi(argv[2]), &str);
   return(StringTable->insert((const char *)str));
}

static bool cAudio_isEnabled(SimObject *, S32, const char ** argv)
{
   if(!dStricmp(argv[1], "system"))
      return(mInitialized);
   if(!dStricmp(argv[1], "capture"))
      return(mCaptureInitialized);
   if(!dStricmp(argv[1], "environment_iasig"))
      return(mEnvironment && mEnvironmentEnabled);
   return(false);
}

static bool cAudio_isExtensionPresent(SimObject *, S32, const char ** argv)
{
   return((bool)alIsExtensionPresent((const ALubyte *)argv[1]));
}


// Namespace: Audio ---------------------------------------------------------
namespace Audio
{

//---------------------------------------------------------------------------
// the following db<->linear conversion functions come from Loki openAL linux driver
// code, here more for completeness than anything else (all current audio code
// uses AL_GAIN_LINEAR)... in Audio:: so that looping updates and audio channel updates
// can convert gain types and to give the miles driver access
static const F32 logtab[] = {
   0.00,    0.001,   0.002,   0.003,   0.004, 
   0.005,   0.01,    0.011,   0.012,   0.013, 
   0.014,   0.015,   0.016,   0.02,    0.021, 
   0.022,   0.023,   0.024,   0.025,   0.03, 
   0.031,   0.032,   0.033,   0.034,   0.04, 
   0.041,   0.042,   0.043,   0.044,   0.05, 
   0.051,   0.052,   0.053,   0.054,   0.06, 
   0.061,   0.062,   0.063,   0.064,   0.07,
   0.071,   0.072,   0.073,   0.08,    0.081, 
   0.082,   0.083,   0.084,   0.09,    0.091, 
   0.092,   0.093,   0.094,   0.10,    0.101, 
   0.102,   0.103,   0.11,    0.111,   0.112, 
   0.113,   0.12,    0.121,   0.122,   0.123, 
   0.124,   0.13,    0.131,   0.132,   0.14, 
   0.141,   0.142,   0.143,   0.15,    0.151, 
   0.152,   0.16,    0.161,   0.162,   0.17,
   0.171,   0.172,   0.18,    0.181,   0.19,
   0.191,   0.192,   0.20,    0.201,   0.21, 
   0.211,   0.22,    0.221,   0.23,    0.231, 
   0.24,    0.25,    0.251,   0.26,    0.27, 
   0.271,   0.28,    0.29,    0.30,    0.301, 
   0.31,    0.32,    0.33,    0.34,    0.35, 
   0.36,    0.37,    0.38,    0.39,    0.40,
   0.41,    0.43,    0.50,    0.60,    0.65, 
   0.70,    0.75,    0.80,    0.85,    0.90, 
   0.95,    0.97,    0.99 };
const int logmax = sizeof logtab / sizeof *logtab;
   
F32 DBToLinear(F32 value)
{
   if(value <= 0.f)
      return(0.f);
   if(value >= 1.f)
      return(1.f);

   S32 max = logmax;
   S32 min = 0;
   S32 mid;
   S32 last = -1;

   mid = (max - min) / 2;
   do {
      last = mid;

      if(logtab[mid] == value)
         break;

      if(logtab[mid] < value)
         min = mid;
      else
         max = mid;

      mid = min + ((max - min) / 2);
   } while(last != mid);

   return((F32)mid / logmax);
}

F32 linearToDB(F32 value)
{
   if(value <= 0.f)
      return(0.f);
   if(value >= 1.f)
      return(1.f);
   return(logtab[(U32)(logmax * value)]);
}
//---------------------------------------------------------------------------

static ALvoid errorCallback(ALbyte *msg)
{
   // used to allow our OpenAL implementation to display info on the console
   Con::errorf(ConsoleLogEntry::General, (const char *)msg);
} 

//--------------------------------------------------------------------------
bool prepareContext()
{
//   mForceMaxDistanceUpdate = Con::getBoolVariable("$pref::audio::forceMaxDistanceUpdate", false);
   mForceMaxDistanceUpdate = false;
   
   // allocate source channels: can only get 16 sources on NT, so check the max before
// jff: todo, allow for more than 16 channels on non-NT boxes
   mNumSources = mRequestSources;
   alGenSources(mRequestSources, mSource);

   // invalidate all existing handles
   dMemset(mHandle, NULL_AUDIOHANDLE, sizeof(mHandle));
   mCaptureInitialized = AL_FALSE;

   // pre-load profile data
   SimGroup* grp = Sim::getDataBlockGroup();
   if (grp != NULL)
   {
      SimObjectList::iterator itr = grp->begin();
      for (; itr != grp->end(); itr++)
      {
         AudioProfile *profile = dynamic_cast<AudioProfile*>(*itr);
         if(profile != NULL && profile->isPreload())
         {
            Resource<AudioBuffer> buffer = AudioBuffer::find(profile->mFilename);
            if((bool)buffer)
            {
               ALuint bufferId = buffer->getALBuffer(true);
               alBufferi_EXT(bufferId, AL_BUFFER_KEEP_RESIDENT, AL_TRUE);
            }
         }
      }
   }
   mInitialized = true;   
   return(true);
}  
 
void shutdownContext()
{
   alxCaptureStop();

   // invalidate active handles
   dMemset(mSource, 0, sizeof(mSource));
}   

//--------------------------------------------------------------------------
void init()
{
   Con::addCommand("AudioDetect",            cAudio_detect,             "AudioDetect()", 1, 1);
   Con::addCommand("AudioDestroy",           cAudio_destroy,            "AudioDestroy()", 1, 1);
   Con::addCommand("AudioSetDriver",         cAudio_setDriver,          "AudioSetDriver(name)", 2, 2);

   Con::addCommand("alxIsEnabled",           cAudio_isEnabled,          "alxIsEnabled(name)", 2, 2);
   
   Con::addCommand("alxIsExtensionPresent",  cAudio_isExtensionPresent, "alxIsExtensionPresent(name)", 2, 2);
   Con::addCommand("alxCreateSource",        cAudio_alxCreateSource,    "alxCreateSource(profile, {x,y,z} | description, filename, {x,y,z})", 2, 6);
   
   Con::addCommand("alxSourcef",             cAudio_alxSourcef,         "alxSourcef(handle, ALenum, value)", 4, 4);
   Con::addCommand("alxSource3f",            cAudio_alxSource3f,        "alxSource3f(handle, ALenum, \"x y z\" | x, y, z)", 3, 6);
   Con::addCommand("alxSourcei",             cAudio_alxSourcei,         "alxSourcei(handle, ALenum, value)", 4, 4);

   Con::addCommand("alxGetSourcef",          cAudio_alxGetSourcef,      "alxGetSourcef(handle, ALenum)", 3, 3);
   Con::addCommand("alxGetSource3f",         cAudio_alxGetSource3f,     "alxGetSource3f(handle, ALenum)", 3, 3);
   Con::addCommand("alxGetSourcei",          cAudio_alxGetSourcei,      "alxGetSourcei(handle, ALenum)", 3, 3);

   Con::addCommand("alxListenerf",           cAudio_alxListenerf,       "alxListenerf(ALenum, value)", 3, 3);
   Con::addCommand("alxListener3f",          cAudio_alxListener3f,      "alxListener3f(ALenum, \"x y z\" | x, y, z)", 3, 5);

   Con::addCommand("alxGetListenerf",        cAudio_alxGetListenerf,    "alxGetListenerf(Alenum)", 2, 2);
   Con::addCommand("alxGetListener3f",       cAudio_alxGetListener3f,   "alxGetListener3f(Alenum)", 2, 2);
   Con::addCommand("alxGetListeneri",        cAudio_alxGetListeneri,    "alxGetListeneri(Alenum)", 2, 2);

   Con::addCommand("alxContexti",            cAudio_alxContexti,        "alxContexti(Alenum, value)", 3, 3);
   Con::addCommand("alxGetContexti",         cAudio_alxGetContexti,     "alxGetContexti(Alenum)", 2, 2);
   Con::addCommand("alxGetContextstr",       cAudio_alxGetContextstr,   "alxGetContextstr(Alenum, idx)", 3, 3);

   Con::addCommand("alxPlay",                cAudio_alxPlay,            "alxPlay(handle) | alxPlay(profile, {x,y,z})", 2, 5);
   Con::addCommand("alxStop",                cAudio_alxStop,            "alxStop(handle)", 2, 2);
   Con::addCommand("alxStopAll",             cAudio_alxStopAll,         "alxStopAll()", 1, 1);

   Con::addCommand("alxCaptureInit",         cAudio_alxCaptureInit,     "alxCaptureInit()", 1, 1);
   Con::addCommand("alxCaptureDestroy",      cAudio_alxCaptureDestroy,  "alxCaptureDestroy()", 1, 1);
   Con::addCommand("alxCaptureStart",        cAudio_alxCaptureStart,    "alxCaptureStart(<local>)", 1, 2);
   Con::addCommand("alxCaptureStop",         cAudio_alxCaptureStop,     "alxCaptureStop()", 1, 1);
   Con::addCommand("alxIsCapturing",         cAudio_alxIsCapturing,     "alxIsCapturing()", 1, 1);

   Con::addCommand("alxEnvironmenti",        cAudio_alxEnvironmenti,    "alxEnvironmenti(Alenum, value)", 3, 3);
   Con::addCommand("alxEnvironmentf",        cAudio_alxEnvironmentf,    "alxEnvironmentf(Alenum, value)", 3, 3);
   Con::addCommand("alxGetEnvironmenti",     cAudio_alxGetEnvironmenti, "alxGetEnvironmenti(Alenum)", 2, 2);
   Con::addCommand("alxGetEnvironmentf",     cAudio_alxGetEnvironmentf, "alxGetEnvironmentf(Alenum)", 2, 2);

   Con::addCommand("alxSetEnvironment",      cAudio_alxSetEnvironment,      "alxSetEnvironment(AudioEnvironmentData)", 2, 2);
   Con::addCommand("alxEnableEnvironmental", cAudio_alxEnableEnvironmental, "alxEnableEnvironmental(bool)", 2, 2 );

   Con::addCommand("alxGetWaveLen",          cAudio_alxGetWaveLen,      "alxGetWaveLen(profile|filename)", 2, 2);

   Con::addCommand("getAudioDriverList",     cGetAudioDriverList,       "getAudioDriverList();", 1, 1);
   Con::addCommand("getAudioDriverInfo",     cGetAudioDriverInfo,       "getAudioDriverInfo();", 1, 1);

   ResourceManager->registerExtension(".wav", AudioBuffer::construct);

   Con::setIntVariable("DefaultAudioType",   Audio::DefaultAudioType);
   Con::setIntVariable("ChatAudioType",      Audio::ChatAudioType);
   Con::setIntVariable("GuiAudioType",       Audio::GuiAudioType);
   Con::setIntVariable("EffectAudioType",    Audio::EffectAudioType);
   Con::setIntVariable("VoiceAudioType",     Audio::VoiceAudioType);
   Con::setIntVariable("MusicAudioType",     Audio::MusicAudioType);

   Con::addCommand("alxGetChannelVolume",    cAudio_alxGetChannelVolume, "alxGetChannelVolume(channel)", 2, 2);
   Con::addCommand("alxSetChannelVolume",    cAudio_alxSetChannelVolume, "alxSetChannelVolume(channel, volume)", 3, 3);

   Con::addCommand("alxSetCaptureGainScale", cAudio_alxSetCaptureGainScale, "alxSetCaptureGainScale(scale)", 2, 2);
   Con::addCommand("alxGetCaptureGainScale", cAudio_alxGetCaptureGainScale, "alxGetCaptureGainScale()", 1, 1);
   
   Con::addCommand("alxPlayMusic",           cAudio_alxPlayMusic,             "alxPlayMusic(file)", 2, 2);
   Con::addCommand("alxStopMusic",           cAudio_alxStopMusic,             "alxStopMusic()", 1, 1);

   Con::addCommand("alxDisableOuterFalloffs",   cAudio_alxDisableOuterFalloffs,  "alxDisableOuterFalloffs(bool)",       2, 2);
   Con::addCommand("alxSetInnerFalloffScale",   cAudio_alxSetInnerFalloffScale,  "alxSetInnerFalloffScale(scale)",      2, 2);
   Con::addCommand("alxGetInnerFalloffScale",   cAudio_alxGetInnerFalloffScale,  "alxGetInnerFalloffScale()",           1, 1);

   Con::addCommand("alxForceMaxDistanceUpdate", cAudio_alxForceMaxDistanceUpdate, "alxForceMaxDistanceUpdate(bool)", 2, 2);
   
   // default all channels to full gain
   for(U32 i = 0; i < Audio::NumAudioTypes; i++)
      mAudioTypeVolume[i] = 1.f;
      
#ifndef DISABLE_AUDIO_THREAD
   AudioThread::create();
#endif
}  

//--------------------------------------------------------------------------
void detect()
{
   // destroy method takes down the audio thread
   if(mDriverInfoList.size())
   {
      destroy();
#ifndef DISABLE_AUDIO_THREAD
      AudioThread::create();
#endif
   }

   DriverInfo di;
   di.mName = dStrdup("None");
   di.mVender = dStrdup((const char*)alGetString(AL_VENDOR));
   di.mVersion = dStrdup((const char*)alGetString(AL_VERSION));
   di.mRenderer = dStrdup((const char*)alGetString(AL_RENDERER));
   di.mExtensions = dStrdup((const char*)alGetString(AL_EXTENSIONS));
   mDriverInfoList.push_back(di);
   mActiveDriver = mDriverInfoList.begin();

   const char *str = Con::getVariable("$pref::Audio::drivers");
   if (str)
   {
      char driverString[1024];
      dStrcpy(driverString, str);
      char *tok = dStrtok(driverString, " \t");
      while (tok != NULL)
      {
         if(libraryInit((const char *)tok))
         {
            if(alGetError() == AL_NO_ERROR)
            {
               if(alIsExtensionPresent((const ALubyte *)"AL_EXT_DYNAMIX"))
               {
                  di.mName = dStrdup(tok);
                  di.mVender = dStrdup((const char*)alGetString(AL_VENDOR));
                  di.mVersion = dStrdup((const char*)alGetString(AL_VERSION));
                  di.mRenderer = dStrdup((const char*)alGetString(AL_RENDERER));
                  di.mExtensions = dStrdup((const char*)alGetString(AL_EXTENSIONS));
                  mDriverInfoList.push_back(di);
               }
            }
            libraryShutdown();
         }
         tok = dStrtok(NULL, " \t");
      }
   }
}  

//--------------------------------------------------------------------------
void destroy()
{
#ifndef DISABLE_AUDIO_THREAD
   AudioThread::destroy();
#endif

   alxCaptureDestroy();

   if(mInitialized)
   {
      alxEnvironmentDestroy();
      alutExit();
   }

   libraryShutdown();

   while(mLoopingList.size())
   {
      mLoopingList.last()->mBuffer.purge();
      delete mLoopingList.last();
      mLoopingList.pop_back();
   }

   while(mLoopingFreeList.size())
   {
      mLoopingFreeList.last()->mBuffer.purge();
      delete mLoopingFreeList.last();
      mLoopingFreeList.pop_back();
   }

   mInitialized = false;
   while (mDriverInfoList.size())
   {
      dFree(mDriverInfoList.last().mName);
      dFree(mDriverInfoList.last().mVender);
      dFree(mDriverInfoList.last().mVersion);
      dFree(mDriverInfoList.last().mRenderer);
      dFree(mDriverInfoList.last().mExtensions);
      mDriverInfoList.pop_back();
   }
   mActiveDriver = mDriverInfoList.begin();
   
   for(U32 i = 0; i < MAX_AUDIOSOURCES; i++)
      mBuffer[i] = 0;
}   

//--------------------------------------------------------------------------
bool setDriver(const char *name)
{
   // handle a couple special keywords
   if (name == NULL || dStricmp(name, "none") == 0)
      name = mDriverInfoList.first().mName;

   if (dStricmp(name, "default") == 0)
      name = mDriverInfoList.last().mName;

   // don't reset if driver is already active
   if (mActiveDriver && (dStricmp(name, mActiveDriver->mName) == 0) )
      return true;
      
   DriverInfo *newDriver = NULL;

   // locate the driver
   Vector<DriverInfo>::iterator itr = mDriverInfoList.begin();
   for (; itr != mDriverInfoList.end(); itr++)
   {
      if (dStricmp(name, itr->mName) == 0)
      {
         newDriver = itr;
         break;
      }
   }
   
   // no driver by that name
   if (newDriver == NULL)
   {  
      Con::errorf(ConsoleLogEntry::General, "Unknown OpenAL audio driver '%s'", name );
      return false;
   }

   // shut down existing driver
   if (mActiveDriver)
   {
      alxCaptureDestroy();
      alutExit();
      libraryShutdown();
      mActiveDriver = NULL;
   }

   // stub driver
   if (newDriver == mDriverInfoList.begin())
   {  
      libraryShutdown();
      prepareContext();
      mActiveDriver = mDriverInfoList.begin();
      return true;
   }

   // load driver
   if(!libraryInit((const char *)name))
   {
      // install stub driver
      libraryShutdown();
      mActiveDriver = mDriverInfoList.begin();
      return(false);
   }


   S32 attribs[] = { ALC_FREQUENCY, 0,
                     ALC_RESOLUTION, 0,
                     ALC_CHANNELS, 0, 0 };

   // grab the console prefs   
   attribs[1] = Con::getIntVariable("$pref::Audio::frequency", ALX_DEF_SAMPLE_RATE);
   attribs[3] = Con::getIntVariable("$pref::Audio::sampleBits", ALX_DEF_SAMPLE_BITS);
   attribs[5] = Con::getIntVariable("$pref::Audio::channels", ALX_DEF_CHANNELS);

   // initialize the system
   alutInit(attribs, 0);
   if(alGetError() != AL_NO_ERROR)
   {
      // exit, load stub
      alutExit();
      libraryShutdown();
      mActiveDriver = mDriverInfoList.begin();
      return(false);
   }

   // must have dynamix extension
   if(alIsExtensionPresent((const ALubyte *)"AL_EXT_DYNAMIX"))
   {
      // bind the _EXT methods
      libraryInitExtensions();
      prepareContext();
      mActiveDriver = itr;

      Con::setVariable("$pref::Audio::activeDriver", mActiveDriver->mName);
      mInitialized = true;

      // generate an environment
      alxEnvironmentInit();

// set the listener gain to full and handle through this layer?
#ifdef HANDLE_LISTENER_GAIN
      alListenerf(AL_GAIN_LINEAR, 1.f);
#endif
      return(true);
   }
   
   // driver load failed, load the stub driver
   libraryShutdown();
   mActiveDriver = mDriverInfoList.begin();
   return(false);
}  

//--------------------------------------------------------------------------
const Vector<DriverInfo>* getDriverList()
{
   return &mDriverInfoList;
}

//--------------------------------------------------------------------------
const char* getDriverListString()
{
   U32 deviceCount = mDriverInfoList.size();
   if ( deviceCount > 0 )  // It better be...
   {
      U32 i, bufSize = 0;
      for ( i = 0; i < deviceCount; i++ )
         bufSize += ( dStrlen( mDriverInfoList[i].mName ) + 1 );

      char* returnString = Con::getReturnBuffer( bufSize );
      dStrcpy( returnString, mDriverInfoList[0].mName );
      for ( i = 1; i < deviceCount; i++ )
      {
         dStrcat( returnString, "\t" );
         dStrcat( returnString, mDriverInfoList[i].mName );
      }

      return( returnString );
   }

   return( "" );  
}

//--------------------------------------------------------------------------
const char* getCurrentDriverInfo()
{
   if ( mActiveDriver )
   {
      U32 bufSize = ( mActiveDriver->mName ? dStrlen( mActiveDriver->mName ) : 0 ) 
                  + ( mActiveDriver->mVender ? dStrlen( mActiveDriver->mVender ) : 0 ) 
                  + ( mActiveDriver->mRenderer ? dStrlen( mActiveDriver->mRenderer ) : 0 ) 
                  + ( mActiveDriver->mVersion ? dStrlen( mActiveDriver->mVersion ) : 0 ) 
                  + ( mActiveDriver->mExtensions ? dStrlen( mActiveDriver->mExtensions ) : 0 ) 
                  + 5;

      char* returnString = Con::getReturnBuffer( bufSize );
      dSprintf( returnString, bufSize, "%s\t%s\t%s\t%s\t%s",
            ( mActiveDriver->mName ? mActiveDriver->mName : "" ),
            ( mActiveDriver->mVender ? mActiveDriver->mVender : "" ),
            ( mActiveDriver->mRenderer ? mActiveDriver->mRenderer : "" ),
            ( mActiveDriver->mVersion ? mActiveDriver->mVersion : "" ),
            ( mActiveDriver->mExtensions ? mActiveDriver->mExtensions : "" ) );
      return( returnString );
   }

   return( "" );
}
   
} // end OpenAL namespace
