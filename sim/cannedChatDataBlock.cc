//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Sim/cannedChatDataBlock.h"
#include "console/consoleTypes.h"

//--------------------------------------------------------------------------
static const char* getDataTypeCannedChatItemPtr( void* dptr, EnumTable*, BitSet32 )
{
   CannedChatItem** obj = reinterpret_cast<CannedChatItem**>( dptr );
   return ( *obj ? (*obj)->getName() : "" );
}


static void setDataTypeCannedChatItemPtr( void* dptr, S32 argc, const char** argv, EnumTable*, BitSet32 )
{
   if ( argc == 1 )
   {
      if ( !Sim::findObject( argv[0], *reinterpret_cast<CannedChatItem**>( dptr ) ) )
         Con::printf( "Object \"%s\" is not a CannedChatItem data block", argv[0] );
   }
   else
      Con::printf( "(TypeCannedChatItemPtr) Can't set multiple args to a single pointer." );
}


//--------------------------------------------------------------------------
IMPLEMENT_CO_DATABLOCK_V1( CannedChatItem );
CannedChatItem::CannedChatItem()
{
   mName       = NULL;
   mText       = NULL;
   mAudioFile  = NULL;
   mAnimation  = NULL;
   mTeamOnly   = false;
	mPlay3D		= false;
}


//--------------------------------------------------------------------------
void CannedChatItem::consoleInit()
{
   addField( "name",       TypeString, Offset( mName, CannedChatItem ) );
   addField( "text",       TypeString, Offset( mText, CannedChatItem ) );
   addField( "audioFile",  TypeString, Offset( mAudioFile, CannedChatItem ) );
   addField( "animation",  TypeString, Offset( mAnimation, CannedChatItem ) );
   // more?
   addField( "teamOnly",   TypeBool,   Offset( mTeamOnly, CannedChatItem ) );
   addField( "play3D",   	TypeBool,   Offset( mPlay3D, CannedChatItem ) );
   Con::registerType( TypeCannedChatItemPtr, sizeof( CannedChatItem* ), getDataTypeCannedChatItemPtr, setDataTypeCannedChatItemPtr );
}


//--------------------------------------------------------------------------
void CannedChatItem::initPersistFields()
{
   Parent::initPersistFields();
}


//--------------------------------------------------------------------------
bool CannedChatItem::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   return true;
}
