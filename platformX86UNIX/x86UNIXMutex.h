//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _X86UNIXMUTEX_H_
#define _X86UNIXMUTEX_H_

// A process-shared mutex class, used by Platform::excludeOtherInstances()
class ProcessMutex
{
  public:
    ProcessMutex();
    ~ProcessMutex();

    bool acquire(const char *mutexName);
    void release();
  private:
    int mFD;
    static const int LockFileNameSize=256;
    char mLockFileName[LockFileNameSize];
};

#endif
