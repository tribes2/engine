//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Core/stream.h"
#include "Core/fileStream.h"
#include "Core/memstream.h"
#include "dgl/gPalette.h"
#include "dgl/gBitmap.h"

#include "jpeglib.h"


//-------------------------------------- Replacement I/O for standard LIBjpeg
//                                        functions.  we don't wanna use
//                                        FILE*'s...
static int jpegReadDataFn(void *client_data, unsigned char *data, int length)
{
   Stream *stream = (Stream*)client_data;
   AssertFatal(stream != NULL, "jpegReadDataFn::No stream.");
   int pos = stream->getPosition();
   if (stream->read(length, data))
      return length;

   if (stream->getStatus() == Stream::EOS)
      return (stream->getPosition()-pos);
   else
      return 0;
}


//--------------------------------------
static int jpegWriteDataFn(void *client_data, unsigned char *data, int length)
{
   Stream *stream = (Stream*)client_data;
   AssertFatal(stream != NULL, "jpegWriteDataFn::No stream.");
   if (stream->write(length, data))
      return length;
   else
      return 0;
}


//--------------------------------------
static void jpegFlushDataFn(void *)
{
   //
}


//--------------------------------------
static int jpegErrorFn(void *client_data)
{
   Stream *stream = (Stream*)client_data;
   AssertFatal(stream != NULL, "jpegErrorFn::No stream.");
   return (stream->getStatus() != Stream::Ok);      
}   


//--------------------------------------
bool GBitmap::readJPEG(Stream &stream)
{
   JFREAD  = jpegReadDataFn;
   JFERROR = jpegErrorFn;

   jpeg_decompress_struct cinfo;
   jpeg_error_mgr jerr;

   // We set up the normal JPEG error routines, then override error_exit.
   //cinfo.err = jpeg_std_error(&jerr.pub);
   //jerr.pub.error_exit = my_error_exit;

   // if (setjmp(jerr.setjmp_buffer)) 
   // {
   //    // If we get here, the JPEG code has signaled an error.
   //    // We need to clean up the JPEG object, close the input file, and return.
   //    jpeg_destroy_decompress(&cinfo);
   //    return false;
   // }

   
   cinfo.err = jpeg_std_error(&jerr);    // set up the normal JPEG error routines.
   cinfo.client_data = (void*)&stream;       // set the stream into the client_data

   // Now we can initialize the JPEG decompression object.
   jpeg_create_decompress(&cinfo);

   jpeg_stdio_src(&cinfo);

   // Read file header, set default decompression parameters
   jpeg_read_header(&cinfo, true);

   BitmapFormat format;
   switch (cinfo.out_color_space)
   {
      case JCS_GRAYSCALE:  format = Alpha; break;
      case JCS_RGB:        format = RGB;   break;
      default:
         jpeg_destroy_decompress(&cinfo);
         return false;
   }

   // Start decompressor
   jpeg_start_decompress(&cinfo);

   // allocate the bitmap space and init internal variables...
   allocateBitmap(cinfo.output_width, cinfo.output_height, false, format);

   // Set up the row pointers...
   U32 rowBytes = cinfo.output_width * cinfo.output_components;

   U8* pBase = (U8*)getBits();
   for (U32 i = 0; i < height; i++)
   {
      JSAMPROW rowPointer = pBase + (i * rowBytes);
      jpeg_read_scanlines(&cinfo, &rowPointer, 1);
   }

   // Finish decompression
   jpeg_finish_decompress(&cinfo);

   // Release JPEG decompression object
   // This is an important step since it will release a good deal of memory.
   jpeg_destroy_decompress(&cinfo);
   
   return true;
}


//--------------------------------------------------------------------------
bool GBitmap::writeJPEG(Stream&) const
{
   return false;
/*
   if (compressHard == false) {
      return _writePNG(stream, 6, 0, PNG_ALL_FILTERS);
   } else {
      U8* buffer = new U8[1 << 22]; // 4 Megs.  Should be enough...
      MemStream* pMemStream = new MemStream(1 << 22, buffer, false, true);

      // We have to try the potentially useful compression methods here.

      const U32 zStrategies[] = { Z_DEFAULT_STRATEGY,
                                  Z_FILTERED };
      const U32 pngFilters[]  = { PNG_FILTER_NONE,
                                  PNG_FILTER_SUB,
                                  PNG_FILTER_UP,
                                  PNG_FILTER_AVG,
                                  PNG_FILTER_PAETH,
                                  PNG_ALL_FILTERS };

      U32 minSize      = 0xFFFFFFFF;
      U32 bestStrategy = 0xFFFFFFFF;
      U32 bestFilter   = 0xFFFFFFFF;
      U32 bestCLevel   = 0xFFFFFFFF;

      for (U32 cl = 0; cl <=9; cl++) {
         for (U32 zs = 0; zs < 2; zs++) {
            for (U32 pf = 0; pf < 6; pf++) {
               pMemStream->setPosition(0);

               if (_writePNG(*pMemStream, cl, zStrategies[zs], pngFilters[pf]) == false)
                  AssertFatal(false, "Handle this error!");

               if (pMemStream->getPosition() < minSize) {
                  minSize = pMemStream->getPosition();
                  bestStrategy = zs;
                  bestFilter   = pf;
                  bestCLevel   = cl;
               }
            }
         }
      }
      AssertFatal(minSize != 0xFFFFFFFF, "Error, no best found?");

      delete pMemStream;
      delete [] buffer;


      return _writePNG(stream,
                       bestCLevel,
                       zStrategies[bestStrategy],
                       pngFilters[bestFilter]);
   }
*/
}
