//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "console/consoleTypes.h"
#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiTextListCtrl.h"

static int sortColumn;
static bool sIncreasing;

static const char *getColumn(const char *text)
{
   int ct = sortColumn;
   while(ct--)
   {
      text = dStrchr(text, '\t');
      if(!text)
         return "";
      text++;
   }
   return text;
}

static S32 QSORT_CALLBACK textCompare( const void* a, const void* b )
{
   GuiTextListCtrl::Entry *ea = (GuiTextListCtrl::Entry *) (a);
   GuiTextListCtrl::Entry *eb = (GuiTextListCtrl::Entry *) (b);
   S32 result = dStricmp( getColumn( ea->text ), getColumn( eb->text ) );
   return ( sIncreasing ? result : -result );
}

static S32 QSORT_CALLBACK numCompare(const void *a,const void *b)
{
   GuiTextListCtrl::Entry *ea = (GuiTextListCtrl::Entry *) (a);
   GuiTextListCtrl::Entry *eb = (GuiTextListCtrl::Entry *) (b);
   const char* aCol = getColumn( ea->text );
   const char* bCol = getColumn( eb->text );
   char* aBuf = new char[dStrlen( aCol ) + 1];
   char* bBuf = new char[dStrlen( bCol ) + 1];
   dStrcpy( aBuf, aCol );
   dStrcpy( bBuf, bCol );
   char* ptr = dStrchr( aBuf, '\t' );
   if ( ptr )
      *ptr = '\0';
   ptr = dStrchr( bBuf, '\t' );
   if ( ptr )
      *ptr = '\0';
   S32 result = dAtoi( aBuf ) - dAtoi( bBuf );
   if ( result == 0 )
      return( dStricmp( ea->text, eb->text ) );
   delete [] aBuf;
   delete [] bBuf;
   return ( sIncreasing ? result : -result );
}

GuiTextListCtrl::GuiTextListCtrl()
{
   VECTOR_SET_ASSOCIATION(mList);
   VECTOR_SET_ASSOCIATION(mColumnOffsets);

   mActive = true;
   mEnumerate = false;
   mResizeCell = true;
   mSize.set(1, 0);
   mColumnOffsets.push_back(0);
   mFitParentWidth = true;
   mClipColumnText = false;
}

void GuiTextListCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("enumerate",               TypeBool, Offset(mEnumerate, GuiTextListCtrl));
   addField("resizeCell",              TypeBool, Offset(mResizeCell, GuiTextListCtrl));
   addField("columns",                 TypeS32Vector, Offset(mColumnOffsets, GuiTextListCtrl));  
   addField("fitParentWidth",          TypeBool, Offset(mFitParentWidth, GuiTextListCtrl));
   addField("clipColumnText",          TypeBool, Offset(mClipColumnText, GuiTextListCtrl));
}

static S32 cTextListGetSelectedCellId(SimObject *obj, S32, const char **)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   return ctrl->getSelectedId();
}

static void cTextListSetSelectedCellId(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   U32 id = dAtoi(argv[2]);
   S32 index = ctrl->findEntryById(id);
   if(index < 0)
      return ; 

   ctrl->setSelectedCell(Point2I(0, index));
}

static void cTextListSetSelectedRow(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>( obj );
   ctrl->setSelectedCell( Point2I( 0, dAtoi( argv[2] ) ) );
} 

static void cTextListClearSelection(SimObject *obj, S32, const char **)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   ctrl->setSelectedCell(Point2I(-1, -1));
}

static S32 cTextListAdd(SimObject *obj, S32 argc, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   S32 ret = ctrl->mList.size();
   if(argc < 5)
      ctrl->addEntry(dAtoi(argv[2]), argv[3]);
   else 
      ctrl->insertEntry(dAtoi(argv[2]), argv[3], dAtoi(argv[4]));
      
   return ret;
}

static void cTextListSet(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   ctrl->setEntry(dAtoi(argv[2]), argv[3]);
}

static void cTextListSort(SimObject *obj, S32 argc, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   if ( argc == 3 )
      ctrl->sort(dAtoi(argv[2]));
   else
      ctrl->sort( dAtoi( argv[2] ), dAtob( argv[3] ) );
}

static void cTextListSortNumerical( SimObject *obj, S32 argc, const char **argv )
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   if ( argc == 3 )
      ctrl->sortNumerical( dAtoi( argv[2] ) );
   else
      ctrl->sortNumerical( dAtoi( argv[2] ), dAtob( argv[3] ) );
}

static void cTextListClear(SimObject *obj, S32, const char **)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   ctrl->clear();
}

static S32 cTextListCount(SimObject *obj, S32, const char **)
{
   return ((GuiTextListCtrl *) obj)->getNumEntries();
}

static S32 cTextListGetRowId(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   U32 index = dAtoi(argv[2]);
   if(index >= ctrl->getNumEntries())
      return -1;
      
   return ctrl->mList[index].id;
}

static const char *cTextListGetRowTextById(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   U32 id = dAtoi(argv[2]);
   S32 index = ctrl->findEntryById(id);
   if(index < 0)
      return "";
   return ctrl->mList[index].text;
}

static S32 cTextListGetRowNumById(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   U32 id = dAtoi(argv[2]);

   S32 index = ctrl->findEntryById(id);
   if(index < 0)
      return -1;
   return index;
}

static const char *cTextListGetRowText(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   S32 index = dAtoi(argv[2]);
   if(index < 0 || index >= ctrl->mList.size())
      return "";
   return ctrl->mList[index].text;
}

static void cTextListRemoveRowById(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   U32 id = dAtoi(argv[2]);
   ctrl->removeEntry(id);
}

static void cTextListRemoveRow(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   U32 index = dAtoi(argv[2]);
   ctrl->removeEntryByIndex(index);
}

static void cTextListScrollRowVisible(SimObject *obj, S32, const char **argv)
{
   GuiTextListCtrl *ctrl = static_cast<GuiTextListCtrl*>(obj);
   ctrl->scrollCellVisible(Point2I(0, dAtoi(argv[2])));
}

static S32 cTextListFindText( SimObject* obj, S32, const char** argv )
{
   GuiTextListCtrl* ctrl = static_cast<GuiTextListCtrl*>( obj );
   return( ctrl->findEntryByText( argv[2] ) );
}

static void cTextListSetRowActive( SimObject* obj, S32, const char** argv )
{
   GuiTextListCtrl* ctrl = static_cast<GuiTextListCtrl*>( obj );
   ctrl->setEntryActive( U32( dAtoi( argv[2] ) ), dAtob( argv[3] ) );
}

static bool cTextListIsRowActive( SimObject* obj, S32, const char** argv )
{
   GuiTextListCtrl* ctrl = static_cast<GuiTextListCtrl*>( obj );
   return( ctrl->isEntryActive( U32( dAtoi( argv[2] ) ) ) );
}

void GuiTextListCtrl::consoleInit()
{
   Con::addCommand("GuiTextListCtrl", "getSelectedId",   cTextListGetSelectedCellId,   "textList.getSelectedId()", 2, 2);
   Con::addCommand("GuiTextListCtrl", "setSelectedById", cTextListSetSelectedCellId,   "textList.setSelectedById(id)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "setSelectedRow",  cTextListSetSelectedRow,      "textList.setSelectedRow(index)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "clearSelection",  cTextListClearSelection,      "textList.clearSelection()", 2, 2);
   Con::addCommand("GuiTextListCtrl", "clear",           cTextListClear,               "textList.clear()", 2, 2);
   Con::addCommand("GuiTextListCtrl", "addRow",          cTextListAdd,                 "textList.addRow(id,text,index)", 4, 5);
   Con::addCommand("GuiTextListCtrl", "setRowById",      cTextListSet,                 "textList.setRow(id,text)", 4, 4);
   Con::addCommand("GuiTextListCtrl", "getRowId",        cTextListGetRowId,            "textList.getRowId(index)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "removeRowById",   cTextListRemoveRowById,       "textList.removeRowById(id)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "getRowTextById",  cTextListGetRowTextById,      "textList.getRowTextById(id)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "getRowNumById",   cTextListGetRowNumById,       "textList.getRowNumById(id)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "getRowText",      cTextListGetRowText,          "textList.getRowText(index)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "removeRow",       cTextListRemoveRow,           "textList.removeRow(index)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "rowCount",        cTextListCount,               "textList.rowCount()", 2, 2);
   Con::addCommand("GuiTextListCtrl", "scrollVisible",   cTextListScrollRowVisible,    "textList.scrollVisible(index)", 3, 3);
   Con::addCommand("GuiTextListCtrl", "sort",            cTextListSort,                "textList.sort(colId{, increasing})", 3, 4);
   Con::addCommand("GuiTextListCtrl", "sortNumerical",   cTextListSortNumerical,       "textList.sortNumerical(colId{, increasing})", 3, 4);
   Con::addCommand("GuiTextListCtrl", "findTextIndex",   cTextListFindText,            "textList.findText(text)", 3, 3 ); 
   Con::addCommand("GuiTextListCtrl", "setRowActive",    cTextListSetRowActive,        "textlist.setRowActive(id, <bool>)", 4, 4 );
   Con::addCommand("GuiTextListCtrl", "isRowActive",     cTextListIsRowActive,         "textlist.isRowActive(id)", 3, 3 );
}

bool GuiTextListCtrl::onWake()
{
   if(!Parent::onWake())
      return false;
   
   setSize(mSize);
   return true;
}

U32 GuiTextListCtrl::getSelectedId()
{
   if (mSelectedCell.y == -1)
      return InvalidId;

   return mList[mSelectedCell.y].id;
}

void GuiTextListCtrl::onCellSelected(Point2I cell)
{
   Con::executef(this, 3, "onSelect", Con::getIntArg(mList[cell.y].id), mList[cell.y].text);
   
   if (mConsoleCommand[0])
      Con::evaluate(mConsoleCommand, false);
}

void GuiTextListCtrl::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   if ( mList[cell.y].active )
   {
      if (selected)
      {
         dglDrawRectFill(RectI(offset.x, offset.y, mCellSize.x, mCellSize.y), mProfile->mFillColorHL);
         dglSetBitmapModulation(mProfile->mFontColorHL);
      }
      else
         dglSetBitmapModulation(mouseOver ? mProfile->mFontColorHL : mProfile->mFontColor);
   }
   else
      dglSetBitmapModulation( mProfile->mFontColorNA );
   
   const char *text = mList[cell.y].text;
   for(U32 index = 0; index < mColumnOffsets.size(); index++)
   {
      const char *nextCol = dStrchr(text, '\t');
      if(mColumnOffsets[index] >= 0)
      {
         U32 slen;
         if(nextCol)
            slen = nextCol - text;
         else
            slen = dStrlen(text);

         Point2I pos(offset.x + 4 + mColumnOffsets[index], offset.y);

         RectI saveClipRect;
         bool clipped = false;

         if(mClipColumnText && (index != (mColumnOffsets.size() - 1)))
         {
            saveClipRect = dglGetClipRect();
            
            RectI clipRect(pos, Point2I(mColumnOffsets[index+1] - mColumnOffsets[index] - 4, mCellSize.y));
            if(clipRect.intersect(saveClipRect))
            {
               clipped = true;
               dglSetClipRect(clipRect);
            }
         }

         dglDrawTextN(mFont, pos, text, slen, mProfile->mFontColors);

         if(clipped)
            dglSetClipRect(saveClipRect);
      }
      if(!nextCol)
         break;
      text = nextCol+1;
   }
}   

U32 GuiTextListCtrl::getRowWidth(Entry *row)
{
   U32 width = 1;
   const char *text = row->text;
   for(U32 index = 0; index < mColumnOffsets.size(); index++)
   {
      const char *nextCol = dStrchr(text, '\t');
      U32 textWidth;
      if(nextCol)
         textWidth = mFont->getStrNWidth(text, nextCol - text);
      else
         textWidth = mFont->getStrWidth(text);
      if(mColumnOffsets[index] >= 0)
         width = getMax(width, mColumnOffsets[index] + textWidth);
      if(!nextCol)
         break;
      text = nextCol+1;
   }
   return width;
}

void GuiTextListCtrl::insertEntry(U32 id, const char *text, S32 index)
{
   Entry e;
   e.text = dStrdup(text);
   e.id = id;
   e.active = true;
   if(!mList.size())
      mList.push_back(e);
   else
   {
      if(index > mList.size())
         index = mList.size();
      mList.insert(&mList[index],e);
   }
   setSize(Point2I(1, mList.size()));
}

void GuiTextListCtrl::addEntry(U32 id, const char *text)
{
   Entry e;
   e.text = dStrdup(text);
   e.id = id;
   e.active = true;
   mList.push_back(e);
   setSize(Point2I(1, mList.size()));
}

void GuiTextListCtrl::setEntry(U32 id, const char *text)
{
   S32 e = findEntryById(id);
   if(e == -1)
      addEntry(id, text);
   else
   {
      dFree(mList[e].text);
      mList[e].text = dStrdup(text);

      // Still have to call this to make sure cells are wide enough for new values:
      setSize( Point2I( 1, mList.size() ) );
   }
   setUpdate();
}

void GuiTextListCtrl::setEntryActive(U32 id, bool active)
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      return;

   if ( mList[index].active != active )
   {
      mList[index].active = active;

      // You can't have an inactive entry selected...
      if ( !active && mSelectedCell.y >= 0 && mSelectedCell.y < mList.size() 
           && mList[mSelectedCell.y].id == id )
         setSelectedCell( Point2I( -1, -1 ) );
   
      setUpdate();
   }
}

S32 GuiTextListCtrl::findEntryById(U32 id)
{
   for(U32 i = 0; i < mList.size(); i++)
      if(mList[i].id == id)
         return i;
   return -1;
}

S32 GuiTextListCtrl::findEntryByText(const char *text)
{
   for(U32 i = 0; i < mList.size(); i++)
      if(!dStricmp(mList[i].text, text))
         return i;
   return -1;
}

bool GuiTextListCtrl::isEntryActive(U32 id)
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      return( false );

   return( mList[index].active );
}

void GuiTextListCtrl::setSize(Point2I newSize)
{
   mSize = newSize;

   if ( bool( mFont ) )
   {
      if ( mSize.x == 1 && mFitParentWidth )
      {
         GuiControl* parent = getParent();
         if ( parent )
            mCellSize.x = parent->mBounds.extent.x - mBounds.point.x;
      }
      else
      {
         // Find the maximum width cell:
         S32 maxWidth = 1;
         for ( U32 i = 0; i < mList.size(); i++ )
         {
            U32 rWidth = getRowWidth( &mList[i] );
            if ( rWidth > maxWidth )
               maxWidth = rWidth;
         }
         
         mCellSize.x = maxWidth + 8;
      }

      mCellSize.y = mFont->getHeight() + 2;
   }
   
   Point2I newExtent( newSize.x * mCellSize.x + mHeaderDim.x, newSize.y * mCellSize.y + mHeaderDim.y );
   resize( mBounds.point, newExtent );
}

void GuiTextListCtrl::clear()
{
   while (mList.size())
      removeEntry(mList[0].id);
      
   mMouseOverCell.set( -1, -1 );
   setSelectedCell(Point2I(-1, -1));
} 

void GuiTextListCtrl::sort(U32 column, bool increasing)
{
   if (getNumEntries() < 2)
      return;
   sortColumn = column;
   sIncreasing = increasing;
   dQsort((void *)&(mList[0]), mList.size(), sizeof(Entry), textCompare);
}  

void GuiTextListCtrl::sortNumerical( U32 column, bool increasing )
{
   if ( getNumEntries() < 2 )
      return;

   sortColumn = column;
   sIncreasing = increasing;
   dQsort( (void*) &( mList[0] ), mList.size(), sizeof( Entry ), numCompare );
}  

void GuiTextListCtrl::onRemove()
{
   clear();
   Parent::onRemove();
}   

U32 GuiTextListCtrl::getNumEntries()
{
   return mList.size();
}

void GuiTextListCtrl::removeEntryByIndex(S32 index)
{
   if(index < 0 || index >= mList.size())
      return;
   dFree(mList[index].text);
   mList.erase(index);

   setSize(Point2I( 1, mList.size()));
   setSelectedCell(Point2I(-1, -1));
}

void GuiTextListCtrl::removeEntry(U32 id)
{
   S32 index = findEntryById(id);
   removeEntryByIndex(index);
}                                

const char *GuiTextListCtrl::getSelectedText()
{
   if (mSelectedCell.y == -1)
      return NULL;

   return mList[mSelectedCell.y].text;
}

const char *GuiTextListCtrl::getScriptValue()
{
   return getSelectedText();
}

void GuiTextListCtrl::setScriptValue(const char *val)
{
   S32 e = findEntryByText(val);
   if(e == -1)
      setSelectedCell(Point2I(-1, -1));
   else
      setSelectedCell(Point2I(0, e));
}

bool GuiTextListCtrl::onKeyDown( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mActive || !mAwake )
      return true;

   if ( event.keyCode == KEY_RETURN )
   { 
      if ( mAltConsoleCommand[0] )
         Con::evaluate( mAltConsoleCommand, false );
      return( true );
   }

   if ( event.keyCode == KEY_DELETE && ( mSelectedCell.y >= 0 && mSelectedCell.y < mList.size() ) )
   {
      Con::executef( this, 2, "onDeleteKey", Con::getIntArg( mList[mSelectedCell.y].id ) );
      return( true );
   }

   return( Parent::onKeyDown( event ) );  
}

