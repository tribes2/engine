//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/guiControl.h"
#include "console/consoleTypes.h"
#include "Core/fileStream.h"
#include "dgl/dgl.h"
#include "GUI/guiScrollCtrl.h"
#include "terrain/terrData.h"
#include <math.h>

//-----------------------------------------------------------------------------

class CompTest : public GuiScrollContentCtrl
{
   private:
      typedef GuiControl Parent;
      S32   mHeights[65536];
      S32   mHeightMin;
      S32   mHeightMax;
      void calcMinMax();
      
      U16 mTerrainHeights[65536];
      U32 mCurrentIndex;
      Point2I mHilbertPos;
      
   public:
   
      enum {
        UP,
        LEFT,
        DOWN,
        RIGHT
      };
      
      DECLARE_CONOBJECT(CompTest);

      static S32 smShift;
      static S32 smHisto;
      static S32 smSaveHiLo;
        
      static void consoleInit();
      void onRender(Point2I offset, const RectI & updateRect, GuiControl * firstResponder);
      void openFile(StringTableEntry fileName);
      void saveFile(StringTableEntry fileName);
      void buildRep(StringTableEntry type);
      void windowCompress();

      TerrainBlock * getTerrainObj();
 
      // hilbert curve stuff
      void hilbertFill(S32 level, S32 direction = UP);
      void move(S32 direction);
};

S32 CompTest::smShift;
S32 CompTest::smHisto;
S32 CompTest::smSaveHiLo;

IMPLEMENT_CONOBJECT(CompTest);

//-----------------------------------------------------------------------------

static void cCompTestOpenFile(SimObject * obj, S32, const char ** argv)
{
   static_cast<CompTest*>(obj)->openFile(argv[2]);
}

static void cCompTestSaveFile(SimObject * obj, S32, const char ** argv)
{
   static_cast<CompTest*>(obj)->saveFile(argv[2]);
}

static void cCompTestBuildRep(SimObject * obj, S32, const char ** argv)
{
   static_cast<CompTest*>(obj)->buildRep(argv[2]);
}

void CompTest::consoleInit()
{
   Con::addCommand("CompTest", "buildRep", cCompTestBuildRep, "compTest.buildRep(type)", 3, 3);
   Con::addCommand("CompTest", "openFile", cCompTestOpenFile, "compTest.openFile(filename)", 3, 3);
   Con::addCommand("CompTest", "saveFile", cCompTestSaveFile, "compTest.saveFile(filename)", 3, 3);

   Con::addVariable("CompTestShift", TypeS32, &smShift);
   Con::addVariable("CompTestHisto", TypeS32, &smHisto);
   Con::addVariable("CompTestSaveHiLo", TypeS32, &smSaveHiLo);
}

//-----------------------------------------------------------------------------
// BEGIN Hilbert curve stuff
void CompTest::move(S32 direction)
{
   switch(direction)
   {
      case LEFT:
         mHilbertPos.x--;
         break;
      case RIGHT:
         mHilbertPos.x++;
         break;
      case UP:
         mHilbertPos.y--;
         break;
      case DOWN:
         mHilbertPos.y++;
         break;
   }
   
   AssertFatal(mCurrentIndex < 256 * 256, "Doh!");
   AssertFatal(mHilbertPos.x + 256 * mHilbertPos.y < 256 * 256, "Blah!");
   
   // copy at the hilbert position
   mHeights[mCurrentIndex++] = mTerrainHeights[mHilbertPos.x + 256 * mHilbertPos.y];
}

void CompTest::hilbertFill(S32 level, S32 direction)
{
  if (level==1) {
    switch (direction) {
    case LEFT:
      move(RIGHT);      /* move() could draw a line in... */
      move(DOWN);       /* ...the indicated direction */
      move(LEFT);
      break;
    case RIGHT:
      move(LEFT);
      move(UP);
      move(RIGHT);
      break;
    case UP:
      move(DOWN);
      move(RIGHT);
      move(UP);
      break;
    case DOWN:
      move(UP);
      move(LEFT);
      move(DOWN);
      break;
    } /* switch */
  } else {
    switch (direction) {
    case LEFT:
      hilbertFill(level-1,UP);
      move(RIGHT);
      hilbertFill(level-1,LEFT);
      move(DOWN);
      hilbertFill(level-1,LEFT);
      move(LEFT);
      hilbertFill(level-1,DOWN);
      break;
    case RIGHT:
      hilbertFill(level-1,DOWN);
      move(LEFT);
      hilbertFill(level-1,RIGHT);
      move(UP);
      hilbertFill(level-1,RIGHT);
      move(RIGHT);
      hilbertFill(level-1,UP);
      break;
    case UP:
      hilbertFill(level-1,LEFT);
      move(DOWN);
      hilbertFill(level-1,UP);
      move(RIGHT);
      hilbertFill(level-1,UP);
      move(UP);
      hilbertFill(level-1,RIGHT);
      break;
    case DOWN:
      hilbertFill(level-1,RIGHT);
      move(UP);
      hilbertFill(level-1,DOWN);
      move(LEFT);
      hilbertFill(level-1,DOWN);
      move(DOWN);
      hilbertFill(level-1,LEFT);
      break;
    }
  } 
}

// END Hilbert curve stuff
//------------------------------------------------------------------------------

static S32 QSORT_CALLBACK sortHisto(const void * a, const void * b)
{
   S16 intA = *((S16*)a);
   S16 intB = *((S16*)b);
   return(intA == intB ? 0 : intA > intB ? -1 : 1);
}

//-----------------------------------------------------------------------------

TerrainBlock * CompTest::getTerrainObj()
{
   // get the terrain obj
   SimObject* obj = Sim::findObject("Terrain");
   if(!obj)
      return(0);
   TerrainBlock * terrain = static_cast<TerrainBlock *>(obj);
   return(terrain);
}

//-----------------------------------------------------------------------------

void CompTest::windowCompress()
{
   U16 window[32];
   S32 size = 32 * 16;
   S32 windowCount = 0;
   S32 i;
   for(i = 0; i < 65536 && windowCount < 32; i++)
   {
      U16 h = mHeights[i];
      S32 j;
      for(j = 0; j < windowCount;j++)
         if(window[j] == h)
            break;
      if(j == windowCount)
         window[windowCount++] = h;
   }
   S32 runStart = 0;
   S32 runCount = 0;
   S32 winPos = 0;
   for(i = 0; i < 65536; i++)
   {
      U16 h = mHeights[i];
      S32 j;
      for(j = 0; j < windowCount; j++)
         if(h == window[j])
            break;
      if(j == windowCount)
      {
         size += runCount * 5 + 32;
         window[winPos++] = h;
         if(winPos == windowCount)
            winPos = 0;
         runStart = i+1;
         runCount = 0;
      }
      else
         runCount++;
   }
   Con::printf("Compressed size: %d", size >> 3);
}

void CompTest::buildRep(StringTableEntry type)
{
   TerrainBlock * terrain = getTerrainObj();
   if(!terrain)
      return;
      
   U16 *htcpy = terrain->heightMap;

   for(S32 y = 0; y < 256; y++)
      for(S32 x = 0; x < 256; x++)
         mTerrainHeights[y * 256 + x] = htcpy[y * 257 + x];
   
   // check for shift
   if(smShift)
      for(U32 i = 0; i < 256<<8; i++)
         mTerrainHeights[i] >>= smShift;
   
   // just copy
   if(!dStricmp(type, "baseline"))
   {
      for(U32 i = 0; i < 256 * 256; i++)
         mHeights[i] = mTerrainHeights[i];
   }
   // hilbert curve
   else if(!dStricmp(type, "hilbert"))
   {
      mHilbertPos.x = mHilbertPos.y = 0;
      mHeights[0] = mTerrainHeights[0];
      mCurrentIndex = 1;
      hilbertFill(8,UP);
   }
   // MarkF delta type
   else if(!dStricmp(type, "delta"))
   {
      for(U32 y = 0; y < 256; y++)
         for(U32 x = 0; x < 256; x++)
         {
            U16 h1 = mTerrainHeights[x + (y<<8)];
            U16 h2 = mTerrainHeights[((x+1) % 256) + (y << 8)];
            U16 h3 = mTerrainHeights[x + (((y+1) % 256) << 8)];
            U16 h4 = mTerrainHeights[((x+1) % 256) + (((y+1) % 256) << 8)];
            mHeights[x + (y<<8)] = h4 - h1 + (h2 - h1) + (h3 - h1);
         }
   }
   else if(!dStricmp(type, "delta2"))
   {
      U16 prevHeight = 0;
      for(U32 i = 0; i < 65536; i++)
      {
         mHeights[i] = mTerrainHeights[i] - prevHeight;
         prevHeight = mTerrainHeights[i];
      }
   }
   else if(!dStricmp(type, "delta3"))
   {
      mHeights[0] = mTerrainHeights[0];
      S32 delta = mTerrainHeights[0] / 2;
      S32 deltaPrev = mTerrainHeights[0] - delta;
      for(U32 i = 0; i < 65536; i++)
      {
         S32 ph = mTerrainHeights[i-1];
         S32 h = mTerrainHeights[i];

         mHeights[i] = h - ph - ((delta + deltaPrev) >> 1);
         deltaPrev = delta;
         delta = h - ph;
      }
      /*for(U32 y = 0; y < 256; y++)
      {
         U32 prevy = (y - 1) & 0xFF;
         
         U16 *prevRow = mTerrainHeights + prevy * 256;
         U16 *curRow = mTerrainHeights + y * 256;
         S16 *dest = mHeights  + y *256;
         for(U32 x = 0; x < 256; x++)
            dest[x] = curRow[x] - prevRow[x];
      }*/
/*      U16 prevDelta = 0;
      U16 prevHeight = 0;
      for(U32 i = 0; i < 65536; i++)
      {
         U16 h = mTerrainHeights[i];
         mHeights[i] = h - (prevHeight + prevDelta);
         prevHeight = h;
         prevDelta = mHeights[i];
      }
      prevHeight = 0;
      prevDelta = 0;
      for(U32 i = 0; i < 65536; i++)
      {
         U16 newDelta = mHeights[i];
         mHeights[i] = prevHeight + prevDelta + newDelta;
         prevDelta = newDelta;
         prevHeight = mHeights[i];
      }*/
   }
   else
      return;
   
   // check for histo display
   windowCompress();
   if(smHisto)
   {
      S16 *hist = new S16[256*256];
      U32 i;
      for(i = 0; i < 256*256; i++)
         hist[i] = 0;
      for(i = 0; i < 256*256; i++)
         hist[U16(mHeights[i])]++;
   
      dQsort(hist, 256*256, 2, sortHisto);
      for(i = 0; i < 256*256; i++)
         mHeights[i] = hist[i];
         
      U32 used = 0;
      for(i = 0; i < 256*256; i++)
         if(mHeights[i])
            used++;
      delete [] hist;
            
      Con::printf("compTest => number of unique histogram entries: %d", used);
      F32 sum = 0;
      for(i = 0; i < 65536; i++)
      {
         if(hist[i] == 0)
            continue;
         F32 p = hist[i] / 65536.0f;
         sum += p * log(p) / log(2.0);
      }
      Con::printf("Compressability sum: %f", -sum);
   }
   calcMinMax();
   
   Con::printf("compTest => min/max terrain height: %f / %f", 
      F32(mHeightMin) * 0.03125, F32(mHeightMax) * 0.03125);
}


//-----------------------------------------------------------------------------

void CompTest::calcMinMax()
{
   mHeightMin = 32000;
   mHeightMax = -32000;
      
   for(U32 i = 0; i < (256*256); i++)
   {
      if(mHeights[i] < mHeightMin)
         mHeightMin = mHeights[i];
      if(mHeights[i] > mHeightMax)
         mHeightMax = mHeights[i];
   }
}

//-----------------------------------------------------------------------------

void CompTest::openFile(StringTableEntry fileName)
{
   if(!fileName || !dStrlen(fileName))
      return;
      
   FileStream file;
   if(!file.open(fileName, FileStream::Read))
      return;

   for(U32 i = 0; i < (256*256); i++)
   {
      if(!file.read(&mHeights[i]))
         return;
   }
   calcMinMax();
}

void CompTest::saveFile(StringTableEntry fileName)
{
   if(!fileName || !dStrlen(fileName) || !mHeights)
      return;
      
   FileStream file;
   file.open(fileName, FileStream::ReadWrite);
   U32 i;
   for(i = 0; i < (256*256); i++)
      file.write(mHeights[i]);
   file.close();
   
   if(smSaveHiLo)
   {
      // save lo data
      file.open("lo_byte.out", FileStream::ReadWrite);
      for(i = 0; i < (256*256); i++)
      {
         U8 out = mHeights[i];
         file.write(out);
      }
      // save high data
      file.open("hi_byte.out", FileStream::ReadWrite);
      for(i = 0; i < (256*256); i++)
      {
         U8 out = mHeights[i] >> 8;
         file.write(out);
      }
   }
}

//-----------------------------------------------------------------------------

void CompTest::onRender(Point2I offset, const RectI & updateRect, GuiControl * firstResponder)
{
   firstResponder;
   if(!mHeights)
      return;

   dglSetViewport(updateRect);
   
   GuiControl * parent = getParent();
   if(!parent)
      return;
      
   Point2I parentOff = parent->localToGlobalCoord(Point2I(0,0));
   offset -= parentOff;
   
   // clear the background only
//   glClear(GL_COLOR_BUFFER_BIT);
   glBegin(GL_QUADS);
   glColor3f(0.2, 0.2, 0.2);
   glVertex2f(updateRect.point.x, updateRect.point.y);
   glVertex2f(updateRect.point.x + updateRect.extent.x, updateRect.point.y);
   glVertex2f(updateRect.point.x + updateRect.extent.x, updateRect.point.y + updateRect.extent.y);
   glVertex2f(updateRect.point.x, updateRect.point.y + updateRect.extent.y);
   glEnd();
   
   // draw forest, draw!
   glBegin(GL_LINES);
   glColor3f(0.8,0,0);
   
   // check for neg vals
   if(mHeightMin < 0)
   {
      U16 range = mHeightMax > -mHeightMin ? mHeightMax : -mHeightMin;
   
      for(U32 i = 0; i < updateRect.extent.x; i++)
      {
         U32 index = i + -offset.x;
         if(i >= 256 * 256)
            break;
            
         S16 curHeight = mHeights[index];
         
         F32 bottom = updateRect.point.y + updateRect.extent.y / 2;
         glVertex2f(i, bottom);
         
         F32 scale = F32(curHeight) / F32(range);
         
         F32 top = bottom - F32(updateRect.extent.y / 2) * scale;
         glVertex2f(i, top);
      }
   }
   else
   {
      U16 range = mHeightMax - mHeightMin;
      
      F32 maxScale = 0.f;
      
      for(U32 i = 0; i < updateRect.extent.x; i++)
      {
         U32 index = i + -offset.x;
         if(index >= 256 * 256)
            break;
            
         U16 curHeight = mHeights[index];
         
         F32 bottom = updateRect.point.y + updateRect.extent.y;
         glVertex2f(updateRect.point.x + i, bottom);
         
         F32 scale = (F32(curHeight) - F32(mHeightMin)) / F32(range);
         
         if(scale > maxScale)
            maxScale = scale;
            
         F32 top = bottom - F32(updateRect.extent.y) * scale;
         glVertex2f(updateRect.point.x + i, top);
      }
   }
   glEnd();
}
