//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include "Core/fileio.h"
#include "Core/tVector.h"
#include "Core/stringTable.h"
#include "console/console.h"
#include <stdio.h>
#include <errno.h>

//-------------------------------------- Helper Functions
static void forwardslash(char *str)
{
   while(*str)
   {
      if(*str == '\\')
         *str = ':';
      str++;
   }
}

static void toHostFilename(const char *str, char *dst)
{
	*dst++ = ':';
   while(*str)
   {
      if(*str == '\\' || *str == '/')
         *dst++ = ':';
    	else
    		*dst++ = *str;
      str++;
   }
   *dst = 0;
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
   
   AssertFatal(dStrlen(filename) <= 255, "File::open: Max Mac file length exceeded. MAX=255");
   AssertFatal(NULL != filename, "File::open: NULL filename");
   AssertWarn(NULL == handle, "File::open: handle already valid");
	
   toHostFilename(filename, hostFilename);
    
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

//-----------------------------------------------------------------------------
// Get the current position of the file pointer.
//-----------------------------------------------------------------------------
U32 File::getPosition() const
{
	AssertFatal(Closed != currentStatus, "File::getPosition: file closed");
   AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::getPosition: invalid file handle");
	
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
   case EACCESS:	// permission denied
      return currentStatus = IOError;
   case EBADF:	// Bad File Pointer
      errno++;
   case EINVAL:	// Invalid argument
      errno++;
   case ENOENT:	// file not found
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
        {	// fread onlu reports the number of chunks read not bytes
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
        {	// fwrite onlu reports the number of chunks written not bytes
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

S32 Platform::compareFileTimes(const FileTime &a, const FileTime &b)
{
   if(a.time > b.time)
      return 1;
   if(a.time < b.time)
      return -1;
   return 0;
}


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
		cinfo.hFileInfo.ioVRefNum   = ppcState.volRefNum;	// volume ID to search in
		cinfo.hFileInfo.ioDirID     = dirID;					// directory ID to search in
		cinfo.hFileInfo.ioFDirIndex = index++;					// specify which entry you are interested in
		cinfo.hFileInfo.ioNamePtr   = nameField;				// supply a buffer to store the name
				
		result = PBGetCatInfoSync(&cinfo);
		if (result == noErr)
		{
			if (cinfo.dirInfo.ioFlAttrib & ioDirMask) 
			{	// it's a directory
				char *dirname = p2str(cinfo.hFileInfo.ioNamePtr);
            scratchBuf[0] = '\0';
            if (curPath[0] != '\0') {
               dStrcpy(scratchBuf, curPath);
               dStrcat(scratchBuf, "/");
            }
            dStrcat(scratchBuf, dirname);

            _recurseDumpPath(basePath, scratchBuf, cinfo.hFileInfo.ioDirID, out_rFileVector);
			}
			else
			{	// it's a file
				char *filename = p2str(cinfo.hFileInfo.ioNamePtr);
								
          	out_rFileVector.increment();
          	Platform::FileInfo& rInfo = out_rFileVector.last();

          	if (curPath[0] != '\0') {
             	dSprintf(scratchBuf, sizeof(scratchBuf), "%s/%s", basePath, curPath);
             	rInfo.pFullPath = StringTable->insert(scratchBuf);
             	rInfo.pVirtPath = StringTable->insert(curPath);
            } else {
             	rInfo.pFullPath = StringTable->insert(basePath);
             	rInfo.pVirtPath = NULL;
          	}

          	rInfo.pFileName = StringTable->insert(filename);
          	rInfo.fileSize  = cinfo.hFileInfo.ioFlLgLen;
          	//rInfo.createTime.time = cinfo.hFileInfo.ioFlCrDat;
          	//rInfo.modifyTime.time = cinfo.hFileInfo.ioFlMdDat;
         }
		} 
	}while (result == noErr);

	return true;
}

//--------------------------------------
bool Platform::getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime)
{
   return false;
/*      
   WIN32_FIND_DATA findData;
   HANDLE h = FindFirstFile(filePath, &findData);
   if(h == INVALID_HANDLE_VALUE)
      return false;

   if(createTime)
   {   
      createTime->v1 = findData.ftCreationTime.dwLowDateTime;
      createTime->v2 = findData.ftCreationTime.dwHighDateTime;
   }
   if(modifyTime)
   {
      modifyTime->v1 = findData.ftLastWriteTime.dwLowDateTime;
      modifyTime->v2 = findData.ftLastWriteTime.dwHighDateTime;
   }
   FindClose(h);
   return true;
*/   
}



//--------------------------------------
bool Platform::createPath(const char *file)
{
   char pathBuf[1024];
   char dirBuf[256];
   const char *dir;
   U32 pathLen = 0;
   S32 parentDirID = ppcState.dirID;

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
			
		cinfo.hFileInfo.ioVRefNum   = ppcState.volRefNum;	// volume ID to search in
		cinfo.hFileInfo.ioDirID     = parentDirID;					// directory ID to search in
		cinfo.hFileInfo.ioFDirIndex = 0;							// get info on ioNamePtr
		cinfo.hFileInfo.ioNamePtr   = nameField;				// supply a buffer with name
		str2p(dirBuf, nameField);
		OSErr err = PBGetCatInfoSync(&cinfo);
      switch(err)
      {
		   case noErr:
   			if (cinfo.dirInfo.ioFlAttrib & ioDirMask) 
   			{	// it's a directory
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
               OSErr err = DirCreate(ppcState.volRefNum, parentDirID, str2p(dirBuf), &newId);
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


//--------------------------------------
bool Platform::dumpPath(const char *in_pBasePath, Vector<Platform::FileInfo>& out_rFileVector)
{
	// for now we can only search directories in the apps current workinng directory
	// specifying another drive, path or sub path (base/art) will not work.
	S32 dirID = ppcState.dirID;
	
	if (in_pBasePath)
	{
		CInfoPBRec cinfo;
		Str63 nameField;
		OSErr result;
	
		cinfo.hFileInfo.ioVRefNum   = ppcState.volRefNum;	// volume ID to search in
		cinfo.hFileInfo.ioDirID     = dirID;					// directory ID to search in
		cinfo.hFileInfo.ioFDirIndex = 0;							// get info on ioNamePtr
		cinfo.hFileInfo.ioNamePtr   = nameField;				// supply a buffer with name
		str2p(in_pBasePath, nameField);
		result = PBGetCatInfoSync(&cinfo);
		if (result == noErr)
		{
			if (cinfo.dirInfo.ioFlAttrib & ioDirMask) 
			{	// it's a directory
				char *dirname = p2str(cinfo.hFileInfo.ioNamePtr);
            return _recurseDumpPath(in_pBasePath, NULL, cinfo.hFileInfo.ioDirID, out_rFileVector);
			}
			return false; 	// not a directory
		}
	}
	return _recurseDumpPath(in_pBasePath, NULL, dirID, out_rFileVector);
}


//--------------------------------------
void Platform::getCurrentDirectory(char *dirBuf, const U32 in_bufferSize)
{
    dirBuf, in_bufferSize;
    #pragma message("todo: Platform::getCurrentDirectory")
//   GetCurrentDirectory(in_bufferSize - 1, dirBuf);
//   forwardslash(dirBuf);

	*dirBuf = '\0';

/*
FUNCTION GetFullPath (DirID: LongInt; vRefnum: Integer): Str255;
VAR
   myPB:       CInfoPBRec;    {parameter block for PBGetCatInfo}
   dirName:    Str255;        {a directory name}
   fullPath:   Str255;        {full pathname being constructed}
   myErr:      OSErr;
BEGIN
   fullPath := '';               {initialize full pathname}
   myPB.ioNamePtr := @dirName;
   myPB.ioVRefNum := vRefNum;    {indicate target volume}
   myPB.ioDrParID := DirId;      {initialize parent directory ID}
   myPB.ioFDirIndex := -1;       {get info about a directory}
   {Get name of each parent directory, up to root directory.}
   REPEAT
      myPB.ioDrDirID := myPB.ioDrParID;
      myErr := PBGetCatInfo(@myPB, FALSE);
      IF gHaveAUX THEN

         BEGIN
            IF dirName[1] <> '/' THEN
               dirName := concat(dirName, '/');
         END
      ELSE
         dirName := concat(dirName, ':');
      fullPath := concat(dirName, fullPath);
   UNTIL myPB.ioDrDirID = fsRtDirID;
   GetFullPath := fullPath;      {return full pathname}
END;

Note that GetFullPath uses either a slash (/) or a colon (:) to separate names in the 
full path, depending on whether A/UX is running or not. The GetFullPath function reads 
the value of the global variable gHaveAUX to determine whether A/UX is running; your 
application must initialize this variable (preferably by calling the Gestalt function) 
before it calls GetFullPath.
The GetFullPath function defined in Listing 2-5 returns a result of type Str255, which 
limits the full pathname to 255 characters. An actual full pathname, however, might 
exceed 255 characters. A volume name can be up to 27 characters, and each directory name 
can be up to 31 characters. If the average volume and directory name is about 20 
characters long, GetFullPath can handle files located only about 12 levels deep. If the 
length of the average directory name is closer to the maximum, GetFullPath provides a 
full pathname for files located only about 8 levels deep. If necessary, you can overcome 
this limitation by rewriting GetFullPath to return a handle to the full pathname; the 
algorithm for ascending the directory hierarchy using PBGetCatInfo will still work, however.
*/




}

