//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/platformX86UNIX.h"
#include "core/fileio.h"
#include "core/tVector.h"
#include "core/stringTable.h"
#include "console/console.h"

#include <utime.h>

/* include sys/param.h for MAXPATHLEN */
#include <sys/param.h>
#ifndef MAX_PATH
#define MAX_PATH MAXPATHLEN
#endif

/* these are for reading directors, getting stats, etc. */
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

extern int x86UNIXOpen(const char *path, int oflag);
extern int x86UNIXClose(int fd);
extern ssize_t x86UNIXRead(int fd, void *buf, size_t nbytes);
extern ssize_t x86UNIXWrite(int fd, const void *buf, size_t nbytes);

//-------------------------------------- Helper Functions
static void forwardslash(char *str)
{
   while(*str)
   {
      if(*str == '\\')
         *str = '/';
      str++;
   }
}

static void backslash(char *str)
{
   while(*str)
   {
      if(*str == '/')
         *str = '\\';
      str++;
   }
}

//-----------------------------------------------------------------------------
bool dFileDelete(const char * name)
{
   if(!name || (dStrlen(name) >= MAX_PATH))
      return(false);
   if (remove(name) == -1)
      return(false);

   return(true);
}

bool dFileTouch(const char * name)
{
   // change the modified time to the current time (0byte WriteFile fails!)
   return(utime(name, 0) != -1);
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
//    AssertFatal(sizeof(int) == sizeof(void *), "File::File: cannot cast void* to int");

    handle = (void *)NULL;
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
    handle = (void *)NULL;
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
   static char filebuf[2048];
   dStrcpy(filebuf, filename);
   filename = filebuf;
   
    AssertFatal(NULL != filename, "File::open: NULL filename");
    AssertWarn(NULL == handle, "File::open: handle already valid");

    // Close the file if it was already open...
    if (Closed != currentStatus)
        close();

    int fd;
    handle = (void *)dRealMalloc(sizeof(int));
    // create the appropriate type of file...
    switch (openMode)
    {
    case Read:
        fd = x86UNIXOpen(filename, O_RDONLY);
        dMemcpy(handle, &fd, sizeof(int));
//        handle = (void *)&x86UNIXOpen(filename, O_RDONLY);
        break;
    case Write:
        fd = x86UNIXOpen(filename, O_WRONLY | O_CREAT);
        dMemcpy(handle, &fd, sizeof(int));
//        handle = (void *)x86UNIXOpen(filename, O_WRONLY | O_CREAT);
        break;
    case ReadWrite:
        fd = x86UNIXOpen(filename, O_RDWR);
        dMemcpy(handle, &fd, sizeof(int));
//        handle = (void *)x86UNIXOpen(filename, O_RDWR);
        break;
    case WriteAppend:
        fd = x86UNIXOpen(filename, O_APPEND);
        dMemcpy(handle, &fd, sizeof(int));
//        handle = (void *)x86UNIXOpen(filename, O_APPEND);
        break;
    default:
        AssertFatal(false, "File::open: bad access mode");    // impossible
      }
    
#ifdef DEBUG
//   fprintf(stdout,"fd = %d, handle = %d\n", fd, *((int *)handle));
#endif

    if (*((int *)handle) == -1)                // handle not created successfully
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
        case WriteAppend:
            capability = U32(FileWrite);
            break;
        case ReadWrite:
            capability = U32(FileRead)  |
                         U32(FileWrite);
            break;
        default:
            AssertFatal(false, "File::open: bad access mode");
        }
        return currentStatus = Ok;                                // success!
    }
}

//-----------------------------------------------------------------------------
// Get the current position of the file pointer.
//-----------------------------------------------------------------------------
U32 File::getPosition() const
{
    AssertFatal(Closed != currentStatus, "File::getPosition: file closed");
    AssertFatal(NULL != handle, "File::getPosition: invalid file handle");

#ifdef DEBUG
//   fprintf(stdout, "handle = %d\n",*((int *)handle));fflush(stdout);
#endif
    return (U32) lseek(*((int *)handle), 0, SEEK_CUR);
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
    AssertFatal(NULL != handle, "File::setPosition: invalid file handle");
    
    if (Ok != currentStatus && EOS != currentStatus)
        return currentStatus;
    
    U32 finalPos;
    switch (absolutePos)
    {
    case true:                                                    // absolute position
        AssertFatal(0 <= position, "File::setPosition: negative absolute position");
        
        // position beyond EOS is OK
        finalPos = lseek(*((int *)handle), position, SEEK_SET);
        break;
    case false:                                                    // relative position
        AssertFatal((getPosition() >= (U32)abs(position) && 0 > position) || 0 <= position, "File::setPosition: negative relative position");
        
        // position beyond EOS is OK
        finalPos = lseek(*((int *)handle), position, SEEK_CUR);
	break;
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
    AssertFatal(NULL != handle, "File::getSize: invalid file handle");
    
    if (Ok == currentStatus || EOS == currentStatus)
    {
	long	currentOffset = getPosition();                  // keep track of our current position
	long	fileSize;
	lseek(*((int *)handle), 0, SEEK_END);                     // seek to the end of the file
	fileSize = getPosition();                               // get the file size
	lseek(*((int *)handle), currentOffset, SEEK_SET);         // seek back to our old offset
        return fileSize;                                        // success!
    }
    else
        return 0;                                               // unsuccessful
}

//-----------------------------------------------------------------------------
// Flush the file.
// It is an error to flush a read-only file.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::flush()
{
    AssertFatal(Closed != currentStatus, "File::flush: file closed");
    AssertFatal(NULL != handle, "File::flush: invalid file handle");
    AssertFatal(true == hasCapability(FileWrite), "File::flush: cannot flush a read-only file");

    if (fdatasync(*((int *)handle)) == 0)
        return currentStatus = Ok;                                // success!
    else
        return setStatus();                                       // unsuccessful
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
    if (NULL != handle)
    {
        if (x86UNIXClose(*((int *)handle)) != 0)
            return setStatus();                                    // unsuccessful
    }
    handle = (void *)NULL;
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
    switch (errno)
    {
    case EINVAL:
    case EBADFD:
    case EIO:
	return currentStatus = IOError;
    default:
	return currentStatus = Ok;
    }
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
#ifdef DEBUG
//   fprintf(stdout,"reading %d bytes\n",size);fflush(stdout);
#endif
    AssertFatal(Closed != currentStatus, "File::read: file closed");
    AssertFatal(NULL != handle, "File::read: invalid file handle");
    AssertFatal(NULL != dst, "File::read: NULL destination pointer");
    AssertFatal(true == hasCapability(FileRead), "File::read: file lacks capability");
    AssertWarn(0 != size, "File::read: size of zero");

/* show stats for this file */
#ifdef DEBUG
//struct stat st;
//fstat(*((int *)handle), &st);
//fprintf(stdout,"file size = %d\n", st.st_size);
#endif
/****************************/
    
    if (Ok != currentStatus || 0 == size)
        return currentStatus;
    else
    {
        long lastBytes;
        long *bytes = (NULL == bytesRead) ? &lastBytes : (long *)bytesRead;
//if ((FILE *)handle == NULL || dst == NULL) {
//fprintf(stdout, "ack!\n");
//}
//fprintf(stdout, "%d-%p\n",strlen(dst),dst);
//fprintf(stdout, "feof() = %d\n",feof((FILE *)handle));
//fprintf(stdout, "sizeof(U32) = %d\nsize = %d\n",sizeof(U32), size);
//fprintf(stdout, "bytesRead = %d\n", *((U32 *)bytes) = x86UNIXRead(fileno((FILE *)handle), dst, size) );

//        (*((U32 *)bytes)) = fread(dst, sizeof(U32), size, (FILE *)handle);
//fprintf(stdout, "bytes = %d\n", (*(int *)bytes));fflush(NULL);
//exit(*((int *)bytes));
//        if (0 != (*((size_t *)bytes) = fread(dst, sizeof(U32), size, (FILE *)handle)) )
        if ( (*((U32 *)bytes) = x86UNIXRead(*((int *)handle), dst, size)) == -1)
        {
#ifdef DEBUG
//   fprintf(stdout,"unsuccessful: %d\n", *((U32 *)bytes));fflush(stdout);
#endif
           return setStatus();                                    // unsuccessful
        } else {
//            dst[*((U32 *)bytes)] = '\0';
            if (*((U32 *)bytes) != size || *((U32 *)bytes) == 0) {
#ifdef DEBUG
//  fprintf(stdout,"end of stream: %d\n", *((U32 *)bytes));fflush(stdout);
#endif
                return currentStatus = EOS;                        // end of stream
            }
        }
    }
//    dst[*bytesRead] = '\0';
#ifdef DEBUG
//fprintf(stdout, "We read:\n");
//fprintf(stdout, "====================================================\n");
//fprintf(stdout, "%s\n",dst);
//fprintf(stdout, "====================================================\n");
//fprintf(stdout,"read ok: %d\n", *bytesRead);fflush(stdout);
#endif
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
    AssertFatal(NULL != handle, "File::write: invalid file handle");
    AssertFatal(NULL != src, "File::write: NULL source pointer");
    AssertFatal(true == hasCapability(FileWrite), "File::write: file lacks capability");
    AssertWarn(0 != size, "File::write: size of zero");
    
    if ((Ok != currentStatus && EOS != currentStatus) || 0 == size)
        return currentStatus;
    else
    {
        long lastBytes;
        long *bytes = (NULL == bytesWritten) ? &lastBytes : (long *)bytesWritten;
        if (0 != (*((U32 *)bytes) = x86UNIXWrite(*((int *)handle), src, size)))
            return currentStatus = Ok;                            // success!
        else
            return setStatus();                                    // unsuccessful
    }
}

//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
bool File::hasCapability(Capability cap) const
{
    return (0 != (U32(cap) & capability));
}

S32 Platform::compareFileTimes(const FileTime &a, const FileTime &b)
{
   if(a > b)
      return 1;
   if(a < b)
      return -1;
   if(a > b)
      return 1;
   if(a < b)
      return -1;
   return 0;
}


static bool recurseDumpPath(const char *path, const char *pattern, Vector<Platform::FileInfo> &fileVector)
{
   char search[1024];

   dSprintf(search, sizeof(search), "%s", path, pattern);
   
   DIR *directory = opendir(search);

   if (directory == NULL)
      return false;

   struct dirent *fEntry;
   fEntry = readdir(directory);		// read the first "file" in the directory

   if (fEntry == NULL)
      return false;

   do
   {
      char filename[BUFSIZ+1];
      struct stat fStat;

      dSprintf(filename, sizeof(filename), "%s/%s", search, fEntry->d_name); // "construct" the file name
      stat(filename, &fStat); // get the file stats

      if ( (fStat.st_mode & S_IFMT) == S_IFDIR )
      {
         // Directory
         // skip . and .. directories
         if (dStrcmp(fEntry->d_name, ".") == 0 || dStrcmp(fEntry->d_name, "..") == 0)
            continue;

         char child[1024];
         dSprintf(child, sizeof(child), "%s/%s", path, fEntry->d_name);
         recurseDumpPath(child, pattern, fileVector);
      }      
      else
      {
         // File
         
         // add it to the list
         fileVector.increment();
         Platform::FileInfo& rInfo = fileVector.last();

         rInfo.pFullPath = StringTable->insert(path);
         rInfo.pFileName = StringTable->insert(fEntry->d_name);
         rInfo.fileSize  = fStat.st_size;
      }

   } while( (fEntry = readdir(directory)) != NULL );

   closedir(directory);
   return true;
}   

//static bool _recurseDumpPath(const char* in_pBasePath,
//                      const char* in_pCurPath,
//                      Vector<Platform::FileInfo>& out_rFileVector)
//{
//   char buf[1024];
//   char curPath[1024];
//   char basePath[1024];
//   char scratchBuf[1024];
//   
//   if(in_pCurPath)
//      dStrcpy(curPath, in_pCurPath);
//   else
//      curPath[0] = 0;
//      
//   dStrcpy(basePath, in_pBasePath);
//   in_pBasePath = basePath;
//
//
//   if (curPath[0] != '\0')
//      dSprintf(buf, sizeof(buf), "%s/%s", basePath, curPath);
//   else
//      dSprintf(buf, sizeof(buf), "%s", basePath);
//
//
//   backslash(buf);
//   DIR *directory = opendir(buf);	// open the directory
//
//   if (directory == (DIR *)NULL)
//      return false;
//
//   struct dirent *fEntry;
//   fEntry = readdir(directory);		// read the first "file" in the directory
//
//   while (fEntry != (struct dirent *)NULL) {
//      char filename[BUFSIZ+1];
//      struct stat fStat;
//
//      dSprintf(filename, sizeof(filename), "%s/%s", buf, fEntry->d_name);	// "construct" the file name
//      stat(filename, &fStat);				// get the file stats
//
//      /* we AND the st_mode and S_IFMT to get the type of file */
//      if ( (fStat.st_mode & S_IFMT) == S_IFDIR ) {
//         // Directory
//         if (filename[0] != '.') {
//            scratchBuf[0] = '\0';
//            if (curPath[0] != '\0') {
//               dStrcpy(scratchBuf, curPath);
//               dStrcat(scratchBuf, "/");
//            }
//            dStrcat(scratchBuf, filename);
//
//            _recurseDumpPath(basePath, scratchBuf, out_rFileVector);
//         }
//      } else {
//         // File
//         out_rFileVector.increment();
//         Platform::FileInfo& rInfo = out_rFileVector.last();
//
//         if (curPath[0] != '\0') {
//            dSprintf(scratchBuf, sizeof(scratchBuf), "%s/%s", basePath, curPath);
//            rInfo.pFullPath = StringTable->insert(scratchBuf);
//            rInfo.pVirtPath = StringTable->insert(curPath);
//         } else {
//            rInfo.pFullPath = StringTable->insert(basePath);
//            rInfo.pVirtPath = NULL;
//         }
//         rInfo.pFileName = StringTable->insert(filename);
//         rInfo.fileSize  = fStat.st_size;
//      }
//
//      if ( (fEntry = readdir(directory)) == (struct dirent *)NULL) {
//         closedir(directory);
//         fEntry = (struct dirent *)NULL;
//      }
//   }
//
//   return true;
//}

//--------------------------------------

bool Platform::getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime)
{
   struct stat fStat;

   if (stat(filePath, &fStat) == -1)
      return false;

   if(createTime)
   {
      // no where does SysV/BSD UNIX keep a record of a file's
      // creation time.  instead of creation time I'll just use
      // changed time for now.
      createTime = (FileTime *)&fStat.st_ctime;
   }
   if(modifyTime)
   {
      modifyTime = (FileTime *)&fStat.st_mtime;
   }

   return true;
}

//--------------------------------------
bool Platform::createPath(const char *file)
{
   char pathbuf[1024];
   const char *dir;
   pathbuf[0] = 0;
   U32 pathLen = 0;
   
   while((dir = dStrchr(file, '/')) != NULL)
   {
      dStrncpy(pathbuf + pathLen, file, dir - file);
      pathbuf[pathLen + dir-file] = 0;
      bool ret = mkdir(pathbuf, 0666);
      pathLen += dir - file;
      pathbuf[pathLen++] = '/';
      file = dir + 1;
   }
   return true;
}

/* this will be a pain and it's not being used so we are leaving it out, for now :-D
	- rjp
*/
/***************************************************************************************
bool Platform::cdFileExists(const char *filePath, const char *volumeName, S32 serialNum)
{
   if (!filePath || !filePath[0])
      return true;

   //first find the CD device...
   char fileBuf[256];
   char drivesBuf[256];
   S32 length = GetLogicalDriveStrings(256, drivesBuf);
   char *drivePtr = drivesBuf;
   while (S32(drivePtr - drivesBuf) < length)
   {
      char driveVolume[256], driveFileSystem[256];
      U32 driveSerial, driveFNLength, driveFlags;
      if ((dStricmp(drivePtr, "A:\\") != 0 && dStricmp(drivePtr, "B:\\") != 0) &&
          GetVolumeInformation((const char*)drivePtr, &driveVolume[0], (unsigned long)255,
                               (unsigned long*)&driveSerial, (unsigned long*)&driveFNLength,
                               (unsigned long*)&driveFlags, &driveFileSystem[0], (unsigned long)255))
      {
#if defined (DEBUG) || defined (INTERNAL_RELEASE)
         Con::printf("Found Drive: %s, vol: %s, serial: %d", drivePtr, driveVolume, driveSerial);
#endif
         //see if the volume and serial number match
         if (!dStricmp(volumeName, driveVolume) && (!serialNum || (serialNum == driveSerial)))
         {
            //see if the file exists on this volume
            if(dStrlen(drivePtr) == 3 && drivePtr[2] == '\\' && filePath[0] == '\\')
               dSprintf(fileBuf, sizeof(fileBuf), "%s%s", drivePtr, filePath + 1);
            else
               dSprintf(fileBuf, sizeof(fileBuf), "%s%s", drivePtr, filePath);
#if defined (DEBUG) || defined (INTERNAL_RELEASE)
            Con::printf("Looking for file: %s on %s", fileBuf, driveVolume);
#endif
            WIN32_FIND_DATA findData;
            HANDLE h = FindFirstFile(fileBuf, &findData);
            if(h != INVALID_HANDLE_VALUE)
            {
               FindClose(h);
               return true;
            }
            FindClose(h);
         }
      }

      //check the next drive
      drivePtr += dStrlen(drivePtr) + 1;
   }

   return false;
}
***************************************************************************************/

//--------------------------------------
bool Platform::dumpPath(const char *path, Vector<Platform::FileInfo> &fileVector)
{
   return recurseDumpPath(path, "*", fileVector);
}

//--------------------------------------
StringTableEntry Platform::getWorkingDirectory()
{
   static StringTableEntry cwd = NULL;

   if (!cwd)
   {
      char cwd_buf[2048];
      getcwd(cwd_buf, 2047);
      cwd = StringTable->insert(cwd_buf);
   }
   return cwd;
}

//--------------------------------------
bool Platform::isFile(const char *pFilePath)
{
   if (!pFilePath || !*pFilePath)
      return false;
   // Get file info
   struct stat fStat;
   if (stat(pFilePath, &fStat) < 0)
      return false;

   // if the file is a "regular file" then true
   if ( (fStat.st_mode & S_IFMT) == S_IFREG)
      return true;
   // must be some other file (directory, device, etc.)
   return false;
}


//--------------------------------------
bool Platform::isDirectory(const char *pDirPath)
{
   if (!pDirPath || !*pDirPath)
      return false;
 
   // Get file info
   struct stat fStat;
   if (stat(pDirPath, &fStat) < 0)
      return false;
      
   // if the file is a Directory then true
   if ( (fStat.st_mode & S_IFMT) == S_IFDIR)
      return true;

   return false;
}

//--------------------------------------
bool Platform::isSubDirectory(const char *pParent, const char *pDir)
{
   if (!pParent || !*pDir)
      return false;
   
   // this is somewhat of a brute force method but we need to be 100% sure
   // that the user cannot enter things like ../dir or /dir etc,...
   DIR *directory;

   directory = opendir(pParent);
   if (directory == NULL)
      return false;

   struct dirent *fEntry;
   fEntry = readdir(directory);
   if ( fEntry == NULL )
      return false;

   do
   {
      char dirBuf[MAXPATHLEN];
      struct stat fStat;

      dSprintf(dirBuf, sizeof(dirBuf), "%s/%s", pParent, fEntry->d_name);
      if (stat(dirBuf, &fStat) < 0)
         return false;
      // if it is a directory...
      if ( (fStat.st_mode & S_IFMT) == S_IFDIR)
      {
         // and the names match
         if (dStrcmp(pDir, fEntry->d_name ) == 0)
         {
            // then we have a real sub directory
            closedir(directory);
            return true;
         }
      }
   } while( (fEntry = readdir(directory)) != NULL );
   
   closedir(directory);
   return false;
}
