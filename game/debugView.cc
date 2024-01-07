//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "gui/guiTSControl.h"
#include "game/gameConnection.h"
#include "game/player.h"
#include "game/DebugView.h"

IMPLEMENT_CONOBJECT(DebugView);

DebugView::DebugView()
{
   for (int i = 0; i < MaxTextLines; i++)
      mTextLines[i][0] = '\0';
}

static void cDebugAddLine(SimObject *obj, S32, const char **argv)
{
   Point3F start(0, 0, 0);
   Point3F end(0, 0, 0);
   ColorF color(0, 0, 0, 1.0f);
   int numArgsRead;
   
   //read the args in
   numArgsRead = dSscanf(argv[2], "%f %f %f", &start.x, &start.y, &start.z);
   if (numArgsRead != 3)
   {
      Con::printf("%s() - invalid start point.", argv[0]);
      return;
   }
   
   numArgsRead = dSscanf(argv[3], "%f %f %f", &end.x, &end.y, &end.z);
   if (numArgsRead != 3)
   {
      Con::printf("%s() - invalid end point.", argv[0]);
      return;
   }
   
   numArgsRead = dSscanf(argv[4], "%f %f %f", &color.red, &color.green, &color.blue);
   if (numArgsRead != 3)
   {
      Con::printf("%s() - invalid color.", argv[0]);
      return;
   }
   
   DebugView *dv = static_cast<DebugView*>(obj);
   dv->addLine(start, end, color);
}

static void cDebugClearLines(SimObject *obj, S32, const char **)
{
   DebugView *dv = static_cast<DebugView*>(obj);
   dv->clearLines();
}

static void cDebugViewSetText(SimObject *obj, S32 argc, const char **argv)
{
   DebugView *dv = static_cast<DebugView*>(obj);
	ColorF color(0.0f, 0.0f, 0.0f, 1.0f);
	bool setColor = false;
	if (argc >= 5)
	{
	   int numArgsRead = dSscanf(argv[4], "%f %f %f", &color.red, &color.green, &color.blue);
	   if (numArgsRead == 3)
			setColor = true;
	}
   dv->setTextLine(dAtoi(argv[2]), argv[3], setColor ? &color : NULL);
}

static void cDebugViewClearText(SimObject *obj, S32 argc, const char **argv)
{
   DebugView *dv = static_cast<DebugView*>(obj);
   int lineNum = -1;
   if (argc == 3)
      lineNum = dAtoi(argv[2]);
   dv->clearTextLine(lineNum);
}

void DebugView::consoleInit()
{
   Con::addCommand("DebugView", "addLine", cDebugAddLine, "debugView.addLine(startPt, endPt, color)", 5, 5);
   Con::addCommand("DebugView", "clearLines", cDebugClearLines, "debugView.clearLines()", 2, 2);
   
   Con::addCommand("DebugView", "setText", cDebugViewSetText, "debugView.SetText(line, text [, colorF])", 4, 5);
   Con::addCommand("DebugView", "clearText", cDebugViewClearText, "debugView.ClearText(<line>)", 2, 3);
}

void DebugView::addLine(const Point3F &start, const Point3F &end, const ColorF &color)
{
   DebugLine newLine(start, end, color);
   mLines.push_back(newLine);
}

void DebugView::clearLines()
{
   mLines.clear();
}

void DebugView::setTextLine(int line, const char *text, ColorF *color)
{
   if (line < 0 || line >= MaxTextLines || !text)
      return;
   dStrncpy(&mTextLines[line][0], text, MaxTextLineLength);
   mTextLines[line][MaxTextLineLength] = '\0';
	
	if (!color)
		mTextColors[line] = mProfile->mFontColor;
	else
		mTextColors[line] = *color;
}

void DebugView::clearTextLine(int line)
{
   if (line < 0)
   {
      for (int i = 0; i < MaxTextLines; i++)
         mTextLines[i][0] = '\0';
   }
   else if (line < MaxTextLines)
      mTextLines[line][0] = '\0';
}

void DebugView::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   S32 i;
#if defined(DEBUG) || defined(INTERNAL_RELEASE)
   GuiTSCtrl *tsCtrl;
   if (! Sim::findObject("PlayGui", tsCtrl))
   {
      Con::printf("DebugView failed - unable to find TS ctrl.");
      return;
   }  
   
   //draw the lines first
   for (i = 0; i < mLines.size(); i++)
   { 
      //project the line to the screen
      Point3F startPos, endPos;
      if (tsCtrl->project(mLines[i].start, &startPos) && tsCtrl->project(mLines[i].end, &endPos))
      {
         glBegin(GL_LINES);
         glColor4f(mLines[i].color.red, mLines[i].color.green, mLines[i].color.blue, 1.0f);
         glVertex2i((S32)startPos.x, (S32)startPos.y);
         glVertex2i((S32)endPos.x, (S32)endPos.y);
         glEnd();
      }
   }
   
   //draw the task above each player's head
   SimGroup *g = Sim::getClientGroup();
   SimGroup::iterator j;
   for (j = g->begin(); j != g->end(); j++)
   {
      GameConnection *client = static_cast<GameConnection*>(*j);
      Player *player = NULL;
      if (! client->getControlObject())
         continue;
         
      player = dynamic_cast<Player*>(client->getControlObject());   
      if (! player)
         continue;
         
      //draw a test string above everyone's head
      Point3F playerPos;
      MatrixF const& tempTransform = player->getTransform();
      tempTransform.getColumn(3, &playerPos);
      playerPos.z += 1.7f;
      Point3F textPos;
      if (tsCtrl->project(playerPos, &textPos))
      {
         //const char *textStr = client->getDataField("objective", NULL);
         const char *textStr = Con::executef(2, "aiGetTaskDesc", avar("%d", client->getId()));
         if (!textStr || !textStr[0])
            textStr = "Shoot Me!";
			if ((textStr[0] == 'E' || textStr[0] == 'F') && textStr[1] == ':')
			{
				if (textStr[0] == 'E')
		         dglSetBitmapModulation(ColorF(1.0, 0.0, 0.0, 1.0));
				else
		         dglSetBitmapModulation(ColorF(0.0, 1.0, 0.0, 1.0));
	         dglDrawText(mFont, Point2I(textPos.x, textPos.y), &textStr[2]);
			}
			else
			{
	         dglSetBitmapModulation(mProfile->mFontColor);
	         dglDrawText(mFont, Point2I(textPos.x, textPos.y), textStr);
			}
      }
   }
   
#endif

   //draw the text - for final release, this is the only thing to be rendered
   Point2I textOffset = offset;
   for (i = 0; i < MaxTextLines; i++)
   {
	   dglSetBitmapModulation(mTextColors[i]);
      if (mTextLines[i][0] != '\0')
         dglDrawText(mFont, textOffset, mTextLines[i]);
      textOffset.y += mFont->getHeight();
   }
   
   renderChildControls(offset, updateRect, firstResponder);
}


