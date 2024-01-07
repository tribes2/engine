//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMASSERT_H_
#define _PLATFORMASSERT_H_


#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif


class PlatformAssert
{
public:
   enum Type {
      Warning   = 3,
      Fatal     = 2,
      Fatal_ISV = 1 
   };

private:
   static PlatformAssert *platformAssert;
   bool processing;
   
   bool displayMessageBox(const char *title, const char *message, bool retry);
   void process(Type         assertType,
                const char*  filename,
                U32          lineNumber,
                const char*  message);

   PlatformAssert(); 

public:
   static void create();
   static void destroy();
   static void processAssert(Type         assertType,
                             const char*  filename,
                             U32          lineNumber,
                             const char*  message);
   static char* message(const char *message, ...);   
   static bool processingAssert();
};


#ifdef ENABLE_ASSERTS
   #define AssertWarn(x, y)      \
         { if (!bool(x)) \
            PlatformAssert::processAssert(PlatformAssert::Warning, __FILE__, __LINE__,  y); }
                                
   #define AssertFatal(x, y)     \
         { if (!bool(x)) \
            PlatformAssert::processAssert(PlatformAssert::Fatal, __FILE__, __LINE__,  y); }

//   #define AssertFatal(x, y)   { }

#else
   #define AssertFatal(x, y)   { }
   #define AssertWarn(x, y)    { }
#endif


#define AssertISV(x, y)             \
   { if (!bool(x)) \
      PlatformAssert::processAssert(PlatformAssert::Fatal_ISV, __FILE__, __LINE__,  y); }


const char* avar(const char *in_msg, ...);



#endif // _PLATFORM_ASSERT_H_

