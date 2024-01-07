//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformMacCarb/platformMacCarb.h"
#include "dgl/gFont.h"
#include "dgl/gBitmap.h"
#include "Math/mRect.h"
#include "console/console.h"

//#include <Fonts.h>

static GWorldPtr fontgw = NULL;
static Rect fontrect = {0,0,256,256};
static short fontdepth = 32;
static U8 rawmap[256*256];


void createFontInit(void);
void createFontShutdown(void);
S32 CopyCharToRaw(U8 *raw, PixMapHandle pm, const Rect &r);

// !!!!! TBD this should be returning error, or raising exception...
void createFontInit()
{
      OSErr err = NewGWorld(&fontgw, fontdepth, &fontrect, NULL, NULL, keepLocal);
      AssertWarn(err==noErr, "Font system failed to initialize GWorld.");
}
 
void createFontShutdown()
{
   DisposeGWorld(fontgw);
   fontgw = NULL;
}

U8 ColorAverage8(RGBColor &rgb)
{
   return ((rgb.red>>8) + (rgb.green>>8) + (rgb.blue>>8)) / 3;
}

S32 CopyCharToRaw(U8 *raw, PixMapHandle pmh, const Rect &r)
{
   // walk the pixmap, copying into raw buffer.
   // we want bg black, fg white, which is opposite 'sign' from the pixmap (bg white, with black text)
   
//   int off = GetPixRowBytes(pmh);
//   U32 *c = (U32*)GetPixBaseAddr(pmh);
//   U32 *p;

   RGBColor rgb;

   S32 i, j;
   U8 val;
   S32 lastRow = -1;
   
   for (i = r.left; i <= r.right; i++)
   {
      for (j = r.top; j <= r.bottom; j++)
      {
//         p = (U32*)(((U8*)c) + (j*off)); // since rowbytes is bytes not pixels, need to convert to byte * before doing the math...
         val = 0;
//         if (((*p)&0x00FFFFFF)==0) // then black pixel in current port, so want white in new buffer.
         GetCPixel(i, j, &rgb);
         if ((ColorAverage8(rgb)>>2) < 2) // get's us some grays with a small slop factor.
         {
            val = 255;
            lastRow = j; // track the last row in the rect that we actually saw an active pixel, finding descenders basically...
         }
         raw[i + (j<<8)] = val;
//         p++;
      }
   }
   
   return(lastRow);
}

GFont *createFont(const char *name, S32 size)
{
   if(!name)
      return NULL;
   if(size < 1)
      return NULL;

   short fontid;
   GetFNum(str2p(name), &fontid);
   if (fontid == 0)
   {
      Con::errorf(ConsoleLogEntry::General,"Error creating font -- it doesn't exist: %s %d",name, size);
      return(NULL);
   }

   Boolean aaWas;
   S16 aaSize;
   CGrafPtr savePort;
   GDHandle saveGD;
   GetGWorld(&savePort, &saveGD);
   
   aaWas = IsAntiAliasedTextEnabled(&aaSize);
   if (aaWas)
      SetAntiAliasedTextEnabled(0, 0);
   
   RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF};
   RGBColor black = {0, 0, 0};
   PixMapHandle pmh;

   SetGWorld(fontgw, NULL);
   PenNormal(); // reset pen attribs.
   // shouldn't really need to do this, right?
   RGBBackColor(&white);
   RGBForeColor(&black);

   // set proper font.
   TextSize(size);
   TextFont(fontid);
   TextMode(srcCopy);

   // get font info
     int cx, cy, ry, my;
   FontInfo fi;
   GetFontInfo(&fi); // gets us basic glyphs.
   cy = fi.ascent + fi.descent + fi.leading; // !!!! HACK.  Not per-char-specific.
   
   pmh = GetGWorldPixMap(fontgw);

   GFont *retFont = new GFont;

   Rect b = {0,0,0,0};
   for(S32 i = 32; i < 256; i++)
   {
      // clear the port.
      EraseRect(&fontrect);
      // reset position to left edge, bottom of line for this font style.
      MoveTo(0,cy);
      // draw char & calc its pixel width.
      DrawChar(i);
      cx = CharWidth(i);      

      if (!LockPixels(pmh))
      {
         UpdateGWorld(&fontgw, fontdepth, &fontrect, NULL, NULL, keepLocal);
         pmh = GetGWorldPixMap(fontgw);
         // for now, assume we're good to go... TBD!!!!
         LockPixels(pmh);

         MoveTo(0,fi.ascent); // reset position to left edge, bottom of line for this font style.
         DrawChar(i);
         cx = CharWidth(i);      
      }

      b.right = cx+1;
      b.bottom = cy+2; // in case descenders drop too low, we want to catch the chars.
      ry = CopyCharToRaw(rawmap, pmh, b);

      UnlockPixels(pmh);

      if (ry<0) // bitmap was blank.
      {
         Con::printf("Blank character %c", i);
         if (cx)
            retFont->insertBitmap(i, rawmap, 0, 0, 0, 0, 0, cx);
      }
      else
      {
         retFont->insertBitmap(i, rawmap, 256, cx+1, cy+2 /*ry*/, 0, fi.ascent+fi.descent, cx+1);
      }
   }

   retFont->pack(cy, fi.ascent);

//   if (actualChars==0)
//      Con::errorf(ConsoleLogEntry::General,"Error creating font: %s %d",name, size);

   //clean up local vars
   if (aaWas)
      SetAntiAliasedTextEnabled(aaWas, aaSize);
   SetGWorld(savePort, saveGD);
   
   return retFont;
}
