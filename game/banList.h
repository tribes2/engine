//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BANLIST_H_
#define _BANLIST_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif

class BanList : public SimObject
{
public:

   struct BanInfo
   {
      S32      uniqueId;
      char     transportAddress[128];
      S32      bannedUntil;
   };

   Vector<BanInfo> list;

   BanList(){}
   ~BanList(){}
   
   void addBan(S32 uniqueId, const char *TA, S32 banTime);
   void addBanRelative(S32 uniqueId, const char *TA, S32 numSeconds);
   void removeBan(S32 uniqueId, const char *TA);
   bool isBanned(S32 uniqueId, const char *TA);
   bool isTAEq(const char *bannedTA, const char *TA);
   void exportToFile(const char *fileName);
   
   DECLARE_CONOBJECT(BanList);

   static void consoleInit();
};

extern BanList gBanList;

#endif
