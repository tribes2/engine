//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/platformX86UNIX.h"
#include "console/console.h"
#define AL_NO_PROTOTYPES
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

// Stub functions: ---------------------------------------------------------
// AL:
ALvoid stub_alEnable( ALenum ) {}
ALvoid stub_alDisable( ALenum ) {}
ALboolean stub_alIsEnabled( ALenum ) {return(AL_FALSE);}
ALvoid stub_alHint( ALenum , ALenum ) {}
ALboolean stub_alGetBoolean( ALenum ) {return(AL_FALSE);}
ALint stub_alGetInteger( ALenum ) {return(0);}
ALfloat stub_alGetFloat( ALenum ) {return(0.f);}
ALdouble stub_alGetDouble( ALenum ) {return(0.f);}
ALvoid stub_alGetBooleanv( ALenum, ALboolean* ) {}
ALvoid stub_alGetIntegerv( ALenum, ALint* ) {}
ALvoid stub_alGetFloatv( ALenum, ALfloat* ) {}
ALvoid stub_alGetDoublev( ALenum, ALdouble* ) {}
const ALubyte* stub_alGetString( ALenum pname )
{
   switch(pname)
   {
      case AL_VENDOR:      return (ALubyte*)"None";
      case AL_VERSION:     return (ALubyte*)"OpenAL 0.1";
      case AL_RENDERER:    return (ALubyte*)"None";
      case AL_EXTENSIONS:  return (ALubyte*)"";
   }
   return(0);
}
ALenum stub_alGetError( ALvoid ) {return(0);}
ALboolean stub_alIsExtensionPresent( const ALubyte* ) {return(AL_FALSE);}
ALvoid* stub_alGetProcAddress( const ALubyte* ) {return(0);}
ALenum stub_alGetEnumValue( const ALubyte* ) {return(0);}
ALvoid stub_alListenerf( ALenum, ALfloat ) {}
ALvoid stub_alListener3f( ALenum, ALfloat, ALfloat, ALfloat ) {}
ALvoid stub_alListenerfv( ALenum, ALfloat* ) {}
ALvoid stub_alGetListeneri( ALenum, ALint* ) {}
ALvoid stub_alGetListenerf( ALenum, ALfloat* ) {}
ALvoid stub_alGetListenerfv( ALenum, ALfloat* ) {}
ALvoid stub_alGenSources( ALsizei, ALuint* ) {}
ALvoid stub_alDeleteSources( ALsizei, ALuint* ) {}
ALboolean stub_alIsSource( ALuint ) {return(AL_FALSE);}
ALvoid stub_alSourcei( ALuint, ALenum, ALint ) {}
ALvoid stub_alSourcef( ALuint, ALenum, ALfloat ) {}
ALvoid stub_alSource3f( ALuint, ALenum, ALfloat, ALfloat, ALfloat ) {}
ALvoid stub_alSourcefv( ALuint, ALenum, ALfloat* ) {}
ALvoid stub_alGetSourcei( ALuint, ALenum, ALint* ) {}
ALvoid stub_alGetSourcef( ALuint, ALenum, ALfloat* ) {}
ALvoid stub_alGetSourcefv( ALuint, ALenum, ALfloat* ) {}
ALvoid stub_alSourcePlayv( ALuint, ALuint* ) {}
ALvoid stub_alSourceStopv( ALuint, ALuint* ) {}
ALvoid stub_alSourcePlay( ALuint ) {}
ALvoid stub_alSourcePause( ALuint ) {}
ALvoid stub_alSourceStop( ALuint ) {}
ALvoid stub_alGenBuffers( ALsizei, ALuint* ) {}
ALvoid stub_alDeleteBuffers( ALsizei, ALuint* ) {}
ALboolean stub_alIsBuffer( ALuint ) {return(AL_FALSE);}
ALvoid stub_alBufferData( ALuint, ALenum, ALvoid*, ALsizei, ALsizei ) {}
ALsizei stub_alBufferAppendData( ALuint, ALenum, ALvoid*, ALsizei, ALsizei ) {return(0);}
ALvoid stub_alGetBufferi( ALuint, ALenum, ALint* ) {}
ALvoid stub_alGetBufferf( ALuint, ALenum, ALfloat* ) {}

// ALC:
ALvoid* stub_alcCreateContext( ALint *) {return(0);}
ALCenum stub_alcMakeContextCurrent( ALvoid *) {return(ALC_INVALID);}
ALvoid* stub_alcUpdateContext( ALvoid * ) {return(0);}
ALCenum stub_alcDestroyContext( ALvoid * ) {return(ALC_INVALID);}
ALCenum stub_alcGetError( ALvoid ) {return(ALC_INVALID);}
const ALubyte* stub_alcGetErrorString( ALvoid ) {return(0);}
ALvoid* stub_alcGetCurrentContext( ALvoid ) {return(0);}

// ALUT:
void stub_alutInit( int *, char ** ) {}
void stub_alutExit( ALvoid ) {}
ALboolean stub_alutLoadWAV( const char *, ALvoid **, ALsizei *, ALsizei *, ALsizei *, ALsizei *) {return(AL_FALSE);}

// Extension: IASIG
ALvoid stub_alGenEnvironmentIASIG( ALsizei, ALuint* ) {}
ALvoid stub_alDeleteEnvironmentIASIG( ALsizei, ALuint*) {}
ALboolean stub_alIsEnvironmentIASIG( ALuint ) {return(AL_FALSE);}
ALvoid stub_alEnvironmentiIASIG( ALuint, ALenum, ALint ) {}
ALvoid stub_alEnvironmentfIASIG( ALuint, ALenum, ALfloat ) {}
ALvoid stub_alGetEnvironmentiIASIG_EXT( ALuint, ALenum, ALint * ) {}
ALvoid stub_alGetEnvironmentfIASIG_EXT( ALuint, ALenum, ALfloat * ) {}

// Extension: Dynamix
ALboolean stub_alBufferi_EXT( ALuint, ALenum, ALint ) { return(AL_FALSE); }
ALboolean stub_alBufferSyncData_EXT( ALuint, ALenum, ALvoid *, ALsizei, ALsizei ) {return(AL_FALSE); }
ALboolean stub_alBufferStreamFile_EXT( ALuint, const ALubyte * ) { return(AL_FALSE); }
ALboolean stub_alSourceCallback_EXT( ALuint, ALvoid * ) { return(AL_FALSE); }
ALvoid stub_alSourceResetEnvironment_EXT( ALuint ) {}
ALboolean stub_alContexti_EXT( ALenum, ALint) { return(AL_FALSE); }
ALboolean stub_alGetContexti_EXT( ALenum, ALint * ) { return(AL_FALSE); }
ALboolean stub_alGetContextstr_EXT( ALenum, ALuint, ALubyte ** ) { return(AL_FALSE); }
ALboolean stub_alCaptureInit_EXT( ALenum, ALuint, ALsizei ) { return(AL_FALSE); }
ALboolean stub_alCaptureDestroy_EXT( ALvoid ) { return(AL_FALSE); }
ALboolean stub_alCaptureStart_EXT( ALvoid ) { return(AL_FALSE); }
ALboolean stub_alCaptureStop_EXT( ALvoid ) { return(AL_FALSE); }
ALsizei stub_alCaptureGetData_EXT( ALvoid *, ALsizei, ALenum, ALuint ) { return(AL_FALSE); }

// declare OpenAL functions
#define AL_EXTENSION(ext_name) bool gDoesSupport_##ext_name = false;
#define AL_FUNCTION(fn_return,fn_name,fn_args) fn_return (*fn_name)fn_args = stub_##fn_name;
#define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) fn_return (*fn_name)fn_args = stub_##fn_name;
#include <openALFn.h>

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

// Stub: -------------------------------------------------------------------
static void bindStubFunctions()
{
   #define AL_EXTENSION(ext_name) gDoesSupport_##ext_name = false;
   #define AL_FUNCTION(fn_return,fn_name,fn_parms) fn_name = stub_##fn_name;
   #define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) fn_name = stub_##fn_name;
   #include <openALFn.h>
}

namespace Audio
{

static bool sStaticLibrary;

void libraryShutdown()
{
   bindStubFunctions();   
   sStaticLibrary = true;
}   

bool libraryInit(const char *library)
{
   libraryShutdown();

   if(!library || !library[0])
      return(false);

   libraryShutdown();
   return(false);
}

void libraryInitExtensions()
{
   // static library extensions are bound on libraryInit (need to be defined anyways...)
   if(sStaticLibrary)
   {
      #define AL_EXTENSION(ext_name) gDoesSupport_##ext_name = findExtension( #ext_name );
      #include <openALFn.h>
   }
   else
   {
      #define AL_EXTENSION(ext_name) gDoesSupport_##ext_name = findExtension( #ext_name );
//      #define AL_EXT_FUNCTION(ext_name,fn_return,fn_name,fn_args) bindExtension(*(void**)&fn_name, #fn_name, gDoesSupport_##ext_name );
      #include <openALFn.h>
   }
}

} // end namespace Audio
