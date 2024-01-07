//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _X86UNIXUTILS_H_
#define _X86UNIXUTILS_H_

class UnixUtils
{
public:
   UnixUtils();

   /**
      Return true if we're running in the background, false otherwise.
      There's no "standard" way to determine this in unix, but 
      modern job control unices should support the method described
      here:

      http://www.faqs.org/faqs/unix-faq/faq/part3/

      (question 3.7)
    */
   bool inBackground();
};

extern UnixUtils *UUtils;

#endif
