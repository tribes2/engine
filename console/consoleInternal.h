//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CONSOLEINTERNAL_H_
#define _CONSOLEINTERNAL_H_

#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif
#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif
#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

class ExprEvalState;
struct FunctionDecl;
class CodeBlock;

class Namespace
{
   enum {
      MaxActivePackages = 512,
   };

   static U32 mNumActivePackages;
   static U32 mOldNumActivePackages;
   static StringTableEntry mActivePackages[MaxActivePackages];
public:
   StringTableEntry mName;
   StringTableEntry mPackage;

   Namespace *mParent;
   Namespace *mNext;
   U32 mRefCountToParent;
   
   struct Entry
   {
      enum {
         InvalidFunctionType = -1,
         ScriptFunctionType,
         StringCallbackType,
         IntCallbackType,
         FloatCallbackType,
         VoidCallbackType,
         BoolCallbackType
      };
      
      Namespace *mNamespace;
      Entry *mNext;
      StringTableEntry mFunctionName;
      S32 mType;
      S32 mMinArgs;
      S32 mMaxArgs;
      const char *mUsage;

      CodeBlock *mCode;
      U32 mFunctionOffset;
      union {
		   StringCallback mStringCallbackFunc;
		   IntCallback mIntCallbackFunc;
		   VoidCallback mVoidCallbackFunc;
		   FloatCallback mFloatCallbackFunc;
		   BoolCallback mBoolCallbackFunc;
      } cb;
      Entry();
      void clear();
      
      const char *execute(S32 argc, const char **argv, ExprEvalState *state);
      
   };
   Entry *mEntryList;

   Entry **mHashTable;
   U32 mHashSize;
   U32 mHashSequence;

   Namespace();
   void addFunction(StringTableEntry name, CodeBlock *cb, U32 functionOffset);
	void addCommand(StringTableEntry name,StringCallback, const char *usage, S32 minArgs, S32 maxArgs);
	void addCommand(StringTableEntry name,IntCallback, const char *usage, S32 minArgs, S32 maxArgs);
	void addCommand(StringTableEntry name,FloatCallback, const char *usage, S32 minArgs, S32 maxArgs);
	void addCommand(StringTableEntry name,VoidCallback, const char *usage, S32 minArgs, S32 maxArgs);
	void addCommand(StringTableEntry name,BoolCallback, const char *usage, S32 minArgs, S32 maxArgs);

   void getEntryList(Vector<Entry *> *);

   Entry *lookup(StringTableEntry name);
   Entry *lookupRecursive(StringTableEntry name);
   Entry *createLocalEntry(StringTableEntry name);
   void buildHashTable();
   void clearEntries();
   void classLinkTo(Namespace *parent);

   const char *tabComplete(const char *prevText, S32 baseLen, bool fForward);

   static U32 mCacheSequence;
   static DataChunker mCacheAllocator;
   static DataChunker mAllocator;
   static void trashCache();
   static Namespace *mNamespaceList;
   static Namespace *mGlobalNamespace;
   
   static void init();
   static void shutdown();
   static Namespace *global();
   
   static Namespace *find(StringTableEntry name, StringTableEntry package=NULL);

   static void activatePackage(StringTableEntry name);
   static void deactivatePackage(StringTableEntry name);
   static void unlinkPackages();
   static void relinkPackages();
   static bool isPackage(StringTableEntry name);
};

extern char *typeValueEmpty;

class Dictionary
{
public:
   struct Entry
   {
      enum
      {
         TypeInternalInt = -3,
         TypeInternalFloat = -2,
         TypeInternalString = -1,
      };

      StringTableEntry name;
      Entry *nextEntry;
      S32 type;
      char *sval;
      U32 ival;  // doubles as strlen when type = -1
      F32 fval;
      U32 bufferLen;
      void *dataPtr;

      Entry(StringTableEntry name);
      ~Entry();

      U32 getIntValue()
      {
         if(type <= TypeInternalString)
            return ival;
         else
            return dAtoi(Con::getData(type, dataPtr, 0));
      }
      F32 getFloatValue()
      {
         if(type <= TypeInternalString)
            return fval;
         else
            return dAtof(Con::getData(type, dataPtr, 0));
      }
      const char *getStringValue()
      {
         if(type == TypeInternalString)
            return sval;
         if(type == TypeInternalFloat)
            return Con::getData(TypeF32, &fval, 0);
         else if(type == TypeInternalInt)
            return Con::getData(TypeS32, &ival, 0);
         else
            return Con::getData(type, dataPtr, 0);
      }
      void setIntValue(U32 val)
      {
         if(type <= TypeInternalString)
         {
            fval = val;
            ival = val;
            if(sval != typeValueEmpty)
            {
               dFree(sval);
               sval = typeValueEmpty;
            }
            type = TypeInternalInt;
            return;
         }
         else
         {
            const char *dptr = Con::getData(TypeS32, &val, 0);
            Con::setData(type, dataPtr, 0, 1, &dptr);
         }
      }
      void setFloatValue(F32 val)
      {
         if(type <= TypeInternalString)
         {
            fval = val;
            ival = val;
            if(sval != typeValueEmpty)
            {
               dFree(sval);
               sval = typeValueEmpty;
            }
            type = TypeInternalFloat;
            return;
         }
         else
         {
            const char *dptr = Con::getData(TypeF32, &val, 0);
            Con::setData(type, dataPtr, 0, 1, &dptr);
         }
      }
      void setStringValue(const char *value);
   };

private:
   S32 hashTableSize;
   S32 entryCount;

   Entry **hashTable;
   ExprEvalState *exprState;
public:
   StringTableEntry scopeName;
   Namespace *scopeNamespace;
   CodeBlock *code;
   U32 ip;

   Dictionary() {}
   Dictionary(ExprEvalState *state);
   ~Dictionary();
   Entry *lookup(StringTableEntry name);
   Entry *add(StringTableEntry name);
   void setState(ExprEvalState *state);
   void remove(Entry *);
   void reset();
   
   void exportVariables(const char *varString, const char *fileName, bool append);
   void deleteVariables(const char *varString);

   void setVariable(StringTableEntry name, const char *value);
   const char *getVariable(StringTableEntry name, bool *valid = NULL);

   void addVariable(const char *name, S32 type, void *dataPtr);
   bool removeVariable(StringTableEntry name); 

   // return the best tab completion for prevText, with the length
   // of the pre-tab string in baseLen

   const char *tabComplete(const char *prevText, S32 baseLen, bool);
};

class ExprEvalState
{
public:
   // stuff for doing expression evaluation

   SimObject *thisObject;
   Dictionary::Entry *currentVariable;
   bool traceOn;

   ExprEvalState();
   ~ExprEvalState();

   // stack management
   Dictionary globalVars;
   Vector<Dictionary *> stack;
   void setCurVarName(StringTableEntry name);
   void setCurVarNameCreate(StringTableEntry name);
   S32 getIntVariable();
   F64 getFloatVariable();
   const char *getStringVariable();
   void setIntVariable(S32 val);
   void setFloatVariable(F64 val);
   void setStringVariable(const char *str);

   void pushFrame(const char *frameName, Namespace *ns);
   void popFrame();
};

#endif
