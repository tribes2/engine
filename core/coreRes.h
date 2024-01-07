//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CORERES_H_
#define _CORERES_H_

#ifndef _RESMANAGER_H_
#include "Core/resManager.h"
#endif

class RawData
{
private:
   bool ownMemory;

public:
   char *data;
   S32 size;

   RawData()  { ownMemory = false; }
   RawData(Stream &s, S32 sz) {
      ownMemory = true;
      size      = sz;
      data      = new char[size];
      if (data)
         s.read(size, data);
   }
   ~RawData() {
      if (ownMemory)
         delete [] data;
      data      = NULL;
      ownMemory = false;
      size      = 0;
   }
};   


//-------------------------------------- RawData type
class ResourceTypeRawData : public ResourceType
{
public:   
   ResourceTypeRawData(const char *ext = ".dat"):
      ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(Stream *stream, S32 size) 
      { return (void*)new RawData(*stream, size); }
   void destruct(void *p) 
      { delete (RawData*)p; }
};   

class ResourceTypeStaticRawData : public ResourceType
{
public:   
   ResourceTypeStaticRawData(const char *ext = ".sdt"):
      ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(Stream *stream, S32 size) 
      { return (void*)new RawData(*stream, size); }
   void destruct(void *p) 
      { }
};   

#endif //_CORERES_H_

