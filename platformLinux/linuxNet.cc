//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

/* The ares library is a C API */
extern "C" {
#define class ares_class
#include <ares.h>
#undef class
}

#include "platformLinux/async.h"

#include "platformLinux/platformLinux.h"
#include "Platform/platform.h"
#include "Platform/event.h"
#include "console/console.h"
#include "Platform/gameInterface.h"
#include "Core/fileStream.h"


//
// Local types
//
typedef struct {
	int fd;
	U16 port;
} NameLookup;


//
// Sundry module-local variables.
//
static ares_channel channel;
static U16 defaultPort = 28000;
static U16 netPort = 0;
static NetSocket udpSocket = -1;


//
// Our asychronous callbacks.
//
static int selectCallback( int fd, int event, int status )
{
	static ConnectedNotifyEvent notifyEvent;
	static ConnectedReceiveEvent receiveEvent;
	static ConnectedAcceptEvent acceptEvent;

	int bytesRead = 0;
	Net::Error error = Net::NoError;

	switch( event ) {
	case FD_READ:
		error = Net::recv( fd, receiveEvent.data, MaxPacketDataSize, &bytesRead );

		if( error == Net::NoError & bytesRead != 0 ) {
			receiveEvent.tag = fd;
			receiveEvent.size = ConnectedReceiveEventHeaderSize + bytesRead;
			Game->postEvent( receiveEvent );
		}

		break;
	case FD_CONNECT:
		notifyEvent.tag = fd;

		if( status ) {
			notifyEvent.state = ConnectedNotifyEvent::Connected;
		} else {
			notifyEvent.state = ConnectedNotifyEvent::ConnectFailed;
		}

		Game->postEvent( notifyEvent );
		break;
	case FD_CLOSE:

		for( ; ; ) {
			error = Net::recv( fd, receiveEvent.data, MaxPacketDataSize, &bytesRead );

			if( error != Net::NoError || bytesRead == 0 ) {
				break;
			}

			receiveEvent.tag = fd;
			receiveEvent.size = ConnectedReceiveEventHeaderSize + bytesRead;
			Game->postEvent( receiveEvent );
		}

		notifyEvent.tag = fd;
		notifyEvent.state = ConnectedNotifyEvent::Disconnected;
		Game->postEvent( notifyEvent );
		break;
	case FD_ACCEPT:
		acceptEvent.portTag = fd;
		acceptEvent.connectionTag = Net::accept( fd, &acceptEvent.address );

		if( acceptEvent.connectionTag != InvalidSocket ) {
			Net::setBlocking( acceptEvent.connectionTag, false );
			AsyncSelect( acceptEvent.connectionTag, selectCallback, FD_READ | FD_CONNECT | FD_CLOSE );
			Game->postEvent( acceptEvent );
		}

		break;
	}

}

static void lookupCallback( void* arg, int status, struct hostent* host )
{
	static ConnectedNotifyEvent notifyEvent;

	char* errorMem = 0;
	char** addr = 0;
	NameLookup* lookup = static_cast<NameLookup*>( arg );

	if( status != ARES_SUCCESS ) {
		ares_strerror( status, &errorMem ); 
		Con::printf( "couldn't perform lookup: %s", errorMem );
		ares_free_errmem( errorMem );
		delete lookup;
		return;
	}

	struct sockaddr_in ipAddr;

	dMemcpy( &ipAddr.sin_addr.s_addr, host->h_addr_list[0], sizeof( struct in_addr ) );
	ipAddr.sin_port = lookup->port;
	ipAddr.sin_family = AF_INET;

	notifyEvent.tag = lookup->fd;

	int error = ::connect( lookup->fd, (struct sockaddr*) &ipAddr, sizeof( ipAddr ) );

	if( error == -1 && errno != EINPROGRESS ) {
		notifyEvent.state = ConnectedNotifyEvent::DNSFailed;
		::close( lookup->fd );
	} else {
		AsyncSelect( lookup->fd, selectCallback, FD_READ | FD_CONNECT | FD_CLOSE );
		notifyEvent.state = ConnectedNotifyEvent::DNSResolved;
	}

	Game->postEvent( notifyEvent );
	delete lookup;
}


//
// Utilities.
//
static void netToIPSocketAddress( const NetAddress* addr, struct sockaddr_in* ip )
{
	memset( ip, 0, sizeof( struct sockaddr_in ) );
	ip->sin_family = AF_INET;
	ip->sin_port = htons( addr->port );
	ip->sin_addr.s_addr = *( (U32*) addr->netNum );
}

static void IPSocketToNetAddress( const sockaddr_in* ip, NetAddress* addr )
{
	addr->type = NetAddress::IPAddress;
	addr->port = htons( ip->sin_port );
	*( (U32*) addr->netNum ) = ip->sin_addr.s_addr;
}

static Net::Error getLastError( void )
{

	// God knows what distinguishes Invalid from Wrong...
	switch( errno ) {
	case 0:
		return Net::NoError;
	case EBADF:
	case ENOTSOCK:
		return Net::NotASocket;
	case EAGAIN:
		return Net::WouldBlock;
	case ENOTCONN:
		return Net::InvalidPacketProtocol;
	case EOPNOTSUPP:
		return Net::WrongProtocolType;
	default:
		return Net::UnknownError;
	}

}


//
// Net:: implementation
//
bool Net::init( void )
{
	int status;
	char* errorMem;

	if( AsyncInit( ) == -1 ) {
		Con::printf( "AsyncInit failed" );
		return false;
	}

	status = ares_init( &channel );

	if( status != ARES_SUCCESS ) {
		Con::printf( "ares_init: %s", ares_strerror( status, &errorMem ) );
		ares_free_errmem( errorMem );
		return false;
	}

	return true;
}

void Net::shutdown( void )
{
	AsyncShutdown( );
	ares_destroy( channel );
	closePort( );
}

NetSocket Net::openListenPort( U16 port )
{

	if( Game->isJournalReading( ) ) {
		U32 ret;
		Game->journalRead( &ret );
		return ret;
	}

	NetSocket fd = openSocket( );
	bind( fd, port );
	listen( fd, 4 );
	setBlocking( fd, false );

	if( AsyncSelect( fd, selectCallback, FD_ACCEPT ) == -1 ) {
		Con::printf( "Error: %d", errno );
	}

	if( Game->isJournalWriting( ) ) {
		Game->journalWrite( fd );
	}

	return fd;
}

NetSocket Net::openConnectTo( const char* addressString )
{

	if( !dStrnicmp( addressString, "ip:", 3 ) ) {
		addressString += 3;
	}

	char remoteAddr[256];
	dStrncpy( remoteAddr, addressString, 256 );

	char* portString = dStrchr( remoteAddr, ':' );
	U16 port;

	if( portString ) {
		*portString++ = 0;
		port = htons( dAtoi( portString ) );
	} else {
		port = htons( defaultPort );
	}

	if( !dStricmp( remoteAddr, "broadcast" ) ) {
		return InvalidSocket;
	}

	if( Game->isJournalReading( ) ) {
		U32 ret;
		Game->journalRead( &ret );
		return ret;
	}

	NetSocket fd = openSocket( );
	setBlocking( fd, false );

	struct sockaddr_in ipAddr;

	ipAddr.sin_addr.s_addr = inet_addr( remoteAddr );

	if( ipAddr.sin_addr.s_addr != INADDR_NONE ) {
		ipAddr.sin_port = port;
		ipAddr.sin_family = AF_INET;

		int error = ::connect( fd, (struct sockaddr*) &ipAddr, sizeof( ipAddr ) );

		if( error == -1 && errno != EINPROGRESS ) {
			Con::printf( "Last error: %d", errno );
			::close( fd );
			fd = InvalidSocket;
		}

		if( fd != InvalidSocket ) {
			AsyncSelect( fd, selectCallback, FD_READ  | FD_CONNECT | FD_CLOSE );
		}

	} else {
		NameLookup* lookup = new NameLookup;
		lookup->fd = fd;
		lookup->port = port;
		ares_gethostbyname( channel, remoteAddr, AF_INET, lookupCallback, lookup );
	}

	if( Game->isJournalWriting( ) ) {
		Game->journalWrite( fd );
	}

	return fd;
}

void Net::closeConnectTo( NetSocket fd )
{

	if( Game->isJournalReading( ) ) {
		return;
	}

	AsyncCancel( fd );

	::close( fd );
}

Net::Error Net::sendTo( NetSocket fd, const U8* buffer, int size )
{

	if( Game->isJournalReading( ) ) {
		U32 error;
		Game->journalRead( &error );
		return static_cast<Net::Error>( error );
	}

	Net::Error error = send( fd, buffer, size );

	if( Game->isJournalWriting( ) ) {
		Game->journalWrite( static_cast<U32>( error ) );
	}

	return error;
}

bool Net::openPort( S32 port )
{
	closePort( );

	udpSocket = socket( AF_INET, SOCK_DGRAM, 0 );

	if( udpSocket != -1 ) {
		Net::Error error;

		error = bind( udpSocket, port );

		if( error == NoError ) {
			error = setBufferSize( udpSocket, 32768 );
		}

		if( error == NoError ) {
			error = setBroadcast( udpSocket, true );
		}

		if( error == NoError ) {
			error = setBlocking( udpSocket, false );
		}

		if( error == NoError ) {
			Con::printf( "UDP initialized on port %d", port );
		} else {
			close( udpSocket );
			udpSocket = -1;
			Con::printf( "Unable to initialize UDP -- error %d", error );
		}

	}

	netPort = port;
	return ( udpSocket != -1 );
}

void Net::closePort( void )
{

	if( udpSocket != -1 ) {
		close( udpSocket );
	}

}

Net::Error Net::sendto( const NetAddress* addr, const U8* buffer, S32 size )
{

	if( Game->isJournalReading( ) ) {
		return NoError;
	}

	struct sockaddr_in ip;
	netToIPSocketAddress( addr, &ip );

	if( ::sendto( udpSocket, buffer, size, 0, (struct sockaddr*) &ip, sizeof( ip ) ) == -1 ) {
		return getLastError( );
	}

	return NoError;
}

void Net::process( void )
{
	PacketReceiveEvent event;

	for( ; ; ) {
		struct sockaddr sa;
		socklen_t length = sizeof( sa );
		S32 bytes = -1;

		if( udpSocket != -1 ) {
			bytes = recvfrom( udpSocket, event.data, MaxPacketDataSize, 0, &sa, &length );
		}

		if( bytes == -1 ) {
			break;
		}

		if( sa.sa_family == AF_INET ) {
			IPSocketToNetAddress( (struct sockaddr_in*) &sa, &event.sourceAddress );
		} else {
			continue;
		}

		NetAddress& addr = event.sourceAddress;

		if( addr.type == NetAddress::IPAddress &&
		    addr.netNum[0] == 127 && addr.netNum[1] == 0 &&
		    addr.netNum[1] == 0   && addr.netNum[2] == 1 &&
		    addr.port == netPort ) {
			continue;
		}

		if( bytes <= 0 ) {
			continue;
		}

		event.size = PacketReceiveEventHeaderSize + bytes;
		Game->postEvent( event );
	}

	// do ares DNS processing
	fd_set read_fds, write_fds;
	int nfds = 0;
	struct timeval* timeout = 0;
	struct timeval tv;
	struct timezone tz;

	FD_ZERO( &read_fds );
	FD_ZERO( &write_fds );
	nfds = ares_fds( channel, &read_fds, &write_fds );

	if( nfds == 0 ) {
		return;
	}

	timeout = ares_timeout( channel, 0, &tv );
	select( nfds, &read_fds, &write_fds, 0, timeout );
	ares_process( channel, &read_fds, &write_fds );
}

NetSocket Net::openSocket( void )
{
	int fd;

	if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		return InvalidSocket;
	}

	return fd;
}

Net::Error Net::closeSocket( NetSocket fd )
{

	if( fd == InvalidSocket ) {
		return NotASocket;
	}

	if( close( fd ) ) {
		return getLastError( );
	}

	return NoError;
}

Net::Error Net::connect( NetSocket fd, const NetAddress* addr )
{

	if( addr->type != NetAddress::IPAddress ) {
		return WrongProtocolType;
	}

	struct sockaddr_in addrIn;

	netToIPSocketAddress( addr, &addrIn );

	if( ::connect( fd, (struct sockaddr*) &addrIn, sizeof( addrIn ) ) ) {
		return getLastError( );
	}

	return NoError;
}

Net::Error Net::listen( NetSocket fd, S32 backlog )
{

	if( ::listen( fd, backlog ) ) {
		return getLastError( );
	}

	return NoError;
}

NetSocket Net::accept( NetSocket fd, NetAddress* addr )
{
	struct sockaddr_in ip;
	socklen_t length = sizeof( ip );

	int ret = ::accept( fd, (struct sockaddr*) &ip, &length );

	if( ret != -1 ) {
		IPSocketToNetAddress( &ip, addr );
		return ret;
	}

	return InvalidSocket;
}

Net::Error Net::bind( NetSocket fd, U16 port )
{
	struct sockaddr_in ip;

	dMemset( &ip, 0, sizeof( ip ) );
	ip.sin_family = AF_INET;

	// It's entirely possible that there are two NIC cards.
	// We let the user specify which one the server runs on.

	// thanks to [TPG]P1aGu3 for the name
	const char* serverIP = Con::getVariable( "Host::BindAddress" );
	// serverIP is guaranteed to be non-0.
	AssertFatal( serverIP, "serverIP is NULL!" );

	if( serverIP[0] != '\0' ) {
		// we're not empty
		ip.sin_addr.s_addr = inet_addr( serverIP );

		if( ip.sin_addr.s_addr != INADDR_NONE ) {
			Con::printf( "Binding server port to %s", serverIP );
		} else {
			Con::warnf( ConsoleLogEntry::General,
				    "inet_addr() failed for %s while binding!",
				    serverIP );
			ip.sin_addr.s_addr = INADDR_ANY;
		}

	} else {
		Con::printf( "Binding server port to default IP" );
		ip.sin_addr.s_addr = INADDR_ANY;
	}

	ip.sin_port = htons( port );

	if( ::bind( fd, (struct sockaddr*) &ip, sizeof( ip ) ) ) {
		return getLastError( );
	}

	return NoError;
}

Net::Error Net::setBufferSize( NetSocket fd, S32 size )
{

	if( setsockopt( fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof( size ) ) ) {
		return getLastError( );
	}

	if( setsockopt( fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof( size ) ) ) {
		return getLastError( );
	}

	return NoError;
}

Net::Error Net::setBroadcast( NetSocket fd, bool broadcast )
{
	S32 bc = broadcast;

	if( setsockopt( fd, SOL_SOCKET, SO_BROADCAST, &bc, sizeof( bc ) ) ) {
		return getLastError( );
	}

	return NoError;
}

Net::Error Net::setBlocking( NetSocket fd, bool blocking )
{
	int flags;

	// Let's do it the Posix.1g way!
	if( ( flags = fcntl( fd, F_GETFL, 0 ) ) < 0 ) {
		return getLastError( );
	}

	if( blocking ) {
		flags &= ~O_NONBLOCK;
	} else {
		flags |= O_NONBLOCK;
	}

	if( fcntl( fd, F_SETFL, flags ) < 0 ) {
		return getLastError( );
	}

	return NoError;
}

Net::Error Net::send( NetSocket fd, const U8* buffer, S32 size )
{

	if( ::send( fd, buffer, size, 0 ) == -1 ) {
		return getLastError( );
	}

	return NoError;
}

Net::Error Net::recv( NetSocket fd, U8* buffer, S32 size, S32* read )
{

	if( ( *read = ::recv( fd, buffer, size, 0 ) ) == -1 ) {
		return getLastError( );
	}

	return NoError;
}

bool Net::compareAddresses( const NetAddress* a1, const NetAddress* a2 )
{
	// cast our separated ipv4 into a 32-bit word for compare
	bool sameType = ( a1->type == a2->type );
	bool sameNum = ( *( (U32*) a1->netNum ) == *( (U32*) a2->netNum ) );
	bool samePort = ( a1->port == a2->port );
	return ( sameType && sameNum && samePort );
}

bool Net::stringToAddress( const char* addressString, NetAddress* address )
{

	if( dStrnicmp( addressString, "ip:", 3 ) == 0 ) {
		addressString += 3;
	}

	if( dStrnicmp( addressString, "ipx:", 4 ) == 0 ) {
		// foo on IPX.
		return false;
	}

	if( dStrlen( addressString ) > 255 ) {
		return false;
	}

	struct sockaddr_in ipAddr;
	char remoteAddr[256];
	dStrcpy( remoteAddr, addressString );
	char* portString = dStrchr( remoteAddr, ':' );

	if( portString ) {
		*portString++ = 0;
	}

	struct hostent* he = 0;

	if( dStricmp( remoteAddr, "broadcast" ) == 0 ) {
		ipAddr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
	} else {
		ipAddr.sin_addr.s_addr = inet_addr( remoteAddr );

		if( ipAddr.sin_addr.s_addr == INADDR_NONE ) {

			if( ( he == gethostbyname( remoteAddr ) ) == NULL ) {
				return false;
			} else {
				memcpy( &ipAddr.sin_addr.s_addr, he->h_addr, sizeof( struct in_addr ) );
			}

		}

	}

	if( portString ) {
		ipAddr.sin_port = htons( dAtoi( portString ) );
	} else {
		ipAddr.sin_port = htons( defaultPort );
	}

	ipAddr.sin_family = AF_INET;
	IPSocketToNetAddress( &ipAddr, address );
	return true;
}

void Net::addressToString( const NetAddress* address, char addressString[256] )
{

	if( address->type == NetAddress::IPAddress ) {
		struct sockaddr_in ipAddr;

		netToIPSocketAddress( address, &ipAddr );

		if( ipAddr.sin_addr.s_addr == htonl( INADDR_BROADCAST ) ) {
			dSprintf( addressString, 256, "IP:Broadcast:%d", ntohs( ipAddr.sin_port ) );
		} else {
			dSprintf( addressString, 256, "IP:%d.%d.%d.%d:%d",
				  ( ipAddr.sin_addr.s_addr ) & 0xff,
				  ( ipAddr.sin_addr.s_addr >> 8  ) & 0xff,
				  ( ipAddr.sin_addr.s_addr >> 16 ) & 0xff,
				  ( ipAddr.sin_addr.s_addr >> 24 ) & 0xff,
				  ntohs( ipAddr.sin_port ) );
		}

	}

}
