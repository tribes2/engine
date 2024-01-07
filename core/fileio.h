//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _FILEIO_H_
#define _FILEIO_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

class File
{
public:
    enum Status
    {
        Ok = 0,           // obvious
        IOError,          // Read or Write error
        EOS,              // End of Stream reached (mostly for reads)
        IllegalCall,      // An unsupported operation used.  Always w/ accompanied by AssertWarn
        Closed,           // Tried to operate on a closed stream (or detached filter)
        UnknownError      // Catchall
    };
    enum AccessMode
    {
        Read         = 0,
        Write        = 1,
        ReadWrite    = 2,
        WriteAppend  = 3
    };
    enum Capability
    {
        FileRead         = (1<<0),
        FileWrite        = (1<<1)
    };

private:
    void *handle;               // pointer to the file handle
    Status currentStatus;       // current status of the file (Ok, IOError, etc.)
    U32 capability;          // keeps track of file capabilities

    File(const File&);              // disable copy constructor
    File& operator=(const File&);   // disable assignment
    
public:
    File();                     // default constructor
    virtual ~File();            // destructor

    Status open(const char *filename, const AccessMode openMode);
    U32 getPosition() const;
    Status setPosition(S32 position, bool absolutePos = true);
    U32 getSize() const;
    Status flush();
    Status close();
    Status getStatus() const;
    Status read(U32 size, char *dst, U32 *bytesRead = NULL);
    Status write(U32 size, const char *src, U32 *bytesWritten = NULL);
    bool hasCapability(Capability cap) const;
    
protected:
    Status setStatus();                 // called after error encountered
    Status setStatus(Status status);    // assign specific value
};

#endif // _FILE_IO_H_
