//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/guiChannelVectorCtrl.h"
#include "GUI/channelVector.h"
#include "GUI/guiScrollCtrl.h"
#include "dgl/dgl.h"

IMPLEMENT_CONOBJECT(GuiChannelVectorCtrl);


//--------------------------------------------------------------------------
void GuiChannelVectorCtrl::lineInserted(const U32 arg)
{
   AssertFatal(mMessageVector != NULL, "Should not be here unless we're attached!");

   GuiScrollCtrl* pScroll = NULL;
   GuiControl* pParent = getParent();
   if (pParent) {
      GuiControl* pGrandParent = pParent->getParent();
      if (pGrandParent)
         pScroll = dynamic_cast<GuiScrollCtrl*>(pGrandParent);
   }

   bool scrollToBottom = false;
   if (pScroll != NULL)
      scrollToBottom = pScroll->getCurrVPos() == 1.0;

   mSpecialMarkers.insert(arg);
   createSpecialMarkers(mSpecialMarkers[arg], mMessageVector->getLine(arg).message);

   const ChannelVector::SpecialMarkers &tags = ((ChannelVector *) mMessageVector)->getLineTags(arg);

   if (tags.numSpecials)
   {
      U32 n = mSpecialMarkers[arg].numSpecials+tags.numSpecials;
      SpecialMarkers::Special *s = new SpecialMarkers::Special[n];
	
      for (U32 i = 0, j = 0, k = 0; k < n; ++k)
      {
         if (i < mSpecialMarkers[arg].numSpecials &&
             (j == tags.numSpecials ||
              mSpecialMarkers[arg].specials[i].start <= tags.specials[j].start))
            s[k] = mSpecialMarkers[arg].specials[i++];
         else
         {
            s[k].specialType = tags.specials[j].specialType;
            s[k].start = tags.specials[j].start;
            s[k].end = tags.specials[j].end;
            ++j;
         }
      }
      mSpecialMarkers[arg].numSpecials = n;
      delete [] mSpecialMarkers[arg].specials;
      mSpecialMarkers[arg].specials = s;
   }

   mLineWrappings.insert(arg);
   createLineWrapping(mLineWrappings[arg], mMessageVector->getLine(arg).message);

   mLineElements.insert(arg);
   createLineElement(mLineElements[arg], mLineWrappings[arg], mSpecialMarkers[arg]);

   U32 numLines = 0;
   for (U32 i = 0; i < mLineWrappings.size(); i++) {
      // We need to rebuild the physicalLineStart markers at the same time as
      //  we find out how many of them are left...
      mLineElements[i].physicalLineStart = numLines;
      
      numLines += mLineWrappings[i].numLines;
   }

   U32 newHeight = (mProfile->mFont->getHeight() + mLineSpacingPixels) * getMax(numLines, U32(1));
   resize(mBounds.point, Point2I(mBounds.extent.x, newHeight));

   if (arg == mSpecialMarkers.size() - 1 && scrollToBottom == true)
      pScroll->scrollTo(0, 1);
}


//--------------------------------------------------------------------------
void GuiChannelVectorCtrl::onRender(Point2I      offset,
                                    const RectI& updateRect,
                                    GuiControl*fr)
{
   GuiControl::onRender(offset, updateRect,fr);
   if (isAttached()) {
      U32 linePixels = mProfile->mFont->getHeight() + mLineSpacingPixels;
      U32 currLine   = 0;
      for (U32 i = 0; i < mMessageVector->getNumLines(); i++) {

         TextElement* pElement = mLineElements[i].headLineElements;
         ColorI lastColor = mProfile->mFontColor;

         dglSetBitmapModulation(lastColor);
         while (pElement != NULL) {
            Point2I localStart(pElement == mLineElements[i].headLineElements ? 0 : mLineContinuationIndent, currLine * linePixels);

            Point2I globalCheck  = localToGlobalCoord(localStart);
            U32 globalRangeStart = globalCheck.y;
            U32 globalRangeEnd   = globalCheck.y + mProfile->mFont->getHeight();
            if (globalRangeStart > updateRect.point.y + updateRect.extent.y ||
                globalRangeEnd   < updateRect.point.y) {
               currLine++;
               pElement = pElement->nextPhysicalLine;
               continue;
            }

            TextElement* walkAcross = pElement;
            while (walkAcross) {
               if (walkAcross->start > walkAcross->end)
                  break;

               Point2I globalStart  = localToGlobalCoord(localStart);

               U32 strWidth;
               if (walkAcross->specialReference == -1) {
                  dglSetBitmapModulation(lastColor);
                  dglSetTextAnchorColor(mProfile->mFontColor);
                  strWidth = dglDrawTextN(mProfile->mFont, globalStart, &mMessageVector->getLine(i).message[walkAcross->start],
                                          walkAcross->end - walkAcross->start + 1, mProfile->mFontColors);
                  dglGetBitmapModulation(&lastColor);
               } else {
                  SpecialMarkers::Special &s = mSpecialMarkers[i].specials[walkAcross->specialReference];
                  
                  dglGetBitmapModulation(&lastColor);
                  if (s.specialType >= 0)
                     dglSetBitmapModulation(mSpecialColor);
                  else
                  {
                     U32 colorIndex;
							
                     switch(s.specialType)
                     {
                        case ChannelVector::NickTag:
                           colorIndex = NickColor;

                           break;
                        case ChannelVector::TribeTag:
                           colorIndex = TribeColor;

                           break;
                        case ChannelVector::ServerTag:
                           colorIndex = ServerColor;

                           break;
                     }
							
                     dglSetBitmapModulation(mProfile->mFontColors[colorIndex]);
                  }
                  dglSetTextAnchorColor(mProfile->mFontColor);
                  strWidth = dglDrawTextN(mProfile->mFont, globalStart, &mMessageVector->getLine(i).message[walkAcross->start],
                                          walkAcross->end - walkAcross->start + 1);

                  // in case we have 2 in a row...
                  dglSetBitmapModulation(lastColor);
               }

               if (walkAcross->specialReference != -1) {
                  Point2I lineStart = localStart;
                  Point2I lineEnd   = localStart;
                  SpecialMarkers::Special &s = mSpecialMarkers[i].specials[walkAcross->specialReference];

                  lineStart.y += mProfile->mFont->getBaseline() + 2;
                  lineEnd.x += strWidth;
                  lineEnd.y += mProfile->mFont->getBaseline() + 2;

                  if (s.specialType >= 0)
                     dglDrawLine(localToGlobalCoord(lineStart),
                                 localToGlobalCoord(lineEnd),
                                 mSpecialColor);
                  else
                  {
                     U32 colorIndex;
							
                     switch(s.specialType)
                     {
                        case ChannelVector::NickTag:
                           colorIndex = NickColor;

                           break;
                        case ChannelVector::TribeTag:
                           colorIndex = TribeColor;

                           break;
                        case ChannelVector::ServerTag:
                           colorIndex = ServerColor;

                           break;
                     }

                     dglDrawLine(localToGlobalCoord(lineStart),
                                 localToGlobalCoord(lineEnd),
                                 mProfile->mFontColors[colorIndex]);
                  }
               }

               localStart.x += strWidth; 
               walkAcross = walkAcross->nextInLine;
            }

            currLine++;
            pElement = pElement->nextPhysicalLine;
         }
      }
      dglClearBitmapModulation();
   }
}


//--------------------------------------------------------------------------
void GuiChannelVectorCtrl::onMouseUp(const GuiEvent& event)
{
   GuiControl::onMouseUp(event);
   mouseUnlock();

   // Is this an up from a dragged click?
   if (mMouseDown == false)
      return;

   // Find the special we are in, if any...
   
   S32 currSpecialLine;
   S32 currSpecialRef;
   findSpecialFromCoord(globalToLocalCoord(event.mousePoint), &currSpecialLine, &currSpecialRef);

   if (currSpecialRef != -1 &&
       (currSpecialLine == mMouseSpecialLine &&
        currSpecialRef  == mMouseSpecialRef))
   {
      // Execute the callback
      char type[16];
      const char *content;

      SpecialMarkers& rSpecial = mSpecialMarkers[currSpecialLine];
      S32 specialStart = rSpecial.specials[currSpecialRef].start;
      S32 specialEnd   = rSpecial.specials[currSpecialRef].end;
		
      switch (rSpecial.specials[currSpecialRef].specialType)
      {
         case ChannelVector::TribeTag:
            if (currSpecialRef &&
                rSpecial.specials[currSpecialRef-1].specialType == ChannelVector::NickTag)
            {
               --currSpecialRef;
               specialStart = rSpecial.specials[currSpecialRef].start;
               specialEnd = rSpecial.specials[currSpecialRef].end;
            }
            else
            {
               ++currSpecialRef;
               specialStart = rSpecial.specials[currSpecialRef].start;
               specialEnd = rSpecial.specials[currSpecialRef].end;
            }
         case ChannelVector::NickTag:
            dStrcpy(type,"warrior");
            content = "";

            break;
         case ChannelVector::ServerTag:
         {
            const ChannelVector::SpecialMarkers &tags = ((ChannelVector *) mMessageVector)->getLineTags(currSpecialLine);
				
            dStrcpy(type,"server");
            for (U32 i = 0; i < tags.numSpecials; ++i)
               if (tags.specials[i].start == specialStart)
               {
                  content = tags.specials[i].content;
                  break;
               }

            break;
         }
         default:
         {
            dStrcpy(type,"http");
            content = "";
         }

         break;
      }

      char* copyURL = new char[specialEnd - specialStart + 2];

      dStrncpy(copyURL, &mMessageVector->getLine(currSpecialLine).message[specialStart], specialEnd - specialStart + 1);
      copyURL[specialEnd - specialStart + 1] = '\0';

      Con::executef(this, 4, "urlClickCallback", type, copyURL, content);
      
      delete [] copyURL;
   }

   mMouseDown      = false;
   mMouseSpecialLine = -1;
   mMouseSpecialRef  = -1;
}
