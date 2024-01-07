//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _NETDISPATCH_H_
#define _NETDISPATCH_H_

extern void dispatchInit();
extern void dispatchCheckTimeouts();

enum PacketType
{
   MasterServerGameTypesRequest  = 2,
   MasterServerGameTypesResponse = 4,
   MasterServerListRequest       = 6,
   MasterServerListResponse      = 8,
   GameMasterInfoRequest         = 10,
   GameMasterInfoResponse        = 12,
   GamePingRequest               = 14,
   GamePingResponse              = 16,
   GameInfoRequest               = 18,
   GameInfoResponse              = 20,
   GameHeartbeat                 = 22,

   ConnectChallengeRequest       = 26,
   ConnectChallengeReject        = 28,
   ConnectChallengeResponse      = 30,
   ConnectRequest                = 32,
   ConnectReject                 = 34,
   ConnectAccept                 = 36,
   Disconnect                    = 38,
};

const U32 CurrentProtocolVersion = 34;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  - if this is bumped (currently 34) then let BradH know so 
//    he can change some server query things.. then you can delete
//    this message.
const U32 MinRequiredProtocolVersion = 34;
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Connect protocol is as follows:
// Client issues ConnectChallengeRequest
// Server responds with ConnectChallengeReject or ConnectChallengeResponse
// Client issues ConnectRequest
// Server responds with with ConnectReject or ConnectAccept

bool isServerOnline();
bool isClientOnline();
extern void handleInfoPacket( const NetAddress* address, U8 packetType, BitStream* stream );

#endif
