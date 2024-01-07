//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CONSOLEOBJECT_H_
#define _CONSOLEOBJECT_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif
#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif
#ifndef _BITSET_H_
#include "core/bitSet.h"
#endif
#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

//------------------------------------------------------------------------------
//-------------------------------------- AbstractClassRep
//

class Namespace;

enum {
   NetObjectClassFirst = 0,
   NetObjectClassLast = 127,
   NetObjectClassBitSize = 7,

   DataBlockClassFirst = 128,
   DataBlockClassLast = 255,
   DataBlockClassBitSize = 7,

   NetEventClassFirst = 255,
   NetEventClassLast = 318,
   NetEventClassBitSize = 6,

   MaxClassId = 318
};

class AbstractClassRep
{
   friend class ConsoleObject;

   //-------------------------------------- Public interface
  public:
   enum
   {
      DepricatedFieldType = 0xFFFFFFFF
   };
   virtual ~AbstractClassRep() { }
   AbstractClassRep() {
      VECTOR_SET_ASSOCIATION(mFieldList);
   }

   S32         getClassId()   const;
   const char* getClassName() const;
   Namespace * getNameSpace();
   virtual ConsoleObject* create()    const = 0;
   AbstractClassRep *getNextClass();
   static AbstractClassRep *getClassList();
   
   //-------------------------------------- Field interface
  public:
   struct Field {
      const char* pFieldname;
      U32 type;
      U32 offset;
      S32 elementCount;
		EnumTable *table;
		BitSet32   flag;
   };
   typedef Vector<Field> FieldList;
   
   FieldList mFieldList;
   const Field *findField(StringTableEntry fieldName) const;
  public:
   S32 mClassIdBase;
   S32 mClassVersion;
   S32 mClassNetClass;
   static void initialize(); // Called from CMDCon::init once on startup

  protected:
   static AbstractClassRep *classTable[MaxClassId+1];
   static AbstractClassRep *classLinkList;
   static bool initialized;
   
   static void registerClassRep(AbstractClassRep*);

  protected:
   virtual void init() const = 0;   
   
   //---------------------------------------------------------------
   // each registered tagged class has a id tag and a compiler
   //  independant class name
   //  if the class can be constructed by tag (network)
   //  the tag is >= 0.

  protected:

   const char *mClassName;
   S32 mClassId;
   AbstractClassRep *nextClass;
   Namespace *mNamespace;

   // Helper functions for ConsoleObject
  protected:
   static ConsoleObject* create(const char*  in_pClassName);
   static ConsoleObject* create(const S32 in_classId);

};

inline AbstractClassRep *AbstractClassRep::getClassList()
{
   return classLinkList;
}

inline AbstractClassRep *AbstractClassRep::getNextClass()
{
   return nextClass;
}

inline S32 AbstractClassRep::getClassId() const
{
   return mClassId;
}

inline const char* AbstractClassRep::getClassName() const
{
   return mClassName;
}

inline Namespace *AbstractClassRep::getNameSpace()
{
   return mNamespace;
}

//------------------------------------------------------------------------------
//-------------------------------------- ConcreteClassRep
//

enum
{
   NetEventClassAny,
   NetEventClassClient,
   NetEventClassServer
};

template <class T>
class ConcreteClassRep : public AbstractClassRep
{
  public:
   ConcreteClassRep(const char *name, S32 idBase = -1, S32 version=1,S32 netClass = NetEventClassAny)
   {
      // name is a static compiler string so no need to worry about copying or deleting
      mClassName = name;
      mClassId = -1;
      mClassIdBase = idBase;
      mClassVersion = version;
      mClassNetClass = netClass;
      registerClassRep(this);
   };
   void init() const
   {
      AbstractClassRep *parent = T::getParentStaticClassRep();
      AbstractClassRep *child = T::getStaticClassRep();
      if(parent && child)
         Con::classLinkNamespaces(parent->getNameSpace(), child->getNameSpace());
      T::initPersistFields();
      T::consoleInit();
   }
   ConsoleObject* create() const { return new T; }
};

//------------------------------------------------------------------------------
//-------------------------------------- USER interface class.  Derive from this,
//                                        and access functionality through its
//                                        static interface.
class ConsoleObject
{
  protected:
   ConsoleObject() { /* disallowed */ }
   ConsoleObject(const ConsoleObject&); // disallowed ?

  protected:
   const AbstractClassRep::Field *findField(StringTableEntry fieldName) const;

  public:
   virtual AbstractClassRep* getClassRep() const;
	bool setField(const char *fieldName, const char *value);
   virtual ~ConsoleObject();

   // Object creation interface
  public:
   static ConsoleObject* create(const char*  in_pClassName);
   static ConsoleObject* create(const U32 in_classId);

   // Query interface
  public:
   static const char* lookupClassName(const U32 in_classTag);

   // Field interface
  protected:
   static void addField(const char*  in_pFieldname,
                        const U32 in_fieldType,
                        const U32 in_fieldOffset,
                        const U32 in_elementCount = 1,
                        EnumTable *in_table = NULL);
   static void addDepricatedField(const char *fieldName);
   static bool removeField(const char* in_pFieldname);

  public:
   static void initPersistFields(); // declare one of these in a class to register dynamic fields in a class.
   static void consoleInit(); // declare one of these in a class to register console functions and establish namespace linkage
   const AbstractClassRep::FieldList& getFieldList() const;
   
   static AbstractClassRep *getStaticClassRep() { return NULL; }
   static AbstractClassRep *getParentStaticClassRep() { return NULL; }

   S32 getClassId() const;
   const char *getClassName() const;
};



//------------------------------------------------------------------------------
//-------------------------------------- Inlines
//
inline S32 ConsoleObject::getClassId() const
{
   AssertFatal(getClassRep() != NULL,
               "Cannot get tag from non-declared dynamic class");
   return getClassRep()->getClassId();
}

inline const char * ConsoleObject::getClassName() const
{
   AssertFatal(getClassRep() != NULL,
               "Cannot get tag from non-declared dynamic class");
   return getClassRep()->getClassName();
}

inline const AbstractClassRep::Field * ConsoleObject::findField(StringTableEntry name) const
{
   AssertFatal(getClassRep() != NULL,
               "Cannot get tag from non-declared dynamic class");
   return getClassRep()->findField(name);
}

inline bool ConsoleObject::setField(const char *fieldName, const char *value)
{
	//sanity check
	if ((! fieldName) || (! fieldName[0]) || (! value))
		return false;

	if (! getClassRep())
		return false;
	const AbstractClassRep::Field *myField = getClassRep()->findField(StringTable->insert(fieldName));
	if (! myField)
		return false;
   Con::setData(myField->type, (void *) (S32(this) + myField->offset), 0, 1, &value, myField->table, myField->flag);
   return true;
}

inline ConsoleObject* ConsoleObject::create(const char* in_pClassName)
{
   return AbstractClassRep::create(in_pClassName);
}

inline ConsoleObject* ConsoleObject::create(const U32 in_classId)
{
   return AbstractClassRep::create(in_classId);
}

inline const AbstractClassRep::FieldList& ConsoleObject::getFieldList() const
{
   return getClassRep()->mFieldList;
}

//------------------------------------------------------------------------------
//-------------------------------------- MACROS for dynamic objects...
//
#define DECLARE_CONOBJECT(className)                      \
   static ConcreteClassRep<className> dynClassRep;      \
   static AbstractClassRep* getParentStaticClassRep(); \
   static AbstractClassRep* getStaticClassRep(); \
   virtual AbstractClassRep* getClassRep() const

#define IMPLEMENT_CONOBJECT(className)                         \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   ConcreteClassRep<className> className::dynClassRep(#className)

#define IMPLEMENT_CO_NETOBJECT_V1(className)                    \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   ConcreteClassRep<className> className::dynClassRep(#className,NetObjectClassFirst,1)

#define IMPLEMENT_CO_DATABLOCK_V1(className)                   \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   ConcreteClassRep<className> className::dynClassRep(#className,DataBlockClassFirst,1)

#define IMPLEMENT_CO_NETEVENT_V1(className)                    \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   ConcreteClassRep<className> className::dynClassRep(#className,NetEventClassFirst,1,NetEventClassAny)

#define IMPLEMENT_CO_CLIENTEVENT_V1(className)                    \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   ConcreteClassRep<className> className::dynClassRep(#className,NetEventClassFirst,1,NetEventClassClient)

#define IMPLEMENT_CO_SERVEREVENT_V1(className)                    \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; } \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; } \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   ConcreteClassRep<className> className::dynClassRep(#className,NetEventClassFirst,1,NetEventClassServer)

#endif //_CONSOLEOBJECT_H_
