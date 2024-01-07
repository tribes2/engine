//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/ast.h"
#include "core/resManager.h"
#include "core/fileStream.h"
#include "console/compiler.h"
#include "platform/event.h"
#include "platform/gameInterface.h"

//----------------------------------------------------------------

ConsoleFunction(strcmp, S32, 3, 3, "strcmp(one, two)")
{
   argc;
   return dStrcmp(argv[1], argv[2]);
}

ConsoleFunction(stricmp, S32, 3, 3, "stricmp(one, two)")
{
   argc;
   return dStricmp(argv[1], argv[2]);
}

ConsoleFunction(strlen, S32, 2, 2, "strlen(str)")
{
   argc;
   return dStrlen(argv[1]);
}

ConsoleFunction(strstr, S32 , 3, 3, "strstr(string, substr)")
{
   argc;
   // returns the start of the sub string argv[2] in argv[1]
   // or -1 if not found.

   const char *retpos = dStrstr(argv[1], argv[2]);
   if(!retpos)
      return -1;
   return retpos - argv[1];
}

ConsoleFunction(strpos, S32, 3, 4, "pos = strpos(string hay, string needle [, int offset]);")
{
   S32 ret = -1;
   S32 start = 0;
   if(argc == 4)
      start = dAtoi(argv[3]);
   U32 sublen = dStrlen(argv[2]);
   U32 strlen = dStrlen(argv[1]);
   if(start < 0)
      return -1;
   if(sublen + start > strlen)
      return -1;
   for(; start + sublen <= strlen; start++)
      if(!dStrncmp(argv[1] + start, argv[2], sublen))
         return start;
   return -1;
}

ConsoleFunction(ltrim, const char *,2,2,"ltrim(string)")
{
   argc;
   const char *ret = argv[1];
   while(*ret == ' ' || *ret == '\n' || *ret == '\t')
      ret++;
   return ret;
}

ConsoleFunction(rtrim, const char *,2,2,"rtrim(string)")
{
   argc;
   S32 firstWhitespace = 0;
   S32 pos = 0;
   const char *str = argv[1];
   while(str[pos])
   {
      if(str[pos] != ' ' && str[pos] != '\n' && str[pos] != '\t')
         firstWhitespace = pos + 1;
      pos++;
   }
   char *ret = Con::getReturnBuffer(firstWhitespace + 1);
   dStrncpy(ret, argv[1], firstWhitespace);
   ret[firstWhitespace] = 0;
   return ret;
}

ConsoleFunction(trim, const char *,2,2,"trim(string)")
{
   argc;
   const char *ptr = argv[1];
   while(*ptr == ' ' || *ptr == '\n' || *ptr == '\t')
      ptr++;
   S32 firstWhitespace = 0;
   S32 pos = 0;
   const char *str = ptr;
   while(str[pos])
   {
      if(str[pos] != ' ' && str[pos] != '\n' && str[pos] != '\t')
         firstWhitespace = pos + 1;
      pos++;
   }
   char *ret = Con::getReturnBuffer(firstWhitespace + 1);
   dStrncpy(ret, ptr, firstWhitespace);
   ret[firstWhitespace] = 0;
   return ret;
}

ConsoleFunction(stripChars, const char*, 3, 3, "stripChars( string, chars )" )
{
   argc;
   char* ret = Con::getReturnBuffer( dStrlen( argv[1] ) + 1 );
   dStrcpy( ret, argv[1] );
   U32 pos = dStrcspn( ret, argv[2] );
   while ( pos < dStrlen( ret ) )
   {
      dStrcpy( ret + pos, ret + pos + 1 );
      pos = dStrcspn( ret, argv[2] );
   }
   return( ret );
}

ConsoleFunction(strlwr,const char *,2,2,"strlwr(string)")
{
   argc;
   char *ret = Con::getReturnBuffer(dStrlen(argv[1]) + 1);
   dStrcpy(ret, argv[1]);
   return dStrlwr(ret);
}

ConsoleFunction(strupr,const char *,2,2,"strupr(string)")
{
   argc;
   char *ret = Con::getReturnBuffer(dStrlen(argv[1]) + 1);
   dStrcpy(ret, argv[1]);
   return dStrupr(ret);
}

ConsoleFunction(strchr,const char *,3,3,"strchr(string,char)")
{
   argc;
   const char *ret = dStrchr(argv[1], argv[2][0]);
   return ret ? ret : "";
}

ConsoleFunction(strreplace, const char *, 4, 4, "strreplace(string, from, to)")
{
   argc;
   S32 fromLen = dStrlen(argv[2]);
   if(!fromLen)
      return argv[1];

   S32 toLen = dStrlen(argv[3]);
   S32 count = 0;
   const char *scan = argv[1];
   while(scan)
   {
      scan = dStrstr(scan, argv[2]);
      if(scan)
      {
         scan += fromLen;
         count++;
      }
   }
   char *ret = Con::getReturnBuffer(dStrlen(argv[1]) + 1 + (toLen - fromLen) * count);
   U32 scanp = 0;
   U32 dstp = 0;
   for(;;)
   {
      const char *scan = dStrstr(argv[1] + scanp, argv[2]);
      if(!scan)
      {
         dStrcpy(ret + dstp, argv[1] + scanp);
         return ret;
      }
      U32 len = scan - (argv[1] + scanp);
      dStrncpy(ret + dstp, argv[1] + scanp, len);
      dstp += len;
      dStrcpy(ret + dstp, argv[3]);
      dstp += toLen;
      scanp += len + fromLen;
   }
   return ret;
}

ConsoleFunction(getSubStr, const char *, 4, 4, "getSubStr(string, start, numChars)")
{
   argc;
   // Returns the substring of argv[1], starting at argv[2], and continuing
   //  to either the end of the string, or argv[3] characters, whichever
   //  comes first.
   //
   S32 startPos   = dAtoi(argv[2]);
   S32 desiredLen = dAtoi(argv[3]);
   if (startPos < 0 || desiredLen < 0) {
      Con::errorf(ConsoleLogEntry::Script, "getSubStr(...): error, starting position and desired length must be >= 0: (%d, %d)", startPos, desiredLen);

      return "";
   }

   S32 baseLen = dStrlen(argv[1]);
   if (baseLen < startPos)
      return "";

   U32 actualLen = desiredLen;
   if (startPos + desiredLen > baseLen)
      actualLen = baseLen - startPos;

   char *ret = Con::getReturnBuffer(actualLen + 1);
   dStrncpy(ret, argv[1] + startPos, actualLen);
   ret[actualLen] = '\0';

   return ret;
}

//--------------------------------------
static const char *getUnit(const char *string, U32 index, const char *set)
{
   U32 sz;
   while(index--)
   {
      if(!*string)
         return "";
      sz = dStrcspn(string, set);
      if (string[sz] == 0)
         return "";
      string += (sz + 1);    
   }
   sz = dStrcspn(string, set);
   if (sz == 0)
      return "";
   char *ret = Con::getReturnBuffer(sz+1);
   dStrncpy(ret, string, sz);
   ret[sz] = '\0';
   return ret;
}   

static const char *getUnits(const char *string, S32 startIndex, S32 endIndex, const char *set)
{
   S32 sz;
   S32 index = startIndex;
   while(index--)
   {
      if(!*string)
         return "";
      sz = dStrcspn(string, set);
      if (string[sz] == 0)
         return "";
      string += (sz + 1);    
   }
   const char *startString = string;
   while(startIndex <= endIndex--)
   {
      sz = dStrcspn(string, set);
      string += sz;
      if (*string == 0)
         break;
      string++;
   }
   if(!*string)
      string++;
   U32 totalSize = (U32(string - startString));
   char *ret = Con::getReturnBuffer(totalSize);
   dStrncpy(ret, startString, totalSize - 1);
   ret[totalSize-1] = '\0';
   return ret;
}

static U32 getUnitCount(const char *string, const char *set)
{
   U32 count = 0;
   U8 last = 0;
   while(*string)
   {
      last = *string++;
      
      for(U32 i =0; set[i]; i++)
      {
         if(last == set[i])
         {
            count++;
            last = 0;
            break;
         }   
      }
   }
   if(last)
      count++;
   return count;
}   


static const char* setUnit(const char *string, U32 index, const char *replace, const char *set)
{
   U32 sz;
   const char *start = string;
   char *ret = Con::getReturnBuffer(dStrlen(string) + dStrlen(replace) + 1);
   ret[0] = '\0';
   U32 padCount = 0;
   
   while(index--)
   {
      sz = dStrcspn(string, set);
      if(string[sz] == 0)
      {
         string += sz;
         padCount = index + 1;
         break;
      }
      else
         string += (sz + 1);    
   }
   // copy first chunk
   sz = string-start;
   dStrncpy(ret, start, sz);
   for(U32 i = 0; i < padCount; i++)
      ret[sz++] = set[0];

   // replace this unit
   ret[sz] = '\0';
   dStrcat(ret, replace);

   // copy remaining chunks
   sz = dStrcspn(string, set);         // skip chunk we're replacing
   if(!sz && !string[sz])
      return ret;
      
   string += sz;
   dStrcat(ret, string);
   return ret;
}


static const char* removeUnit(const char *string, U32 index, const char *set)
{
   U32 sz;
   const char *start = string;
   char *ret = Con::getReturnBuffer(dStrlen(string) + 1);
   ret[0] = '\0';
   U32 padCount = 0;
   
   while(index--)
   {
      sz = dStrcspn(string, set);
      // if there was no unit out there... return the original string
      if(string[sz] == 0)
         return start;
      else
         string += (sz + 1);    
   }
   // copy first chunk
   sz = string-start;
   dStrncpy(ret, start, sz);
   ret[sz] = 0;

   // copy remaining chunks
   sz = dStrcspn(string, set);         // skip chunk we're removing
   if(!sz && !string[sz])
      return ret;
      
   string += sz + 1; // skip the extra field delimiter
   dStrcat(ret, string);
   return ret;
}


//--------------------------------------
ConsoleFunction(getWord, const char *, 3, 3, "getWord(text, index)")
{
   argc;
   return getUnit(argv[1], dAtoi(argv[2]), " \t\n");
}

ConsoleFunction(getWords, const char *, 3, 4, "getWord(text, index [,endIndex])")
{
   U32 endIndex;
   if(argc==3)
      endIndex = 1000000;
   else
      endIndex = dAtoi(argv[3]);
   return getUnits(argv[1], dAtoi(argv[2]), endIndex, " \t\n");
}

ConsoleFunction(setWord, const char *, 4, 4, "newText = setWord(text, index, replace)")
{
   argc;
   return setUnit(argv[1], dAtoi(argv[2]), argv[3], " \t\n");
}

ConsoleFunction(removeWord, const char *, 3, 3, "newText = removeWord(text, index)")
{
   argc;
   return removeUnit(argv[1], dAtoi(argv[2]), " \t\n");
}

ConsoleFunction(getWordCount, S32, 2, 2, "getWordCount(text)")
{
   argc;
   return getUnitCount(argv[1], " \t\n");
}

//--------------------------------------
ConsoleFunction(getField, const char *, 3, 3, "getField(text, index)")
{
   argc;
   return getUnit(argv[1], dAtoi(argv[2]), "\t\n");
}

ConsoleFunction(getFields, const char *, 3, 4, "getFields(text, index [,endIndex])")
{
   U32 endIndex;
   if(argc==3)
      endIndex = 1000000;
   else
      endIndex = dAtoi(argv[3]);
   return getUnits(argv[1], dAtoi(argv[2]), endIndex, "\t\n");
}

ConsoleFunction(setField, const char *, 4, 4, "newText = setField(text, index, replace)")
{
   argc;
   return setUnit(argv[1], dAtoi(argv[2]), argv[3], "\t\n");
}

ConsoleFunction(removeField, const char *, 3, 3, "newText = removeField(text, index)" )
{
   argc;
   return removeUnit(argv[1], dAtoi(argv[2]), "\t\n");
}

ConsoleFunction(getFieldCount, S32, 2, 2, "getFieldCount(text)")
{
   argc;
   return getUnitCount(argv[1], "\t\n");
}

//--------------------------------------
ConsoleFunction(getRecord, const char *, 3, 3, "getRecord(text, index)")
{
   argc;
   return getUnit(argv[1], dAtoi(argv[2]), "\n");
}

ConsoleFunction(getRecords, const char *, 3, 4, "getRecords(text, index [,endIndex])")
{
   U32 endIndex;
   if(argc==3)
      endIndex = 1000000;
   else
      endIndex = dAtoi(argv[3]);
   return getUnits(argv[1], dAtoi(argv[2]), endIndex, "\n");
}

ConsoleFunction(setRecord, const char *, 4, 4, "newText = setRecord(text, index, replace)")
{
   argc;
   return setUnit(argv[1], dAtoi(argv[2]), argv[3], "\n");
}

ConsoleFunction(removeRecord, const char *, 3, 3, "newText = removeRecord(text, index)" )
{
   argc;
   return removeUnit(argv[1], dAtoi(argv[2]), "\n");
}

ConsoleFunction(getRecordCount, S32, 2, 2, "getRecordCount(text)")
{
   argc;
   return getUnitCount(argv[1], "\n");
}
//--------------------------------------
ConsoleFunction(firstWord, const char *, 2, 2, "firstWord(text)")
{
   argc;
   const char *word = dStrchr(argv[1], ' ');
   U32 len;
   if(word == NULL)
      len = dStrlen(argv[1]);
   else
      len = word - argv[1];
   char *ret = Con::getReturnBuffer(len + 1);
   dStrncpy(ret, argv[1], len);
   ret[len] = 0;
   return ret;
}

ConsoleFunction(restWords, const char *, 2, 2, "restWords(text)")
{
   argc;
   const char *word = dStrchr(argv[1], ' ');
   if(word == NULL)
      return "";
   char *ret = Con::getReturnBuffer(dStrlen(word + 1) + 1);
   dStrcpy(ret, word + 1);
   return ret;
}

//----------------------------------------------------------------

ConsoleFunction(detag, const char *, 2, 2, "detag(textTagString)")
{
   argc;
   if(argv[1][0] == StringTagPrefixByte)
   {
      const char *word = dStrchr(argv[1], ' ');
      if(word == NULL)
         return "";
      char *ret = Con::getReturnBuffer(dStrlen(word + 1) + 1);
      dStrcpy(ret, word + 1);
      return ret;
   }
   else
      return argv[1];
}

ConsoleFunction(getTag, const char *, 2, 2, "getTag(textTagString)")
{
   argc;
   if(argv[1][0] == StringTagPrefixByte)
   {
      const char * space = dStrchr(argv[1], ' ');

      U32 len;
      if(space)
         len = space - argv[1];
      else
         len = dStrlen(argv[1]) + 1;
   
      char * ret = Con::getReturnBuffer(len);
      dStrncpy(ret, argv[1] + 1, len - 1);
      ret[len - 1] = 0;

      return(ret);
   }
   else
      return(argv[1]);
}

//----------------------------------------------------------------

ConsoleFunction(echo, void, 2, 0, "echo(text [, ... ])")
{
   U32 len = 0;
   S32 i;
   if(gEvalState.traceOn)
      len = (gEvalState.stack.size() - 1) * 2;
   for(i = 1; i < argc; i++)
      len += dStrlen(argv[i]);
   
   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   if(gEvalState.traceOn)
   {
      for(i = 0; i < (S32)gEvalState.stack.size() - 1; i++)
         dStrcat(ret, "  ");
   }
   for(i = 1; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::printf("%s", ret);
   ret[0] = 0;
}

ConsoleFunction(warn, void, 2, 0, "warn(text [, ... ])")
{
   U32 len = 0;
   S32 i;
   if(gEvalState.traceOn)
      len = (gEvalState.stack.size() - 1) * 2;
   for(i = 1; i < argc; i++)
      len += dStrlen(argv[i]);
   
   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   if(gEvalState.traceOn)
   {
      for(i = 0; i < (S32)gEvalState.stack.size() - 1; i++)
         dStrcat(ret, "  ");
   }
   for(i = 1; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::warnf(ConsoleLogEntry::General, "%s", ret);
   ret[0] = 0;
}

ConsoleFunction(error, void, 2, 0, "error(text [, ... ])")
{
   U32 len = 0;
   S32 i;
   if(gEvalState.traceOn)
      len = (gEvalState.stack.size() - 1) * 2;
   for(i = 1; i < argc; i++)
      len += dStrlen(argv[i]);
   
   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   if(gEvalState.traceOn)
   {
      for(i = 0; i < (S32)gEvalState.stack.size() - 1; i++)
         dStrcat(ret, "  ");
   }
   for(i = 1; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::errorf(ConsoleLogEntry::General, "%s", ret);
   ret[0] = 0;
}

ConsoleFunction(expandEscape, const char *, 2, 2, "expandEscape(text)")
{
   argc;
   char *ret = Con::getReturnBuffer(dStrlen(argv[1])*2 + 1);  // worst case situation
   expandEscape(ret, argv[1]);
   return ret;
}

ConsoleFunction(collapseEscape, const char *, 2, 2, "collapseEscape(text)")
{
   argc;
   char *ret = Con::getReturnBuffer(dStrlen(argv[1]) + 1);  // worst case situation
   dStrcpy( ret, argv[1] );
   collapseEscape( ret );
   return ret;
}

//----------------------------------------------------------------

ConsoleFunction(quit, void, 1, 1, "quit()")
{
   argc; argv;
   Platform::postQuitMessage(0);
}

//----------------------------------------------------------------
ConsoleFunction(call, const char *, 2, 0, "call(funcName [,args ...])")
{
   return Con::execute(argc - 1, argv + 1);
}

static U32 execDepth = 0;
static U32 journalDepth = 1;

ConsoleFunction(compile, bool, 2, 2, "compile(fileName)")
{
   argc;
   char nameBuffer[512];
   char* script = NULL;
   U32 scriptSize = 0;

   Stream *compiledStream = NULL;
   FileTime comModifyTime, scrModifyTime;

   const char *scriptFileName = argv[1];

   dSprintf(nameBuffer, sizeof(nameBuffer), "%s.dso", scriptFileName);
   ResourceObject *rScr = ResourceManager->find(scriptFileName);
   ResourceObject *rCom = ResourceManager->find(nameBuffer);

   if(rCom)
      rCom->getFileTimes(NULL, &comModifyTime);
   if(rScr)
      rScr->getFileTimes(NULL, &scrModifyTime);

   Stream *s = ResourceManager->openStream(scriptFileName);
   if(s)
   {
      scriptSize = ResourceManager->getSize(scriptFileName);
      script = new char [scriptSize+1];
      s->read(scriptSize, script);
      ResourceManager->closeStream(s);
      script[scriptSize] = 0;
   }

   if (!scriptSize || !script)
   {
      delete [] script;
      Con::errorf(ConsoleLogEntry::Script, "exec: invalid script file %s.", scriptFileName);
      return false;
   }
   // compile this baddie.
   Con::printf("Compiling %s...", scriptFileName);
   CodeBlock *code = new CodeBlock();
   code->compile(nameBuffer, scriptFileName, script);
   delete code;
   code = NULL;

   delete[] script;
   return true;
}

ConsoleFunction(exec, bool, 2, 4, "exec(fileName [, nocalls [,journalScript]])")
{
   bool journal = false;

   execDepth++;
   if(journalDepth >= execDepth)
      journalDepth = execDepth + 1;
   else
      journal = true;

   bool noCalls = false;
   bool ret = false;

   if(argc >= 3 && dAtoi(argv[2]))
      noCalls = true;
   if(argc >= 4 && dAtoi(argv[3]) && !journal)
   {
      journal = true;
      journalDepth = execDepth;
   }

   const char *ext = dStrrchr(argv[1], '.');

   if(!ext)
   {
      Con::errorf(ConsoleLogEntry::Script, "exec: invalid script file name %s.", argv[1]);
      execDepth--;
      return false;
   }
   StringTableEntry scriptFileName = StringTable->insert(argv[1]);

   bool compiled = dStricmp(ext, ".mis") && !journal;

   if(journal && Game->isJournalReading())
   {
      char fileNameBuf[256];
      bool fileRead;
      U32 fileSize;

      Game->getJournalStream()->readString(fileNameBuf);
      Game->getJournalStream()->read(&fileRead);
      if(!fileRead)
      {
         Con::errorf(ConsoleLogEntry::Script, "Journal script read (failed) for %s", fileNameBuf);
         execDepth--;
         return false;
      }
      Game->journalRead(&fileSize);
      char *script = new char[fileSize + 1];
      Game->journalRead(fileSize, script);
      script[fileSize] = 0;
      Con::printf("Executing (journal-read) %s.", scriptFileName);
      CodeBlock *newCodeBlock = new CodeBlock();
      newCodeBlock->compileExec(scriptFileName, script, noCalls);
      delete [] script;

      execDepth--;
      return true;
   }

   ResourceObject *rScr = ResourceManager->find(scriptFileName);
   ResourceObject *rCom = NULL;
   
   char nameBuffer[512];
   char* script = NULL;
   U32 scriptSize = 0;
   U32 version;

   Stream *compiledStream = NULL;
   FileTime comModifyTime, scrModifyTime;

   if(compiled)
   {
      dSprintf(nameBuffer, sizeof(nameBuffer), "%s.dso", scriptFileName);
      rCom = ResourceManager->find(nameBuffer);

      if(rCom)
         rCom->getFileTimes(NULL, &comModifyTime);
      if(rScr)
         rScr->getFileTimes(NULL, &scrModifyTime);
   }

   if(compiled && rCom && (!rScr || Platform::compareFileTimes(comModifyTime, scrModifyTime) >= 0))
   {
      compiledStream = ResourceManager->openStream(nameBuffer);
      compiledStream->read(&version);
      if(version != ConsoleDSOVersion)
      {
         ResourceManager->closeStream(compiledStream);
         compiledStream = NULL;
      }
   }

   if(journal && Game->isJournalWriting())
      Game->getJournalStream()->writeString(scriptFileName);

   if(rScr && !compiledStream)
   {
      Stream *s = ResourceManager->openStream(scriptFileName);
      if(journal && Game->isJournalWriting())
         Game->getJournalStream()->write(bool(s != NULL));

      if(s)
      {
         scriptSize = ResourceManager->getSize(scriptFileName);
         script = new char [scriptSize+1];
         s->read(scriptSize, script);

         if(journal && Game->isJournalWriting())
         {
            Game->journalWrite(scriptSize);
            Game->journalWrite(scriptSize, script);
         }
         ResourceManager->closeStream(s);
         script[scriptSize] = 0;
      }

      if (!scriptSize || !script)
      {
         delete [] script;
         Con::errorf(ConsoleLogEntry::Script, "exec: invalid script file %s.", scriptFileName);
         execDepth--;
         return false;
      }
      if(compiled)
      {
         // compile this baddie.
         Con::printf("Compiling %s...", scriptFileName);
         CodeBlock *code = new CodeBlock();
         code->compile(nameBuffer, scriptFileName, script);
         delete code;
         code = NULL;

         compiledStream = ResourceManager->openStream(nameBuffer);
         if(compiledStream)
            compiledStream->read(&version);
      }
   }
   else
   {
      if(journal && Game->isJournalWriting())
         Game->getJournalStream()->write(bool(false));
   }

   if(rScr && !compiledStream)
   {
      // let's just try executing it directly... maybe we're on a readonly volume
      Con::printf("Executing %s.", scriptFileName);
      CodeBlock *newCodeBlock = new CodeBlock();
      StringTableEntry name = StringTable->insert(scriptFileName);
   
      newCodeBlock->compileExec(name, script, noCalls);
   }
   delete [] script;
   ret = true;
   if(compiledStream)
   {
      Con::printf("Loading compiled script %s.", scriptFileName);
      CodeBlock *code = new CodeBlock;
      code->read(scriptFileName, *compiledStream);
      ResourceManager->closeStream(compiledStream);
      code->exec(0, scriptFileName, NULL, 0, NULL, noCalls);
   }
   else
   {
      Con::warnf(ConsoleLogEntry::Script, "Missing file: %s!", scriptFileName);
      ret = false;
   }
   execDepth--;
   return ret;
}

ConsoleFunction(eval, const char *, 2, 2, "eval(consoleString)")
{
   argc;
   return Con::evaluate(argv[1], false, NULL);
}

//----------------------------------------------------------------

ConsoleFunction(export, void, 2, 4, "export(searchString [, fileName [,append]])")
{
   const char *fileName =(argc >= 3) ? argv[2] : NULL;
   bool append = (argc == 4) ? dAtob(argv[3]) : false;
   
   gEvalState.globalVars.exportVariables(argv[1], fileName, append);
}

ConsoleFunction(deleteVariables, void, 2, 2, "deleteVariables(wildCard)")
{
   argc;
   gEvalState.globalVars.deleteVariables(argv[1]);
}

//----------------------------------------------------------------

ConsoleFunction(trace, void, 2, 2, "trace(bool)")
{
   argc;
   gEvalState.traceOn = dAtob(argv[1]);
   Con::printf("Console trace is %s", gEvalState.traceOn ? "on." : "off.");
}

//----------------------------------------------------------------

#if defined(DEBUG) || defined(INTERNAL_RELEASE)
ConsoleFunction(debug, void, 1, 1, "debug()")
{
   argv; argc;
   Platform::debugBreak();
}
#endif



//----------------------------------------------------------------

//----------------------------------------------------------------

static ResourceObject *firstMatch = NULL;

ConsoleFunction(findFirstFile, const char *, 2, 2, "findFirstFile(pattern)")
{
   argc;
   const char *fn;
   firstMatch = ResourceManager->findMatch(argv[1], &fn, NULL);
   if(firstMatch)
      return fn;
   else
      return "";
}

ConsoleFunction(findNextFile, const char *, 2, 2, "findNextFile(pattern)")
{
   argc;
   const char *fn;
   firstMatch = ResourceManager->findMatch(argv[1], &fn, firstMatch);
   if(firstMatch)
      return fn;
   else
      return "";
}

ConsoleFunction(getFileCount, S32, 2, 2, "getFileCount(pattern)")
{
   argc;
   const char* fn;
   U32 count = 0;
   firstMatch = ResourceManager->findMatch(argv[1], &fn, NULL);
   if ( firstMatch )
   {
      count++;
      while ( 1 )
      {
         firstMatch = ResourceManager->findMatch(argv[1], &fn, firstMatch);
         if ( firstMatch )
            count++;
         else
            break;
      }
   }
      
   return( count );
}

ConsoleFunction(getFileCRC, S32, 2, 2, "getFileCRC(filename)")
{
   argc;
   U32 crcVal;
   if(!ResourceManager->getCrc(argv[1], crcVal))
      return(-1);
   return(S32(crcVal));
}

ConsoleFunction(isFile, bool, 2, 2, "isFile(fileName)")
{
   argc;
   return bool(ResourceManager->find(argv[1]));
}

ConsoleFunction(isWriteableFileName, bool, 2, 2, "isWriteableFileName(fileName)")
{
   argc;
   // in a writeable directory?
   if(!ResourceManager->isValidWriteFileName(argv[1]))
      return(false);

   // exists?
   FileStream fs;
   if(!fs.open(argv[1], FileStream::Read))
      return(true);
  
   // writeable? (ReadWrite will create file if it does not exist)
   fs.close();
   if(!fs.open(argv[1], FileStream::ReadWrite))
      return(false);

   return(true);
}

ConsoleFunction(cdFileCheck, bool, 1, 1, "cdFileCheck(fileName, volume, serial)")
{
   argc; argv;
   //see if the file exists...
   //return Platform::cdFileExists("575060.bin", "UKD_575060.001", 0x7B9EE4E7);
   bool ret = Platform::cdFileExists("\\Tribes2\\GameData\\Tribes2.exe", "T2 INSTALL", 0);
#if defined(DEBUG) || defined(INTERNAL_RELEASE)
   return 1;
#else
   return ret;
#endif
}

//----------------------------------------------------------------

ConsoleFunction(fileExt, const char *, 2, 2, "fileExt(fileName)")
{
   argc;
   const char *ret = dStrrchr(argv[1], '.');
   if(ret)
      return ret;
   return "";
}

ConsoleFunction(fileBase, const char *, 2, 2, "fileBase(fileName)")
{
   argc;
   const char *path = dStrrchr(argv[1], '/');
   if(!path)
      path = argv[1];
   else
      path++;
   char *ret = Con::getReturnBuffer(dStrlen(path) + 1);
   dStrcpy(ret, path);
   char *ext = dStrrchr(ret, '.');
   if(ext)
      *ext = 0;
   return ret;
}

ConsoleFunction(filePath, const char *, 2, 2, "filePath(fileName)")
{
   argc;
   const char *path = dStrrchr(argv[1], '/');
   if(!path)
      return "";
   U32 len = path - argv[1];
   char *ret = Con::getReturnBuffer(len + 1);
   dStrncpy(ret, argv[1], len);
   ret[len] = 0;
   return ret;
}

static bool isInSet(char c, const char *set)
{
   if (set)
      while (*set)
         if (c == *set++)
            return true;

   return false;
}

ConsoleFunction(NextToken,const char *,4,4,"nextToken(str,token,delim)")
{
   argc;

   char *str = (char *) argv[1];
   const char *token = argv[2];
   const char *delim = argv[3];

   if (str)
   {
      // skip over any characters that are a member of delim
      // no need for special '\0' check since it can never be in delim
      while (isInSet(*str, delim))
         str++;
   
      // skip over any characters that are NOT a member of delim
      const char *tmp = str;

      while (*str && !isInSet(*str, delim))
         str++;

      // terminate the token
      if (*str)
         *str++ = 0;
	  Con::setLocalVariable(token,tmp);

      // advance str past the 'delim space'
      while (isInSet(*str, delim))
         str++;
   }

   return str;
}

//----------------------------------------------------------------

ConsoleFunction(setLogMode, void, 2, 2, "setLogMode(mode);")
{
   argc;
   Con::setLogMode(dAtoi(argv[1]));
}

ConsoleFunction(setEchoFileLoads, void, 2, 2, "setEchoFileLoads(bool);")
{
   argc;
   ResourceManager->setFileNameEcho(dAtob(argv[1]));
}


