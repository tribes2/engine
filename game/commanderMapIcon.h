//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _COMMANDERMAPICON_H_
#define _COMMANDERMAPICON_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif

class MaterialList;

//------------------------------------------------------------------------------
class CommanderIconImage 
{
   public:
      enum Type {
         Static = 0,
         Animation
      };
      Type                       mType;

      bool                       mOverlay;
      bool                       mModulate;

      TextureHandle              mTexture;
      MaterialList *             mMaterialList;

      enum AnimationType {
         Looping,
         FlipFlop,
         OneShot,
      };
      AnimationType              mAnimationType;
      S32                        mAnimationSpeed;
      
      CommanderIconImage();
      ~CommanderIconImage();

      bool getFrameSize(Point2I &, U32 frame = 0);

      static CommanderIconImage * construct(const char *);
};

//------------------------------------------------------------------------------
class CommanderIconData : public SimDataBlock
{
   private:
      typedef SimDataBlock    Parent;

   public:
      enum {
         BaseImage = 0,
         ActiveImage,
         InactiveImage,
         SelectImage,
         HilightImage,

         NumImages
      };
      CommanderIconImage *    mImages[NumImages];

   private:
      StringTableEntry        mImageDesc[NumImages];
      bool mLoaded;

   public:
      CommanderIconData();
      ~CommanderIconData();

      void packData(BitStream *);
      void unpackData(BitStream *);
      bool preload(bool, char errorBuffer[256]);

      static void initPersistFields();

      DECLARE_CONOBJECT(CommanderIconData);
};

#endif
