//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "game/guiServerBrowser.h"
#include "gui/guiCanvas.h"
#include "console/consoleTypes.h"
#include "game/serverQuery.h"
#include "game/version.h"

//------------------------------------------------------------------------------
// Static stuff for sorting:
//------------------------------------------------------------------------------
static S32  sSortColumnKey;
static bool sSortInc;
static S32  sSecondarySortColumnKey;
static bool sSecondarySortInc;

//------------------------------------------------------------------------------
static int FN_CDECL serverListRowCompare( const void* a, const void* b )
{
   ServerInfo* siA = (ServerInfo*) a;
   ServerInfo* siB = (ServerInfo*) b;

   enum SortStage
   {
      FirstSort,
      SecondSort,
      Done,
   };
   
   SortStage stage = FirstSort;
   //bool done = false;
   bool AEqualB = false;
   bool ALessB = true;
   bool sortInc = true;
   S32 cmpResult;

   // First push timed-out servers to the end:
   if ( siA->isTimedOut() )
   {
      if ( !siB->isTimedOut() )
      {
         ALessB = false;
         stage = Done;
      }
   }
   else if ( siB->isTimedOut() )
      stage = Done;

   // Now bring servers that have already responded to the top:
   if ( stage != Done )
   {
      if ( !siA->hasResponded() )
      {
         if ( siB->hasResponded() )
         {
            ALessB = false;
            stage = Done;
         }
      }
      else if ( !siB->hasResponded() )
         stage = Done;
   }

   while ( stage != Done )
   {
      sortInc = ( stage == FirstSort ) ? sSortInc : sSecondarySortInc;
      switch ( ( stage == FirstSort ) ? sSortColumnKey : sSecondarySortColumnKey )
      {
         case GuiServerBrowser::Name_Column:
				if ( siA->name && siB->name )
				{
	            cmpResult = dStricmp( siA->name, siB->name );
	            if ( cmpResult == 0 )
	               AEqualB = true;
	            else
	               ALessB = ( cmpResult < 0 );
				}
            break;

         case GuiServerBrowser::Status_Column:
         {
            U32 numIconsA = 0, numIconsB = 0;
            U32 iconMaskA = 0, iconMaskB = 0;
            if ( siA->isPassworded() )
            {
               numIconsA++;
               iconMaskA += 4;
            }
            if ( siA->isDedicated() )
            {
               numIconsA++;
               iconMaskA += 2;
            }
            if ( siA->isTournament() )
            {
               numIconsA++;
               iconMaskA++;
            }
            if ( siA->isLinux() )
               numIconsA++;

            if ( siB->isPassworded() )
            {
               numIconsB++;
               iconMaskB += 4;
            }
            if ( siB->isDedicated() )
            {
               numIconsB++;
               iconMaskB += 2;
            }
            if ( siB->isTournament() )
            {
               numIconsB++;
               iconMaskB++;
            }
            if ( siB->isLinux() )
               numIconsB++;

            if ( numIconsA == numIconsB )
            {
               if ( iconMaskA == iconMaskB )
                  AEqualB = true;
               else
                  ALessB = iconMaskA < iconMaskB;
            }
            else
               ALessB = numIconsA < numIconsB;

            break;
         }

         case GuiServerBrowser::Favorite_Column:
            if ( siA->isFavorite == siB->isFavorite )
               AEqualB = true;
            else
               ALessB = !siB->isFavorite;
            break;

         case GuiServerBrowser::Ping_Column:
            if ( siA->ping == siB->ping )
               AEqualB = true;
            else
               ALessB = ( siA->ping < siB->ping );
            break;

         case GuiServerBrowser::MissionType_Column:
				if ( siA->missionType && siB->missionType )
				{
	            cmpResult = dStricmp( siA->missionType, siB->missionType );
	            if ( cmpResult == 0 )
	               AEqualB = true;
	            else
	               ALessB = ( cmpResult < 0 );
				}
            break;

         case GuiServerBrowser::Map_Column:
				if ( siA->missionName && siB->missionName )
				{
	            cmpResult = dStricmp( siA->missionName, siB->missionName );
	            if ( cmpResult == 0 )
	               AEqualB = true;
	            else
	               ALessB = ( cmpResult < 0 );
				}
            break;

         case GuiServerBrowser::GameType_Column:
				if ( siA->gameType && siB->gameType )
				{
	            cmpResult = dStricmp( siA->gameType, siB->gameType );
	            if ( cmpResult == 0 )
	               AEqualB = true;
	            else
	               ALessB = ( cmpResult < 0 );
				}
            break;

         case GuiServerBrowser::Players_Column:
            if ( siA->numPlayers == siB->numPlayers )
               AEqualB = true;
            else
               ALessB = ( siA->numPlayers < siB->numPlayers );
            break;

         case GuiServerBrowser::CPU_Column:
            if ( siA->cpuSpeed == siB->cpuSpeed )
               AEqualB = true;
            else
               ALessB = ( siA->cpuSpeed < siB->cpuSpeed );
            break;

         case GuiServerBrowser::IP_Column:
			{
				char addrA[256], addrB[256];
				Net::addressToString( &siA->address, addrA );
				Net::addressToString( &siB->address, addrB );
				cmpResult = dStricmp( addrA, addrB );
				if ( cmpResult == 0 )
					AEqualB = true;
				else
					ALessB = ( cmpResult < 0 );
            break;
			}

         case GuiServerBrowser::Version_Column:
            if ( siA->version == siB->version )
               AEqualB = true;
            else
               ALessB = ( siA->version < siB->version );
            break;
      }

      if ( stage == FirstSort && AEqualB )
         stage = SecondSort;
      else
         stage = Done;
   }

   if ( AEqualB )
   {
      cmpResult = dStricmp( siA->name, siB->name );
      if ( cmpResult == 0 )
         return 0;
      else
         ALessB = ( cmpResult < 0 );
   }

   if ( ALessB )
      return ( sortInc ? -1 : 1 );
   else
      return ( sortInc ? 1 : -1 );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(GuiServerBrowser);

//------------------------------------------------------------------------------
GuiServerBrowser::GuiServerBrowser() : ShellFancyArray()
{
	mIconBase = StringTable->insert( "gui/shll_icon" );
   mTexFavorite      = NULL;
   mTexFavoriteHI    = NULL;
   mTexNotQueried    = NULL;
   mTexNotQueriedHI  = NULL;
   mTexQuerying      = NULL;
   mTexQueryingHI    = NULL;
   mTexTimedOut      = NULL;
   mTexDedicated     = NULL;
   mTexDedicatedHI   = NULL;
   mTexPassworded    = NULL;
   mTexPasswordedHI  = NULL;
   mTexTournament    = NULL;
   mTexTournamentHI  = NULL;
}

//------------------------------------------------------------------------------
ConsoleFunction( queryLanServers, void, 2, 3, "queryLanServers( port{, flags} )" )
{
   queryLanServers( dAtoi( argv[1] ), ( argc == 2 ? U8( dAtoi( argv[2] ) ) : 0 ) );
}

//------------------------------------------------------------------------------
ConsoleFunction( queryMasterGameTypes, void, 1, 1, "queryMasterGameTypes()" )
{
   argc;
   argv;
   queryMasterGameTypes();
}

//------------------------------------------------------------------------------
ConsoleFunction( queryMasterServer, void, 2, 13, "queryMasterServer( port{, flags{, rulesSet{, missionType{, minPlayers{, maxPlayers{, maxBots{, regionMask{, maxPing{, minCPUSpeed{, filterFlags{, buddyList }}}}}}}}}}} )" )
{
   char rulesSet[64];
   char missionType[64];
   dStrncpy( rulesSet, ( argc > 3 ? argv[3] : "any" ), sizeof( rulesSet ) );
   dStrncpy( missionType, ( argc > 4 ? argv[4] : "any" ), sizeof( missionType ) );
   U8 buddyCount = 0;
   U32* guidArray = NULL;
   if ( argc == 13 )
   {
      // fill the guid array:
      char* buf = new char[dStrlen( argv[11] ) + 1];
      dStrcpy( buf, argv[11] );

      char* slave = buf;
      U8 last = 0;
      while ( *slave )
      {
         last = *slave++;
         if ( last == '\t' )
         {
            buddyCount++;
            last = 0;
         }
      }
      if ( last )
         buddyCount++;

      if ( buddyCount )
      {
         guidArray = new U32[buddyCount];

         slave = dStrtok( buf, "\t" );
         for ( U32 i = 0; i < buddyCount && slave; i++ )
         {
            guidArray[i] = U32( dAtoi( slave ) );
            slave = dStrtok( NULL, "\t" );
         }
      }

      delete [] buf;
   }

   queryMasterServer( dAtoi(argv[1]), 
         ( argc > 2 ? U8( dAtoi( argv[2] ) ) : 0 ), 
         rulesSet, 
         missionType, 
         ( argc > 5 ? U8( dAtoi( argv[5] ) ) : 0 ), 
         ( argc > 6 ? U8( dAtoi( argv[6] ) ) : 255 ),
         ( argc > 7 ? U8( dAtoi( argv[7] ) ) : 16 ), 
         ( argc > 8 ? U32( dAtoi( argv[8] ) ) : 0xFFFFFFFF ), 
         ( argc > 9 ? U32( dAtoi( argv[9] ) ) : 0 ),
         ( argc > 10 ? U32( dAtoi( argv[10] ) ) : 0 ),
         ( argc > 11 ? U32( dAtoi( argv[11] ) ) : 0 ), 
         buddyCount, 
         guidArray );

   if ( guidArray )
      delete [] guidArray;
}

//------------------------------------------------------------------------------
ConsoleFunction( queryFavoriteServers, void, 1, 2, "queryFavoriteServers( {, flags} )" )
{
   queryFavoriteServers( argc == 2 ? U8( dAtoi( argv[1] ) ) : 0 );
}

//------------------------------------------------------------------------------
ConsoleFunction( querySingleServer, void, 2, 3, "querySingleServer( address{, flags} )" )
{
   NetAddress addr;
   Net::stringToAddress( argv[1], &addr );
   querySingleServer( &addr, ( argc == 3 ? U8( dAtoi( argv[2] ) ) : 0 ) );
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiServerBrowser, sort, void, 2, 2, "browser.sort()" )
{
   argc; argv;
   static_cast<GuiServerBrowser*>( object )->sort();
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiServerBrowser, getServerStatus, const char*, 2, 2, "browser.getServerStatus()" )
{
   argc; argv;
   return( static_cast<GuiServerBrowser*>( object )->getSelectedServerStatus() );
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiServerBrowser, getServerInfoString, const char*, 2, 2, "browser.getServerInfoString()" )
{
   argc; argv;
   return( static_cast<GuiServerBrowser*>( object )->getSelectedServerInfoString() );
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiServerBrowser, getServerContentString, const char*, 2, 2, "browser.getServerContentString()" )
{
   argc; argv;
   return( static_cast<GuiServerBrowser*>( object )->getSelectedServerContentString() );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef DEBUG
ConsoleFunction( addFakeServers, void, 2, 2, "addFakeServers( howMany )" )
{
   argc;
   addFakeServers( dAtoi( argv[1] ) );
}
#endif // DEBUG

//------------------------------------------------------------------------------
void GuiServerBrowser::initPersistFields()
{
	Parent::initPersistFields();
	addField( "iconBase",   TypeString, Offset( mIconBase, GuiServerBrowser ) );
}

//------------------------------------------------------------------------------
const char* GuiServerBrowser::getScriptValue()
{
   if ( mSelectedRow < 0 || mSelectedRow >= gServerList.size() )
      return "";

   static char buf[256];
   char address[256];

   ServerInfo &si = gServerList[mSelectedRow];
   Net::addressToString( &si.address, address );
   dSprintf( buf, sizeof( buf ), "%s\t%s\t%d\t%s\t%s", si.name ? si.name : "?", address, si.ping, si.missionName ? si.missionName : "?", si.missionType ? si.missionType : "?" );
   return buf;
}

//------------------------------------------------------------------------------
bool GuiServerBrowser::onWake()
{
   if ( !Parent::onWake() )
      return false;

	// Set the row height based on the font:
	if ( mFont )
		mRowHeight = mFont->getHeight() + 3;

   // Get the browser icons:
	if ( mIconBase[0] )
	{
		char buf[256];
		dSprintf( buf, sizeof( buf ), "%s_favorite.png", mIconBase );
	   mTexFavorite = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_favorite_hi.png", mIconBase );
	   mTexFavoriteHI = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_notqueried.png", mIconBase );
	   mTexNotQueried = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_notqueried_hi.png", mIconBase );
	   mTexNotQueriedHI = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_querying.png", mIconBase );
	   mTexQuerying = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_querying_hi.png", mIconBase );
	   mTexQueryingHI = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_timedout.png", mIconBase );
	   mTexTimedOut = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_dedicated.png", mIconBase );
	   mTexDedicated = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_dedicated_hi.png", mIconBase );
	   mTexDedicatedHI = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_passworded.png", mIconBase );
	   mTexPassworded = TextureHandle( buf, BitmapTexture );
		dSprintf( buf, sizeof( buf ), "%s_passworded_hi.png", mIconBase );
	   mTexPasswordedHI = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_tourney.png", mIconBase );
      mTexTournament = TextureHandle( buf, BitmapTexture );
      dSprintf( buf, sizeof( buf ), "%s_tourney_hi.png", mIconBase );
      mTexTournamentHI = TextureHandle( buf, BitmapTexture );
	}

   return true;
}

//------------------------------------------------------------------------------
void GuiServerBrowser::onSleep()
{
   Parent::onSleep();

   mTexFavorite      = NULL;
   mTexFavoriteHI    = NULL;
   mTexNotQueried    = NULL;
   mTexNotQueriedHI  = NULL;
   mTexQuerying      = NULL;
   mTexQueryingHI    = NULL;
   mTexTimedOut      = NULL;
   mTexDedicated     = NULL;
   mTexDedicatedHI   = NULL;
   mTexPassworded    = NULL;
   mTexPasswordedHI  = NULL;
   mTexTournament    = NULL;
   mTexTournamentHI  = NULL;
}

//------------------------------------------------------------------------------
void GuiServerBrowser::onPreRender()
{
   if ( gServerBrowserDirty )
   {
      sort();
      gServerBrowserDirty = false;
   }
}

//------------------------------------------------------------------------------
void GuiServerBrowser::updateList()
{
	if ( mNumRows != gServerList.size() )
		setNumRows( gServerList.size() );

	setUpdate();
}

//------------------------------------------------------------------------------
void GuiServerBrowser::sort()
{
   mNumRows = gServerList.size();
   sSortColumnKey = mSortColumnKey;
   sSortInc = mSortInc;
   sSecondarySortColumnKey = mSecondarySortColumnKey;
   sSecondarySortInc = mSecondarySortInc;

   if ( gServerList.size() > 1 )
      dQsort( (void*) &(gServerList[0]), gServerList.size(), sizeof(ServerInfo), serverListRowCompare );

   selectRowByAddress();
   setUpdate();
}

//------------------------------------------------------------------------------
void GuiServerBrowser::onCellSelected( S32 row, S32 column )
{
   AssertFatal( ( column >= 0 && column < mColumnInfoList.size() ), "Invalid column selected!" );
   AssertFatal( ( row >= 0 && row < mNumRows ), "Invalid row selected!" );

   char addrString[256];
   Net::addressToString( &gServerList[row].address, addrString );
   if ( mColumnInfoList[column].key == Favorite_Column )
   {
      gServerList[row].isFavorite = !gServerList[row].isFavorite;
      setUpdate();

      // Execute the script function:
      if ( gServerList[row].isFavorite )
         Con::executef( this, 3, "addFavorite", gServerList[row].name, (const char*) addrString );
      else
         Con::executef( this, 2, "removeFavorite", (const char*) addrString );
   }

   mSelectedAddress = gServerList[row].address;
	Con::executef( this, 2, "onSelect", (const char*) addrString );

	// Call the console function:
	if ( mConsoleCommand[0] )
		Con::evaluate( mConsoleCommand, false );
}

//------------------------------------------------------------------------------
void GuiServerBrowser::onRenderCell( Point2I offset, Point2I cell, bool selected, bool mouseOver )
{
   ServerInfo &si = gServerList[cell.y];
   ColumnInfo* ci = &mColumnInfoList[cell.x];

	// Let the parent take care of the basics:
	Parent::onRenderCell( offset, cell, selected, mouseOver );

   bool drawText = true;
   const char* text = NULL;
   char buffer[256];
   S32 bmpOffsetX = 0;
   Point2I drawPos;
   TextureHandle hTex = NULL;
   dglClearBitmapModulation();

   switch( ci->key )
   {
      case Name_Column:
         text = si.name ? si.name : "--";
         break;

      case Status_Column:
         // Draw bitmap(s):
			if ( si.isNew() )
         {
            hTex = selected ? mTexNotQueriedHI : mTexNotQueried;
            if ( hTex )
            {
               drawPos.x = offset.x + ( ( ci->width - hTex.getWidth() ) / 2 );
               drawPos.y = offset.y;
               if ( mRowHeight > hTex.getHeight() )
                  drawPos.y += ( ( mRowHeight - hTex.getHeight() ) / 2 );
               dglDrawBitmap( hTex, drawPos );
            }
            else
				   text = "?";
         }
			else if ( si.isQuerying() )
         {
            hTex = selected ? mTexQueryingHI : mTexQuerying;
            if ( hTex )
            {
               drawPos.x = offset.x + ( ( ci->width - hTex.getWidth() ) / 2 );
               drawPos.y = offset.y;
               if ( mRowHeight > hTex.getHeight() )
                  drawPos.y += ( ( mRowHeight - hTex.getHeight() ) / 2 );
               dglDrawBitmap( hTex, drawPos );
            }
            else
				   text = "Q";
         }
			else if ( si.hasResponded() )
			{
            TextureHandle hTexDedicated = NULL, hTexTournament = NULL;
            U32 numIcons = 0;
            U32 width = 0, height = 0;
            if ( si.isPassworded() )
            {
               hTex = selected ? mTexPasswordedHI : mTexPassworded;
               if ( hTex )
               {
                  width += hTex.getWidth();
                  height = hTex.getHeight();
                  numIcons++;
               }
            }

            if ( si.isDedicated() )
            {
               hTexDedicated = selected ? mTexDedicatedHI : mTexDedicated;
               if ( hTexDedicated )
               {
                  width += hTexDedicated.getWidth();
                  if ( hTexDedicated.getHeight() > height )
                     height = hTexDedicated.getHeight();
                  numIcons++;
               }
            }

            if ( si.isTournament() )
            {
               hTexTournament = selected ? mTexTournamentHI : mTexTournament;
               if ( hTexTournament )
               {
                  width += hTexTournament.getWidth();
                  if ( hTexTournament.getHeight() > height )
                     height = hTexTournament.getHeight();
                  numIcons++;
               }
            }

            if ( si.isLinux() )
            {
               width += mFont->getStrWidth( "L" );
               numIcons++;
            }

            // Draw icons if there are any:
            if ( numIcons > 0 )
            {
               // Try to center the icons:
               drawPos = offset;
               if ( ci->width > width )
                  drawPos.x += ( ( ci->width - width ) / 2 );

               if ( mRowHeight > height )
                  drawPos.y += ( ( mRowHeight - height ) / 2 );

               if ( hTex )
               {
                  dglDrawBitmap( hTex, drawPos );
                  drawPos.x += hTex.getWidth();
               }

               if ( hTexDedicated )
               {
                  dglDrawBitmap( hTexDedicated, drawPos );
                  drawPos.x += hTexDedicated.getWidth();
               }

               if ( hTexTournament )
               {
                  dglDrawBitmap( hTexTournament, drawPos );
                  drawPos.x += hTexTournament.getWidth();
               }

               if ( si.isLinux() )
               {
                  dglSetBitmapModulation( selected ? ColorI( 4, 40, 5 ) : ColorI( 0, 255, 0 ) );
                  drawPos.y = offset.y + ( ( mRowHeight - mFont->getHeight() ) / 2 );
                  dglDrawText( mFont, drawPos, "L" );
               }
            }
			}
			else if ( mTexTimedOut )
			{
            drawPos.x = offset.x + ( ( ci->width - mTexTimedOut.getWidth() ) / 2 );
            drawPos.y = offset.y;
            if ( mRowHeight > mTexTimedOut.getHeight() )
               drawPos.y += ( ( mRowHeight - mTexTimedOut.getHeight() ) / 2 );
            dglDrawBitmap( mTexTimedOut, drawPos );
			}
         else
				text = "X";
         break;

      case Favorite_Column:
         if ( si.isFavorite )
         {
            hTex = selected ? mTexFavoriteHI : mTexFavorite;
            if ( hTex )
            {
               drawPos.x = offset.x + ( ( ci->width - hTex.getWidth() ) / 2 );
               drawPos.y = offset.y;
               if ( mRowHeight > hTex.getHeight() )
                  drawPos.y += ( ( mRowHeight - hTex.getHeight() ) / 2 );
               dglDrawBitmap( hTex, drawPos );   
            }
            else
               text = "X";
         }
         break;

      case Ping_Column:
         dSprintf( buffer, 255, "%d", si.ping );
         text = buffer;
         break;

      case IP_Column:
         Net::addressToString( &si.address, buffer );
         dStrcpy( buffer, buffer + 3 );
         text = buffer;
         break;

      case Players_Column:
         dSprintf( buffer, 255, "%d/%d (%d)", si.numPlayers, si.maxPlayers, si.numBots );
         text = buffer;
         break;

      case GameType_Column:
         text = si.gameType ? si.gameType : "--";
         break;

      case MissionType_Column:
         text = si.missionType ? si.missionType : "--";
         break;

      case Map_Column:
         if ( si.missionName )
         {
            dStrcpy( buffer, si.missionName );
            // Clip off the file extension:
            char* temp = dStrstr( buffer, const_cast<char*>( ".mis" ) );
            if ( temp )
               *temp = 0;
            text = buffer;
         }
         else
            text = "--";
         break;

      case CPU_Column:
         if ( si.cpuSpeed )
         {
            dSprintf( buffer, 255, "%d MHz", si.cpuSpeed );
            text = buffer;
         }
         else
            text = "--";
         break;
      
      case Version_Column:
         if ( si.version )
         {
            dSprintf( buffer, 255, "%d", si.version );
            text = buffer;
         }
         else
            text = "--";
         break;

      default:
         Con::errorf( ConsoleLogEntry::General, "Invalid column key encountered!" );
         return;
   }

   // Finally, draw the text ( if there is any ):
   if ( drawText && text && *text )
   {
      // Determine the font color:
      ColorI color;
      if ( si.gameType && dStricmp( si.gameType, "base" ) != 0 ) // Mod colors
         color = selected ? mProfile->mFontColors[6] : ( mouseOver ? mProfile->mFontColors[5] : mProfile->mFontColors[4] );
      else if ( si.version != getVersionNumber() ) // Disparate build version colors
         color = selected ? mProfile->mFontColors[9] : ( mouseOver ? mProfile->mFontColors[8] : mProfile->mFontColors[7] );
      else // Base colors
         color = selected ? mProfile->mFontColorSEL : ( mouseOver ? mProfile->mFontColorHL : mProfile->mFontColor );

      S32 textWidth = mFont->getStrWidth( text );
      Point2I textStart;
      textStart.x = offset.x + getMax( 4, ( ci->width - textWidth ) / 2 );
      textStart.y = offset.y + ( ( mRowHeight - ( mFont->getHeight() - 2 ) ) / 2 );
      dglSetBitmapModulation( color );
      dglDrawText( mFont, textStart, text );
   }
}  

//------------------------------------------------------------------------------
void GuiServerBrowser::selectRowByAddress()
{
   if ( mSelectedRow >= 0 || mSelectedRow < gServerList.size() )
   {
      bool foundIt = false;
      for ( S32 i = 0; i < gServerList.size(); i++ )
      {
         if ( Net::compareAddresses( &mSelectedAddress, &gServerList[i].address ) )
         {
            selectCell( i, 0 );
            foundIt = true;
            break;
         }
      }

      if ( !foundIt )
         mSelectedRow = -1;
   }
}

//------------------------------------------------------------------------------
const char* GuiServerBrowser::getSelectedServerStatus()
{
   if ( mSelectedRow < 0 || mSelectedRow >= gServerList.size() )
      return( "invalid" );

   ServerInfo* si = &gServerList[mSelectedRow];
   if ( si->isNew() )
      return( "new" );
   if ( si->isQuerying() )
      return( "querying" );
   if ( si->hasResponded() )   
      return( "responded" );
   if ( si->isUpdating() )
      return( "updating" );
      
   return( "timedOut" );
}

//------------------------------------------------------------------------------
const char* GuiServerBrowser::getSelectedServerInfoString()
{
   if ( mSelectedRow < 0 || mSelectedRow >= gServerList.size() )
      return( "" );

   ServerInfo* si = &gServerList[mSelectedRow];
   U32 bufSize = 0;
   if ( si->name )
      bufSize += ( dStrlen( si->name ) + 1 );
   char addrString[256];
   Net::addressToString( &si->address, addrString );
   bufSize += dStrlen( addrString ) - 2;
   if ( si->gameType )
      bufSize += ( dStrlen( si->gameType ) + 1 );

   // Build the flag string:
   char flagString[256];
   flagString[0] = 0;
   bool firstFlag = true;
   if ( si->isDedicated() )
   {
      dStrcpy( flagString, "Dedicated" );
      firstFlag = false;
   }
   if ( si->isPassworded() )
   {
      dStrcat( flagString, firstFlag ? "Passworded" : "; Passworded" );
      firstFlag = false;
   }
   if ( si->isTournament() )
   {
      dStrcat( flagString, firstFlag ? "Tournament Mode" : "; Tournament Mode" );
      firstFlag = false;
   }
   if ( si->isLinux() )
   {
      dStrcat( flagString, firstFlag ? "Linux" : "; Linux" );
      firstFlag = false;
   }
   if ( !si->areSmurfsAllowed() )
      dStrcat( flagString, firstFlag ? "No Aliases" : "; No Aliases" );
   bufSize += ( dStrlen( flagString ) + 1 );

   if ( si->missionType )
      bufSize += ( dStrlen( si->missionType ) + 1 );
   if ( si->missionName )
      bufSize += ( dStrlen( si->missionName ) + 1 );
   if ( si->infoString )
      bufSize += ( dStrlen( si->infoString ) + 1 );

   char* returnString = Con::getReturnBuffer( bufSize );
   dSprintf( returnString, bufSize, "%s\n%s\n%s\n%s\n%s\n%s\n%s",
         ( si->name ? si->name : "" ),
         &addrString[3],
         ( si->gameType ? si->gameType : "" ),
         flagString,
         ( si->missionType ? si->missionType : "" ),
         ( si->missionName ? si->missionName : "" ),
         ( si->infoString ? si->infoString : "" ) );

   return( returnString );
}

//------------------------------------------------------------------------------
const char* GuiServerBrowser::getSelectedServerContentString()
{
   if ( mSelectedRow < 0 || mSelectedRow >= gServerList.size() )
      return( "" );

   return( gServerList[mSelectedRow].contentString ? gServerList[mSelectedRow].contentString : "" );
}
