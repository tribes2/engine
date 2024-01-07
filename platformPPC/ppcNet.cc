//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <OpenTransport.h>
#include <OpenTptInternet.h>

#include "PlatformPPC/platformPPC.h"
#include "Platform/platform.h"
#include "Platform/event.h"
//#include <winsock.h>
//#include <wsipx.h>
#include "console/console.h"


// stubbed stuff to get this to compile
typedef void* HANDLE;
typedef void* SOCKET;
#define INVALID_SOCKET NULL
typedef InetAddress SOCKADDR_IN;
typedef InetAddress SOCKADDR_IPX;


struct Connection
{
   NetSocket socket;
   S32 state;
   S32 prevState;
   bool listenConnection;
   Connection *nextConnection;
   Connection *nextInTable;
   HANDLE connectThreadHandle;
   U32 tag;
};

struct Status
{
   bool mSignal;
   OSErr mErr;
   void *mData;
   Status()
   { 
      mSignal = false;
      mErr = kOTNoError;
      mData = NULL;
   }
};


static Net::Error getLastError();
static S32 defaultPort = 28000;
static S32 netPort = 0;
//static SOCKET ipxSocket = INVALID_SOCKET;
//static SOCKET udpSocket = INVALID_SOCKET;
static bool sendPackets = true;
static U32 nextConnectionId = 1;
static U32 nextAcceptId = 1000000000;
static Connection *connectionList = NULL;
enum {
   ConnectionTableSize = 256,
   ConnectionTableMask = 0xFF
};
static Connection *connectionTable[ConnectionTableSize] = { 0, };


// kOTAnyInetAddress
static U32 OTReferenceCount = 0;
static TEndpoint *EndPoint = NULL;
static OSErr err = kOTNoError;


static pascal void EventProc( void *_pSession, OTEventCode event, OTResult result, void *cookie )
{
	Status *status = (Status *) _pSession;

	switch ( event )
	{
		case T_DATA:
			//pSess->m_dataToRead = 1;
			break;
	
		case T_BINDCOMPLETE:
			status->mErr = (OSErr) result;
			status->mSignal = true;
			break;	

		case T_OPENCOMPLETE:
			status->mErr = (OSErr) result;
			status->mData = cookie;  // the EndPoint
			status->mSignal = true;
			break;
	}
}


void Net::setJournaling(bool jrn)
{
   sendPackets = !jrn;
}

bool Net::init()
{
//   WSADATA stWSAData;
//   return !WSAStartup(0x0101, &stWSAData);
   if ( OTReferenceCount == 0 )
      if (InitOpenTransport() != kOTNoError)
		   return false;
	
   // successfully opened Open Transport, inncrement the reference count		
   OTReferenceCount++;
   return true;
}

void Net::shutdown()
{
//   while(connectionList)
//      Net::closeConnectTo(connectionList->tag);
//
//   closePort();
//   WSACleanup();
   
   while(connectionList)
      Net::closeConnectTo(connectionList->tag);
      
   closePort();
   if ( OTReferenceCount <= 0 )
      return;
   
   // close Open Transport if there are no more references to it
   OTReferenceCount--;   
   if ( OTReferenceCount == 0 )
      CloseOpenTransport();   
}

static void netToIPSocketAddress(const NetAddress *address, SOCKADDR_IN *sockAddr)
{
   address;
   dMemset(sockAddr, 0, sizeof(SOCKADDR_IN));
//   sockAddr->sin_family = AF_INET;
//   sockAddr->sin_port = htons(address->port);
//   sockAddr->sin_addr.s_net = address->netNum[0];
//   sockAddr->sin_addr.s_host = address->netNum[1];
//   sockAddr->sin_addr.s_lh = address->netNum[2];
//   sockAddr->sin_addr.s_impno = address->netNum[3];
}

static void IPSocketToNetAddress(const SOCKADDR_IN *sockAddr, NetAddress *address)
{
   sockAddr, address;
//   address->type = NetAddress::IPAddress;
//   address->port = htons(sockAddr->sin_port);
//   address->netNum[0] = sockAddr->sin_addr.s_net;
//   address->netNum[1] = sockAddr->sin_addr.s_host;
//   address->netNum[2] = sockAddr->sin_addr.s_lh;
//   address->netNum[3] = sockAddr->sin_addr.s_impno;
}

static void netToIPXSocketAddress(const NetAddress *address, SOCKADDR_IPX *sockAddr)
{
    sockAddr, address;
//   dMemset(sockAddr, 0, sizeof(SOCKADDR_IPX));
//   sockAddr->sa_family = AF_INET;
//   sockAddr->sa_socket = htons(address->port);
//   sockAddr->sa_netnum[0] = address->netNum[0];
//   sockAddr->sa_netnum[1] = address->netNum[1];
//   sockAddr->sa_netnum[2] = address->netNum[2];
//   sockAddr->sa_netnum[3] = address->netNum[3];
//   sockAddr->sa_nodenum[0] = address->nodeNum[0];
//   sockAddr->sa_nodenum[1] = address->nodeNum[1];
//   sockAddr->sa_nodenum[2] = address->nodeNum[2];
//   sockAddr->sa_nodenum[3] = address->nodeNum[3];
//   sockAddr->sa_nodenum[4] = address->nodeNum[4];
//   sockAddr->sa_nodenum[5] = address->nodeNum[5];
}

static void IPXSocketToNetAddress(const SOCKADDR_IPX *sockAddr, NetAddress *address)
{
    sockAddr, address;
//   address->type = NetAddress::IPXAddress;
//   address->port = htons(sockAddr->sa_socket);
//   address->netNum[0]  = sockAddr->sa_netnum[0] ;
//   address->netNum[1]  = sockAddr->sa_netnum[1] ;
//   address->netNum[2]  = sockAddr->sa_netnum[2] ;
//   address->netNum[3]  = sockAddr->sa_netnum[3] ;
//   address->nodeNum[0] = sockAddr->sa_nodenum[0];
//   address->nodeNum[1] = sockAddr->sa_nodenum[1];
//   address->nodeNum[2] = sockAddr->sa_nodenum[2];
//   address->nodeNum[3] = sockAddr->sa_nodenum[3];
//   address->nodeNum[4] = sockAddr->sa_nodenum[4];
//   address->nodeNum[5] = sockAddr->sa_nodenum[5];
}

//DWORD WINAPI connectThreadFunction(LPVOID param)
//{
//   Connection *con = (Connection *) param;
//   con;
//   return 0;
//}

static Connection *newConnection(U32 id)
{
    Connection *conn = new Connection;
    conn->nextConnection = connectionList;
    conn->tag = id;
    connectionList = conn;
    conn->nextInTable = connectionTable[conn->tag & ConnectionTableMask];
    connectionTable[conn->tag & ConnectionTableMask] = conn;
   return conn;
}

U32 Net::openListenPort(U16 port)
{
   nextConnectionId++;
   if(!sendPackets)
      return nextConnectionId;

   Connection *conn = newConnection(nextConnectionId);

   conn->listenConnection = true;

   conn->socket = openSocket();
   bind(conn->socket, port);
   listen(conn->socket, 4);
   setBlocking(conn->socket, false);
   return conn->tag;
}

U32 Net::openConnectTo(U16 port, const NetAddress *address)
{
   port;
   
   nextConnectionId++;
   if(!sendPackets)
      return nextConnectionId;
      
   Connection *conn = newConnection(nextConnectionId);

   conn->listenConnection = false;
   conn->prevState = ConnectedNotifyEvent::DNSResolve;
   conn->state = ConnectedNotifyEvent::DNSResolve;
   
   conn->socket = openSocket();
   connect(conn->socket, address);
   setBlocking(conn->socket, false);
   
//threadHandle = CreateThread(NULL, 0, connectThreadFunction, (LPVOID) conn, 0, &threadId);


//CloseHandle(threadHandle);
   conn->state = ConnectedNotifyEvent::Connected;
   return conn->tag;
}

void Net::processConnected()
{
   Connection **walk = &connectionList;
   Connection *con;
   while((con = *walk) != NULL)
   {
      bool del = false;
      if(con->listenConnection)
      {
         NetSocket newSocket;
         ConnectedAcceptEvent event;
         newSocket = accept(con->socket, &event.address);

         if(newSocket != InvalidSocket)
         {
            Connection *nc = newConnection(nextAcceptId++);
            nc->listenConnection = false;
            nc->prevState = ConnectedNotifyEvent::Connected;
            nc->state = ConnectedNotifyEvent::Connected;
            nc->socket = newSocket;
            setBlocking(nc->socket, false);
            
            event.portTag = con->tag;
            event.connectionTag = nc->tag;
            GamePostEvent(event);
         }
         walk = &con->nextConnection;
      }
      else
      {
         if(con->state != con->prevState)
         {
            ConnectedNotifyEvent event;
            event.tag = con->tag;
            event.state = con->state;
            GamePostEvent(event);
            con->prevState = con->state;
         }
         if(con->state == ConnectedNotifyEvent::Connected)
         {
            ConnectedReceiveEvent event;
            Net::Error err;
            S32 bytesRead;
            event.tag = con->tag;
         
            do {
//                err = recv(con->socket, event.data, MaxPacketDataSize, &bytesRead);
               if(err == NoError && bytesRead != 0)
               {
                  event.size = ConnectedReceiveEventHeaderSize + bytesRead;
                  GamePostEvent(event);
               }
               else if(err != WouldBlock)
               {
                  // bad news... this disconnected
                  ConnectedNotifyEvent event;
                  event.tag = con->tag;
                  event.state = ConnectedNotifyEvent::Disconnected; 
                  GamePostEvent(event);
                  del = true;
               }
            }
            while(err == NoError && bytesRead != 0);
         }
         if(del)
         {
            *walk = con->nextConnection;
            closeSocket(con->socket);
            for(Connection **tbWalk = &connectionTable[con->tag & ConnectionTableMask]; *tbWalk != NULL; tbWalk = &(*tbWalk)->nextInTable)
            {
               Connection *dc = *tbWalk;
               if(dc->tag == con->tag)
               {
                  *tbWalk = dc->nextInTable;
                  break;
               }
            }
            delete con;
         }
         else
            walk = &con->nextConnection;
      }
   }
}

void Net::sendTo(U32 tag, const U8 *buffer, S32 bufferSize)
{
   Connection *walk;
   for(walk = connectionTable[tag & ConnectionTableMask]; walk != NULL; walk = walk->nextConnection)
      if(walk->tag == tag)
         break;
   if(walk)
      send(walk->socket, buffer, bufferSize);
}

void Net::closeConnectTo(U32 tag)
{
   Connection **walk;
   for(walk = &connectionList; *walk != NULL; walk = &(*walk)->nextConnection)
   {
      Connection *con = *walk;
      if(con->tag == tag)
      {
         *walk = con->nextConnection;
         closeSocket(con->socket);
         break;
      }
   }
   for(walk = &connectionTable[tag & ConnectionTableMask]; *walk != NULL; walk = &(*walk)->nextInTable)
   {
      Connection *con = *walk;
      if(con->tag == tag)
      {
         *walk = con->nextInTable;
         delete con;
         return;
      }
   }
}
   
bool Net::openPort(S32 port)
{
//    if(udpSocket != INVALID_SOCKET)
//       closesocket(udpSocket);
//    if(ipxSocket != INVALID_SOCKET)
//       closesocket(ipxSocket);
//       
//    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
//    ipxSocket = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
// 
//    if(udpSocket != INVALID_SOCKET)
//    {
//       Net::Error error;
//       error = bind(udpSocket, port);
//       if(error == NoError)
//          error = setBufferSize(udpSocket, 32768);
//       if(error == NoError)
//          error = setBroadcast(udpSocket, true);
//       if(error == NoError)
//          error = setBlocking(udpSocket, false);
//       if(error == NoError)
//          Con::printf("UDP initialized on port %d", port);
//       else
//       {
//          closesocket(udpSocket);
//          udpSocket = INVALID_SOCKET;
//          Con::printf("Unable to initialize UDP - error %d", error);
//       }
//    }
//    if(ipxSocket != INVALID_SOCKET)
//    {
//       Net::Error error = NoError;
//       SOCKADDR_IPX ipxAddress;   
//    	memset((char *)&ipxAddress, 0, sizeof(ipxAddress));
// 	   ipxAddress.sa_family = AF_IPX;
// 	   ipxAddress.sa_socket = htons(port);
//       S32 err = ::bind(ipxSocket, (PSOCKADDR) &ipxAddress, sizeof(ipxAddress));
//       if(err)
//          error = getLastError();
//       if(error == NoError)
//          error = setBufferSize(ipxSocket, 32768);
//       if(error == NoError)
//          error = setBroadcast(ipxSocket, true);
//       if(error == NoError)
//          error = setBlocking(ipxSocket, false);
//       if(error == NoError)
//          Con::printf("IPX initialized on port %d", port);
//       else
//       {
//          closesocket(ipxSocket);
//          ipxSocket = INVALID_SOCKET;
//          Con::printf("Unable to initialize IPX - error %d", error);
//       }
//    }
   netPort = port;
//    return ipxSocket != INVALID_SOCKET || udpSocket != INVALID_SOCKET;
   return true;
}

void Net::closePort()
{
//    if(ipxSocket != INVALID_SOCKET)
//       closesocket(ipxSocket);
//    if(udpSocket != INVALID_SOCKET)
//       closesocket(udpSocket);
}

Net::Error Net::sendto(const NetAddress *address, const U8 *buffer, S32 bufferSize)
{
   address, buffer, bufferSize;
   return NoError;
//    if(!sendPackets)
//       return NoError;
//    if(address->type == NetAddress::IPXAddress)
//    {
//       SOCKADDR_IPX ipxAddr;
//       netToIPXSocketAddress(address, &ipxAddr);
//       if(::sendto(ipxSocket, (const char*)buffer, bufferSize, 0,
//             (PSOCKADDR) &ipxAddr, sizeof(SOCKADDR_IPX)) == SOCKET_ERROR)
//          return getLastError();
//       else
//          return NoError;
//    }
//    else
//    {
//       SOCKADDR_IN ipAddr;
//       netToIPSocketAddress(address, &ipAddr);
//       if(::sendto(udpSocket, (const char*)buffer, bufferSize, 0,
//             (PSOCKADDR) &ipAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
//          return getLastError();
//       else
//          return NoError;
//    }
}

void Net::process()
{
//    SOCKADDR sa;
// 
//    PacketReceiveEvent receiveEvent;
//    for(;;)
//    {
//       S32 addrLen = sizeof(sa);
//       S32 bytesRead = SOCKET_ERROR;
//       if(udpSocket != INVALID_SOCKET)
//          bytesRead = recvfrom(udpSocket, (char *) receiveEvent.data, MaxPacketDataSize, 0, &sa, &addrLen);
//       if(bytesRead == SOCKET_ERROR && ipxSocket != INVALID_SOCKET)
//       {
//          addrLen = sizeof(sa);
//          bytesRead = recvfrom(ipxSocket, (char *) receiveEvent.data, MaxPacketDataSize, 0, &sa, &addrLen);
//       }
//       
//       if(bytesRead == SOCKET_ERROR)
//          break;
//       
//       if(sa.sa_family == AF_INET)
//          IPSocketToNetAddress((SOCKADDR_IN *) &sa, &receiveEvent.sourceAddress);
//       else if(sa.sa_family == AF_IPX)
//          IPXSocketToNetAddress((SOCKADDR_IPX *) &sa, &receiveEvent.sourceAddress);
//       else
//          continue;
//          
//       NetAddress &na = receiveEvent.sourceAddress;
//       if(na.type == NetAddress::IPAddress &&
//             na.netNum[0] == 127 &&
//             na.netNum[1] == 0 &&
//             na.netNum[2] == 0 &&
//             na.netNum[3] == 1 &&
//             na.port == netPort)
//          continue;
//       if(bytesRead <= 0)
//          continue;
//       receiveEvent.size = PacketReceiveEventHeaderSize + bytesRead;
//       GamePostEvent(receiveEvent);
//    }
}      
                  
NetSocket Net::openSocket()
{
   return InvalidSocket;
//    SOCKET retSocket;
//    retSocket = socket(AF_INET, SOCK_STREAM, 0);
// 
//    if(retSocket == INVALID_SOCKET)
//       return InvalidSocket;
//    else
//       return retSocket;
}

Net::Error Net::closeSocket(NetSocket socket)
{
    if(socket != InvalidSocket)
    {
//       if(!closesocket(socket))
//          return NoError;
//       else
          return getLastError();
    }
    else
       return NotASocket;
}

Net::Error Net::connect(NetSocket socket, const NetAddress *address)
{
   socket;
   if(address->type != NetAddress::IPAddress)
       return WrongProtocolType;
//    SOCKADDR_IN socketAddress;
//    netToIPSocketAddress(address, &socketAddress);
//    if(!::connect(socket, (PSOCKADDR) &socketAddress, sizeof(socketAddress)))
//       return NoError;
   return getLastError();
}

Net::Error Net::listen(NetSocket socket, S32 backlog)
{
   socket, backlog;
//    if(!::listen(socket, backlog))
//       return NoError;
   return getLastError();
}

NetSocket Net::accept(NetSocket acceptSocket, NetAddress *remoteAddress)
{
   acceptSocket, remoteAddress;
//    SOCKADDR_IN socketAddress;
//    S32 addrLen = sizeof(socketAddress);
//    
//    SOCKET retVal = ::accept(acceptSocket, (PSOCKADDR) &socketAddress, &addrLen);
//    if(retVal != INVALID_SOCKET)
//    {
//       IPSocketToNetAddress(&socketAddress, remoteAddress);
//       return retVal;
//    }
   return InvalidSocket;
}

Net::Error Net::bind(NetSocket socket, U16 port)
{
   socket, port;
//    S32 error;
//    
//    SOCKADDR_IN socketAddress;
// 	dMemset((char *)&socketAddress, 0, sizeof(socketAddress));
// 	socketAddress.sin_family = AF_INET;
// 	socketAddress.sin_addr.s_addr = INADDR_ANY;
// 	socketAddress.sin_port = htons(port);
//    error = ::bind(socket, (PSOCKADDR) &socketAddress, sizeof(socketAddress));
// 
//    if(!error)
//       return NoError;
   return getLastError();
}

Net::Error Net::setBufferSize(NetSocket socket, S32 bufferSize)
{
   socket, bufferSize;
//    S32 error;
//    error = setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *) &bufferSize, sizeof(bufferSize));
//    if(!error)
//       error = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *) &bufferSize, sizeof(bufferSize));
//    if(!error)
//       return NoError;
   return getLastError();
}

Net::Error Net::setBroadcast(NetSocket socket, bool broadcast)
{
   socket, broadcast;
//    S32 bc = broadcast;
//    S32 error = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&bc, sizeof(bc));
//    if(!error)
//       return NoError;
   return getLastError();   
}

Net::Error Net::setBlocking(NetSocket socket, bool blockingIO)
{
   socket, blockingIO;
//    DWORD notblock = !blockingIO;
//    S32 error = ioctlsocket(socket, FIONBIO, &notblock);
//    if(!error)
//       return NoError;
   return getLastError();   
}

Net::Error Net::send(NetSocket socket, const U8 *buffer, S32 bufferSize)
{
   socket, buffer, bufferSize;
//    S32 error = ::send(socket, (const char*)buffer, bufferSize, 0);
//    if(!error)
//       return NoError;
   return getLastError();
}

Net::Error Net::recv(NetSocket socket, U8 *buffer, S32 bufferSize, S32 *bytesRead)
{
   socket, buffer, bufferSize, bytesRead;
//    *bytesRead = ::recv(socket, (char*)buffer, bufferSize, 0);
//    if(*bytesRead == SOCKET_ERROR)
//       return getLastError();
   return NoError;
}

bool Net::compareAddresses(const NetAddress *a1, const NetAddress *a2)
{
   if(a1->type != a2->type)
      return false;
   if(*((U32 *)a1->netNum) != *((U32 *)a2->netNum))
      return false;
   if(a1->type == NetAddress::IPAddress)
      return true;
   for(S32 i = 0; i < 6; i++)
      if(a1->nodeNum[i] != a2->nodeNum[i])
         return false;
   return true;
}

bool Net::stringToAddress(const char *addressString, NetAddress *address)
{
   addressString, address;
/*
   if(dStrnicmp(addressString, "ipx:", 4))
   {
      // assume IP if it doesn't have ipx: at the front.
      
      if(!dStrnicmp(addressString, "ip:", 3))
         addressString += 3;  // eat off the ip:
      
      SOCKADDR_IN ipAddr;
      char remoteAddr[256];
      if(strlen(addressString) > 255)
         return false;
         
      dStrcpy(remoteAddr, addressString);
         
      char *portString = dStrchr(remoteAddr, ':');
      if(portString)
         *portString++ = 0;
      
      struct hostent *hp;
      
      if(!dStricmp(remoteAddr, "broadcast"))
         ipAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
      else
      {
         ipAddr.sin_addr.s_addr = inet_addr(remoteAddr);
         if(ipAddr.sin_addr.s_addr == INADDR_NONE)
         {
            if((hp = gethostbyname(remoteAddr)) == NULL)
               return false;
   	      else
   		      memcpy(&ipAddr.sin_addr.s_addr, hp->h_addr, sizeof(IN_ADDR));
         }
      }
      if(portString)
         ipAddr.sin_port = htons(dAtoi(portString));
      else
         ipAddr.sin_port = htons(defaultPort);
      ipAddr.sin_family = AF_INET;
      IPSocketToNetAddress(&ipAddr, address);
      return true;
   }
   else
   {
      S32 i;
      S32 port;

      address->type = NetAddress::IPXAddress;      
      for(i = 0; i < 6; i++)
         address->nodeNum[i] = 0xFF;
         
      // it's an IPX string
      addressString += 4;
      if(!dStricmp(addressString, "broadcast"))
      {
         address->port = defaultPort;
         return true;
      }
      else if(sscanf(addressString, "broadcast:%d", &port) == 1)
      {
         address->port = port;
         return true;
      }
      else
      {
         S32 nodeNum[6];
         S32 netNum[4];
         S32 count = dSscanf(addressString, "%2x%2x%2x%2x:%2x%2x%2x%2x%2x%2x:%d",
            &netNum[0], &netNum[1], &netNum[2], &netNum[3],
            &nodeNum[0], &nodeNum[1], &nodeNum[2], &nodeNum[3], &nodeNum[4], &nodeNum[5], 
            &port);
      
         if(count == 10)
         {
            port = defaultPort;
            count++;
         }
         if(count != 11)
            return false;

         for(i = 0; i < 6; i++)
            address->nodeNum[i] = nodeNum[i];
         for(i = 0; i < 4; i++)
            address->netNum[i] = netNum[i];
         address->port = port;
         return true;
      }
   }
   */
   return false;
}

void Net::addressToString(const NetAddress *address, char addressString[256])
{
   address, addressString;
/*
   if(address->type == NetAddress::IPAddress)
   {
      SOCKADDR_IN ipAddr;
      netToIPSocketAddress(address, &ipAddr);
      
      if(ipAddr.sin_addr.s_addr == htonl(INADDR_BROADCAST))
         dSprintf(addressString, 256, "IP:Broadcast:%d", ntohs(ipAddr.sin_port));
      else
         dSprintf(addressString, 256, "IP:%d.%d.%d.%d:%d", ipAddr.sin_addr.s_net,
            ipAddr.sin_addr.s_host, ipAddr.sin_addr.s_lh,
            ipAddr.sin_addr.s_impno, ntohs(ipAddr.sin_port));
   }
   else
   {
      dSprintf(addressString, 256, "IPX:%.2X%.2X%.2X%.2X:%.2X%.2X%.2X%.2X%.2X%.2X:%d",
         address->netNum[0], address->netNum[1], address->netNum[2], address->netNum[3], 
         address->nodeNum[0], address->nodeNum[1], address->nodeNum[2], address->nodeNum[3], address->nodeNum[4], address->nodeNum[5], 
         address->port);
   }
*/
}

Net::Error getLastError()
{
   return Net::UnknownError;
//    S32 err = WSAGetLastError();
//    switch(err)
//    {
//       case WSAEWOULDBLOCK:
//          return Net::WouldBlock;
//       default:
//          return Net::UnknownError;
//    }
}
