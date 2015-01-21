

#include "lockstep.h"
#include "packets.h"
#include "readpackets.h"
#include "sendpackets.h"
#include "../sim/simdef.h"
#include "../sim/simflow.h"

std::list<PacketHeader*> g_nextcmdq;
std::list<PacketHeader*> g_localcmd;
bool g_canturn;	//only for client's use

/*

netfr 2:	cl sends pl bl packet
netfr 4:	sv recvs pl bl packet
netfr 5:	sv checks all outgoing OldPacket's, finds no NetTurnPacket that hasn't been ack'd. this means next turn can be sent.
			sv sends out NetTurnPacket for netfr 12.
netfr 5:	cl has g_canturn==true, so execs g_nextcmdq, sets g_canturn=false
netfr 8:	cl recv's NetTurnPacket for netfr 12, sets g_canturn=true, sends ack to sv

netfr 12:	sv hasn't recv'd ack, can't send out next NetTurnPacket for netfr 18
netfr 17:	cl can't step, the counter is at 18 but the sim state won't be updated until NetTurnPacket arrives

netfr 17:	sv finally recv's ack for NetTurnPacket for netfr 12, sends out for netfr 18

*/

void UpdTurn()
{
	if((g_netframe+1) % NETTURN != 0)
		return;

	if(g_netmode == NETM_SINGLE)
	{
		SP_StepTurn();
	}
	else if(g_netmode == NETM_HOST)
	{
#if 0
		char msg[128];
		sprintf(msg, "upd turn netfr%u sv", g_netframe);
		InfoMess("dt", msg);
#endif
		Sv_StepTurn();
	}
	else if(g_netmode == NETM_CLIENT)
	{
		Cl_StepTurn();

#if 0
		char msg[128];
		sprintf(msg, "done turn netfr%u packet send", g_netframe);
		InfoMess("dt", msg);
#endif

		DoneTurnPacket dtp;
		dtp.header.type = PACKET_DONETURN;
		dtp.player = g_localP;
		dtp.fornetfr = g_netframe;
		SendData((char*)&dtp, sizeof(DoneTurnPacket), &g_svconn->addr, true, false, g_svconn, &g_sock, 0, NULL);
	}
}

bool CanTurn()
{
	if(g_netmode == NETM_SINGLE)
		return true;

	if((g_netframe+1) % NETTURN == 0)
	{
		if(g_netmode == NETM_CLIENT)
		{
			//recv'd NetTurnPacket for next turn?
			if(g_canturn)
				return true;

			//buggable?
			//if(g_netframe == 0)
			//	return true;

			return false;
		}
		else if(g_netmode == NETM_HOST)
		{
			
#if 0
			char msg[128];
			sprintf(msg, "host check netfr%u finale sv", g_netframe);
			InfoMess("dt", msg);
#endif

			//temp check for cl's
			if(g_conn.size() <= 0)
				return false;

			bool havecl = false;

			for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
			{
				if(!ci->isclient)
					continue;

				if(!ci->handshook)
					continue;

				havecl = true;
				break;
			}

			if(!havecl)
				return false;

			//check for non-ack'd NetTurnPackets in outgoing OldPacket list
			for(auto pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
			{
				NetConn* nc = Match(&pi->addr);

				if(!nc)
					continue;

				if(!nc->handshook)
					continue;

				//if(nc->ctype != NETM_CLIENT)
				//	continue;

				if(!nc->isclient)
					continue;

				//also check if client is spectator or progressing through the history of the session TO DO

				if(((PacketHeader*)pi->buffer)->type != PACKET_NETTURN)
					continue;

				return false;	//ack not recv'd
			}

			// check for cl's being up to current turn
			//for(int pi=0; pi<PLAYERS; pi++)
			for(int pi=0; pi<1; pi++)
			{
				Player* py = &g_player[pi];

				//InfoMess("a", "all py's passed netfr c0");

				if(!py->on)
					continue;

				//InfoMess("a", "all py's passed netfr c1");

				if(py->ai)
					continue;
				
#if 0
				char msg1[128];
				sprintf(msg1, "all py's passed netfr c2, clnetfr%u", (py->curnetfr / NETTURN) * NETTURN);
				InfoMess("a", msg1);
#endif

				if( (py->curnetfr / NETTURN) * NETTURN != (g_netframe / NETTURN) * NETTURN )
					return false;

#if 0
				InfoMess("a", "all py's passed netfr finale");

				char msg[128];
				sprintf(msg, "netfr%u finale sv", g_netframe);
				InfoMess("dt", msg);
#endif
			}

			return true;
		}

		return false;
	}

	return true;
}

void ExecCmds(std::list<PacketHeader*>* q)
{
	//InfoMess("e", "e");

	for(auto pi=q->begin(); pi!=q->end(); pi++)
	{
#if 0
		if(((PacketHeader*)*pi)->type == PACKET_PLACEBL)
			InfoMess("plp", "plp");
#endif

#if 0
		char msg[128];
		sprintf(msg, "exec t%d", ((PacketHeader*)*pi)->type);
		InfoMess("exe", msg);
#endif

		TranslatePacket((char*)*pi, -1, false, NULL, NULL);
	}
}

void AppendCmd(std::list<PacketHeader*>* q, PacketHeader* p, short sz)
{
	unsigned char* newp = (unsigned char*)malloc(sz);
	if(!newp) OutOfMem(__FILE__, __LINE__);
	memcpy(newp, p, sz);
	q->push_back((PacketHeader*)newp);
}

void AppendCmds(std::list<PacketHeader*>* q, NetTurnPacket* ntp)
{
	unsigned char* pstart = ((unsigned char*)ntp)+sizeof(NetTurnPacket);
	unsigned char* pload = pstart;
	unsigned char* newp;

#if 0
	if(ntp->loadsz > 0)
		InfoMess("app", "appcmd");
#endif

	//go through all packets, add each to queue
	while( (int)(pload-pstart) < ntp->loadsz )
	{
		PacketHeader* ph = (PacketHeader*)pload;

#if 0
		char msg[128];
		sprintf(msg, "load ph->type=%d", ph->type);
		InfoMess("appcmd", msg);
#endif

		/*
		Must fill out all of these function switches with
		packets that will be placed in NetTurnPacket's.
		Some packets, like move orders, are of variable length,
		depending on number of unit selected etc.
		*/

		switch(ph->type)
		{
			//some packets dont get bundled in NetTurnPacket's, but they're here anyway
		case PACKET_ACKNOWLEDGMENT:
			newp = (unsigned char*)malloc(sizeof(AckPacket));
			if(!newp) OutOfMem(__FILE__, __LINE__);
			memcpy(newp, ph, sizeof(AckPacket));
			q->push_back((PacketHeader*)newp);
			newp = NULL;
			pload += sizeof(AckPacket);
			break;
		case PACKET_CONNECT:
			newp = (unsigned char*)malloc(sizeof(ConnectPacket));
			if(!newp) OutOfMem(__FILE__, __LINE__);
			memcpy(newp, ph, sizeof(ConnectPacket));
			q->push_back((PacketHeader*)newp);
			newp = NULL;
			pload += sizeof(ConnectPacket);
			break;
		case PACKET_DISCONNECT:
			newp = (unsigned char*)malloc(sizeof(DisconnectPacket));
			if(!newp) OutOfMem(__FILE__, __LINE__);
			memcpy(newp, ph, sizeof(DisconnectPacket));
			q->push_back((PacketHeader*)newp);
			newp = NULL;
			pload += sizeof(DisconnectPacket);
			break;
		case PACKET_PLACEBL:
			//InfoMess("bundl", "placebl detected in AppendCmds");
			newp = (unsigned char*)malloc(sizeof(PlaceBlPacket));
			if(!newp) OutOfMem(__FILE__, __LINE__);
			memcpy(newp, ph, sizeof(PlaceBlPacket));
			q->push_back((PacketHeader*)newp);
			newp = NULL;
			pload += sizeof(PlaceBlPacket);
			break;
		case PACKET_CHVAL:
			newp = (unsigned char*)malloc(sizeof(ChValPacket));
			if(!newp) OutOfMem(__FILE__, __LINE__);
			memcpy(newp, ph, sizeof(ChValPacket));
			q->push_back((PacketHeader*)newp);
			newp = NULL;
			pload += sizeof(ChValPacket);
			break;
		default:
			break;
		}
	}
}

void FillNetTurnPacket(NetTurnPacket** pp, std::list<PacketHeader*>* q)
{
	int loadsz = 0;

	//go through all packets, calc size
	for(auto pi=q->begin(); pi!=q->end(); pi++)
	{
		switch((*pi)->type)
		{
			//some packets dont get bundled in NetTurnPacket's, but they're here anyway
		case PACKET_ACKNOWLEDGMENT:
			loadsz += sizeof(AckPacket);
			break;
		case PACKET_CONNECT:
			loadsz += sizeof(ConnectPacket);
			break;
		case PACKET_DISCONNECT:
			loadsz += sizeof(DisconnectPacket);
			break;
		case PACKET_PLACEBL:
			loadsz += sizeof(PlaceBlPacket);
			break;
		case PACKET_CHVAL:
			loadsz += sizeof(ChValPacket);
			break;
		default:
			break;
		}
	}

	*pp = (NetTurnPacket*)malloc(sizeof(NetTurnPacket)+loadsz);

	NetTurnPacket* ntp = *pp;
	ntp->header.type = PACKET_NETTURN;
	ntp->fornetfr = (g_netframe / NETTURN + 1) * NETTURN;
	ntp->loadsz = loadsz;

	unsigned char* pload = ((unsigned char*)ntp)+sizeof(NetTurnPacket);

	//go through all packets, add each to load
	for(auto pi=q->begin(); pi!=q->end(); pi++)
	{
		switch((*pi)->type)
		{
			//some packets dont get bundled in NetTurnPacket's, but they're here anyway
		case PACKET_ACKNOWLEDGMENT:
			memcpy(pload, *pi, sizeof(AckPacket));
			pload += sizeof(AckPacket);
			break;
		case PACKET_CONNECT:
			memcpy(pload, *pi, sizeof(ConnectPacket));
			pload += sizeof(ConnectPacket);
			break;
		case PACKET_DISCONNECT:
			memcpy(pload, *pi, sizeof(DisconnectPacket));
			pload += sizeof(DisconnectPacket);
			break;
		case PACKET_PLACEBL:
			memcpy(pload, *pi, sizeof(PlaceBlPacket));
			pload += sizeof(PlaceBlPacket);
			break;
		case PACKET_CHVAL:
			memcpy(pload, *pi, sizeof(ChValPacket));
			pload += sizeof(ChValPacket);
			break;
		default:
			break;
		}
	}
}

#if 0	//no more
//internal use
void Cl_StepTurn(NetTurnPacket* ntp)
{
	ExecCmds(&g_nextcmdq);
	FreeCmds(&g_nextcmdq);
	AppendCmds(&g_nextcmdq, ntp);
	g_canturn = false;
}
#endif

void Cl_StepTurn()
{
	//InfoMess("c", "cl st");
	ExecCmds(&g_nextcmdq);
	FreeCmds(&g_nextcmdq);
	g_canturn = false;
}

//single player
void SP_StepTurn()
{
	ExecCmds(&g_nextcmdq);
	FreeCmds(&g_nextcmdq);
	//g_canturn = true;
}

//previous NetTurnPacket must have been ack'd by all clients before proceeding
//and all cl's must be up to previous turn
void Sv_StepTurn()
{
	//InfoMess("c", "sv st");
	NetTurnPacket *ntp;
	FillNetTurnPacket(&ntp, &g_localcmd);
	FreeCmds(&g_localcmd);
	SendAll((char*)ntp, sizeof(NetTurnPacket) + ntp->loadsz, true, false, NULL);
	ExecCmds(&g_nextcmdq);
	FreeCmds(&g_nextcmdq);
	AppendCmds(&g_nextcmdq, ntp);

#if 0	
	if(g_localcmd.size() > 0)
		InfoMess("sv", "sv has local cmds");

	if(ntp->loadsz > 0)
		InfoMess("sv", "sv has load");

	if(g_nextcmdq.size() > 0)
		InfoMess("sv", "sv has cmds");
#endif

	free(ntp);
}

void AppendCmd(PacketHeader* p, int sz, std::list<PacketHeader*>* q)
{
	PacketHeader* p2 = (PacketHeader*)malloc(sz);
	if(!p2) OutOfMem(__FILE__, __LINE__);
	memcpy(p2, p, sz);
	q->push_back(p2);
}

void FreeCmds(std::list<PacketHeader*>* q)
{
	auto it = q->begin();

	while(it != q->end())
	{
		free(*it);
		it = q->erase(it);
	}
}

//Use for all commands that are in lockstep.
//E.g., move orders, place building action, changing values.
void LockCmd(PacketHeader* p, short sz)
{
#ifndef MATCHMAKER
	if(g_netmode == NETM_HOST)
	{
		PacketHeader* newp = (PacketHeader*)malloc(sz);
		memcpy(newp, p, sz);
		g_localcmd.push_back((PacketHeader*)newp);

#if 0
		NetConn* nc = &*g_conn.begin();

		if(nc)
			SendData((char*)&pbp, sizeof(PlaceBlPacket), &nc->addr, true, nc, &g_sock);
#endif
	}
	else if(g_netmode == NETM_CLIENT)
	{
		NetConn* nc = g_svconn;

		if(nc)
			SendData((char*)p, sz, &nc->addr, true, false, nc, &g_sock, 0, NULL);
	}
	else if(g_netmode == NETM_SINGLE)
	{
#if 0
		char msg[128];
		sprintf(msg, "send %d at %d,%d", pbp.btype, pbp.tpos.x, pbp.tpos.y);
		InfoMess("polk", msg);
#endif
		AppendCmd(&g_nextcmdq, (PacketHeader*)p, sz);
	}
#endif
}

