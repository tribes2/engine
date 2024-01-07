//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platformThread.h"
#include "Platform/platformSemaphore.h"

#include <pthread.h>

typedef struct {
	ThreadRunFunction mRunFunc;
	S32 mRunArg;
	Thread* mThread;
	void* mSemaphore;

	LinuxThreadData( void ) {
		mRunFunc = 0;
		mRunArg = 0;
		mThread = 0;
		mSemaphore = 0;
	}

} LinuxThreadData;

Thread::Thread( ThreadRunFunction func, S32 arg, bool start_thread )
{
	LinuxThreadData* threadData = new LinuxThreadData( );

	threadData->mRunFunc = func;
	threadData->mRunArg = arg;
	threadData->mThread = this;
	threadData->mSemaphore = Semaphore::createSemaphore( );

	mData = threadData;

	if ( start_thread ) {
		start( );
	}
}

Thread::~Thread( void )
{
	join( );

	LinuxThreadData* threadData = reinterpret_cast<LinuxThreadData*>( mData );
	Semaphore::destroySemaphore( threadData->mSemaphore );

	delete threadData;
}

static int threadRunHandler( void* arg )
{
	LinuxThreadData* threadData = reinterpret_cast<LinuxThreadData*>( arg );

	threadData->mThread->run( threadData->mRunArg );
	Semaphore::releaseSemaphore( threadData->mSemaphore );

	return 0;
}

typedef void* (*pthread_func)( void* );

void Thread::start( void )
{

	if( isAlive( ) ) {
		return;
	}

	LinuxThreadData* threadData = reinterpret_cast<LinuxThreadData*>( mData );
	Semaphore::acquireSemaphore( threadData->mSemaphore );

	pthread_attr_t attr;
	pthread_t thread;

	pthread_attr_init( &attr );
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
	pthread_create( &thread, &attr, (pthread_func) threadRunHandler, threadData );
}

bool Thread::join( void )
{

	if( !isAlive( ) ) {
		return false;
	}

	LinuxThreadData* threadData = reinterpret_cast<LinuxThreadData*>( mData );
	bool result = Semaphore::acquireSemaphore( threadData->mSemaphore );

	return result;
}

void Thread::run( S32 arg )
{
	LinuxThreadData* threadData = reinterpret_cast<LinuxThreadData*>( mData );

	if( threadData->mRunFunc ) {
		threadData->mRunFunc( arg );
	}

}

bool Thread::isAlive( void )
{
	LinuxThreadData* threadData = reinterpret_cast<LinuxThreadData*>( mData );

	bool signal = Semaphore::acquireSemaphore( threadData->mSemaphore, false );

	if( signal ) {
		Semaphore::releaseSemaphore( threadData->mSemaphore );
	}

	return !signal;
}
