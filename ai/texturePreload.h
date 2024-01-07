//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TEXTUREPRELOAD_H_
#define _TEXTUREPRELOAD_H_

class PreloadTextures
{
      enum {MaxHandles = 512};
      TextureHandle     mTextures[MaxHandles];
      S32               mNext;
   
   public:
      PreloadTextures();
      ~PreloadTextures();
      void load(const char * name, bool clamp);
};

#endif
