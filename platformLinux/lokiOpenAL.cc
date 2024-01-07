//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifdef DEDICATED

// Stubs for the dedicated server

bool bindOpenALFunctions(void)
{
   return false;
}

void unbindOpenALFunctions(void)
{
   return;
}

#else

// Code for the Linux client

#include <dlfcn.h>

#include "engine/console/console.h"
#include "engine/core/fileio.h"
#include "engine/core/tVector.h"
#include "engine/platformWIN32/platformAL.h"

// For the MP3 playback support
#include "SDL.h"
#include "smpeg.h"

static void *hinstOpenAL = NULL;

// declare DLL loaded OpenAL functions
#define AL_FUNCTION(fn_return,fn_name,fn_args) fn_return (*OpenAL_##fn_name)fn_args = NULL;
#define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) fn_return (*OpenAL_##fn_name)fn_args = NULL;
// An API change in OpenAL 1.0 - the context creation takes a device handle
AL_FUNCTION(ALCdevice *, alcOpenDevice, ( const ALubyte *tokstr ));
AL_FUNCTION(ALvoid, alcCloseDevice, ( ALCdevice *dev ));
AL_FUNCTION(ALvoid, alDistanceModel, ( ALenum distanceModel ));
ALvoid* (*OpenAL10_alcCreateContext)( struct _AL_device *dev,  ALint* attrlist ) = NULL;
#include "lib/openal/win32/openALFn.h"

// Loki functions: ---------------------------------------------------------

// Used by alutInit() and alutExit() for device context handling
static void *context_id;
static ALCdevice *dev;

typedef struct fakeCallbackPair_s {
   U32 sid;
   void (*proc)(U32, bool);
} fakeCallbackPair_t;

static Vector< fakeCallbackPair_t > fakeCallbackFake(__FILE__, __LINE__);

void alxFakeCallbackUpdate( void )
{
   fakeCallbackPair_t pi;
   ALuint state;
   int i;

   i = fakeCallbackFake.size();

   while( i-- ) {
      pi = fakeCallbackFake.last();
      fakeCallbackFake.pop_back();

      state = AL_INITIAL;

      alGetSourcei( pi.sid, AL_SOURCE_STATE, &state );

      if( state == AL_STOPPED )
      {
         Con::printf( "Calling callback for %d", pi.sid);
         pi.proc( pi.sid, false );
      } else {
         fakeCallbackFake.push_front( pi );
      }
   }
}

// Code for MP3 playback support

static SMPEG *mpeg = NULL;

#define MAX_MPEG_READ 512

static ALint MP3_Callback(ALuint sid,
		ALuint bid,
		ALshort *outdata,
		ALenum format,
		ALint freq,
		ALint samples)
{
   int bytesRequested = samples * sizeof( ALshort );
   int bytesPlayed;
   int i;

   if(samples > MAX_MPEG_READ) {
      int first, second;
      first  = MP3_Callback(sid, bid, outdata, format, freq, MAX_MPEG_READ);
      second = MP3_Callback(sid, bid, outdata + MAX_MPEG_READ, format, freq, samples - MAX_MPEG_READ);
      return first + second;
   }

   if( SMPEG_status(mpeg) != SMPEG_PLAYING ) {
      SMPEG_play( mpeg );
   }

   memset( outdata, 0, bytesRequested );

   bytesPlayed = SMPEG_playAudio( mpeg, (ALubyte *) outdata, bytesRequested );
   bytesPlayed /= sizeof( ALshort );

   if(bytesPlayed < samples) {
      SMPEG_stop( mpeg );
      SMPEG_rewind( mpeg );

      return bytesPlayed;
   }
   return samples;
}

ALboolean alutLoadMP3_LOKI(ALuint bid, ALvoid *data, ALint size)
{
   void (*alBufferDataWithCallback)(ALuint bid,
             int (*Callback)(ALuint, ALuint, ALshort *, ALenum, ALint, ALint));
   SDL_AudioSpec spec;

   alBufferDataWithCallback = (void (*)(ALuint bid,
             int (*Callback)(ALuint, ALuint, ALshort *, ALenum, ALint, ALint)))
             alGetProcAddress((ALubyte *) "alBufferDataWithCallback_LOKI");

   if(alBufferDataWithCallback == NULL) {
      Con::errorf(ConsoleLogEntry::General, "Need alBufferDataWithCallback()");
      return AL_FALSE;
   }

   if ( mpeg != NULL ) {
      SMPEG_stop(mpeg);
      SMPEG_delete(mpeg);
   }
   mpeg = SMPEG_new_data( data, size, NULL, 0 );
   if ( mpeg == NULL ) {
      Con::errorf( ConsoleLogEntry::General, "Unable to allocate MP3 data");
      return AL_FALSE;
   }

   SMPEG_wantedSpec( mpeg, &spec );

   spec.freq     = Con::getIntVariable( "$pref::Audio::frequency", 22050 );
   spec.format   = AUDIO_S16;

   /* implicitly multichannel */
   alBufferi_EXT( bid, AL_CHANNELS, spec.channels );

   SMPEG_actualSpec( mpeg, &spec );

   SMPEG_enableaudio( mpeg, 1 );
   SMPEG_enablevideo( mpeg, 0 ); /* sanity check */

   alBufferDataWithCallback(bid, MP3_Callback);

   return AL_TRUE;
}

// AL functions
ALvoid loki_alEnable( ALenum capability )
{
   OpenAL_alEnable( capability );
}
ALvoid loki_alDisable( ALenum capability )
{
   OpenAL_alDisable( capability );
}
ALboolean loki_alIsEnabled( ALenum capability )
{
   return OpenAL_alIsEnabled( capability );
}
ALvoid loki_alHint( ALenum target, ALenum mode )
{
   OpenAL_alHint( target, mode );
}
ALboolean loki_alGetBoolean( ALenum param )
{
   return OpenAL_alGetBoolean( param );
}
ALint loki_alGetInteger( ALenum param )
{
   return OpenAL_alGetInteger( param );
}
ALfloat loki_alGetFloat( ALenum param )
{
   return OpenAL_alGetFloat( param );
}
ALdouble loki_alGetDouble( ALenum param )
{
   return OpenAL_alGetDouble( param );
}
ALvoid loki_alGetBooleanv( ALenum param, ALboolean* data )
{
   OpenAL_alGetBooleanv( param, data );
}
ALvoid loki_alGetIntegerv( ALenum param, ALint* data )
{
   OpenAL_alGetIntegerv( param, data );
}
ALvoid loki_alGetFloatv( ALenum param, ALfloat* data )
{
   OpenAL_alGetFloatv( param, data );
}
ALvoid loki_alGetDoublev( ALenum param, ALdouble* data )
{
   OpenAL_alGetDoublev( param, data );
}
const ALubyte* loki_alGetString( ALenum param )
{
   const ALubyte* string;

   switch (param) {
      case AL_EXTENSIONS:
         string = "AL_EXT_DYNAMIX";
         break;
      default:
         string = OpenAL_alGetString( param );
         break;
   }
   return string;
}
ALenum loki_alGetError( ALvoid )
{
   ALenum error;

   error = AL_NO_ERROR;
   error += OpenAL_alGetError( );
   error += OpenAL_alcGetError( );
   return error;
}
ALboolean loki_alIsExtensionPresent( const ALubyte* fname )
{
   ALboolean value;

   if ( dStrcmp(fname, "AL_EXT_DYNAMIX") == 0 ) {
      value = true;
   } else {
      value = OpenAL_alIsExtensionPresent( fname );
   }
   return value;
}
ALvoid* loki_alGetProcAddress( const ALubyte* fname )
{
   return OpenAL_alGetProcAddress( fname );
}
ALenum loki_alGetEnumValue( const ALubyte* ename )
{
   return OpenAL_alGetEnumValue( ename );
}
ALvoid loki_alListenerf( ALenum pname, ALfloat param )
{
   OpenAL_alListenerf( pname, param );
}
ALvoid loki_alListener3f( ALenum pname, ALfloat param1, ALfloat param2, ALfloat param3 )
{
   OpenAL_alListener3f( pname, param1, param2, param3 );
}
ALvoid loki_alListenerfv( ALenum pname, ALfloat* param )
{
   OpenAL_alListenerfv( pname, param );
}
ALvoid loki_alGetListeneri( ALenum pname, ALint* value )
{
   OpenAL_alGetListeneri( pname, value );
}
ALvoid loki_alGetListenerf( ALenum pname, ALfloat* values )
{
   OpenAL_alGetListenerf( pname, values );
}
ALvoid loki_alGetListenerfv( ALenum pname, ALfloat* values )
{
   OpenAL_alGetListenerfv( pname, values );
}
ALvoid loki_alGenSources( ALsizei n, ALuint* sources )
{
   OpenAL_alGenSources( n, sources );
}
ALvoid loki_alDeleteSources( ALsizei n, ALuint* sources )
{
   OpenAL_alDeleteSources( n, sources );
}
ALboolean loki_alIsSource( ALuint sid )
{
   return OpenAL_alIsSource( sid );
}
ALvoid loki_alSourcei( ALuint sid, ALenum param, ALint value )
{
   switch (param) {
      case AL_STREAMING:
      case AL_ENV_SAMPLE_DIRECT_EXT:
      case AL_ENV_SAMPLE_DIRECT_HF_EXT:
      case AL_ENV_SAMPLE_ROOM_EXT:
      case AL_ENV_SAMPLE_ROOM_HF_EXT:
      case AL_ENV_SAMPLE_OUTSIDE_VOLUME_HF_EXT:
      case AL_ENV_SAMPLE_FLAGS_EXT:
         // Not yet supported
         break;
      case AL_SOURCE_AMBIENT:
         // Special case, emulated by positioning relative sources
         if ( value ) {
            alSourcei(sid, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(sid, AL_POSITION, 0.0, 0.0, 0.0);
         } else {
            alSourcei(sid, AL_SOURCE_RELATIVE, AL_FALSE);
         }
         break;
      default:
         // Pass through to OpenAL 1.0
         OpenAL_alSourcei( sid, param, value );
         break;
   }
}
ALvoid loki_alSourcef( ALuint sid, ALenum param, ALfloat value )
{
   switch (param) {
      case AL_PAN:
      case AL_ENV_SAMPLE_REVERB_MIX_EXT:
      case AL_ENV_SAMPLE_OBSTRUCTION_EXT:
      case AL_ENV_SAMPLE_OBSTRUCTION_LF_RATIO_EXT:
      case AL_ENV_SAMPLE_OCCLUSION_EXT:
      case AL_ENV_SAMPLE_OCCLUSION_ROOM_RATIO_EXT:
      case AL_ENV_SAMPLE_ROOM_ROLLOFF_EXT:
      case AL_ENV_SAMPLE_AIR_ABSORPTION_EXT:
         // Not yet supported
         break;
      default:
         // Pass through to OpenAL 1.0
         OpenAL_alSourcef( sid, param, value );
         break;
   }
}
ALvoid loki_alSource3f( ALuint sid, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 )
{
   OpenAL_alSource3f( sid, param, v1, v2, v3 );
}
ALvoid loki_alSourcefv( ALuint sid, ALenum param, ALfloat* values )
{
   OpenAL_alSourcefv( sid, param, values );
}
ALvoid loki_alGetSourcei( ALuint sid, ALenum pname, ALint* value )
{
   switch (pname) {
      case AL_SOURCE_AMBIENT:
         // Special case, emulated by positioning relative sources
         alGetSourcei(sid, AL_SOURCE_RELATIVE, value);
         break;
      default:
         // Pass through to OpenAL 1.0
         OpenAL_alGetSourcei( sid, pname, value );
         break;
   }
}
ALvoid loki_alGetSourcef( ALuint sid, ALenum pname, ALfloat* value )
{
   OpenAL_alGetSourcef( sid, pname, value );
}
ALvoid loki_alGetSourcefv( ALuint sid, ALenum pname, ALfloat* values )
{
   OpenAL_alGetSourcefv( sid, pname, values );
}
ALvoid loki_alSourcePlayv( ALuint ns, ALuint* ids )
{
   OpenAL_alSourcePlayv( ns, ids );
}
ALvoid loki_alSourceStopv( ALuint ns, ALuint* ids )
{
   OpenAL_alSourceStopv( ns, ids );
}
ALvoid loki_alSourcePlay( ALuint sid )
{
   OpenAL_alSourcePlay( sid );
}
ALvoid loki_alSourcePause( ALuint sid )
{
   OpenAL_alSourcePause( sid );
}
ALvoid loki_alSourceStop( ALuint sid )
{
   OpenAL_alSourceStop( sid );
}
ALvoid loki_alGenBuffers( ALsizei n, ALuint* samples )
{
   OpenAL_alGenBuffers( n, samples );
}
ALvoid loki_alDeleteBuffers( ALsizei n, ALuint* samples )
{
   OpenAL_alDeleteBuffers( n, samples );
}
ALboolean loki_alIsBuffer( ALuint buffer )
{
   return OpenAL_alIsBuffer( buffer );
}
ALvoid loki_alBufferData( ALuint buffer, ALenum format, ALvoid* data, ALsizei size, ALsizei freq )
{
   OpenAL_alBufferData( buffer, format, data, size, freq );
}
ALsizei loki_alBufferAppendData( ALuint buffer, ALenum format, ALvoid* data, ALsizei size, ALsizei freq )
{
   return OpenAL_alBufferAppendData( buffer, format, data, size, freq );
}
ALvoid loki_alGetBufferi( ALuint buffer, ALenum param, ALint*   value )
{
   OpenAL_alGetBufferi( buffer, param, value );
}
ALvoid loki_alGetBufferf( ALuint buffer, ALenum param, ALfloat* value )
{
   OpenAL_alGetBufferf( buffer, param, value );
}

// ALC functions
ALvoid* loki_alcCreateContext( ALint* attrlist )
{
   return OpenAL10_alcCreateContext( dev, attrlist );
}
ALCenum loki_alcMakeContextCurrent( ALvoid* context )
{
   return OpenAL_alcMakeContextCurrent( context );
}
ALvoid* loki_alcUpdateContext( ALvoid* context )
{
   return OpenAL_alcUpdateContext( context );
}
ALCenum loki_alcDestroyContext( ALvoid* context )
{
   return OpenAL_alcDestroyContext( context );
}
ALCenum loki_alcGetError( ALvoid )
{
   return OpenAL_alcGetError( );
}
const ALubyte * loki_alcGetErrorString( ALvoid )
{
   return OpenAL_alcGetErrorString( );
}
ALvoid* loki_alcGetCurrentContext( ALvoid )
{
   return OpenAL_alcGetCurrentContext( );
}

// ALUT functions
void loki_alutInit( int* argc, char** argv )
{
   char devstr_buf[1024];
   const char *devstr;
   const char *paren;
   int refresh, frequency;
   ALint attrlist[5];

   // Get some basic configuration variables
   refresh = Con::getIntVariable( "$pref::Audio::refresh", 15 );
   frequency = Con::getIntVariable( "$pref::Audio::frequency", 22050 );

   // Open the audio device
   devstr = Con::getVariable("$pref::OpenAL::driver");
   if ( devstr && ((paren=strrchr(devstr, ')')) != NULL) ) {
      // Make sure a sane sampling rate is specified
      if ( strstr(devstr, "sampling-rate") == NULL ) {
         strcpy(devstr_buf, devstr);
         sprintf(&devstr_buf[paren-devstr],
                 " (sampling-rate %d ))", frequency);
         devstr = devstr_buf;
      }
   } else {
      sprintf(devstr_buf, "'( (sampling-rate %d ))", frequency);
      devstr = devstr_buf;
   }
   dev = OpenAL_alcOpenDevice( (const ALubyte *) devstr );

   if( dev == NULL ) {
      Con::errorf( ConsoleLogEntry::General, "Audio device open failed..." );
      return;
   }

   attrlist[0] = ALC_FREQUENCY;
   attrlist[1] = frequency;
   attrlist[2] = ALC_REFRESH;
   attrlist[3] = refresh;
   attrlist[4] = 0;

   context_id = alcCreateContext( attrlist );

   if( context_id == NULL ) {
      Con::errorf( ConsoleLogEntry::General, "Audio context creation failed, exiting..." );
      Platform::forceShutdown( 1 );
      return;
   }

   alcMakeContextCurrent( context_id );

   // Set the distance attenuation model
   OpenAL_alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
}
void loki_alutExit( ALvoid )
{
   if( context_id != NULL ) {
      alcDestroyContext( context_id );
      OpenAL_alcCloseDevice( dev );
      context_id = NULL;
   }
}
ALboolean loki_alutLoadWAV( const char* fname, ALvoid** data, ALsizei* format, ALsizei* size, ALsizei* bits, ALsizei* freq )
{
   return OpenAL_alutLoadWAV( fname, data, format, size, bits, freq );
}

// Extensions
ALvoid loki_alGenEnvironmentIASIG( ALsizei n, ALuint* environs )
{
   OpenAL_alGenEnvironmentIASIG( n, environs );
}
ALvoid loki_alDeleteEnvironmentIASIG( ALsizei n, ALuint* environs )
{
   OpenAL_alDeleteEnvironmentIASIG( n, environs );
}
ALboolean loki_alIsEnvironmentIASIG( ALuint environment )
{
   return OpenAL_alIsEnvironmentIASIG( environment );
}
ALvoid loki_alEnvironmentiIASIG( ALuint eid, ALenum param, ALint value )
{
   OpenAL_alEnvironmentiIASIG( eid, param, value );
}

ALboolean loki_alBufferi_EXT( ALuint buffer, ALenum pname, ALint value )
{
   ALboolean status;

   alGetError();
   switch (pname) {
      case AL_BUFFER_KEEP_RESIDENT:
         status = false;
         break;
      default:
         OpenAL_alBufferi_EXT( buffer, pname, value );
         status = (alGetError() == AL_NO_ERROR);
         break;
   }
   return status;
}
ALboolean loki_alBufferSyncData_EXT( ALuint buffer, ALenum format, ALvoid* data, ALsizei size, ALsizei freq )
{
   ALboolean status;

   // alBufferData doesn't return a value,
   // so clear the error setting, execute,
   // and check again.
   alGetError( );

   alBufferData(buffer, format, data, size, freq);

   if( alGetError( ) == AL_NO_ERROR ) {
      // let AL manage the memory
      dFree( data );
      status = true;
   } else {
      status = false;
   }
   return status;
}
ALboolean loki_alBufferStreamFile_EXT( ALuint buffer, const ALubyte* filename )
{
   File file;

   file.open( (const char*) filename, File::Read );

   if( file.getStatus( ) != File::Ok ) {
      return AL_FALSE;
   }

   U32 size = file.getSize( );
   char* data = (char*) dMalloc( size );

   if( data == NULL ) {
      // technically, 'file' will go out of scope and
      // close itself, but still...
      file.close( );

      return AL_FALSE;
   }

   file.read( size, data );

   if( file.getStatus( ) != File::Ok ) {
      file.close( );
      dFree( data );
      return AL_FALSE;
   }

   U32 ext = dStrlen( filename );
   ext -= 3;
   ext = ( ext >= 0 ) ? ext : 0;
   ALuint ok = AL_FALSE;

   if( dStrnicmp( &filename[ ext ], "mp3", 3 ) != 0 ) {
      alGetError();

      alBufferData( buffer, AL_FORMAT_WAVE_EXT, data, size, 0 );

      ok = (alGetError() == AL_NO_ERROR);

   } else {
      ok = alutLoadMP3_LOKI( buffer, data, size );
   }
   
   dFree( data );
   file.close( );
   
   if( ok == AL_FALSE ) {
      Con::warnf( ConsoleLogEntry::General,
             "could not buffer and convert %s", filename );

      return AL_FALSE;
   }

   return AL_TRUE;
}
ALboolean loki_alSourceCallback_EXT( ALuint source, ALvoid* callback )
{
   fakeCallbackPair_t pusher;

   if( alIsSource( source ) == AL_FALSE) {
      Con::errorf(ConsoleLogEntry::General,
         "alSourceCallback_EXT: %d not a source id", source );
      return;
   }

   pusher.sid  = source;
   pusher.proc = callback;

   fakeCallbackFake.push_back( pusher );
}
ALvoid loki_alSourceResetEnvironment_EXT( ALuint source )
{
   return; // Nothing we can do here yet...
}
ALboolean loki_alContexti_EXT( ALenum pname, ALint value )
{
   return false;
}
ALboolean loki_alGetContexti_EXT( ALenum pname, ALint* value )
{
   ALboolean status;

   status = false;
   if( value ) {
      switch( pname ) {
      case ALC_PROVIDER:
         *value = 0;
         status = true;
         break;
      case ALC_PROVIDER_COUNT:
         *value = 1;
         status = true;
         break;
      case ALC_SPEAKER:
         *value = 0;
         status = true;
         break;
      case ALC_SPEAKER_COUNT:
         *value = 1;
         status = true;
         break;
      case ALC_BUFFER_MEMORY_USAGE:
         *value = 0;
         status = true;
         break;
      case ALC_BUFFER_MEMORY_SIZE:
         *value = 1 << 16;
         status = true;
         break;
      case ALC_BUFFER_MEMORY_COUNT:
         *value = 0;
         status = true;
         break;
      case ALC_BUFFER_COUNT:
         *value = 0;
         status = true;
         break;
      default:
         // error
         break;
      }
   }
   return status;
}
ALboolean loki_alGetContextstr_EXT( ALenum pname, ALuint idx, ALubyte** value )
{
   ALboolean status;

   status = false;
   if( value ) {
      switch( pname ) {
      case ALC_PROVIDER_NAME:
         *value = "Loki Software, Inc.";
         status = true;
         break;
      case ALC_SPEAKER_NAME:
         *value = "Stereo, 2-Speaker";
         status = true;
         break;
      default:
         // error
         break;
      }
   }
   return status;
}
ALboolean loki_alCaptureInit_EXT( ALenum format, ALuint rate, ALsizei bufferSize )
{
   return OpenAL_alCaptureInit_EXT( format, rate, bufferSize );
}
ALboolean loki_alCaptureDestroy_EXT( ALvoid )
{
   return OpenAL_alCaptureDestroy_EXT( );
}
ALboolean loki_alCaptureStart_EXT( ALvoid )
{
   return OpenAL_alCaptureStart_EXT( );
}
ALboolean loki_alCaptureStop_EXT( ALvoid )
{
   return OpenAL_alCaptureStop_EXT( );
}
ALsizei loki_alCaptureGetData_EXT( ALvoid* data, ALsizei n, ALenum format, ALuint rate )
{
   return OpenAL_alCaptureGetData_EXT( data, n, format, rate );
}
ALvoid loki_alEnvironmentfIASIG( ALuint eid, ALenum param, ALfloat value )
{
   return; // Nothing we can do here yet...
}
ALvoid loki_alGetEnvironmentiIASIG_EXT( ALuint eid, ALenum param, ALint * value )
{
   return; // Nothing we can do here yet...
}
ALvoid loki_alGetEnvironmentfIASIG_EXT( ALuint eid, ALenum param, ALfloat * value )
{
   return; // Nothing we can do here yet...
}

// DLL's: ------------------------------------------------------------------
static bool findExtension( const char* name )
{
   bool result = false;
   if (alGetString != NULL)
   {
      result = dStrstr( (const char*)alGetString(AL_EXTENSIONS), name) != NULL;
   }
   return result;
}

static bool bindFunction( void *&fnAddress, const char *name )
{
   fnAddress = dlsym( hinstOpenAL, name );
#ifdef DEBUG_OPENAL
   if( !fnAddress ) {
      Con::errorf(ConsoleLogEntry::General, " Missing OpenAL function '%s'", name);
   }
#endif
   return (fnAddress != NULL);
}

static void bindExtension( void *&fnAddress, const char *name, bool &supported )
{
   if (supported)
   {
      bindFunction(fnAddress, name);
#if 0
      if (fnAddress == NULL)
         supported = false;
#endif
   }
   else
      fnAddress = NULL;
}

static bool bindDLLFunctions()
{
   bool result = true;
   /* It's okay if the following functions are not found:
      alGetBoolean()
      alGetInteger()
      alGetFloat()
      alGetDouble()
      alcUpdateContext()
      alcGetErrorString()
   */
   #define AL_EXTENSION(ext_name) \
      gDoesSupport_##ext_name = findExtension(#ext_name);
   #define AL_FUNCTION(fn_return,fn_name,fn_args) \
   { const char *name = #fn_name; \
      /* Do some renaming of functions to match OpenAL 1.0 API */ \
      if ( dStrcmp(name, "alGetListeneri") == 0 ) \
         name = "alGetListeneriv"; \
      else \
      if ( dStrcmp(name, "alGetListenerf") == 0 ) \
         name = "alGetListenerfv"; \
      else \
      if ( dStrcmp(name, "alGetSourcei") == 0 ) \
         name = "alGetSourceiv"; \
      else \
      if ( dStrcmp(name, "alGetSourcef") == 0 ) \
         name = "alGetSourcefv"; \
      else \
      if ( dStrcmp(name, "alBufferi_EXT") == 0 ) \
         name = "alBufferi_LOKI"; \
      result &= bindFunction( *(void**)&OpenAL_##fn_name, name); \
   }
   #define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) \
   { const char *name = #fn_name; \
      /* Do some renaming of functions to match OpenAL 1.0 API */ \
      if ( dStrcmp(name, "alBufferi_EXT") == 0 ) \
         name = "alBufferi_LOKI"; \
      bindExtension( *(void**)&OpenAL_##fn_name, name, gDoesSupport_##ext_name); \
   }
   AL_FUNCTION(ALCdevice *, alcOpenDevice, ( const ALubyte *tokstr ));
   AL_FUNCTION(ALvoid, alcCloseDevice, ( ALCdevice *dev ));
   AL_FUNCTION(ALvoid, alDistanceModel, ( ALenum distanceModel ));
   result &= bindFunction( *(void**)&OpenAL10_alcCreateContext, "alcCreateContext");
#include "lib/openal/win32/openALFn.h"
   return result;
}

bool bindOpenALFunctions(void)
{
   hinstOpenAL = dlopen( "./libopenal.so.0", RTLD_NOW );
   if(hinstOpenAL == NULL)
   {
      return false;
   }

   // Set the Loki OpenAL function pointers (needed for extension loading)
   #define AL_EXTENSION(ext_name) gDoesSupport_##ext_name = false;
   #define AL_FUNCTION(fn_return,fn_name,fn_parms) fn_name = loki_##fn_name;
   #define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) fn_name = loki_##fn_name;
#include "lib/openal/win32/openALFn.h"

   // Load the real OpenAL functions from the shared library
   bindDLLFunctions();

   // Yay, we're done!
   return true;
}

void unbindOpenALFunctions(void)
{
   if (hinstOpenAL)
      dlclose(hinstOpenAL);
   hinstOpenAL = NULL;
}

#endif // DEDICATED
