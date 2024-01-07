//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMAL_H_
#define _PLATFORMAL_H_

#ifndef _PLATFORM_H_
#include "engine/platform/platform.h"
#endif
#include <AL/altypes.h>
#include <AL/alctypes.h>
#include <AL/alexttypes.h>


// rename
#define AL_GAIN_LINEAR             AL_GAIN_LINEAR_LOKI
#define AL_SOURCE_LOOPING          AL_LOOPING
#define AL_MIN_DISTANCE            AL_REFERENCE_DISTANCE

// fictional tokens
#define AL_PAN                     0xFFFFFFFE
#define AL_SOURCE_AMBIENT          0xFFFFFFFD
#define AL_BUFFER_KEEP_RESIDENT    0xFFFFFFFC

#define ALC_PROVIDER               0x1FFFFFFE
#define ALC_PROVIDER_COUNT         0x1FFFFFFD
#define ALC_PROVIDER_NAME          0x1FFFFFFC

#define ALC_SPEAKER                0x1FFFFFFB
#define ALC_SPEAKER_COUNT          0x1FFFFFFA
#define ALC_SPEAKER_NAME           0x1FFFFFF9

#define ALC_BUFFER_MEMORY_USAGE    0x1FFFFFF8
#define ALC_BUFFER_MEMORY_SIZE     0x1FFFFFF7
#define ALC_BUFFER_MEMORY_COUNT    0x1FFFFFF6
#define ALC_BUFFER_COUNT           0x1FFFFFF5
#define ALC_BUFFER_DYNAMIC_MEMORY_USAGE 0x1FFFFFF4
#define ALC_BUFFER_DYNAMIC_MEMORY_SIZE  0x1FFFFFF3
#define ALC_BUFFER_DYNAMIC_MEMORY_COUNT 0x1FFFFFF2
#define ALC_BUFFER_DYNAMIC_COUNT   0x1FFFFFF1
#define ALC_BUFFER_LATENCY         0x1FFFFFF0
#define ALC_RESOLUTION             0x1FFFFFEF
#define ALC_CHANNELS               0x1FFFFFEE

#define AL_ENV_FLAGS_EXT           0x2FFFFFFE
#define AL_ENV_ROOM_VOLUME_EXT     0x2FFFFFFD
#define AL_ENV_EFFECT_VOLUME_EXT   0x2FFFFFFC
#define AL_ENV_DAMPING_EXT         0x2FFFFFFB
#define AL_ENV_ENVIRONMENT_SIZE_EXT 0x2FFFFFFA
#define AL_ENV_SAMPLE_REVERB_MIX_EXT 0x2FFFFFF9
#define AL_ENV_SAMPLE_DIRECT_EXT   0x2FFFFFF8
#define AL_ENV_SAMPLE_DIRECT_HF_EXT 0x2FFFFFF7
#define AL_ENV_SAMPLE_ROOM_EXT     0x2FFFFFF6
#define AL_ENV_SAMPLE_ROOM_HF_EXT  0x2FFFFFF5
#define AL_ENV_SAMPLE_OUTSIDE_VOLUME_HF_EXT 0x2FFFFFF4
#define AL_ENV_SAMPLE_FLAGS_EXT    0x2FFFFFF3
#define AL_ENV_SAMPLE_OBSTRUCTION_EXT 0x2FFFFFF2
#define AL_ENV_SAMPLE_OBSTRUCTION_LF_RATIO_EXT 0x2FFFFFF1
#define AL_ENV_SAMPLE_OCCLUSION_EXT 0x2FFFFFF0
#define AL_ENV_SAMPLE_OCCLUSION_LF_RATIO_EXT 0x2FFFFFEF
#define AL_ENV_SAMPLE_OCCLUSION_ROOM_RATIO_EXT 0x2FFFFFEE
#define AL_ENV_SAMPLE_ROOM_ROLLOFF_EXT 0x2FFFFFED
#define AL_ENV_SAMPLE_AIR_ABSORPTION_EXT 0x2FFFFFEC


// room types: same as miles/eax
enum {
    AL_ENVIRONMENT_GENERIC = 0,
    AL_ENVIRONMENT_PADDEDCELL,
    AL_ENVIRONMENT_ROOM,
    AL_ENVIRONMENT_BATHROOM,
    AL_ENVIRONMENT_LIVINGROOM,
    AL_ENVIRONMENT_STONEROOM,
    AL_ENVIRONMENT_AUDITORIUM,
    AL_ENVIRONMENT_CONCERTHALL,
    AL_ENVIRONMENT_CAVE,
    AL_ENVIRONMENT_ARENA,
    AL_ENVIRONMENT_HANGAR,
    AL_ENVIRONMENT_CARPETEDHALLWAY,
    AL_ENVIRONMENT_HALLWAY,
    AL_ENVIRONMENT_STONECORRIDOR,
    AL_ENVIRONMENT_ALLEY,
    AL_ENVIRONMENT_FOREST,
    AL_ENVIRONMENT_CITY,
    AL_ENVIRONMENT_MOUNTAINS,
    AL_ENVIRONMENT_QUARRY,
    AL_ENVIRONMENT_PLAIN,
    AL_ENVIRONMENT_PARKINGLOT,
    AL_ENVIRONMENT_SEWERPIPE,
    AL_ENVIRONMENT_UNDERWATER,
    AL_ENVIRONMENT_DRUGGED,
    AL_ENVIRONMENT_DIZZY,
    AL_ENVIRONMENT_PSYCHOTIC,

    AL_ENVIRONMENT_COUNT
};

// declare OpenAL functions
#define AL_EXTENSION(ext_name) extern bool gDoesSupport_##ext_name;
#define AL_FUNCTION(fn_return,fn_name,fn_args) extern fn_return (FN_CDECL *fn_name)fn_args;
#define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) extern fn_return (FN_CDECL *fn_name)fn_args;
#ifndef _OPENALFN_H_
#include "lib/openal/win32/openALFn.h"
#endif

namespace Audio
{

bool libraryInit(const char *library);
void libraryInitExtensions();
void libraryShutdown();

inline bool doesSupportIASIG()
{
   return gDoesSupport_AL_EXT_IASIG;
}   

inline bool doesSupportDynamix()
{
   return gDoesSupport_AL_EXT_DYNAMIX;
}  

// helpers
F32 DBToLinear(F32 value);
F32 linearToDB(F32 value);

}  // end namespace Audio

// Used by the Linux client audio
extern void alxFakeCallbackUpdate( void );

#endif
