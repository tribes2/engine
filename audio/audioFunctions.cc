//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platform/platformAudio.h"
#include "console/simBase.h"
#include "audio/audioDataBlock.h"



extern F32 mAudioTypeVolume[Audio::NumAudioTypes];   

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
   Float6      =  BIT(9)
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
      //{ "AL_SOURCE_TYPE",                 AL_SOURCE_TYPE,                  (Source|Get|Set|Float) },
      { "AL_GAIN",                        AL_GAIN,                         (Source|Listener|Get|Set|Float) },
      { "AL_GAIN_LINEAR",                 AL_GAIN_LINEAR,                  (Source|Listener|Get|Set|Float) },
      { "AL_PITCH",                       AL_PITCH,                        (Source|Get|Set|Float) },
      //{ "AL_MIN_DISTANCE",                AL_MIN_DISTANCE,                 (Source|Get|Set|Float) },
      { "AL_MAX_DISTANCE",                AL_MAX_DISTANCE,                 (Source|Get|Set|Float) },
      { "AL_CONE_OUTER_GAIN",             AL_CONE_OUTER_GAIN,              (Source|Get|Set|Float) },
      { "AL_POSITION",                    AL_POSITION,                     (Source|Listener|Get|Set|Float3) },
      { "AL_DIRECTION",                   AL_DIRECTION,                    (Source|Get|Set|Float3) },
      { "AL_VELOCITY",                    AL_VELOCITY,                     (Source|Listener|Get|Set|Float3) },
      { "AL_ORIENTATION",                 AL_ORIENTATION,                  (Listener|Set|Float6) },
      { "AL_CONE_INNER_ANGLE",            AL_CONE_INNER_ANGLE,             (Source|Get|Set|Int) },      
      { "AL_CONE_OUTER_ANGLE",            AL_CONE_OUTER_ANGLE,             (Source|Get|Set|Int) },
      //{ "AL_SOURCE_LOOPING",              AL_SOURCE_LOOPING,               (Source|Get|Set|Int) },
      //{ "AL_STREAMING",                   AL_STREAMING,                    (Source|Get|Set|Int) },
      //{ "AL_BUFFER",                      AL_BUFFER,                       (Source|Get|Set|Int) },
      //{ "AL_SOURCE_AMBIENT",              AL_SOURCE_AMBIENT,               (Source|Get|Set|Int) },

      { "AL_VENDOR",                      AL_VENDOR,                       (Context|Get) },
      { "AL_VERSION",                     AL_VERSION,                      (Context|Get) },
      { "AL_RENDERER",                    AL_RENDERER,                     (Context|Get) },
      { "AL_EXTENSIONS",                  AL_EXTENSIONS,                   (Context|Get) },

      //{ "ALC_PROVIDER",                   ALC_PROVIDER,                    (Context|Get|Set|Int) },
      //{ "ALC_PROVIDER_COUNT",             ALC_PROVIDER_COUNT,              (Context|Get|Int) },
      //{ "ALC_PROVIDER_NAME",              ALC_PROVIDER_NAME,               (Context|Get|Int) },
      //{ "ALC_SPEAKER",                    ALC_SPEAKER,                     (Context|Get|Set|Int) },
      //{ "ALC_SPEAKER_COUNT",              ALC_SPEAKER_COUNT,               (Context|Get|Int) },
      //{ "ALC_SPEAKER_NAME",               ALC_SPEAKER_NAME,                (Context|Get|Int) },
      //{ "ALC_BUFFER_DYNAMIC_MEMORY_SIZE", ALC_BUFFER_DYNAMIC_MEMORY_SIZE,  (Context|Get|Set|Int) },
      //{ "ALC_BUFFER_DYNAMIC_MEMORY_USAGE",ALC_BUFFER_DYNAMIC_MEMORY_USAGE, (Context|Get|Int) },
      //{ "ALC_BUFFER_DYNAMIC_COUNT",       ALC_BUFFER_DYNAMIC_COUNT,        (Context|Get|Int) },
      //{ "ALC_BUFFER_MEMORY_USAGE",        ALC_BUFFER_MEMORY_USAGE,         (Context|Get|Int) },
      //{ "ALC_BUFFER_COUNT",               ALC_BUFFER_COUNT,                (Context|Get|Int) },
      //{ "ALC_BUFFER_LATENCY",             ALC_BUFFER_LATENCY,              (Context|Get|Int) },
      /*
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
   */
   };
   for(U32 i = 0; i < (sizeof(table) / sizeof(table[0])); i++)
   {
      if((table[i].mFlags & flags) != flags)
         continue;

      if(dStricmp(table[i].mName, name) == 0)
         return(table[i].mAlenum);
   }

   return(AL_INVALID);
}


//-----------------------------------------------
ConsoleFunction(OpenALInitDriver, bool, 1, 1, "OpenALInitDriver()")
{
   if(Audio::OpenALInit())
   {
      ResourceManager->registerExtension(".wav", AudioBuffer::construct);
      Con::setIntVariable("DefaultAudioType",   Audio::DefaultAudioType);
      Con::setIntVariable("ChatAudioType",      Audio::ChatAudioType);
      Con::setIntVariable("GuiAudioType",       Audio::GuiAudioType);
      Con::setIntVariable("EffectAudioType",    Audio::EffectAudioType);

      return true;
   }
   return false;
}   

//-----------------------------------------------
ConsoleFunction(OpenALShutdownDriver, void, 1, 1, "OpenALShutdownDriver()")
{
   Audio::OpenALShutdown();
}   


//-----------------------------------------------
ConsoleFunction(alGetString, const char *, 2, 2, "alGetString(enum)")
{
   argc;
   ALenum e = getEnum(argv[1], (Context|Get));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "alGetString: invalid enum name '%s'", argv[1]);
      return "";
   }

   return (const char*)alGetString(e);
}   


//--------------------------------------------------------------------------
// Source
//--------------------------------------------------------------------------
ConsoleFunction(alxCreateSource, S32, 2, 6, "alxCreateSource(profile, {x,y,z} | description, filename, {x,y,z})")
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


//-----------------------------------------------
ConsoleFunction(alxSourcef, void, 4, 4, "alxSourcef(handle, ALenum, value)")
{
   ALenum e = getEnum(argv[2], (Source|Set|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxSourcef: invalid enum name '%s'", argv[2]);
      return;
   }

   alxSourcef(dAtoi(argv[1]), e, dAtof(argv[3]));
}   


//-----------------------------------------------
ConsoleFunction(alxSource3f, void, 3, 6, "alxSource3f(handle, ALenum, \"x y z\" | x, y, z)")
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


//-----------------------------------------------
ConsoleFunction(alxSourcei, void, 4, 4, "alxSourcei(handle, ALenum, value)")
{
   ALenum e = getEnum(argv[2], (Source|Set|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "cAudio_alxSourcei: invalid enum name '%s'", argv[2]);
      return;
   }

   alxSourcei(dAtoi(argv[1]), e, dAtoi(argv[3]));
}


//-----------------------------------------------
ConsoleFunction(alxGetSourcef, F32, 3, 3, "alxGetSourcef(handle, ALenum)")
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


//-----------------------------------------------
ConsoleFunction(alxGetSource3f, const char *, 3, 3, "alxGetSource3f(handle, ALenum)" )
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


//-----------------------------------------------
ConsoleFunction(alxGetSourcei, S32, 3, 3, "alxGetSourcei(handle, ALenum)")
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


//-----------------------------------------------
ConsoleFunction(alxPlay, S32, 2, 5, "alxPlay(handle) | alxPlay(profile, {x,y,z})")
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

//-----------------------------------------------
ConsoleFunction(alxStop, void, 2, ,2, "alxStop(handle)")
{
   AUDIOHANDLE handle = dAtoi(argv[1]);
   if(handle == NULL_AUDIOHANDLE)
      return;
   alxStop(handle);
}

//-----------------------------------------------
ConsoleFunction(alxStopAll, void, 1, 1, "alxStopAll()")
{
   alxStopAll();
}


//--------------------------------------------------------------------------
// Listener
//--------------------------------------------------------------------------
ConsoleFunction(alxListenerf, void, 2, 2, "alxListener(ALenum, value)")
{
   ALenum e = getEnum(argv[1], (Listener|Set|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "alxListenerf: invalid enum name '%s'", argv[1]);
      return;
   }

   alxListenerf(e, dAtof(argv[2]));
}   


//-----------------------------------------------
ConsoleFunction(alxListener3f, void, 3, 5, "alxListener3f(ALenum, \"x y z\" | x, y, z)")
{
   ALenum e = getEnum(argv[1], (Listener|Set|Float3));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "alxListener3f: invalid enum name '%s'", argv[1]);
      return;
   }

   if(argc != 3 || argc != 5)
   {
      Con::errorf(ConsoleLogEntry::General, "alxListener3f: wrong number of arguments");
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


//-----------------------------------------------
ConsoleFunction(alxGetListenerf, F32, 2, 2, "alxGetListenerf(Alenum)")
{
   ALenum e = getEnum(argv[1], (Source|Get|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "alxGetListenerf: invalid enum name '%s'", argv[1]);
      return(0.f);
   }

   F32 value;
   alxGetListenerf(e, &value);
   return(value);
}


//-----------------------------------------------
ConsoleFunction(alxGetListener3f, const char *, 2, 2, "alxGetListener3f(Alenum)")
{
   ALenum e = getEnum(argv[2], (Source|Get|Float));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "alxGetListener3f: invalid enum name '%s'", argv[1]);
      return("0 0 0");
   }

   F32 value1, value2, value3;
   alxGetListener3f(e, &value1, &value2, &value3);

   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%7.3f %7.3 %7.3", value1, value2, value3);
   return(ret);
}


//-----------------------------------------------
ConsoleFunction(alxGetListeneri, S32, 2, 2, "alxGetListeneri(Alenum)")
{
   ALenum e = getEnum(argv[1], (Source|Get|Int));
   if(e == AL_INVALID)
   {
      Con::errorf(ConsoleLogEntry::General, "alxGetListeneri: invalid enum name '%s'", argv[1]);
      return(0);
   }

   S32 value;
   alxGetListeneri(e, &value);
   return(value);
}


//--------------------------------------------------------------------------
// Channel Volumes
//--------------------------------------------------------------------------
ConsoleFunction(alxGetChannelVolume, S32, 2, 2, "alxGetChannelVolume(channel_id)")
{
   U32 type = dAtoi(argv[1]);
   if(type >= Audio::NumAudioTypes)
   {
      Con::errorf(ConsoleLogEntry::General, "alxGetChannelVolume: invalid channel '%d'", dAtoi(argv[1]));
      return(0.f);
   }

   return(mAudioTypeVolume[type]);
}

//-----------------------------------------------
ConsoleFunction(alxSetChannelVolume, bool, 3, 3, "alxGetChannelVolume(channel_id, volume 0.0-1.0)")
{
   U32 type = dAtoi(argv[1]);
   F32 volume = mClampF(dAtof(argv[2]), 0.f, 1.f);

   if(type >= Audio::NumAudioTypes)
   {
      Con::errorf(ConsoleLogEntry::General, "alxSetChannelVolume: invalid channel '%d'", dAtoi(argv[1]));
      return false;
   }

   mAudioTypeVolume[type] = volume;
#pragma message("todo") /*
   alxUpdateTypeGain(1 << type);
*/
   return true;
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------


#if 0

ConsoleFunction(ExpandFilename, const char*, 2, 2, "ExpandFilename(filename)")
{
}


//--------------------------------------------------------------------------
void init()
{
   Con::addCommand("alxIsEnabled",           cAudio_isEnabled,          "alxIsEnabled(name)", 2, 2);
   
   Con::addCommand("alxIsExtensionPresent",  cAudio_isExtensionPresent, "alxIsExtensionPresent(name)", 2, 2);

   Con::addCommand("alxContexti",            cAudio_alxContexti,        "alxContexti(Alenum, value)", 3, 3);
   Con::addCommand("alxGetContexti",         cAudio_alxGetContexti,     "alxGetContexti(Alenum)", 2, 2);
   Con::addCommand("alxGetContextstr",       cAudio_alxGetContextstr,   "alxGetContextstr(Alenum, idx)", 3, 3);

   Con::addCommand("alxEnvironmenti",        cAudio_alxEnvironmenti,    "alxEnvironmenti(Alenum, value)", 3, 3);
   Con::addCommand("alxEnvironmentf",        cAudio_alxEnvironmentf,    "alxEnvironmentf(Alenum, value)", 3, 3);
   Con::addCommand("alxGetEnvironmenti",     cAudio_alxGetEnvironmenti, "alxGetEnvironmenti(Alenum)", 2, 2);
   Con::addCommand("alxGetEnvironmentf",     cAudio_alxGetEnvironmentf, "alxGetEnvironmentf(Alenum)", 2, 2);

   Con::addCommand("alxSetEnvironment",      cAudio_alxSetEnvironment,      "alxSetEnvironment(AudioEnvironmentData)", 2, 2);
   Con::addCommand("alxEnableEnvironmental", cAudio_alxEnableEnvironmental, "alxEnableEnvironmental(bool)", 2, 2 );

   Con::addCommand("alxGetWaveLen",          cAudio_alxGetWaveLen,      "alxGetWaveLen(profile|filename)", 2, 2);

   Con::addCommand("getAudioDriverList",     cGetAudioDriverList,       "getAudioDriverList();", 1, 1);
   Con::addCommand("getAudioDriverInfo",     cGetAudioDriverInfo,       "getAudioDriverInfo();", 1, 1);

   Con::addCommand("alxSetCaptureGainScale", cAudio_alxSetCaptureGainScale, "alxSetCaptureGainScale(scale)", 2, 2);
   Con::addCommand("alxGetCaptureGainScale", cAudio_alxGetCaptureGainScale, "alxGetCaptureGainScale()", 1, 1);
   
   Con::addCommand("alxDisableOuterFalloffs",   cAudio_alxDisableOuterFalloffs,  "alxDisableOuterFalloffs(bool)",       2, 2);
   Con::addCommand("alxSetInnerFalloffScale",   cAudio_alxSetInnerFalloffScale,  "alxSetInnerFalloffScale(scale)",      2, 2);
   Con::addCommand("alxGetInnerFalloffScale",   cAudio_alxGetInnerFalloffScale,  "alxGetInnerFalloffScale()",           1, 1);

   Con::addCommand("alxForceMaxDistanceUpdate", cAudio_alxForceMaxDistanceUpdate, "alxForceMaxDistanceUpdate(bool)", 2, 2);
   
   // default all channels to full gain
   for(U32 i = 0; i < Audio::NumAudioTypes; i++)
      mAudioTypeVolume[i] = 1.f;
}  


//--------------------------------------------------------------------------
// Console functions
//--------------------------------------------------------------------------
static bool cAudio_setDriver(SimObject *, S32, const char *argv[])
{
   return(Audio::setDriver(argv[1]));
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
#pragma message("todo") /*
   return((bool)alIsExtensionPresent((const ALubyte *)argv[1]));
*/
   return false;
}




#endif