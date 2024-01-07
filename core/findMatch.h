//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _FINDMATCH_H_
#define _FINDMATCH_H_

#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif

class   FindMatch
{
   char*  expression;
   U32 maxMatches;

  public:
   static bool isMatch( const char *exp, const char *string, bool caseSensitive = false );
   Vector<char *> matchList;

   FindMatch( U32 _maxMatches = 256 );
   FindMatch( char *_expression, U32 _maxMatches = 256 );
   ~FindMatch();

   bool findMatch(const char *string, bool caseSensitive = false);
   void setExpression( const char *_expression );

   S32  numMatches() const   { return(matchList.size());                }
   bool isFull() const       { return (matchList.size() >= maxMatches); }
   void clear()              { matchList.clear();                       }
};

#endif // _FINDMATCH_H_
