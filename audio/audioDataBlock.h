//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIODATABLOCK_H_
#define _AUDIODATABLOCK_H_

#ifndef _PLATFORMAUDIO_H_
#include "Platform/platformAudio.h"
#endif
#ifndef _AUDIOBUFFER_H_
#include "audio/audioBuffer.h"
#endif
#ifndef _BITSTREAM_H_
#include "Core/bitStream.h"
#endif
#ifndef _NETOBJECT_H_
#include "Sim/netObject.h"
#endif

//--------------------------------------------------------------------------
class AudioEnvironment : public SimDataBlock
{
   typedef SimDataBlock Parent;
   
   public:

      bool  mUseRoom;
      S32   mRoom;
      S32   mRoomHF;
      S32   mReflections;
      S32   mReverb;
      F32   mRoomRolloffFactor;
      F32   mDecayTime;
      F32   mDecayHFRatio;
      F32   mReflectionsDelay;
      F32   mReverbDelay;
      S32   mRoomVolume;
      F32   mEffectVolume;
      F32   mDamping;
      F32   mEnvironmentSize;
      F32   mEnvironmentDiffusion;
      F32   mAirAbsorption;
      S32   mFlags;
         
      AudioEnvironment();
      
      static void consoleInit();
      void packData(BitStream* stream);
      void unpackData(BitStream* stream);
      
      DECLARE_CONOBJECT(AudioEnvironment);
};

//--------------------------------------------------------------------------
class AudioSampleEnvironment : public SimDataBlock
{
   typedef SimDataBlock Parent;
   
   public:

      S32      mDirect;
      S32      mDirectHF;
      S32      mRoom;
      S32      mRoomHF;
      F32      mObstruction;
      F32      mObstructionLFRatio;
      F32      mOcclusion;
      F32      mOcclusionLFRatio;
      F32      mOcclusionRoomRatio;
      F32      mRoomRolloff;
      F32      mAirAbsorption;
      S32      mOutsideVolumeHF;
      S32      mFlags;
         
      AudioSampleEnvironment();
      
      static void consoleInit();
      void packData(BitStream* stream);
      void unpackData(BitStream* stream);
      
      DECLARE_CONOBJECT(AudioSampleEnvironment);
};

//--------------------------------------------------------------------------
class AudioDescription: public SimDataBlock 
{
   typedef SimDataBlock Parent;
  public:

   // field info
   Audio::Description      mDescription;

   AudioDescription();
   DECLARE_CONOBJECT(AudioDescription);
   static void consoleInit();
   virtual bool onAdd();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   const Audio::Description* getDescription() const { return &mDescription; }
};

//----------------------------------------------------------------------------
class AudioProfile: public SimDataBlock 
{
   typedef SimDataBlock Parent;
  public:

   // field info
   AudioDescription *               mDescriptionObject;
   AudioSampleEnvironment *         mSampleEnvironment;

   StringTableEntry                 mFilename;
   bool                             mPreload;

   AudioProfile();
   DECLARE_CONOBJECT(AudioProfile);
   static void consoleInit();
   
   virtual bool onAdd();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   const Audio::Description* getDescription() const { return mDescriptionObject ? mDescriptionObject->getDescription() : NULL; }
   bool isPreload() { return mPreload; }
};

#endif  // _H_AUDIODATABLOCK_
