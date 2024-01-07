//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

char *dStrdup(const char *src)
{
   char *buffer = new char[dStrlen(src) + 1];
   dStrcpy(buffer, src);
   return buffer;
}

char *dStrnew(const char *src)
{
   char *buffer = new char[dStrlen(src) + 1];
   dStrcpy(buffer, src);
   return buffer;
}

char* dStrcat(char *dst, const char *src)
{
   return strcat(dst,src);
}   


S32 dStrcmp(const char *str1, const char *str2)
{
   return strcmp(str1, str2);   
}  

 
S32 dStricmp(const char *str1, const char *str2)
{
	char c1, c2;
   while (1)
   {
   	c1 = tolower(*str1++);
   	c2 = tolower(*str2++);
      if (c1 < c2) return -1;
      if (c1 > c2) return 1;
      if (c1 == 0) return 0;
	}
}  

S32 dStrncmp(const char *str1, const char *str2, U32 len)
{
   return strncmp(str1, str2, len);   
}  
 
S32 dStrnicmp(const char *str1, const char *str2, U32 len)
{
	S32 i;
   char c1, c2;
   for (i=0; i<len; i++)
   {
   	c1 = tolower(*str1++);
      c2 = tolower(*str2++);
      if (c1 < c2) return -1;
      if (c1 > c2) return 1;
      if (!c1) return 0;
  	}
   return 0;
}   


char* dStrcpy(char *dst, const char *src)
{
   return strcpy(dst,src);
}   


char* dStrncpy(char *dst, const char *src, U32 len)
{
   return strncpy(dst,src,len);
}   


U32 dStrlen(const char *str)
{
   return strlen(str);
}   


char* dStrupr(char *str)
{
	while (*str)
   {
   	*str = toupper(*str);
      str++;
	}
   return str;
}   


char* dStrlwr(char *str)
{
	while (*str)
   {
   	*str = tolower(*str);
      str++;
	}
   return str;
}   


char* dStrchr(char *str, S32 c)
{
   return strchr(str,c);
}   


const char* dStrchr(const char *str, S32 c)
{
   return strchr(str,c);
}   

const char* dStrrchr(const char *str, S32 c)
{
   return strrchr(str,c);
}   


char* dStrrchr(char *str, S32 c)
{
   return strrchr(str,c);
}   

U32 dStrspn(const char *str, const char *set)
{
   return(strspn(str, set));
}

U32 dStrcspn(const char *str, const char *set)
{
   return strcspn(str, set);
}   


char* dStrstr(char *str1, char *str2)
{
   return strstr(str1,str2);
}   


const char* dStrstr(const char *str1, const char *str2)
{
   return strstr(str1,str2);
}   

char* dStrtok(char *str, const char *sep)
{
   return strtok(str, sep);
}


S32 dAtoi(const char *str)
{
   return atoi(str);   
}  

 
F32 dAtof(const char *str)
{
   return atof(str);   
}   

bool dAtob(const char *str)
{
   return !dStricmp(str, "true") || dAtof(str);
}   

bool dIsalnum(const char c)
{
   return isalnum(c);
}

bool dIsalpha(const char c)
{
   return(isalpha(c));
}

bool dIsspace(const char c)
{
   return(isspace(c));
}

bool dIsdigit(const char c)
{
   return(isdigit(c));
}

void dPrintf(const char *format, ...)
{
   va_list args;
   va_start(args, format);
   vprintf(format, args);
}   

S32 dVprintf(const char *format, void *arglist)
{
   S32 len = vprintf(format, (char*)arglist);

   // to-do
   // The intended behavior is to zero-terminate and not allow the overflow
   return (len);
}   

S32 dSprintf(char *buffer, U32 /*bufferSize*/, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   S32 len = vsprintf(buffer, format, args);

   // to-do
   // The intended behavior is to zero-terminate and not allow the overflow
   return (len);
}   


S32 dVsprintf(char *buffer, U32 /*bufferSize*/, const char *format, void *arglist)
{
   S32 len = vsprintf(buffer, format, (char*)arglist);

   // to-do
   // The intended behavior is to zero-terminate and not allow the overflow
   return (len);
}   


S32 dSscanf(const char *buffer, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   return __vsscanf(buffer, format, args);   
}   

S32 dFflushStdout()
{
   return fflush(stdout);
}

S32 dFflushStderr()
{
   return fflush(stderr);
}

void dQsort(void *base, U32 nelem, U32 width, S32 (QSORT_CALLBACK *fcmp)(const void *, const void *))
{
   qsort(base, nelem, width, fcmp);
}   


//---------------------------------------------------------------------------
// Mac Strinng conversion routines
U8* str2p(const char *str)
{
	static U8 buffer[256];
	str2p(str, buffer);
	return buffer;
}


U8* str2p(const char *str, U8 *p)
{
	AssertFatal(dStrlen(str) <= 255, "str2p:: Max Pascal String length exceeded (max=255).");
	U8 *dst = p+1;
	U8 *src = (U8*)str;
	*p = 0;
	while(*src != '\0')
	{
		*dst++ = *src++;
		(*p) += 1;
	}
	return p;
}


char* p2str(U8 *p)
{
	static char buffer[256];
	p2str(p, buffer);
	return buffer;
}


char* p2str(U8 *p, char *dst_str)
{
	U8 len = *p++;
	char *src = (char*)p;
	char *dst = dst_str;
	while (len--)
	{
		*dst++ = *src++;
	}
	*dst = '\0';
	return dst_str;
}

