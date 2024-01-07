//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "Core/stream.h"
#include "Core/zipHeaders.h"

const U32 ZipLocalFileHeader::csm_localFileHeaderSig = 0x04034b50;
const U32 ZipDirFileHeader::csm_dirFileHeaderSig     = 0x02014b50;
const U32 ZipEOCDRecord::csm_eocdRecordSig           = 0x06054b50;

bool
ZipLocalFileHeader::readFromStream(Stream& io_rStream)
{
   AssertFatal(io_rStream.getStatus() == Stream::Ok,
               "Error, stream is closed or has an uncleared error.");
   AssertFatal(io_rStream.hasCapability(Stream::StreamPosition),
               "Must be positionable stream to read zip headers...");

   // Read the initial header fields, marking the initial position...
   //
   U32 initialPosition = io_rStream.getPosition();
   bool success = io_rStream.read(sizeof(m_header), &m_header);

   if (success == false || m_header.headerSig != csm_localFileHeaderSig)  {
      AssertWarn(0, "Unable to retrieve local file header from stream position...");
      io_rStream.setPosition(initialPosition);
      return false;
   }

   // Read the variable length file name from the stream...
   //
   AssertFatal(m_header.fileNameLength < (MaxFileNameLength - 1),
               "Filename too long, increase structure size");
   success = io_rStream.read(m_header.fileNameLength, m_pFileName);
   m_pFileName[m_header.fileNameLength] = '\0';
   if (success == false) {
      AssertWarn(0, "Unable to read file name from stream position...");
      io_rStream.setPosition(initialPosition);
      return false;
   }


   // And seek to the end of the header, ignoring the extra field.
   io_rStream.setPosition(initialPosition +
                          (sizeof(m_header)        +
                           m_header.fileNameLength +
                           m_header.extraFieldLength));
   return true;
}

bool
ZipDirFileHeader::readFromStream(Stream& io_rStream)
{
   AssertFatal(io_rStream.getStatus() == Stream::Ok,
               "Error, stream is closed or has an uncleared error.");
   AssertFatal(io_rStream.hasCapability(Stream::StreamPosition),
               "Must be positionable stream to read zip headers...");

   // Read the initial header fields, marking the initial position...
   //
   U32 initialPosition = io_rStream.getPosition();
   bool success = io_rStream.read(sizeof(m_header), &m_header);

   if (success == false || m_header.headerSig != csm_dirFileHeaderSig)  {
      AssertWarn(0, "Unable to retrieve local file header from stream position...");
      io_rStream.setPosition(initialPosition);
      return false;
   }

   // Read the variable length file name from the stream...
   //
   AssertFatal(m_header.fileNameLength < (MaxFileNameLength - 1),
               "Filename too long, increase structure size");
   success = io_rStream.read(m_header.fileNameLength, m_pFileName);
   m_pFileName[m_header.fileNameLength] = '\0';
   if (success == false) {
      AssertWarn(0, "Unable to read file name from stream position...");
      io_rStream.setPosition(initialPosition);
      return false;
   }


   // And seek to the end of the header, ignoring the extra field.
   io_rStream.setPosition(initialPosition +
                          (sizeof(m_header)        +
                           m_header.fileNameLength +
                           m_header.extraFieldLength));
   return true;
}

bool
ZipEOCDRecord::readFromStream(Stream& io_rStream)
{
   AssertFatal(io_rStream.getStatus() == Stream::Ok,
               "Error, stream is closed or has an uncleared error.");
   AssertFatal(io_rStream.hasCapability(Stream::StreamPosition),
               "Must be positionable stream to read zip headers...");

   // Read the initial header fields, marking the initial position...
   //
   U32 initialPosition = io_rStream.getPosition();
   bool success = io_rStream.read(sizeof(m_record), &m_record);

   if (success == false || m_record.eocdSig != csm_eocdRecordSig)  {
      AssertWarn(0, "Unable to retrieve EOCD header from stream position...");
      io_rStream.setPosition(initialPosition);
      return false;
   }

   // And seek to the end of the header, ignoring the extra field.
   io_rStream.setPosition(initialPosition +
                          (sizeof(m_record) + m_record.zipFileCommentLength));
   return true;
}

