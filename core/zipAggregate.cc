//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h" 
#include "Core/stringTable.h"

#include "Core/fileStream.h"       // Streams

#include "Core/zipAggregate.h"     // Own header, and private includes
#include "Core/zipHeaders.h"

ZipAggregate::ZipAggregate()
 : m_pZipFileName(NULL)
{
   VECTOR_SET_ASSOCIATION(m_fileList);
}

ZipAggregate::~ZipAggregate()
{
   closeAggregate();
}

bool
ZipAggregate::refreshAggregate()
{
   AssertFatal(m_pZipFileName != NULL, "No filename?  Must not be open.  Disallowed");

   char tmpBuff[512];
   dStrcpy(tmpBuff, m_pZipFileName);

   return openAggregate(tmpBuff);
}

bool
ZipAggregate::openAggregate(const char* in_pFileName)
{
   closeAggregate();

   AssertFatal(in_pFileName != NULL, "No filename to open!");

   m_pZipFileName = new char[dStrlen(in_pFileName) + 1];
   dStrcpy(m_pZipFileName, in_pFileName);

   FileStream* pStream = new FileStream;
   if (pStream->open(m_pZipFileName, FileStream::Read) == false ||
       createZipDirectory(pStream)   == false) {
      // Failure, abort the open...
      //
      delete pStream;

      delete [] m_pZipFileName;
      m_pZipFileName = NULL;
      return false;
   }
   
   // Finished!  Open for business
   delete pStream;
   return true;
}

void
ZipAggregate::closeAggregate()
{
   destroyZipDirectory();

   delete [] m_pZipFileName;
   m_pZipFileName = NULL;
}

void
ZipAggregate::destroyZipDirectory()
{
   m_fileList.clear();
}

bool
ZipAggregate::createZipDirectory(Stream* io_pStream)
{
   AssertFatal(io_pStream != NULL,      "Error, stream not open.");

   U32 streamSize      = io_pStream->getStreamSize();
   U32 initialPosition = io_pStream->getPosition();

   // We assume that the CD is 22 bytes from the end.  This will be invalid
   //  in the case that the zip file has comments.  Perhaps test the quick
   //  way, then degrade to seaching the final 64k+22b (!) of the stream?
   //
   bool posSuccess = io_pStream->setPosition(streamSize - sizeof(ZipEOCDRecord::EOCDRecord));
   if (posSuccess == false) {
      AssertWarn(false, "Unable to position stream to start of EOCDRecord");
      return false;
   }

   ZipEOCDRecord* pEOCDRecord = new ZipEOCDRecord;
   if (pEOCDRecord->readFromStream(*io_pStream) == false) {
      // This is where we would try to degrade to general case...
      //
      AssertWarn(false, "Unable to locate central directory.  "
                        "Zip File might have comments");
      delete pEOCDRecord;
      return false;
   }

   // Check the consistency of the zipFile.
   //
   if ((pEOCDRecord->m_record.diskNumber       != pEOCDRecord->m_record.eocdDiskNumber)    ||
       (pEOCDRecord->m_record.numCDEntriesDisk != pEOCDRecord->m_record.numCDEntriesTotal)) {
      AssertWarn(false, "Zipfile appears to be part of a "
                        "multi-zip disk span set, unsupported");
      delete pEOCDRecord;
      return false;
   }

   // If we're here, we're good!  Scan to the start of the CDirectory, and
   //  start scanning the entries into our directory structure...
   //
   U32 startCDPosition = pEOCDRecord->m_record.cdOffset;
   U32 endCDPosition   = pEOCDRecord->m_record.cdOffset +
                            pEOCDRecord->m_record.cdSize;

   posSuccess = io_pStream->setPosition(startCDPosition);
   if (posSuccess == false) {
      AssertWarn(false, "Unable to position to CD entries.");
      delete pEOCDRecord;
      return false;
   }

   bool dirReadSuccess = true;
   for (U16 i = 0; i < pEOCDRecord->m_record.numCDEntriesTotal; i++) {
      ZipDirFileHeader zdfHeader;

      bool hrSuccess = zdfHeader.readFromStream(*io_pStream);
      if (hrSuccess == false) {
         AssertWarn(false, "Error reading a CD Entry in zip aggregate");
         dirReadSuccess = false;
         break;
      }

      enterZipDirRecord(zdfHeader);
   }

   delete pEOCDRecord;
   if (dirReadSuccess == true) {
      // Every thing went well, we're done, position the stream to the end of the
      //  CD...
      //
      io_pStream->setPosition(endCDPosition);
      return true;
   } else {
      // Oh, crap.
      io_pStream->setPosition(initialPosition);
      destroyZipDirectory();
      return false;
   }
}

void
ZipAggregate::enterZipDirRecord(const ZipDirFileHeader& in_rHeader)
{
   // Ok, the first thing to do is figure out whether this is
   //  a directory or a file.  Directories have a trailing /
   //  in the file name, and a filelength (comp/uncomp) of 0
   // Note: this is not specified in
   //  the file format spec I have, but seems fairly likely to
   //  be correct.  
   //
   if (in_rHeader.m_pFileName[dStrlen(in_rHeader.m_pFileName) - 1] == '/' &&
       (in_rHeader.m_header.compressedSize   == 0 &&
        in_rHeader.m_header.uncompressedSize == 0))
      return;

   // It's a file.  Enter it into the directory...
   m_fileList.increment();
   FileEntry& rEntry = m_fileList.last();

   char tempString[1024];
   dStrcpy(tempString, in_rHeader.m_pFileName);
   char* scan = tempString;
   while (*scan != '\0') {
      if (*scan == '\\')
         *scan = '/';
      scan++;
   }
   char* pPathEnd = dStrrchr(tempString, '/');
   if (pPathEnd != NULL) {
      pPathEnd[0] = '\0';
      rEntry.pPath     = StringTable->insert(tempString);
      rEntry.pFileName = StringTable->insert(pPathEnd + 1);
   } else {
      rEntry.pPath     = NULL;
      rEntry.pFileName = StringTable->insert(in_rHeader.m_pFileName);
   }

   rEntry.fileSize           = in_rHeader.m_header.uncompressedSize;
   rEntry.compressedFileSize = in_rHeader.m_header.compressedSize;
   rEntry.fileOffset         = in_rHeader.m_header.relativeOffsetOfLocalHeader;

   if (in_rHeader.m_header.compressionMethod == ZipDirFileHeader::Deflated) {
      rEntry.flags = FileEntry::Compressed;
   } else if (in_rHeader.m_header.compressionMethod == ZipDirFileHeader::Stored) {
      rEntry.flags = FileEntry::Uncompressed;
   } else {
      AssertWarn(0, avar("Warning, non-stored or deflated resource in %s",
                         m_pZipFileName));
      m_fileList.decrement();
   }
}

