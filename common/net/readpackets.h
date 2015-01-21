
#ifndef READPACKETS_H
#define READPACKETS_H

#include "../platform.h"
#include "packets.h"

class NetConn;

void TranslatePacket(char* buffer, int bytes, bool checkprev, UDPsocket* sock, IPaddress* from);
void PacketSwitch(int type, char* buffer, int bytes, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadPlaceBlPacket(PlaceBlPacket* pbp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadAckPacket(AckPacket* ap, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDisconnectPacket(DisconnectPacket* dp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadNoConnPacket(NoConnectionPacket* ncp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadConnectPacket(ConnectPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDoneTurnPacket(DoneTurnPacket* dtp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadNetTurnPacket(NetTurnPacket* ntp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadJoinPacket(JoinPacket* jp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadAddSvPacket(AddSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadAddedSvPacket(AddedSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSvAddrPacket(SvAddrPacket* sap, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadGetSvListPacket(GetSvListPacket* gslp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSendNextHostPacket(SendNextHostPacket* snhp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadNoMoreHostsPacket(NoMoreHostsPacket* nmhp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSvInfoPacket(SvInfoPacket* sip, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadGetSvInfoPacket(GetSvInfoPacket* gsip, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadAddClPacket(AddClientPacket* acp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSelfClPacket(SelfClientPacket* scp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadSetClNamePacket(SetClNamePacket* scnp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadClientLeftPacket(ClientLeftPacket* clp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadClientRolePacket(ClientRolePacket* crp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadDoneJoinPacket(DoneJoinPacket* djp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadTooManyClPacket(TooManyClPacket* tmcp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadMapChangePacket(MapChangePacket* mcp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadChValPacket(ChValPacket* cvp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void ReadClDisconnectedPacket(ClDisconnectedPacket* cdp, NetConn* nc, IPaddress* from, UDPsocket* sock);
void OnAck_Connect(OldPacket* p, NetConn* nc);

#endif	//READPACKETS_H