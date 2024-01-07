//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include "dgl/gFont.h"
#include "dgl/gBitmap.h"
#include "Math/mRect.h"

// static HDC fontHDC = NULL;
// static HBITMAP fontBMP = NULL;

// void createFontInit(void);
// void createFontShutdown(void);
// void CopyCharToBitmap(GBitmap *pDstBMP, HDC hSrcHDC, const RectI &r);
// 
void createFontInit()
{
//    fontHDC = CreateCompatibleDC(NULL);
//    fontBMP = CreateCompatibleBitmap(fontHDC, 256, 256);
}
// 
void createFontShutdown()
{
//    DeleteObject(fontBMP);
//    DeleteObject(fontHDC);
}
// 
// void CopyCharToBitmap(GBitmap *pDstBMP, HDC hSrcHDC, const RectI &r)
// {
//    for (S32 i = r.point.y; i < r.point.y + r.extent.y; i++)
//    {
//       for (S32 j = r.point.x; j < r.point.x + r.extent.x; j++)
//       {
//          COLORREF color = GetPixel(hSrcHDC, j, i);
//          if (color)
//             *pDstBMP->getAddress(j, i) = 255;
//          else
//             *pDstBMP->getAddress(j, i) = 0;
//       }
//    }
// }

GFont *createFont(const char *name, S32 size)
{
   return(NULL);
   
//    if(!name)
//       return NULL;
//    if(size < 1)
//       return NULL;
//    
// 
//    HFONT hNewFont = CreateFont(size,0,0,0,0,0,0,0,0,0,0,0,0,name);
//    if(!hNewFont)
//       return NULL;
//    
//    GFont *retFont = new GFont;
//    GBitmap scratchPad(256, 256);
//    
//    TEXTMETRIC textMetric;
// 	COLORREF backgroundColorRef = RGB(  0,   0,   0);
// 	COLORREF foregroundColorRef = RGB(255, 255, 255);
// 	
// 	SelectObject(fontHDC, fontBMP);
// 	SelectObject(fontHDC, hNewFont);
// 	SetBkColor(fontHDC, backgroundColorRef);
// 	SetTextColor(fontHDC, foregroundColorRef);
// 	GetTextMetrics(fontHDC, &textMetric);
// 
//    RectI clip;
//    for(S32 i = 32; i < 256; i++)
//    {
//    	SIZE size;
//       char buf[4];
//       buf[0] = ' ';
// 		buf[1] = (char)i;
//       buf[2] = ' ';
//       buf[3] = '\0';
// 
// 		TextOut(fontHDC, 0, 0, buf, 3);
// 		GetTextExtentPoint32(fontHDC, buf, 3, &size);
//       RectI r(0, 0, size.cx + 1, size.cy + 1);
//       CopyCharToBitmap(&scratchPad, fontHDC, r);
//       
// 		// now clip the raw bitmap so we don't waste any space
// 		clip.point.x = 256;
// 		clip.point.y = 256;
// 		clip.extent.x = -257;
// 		clip.extent.y = -257;
// 		
// 		S32 row, col;
//       bool found = FALSE;
// 		Point2I upperL(256, 256);
//       Point2I lowerR(-1, -1);
//       
// 		for (row = 0; row < size.cy; row++)
// 			for (col = 0; col < size.cx; col++)
// 			{
// 				if (*scratchPad.getAddress(col, row) == 255)
// 				{
// 					found = TRUE;
// 					if (upperL.x > col)
//                   upperL.x = col;
//                if (upperL.y > row)
//                   upperL.y = row;
//                if (lowerR.x < col)
//                   lowerR.x = col;
//                if (lowerR.y < row)
//                   lowerR.y = row;
// 				}
// 			}
// 
// 		if (!found)
// 		{
// 			// bitmap is blank, probably a space, leave the width alone
// 			// but truncate height to one line
// 			clip.point.x = 0; 
// 			clip.point.y = 0x7fffffff;
// 			clip.extent.x = size.cx / 3 + 1;
// 			clip.extent.y = 0;
// 		}
//       else
//       {
//          clip.point = upperL;
//          clip.point.x -= 1;
//          if (clip.point.x < 0)
//             clip.point.x = 0;
//          clip.point.y -= 1;
//          if (clip.point.y < 0)
//             clip.point.y = 0;
//          clip.extent.set(lowerR.x - upperL.x + 2, lowerR.y - upperL.y + 2);
//       }
// 		retFont->insertBitmap(i, &scratchPad, &clip);
//    }
//    retFont->pack(textMetric.tmHeight, textMetric.tmAscent);
//    //clean up local vars      
//    DeleteObject(hNewFont);
//    
//    return retFont;
}
