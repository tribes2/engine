//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AMBIENTAUDIOMANAGER_H_
#define _AMBIENTAUDIOMANAGER_H_

#ifndef _AUDIOEMITTER_H_
#include "game/audioEmitter.h"
#endif

static void cSetPowerAudioProfiles(SimObject *, S32, const char **);

class InteriorInstance;
class AmbientAudioManager
{
   friend void cSetPowerAudioProfiles(SimObject *, S32, const char **);

   private:
      F32                              mOutsideScale;    // 0:inside -> 1:outside
      Vector<AudioEmitter*>            mEmitters;
      SimObjectPtr<InteriorInstance>   mInteriorInstance;

      SimObjectPtr<AudioEnvironment>   mCurrentEnvironment;
      F32                              mEnvironmentScale;

      AUDIOHANDLE                      mInteriorAudioHandle;
      AUDIOHANDLE                      mPowerAudioHandle;
      bool                             mLastAlarmState;

      bool getOutsideScale(F32 *, InteriorInstance **);
      void updateEnvironment();
      void updateEmitter(AudioEmitter *);
      void stopInteriorAudio();
   
      SimObjectPtr<AudioProfile>       mPowerUpProfile;
      SimObjectPtr<AudioProfile>       mPowerDownProfile;

   public:        
      AmbientAudioManager();

      static void consoleInit();
      void addEmitter(AudioEmitter*);
      void removeEmitter(AudioEmitter*);
      void update();
};

extern AmbientAudioManager gAmbientAudioManager;

#endif
