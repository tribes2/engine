//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Connect protocol is as follows:
// Client issues ConnectChallengeRequest
// Server responds with ConnectChallengeReject or ConnectChallengeResponse
// Client issues ConnectRequest
// Server responds with with ConnectReject or ConnectAccept
//-----------------------------------------------------------------------------

#include "game/net/netDispatch.h"

#include "platform/platform.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "core/dnet.h"
#include "core/bitStream.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "sim/netConnection.h"
#include "sim/netObject.h"
#include "game/gameConnection.h"
#include "game/demoGame.h"
#include "game/banList.h"
#include "game/auth.h"
#include "game/net/serverQuery.h"

//-----------------------------------------------------------------------------

enum {
   MaxConnectArgs = 16,
   MaxPendingConnects = 20,
   PendingConnectTimeout = 7000, // 7 seconds

   ChallengeRetryCount = 4,
   ChallengeRetryTime = 2500, // 2.5 seconds

   ConnectRetryCount = 4,
   ConnectRetryTime = 2500,
   TimeoutCheckInterval = 1500, // check for timeouts every 1.5 secs
   MaxAuthInfoSize = 1024,
   MaxPasswordLength = 16,
};

const char *gServerConnectionName = "ServerConnection";
const char *gLocalClientConnectionName = "LocalClientConnection";
bool gAllowConnections = false;

void GameClientAdded(GameConnection *newClient, S32 argc, const char **argv);


//-----------------------------------------------------------------------------
// Connect protocol message formats
//-----------------------------------------------------------------------------

// ConnectChallengeRequest
// ProtocolVersion
// Certificate
// ClientConnectSequence
// Password

// ConnectChallengeResponse
// ProtocolVersion
// ServerConnectSequence
// ClientConnectSequence

// ConnectChallengeReject
// ClientConnectSequence
// Reason

// ConnectRequest
// ServerConnectSequence
// ClientConnectSequence
// ProtocolVersion
// console args

// ConnectAccept
// ServerConnectSequence
// ClientConnectSequence
// ProtocolVersion
// ClientID

// ConnectReject
// ServerConnectSequence
// ClientConnectSequence
// Reason

// Disconnect
// ServerConnectSequence
// ClientConnectSequence
// Reason string

// Connect request and server response structures

struct ConnectRequestStruct
{
   enum {
      NotConnecting,
      Challenging,
      Connecting,
   } state;

   U32 clientConnectSequence;
   U32 serverConnectSequence;
   U32 protocolVersion;
   U32 lastSendTime;
   U32 sendCount;
   NetAddress serverAddress;
   U32 challenge;
   bool authenticated;
   char password[MaxPasswordLength + 1];
   
   U32 argc;
   char *argv[MaxConnectArgs];
   U8 argBuffer[MaxPacketDataSize];
};

struct ConnectRequestPending
{
   // Certificate needs to be stored here.
   // V12: Challenge stuff was partially hacked out
   U32 challenge1;
    
   NetAddress sourceAddress;   
   U32 clientProtocolVersion;
   U32 clientConnectSequence;
   U32 serverConnectSequence;
   U32 lastRecvTime;
};

ConnectRequestStruct gConnectRequest;
ConnectRequestPending gPendingConnects[MaxPendingConnects];
U32 gNumPendingConnects;


//-----------------------------------------------------------------------------

void connectRequestRejected(const char *buffer)
{
   Con::executef(2, "onConnectRequestRejected", buffer);
}

void challengeRequestRejected(const char *buffer)
{
   Con::executef(2, "onChallengeRequestRejected", buffer);
}

void connectRequestTimedOut()
{
   gConnectRequest.state = ConnectRequestStruct::NotConnecting;
   Con::executef(1, "onConnectRequestTimedOut");
}

void connectionToServerTimedOut()
{
   Con::executef(1, "onConnectionToServerTimedOut");
}

void connectionToServerLost(const char* msg)
{
   Con::executef(2, "onConnectionToServerLost", msg);
}


//-----------------------------------------------------------------------------

bool isServerOnline()
{
   if(Game->isJournalReading())
   {
      U32 res;
      Game->journalRead(&res);
      return bool(res);
   }
   else
   {
      bool ret = false;
      // Authentication removed...
      ret = true;
      
      if(Game->isJournalWriting())
         Game->journalWrite(U32(ret));
      return ret;
   }
}

bool isClientOnline()
{
   if(Game->isJournalReading())
   {
      U32 res;
      Game->journalRead(&res);
      return bool(res);
   }
   else
   {
      bool ret = false;
      // Authentication removed...
      ret = true;

      if(Game->isJournalWriting())
         Game->journalWrite(U32(ret));
      return ret;
   }
}


//-----------------------------------------------------------------------------
// Connect request and server response functions
//-----------------------------------------------------------------------------

void rejectConnectChallengeRequest(const NetAddress *source, U32 clientConnectSequence, const char *reason);
void sendConnectChallengeResponse(U32 index);
void sendConnectChallengeRequest();
void sendConnectAccept(NetConnection *conn);
void sendConnectReject(const NetAddress *address,U32 clientSeq,U32 serverSeq,const char *reason);
void sendConnectRequest();

void handleConnectChallengeReject(const NetAddress *address, BitStream *stream);
void handleConnectChallengeResponse(const NetAddress *addr, BitStream *stream);
void handleConnectChallengeRequest(const NetAddress *source, BitStream *stream);
void handleConnectAccept(const NetAddress *addr, BitStream *stream);
void handleConnectReject(const NetAddress *address, BitStream *stream);
void handleConnectRequest(const NetAddress *address, BitStream *stream);


//-----------------------------------------------------------------------------
// Challenge Request
//-----------------------------------------------------------------------------

void sendConnectChallengeRequest()
{
   BitStream *out = BitStream::getPacketStream();
   out->write(U8(ConnectChallengeRequest));
   out->write(CurrentProtocolVersion);
   out->write(U32(AbstractClassRep::getClassCRC()));
   out->write(gConnectRequest.clientConnectSequence);
   out->writeString(gConnectRequest.password);

   // Authentication removed...
   // BUT flag still checked below -- need to write clear-flag.
   out->writeFlag(0);
   // need to add out->write for challenge...

   gConnectRequest.sendCount++;
   gConnectRequest.lastSendTime = Platform::getVirtualMilliseconds();

   Con::printf("Connect Challenge Request: %d", gConnectRequest.sendCount);

   BitStream::sendPacketStream(&gConnectRequest.serverAddress);
}

void handleConnectChallengeRequest(const NetAddress *source, BitStream *stream)
{
   if(!gAllowConnections)
      return;
      
   // reasons for rejection
   // password wrong
   // server full
   // bad protocol version
   // invalid certificate
   // don't meet v12/UID connect restrictions

   U32 protoVersion, classCRC;
   U32 clientConnectSequence;
   const char *rejectString = NULL;
   char joinPassword[MaxPasswordLength + 1];
   
   stream->read(&protoVersion);
   stream->read(&classCRC);
   stream->read(&clientConnectSequence);

   if(protoVersion < MinRequiredProtocolVersion)
   {
      rejectConnectChallengeRequest(source, clientConnectSequence, "CHR_PROTOCOL");
      return;
   }
   if(classCRC != AbstractClassRep::getClassCRC())
   {
      rejectConnectChallengeRequest(source, clientConnectSequence, "CHR_CLASSCRC");
      return;
   }
   
   if(protoVersion > CurrentProtocolVersion)
      protoVersion = CurrentProtocolVersion;
   
   // first time out any outstanding challenge requests:
   U32 time = Platform::getVirtualMilliseconds();
   
   for(U32 i = 0; i < gNumPendingConnects;)
   {
      ConnectRequestPending *p = gPendingConnects + i;
      
      // see if this is already in the list:
      if(Net::compareAddresses(source, &p->sourceAddress) && p->clientConnectSequence == clientConnectSequence)
      {
         // just up the time and ping back
         sendConnectChallengeResponse(i);
         return;
      }
      if(time > p->lastRecvTime + PendingConnectTimeout)
         *p = gPendingConnects[--gNumPendingConnects];
      else
         i++;
   }

   // Test the password:
   stream->readString(joinPassword);
   const char* password = Con::getVariable( "Pref::Server::Password" );
   if ( password[0] )
   {
      if ( dStrncmp( password, joinPassword, MaxPasswordLength ) != 0 )
      {
         // IMPORTANT! Do NOT change the message here, it is used by the game.
         rejectConnectChallengeRequest(source, clientConnectSequence, "CHR_PASSWORD");
         return;
      }
   }
   
   // V12: Challenge stuff was partially hacked out
   U32 challenge1 = 0;

   if(stream->readFlag())
   {
      U32 size = stream->readInt(16);
      if(size > MaxAuthInfoSize)
      {
         rejectConnectChallengeRequest(source, clientConnectSequence, "CHR_INVALID_CHALLENGE_PACKET");
         return;
      }

      // V12: Used to have WON stuff here...
   }

   U32 index;
   if(gNumPendingConnects == MaxPendingConnects)
      index = mRandI(0, MaxPendingConnects-1);
   else
      index = gNumPendingConnects++;

   ConnectRequestPending *p = gPendingConnects + index;
   p->challenge1 = challenge1;
   p->clientConnectSequence = clientConnectSequence;
   p->sourceAddress = *source;
   p->clientProtocolVersion = protoVersion;
   p->serverConnectSequence = Platform::getVirtualMilliseconds();
   
   Con::printf("Got challenge request");
   sendConnectChallengeResponse(index);
}


//-----------------------------------------------------------------------------
// Challenge Reject
//-----------------------------------------------------------------------------

void rejectConnectChallengeRequest(const NetAddress *source, U32 clientConnectSequence, const char *reason)
{
   BitStream *out = BitStream::getPacketStream();

   out->write(U8(ConnectChallengeReject));
   out->write(clientConnectSequence);
   out->writeString(reason);
   BitStream::sendPacketStream(source);
}

void handleConnectChallengeReject(const NetAddress *address, BitStream *stream)
{
   if(gConnectRequest.state != ConnectRequestStruct::Challenging)
      return;
   U32 clientConnectSequence;
   stream->read(&clientConnectSequence);
   
   if(!Net::compareAddresses(address, &gConnectRequest.serverAddress) ||
      clientConnectSequence != gConnectRequest.clientConnectSequence)
      return;
   char buffer[256];
   stream->readString(buffer);
   gConnectRequest.state = ConnectRequestStruct::NotConnecting;
   challengeRequestRejected(buffer);
}


//-----------------------------------------------------------------------------
// Challenge Response
//-----------------------------------------------------------------------------

void sendConnectChallengeResponse(U32 index)
{
   ConnectRequestPending *p = gPendingConnects + index;

   BitStream *out = BitStream::getPacketStream();
   
   out->write(U8(ConnectChallengeResponse));
   out->write(CurrentProtocolVersion);
   out->write(p->serverConnectSequence);
   out->write(p->clientConnectSequence);

   // Authentication removed...
   // BUT flag still checked below -- need to write clear-flag.
   out->writeFlag(0);
   // need to write out challenge somethingorother
   
   p->lastRecvTime = Platform::getVirtualMilliseconds();
   Con::printf("Sent challenge response");
   
   BitStream::sendPacketStream(&p->sourceAddress);
}

void handleConnectChallengeResponse(const NetAddress *addr, BitStream *stream)
{
   if(gConnectRequest.state != ConnectRequestStruct::Challenging)
      return;
   if(!Net::compareAddresses(&gConnectRequest.serverAddress, addr))
      return;
   U32 serverProtocol;
   U32 serverConnectSequence;
   U32 clientConnectSequence;
   
   stream->read(&serverProtocol);
   stream->read(&serverConnectSequence);
   stream->read(&clientConnectSequence);
   
   // must be an old connect request...
   if(clientConnectSequence != gConnectRequest.clientConnectSequence)
      return;

   // check if it's authenticated
   gConnectRequest.authenticated = stream->readFlag();
   if(gConnectRequest.authenticated)
   {
      // V12: Authentication removed...
      // V12: Used to have WON stuff here...

      // need to read in challenge somethingorother
   }

   gConnectRequest.state = ConnectRequestStruct::Connecting;
   gConnectRequest.serverConnectSequence = serverConnectSequence;
   gConnectRequest.sendCount = 0;
   gConnectRequest.protocolVersion = serverProtocol < CurrentProtocolVersion ? serverProtocol : CurrentProtocolVersion;
   Con::printf("Got challenge response");

   sendConnectRequest();
}


//-----------------------------------------------------------------------------
// Connection Request
//-----------------------------------------------------------------------------

void sendConnectRequest()
{
   gConnectRequest.sendCount++;
   gConnectRequest.lastSendTime = Platform::getVirtualMilliseconds();

   BitStream *out = BitStream::getPacketStream();
   
   out->write(U8(ConnectRequest));
   out->write(gConnectRequest.serverConnectSequence);
   out->write(gConnectRequest.clientConnectSequence);
   out->write(gConnectRequest.protocolVersion);
   if(out->writeFlag(gConnectRequest.authenticated))
   {
      // we shouldn't actually hit this at the moment.
      out->write(gConnectRequest.challenge);
   }
   out->write(gConnectRequest.argc);
   for(U32 i = 0; i < gConnectRequest.argc; i++)
      out->writeString(gConnectRequest.argv[i]);
   
   Con::printf("Sent connect request");
   BitStream::sendPacketStream(&gConnectRequest.serverAddress);
}

void handleConnectRequest(const NetAddress *address, BitStream *stream)
{
   if(!gAllowConnections)
      return;
      
   char addrString[256];
   
   Net::addressToString(address, addrString);
   Con::printf("Connection request from: %s", addrString);

   U32 serverConnectSequence;
   U32 clientConnectSequence;
   U32 protocolRequestVersion;
   
   stream->read(&serverConnectSequence);
   stream->read(&clientConnectSequence);
   stream->read(&protocolRequestVersion);

   const char *rejectString = NULL;
   
   if(protocolRequestVersion < MinRequiredProtocolVersion ||
      protocolRequestVersion > CurrentProtocolVersion)
      rejectString = "CR_INVALID_PROTOCOL_VERSION";
   
   U32 i;   
   for(i = 0; i < gNumPendingConnects; i++)
   {
      ConnectRequestPending *p = gPendingConnects + i;
      
      if(Net::compareAddresses(address, &p->sourceAddress) && 
         p->clientConnectSequence == clientConnectSequence &&
         p->serverConnectSequence == serverConnectSequence)
         break;
   }
   if(i == gNumPendingConnects) 
   {
      // if it's not in the pending list, check if
      // we're already connected to this guy:
      NetConnection *connect = NetConnection::lookup(address);
      if(connect)
      {
         U32 clientConSeq, serverConSeq;
         connect->getSequences(&clientConSeq, &serverConSeq);
         
         // we already accepted this connection:
         if(clientConSeq == clientConnectSequence &&
            serverConSeq == serverConnectSequence)
            sendConnectAccept(connect);
      }
      return;
   }
   U32 clientId = 0;

   // check the peer auth...
   if(stream->readFlag())
   {
      // V12: Authentication removed...
      // V12: Also used to have WON stuff here...
      
      // for now, since authentication SHOULD be OFF,
      // we post an error if we get into this block.
      rejectString = "CR_INVALID_CONNECT_PACKET";
      
      // however, the write code will write the challenge if flag set,
      // so we must have the code read it here or stream gets out of sync.
      U32 challengeResp;
      stream->read(&challengeResp);
   }
   
#if !defined(__linux__) && !defined(__OpenBSD__) // can't have this for now, - rjp
   if ( !rejectString )
   {   
      if ( gBanList.isBanned( 0 /* V12: Was Won net ID */, addrString ) )
         rejectString = "CR_YOUAREBANNED";
      else if ( Con::getIntVariable( "Server::PlayerCount" ) >= Con::getIntVariable( "Pref::Server::MaxPlayers" ) )
         rejectString = "CR_SERVERFULL";
   }
#endif

   // erase the request from the pending list
   gPendingConnects[i] = gPendingConnects[--gNumPendingConnects];
   gPendingConnects[gNumPendingConnects].challenge1 = NULL; // kill the byte buffer.
      
   const char *argv[MaxConnectArgs];
   char argbuf[MaxConnectArgs][256];
   U32 argc;
   
   stream->read(&argc);
   if(argc > MaxConnectArgs)
      rejectString = "CR_INVALID_CONNECT_PACKET";
   else
   {
      for(U32 i = 0; i < argc; i++)
      {
         argv[i] = argbuf[i];
         stream->readString(argbuf[i]);
      }
   }

   if(rejectString)
      sendConnectReject(address, clientConnectSequence,
         serverConnectSequence, rejectString);
   else
   {
      Con::printf("Accepting connect... CLIENT ID = %d", clientId);
      // let this guy into the game:
      GameConnection *conn = new GameConnection(true, false, true);

      conn->registerObject();
      conn->setProtocolVersion(protocolRequestVersion);
      conn->setNetAddress(address);
      conn->setNetworkConnection(true);

      conn->setSequences(clientConnectSequence, serverConnectSequence);
      conn->setConnectSequence(clientConnectSequence ^ serverConnectSequence);
      sendConnectAccept(conn);
      GameClientAdded(conn, argc, argv);
   }
}


//-----------------------------------------------------------------------------
// Connection Accept
//-----------------------------------------------------------------------------

void sendConnectAccept(NetConnection *conn)
{
   BitStream *out = BitStream::getPacketStream();
   
   out->write(U8(ConnectAccept));
   U32 clientConnectSeq, serverConnectSeq;
   U32 protocol;
   
   conn->getSequences(&clientConnectSeq, &serverConnectSeq);
   protocol = conn->getProtocolVersion();
   
   out->write(serverConnectSeq);
   out->write(clientConnectSeq);
   out->write(protocol);
   out->write(U32(conn->getId()));
   Con::printf("Sending connect accept: %d", conn->getId());
   
   BitStream::sendPacketStream(conn->getNetAddress());
}

void handleConnectAccept(const NetAddress *addr, BitStream *stream)
{
   if(gConnectRequest.state != ConnectRequestStruct::Connecting)
      return;
   if(!Net::compareAddresses(&gConnectRequest.serverAddress, addr))
      return;
   
   U32 serverConnectSequence, clientConnectSequence;
   U32 protocol, id;
   stream->read(&serverConnectSequence);
   stream->read(&clientConnectSequence);
   stream->read(&protocol);
   stream->read(&id);
   
   if(serverConnectSequence != gConnectRequest.serverConnectSequence ||
      clientConnectSequence != gConnectRequest.clientConnectSequence ||
      protocol > CurrentProtocolVersion ||
      protocol < MinRequiredProtocolVersion)
      return;
   
   // ok, we can connect up now:
   gConnectRequest.state = ConnectRequestStruct::NotConnecting;

   GameConnection *conn = new GameConnection(false, true, true);
   conn->registerObject();
   NetConnection::setServerConnection(conn);
   
   conn->setProtocolVersion(protocol);
   conn->setSequences(clientConnectSequence, serverConnectSequence);
   conn->setConnectSequence(clientConnectSequence ^ serverConnectSequence);
   conn->setNetAddress(addr);
   conn->setNetworkConnection(true);
   
   Sim::getRootGroup()->addObject(conn, gServerConnectionName);
   Con::executef(conn, 2, "onConnectionAccepted", Con::getIntArg(conn->getId()));
   Con::printf("Connection accepted - id %d  protocol %d", id, protocol);
}


//-----------------------------------------------------------------------------
// Connection Reject
//-----------------------------------------------------------------------------

void sendConnectReject(const NetAddress *address,U32 clientSeq,U32 serverSeq,const char *reason)
{
      BitStream *out = BitStream::getPacketStream();
      out->write(U8(ConnectReject));
      out->write(serverSeq);
      out->write(clientSeq);
      out->writeString(reason);
      BitStream::sendPacketStream(address);
}

void handleConnectReject(const NetAddress *address, BitStream *stream)
{
   if(gConnectRequest.state != ConnectRequestStruct::Connecting)
      return;
   U32 clientConnectSequence, serverConnectSequence;
   stream->read(&serverConnectSequence);
   stream->read(&clientConnectSequence);
   
   if(!Net::compareAddresses(address, &gConnectRequest.serverAddress) ||
      clientConnectSequence != gConnectRequest.clientConnectSequence ||
      serverConnectSequence != gConnectRequest.serverConnectSequence)
      return;
   char buffer[256];
   stream->readString(buffer);
   gConnectRequest.state = ConnectRequestStruct::NotConnecting;
   connectRequestRejected(buffer);
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void handleDisconnect(const NetAddress *addr, BitStream *stream)
{
   Con::printf("Got disconnect packet.");
   
   NetConnection *conn = NetConnection::lookup(addr);
   U32 serverConnectSequence, clientConnectSequence;
   stream->read(&serverConnectSequence);
   stream->read(&clientConnectSequence);
   char reason[256];
   stream->readString(reason);
   
   U32 cServerConnectSequence, cClientConnectSequence;
   if(!conn)
      return;
   conn->getSequences(&cClientConnectSequence, &cServerConnectSequence);
   if(cClientConnectSequence != clientConnectSequence ||
      cServerConnectSequence != serverConnectSequence)
      return;
   
   // ok, it's gone.
   if(conn->isServerConnection())
   {   
      Con::printf("Connection with server lost.");
      connectionToServerLost( reason );
   }
   else
   {
      Con::printf("Client %d disconnected.", conn->getId());
      ((GameConnection *) conn)->setDisconnectReason(reason);
   }
   conn->deleteObject();
}


//-----------------------------------------------------------------------------

void GameConnection::connectionError(const char *errorString)
{
   if(isServerConnection())
   {   
      Con::printf("Connection error: %s.", errorString);
      connectionToServerLost( errorString );
   }
   else
   {
      Con::printf("Client %d packet error: %s.", getId(), errorString);
      setDisconnectReason("Packet Error.");
   }
   deleteObject();
}


//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------

void dispatchInit()
{
   gConnectRequest.state = ConnectRequestStruct::NotConnecting;
   gNumPendingConnects = 0;
}

void clientNetProcess()
{
   NetConnection *con = NetConnection::getServerConnection();
   if (con)
      con->checkPacketSend();
}

void serverNetProcess()
{
   NetObject::collapseDirtyList(); // collapse all the mask bits...

   SimGroup *clientGroup = Sim::getClientGroup();
   for(SimGroup::iterator i = clientGroup->begin(); i != clientGroup->end(); i++)
   {
      NetConnection *con = (NetConnection *)(*i);
      if(con->isLocalConnection() || con->isNetworkConnection())
         con->checkPacketSend();   
   }
}

BitStream gPacketStream(NULL, 0);

void DemoGame::processPacketReceiveEvent(PacketReceiveEvent * prEvent)
{
   U32 dataSize = prEvent->size - PacketReceiveEventHeaderSize;
   gPacketStream.setBuffer(prEvent->data, dataSize);
   if(!(prEvent->data[0] & 0x01)) // it's a non-protocol packet of some sort...
   {
      U8 packetType;
      gPacketStream.read(&packetType);
      NetAddress *addr = &prEvent->sourceAddress;
      
      if(packetType <= GameHeartbeat)
         handleInfoPacket(addr, packetType, &gPacketStream);
      else
      {
         switch(packetType)
         {
            case ConnectChallengeRequest:
               handleConnectChallengeRequest(addr, &gPacketStream);
               break;
            case ConnectRequest:
               handleConnectRequest(addr, &gPacketStream);
               break;
            case ConnectChallengeResponse:
               handleConnectChallengeResponse(addr, &gPacketStream);
               break;
            case ConnectAccept:
               handleConnectAccept(addr, &gPacketStream);
               break;
            case Disconnect:
               handleDisconnect(addr, &gPacketStream);
               break;
            case ConnectReject:
               handleConnectReject(addr, &gPacketStream);
               break;
            case ConnectChallengeReject:
               handleConnectChallengeReject(addr, &gPacketStream);
               break;
         }
      }
   }
   else
   {
      // lookup the connection in the addressTable
      NetConnection *conn = NetConnection::lookup(&prEvent->sourceAddress);
      if(conn)
         conn->processRawPacket(&gPacketStream);
   }
}

void dispatchCheckTimeouts()
{
   static U32 lastTimeoutCheckTime = 0;
   U32 time = Platform::getVirtualMilliseconds();
   if(time > lastTimeoutCheckTime + TimeoutCheckInterval)
   {
      // check the connection state:
      if(gConnectRequest.state == ConnectRequestStruct::Challenging && 
         time > gConnectRequest.lastSendTime + ChallengeRetryTime)
      {
         if(gConnectRequest.sendCount > ChallengeRetryCount)
            connectRequestTimedOut();
         else
            sendConnectChallengeRequest();
      }
      else if(gConnectRequest.state == ConnectRequestStruct::Connecting &&
         time > gConnectRequest.lastSendTime + ConnectRetryTime)
      {
         if(gConnectRequest.sendCount > ConnectRetryCount)
            connectRequestTimedOut();         
         else
            sendConnectRequest();
      }

      lastTimeoutCheckTime = time;
      NetConnection *walk = NetConnection::getConnectionList();

      while(walk)
      {
         NetConnection *next = walk->getNext();
         if(walk->checkTimeout(time))
         {
            // this baddie timed out
            if(walk->isServerConnection())
            {
               Con::printf("Connection to server timed out");
               connectionToServerTimedOut();
            }
            else
            {
               Con::printf("Client %d timed out.", walk->getId());
               ((GameConnection *) walk)->setDisconnectReason("TimedOut");
            }
            walk->deleteObject();
         }
         walk = next;
      }
   }
}


//-----------------------------------------------------------------------------
// DNet external Game function declarations
//-----------------------------------------------------------------------------

void GameConnectionRejected(NetConnectionId id, BitStream *stream)
{
   GameConnection *conn = (GameConnection *) Sim::findObject(id);
   if(conn)
      conn->deleteObject();
   Con::printf("Connection rejected: %s", stream->getBuffer());
}


void GameConnectionEstablished(NetConnectionId id)
{
   Con::printf("Connection established %d", id);
}

void GameClientAdded(GameConnection *newClient, S32 argc, const char **argv)
{
   SimGroup *g = Sim::getClientGroup();
   g->addObject(newClient);
   const char *vargs[MaxConnectArgs + 2];
   vargs[0] = "onConnect";
   vargs[1] = NULL;
   if(argc > MaxConnectArgs)
      argc = MaxConnectArgs;
   for(S32 i = 0; i < argc; i++)
      vargs[i + 2] = argv[i];
      
   Con::execute(newClient, argc + 2, vargs);
}

//void GameConnectionRemoved(GameConnection *conn)
//{
//   if(conn->isNetworkConnection())
//   {
//      Con::printf("Issuing Disconnect packet.");
//      
//      // send a disconnect packet...
//      U32 serverConnectSequence, clientConnectSequence;
//      conn->getSequences(&clientConnectSequence, &serverConnectSequence);
//      
//      BitStream *out = BitStream::getPacketStream();
//      out->write(U8(Disconnect));
//      out->write(serverConnectSequence);
//      out->write(clientConnectSequence);
//      out->writeString("");
//   
//      BitStream::sendPacketStream(conn->getNetAddress());
//   }
//}


//-----------------------------------------------------------------------------
// Connection initiation console commands   
//-----------------------------------------------------------------------------

ConsoleFunction(allowConnections,void,2,2,"allowConnections(bool);")
{
   argc;
   if(!validateAuthenticatedServer())
      return;
   gAllowConnections = dAtob(argv[1]);   
}

ConsoleFunction(connect,void,2,17,"connect(addr,[password],[arg1..n]);")
{
   if(!validateAuthenticatedClient())
      return;
   gConnectRequest.state = ConnectRequestStruct::Challenging;
   gConnectRequest.clientConnectSequence = Platform::getVirtualMilliseconds();

   if(!Net::stringToAddress(argv[1], &gConnectRequest.serverAddress))
      return;

   argc -= 2;
   if (argc > MaxConnectArgs)
      argc = MaxConnectArgs;

   // First optional arg is the password
   if (argc > 1)  {
      dStrncpy( gConnectRequest.password, argv[2], MaxPasswordLength );
   }
   else
      gConnectRequest.password[0] = 0;

   // Get the rest of the optional args....
   U32 i, bufPos = 0;
   for(i = 0; i < argc; i++)
   {
      const char *str = argv[i + 3];
      U32 len = dStrlen(str);
      
      if(bufPos + len + 1 > MaxPacketDataSize)
         break;
      dStrcpy((char *) gConnectRequest.argBuffer + bufPos, str);
      gConnectRequest.argv[i] = (char *) gConnectRequest.argBuffer + bufPos;
      bufPos += len + 1;
   }
   gConnectRequest.argc = i;

   // Ship off the request...
   // Authentication removed...
   gConnectRequest.sendCount = 0;
   gConnectRequest.protocolVersion = CurrentProtocolVersion;
   sendConnectChallengeRequest();
}

ConsoleFunction(localConnect,void,1,16,"localConnect();")
{
   if(!validateAuthenticatedClient())
      return;

   GameConnection *clientConnection = new GameConnection(false, true, true);
   GameConnection *serverConnection = new GameConnection(true, false, true);

   clientConnection->registerObject();
   Sim::getRootGroup()->addObject(clientConnection, gServerConnectionName);
   
   serverConnection->registerObject();
   serverConnection->assignName(gLocalClientConnectionName);
   NetConnection::setServerConnection(clientConnection);
   NetConnection::setLocalClientConnection(serverConnection);

   clientConnection->setRemoteConnectionObjectId(serverConnection->getId());
   serverConnection->setRemoteConnectionObjectId(clientConnection->getId());

   clientConnection->setProtocolVersion(CurrentProtocolVersion);
   serverConnection->setProtocolVersion(CurrentProtocolVersion);
   clientConnection->setConnectSequence(0);
   serverConnection->setConnectSequence(0);
   
   GameClientAdded(serverConnection, argc - 1, argv + 1);
   Con::executef(clientConnection, 2, "onConnectionAccepted", Con::getIntArg(clientConnection->getId()));
}

ConsoleFunction(startRecord, void, 2, 2, "startRecord(fileName)")
{
   argc;
   NetConnection *conn = NetConnection::getServerConnection();
   if(!conn)
      return;
   char filename[1024];
   Con::expandScriptFilename(filename, sizeof(filename), argv[1]);
   conn->startDemoRecord(filename);
}

ConsoleFunction(stopRecord, void, 1, 1, "stopRecord();")
{
   argc; argv;
   NetConnection *conn = NetConnection::getServerConnection();
   if(!conn)
      return;
   conn->stopRecording();
}

ConsoleFunction(playDemo, void, 2, 2, "playDemo(recFileName)")
{
   argc;
   GameConnection *conn = new GameConnection(false, true, false);
   conn->registerObject();
   NetConnection::setServerConnection(conn);

   Sim::getRootGroup()->addObject(conn, gServerConnectionName);

   char filename[1024];
   Con::expandScriptFilename(filename, sizeof(filename), argv[1]);
   if(!conn->replayDemoRecord(filename))
   {
      Con::printf("Unable to open demo file %s.", filename);
      conn->deleteObject();
   }
}

