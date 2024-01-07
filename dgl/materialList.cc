//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "dgl/gTexManager.h"
#include "Core/resManager.h"
#include "Core/stream.h"
#include "dgl/materialList.h"

//--------------------------------------
MaterialList::MaterialList()
{
   mTextureType = BitmapTexture;
   mClampToEdge = false;

   VECTOR_SET_ASSOCIATION(mMaterialNames);
   VECTOR_SET_ASSOCIATION(mMaterials);
}

MaterialList::MaterialList(const MaterialList* pCopy)
{
   VECTOR_SET_ASSOCIATION(mMaterialNames);
   VECTOR_SET_ASSOCIATION(mMaterials);

   mClampToEdge = pCopy->mClampToEdge;
   mTextureType = pCopy->mTextureType;

   mMaterialNames.setSize(pCopy->mMaterialNames.size());
   U32 i;
   for (i = 0; i < mMaterialNames.size(); i++) {
      if (pCopy->mMaterialNames[i]) {
         mMaterialNames[i] = new char[dStrlen(pCopy->mMaterialNames[i]) + 1];
         dStrcpy(mMaterialNames[i], pCopy->mMaterialNames[i]);
      } else {
         mMaterialNames[i] = NULL;
      }
   }

   mMaterials.setSize(pCopy->mMaterials.size());
   for (i = 0; i < mMaterials.size(); i++) {
      constructInPlace(&mMaterials[i]);
      mMaterials[i] = pCopy->mMaterials[i];
   }
}



MaterialList::MaterialList(U32 materialCount, const char **materialNames)
{
   VECTOR_SET_ASSOCIATION(mMaterialNames);
   VECTOR_SET_ASSOCIATION(mMaterials);

   set(materialCount, materialNames);
}


//--------------------------------------
void MaterialList::set(U32 materialCount, const char **materialNames)
{
   free();
   mMaterials.setSize(materialCount);
   mMaterialNames.setSize(materialCount);
   for(U32 i = 0; i < materialCount; i++)
   {
      // vectors DO NOT initialize classes so manually call the constructor
      constructInPlace(&mMaterials[i]);
      mMaterialNames[i] = new char[dStrlen(materialNames[i]) + 1];
      dStrcpy(mMaterialNames[i], materialNames[i]);
   }
}


//--------------------------------------
MaterialList::~MaterialList()
{
   free();
}


//--------------------------------------
void MaterialList::load(U32 index)
{
   AssertFatal(index < size(), "MaterialList:: index out of range.");
   if (index < size()) 
   {
      TextureHandle &handle = mMaterials[index];
      if (handle.getBitmap() == NULL)
      {
         const char *name = mMaterialNames[index];
         if (name && *name)
            handle.set(name, mTextureType, mClampToEdge);   
      }
   }
}   


//--------------------------------------
bool MaterialList::load()
{
   AssertFatal(mMaterials.size() == mMaterials.size(), "MaterialList::load: internal vectors out of sync.");
   
   for(U32 i=0; i < mMaterials.size(); i++)
      load(i);
   return true;
}


//--------------------------------------
void MaterialList::unload()
{
   AssertFatal(mMaterials.size() == mMaterials.size(), "MaterialList::unload: internal vectors out of sync.");
   for(U32 i=0; i < mMaterials.size(); i++)
      mMaterials[i].~TextureHandle();
}   


//--------------------------------------
void MaterialList::free()
{
   AssertFatal(mMaterials.size() == mMaterials.size(), "MaterialList::free: internal vectors out of sync.");
   for(U32 i=0; i < mMaterials.size(); i++)
   {
      if(mMaterialNames[i]) 
         delete [] mMaterialNames[i];
      mMaterials[i].~TextureHandle();
   }
   mMaterialNames.setSize(0);
   mMaterials.setSize(0);
}   


//--------------------------------------
U32 MaterialList::push_back(TextureHandle textureHandle, const char * filename)
{
   mMaterials.increment();
   mMaterialNames.increment();
   
   // vectors DO NOT initialize classes so manually call the constructor
   constructInPlace(&mMaterials.last());
   mMaterials.last()    = textureHandle;
   mMaterialNames.last() = new char[dStrlen(filename) + 1];
   dStrcpy(mMaterialNames.last(), filename);

   // return the index
   return mMaterials.size()-1;
}   

//--------------------------------------
U32 MaterialList::push_back(const char *filename)
{
   mMaterials.increment();
   mMaterialNames.increment();
   
   // vectors DO NOT initialize classes so manually call the constructor
   constructInPlace(&mMaterials.last());
   mMaterialNames.last() = new char[dStrlen(filename) + 1];
   dStrcpy(mMaterialNames.last(), filename);

   // return the index
   return mMaterials.size()-1;
}   


//--------------------------------------
U32 MaterialList::push_back(const char *filename, GBitmap *bmp, TextureHandleType type, bool clampToEdge)
{
   mMaterials.increment();
   mMaterialNames.increment();
   
   // vectors DO NOT initialize classes so manually call the constructor
   constructInPlace(&mMaterials.last());
   mMaterials.last().set(filename, bmp, type, clampToEdge);
   mMaterialNames.last() = new char[dStrlen(filename) + 1];
   dStrcpy(mMaterialNames.last(), filename);

   // return the index
   return mMaterials.size()-1;
}   


//--------------------------------------
bool MaterialList::read(Stream &stream)
{
   free();

   // check the stream version
   U8 version;
   if ( stream.read(&version) && version != BINARY_FILE_VERSION)
      return readText(stream,version);

   // how many materials?
   U32 count;
   if ( !stream.read(&count) )
      return false;

   // pre-size the vectors for efficiency
   mMaterials.reserve(count);
   mMaterialNames.reserve(count);

   // read in the materials
   for (U32 i=0; i<count; i++)
   {
      char buffer[256];
      stream.readString(buffer);
      if( !buffer[0] )
      {
         AssertWarn(0, "MaterialList::read: error reading stream");
         return false;
      }

      // add it to the list
      mMaterials.increment();
      mMaterialNames.increment();
      // vectors DO NOT initialize classes so manually call the constructor
      constructInPlace(&mMaterials.last());
      mMaterialNames.last() = new char[dStrlen(buffer) + 1];
      dStrcpy(mMaterialNames.last(), buffer);
   }

   return (stream.getStatus() == Stream::Ok);
}


//--------------------------------------
bool MaterialList::write(Stream &stream)
{
   AssertFatal(mMaterials.size() == mMaterialNames.size(), "MaterialList::write: internal vectors out of sync.");
   
   stream.write((U8)BINARY_FILE_VERSION);    // version
   stream.write((U32)mMaterials.size());     // material count

   for(U32 i=0; i < mMaterials.size(); i++)  // material names
      stream.writeString(mMaterialNames[i]); 
   
   return (stream.getStatus() == Stream::Ok);
}


//--------------------------------------
bool MaterialList::readText(Stream &stream, U8 firstByte)
{
   free();

   if (!firstByte)
      return (stream.getStatus() == Stream::Ok || stream.getStatus() == Stream::EOS);

   char buf[1024];
   buf[0] = firstByte;
   U32 offset = 1;

   for(;;)
   {
      stream.readLine((U8*)(buf+offset), sizeof(buf)-offset);
      if(!buf[0])
         break;
      offset = 0;
     
      mMaterials.increment();
      mMaterialNames.increment();

      // vectors DO NOT initialize classes so manually call the constructor
      constructInPlace(&mMaterials.last());
      mMaterialNames.last() = new char[dStrlen(buf) + 1];

      dStrcpy(mMaterialNames.last(), buf);
   }
   return (stream.getStatus() == Stream::Ok || stream.getStatus() == Stream::EOS);
}

bool MaterialList::readText(Stream &stream)
{
   U8 firstByte;
   stream.read(&firstByte);
   return readText(stream,firstByte);
}

//--------------------------------------
bool MaterialList::writeText(Stream &stream)
{
   AssertFatal(mMaterials.size() == mMaterialNames.size(), "MaterialList::writeText: internal vectors out of sync.");

   for(U32 i=0; i < mMaterials.size(); i++)
      stream.writeLine((U8*)mMaterialNames[i]);
   stream.writeLine((U8*)"");

   return (stream.getStatus() == Stream::Ok);
}


//--------------------------------------
ResourceInstance* constructMaterialList(Stream &stream)
{
   MaterialList *matList = new MaterialList;
   if(matList->readText(stream))
      return matList;
   else
   {
      delete matList;
      return NULL;
   }
}
