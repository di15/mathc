

#include "readpackets.h"
#include "sendpackets.h"
#include "packets.h"
#include "net.h"
#include "netconn.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../utils.h"
#include "lockstep.h"
#include "../sim/build.h"
#include "../sim/simdef.h"
#include "../sim/simflow.h"
#include "../sys/unicode.h"
#include "client.h"
#include "../save/savemap.h"

#ifndef MATCHMAKER
#include "../gui/widgets/spez/svlist.h"
#include "../gui/widgets/spez/lobby.h"
#include "../../game/gui/ggui.h"

//not engine
#include "../../game/gui/chattext.h"
#endif

/*
What this function does is take a range of packet ack's (acknowledgment number for reliable UDP transmission)
and executes that range of buffered received packets. This is needed because packets might arrive out of order,
be missing some in between, and I execute them only after a whole range up to the latest ack has been received.

The out-of-order packets are stored in the g_recv vector.

Notice that there is preprocessor check if we are compiling this for the master server MATCHMAKER (because I'm
making this for a persistent, online world) or client. If server, there's extra parameters to match the packets
to the right client; we're only interested in processing the packet range for a certain client.

Each packet goes to the PacketSwitch function, that is like a switch-table that executes the right
packet-execution function based on the packet type ID. The switch-table could probably be turned into
an array of function pointers to improve performance, probably only slightly.

The function takes a time of log(O) to execute, because it has to search through all the buffered packets
several times to execute them in the right order. And before that, there's a check to see if we even have
the whole range of packets from the last "recvack" before calling this function.

I keep a "sendack" and "recvack" for each client, for sent packets and received packets. I only update the
recvack up to the latest one once a continuous range has been received, with no missing packets. Recvack
is thus the last executed received packet.
*/

void ParseRecieved(unsigned int first, unsigned int last, NetConn* nc)
{
	OldPacket* p;
	PacketHeader* header;
	unsigned int current = first;
	unsigned int afterlast = NextAck(last);

	do
	{
		for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		{
			p = &*i;
			header = (PacketHeader*)&p->buffer;

			if(header->ack != current)
				continue;

			//if(memcmp((void*)&p->addr, (void*)&nc->addr, sizeof(IPaddress)) != 0)
			if(!Same(&p->addr, &nc->addr))
				continue;

			PacketSwitch(header->type, p->buffer, p->len, nc, &p->addr, &g_sock);

			//p->freemem();
			i = g_recv.erase(i);
			current = NextAck(current);
			break;
		}
	} while(current != afterlast);
}

//do what needs to be done when we've recieved a packet range [first,last]
bool Recieved(unsigned short first, unsigned short last, NetConn* nc)
{
	OldPacket* p;
	PacketHeader* header;
	unsigned short current = first;
	unsigned short afterlast = NextAck(last);
	bool missed;

	do
	{
		missed = true;
		for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		{
			p = &*i;
			header = (PacketHeader*)&p->buffer;

			if(header->ack != current)
				continue;
			
			//if(memcmp((void*)&p->addr, (void*)&nc->addr, sizeof(IPaddress)) != 0)
			if(!Same(&p->addr, &nc->addr))
				continue;

			current = NextAck(current);
			missed = false;
			break;
		}

		if(missed)
			return false;
	} while(current != afterlast);

	return true;
}

void AddRecieved(char* buffer, int len, NetConn* nc)
{
	OldPacket p;
	p.buffer = new char[ len ];
	p.len = len;
	memcpy((void*)p.buffer, (void*)buffer, len);	
	memcpy((void*)&p.addr, (void*)&nc->addr, sizeof(IPaddress));

	g_recv.push_back(p);
}

void TranslatePacket(char* buffer, int bytes, bool checkprev, UDPsocket* sock, IPaddress* from)
{
	PacketHeader* header = (PacketHeader*)buffer;

	NetConn* nc = Match(from);
	if(nc)
	{
#ifdef NET_DEBUG
//#if 1
	//unsigned int ipaddr = SDL_SwapBE32(ip.host);
	//unsigned short port = SDL_SwapBE16(ip.port);

		g_log<<"upd last "<<SDL_SwapBE32(nc->addr.host)<<":"<<SDL_SwapBE16(nc->addr.port)<<" "<<DateTime()<<" msec"<<GetTickCount64()<<std::endl;
		g_log.flush();
#endif
		nc->lastrecv = GetTickCount64();
	}

	//bool bindaddr = true;

	switch(header->type)
	{
	case PACKET_ACKNOWLEDGMENT:
	case PACKET_CONNECT:
	case PACKET_DISCONNECT:
		checkprev = false;
		break;
	default:
		break;
	}

#ifndef MATCHMAKER
	//if(g_loadbytes > 0)
	{
		//char msg[128];
		//sprintf(msg, DateTime().c_str());
		//MessageBlock(msg, true);
	}
#endif

	//g_log<<"pack ack"<<header->ack<<" t"<<header->type<<" ::"<<SDL_SwapBE32(from->host)<<":"<<SDL_SwapBE16(from->port)<<" "<<DateTime()<<std::endl;

	if(checkprev && nc != NULL)
	{
		if(PastAck(header->ack, nc->recvack) || Recieved(header->ack, header->ack, nc))
		{
			Acknowledge(header->ack, nc, from, sock, buffer, bytes);
			//InfoMess("a", "pa");
			//g_log<<"past ack "<<header->ack<<" pa"<<PastAck(header->ack, nc->recvack)<<",r"<<Recieved(header->ack, header->ack, nc)<<std::endl;
#ifdef NET_DEBUG
//#if 1
			char msg[128];
			sprintf(msg, "\tpast ack%u t%d nc->recack=%u", (unsigned int)header->ack, header->type, (unsigned int)nc->recvack);
			g_log<<msg<<std::endl;
#endif
			//InfoMess("pa", msg);
			return;
		}

		unsigned short next = NextAck(nc->recvack);

		if(header->ack == next) {}  // Translate packet
		else  // More than +1 after recvack?
		{
			unsigned short last = PrevAck(header->ack);

			if(Recieved(next, last, nc))
				ParseRecieved(next, last, nc);  // Translate in order

			else
			{
				AddRecieved(buffer, bytes, nc);
				
				if(Recieved(next, last, nc))
					ParseRecieved(next, last, nc);  // Translate in order

				return;
			}
		}
	}

#if 0
	//ack SvInfoPacket's before disconnect...
	nc = Match(from);
	bool acked = false;
	if(header->type != PACKET_ACKNOWLEDGMENT && sock && nc && nc->handshook)
	{
#if 0
		if(!nc)
		{
			NetConn newnc;
			newnc.addr = *from;
			newnc.handshook = false;
			newnc.sendack = 0;
			newnc.lastrecv = GetTickCount64();
			g_conn.push_back(newnc);
			nc = &*g_conn.rbegin();
		}
#endif

		nc->recvack = header->ack;
		Acknowledge(header->ack, nc, from, sock, buffer, bytes);
		acked = true;
	}
#endif

	//We're getting an anonymous packet.
	//Maybe we've timed out and they still have a connection.
	//Tell them we don't have a connection.
	if(!nc && header->type != PACKET_CONNECT)
	{
		NoConnectionPacket ncp;
		ncp.header.type = PACKET_NOCONN;
		SendData((char*)&ncp, sizeof(NoConnectionPacket), from, false, true, NULL, &g_sock, 0, NULL);
		return;
	}

	PacketSwitch(header->type, buffer, bytes, nc, from, sock);
	
	//have to do this again because PacketSwitch might 
	//read a ConnectPacket, which adds new connections.
	//have to comment this out because connection might have 
	//been Disconnected(); and erased.
	//if(!nc)	
	nc = Match(from);

	//ack Connect packets after new NetConn added...
	if(header->type != PACKET_ACKNOWLEDGMENT && sock && nc /* && !acked */)
	{
#if 0
		if(!nc)
		{
			NetConn newnc;
			newnc.addr = *from;
			newnc.handshook = false;
			newnc.sendack = 0;
			newnc.lastrecv = GetTickCount64();
			g_conn.push_back(newnc);
			nc = &*g_conn.rbegin();
		}
#endif

		if(header->type != PACKET_CONNECT &&
			header->type != PACKET_DISCONNECT)
			nc->recvack = header->ack;

		Acknowledge(header->ack, nc, from, sock, buffer, bytes);
	}
}

void PacketSwitch(int type, char* buffer, int bytes, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef NET_DEBUG
	//unsigned int ipaddr = SDL_SwapBE32(ip.host);
	//unsigned short port = SDL_SwapBE16(ip.port);
	//warning: "from" might be NULL
	g_log<<"psw "<<((PacketHeader*)buffer)->type<<" ack"<<((PacketHeader*)buffer)->ack<<" from "<<(from ? SDL_SwapBE32(from->host) : 0)<<":"<<(from ? SDL_SwapBE16(from->port) : 0)<<std::endl;
	
	int nhs = 0;
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(ci->handshook)
			nhs++;

	g_log<<"g_conn.sz = "<<g_conn.size()<<" numhandshook="<<nhs<<std::endl;
	g_log.flush();
#endif

	switch(type)
	{
	case PACKET_ACKNOWLEDGMENT:
		ReadAckPacket((AckPacket*)buffer, nc, from, sock);
		break;
	case PACKET_CONNECT:
		ReadConnectPacket((ConnectPacket*)buffer, nc, from, sock);
		break;
	case PACKET_DISCONNECT:
		ReadDisconnectPacket((DisconnectPacket*)buffer, nc, from, sock);
		break;
	case PACKET_NOCONN:
		ReadNoConnPacket((NoConnectionPacket*)buffer, nc, from, sock);
		break;
	case PACKET_PLACEBL:
		ReadPlaceBlPacket((PlaceBlPacket*)buffer, nc, from, sock);
		break;
	case PACKET_NETTURN:
		ReadNetTurnPacket((NetTurnPacket*)buffer, nc, from, sock);
		break;
	case PACKET_DONETURN:
		ReadDoneTurnPacket((DoneTurnPacket*)buffer, nc, from, sock);
		break;
	case PACKET_JOIN:
		ReadJoinPacket((JoinPacket*)buffer, nc, from, sock);
		break;
	case PACKET_ADDSV:
		ReadAddSvPacket((AddSvPacket*)buffer, nc, from, sock);
		break;
	case PACKET_ADDEDSV:
		ReadAddedSvPacket((AddedSvPacket*)buffer, nc, from, sock);
		break;
	case PACKET_KEEPALIVE:
		//g_log<<"recv kap"<<std::endl;
		//don't need to do anything here. TranslatePacket already upped the nc->lastrecv.
		//ReadKeepAlivePacket((KeepAlivePacket*)buffer, nc, from, sock);
		break;
	case PACKET_GETSVLIST:
		ReadGetSvListPacket((GetSvListPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SVADDR:
		ReadSvAddrPacket((SvAddrPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SENDNEXTHOST:
		ReadSendNextHostPacket((SendNextHostPacket*)buffer, nc, from, sock);
		break;
	case PACKET_NOMOREHOSTS:
		ReadNoMoreHostsPacket((NoMoreHostsPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SVINFO:
		ReadSvInfoPacket((SvInfoPacket*)buffer, nc, from, sock);
		break;
	case PACKET_GETSVINFO:
		ReadGetSvInfoPacket((GetSvInfoPacket*)buffer, nc, from, sock);
		break;
	case PACKET_ADDCLIENT:
		ReadAddClPacket((AddClientPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SELFCLIENT:
		ReadSelfClPacket((SelfClientPacket*)buffer, nc, from, sock);
		break;
	case PACKET_SETCLNAME:
		ReadSetClNamePacket((SetClNamePacket*)buffer, nc, from, sock);
		break;
	case PACKET_CLIENTLEFT:
		ReadClientLeftPacket((ClientLeftPacket*)buffer, nc, from, sock);
		break;
	case PACKET_CLIENTROLE:
		ReadClientRolePacket((ClientRolePacket*)buffer, nc, from, sock);
		break;
	case PACKET_DONEJOIN:
		ReadDoneJoinPacket((DoneJoinPacket*)buffer, nc, from, sock);
		break;
	case PACKET_TOOMANYCL:
		ReadTooManyClPacket((TooManyClPacket*)buffer, nc, from, sock);
		break;
	case PACKET_MAPCHANGE:
		ReadMapChangePacket((MapChangePacket*)buffer, nc, from, sock);
		break;
	case PACKET_CHVAL:
		ReadChValPacket((ChValPacket*)buffer, nc, from, sock);
		break;
	default:
		break;
	}
}

void ReadAckPacket(AckPacket* ap, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	OldPacket* p;
	PacketHeader* header;
	
#ifndef MATCHMAKER
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
#endif

	for(auto i=g_outgo.begin(); i!=g_outgo.end(); i++)
	{
		p = &*i;
		header = (PacketHeader*)p->buffer;
		if(header->ack == ap->header.ack &&
			//memcmp((void*)&p->addr, (void*)from, sizeof(IPaddress)) == 0
			Same(&p->addr, from))
		{
			if(!nc)
				nc = Match(from);

			if(nc)
			{
				//nc->ping = ((float)(GetTickCount64() - i->first) + nc->ping) / 2.0f;
				nc->ping = (float)(GetTickCount64() - i->first);

#ifndef MATCHMAKER
				//update sv listing info
				if(nc->ishostinfo)
				{
					for(auto sit=v->m_svlist.begin(); sit!=v->m_svlist.end(); sit++)
					{
						if(!Same(&sit->addr, from))
							continue;

						sit->ping = (int)nc->ping;
						char pingstr[16];
						sprintf(pingstr, "%d", (int)nc->ping);
						sit->pingrt = RichText(pingstr);
					}
				}
#endif

#ifdef NET_DEBUG
				g_log<<"new ping for "<<nc->addr.host<<": "<<nc->ping<<std::endl;
				g_log.flush();
#endif
			}

			if(p->onackfunc)
				p->onackfunc(p, nc);

			//p->freemem();
			i = g_outgo.erase(i);
#if 0
			g_log<<"left to ack "<<g_outgo.size()<<std::endl;
			g_log.flush();
#endif
			return;
		}
	}
}

void ReadDoneTurnPacket(DoneTurnPacket* dtp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	//TO DO check for player==nc->player

	if(!nc)
		nc = Match(from);

	if(!nc)
		return;

	//if(nc->ctype != CONN_CLIENT)
	//	return;
	if(!nc->isclient)
		return;

	//if(dtp->player != nc->player)
	//	return;

	if(dtp->player < 0 || dtp->player >= PLAYERS)
		return;

	Player* py = &g_player[dtp->player];

	//we added 1 because the client is on the verge of the next turn
	//adding 1 gives the next turn
	py->curnetfr = dtp->fornetfr + 1;

#if 0
	char msg[128];
	sprintf(msg, "read done turn packet netf%u", py->curnetfr);
	InfoMess("r", msg);
#endif
#endif
}

void ReadNetTurnPacket(NetTurnPacket* ntp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	//InfoMess("a", "rntp");

	if(g_netmode == NETM_CLIENT)
	{
#if 0
		char msg[128];
		sprintf(msg, "recv netturn for netfr%u load=%u", ntp->fornetfr, (unsigned int)ntp->loadsz);
		//InfoMess("r", msg);
		//if(ntp->loadsz > 0)
		{
			FILE* fp = fopen("rntp.txt", "wb");
			fwrite(msg, strlen(msg)+1, 1, fp);
			fclose(fp);
		}
#endif

		//for next turn?
		if(ntp->fornetfr != (g_netframe / NETTURN + 1) * NETTURN)
		{
			//something wrong, did we miss a turn?
			if(PastFr(g_netframe, ntp->fornetfr))
				ErrMess("Error", "Turn missed?");
			else
				ErrMess("Error", "Future turn?");

			return;	//if not, discard
		}

		//Cl_StepTurn(ntp);
		AppendCmds(&g_nextcmdq, ntp);
		g_canturn = true;

#if 0
		//char msg[128];
		sprintf(msg, "passed for netfr%u", ntp->fornetfr);
		InfoMess("Error", msg);
#endif
	}
	else if(g_netmode == NETM_HOST)	//cl can't send batch of commands packet
	{
		//AppendCmds(&g_localcmd, ntp);
	}
#endif
}

void ReadPlaceBlPacket(PlaceBlPacket* pbp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(!from)
	{
		if(CheckCanPlace(pbp->btype, pbp->tpos))
			PlaceBl(pbp->btype, pbp->tpos, false, pbp->player, NULL);

		return;
	}

	if(g_netmode == NETM_HOST)
	{
#if 0
		PlaceBlPacket* pbp2 = (PlaceBlPacket*)malloc(sizeof(PlaceBlPacket));
		memcpy(pbp2, pbp, sizeof(PlaceBlPacket));
		g_localcmd.push_back((PacketHeader*)pbp2);
#else
		AppendCmd(&g_localcmd, (PacketHeader*)pbp, sizeof(PlaceBlPacket));
#endif
	}
#if 0	//no longer required, exec'd with NULL "from" addr
#if 1	//cl can only exec command batch packets, but it will then call this func
	else if(g_netmode == NETM_CLIENT)
	{
		//InfoMess("polk", "p");
#ifndef MATCHMAKER
		if(CheckCanPlace(pbp->btype, pbp->tpos))
			PlaceBl(pbp->btype, pbp->tpos, false, pbp->player, NULL);
#endif
	}
#endif
	else if(g_netmode == NETM_SINGLE)
	{
#if 0
		char msg[128];
		sprintf(msg, "%d at %d,%d", pbp->btype, pbp->tpos.x, pbp->tpos.y);
		InfoMess("polk", msg);
#endif
#ifndef MATCHMAKER
		if(CheckCanPlace(pbp->btype, pbp->tpos))
			PlaceBl(pbp->btype, pbp->tpos, false, pbp->player, NULL);
#endif
	}
#endif
#endif
}

//If we got a "no connection" packet while attempting to send
//data to a connection we have, reconnect to them, setting their
//recvack to the one before our current sendack (?)
//Will that work? If we have outgoing packets. It should be the earliest
//outgoing packet sendack. But what if one ahead has been ack'd? 
//Recvack will still be at the first one. If they have a connection (or buffered packet).
void ReadNoConnPacket(NoConnectionPacket* ncp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(!nc)
		nc = Match(from);

	if(!nc)
		return;	//Not our problem

	//Reconnect(from);	//Might want to encapsulate it in a function later

	unsigned short early;	//earliest outgoing ack
	bool earlyset = false;

	for(auto oit=g_outgo.begin(); oit!=g_outgo.end(); oit++)
	{
		if(!Same(&oit->addr, from))
			continue;

		PacketHeader* ph = (PacketHeader*)oit->buffer;

		if(!earlyset && PastAck(ph->ack, early))
		{
			earlyset = true;
			early = ph->ack;
		}
	}

	if(!earlyset)
		early = 0;	//Strange, should have outgoing

	ConnectPacket scp;
	scp.header.type = PACKET_CONNECT;
	scp.reply = false;
	scp.header.ack = early - 1;	//Must be -1; next packet to be read is recvack+1.
	//Do we need OnAck_Connect?
	SendData((char*)&scp, sizeof(ConnectPacket), from, false, true, nc, &g_sock, 0, NULL);
}

void ReadClDisconnectedPacket(ClDisconnectedPacket* cdp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER

#endif
}

void ReadDisconnectPacket(DisconnectPacket* dp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	//char msg[128];
	//sprintf(msg, "dis %u:%u", from->host, (unsigned int)from->port);
	//InfoMess("d", msg);
	
	if(!nc)
		nc = Match(from);

	if(!nc)
		return;

	//if(nc->ctype == CONN_HOST)
	if(nc->isourhost)
		g_svconn = NULL;
	//else if(nc->ctype == CONN_MATCHER)
	if(nc->ismatcher)
		g_mmconn = NULL;

	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(&*ci == nc)
		{
			ci->closed = true;
			FlushPrev(&ci->addr);

			if(dp->reply)
			{
				//FlushPrev(&ci->addr);
				//g_conn.erase(ci);
			}

			break;
		}

	//TODO get rid of client, inform players
	if(nc->client >= 0)
	{
		Client* c = &g_client[nc->client];
		c->nc = NULL;
		c->on = false;
	}

	nc->client = -1;
}

void ReadChValPacket(ChValPacket* cvp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(!from)
	{
		Building* b = NULL;
		Player* py = &g_player[cvp->player];
		CdTile* cdtile = NULL;
		Resource* r = NULL;
		BlType* bt = NULL;
		CdType* ct = NULL;
		UType* ut = NULL;

		RichText chat;

		chat = chat + py->name;
		char add[128];

		switch(cvp->chtype)
		{
		//TODO verify that player owns this
		case CHVAL_BLPRICE:
			b = &g_building[cvp->bi];
			b->price[cvp->res] = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			sprintf(add, " set price ");
			chat = chat + RichText(add);
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " at %s to ", bt->name);
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			//TODO messages for the rest
			break;
		case CHVAL_BLWAGE:
			b = &g_building[cvp->bi];
			b->opwage = cvp->value;
#if 1
			//r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			sprintf(add, " set wage");
			chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " at %s to ", bt->name);
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_CSTWAGE:
			b = &g_building[cvp->bi];
			b->conwage = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			sprintf(add, " set con. wage");
			chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " at %s to ", bt->name);
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_TRPRICE:
			py->transpcost = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			sprintf(add, " set transp. price");
			chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " to ");
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_TRWAGE:
			py->truckwage = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			sprintf(add, " set driver wage");
			chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " to ");
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_PRODLEV:
			b = &g_building[cvp->bi];
			b->prodlevel = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			sprintf(add, " set prod. level");
			chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " at %s to ", bt->name);
			chat = chat + RichText(add);
			//r = &g_resource[RES_DOLLARS];
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_CDWAGE:
			//TODO verify that player owns this
			cdtile = GetCd(cvp->cdtype, cvp->x, cvp->y, false);
			cdtile->conwage = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			ct = &g_cdtype[cvp->cdtype];
			sprintf(add, " set con. wage");
			chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " at %s to ", ct->name);
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		case CHVAL_MANPRICE:
			b = &g_building[cvp->bi];
			b->manufprc[cvp->utype] = cvp->value;
#if 1
			r = &g_resource[cvp->res];
			bt = &g_bltype[b->type];
			sprintf(add, " set manuf. price");
			//chat = chat + RichText(add);
			//chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			//chat = chat + RichText(r->name.c_str());
			sprintf(add, " of %s to ", ut->name);
			chat = chat + RichText(add);
			r = &g_resource[RES_DOLLARS];
			chat = chat + RichText(RichPart(RICHTEXT_ICON, r->icon));
			sprintf(add, "%d", cvp->value);
			chat = chat + RichText(add);
#endif
			break;
		default:
			break;
		};

		AddChat(&chat);

		return;
	}

	if(g_netmode == NETM_HOST)
	{
		if(!nc)
			return;
		if(!nc->isclient)
			return;
		if(nc->client < 0)
			return;
		
		Client* c = &g_client[nc->client];

		if(cvp->player != c->player)
			return;

		AppendCmd(&g_localcmd, (PacketHeader*)cvp, sizeof(ChValPacket));
		//TODO change to LockCmd?
	}
#endif
}

void ReadAddSvPacket(AddSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef MATCHMAKER
	if(!nc)
		return;

	nc->ishostinfo = true;
	//nc->svinfo = asp->svinfo;
	//nc->svinfo.addr = *from;

#if 0
	g_log<<"addsv "<<nc->addr.port<<std::endl;
	g_log.flush();
#endif

	AddedSvPacket asp2;
	asp2.header.type = PACKET_ADDEDSV;
	SendData((char*)&asp2, sizeof(AddedSvPacket), from, true, false, nc, &g_sock, 0);
#else
#endif
}

//cl reads a sv addr of game host
void ReadSvAddrPacket(SvAddrPacket* sap, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
#if 0
	//temp
	if(g_netmode == NETM_HOST)
		return;

	//InfoMess("g", "gsl");
	
	//temp
	if(g_svconn)
		return;

	Click_NewGame();
	
	//Connect(&sap->addr, false, true, false, true);
	Connect("localhost", PORT, false, true, false, true);
	BegSess();
#else
	if(!nc->ismatcher)
		return;

	//we can get next host now
	g_reqdnexthost = false;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
	auto sl = &v->m_svlist;

	bool found = false;

	for(auto sit=sl->begin(); sit!=sl->end(); sit++)
	{
		//if have, update info
		if(!Same(&sit->addr, &sap->addr))
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		//InfoMess("rsap", "rsap");

		SvList::SvInfo svinfo;
		svinfo.addr = sap->addr;
		svinfo.mapnamert = RichText("???");
		svinfo.name = RichText("???");
		svinfo.pingrt = RichText("???");
		sl->push_back(svinfo);
		NetConn* havenc = Match(&sap->addr);
		Connect(&sap->addr, false, false, false, true);
		if(havenc && havenc->handshook)
		{
			GetSvInfoPacket gsip;
			gsip.header.type = PACKET_GETSVINFO;
			SendData((char*)&gsip, sizeof(GetSvInfoPacket), &havenc->addr, true, false, nc, &g_sock, 0, NULL);
		}
	}
#endif
#endif
}

void ReadGetSvInfoPacket(GetSvInfoPacket* gsip, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode != NETM_HOST)
		return;

	SvInfoPacket sip;
	sip.header.type = PACKET_SVINFO;
	//sip.svinfo.addr = {0};	//?
	strcpy(sip.svinfo.mapname, g_mapname);
	strcpy(sip.svinfo.svname, g_svname);
	sip.svinfo.mapname[MAPNAME_LEN] = 0;
	sip.svinfo.svname[SVNAME_LEN] = 0;
	//sip.svinfo.nplayers = g_nplayers;	//TO DO
	SendData((char*)&sip, sizeof(SvInfoPacket), from, true, false, nc, &g_sock, 0, NULL);
#endif
}

void ReadSvInfoPacket(SvInfoPacket* sip, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(!nc)
		return;

	if(!nc->ishostinfo)
		return;

	//InfoMess("rgsip", "rgsip");
	//g_log<<"rgsip ack"<<sip->header.ack<<std::endl;

	//check if we already have this addr
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
	auto sl = &v->m_svlist;

	for(auto sit=sl->begin(); sit!=sl->end(); sit++)
	{
		//if have, update info
		if(!Same(&sit->addr, from))	//self-address might be different on LAN
		//if(!Same(&sit->addr, &sip->svinfo.addr))	//how do we know self-address?
			continue;

		sit->replied = true;
		
		sip->svinfo.mapname[MAPNAME_LEN] = 0;
		sip->svinfo.svname[SVNAME_LEN] = 0;

		sit->nplayers = sip->svinfo.nplayers;

		char pingstr[16];
		sprintf(pingstr, "%d", (int)nc->ping);
		sit->pingrt = RichText(pingstr);

#if 1
		//yes unicode?
		//unsigned int* mapnameuni = ToUTF32((const unsigned char*)sip->svinfo.mapname, strlen(sip->svinfo.mapname));
		//unsigned int* svnameuni = ToUTF32((const unsigned char*)sip->svinfo.svname, strlen(sip->svinfo.svname));
		unsigned int* mapnameuni = ToUTF32((const unsigned char*)sip->svinfo.mapname);
		unsigned int* svnameuni = ToUTF32((const unsigned char*)sip->svinfo.svname);
		sit->mapnamert = RichText(UString(mapnameuni));
		sit->name = RichText(UString(svnameuni));
		sit->name = ParseTags(sit->name, NULL);
		delete [] mapnameuni;
		delete [] svnameuni;
#else
		//no unicode?
		//sit->mapnamert = sip->svinfo.mapname;
		//sit->name = sip->svinfo.svname;
#endif

		//break;	//multiple copies?
	}

	//return;	//temp

	//if it's only a hostinfo and nc isn't closed
	if(!nc->closed && !nc->isourhost && !nc->isclient && !nc->ismatcher)
		Disconnect(nc);
#endif
}

void ReadGetSvListPacket(GetSvListPacket* gslp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef MATCHMAKER
	if(!nc)
		return;

	//if(!nc->isclient)
	//	return;

	nc->svlistoff = 0;

	//g_log<<"req sv l"<<std::endl;
	//g_log.flush();

	ReadSendNextHostPacket(NULL, nc, from, sock);

	//g_log<<"/req sv l"<<std::endl;
	//g_log.flush();
#endif
}

void ReadSendNextHostPacket(SendNextHostPacket* snhp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifdef MATCHMAKER
	//g_log<<"sendnexthost1 g_conn.size()="<<g_conn.size()<<std::endl;
	//g_log.flush();

	if(!nc)
		nc = Match(from);

	//g_log<<"sendnexthost2"<<std::endl;
	//g_log.flush();

	if(!nc)
		return;

	//g_log<<"sendnexthost3"<<std::endl;
	//g_log.flush();

	int hin = -1;
	auto hit = g_conn.begin();
	while(hit != g_conn.end())
	{
		if(hit->ishostinfo)
		{
			hin++;
		
			//g_log<<"hin "<<hin<<" svlistoff "<<nc->svlistoff<<std::endl;

			if(hin == nc->svlistoff)
			{
				SvAddrPacket sap;
				sap.header.type = PACKET_SVADDR;
				sap.addr = hit->addr;
				SendData((char*)&sap, sizeof(SvAddrPacket), from, true, false, nc, &g_sock, 0);
				nc->svlistoff++;
				return;
			}
		}

		hit++;
	}

	NoMoreHostsPacket nmhp;
	nmhp.header.type = PACKET_NOMOREHOSTS;
	SendData((char*)&nmhp, sizeof(NoMoreHostsPacket), from, true, false, nc, &g_sock, 0);
#endif
}

void ReadNoMoreHostsPacket(NoMoreHostsPacket* nmhp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	g_reqsvlist = false;
#endif
}

void ReadAddedSvPacket(AddedSvPacket* asp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(!nc)
		return;

	if(!nc->ismatcher)
		return;

	if(g_netmode != NETM_HOST)
		return;

	g_sentsvinfo = true;
	//InfoMess("added", "added sv");
#endif
}

void ReadJoinPacket(JoinPacket* jp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_HOST)
	{
		if(!nc)
			return;

		nc->isclient = true;
		
		//InfoMess("conn", "read join");
		//TO DO send join info map etc.

		RichText name;
		unsigned int* uname = ToUTF32((unsigned char*)jp->name);
		name.m_part.push_back(UString(uname));
		delete [] uname;
		int joinci;

		//InfoMess(" ? mcp", " ? mcp");
		
		//unsigned int ipaddr = SDL_SwapBE32(ip.host);
		//unsigned short port = SDL_SwapBE16(ip.port);
#if 1
		char ipname[128];
		sprintf(ipname, "%u:%u", SDL_SwapBE32(nc->addr.host), (unsigned int)SDL_SwapBE16(nc->addr.port));
		name = RichText(ipname);
#endif

		if(!AddClient(nc, name, &joinci))
		{
			TooManyClPacket tmcp;
			tmcp.header.type = PACKET_TOOMANYCL;
			SendData((char*)&tmcp, sizeof(TooManyClPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
			return;
		}

		int msdelay = RESEND_DELAY;
		
		MapChangePacket mcp;
		mcp.header.type = PACKET_MAPCHANGE;
		strcpy(mcp.map, g_mapname);
		SendData((char*)&mcp, sizeof(MapChangePacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
		msdelay += RESEND_DELAY;

		AddClientPacket acp;
		acp.header.type = PACKET_ADDCLIENT;
		//acp.client = joinci;
		//strcpy(acp.name, jp->name);
		//acp.player = -1;
		for(int i=0; i<CLIENTS; i++)
		{
			Client* c = &g_client[i];

			if(!c->on)
				continue;

			RichText* cname = &c->name;
			acp.client = i;

			if(cname->m_part.size() > 0)
			{
				unsigned char* name8 = ToUTF8(cname->m_part.begin()->m_text.m_data);
				name8[PYNAME_LEN] = 0;
				strcpy(acp.name, (char*)name8);
				delete [] name8;
			}
			else
				strcpy(acp.name, "");

			acp.player = c->player;

			SendData((char*)&acp, sizeof(AddClientPacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
			msdelay += RESEND_DELAY;

			if(i == joinci)
			{
				SendAll((char*)&acp, sizeof(AddClientPacket), true, false, &nc->addr);
			}
		}

		SelfClientPacket scp;
		scp.header.type = PACKET_SELFCLIENT;
		scp.client = joinci;
		SendData((char*)&scp, sizeof(SelfClientPacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
		msdelay += RESEND_DELAY;

		DoneJoinPacket djp;
		djp.header.type = PACKET_DONEJOIN;
		SendData((char*)&djp, sizeof(DoneJoinPacket), &nc->addr, true, false, nc, &g_sock, msdelay, NULL);
		msdelay += RESEND_DELAY;
	}
#endif
}

void ReadAddClPacket(AddClientPacket* acp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		//g_log<<"acp"<<std::endl;
		unsigned int* uname = ToUTF32((unsigned char*)acp->name);
		int addci;
		AddClient(nc, RichText(UString(uname)), &addci);
		delete [] uname;
		Client* c = &g_client[addci];
		c->player = acp->player;

		if(acp->player >= 0)
		{
			Player* py = &g_player[acp->player];
			py->client = addci;
		}
	}
	else if(g_netmode == NETM_HOST)
	{
	}
#endif
}

void ReadSelfClPacket(SelfClientPacket* scp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
#ifndef MATCHMAKER
		//g_log<<"scp"<<std::endl;
		g_localC = scp->client;
		Client* c = &g_client[scp->client];
		g_localP = c->player;
#endif
	}
}

void ReadSetClNamePacket(SetClNamePacket* scnp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		Client* c = &g_client[scnp->client];
		unsigned int* uname = ToUTF32((unsigned char*)scnp->name);
		c->name = RichText(UString(uname));
		delete [] uname;
	}
	else if(g_netmode == NETM_HOST && nc && nc->isclient)
	{
		Client* c = &g_client[scnp->client];
		unsigned int* uname = ToUTF32((unsigned char*)scnp->name);
		c->name = RichText(UString(uname));
		delete [] uname;

		SetClNamePacket scnp2;
		memcpy(&scnp2, scnp, sizeof(SetClNamePacket));
		SendAll((char*)&scnp2, sizeof(SetClNamePacket), true, false, &nc->addr);
	}
#endif
}

void ReadClientLeftPacket(ClientLeftPacket* clp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		Client* c = &g_client[clp->client];
		c->on = false;
		c->name = RichText("Player");

		if(c->player >= 0)
		{
			Player* py = &g_player[c->player];
			py->client = -1;
			c->player = -1;
		}
	}
	else if(g_netmode == NETM_HOST && nc && nc->isclient)
	{
		Client* c = &g_client[clp->client];
		c->on = false;
		c->name = RichText("Player");

		if(c->player >= 0)
		{
			Player* py = &g_player[c->player];
			py->client = -1;
			c->player = -1;
		}

		ClientLeftPacket clp2;
		memcpy(&clp2, clp, sizeof(ClientLeftPacket));
		SendAll((char*)&clp2, sizeof(ClientLeftPacket), true, false, &nc->addr);
	}
#endif
}

void ReadClientRolePacket(ClientRolePacket* crp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		//g_log<<"crp"<<std::endl;
		Client* c = &g_client[crp->client];
		c->on = false;

		if(c->player >= 0)
		{
			Player* py = &g_player[c->player];
			py->client = -1;
			c->player = -1;
		}

		c->player = crp->player;
		Player* py = &g_player[crp->player];
		py->client = crp->client;
	}
	else if(g_netmode == NETM_HOST && nc && nc->isclient)
	{
		Client* c = &g_client[crp->client];
		c->on = false;

		if(c->player >= 0)
		{
			Player* py = &g_player[c->player];
			py->client = -1;
			c->player = -1;
		}

		//TO DO reject if another client controls crp->player

		c->player = crp->player;
		Player* py = &g_player[crp->player];
		py->client = crp->client;

		ClientRolePacket crp2;
		memcpy(&crp2, crp, sizeof(ClientRolePacket));
		SendAll((char*)&crp2, sizeof(ClientRolePacket), true, false, &nc->addr);
	}
#endif
}

void ReadDoneJoinPacket(DoneJoinPacket* djp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
#ifndef MATCHMAKER
		//g_log<<"djp"<<std::endl;
		GUI* gui = &g_gui;
		gui->closeall();
		gui->open("lobby");
		((Lobby*)gui->get("lobby"))->regen();
#endif
	}
}

void ReadTooManyClPacket(TooManyClPacket* tmcp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
#ifndef MATCHMAKER

		Disconnect(nc);
		GUI* gui = &g_gui;
#if 0
		gui->closeall();
		gui->open("menu");
#else
		ViewLayer* v = (ViewLayer*)gui->get("join");
		Text* status = (Text*)v->get("status");
		status->m_text = RichText("Cannot join. Server is full.");
#endif

		//TO DO info message

#endif
	}
}

void ReadMapChangePacket(MapChangePacket* mcp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_CLIENT && nc && nc->isourhost)
	{
		//g_log<<"mcp"<<std::endl;
		//g_log.flush();
		strcpy(g_mapname, mcp->map);
		FreeMap();
		//TO DO load, download etc. check sum
	}
#endif
}

//connect packet won't be discarded if it's a (reply's or otherwise) copy, so this function needs to be durable.
//i.e., no repeat action if cp->header.ack is PastAck(...);.
void ReadConnectPacket(ConnectPacket* cp, NetConn* nc, IPaddress* from, UDPsocket* sock)
{
#if 0
	char msg[128];
	sprintf(msg, "\tcon %u:%u reply=%d", from->host, (unsigned int)from->port, (int)cp->reply);
	g_log<<msg<<std::endl;
	//InfoMess("d", msg);
#endif

	//InfoMess("d", "rc");

#if 1	//flush all previous incoming and outgoing packets from this addr
	//FlushPrev(from);
	//actually, that might be bad, if we've got a game host we're playing in,
	//and we request a sv list and get connected to this same host to get its
	//game info. actually, just need to be make sure Connect(); doesn't send 
	//another ConnectPacket if we've already handshook.
#endif

	if(!nc)
	{
		nc = Match(from);

		if(!nc)
		{
			
	//char msg[128];
	//sprintf(msg, "con %u:%u", from->host, (unsigned int)from->port);
	//InfoMess("d", msg);

			NetConn newnc;
			//newnc.ctype = CONN_CLIENT;
			//temporary - must get some packet telling us this is client that wants to join TO DO
			//newnc.isclient = true;
			newnc.addr = *from;
			newnc.handshook = true;
			newnc.recvack = cp->header.ack;
			newnc.sendack = 0;
			newnc.lastrecv = GetTickCount64();
			g_conn.push_back(newnc);
			nc = &*g_conn.rbegin();

#if 0
		{
			auto ci1 = g_conn.begin();
			auto ci2 = g_conn.rbegin();

			if(g_conn.size() > 1 && 
				ci1->addr.host == ci2->addr.host &&
				ci1->addr.port == ci2->addr.port)
			{
				char msg[128];
				sprintf(msg, "mult c same at f%s, l%d", __FILE__, __LINE__);
				InfoMess("e", msg);
			}
		}
#endif

#if 0	//now done by ack
			ConnectPacket replycp;
			replycp.header.type = PACKET_CONNECT;
			//replycp.header.ack = 0;
			//nc->sendack = 0;
			replycp.reply = true;
			SendData((char*)&replycp, sizeof(ConnectPacket), from, true, false, nc, &g_sock, 0);

			//temp
			//g_canturn = true;
#endif

			return;
		}
	}

	nc->handshook = true;
	nc->closed = false;
	//nc->sendack = 0;

#if 1	//for ack to work
	nc->recvack = cp->header.ack;
	nc->sendack = 0;
#endif

#if 0	//now done by ack
	//we already have a connection to them, 
	//so they must have lost theirs if this isn't a reply to ours.
	if(!cp->reply)
	{
		//this is probably a copy since we already have a connection 
		//(or else they might have closed their connection and reconnected).
		//we need to check if we already have an outgoing reply ConnectPacket.

		bool outgoing = false;

		for(auto pit=g_outgo.begin(); pit!=g_outgo.end(); pit++)
		{
			if(!Same(&pit->addr, from))
				continue;

			PacketHeader* ph = (PacketHeader*)pit->buffer;

			if(ph->type != PACKET_CONNECT)
				continue;

#if 1	//necessary to know if it's a reply=true?
			ConnectPacket* oldcp = (ConnectPacket*)pit->buffer;

			if(!oldcp->reply)
				continue;
#endif

			outgoing = true;
			break;
		}

		if(!outgoing)
		{
			FlushPrev(from);
			
			nc->recvack = cp->header.ack;

			ConnectPacket replycp;
			replycp.header.type = PACKET_CONNECT;
			replycp.header.ack = 0;
			//nc->sendack = 1;
			replycp.reply = true;
			SendData((char*)&replycp, sizeof(ConnectPacket), from, true, false, nc, &g_sock, 0);
		}
	}

	//temp
	//g_canturn = true;
#endif

	//we got this in reply to a ConnectPacket sent?

#ifndef MATCHMAKER
#if 0	//now done in OnAck_Connect
	else
	{
		//is this a reply copy?
		if(PastAck(cp->header.ack, nc->recvack))
			return;	//if so, discard, because we've already dealt with a previous copy

		//update recvack since TranslatePacket won't do it for a ConnectPacket
		nc->recvack = cp->header.ack;

		//if(nc->ctype == CONN_HOST)
		if(nc->isourhost)
		{
			g_svconn = nc;

			//InfoMess("conn", "conn to our host");

			//TO DO request data, get ping, whatever, server info

			//g_canturn = true;
			//
			//char msg[128];
			//sprintf(msg, "send join to %u:%u aka %u:%u", from->host, (unsigned int)from->port, nc->addr.host, (unsigned int)nc->addr.port);
			//InfoMess("j", msg);

			JoinPacket jp;
			jp.header.type = PACKET_JOIN;
			std::string name = g_name.rawstr();
			if(name.length() >= PYNAME_LEN)
				name[PYNAME_LEN] = 0;
			strcpy(jp.name, name.c_str());
			SendData((char*)&jp, sizeof(JoinPacket), from, true, false, nc, &g_sock, 0);
		}
		//else if(nc->ctype == CONN_MATCHER)
	
		if(nc->ishostinfo)
		{
			//TO DO request data, get ping, whatever, server info
			GetSvInfoPacket gsip;
			gsip.header.type = PACKET_GETSVINFO;
			SendData((char*)&gsip, sizeof(GetSvInfoPacket), from, true, false, nc, &g_sock, 0);
		}

		if(nc->ismatcher)
		{
			//InfoMess("got mm", "got mm");
			//g_log<<"got mm"<<std::endl;
			//g_log.flush();
			g_mmconn = nc;
			g_sentsvinfo = false;

			if(g_reqsvlist && !g_reqdnexthost)
			{
				//g_log<<"got mm send f svl"<<std::endl;
				//g_log.flush();

				//g_reqdsvlist = true;
				//g_needsvlist = false;
				g_reqdnexthost = true;

				GetSvListPacket gslp;
				gslp.header.type = PACKET_GETSVLIST;
				SendData((char*)&gslp, sizeof(GetSvListPacket), &nc->addr, true, false, nc, &g_sock, 0);
				//InfoMess("sglp", "sglp");
			}
		}
	}
#endif
#else
#endif
}

//on connect packed ack'd
void OnAck_Connect(OldPacket* p, NetConn* nc)
{
	if(!nc)
		nc = Match(&p->addr);

	if(!nc)
		return;
	
	ConnectPacket* scp = (ConnectPacket*)p->buffer;

	if(!scp->reply)
	{
		//if(nc->ctype == CONN_HOST)
		if(nc->isourhost)
		{
			g_svconn = nc;

			//InfoMess("conn", "conn to our host");

			//TO DO request data, get ping, whatever, server info

			//g_canturn = true;
			//
			//char msg[128];
			//sprintf(msg, "send join to %u:%u aka %u:%u", from->host, (unsigned int)from->port, nc->addr.host, (unsigned int)nc->addr.port);
			//InfoMess("j", msg);

			JoinPacket jp;
			jp.header.type = PACKET_JOIN;
			std::string name = g_name.rawstr();
			if(name.length() >= PYNAME_LEN)
				name[PYNAME_LEN] = 0;
			strcpy(jp.name, name.c_str());
			SendData((char*)&jp, sizeof(JoinPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
		}
		//else if(nc->ctype == CONN_MATCHER)
	
		if(nc->ishostinfo)
		{
			//TO DO request data, get ping, whatever, server info
			GetSvInfoPacket gsip;
			gsip.header.type = PACKET_GETSVINFO;
			SendData((char*)&gsip, sizeof(GetSvInfoPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
		}

		if(nc->ismatcher)
		{
			//InfoMess("got mm", "got mm");
			//g_log<<"got mm"<<std::endl;
			//g_log.flush();
			g_mmconn = nc;
			g_sentsvinfo = false;

			if(g_reqsvlist && !g_reqdnexthost)
			{
				//g_log<<"got mm send f svl"<<std::endl;
				//g_log.flush();

				//g_reqdsvlist = true;
				//g_needsvlist = false;
				g_reqdnexthost = true;

				GetSvListPacket gslp;
				gslp.header.type = PACKET_GETSVLIST;
				SendData((char*)&gslp, sizeof(GetSvListPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
				//InfoMess("sglp", "sglp");
			}
		}
	}
}