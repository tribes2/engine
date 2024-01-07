//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TALGORITHM_H_
#define _TALGORITHM_H_

//Includes

template <class Iterator, class Value>
Iterator find(Iterator first, Iterator last, Value value)
{
   while (first != last && *first != value)
      ++first;
   return first;
}

#endif //_TALGORITHM_H_
