//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef DEDICATED

#include <SDL/SDL.h>

#include "platformLinux/platformLinux.h"
#include "Platform/platformRedBook.h"
#include "PlatformWin32/platformAL.h"

class LinuxRedBookDevice : public RedBookDevice
{
private:
	int mDeviceId;
	SDL_CD* cdrom;

	void setLastError( const char* );

public:
	LinuxRedBookDevice( int id = -1 );
	~LinuxRedBookDevice( void );

	U32 getDeviceId( void );

	bool open( void );
	bool close( void );
	bool play( U32 );
	bool stop( void );
	bool getTrackCount( U32* );
	bool getVolume( F32* );
	bool setVolume( F32 );
};

static ALfloat (*alcGetAudioChannel)( ALuint channel ) = 0;
static void (*alcSetAudioChannel)( ALuint channel, ALfloat value ) = 0;

void installRedBookDevices( void )
{

	if( SDL_Init( SDL_INIT_CDROM ) < 0 ) {
		return;
	}

	int numCDs = SDL_CDNumDrives( );

	if( numCDs <= 0 ) {
		return;
	}

	for( int i = 0; i < numCDs; i++ ) {
		SDL_CD* temp = SDL_CDOpen( i );

		if( temp ) {
			LinuxRedBookDevice* device = new LinuxRedBookDevice( i );
			const char* name = SDL_CDName( i );
			device->mDeviceName = new char[ dStrlen( name ) + 1 ];
			dStrcpy( device->mDeviceName, name );

			RedBook::installDevice( device );
			SDL_CDClose( temp );
		}

	}

}

LinuxRedBookDevice::LinuxRedBookDevice( int id )
	: mDeviceId( id ), cdrom( 0 )
{
	// empty
}

LinuxRedBookDevice::~LinuxRedBookDevice( void )
{
	close( );
}

U32 LinuxRedBookDevice::getDeviceId( void )
{
	return static_cast<U32>( mDeviceId );
}

bool LinuxRedBookDevice::open( void )
{

	if( mAcquired ) {
		setLastError( "already open" );
		return false;
	}

	cdrom = SDL_CDOpen( mDeviceId );

	if( !cdrom ) {
		setLastError( "open failed" );
		return false;
	}

	mAcquired = true;
	return true;
}

bool LinuxRedBookDevice::close( void )
{

	if( !mAcquired ) {
		setLastError( "not acquired" );
		return false;
	}

	stop( );

	if( cdrom ) {
		SDL_CDClose( cdrom );
		cdrom = 0;
	}

	mAcquired = false;
	return true;
}

bool LinuxRedBookDevice::play( U32 track )
{

	if( !mAcquired ) {
		setLastError( "not acquired" );
		return false;
	}

	if( track >= cdrom->numtracks ) {
		setLastError( "track out of range" );
		return false;
	}

	if( CD_INDRIVE( SDL_CDStatus( cdrom ) ) ) {

		if( SDL_CDPlayTracks( cdrom, track, 0, 0, cdrom->track[track].length ) ) {
			setLastError( SDL_GetError( ) );
			return false;
		}

	}

	return true;
}

bool LinuxRedBookDevice::stop( void )
{

	if( !mAcquired ) {
		setLastError( "not acquired" );
		return false;
	}

	if( SDL_CDStop( cdrom ) ) {
		setLastError( SDL_GetError( ) );
		return false;
	}

	return true;
}

bool LinuxRedBookDevice::getTrackCount( U32* numTracks )
{

	if( !mAcquired ) {
		setLastError( "not acquried" );
		return false;
	}

	return cdrom->numtracks;
}

bool LinuxRedBookDevice::getVolume( F32* volume )
{

	if( volume && alcGetAudioChannel ) {
		*volume = alcGetAudioChannel( ALC_CHAN_CD_LOKI );
		return true;
	}

	return false;
}

bool LinuxRedBookDevice::setVolume( F32 volume )
{

	if( alcSetAudioChannel ) {
		alcSetAudioChannel( ALC_CHAN_CD_LOKI, volume );
		return true;
	}

	return false;
}

void LinuxRedBookDevice::setLastError( const char* error )
{
	RedBook::setLastError( error );
}

#endif
