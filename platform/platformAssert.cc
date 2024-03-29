//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platformAssert.h"
#include "console/console.h"
#include <stdarg.h>

//-------------------------------------- STATIC Declaration
PlatformAssert *PlatformAssert::platformAssert = NULL;

//--------------------------------------
PlatformAssert::PlatformAssert()
{
   processing = false;
}   

//--------------------------------------
void PlatformAssert::create()
{
   if (!platformAssert)   
      platformAssert = new PlatformAssert;
}   


//--------------------------------------
void PlatformAssert::destroy()
{
   if (platformAssert)   
      delete platformAssert;
   platformAssert = NULL;
}  


//--------------------------------------
bool PlatformAssert::displayMessageBox(const char *title, const char *message, bool retry)
{
   if (retry)
      return Platform::AlertRetry(title, message);

   Platform::AlertOK(title, message);
   return false;
}   


//--------------------------------------
void PlatformAssert::process(Type         assertType,
                             const char  *filename,
                             U32          lineNumber,
                             const char  *message)
{
   processing = true;
   char *typeName[] = { "Unknown", "Fatal-ISV", "Fatal", "Warning" };
   
   // always dump to the Assert to the Console
   if (Con::isActive())
   {
      if (assertType == Warning)
	      Con::warnf(ConsoleLogEntry::Assert, "%s: (%s @ %ld) %s", typeName[assertType], filename, lineNumber, message);   
      else
	      Con::errorf(ConsoleLogEntry::Assert, "%s: (%s @ %ld) %s", typeName[assertType], filename, lineNumber, message);   
   }
   
   // if not a WARNING pop-up a dialog box
   if (assertType != Warning)
   {
      // used for processing navGraphs (an assert won't botch the whole build) 
      if(Con::getBoolVariable("$FP::DisableAsserts", false) == true)
         Platform::forceShutdown(1);
      
      char buffer[2048];
      dSprintf(buffer, 2048, "%s: (%s @ %ld)", typeName[assertType], filename, lineNumber);

#ifdef DEBUG
      // In debug versions, allow a retry even for ISVs...
      bool retry = displayMessageBox(buffer, message, true);
#else
      bool retry = displayMessageBox(buffer, message, ((assertType == Fatal) ? true : false) );
#endif
      if (retry)
         Platform::debugBreak();
      else
         Platform::forceShutdown(1);
   }
   processing = false;
}

bool PlatformAssert::processingAssert()
{
   return platformAssert ? platformAssert->processing : false;
}
 
//--------------------------------------
void PlatformAssert::processAssert(Type        assertType,
                                   const char  *filename,
                                   U32         lineNumber,
                                   const char  *message)
{
   if (platformAssert)
      platformAssert->process(assertType, filename, lineNumber, message);
}


//--------------------------------------
const char* avar(const char *message, ...)
{
   static char buffer[1024];
   va_list args;
   va_start(args, message);
   dVsprintf(buffer, sizeof(buffer), message, args);
   return( buffer );
}
