//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/banList.h"
#include "console/consoleTypes.h"
#include "core/resManager.h"
#include "core/fileStream.h"

IMPLEMENT_CONOBJECT(BanList);
BanList gBanList;

//------------------------------------------------------------------------------

void BanList::addBan(S32 uniqueId, const char *TA, S32 banTime)
{
   S32 curTime = Platform::getTime();
   
   if(banTime != 0 && banTime < curTime)
      return;

   // make sure this bastard isn't already banned on this server
   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();i++)
   {
      if(uniqueId == i->uniqueId)
      {
         i->bannedUntil = banTime;
         return;
      }
   }
   
   BanInfo b;
   dStrcpy(b.transportAddress, TA);
   b.uniqueId = uniqueId;
   b.bannedUntil = banTime;
   
   if(!dStrnicmp(b.transportAddress, "ip:", 3))
   {
      char *c = dStrchr(b.transportAddress+3, ':');
      if(c)
      {
         *(c+1) = '*';
         *(c+2) = 0;
      }
   }
   
   list.push_back(b);
}

//------------------------------------------------------------------------------

void BanList::addBanRelative(S32 uniqueId, const char *TA, S32 numSeconds)
{
   S32 curTime = Platform::getTime();
   S32 banTime = 0;
   if(numSeconds != -1)
      banTime = curTime + numSeconds;
   
   addBan(uniqueId, TA, banTime);
}

//------------------------------------------------------------------------------

void BanList::removeBan(S32 uniqueId, const char *TA)
{
   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();i++)
   {
      if(uniqueId == i->uniqueId)
      {
         list.erase(i);
         return;
      }
   }
}

//------------------------------------------------------------------------------

bool BanList::isBanned(S32 uniqueId, const char *TA)
{
   S32 curTime = Platform::getTime();
   
   Vector<BanInfo>::iterator i;
   for(i = list.begin();i != list.end();)
   {
      if(i->bannedUntil != 0 && i->bannedUntil < curTime)
      {
         list.erase(i);
         continue;
      }
      else if(uniqueId == i->uniqueId)
         return true;
      i++;
   }
   return false;
}

//------------------------------------------------------------------------------

bool BanList::isTAEq(const char *bannedTA, const char *TA)
{
   char a, b;
   for(;;)
   {
      a = *bannedTA++;
      b = *TA++;
      if(a == '*' || (!a && b == ':')) // ignore port
         return true;
      if(dTolower(a) != dTolower(b))
         return false;
      if(!a)
         return true;
   }
}

//------------------------------------------------------------------------------

void BanList::exportToFile(const char *fileName)
{
   FileStream banlist;
   
   if(ResourceManager->openFileForWrite(banlist, ResourceManager->getBasePath(), fileName, FileStream::Write))
   {
      char buf[1024];
      Vector<BanInfo>::iterator i;
      for(i = list.begin(); i != list.end(); i++)
      {
         dSprintf(buf, sizeof(buf), "BanList::addAbsolute(%d, \"%s\", %d);\r\n", i->uniqueId, i->transportAddress, i->bannedUntil);
         banlist.write(dStrlen(buf), buf);
      }
   }
   
   banlist.close();
}

// ---------------------------------------------------------
// console methods
// ---------------------------------------------------------

static void cAdd(SimObject *, S32, const char **argv)
{
   gBanList.addBan( dAtoi( argv[1] ), argv[2], dAtoi( argv[3] ) );
}

//------------------------------------------------------------------------------

static void cAddRelative(SimObject *, S32, const char **argv)
{
   gBanList.addBanRelative( dAtoi( argv[1] ), argv[2], dAtoi( argv[3] ) );
}

//------------------------------------------------------------------------------

static void cRemoveBan(SimObject *, S32, const char **argv)
{
   gBanList.removeBan( dAtoi( argv[1] ), argv[2] );
}

//------------------------------------------------------------------------------

static bool cIsBanned(SimObject *, S32, const char **argv)
{
   return (gBanList.isBanned( dAtoi( argv[1] ), argv[2] ));
}

//------------------------------------------------------------------------------

static void cExport(SimObject *, S32, const char **argv)
{
   gBanList.exportToFile( argv[1] );
}

//------------------------------------------------------------------------------

void BanList::consoleInit()
{
   Con::addCommand("BanList", "add", cAddRelative, "BanList::add( id, TA, banTime )", 4, 4);
   Con::addCommand("BanList", "addAbsolute", cAdd, "BanList::addAbsolute( id, TA, banTime )", 4, 4);
   Con::addCommand("BanList", "removeBan", cRemoveBan, "BanList::removeBan( id, TA )", 3, 3);
   Con::addCommand("BanList", "isBanned", cIsBanned, "BanList::isBanned( id, TA )", 3, 3);
   Con::addCommand("BanList", "export", cExport, "BanList::export( filename )", 2, 2);
}

//------------------------------------------------------------------------------
