//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platformSemaphore.h"

#include <semaphore.h>

void* Semaphore::createSemaphore( U32 initialCount )
{
	sem_t* sem = new sem_t;

	if( sem ) {

		if( sem_init( sem, 0, initialCount ) ) {
			delete sem;
			return 0;
		}

	}

	return sem;
}

void Semaphore::destroySemaphore( void* semaphore )
{

	if( semaphore ) {
		sem_t* sem = (sem_t*) semaphore;

		sem_destroy( sem );
		delete sem;
	}

}

bool Semaphore::acquireSemaphore( void* semaphore, bool block )
{
	bool r = false;

	if( !semaphore ) {
		return false;
	}

	sem_t* sem = (sem_t*) semaphore;

	if( block ) {
		sem_wait( sem );
		r = true;
	} else {

		if( sem_trywait( sem ) == 0 ) {
			r = true;
		} else {
			r = false;
		}

	}

	return r;
}

void Semaphore::releaseSemaphore( void* semaphore )
{

	if( semaphore ) {
		sem_post( (sem_t*) semaphore );
	}

}
