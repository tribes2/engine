//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMMUTEX_H_
#define _PLATFORMMUTEX_H_

struct Mutex
{
	static void* createMutex( void );
	static void destroyMutex( void* );
	static void lockMutex( void* );
	static void unlockMutex( void* );
};

#endif
