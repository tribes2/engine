//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _OVECTOR_H_
#define _OVECTOR_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

#ifdef DEBUG_GUARD
extern bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize,
                         const char* fileName,
                         const U32   lineNum);
#else
extern bool VectorResize(U32 *aSize, U32 *aCount, void **arrayPtr, U32 newCount, U32 elemSize);
#endif

template<class T> class OVector
{
   protected:
      U16   mElementCount;
      U16   mArraySize;
      T  *  mArray;

      // Could probably use the low bit as flag and double our max size here...
      U16   userOwned() const       {return (mArraySize & 0x8000);}
      U16   arraySize() const       {return (mArraySize & 0x7fff);}
      
      bool resize(U32 ecount) {
         bool  Ok = true;
         AssertFatal(ecount < (1<<15), "OVector: 32K maximum exceeded");
         if (!userOwned()) {
            U32   size = arraySize();        // Want to use existing VectorResize(), 
            U32   count = mElementCount;     // so convert to U32s for it... 
#ifdef DEBUG_GUARD
            Ok = VectorResize(&size, &count, (void**) &mArray, ecount, sizeof(T), __FILE__, __LINE__);
#else
            Ok = VectorResize(&size, &count, (void**) &mArray, ecount, sizeof(T));
#endif
            mArraySize = size | userOwned();
            mElementCount = count;
         }
         else {
            AssertISV(ecount <= arraySize(), "OVector: overgrown owned vector");
            mElementCount = ecount;
         }
         return Ok;
      }
      
  public:
      OVector() {
         mArray = NULL;  
         mElementCount = mArraySize = 0;
      }
      
      ~OVector() {
         if (!userOwned())
            dFree(mArray);
      }

      typedef T*        iterator;
      typedef const T*  const_iterator;

      // One-liners- 
      iterator begin()                    {return mArray;}
      iterator end()                      {return mArray + mElementCount;}
      T& front()                          {return * begin();}
      T& back()                           {return * end();}
      T& first()                          {return mArray[0];}
      T& last()                           {return mArray[mElementCount - 1];}
      T& operator[](S32 i)                {return operator[](U32(i));}
      T& operator[](U32 i)                {return mArray[i];}
      const T& first() const              {return mArray[0];}
      const T& last() const               {return mArray[mElementCount - 1];}
      const_iterator begin() const        {return mArray;}
      const_iterator end() const          {return mArray + mElementCount;}
      const T& front() const              {return * begin();}
      const T& back() const               {return * end();}
      const T& operator[](U32 i) const    {return mArray[i];}
      const T& operator[](S32 i) const    {return operator[](U32(i));}
      void  clear()                       {mElementCount = 0;}
      void  compact()                     {resize(mElementCount);}
      bool  empty() const                 {return (mElementCount == 0);}
      bool  isOwned() const               {return (userOwned() != 0);}
      S32   memSize() const               {return capacity() * sizeof(T);}
      U32   capacity() const              {return arraySize();}
      T *   address() const               {return mArray;}
      S32   size() const                  {return S32(mElementCount);}

      // This is where user sets their own data.  We then allow all other operations to
      // go on as normal - errors will be caught in resize(), which everything uses.  
      void setOwned(T * data, U16 available, bool setSize = false) {
         if (!userOwned())
            dFree(mArray);
         AssertFatal(available < (1<<15), "OVector: can only hold 32K");
         mElementCount = (setSize ? available : 0);
         mArraySize = (available | 0x8000);
         mArray = data;
      }
      
      void clearOwned() {
         if (userOwned()) {
            mElementCount = 0;
            mArraySize = 0;
            mArray = 0;
         }
      }
      
      S32 setSize(U32 size) {
         if (size > arraySize())
            resize(size);
         else
            mElementCount = size;
         return mElementCount;
      }

      void increment(U32 delta=1) {
         if ((mElementCount += delta) > arraySize())
            resize(mElementCount);
      }

      void decrement(U32 delta = 1) {
         if (mElementCount > delta)
            mElementCount -= delta;
         else
            mElementCount = 0;
      }
      
      void insert(U32 i) {
         increment();
         dMemmove(&mArray[i + 1], &mArray[i], (mElementCount - i - 1) * sizeof(T));
      }

      void erase(U32 i) {
         dMemmove(&mArray[i], &mArray[i + 1], (mElementCount - i - 1) * sizeof(T));
         decrement();
      }

      void erase_fast(U32 i) {         // CAUTION: this does not maintain list order
         if (i < (mElementCount - 1))  // Copies the last element into the deleted hole
            dMemmove(&mArray[i], &mArray[mElementCount - 1], sizeof(T));
         decrement();
      }
      
      void erase_fast(iterator q) {
         erase_fast(U32(q - mArray));
      }

      void push_back(const T& x) {
         increment();
         mArray[mElementCount - 1] = x;
      }
      
      void push_front(const T & x) {
         insert(0);
         mArray[0] = x;
      }

      void pop_front() {
         erase(U32(0));
      }

      void pop_back() {
         decrement();
      }

      void reserve(U32 size) {
         if (size > arraySize()) {
            S32 ec = S32(mElementCount);
            if (resize(size))
               mElementCount = U32(ec);
         }
      }

      void operator=(const OVector& p) {
         resize(p.mElementCount);
         if (p.mElementCount)
            dMemcpy(mArray,p.mArray,mElementCount * sizeof(T));
      }

      void merge(const OVector& p) {
         if (p.size()) {
            S32 oldsize = size();
            resize(oldsize + p.size());
            dMemcpy( &mArray[oldsize], p.address(), p.size() * sizeof(T) );
         }
      }
};

#endif //_OVECTOR_H_
