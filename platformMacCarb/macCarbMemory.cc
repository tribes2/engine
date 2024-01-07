//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformMacCarb/platformMacCarb.h"
#include <stdlib.h>
#include <string.h>

//--------------------------------------
#ifdef new
#undef new
#endif

void* FN_CDECL operator new(dsize_t dt, void* ptr)
{
   dMemset(ptr, 0xFF, dt);
   return (ptr);
}   


//--------------------------------------
void* dRealMalloc(dsize_t in_size)
{
   return malloc(in_size);
}


//--------------------------------------
void dRealFree(void* in_pFree)
{
   free(in_pFree);
}


void* dMemcpy(void *dst, const void *src, unsigned size)
{
   return memcpy(dst,src,size);
}   


//--------------------------------------
void* dMemmove(void *dst, const void *src, unsigned size)
{
   return memmove(dst,src,size);
}  
 
//--------------------------------------
void* dMemset(void *dst, S32 c, unsigned size)
{
   return memset(dst,c,size);   
}   

//--------------------------------------
S32 dMemcmp(const void *ptr1, const void *ptr2, unsigned len)
{
   return(memcmp(ptr1, ptr2, len));
}
