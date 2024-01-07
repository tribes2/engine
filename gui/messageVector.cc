//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/messageVector.h"
#include "Core/fileObject.h"

IMPLEMENT_CONOBJECT(MessageVector);

//-------------------------------------- Console functions
namespace {

void cMVClear(SimObject *obj, S32, const char **)
{
   MessageVector* pMV = static_cast<MessageVector*>(obj);
   
   pMV->clear();
}

void cMVPushBackLine(SimObject* obj, S32 argc, const char** argv)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   U32 tag = 0;
   if (argc == 4)
      tag = dAtoi(argv[3]);

   pMV->pushBackLine(argv[2], tag);
}

bool cMVPopBackLine(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   if (pMV->getNumLines() == 0) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::popBackLine(): underflow");
      return false;
   }

   pMV->popBackLine();
   return true;
}

void cMVPushFrontLine(SimObject* obj, S32 argc, const char** argv)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   U32 tag = 0;
   if (argc == 4)
      tag = dAtoi(argv[3]);

   pMV->pushFrontLine(argv[2], tag);
}

bool cMVPopFrontLine(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   if (pMV->getNumLines() == 0) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::popFrontLine(): underflow");
      return false;
   }

   pMV->popFrontLine();
   return true;
}

bool cMVInsertLine(SimObject* obj, S32 argc, const char** argv)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   U32 pos = U32(dAtoi(argv[2]));
   if (pos > pMV->getNumLines())
      return false;

   S32 tag = 0;
   if (argc == 5)
      tag = dAtoi(argv[4]);

   pMV->insertLine(pos, argv[3], tag);
   return true;
}

bool cMVDeleteLine(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   U32 pos = U32(dAtoi(argv[2]));
   if (pos >= pMV->getNumLines())
      return false;

   pMV->deleteLine(pos);
   return true;
}

void cMVDump(SimObject* obj, S32 argc, const char** argv)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);
   if ( argc == 4 )
      pMV->dump( argv[2], argv[3] );
   else
      pMV->dump( argv[2] );
}

S32 cMVGetNumLines(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   return pMV->getNumLines();
}

const char *cMVGetLineTextByTag(SimObject *obj, S32, const char **argv)
{
   MessageVector* pMV = static_cast<MessageVector*>(obj);
   U32 tag = dAtoi(argv[2]);
   
   for(U32 i = 0; i < pMV->getNumLines(); i++)
      if(pMV->getLine(i).messageTag == tag)
         return pMV->getLine(i).message;
   return "";
}

S32 cMVGetLineIndexByTag(SimObject *obj, S32, const char **argv)
{
   MessageVector* pMV = static_cast<MessageVector*>(obj);
   U32 tag = dAtoi(argv[2]);
   
   for(U32 i = 0; i < pMV->getNumLines(); i++)
      if(pMV->getLine(i).messageTag == tag)
         return i;
   return -1;
}

const char* cMVGetLineText(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   U32 pos = U32(dAtoi(argv[2]));
   if (pos >= pMV->getNumLines()) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::getLineText(con): out of bounds line");
      return "";
   }

   return pMV->getLine(pos).message;
}

S32 cMVGetLineTag(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<MessageVector*>(obj) != NULL, "Error, how did a non-MessageVector get here?");
   MessageVector* pMV = static_cast<MessageVector*>(obj);

   U32 pos = U32(dAtoi(argv[2]));
   if (pos >= pMV->getNumLines()) {
      Con::errorf(ConsoleLogEntry::General, "MessageVector::getLineTag(con): out of bounds line");
      return 0;
   }

   return pMV->getLine(pos).messageTag;
}

} // namespace {}


//--------------------------------------------------------------------------
MessageVector::MessageVector()
{
   VECTOR_SET_ASSOCIATION(mMessageLines);
   VECTOR_SET_ASSOCIATION(mSpectators);
}


//--------------------------------------------------------------------------
MessageVector::~MessageVector()
{
   for (U32 i = 0; i < mMessageLines.size(); i++) {
      char* pDelete = const_cast<char*>(mMessageLines[i].message);
      delete [] pDelete;
      mMessageLines[i].message = 0;
      mMessageLines[i].messageTag = 0xFFFFFFFF;
   }
   mMessageLines.clear();
}


//--------------------------------------------------------------------------
void MessageVector::initPersistFields()
{
   Parent::initPersistFields();
}


//--------------------------------------------------------------------------
void MessageVector::consoleInit()
{
   Con::addCommand("MessageVector", "pushBackLine",  cMVPushBackLine,  "[MessageVector].pushBackLine(\"Message\"[, Tag=0])",  3, 4);
   Con::addCommand("MessageVector", "popBackLine",   cMVPopBackLine,   "[MessageVector].popBackLine()",                       2, 2);
   Con::addCommand("MessageVector", "pushFrontLine", cMVPushFrontLine, "[MessageVector].pushFrontLine(\"Message\"[, Tag=0])", 3, 4);
   Con::addCommand("MessageVector", "popFrontLine",  cMVPopFrontLine,  "[MessageVector].popFrontLine()",                      2, 2);

   Con::addCommand("MessageVector", "insertLine", cMVInsertLine, "[MessageVector].insertLine(InsertPos, \"Message\"[, Tag=0])", 4, 5);
   Con::addCommand("MessageVector", "deleteLine", cMVDeleteLine, "[MessageVector].deleteLine(DeletePos)",                       3, 3);
   Con::addCommand("MessageVector", "clear", cMVClear, "[MessageVector].clear()", 2, 2);

   Con::addCommand("MessageVector", "dump", cMVDump, "[MessageVector].dump(filename{, header})", 3, 4);

   Con::addCommand("MessageVector", "getNumLines", cMVGetNumLines, "[MessageVector].getNumLines()",     2, 2);
   Con::addCommand("MessageVector", "getLineText", cMVGetLineText, "[MessageVector].getLineText(Line)", 3, 3);
   Con::addCommand("MessageVector", "getLineTag",  cMVGetLineTag,  "[MessageVector].getLineTag(Line)",  3, 3);
   Con::addCommand("MessageVector", "getLineTextByTag", cMVGetLineTextByTag, "[MessageVector].getLineTextByTag(Tag)", 3, 3);
   Con::addCommand("MessageVector", "getLineIndexByTag", cMVGetLineIndexByTag, "[MessageVector].getLineIndexByTag(Tag)", 3, 3);
}


//--------------------------------------------------------------------------
bool MessageVector::onAdd()
{
   return Parent::onAdd();
}


//--------------------------------------------------------------------------
void MessageVector::onRemove()
{
   // Delete all the lines from the observers, and then forcibly detatch ourselves
   //
   for (S32 i = mMessageLines.size() - 1; i >= 0; i--)
      spectatorMessage(LineDeleted, i);
   spectatorMessage(VectorDeletion, 0);
   mSpectators.clear();

   Parent::onRemove();
}


//--------------------------------------------------------------------------
void MessageVector::pushBackLine(const char* newMessage, const S32 newMessageTag)
{
   insertLine(mMessageLines.size(), newMessage, newMessageTag);
}


void MessageVector::popBackLine()
{
   AssertFatal(mMessageLines.size() != 0, "MessageVector::popBackLine: nothing to pop!");
   if (mMessageLines.size() == 0)
      return;

   deleteLine(mMessageLines.size() - 1);
}

void MessageVector::clear()
{
   while(mMessageLines.size())
      deleteLine(mMessageLines.size() - 1);
}

//--------------------------------------------------------------------------
void MessageVector::pushFrontLine(const char* newMessage, const S32 newMessageTag)
{
   insertLine(0, newMessage, newMessageTag);
}


void MessageVector::popFrontLine()
{
   AssertFatal(mMessageLines.size() != 0, "MessageVector::popBackLine: nothing to pop!");
   if (mMessageLines.size() == 0)
      return;

   deleteLine(0);
}


//--------------------------------------------------------------------------
void MessageVector::insertLine(const U32   position,
                               const char* newMessage,
                               const S32   newMessageTag)
{
   AssertFatal(position >= 0 && position <= mMessageLines.size(), "MessageVector::insertLine: out of range position!");
   AssertFatal(newMessage != NULL, "Error, no message to insert!");

   U32 len = dStrlen(newMessage) + 1;
   char* copy = new char[len];
   dStrcpy(copy, newMessage);

   mMessageLines.insert(position);
   mMessageLines[position].message    = copy;
   mMessageLines[position].messageTag = newMessageTag;

   // Notify of insert
   spectatorMessage(LineInserted, position);
}


//--------------------------------------------------------------------------
void MessageVector::deleteLine(const U32 position)
{
   AssertFatal(position >= 0 && position < mMessageLines.size(), "MessageVector::deleteLine: out of range position!");
   
   char* pDelete = const_cast<char*>(mMessageLines[position].message);
   delete [] pDelete;
   mMessageLines[position].message    = NULL;
   mMessageLines[position].messageTag = 0xFFFFFFFF;

   mMessageLines.erase(position);

   // Notify of delete
   spectatorMessage(LineDeleted, position);
}


//--------------------------------------------------------------------------
bool MessageVector::dump( const char* filename, const char* header )
{
   Con::printf( "Dumping message vector %s to %s...", getName(), filename );
   FileObject file;
   if ( !file.openForWrite( filename ) )
      return( false );

   // If passed a header line, write it out first:
   if ( header )
      file.writeLine( (const U8*) header );

   // First write out the record count:
   char* lineBuf = (char*) dMalloc( 10 );
   dSprintf( lineBuf, 10, "%d", mMessageLines.size() );
   file.writeLine( (const U8*) lineBuf );

   // Write all of the lines of the message vector:
   U32 len;
   for ( U32 i = 0; i < mMessageLines.size(); i++ )
   {
      len = ( dStrlen( mMessageLines[i].message ) * 2 ) + 10;
      lineBuf = (char*) dRealloc( lineBuf, len );
      dSprintf( lineBuf, len, "%d ", mMessageLines[i].messageTag );
      expandEscape( lineBuf + dStrlen( lineBuf ), mMessageLines[i].message );
      file.writeLine( (const U8*) lineBuf );
   }

   file.close();
   return( true );
}


//--------------------------------------------------------------------------
void MessageVector::registerSpectator(SpectatorCallback callBack, U32 spectatorKey)
{
   AssertFatal(callBack != NULL, "Error, must have a callback!");

   // First, make sure that this hasn't been registered already...
   U32 i;
   for (i = 0; i < mSpectators.size(); i++) {
      AssertFatal(mSpectators[i].key != spectatorKey, "Error, spectator key already registered!");
      if (mSpectators[i].key == spectatorKey)
         return;
   }

   mSpectators.increment();
   mSpectators.last().callback = callBack;
   mSpectators.last().key      = spectatorKey;

   // Need to message this spectator of all the lines currently inserted...
   for (i = 0; i < mMessageLines.size(); i++) {
      (*mSpectators.last().callback)(mSpectators.last().key,
                                     LineInserted, i);
   }
}

void MessageVector::unregisterSpectator(U32 spectatorKey)
{
   for (U32 i = 0; i < mSpectators.size(); i++) {
      if (mSpectators[i].key == spectatorKey) {
         // Need to message this spectator of all the lines currently inserted...
         for (S32 j = mMessageLines.size() - 1; j >= 0 ; j--) {
            (*mSpectators[i].callback)(mSpectators[i].key,
                                       LineDeleted, j);
         }

         mSpectators.erase(i);
         return;
      }
   }

   AssertFatal(false, "MessageVector::unregisterSpectator: tried to unregister a spectator that isn't subscribed!");
   Con::errorf(ConsoleLogEntry::General, 
               "MessageVector::unregisterSpectator: tried to unregister a spectator that isn't subscribed!");
}

void MessageVector::spectatorMessage(MessageCode code, const U32 arg)
{
   for (U32 i = 0; i < mSpectators.size(); i++) {
      (*mSpectators[i].callback)(mSpectators[i].key,
                                 code, arg);
   }
}

