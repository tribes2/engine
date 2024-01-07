//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _RESMANAGER_H_
#define _RESMANAGER_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif
#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif

#ifndef _FILESTREAM_H_
#include "core/fileStream.h"
#endif
#ifndef _ZIPSUBSTREAM_H_
#include "core/zipSubStream.h"
#endif
#ifndef _ZIPAGGREGATE_H_
#include "core/zipAggregate.h"
#endif
#ifndef _ZIPHEADERS_H_
#include "core/zipHeaders.h"
#endif


#define RES_DEFAULT_TIMEOUT (5*60*1000)   //5-minutes

class Stream;
class FileStream;
class ZipSubRStream;
class ResManager;
class FindMatch;


extern ResManager *ResourceManager;

//------------------------------------------------------------------------------
// Basic resource manager behavior
// -set the mod path
//    ResManager scans directory tree under listed base directories
//    Any .rpk (.zip) file in the root directory of a mod is added (scanned)
//    for resources
//    Any files currently in the resource manager become memory resources
//    They can be "reattached" to the first file that matches the file name
//
//------------------------------------------------------------------------------
// all classes which wish to be handled by the resource manager 
// need to be 
//    1) derrived from ResourceInstance
//    2) register a creation function and file extension with the manager

class ResourceInstance
{
private:
public:
   virtual ~ResourceInstance() {}
};


typedef ResourceInstance* (*RESOURCE_CREATE_FN)(Stream &stream);


//------------------------------------------------------------------------------
#define InvalidCRC 0xFFFFFFFF

class ResourceObject
{
   friend class ResDictionary;
   friend class ResManager;

   ResourceObject *prev, *next; // timeout list
   ResourceObject *nextEntry; // objects are inserted by id or name
   ResourceObject *nextResource; // in linked list of all resources
   ResourceObject *prevResource;
public:
   enum
   {
      VolumeBlock   = 1 << 0,
      File          = 1 << 1,
      Added         = 1 << 2,
   };
   S32 flags;

   StringTableEntry path;     // resource path
   StringTableEntry name;     // resource name

   StringTableEntry filePath; // path/name of file of volume if in volume
   StringTableEntry fileName;
   
   S32 fileOffset;            // offset on disk in fileName file of resource
   S32 fileSize;              // size on disk of resource block
   S32 compressedFileSize;    // Actual size of resource data.

   ResourceInstance *mInstance;     // ptr ot actual object instance
   S32 lockCount;
   U32 crc;

   ResourceObject();
   ~ResourceObject() { unlink(); }

   void destruct();

   ResourceObject* getNext() const { return next; }
   void unlink();
   void linkAfter(ResourceObject* res);
   void getFileTimes(FileTime *createTime, FileTime *modifyTime);
};   


inline void ResourceObject::unlink()
{
   if (next)
      next->prev = prev;
   if (prev)
      prev->next = next;
   next = prev = 0;
}

inline void ResourceObject::linkAfter(ResourceObject* res)
{
   unlink();
   prev = res;
   if ((next = res->next) != 0)
      next->prev = this;
   res->next = this;
}


//------------------------------------------------------------------------------
template <class T> class Resource
{
private:
   ResourceObject *obj;
   // ***WARNING***
   // Using a faster lock that bypasses the resource manger.
   // void _lock() { if (obj) obj->rm->lockResource( obj ); }
   void _lock();
   void _unlock();

public:
   // If assigned a ResourceObject, it's assumed to already have
   // been locked, lock count is incremented only for copies or
   // assignment from another Resource.
   Resource() : obj(NULL) { ; }
   Resource(ResourceObject *p) : obj(p) { ; }
   Resource(const Resource &res) : obj(res.obj) { _lock(); }
   ~Resource() { unlock(); }
   
   const char *getFilePath() const { return (obj ? obj->path : NULL); }
   const char *getFileName() const { return (obj ? obj->name : NULL); }

   Resource& operator= (ResourceObject *p) { _unlock(); obj = p; return *this; }
   Resource& operator= (const Resource &r) { _unlock(); obj = r.obj; _lock(); return *this; }

   U32 getCRC() { return (obj ? obj->crc : 0); }
   operator bool() const { return ((obj != NULL) && (obj->mInstance != NULL)); }
   T* operator->()   { return (T*)obj->mInstance; }
   T& operator*()    { return *((T*)obj->mInstance); }
   operator T*()     { return (obj) ? (T*)obj->mInstance : (T*)NULL; }
   const T* operator->() const  { return (const T*)obj->mInstance; }
   const T& operator*() const   { return *((const T*)obj->mInstance); }
   operator const T*() const    { return (obj) ? (const T*)obj->mInstance :  (const T*)NULL; }
   void unlock();
   void purge();
};

template<class T> inline void Resource<T>::unlock()
{
   if (obj) {
      ResourceManager->unlock( obj );
      obj=NULL;
   }
}

template<class T> inline void Resource<T>::purge()
{
   if (obj) { 
      ResourceManager->unlock( obj );
      if (obj->lockCount == 0)
         ResourceManager->purge(obj); 
      obj = NULL;
   }
}
template <class T> inline void Resource<T>::_lock()
{
   if (obj)
      obj->lockCount++;
}

template <class T> inline void Resource<T>::_unlock()
{
   if (obj)
      ResourceManager->unlock( obj );
}

#define INVALID_ID ((U32)(~0))

//----------------------------------------------------------------------------
// Map of Names and Object IDs to objects
// Provides fast lookup for name->object, id->object and
// for fast removal of an object given object*
//
class ResDictionary
{
   enum { DefaultTableSize = 1029 };

   ResourceObject **hashTable;
   S32 entryCount;
   S32 hashTableSize;
   DataChunker memPool;
   S32 hash(StringTableEntry path, StringTableEntry name);
   S32 hash(ResourceObject *obj) { return hash(obj->path, obj->name); }
public:
   ResDictionary();
   ~ResDictionary();

   void insert(ResourceObject *obj, StringTableEntry path, StringTableEntry file);
   ResourceObject* find(StringTableEntry path, StringTableEntry file);
   ResourceObject* find(StringTableEntry path, StringTableEntry file, StringTableEntry filePath, StringTableEntry fileName);
   ResourceObject* find(StringTableEntry path, StringTableEntry file, U32 flags);
   void pushBehind(ResourceObject *obj, S32 mask);
   void remove(ResourceObject *obj);
};


//------------------------------------------------------------------------------
class ResManager
{
private:
   char writeablePath[1024];
   char primaryPath[1024];
   char* pathList;
   
   ResourceObject timeoutList;
   ResourceObject resourceList;

   ResDictionary dictionary;
   bool echoFileNames;

   bool scanZip(ResourceObject *zipObject);
   
   ResourceObject* createResource(StringTableEntry path, StringTableEntry file, StringTableEntry filePath, StringTableEntry fileName);
   void freeResource(ResourceObject *resObject);
   void searchPath(const char *pathStart);

   struct RegisteredExtension
   {
      StringTableEntry     mExtension;
      RESOURCE_CREATE_FN   mCreateFn;         
      RegisteredExtension  *next;
   };
   
   RegisteredExtension *registeredList;

   ResManager();
public:
   RESOURCE_CREATE_FN getCreateFunction( const char *name );

   ~ResManager();
   static void create();
   static void destroy();

   void setFileNameEcho(bool on);
   void setModPaths(U32 numPaths, const char **dirs);
   const char* getModPaths();

   void registerExtension(const char *extension, RESOURCE_CREATE_FN create_fn);

   S32 getSize(const char* filename);
   const char* getFullPath(const char * filename, char * path, U32 pathLen);
   const char* getModPathOf(const char* fileName);
   const char* getPathOf(const char * filename);
   const char* getBasePath();

   ResourceObject* load(const char * fileName, bool computeCRC = false);
   Stream*  openStream(const char * fileName);
   Stream*  openStream(ResourceObject *object);
   void     closeStream(Stream *stream);

   void unlock( ResourceObject* );
   bool add(const char* name, ResourceInstance *addInstance, bool extraLock = false);

   ResourceObject* find(const char * fileName);
   ResourceInstance* loadInstance(const char *fileName, bool computeCRC = false);
   ResourceInstance* loadInstance(ResourceObject *object, bool computeCRC = false);

   ResourceObject* find(const char * fileName, U32 flags);
   ResourceObject* findMatch(const char *expression, const char **fn, ResourceObject *start = NULL);

   void purge();
   void purge( ResourceObject *obj );
   void serialize(VectorPtr<const char *> &filenames);
   
   S32  findMatches( FindMatch *pFM );
   bool findFile( const char *name );
   
   bool getCrc(const char * fileName, U32 & crcVal, const U32 crcInitialVal = 0xffffffff );

   void setWriteablePath(const char *path);
   bool isValidWriteFileName(const char *fn);
   
   bool openFileForWrite(FileStream &fs, const char *modPath, const char *fileName, U32 accessMode = 1);

#ifdef DEBUG
   void dumpLoadedResources();
#endif
};

// simple crc - may need to move somewhere else someday
// will generate the table on the first call
U32 calculateCRC(void * buffer, S32 len, U32 crcVal = 0xffffffff);
U32 calculateCRCStream(Stream *stream, U32 crcVal = 0xffffffff);

#endif //_RESMANAGER_H_
