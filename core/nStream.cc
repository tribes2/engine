//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "Core/stream.h"
#include "Core/stringTable.h"
#include "Core/color.h"

Stream::Stream()
 : m_streamStatus(Closed)
{
   //
}

Stream::~Stream()
{
   //
}

const char* Stream::getStatusString(const Status in_status)
{
   switch (in_status) {
      case Ok:
         return "StreamOk";
      case IOError:
         return "StreamIOError";
      case EOS:
         return "StreamEOS";
      case IllegalCall:
         return "StreamIllegalCall";
      case Closed:
         return "StreamClosed";
      case UnknownError:
         return "StreamUnknownError";

     default:
      return "Invalid Stream::Status";
   }
}

void Stream::writeString(const char *string, S32 maxLen)
{
   S32 len = string ? dStrlen(string) : 0;
   if(len > maxLen)
      len = maxLen;

   write(U8(len));
   if(len)
      write(len, string);
}

void Stream::readString(char buf[256])
{
   U8 len;
   read(&len);
   read(S32(len), buf);
   buf[len] = 0;
}

const char *Stream::readSTString(bool casesens)
{
   char buf[256];
   readString(buf);
   return StringTable->insert(buf, casesens);
}

void Stream::readLongString(U32 maxStringLen, char *stringBuf)
{
   U32 len;
   read(&len);
   if(len > maxStringLen)
   {
      m_streamStatus = IOError;
      return;
   }
   read(len, stringBuf);
   stringBuf[len] = 0;
}

void Stream::writeLongString(U32 maxStringLen, const char *string)
{
   U32 len = dStrlen(string);
   if(len > maxStringLen)
      len = maxStringLen;
   write(len);
   write(len, string);
}

void Stream::readLine(U8 *buffer, U32 bufferSize)
{
   bufferSize--;  // account for NULL terminator
   U8 *buff = buffer;
   U8 *buffEnd = buff + bufferSize;
   *buff = '\r';
   
   // strip off preceding white space   
   while ( *buff == '\r' )
    if ( !read(buff) || *buff == '\n' )
    {
       *buff = 0;
       return;
    }
 
   // read line   
   while ( buff != buffEnd && read(++buff) && *buff != '\n' )
    if ( *buff == '\r' )
       buff--;
   *buff = 0;   
}

void Stream::writeLine(U8 *buffer)
{
   write(dStrlen((StringTableEntry)buffer), buffer);
   write(2, "\r\n");
}

bool Stream::write(const ColorI& rColor)
{
   bool success = write(rColor.red);
   success     |= write(rColor.green);
   success     |= write(rColor.blue);
   success     |= write(rColor.alpha);

   return success;
}

bool Stream::write(const ColorF& rColor)
{
   ColorI temp = rColor;
   return write(temp);
}

bool Stream::read(ColorI* pColor)
{
   bool success = read(&pColor->red);
   success     |= read(&pColor->green);
   success     |= read(&pColor->blue);
   success     |= read(&pColor->alpha);

   return success;
}

bool Stream::read(ColorF* pColor)
{
   ColorI temp;
   bool success = read(&temp);

   *pColor = temp;
   return success;
}

