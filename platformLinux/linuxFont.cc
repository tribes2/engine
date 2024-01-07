//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <limits.h>

#include <freetype/freetype.h>

#include "platformLinux/platformLinux.h"
#include "dgl/gFont.h"
#include "Math/mRect.h"
#include "console/console.h"
#include "Core/fileio.h"

#ifdef DEDICATED

void createFontInit( void )
{
	// empty
}

void createFontShutdown( void )
{
	// emtpy
}

GFont* createFont( const char* name, S32 size )
{
	// empty
	return 0;
}

#else

static FT_Library library;

void createFontInit( void )
{
	FT_Error error = FT_Init_FreeType( &library );

	if( error ) {
		Con::warnf( ConsoleLogEntry::General, "Freetype initialization failed" );
	}

}

void createFontShutdown( void )
{
	FT_Error error = FT_Done_FreeType( library );

	if( error ) {
		Con::warnf( ConsoleLogEntry::General, "Freetype shutdown failed" );
	}

}

GFont* createFont( const char* name, S32 size )
{

	if( !name ) {
		return 0;
	}

#ifdef DEBUG
	Con::printf( "Creating font %s %d", name, size );
#endif

	// open the file and read the contents
	char filename[PATH_MAX];

	dSprintf( filename, PATH_MAX, "base/fonts/%s.ttf", name );

	File file;

	file.open( filename, File::Read );

	if( file.getStatus( ) != File::Ok ) {
		file.close( );
		return 0;
	}

	U32 fileSize = file.getSize( );
	U8* buffer = new U8[fileSize];
	U32 bytesRead = 0;

	if( !buffer ) {
		// Don't bother trying to explain OOM :)
		file.close( );
		return 0;
	}

	file.read( fileSize, (char*) buffer, &bytesRead );

	if( file.getStatus( ) != File::Ok ||
	    bytesRead != fileSize ) {
		Con::warnf( ConsoleLogEntry::General,
			    "Couldn't read all of %s", filename );
		file.close( );
		delete[] buffer;
		return 0;
	}

	file.close( );

	// tweak point size--non-linear!
	F32 points = size;

	if( points < 12.0f ) {
		points *= 0.80f;
	} else {
		points *= 0.77f;
	}

	// to fractional points
	points *= 64.0f;

	// actual ft API usage
	FT_Error error;

	FT_Face face;

	error = FT_New_Memory_Face( library, buffer, fileSize, 0, &face );

	if( error ) {
		Con::warnf( ConsoleLogEntry::General, "could not open font %s", filename );
		delete[] buffer;
		return 0;
	}

	error = FT_Set_Char_Size( face, 0, points, 0, 0 );

	if( error ) {
		Con::warnf( ConsoleLogEntry::General, "couldn't set char size" );
		delete[] buffer;
		return 0;
	}

	F32 units = face->units_per_EM;
	F32 ascent = static_cast<F32>( face->ascender ) / units;
	ascent = ceil( ascent * face->size->metrics.y_ppem );
	// tweak ascent (?)
	ascent *= 1.4f;
	F32 descent = static_cast<F32>( face->descender ) / units;
	descent *= floor( face->size->metrics.y_ppem );
	U32 height = 0;

	if( descent > 0 ) {
		height = ascent + descent;
	} else {
		height = ascent - descent;
	}

	GFont* gfont = new GFont;

	for( S32 i = 32; i < 256; i++ ) {
		int glyphIndex = FT_Get_Char_Index( face, i );

		error = FT_Load_Glyph( face, glyphIndex, FT_LOAD_DEFAULT );

		if( error ) {
			Con::warnf( ConsoleLogEntry::General, "couldn't load glyph %d", i );
			continue;
		}

		FT_GlyphSlot glyph = face->glyph;

		error = FT_Render_Glyph( glyph, ft_render_mode_normal );

		if( error ) {
			Con::warnf( ConsoleLogEntry::General, "couldn't render glyph %d", i );
			continue;
		}

		FT_Bitmap* bitmap = &glyph->bitmap;
		FT_Glyph_Metrics* metrics = &glyph->metrics;

		int glyphWidth = ( ( metrics->width + 63 ) & -64 ) / 64;
		int glyphHeight = ( ( metrics->height + 63 ) & -64 ) / 64;
		int advance = ( ( metrics->horiAdvance + 63 ) & -64 ) / 64;
		int hbx = ( ( metrics->horiBearingX + 63 ) & -64 ) / 64;
		int hby = ( ( metrics->horiBearingY + 63 ) & -64 ) / 64;

		gfont->insertBitmap( i,
				     bitmap->buffer,
				     bitmap->pitch,
				     glyphWidth,
				     glyphHeight,
				     hbx,
				     hby,
				     advance );
	}

	gfont->pack( height, ascent );

	error = FT_Done_Face( face );

	if( error ) {
		Con::warnf( ConsoleLogEntry::General, "could not close face" );
	}

	delete[] buffer;

	return gfont;
}

#endif
