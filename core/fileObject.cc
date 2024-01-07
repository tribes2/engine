//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Core/fileObject.h"

IMPLEMENT_CONOBJECT(FileObject);

bool FileObject::isEOF()
{
   return mCurPos == mBufferSize;
}

FileObject::FileObject()
{
   mFileBuffer = NULL;
   mBufferSize = 0;
   mCurPos = 0;
}

FileObject::~FileObject()
{
   dFree(mFileBuffer);
}

void FileObject::close()
{
   stream.close();
   dFree(mFileBuffer);
   mFileBuffer = NULL;
   mBufferSize = mCurPos = 0;
}

bool FileObject::openForWrite(const char *fileName, const bool append)
{
   close();
   if ( !append )
      return( ResourceManager->openFileForWrite(stream, NULL, fileName) );

   // Use the WriteAppend flag so it doesn't clobber the existing file:
   if ( !ResourceManager->openFileForWrite(stream, NULL, fileName, File::WriteAppend) )
      return( false );

   stream.setPosition( stream.getStreamSize() );
   return( true );
}

bool FileObject::openForRead(const char* /*fileName*/)
{
   AssertFatal(false, "Error, not yet implemented!");
   return false;
}

bool FileObject::readMemory(const char *fileName)
{
   close();
   Stream *s = ResourceManager->openStream(fileName);
   if(!s)
      return false;
   mBufferSize = ResourceManager->getSize(fileName);
   mFileBuffer = (U8 *) dMalloc(mBufferSize + 1);
   mFileBuffer[mBufferSize] = 0;
   s->read(mBufferSize, mFileBuffer);
   ResourceManager->closeStream(s);
   mCurPos = 0;
   
   return true;
}

const U8 *FileObject::readLine()
{
   if(!mFileBuffer)
      return (U8 *) "";
   U32 tokPos = mCurPos;
   for(;;)
   {
      if(mCurPos == mBufferSize)
         break;
      if(mFileBuffer[mCurPos] == '\r')
      {
         mFileBuffer[mCurPos++] = 0;
         if(mFileBuffer[mCurPos] == '\n')
            mCurPos++;
         break;
      }
      if(mFileBuffer[mCurPos] == '\n')
      {
         mFileBuffer[mCurPos++] = 0;
         break;
      }
      mCurPos++;
   }
   return mFileBuffer + tokPos;
}

void FileObject::writeLine(const U8 *line)
{
   stream.write(dStrlen((const char *) line), line);
   stream.write(2, "\r\n");
}

static bool cFileOpenRead(SimObject *obj, S32, const char **argv)
{
   FileObject *fo = (FileObject *) obj;
   return fo->readMemory(argv[2]);
}

static bool cFileOpenWrite(SimObject *obj, S32, const char **argv)
{
   FileObject *fo = (FileObject *) obj;
   return fo->openForWrite(argv[2]);
}

static bool cFileOpenAppend(SimObject *obj, S32, const char **argv)
{
   FileObject *fo = (FileObject *) obj;
   return fo->openForWrite(argv[2], true);
}

static bool cFileIsEOF(SimObject *obj, S32, const char **)
{
   FileObject *fo = (FileObject *) obj;
   return fo->isEOF();
}

static const char * cFileReadLine(SimObject *obj, S32, const char **)
{
   FileObject *fo = (FileObject *) obj;
   return (const char *) fo->readLine();
}

static void cFileWriteLine(SimObject *obj, S32, const char **argv)
{
   FileObject *fo = (FileObject *) obj;
   fo->writeLine((const U8 *) argv[2]);
}

static void cFileClose(SimObject *obj, S32, const char **)
{
   FileObject *fo = (FileObject *) obj;
   fo->close();
}

void FileObject::consoleInit()
{
   Con::addCommand("FileObject", "openForRead", cFileOpenRead, "file.openForRead(fileName)", 3, 3);
   Con::addCommand("FileObject", "openForWrite", cFileOpenWrite, "file.openForWrite(fileName", 3, 3);
   Con::addCommand("FileObject", "openForAppend", cFileOpenAppend, "file.openForAppend(fileName)", 3, 3);
   Con::addCommand("FileObject", "writeLine", cFileWriteLine, "file.writeLine(text)", 3, 3);
   Con::addCommand("FileObject", "isEOF", cFileIsEOF, "file.isEOF()", 2, 2);
   Con::addCommand("FileObject", "readLine", cFileReadLine, "file.readLine()", 2, 2);
   Con::addCommand("FileObject", "close", cFileClose, "file.close()", 2, 2);
}
