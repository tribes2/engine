//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PROFILER_H_
#define _PROFILER_H_

#ifdef ENABLE_PROFILER

struct ProfilerData;
struct ProfilerRootData;

class Profiler
{
   enum {
      MaxStackDepth = 256,
      DumpFileNameLength = 256
   };
   U32 mCurrentHash;

   ProfilerData *mCurrentProfilerData;
   ProfilerData *mProfileList;
   ProfilerData *mRootProfilerData;

   bool mEnabled;
   S32 mStackDepth;
   bool mNextEnable;
   U32 mMaxStackDepth;
   bool mDumpToConsole;
   bool mDumpToFile;
   char mDumpFileName[DumpFileNameLength];
   void dump();
   void validate();
public:
   Profiler();
   ~Profiler();

   void reset();
   void dumpToConsole();
   void dumpToFile(const char*);
   void enable(bool enabled);
   void hashPush(ProfilerRootData *data);
   void hashPop();
   void enableMarker(const char *marker, bool enabled);
};

extern Profiler *gProfiler;

struct ProfilerRootData
{
   const char *mName;
   U32 mNameHash;
   ProfilerData *mFirstProfilerData;
   ProfilerRootData *mNextRoot;
   F64 mTotalTime;
   F64 mSubTime;
   U32 mTotalInvokeCount;
   bool mEnabled;

   static ProfilerRootData *sRootList;

   ProfilerRootData(const char *name);
};

struct ProfilerData
{
   ProfilerRootData *mRoot; // link to root node.
   ProfilerData *mNextForRoot; // links together all ProfilerData's for a particular root
   ProfilerData *mNextProfilerData; // links all the profilerDatas
   ProfilerData *mNextHash;
   ProfilerData *mParent;
   ProfilerData *mNextSibling;
   ProfilerData *mFirstChild;
   enum {
      HashTableSize = 32,
   };
   ProfilerData *mChildHash[HashTableSize];
   ProfilerData *mLastSeenProfiler;

   U32 mHash;
   U32 mSubDepth;
   U32 mInvokeCount;
   U32 mStartTime[2];
   F64 mTotalTime;
   F64 mSubTime;
};


#define PROFILE_START(name) \
static ProfilerRootData pdata##name##obj (#name); \
if(gProfiler) gProfiler->hashPush(& pdata##name##obj )

#define PROFILE_END() if(gProfiler) gProfiler->hashPop()

#else
#define PROFILE_START(x) 
#define PROFILE_END()
#endif

#endif
