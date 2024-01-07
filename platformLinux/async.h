//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _ASYNC_H_
#define _ASYNC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FD_READ               0x0001
#define FD_WRITE              0x0002
#define FD_OOB                0x0004
#define FD_ACCEPT             0x0008
#define FD_CONNECT            0x0010
#define FD_CLOSE              0x0020

typedef int (*AsyncCallback)( int fd, int event, int status );


extern int AsyncInit( void );
extern int AsyncShutdown( void );
extern int AsyncCancel( int fd );
extern int AsyncSelect( int fd, AsyncCallback callback, int mask );

#ifdef __cplusplus
}
#endif

#endif
