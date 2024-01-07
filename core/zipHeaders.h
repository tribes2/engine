//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _ZIPHEADERS_H_
#define _ZIPHEADERS_H_

//Includes
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

//-------------------------------------- NB: Structures in this header are BYTE
//                                        aligned!
#ifdef __BORLANDC__
#  pragma option -a1
#endif

#ifdef _MSC_VER
#  pragma pack(push,1)
#endif

#ifdef __MWERKS__
#  pragma options align=packed
#endif


class Stream;

//-------------------------------------- Structure designed to fit exactly 256 bytes.
class ZipLocalFileHeader
{
   // NB: Extra field in the header is ignored, but the stream read seeks
   //      past it...
   //
  private:
   static const U32 csm_localFileHeaderSig;

  public:
   enum {
      MaxFileNameLength = 211
   };
   enum CompressionMethod {
     Stored            = 0,
     Shrunk            = 1,
     ReducedL1         = 2,
     ReducedL2         = 3,
     ReducedL3         = 4,
     ReducedL4         = 5,
     Imploded          = 6,
     ReservedTokenized = 7,
     Deflated          = 8,
     EnhDefalted       = 9,
     DateCompression   = 10
   };

   struct LocalFileHeader {
      U32 headerSig;
      U16 versionToDecompress;
      U16 bitFlags;
      U16 compressionMethod;
      U16 lastModTime;
      U16 lastModDate;
      U32 crc32;
      U32 compressedSize;
      U32 uncompressedSize;

      U16 fileNameLength;
      U16 extraFieldLength;
#ifndef linux
   };
#else
   } __attribute__ ((packed));
#endif

   LocalFileHeader m_header;         // Fixed size header
   char            m_pFileName[226]; // Variable size: FileName.  Note that the
                                     //  number of chars here is more than the
                                     //  max allowed filename for alignment
                                     //  purposes
                        
   // Stream read routines
  public:
   bool readFromStream(Stream& io_rStream);
#ifndef linux
};
#else
} __attribute__ ((packed));
#endif


//-------------------------------------- Also designed to fit into 256 bytes, note
//                                        that we ignore the extra and file comment
//                                        fields.
class ZipDirFileHeader
{
  private:
   static const U32 csm_dirFileHeaderSig;

  public:
   enum {
      MaxFileNameLength = 211
   };
   enum CompressionMethod {
     Stored            = 0,
     Shrunk            = 1,
     ReducedL1         = 2,
     ReducedL2         = 3,
     ReducedL3         = 4,
     ReducedL4         = 5,
     Imploded          = 6,
     ReservedTokenized = 7,
     Deflated          = 8,
     EnhDefalted       = 9,
     DateCompression   = 10
   };

   struct DirFileHeader {
      U32   headerSig;
      U16   versionMadeBy;
      U16   versionToDecompress;
      U16   bitFlags;
      U16   compressionMethod;
      U16   lastModTime;
      U16   lastModDate;
      U32   crc32;
      U32   compressedSize;
      U32   uncompressedSize;
      U16   fileNameLength;
      U16   extraFieldLength;
      U16   fileCommentLength;
      U16   diskNumberStart;
      U16   internalFileAttributes;
      U32   externalFileAttributes;
      U32   relativeOffsetOfLocalHeader;
#ifndef linux
   };
#else
   } __attribute__ ((packed));
#endif

   DirFileHeader  m_header;
   char           m_pFileName[212];
                                   
   // Stream read routines
  public:
   bool readFromStream(Stream& io_rStream);
#ifndef linux
};
#else
} __attribute__ ((packed));
#endif


//-------------------------------------- Padded to 32 bytes.  Note that we completely
//                                        ignore any zip file comments.
class ZipEOCDRecord
{
  private:
   static const U32 csm_eocdRecordSig;

  public:
   enum {
      ProperRecordSize = 22
   };

   struct EOCDRecord {
      U32   eocdSig;
      U16   diskNumber;
      U16   eocdDiskNumber;
      U16   numCDEntriesDisk;
      U16   numCDEntriesTotal;
      U32   cdSize;
      U32   cdOffset;
      U16   zipFileCommentLength;
#ifndef linux
   };
#else
   } __attribute__ ((packed));
#endif

   EOCDRecord m_record;
   char       __padding[10];
   // Stream read routines
  public:
   bool readFromStream(Stream& io_rStream);
#ifndef linux
};
#else
} __attribute__ ((packed));
#endif



#ifdef __BORLANDC__
#  pragma option -a.
#endif

#ifdef _MSC_VER
#  pragma pack(pop)
#endif

#ifdef __MWERKS__
#  pragma options align=reset
#endif


#endif //_NZIPHEADERS_H_
