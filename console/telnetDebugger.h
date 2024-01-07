//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TELNETDEBUGGER_H_
#define _TELNETDEBUGGER_H_

class CodeBlock;

class TelnetDebugger
{
   S32 mAcceptPort;
   NetSocket mAcceptSocket;
   NetSocket mDebugSocket;

   enum {
      PasswordMaxLength = 32,
      MaxCommandSize = 2048
   };

   char mDebuggerPassword[PasswordMaxLength+1];
   enum State
   {
      NotConnected,
      PasswordTry,
      Connected
   };
   S32 mState;
   char mLineBuffer[MaxCommandSize];
   S32 mCurPos;
   
   TelnetDebugger();
   ~TelnetDebugger();
   
   struct Breakpoint
   {
      CodeBlock *code;
      U32 lineNumber;
      S32 passCount;
      S32 curCount;
      char *testExpression;
      bool clearOnHit;
      Breakpoint *next;
   };
   Breakpoint *mBreakpoints;
   
   Breakpoint **findBreakpoint(StringTableEntry fileName, S32 lineNumber);

   bool mProgramPaused;
   bool mBreakOnNextStatement;
   S32 mStackPopBreakIndex;

   void addVariableBreakpoint(const char *varName, S32 passCount, const char *evalString);
   void removeVariableBreakpoint(const char *varName);
   void addBreakpoint(const char *fileName, S32 line, bool clear, S32 passCount, const char *evalString);
   void removeBreakpoint(const char *fileName, S32 line);
   void removeAllBreakpoints();
   
   void debugContinue();
   void debugStepIn();
   void debugStepOver();
   void debugStepOut();
   void evaluateExpression(const char *tag, S32 frame, const char *evalBuffer);
   void dumpFileList();
   void dumpBreakableList(const char *fileName);
   void removeBreakpointsFromCode(CodeBlock *code);

   void checkDebugRecv();
   void processLineBuffer(S32);
   void breakProcess();
   void breakOnNextStatement();
public:
   static void create();
   static void destroy();
   
   void process();
   void popStackFrame();
   
   virtual void executionStopped(CodeBlock *code, U32 lineNumber);
   void send(const char *s);
   void setDebugParameters(S32 port, const char *password);
   void processConsoleLine(const char *consoleLine);
};

extern TelnetDebugger *TelDebugger;

#endif
