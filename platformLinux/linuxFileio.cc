//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "loki_utils.h"

#include "engine/platformLinux/platformLinux.h"
#include "engine/core/fileio.h"
#include "engine/core/tVector.h"
#include "engine/core/stringTable.h"
#include "engine/console/console.h"

static void build_canonical_name( const char* path, char* buffer, int size )
{
	struct stat sb;

	/* default value is identity */
	strncpy( buffer, path, size );
	buffer[size-1] = '\0';

	/* Shortcut, most of the time the file name is fine */
	if ( stat(buffer, &sb) != 0 ) {
		char* r;
		char *pathname;
		char *filename;
		DIR* dir;
		struct dirent* de;

		/* find the last slash */
		pathname = buffer;
		r = strrchr( pathname, '/' );
		if( r ) {
			*r = '\0';
			filename = r+1;
		}

		/*
		 * Look for an existing name.
		 */
		dir = opendir( pathname );
		if( dir ) {
			while( ( de = readdir( dir ) ) != NULL ) {

				if( strcasecmp( de->d_name, filename ) == 0 ) {
					strncpy(filename, de->d_name, size-(strlen(pathname)+1));
					break;
				}

			}
			closedir( dir );
		}
		if ( r ) {
			*r = '/';
		}
	}
}

int iopen( const char* path, int flags )
{
	char buffer[PATH_MAX];
	int fd = -1;

	build_canonical_name( path, buffer, PATH_MAX );
	fd = open( buffer, flags );
	return fd;
}

FILE* ifopen( const char* path, const char* mode )
{
	char buffer[PATH_MAX];
	FILE* f = NULL;

	build_canonical_name( path, buffer, PATH_MAX );
	f = fopen( buffer, mode );
	return f;
}

static void forwardSlash( char* s )
{
	while( *s ) {
		if( *s == '\\' ) {
			*s = '/';
		}
		s++;
	}
}

static bool copyFile(const char *src, const char *dst)
{
	int status;
	FILE *i, *o;
	int len;
	char data[BUFSIZ];

	i = fopen(src, "rb");
	o = fopen(dst, "wb");
	if ( !i || !o ) {
		if ( i ) {
			fclose(i);
		}
		if ( o ) {
			fclose(i);
		}
		return(0);
	}
	status = 0;
	while ( (len = fread(data, 1, sizeof(data), i)) > 0 ) {
		if ( fwrite(data, 1, len, o) != len ) {
			status = -1;
			break;
		}
	}
	fclose(i);
	if ( fclose(o) != 0 ) {
		status = -1;
	}
	if ( status < 0 ) {
		unlink(dst);
	}
	return(status == 0);
}

//-----------------------------------------------------------------------------
bool dFileDelete(const char * name)
{
	bool status;

	status = false;
	if (name && !dStrstr(name, "../")) {
		char canonical[PATH_MAX];
		char path[PATH_MAX];

		// We can only write and delete files in the prefs directory
		dSprintf(canonical, sizeof(canonical), "%s/%s", loki_getprefpath(), name);
		build_canonical_name( canonical, path, sizeof(path) );
		if ( unlink(path) == 0 ) {
			status = true;
		}
	}
	return status;
}

// change the modified time to the current time
bool dFileTouch(const char * name)
{
	bool status;

	status = false;
	if (name && !dStrstr(name, "../")) {
		char canonical[PATH_MAX];
		char path[PATH_MAX];

		// We can only modify files in the prefs directory
		dSprintf(canonical, sizeof(canonical), "%s/%s", loki_getprefpath(), name);
		build_canonical_name( canonical, path, sizeof(path) );
		if ( utime(path, NULL) == 0 ) {
			status = true;
		}
	}
	return status;
}

File::File( void ) : currentStatus( Closed ), capability( 0 )
{
	handle = NULL;
}

File::~File( void )
{
	close( );
	handle = NULL;
}

File::Status File::open( const char* filename, const AccessMode mode )
{
	char modded[PATH_MAX];
	char canonical[PATH_MAX];

	assert( filename );

	dStrncpy( modded, filename, PATH_MAX );
	forwardSlash( modded );
	filename = modded;

	// if it's not absolute already, put it in the user dir.
	if( filename[0] != '/' ) {
		dSprintf( canonical, PATH_MAX, "%s/%s", loki_getprefpath( ), filename );
	} else {
		dStrncpy( canonical, filename, PATH_MAX );
	}

	if( currentStatus != Closed ) {
		close( );
	}

	switch( mode ) {
	case Read:
#if 1
#warning FIXME for performance increase!
		// FIXME:
		// Cache the contents of the preferences directory
		// so we don't have to keep searching for each file
		// Can we use the output of dumpPath() in the game?
#endif
		handle = ifopen( canonical, "r" );

		if( !handle ) {
			// if read-only we can try to open in the game dir itself
			// because of the "if absolute" check above, it's possible
			// that canonical == filename.
			handle = ifopen( filename, "r" );
		}
		break;
	case Write:
		handle = ifopen( canonical, "w" );
		break;
	case ReadWrite:
		handle = ifopen( canonical, "a+" );

		// mimic win32 create on missing by seeking to begin
		if( handle ) {
			fseek( (FILE*) handle, 0, SEEK_SET );
		}

		break;
	case WriteAppend:
		handle = ifopen( canonical, "r" );
		if( handle ) {
			fclose(handle);
		} else {
			// We have to copy the original file to append
			if ( ! copyFile(filename, canonical) ) {
				return setStatus( );
			}
		}
		handle = ifopen( canonical, "a" );
		break;
	}

	if( !handle ) {
#ifdef DEBUG
		fprintf(stderr, "Unable to open %s\n", filename);
#endif
		return setStatus( );
	}

	switch( mode ) {
	case Read:
		capability = static_cast<U32>( FileRead );
		break;
	case Write:
	case WriteAppend:
		capability = static_cast<U32>( FileWrite );
		break;
	case ReadWrite:
		capability = static_cast<U32>( FileRead ) |
			     static_cast<U32>( FileWrite );
		break;
	}

	return ( currentStatus = Ok );
}

U32 File::getPosition( void ) const
{
	return ftell( (FILE*) handle );
}

File::Status File::setPosition( S32 position, bool absolute )
{
	FILE* file = (FILE*) handle;

	if( currentStatus != Ok && currentStatus != EOS ) {
		return currentStatus;
	}

	if( absolute ) {
		fseek( file, position, SEEK_SET );
	} else {
		fseek( file, position, SEEK_CUR );
	}

	long current = ftell( file );

	if( current == -1 ) {
		return setStatus( );
	} else if( current >= getSize( ) ) {
		return ( currentStatus = EOS );
	} else {
		return ( currentStatus = Ok );
	}

}

U32 File::getSize( void ) const
{

	if( currentStatus == Ok || currentStatus == EOS ) {
		struct stat buf;

		int fd = fileno( (FILE*) handle );

		if( fstat( fd, &buf ) ) {
			return 0;
		}

		return buf.st_size;
	}

	return 0;
}

File::Status File::flush( void )
{

	if( fflush( (FILE*) handle ) ) {
		return setStatus( );
	}

	return ( currentStatus = Ok );
}

File::Status File::close( void )
{

	if( currentStatus == Closed ) {
		return currentStatus;
	}

	if( handle ) {

		if( fclose( (FILE*) handle ) ) {
			return setStatus( );
		}

	}

	handle = NULL;
	return ( currentStatus = Closed );
}

File::Status File::getStatus( void ) const
{
	return currentStatus;
}

File::Status File::setStatus( void )
{

	switch( errno ) {
	case EINVAL:
	case EACCES:
	case EMFILE:
	case ENFILE:
	case ENOSPC:
		return ( currentStatus = IOError );

	default:
		return ( currentStatus = UnknownError );
	}

}

File::Status File::setStatus( File::Status status )
{
	return ( currentStatus = status );
}

File::Status File::read( U32 size, char* dst, U32* bytes )
{

	if( currentStatus != Ok || size == 0 ) {
		return currentStatus;
	}

	U32 dummyBytes = 0;
	U32* bytesWritten = ( bytes == 0 ) ? &dummyBytes : bytes;

	if( ( *bytesWritten = fread( dst, 1, size, (FILE*) handle ) ) != size ) {

		if( ferror( (FILE*) handle ) ) {
			return setStatus( );
		} else {
			return ( currentStatus = EOS );
		}

	}

	return ( currentStatus = Ok );
}

File::Status File::write( U32 size, const char* src, U32* bytes )
{

	if( ( currentStatus != Ok && currentStatus != EOS ) || 0 == size ) {
		return currentStatus;
	}

	U32 dummyBytes = 0;
	U32* bytesWritten = ( bytes == 0 ) ? &dummyBytes : bytes;

	if( ( *bytesWritten = fwrite( src, 1, size, (FILE*) handle ) ) != size ) {

		if( ferror( (FILE*) handle ) ) {
			return setStatus( );
		}

	}

	return ( currentStatus = Ok );
}

bool File::hasCapability( Capability cap ) const
{
	return ( ( static_cast<U32>( cap ) & capability ) != 0 );
}

S32 Platform::compareFileTimes( const FileTime& a, const FileTime& b )
{

	if( a < b ) {
		return -1;
	} else if( a > b ) {
		return 1;
	} else {
		return 0;
	}

}

typedef void* HANDLE;
typedef unsigned char BOOL;
typedef unsigned int DWORD;
typedef char TCHAR;
typedef const char* LPCTSTR;

#define INVALID_HANDLE_VALUE NULL
#define FILE_ATTRIBUTE_NORMAL    0x0000
#define FILE_ATTRIBUTE_DIRECTORY 0x0040
#define MAX_PATH PATH_MAX
#define TRUE 1
#define FALSE 0

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

typedef struct _WIN32_FIND_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    TCHAR cFileName[ MAX_PATH ];
    TCHAR cAlternateFileName[ 14 ];
} WIN32_FIND_DATA, *LPWIN32_FIND_DATA;

HANDLE FindFirstFile( LPCTSTR, LPWIN32_FIND_DATA );
BOOL FindNextFile( HANDLE, LPWIN32_FIND_DATA );
BOOL FindClose( HANDLE );

/* Opaque handle for Find* functions. */
typedef struct tagFINDSTRUCT {
    char dirpath[PATH_MAX];
    DIR* dp;
    regex_t preg;
} FINDSTRUCT;

ssize_t PreprocessFilename( const char *src, char *dst, ssize_t maxlen )
{
    int len;
    const char *tmp;

    /* Check the size of the destination buffer */
    if( dst ) {

	if( PreprocessFilename( src, NULL, 0 ) > maxlen ) {
	    /* Not enough space in destination */
	    return -1;
	}

    }

    /* "*.*" is a special match anything case */
    if( strcmp( src, "*.*") == 0 ) {
	const char *match_all = "^.*$";

	len = strlen( match_all ) + 1;

	if( dst ) {
	    strcpy( dst, match_all );
	}

    } else

	/* "*." is also a special case -- match anything with no suffix
	   Warning:  "?*." also matches this case, but hope we don't hit it
	 */
	if( strcmp( src, "*." ) == 0 ) {
	    const char *match_all_nodot = "^[^\\.]*$";

	    len = strlen( match_all_nodot ) + 1;

	    if ( dst ) {
		strcpy( dst, match_all_nodot );
	    }

	} else {
	    len = 1;

	    for( tmp = src; *tmp; ++tmp ) {

		switch( *tmp ) {
		case '*':
		case '.':
		    ++len;
		default:
		    ++len;
		}

	    }

	    len += 2;
		
	    if( dst ) {
		*dst++ = '^';

		for( tmp = src; *tmp; ++tmp ) {

		    switch( *tmp ) {
		    case '*':
			*dst++ = '.';
			*dst++ = '*';
			break;
		    case '?':
			*dst++ = '.';
			break;
		    case '.':
			*dst++ = '\\';
			*dst++ = '.';
			break;
		    default:
			*dst++ = *tmp;
		    }

		}

		*dst++ = '$';
		*dst++ = '\0';
	    }

	}

    return len;
}

HANDLE FindFirstFile( LPCTSTR lpszSearchFile, LPWIN32_FIND_DATA lpffd )
{
    FINDSTRUCT * findhandle;
    int name_pos;
    char SearchFilename[256];
	
    assert( lpffd != NULL );
    assert( lpszSearchFile != NULL );
    assert( strstr( (char*)lpszSearchFile, ":"  ) == NULL );
    assert( strstr( (char*)lpszSearchFile, "\\" ) == NULL );
	
    /* Allocate memory for the file handle we return */
    findhandle = (FINDSTRUCT*) malloc( sizeof( *findhandle ) );

    if( findhandle == NULL ) {
	return INVALID_HANDLE_VALUE;
    }

    memset( findhandle, 0, sizeof( *findhandle ) );

    /* Detect and separate a path from a filename in the event there is one */
    strncpy( findhandle->dirpath, lpszSearchFile, 256 - 1 );
    findhandle->dirpath[256-1] = '\0';

    for( name_pos = strlen( findhandle->dirpath ) - 1; name_pos >= 0; name_pos-- ) {

	if( findhandle->dirpath[name_pos] == '/' ) {
	    findhandle->dirpath[name_pos] = '\0';
	    ++name_pos;
	    break;
	}

    }

    /* Get the filename as a regular expression pattern */
    PreprocessFilename( &lpszSearchFile[name_pos], SearchFilename, 256 );

    if( regcomp( &(findhandle->preg), SearchFilename, REG_ICASE ) != 0 ) {
	free( findhandle );
	return INVALID_HANDLE_VALUE;
    }

    /* Make sure we still have a directory after removing filename */
    if ( ! findhandle->dirpath[0] )
	strcpy( findhandle->dirpath, "." );

    /* Open the directory for reading */
    findhandle->dp = opendir( findhandle->dirpath );

    if( findhandle->dp == NULL ){
	FindClose( findhandle );
	return INVALID_HANDLE_VALUE;
    }

    /* Perform search */
    if( ! FindNextFile( findhandle, lpffd ) ) {
	FindClose( findhandle );
	return INVALID_HANDLE_VALUE;
    }

    return findhandle;
}

BOOL FindNextFile( HANDLE handle, LPWIN32_FIND_DATA lpffd )
{
    FINDSTRUCT* findhandle = (FINDSTRUCT*) handle;
    char filepath[256];
    struct stat statbuf;
    struct dirent *entry;

    assert( lpffd != NULL );
    assert( findhandle != NULL );

    while( ( entry = readdir( ( (FINDSTRUCT*)findhandle )->dp ) ) != NULL ) {

	if( (entry->d_name[0] != '.') && regexec( &( ( (FINDSTRUCT*) findhandle )->preg ),
						  entry->d_name,
						  0,
						  0,
						  0 ) == 0 ) {
	    sprintf( filepath, "%s/%s", findhandle->dirpath, entry->d_name );

	    if( stat( filepath, &statbuf ) < 0 ) {
		continue;
	    }

	    lpffd->ftLastAccessTime.dwLowDateTime = statbuf.st_atime;
	    lpffd->ftLastWriteTime.dwLowDateTime  = statbuf.st_ctime;

	    //Is path also included?
	    strncpy( lpffd->cFileName, entry->d_name, 256 - 1 );
	    lpffd->cFileName[256 - 1] = '\0';

            if( S_ISDIR( statbuf.st_mode ) ) {
		lpffd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	    } else if( S_ISREG( statbuf.st_mode ) ) {
		lpffd->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	    } else {
		lpffd->dwFileAttributes = 0;
	    }

	    lpffd->nFileSizeLow = statbuf.st_size;
	    return TRUE;
	}
    }

    return FALSE;
}

BOOL FindClose( HANDLE hFindFile )
{
    FINDSTRUCT* findhandle;

    findhandle = (FINDSTRUCT*) hFindFile;

    if( findhandle == NULL ) {
	return FALSE;
    }

    regfree( &findhandle->preg );

    if( findhandle->dp ) {
	closedir( findhandle->dp );
    }

    free( findhandle );

    return TRUE;
}

static bool recurseDumpPath( const char* basep,
			     const char* currentp,
			     Vector<Platform::FileInfo>& out )
{
	char current[PATH_MAX];
	char base[PATH_MAX];
	char buffer[PATH_MAX];
	char scratch[PATH_MAX];

	if( currentp ) {
		dStrncpy( current, currentp, PATH_MAX );
	} else {
		current[0] = 0;
	}

	dStrncpy( base, basep, PATH_MAX );
	basep = base;

	if( current[0] ) {
		dSprintf( buffer, sizeof( buffer ), "%s/%s/*", base, current );
	} else {
		dSprintf( buffer, sizeof( buffer ), "%s/*", base );
	}

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile( buffer, &findData );

	if( hFind == INVALID_HANDLE_VALUE ) {
		return false;
	}

	while( hFind != INVALID_HANDLE_VALUE ) {

		if( ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) {

			if( findData.cFileName[0] != '.' ) {
				scratch[0] = 0;
				int length = PATH_MAX;

				if( current[0] ) {
					dStrncpy( scratch, current, length );
					length -= dStrlen( current );
					dStrncat( scratch, "/", length );
					length--;
				}

				dStrncat( scratch, findData.cFileName, length );
				recurseDumpPath( base, scratch, out );
			}

		} else {
			out.increment( );
			Platform::FileInfo& info = out.last( );

			if( current[0] ) {
				dSprintf( scratch, sizeof( scratch ), "%s/%s", base, current );
				info.pFullPath = StringTable->insert( scratch );
				info.pVirtPath = StringTable->insert( current );
			} else {
				info.pFullPath = StringTable->insert( base );
				info.pVirtPath = NULL;
			}

			info.pFileName = StringTable->insert( findData.cFileName );
			info.fileSize = findData.nFileSizeLow;
		}

		if( FindNextFile( hFind, &findData ) == 0 ) {
			FindClose( hFind );
			hFind = INVALID_HANDLE_VALUE;
		}

	}

	return true;
}

bool Platform::getFileTimes( const char* path, FileTime* create, FileTime* modify )
{
	WIN32_FIND_DATA findData;
	HANDLE h = FindFirstFile( path, &findData );

	if( h == INVALID_HANDLE_VALUE ) {
		return false;
	}

	if( create ) {
		// NOTE: always 0 under Linux, but shouldn't be
		// an issue with the current usage in the codebase
		*create = findData.ftCreationTime.dwLowDateTime;
	}

	if( modify ) {
		*modify = findData.ftLastWriteTime.dwLowDateTime;
	}

	FindClose( h );
	return true;
}

bool Platform::createPath( const char* file )
{
	char filename[PATH_MAX];
	char pathbuf[PATH_MAX];
	const char* dir = 0;
	U32 length = 0;

	pathbuf[0] = 0;
	dSprintf( filename, PATH_MAX, "%s/%s", loki_getprefpath( ), file );
	file = filename;

	while( ( dir = dStrchr( file, '/' ) ) != 0 ) {
		dStrncpy( pathbuf + length, file, dir - file );
		pathbuf[ length + dir - file ] = 0;
		mkdir( pathbuf, 0700 );
		length += dir - file;
		pathbuf[ length++ ] = '/';
		file = dir + 1;
	}

	return true;
}

bool Platform::cdFileExists(const char *filePath, const char *volumeName, S32 serialNum)
{
   if (!filePath || !filePath[0])
      return true;

   // FIXME: Add code to scan for CD drives
   return true;

   return false;
}

bool Platform::dumpPath( const char* spec, Vector<Platform::FileInfo>& out )
{
	return recurseDumpPath( spec, 0, out );
}

void Platform::getCurrentDirectory( char* cwd, const U32 size )
{
	getcwd( cwd, size );
}

// FIXME: Add to Platform::.
bool platformGetUserPath( const char* base, char* user, U32 size )
{
	dSprintf( user, size, "%s/%s", loki_getprefpath( ), base );
	return true;
}
