//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _DATACHUNKER_H_
#define _DATACHUNKER_H_


//----------------------------------------------------------------------------

class DataChunker
{
  public:
   enum {
      ChunkSize = 16376
   };
  
  private:
   struct DataBlock
   {
      DataBlock *next;
      U8 *data;
      S32 curIndex;
      DataBlock(S32 size);
      ~DataBlock();
   };
   DataBlock *curBlock;
   S32 chunkSize;
  public:
   void *alloc(S32 size);
   void freeBlocks();

   DataChunker(S32 size=ChunkSize);
   ~DataChunker();
};


//----------------------------------------------------------------------------

template<class T>
class Chunker: private DataChunker
{
public:
   Chunker(S32 size = DataChunker::ChunkSize) : DataChunker(size) {};
   T* alloc()  { return reinterpret_cast<T*>(DataChunker::alloc(S32(sizeof(T)))); }
   void clear()  { freeBlocks(); };
};


#endif
