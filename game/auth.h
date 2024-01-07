//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUTH_H_
#define _AUTH_H_

#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

class Auth2Certificate
{
   U32 xxx;
};

struct InfoServer
{
   NetAddress address;
   U32 region;
};

enum {
   MaxWarriorNameLen = 31,
   MaxTribeTagLen = 15,
   MaxTribeNameLen = 63,
   MaxTribes = 10,

   Tribes2ClientCommunityId = 98,
   Tribes2ServerCommunityId = 99,
};

struct AuthTribeInfo
{
   char tribeName[MaxTribeNameLen + 1];
   char tribeTag[MaxTribeTagLen + 1];
   U32 tribeId;
   U32 privLevel;
   bool tagAppend;
};

struct AuthInfo
{
   bool valid;
   char warriorName[MaxWarriorNameLen + 1];
   U32 wonID;
   U32 tribeCount;
   AuthTribeInfo ti[MaxTribes];
};

#endif
