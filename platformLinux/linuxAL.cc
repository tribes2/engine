//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformAL.h"
#include "console/console.h"
#include "Core/fileio.h"

#include <stdio.h>

ALuint alBufferSyncData_EXT(ALuint bid,
			    ALenum format,
			    ALvoid *data,
			    ALsizei size,
			    ALsizei frequency)
{
	ALuint r = 0;

	// alBufferData doesn't return a value,
	// so clear the error setting, execute,
	// and check again.
	alGetError( );

	alBufferData(bid, format, data, size, frequency);

	if( alGetError( ) == AL_NO_ERROR ) {
		// let AL manage the memory
		dFree( data );
		r = 1;
	}

	return r;
}

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
			pi.proc( pi.sid, false );

      			Con::printf( "Calling callback for %d", pi.sid);
		} else {
			fakeCallbackFake.push_front( pi );
		}
	}

	return;
}

void alSourceCallback_EXT(ALuint sid, void (*dummy)(U32, bool))
{
	fakeCallbackPair_t pusher;

	if( alIsSource( sid ) == AL_FALSE) {
      		Con::errorf(ConsoleLogEntry::General,
			"alSourceCallback_EXT: %d not a source id", sid );
		return;
	}

	pusher.sid  = sid;
	pusher.proc = dummy;

      	Con::printf( "adding Faker sid %d", sid);

	fakeCallbackFake.push_back( pusher );

	return;
}

ALuint alBufferStreamFile_EXT( ALuint bid, const ALubyte* filename )
{
	static ALboolean (*LoadMP3)(ALuint bid, ALvoid *data, ALint size) = NULL;
		
#define GP(x) alGetProcAddress((const ALubyte *) x)

	if(LoadMP3 == NULL) {
		LoadMP3 = (ALboolean (*)(ALuint, ALvoid *, ALint)) GP("alutLoadMP3_LOKI");
		if(LoadMP3 == NULL) {
			Con::warnf( ConsoleLogEntry::General,
				    "could not get alutLoadMP3_LOKI, no mp3 support" );
		}
	}
#undef GP

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
	ALuint err = AL_FALSE;

	if( dStrnicmp( &filename[ ext ], "mp3", 3 ) != 0 ) {
		alGetError();

		alBufferData( bid, AL_FORMAT_WAVE_EXT, data, size, 0 );

		err = (alGetError() == AL_NO_ERROR);

	} else {

		if( LoadMP3 ) {
			err = LoadMP3( bid, data, size );
		} else {
			Con::warnf( ConsoleLogEntry::General,
				    "cannot load MP3 file, no mp3 support present" );
			err = AL_FALSE;
		}
		
	}
	
	dFree( data );
	file.close( );
	
	if( err == AL_FALSE ) {
		Con::warnf( ConsoleLogEntry::General,
			    "could not buffer and convert MP3 file" );

		return AL_FALSE;
	}

	return AL_TRUE;
}

static void *context_id;
static ALCdevice *dev;

extern void alutInitFake( int *argc, char *argv[] )
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
	dev = alcOpenDevice( (const ALubyte *) devstr );

	// If there's no audio, use a null device
	if( dev == NULL ) {
		const char *nulldev = "'( ( devices '( null )))";
		dev = alcOpenDevice( (const ALubyte *) nulldev );
	}

	if( dev == NULL ) {
		Con::errorf( ConsoleLogEntry::General, "Audio device open failed, exiting..." );
		Platform::forceShutdown( 1 );
		return;
	}

	attrlist[0] = ALC_FREQUENCY;
	attrlist[1] = frequency;
	attrlist[2] = ALC_REFRESH;
	attrlist[3] = refresh;
	attrlist[4] = 0;

	context_id = alcCreateContext( dev, attrlist );

	if( context_id == NULL ) {
		Con::errorf( ConsoleLogEntry::General, "Audio context creation failed, exiting..." );
		Platform::forceShutdown( 1 );
		return;
	}

	alcMakeContextCurrent( context_id );

	// Set the distance attenuation model
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

	return;
}

extern void alutExitFake( void )
{

	if( context_id == NULL ) {
		return;
	}

	alcDestroyContext( context_id );
	alcCloseDevice( dev );

	return;
}

void alBufferi_EXT( ALuint bid, ALenum pname, ALint value )
{
	// ignore
}

void alContexti_EXT( ALenum pname, ALint value )
{
	// ignore.
}

void alGetContexti_EXT( ALenum pname, ALint* value )
{

	if( !value ) {
		return;
	}

	switch( pname ) {
	case ALC_PROVIDER:
		*value = 0;
		break;
	case ALC_PROVIDER_COUNT:
		*value = 1;
		break;
	case ALC_SPEAKER:
		*value = 0;
		break;
	case ALC_SPEAKER_COUNT:
		*value = 1;
		break;
	case ALC_BUFFER_MEMORY_USAGE:
		*value = 0;
		break;
	case ALC_BUFFER_MEMORY_SIZE:
		*value = 1 << 16;
		break;
	case ALC_BUFFER_MEMORY_COUNT:
		*value = 0;
		break;
	case ALC_BUFFER_COUNT:
		*value = 0;
		break;
	default:
		// error
		break;
	}

}

void alGetContextstr_EXT( ALenum pname, ALuint idx, ALubyte** value )
{

	if( !value ) {
		return;
	}

	switch( pname ) {
	case ALC_PROVIDER_NAME:
		*value = "Loki Software, Inc.";
		break;
	case ALC_SPEAKER_NAME:
		*value = "Stereo, 2-Speaker";
		break;
	default:
		// error
		break;
	}

}

void alGetEnvironmentiIASIG_EXT( ALuint eid, ALenum param, ALint * value )
{
	// Nothing we can do here yet...
}

void alGetEnvironmentfIASIG_EXT( ALuint eid, ALenum param, ALfloat * value )
{
	// Nothing we can do here yet...
}

void alSourceResetEnvironment_EXT( ALuint source )
{
        // Nothing we can do here yet...
}

