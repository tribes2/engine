//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiDebugger.h"
#include "Core/stream.h"

static const char* itoa(S32 i)
{
   static char buf[32];
   dSprintf(buf, sizeof(buf), "%d", i);
   return buf;
}

static const char* itoa2(S32 i)
{
   static char buf[32];
   dSprintf(buf, sizeof(buf), "%d", i);
   return buf;
}

DbgFileView::~DbgFileView()
{
   clear();
}

DbgFileView::DbgFileView()
{
   VECTOR_SET_ASSOCIATION(mFileView);

   mFileName = NULL;
   mPCFileName = NULL;
   mPCCurrentLine = -1;
   
   mBlockStart = -1;
   mBlockEnd = -1;

   mFindString[0] = '\0';
	S32 mFindLineNumber = -1;
   
   mSize.set(1, 0);
}

static void cDbgFileViewSetCurrentLine(SimObject *obj, S32, const char **argv)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
   dbgCtrl->setCurrentLine(dAtoi(argv[2]), dAtob(argv[3]));
}

static const char* cDbgFileViewGetCurrentLine(SimObject *obj, S32, const char **)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
	S32 lineNum;
   const char *file = dbgCtrl->getCurrentLine(lineNum);
   char* ret = Con::getReturnBuffer(256);
	dSprintf(ret, sizeof(ret), "%s\t%d", file, lineNum);
	return ret;
}

static bool cDbgFileViewOpen(SimObject *obj, S32, const char **argv)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
   return dbgCtrl->openFile(argv[2]);
}

static void cDbgFileClearBreakPositions(SimObject *obj, S32, const char **)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
   dbgCtrl->clearBreakPositions();
}

static void cDbgFileSetBreakPosition(SimObject *obj, S32, const char **argv)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
   dbgCtrl->setBreakPosition(dAtoi(argv[2]));
}

static void cDbgFileSetBreak(SimObject *obj, S32, const char **argv)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
   dbgCtrl->setBreakPointStatus(dAtoi(argv[2]), true);
}

static void cDbgFileRemoveBreak(SimObject *obj, S32, const char **argv)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
   dbgCtrl->setBreakPointStatus(dAtoi(argv[2]), false);
}

static bool cDbgFindString(SimObject *obj, S32, const char **argv)
{
   DbgFileView *dbgCtrl = static_cast<DbgFileView*>(obj);
   return dbgCtrl->findString(argv[2]);
}

void DbgFileView::consoleInit()
{
   Con::addCommand("DbgFileView", "open",            cDbgFileViewOpen,          "fileView.open(file)",   3, 3);
   Con::addCommand("DbgFileView", "setCurrentLine",  cDbgFileViewSetCurrentLine,"fileView.setCurrentLine(line, displayLine)",   4, 4);
   Con::addCommand("DbgFileView", "getCurrentLine",  cDbgFileViewGetCurrentLine,"fileView.getCurrentLine()",   2, 2);
   Con::addCommand("DbgFileView", "clearBreakPositions",cDbgFileClearBreakPositions,"fileView.clearBreakPositions()",    2, 2);
   Con::addCommand("DbgFileView", "setBreakPosition",cDbgFileSetBreakPosition,  "fileView.setBreakPosition(line)",      3, 3);
   Con::addCommand("DbgFileView", "setBreak",        cDbgFileSetBreak,          "fileView.setBreak(line)",                      3, 3);
   Con::addCommand("DbgFileView", "removeBreak",     cDbgFileRemoveBreak,       "fileView.removeBreak(line)",                   3, 3);
   Con::addCommand("DbgFileView", "findString",      cDbgFindString,       	  "fileView.findString(Text)",            3, 3);
}

//this value is the offset used in the ::onRender() method...
static S32 gFileXOffset = 44;
void DbgFileView::AdjustCellSize()
{
   if (! bool(mFont))
      return;
   S32 maxWidth = 1;
   for (U32 i = 0; i < mFileView.size(); i++)
   {
      S32 cellWidth = gFileXOffset + mFont->getStrWidth(mFileView[i].text);
      maxWidth = getMax(maxWidth, cellWidth);
   }
   
   mCellSize.set(maxWidth, mFont->getHeight() + 2);
   setSize(mSize);
}

bool DbgFileView::onWake()
{
   if (! Parent::onWake())
      return false;
   
   //clear the mouse over var
   mMouseOverVariable[0] = '\0';
   mbMouseDragging = false;
   
   //adjust the cellwidth to the maximum line length
   AdjustCellSize();
   mSize.set(1, mFileView.size());
   
   return true;
}

void DbgFileView::addLine(const char *string, U32 strLen)
{
   // first compute the size
   U32 size = 1; // for null
   for(U32 i = 0; i < strLen; i++)
   {
      if(string[i] == '\t')
         size += 3;
      else if(string[i] != '\r')
         size++;
   }
   FileLine fl;
   fl.breakPosition = false;
   fl.breakOnLine = false;
   if(size)
   {
      fl.text = (char *) dMalloc(size);
   
      U32 dstIndex = 0;
      for(U32 i = 0; i < strLen; i++)
      {
         if(string[i] == '\t')
         {
            fl.text[dstIndex] = ' ';
            fl.text[dstIndex + 1] = ' ';
            fl.text[dstIndex + 2] = ' ';
            dstIndex += 3;
         }
         else if(string[i] != '\r')
            fl.text[dstIndex++] = string[i];
      }
      fl.text[dstIndex] = 0;   
   }
   else
      fl.text = NULL;
   mFileView.push_back(fl);
}

void DbgFileView::clear()
{
   for(Vector<FileLine>::iterator i = mFileView.begin(); i != mFileView.end(); i++)
      dFree(i->text);
   mFileView.clear();
}

bool DbgFileView::findString(const char *text)
{
	//make sure we have a valid string to find
	if (!text || !text[0])
		return false;

	//see which line we start searching from
	S32 curLine = 0;
	bool searchAgain = false;
	if (mFindLineNumber >= 0 && !dStricmp(mFindString, text))
	{
		searchAgain = true;
		curLine = mFindLineNumber;
	}
	else
		mFindLineNumber = -1;

	//copy the search text
	dStrncpy(mFindString, text, 255);
	S32 length = dStrlen(mFindString);

	//loop through looking for the next instance
	while (curLine < mFileView.size())
	{
		char *curText;
		if (curLine == mFindLineNumber && mBlockStart >= 0)
			curText = &mFileView[curLine].text[mBlockStart + 1];
		else
			curText = &mFileView[curLine].text[0];

		//search for the string (the hard way... - apparently dStrupr is broken...
		char *found = NULL;
		char *curTextPtr = curText;
		while (*curTextPtr != '\0')
		{
			if (!dStrnicmp(mFindString, curTextPtr, length))
			{
				found = curTextPtr;
				break;
			}
			else
				curTextPtr++;
		}

		//did we find it?
		if (found)
		{
			//scroll first
			mFindLineNumber = curLine;
	      scrollToLine(mFindLineNumber + 1);

			//then hilite
			mBlockStart = (S32)(found - &mFileView[curLine].text[0]);
			mBlockEnd = mBlockStart + length;

			return true;
		}
		else
			curLine++;
	}

	//didn't find anything - reset the vars for the next search
	mBlockStart = -1;
	mBlockEnd = -1;
	mFindLineNumber = -1;

	setSelectedCell(Point2I(-1, -1));
	return false;
}

void DbgFileView::setCurrentLine(S32 lineNumber, bool setCurrentLine)
{
   //update the line number
   if (setCurrentLine)
   {
      mPCFileName = mFileName;
      mPCCurrentLine = lineNumber;
      mBlockStart = -1;
      mBlockEnd = -1;
		if (lineNumber >= 0)
	      scrollToLine(mPCCurrentLine);
   }
   else
   {
      scrollToLine(lineNumber);
   }
}   

const char* DbgFileView::getCurrentLine(S32 &lineNumber)
{
	lineNumber = mPCCurrentLine;
	return mPCFileName;
}   

bool DbgFileView::openFile(const char *fileName)
{
   if ((! fileName) || (! fileName[0]))
      return false;
   
   StringTableEntry newFile = StringTable->insert(fileName);
   if (mFileName == newFile)
      return true;
   
   U32 fileSize = ResourceManager->getSize(fileName);
   char *fileBuf;
   if (fileSize)
   {
      fileBuf = new char [fileSize+1];
      Stream *s = ResourceManager->openStream(fileName);
      if (s)
      {
         s->read(fileSize, fileBuf);
         ResourceManager->closeStream(s);
         fileBuf[fileSize] = '\0';
      }
      else
      {
         delete [] fileBuf;
         fileBuf = NULL;
      }
   }
   if (!fileSize || !fileBuf)
   {
      Con::printf("DbgFileView: unable to open file %s.", fileName);
      return false;
   }
   
   //copy the file name
   mFileName = newFile;
   
   //clear the old mFileView
   clear();
   setSize(Point2I(1, 0));
   
   //begin reading and parsing at each '\n'
   char *parsePtr = fileBuf;
   for(;;) {
      char *tempPtr = dStrchr(parsePtr, '\n');
      if(tempPtr)
         addLine(parsePtr, tempPtr - parsePtr);
      else if(parsePtr[0])
         addLine(parsePtr, dStrlen(parsePtr));
      if(!tempPtr)
         break;
      parsePtr = tempPtr + 1;
   }
   //delete the buffer
   delete [] fileBuf;
   
   //set the file size
   AdjustCellSize();
   setSize(Point2I(1, mFileView.size()));
   
   return true;
}

void DbgFileView::scrollToLine(S32 lineNumber)
{
   GuiControl *parent = getParent();
   if (! parent)
      return;
   
   S32 yOffset = (lineNumber - 1) * mCellSize.y;
   
   //see if the line is already visible
   if (! (yOffset + mBounds.point.y >= 0 && yOffset + mBounds.point.y < parent->mBounds.extent.y - mCellSize.y))
   {
      //reposition the control
      S32 newYOffset = getMin(0, getMax(parent->mBounds.extent.y - mBounds.extent.y, (mCellSize.y * 4) - yOffset));
      resize(Point2I(mBounds.point.x, newYOffset), mBounds.extent);
   }
   
   //hilite the line
   cellSelected(Point2I(0, lineNumber - 1));
}


S32 DbgFileView::findMouseOverChar(const char *text, S32 stringPosition)
{
   //find which character we're over
   char tempBuf[256], *bufPtr = &tempBuf[1];
   dStrcpy(tempBuf, text);
   bool found = false;
   bool finished = false;
   do
   {
      char c = *bufPtr;
      *bufPtr = '\0';
      if ((S32)mFont->getStrWidth(tempBuf) > stringPosition)
      {
         *bufPtr = c;
         bufPtr--;
         found = true;
         finished = true;
      }
      else
      {
         *bufPtr = c;
         bufPtr++;
         if (*bufPtr == '\0') finished = true;
      }
      
   } while (! finished);
   
   //see if we found a char
   if (found)
      return bufPtr - tempBuf;
   else return -1;
}

bool DbgFileView::findMouseOverVariable()
{
   GuiCanvas *root = getRoot();
   AssertFatal(root, "Unable to get the root Canvas.");
   
   Point2I curMouse = root->getCursorPos();
   Point2I pt = globalToLocalCoord(curMouse);
   
   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if(cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      S32 stringPosition = pt.x - gFileXOffset;
      char tempBuf[256], *varNamePtr = &tempBuf[1];
      dStrcpy(tempBuf, mFileView[cell.y].text);
      
      //find the current mouse over char
      S32 charNum = findMouseOverChar(mFileView[cell.y].text, stringPosition);
      if (charNum >= 0)
      {
         varNamePtr = &tempBuf[charNum];   
      }
      else
      {
         mMouseOverVariable[0] = '\0';
         mMouseOverValue[0] = '\0';
         return false;
      }
      
      //now make sure we can go from the current cursor mPosition to the beginning of a var name
      bool found = false;
      while (varNamePtr >= &tempBuf[0])
      {
         if (*varNamePtr == '%' || *varNamePtr == '$')
         {
            found = true;
            break;
         }
         else if ((dToupper(*varNamePtr) >= 'A' && dToupper(*varNamePtr) <= 'Z') ||
                  (*varNamePtr >= '0' && *varNamePtr <= '9') || *varNamePtr == '_' || *varNamePtr == ':')
         {
            varNamePtr--;
         }
         else
         {
            break;
         }
      }
      
      //mouse wasn't over a possible variable name
      if (! found)
      {
         mMouseOverVariable[0] = '\0';
         mMouseOverValue[0] = '\0';
         return false;
      }
      
      //set the var char start positions
      mMouseVarStart = varNamePtr - tempBuf;
      
      //now copy the (possible) var name into the buf
      char *tempPtr = &mMouseOverVariable[0];
      
      //copy the leading '%' or '$'
      *tempPtr++ = *varNamePtr++;
      
      //now copy letters and numbers until the end of the name
      while ((dToupper(*varNamePtr) >= 'A' && dToupper(*varNamePtr) <= 'Z') ||
                  (*varNamePtr >= '0' && *varNamePtr <= '9') || *varNamePtr == '_' || *varNamePtr == ':')
      {
         *tempPtr++ = *varNamePtr++;
      }
      *tempPtr = '\0';
      
      //set the var char end positions
      mMouseVarEnd = varNamePtr - tempBuf;
      
      return true;
   }
   return false;
}

void DbgFileView::clearBreakPositions()
{
   for(Vector<FileLine>::iterator i = mFileView.begin(); i != mFileView.end(); i++)
   {
      i->breakPosition = false;
      i->breakOnLine = false;
   }
}

void DbgFileView::setBreakPosition(U32 line)
{
   if(line > mFileView.size())
      return;
   mFileView[line-1].breakPosition = true;
}

void DbgFileView::setBreakPointStatus(U32 line, bool set)
{
   if(line > mFileView.size())
      return;
   mFileView[line-1].breakOnLine = set;
}
   
void DbgFileView::onPreRender()
{
	setUpdate();
   char oldVar[256];
   dStrcpy(oldVar, mMouseOverVariable);
   bool found = findMouseOverVariable();
   if (found && mPCCurrentLine >= 0)
   {
      //send the query only when the var changes
      if (dStricmp(oldVar, mMouseOverVariable))
			Con::executef(2, "DbgSetCursorWatch", mMouseOverVariable);
   }
	else
		Con::executef(2, "DbgSetCursorWatch", "");
}

void DbgFileView::onMouseDown(const GuiEvent &event)
{
   if (! mActive)
   {
      Parent::onMouseDown(event);
      return;
   }
   
   Point2I pt = globalToLocalCoord(event.mousePoint);
   bool doubleClick = (event.mouseClickCount > 1);
   
   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if(cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      //if we clicked on the breakpoint mark
      if (pt.x >= 0 && pt.x <= 12)
      {
         //toggle the break point
         if (mFileView[cell.y].breakPosition)
         {
            if (mFileView[cell.y].breakOnLine)
               Con::executef(this, 2, "onRemoveBreakPoint", itoa(cell.y + 1));
            else
               Con::executef(this, 2, "onSetBreakPoint", itoa(cell.y + 1));
         }
      }
      else
      {
         Point2I prevSelected = mSelectedCell;
         Parent::onMouseDown(event);
         mBlockStart= -1;
         mBlockEnd = -1;
         
         //open the file view
         if (mSelectedCell.y == prevSelected.y && doubleClick && mMouseOverVariable[0])
         {
            Con::executef(this, 2, "onSetWatch", mMouseOverVariable);
            mBlockStart = mMouseVarStart;
            mBlockEnd = mMouseVarEnd;
         }
         else
         {
            S32 stringPosition = pt.x - gFileXOffset;
            
            //find which character we're over
            S32 charNum = findMouseOverChar(mFileView[mSelectedCell.y].text, stringPosition);
            if (charNum >= 0)
            {
               //lock the mouse
               mouseLock(); 
               setFirstResponder();
            
               //set the block hilite start and end
               mbMouseDragging = true;
               mMouseDownChar = charNum;
            }
         }
      }
   }
   else
   {
      Parent::onMouseDown(event);
   }
}

void DbgFileView::onMouseDragged(const GuiEvent &event)
{
   if (mbMouseDragging)
   {
      Point2I pt = globalToLocalCoord(event.mousePoint);
      S32 stringPosition = pt.x - gFileXOffset;
      
      //find which character we're over
      S32 charNum = findMouseOverChar(mFileView[mSelectedCell.y].text, stringPosition);
      if (charNum >= 0)
      {
         if (charNum < mMouseDownChar)
         {
            
            mBlockEnd = mMouseDownChar + 1;
            mBlockStart = charNum;
         }
         else
         {
            mBlockEnd = charNum + 1;
            mBlockStart = mMouseDownChar;
         }
      }
      
      //otherwise, the cursor is past the end of the string
      else
      {
         mBlockStart = mMouseDownChar;
         mBlockEnd = dStrlen(mFileView[mSelectedCell.y].text) + 1;
      }
   }
}

void DbgFileView::onMouseUp(const GuiEvent &)
{
   //unlock the mouse
   mouseUnlock(); 
               
   mbMouseDragging = false;
}

void DbgFileView::onRenderCell(Point2I offset, Point2I cell, bool selected, bool)
{
   Point2I cellOffset = offset;
   cellOffset.x += 4;
   
   //draw the break point marks
   if (mFileView[cell.y].breakOnLine)
   {
      dglSetBitmapModulation(mProfile->mFontColorHL);
      dglDrawText(mFont, cellOffset, "#");
   }
   else if (mFileView[cell.y].breakPosition)
   {
      dglSetBitmapModulation(mProfile->mFontColor);
      dglDrawText(mFont, cellOffset, "-");
   }
   cellOffset.x += 8;
   
   //draw in the "current line" indicator
   if (mFileName == mPCFileName && (cell.y + 1 == mPCCurrentLine))
   {
      dglSetBitmapModulation(mProfile->mFontColorHL);
      dglDrawText(mFont, cellOffset, "=>");
   }

	//by this time, the cellOffset has been incremented by 44 - the value of gFileXOffset
   cellOffset.x += 32;
   
   //hilite the line if selected
   if (selected)
   {
      if (mBlockStart == -1)
      {
         dglDrawRectFill(RectI(cellOffset.x - 2, cellOffset.y - 3,
                                 mCellSize.x + 4, mCellSize.y + 6), mProfile->mFillColorHL);
      }
      else if (mBlockStart >= 0 && mBlockEnd > mBlockStart && mBlockEnd <= S32(dStrlen(mFileView[cell.y].text) + 1))
      {
         S32 startPos, endPos;
         char tempBuf[256];
         dStrcpy(tempBuf, mFileView[cell.y].text);
         
         //get the end coord
         tempBuf[mBlockEnd] = '\0';
         endPos = mFont->getStrWidth(tempBuf);
         
         //get the start coord
         tempBuf[mBlockStart] = '\0';
         startPos = mFont->getStrWidth(tempBuf);
         
         //draw the hilite
         dglDrawRectFill(RectI(cellOffset.x + startPos, cellOffset.y - 3, endPos - startPos + 2, mCellSize.y + 6), mProfile->mFillColorHL);
      }
   }
   
   //draw the line of text
   dglSetBitmapModulation(mFileView[cell.y].breakOnLine ? mProfile->mFontColorHL : mProfile->mFontColor);
   dglDrawText(mFont, cellOffset, mFileView[cell.y].text);
}

// // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// 
// static void cDbgBreakPointSet(SimObject *obj, S32 argc, const char **argv)
// {
//    DbgBreakPointView *dbgBreakCtrl = static_cast<DbgBreakPointView*>(obj);
//    
//    if (argc == 4)
//    {
//       //this call prevent the expression, passct, and clear flag from being overwritten
//       dbgBreakCtrl->setBreakPointStatus(argv[2], dAtoi(argv[3]), true);
//    }
//    else
//    {
//       //optional params
//       bool clear = false;
//       S32 passct = 0;
//       const char *expr = NULL;
//       if (argc >= 5)
//          clear = !dStricmp(argv[4], "true");
//       if (argc >= 6)
//          passct = dAtoi(argv[5]);
//       if (argc == 7)
//          expr = argv[6];
//          
//       dbgBreakCtrl->setBreakPointStatus(argv[2], dAtoi(argv[3]), true, clear, passct, expr);
//    }
// }
// 
// static void cDbgBreakPointRemove(SimObject *obj, S32, const char **argv)
// {
//    DbgBreakPointView *dbgBreakCtrl = static_cast<DbgBreakPointView*>(obj);
//    dbgBreakCtrl->setBreakPointStatus(argv[2], dAtoi(argv[3]), false);
// }
// 
// static void cDbgBreakPointSetCondition(SimObject *obj, S32, const char **argv)
// {
//    DbgBreakPointView *dbgBreakCtrl = static_cast<DbgBreakPointView*>(obj);
//    dbgBreakCtrl->setBreakPointCondition(!dStricmp(argv[4], "true"), dAtoi(argv[3]), argv[2]);
// }
// 
// static const char* cDbgBreakPointIsSet(SimObject *obj, S32, const char **argv)
// {
//    DbgBreakPointView *dbgBreakCtrl = static_cast<DbgBreakPointView*>(obj);
//       
//    if (dbgBreakCtrl->isBreakPoint(argv[2], dAtoi(argv[3])))
//       return "true";
//    else
//       return "false";
// }
// 
// static void cDbgBreakPointClearAll(SimObject *obj, S32, const char **)
// {
//    DbgBreakPointView *dbgBreakCtrl = static_cast<DbgBreakPointView*>(obj);
//    dbgBreakCtrl->clearBreakPoints();
// }
// 
// DbgBreakPointView::DbgBreakPointView()
// {
//    mSize.set(1, 0);
// }
// 
// void DbgBreakPointView::consoleInit()
// {
//    Con::addCommand("DbgBreakPointView", "set",          cDbgBreakPointSet,            "breakPoints.set(file, line, <clear>, <passct>, <expr>)",   4, 7);
//    Con::addCommand("DbgBreakPointView", "remove",       cDbgBreakPointRemove,         "breakPoints.remove(file, line)",                           4, 4);
//    Con::addCommand("DbgBreakPointView", "condition",    cDbgBreakPointSetCondition,   "breakPoints.condition(clear, passct, condition)",          5, 5);
//    Con::addCommand("DbgBreakPointView", "isSet",        cDbgBreakPointIsSet,          "breakPoints.isSet(file, line)",                            4, 4);
//    Con::addCommand("DbgBreakPointView", "clear",        cDbgBreakPointClearAll,       "breakPoints.clear()",                                      2, 2);
//    Con::linkNamespaces("GuiArrayCtrl", "DbgBreakPointView");
// }
// 
// bool DbgBreakPointView::onWake()
// {
//    if (! Parent::onWake())
//       return false;
//    
//    //adjust the cellwidth to the maximum line length
//    AdjustCellSize();
//    mSize.set(1, mBreakPointList.size());
//    
//    return true;
// }
// 
// void DbgBreakPointView::AdjustCellSize()
// {
//    if (! bool(mFont))
//       return;
//    S32 maxWidth = 1;
//    for (U32 i = 0; i < mBreakPointList.size(); i++)
//    {
//       S32 cellWidth = 158 + mFont->getStrWidth(mBreakPointList[i].condition) + 4;
//       maxWidth = getMax(maxWidth, cellWidth);
//    }
//    
//    mCellSize.set(maxWidth, mFont->getHeight() + 2);
//    setSize(mSize);
// }
// 
// void DbgBreakPointView::setBreakPointStatus(const char *fileName, S32 lineNumber, bool value)
// {
//    //sanity check
//    if ((! fileName) || (! fileName[0]))
//       return;
//    if (lineNumber < 0)
//       return;
//    
//    //see if it's already in the list
//    U32 index;
//    for (index = 0; index < mBreakPointList.size(); index++)
//    {
//       if ((! dStricmp(mBreakPointList[index].fileName, fileName)) && (mBreakPointList[index].lineNumber == lineNumber))
//          break;
//    }
//    
//    if (index < mBreakPointList.size())
//    {
//       char buf[256];
//       dStrcpy(buf, mBreakPointList[index].condition);
//       setBreakPointStatus(fileName, lineNumber, value,
//                            mBreakPointList[index].clear, mBreakPointList[index].passct, buf);
//    }
//    //else, add it in with default params
//    else
//    {
//       setBreakPointStatus(fileName, lineNumber, value, false, 0, NULL);
//    }
// }
// 
// void DbgBreakPointView::setBreakPointStatus(const char *fileName, S32 lineNumber, bool value,
//                         										bool clear, S32 passct, const char *expr)
// {
//    //sanity check
//    if ((! fileName) || (! fileName[0]))
//       return;
//    if (lineNumber < 0)
//       return;
//    
//    //see if it's already in the list
//    S32 found = -1;
//    for (U32 i = 0; i < mBreakPointList.size(); i++)
//    {
//       if ((! dStricmp(mBreakPointList[i].fileName, fileName)) && (mBreakPointList[i].lineNumber == lineNumber))
//       {
//          mBreakPointList[i].active = value;
//          found = i;
//          break;
//       }
//    }
//    
//    //if not found, add it to the list
//    if (found < 0)
//    {
//       BreakPointRep newBreakPoint;
//       mBreakPointList.push_back(newBreakPoint);
//       found = mBreakPointList.size() - 1;
//    }
//    
//    //set the values
//    dStrcpy(mBreakPointList[found].fileName, fileName);
//    mBreakPointList[found].lineNumber = lineNumber;
//    if (expr && expr[0])
//       dStrncpy(mBreakPointList[found].condition, expr, 255);
//    else
//       dStrcpy(mBreakPointList[found].condition, "true");
//    mBreakPointList[found].condition[255] = '\0';
//    mBreakPointList[found].clear = clear;
//    mBreakPointList[found].passct = passct;
//    mBreakPointList[found].active = value;
//    
//    //update the size()
//    mSelectedCell.set(1, found);
//    AdjustCellSize();
//    setSize(Point2I(1, mBreakPointList.size()));
//    
//    //send the message to the server
//    if (value)
//    {
//       char buf[2048];
//       dSprintf(buf, sizeof(buf), "BRKSET:%s %d %d %d %s", mBreakPointList[found].fileName,
//                                                          mBreakPointList[found].lineNumber,
//                                                          mBreakPointList[found].clear,
//                                                          mBreakPointList[found].passct,
//                                                          mBreakPointList[found].condition);
//       Con::executef(2, "DbgRemoteSend", buf);
//    }
//    else
//    {
//       char buf[2048];
//       dSprintf(buf, sizeof(buf), "BRKCLR:%s %d", fileName, lineNumber);
//       Con::executef(2, "DbgRemoteSend", buf);
//    }
// }
// 
// void DbgBreakPointView::setBreakPointCondition(bool clear, S32 passct, const char *expr)
// {
//    if (mSelectedCell.y < 0 || mSelectedCell.y >= (S32)mBreakPointList.size())
//       return;
//       
//    setBreakPointStatus(mBreakPointList[mSelectedCell.y].fileName,
//                         mBreakPointList[mSelectedCell.y].lineNumber,
//                         mBreakPointList[mSelectedCell.y].active,
//                         clear, passct, expr);
// }
// 
// bool DbgBreakPointView::isBreakPoint(const char *fileName, S32 lineNumber)
// {
//    if ((! fileName) || (! fileName[0]))
//       return false;
//    if (lineNumber < 0)
//       return false;
//    
//    //see if it's in the list
//    for (U32 i = 0; i < mBreakPointList.size(); i++)
//    {
//       if ((! dStricmp(mBreakPointList[i].fileName, fileName)) && (mBreakPointList[i].lineNumber == lineNumber))
//          return mBreakPointList[i].active;
//    }
//    
//    //not found
//    return false;
// }
// 
// void DbgBreakPointView::clearBreakPoints()
// {
//    for (U32 i = 0; i < mBreakPointList.size(); i++)
//    {
//       if (mBreakPointList[i].active)
//       {
//          Con::executef(3, "DbgRemoveBreakPoint", mBreakPointList[i].fileName,
//                                                       itoa(mBreakPointList[i].lineNumber));
//       }
//    }
//    
//    //empty the list
//    mBreakPointList.clear();
//    AdjustCellSize();
//    setSize(Point2I(1, 0));
//    mSelectedCell.set(-1, -1);
// }
// 
// void DbgBreakPointView::onMouseDown(const GuiEvent &event)
// {
//    if (! mActive)
//    {
//       Parent::onMouseDown(event);
//       return;
//    }
//    
//    Point2I pt = globalToLocalCoord(event.mousePoint);
//    bool doubleClick = (event.mouseClickCount > 1);
//    
//    //find out which cell was hit
//    Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
//    if(cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
//    {
//       //if we clicked on the breakpoint mark
//       if (pt.x >= 0 && pt.x <= 12)
//       {
//          //toggle the break point
//          if (! mBreakPointList[cell.y].active)
//             Con::executef(6, "DbgSetBreakPoint", mBreakPointList[cell.y].fileName,
//                                                       itoa(mBreakPointList[cell.y].lineNumber),
//                                                       mBreakPointList[cell.y].clear ? "true" : "false",
//                                                       itoa2(mBreakPointList[cell.y].passct),
//                                                       mBreakPointList[cell.y].condition);
//          else
//             Con::executef(3, "DbgRemoveBreakPoint", mBreakPointList[cell.y].fileName,
//                                                          itoa(mBreakPointList[cell.y].lineNumber));
//       }
//       else
//       {
//          Point2I prevSelected = mSelectedCell;
//          Parent::onMouseDown(event);
//          
//          //open the file view
//          if (mSelectedCell.y == prevSelected.y && doubleClick)
//          {
//             Con::executef(4, "DbgOpenFile", mBreakPointList[cell.y].fileName,
//                                                    itoa(mBreakPointList[cell.y].lineNumber), "false");
//          }
//       }
//    }
//    else
//    {
//       Parent::onMouseDown(event);
//    }
// }
// 
// void DbgBreakPointView::onRenderCell(Point2I offset, Point2I cell, bool selected, bool)
// {
//    Point2I cellOffset = offset;
//    cellOffset.x += 4;
//    
//    //draw the break point marks
//    if (mBreakPointList[cell.y].active)
//    {
//       dglSetBitmapModulation(mProfile->mFontColorHL);
//       dglDrawText(mFont, cellOffset, "#");
//    }
//    else
//    {
//       dglSetBitmapModulation(mProfile->mFontColor);
//       dglDrawText(mFont, cellOffset, "-");
//    }
//    cellOffset.x += 16;
//    
//    //hilite the line if selected
//    if (selected)
//    {
//       dglDrawRectFill(RectI(cellOffset.x - 2, cellOffset.y - 3, mCellSize.x + 4, mCellSize.y + 6),
//                            mProfile->mFillColorHL);
//    }
//    
//    //draw the line number
//    char bufLine[32];
//    dSprintf(bufLine, sizeof(bufLine), "%d", mBreakPointList[cell.y].lineNumber);
//    dglSetBitmapModulation(mBreakPointList[cell.y].active ? mProfile->mFontColorHL : mProfile->mFontColor);
//    dglDrawText(mFont, cellOffset, bufLine);
//    
//    cellOffset.x += 40;
//    
//    //draw the file name clipped to 120 pix
//    char clippedBuf[256], *clipPtr;
//    dStrcpy(clippedBuf, mBreakPointList[cell.y].fileName);
//    clipPtr = &clippedBuf[dStrlen(clippedBuf)];
//    bool finished = false;
//    while ((clipPtr > &clippedBuf[0]) && (! finished))
//    {
//       if (mFont->getStrWidth(clippedBuf) <= 90)
//       {
//          finished = true;
//       }
//       else
//       {
//          *--clipPtr = '\0';
//       }
//    }
//    dglSetBitmapModulation(mProfile->mFontColor);
//    dglDrawText(mFont, cellOffset, clippedBuf);
//    cellOffset.x += 98;
//       
//    //draw the condition
//    dglSetBitmapModulation(mProfile->mFontColorHL);
//    dglDrawText(mFont, cellOffset, mBreakPointList[cell.y].condition);
// }
// 
// // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// 
// static void cDbgWatchViewSet(SimObject *obj, S32, const char **argv)
// {
//    DbgWatchView *dbgCtrl = static_cast<DbgWatchView*>(obj);
//    dbgCtrl->addWatch(argv[2]);
// }
// 
// static void cDbgWatchViewUpdate(SimObject *obj, S32, const char **argv)
// {
//    DbgWatchView *dbgCtrl = static_cast<DbgWatchView*>(obj);
//    dbgCtrl->updateWatch(dAtoi(argv[2]), argv[3]);
// }
// 
// static void cDbgWatchViewEdit(SimObject *obj, S32, const char **argv)
// {
//    DbgWatchView *dbgCtrl = static_cast<DbgWatchView*>(obj);
//    dbgCtrl->editCurrentWatch(argv[2]);
// }
// 
// static void cDbgWatchViewRemove(SimObject *obj, S32, const char **)
// {
//    DbgWatchView *dbgCtrl = static_cast<DbgWatchView*>(obj);
//    dbgCtrl->deleteCurrentWatch();
// }
// 
// static void cDbgWatchViewQueryAll(SimObject *obj, S32, const char **)
// {
//    DbgWatchView *dbgCtrl = static_cast<DbgWatchView*>(obj);
//    dbgCtrl->queryAll();
// }
// 
// static void cDbgWatchViewClear(SimObject *obj, S32, const char **)
// {
//    DbgWatchView *dbgCtrl = static_cast<DbgWatchView*>(obj);
//    dbgCtrl->clearWatches();
// }
// 
// DbgWatchView::DbgWatchView()
// {
//    mSize.set(1, 0);
//    mNextQueryID = 1;
// }
// 
// void DbgWatchView::consoleInit()
// {
//    Con::addCommand("DbgWatchView", "set",       cDbgWatchViewSet,       "watchView.set(expression)",           3, 3);
//    Con::addCommand("DbgWatchView", "update",    cDbgWatchViewUpdate,    "watchView.update(queryId, value)",    4, 4);
//    Con::addCommand("DbgWatchView", "edit",      cDbgWatchViewEdit,      "watchView.edit(newValue)",            3, 3);
//    Con::addCommand("DbgWatchView", "remove",    cDbgWatchViewRemove,    "watchView.remove()",                  2, 2);
//    Con::addCommand("DbgWatchView", "queryAll",  cDbgWatchViewQueryAll,  "watchView.queryAll()",                2, 2);
//    Con::addCommand("DbgWatchView", "clear",     cDbgWatchViewClear,     "watchView.clear()",                   2, 2);
//    Con::linkNamespaces("GuiArrayCtrl", "DbgWatchView");
// }
// 
// bool DbgWatchView::onWake()
// {
//    if (! Parent::onWake())
//       return false;
//    
//    //adjust the cellwidth to the maximum line length
//    AdjustCellSize();
//    mSize.set(1, mWatchList.size());
//    
//    return true;
// }
// 
// void DbgWatchView::AdjustCellSize()
// {
//    if (! bool(mFont))
//       return;
//    S32 maxWidth = 1;
//    for (U32 i = 0; i < mWatchList.size(); i++)
//    {
//       S32 cellWidth = 132 + mFont->getStrWidth(mWatchList[i].value) + 8;
//       maxWidth = getMax(maxWidth, cellWidth);
//    }
//    
//    mCellSize.set(maxWidth, mFont->getHeight() + 2);
//    setSize(mSize);
// }
// 
// void DbgWatchView::addWatch(const char *varName)
// {
//    if ((! varName) || (! varName[0])) return;
//    
//    //see if it's already in the list
//    for (U32 i = 0; i < mWatchList.size(); i++)
//    {
//       if (! dStricmp(mWatchList[i].variable, varName))
//       {
//          cellSelected(Point2I(0, i));
//          return;
//       }
//    }
//    
//    //it wasn't found, add it to the list
//    WatchRep newWatch;
//    dStrcpy(newWatch.variable, varName);
//    newWatch.id = mNextQueryID++;
//    newWatch.value[0] = '\0';
//    mWatchList.push_back(newWatch);
//    
//    //update the size()
//    AdjustCellSize();
//    setSize(Point2I(1, mWatchList.size()));
//    cellSelected(Point2I(0, mWatchList.size() - 1));
//    
//    //send the query
//    char buf[300];
//    const char *frame = Con::executef(1, "DbgStackGetFrame");
//    dSprintf(buf, sizeof(buf), "EVAL:%d %d %s", newWatch.id, dAtoi(frame), newWatch.variable);
//    Con::executef(2, "DbgRemoteSend", buf);
// }
// 
// void DbgWatchView::deleteWatch(const char *varName)
// {
//    if ((! varName) || (! varName[0])) return;
//    
//    //find it in the list
//    U32 cell;
//    for (cell = 0; cell < mWatchList.size(); cell++)
//    {
//       if (! dStricmp(mWatchList[cell].variable, varName))
//          break;
//    }
//    
//    //see if we found it
//    if (cell < mWatchList.size())
//    {
//       //send the message to the server
//       char buf[256];
//       dSprintf(buf, sizeof(buf), "dbgRemoveWatch(%s);", mWatchList[cell].variable);
//       Con::executef(2, "remoteDebugSend", buf);
//    
//       //delete the watch
//       mWatchList.erase(cell);
//       
//       //update the size
//       AdjustCellSize();
//       setSize(Point2I(1, mWatchList.size()));
//       mSelectedCell.set(-1, -1);
//    }
// }
// 
// void DbgWatchView::deleteCurrentWatch()
// {
//    if (mSelectedCell.y < 0 || mSelectedCell.y >= (S32)mWatchList.size())
//       return;
//    deleteWatch(mWatchList[mSelectedCell.y].variable);
// }
// 
// void DbgWatchView::updateWatch(S32 id, const char *value)
// {
//    if (! value)
//       return;
//    
//    //find it in the list
//    for (U32 i = 0; i < mWatchList.size(); i++)
//    {
//       if (mWatchList[i].id == id)
//       {
//          if (! dStrcmp(value, "\"\""))
//             dStrcpy(mWatchList[i].value, value);
//          else
//             dSprintf(mWatchList[i].value, sizeof(mWatchList[i].value), "\"%s\"", value);
//          break;
//       }
//    }
//    
//    //adjust the cellwidth to the maximum line length
//    AdjustCellSize();
//    mSize.set(1, mWatchList.size());
// }
// 
// void DbgWatchView::editCurrentWatch(const char *newValue)
// {
//    if (! newValue)
//       return;
//    if (mSelectedCell.y < 0 || mSelectedCell.y >= (S32)mWatchList.size())
//       return;
//    
//    //send the edit to the server - at the same time, it will generate a query
//    char valBuf[256];
//    char buf[512];
//    
//    //make a copy, because executing DbgStackGetFrame will change the contents of argv...
//    dStrcpy(valBuf, newValue);
//    
//    const char *frame = Con::executef(1, "DbgStackGetFrame");
//    dSprintf(buf, sizeof(buf), "EVAL:%d %d %s=%s", mWatchList[mSelectedCell.y].id, dAtoi(frame),
//                                                    mWatchList[mSelectedCell.y].variable,
//                                                    valBuf[0] ? valBuf : "\"\"");
//    Con::executef(2, "DbgRemoteSend", buf);
// }
// 
// void DbgWatchView::queryAll()
// {
//    //loop through the list
//    for (U32 i = 0; i < mWatchList.size(); i++)
//    {
//       char buf[300];
//       const char *frame = Con::executef(1, "DbgStackGetFrame");
//       dSprintf(buf, sizeof(buf), "EVAL:%d %d %s", mWatchList[i].id, dAtoi(frame), mWatchList[i].variable);
//       Con::executef(2, "DbgRemoteSend", buf);
//    }
// }
// 
// void DbgWatchView::clearWatches()
// {
//    //loop through the list
//    for (U32 i = 0; i < mWatchList.size(); i++)
//    {
//       //send the message to the server
//       char buf[256];
//       dSprintf(buf, sizeof(buf), "dbgRemoveWatch(%s);", mWatchList[i].variable);
//       Con::executef(2, "remoteDebugSend", buf);
//    }
//    
//    //clear the list
//    mWatchList.clear();
//    
//    //update the size
//    AdjustCellSize();
//    setSize(Point2I(1, mWatchList.size()));
//    mSelectedCell.set(-1, -1);
// }
// 
// void DbgWatchView::onMouseDown(const GuiEvent &event)
// {
//    if (! mActive)
//    {
//       Parent::onMouseDown(event);
//       return;
//    }
//    
//    Point2I pt = globalToLocalCoord(event.mousePoint);
//    bool doubleClick = (event.mouseClickCount > 1);
//    
//    //find out which cell was hit
//    Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
//    if(cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
//    {
//       Point2I prevSelected = mSelectedCell;
//       Parent::onMouseDown(event);
//       
//       //open the file view
//       //if (mSelectedCell.y == prevSelected.y && doubleClick)
//       //{
//       //   Con::executef(1, "DbgEditWatchDialog::open");
//       //}
//    }
//    else
//    {
//       Parent::onMouseDown(event);
//    }
// }
// 
// void DbgWatchView::onRenderCell(Point2I offset, Point2I cell, bool selected, bool)
// {
//    Point2I cellOffset = offset;
//    cellOffset.x += 4;
//    
//    //hilite the line if selected
//    if (selected)
//       dglDrawRectFill(RectI(cellOffset.x - 2, cellOffset.y - 3, mCellSize.x + 4, mCellSize.y + 6), mProfile->mFillColorHL);
//    
//    //draw the variable name clipped to 120 pix
//    char clippedBuf[256], *clipPtr;
//    dStrcpy(clippedBuf, mWatchList[cell.y].variable);
//    clipPtr = &clippedBuf[dStrlen(clippedBuf)];
//    bool finished = false;
//    while ((clipPtr > &clippedBuf[0]) && (! finished))
//    {
//       if (mFont->getStrWidth(clippedBuf) <= 120)
//       {
//          finished = true;
//       }
//       else
//       {
//          *--clipPtr = '\0';
//       }
//    }
//    dglSetBitmapModulation(mProfile->mFontColor);
//    dglDrawText(mFont, cellOffset, clippedBuf);
//    cellOffset.x += 128;
//       
//    //draw the value
//    dglSetBitmapModulation(mProfile->mFontColorHL);
//    dglDrawText(mFont, cellOffset, mWatchList[cell.y].value);
// }
// 
// // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// 
// static void cDbgCallStackViewFunctionCall(SimObject *obj, S32, const char **argv)
// {
//    DbgCallStackView *dbgCtrl = static_cast<DbgCallStackView*>(obj);
//    dbgCtrl->addFunctionCall(argv[2], dAtoi(argv[3]), argv[4]);
// }
// 
// static const char* cDbgCallStackViewGetFrame(SimObject *obj, S32, const char **)
// {
//    DbgCallStackView *dbgCtrl = static_cast<DbgCallStackView*>(obj);
//    return itoa(dbgCtrl->getFrameNumber());
// }
// 
// static void cDbgCallStackViewClear(SimObject *obj, S32, const char **)
// {
//    DbgCallStackView *dbgCtrl = static_cast<DbgCallStackView*>(obj);
//    dbgCtrl->clear();
// }
// 
// DbgCallStackView::DbgCallStackView()
// {
//    mSize.set(1, 0);
// }
// 
// void DbgCallStackView::consoleInit()
// {
//    Con::addCommand("DbgCallStackView", "add",        cDbgCallStackViewFunctionCall,    "callStack.add(file, line, function)",    5, 5);
//    Con::addCommand("DbgCallStackView", "getFrame",   cDbgCallStackViewGetFrame,        "callStack.getFrame()",                   2, 2);
//    Con::addCommand("DbgCallStackView", "clear",      cDbgCallStackViewClear,           "callStack.clear()",                      2, 2);
//    Con::linkNamespaces("GuiArrayCtrl", "DbgCallStackView");
// }
// 
// bool DbgCallStackView::onWake()
// {
//    if (! Parent::onWake())
//       return false;
//    
//    //adjust the cellwidth to the maximum line length
//    AdjustCellSize();
//    mSize.set(1, mCallStack.size());
//    
//    return true;
// }
// 
// void DbgCallStackView::AdjustCellSize()
// {
//    if (! bool(mFont))
//       return;
//    S32 maxWidth = 1;
//    for (U32 i = 0; i < mCallStack.size(); i++)
//    {
//       S32 cellWidth = mFont->getStrWidth(mCallStack[i].functionName) + 8;
//       maxWidth = getMax(maxWidth, cellWidth);
//    }
//    
//    mCellSize.set(maxWidth, mFont->getHeight() + 2);
//    setSize(mSize);
// }
// 
// void DbgCallStackView::addFunctionCall(const char *fileName, S32 lineNumber, const char *functionName)
// {
//    if ((! functionName) || (! functionName[0]))
//       return;
//    if ((! fileName) || (! fileName[0]))
//       return;
//    if (lineNumber <= 0)
//       return;
//    
//    CallStackRep newCall;
//    dSprintf(newCall.functionName, sizeof(newCall.functionName), "%s();", functionName);
//    dStrcpy(newCall.fileName, fileName);
//    newCall.lineNumber = lineNumber;
//    mCallStack.push_back(newCall);
//    
//    //update the size()
//    AdjustCellSize();
//    setSize(Point2I(1, mCallStack.size()));
//    mSelectedCell.set(1, 0);
// }
// 
// S32 DbgCallStackView::getFrameNumber()
// {
//    if (mSelectedCell.y < 0 || mSelectedCell.y >= (S32)mCallStack.size())
//       return 0;
//    return mCallStack.size() - 1 - mSelectedCell.y;
// }
// 
// void DbgCallStackView::clear()
// {
//    mCallStack.clear();
//    mSelectedCell.set(-1, -1);
//    
//    //update the size()
//    AdjustCellSize();
//    setSize(Point2I(1, mCallStack.size()));
// }
// 
// void DbgCallStackView::onMouseDown(const GuiEvent &event)
// {
//    if (! mActive)
//    {
//       Parent::onMouseDown(event);
//       return;
//    }
//    
//    bool doubleClick = (event.mouseClickCount > 1);
//    
//    Point2I prevSelected = mSelectedCell;
//    Parent::onMouseDown(event);
//       
//    //open the file view
//    if (mSelectedCell.y == prevSelected.y && doubleClick)
//    {
//       Con::executef(this, 4, "onCellSelect", mCallStack[mSelectedCell.y].fileName,
//                                           itoa(mCallStack[mSelectedCell.y].lineNumber),
//                                           mSelectedCell.y == 0 ? "true" : "false");
//    }
// }
// 
// void DbgCallStackView::onRenderCell(Point2I offset, Point2I cell, bool selected, bool)
// {
//    Point2I cellOffset = offset;
//    cellOffset.x += 4;
//    
//    //hilite the line if selected
//    //if (selected)
//    //   dglDrawRectFill(RectI(cellOffset.x - 2, cellOffset.y - 3,
//    //                           mCellSize.x, mCellSize.y), mFillColorHL);
//    
//    //draw the function name
//    dglSetBitmapModulation(selected ? mProfile->mFontColorHL : mProfile->mFontColor);
//    dglDrawText(mFont, cellOffset, mCallStack[cell.y].functionName);
// }
// 
