//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CANNEDCHATDATABLOCK_H_
#define _CANNEDCHATDATABLOCK_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

struct CannedChatItem : public SimDataBlock
{
   typedef SimDataBlock Parent;

   StringTableEntry  mName;
   StringTableEntry  mText;
   StringTableEntry  mAudioFile;
   StringTableEntry  mAnimation;
   bool              mTeamOnly;
   bool              mPlay3D;
   // more...

   CannedChatItem();
   DECLARE_CONOBJECT( CannedChatItem );
   static void consoleInit();
   static void initPersistFields();
   virtual bool onAdd();
};

#endif // _H_CANNEDCHATDATABLOCK_
