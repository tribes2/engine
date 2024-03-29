//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "core/tVector.h"
#include "core/stream.h"

#include "core/fileStream.h"
#include "core/zipSubStream.h"
#include "core/zipAggregate.h"
#include "core/zipHeaders.h"
#include "core/resizeStream.h"
#include "sim/frameAllocator.h"

#include "core/resManager.h"
#include "core/findMatch.h"

#include "console/console.h"

ResManager* ResourceManager = NULL;
static char sgCurExeDir[1024];
static S32 sgCurExeDirStrLen;

//------------------------------------------------------------------------------
ResourceObject::ResourceObject() 
{
   next = NULL;
   prev = NULL;
   lockCount  = 0;
   mInstance  = NULL;
}

void ResourceObject::destruct()
{
   // If the resource was not loaded because of an error, the resource
   // pointer will be NULL
   if (mInstance) {
      delete mInstance;
      mInstance = NULL;   
   }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

ResManager::ResManager()
{
   echoFileNames = 0;
   primaryPath[0] = 0;
   writeablePath[0] = 0;
   pathList = NULL;
   resourceList.nextResource = NULL;
   resourceList.next = NULL;
   resourceList.prev = NULL;
   timeoutList.nextResource = NULL;
   timeoutList.next = NULL;
   timeoutList.prev = NULL;
   sgCurExeDir[0] = '\0';
   Platform::getCurrentDirectory(sgCurExeDir, 1024);
   sgCurExeDirStrLen = dStrlen(sgCurExeDir);
   registeredList = NULL;
}

void ResourceObject::getFileTimes(FileTime *createTime, FileTime *modifyTime)
{
   char buffer[1024]; 
#ifdef __linux
   dSprintf( buffer, sizeof( buffer ), "%s/%s", filePath, fileName );
#else
   dSprintf(buffer, sizeof(buffer), "%s/%s/%s", sgCurExeDir, filePath, fileName);
#endif
   Platform::getFileTimes(buffer, createTime, modifyTime);
}

//------------------------------------------------------------------------------
ResManager::~ResManager()
{
   purge();
   // volume list should be gone.

   if ( pathList )
      dFree( pathList );

   for(ResourceObject *walk = resourceList.nextResource; walk; walk = walk->nextResource)
      walk->destruct();
   
   while(resourceList.nextResource)
      freeResource(resourceList.nextResource);

   while(registeredList)
   {
      RegisteredExtension *temp = registeredList->next;
      delete registeredList;
      registeredList = temp;   
   }
}

#ifdef DEBUG
void ResManager::dumpLoadedResources()
{
   ResourceObject* walk = resourceList.nextResource;
   while (walk != NULL)
   {
      if (walk->mInstance != NULL)
      {
         Con::errorf("LoadedRes: %s/%s (%d)", walk->path, walk->name, walk->lockCount);
      }
      walk = walk->nextResource;
   }
}
#endif

//------------------------------------------------------------------------------
void ResManager::create()
{
   AssertFatal(ResourceManager == NULL, "ResourceManager::create: manager already exists.");
   ResourceManager = new ResManager;
}  

 
//------------------------------------------------------------------------------
void ResManager::destroy()
{
   AssertFatal(ResourceManager != NULL, "ResourceManager::destroy: manager does not exist.");
   delete ResourceManager;
   ResourceManager = NULL;
}   

//------------------------------------------------------------------------------

void ResManager::setFileNameEcho(bool on)
{
   echoFileNames = on;
}

//------------------------------------------------------------------------------

bool ResManager::isValidWriteFileName(const char *fn)
{
   // all files must be based off the VFS
   if(fn[0] == '/' || dStrchr(fn, ':'))
      return false;
   
   if(!writeablePath[0])
      return true;

   // get the path to the file
   const char *path = dStrrchr(fn, '/');
   if(!path)
      path = fn;
   else
   {
      if(!dStrchr(path, '.'))
         return false;
   }
   // now loop through the writeable path.
   const char *start = writeablePath;
   S32 pathLen = path - fn;
   for(;;)
   {
      const char *end = dStrchr(writeablePath, ';');
      if(!end)
         end = writeablePath + dStrlen(writeablePath);
         
      if(end - start == pathLen && !dStrnicmp(start, path, pathLen))
         return true;
      if(end[0])
         start = end + 1;
      else
         break;
   }   
   return false;
}

void ResManager::setWriteablePath(const char *path)
{
   dStrcpy(writeablePath, path);
}

//------------------------------------------------------------------------------
static const char *buildPath(StringTableEntry path, StringTableEntry file)
{
   static char buf[1024];
   if(path)
      dSprintf(buf, sizeof(buf), "%s/%s", path, file);
   else
      dStrcpy(buf, file);
   return buf;
}

//------------------------------------------------------------------------------
static void getPaths(const char *fullPath, StringTableEntry &path, StringTableEntry &fileName)
{
   static char buf[1024];
   char *ptr = (char *) dStrrchr(fullPath, '/');
   if(!ptr)
   {
      path = NULL;
      fileName = StringTable->insert(fullPath);
   }
   else
   {
      S32 len = ptr - fullPath;
      dStrncpy(buf, fullPath, len);
      buf[len] = 0;
      fileName = StringTable->insert(ptr + 1);
      path = StringTable->insert(buf);
   }
}

//------------------------------------------------------------------------------
bool ResManager::scanZip(ResourceObject *zipObject)
{
   // now open the volume and add all its resources to the dictionary
   ZipAggregate zipAggregate;
   if (zipAggregate.openAggregate(buildPath(zipObject->filePath, zipObject->fileName)) == false) {
      AssertFatal(false, "Error opening zip, need to handle this better...");
      return false;
   }
   ZipAggregate::iterator itr;
   for (itr = zipAggregate.begin(); itr != zipAggregate.end(); itr++) {
      const ZipAggregate::FileEntry& rEntry = *itr;

      ResourceObject* ro = createResource(rEntry.pPath, rEntry.pFileName,
                                          zipObject->filePath, zipObject->fileName);

      ro->flags                = ResourceObject::VolumeBlock;
      ro->fileSize             = rEntry.fileSize;
      ro->compressedFileSize   = rEntry.compressedFileSize;
      ro->fileOffset           = rEntry.fileOffset;

      dictionary.pushBehind(ro, ResourceObject::File);
   }
   zipAggregate.closeAggregate();

   return true;
}

//------------------------------------------------------------------------------
void ResManager::searchPath(const char* basePath)
{
   AssertFatal(basePath != NULL, "No path to dump?");

   Vector<Platform::FileInfo> fileInfoVec;
   Platform::dumpPath(basePath, fileInfoVec);

   for (U32 i = 0; i < fileInfoVec.size(); i++) {
      Platform::FileInfo& rInfo = fileInfoVec[i];

      // Create a resource for this file...
      //
      ResourceObject *ro = createResource(rInfo.pVirtPath, rInfo.pFileName, rInfo.pFullPath, rInfo.pFileName);
      dictionary.pushBehind(ro, ResourceObject::File);

      ro->flags                = ResourceObject::File;
      ro->fileOffset           = 0;
      ro->fileSize             = rInfo.fileSize;
      ro->compressedFileSize   = rInfo.fileSize;

      // see if it's a zip
      const char *extension = dStrrchr(ro->fileName, '.');
      if(extension && !dStricmp(extension, ".zip"))
         scanZip(ro);
   }
}

//------------------------------------------------------------------------------
void ResManager::setModPaths(U32 numPaths, const char **paths)
{
   // detach all the files.
   for(ResourceObject *pwalk = resourceList.nextResource; pwalk; pwalk = pwalk->nextResource)
      pwalk->flags = ResourceObject::Added;

   bool primaryWriteSet = false;
   primaryPath[0] = 0;
   U32 pathLen = 0;
   
   U32 i;
   for(i = 0; i < numPaths; i++)
   {
      // silent fail on any invalid paths
      if(dStrchr(paths[i], '/') || dStrchr(paths[i], '.') || dStrchr(paths[i], ':') || dStrlen(paths[i]) == 0)
         continue;
      if(!primaryWriteSet)
      {
         dStrcpy(primaryPath, paths[i]);
         primaryWriteSet = true;
      }
      pathLen += ( dStrlen( paths[i] ) + 1 );
      searchPath(paths[i]);
#ifdef __linux
      // FIXME: Add this to Platform:: and implement this for Windows/PPC.
      extern bool platformGetUserPath( const char*, char*, U32 );
      char user[256];

      if( platformGetUserPath( paths[i], user, 256 ) ) {
	      searchPath( user );
      }
#endif
   }

   pathList = (char*) dRealloc( pathList, pathLen );
   dStrcpy( pathList, paths[0] );
   U32 strlen;
   for ( i = 1; i < numPaths; i++ )
   {
      strlen = dStrlen( pathList );
      dSprintf( pathList + strlen, pathLen - strlen, ";%s", paths[i] );
   }

   // unlink all the added baddies that aren't loaded.
   ResourceObject *rwalk = resourceList.nextResource, *rtemp;
   while(rwalk != NULL)
   {
      if((rwalk->flags & ResourceObject::Added) && !rwalk->mInstance)
      {
         rwalk->unlink();
         dictionary.remove(rwalk);
         rtemp = rwalk->nextResource;
         freeResource(rwalk);
         rwalk = rtemp;
      }
      else
         rwalk = rwalk->nextResource;
   }
}

//------------------------------------------------------------------------------
const char* ResManager::getModPaths()
{
   return( (const char*) pathList );
}

//------------------------------------------------------------------------------

S32 ResManager::getSize(const char *fileName)
{
   ResourceObject *ro = find(fileName);
   if(!ro)
      return 0;
   else
      return ro->fileSize;
}

//------------------------------------------------------------------------------
const char* ResManager::getFullPath(const char* fileName, char *path, U32 pathlen)
{
   AssertFatal(fileName, "ResourceManager::getFullPath: fileName is NULL");
   AssertFatal(path, "ResourceManager::getFullPath: path is NULL");
   ResourceObject *obj = find(fileName);
   if(!obj)
      dStrcpy(path, fileName);
   else
      dSprintf(path, pathlen, "%s/%s", obj->filePath, fileName);
   return path;
}   

//------------------------------------------------------------------------------
const char* ResManager::getPathOf(const char* fileName)
{
   AssertFatal(fileName, "ResourceManager::getPathOf: fileName is NULL");
   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;
   else
      return obj->filePath;
}   

//------------------------------------------------------------------------------
const char* ResManager::getModPathOf(const char* fileName)
{
   AssertFatal(fileName, "ResourceManager::getModPathOf: fileName is NULL");
   
   if (!pathList)
      return NULL;

   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;

   char buffer[256];
   char *base;
   const char *list = pathList;
   do
   {
      base = buffer;
      *base = 0;
      while (*list && *list != ';')
      {
         *base++ = *list++;
      }
      if (*list == ';')
         ++list;

      *base = 0;

      if (dStrncmp(buffer, obj->filePath, (base-buffer)) == 0)
         return StringTable->insert(buffer);

   }while(*list);

   return NULL;
}   

//------------------------------------------------------------------------------
const char* ResManager::getBasePath()
{
   if (!pathList) 
      return NULL;
   const char *base = dStrrchr(pathList, ';');
   return base ? (base+1) : pathList;
}   


//------------------------------------------------------------------------------

void ResManager::registerExtension(const char *name, RESOURCE_CREATE_FN create_fn)
{
   AssertFatal(!getCreateFunction(name), "ResourceManager::registerExtension: file extension already registered.");

   const char *extension = dStrrchr( name, '.' );
   AssertFatal(extension, "ResourceManager::registerExtension: file has no extension.");

   RegisteredExtension *add = new RegisteredExtension;
   add->mExtension = StringTable->insert(extension);
   add->mCreateFn  = create_fn;
   add->next = registeredList;
   registeredList = add;
}   

//------------------------------------------------------------------------------
RESOURCE_CREATE_FN ResManager::getCreateFunction( const char *name )
{
   const char *s = dStrrchr( name, '.' );
   if (!s) return (NULL);

   RegisteredExtension *itr = registeredList;
   while (itr)
   {
      if (dStricmp(s, itr->mExtension) == 0)
         return (itr->mCreateFn);
      itr = itr->next;
   }
   return (NULL);
}


//------------------------------------------------------------------------------
void ResManager::unlock(ResourceObject *obj)
{
   if (!obj) return;
   AssertFatal(obj->lockCount > 0, "ResourceManager::unlock: lock count is zero.");
   //set the timeout to the max requested 
   if (--obj->lockCount == 0)
      obj->linkAfter(&timeoutList);
}

//------------------------------------------------------------------------------
// gets the crc of the file, ignores the stream type
bool ResManager::getCrc(const char * fileName, U32 & crcVal, const U32 crcInitialVal )
{
   ResourceObject * obj = find(fileName);
   if(!obj) 
      return(false);

   // check if in a volume
   if(obj->flags & (ResourceObject::VolumeBlock | ResourceObject::File))
   {
      // can't crc locked resources...
      if(obj->lockCount)
         return false;
      
      // get rid of the resource
      // have to make sure user can't have it sitting around in the resource cache

      obj->unlink();
      obj->destruct();

      Stream *stream = openStream(obj);
      
      U32 waterMark = 0xFFFFFFFF;

      U8 *buffer;
      U32 maxSize = FrameAllocator::getHighWaterMark() - FrameAllocator::getWaterMark();
      if(maxSize < obj->fileSize)
         buffer = new U8[obj->fileSize];
      else
      {
         waterMark = FrameAllocator::getWaterMark();
         buffer = (U8*) FrameAllocator::alloc(obj->fileSize);
      }
      stream->read(obj->fileSize, buffer);
      // get the crc value
      crcVal = calculateCRC(buffer, obj->fileSize, crcInitialVal);
      if(waterMark == 0xFFFFFFFF)
         delete [] buffer;
      else
         FrameAllocator::setWaterMark(waterMark);

      closeStream(stream);
         
      return(true);
   }
   
   return(false);
}

//------------------------------------------------------------------------------

ResourceObject* ResManager::load(const char *fileName, bool computeCRC)
{
   // if filename is not known, exit now
   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;

   // if noone has a lock on this, but it's loaded and it needs to
   // be CRC'd, delete it and reload it.
   if(!obj->lockCount && computeCRC && obj->mInstance)
      obj->destruct();

   obj->lockCount++;
   obj->unlink();    // remove from purge list
   if(!obj->mInstance)
   {
      obj->mInstance = loadInstance(obj, computeCRC);
      if(!obj->mInstance)
      {
         obj->lockCount--;
         return NULL;
      }
   }
   return obj;
}

//------------------------------------------------------------------------------
ResourceInstance *ResManager::loadInstance(const char *fileName, bool computeCRC)
{
   // if filename is not known, exit now
   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;

   return loadInstance(obj, computeCRC);
}

//------------------------------------------------------------------------------

static const char *alwaysCRCList = ".ter.dif.dts";

ResourceInstance *ResManager::loadInstance(ResourceObject *obj, bool computeCRC)
{      
   Stream *stream = openStream(obj);
   if(!stream)
      return NULL;

   if(!computeCRC)
   {
      const char *x = dStrrchr(obj->name, '.');
      if(x && dStrstr(alwaysCRCList, x))
         computeCRC = true;
   }

   if(computeCRC)
      obj->crc = calculateCRCStream(stream, InvalidCRC);
   else
      obj->crc = InvalidCRC;

   RESOURCE_CREATE_FN createFunction = ResourceManager->getCreateFunction(obj->name);
   AssertFatal(createFunction, "ResourceObject::construct: NULL resource create function.");
   ResourceInstance *ret = createFunction( *stream );
   closeStream(stream);
   return ret;
}

//------------------------------------------------------------------------------
Stream* ResManager::openStream(const char * fileName)
{
   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;
   return openStream(obj);
}

//------------------------------------------------------------------------------
Stream* ResManager::openStream(ResourceObject *obj)
{
   // if filename is not known, exit now
   if(!obj)
      return NULL;

   if(echoFileNames)
      Con::printf("FILE ACCESS: %s/%s", obj->path, obj->name);

   // used for openStream stream access
   FileStream* diskStream = NULL;

   if(obj->flags & (ResourceObject::File | ResourceObject::VolumeBlock))
   {
      diskStream = new FileStream;
      diskStream->open(buildPath(obj->filePath, obj->fileName), FileStream::Read);

      if(obj->flags & ResourceObject::File) {
         obj->fileSize = diskStream->getStreamSize();
      }
      diskStream->setPosition(obj->fileOffset);

      if (obj->flags & ResourceObject::VolumeBlock)
      {
         ZipLocalFileHeader zlfHeader;

         if (zlfHeader.readFromStream(*diskStream) == false)
         {
            AssertFatal(false, avar("ResourceManager::loadStream: '%s' Not in the zip! (%s)", obj->name, obj->fileName));
            diskStream->close();
            return NULL;
         } 

         if (zlfHeader.m_header.compressionMethod == ZipLocalFileHeader::Stored || obj->fileSize == 0)
         {
            // Just read straight from the stream...
            ResizeFilterStream *strm = new ResizeFilterStream;
            strm->attachStream(diskStream);
            strm->setStreamOffset(diskStream->getPosition(), obj->fileSize);
            return strm;
         } 
         else 
         {
            if (zlfHeader.m_header.compressionMethod == ZipLocalFileHeader::Deflated)
            {
               ZipSubRStream* zipStream = new ZipSubRStream;
               zipStream->attachStream(diskStream);
               zipStream->setUncompressedSize(obj->fileSize);
               return zipStream;
            } 
            else 
            {
               AssertFatal(false, avar("ResourceManager::loadStream: '%s' Compressed inappropriately in the zip! (%s)", obj->name, obj->fileName));
               diskStream->close();
               return NULL;
            }
         }
      } 
      else 
      {
         return diskStream;
      }
   }
   return NULL;
}   


//------------------------------------------------------------------------------
void ResManager::closeStream(Stream *stream)
{
   FilterStream *subStream = dynamic_cast<FilterStream *>(stream);
   if(subStream)
   {
      stream = subStream->getStream();
      subStream->detachStream();
      delete subStream;
   }
   delete stream;
}   


//------------------------------------------------------------------------------
ResourceObject* ResManager::find(const char *fileName)
{
   if(!fileName)
      return NULL;
   StringTableEntry path, file;
   getPaths(fileName, path, file);
   return dictionary.find(path, file);
}

//------------------------------------------------------------------------------
ResourceObject* ResManager::find(const char *fileName, U32 flags)
{
   if(!fileName)
      return NULL;
   StringTableEntry path, file;
   getPaths(fileName, path, file);
   return dictionary.find(path, file, flags);
}


//------------------------------------------------------------------------------
// Add resource constructed outside the manager

bool ResManager::add(const char* name, ResourceInstance *addInstance, bool extraLock)
{
   StringTableEntry path, file;
   getPaths(name, path, file);

   ResourceObject* obj = dictionary.find(path, file);
   if (obj && obj->mInstance)
      // Resource already exists?
      return false;

   if (!obj)
      obj = createResource(path, file, NULL, NULL);

   dictionary.pushBehind(obj, ResourceObject::File | ResourceObject::VolumeBlock);
   obj->mInstance = addInstance;
   obj->lockCount = extraLock ? 2 : 1;
   unlock(obj);
   return true;
}

//------------------------------------------------------------------------------

void ResManager::purge()
{
   bool found;
   do {
      ResourceObject *obj = timeoutList.getNext();
      found = false;
      while(obj)
      {
         ResourceObject *temp = obj;
         obj = obj->next;
         temp->unlink();
         temp->destruct();
         found = true;
         if(temp->flags & ResourceObject::Added)
            freeResource(temp);
      }
   } while(found);
}

//------------------------------------------------------------------------------

void ResManager::purge( ResourceObject *obj )
{
   AssertFatal(obj->lockCount == 0, "ResourceManager::purge: handle lock count is not ZERO.")
   obj->unlink();
   obj->destruct();   
}

//------------------------------------------------------------------------------
// serialize sorts a list of files by .zip and position within the zip
// it allows an aggregate (material list, etc) to find the preffered
// loading order for a set of files.
//------------------------------------------------------------------------------

struct ResourceObjectIndex
{
   ResourceObject *ro;
   const char *fileName;
   
   static S32 QSORT_CALLBACK compare(const void *s1, const void *s2)
   {
      const ResourceObjectIndex *r1 = (ResourceObjectIndex *) s1;
      const ResourceObjectIndex *r2 = (ResourceObjectIndex *) s2;
   
      if(r1->ro->filePath != r2->ro->filePath)
         return r1->ro->filePath - r2->ro->filePath;
      if(r1->ro->fileName != r2->ro->fileName)
         return r1->ro->fileName - r2->ro->fileName;
      return r1->ro->fileOffset - r2->ro->fileOffset;
   }
};

//------------------------------------------------------------------------------

void ResManager::serialize(VectorPtr<const char *> &filenames)
{
   Vector<ResourceObjectIndex> sortVector;
   
   sortVector.reserve(filenames.size());

   U32 i;   
   for(i = 0; i < filenames.size(); i++)
   {
      ResourceObjectIndex roi;
      roi.ro = find(filenames[i]);
      roi.fileName = filenames[i];
      sortVector.push_back(roi);
   }

   dQsort((void *) &sortVector[0], sortVector.size(), sizeof(ResourceObjectIndex), ResourceObjectIndex::compare);
   for(i = 0; i < filenames.size(); i++)
      filenames[i] = sortVector[i].fileName;
}

//------------------------------------------------------------------------------
ResourceObject* ResManager::findMatch(const char *expression, const char **fn, ResourceObject *start)
{
   if(!start)
      start = resourceList.nextResource;
   else
      start = start->nextResource;
   while(start)
   {
      const char *fname = buildPath(start->path, start->name);
      if(FindMatch::isMatch(expression, fname, false))
      {
         *fn = fname;
         return start;
      }
      start = start->nextResource;
   }
   return NULL;
}

S32 ResManager::findMatches( FindMatch *pFM )
{
   static char buffer[16384];
   S32 bufl = 0;
   ResourceObject *walk;
   for(walk = resourceList.nextResource; walk && !pFM->isFull(); walk = walk->nextResource)
   {
      const char *fpath = buildPath(walk->path, walk->name);
      if(bufl + dStrlen(fpath) >= 16380)
         return pFM->numMatches();
      dStrcpy(buffer + bufl, fpath);
      if(pFM->findMatch(buffer + bufl))
         bufl += dStrlen(fpath) + 1;
   }
   return ( pFM->numMatches() );
}

//------------------------------------------------------------------------------
bool ResManager::findFile(const char *name)
{
   return (bool) find(name);
}

//------------------------------------------------------------------------------
ResourceObject *ResManager::createResource(StringTableEntry path, StringTableEntry file, StringTableEntry filePath, StringTableEntry fileName)
{
   ResourceObject *newRO = dictionary.find(path, file, filePath, fileName);
   if(newRO)
      return newRO;
      
   newRO = new ResourceObject;
   newRO->path = path;
   newRO->name = file;
   newRO->lockCount = 0;
   newRO->mInstance = NULL;
//   newRO->lockedData = NULL;
   newRO->flags = ResourceObject::Added;
   newRO->next = newRO->prev = NULL;
   newRO->nextResource = resourceList.nextResource;
   resourceList.nextResource = newRO;
   newRO->prevResource = &resourceList;
   if(newRO->nextResource)
      newRO->nextResource->prevResource = newRO;
   dictionary.insert(newRO, path, file);
   newRO->fileSize = newRO->fileOffset = newRO->compressedFileSize = 0;
   newRO->filePath = filePath;
   newRO->fileName = fileName;
   newRO->crc = InvalidCRC;

   return newRO;
}

//------------------------------------------------------------------------------
void ResManager::freeResource(ResourceObject *ro)
{
   ro->destruct();
   ro->unlink();

//   if((ro->flags & ResourceObject::File) && ro->lockedData)
//      delete[] ro->lockedData;

   if(ro->prevResource)
      ro->prevResource->nextResource = ro->nextResource;
   if(ro->nextResource)
      ro->nextResource->prevResource = ro->prevResource;
   dictionary.remove(ro);
   delete ro;
}


//------------------------------------------------------------------------------
// simple crc function - generates lookup table on first call

static U32 crcTable[256];
static bool crcTableValid;

static void calculateCRCTable()
{
   U32 val;

   for(S32 i = 0; i < 256; i++)
   {
      val = i;
      for(S32 j = 0; j < 8; j++)
      {
         if(val & 0x01)
            val = 0xedb88320 ^ (val >> 1);
         else
            val = val >> 1;
      }
      crcTable[i] = val;
   }
   
   crcTableValid = true;
}

U32 calculateCRC(void * buffer, S32 len, U32 crcVal )
{
   
   // check if need to generate the crc table
   if(!crcTableValid)
      calculateCRCTable();
   
   // now calculate the crc
   char * buf = (char*)buffer;
   for(S32 i = 0; i < len; i++)
      crcVal = crcTable[(crcVal ^ buf[i]) & 0xff] ^ (crcVal >> 8);
   return(crcVal);
}

U32 calculateCRCStream(Stream *stream, U32 crcVal )
{
   stream->setPosition(0);
   // check if need to generate the crc table
   if(!crcTableValid)
      calculateCRCTable();
   
   // now calculate the crc
   S32 len = stream->getStreamSize();
   U8 buf[4096];

   S32 segCount = (len + 4095) / 4096;

   for(S32 j = 0; j < segCount; j++)
   {
      S32 slen = getMin(4096, len - (j * 4096));
      stream->read(slen, buf);
      crcVal = calculateCRC(buf, slen, crcVal);
   }
   stream->setPosition(0);
   return(crcVal);
}

bool ResManager::openFileForWrite(FileStream &stream, const char *modPath, const char *fileName, U32 accessMode)
{
   if(!primaryPath[0])
      return false;
   if(!isValidWriteFileName(fileName))
      return false;
   // tag it on to the first directory
   char fnBuf[1024];
   dSprintf(fnBuf, sizeof(fnBuf), "%s/%s", modPath ? modPath : primaryPath, fileName);
   if(!Platform::createPath(fnBuf)) // create directory tree
      return false;
   if(!stream.open(fnBuf, (FileStream::AccessMode) accessMode))
      return false;

   // create a resource for the file.
   StringTableEntry rPath, rFileName, vPath, vFileName;
   getPaths(fnBuf, rPath, rFileName);
   getPaths(fileName, vPath, vFileName);
   
   ResourceObject *ro = createResource(vPath, vFileName, rPath, rFileName);
   ro->flags = ResourceObject::File;
   ro->fileOffset = 0;
   ro->fileSize = 0;
   ro->compressedFileSize = 0;
   return true;
}
