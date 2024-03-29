//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformMacCarb/platformMacCarb.h"
#include "core/fileio.h"
#include "core/tVector.h"
#include "core/stringTable.h"
#include "console/console.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <utime.h>

#pragma message("todo: file io needs some real work...")

// this is the main working dir path.
static StringTableEntry cwd = NULL;
static Boolean homeInPackage = FALSE;

//--------- Helper Functions ------------------------------
#define CAT_FOLDER_ATTRIB     0x10 // which bit stores file vs folder status.
#define CAT_IS_FOLDER(c)      (((c).dirInfo.ioFlAttrib & CAT_FOLDER_ATTRIB) != 0)
// look for PC/unix delimiters
#define ISA_SLASH(c)          ((c) == '\\' || (c) == '/')
// while mac paths are typically <255, I think HFS+ and UDF can have
// like 1024 or something.  I'll cap to something larger, so we don't
// go running off into memory...
#define MAX_MAC_PATH_LONG     1200
// the str255 maxsize, allowing null termination and length byte.
#define MAX_MAC_PATH          254

//-----------------------------------------------------------------------------
static U32 makeMacPath(const char *src, char *dst)
{
   U32 num = 0;
   // don't need these aliases, but aids in debugging.
   char *s = (char*)src, *d = dst;
   
   // clear dest.
   *d = '\0';
   
   // if the first char is a slash, then it's a top-level folder
   if (ISA_SLASH(*s))
   {
      // then no leading colon.
      s++; // skip the slash now.
   }
   else // curr-dir-relative
   {
      // start new string with a colon.
      *d++ = ':';
      num++;
   }
   
   while(*s && num<MAX_MAC_PATH-1)
   {
      if (ISA_SLASH(*s))
         *d++ = ':';
       else
          *d++ = *s;
      s++;
      num++;
   }
   
   // null terminate dest.
   *d = 0;
   
   return(num);
}


//-----------------------------------------------------------------------------
#ifdef __APPLE__
#include <CFBundle.h>
#define MACCARB_CONTENT_OUTSIDE_BUNDLE      1
#endif

void macGetHomeDirectory()
{
   HGetVol(NULL, &platState.volRefNum, &platState.dirID);

   // then, we need to check if we're in a package/bundle.  if so, need to find what dir
   // we are really in (potentially!!!!!tbd - we may WANT to do everything inside the
   // bundle dir anyway, so...
   
#ifdef __APPLE__ // for now, only do this for OSX Project Builder compiles...
#if MACCARB_CONTENT_OUTSIDE_BUNDLE
   CFBundleRef ref;
   ref = CFBundleGetMainBundle();
   if (ref)
   { // then we need to find the matching dirID.
      CFURLRef url;
      Boolean success = FALSE;
      Boolean wantAbsolute = TRUE; // !!!!TBD for now, get the full path.

      OSErr err;
      CInfoPBRec catinfo;
      Str255 dirname;
      char wdbuf[2048];
      U32 len = 0, totallen = 0;

      url = CFBundleCopyBundleURL(ref);
      success = CFURLGetFileSystemRepresentation(url, wantAbsolute, wdbuf, sizeof(wdbuf));
      if (!success) return; // done.

      // find the directory above the bundle's dir in the path.
      // walk backward through the string to the next / or \ or :
      len = dStrlen(wdbuf);
      while(len)
      {
         len--;
         if (wdbuf[len]=='/')
         { // stop here and null.
            wdbuf[len]=0;
            break;
         }
      }
      
      if (len==0) return; // done.

      // we now have the valid home path.  save the workingdir refstring and copy into platState.
      // offset the buffer by the length of the prefix "/Volumes" to strip it off...
      dMemmove(platState.absAppPath, wdbuf+8, len-8);

      // now, we need to decode what our parent dirID really is.
      // convert into dirname as pstr.
      totallen = makeMacPath(platState.absAppPath, &dirname[1]);
      dirname[0] = totallen;

      // clear the paramblock.
      dMemset((void*)&catinfo, 0L, sizeof(catinfo));
   
      catinfo.dirInfo.ioVRefNum = platState.volRefNum;
      catinfo.dirInfo.ioDrParID = 0; // hoping the dirname suffices...
      catinfo.dirInfo.ioDrDirID = 0; // hoping the dirname suffices...
      catinfo.dirInfo.ioNamePtr = (StringPtr)&dirname;
      // ask for info about the folder named in ioNamePtr by specifying zero as the dirindex.
      catinfo.dirInfo.ioFDirIndex = 0;

      err = PBGetCatInfoSync(&catinfo);
      if (err!=noErr) return;
   
      // now we have our REAL dirID.
      platState.dirID = catinfo.dirInfo.ioDrDirID;

      // can't do this here!
//      cwd = StringTable->insert(wdbuf);
//      Platform::getWorkingDirectory();

      homeInPackage = TRUE;
   }
   else // we're not in a bundle
#endif
#endif
   { // run the normal working-dir-retrieve code.
      // can't do this here!
//      Platform::getWorkingDirectory();
   }
}


//-----------------------------------------------------------------------------
bool dFileDelete(const char * name)
{
   if(!name || (dStrlen(name) >= MAX_MAC_PATH))
      return(false);
   return(remove(name));
}


//-----------------------------------------------------------------------------
bool dFileTouch(const char *path)
{
   if (!path || !*path) 
      return false;
   
   OSErr err = noErr;
   CInfoPBRec catinfo;
   U8 macpath[MAX_MAC_PATH_LONG];
   U32 size;
   
   size = makeMacPath(path, (char*)(macpath+1));
   if (size>MAX_MAC_PATH)
      return false;
   macpath[0] = size; // set PString length.

   // clear the paramblock.
   dMemset((void*)&catinfo, 0L, sizeof(catinfo));

   // fill in paramblock.
   catinfo.hFileInfo.ioVRefNum = macpath[1]==':'?platState.volRefNum:0;
   catinfo.hFileInfo.ioDirID = macpath[1]==':'?platState.dirID:0;
   catinfo.hFileInfo.ioNamePtr = macpath;
   // need to clear the dir index.
   catinfo.hFileInfo.ioFDirIndex = 0;

   // get the current info on this file.
   err = PBGetCatInfoSync(&catinfo);
   if (err!=noErr)
      return false; // !!!!TBD should log?

   // get curr time and update the modified time field
   unsigned long curtime;
   GetDateTime(&curtime);
   catinfo.hFileInfo.ioFlMdDat = curtime;
   
   // reset the dirID, as it came back with the fileID.
   catinfo.hFileInfo.ioDirID = macpath[1]==':'?platState.dirID:0;

   // save it all back.
   err = PBSetCatInfoSync(&catinfo);
   if (err!=noErr)
      return false; // !!!!TBD should log?

   return true;
}


//-----------------------------------------------------------------------------
// Constructors & Destructor
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// After construction, the currentStatus will be Closed and the capabilities
// will be 0.
//-----------------------------------------------------------------------------
File::File()
: currentStatus(Closed), capability(0)
{
   handle = NULL;
}

//-----------------------------------------------------------------------------
// insert a copy constructor here... (currently disabled)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
File::~File()
{
    close();
    handle = NULL;
}


//-----------------------------------------------------------------------------
// Open a file in the mode specified by openMode (Read, Write, or ReadWrite).
// Truncate the file if the mode is either Write or ReadWrite and truncate is
// true.
//
// Sets capability appropriate to the openMode.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::open(const char *filename, const AccessMode openMode)
{
   char hostFilename[256];
   char tmpFilename[256];
   
   AssertFatal(dStrlen(filename) <= 255, "File::open: Max Mac file length exceeded. MAX=255");
   AssertFatal(NULL != filename, "File::open: NULL filename");
   AssertWarn(NULL == handle, "File::open: handle already valid");

   // if the string BEGINS with a slash, assume it's a full path reference
   // otherwise, create a reference off the /Volumes dir + the working directory (since we stripped off /Volumes...)
#if __APPLE__
   if (homeInPackage && filename[0]!='/')
   {
      dStrcpy(tmpFilename, "/Volumes");
      dStrcat(tmpFilename, platState.absAppPath);
      dStrcat(tmpFilename, "/");
      dStrcat(tmpFilename, filename);
   }
   else
#endif
      dStrcpy(tmpFilename, filename);

#if __APPLE__
   dStrcpy(hostFilename, tmpFilename);
#else
   // change from posix to Mac
   makeMacPath(tmpFilename, hostFilename);
#endif
    
   // Close the file if it was already open...
   if (Closed != currentStatus)
      close();

   // create the appropriate type of file...
   switch (openMode)
   {
      case Read:
         handle = (void *)fopen(hostFilename, "rb");
         break;
      case Write:
         handle = (void *)fopen(hostFilename, "wb");
         break;
      case ReadWrite:
         handle = (void *)fopen(hostFilename, "ab+");
         break;
      default:
         AssertFatal(false, "File::open: bad access mode");    // impossible
   }
    
   if (handle == NULL)                // handle not created successfully
      return setStatus();
   else
   {
      // successfully created file, so set the file capabilities...
      switch (openMode)
      {
         case Read:
            capability = U32(FileRead);
            break;
           case Write:
            capability = U32(FileWrite);
            break;
           case ReadWrite:
            capability = U32(FileRead) | U32(FileWrite);
            break;
           default:
            AssertFatal(false, "File::open: bad access mode");
      }
   
      if (openMode == ReadWrite)
         setPosition(0);

      return currentStatus = Ok;                                // success!
   }
}


#define HANDLE_CAST            void *
#define INVALID_HANDLE_VALUE   ((HANDLE_CAST)0L)

//-----------------------------------------------------------------------------
// Get the current position of the file pointer.
//-----------------------------------------------------------------------------
U32 File::getPosition() const
{
   AssertFatal(Closed != currentStatus, "File::getPosition: file closed");
   AssertFatal(INVALID_HANDLE_VALUE != (HANDLE_CAST)handle, "File::getPosition: invalid file handle");
   
   return ftell((FILE*)handle);
}

//-----------------------------------------------------------------------------
// Set the position of the file pointer.
// Absolute and relative positioning is supported via the absolutePos
// parameter.
//
// If positioning absolutely, position MUST be positive - an IOError results if
// position is negative.
// Position can be negative if positioning relatively, however positioning
// before the start of the file is an IOError.
//
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::setPosition(S32 position, bool absolutePos)
{
    AssertFatal(Closed != currentStatus, "File::setPosition: file closed");
    AssertFatal(handle != NULL, "File::setPosition: invalid file handle");
    
    if (Ok != currentStatus && EOS != currentStatus)
        return currentStatus;
    
    U32 finalPos;
    switch (absolutePos)
    {
    case true:                                                    // absolute position
        AssertFatal(0 <= position, "File::setPosition: negative absolute position");
        // position beyond EOS is OK
        fseek((FILE*)handle, position, SEEK_SET);
        finalPos = ftell((FILE*)handle);
        break;
    case false:                                                    // relative position
        AssertFatal((getPosition() >= (U32)abs(position) && 0 > position) || 0 <= position, "File::setPosition: negative relative position");
        // position beyond EOS is OK
        fseek((FILE*)handle, position, SEEK_CUR);
        finalPos = ftell((FILE*)handle);
    }

    if (0xffffffff == finalPos)
        return setStatus();                                        // unsuccessful
    else if (finalPos >= getSize())
        return currentStatus = EOS;                                // success, at end of file
    else
        return currentStatus = Ok;                                // success!
}

//-----------------------------------------------------------------------------
// Get the size of the file in bytes.
// It is an error to query the file size for a Closed file, or for one with an
// error status.
//-----------------------------------------------------------------------------
U32 File::getSize() const
{
   AssertWarn(Closed != currentStatus, "File::getSize: file closed");
   AssertFatal(handle != NULL, "File::getSize: invalid file handle");
    
   if (Ok == currentStatus || EOS == currentStatus)
   {
      // this is not a very good way to do this
      U32 pos  = ftell((FILE*)handle);
      fseek((FILE*)handle, 0, SEEK_END);
      U32 size = ftell((FILE*)handle);
      fseek((FILE*)handle, pos, SEEK_SET);
       return size;   
   }
   else
      return 0;                                                // unsuccessful
}

//-----------------------------------------------------------------------------
// Flush the file.
// It is an error to flush a read-only file.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::flush()
{
    AssertFatal(Closed != currentStatus, "File::flush: file closed");
    AssertFatal(handle != NULL, "File::flush: invalid file handle");
    AssertFatal(true == hasCapability(FileWrite), "File::flush: cannot flush a read-only file");

    if (fflush((FILE*)handle) == EOF)
        return setStatus();                                        // unsuccessful
    else
        return currentStatus = Ok;                                // success!
}

//-----------------------------------------------------------------------------
// Close the File.
//
// Returns the currentStatus
//-----------------------------------------------------------------------------
File::Status File::close()
{
    // check if it's already closed...
    if (Closed == currentStatus)
        return currentStatus;

    // it's not, so close it...
    if (handle != NULL)
    {
        if (fclose((FILE*)handle) == EOF)
            return setStatus();                                    // unsuccessful
    }
    handle = NULL;
    return currentStatus = Closed;
}

//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
File::Status File::getStatus() const
{
    return currentStatus;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus when an error has been encountered.
//-----------------------------------------------------------------------------
File::Status File::setStatus()
{
#pragma message("todo: File::setStatus")

   switch (errno)
   {
/*
   case EACCESS:   // permission denied
      return currentStatus = IOError;
   case EBADF:   // Bad File Pointer
      errno++;
   case EINVAL:   // Invalid argument
      errno++;
   case ENOENT:   // file not found
      errno++;
*/
   default:
      return currentStatus = UnknownError;
   }

   return currentStatus = UnknownError;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus to status.
//-----------------------------------------------------------------------------
File::Status File::setStatus(File::Status status)
{
    return currentStatus = status;
}

//-----------------------------------------------------------------------------
// Read from a file.
// The number of bytes to read is passed in size, the data is returned in src.
// The number of bytes read is available in bytesRead if a non-Null pointer is
// provided.
//-----------------------------------------------------------------------------
File::Status File::read(U32 size, char *dst, U32 *bytesRead)
{
    AssertFatal(Closed != currentStatus, "File::read: file closed");
    AssertFatal(handle != NULL, "File::read: invalid file handle");
    AssertFatal(NULL != dst, "File::read: NULL destination pointer");
    AssertFatal(true == hasCapability(FileRead), "File::read: file lacks capability");
    AssertWarn(0 != size, "File::read: size of zero");
    
    if (Ok != currentStatus || 0 == size)
        return currentStatus;
    else
    {
        U32 lastBytes;
        U32 *bytes = (NULL == bytesRead) ? &lastBytes : bytesRead;
        if (fread(dst, size, 1, (FILE*)handle) != 1)
        {   // fread onlu reports the number of chunks read not bytes
              // so we don't know exactly how much was read
            *bytes = getPosition();
            return currentStatus = EOS;                        // end of stream
        }
        else
        {
              *bytes = size;
            return currentStatus = Ok;                            // unsuccessful
        }
    }
    return currentStatus = Ok;                                    // successfully read size bytes
}

//-----------------------------------------------------------------------------
// Write to a file.
// The number of bytes to write is passed in size, the data is passed in src.
// The number of bytes written is available in bytesWritten if a non-Null
// pointer is provided.
//-----------------------------------------------------------------------------
File::Status File::write(U32 size, const char *src, U32 *bytesWritten)
{
    AssertFatal(Closed != currentStatus, "File::write: file closed");
    AssertFatal(handle != NULL, "File::write: invalid file handle");
    AssertFatal(NULL != src, "File::write: NULL source pointer");
    AssertFatal(true == hasCapability(FileWrite), "File::write: file lacks capability");
    AssertWarn(0 != size, "File::write: size of zero");
    
    if ((Ok != currentStatus && EOS != currentStatus) || 0 == size)
        return currentStatus;
    else
    {
        U32 lastBytes;
        U32 *bytes = (NULL == bytesWritten) ? &lastBytes : bytesWritten;
        if (fwrite(src, size, 1, (FILE*)handle) != 1)
        {   // fwrite onlu reports the number of chunks written not bytes
              // so we don't know exactly how much was written
              *bytes = getPosition();
              return setStatus();
        }
        else
        {
           *bytes = size;
            return currentStatus = Ok;                            // success!
        }
    }
}


//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
bool File::hasCapability(Capability cap) const
{
    return (0 != (U32(cap) & capability));
}


//-----------------------------------------------------------------------------
static bool _recurseDumpPath(const char* in_pBasePath, const char* in_pCurPath, S32 dirID, Vector<Platform::FileInfo>& out_rFileVector)
{
   char curPath[1024];
   char basePath[1024];
   char scratchBuf[1024];
   
   if(in_pCurPath)
      dStrcpy(curPath, in_pCurPath);
   else
      curPath[0] = 0;
      
   dStrcpy(basePath, in_pBasePath);
   in_pBasePath = basePath;
   
   CInfoPBRec cinfo;
   Str63 nameField;
   OSErr result;
   S32 index = 1;
   
   do
   {  // setup a catalog information request structure
      cinfo.hFileInfo.ioVRefNum   = platState.volRefNum;   // volume ID to search in
      cinfo.hFileInfo.ioDirID     = dirID;               // directory ID to search in
      cinfo.hFileInfo.ioFDirIndex = index++;               // specify which entry you are interested in
      cinfo.hFileInfo.ioNamePtr   = nameField;            // supply a buffer to store the name
            
      result = PBGetCatInfoSync(&cinfo);
      if (result == noErr)
      {
         if (cinfo.dirInfo.ioFlAttrib & ioDirMask) 
         {   // it's a directory
            char *dirname = p2str(cinfo.hFileInfo.ioNamePtr);
            scratchBuf[0] = '\0';
            if (curPath[0] != '\0')
            {
               dStrcpy(scratchBuf, curPath);
               dStrcat(scratchBuf, "/");
            }
            dStrcat(scratchBuf, dirname);

            _recurseDumpPath(basePath, scratchBuf, cinfo.hFileInfo.ioDirID, out_rFileVector);
         }
         else
         {   // it's a file
            char *filename = p2str(cinfo.hFileInfo.ioNamePtr);
                        
             out_rFileVector.increment();
             Platform::FileInfo& rInfo = out_rFileVector.last();

             if (curPath[0] != '\0')
             {
                dSprintf(scratchBuf, sizeof(scratchBuf), "%s/%s", basePath, curPath);
                rInfo.pFullPath = StringTable->insert(scratchBuf);
            }
            else
            {
                rInfo.pFullPath = StringTable->insert(basePath);
             }

             rInfo.pFileName = StringTable->insert(filename);
             rInfo.fileSize  = cinfo.hFileInfo.ioFlLgLen;
             //rInfo.createTime.time = cinfo.hFileInfo.ioFlCrDat;
             //rInfo.modifyTime.time = cinfo.hFileInfo.ioFlMdDat;
         }
      } 
   }
   while (result == noErr);

   return true;
}


//-----------------------------------------------------------------------------
S32 Platform::compareFileTimes(const FileTime &a, const FileTime &b)
{
   if(a.time > b.time)
      return 1;
   if(a.time < b.time)
      return -1;
   return 0;
}


//-----------------------------------------------------------------------------
// either time param COULD be null.
//-----------------------------------------------------------------------------
bool Platform::getFileTimes(const char *path, FileTime *createTime, FileTime *modifyTime)
{
   if (!path || !*path) 
      return false;
   
   OSErr err = noErr;
   CInfoPBRec catinfo;
   U8 macpath[MAX_MAC_PATH_LONG];
   U32 size;
   
   size = makeMacPath(path, (char*)(macpath+1));
   if (size>MAX_MAC_PATH)
      return false;
   macpath[0] = size; // set PString length.

   // clear the paramblock.
   dMemset((void*)&catinfo, 0L, sizeof(catinfo));

   // fill in paramblock.
   catinfo.hFileInfo.ioVRefNum = macpath[1]==':'?platState.volRefNum:0;
   catinfo.hFileInfo.ioDirID = macpath[1]==':'?platState.dirID:0;
   catinfo.hFileInfo.ioNamePtr = macpath;
   err = PBGetCatInfoSync(&catinfo);
   if (err!=noErr)
      return false; // !!!!TBD should log?

   if (createTime)
      createTime->time = catinfo.hFileInfo.ioFlCrDat;
   if (modifyTime)
      modifyTime->time = catinfo.hFileInfo.ioFlMdDat;

   return true;
}


//-----------------------------------------------------------------------------
bool Platform::createPath(const char *file)
{
   char pathBuf[1024];
   char dirBuf[256];
   const char *dir;
   U32 pathLen = 0;
   S32 parentDirID = platState.dirID;

   pathBuf[0] = 0;
   while((dir = dStrchr(file, '/')) != NULL)
   {
      U32 len = dir-file;
      dStrncpy(dirBuf, file, len);
      dirBuf[len] = 0;

      dStrncpy(pathBuf + pathLen, file, dir - file);
      pathBuf[pathLen + dir-file] = 0;
      
      // does directory/name already exist?
      CInfoPBRec cinfo;
      Str63 nameField;
         
      cinfo.hFileInfo.ioVRefNum   = platState.volRefNum;   // volume ID to search in
      cinfo.hFileInfo.ioDirID     = parentDirID;               // directory ID to search in
      cinfo.hFileInfo.ioFDirIndex = 0;                     // get info on ioNamePtr
      cinfo.hFileInfo.ioNamePtr   = nameField;            // supply a buffer with name
      str2p(dirBuf, nameField);
      OSErr err = PBGetCatInfoSync(&cinfo);
      switch(err)
      {
         case noErr:
            if (cinfo.dirInfo.ioFlAttrib & ioDirMask) 
            {   // it's a directory
               parentDirID = cinfo.hFileInfo.ioDirID;
            }
            else
            {  // the name existed and it was NOT a directory
               Con::printf("CreateDirectory(%s) - failed", pathBuf);
               return false;
            }
            break;

         case fnfErr:
            {  // the name did not exist so create the directory
               long newId;
               OSErr err = DirCreate(platState.volRefNum, parentDirID, str2p(dirBuf), &newId);
               if (err != noErr)
               {
                  Con::printf("CreateDirectory(%s) - failed", pathBuf);
                  return false;
               }
               parentDirID = newId;
            }
            break;
      }

      file = dir + 1;
      pathLen += len;
      pathBuf[pathLen++] = '/';
      pathBuf[pathLen] = 0;
      Con::printf("CreateDirectory(%s) - Succeeded", pathBuf);
     
   }
   return true;
}


//-----------------------------------------------------------------------------
bool Platform::cdFileExists(const char *filePath, const char *volumeName, S32 serialNum)
{
   return true; // !!!!!TBD
}


//-----------------------------------------------------------------------------
bool Platform::dumpPath(const char *in_pBasePath, Vector<Platform::FileInfo>& out_rFileVector)
{
   // for now we can only search directories in the apps current workinng directory
   // specifying another drive, path or sub path (base/art) will not work.
   S32 dirID = platState.dirID;
   
   if (in_pBasePath)
   {
      CInfoPBRec cinfo;
      Str63 nameField;
      OSErr result;
   
      cinfo.hFileInfo.ioVRefNum   = platState.volRefNum;   // volume ID to search in
      cinfo.hFileInfo.ioDirID     = dirID;               // directory ID to search in
      cinfo.hFileInfo.ioFDirIndex = 0;                     // get info on ioNamePtr
      cinfo.hFileInfo.ioNamePtr   = nameField;            // supply a buffer with name
      str2p(in_pBasePath, nameField);
      result = PBGetCatInfoSync(&cinfo);
      if (result == noErr)
      {
         if (cinfo.dirInfo.ioFlAttrib & ioDirMask) 
         {   // it's a directory
            char *dirname = p2str(cinfo.hFileInfo.ioNamePtr);
            return _recurseDumpPath(in_pBasePath, NULL, cinfo.hFileInfo.ioDirID, out_rFileVector);
         }
         return false;    // not a directory
      }
   }
   return _recurseDumpPath(in_pBasePath, NULL, dirID, out_rFileVector);
}


//-----------------------------------------------------------------------------
StringTableEntry Platform::getWorkingDirectory()
{
   if (!cwd)
   {
      OSErr err;
      CInfoPBRec catinfo;
      Str255 dirname;
      char cwd_buf[2048];
      U32 len = 0, totallen = 0;

      *cwd_buf = 0;

      // clear the paramblock.
      dMemset((void*)&catinfo, 0L, sizeof(catinfo));
   
      catinfo.dirInfo.ioVRefNum = platState.volRefNum;
      catinfo.dirInfo.ioDrParID = platState.dirID;
      catinfo.dirInfo.ioNamePtr = (StringPtr)&dirname;
      // ask for info about this directory.
      catinfo.dirInfo.ioFDirIndex = -1;

      do
      {
         dirname[0] = 0; // make sure to null the dirname!
         // reset dir id each time through to the parent's id.
         catinfo.dirInfo.ioDrDirID = catinfo.dirInfo.ioDrParID;
         err = PBGetCatInfoSync(&catinfo);
         if (err!=noErr) break;
         
         // capture st255 len and null terminate it.
         len = dirname[0]+1;
         dirname[len] = 0;
         
         // put a slash as the first char (eff converting it into a cstr...).
         dirname[0] = '/';
         
         // copy the current path forward in the buffer to allow us to prepend the new dir.
         // MUST USE MEMMOVE, as we are copying in-place, and memmove allows moving within
         // a given buffer.  memcpy doesn't.
         // and remember to allow for the null on the end...
         if (totallen) dMemmove(cwd_buf + len, cwd_buf, totallen+1);
         
         // copy the new path in, prepending the string.  don't copy the null here...
         dMemmove(cwd_buf, dirname, len);
         
         // inc the total size.
         totallen += len;
         
         // make damn sure we're null terminated.
         cwd_buf[totallen] = 0;
      }
      while (catinfo.dirInfo.ioDrDirID != fsRtDirID);
   
      cwd = StringTable->insert(cwd_buf);
   }
   return cwd;
}


//-----------------------------------------------------------------------------
bool Platform::isFile(const char *path)
{
   if (!path || !*path) 
      return false;
   
   OSErr err = noErr;
   CInfoPBRec catinfo;
   U8 macpath[MAX_MAC_PATH_LONG];
   U32 size;
   
   size = makeMacPath(path, (char*)(macpath+1));
   if (size>MAX_MAC_PATH)
      return false;
   macpath[0] = size; // set PString length.

   // clear the paramblock.
   dMemset((void*)&catinfo, 0L, sizeof(catinfo));

   // fill in paramblock.
   catinfo.dirInfo.ioVRefNum = macpath[1]==':'?platState.volRefNum:0;
   catinfo.dirInfo.ioDrDirID = macpath[1]==':'?platState.dirID:0;
   catinfo.dirInfo.ioNamePtr = macpath;
   err = PBGetCatInfoSync(&catinfo);
   if (err!=noErr)
      return false; // !!!!TBD should log?

   if (CAT_IS_FOLDER(catinfo))
      return false;

   return true;
}


//-----------------------------------------------------------------------------
bool Platform::isDirectory(const char *path)
{
   if (!path || !*path) 
      return false;
   
   OSErr err = noErr;
   CInfoPBRec catinfo;
   U8 macpath[MAX_MAC_PATH_LONG];
   U32 size;
   
   size = makeMacPath(path, (char*)(macpath+1));
   if (size>MAX_MAC_PATH)
      return false;
   macpath[0] = size; // set PString length.

   // clear the paramblock.
   dMemset((void*)&catinfo, 0L, sizeof(catinfo));

   // fill in paramblock.
   catinfo.dirInfo.ioVRefNum = macpath[1]==':'?platState.volRefNum:0;
   catinfo.dirInfo.ioDrDirID = macpath[1]==':'?platState.dirID:0;
   catinfo.dirInfo.ioNamePtr = macpath;
   err = PBGetCatInfoSync(&catinfo);
   if (err!=noErr)
      return false; // !!!!TBD should log?

   if (CAT_IS_FOLDER(catinfo))
      return true;

   return false;
}



//-----------------------------------------------------------------------------
bool Platform::isSubDirectory(const char *pathParent, const char *pathSub)
{
   char fullpath[MAX_MAC_PATH_LONG];
   dStrcpyl(fullpath, sizeof(fullpath), pathParent, "/", pathSub, NULL);
   return isDirectory((const char *)fullpath);
}

