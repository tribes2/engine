//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/consoleObject.h"
#include "Core/fileStream.h"
#include "Core/resManager.h"
#include "console/ast.h"
#include "Core/tAlgorithm.h"
#include "console/consoleTypes.h"
#include "console/telnetDebugger.h"
#include "console/simBase.h"
#include "console/compiler.h"
#include <stdarg.h>

ExprEvalState gEvalState;
StmtNode *statementList;
ConsoleConstructor *ConsoleConstructor::first = NULL;

static char scratchBuffer[1024];
// TO-DO: Console debugger stuff to be cleaned up later
static S32 dbgGetCurrentFrame(void)
{
   return gEvalState.stack.size() - 1;
}

static const char * prependDollar ( const char * name )
{
   if(name[0] != '$'){
      S32   len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-2, "CONSOLE: name too long");
      scratchBuffer[0] = '$';
      dMemcpy(scratchBuffer + 1, name, len + 1);
      name = scratchBuffer;
   }
   return name;
}

static const char * prependPercent ( const char * name )
{
   if(name[0] != '%'){
      S32   len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-2, "CONSOLE: name too long");
      scratchBuffer[0] = '%';
      dMemcpy(scratchBuffer + 1, name, len + 1);
      name = scratchBuffer;
   }
   return name;
}

//--------------------------------------
void ConsoleConstructor::init(const char *cName, const char *fName, const char *usg, S32 minArgs, S32 maxArgs)
{
   mina = minArgs;
   maxa = maxArgs;
   funcName = fName;
   usage = usg;
   className = cName;
   sc = 0; fc = 0; vc = 0; bc = 0; ic = 0;
   next = first;
   first = this;
}

void ConsoleConstructor::setup()
{
   for(ConsoleConstructor *walk = first; walk; walk = walk->next)
   {
      if(walk->sc)
         Con::addCommand(walk->className, walk->funcName, walk->sc, walk->usage, walk->mina, walk->maxa);
      else if(walk->ic)
         Con::addCommand(walk->className, walk->funcName, walk->ic, walk->usage, walk->mina, walk->maxa);
      else if(walk->fc)
         Con::addCommand(walk->className, walk->funcName, walk->fc, walk->usage, walk->mina, walk->maxa);
      else if(walk->vc)
         Con::addCommand(walk->className, walk->funcName, walk->vc, walk->usage, walk->mina, walk->maxa);
      else if(walk->bc)
         Con::addCommand(walk->className, walk->funcName, walk->bc, walk->usage, walk->mina, walk->maxa);
      
   }
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, StringCallback sfunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   sc = sfunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, IntCallback ifunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   ic = ifunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, FloatCallback ffunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   fc = ffunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, VoidCallback vfunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   vc = vfunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, BoolCallback bfunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   bc = bfunc;
}

namespace Con
{

static Vector<ConsumerCallback> gConsumers(__FILE__, __LINE__);
static DataChunker consoleLogChunker;
static Vector<ConsoleLogEntry> consoleLog(__FILE__, __LINE__);
static bool logBufferEnabled=true;
static S32 printLevel = 10;
static FileStream consoleLogFile;
static const char *defLogFileName = "console.log";
static S32 consoleLogMode = 2; // default to no logging
static bool active = false;
static bool newLogFile;
static const char *logFileName;

static void cClearScreen(SimObject *, S32 , const char **)
{
   consoleLogChunker.freeBlocks();
   consoleLog.setSize(0);
};

static const char* cGetClipboard(SimObject *, S32 , const char **)
{
	return Platform::getClipboard();
};

static bool cSetClipboard(SimObject *, S32 , const char **argv)
{
	return Platform::setClipboard(argv[1]);
};

void init()
{
   AssertFatal(active == false, "Con::init should only be called once.");

   active = true;   
   logFileName = NULL;
   newLogFile  = true;

   Namespace::init();
   // Commands
   addCommand("getClipboard",  cGetClipboard, "getClipboard()", 1, 1);
   addCommand("cls",  cClearScreen, "cls()", 1, 1);
   addCommand("getClipboard",  cGetClipboard, "getClipboard()", 1, 1);
   addCommand("setClipboard",  cSetClipboard, "setClipboard(text)", 2, 2);

   ConsoleConstructor::setup();
   
   // Variables
   setVariable("Con::prompt", "% ");
   addVariable("Con::logBufferEnabled", TypeBool, &logBufferEnabled);
   addVariable("Con::printLevel", TypeS32, &printLevel);

   AbstractClassRep::initialize();
}

//--------------------------------------

void shutdown()
{
   AssertFatal(active == true, "Con::shutdown should only be called once.");
   active = false;
      
   consoleLogFile.close();
   Namespace::shutdown();
}

bool isActive()
{
   return active;
}

//--------------------------------------

void getLog(ConsoleLogEntry *&log, U32 &size)
{
   log = &consoleLog[0];
   size = consoleLog.size();
}

const char *tabComplete(const char *prevText, S32 baseLen, bool fForward)
{
	if (!prevText[0])
		return "";
   else if(prevText[0] == '$')
      return gEvalState.globalVars.tabComplete(prevText, baseLen, fForward);
   else
      return Namespace::global()->tabComplete(prevText, baseLen, fForward);
}

//------------------------------------------------------------------------------
static void log(const char *string)
{
   if(!consoleLogMode)
      return;

   if(consoleLogMode == 1)
      consoleLogFile.open(defLogFileName, FileStream::ReadWrite);

   if(consoleLogFile.getStatus() == Stream::Ok)
   {
      consoleLogFile.setPosition(consoleLogFile.getStreamSize());
      if (newLogFile)
      {
         Platform::LocalTime lt; 
         Platform::getLocalTime(lt);

         char buffer[128];
         dSprintf(buffer, sizeof(buffer), "-------------------------- %d/%d/%d -- %02d:%02d:%02d -----\r\n",   
               lt.month,
               lt.monthday,
               lt.year,
               lt.hour,
               lt.min,
               lt.sec);
         consoleLogFile.write(dStrlen(buffer), buffer);
         newLogFile = false;
      }
      consoleLogFile.write(dStrlen(string), string);
      consoleLogFile.write(2, "\r\n");
   }
   if(consoleLogMode == 1)
      consoleLogFile.close();
}

//------------------------------------------------------------------------------

static void _printf(ConsoleLogEntry::Level level, ConsoleLogEntry::Type type, const char* fmt, va_list argptr)
{
   char buffer[1024];
   U32 offset = 0;
   if(gEvalState.traceOn && gEvalState.stack.size())
   {
      offset = (gEvalState.stack.size() - 1) * 2;
      for(U32 i = 0; i < offset; i++)
         buffer[i] = ' ';
   }
   dVsprintf(buffer + offset, sizeof(buffer) - offset, fmt, argptr);

   for(U32 i = 0; i < gConsumers.size(); i++)
      gConsumers[i](level, buffer);

   if(logBufferEnabled || consoleLogMode)
   {
      char *pos = buffer;
      while(*pos)
      {
         if(*pos == '\t')
            *pos = '^';
         pos++;
      }
      pos = buffer;

      for(;;) 
      {
         char *eofPos = dStrchr(pos, '\n');
         if(eofPos)
            *eofPos = 0;

         log(pos);
         if(logBufferEnabled)
         {
            ConsoleLogEntry entry;
            entry.mLevel  = level;
            entry.mType   = type;
            entry.mString = (const char *)consoleLogChunker.alloc(dStrlen(pos) + 1);
            dStrcpy(const_cast<char*>(entry.mString), pos);
            consoleLog.push_back(entry);
         }
         if(!eofPos)
            break;
         pos = eofPos + 1;
      }
   }
}   

//------------------------------------------------------------------------------
void printf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Normal, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

void warnf(ConsoleLogEntry::Type type, const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Warning, type, fmt, argptr);
   va_end(argptr);
}

void errorf(ConsoleLogEntry::Type type, const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Error, type, fmt, argptr);
   va_end(argptr);
}

void warnf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Warning, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

void errorf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Error, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

//--------------------------------------------------------------------------- 

void setVariable(const char *name, const char *value)
{
   name = prependDollar(name);
   gEvalState.globalVars.setVariable(StringTable->insert(name), value);
}

void setLocalVariable(const char *name, const char *value)
{
   name = prependPercent(name);
   gEvalState.stack.last()->setVariable(StringTable->insert(name), value);
}

void setBoolVariable(const char *varName, bool value)
{
   setVariable(varName, value ? "1" : "0");
}

void setIntVariable(const char *varName, S32 value)
{
   char scratchBuffer[32];
   dSprintf(scratchBuffer, sizeof(scratchBuffer), "%d", value);
   setVariable(varName, scratchBuffer);
}

void setFloatVariable(const char *varName, F32 value)
{
   char scratchBuffer[32];
   dSprintf(scratchBuffer, sizeof(scratchBuffer), "%f", value);
   setVariable(varName, scratchBuffer);   
}

//--------------------------------------------------------------------------- 
void addConsumer(ConsumerCallback consumer)
{
   gConsumers.push_back(consumer);
}

void removeConsumer(ConsumerCallback)
{
}

const char *getVariable(const char *name)
{
   // get the field info from the object..
   if(name[0] != '$' && dStrchr(name, '.') && !isFunction(name))
   {
      S32 len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-1, "CONSOLE: name too long");
      dMemcpy(scratchBuffer, name, len+1);
      
      char * token = dStrtok(scratchBuffer, ".");
      SimObject * obj = Sim::findObject(token);
      if(!obj)
         return("");
      
      token = dStrtok(0, ".\0");
      if(!token)
         return("");
         
      while(token != NULL)
      {
         const char * val = obj->getDataField(StringTable->insert(token), 0);
         if(!val)
            return("");
            
         token = dStrtok(0, ".\0");
         if(token)
         {
            obj = Sim::findObject(token);
            if(!obj)
               return("");
         }
         else
            return(val);
      }
   }
   
   name = prependDollar(name);
   return gEvalState.globalVars.getVariable(StringTable->insert(name));
}

const char *getLocalVariable(const char *name)
{
   name = prependPercent(name);
   
   return gEvalState.stack.last()->getVariable(StringTable->insert(name));
}

bool getBoolVariable(const char *varName, bool def)
{
   const char *value = getVariable(varName);
   return *value ? dAtob(value) : def;
}

S32 getIntVariable(const char *varName, S32 def)
{
   const char *value = getVariable(varName);
   return *value ? dAtoi(value) : def;
}

F32 getFloatVariable(const char *varName, F32 def)
{
   const char *value = getVariable(varName);
   return *value ? dAtof(value) : def;
}

//--------------------------------------------------------------------------- 

bool addVariable(const char *name, S32 t, void *dp)
{
   gEvalState.globalVars.addVariable(name, t, dp);
   return true;
}

bool removeVariable(const char *name)
{
   name = StringTable->lookup(prependDollar(name));
   return name!=0 && gEvalState.globalVars.removeVariable(name);
}

//--------------------------------------------------------------------------- 

void addCommand(const char *nsName, const char *name,StringCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,VoidCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,IntCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,FloatCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,BoolCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,StringCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,VoidCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,IntCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,FloatCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,BoolCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

const char *evaluate(const char* string, bool echo, const char *fileName)
{
   if (echo)
      printf("%s%s", getVariable( "$Con::Prompt" ), string);

   if(fileName)
      fileName = StringTable->insert(fileName);

   CodeBlock *newCodeBlock = new CodeBlock();
   return newCodeBlock->compileExec(fileName, string, false);
}

//------------------------------------------------------------------------------
const char *evaluatef(const char* string, ...)
{
   char buffer[512];
   va_list args;
   va_start(args, string);
   dVsprintf(buffer, sizeof(buffer), string, args);
   CodeBlock *newCodeBlock = new CodeBlock();
   return newCodeBlock->compileExec(NULL, buffer, false);
}   

const char *execute(S32 argc, const char *argv[])
{
   Namespace::Entry *ent;
   StringTableEntry funcName = StringTable->insert(argv[0]);
   ent = Namespace::global()->lookup(funcName);

   if(!ent)
   {
      warnf(ConsoleLogEntry::Script, "%s: Unknown command.", argv[0]);
      return "";
   }
   return ent->execute(argc, argv, &gEvalState);
}

//------------------------------------------------------------------------------
const char *execute(SimObject *object, S32 argc, const char *argv[])
{
   static char idBuf[12];
   if(argc < 2)
      return "";
   if(object->getNamespace())
   {
      dSprintf(idBuf, sizeof(idBuf), "%d", object->getId());
      argv[1] = idBuf;
      
      StringTableEntry funcName = StringTable->insert(argv[0]);
      Namespace::Entry *ent = object->getNamespace()->lookup(funcName);
   
      if(!ent)
      {
         //warnf(ConsoleLogEntry::Script, "%s: undefined for object '%s' - id %d", funcName, object->getName(), object->getId());
         return "";
      }
      else
      {
         SimObject *save = gEvalState.thisObject;
         gEvalState.thisObject = object;
         const char *ret = ent->execute(argc, argv, &gEvalState);
         gEvalState.thisObject = save;
         return ret;
      }
   }
   warnf(ConsoleLogEntry::Script, "Con::execute - %d has no namespace: %s", object->getId(), argv[0]);
   return "";
}

const char *executef(SimObject *object, S32 argc, ...)
{
   static char idBuf[12];
   const char *argv[128];

   va_list args;
   va_start(args, argc);
   for(S32 i = 0; i < argc; i++)
      argv[i+1] = va_arg(args, const char *);
   va_end(args);
   argv[0] = argv[1];
   argc++;
   
   return execute(object, argc, argv);
}   

//------------------------------------------------------------------------------
const char *executef(S32 argc, ...)
{
   const char *argv[128];

   va_list args;
   va_start(args, argc);
   for(S32 i = 0; i < argc; i++)
      argv[i] = va_arg(args, const char *);
   va_end(args);
   return execute(argc, argv);
}   

//------------------------------------------------------------------------------
bool isFunction(const char *fn)
{
   const char *string = StringTable->lookup(fn);
   if(!string)
      return false;
   else
      return Namespace::global()->lookup(string) != NULL;
}

//------------------------------------------------------------------------------

void setLogMode(S32 newMode)
{
   if(newMode != consoleLogMode)
   {
      if(newMode && !consoleLogMode)
         newLogFile = true;
      if(consoleLogMode == 2)
         consoleLogFile.close();
      else if(newMode == 2)
         consoleLogFile.open(defLogFileName, FileStream::Write);
      consoleLogMode = newMode;
   }
}

Namespace *lookupNamespace(const char *ns)
{
   if(!ns)
      return Namespace::global();
   return Namespace::find(StringTable->insert(ns));
}

void linkNamespaces(const char *parent, const char *child)
{
   Namespace *pns = lookupNamespace(parent);
   Namespace *cns = lookupNamespace(child);
   if(pns && cns)
      cns->classLinkTo(pns);
}

void classLinkNamespaces(Namespace *parent, Namespace *child)
{
   if(parent && child)
      child->classLinkTo(parent);
}

enum {
   MaxDataTypes = 256
};

static S32 typeSizes[MaxDataTypes];
static GetDataFunction fnGetData[MaxDataTypes];
static SetDataFunction fnSetData[MaxDataTypes];

void registerType(S32 type, S32 size, GetDataFunction gdf, SetDataFunction sdf)
{
   fnGetData[type] = gdf;
   fnSetData[type] = sdf;
   typeSizes[type] = size;
}

void setData(S32 type, void *dptr, S32 index, S32 argc, const char **argv, EnumTable *tbl, BitSet32 flag)
{
   SetDataFunction fn = fnSetData[type];
   AssertFatal(fn != NULL, "Error, mull setData fn");
   fn((void *) (U32(dptr) + index * typeSizes[type]),argc, argv, tbl, flag);
}

const char *getData(S32 type, void *dptr, S32 index, EnumTable *tbl, BitSet32 flag)
{
   GetDataFunction fn = fnGetData[type];
   AssertFatal(fn != NULL, "Error, mull getData fn");
   return fn((void *) (U32(dptr) + index * typeSizes[type]), tbl, flag);
}

} // end of Console namespace
