//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "shell/shellFancyArray.h"
#include "gui/guiCanvas.h"
#include "console/consoleTypes.h"

//------------------------------------------------------------------------------
//
// VirtualScrollContentCtrl functions
//
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(VirtualScrollContentCtrl);

//------------------------------------------------------------------------------
VirtualScrollContentCtrl::VirtualScrollContentCtrl() : GuiScrollContentCtrl()
{
	mVirtualContent = NULL;
}

//------------------------------------------------------------------------------
bool VirtualScrollContentCtrl::onAdd()
{
	if ( !Parent::onAdd() )
		return( false );
	
	GuiControl* control = new GuiControl();
	if ( !control->registerObject() )
		Con::errorf( ConsoleLogEntry::General, "Failed to add virtual control to VirtualScrollContentCtrl!" );
	addObject( control );

	return( true );	
}

//------------------------------------------------------------------------------
void VirtualScrollContentCtrl::addObject( SimObject* obj )
{
	// The virtual scroll control can accept only one content control.
	GuiControl* control = dynamic_cast<GuiControl*>( obj );
	AssertFatal( control, "Object added to the VirtualScrollContentCtrl is not a GuiControl!" );

	if ( mVirtualContent )
		mVirtualContent->deleteObject();
	Parent::addObject( control );
	mVirtualContent = control;

	// Pass the virtual content back through the line:
	GuiControl* parent = getParent();
	if ( parent )
	{
		AssertFatal( dynamic_cast<VirtualScrollCtrl*>( parent ), "VirtualScrollContentCtrl's parent is not a VirtualScrollCtrl!" );
		VirtualScrollCtrl* scrollCtrl = static_cast<VirtualScrollCtrl*>( parent );
		scrollCtrl->setVirtualContent( control );
	}
}

//------------------------------------------------------------------------------
void VirtualScrollContentCtrl::removeObject( SimObject* obj )
{
	// If we are deleting the virtual content control, we need to let the "family" know:.
	GuiControl* control = dynamic_cast<GuiControl*>( obj );
	if ( mVirtualContent == control )
		mVirtualContent = NULL;

	GuiControl* parent = getParent();
	if ( parent )
	{
		AssertFatal( dynamic_cast<VirtualScrollCtrl*>( parent ), "VirtualScrollContentCtrl's parent is not a VirtualScrollCtrl!" );
		VirtualScrollCtrl* scrollCtrl = static_cast<VirtualScrollCtrl*>( parent );
		scrollCtrl->setVirtualContent( NULL );
	}

	Parent::removeObject( obj );
}

//------------------------------------------------------------------------------
//
// VirtualScrollCtrl functions
//
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(VirtualScrollCtrl);

//------------------------------------------------------------------------------
VirtualScrollCtrl::VirtualScrollCtrl() : ShellScrollCtrl()
{
   // The virtual scroll control should not have a field background:
   mFieldBase = StringTable->insert( "" );
}

//------------------------------------------------------------------------------
void VirtualScrollCtrl::setVirtualContent( GuiControl* control )
{
	GuiControl* parent = getParent();
	if ( parent )
	{
		AssertFatal( dynamic_cast<ShellFancyArrayScrollCtrl*>( parent ), "VirtualScrollCtrl's parent is not a ShellFancyArrayScrollCtrl!" );
		ShellFancyArrayScrollCtrl* arrayCtrl = static_cast<ShellFancyArrayScrollCtrl*>( parent );
		arrayCtrl->setVirtualContent( control );
	}
}

//------------------------------------------------------------------------------
GuiControl* VirtualScrollCtrl::getVirtualContent()
{
	if ( mContentCtrl )
	{
		VirtualScrollContentCtrl* scrollContentCtrl = static_cast<VirtualScrollContentCtrl*>( mContentCtrl );
		return( scrollContentCtrl->mVirtualContent );		
	}

	return( NULL );	
}

//------------------------------------------------------------------------------
bool VirtualScrollCtrl::onAdd()
{
	// We have to bypass the GuiScrollCtrl::onAdd here:
	if ( !GuiControl::onAdd() )
		return( false );

	VirtualScrollContentCtrl* content = new VirtualScrollContentCtrl();
	if ( !content->registerObject() )		
		Con::errorf( ConsoleLogEntry::General, "Failed to add virtual content control to VirtualScrollCtrl!" );
	addObject( content );

	return( true );	
}

//------------------------------------------------------------------------------
void VirtualScrollCtrl::addObject( SimObject* obj )
{
	VirtualScrollContentCtrl* contentCtrl = dynamic_cast<VirtualScrollContentCtrl*>( obj );
	if ( contentCtrl )
	{
		if ( mContentCtrl )
			mContentCtrl->deleteObject();
		// We have to bypass GuiScrollCtrl::addObject here:
		GuiControl::addObject( obj );
		mContentCtrl = (GuiScrollContentCtrl*) contentCtrl;
		return;
	}
	AssertFatal( mContentCtrl, "ERROR - VirtualScrollCtrl has no content control!" );

	mContentCtrl->addObject( obj );
	computeSizes();
}

//------------------------------------------------------------------------------
//
// ShellFancyArray functions
//
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ShellFancyArray);

//------------------------------------------------------------------------------
ShellFancyArray::ShellFancyArray() : GuiControl()
{
	dMemset( mBmpBounds, 0, sizeof( mBmpBounds ) );
	mHeaderBitmap = StringTable->insert( "gui/server_tabs" );
   mSortArrowBitmap = StringTable->insert( "gui/shll_sortarrow" );
	mBarBase = StringTable->insert( "gui/shll_bar" );

	mTexHeader = NULL;
   mTexSortArrow = NULL;
	mTexCellSelected = NULL;
	mTexCellRollover = NULL;

	mFieldBase = StringTable->insert( "" );
	mTexLeftTop = NULL;
   mTexCenterTop = NULL;
   mTexRightTop = NULL;
   mTexLeftCenter = NULL;
   mTexCenter = NULL;
   mTexRightCenter = NULL;
   mTexLeftBottom = NULL;
   mTexCenterBottom = NULL;
   mTexRightBottom = NULL;

	// Initialize object statics:
	mActive = true;
	mFixedHorizontal = false;
	mColumnInfoList.clear();
	mNumColumns = 0;
	mRowHeight = 20;			// Default--can change the field
	mHeaderHeight = 0;		// This is sized automatically in onWake
	mGlowOffset = 4; 	// This should be set based on the header bitmap
	mMinColumnWidth = 2;		// This gets set automatically in onWake
	mStartScrollRgn.set( 0, 0 );

	mFont = NULL;
   mHeaderFont = NULL;

   mHeaderFontType = StringTable->insert( "" );
   mHeaderFontSize = 0;

	mDefaultCursor = NULL;
	mResizeCursor = NULL;
	mRepositionCursor = NULL;

	// Default color values:
	mHeaderFontColor.set( 8, 19, 6 );
	mHeaderFontColorHL.set( 25, 68, 56 );
	mSeparatorColor.set( 192, 192, 192 );

	mDrawCellSeparators = false;
   mHeaderSort = true;
   mAllowReposition = true;
   mNoSelect = false;

	mNumRows = 0;
	mSelectedRow = -1;
	mMouseOverRow = -2;
	mMouseOverColumn = -1;
	mScrollPanePos.set( 0, 0 );

	mColumnState = None;
	mActiveColumn = 0;
	mDragAnchor.set( 0, 0 );

	mSortColumnKey = -1;
	mSecondarySortColumnKey = -1;
	mSortInc = true;
	mSecondarySortInc = true;

   mAbsResizeLeftMargin = 0;
   mAbsResizeRightMargin = 0;
   mResizeFixedColumn = false;
   mResizeColumnOrigSize = 0;

   mRepositionColumnTo = 0;
   mRepositionCursorPos.set( 0, 0 );
}

//------------------------------------------------------------------------------
bool ShellFancyArray::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Initialize cursors:
   SimObject* obj = Sim::findObject( "DefaultCursor" );
   mDefaultCursor = dynamic_cast<GuiCursor*>( obj );
   obj = Sim::findObject( "LeftRightCursor" );
   mResizeCursor = dynamic_cast<GuiCursor*>( obj );
   obj = Sim::findObject( "GrabCursor" );
   mRepositionCursor = dynamic_cast<GuiCursor*>( obj );

   return true;
}

//------------------------------------------------------------------------------
void ShellFancyArray::onRemove()
{
	mColumnInfoList.clear();
	mNumColumns = 0;
	Parent::onRemove();
}

//------------------------------------------------------------------------------
void ShellFancyArray::initPersistFields()
{
	Parent::initPersistFields();
	addField( "startScrollRegion",   TypePoint2I,   Offset( mStartScrollRgn, ShellFancyArray ) );
	addField( "headerBitmap",        TypeString,    Offset( mHeaderBitmap, ShellFancyArray ) );
	addField( "sortArrowBitmap",     TypeString,    Offset( mSortArrowBitmap, ShellFancyArray ) );
	addField( "fieldBase",           TypeString,    Offset( mFieldBase, ShellFancyArray ) );
	addField( "barBase",             TypeString,    Offset( mBarBase, ShellFancyArray ) );
	addField( "glowOffset",          TypeS32,       Offset( mGlowOffset, ShellFancyArray ) );
	addField( "rowHeight",           TypeS32,       Offset( mRowHeight, ShellFancyArray ) );
	addField( "headerFontType",      TypeString,    Offset( mHeaderFontType, ShellFancyArray ) );
	addField( "headerFontSize",      TypeS32,       Offset( mHeaderFontSize, ShellFancyArray ) );
	addField( "headerFontColor",     TypeColorI,    Offset( mHeaderFontColor, ShellFancyArray ) );
	addField( "headerFontColorHL",   TypeColorI,    Offset( mHeaderFontColorHL, ShellFancyArray ) );
	addField( "separatorColor",      TypeColorI,    Offset( mSeparatorColor, ShellFancyArray ) );
	addField( "drawSeparators",      TypeBool,      Offset( mDrawCellSeparators, ShellFancyArray ) );
	addField( "headerSort",          TypeBool,      Offset( mHeaderSort, ShellFancyArray ) );
	addField( "allowReposition",     TypeBool,      Offset( mAllowReposition, ShellFancyArray ) );
	addField( "noSelect",            TypeBool,      Offset( mNoSelect, ShellFancyArray ) );
}

//------------------------------------------------------------------------------
static void cShellFancyArrayClearColumns( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayClearColumns!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->clearColumns();
}

//------------------------------------------------------------------------------
static S32 cShellFancyArrayGetNumColumns( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayClearColumns!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return array->getNumColumns();
}

//------------------------------------------------------------------------------
static void cShellFancyArrayAddColumn( SimObject* obj, S32 argc, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayAddColumn!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->addColumn( dAtoi( argv[2] ), argv[3], dAtoi( argv[4] ), dAtoi( argv[5] ), dAtoi( argv[6] ), ( argc == 8 ) ? argv[7] : NULL );
}

//------------------------------------------------------------------------------
static void cShellFancyArrayAddRow( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayAddRow!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->setNumRows( array->getNumRows() + 1 );
}

//------------------------------------------------------------------------------
static void cShellFancyArraySetNumRows( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArraySetNumRows!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->setNumRows( dAtoi( argv[2] ) );
}

//------------------------------------------------------------------------------
static S32 cShellFancyArrayRowCount( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayRowCount!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return( array->getNumRows() );
}

//------------------------------------------------------------------------------
static void cShellFancyArrayForceUpdate( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayForceUpdate!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->updateList();
}

//------------------------------------------------------------------------------
static void cShellFancyArrayClearList( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayClearList!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->clearList();
}

//------------------------------------------------------------------------------
static void cShellFancyArraySetSelectedRow( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArraySetSelectedRow!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->selectCell( dAtoi( argv[2] ), 0 );
}

//------------------------------------------------------------------------------
static void cShellFancyArraySetSortColumn( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArraySetSortColumn!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->setSortColumnKey( dAtoi( argv[2] ) );
}

//------------------------------------------------------------------------------
static void cShellFancyArraySetSortIncreasing( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArraySetSortIncreasing!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->setSortInc( dAtob( argv[2] ) );
}

//------------------------------------------------------------------------------
static void cSFASetSecondarySortColumn( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cSFASetSecondarySortColumn!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->setSecondarySortColumnKey( dAtoi( argv[2] ) );
}

//------------------------------------------------------------------------------
static void cSFASetSecondarySortIncreasing( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cSFASetSecondarySortIncreasing!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	array->setSecondarySortInc( dAtob( argv[2] ) );
}

//------------------------------------------------------------------------------
static S32 cShellFancyArrayGetSelectedRow( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Non-ShellFancyArray passed to cShellFancyArrayGetSelectedRow!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return( array->getSelectedRow() );
}

//------------------------------------------------------------------------------
static S32 cShellFancyArrayGetColumnKey( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Object passed to cShellFancyArrayGetColumnKey is not a ShellFancyArray!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return ( array->getColumnKey( dAtoi( argv[2] ) ) );
}

//------------------------------------------------------------------------------
static S32 cShellFancyArrayGetColumnWidth( SimObject* obj, S32, const char** argv )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Object passed to cShellFancyArrayGetColumnWidth is not a ShellFancyArray!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return ( array->getColumnWidth( dAtoi( argv[2] ) ) );
}

//------------------------------------------------------------------------------
static S32 cShellFancyArrayGetSortColumnKey( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Object passed to cShellFancyArrayGetSortColumnKey is not a ShellFancyArray!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return ( array->getSortColumnKey() );
}

//------------------------------------------------------------------------------
static bool cShellFancyArrayGetSortIncreasing( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Object passed to cShellFancyArrayGetSortIncreasing is not a ShellFancyArray!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return ( array->getSortIncreasing() );
}

//------------------------------------------------------------------------------
static S32 cSFAGetSecondarySortColumnKey( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Object passed to cSFAGetSecondarySortColumnKey is not a ShellFancyArray!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return ( array->getSecondarySortColumnKey() );
}

//------------------------------------------------------------------------------
static bool cSFAGetSecondarySortIncreasing( SimObject* obj, S32, const char** )
{
	AssertFatal( dynamic_cast<ShellFancyArray*>( obj ), "Object passed to cSFAGetSecondarySortIncreasing is not a ShellFancyArray!" );
	ShellFancyArray* array = static_cast<ShellFancyArray*>( obj );
	return ( array->getSecondarySortIncreasing() );
}

//------------------------------------------------------------------------------
void ShellFancyArray::consoleInit()
{
	Con::addCommand( "ShellFancyArray", "clearColumns",               cShellFancyArrayClearColumns,       "array.clearColumns();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "getNumColumns",              cShellFancyArrayGetNumColumns,      "array.getNumColumns();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "addColumn",                  cShellFancyArrayAddColumn,          "array.addColumn( key, name, defaultWidth, minWidth, maxWidth{, flags} );", 7, 8 );
	Con::addCommand( "ShellFancyArray", "addRow",                     cShellFancyArrayAddRow,             "array.addRow();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "setNumRows",                 cShellFancyArraySetNumRows,         "array.setNumRows( numRows );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "rowCount",                   cShellFancyArrayRowCount,           "array.rowCount();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "clearList",                  cShellFancyArrayClearList,          "array.clearList();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "forceUpdate",                cShellFancyArrayForceUpdate,        "array.forceUpdate();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "setSelectedRow",             cShellFancyArraySetSelectedRow,     "array.setSelectedRow( row );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "setSortColumn",              cShellFancyArraySetSortColumn,      "array.setSortColumn( key );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "setSortIncreasing",          cShellFancyArraySetSortIncreasing,  "array.setSortIncreasing( <bool> );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "setSecondarySortColumn",     cSFASetSecondarySortColumn,         "array.setSecondarySortColumn( key );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "setSecondarySortIncreasing", cSFASetSecondarySortIncreasing,     "array.setSecondarySortIncreasing( <bool> );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "getSelectedRow",             cShellFancyArrayGetSelectedRow,     "array.getSelectedRow();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "getColumnKey",               cShellFancyArrayGetColumnKey,       "array.getColumnKey( index );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "getColumnWidth",             cShellFancyArrayGetColumnWidth,     "array.getColumnWidth( index );", 3, 3 );
	Con::addCommand( "ShellFancyArray", "getSortColumnKey",           cShellFancyArrayGetSortColumnKey,   "array.getSortColumnKey();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "getSortIncreasing",          cShellFancyArrayGetSortIncreasing,  "array.getSortIncreasing();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "getSecondarySortColumnKey",  cSFAGetSecondarySortColumnKey,      "array.getSecondarySortColumnKey();", 2, 2 );
	Con::addCommand( "ShellFancyArray", "getSecondarySortIncreasing", cSFAGetSecondarySortIncreasing,     "array.getSecondarySortIncreasing();", 2, 2 );
}

//------------------------------------------------------------------------------
void ShellFancyArray::clearColumns()
{
	mColumnInfoList.clear();
	mNumColumns = 0;
}

//------------------------------------------------------------------------------
void ShellFancyArray::addColumn( S32 key, const char* name, S32 defaultWidth, S32 minWidth, S32 maxWidth, const char* flags )
{
   ColumnInfo newColumn;
   newColumn.name = StringTable->insert( name, true );
   newColumn.key = key;
   newColumn.minWidth = getMax( mMinColumnWidth, minWidth );
   newColumn.maxWidth = maxWidth;
   newColumn.width = getMin( maxWidth, getMax( newColumn.minWidth, defaultWidth ) );
   newColumn.flags = 0;
   if ( flags )
   {
      if ( dStrstr( flags, "numeric" ) )
         newColumn.flags |= Column_Numeric;
      else if ( dStrstr( flags, "icon" ) )
         newColumn.flags |= Column_Icon;
      if ( dStrstr( flags, "center" ) )
         newColumn.flags |= Column_Center;
      else if ( dStrstr( flags, "right" ) )
         newColumn.flags |= Column_Right;
   }

   mColumnInfoList.push_back( newColumn );
	mNumColumns = mColumnInfoList.size();
	if ( mFixedHorizontal )
		mStartScrollRgn.x = mNumColumns;
}

//------------------------------------------------------------------------------
void ShellFancyArray::clearList()
{
   mSelectedRow = -1;
   mMouseOverRow = -2;
   setNumRows( 0 );
}

//------------------------------------------------------------------------------
void ShellFancyArray::updateList()
{
	// This function should be called whenever the contents of the
	// array have changed and usually only has to call setNumRows
	// and setUpdate.
}

//------------------------------------------------------------------------------
bool ShellFancyArray::onWake()
{
   if ( !Parent::onWake() )
      return false;

   // Get the font:
   mFont = mProfile->mFont;

   // Get the header font:
   if ( mHeaderFontType[0] )
      mHeaderFont = GFont::create( mHeaderFontType, mHeaderFontSize );

   if ( !mHeaderFont )
      mHeaderFont = mFont;

   // Get the cell textures:
   char buf[512];
	bool result;
   if ( mHeaderBitmap[0] )
   {
      dSprintf( buf, sizeof( buf ), "%s.png", mHeaderBitmap );
      mTexHeader = TextureHandle( buf, BitmapKeepTexture );
      result = createBitmapArray( mTexHeader.getBitmap(), mBmpBounds, StateCount, BmpCount );
      AssertFatal( result, "Failed to create the header bitmap array for the ShellFancyArray!" );
      mHeaderHeight = mBmpBounds[0].extent.y - ( 2 * mGlowOffset );
		mMinColumnWidth = mBmpBounds[StateCount * BmpLeft].extent.x + mBmpBounds[StateCount * BmpRight].extent.x - ( 2 * mGlowOffset );

		// Set the minimum extents:
		mMinExtent.y = mGlowOffset + mHeaderHeight;
		resize( mBounds.point, mBounds.extent );
   }

   if ( mSortArrowBitmap[0] )
   {
      dSprintf( buf, sizeof( buf ), "%s.png", mSortArrowBitmap );
      mTexSortArrow = TextureHandle( buf, BitmapTexture );
   }

	// Get the field:
	if ( mFieldBase[0] )
	{
      dSprintf( buf, sizeof( buf ), "%s_TL.png", mFieldBase );
      mTexLeftTop = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_TM.png", mFieldBase );
      mTexCenterTop = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_TR.png", mFieldBase );
      mTexRightTop = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_ML.png", mFieldBase );
      mTexLeftCenter = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_MM.png", mFieldBase );
      mTexCenter = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_MR.png", mFieldBase );
      mTexRightCenter = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_BL.png", mFieldBase );
      mTexLeftBottom = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_BM.png", mFieldBase );
      mTexCenterBottom = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_BR.png", mFieldBase );
      mTexRightBottom = TextureHandle( buf, BitmapTexture );
	}

	if ( mBarBase[0] )
   {
	   dSprintf( buf, sizeof( buf ), "%s_rol.png", mBarBase );
	   mTexCellRollover = TextureHandle( buf, BitmapTexture );
	   dSprintf( buf, sizeof( buf ), "%s_act.png", mBarBase );
	   mTexCellSelected = TextureHandle( buf, BitmapTexture );
	}

   return true;
}

//------------------------------------------------------------------------------
void ShellFancyArray::onSleep()
{
   Parent::onSleep();

   mFont = NULL;
   mHeaderFont = NULL;

   mTexHeader = NULL;
   mTexSortArrow = NULL;
   mTexLeftTop = NULL;
   mTexCenterTop = NULL;
   mTexRightTop = NULL;
   mTexLeftCenter = NULL;
   mTexCenter = NULL;
   mTexRightCenter = NULL;
   mTexLeftBottom = NULL;
   mTexCenterBottom = NULL;
   mTexRightBottom = NULL;
   mTexCellRollover = NULL;
   mTexCellSelected = NULL;
}

//------------------------------------------------------------------------------
bool ShellFancyArray::pointInColumn( bool inHeader, Point2I pt, S32 &column, bool &inResizeRgn, bool &resizeLeft )
{
   S32 columnLimit = ( mNumColumns > mStartScrollRgn.x ) ? mStartScrollRgn.x : mNumColumns;
   S32 startPos = 0, endPos = 0;
   inResizeRgn = false;

   if ( inHeader )
   {
      if ( pt.x < 0 || pt.y < 0 || pt.x >= mBounds.extent.x || pt.y >= ( mHeaderHeight + mGlowOffset ) )
         return false;
   }
   else
   {
      if ( pt.x < 0 || pt.y < mHeaderHeight || pt.x >= mBounds.extent.x || pt.y >= mBounds.extent.y )
         return false;
   }

   for ( column = 0; column < columnLimit; column++ )
   {
      endPos = startPos + mColumnInfoList[column].width;
      if ( pt.x < endPos )
      {
         if ( ( ( pt.x - startPos ) < 4 && column ) || ( ( endPos - pt.x ) < 4 
           && ( !mFixedHorizontal || column != mNumColumns - 1 ) ) )
            inResizeRgn = true;
         resizeLeft = pt.x < ( startPos + ( ( endPos - startPos ) / 2 ) );
         return true;
      }
      startPos = endPos;
   }

   RectI columnScrollRect;
   if ( getColumnScrollViewRect( columnScrollRect ) )
   {
      startPos = columnScrollRect.point.x + mScrollPanePos.x;
      for ( column = mStartScrollRgn.x; column < mColumnInfoList.size(); column++ )
      {
         endPos = startPos + mColumnInfoList[column].width;
         if ( pt.x < endPos )
         {
            if ( ( ( pt.x - startPos ) < 4 && column ) || ( ( endPos - pt.x ) < 4 
              && ( !mFixedHorizontal || column != mNumColumns - 1 ) ) )
               inResizeRgn = true;
            resizeLeft = ( pt.x < ( startPos + ( ( endPos - startPos ) / 2 ) ) );
            return true;
         }
         startPos = endPos;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
bool ShellFancyArray::getScrollRect( RectI &rect )
{
   bool hasHorizontalScroll = getColumnScrollViewRect( rect );
   rect.point.y = mHeaderHeight + mGlowOffset + ( mRowHeight * mStartScrollRgn.y );
   rect.extent.y = mBounds.extent.y - rect.point.y;

   return hasHorizontalScroll;
}

//------------------------------------------------------------------------------
void ShellFancyArray::setSortColumnKey( S32 newKey )
{
   if ( mSortColumnKey == newKey )
      mSortInc = !mSortInc;
   else
      mSortColumnKey = newKey;

   sort();
}

//------------------------------------------------------------------------------
void ShellFancyArray::setSecondarySortColumnKey( S32 newKey )
{
   if ( mSecondarySortColumnKey == newKey )
      mSecondarySortInc = !mSecondarySortInc;
   else
      mSecondarySortColumnKey = ( newKey == mSortColumnKey ) ? -1 : newKey;

   sort();
}

//------------------------------------------------------------------------------
void ShellFancyArray::sort()
{
}

//------------------------------------------------------------------------------
void ShellFancyArray::selectCell( S32 row, S32 column )
{
   if ( row < 0 || row >= mNumRows )
   {
      mSelectedRow = -1;
      return;
   }

   mSelectedRow = row;
   onCellSelected( row, column );
   scrollSelectedRowVisible();
   setUpdate();
}

//------------------------------------------------------------------------------
void ShellFancyArray::setNumRows( S32 numRows )
{
   if ( mNumRows > numRows )
   {
      // Need to scroll off the dead rows:
      if ( numRows > mStartScrollRgn.y )
         mScrollPanePos.y += ( mNumRows - numRows ) * mRowHeight;
      else
         mScrollPanePos.y = 0;

      if ( mScrollPanePos.y > 0 )
         mScrollPanePos.y = 0;
   }

   if ( numRows != mNumRows )
   {
      mNumRows = numRows;
      if ( mNumRows )
         scrollSelectedRowVisible();
      setUpdate();
   }
}

//------------------------------------------------------------------------------
S32 ShellFancyArray::getNoScrollWidth()
{
   S32 result = 0;
   U32 colCount = getMin( mStartScrollRgn.x, mColumnInfoList.size() );
   for ( U32 i = 0; i < colCount; i++ )
      result += mColumnInfoList[i].width;

   return( result );
}

//------------------------------------------------------------------------------
S32 ShellFancyArray::getColumnKey( S32 index )
{
   if ( index < 0 || index >= mColumnInfoList.size() )
      return( -1 );

   return( mColumnInfoList[index].key );
}

//------------------------------------------------------------------------------
S32 ShellFancyArray::getColumnWidth( S32 index )
{
   if ( index < 0 || index >= mColumnInfoList.size() )
      return( -1 );

   return( mColumnInfoList[index].width );
}

//------------------------------------------------------------------------------
void ShellFancyArray::forceFillScrollRegion()
{
   if ( mStartScrollRgn.x >= mColumnInfoList.size() )
      return;

   RectI r;
   if ( !getColumnScrollViewRect( r ) )
      return;

   S32 i, len = 0;
   for ( i = mStartScrollRgn.x; i < mColumnInfoList.size(); i++ )
      len += mColumnInfoList[i].width;

   if ( len >= r.extent.x )
      return;

   S32 delta = ( r.extent.x - len ) / ( mColumnInfoList.size() - mStartScrollRgn.x );
   len = 0;

   for ( i = mStartScrollRgn.x; i < mColumnInfoList.size() - 1; i++ )
   {
      mColumnInfoList[i].width += delta;
      if ( mColumnInfoList[i].maxWidth < mColumnInfoList[i].width )
         mColumnInfoList[i].maxWidth = mColumnInfoList[i].width;

      len += mColumnInfoList[i].width;
   }

   mColumnInfoList[i].width = r.extent.x - len;
   if ( mColumnInfoList[i].maxWidth < mColumnInfoList[i].width )
      mColumnInfoList[i].maxWidth = mColumnInfoList[i].width;
   if ( mColumnInfoList[i].minWidth > mColumnInfoList[i].width )
      mColumnInfoList[i].minWidth = mColumnInfoList[i].width;
}

//------------------------------------------------------------------------------
void ShellFancyArray::onHeaderAction( S32 column )
{
   setSortColumnKey( mColumnInfoList[column].key );
   Con::executef( this, 3, "onSetSortKey", Con::getIntArg( mSortColumnKey ), Con::getIntArg( mSortInc ) );
}

//------------------------------------------------------------------------------
void ShellFancyArray::onSecondaryHeaderAction( S32 column )
{
   setSecondarySortColumnKey( mColumnInfoList[column].key );
   Con::executef( this, 3, "onSetSecondarySortKey", Con::getIntArg( mSecondarySortColumnKey ), Con::getIntArg( mSecondarySortInc ) );
}

//------------------------------------------------------------------------------
void ShellFancyArray::onCellSelected( S32 row, S32 column )
{
	Con::executef( this, 3, "onSelect", Con::getIntArg( row ), Con::getIntArg( column ) );

	// Call the console function:
	if ( mConsoleCommand[0] )
		Con::evaluate( mConsoleCommand, false );
}

//------------------------------------------------------------------------------
void ShellFancyArray::scrollSelectedRowVisible()
{
   RectI r;
   if ( !getRowScrollViewRect( r ) )
      return;

   // First, make sure we are within the acceptable range:
   S32 totalLen = ( mNumRows - mStartScrollRgn.y ) * mRowHeight;
   if ( totalLen > r.extent.y && ( ( mScrollPanePos.y + totalLen ) < r.extent.y ) )
   {
      mScrollPanePos.y = r.extent.y - totalLen;
      setUpdate();
   }

   if ( mSelectedRow < 0 || mSelectedRow >= mNumRows || mSelectedRow < mStartScrollRgn.y )
      return;

   S32 rowStartPos_y = r.point.y + mScrollPanePos.y + ( mRowHeight * ( mSelectedRow - mStartScrollRgn.y ) );
   S32 rowEndPos_y = rowStartPos_y + mRowHeight;

   if ( rowStartPos_y >= r.point.y && rowEndPos_y <= ( r.point.y + r.extent.y ) )
      return;

   // Position the scrollable row region such that the selected row is visible:
   if ( rowStartPos_y < r.point.y )
      mScrollPanePos.y += ( r.point.y - rowStartPos_y );
   if ( rowEndPos_y > ( r.point.y + r.extent.y ) )
      mScrollPanePos.y += ( ( r.point.y + r.extent.y ) - rowEndPos_y ) + 1;
}

//------------------------------------------------------------------------------
void ShellFancyArray::computeFixedResizingVals()
{
   if ( mActiveColumn < 0 || mActiveColumn >= mNumColumns || mActiveColumn >= mStartScrollRgn.x )
      return;

   S32 i;
   mAbsResizeLeftMargin = 0;
   for ( i = 0; i < mActiveColumn; i++ )
      mAbsResizeLeftMargin += mColumnInfoList[i].width;
   mAbsResizeLeftMargin += mColumnInfoList[mActiveColumn].minWidth;

   mAbsResizeRightMargin = mBounds.extent.x;
   S32 columnLimit = mNumColumns - 1;
   if ( mStartScrollRgn.x < mNumColumns )
   {
      // Subtract 100 if we have a scroll region.
      // ( the minimum scroll region size )
      mAbsResizeRightMargin -= 100;
      columnLimit = mStartScrollRgn.x - 1;
   }

   for ( i = columnLimit; i < mActiveColumn; i-- )
      mAbsResizeRightMargin -= mColumnInfoList[i].minWidth;
}

//------------------------------------------------------------------------------
void ShellFancyArray::resizeFixedColumn( const GuiEvent &event )
{
   if ( mActiveColumn < 0 || mActiveColumn >= mNumColumns || mActiveColumn > mStartScrollRgn.x )
      return;

   Point2I pt = globalToLocalCoord( event.mousePoint );

   if ( pt.x < mAbsResizeLeftMargin )
      pt.x = mAbsResizeLeftMargin;

   if ( pt.x > mAbsResizeRightMargin )
      pt.x = mAbsResizeRightMargin;

   S32 widthDelta = mColumnInfoList[mActiveColumn].width;
   mColumnInfoList[mActiveColumn].width = pt.x - mAbsResizeLeftMargin + mColumnInfoList[mActiveColumn].minWidth;
   if ( mColumnInfoList[mActiveColumn].width > mColumnInfoList[mActiveColumn].maxWidth )
      mColumnInfoList[mActiveColumn].width = mColumnInfoList[mActiveColumn].maxWidth;
   widthDelta = mColumnInfoList[mActiveColumn].width - widthDelta;

   ShellFancyArrayScrollCtrl* daddy = static_cast<ShellFancyArrayScrollCtrl*>( getParent() );
   if ( daddy )
      daddy->positionChildren();

   setUpdate();
}

//------------------------------------------------------------------------------
void ShellFancyArray::resizeScrollColumn( const GuiEvent &event )
{
   if ( mActiveColumn < mStartScrollRgn.x || mActiveColumn >= mNumColumns )
      return;

   S32 deltaX = event.mousePoint.x - mDragAnchor.x;
   mColumnInfoList[mActiveColumn].width += ( deltaX - mColumnInfoList[mActiveColumn].width + mResizeColumnOrigSize );
   if ( mColumnInfoList[mActiveColumn].width < mColumnInfoList[mActiveColumn].minWidth )
      mColumnInfoList[mActiveColumn].width = mColumnInfoList[mActiveColumn].minWidth;
   if ( mColumnInfoList[mActiveColumn].width > mColumnInfoList[mActiveColumn].maxWidth )
      mColumnInfoList[mActiveColumn].width = mColumnInfoList[mActiveColumn].maxWidth;

   ShellFancyArrayScrollCtrl* daddy = static_cast<ShellFancyArrayScrollCtrl*>( getParent() );
   if ( daddy )
   {
      if ( mActiveColumn == mNumColumns - 1 )
      {
         RectI r;
         if ( getScrollRect( r ) )
            mScrollPanePos.x = r.extent.x - getScrollExtent().x;
      }

      daddy->positionChildren();
   }

   setUpdate();
}

//------------------------------------------------------------------------------
Point2I ShellFancyArray::getScrollExtent()
{
   Point2I extent( 0, 0 );
   if ( mNumRows > mStartScrollRgn.y )
      extent.y = ( mNumRows - mStartScrollRgn.y ) * mRowHeight;

   if ( mNumColumns > mStartScrollRgn.x )
   {
      for ( S32 i = mStartScrollRgn.x; i < mNumColumns; i++ )
         extent.x += mColumnInfoList[i].width;
   }

   return extent;
}

//------------------------------------------------------------------------------
void ShellFancyArray::onMouseDown( const GuiEvent& event )
{
   if ( !mActive || !mVisible || !mAwake )
      return;

	if ( mProfile->mCanKeyFocus )
		setFirstResponder();

	// Lock the mouse to this control:
	mouseLock();

   Point2I pt = globalToLocalCoord( event.mousePoint );
   S32 prevSelected, column;
   bool inResizeRgn, resizeLeft;

   if ( pt.y > ( mHeaderHeight + mGlowOffset ) )
   {
      if ( mNoSelect )
         return;

      // In browser region, so select the row:
      if ( pointInColumn( false, pt, column, inResizeRgn, resizeLeft ) )
      {
         S32 rowPos = ( pt.y - ( mHeaderHeight + mGlowOffset ) ) / mRowHeight;
         if ( rowPos < mStartScrollRgn.y )
         {
				prevSelected = mSelectedRow;
            selectCell( rowPos, column );

				// Test for double-click on same cell:
				if ( ( event.mouseClickCount > 1 ) && ( prevSelected == mSelectedRow ) && mAltConsoleCommand[0] )
					Con::evaluate( mAltConsoleCommand );
            return;
         }

         RectI rowScrollRect;
         if ( getRowScrollViewRect( rowScrollRect ) )
         {
            rowPos = ( ( pt.y - rowScrollRect.point.y - mScrollPanePos.y ) / mRowHeight ) + mStartScrollRgn.y;
            if ( rowPos < mNumRows )
            {
					prevSelected = mSelectedRow;
               selectCell( rowPos, column );

					// Test for double-click on same cell:
					if ( ( event.mouseClickCount > 1 ) && ( prevSelected == mSelectedRow ) && mAltConsoleCommand[0] )
						Con::evaluate( mAltConsoleCommand );
            }
         }
      }
      return;
   }

   if ( pointInColumn( true, pt, column, inResizeRgn, resizeLeft ) )
   {
      if ( inResizeRgn )
      {
			// Start resizing:
         mColumnState = Resizing;
         mActiveColumn = column;
			mMouseOverColumn = -1;
         if ( resizeLeft )
            mActiveColumn--;
         mResizeFixedColumn = mActiveColumn < mStartScrollRgn.x;
         if ( mResizeFixedColumn )
            computeFixedResizingVals();
         else
         {
            mDragAnchor = event.mousePoint;
            mResizeColumnOrigSize = mColumnInfoList[mActiveColumn].width;
         }
      }
      else if ( mHeaderSort )
      {
      	// Select the column header:
         mColumnState = Sorting;
         mActiveColumn = column;
         mDragAnchor = event.mousePoint;
         determineRepositionColumn( pt );

		   if ( mProfile->mSoundButtonDown )
		   {
		      F32 pan = ( F32( event.mousePoint.x ) / F32( Canvas->mBounds.extent.x ) * 2.0f - 1.0f ) * 0.8f;
		      AUDIOHANDLE handle = alxCreateSource( mProfile->mSoundButtonDown );
		      alxSourcef( handle, AL_PAN, pan );
		      alxPlay( handle );
		   }

			setUpdate();
      }
   }
}

//------------------------------------------------------------------------------
void ShellFancyArray::onMouseUp( const GuiEvent& event )
{
   if ( !mActive || !mVisible || !mAwake )
      return;

   Point2I pt = globalToLocalCoord( event.mousePoint );

   switch ( mColumnState )
   {
      case Sorting:
      {
         S32 column;
         bool inResizeRgn, resizeLeft;

         if ( pt.y <= ( mHeaderHeight + mGlowOffset ) && pointInColumn( true, pt, column, inResizeRgn, resizeLeft ) )
         {
            if ( column == mActiveColumn )
               onHeaderAction( column );
         }
         break;
      }

      case Repositioning:
      {
         if ( mActiveColumn == mRepositionColumnTo || ( mActiveColumn + 1 ) == mRepositionColumnTo )
            break;

         ColumnInfo temp = mColumnInfoList[mActiveColumn];
         S32 insertPos = mRepositionColumnTo;
         if ( mActiveColumn < mRepositionColumnTo )
            insertPos--;

         mColumnInfoList.erase( mActiveColumn );
         mColumnInfoList.insert( insertPos );
         mColumnInfoList[insertPos] = temp;

         // Call the console function:
  	      Con::executef( this, 3, "onColumnRepositioned", Con::getIntArg( mActiveColumn ), Con::getIntArg( mRepositionColumnTo ) );
         break;
      }

      case Resizing:
         // Call the console function:
  	      Con::executef( this, 4, "onColumnResize", Con::getIntArg( mActiveColumn ), 
  	            Con::getIntArg( mColumnInfoList[mActiveColumn].width ), 
  	            Con::getIntArg( mColumnInfoList[mActiveColumn].key ) );
         break;
   }

	// Unlock the mouse:
	setUpdate();
	mouseUnlock();
   Canvas->setCursor( mDefaultCursor );
   mColumnState = None;
}

//------------------------------------------------------------------------------
void ShellFancyArray::onMouseMove( const GuiEvent& event )
{
   Point2I pt = globalToLocalCoord( event.mousePoint );

   if ( pt.y < ( mGlowOffset + mHeaderHeight ) )
   {
		GuiCursor* newCursor = mDefaultCursor;
      S32 column;
      bool inResizeRgn, unused2;
      if ( pointInColumn( true, pt, column, inResizeRgn, unused2 ) )
		{
			if ( inResizeRgn && mResizeCursor )
			{
         	setMouseOverColumn( -1 );
				newCursor = mResizeCursor;
			}
      	else if ( mHeaderSort )
         	setMouseOverColumn( column, event.mousePoint.x );
		}
      setMouseOverRow( -1 );  // -1 is the code for the header
		Canvas->setCursor( newCursor );
      return;
   }
   else
   {
		Canvas->setCursor( mDefaultCursor );
		setMouseOverColumn( -1 );
      S32 row = ( pt.y - mGlowOffset - mHeaderHeight ) / mRowHeight;
      if ( row < mStartScrollRgn.y )
      {
         setMouseOverRow( row );
         return;
      }

      RectI rowScrollRect;
      if ( getRowScrollViewRect( rowScrollRect ) )
      {
         row = ( ( pt.y - rowScrollRect.point.y - mScrollPanePos.y ) / mRowHeight ) + mStartScrollRgn.y;
         if ( row < mNumRows )
         {
            setMouseOverRow( row );
            return;
         }
      }
   }

   setMouseOverRow( -2 );
}

//------------------------------------------------------------------------------
void ShellFancyArray::onMouseDragged( const GuiEvent& event )
{
   if ( mColumnState == None )
      return;

   if ( mColumnState == Resizing )
   {
      if ( mResizeFixedColumn )
         resizeFixedColumn( event );
      else
         resizeScrollColumn( event );
   }
   else
   {
      if ( !mAllowReposition || mActiveColumn < mStartScrollRgn.x || ( mColumnInfoList.size() - mStartScrollRgn.x ) <= 1 )
         mColumnState = Sorting;
      else
         mColumnState = ( mAbs( event.mousePoint.x - mDragAnchor.x ) > 4 ) ? Repositioning : Sorting;

      if ( mColumnState == Repositioning )
      {
         if ( mRepositionCursor )
            Canvas->setCursor( mRepositionCursor );
         RectI columnRect;
         if ( getColumnScrollViewRect( columnRect ) )
         {
            // Scroll the columns for positioning if necessary:
            columnRect.point = localToGlobalCoord( columnRect.point );
            Point2I newMousePt = event.mousePoint;

            if ( event.mousePoint.x < columnRect.point.x )
            {
               // Scroll right:
               newMousePt.set( columnRect.point.x, event.mousePoint.y );

               mScrollPanePos.x += ( columnRect.point.x - event.mousePoint.x );
               if ( mScrollPanePos.x > 0 )
               {
                  mScrollPanePos.x = 0;
                  ShellFancyArrayScrollCtrl* daddy = static_cast<ShellFancyArrayScrollCtrl*>( getParent() );
                  if ( daddy )
                     daddy->positionChildren();
               }
               GuiCanvas* root = getRoot();
               if ( root )
                  root->setCursorPos( newMousePt );
               setUpdate();
            }
            else if ( event.mousePoint.x > ( columnRect.point.x + columnRect.extent.x ) )
            {
               // Scroll left:
               newMousePt.set( columnRect.point.x + columnRect.extent.x, event.mousePoint.y );

               mScrollPanePos.x += ( columnRect.point.x + columnRect.extent.x - event.mousePoint.x );
               if ( columnRect.extent.x > ( getScrollExtent().x + mScrollPanePos.x ) )
               {
                  mScrollPanePos.x = columnRect.extent.x - getScrollExtent().x;
                  ShellFancyArrayScrollCtrl* daddy = static_cast<ShellFancyArrayScrollCtrl*>( getParent() );
                  if ( daddy )
                     daddy->positionChildren();
               }
               GuiCanvas* root = getRoot();
               if ( root )
                  root->setCursorPos( newMousePt );
               setUpdate();
            }

            newMousePt = globalToLocalCoord( newMousePt );
            determineRepositionColumn( newMousePt );
         }
      }
   }
}

//------------------------------------------------------------------------------
void ShellFancyArray::onMouseEnter( const GuiEvent &/*event*/ )
{
   // Change the cursor to the appropriate one:
   if ( mColumnState == None )
      return;
   if ( mColumnState == Resizing && mResizeCursor )
      Canvas->setCursor( mResizeCursor );
   if ( mColumnState == Repositioning && mRepositionCursor )
      Canvas->setCursor( mRepositionCursor );
}

//------------------------------------------------------------------------------
void ShellFancyArray::onMouseLeave( const GuiEvent &/*event*/ )
{
   // Restore the default cursor:
   Canvas->setCursor( mDefaultCursor );
   setMouseOverRow( -2 );
   setMouseOverColumn( -1 );
}

//------------------------------------------------------------------------------
void ShellFancyArray::onRightMouseDown( const GuiEvent &event )
{
   if ( !mActive || !mAwake || !mVisible ) 
      return;

   Parent::onRightMouseDown( event );

   // Ignore if left mouse button is down:
   if ( mColumnState != None )
      return;

   Point2I pt = globalToLocalCoord( event.mousePoint );
   S32 column, row;
   bool inResizeRgn, resizeLeft;
   if ( pt.y <= ( mHeaderHeight + mGlowOffset ) )
   {
#if 0 
      // Not yet ready for prime time...
      if ( mHeaderSort )
      {
         if ( pointInColumn( true, pt, column, inResizeRgn, resizeLeft ) )
         {
            if ( !inResizeRgn )
            {
               // Set the secondary sort column:
               mColumnState = SecondarySorting;
               mActiveColumn = column;

		         if ( mProfile->mSoundButtonDown )
		         {
		            F32 pan = ( F32( event.mousePoint.x ) / F32( Canvas->mBounds.extent.x ) * 2.0f - 1.0f ) * 0.8f;
		            AUDIOHANDLE handle = alxCreateSource( mProfile->mSoundButtonDown );
		            alxSourcef( handle, AL_PAN, pan );
		            alxPlay( handle );
		         }
            }
         }
      }
#endif

      return;
   }

   // In browser region, so find the hit row:
   if ( pointInColumn( false, pt, column, inResizeRgn, resizeLeft ) )
   {
      row = ( pt.y - ( mHeaderHeight + mGlowOffset ) ) / mRowHeight;
      if ( row >= mStartScrollRgn.y )
      {
         RectI rowScrollRect;
         if ( getRowScrollViewRect( rowScrollRect ) )
         {
            row = ( ( pt.y - rowScrollRect.point.y - mScrollPanePos.y ) / mRowHeight ) + mStartScrollRgn.y;
            if ( row >= mNumRows )
               return; // Didn't actually hit a row...
         }
      }
   }

   // Pass it to the console: 
   char buf[32];
   dSprintf( buf, sizeof( buf ), "%d %d", event.mousePoint.x, event.mousePoint.y );
  	Con::executef( this, 4, "onRightMouseDown", Con::getIntArg( column ), Con::getIntArg( row ), buf );
}

//------------------------------------------------------------------------------
void ShellFancyArray::onRightMouseUp( const GuiEvent &event )
{
   if ( !mActive || !mAwake || !mVisible ) 
      return;

   if ( mColumnState == SecondarySorting )
   {
      Point2I pt = globalToLocalCoord( event.mousePoint );
      S32 column;
      bool inResizeRgn, resizeLeft;
      if ( pt.y <= ( mHeaderHeight + mGlowOffset )
        && pointInColumn( true, pt, column, inResizeRgn, resizeLeft ) )
      {
         if ( column == mActiveColumn )
            onSecondaryHeaderAction( column );
      }
   }

   mColumnState = None;
}

//------------------------------------------------------------------------------
bool ShellFancyArray::onMouseWheelUp( const GuiEvent &event )
{
   if ( !mVisible || !mAwake )
      return( false );

   // scroll up a row...
   if ( mScrollPanePos.y + mRowHeight > 0 )
      mScrollPanePos.y = 0;
   else
      mScrollPanePos.y += mRowHeight;

   // update the mouse over row...
   onMouseMove( event );
   return( true );
}

//------------------------------------------------------------------------------
bool ShellFancyArray::onMouseWheelDown( const GuiEvent &event )
{
   if ( !mVisible || !mAwake )
      return( false );

   RectI r;
   if ( !getRowScrollViewRect( r ) )
      return( true );

   S32 minPos = getMin( r.extent.y - ( ( mNumRows - mStartScrollRgn.y ) * mRowHeight ), 0 );

   // scroll down a row...
   if ( mScrollPanePos.y - mRowHeight < minPos )
      mScrollPanePos.y = minPos;
   else
      mScrollPanePos.y -= mRowHeight;

   // update the mouse over row...
   onMouseMove( event );
   return( true );
}

//------------------------------------------------------------------------------
bool ShellFancyArray::onKeyDown( const GuiEvent& event )
{
   if ( !mVisible || !mActive || !mAwake )
      return true;

   if ( mSelectedRow < 0 )
      return true;
   
   if ( event.modifier == 0 )
   {
      switch ( event.keyCode )
      {
         case KEY_RETURN:
				if ( mSelectedRow > 0 && mAltConsoleCommand[0] )
				{
					Con::evaluate( mAltConsoleCommand, false );
            	return true;
				}
				break;

         case KEY_UP:
            if ( mSelectedRow > 0 )
               selectCell( mSelectedRow - 1, 0 );
            return true;

         case KEY_DOWN:
            if ( mSelectedRow < ( mNumRows - 1 ) )
               selectCell( mSelectedRow + 1, 0 );
            return true;
      }
   }

   // Not processed, so pass to parent:
   return Parent::onKeyDown( event );
}

//------------------------------------------------------------------------------
void ShellFancyArray::resize( const Point2I &newPos, const Point2I &newExtent )
{
	Parent::resize( newPos, newExtent );

	if ( mFixedHorizontal && mNumColumns > 0 )
	{
		// Fit the columns to the new width:
		S32 i, width = 0;
		S32 realWidth = newExtent.x - mGlowOffset;
		for ( i = 0; i < mNumColumns; i++ )
			width += mColumnInfoList[i].width;

		if ( width == realWidth )
			return;

		S32 delta = realWidth - width;
		if ( delta > 0 )
		{
			// Just add the leftovers to the last column:
			mColumnInfoList[mNumColumns - 1].width += delta;
			if ( mColumnInfoList[mNumColumns - 1].maxWidth < mColumnInfoList[mNumColumns - 1].width )
				mColumnInfoList[mNumColumns - 1].maxWidth = mColumnInfoList[mNumColumns - 1].width;
		}
 		else
 		{
 			// Subtract width to make the columns fit, but don't make
 			// columns smaller than their minimum widths.
 			i = mNumColumns - 1;
 			while ( i >= 0 )
 			{
 				mColumnInfoList[i].width += delta;
 				if ( mColumnInfoList[i].width < mColumnInfoList[i].minWidth )
 				{
 					delta = mColumnInfoList[i].width - mColumnInfoList[i].minWidth;
 					mColumnInfoList[i].width = mColumnInfoList[i].minWidth;
 				}
				else
					break;
 				i--;
 			}
 		}
	}
}

//------------------------------------------------------------------------------
void ShellFancyArray::onRenderColumnHeader( Point2I offset, RectI clipRect, S32 column, bool mouseOver )
{
   ColumnInfo* ci = &mColumnInfoList[column];

   bool realMouseOver = mHeaderSort ? mouseOver : false;

   // Draw the button-like background:
   dglClearBitmapModulation();
	RectI drawRect;
	drawRect.point = offset - Point2I( mGlowOffset, mGlowOffset );
	U32 state = realMouseOver ? ( ( ( mColumnState == Sorting || mColumnState == Repositioning ) && mActiveColumn == column ) ? StatePressed : StateRollover ) : StateNormal;

   // Draw the left edge:
   U32 bitmap = BmpLeft * StateCount + state;
   dglDrawBitmapSR( mTexHeader, drawRect.point, mBmpBounds[bitmap] );

   // Draw the center section:
   drawRect.point.x += mBmpBounds[bitmap].extent.x;
   drawRect.extent.x = ci->width - mBmpBounds[bitmap].extent.x - mBmpBounds[BmpRight * StateCount].extent.x + ( 2 * mGlowOffset ) - 1;
   drawRect.extent.y = mHeaderHeight + ( 2 * mGlowOffset );
   if ( drawRect.extent.x > 0 )
   {
      bitmap = BmpCenter * StateCount + state;
      dglDrawBitmapStretchSR( mTexHeader, drawRect, mBmpBounds[bitmap] );
   }

   // Draw the right edge:
   drawRect.point.x += drawRect.extent.x;
   bitmap = BmpRight * StateCount + state;
   dglDrawBitmapSR( mTexHeader, drawRect.point, mBmpBounds[bitmap] );

   // If this is the sort column, draw the sort arrow:
   bool arrowDrawn = false;
   if ( mHeaderSort && mTexSortArrow
     && ( ci->key == mSortColumnKey || ci->key == mSecondarySortColumnKey ) ) 
   {
      bool flip;
      if ( ci->key == mSortColumnKey )
         flip = mSortInc;
      else
      {  
         dglSetBitmapModulation( ColorF( 1.0f, 1.0f, 1.0f, 0.5f ) );
         flip = mSecondarySortInc;
      }
      drawRect.point.x = offset.x + ci->width - mTexSortArrow.getWidth() - 6;
      drawRect.point.y = ( mHeaderHeight > mTexSortArrow.getHeight() ) ? offset.y + ( ( mHeaderHeight - mTexSortArrow.getHeight() ) / 2 ): offset.y;
      dglDrawBitmap( mTexSortArrow, drawRect.point, ( flip ? GFlip_Y : GFlip_None ) );
      arrowDrawn = true;
   }

   // Draw the text:
   S32 textWidth = mHeaderFont->getStrWidth( ci->name );
	drawRect.point.x = offset.x + getMax( 4, ( ci->width - textWidth ) / 2 );
	drawRect.point.y = offset.y + ( ( mHeaderHeight - mHeaderFont->getHeight() ) / 2 );
	drawRect.extent.x = ci->width + ( offset.x - drawRect.point.x ) - ( arrowDrawn ? mTexSortArrow.getWidth() + 6 : 4 );
   drawRect.extent.y = mHeaderHeight;
	if ( drawRect.extent.x > 0 )
	{
		RectI textRect( drawRect );
		if ( textRect.intersect( clipRect ) )
		{
			dglSetClipRect( textRect );
		   dglSetBitmapModulation( realMouseOver ? mHeaderFontColorHL : mHeaderFontColor );
		   dglDrawText( mHeaderFont, drawRect.point, ci->name );
		}
	}
}

//------------------------------------------------------------------------------
// Most likely you will want to override this function since you probably 
// want something drawn in the cells...left this open so you can use text, bitmaps, etc.
void ShellFancyArray::onRenderCell( Point2I offset, Point2I cell, bool selected, bool mouseOver )
{
   ColumnInfo* ci = &mColumnInfoList[cell.x];

   // If mouse is over or row is selected, draw the background:
   dglClearBitmapModulation();
   RectI drawRect( offset, Point2I( ci->width, mRowHeight ) );
   if ( selected )
      dglDrawBitmapStretch( mTexCellSelected, drawRect );
   else if ( mouseOver )
      dglDrawBitmapStretch( mTexCellRollover, drawRect );

	// Draw cell contents...

   // Draw the cell limit:
	if ( mDrawCellSeparators && mSeparatorColor )
	{
	   Point2I leftPt( offset.x, offset.y + mRowHeight );
	   Point2I rightPt( offset.x + ci->width, leftPt.y );
	   dglDrawLine( leftPt, rightPt, mSeparatorColor );
	}
}  

//------------------------------------------------------------------------------
void ShellFancyArray::onRender( Point2I offset, const RectI &updateRect, GuiControl* /*firstResponder*/ )
{
   RectI clipRect( updateRect );

	// Draw the background field:
	if ( mTexLeftTop && mTexCenterTop && mTexRightTop && mTexLeftCenter && mTexCenter && mTexRightCenter
	  && mTexLeftBottom && mTexCenterBottom && mTexRightBottom )
	{
      RectI drawRect;
      dglClearBitmapModulation();
   
      U32 stretchWidth = mBounds.extent.x - mTexLeftTop.getWidth() - mTexRightTop.getWidth() - mGlowOffset;
      U32 topEdgeHeight = mBounds.extent.y - mHeaderHeight - mGlowOffset - mTexLeftBottom.getHeight();
		if ( topEdgeHeight > mTexLeftTop.getHeight() )
			topEdgeHeight = mTexLeftTop.getHeight();

      // Draw upper left corner:
		drawRect.point = offset + Point2I( mGlowOffset, mHeaderHeight + mGlowOffset );
		drawRect.extent.x = mTexLeftTop.getWidth();
		drawRect.extent.y = topEdgeHeight;
      dglDrawBitmapStretch( mTexLeftTop, drawRect );

      // Draw upper center edge:
      drawRect.point.x += drawRect.extent.x;
      drawRect.extent.x = stretchWidth;
		if ( drawRect.extent.x > 0 )
      	dglDrawBitmapStretch( mTexCenterTop, drawRect );

      // Draw upper right corner:
      drawRect.point.x += drawRect.extent.x;
		drawRect.extent.x = mTexRightTop.getWidth();
      dglDrawBitmapStretch( mTexRightTop, drawRect );

      drawRect.point.x = offset.x + mGlowOffset;
      drawRect.point.y += drawRect.extent.y;
      drawRect.extent.y = mBounds.extent.y - mHeaderHeight - mGlowOffset - topEdgeHeight - mTexLeftBottom.getHeight();
		if ( drawRect.extent.y > 0 )
		{
         // Draw center left edge:
      	drawRect.extent.x = mTexLeftCenter.getWidth();
      	dglDrawBitmapStretch( mTexLeftCenter, drawRect );

	      // Draw center:
	      drawRect.point.x += drawRect.extent.x;
	      drawRect.extent.x = stretchWidth;
			if ( drawRect.extent.x > 0 )
	      	dglDrawBitmapStretch( mTexCenter, drawRect );

	      // Draw center right edge:
	      drawRect.point.x += drawRect.extent.x;
	      drawRect.extent.x = mTexRightCenter.getWidth();
	      dglDrawBitmapStretch( mTexRightCenter, drawRect );
		}

      // Draw bottom left corner:
      drawRect.point.x = offset.x + mGlowOffset;
      drawRect.point.y += drawRect.extent.y;
      dglDrawBitmap( mTexLeftBottom, drawRect.point );

      // Draw bottom center edge:
      drawRect.point.x += mTexLeftBottom.getWidth();
      drawRect.extent.x = stretchWidth;
      drawRect.extent.y = mTexCenterBottom.getHeight();
		if ( drawRect.extent.x > 0 )
      	dglDrawBitmapStretch( mTexCenterBottom, drawRect );

      // Draw bottom right corner:
      drawRect.point.x += drawRect.extent.x;
      dglDrawBitmap( mTexRightBottom, drawRect.point );
	}

   // First draw the non-scrollable columns:
   S32 columnLimit = ( mNumColumns > mStartScrollRgn.x ) ? mStartScrollRgn.x : mNumColumns;
   S32 colStartPos_x = offset.x + mGlowOffset;
	S32 colStartPos_y = offset.y + mGlowOffset;
   RectI rowScrollRect;
   bool hasRowScrollRect = getRowScrollViewRect( rowScrollRect );
   rowScrollRect.point += offset;

   S32 column;
   for ( column = 0; column < columnLimit; column++ )
   {
      dglSetClipRect( clipRect );
      drawColumn( Point2I( colStartPos_x, colStartPos_y ), clipRect, hasRowScrollRect, rowScrollRect, column );
      colStartPos_x += mColumnInfoList[column].width;
   }

   // Now draw the columns within the scroll range:
   RectI columnScrollRect;
   if ( getColumnScrollViewRect( columnScrollRect ) )
   {
      columnScrollRect.point += offset;

      colStartPos_x = columnScrollRect.point.x + mScrollPanePos.x;

      if ( columnScrollRect.intersect( clipRect ) )
      {
         for ( column = mStartScrollRgn.x; column < mColumnInfoList.size(); column++ )
         {
            dglSetClipRect( columnScrollRect );
            drawColumn( Point2I( colStartPos_x, colStartPos_y ), columnScrollRect, hasRowScrollRect, rowScrollRect, column );
            colStartPos_x += mColumnInfoList[column].width;
         }
      }
   }
}

//------------------------------------------------------------------------------
S32 ShellFancyArray::findColumn( S32 key )
{
   for ( S32 col = 0; col < mNumColumns; col++ )
   {
      if ( mColumnInfoList[col].key == key )
         return( col );
   }

   return( -1 );
}

//------------------------------------------------------------------------------
void ShellFancyArray::drawColumn( Point2I offset, RectI clipRect, bool hasRowScrollRect, RectI rowScrollRect, S32 column )
{
   S32 colStartPos_x = offset.x;
   S32 colEndPos_x = offset.x + mColumnInfoList[column].width;

   // Draw the header first:
   RectI headerRect( colStartPos_x - mGlowOffset, offset.y - mGlowOffset, mColumnInfoList[column].width + ( 2 * mGlowOffset ) - 1, mHeaderHeight + ( 2 * mGlowOffset ) );
   if ( headerRect.intersect( clipRect ) )
   {
      dglSetClipRect( headerRect );
      bool mouseOver = ( ( mMouseOverRow == -1 ) && ( mMouseOverColumn == column ) );
      onRenderColumnHeader( Point2I( colStartPos_x, offset.y ), headerRect, column, mouseOver );
   }

   // Now any non-scrollable rows:
   S32 rowLimit = ( mNumRows > mStartScrollRgn.y ) ? mStartScrollRgn.y : mNumRows;
   S32 rowStartPos_y = offset.y + mHeaderHeight + mGlowOffset;
   S32 rowEndPos_y = rowStartPos_y + mRowHeight;
   S32 row;
   for ( row = 0; row < rowLimit; row++ )
   {
      RectI rowRect( colStartPos_x, rowStartPos_y, colEndPos_x - colStartPos_x - 1, rowEndPos_y - rowStartPos_y );
      if ( rowRect.intersect( clipRect ) )
      {
         dglSetClipRect( rowRect );
         onRenderCell( Point2I( colStartPos_x, rowStartPos_y ), Point2I( column, row ), ( row == mSelectedRow ), ( row == mMouseOverRow ) );
      }
      rowStartPos_y += mRowHeight;
      rowEndPos_y += mRowHeight;
   }

   // Finally, draw all of the rows in the scroll range:
   if ( hasRowScrollRect )
   {
      RectI scrollRect = rowScrollRect;
      if ( scrollRect.intersect( clipRect ) )
      {
         dglSetClipRect( scrollRect );

         rowStartPos_y = mScrollPanePos.y + rowScrollRect.point.y;
         rowEndPos_y = rowStartPos_y + mRowHeight;
         for ( row = mStartScrollRgn.y; row < mNumRows; row++ )
         {
            RectI cellRect( colStartPos_x, rowStartPos_y, colEndPos_x - colStartPos_x - 1, rowEndPos_y - rowStartPos_y );
            if ( cellRect.intersect( scrollRect ) )
            {
               dglSetClipRect( cellRect );
               onRenderCell( Point2I( colStartPos_x, rowStartPos_y ), Point2I( column, row ), ( row == mSelectedRow ), ( row == mMouseOverRow ) );
            }
            rowStartPos_y += mRowHeight;
            rowEndPos_y += mRowHeight;
         }
      }
   }

   // Draw the column separators:
	if ( mDrawCellSeparators && mSeparatorColor )
	{
	   dglSetClipRect( clipRect );
	   Point2I lineTop( offset.x + mColumnInfoList[column].width - 1, offset.y + mHeaderHeight );
	   Point2I lineBottom( lineTop.x, lineTop.y + mBounds.extent.y );
	   dglDrawLine( lineTop, lineBottom, mSeparatorColor );
	}
}

//------------------------------------------------------------------------------
void ShellFancyArray::determineRepositionColumn( Point2I pt )
{
   RectI columnScrollRect;
   getColumnScrollViewRect( columnScrollRect );

   S32 startPos = columnScrollRect.point.x + mScrollPanePos.x;
   S32 endPos;
   mRepositionColumnTo = mNumColumns;
   for ( S32 column = mStartScrollRgn.x; column < mNumColumns; column++ )
   {
      endPos = startPos + mColumnInfoList[column].width;
      if ( pt.x < endPos )
      {
         mRepositionColumnTo = column;
         if ( pt.x > ( startPos + ( ( endPos - startPos ) / 2 ) ) )
         {
            mRepositionColumnTo++;
            startPos = endPos;
         }
         break;
      }
      startPos = endPos;
   }

   mRepositionCursorPos.set( startPos, mHeaderHeight );
   if ( mRepositionCursorPos.x < columnScrollRect.point.x )
      mRepositionCursorPos.x = columnScrollRect.point.x;
   if ( mRepositionCursorPos.x > ( columnScrollRect.point.x + columnScrollRect.extent.x ) )
      mRepositionCursorPos.x = columnScrollRect.point.x + columnScrollRect.extent.x;
}

//------------------------------------------------------------------------------
bool ShellFancyArray::getColumnScrollViewRect( RectI &rect )
{
   // Get the view rect that the columns scroll 
   // left and right in, in local coordinates.
   rect.point.set( mGlowOffset, 0 );
   rect.extent.y = mBounds.extent.y;

   if ( mStartScrollRgn.x >= mNumColumns )
      return false;

   for ( int i = 0; i < mStartScrollRgn.x; i++ )
      rect.point.x += mColumnInfoList[i].width;

   rect.extent.x = mBounds.extent.x - rect.point.x;

   if ( rect.extent.x <= 0 )
      return false;

   return true;
}

//------------------------------------------------------------------------------
bool ShellFancyArray::getRowScrollViewRect( RectI &rect )
{
   // Get the view rect that the rows scroll 
   // up and down in, in local coordinates.
   if ( mStartScrollRgn.y >= mNumRows )
      return false;

   rect.point.x = 0;
   rect.point.y = mHeaderHeight + mGlowOffset + ( mRowHeight * mStartScrollRgn.y );
   rect.extent.x = mBounds.extent.x;
   rect.extent.y = mBounds.extent.y - rect.point.y;

   if ( rect.extent.y <= 0 )
      return false;

   return true;
}

//------------------------------------------------------------------------------
void ShellFancyArray::setMouseOverRow( S32 row )
{
   if ( mMouseOverRow == row )
      return;

   mMouseOverRow = row;
   setUpdate();
}

//------------------------------------------------------------------------------
void ShellFancyArray::setMouseOverColumn( S32 column, S32 xPos )
{
   if ( mMouseOverColumn == column || mMouseOverRow != -1 )
      return;

   mMouseOverColumn = column;

	// Play a mouse-over noise:
	if ( column != -1 && mProfile->mSoundButtonOver )
	{
      F32 pan = ( F32( xPos ) / F32( Canvas->mBounds.extent.x ) * 2.0f - 1.0f ) * 0.8f;
      AUDIOHANDLE handle = alxCreateSource(mProfile->mSoundButtonOver);
      alxSourcef( handle, AL_PAN, pan );
      alxPlay( handle );
	}

   setUpdateRegion( mBounds.point, Point2I( mBounds.extent.x, mHeaderHeight ) );
}

//------------------------------------------------------------------------------
//
// ShellFancyArrayScrollCtrl functions
//
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ShellFancyArrayScrollCtrl);

//------------------------------------------------------------------------------
ShellFancyArrayScrollCtrl::ShellFancyArrayScrollCtrl() : GuiControl()
{
   mArray = NULL;
   mScrollView = NULL;
   mVirtualContent = NULL;

   mVSpacerBitmap = StringTable->insert( "gui/shll_vertspacer" );
   mHSpacerBitmap = StringTable->insert( "gui/shll_horzspacer" );
   mTexVSpacer = NULL;
   mTexHSpacer = NULL;

   mPrevArrayPos.set( 0, 0 );
   mPrevArrayExtent.set( 0, 0 );
   mPrevContentPos.set( 0, 0 );

	mFixedHorizontal = false;
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::initPersistFields()
{
	Parent::initPersistFields();
	addField( "fixedHorizontal",  TypeBool,   Offset( mFixedHorizontal, ShellFancyArrayScrollCtrl ) );
   addField( "vertSpacerBitmap", TypeString, Offset( mVSpacerBitmap, ShellFancyArrayScrollCtrl ) );
   addField( "horzSpacerBitmap", TypeString, Offset( mHSpacerBitmap, ShellFancyArrayScrollCtrl ) );
}

//------------------------------------------------------------------------------
bool ShellFancyArrayScrollCtrl::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Add the scroll control:
	VirtualScrollCtrl* scrollView = new VirtualScrollCtrl();
   scrollView->mWillFirstRespond = false;
   scrollView->mForceHScrollBar = ( mFixedHorizontal ? GuiScrollCtrl::ScrollBarAlwaysOff : GuiScrollCtrl::ScrollBarAlwaysOn );
   scrollView->mForceVScrollBar = GuiScrollCtrl::ScrollBarAlwaysOn;
   scrollView->mProfile = mProfile; // Share...
   if ( !scrollView->registerObject() )
      Con::errorf( ConsoleLogEntry::General, "Failed to add scroll control to ShellFancyArrayScrollCtrl!" );
   addObject( scrollView );

	mVirtualContent = scrollView->getVirtualContent();

   // Add the server browser:
   ShellFancyArray* textArray = new ShellFancyArray();
   textArray->mProfile = mProfile; // Share...
   if ( !textArray->registerObject() )
      Con::errorf( ConsoleLogEntry::General, "Failed to add browser to ShellFancyArrayScrollCtrl!" );
   addObject( textArray );

   positionChildren();
   return true;
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::addObject( SimObject* obj )
{
	// Only allow one array:
	ShellFancyArray* array = dynamic_cast<ShellFancyArray*>( obj );
	if ( array )
	{
		if ( mArray )
			mArray->deleteObject();
		Parent::addObject( obj );
		mArray = array;
		if ( mFixedHorizontal )
			mArray->setFixedHorizontal();
		positionChildren();
		return;
	}

	// Only allow one scroll control:
	VirtualScrollCtrl* scrollCtrl = dynamic_cast<VirtualScrollCtrl*>( obj );
	if ( scrollCtrl )
	{
		if ( mScrollView )
		{
			mScrollView->deleteObject();
			// Deleting this also deletes the virtual content:
			mVirtualContent = NULL;
		}
		Parent::addObject( obj );
		mScrollView = scrollCtrl;
		mScrollView->mForceHScrollBar = ( mFixedHorizontal ? GuiScrollCtrl::ScrollBarAlwaysOff : GuiScrollCtrl::ScrollBarAlwaysOn );
		positionChildren();
		return;
	}

	Parent::addObject( obj );
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::removeObject( SimObject* obj )
{
	// Keep track of the kids, so we don't crash:
	ShellFancyArray* array = dynamic_cast<ShellFancyArray*>( obj );
	if ( mArray == array )
		mArray = NULL;

	VirtualScrollCtrl* scrollCtrl = dynamic_cast<VirtualScrollCtrl*>( obj );
	if ( mScrollView == scrollCtrl )
		mScrollView = NULL;

	Parent::removeObject( obj );
}

//------------------------------------------------------------------------------
bool ShellFancyArrayScrollCtrl::onWake()
{
   if ( !Parent::onWake() )
      return( false );

   char buf[256];
   if ( mVSpacerBitmap[0] )
   {
      dSprintf( buf, sizeof( buf ), "%s.png", mVSpacerBitmap );
      mTexVSpacer = TextureHandle( buf, BitmapTexture );
   }

   if ( mHSpacerBitmap[0] )
   {
      dSprintf( buf, sizeof( buf ), "%s.png", mHSpacerBitmap );
      mTexHSpacer = TextureHandle( buf, BitmapTexture );
   }

   // Make sure all the kids are sized and aligned correctly:
   positionChildren();

   return( true );
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::onSleep()
{
   Parent::onSleep();
   mTexVSpacer = NULL;
   mTexHSpacer = NULL;
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::positionChildren()
{
	// Don't position the children until all are present:
   if ( !mArray || !mScrollView || !mVirtualContent )
      return;

	S32 glowOffset = mArray->getGlowOffset();
   S32 scrollWidth = mScrollView->scrollBarThickness();
	mArray->mBounds.point.set( 0, 0 );
	mArray->mBounds.extent.set( mBounds.extent.x - scrollWidth - glowOffset, mBounds.extent.y - glowOffset );
	if ( !mFixedHorizontal )
		mArray->mBounds.extent.y -= ( scrollWidth + 1 );
	mArray->resize( mArray->mBounds.point, mArray->mBounds.extent );

   RectI scrollRect;
   mArray->getScrollRect( scrollRect );
   mScrollView->mBounds.point = scrollRect.point - Point2I( glowOffset, glowOffset );
   mScrollView->mBounds.extent.set( mBounds.extent.x - scrollRect.point.x + glowOffset, mBounds.extent.y - scrollRect.point.y + glowOffset );

   mVirtualContent->mBounds.point = mArray->getScrollPos();
   mVirtualContent->mBounds.extent = mArray->getScrollExtent() + Point2I( 2, 2 );
   mScrollView->resize( mScrollView->mBounds.point, mScrollView->mBounds.extent );
   mArray->forceFillScrollRegion();
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::resize( const Point2I &newPos, const Point2I &newExtent )
{
   Parent::resize( newPos, newExtent );

   // Move the kids around:
   positionChildren();
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::onPreRender()
{
   if ( mScrollView && mArray && mVirtualContent )
   {
      // First see if the browser repositioned itself:
      Point2I newPos = mArray->getScrollPos();
      Point2I newExtent = mArray->getScrollExtent();
      if ( mPrevArrayExtent != newExtent )
      {
         mVirtualContent->mBounds.extent = newExtent + Point2I( 2, 2 );
         mPrevArrayExtent = newExtent;
         mScrollView->resize( mScrollView->mBounds.point, mScrollView->mBounds.extent );
      }

      if ( mPrevArrayPos != newPos )
      {
         mVirtualContent->mBounds.point = newPos;
         mPrevArrayPos = newPos;
         mPrevContentPos = newPos;
         mScrollView->resize( mScrollView->mBounds.point, mScrollView->mBounds.extent );
         mArray->setUpdate();
      }
      else
      {
         newPos = mVirtualContent->mBounds.point;
         if ( mPrevContentPos != newPos )
         {
            mArray->setScrollPos( newPos );
            mPrevArrayPos = newPos;
            mPrevContentPos = newPos;
            mArray->setUpdate();
         }
      }
   }
}

//------------------------------------------------------------------------------
void ShellFancyArrayScrollCtrl::onRender( Point2I offset, const RectI &updateRect, GuiControl* firstResponder )
{
   if ( mArray )
   {
      Point2I drawPos;
      RectI clipRect;
      S32 glowOffset = mArray->getGlowOffset();
      dglClearBitmapModulation();

      if ( mTexVSpacer )
      {
         // Draw the upper right notch:
         S32 fillHeight = mArray->getNoScrollHeight() - 1;
         drawPos.x = offset.x + mArray->mBounds.extent.x;
         drawPos.y = offset.y + glowOffset;
         clipRect.set( drawPos, Point2I( mTexVSpacer.getWidth(), fillHeight ) );
         if ( clipRect.intersect( updateRect ) )
         {
            dglSetClipRect( clipRect );
            while ( fillHeight > 0 )
            {
               dglDrawBitmap( mTexVSpacer, drawPos );
               drawPos.y += mTexVSpacer.getHeight();
               fillHeight -= mTexVSpacer.getHeight();
            }

            dglSetClipRect( updateRect );
         }
      }

      if ( !mFixedHorizontal && mTexHSpacer )
      {
         // Draw the lower left notch:
         S32 fillWidth = mArray->getNoScrollWidth() - 1;
         drawPos.x = offset.x + glowOffset;
         drawPos.y = offset.y + mArray->mBounds.extent.y;
         clipRect.set( drawPos, Point2I( fillWidth, mTexHSpacer.getHeight() ) );
         if ( clipRect.intersect( updateRect ) )
         {
            dglSetClipRect( clipRect );
            while ( fillWidth > 0 )
            {
               dglDrawBitmap( mTexHSpacer, drawPos );
               drawPos.x += mTexHSpacer.getWidth();
               fillWidth -= mTexHSpacer.getWidth();
            }

            dglSetClipRect( updateRect );
         }
      }
   }

   Parent::onRender( offset, updateRect, firstResponder );
}

