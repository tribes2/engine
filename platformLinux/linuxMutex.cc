//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platformMutex.h"

#include <pthread.h>

void* Mutex::createMutex( void )
{
	pthread_mutex_t* mutex = new pthread_mutex_t;

	if( mutex == 0 ) {
		return 0;
	}

	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );

	if( pthread_mutex_init( mutex, &attr ) != 0 ) {
		delete mutex;
		return 0;
	}

	return mutex;
}

void Mutex::destroyMutex( void* mutex )
{

	if( mutex ) {
		pthread_mutex_destroy( (pthread_mutex_t*) mutex );
		delete mutex;
	}

}

void Mutex::lockMutex( void* mutex )
{

	if( mutex ) {
		pthread_mutex_lock( (pthread_mutex_t*) mutex );
	}

}

void Mutex::unlockMutex( void* mutex )
{

	if( mutex ) {
		pthread_mutex_unlock( (pthread_mutex_t*) mutex );
	}

}
