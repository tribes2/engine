//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SIMBASE_H_
#define _SIMBASE_H_

#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _TALGORITHM_H_
#include "Core/tAlgorithm.h"
#endif
#ifndef _BITSET_H_
#include "Core/bitSet.h"
#endif

#ifndef _CONSOLEOBJECT_H_
#include "console/consoleObject.h"
#endif
#ifndef _SIMDICTIONARY_H_
#include "console/simDictionary.h"
#endif

//--------------------------------------------------------------------------- 

enum
{
   DataBlockObjectIdFirst = 3,
   DataBlockObjectIdBitSize = 10,
   DataBlockObjectIdLast = DataBlockObjectIdFirst + (1 << DataBlockObjectIdBitSize) - 1,

   DynamicObjectIdFirst = DataBlockObjectIdLast + 1,
   InvalidEventId = 0,
   RootGroupId = 0xFFFFFFFF,
};

class SimEvent;
class SimObject;
class SimGroup;
class SimManager;
class Namespace;
class BitStream;
class Stream;
class LightManager;

typedef U32 SimTime;
typedef U32 SimObjectId;

class SimObjectList: public VectorPtr<SimObject*>
{
   static S32 QSORT_CALLBACK compareId(const void* a,const void* b);
  public:
   void pushBack(SimObject*);
   void pushBackForce(SimObject*);
   void pushFront(SimObject*);
   void remove(SimObject*);
   void removeStable(SimObject* pObject) { /* remove is stable at the moment */ remove(pObject); }
   void sortId();
};

//---------------------------------------------------------------------------

class SimEvent
{
  public:
   SimEvent *nextEvent;
   SimTime time;
   U32 sequenceCount;
   SimObject *destObject;

   SimEvent() { destObject = NULL; }
   virtual ~SimEvent() {}  // dummy virtual destructor is required
                           // so that subclasses can be deleted properly
   virtual void process(SimObject *object)=0;
};

class SimConsoleEvent : public SimEvent
{
   S32 mArgc;
   char **mArgv;
   bool mOnObject;
  public:
   SimConsoleEvent(S32 argc, const char **argv, bool onObject);
   ~SimConsoleEvent();
   virtual void process(SimObject *object);
};

//--------------------------------------------------------------------------- 
class SimFieldDictionary 
{
   friend class SimFieldDictionaryIterator;

  public:
   struct Entry
   {
      StringTableEntry slotName;
      char *value;
      Entry *next;
   };
  private:
   enum
   {
      HashTableSize = 19
   };
   Entry *mHashTable[HashTableSize];
   
   static Entry *mFreeList;
   static void freeEntry(Entry *entry);
   static Entry *allocEntry();
  public:
   SimFieldDictionary();
   ~SimFieldDictionary();
   void setFieldValue(StringTableEntry slotName, const char *value);
   const char *getFieldValue(StringTableEntry slotName);
   void writeFields(SimObject *obj, Stream &strem, U32 tabStop);
   void printFields(SimObject *obj);
   void assignFrom(SimFieldDictionary *dict);
};

class SimFieldDictionaryIterator
{
   SimFieldDictionary *          mDictionary;
   S32                           mHashIndex;
   SimFieldDictionary::Entry *   mEntry;

  public:
   SimFieldDictionaryIterator(SimFieldDictionary*);
   SimFieldDictionary::Entry* operator++();
   SimFieldDictionary::Entry* operator*();
};

//--------------------------------------------------------------------------- 
class SimObject: public ConsoleObject
{
   typedef ConsoleObject Parent;

   friend class SimManager;
   friend class SimGroup;
   friend class SimNameDictionary;
   friend class SimManagerNameDictionary;
   friend class SimIdDictionary;

   //-------------------------------------- Structures and enumerations
  private:
   enum {
      Deleted   = 1 << 0,
      Removed   = 1 << 1,
      Added     = 1 << 3,
      Selected  = 1 << 4, 
      Expanded  = 1 << 5,
      ModStaticFields  = 1 << 6, // can read/modify static fields
      ModDynamicFields = 1 << 7 // can read/modify dynamic fields
   };
  public:
   struct Notify {
      enum Type {
         ClearNotify,
         DeleteNotify,
         ObjectRef,
         Invalid
      } type;
      void *ptr;
      Notify *next;
   };
   enum WriteFlags {
      SelectedOnly = BIT(0)
   };

  private:
   // dictionary information stored on the object
   StringTableEntry objectName;
   SimObject*       nextNameObject;
   SimObject*       nextManagerNameObject;
   SimObject*       nextIdObject;

   SimGroup*   mGroup;
   BitSet32    mFlags;
   Notify*     mNotifyList;

  protected:
   SimObjectId mId;
   Namespace*  mNameSpace;
   U32         mTypeMask;

  protected:
   static SimObject::Notify *mNotifyFreeList;
   static SimObject::Notify *allocNotify();
   static void freeNotify(SimObject::Notify*);
  private:
   SimFieldDictionary *mFieldDictionary;

  public:
   const char *getDataField(StringTableEntry slotName, const char *array);
   void setDataField(StringTableEntry slotName, const char *array, const char *value);
   SimFieldDictionary * getFieldDictionary() {return(mFieldDictionary);}

   SimObject();
   virtual ~SimObject();
   virtual bool processArguments(S32 argc, const char **argv);

   virtual bool onAdd();
   virtual void onRemove();
   virtual void onGroupAdd();
   virtual void onGroupRemove();
   virtual void onNameChange(const char *name);
   virtual void onStaticModified(const char* slotName);
   virtual void inspectPreApply();
   virtual void inspectPostApply();

   virtual void onDeleteNotify(SimObject *object);

   virtual void onEditorEnable(){};
   virtual void onEditorDisable(){};
   
   virtual SimObject *findObject(const char *name); // find a named sub-object of this object
   
   Notify *removeNotify(void *ptr, Notify::Type);
   void deleteNotify(SimObject* obj);
   void clearNotify(SimObject* obj);
   void clearAllNotifications();
   void processDeleteNotifies();
   void registerReference(SimObject **obj);
   void unregisterReference(SimObject **obj);

   bool registerObject();
   bool registerObject(U32 id);
   bool registerObject(const char *name);
   bool registerObject(const char *name, U32 id);
   
   void unregisterObject();
   void deleteObject();

   SimObjectId getId() const { return mId; }
   const char* getIdString();
   U32         getType() const  { return mTypeMask; }
   const char* getName() const { return objectName; };

   void setId(SimObjectId id);
   void assignName(const char* name);
   SimGroup* getGroup() const { return mGroup; }
   bool isProperlyAdded() const { return mFlags.test(Added); }
   bool isDeleted() const { return mFlags.test(Deleted); }
   bool isRemoved() const { return mFlags.test(Deleted | Removed); }
   bool isLocked();
   void setLocked( bool b );
   bool isHidden();
   void setHidden(bool b);
   
   bool addToSet(SimObjectId);
   bool addToSet(const char *);
   bool removeFromSet(SimObjectId);
   bool removeFromSet(const char *);

   virtual void write(Stream &stream, U32 tabStop, U32 flags = 0);
   void writeFields(Stream &stream, U32 tabStop);
   void assignFieldsFrom(SimObject *obj);

   Namespace* getNamespace() { return mNameSpace; }
   const char *tabComplete(const char *prevText, S32 baseLen, bool);

   bool isSelected() const { return mFlags.test(Selected); }
   bool isExpanded() const { return mFlags.test(Expanded); }
   void setSelected(bool sel) { if(sel) mFlags.set(Selected); else mFlags.clear(Selected); }
   void setExpanded(bool exp) { if(exp) mFlags.set(Expanded); else mFlags.clear(Expanded); }
   void setModDynamicFields(bool dyn) { if(dyn) mFlags.set(ModDynamicFields); else mFlags.clear(ModDynamicFields); }
   void setModStaticFields(bool sta) { if(sta) mFlags.set(ModStaticFields); else mFlags.clear(ModStaticFields); }

   virtual void registerLights(LightManager *, bool) {};

   static void consoleInit();

   DECLARE_CONOBJECT(SimObject);
};

//--------------------------------------------------------------------------- 

template <class T> class SimObjectPtr
{
  private:
   SimObject *mObj;
   
  public:
   SimObjectPtr() { mObj = 0; }
   SimObjectPtr(T* ptr)
   {
      mObj = ptr;
      if(mObj)
         mObj->registerReference(&mObj);
   }
   SimObjectPtr(const SimObjectPtr<T>& rhs)
   {
      mObj = const_cast<T*>(static_cast<const T*>(rhs));
      if(mObj)
         mObj->registerReference(&mObj);   
   }
   SimObjectPtr<T>& operator=(const SimObjectPtr<T>& rhs)
   {
      if(this == &rhs)
         return(*this);
      if(mObj)
         mObj->unregisterReference(&mObj);
      mObj = const_cast<T*>(static_cast<const T*>(rhs));
      if(mObj)
         mObj->registerReference(&mObj);
      return(*this);
   }
   ~SimObjectPtr()
   {
      if(mObj)
         mObj->unregisterReference(&mObj);
   }
   SimObjectPtr<T>& operator= (T *ptr)
   {
      if(mObj != (SimObject *) ptr)
      {
         if(mObj)
            mObj->unregisterReference(&mObj);
         mObj = (SimObject *) ptr;
         if (mObj)
            mObj->registerReference(&mObj);
      }
      return *this;
   }
   operator bool() const  { return mObj != 0; }
   bool isNull() const  { return mObj == 0; }
   T* operator->()   { return static_cast<T*>(mObj); }
   T& operator*()    { return *static_cast<T*>(mObj); }
   operator T*()     { return static_cast<T*>(mObj)? static_cast<T*>(mObj) : 0; }
   const T* operator->() const  { return static_cast<const T*>(mObj); }
   const T& operator*() const   { return *static_cast<const T*>(mObj); }
   operator const T*() const    { return static_cast<T*>(mObj) ? static_cast<const T*>(mObj) : 0; }
};

//--------------------------------------------------------------------------- 
class SimDataBlock: public SimObject
{
   typedef SimObject Parent;

  protected:
   S32  modifiedKey;

  public:
   static SimObjectId sNextObjectId;
   static S32         sNextModifiedKey;

   SimDataBlock();
   DECLARE_CONOBJECT(SimDataBlock);

   static S32 getNextModifiedKey() { return sNextModifiedKey; }
   S32 getModifiedKey() const { return modifiedKey; }

   bool onAdd();
   void onStaticModified(const char* slotName);
   void setLastError(const char*);
   void assignId();

   virtual bool preload(bool server, char errorBuffer[256]);

   //
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};

//--------------------------------------------------------------------------- 
class SimSet: public SimObject
{
   typedef SimObject Parent;
  protected:
   SimObjectList objectList;
   
  public:
   SimSet() {
      VECTOR_SET_ASSOCIATION(objectList);
   }

   typedef SimObjectList::iterator iterator;
   typedef SimObjectList::value_type value;
   SimObject* front() { return objectList.front(); }
   SimObject* first() { return objectList.first(); }
   SimObject* last()  { return objectList.last(); }
   bool       empty() { return objectList.empty();   }
   S32        size()  { return objectList.size(); }
   iterator   begin() { return objectList.begin(); }
   iterator   end()   { return objectList.end(); }
   value operator[] (S32 index) { return objectList[U32(index)]; }

   iterator find( iterator first, iterator last, SimObject *obj)
   { return ::find(first, last, obj); }

   bool reOrder( SimObject *obj, SimObject *target=0 );

   virtual void onRemove();
   virtual void onDeleteNotify(SimObject *object);

   virtual void addObject(SimObject*);
   virtual void removeObject(SimObject*);

   virtual void pushObject(SimObject*);
   virtual void popObject();

   void bringObjectToFront(SimObject* obj) { reOrder(obj, front()); }
   void pushObjectToBack(SimObject* obj) { reOrder(obj, NULL); }

   void write(Stream &stream, U32 tabStop, U32 flags = 0);

   virtual SimObject *findObject(const char *name); // find a named sub-object of this object

   DECLARE_CONOBJECT(SimSet);
};

class SimSetIterator
{
   struct Entry {
      SimSet* set;
      SimSet::iterator itr;
   };
   class Stack: public Vector<Entry> {
     public:
      void push_back(SimSet*);
   };
   Stack stack;

  public:
   SimSetIterator(SimSet*);
   SimObject* operator++();
   SimObject* operator*() {
      return stack.empty()? 0: *stack.last().itr;
   }
};

//--------------------------------------------------------------------------- 
class SimGroup: public SimSet
{
  private:
   friend class SimManager;
   friend class SimObject;
   
   typedef SimSet Parent; 
   SimNameDictionary nameDictionary;

  public:
   ~SimGroup();

   void addObject(SimObject*);
   void addObject(SimObject*, SimObjectId);
   void addObject(SimObject*, const char *name);
   void removeObject(SimObject*);
   void onRemove();

   virtual SimObject* findObject(const char* name);

   bool processArguments(S32 argc, const char **argv);
   
   DECLARE_CONOBJECT(SimGroup);
};

inline void SimGroup::addObject(SimObject* obj, SimObjectId id)
{
   // AddObject will assign it whatever id it already has.
   // This should normally be done only with reserved id's.
   obj->mId = id;
   addObject( obj );
}

inline void SimGroup::addObject(SimObject *obj, const char *name)
{
   addObject( obj );
   obj->assignName(name);
}   

//--------------------------------------------------------------------------- 

class SimDataBlockGroup : public SimGroup
{
  private:
   S32 mLastModifiedKey;
   
  public:
   static S32 QSORT_CALLBACK compareModifiedKey(const void* a,const void* b);
   void sort();
   SimDataBlockGroup();
};

//--------------------------------------------------------------------------- 

// helper macros for named sets and groups in the manager:

#define DeclareNamedSet(set) extern SimSet *g##set;inline SimSet *get##set() { return g##set; }
#define DeclareNamedGroup(set) extern SimGroup *g##set;inline SimGroup *get##set() { return g##set; }
#define ImplementNamedSet(set) SimSet *g##set;
#define ImplementNamedGroup(set) SimGroup *g##set;

//--------------------------------------------------------------------------- 

namespace Sim
{
   DeclareNamedSet(ActiveActionMapSet)
   DeclareNamedSet(GhostAlwaysSet)
   DeclareNamedSet(LightSet)
   DeclareNamedSet(WayPointSet)
   DeclareNamedSet(ClientTargetSet)
   DeclareNamedSet(ServerTargetSet)
   DeclareNamedSet(FlareSet)
   DeclareNamedSet(MissileSet)
   DeclareNamedSet(CommandTargetSet)
   DeclareNamedSet(ScopeSensorVisibleSet)
   DeclareNamedGroup(ActionMapGroup)
   DeclareNamedGroup(ClientGroup)
   DeclareNamedGroup(GuiGroup)
   DeclareNamedGroup(GuiDataGroup)
   DeclareNamedGroup(TCPGroup)
   DeclareNamedGroup(ClientConnectionGroup)

   void init();
   void shutdown();

   SimDataBlockGroup *getDataBlockGroup();
   SimGroup* getRootGroup();

   SimObject* findObject(SimObjectId);
   SimObject* findObject(const char* name);
   template<class T> inline bool findObject(SimObjectId id,T*&t)
   {
      t = dynamic_cast<T*>(findObject(id));
      return t != NULL;
   }
   template<class T> inline bool findObject(const char *objectName,T*&t)
   {
      t = dynamic_cast<T*>(findObject(objectName));
      return t != NULL;
   }

   void advanceToTime(SimTime time);
   void advanceTime(SimTime delta);
   SimTime getCurrentTime();
   SimTime getTargetTime();

   // a target time of 0 on an event means current event
   U32 postEvent(SimObject*, SimEvent*, U32 targetTime);

   inline U32 postEvent(SimObjectId id,SimEvent*evt, U32 targetTime)
   {
      return postEvent(findObject(id), evt, targetTime);
   }
   inline U32 postEvent(const char *objectName,SimEvent*evt, U32 targetTime)
   {
      return postEvent(findObject(objectName), evt, targetTime);
   }
   inline U32 postCurrentEvent(SimObject*obj, SimEvent*evt)
   {
      return postEvent(obj,evt,getCurrentTime());
   }
   inline U32 postCurrentEvent(SimObjectId obj,SimEvent*evt)
   {
      return postEvent(obj,evt,getCurrentTime());
   }
   inline U32 postCurrentEvent(const char *obj,SimEvent*evt)
   {
      return postEvent(obj,evt,getCurrentTime());
   }
   
   void cancelEvent(U32 eventId);
   bool isEventPending(U32 eventId);
}

//----------------------------------------------------------------------------
#define IMPLEMENT_SETDATATYPE(T)                                                                  \
void setDataTypeDataBlockPtr##T(void *dptr, S32 argc, const char **argv, EnumTable *, BitSet32)   \
{                                                                                                 \
   volatile SimDataBlock* pConstraint = static_cast<SimDataBlock*>((T*)NULL);                     \
                                                                                                  \
   if (argc == 1) {                                                                               \
      *reinterpret_cast<T**>(dptr) = NULL;                                                        \
      if (argv[0] && argv[0][0] && !Sim::findObject(argv[0],*reinterpret_cast<T**>(dptr)))        \
         Con::printf("Object \"%s\" is not a member of the expected data block class", argv[0]);  \
   }                                                                                              \
   else                                                                                           \
      Con::printf("Cannot set multiple args to a single pointer.");                               \
}

#define IMPLEMENT_GETDATATYPE(T)                                                    \
const char* getDataTypeDataBlockPtr##T(void *dptr, EnumTable *, BitSet32)           \
{                                                                                   \
   volatile SimDataBlock* pConstraint = static_cast<SimDataBlock*>((T*)NULL);       \
                                                                                    \
   T** obj = reinterpret_cast<T**>(dptr);                                           \
   return *obj ? (*obj)->getName() : "";                                            \
}

#define REF_SETDATATYPE(T) setDataTypeDataBlockPtr##T
#define REF_GETDATATYPE(T) getDataTypeDataBlockPtr##T

//--------------------------------------------------------------------------- 

#endif
