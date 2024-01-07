//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformAL.h"
#include "platformLinux/platformLinux.h"
#include "Platform/platformAudio.h"


static const AudioEnvironment *mCurrentEnvironment;                            // the last environment set

void* alGetProcAddress( const ALubyte* )
{
	return 0;
}

ALenum alGetError( ALvoid )
{
	return AL_NO_ERROR;
}

void alGetSourceiv( ALuint sid, ALenum param, ALint* value )
{
	// empty
}

ALboolean alIsBuffer( ALuint bid )
{
	return AL_FALSE;
}

void alGenBuffers( ALsizei n, ALuint* samples )
{
	// empty
}

void alDeleteBuffers( ALsizei n, ALuint* samples )
{
	// empty
}

void alListeneriv( int pname, int value )
{
	// empty
}

ALboolean alIsSource( ALuint sid )
{
	return AL_FALSE;
}

void alBufferData( ALuint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq )
{
	// emtpy
}

void alDistanceModel( ALenum distanceModel )
{
	// empty
}

ALCdevice* alcOpenDevice( const ALubyte* s )
{
	return 0;
}

ALvoid* alcCreateContext( ALCdevice* c, ALint* a )
{
	return 0;
}

ALCenum alcMakeContextCurrent( ALvoid* c )
{
	return AL_FALSE;
}

ALCenum alcDestroyContext( ALvoid* c )
{
	return AL_FALSE;
}

void alcCloseDevice( ALCdevice* d )
{
	// empty
}

namespace Audio {

	void init( void ) {
		// empty
	}

	void destroy( void ) {
		// empty
	}

}

AUDIOHANDLE alxCreateSource( const Audio::Description* descObject,
			     const char* filename,
			     const MatrixF* transform,
			     EffectDescription* effect,
                             AudioSampleEnvironment* sampleEnvironment )
{
	// empty
	return 0;
}

AUDIOHANDLE alxCreateSource( AudioDescription* descObject,
			     const char* filename,
			     const MatrixF* transform,
			     EffectDescription* effect,
                             AudioSampleEnvironment* sampleEnvironment )
{
	// empty
	return 0;
}

AUDIOHANDLE alxCreateSource( const AudioProfile* profile, const MatrixF* transform )
{
	// empty
	return 0;
}

ALuint alxFindSource( AUDIOHANDLE handle )
{
	// empty
	return 0;
}


bool alxIsPlaying(AUDIOHANDLE handle)
{
	// empty
	return(false);
}

ALuint alxGetWaveLen( ALuint buffer )
{
	// empty
	return 0;
}

void alxStop( AUDIOHANDLE handle )
{
	// empty
}

AUDIOHANDLE alxPlay( AUDIOHANDLE handle )
{
	// empty
	return 0;
}

AUDIOHANDLE alxPlay( const AudioProfile* profile, const MatrixF* transform, const Point3F* velocity )
{
	// empty
	return 0;
}

void alxSourcef( AUDIOHANDLE handle, ALenum pname, ALfloat value )
{
	// empty
}

void alxSourcefv( AUDIOHANDLE handle, ALenum pname, ALfloat* values )
{
	// empty
}

void alxSource3f( AUDIOHANDLE handle, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3 )
{
	// empty
}

void alxSourcei( AUDIOHANDLE handle, ALenum pname, ALint value )
{
	// empty
}

void alxGetSourcefv( AUDIOHANDLE handle, ALenum pname, ALfloat* values )
{
	// empty
}

void alxGetSourcei( AUDIOHANDLE handle, ALenum pname, ALint* value )
{
	// empty
}

void alxSourceMatrixF( AUDIOHANDLE handle, const MatrixF* transform )
{
	// empty
}

void alxListenerMatrixF( const MatrixF* transform )
{
	// empty
}


void alxListener3f( ALenum pname, ALfloat x, ALfloat y, ALfloat z )
{
	// empty
}

void alxGetListener3f(ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
	// empty
}

void alxEnvironmentf(ALenum pname, ALfloat value)
{
	// empty
}

void alxSetEnvironment(const AudioEnvironment * env)
{
	mCurrentEnvironment = env;
}

const AudioEnvironment * alxGetEnvironment()
{
	return(mCurrentEnvironment);
}

void alxUpdate( )
{
	// empty
}

struct SimVoiceStreamEvent;
struct SimVoiceEvent;

void alxReceiveVoiceStream( SimVoiceStreamEvent* event )
{
	// empty
}

void alxReceiveVoiceEvent( SimVoiceEvent* event )
{
	// empty
}
