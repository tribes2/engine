//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/commanderMapIcon.h"
#include "console/consoleTypes.h"
#include "dgl/materialList.h"
#include "dgl/gTexManager.h"
#include "core/resManager.h"
#include "core/bitStream.h"
#include "math/mMath.h"

//------------------------------------------------------------------------------
// Class:: CommanderIconImage
//------------------------------------------------------------------------------
CommanderIconImage::CommanderIconImage()
{
   mType = Static;
   mOverlay = false;
   mModulate = true;
   mMaterialList = 0;
   mTexture = 0;
   mAnimationType = Looping;
   mAnimationSpeed = 100;
}

CommanderIconImage::~CommanderIconImage()
{
   delete mMaterialList;
}

// "type image overlay modulate <animType animSpeed>"
CommanderIconImage * CommanderIconImage::construct(const char * desc)
{
   if(!desc || !desc[0])
      return(0);

   char buf[1024];
   dStrncpy(buf, desc, sizeof(buf));

   char * typeStr = dStrtok(buf, " ");
   char * imageStr = dStrtok(0, " ");
   char * overlayStr = dStrtok(0, " ");
   char * modulateStr = dStrtok(0, " ");

   if(!typeStr || !imageStr || !overlayStr || !modulateStr)
   {
      Con::errorf(ConsoleLogEntry::General, "CommanderIconImage::construct: invalid fields");
      return(0);
   }

   CommanderIconImage * image = new CommanderIconImage();

   // type:
   if(!dStricmp(typeStr, "static")) 
      image->mType = Static;
   else if(!dStricmp(typeStr, "animation"))
      image->mType = Animation;
   else
      goto Failed;

   // overlay/modulate:
   image->mOverlay = dAtob(overlayStr);
   image->mModulate = dAtob(modulateStr);

   // image/animation?:
   if(image->mType == Animation)  // dml
   {
      char * animType = dStrtok(0, " ");
      char * animSpeed = dStrtok(0, " ");
      
      if(!animType || !animSpeed)
         goto Failed;
      
      if(!dStricmp(animType, "looping"))
         image->mAnimationType = Looping;
      else if(!dStricmp(animType, "flipflop"))
         image->mAnimationType = FlipFlop;
      else if(!dStricmp(animType, "oneshot"))
         image->mAnimationType = OneShot;
      else
         goto Failed;

      image->mAnimationSpeed = dAtoi(animSpeed);

      char fileBuf[256];
      dSprintf(fileBuf, sizeof(fileBuf), "textures/commander/icons/%s.dml", imageStr);

      Stream * stream = ResourceManager->openStream(fileBuf);
      if(!stream)
         goto Failed;

      image->mMaterialList = new MaterialList;
      image->mMaterialList->setTextureType(BitmapKeepTexture);

      bool ret = image->mMaterialList->read(*stream);
      ResourceManager->closeStream(stream);

      if(!ret)
         goto Failed;
      
      if(!image->mMaterialList->load())
         goto Failed;

      if(!image->mMaterialList->size())
         goto Failed;
   }
   else  // image
   {
      char fileBuf[256];
      dSprintf(fileBuf, sizeof(fileBuf), "commander/icons/%s", imageStr);
      image->mTexture.set(fileBuf, BitmapKeepTexture, false);
      if(!bool(image->mTexture))
         goto Failed;
   }
   return(image);

 Failed:
   delete image;
   Con::errorf(ConsoleLogEntry::General, "CommanderIconImage::construct: failed to construct image '%s'", desc);
   return(0);
}

bool CommanderIconImage::getFrameSize(Point2I & size, U32 frame)
{
   switch(mType)
   {
      case Static:
      {
         if(!bool(mTexture) || frame)
            return(false);
         size.x = mTexture.getWidth();
         size.y = mTexture.getHeight();
         return(true);
      }

      case Animation:
      {
         if(!mMaterialList || (frame >= mMaterialList->size()))
            return(false);
         TextureHandle handle = mMaterialList->getMaterial(frame);
         if(!bool(handle))
            return(false);
         size.x = handle.getWidth();
         size.y = handle.getHeight();
         return(true);
      }
   }
   return(false);
}

//------------------------------------------------------------------------------
// Class: CommanderIconData
//------------------------------------------------------------------------------
IMPLEMENT_CO_DATABLOCK_V1(CommanderIconData);

CommanderIconData::CommanderIconData()
{
   mLoaded = false;
   for(U32 i = 0; i < NumImages; i++)
   {
      mImages[i] = 0;
      mImageDesc[i] = StringTable->insert("");
   }
}

CommanderIconData::~CommanderIconData()
{
   for(U32 i = 0; i < NumImages; i++)
      delete mImages[i];
}

void CommanderIconData::packData(BitStream * stream)
{
   Parent::packData(stream);
   for(U32 i = 0; i < NumImages; i++)
      stream->writeString(mImageDesc[i]);
}

void CommanderIconData::unpackData(BitStream * stream)
{
   Parent::unpackData(stream);
   for(U32 i = 0; i < NumImages; i++)
      mImageDesc[i] = stream->readSTString();
}

bool CommanderIconData::preload(bool server, char errorBuffer[256])
{
   if(!Parent::preload(server, errorBuffer))
      return(false);

   if(!server && !mLoaded)
   {
      for(U32 i = 0; i < NumImages; i++)
         mImages[i] = CommanderIconImage::construct(mImageDesc[i]);
      mLoaded = true;
   }

   return(true);
}

//------------------------------------------------------------------------------
IMPLEMENT_SETDATATYPE(CommanderIconData)
IMPLEMENT_GETDATATYPE(CommanderIconData)

void CommanderIconData::initPersistFields()
{
   Parent::initPersistFields();

   addField("images",        TypeString, Offset(mImageDesc,                CommanderIconData), NumImages);
   addField("baseImage",     TypeString, Offset(mImageDesc[BaseImage],     CommanderIconData));
   addField("activeImage",   TypeString, Offset(mImageDesc[ActiveImage],   CommanderIconData));
   addField("inactiveImage", TypeString, Offset(mImageDesc[InactiveImage], CommanderIconData));
   addField("selectImage",   TypeString, Offset(mImageDesc[SelectImage],   CommanderIconData));
   addField("hilightImage",  TypeString, Offset(mImageDesc[HilightImage],  CommanderIconData));

   Con::registerType(TypeCommanderIconDataPtr, sizeof(CommanderIconData*),
                     REF_GETDATATYPE(CommanderIconData),
                     REF_SETDATATYPE(CommanderIconData));
}


