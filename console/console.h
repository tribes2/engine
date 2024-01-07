//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _BITSET_H_
#include "Core/bitSet.h"
#endif

class SimObject;
struct EnumTable;
class Namespace;

enum
{
   StringTagPrefixByte = 0x01
};

struct ConsoleLogEntry
{
   enum Level
   {
      Normal = 0,
      Warning,
      Error,
      NUM_CLASS
   } mLevel;
   enum Type
   {
      General = 0,
      Assert,
      Script,
      GUI,
      Network,
      NUM_TYPE
   } mType;
   const char *mString;
};

struct EnumTable
{
	S32 size;

	struct Enums
	{
		S32 index;
		const char *label;
	};

	Enums *table;
	EnumTable(S32 sSize, Enums *sTable)
      { size = sSize; table = sTable; }
};

//--------------------------------------------------------------------------- 
typedef const char *StringTableEntry;
typedef const char *(*StringCallback)(SimObject *obj, S32 argc, const char *argv[]);
typedef S32 (*IntCallback)(SimObject *obj, S32 argc, const char *argv[]);
typedef F32 (*FloatCallback)(SimObject *obj, S32 argc, const char *argv[]);
typedef void (*VoidCallback)(SimObject *obj, S32 argc, const char *argv[]);
typedef bool (*BoolCallback)(SimObject *obj, S32 argc, const char *argv[]);
typedef void (*ConsumerCallback)(ConsoleLogEntry::Level level, const char *consoleLine);

typedef const char* (*GetDataFunction)(void *dptr, EnumTable *tbl, BitSet32 flag);
typedef void        (*SetDataFunction)(void *dptr, S32 argc, const char **argv, EnumTable *tbl, BitSet32 flag);


namespace Con
{
   enum {
      MaxLineLength = 512
   };

   void init();
   void shutdown();
   bool isActive();
   
   void addConsumer(ConsumerCallback cb);
   void removeConsumer(ConsumerCallback cb);

   void setVariable(const char *name, const char *value);
   bool addVariable(const char *name, S32, void *);
   bool removeVariable(const char *name); 
   const char* getVariable(const char* name);
   const char* getLocalVariable(const char* name);
   void setLocalVariable(const char *name, const char *value);

   void addCommand(const char *name, StringCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *name, IntCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *name, FloatCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *name, VoidCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *name, BoolCallback, const char *usage, S32 minArgs, S32 maxArgs);

   void addCommand(const char *nameSpace, const char *name,StringCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *nameSpace, const char *name,IntCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *nameSpace, const char *name,FloatCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *nameSpace, const char *name,VoidCallback, const char *usage, S32 minArgs, S32 maxArgs);
   void addCommand(const char *nameSpace, const char *name,BoolCallback, const char *usage, S32 minArgs, S32 maxArgs);

   bool removeCommand(const char *name);
   void printf(const char *_format, ...);
   void warnf(ConsoleLogEntry::Type type, const char *_format, ...);
   void errorf(ConsoleLogEntry::Type type, const char *_format, ...);
   void warnf(const char *_format, ...);
   void errorf(const char *_format, ...);

   const char *execute(S32 argc, const char* argv[]);
   const char *executef(S32 argc, ...); // first param is funcName, remaining params are args
   
   // first param is func name, second param MUST be empty (gets filled with object ID)
   // also, MUST have at least those two params
   const char *execute(SimObject *, S32 argc, const char *argv[]);

   const char *executef(SimObject *, S32 argc, ...); // first param is funcName, remaining params are args

   const char *evaluate(const char* string, bool echo = false, const char *fileName = NULL);
   const char *evaluatef(const char* string, ...);

   bool isFunction(const char *fn);

   void setBoolVariable(const char* name,bool var);
   bool getBoolVariable(const char* name,bool def = false);
   void setIntVariable(const char* name,S32 var);
   S32  getIntVariable(const char* name,S32 def = 0);
   void setFloatVariable(const char* name,F32 var);
   F32  getFloatVariable(const char* name,F32 def = .0f);

      // console function implementation helpers
   char *getReturnBuffer(U32 bufferSize);

   char *getArgBuffer(U32 bufferSize);
   char *getFloatArg(F64 arg);
   char *getIntArg(S32 arg);


   const char *tabComplete(const char *prevText, S32 baseLen, bool);
   void exportVariables(const char *varString, Vector<const char *> &varName, Vector<const char *> &value);

   Namespace *lookupNamespace(const char *nsName);
   void linkNamespaces(const char *parentName, const char *childName);

   // this should only be called from consoleObject.h
   void classLinkNamespaces(Namespace *parent, Namespace *child);
   
   void getLog(ConsoleLogEntry * &log, U32 &size);
   // dynamic data management functions:
   void setLogMode(S32 mode);

   void registerType(S32 type, S32 size, GetDataFunction gdf, SetDataFunction sdf);
   void setData(S32 type, void *dptr, S32 index, S32 argc, const char **argv, EnumTable *tbl = NULL, BitSet32 flag = 0);
   const char *getData(S32 type, void *dptr, S32 index, EnumTable *tbl = NULL, BitSet32 flag = 0);
}

extern void expandEscape(char *dest, const char *src);
extern bool collapseEscape(char *buf);
extern S32 HashPointer(StringTableEntry ptr);

struct ConsoleConstructor
{
   StringCallback sc;
   IntCallback ic;
   FloatCallback fc;
   VoidCallback vc;
   BoolCallback bc;
   S32 mina, maxa;
   const char *usage;
   const char *funcName;
   const char *className;
   ConsoleConstructor *next;
   static ConsoleConstructor *first;

   void init(const char *cName, const char *fName, const char *usg, S32 minArgs, S32 maxArgs);
   static void setup();
   ConsoleConstructor(const char *className, const char *funcName, StringCallback sfunc, const char *usage, S32 minArgs, S32 maxArgs);
   ConsoleConstructor(const char *className, const char *funcName, IntCallback ifunc, const char *usage, S32 minArgs, S32 maxArgs);
   ConsoleConstructor(const char *className, const char *funcName, FloatCallback ffunc, const char *usage, S32 minArgs, S32 maxArgs);
   ConsoleConstructor(const char *className, const char *funcName, VoidCallback vfunc, const char *usage, S32 minArgs, S32 maxArgs);
   ConsoleConstructor(const char *className, const char *funcName, BoolCallback bfunc, const char *usage, S32 minArgs, S32 maxArgs);
};

#define ConsoleFunction(name,returnType,minArgs,maxArgs,usage) \
   static returnType c##name(SimObject *, S32, const char **argv); \
   static ConsoleConstructor g##name##obj(NULL,#name,c##name,usage,minArgs,maxArgs);\
   static returnType c##name(SimObject *, S32 argc, const char **argv)
   
#define ConsoleMethod(className,name,returnType,minArgs,maxArgs,usage) \
   static returnType c##className##name(SimObject *, S32, const char **argv); \
   static ConsoleConstructor className##name##obj(#className,#name,c##className##name,usage,minArgs,maxArgs);\
   static returnType c##className##name(SimObject *object, S32 argc, const char **argv)

#endif
