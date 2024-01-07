//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "Shell/shellFancyTextList.h"
#include "GUI/guiCanvas.h"
#include "console/consoleTypes.h"

//------------------------------------------------------------------------------
// Static stuff for sorting:
//------------------------------------------------------------------------------
static S32  sSortColumn;
static bool sSortInc;

//------------------------------------------------------------------------------
static const char* getColumn( U32 column, const char* text )
{
   U32 ct = column;
   while ( ct-- )
   {
      text = dStrchr( text, '\t' );
      if ( !text )
         return( "" );
      text++;
   }
   return( text );
}

//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK fancyListRowCompare( const void* a, const void* b )
{
	ShellFancyTextList::Entry* entryA = (ShellFancyTextList::Entry*) a;
	ShellFancyTextList::Entry* entryB = (ShellFancyTextList::Entry*) b;
   S32 result = dStricmp( getColumn( sSortColumn, entryA->text ), getColumn( sSortColumn, entryB->text ) );
   return( sSortInc ? result : -result );
}

//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK fancyListRowNumCompare( const void* a, const void* b )
{
	ShellFancyTextList::Entry* entryA = (ShellFancyTextList::Entry*) a;
	ShellFancyTextList::Entry* entryB = (ShellFancyTextList::Entry*) b;
   const char* aCol = getColumn( sSortColumn, entryA->text );
   const char* bCol = getColumn( sSortColumn, entryB->text );
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
      return( dStricmp( entryA->text, entryB->text ) );
   delete [] aBuf;
   delete [] bBuf;
   return( sSortInc ? result : -result );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ShellFancyTextList);

//------------------------------------------------------------------------------
ShellFancyTextList::ShellFancyTextList() : ShellFancyArray()
{
   mAllowReposition = false;  // Repositioning columns currently not supported for this class.
   mStyleList = NULL;
}

//------------------------------------------------------------------------------
ShellFancyTextList::~ShellFancyTextList()
{
   for ( StyleSet* walk = mStyleList; walk; walk = walk->next )
      walk->font = NULL;
   mStyleList = NULL;
   mResourceChunker.freeBlocks();
}

//------------------------------------------------------------------------------
static S32 cFTLGetSelectedId( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLGetSelectedId is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	return( ftl->getSelectedId() );
}

//------------------------------------------------------------------------------
static void cFTLSelectRowById( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLSelectRowById is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	ftl->selectRowById( dAtoi( argv[2] ) );
}

//------------------------------------------------------------------------------
static void cFTLClearSelection( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Object passed to cFTLClearSelection is not a ShellFancyArray!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->selectCell( -1, -1 );
}

//------------------------------------------------------------------------------
static void cFTLClear( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLClear is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	ftl->clearList();
}
         
//------------------------------------------------------------------------------
static void cFTLAddRow( SimObject* obj, S32 argc, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLAddRow is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
   if ( argc == 4 )
	   ftl->addEntry( dAtoi( argv[2] ), argv[3] );
   else
      ftl->insertEntry( dAtoi( argv[2] ), argv[3], dAtoi( argv[4] ) );
}
          
//------------------------------------------------------------------------------
static void cFTLSetRowById( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLSetRowById is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	ftl->setEntry( dAtoi( argv[2] ), argv[3] );
}
          
//------------------------------------------------------------------------------
static S32 cFTLGetRowId( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLGetRowId is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	return( ftl->getRowId( dAtoi( argv[2] ) ) );
}

//------------------------------------------------------------------------------
static void cFTLRemoveRowById( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLRemoveRowById is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	ftl->removeEntry( dAtoi( argv[2] ) );
}
          
//------------------------------------------------------------------------------
static const char* cFTLGetRowTextById( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLGetRowTextById is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	return( ftl->getText( ftl->findEntryById( dAtoi( argv[2] ) ) ) );
}
       
//------------------------------------------------------------------------------
static void cFTLRemoveRowByIndex( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLRemoveRowByIndex is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	ftl->removeEntryByIndex( dAtoi( argv[2] ) );
}
 
//------------------------------------------------------------------------------
static S32 cFTLFindById( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLFindById is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	return( ftl->findEntryById( dAtoi( argv[2] ) ) );
}
          
//------------------------------------------------------------------------------
static const char* cFTLGetRowText( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLGetRowText is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	return( ftl->getText( dAtoi( argv[2] ) ) );
}
          
//------------------------------------------------------------------------------
static S32 cFTLFindText( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLFindText is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
	return( ftl->findEntryByText( argv[2] ) );
}

//------------------------------------------------------------------------------
static void cFTLSort( SimObject* obj, S32 argc, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLSort is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
   if ( argc > 2 )
   {
      S32 key = ftl->getColumnKey( dAtoi( argv[2] ) );
      ftl->setSortColumnKey( key ); 
      if ( argc > 3 )
         ftl->setSortInc( dAtob( argv[3] ) );
   }
   ftl->sort();
}

//------------------------------------------------------------------------------
static bool cFTLAddStyle( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLAddStyle is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );

   ColorI fontColor, fontColorHL, fontColorSEL;
   fontColor.set( 0, 0, 0, 255 );
   fontColorHL.set( 0, 0, 0, 255 );
   fontColorSEL.set( 0, 0, 0, 255 );
   S32 r, g, b, a;
   S32 args = dSscanf( argv[5], "%d %d %d %d", &r, &g, &b, &a );
   fontColor.red   = r;
   fontColor.green = g;
   fontColor.blue  = b;
   if ( args == 4 )
      fontColor.alpha = a;

   args = dSscanf( argv[6], "%d %d %d %d", &r, &g, &b, &a );
   fontColorHL.red   = r;
   fontColorHL.green = g;
   fontColorHL.blue  = b;
   if ( args == 4 )
      fontColorHL.alpha = a;

   args = dSscanf( argv[7], "%d %d %d %d", &r, &g, &b, &a );
   fontColorSEL.red   = r;
   fontColorSEL.green = g;
   fontColorSEL.blue  = b;
   if ( args == 4 )
      fontColorSEL.alpha = a;

   return( ftl->addStyleSet( dAtoi( argv[2] ), argv[3], dAtoi( argv[4] ), fontColor, fontColorHL, fontColorSEL ) );
}

//------------------------------------------------------------------------------
static void cFTLSetRowStyle( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLSetRowStyle is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
   ftl->setEntryStyle( dAtoi( argv[2] ), dAtoi( argv[3] ) );
}

//------------------------------------------------------------------------------
static void cFTLSetRowStyleById( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLSetRowStyleById is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
   ftl->setEntryStyle( ftl->findEntryById( dAtoi( argv[2] ) ), dAtoi( argv[3] ) );
}

//------------------------------------------------------------------------------
static S32 cFTLGetRowStyle( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLGetRowStyle is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
   return( ftl->getEntryStyle( dAtoi( argv[2] ) ) );
}

//------------------------------------------------------------------------------
static S32 cFTLGetRowStyleById( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyTextList*>( obj ), "Object passed to cFTLGetRowStyleById is not a ShellFancyTextList!" );
	ShellFancyTextList* ftl = static_cast<ShellFancyTextList*>( obj );
   return( ftl->getEntryStyle( ftl->findEntryById( dAtoi( argv[2] ) ) ) );
}
        
//------------------------------------------------------------------------------
void ShellFancyTextList::consoleInit()
{
	Con::addCommand( "ShellFancyTextList", "getSelectedId",     cFTLGetSelectedId,      "fancytextlist.getSelectedId()", 2, 2 );
	Con::addCommand( "ShellFancyTextList", "setSelectedById",   cFTLSelectRowById,      "fancytextlist.setSelectedById( id )", 3, 3 );
   Con::addCommand( "ShellFancyTextList", "clearSelection",    cFTLClearSelection,     "fancytextlist.clearSelection()", 2, 2);
	Con::addCommand( "ShellFancyTextList", "clear",             cFTLClear,              "fancytextlist.clear()", 2, 2 );
	Con::addCommand( "ShellFancyTextList", "addRow",            cFTLAddRow,             "fancytextlist.addRow( id, text{, index } )", 4, 5 );
	Con::addCommand( "ShellFancyTextList", "setRowById",        cFTLSetRowById,         "fancytextlist.setRowById( id, text )", 4, 4 );
	Con::addCommand( "ShellFancyTextList", "getRowId",          cFTLGetRowId,           "fancytextlist.getRowId( index )", 3, 3 );
	Con::addCommand( "ShellFancyTextList", "removeRowById",     cFTLRemoveRowById,      "fancytextlist.removeRowById( id )", 3, 3 );
	Con::addCommand( "ShellFancyTextList", "getRowTextById",    cFTLGetRowTextById,     "fancytextlist.getRowTextById( id )", 3, 3 );
	Con::addCommand( "ShellFancyTextList", "getRowNumById",     cFTLFindById,           "fancytextlist.getRowNumById( id )", 3, 3 );
	Con::addCommand( "ShellFancyTextList", "getRowText",        cFTLGetRowText,         "fancytextlist.getRowText( index )", 3, 3 );
	Con::addCommand( "ShellFancyTextList", "removeRow",         cFTLRemoveRowByIndex,   "fancytextlist.removeRow( index )", 3, 3 );
	Con::addCommand( "ShellFancyTextList", "findTextIndex",     cFTLFindText,           "fancytextlist.findTextIndex( text )", 3, 3 );
   Con::addCommand( "ShellFancyTextList", "sort",              cFTLSort,               "fancytextlist.sort( { column{, increasing} } )", 2, 4 );
   Con::addCommand( "ShellFancyTextList", "addStyle",          cFTLAddStyle,           "fancytextlist.addStyle( id, fontType, fontSize, fontColor, fontColorHL, fontColorSEL )", 8, 8 );
   Con::addCommand( "ShellFancyTextList", "setRowStyle",       cFTLSetRowStyle,        "fancytextlist.setRowStyle( row, style )", 4, 4 );
   Con::addCommand( "ShellFancyTextList", "setRowStyleById",   cFTLSetRowStyleById,    "fancytextlist.setRowStyleById( id, style )", 4, 4 );
   Con::addCommand( "ShellFancyTextList", "getRowStyle",       cFTLGetRowStyle,        "fancytextlist.getRowStyle( row )", 3, 3 );
   Con::addCommand( "ShellFancyTextList", "getRowStyleById",   cFTLGetRowStyleById,    "fancytextlist.getRowStyleById( id )", 3, 3 );
}

//------------------------------------------------------------------------------
void ShellFancyTextList::initPersistFields()
{
	Parent::initPersistFields();
}

//------------------------------------------------------------------------------
const char* ShellFancyTextList::getScriptValue()
{
	return( NULL );
}

//------------------------------------------------------------------------------
U32 ShellFancyTextList::getNumEntries()
{
	return( mList.size() );	
}

//------------------------------------------------------------------------------
void ShellFancyTextList::clearList()
{
	while ( mList.size() )
		removeEntry( mList[0].id );

	mSelectedRow = -1;
	mMouseOverRow = -2;
	mMouseOverColumn = -1;
   setNumRows( 0 );
}

//------------------------------------------------------------------------------
void ShellFancyTextList::addEntry( U32 id, const char* text )
{
	Entry newEntry;
	newEntry.text = dStrdup( text );
	newEntry.id = id;
   newEntry.active = true;
   newEntry.styleId = 0;
	mList.push_back( newEntry );
	setNumRows( mList.size() );
}

//------------------------------------------------------------------------------
void ShellFancyTextList::insertEntry( U32 id, const char* text, S32 index )
{
   Entry newEntry;
   newEntry.text = dStrdup( text );
   newEntry.id = id;
   newEntry.active = true;
   newEntry.styleId = 0;
   mList.insert( &mList[index], newEntry );
   setNumRows( mList.size() ); 
}

//------------------------------------------------------------------------------
void ShellFancyTextList::removeEntry( U32 id )
{
   S32 index = findEntryById( id );
   removeEntryByIndex( index );
}                                

//------------------------------------------------------------------------------
void ShellFancyTextList::removeEntryByIndex( S32 index )
{
   if ( index < 0 || index >= mList.size() )
      return;
   dFree( mList[index].text );
   mList.erase( index );

	setNumRows( mList.size() );
   selectCell( -1, -1 );
}

//------------------------------------------------------------------------------
void ShellFancyTextList::setEntry( U32 id, const char* text )
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      addEntry( id, text );
   else
   {
	   dFree( mList[index].text );
		mList[index].text = dStrdup( text );
   }
	setUpdate();
}

//------------------------------------------------------------------------------
void ShellFancyTextList::setEntryActive( U32 id, bool active )
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      return;
   
   if ( mList[index].active != active )
   {
      mList[index].active = active;
	   setUpdate();
   }
}

//------------------------------------------------------------------------------
S32 ShellFancyTextList::findEntryById( U32 id )
{
   for ( U32 i = 0; i < mList.size(); i++ )
      if ( mList[i].id == id )
         return i;
   return -1;
}

//------------------------------------------------------------------------------
S32 ShellFancyTextList::findEntryByText( const char* text )
{
   for ( U32 i = 0; i < mList.size(); i++ )
      if ( dStrcmp( mList[i].text, text ) == 0 )
         return i;
   return -1;
}

//------------------------------------------------------------------------------
bool ShellFancyTextList::isEntryActive( U32 id )
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      return( false );

   return( mList[index].active );
}

//------------------------------------------------------------------------------
void ShellFancyTextList::selectRowById( U32 id )
{
	selectCell( findEntryById( id ), 0 );
}

//------------------------------------------------------------------------------
U32 ShellFancyTextList::getSelectedId()
{
	if ( mSelectedRow == -1 || mSelectedRow >= mList.size() )
		return( InvalidId );
		
	return( mList[mSelectedRow].id );	
}

//------------------------------------------------------------------------------
U32 ShellFancyTextList::getRowId( S32 index )
{
	if ( index == -1 || index >= mList.size() )
		return( InvalidId );
		
	return( mList[index].id );	
}

//------------------------------------------------------------------------------
const char* ShellFancyTextList::getText( S32 index )
{
	if ( index == -1 || index >= mList.size() )
		return( "" );
		
	return( mList[index].text );	
}

//------------------------------------------------------------------------------
bool ShellFancyTextList::addStyleSet( U32 id, const char* fontType, U32 fontSize, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL )
{
   // Make sure the id isn't taken already:
   if ( id == 0 ) 
   {
      Con::warnf( ConsoleLogEntry::General, "ShellFancyTextList::addStyleSet - style id 0 is reserved for the default!" );
      return( false );
   }

   if ( getStyleSet( id ) != NULL )
   {
      Con::warnf( ConsoleLogEntry::General, "ShellFancyTextList::addStyleSet - style id %d used already!", id );
      return( false );
   }

   // Add the new style set:
   StyleSet* newStyle;
   newStyle = constructInPlace( (StyleSet*) mResourceChunker.alloc( sizeof( StyleSet ) ) );
   newStyle->fontType = StringTable->insert( fontType );
   newStyle->fontSize = fontSize;
   if ( mAwake )
      newStyle->font = GFont::create( newStyle->fontType, newStyle->fontSize );
   newStyle->id = id;
   newStyle->fontColor = fontColor;
   newStyle->fontColorHL = fontColorHL;
   newStyle->fontColorSEL = fontColorSEL;

   newStyle->next = mStyleList;
   mStyleList = newStyle;
   return( true );
}

//------------------------------------------------------------------------------
const ShellFancyTextList::StyleSet* ShellFancyTextList::getStyleSet( U32 id )
{
   if ( id )
   {
      for ( StyleSet* walk = mStyleList; walk; walk = walk->next )
      {
         if ( walk->id == id )
            return( walk );
      }
   }

   return( NULL );
}

//------------------------------------------------------------------------------
void ShellFancyTextList::setEntryStyle( S32 index, U32 styleId )
{
   if ( index < 0 || index >= mList.size() )
      return;

   if ( mList[index].styleId != styleId )
   {
      mList[index].styleId = styleId;
      setUpdate();
   }
}

//------------------------------------------------------------------------------
U32 ShellFancyTextList::getEntryStyle( S32 index )
{
   if ( index < 0 || index >= mList.size() )
      return( 0 );

   return( mList[index].styleId );
}

//------------------------------------------------------------------------------
void ShellFancyTextList::onCellSelected( S32 row, S32 /*column*/ )
{
   Con::executef( this, 3, "onSelect", Con::getIntArg( mList[row].id ), mList[row].text );
   if ( mConsoleCommand[0] )
      Con::evaluate( mConsoleCommand, false );
}

//------------------------------------------------------------------------------
bool ShellFancyTextList::onWake()
{
   if ( !Parent::onWake() )
      return false;

	// Set the row height based on the font:
	if ( mFont )
		mRowHeight = mFont->getHeight() + 3;

   // Get all of the style fonts:
   for ( StyleSet* walk = mStyleList; walk; walk = walk->next )
      walk->font = GFont::create( walk->fontType, walk->fontSize );

   return true;
}

//------------------------------------------------------------------------------
void ShellFancyTextList::onSleep()
{
   Parent::onSleep();

   // Release all of the style fonts:
   for ( StyleSet* walk = mStyleList; walk; walk = walk->next )
      walk->font = NULL;
}

//------------------------------------------------------------------------------
void ShellFancyTextList::updateList()
{
	if ( mNumRows != mList.size() )
		setNumRows( mList.size() );

	setUpdate();
}

//------------------------------------------------------------------------------
void ShellFancyTextList::sort()
{
   if ( mSortColumnKey == -1 )
      return;

   mNumRows = mList.size();
   sSortColumn = findColumn( mSortColumnKey );
   sSortInc = mSortInc;

	U32 selId = getSelectedId();

   if ( mNumRows > 1 )
   {
      if ( mColumnInfoList[sSortColumn].flags.test( Column_Numeric ) )
         dQsort( (void*) &(mList[0]), mNumRows, sizeof(Entry), fancyListRowNumCompare );
      else
         dQsort( (void*) &(mList[0]), mNumRows, sizeof(Entry), fancyListRowCompare );
   }

   selectRowById( selId );
   setUpdate();
}

//------------------------------------------------------------------------------
void ShellFancyTextList::onRenderCell( Point2I offset, Point2I cell, bool selected, bool mouseOver )
{
	ColumnInfo* ci = &mColumnInfoList[cell.x];
	Entry* entry = &mList[cell.y];

	// Let the parent take care of the basics:
	Parent::onRenderCell( offset, cell, selected, mouseOver );

   // Draw the text ( if there is any ):
   const char* text = getColumn( cell.x, entry->text );
   if ( text[0] )
   {
      const char* temp = dStrchr( text, '\t' );
      U32 textLen = temp ? ( temp - text ) : dStrlen( text );
      char* drawText = new char[textLen + 4];
      dStrncpy( drawText, text, textLen );
      drawText[textLen] = '\0';
   
      if ( ci->flags.test( Column_Icon ) )
      {
         char buf[64];
         dSprintf( buf, sizeof( buf ), "gui/%s.png", drawText );
         TextureHandle tex = TextureHandle( buf, BitmapTexture );
         if ( tex )
         {
            Point2I drawPt = offset;
            if ( ci->width > tex.getWidth() )
               drawPt.x += ( ci->width - tex.getWidth() ) / 2;
            if ( mRowHeight > tex.getHeight() )
               drawPt.y += ( mRowHeight - tex.getHeight() ) / 2;
            dglDrawBitmap( tex, drawPt );
            tex = NULL;
         }
      }
      else
      {
         Resource<GFont> drawFont = NULL;
         ColorI drawColor;
         const StyleSet* style = getStyleSet( entry->styleId );
         if ( bool( style ) )
         {
            drawFont = style->font;
            drawColor = entry->active ? ( selected ? style->fontColorSEL : ( mouseOver ? style->fontColorHL : style->fontColor ) ) : mProfile->mFontColorNA;
         }
         else
         {
            // Fall back to the default:
            drawFont = mFont;
            drawColor = entry->active ? ( selected ? mProfile->mFontColorSEL : ( mouseOver ? mProfile->mFontColorHL : mProfile->mFontColor ) ) : mProfile->mFontColorNA;
         }

         S32 textWidth = drawFont->getStrWidth( drawText );
         Point2I textStart;

         if ( ci->flags.test( Column_Center ) )
            textStart.x = offset.x + getMax( 4, ( ci->width - textWidth ) / 2 );
         else if ( ci->flags.test( Column_Right ) )
            textStart.x = offset.x + getMax( 4, ci->width - textWidth );
         else
            textStart.x = offset.x + 4;
         textStart.y = offset.y + ( ( mRowHeight - ( drawFont->getHeight() - 2 ) ) / 2 );
         dglSetBitmapModulation( drawColor );
         dglDrawText( drawFont, textStart, drawText, mProfile->mFontColors );
         delete [] drawText;
      }
   }
}  


