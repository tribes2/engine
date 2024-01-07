//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BADWORDFILTER_H_
#define _BADWORDFILTER_H_

#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif

class BadWordFilter
{
private:
   struct FilterTable
   {
      U16 nextState[26]; // only 26 alphabetical chars.
      FilterTable();
   };
   friend struct FilterTable;
   Vector<FilterTable*> filterTables;

   enum {
      TerminateNotFound = 0xFFFE,
      TerminateFound = 0xFFFF,
      MaxBadwordLength = 32,
   };
   char defaultReplaceStr[32];

   BadWordFilter();
   ~BadWordFilter();
   U32 curOffset;
   static U8 remapTable[257];
   static U8 randomJunk[MaxBadwordLength + 1];
   static bool filteringEnabled;

public:
   bool addBadWord(const char *word);
   bool setDefaultReplaceStr(const char *str);
   void filterString(char *string, const char *replaceStr = NULL);
   bool containsBadWords(const char *string);

   static bool isEnabled() { return filteringEnabled; }
   static void setEnabled(bool enable) { filteringEnabled = enable; }
   static void create();
   static void destroy();
};

extern BadWordFilter *gBadWordFilter;

#endif
