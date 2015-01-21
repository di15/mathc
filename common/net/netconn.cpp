

#include "../platform.h"
#include "netconn.h"
#include "lockstep.h"
#include "sendpackets.h"
#include "../sim/simflow.h"
#include "../sim/player.h"
#include "../save/savemap.h"
#include "../path/pathdebug.h"
#include "../utils.h"
#include "readpackets.h"

#ifndef MATCHMAKER
#include "../../game/gmain.h"
#endif

bool g_sentsvinfo = false;	//did we send our hosted game's IP to the sv list?
//bool g_needsvlist = false;	//did we request a sv list?
//bool g_reqdsvlist = false;	//did we send out a request to get sv list?

UDPsocket g_sock = NULL;
std::list<NetConn> g_conn;
std::list<OldPacket> g_outgo;	//outgoing packets. sent are those that have arrived at the other side already.
std::list<OldPacket> g_recv;

NetConn* g_svconn = NULL;
NetConn* g_mmconn = NULL;	//matchmaker

void NetConn::expirein(int millis)
{
	unsigned long long now = GetTickCount64();
	lastrecv = now - NETCONN_TIMEOUT + millis;
}

void OpenSock()
{
	if(g_sock)
	{
		return;
		SDLNet_UDP_Close(g_sock);
		g_sock = NULL;
	}

	//try 10 ports
#ifdef MATCHMAKER
	for(int i=0; i<1; i++)
#else
	for(int i=0; i<10; i++)
#endif
	{
		if(!(g_sock = SDLNet_UDP_Open(PORT+i)))
			continue;

		//char msg[128];
		//sprintf(msg, "open port %d", PORT+i);
		//InfoMess("p", msg);

		//SDLNet_UDP_SetPacketLoss(g_sock, 70);

		return;
	}
	
	char msg[1280];
	sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
	ErrMess("Error", msg);
}

NetConn* Connect(const char* addrstr, unsigned short port, bool ismatcher, bool isourhost, bool isclient, bool ishostinfo)
{
	IPaddress ip;

	if(SDLNet_ResolveHost(&ip, addrstr, port) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
		return NULL;
	}

#if 0
	if(!g_sock && !(g_sock = SDLNet_UDP_Open(0)))
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
		return NULL;
	}
#endif

	return Connect(&ip, ismatcher, isourhost, isclient, ishostinfo);
}

//Safe to call more than once, if connection already established, this will just
//update NetConn booleans.
//Edit: Maybe not safe anymore. Connection will reset to 0 on additional call.
//Necesssary because other side might have lost connection (timeout) and if we 
//send using the last acks we'll just get acks back but other side won't read messages.
//Edit: probably unlikely that one side will time out significantly before the other. Still safe.
NetConn* Connect(IPaddress* ip, bool ismatcher, bool isourhost, bool isclient, bool ishostinfo)
{
	OpenSock();

	//g_log<<"connect();"<<std::endl;

#if 1
	if(SDLNet_UDP_Bind(g_sock, 0, ip) == -1)
	{
		char msg[1280];
		sprintf(msg, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		ErrMess("Error", msg);
		return NULL;
	}
#endif

	NetConn* nc = Match(ip);

	NetConn newnc;

	if(!nc)
	{
		newnc.addr = *ip;
		newnc.handshook = false;
		newnc.lastrecv = GetTickCount64();
		newnc.lastsent = newnc.lastrecv;
		//important - reply ConnectPacket with ack=0 will be 
		//ignored as copy (even though it is original) if new NetConn's recvack=0.
		newnc.recvack = USHRT_MAX;
		newnc.sendack = 0;
		newnc.closed = false;
		g_conn.push_back(newnc);

#if 0
		{
			auto ci1 = g_conn.begin();
			auto ci2 = g_conn.rbegin();

			if(g_conn.size() > 1 && 
				/* ci1->addr.host == ci2->addr.host &&
				ci1->addr.port == ci2->addr.port */
				//memcmp(&ci1->addr, &ci2->addr, sizeof(IPaddress)) == 0
				Same(&ci1->addr, &ci2->addr)
				)
			{
				char msg[128];
				sprintf(msg, "mult c same at f%s, l%d", __FILE__, __LINE__);
				InfoMess("e", msg);
			}
		}
#endif

		nc = &*g_conn.rbegin();
	}
	else
	{
		//force reconnect (sending ConnectPacket).
		//also important for Click_SL_Join to know that we 
		//can't send a JoinPacket immediately after this function,
		//but must wait for a reply ConnectPacket.
		if(nc->closed)
			nc->handshook = false;
	}

	//nc.ctype = CONN_HOST;
	//nc.isourhost = true;
	//g_conn.push_back(nc);
	
	//only "true" it, or retain current state of nc->...
	nc->isclient = isclient ? true : nc->isclient;
	nc->isourhost = isourhost ? true : nc->isourhost;
	nc->ismatcher = ismatcher ? true : nc->ismatcher;
	nc->ishostinfo = ishostinfo ? true : nc->ishostinfo;

	if(isourhost)
		g_svconn = nc;
	if(ismatcher)
		g_mmconn = nc;

	//see if we need to connect for realsies.
	//i.e., send a connect packet and clean previous packets.
	if(!nc->handshook || 
		GetTickCount64() - nc->lastrecv > NETCONN_TIMEOUT ||
		nc->closed)
	{
		//if(!nc->handshook)
		//	InfoMess("!has", "!hsho");

		//if(GetTickCount64() - nc->lastrecv > NETCONN_TIMEOUT)
		//	InfoMess("GetTickCount64() - nc->lastrecv > NETCONN_TIMEOUT", "GetTickCount64() - nc->lastrecv > NETCONN_TIMEOUT");

		bool sending = false;	//sending ConnectPacket?

		for(auto pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
		{
			if(!Same(&pi->addr, &nc->addr))
				continue;

			PacketHeader* ph = (PacketHeader*)pi->buffer;

			if(ph->type != PACKET_CONNECT)
				continue;

			sending = true;
			break;
		}

		if(!sending)
		{
			//don't flush prev packs. maybe this was a gethostinfo 
			//connection and now it's also becoming an ourhost connection.
			//actually, check if we're already handshook.
			FlushPrev(ip);

			ConnectPacket cp;
			cp.header.type = PACKET_CONNECT;
			cp.reply = false;
			SendData((char*)&cp, sizeof(ConnectPacket), ip, true, false, nc, &g_sock, 0, OnAck_Connect);
		}
	}

	nc->closed = false;

	return nc;
}

//flush all previous incoming and outgoing packets from this addr
void FlushPrev(IPaddress* from)
{
	auto it = g_outgo.begin();

	while(it!=g_outgo.end())
	{
		//if(memcmp(&it->addr, from, sizeof(IPaddress)) != 0)
		if(!Same(&it->addr, from))
		{
			it++;
			continue;
		}

		//it->freemem();
		it = g_outgo.erase(it);
	}

	it = g_recv.begin();
	
	while(it!=g_recv.end())
	{
		//if(memcmp(&it->addr, from, sizeof(IPaddress)) != 0)
		if(!Same(&it->addr, from))
		{
			it++;
			continue;
		}

		//it->freemem();
		it = g_recv.erase(it);
	}
}

//keep expiring connections alive (try to)
void KeepAlive()
{	
	//return;

#ifdef MATCHMAKER
	//return;
#endif

	unsigned long long nowt = GetTickCount64();
	auto ci = g_conn.begin();
	
#if 0
	g_log<<"ka 1"<<std::endl;
	g_log.flush();
#endif

	while(g_conn.size() > 0 && ci != g_conn.end())
	{
		
#if 0
			g_log<<"g_conn.size()="<<g_conn.size()<<std::endl;
			g_log.flush();
#endif
			
#if 0
	g_log<<"ka 2"<<std::endl;
	g_log.flush();
#endif

		if(!ci->handshook || ci->closed)
		{
#if 0
	g_log<<"ka 3"<<std::endl;
	g_log.flush();
#endif
			ci++;
			continue;
		}
#if 0
	g_log<<"ka 4"<<std::endl;
	g_log.flush();
#endif

		if(nowt - ci->lastrecv > NETCONN_TIMEOUT/2)
		{
#if 0
	g_log<<"ka 5 "<<nowt<<" - "<<ci->lastrecv<<" = "<<(nowt-ci->lastrecv)<<" > "<<(NETCONN_TIMEOUT/2)<<std::endl;
	g_log.flush();
#endif
			//check if we're already trying to send a packet to get a reply
			bool outgoing = false;
			
#if 0
			g_log<<"g_outgo.size()="<<g_outgo.size()<<std::endl;
			g_log.flush();
#endif

			for(auto pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
			{
#if 0
	g_log<<"ka 6"<<std::endl;
	g_log.flush();
#endif
				//if(memcmp(&pi->addr, &ci->addr, sizeof(IPaddress)) != 0)
				if(!Same(&pi->addr, &ci->addr))
				{
					continue;
				}

#if 0
	g_log<<"ka 7"<<std::endl;
	g_log.flush();
#endif

				outgoing = true;
				break;
			}
			
#if 0
	g_log<<"ka 8"<<std::endl;
	g_log.flush();
#endif
			if(outgoing)
			{
				ci++;
				continue;
			}
#if 0
			g_log<<"kap"<<std::endl;
			//InfoMess("kap", "kap");

			g_log<<"g_conn.size()="<<g_conn.size()<<std::endl;
			g_log.flush();
#endif
			KeepAlivePacket kap;
			kap.header.type = PACKET_KEEPALIVE;
			SendData((char*)&kap, sizeof(KeepAlivePacket), &ci->addr, true, false, &*ci, &g_sock, 0, NULL);
		}

		
#if 0
	g_log<<"ka 9"<<std::endl;
	g_log.flush();
#endif

		ci++;
	}
	
#if 0
	g_log<<"ka 10"<<std::endl;
	g_log.flush();
#endif
}

void CheckConns()
{
	//return;

	unsigned long long now = GetTickCount64();
	auto ci = g_conn.begin();

	while(g_conn.size() > 0 && ci != g_conn.end())
	{
		if(now - ci->lastrecv > NETCONN_TIMEOUT)
		{
			//TO DO any special condition handling, inform user about sv timeout, etc.

#ifndef MATCHMAKER
			if(ci->ismatcher)
			{
#ifdef NET_DEBUG
				g_log<<"time out conn (now - ci->lastrecv > NETCONN_TIMEOUT = "<<now<<" - "<<ci->lastrecv<<" = "<<(now - ci->lastrecv)<<" > "<<NETCONN_TIMEOUT<<")"<<ci->addr.host<<" "<<DateTime()<<" msec"<<GetTickCount64()<<std::endl;
				g_log.flush();
#endif
#if 0
				unsigned long long passed = now - ci->lastrecv;
				char msg[1280];
				sprintf(msg, "Connection to matchmaker server timed out (%f seconds, num conn = %d).", (float)(passed/1000.0f), (int)g_conn.size());
				
				for(auto ci2 = g_conn.begin(); ci2 != g_conn.end(); ci2++)
				{
					char add[128];
					sprintf(add, "\r\n ci2: ip%u,po%u ?shook:%d", ci2->addr.host, (unsigned int)ci2->addr.port, (int)ci2->handshook);
					strcat(msg, add);
				}
				
				ErrMess("Error", msg);
#else
				ErrMess("Error", "Connection to matchmaker server timed out.");
#endif
			}
			else if(ci->isourhost)
			{
#if 0
				unsigned long long passed = now - ci->lastrecv;
				char msg[1280];
				sprintf(msg, "Connection to game host timed out (%f seconds, num conn = %d).", (float)(passed/1000.0f), (int)g_conn.size());
				
				for(auto ci2 = g_conn.begin(); ci2 != g_conn.end(); ci2++)
				{
					char add[128];
					sprintf(add, "\r\n ci2: ip%u,po%u ?shook:%d", ci2->addr.host, (unsigned int)ci2->addr.port, (int)ci2->handshook);
					strcat(msg, add);
				}
				
				ErrMess("Error", msg);
#else
				ErrMess("Error", "Connection to game host timed out.");
				//g_log<<"Connection to game host timed out."<<std::endl;
#endif
			
				g_mode = APPMODE_MENU;
				g_netmode = NETM_SINGLE;
				EndSess();
				//return;
			}
			else if(ci->ishostinfo)
				ErrMess("Error", "Connection to prospective game host timed out.");
			else if(ci->isclient)
				ErrMess("Error", "Connection to client timed out.");

			if(ci->ismatcher)
			{
				g_sentsvinfo = false;
				g_mmconn = NULL;
			}

			if(ci->isourhost)
			{
				g_svconn = NULL;
			}
#else
			g_log<<DateTime()<<" timed out"<<std::endl;
			g_log.flush();
#endif

			FlushPrev(&ci->addr);

			ci = g_conn.erase(ci);
			//ci++;
			continue;
		}

		ci++;
	}
}

NetConn* Match(IPaddress* addr)
{
	if(!addr)
		return NULL;

	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(Same(&ci->addr, addr))
		//if(memcmp((void*)&ci->addr, (void*)addr, sizeof(IPaddress)) == 0)
			return &*ci;

	return NULL;
}

#ifndef MATCHMAKER

//used to clear cmd queues
void EndSess()
{
	FreeCmds(&g_localcmd);
	FreeCmds(&g_nextcmdq);

	FreeMap();

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->closeall();
	gui->open("main");
}

//used to clear cmd queues
void BegSess()
{
	FreeCmds(&g_localcmd);
	FreeCmds(&g_nextcmdq);

	g_simframe = 0;
	g_netframe = 0;
	g_speed = SPEED_PLAY;
	g_gameover = false;

	for(int pi=0; pi<PLAYERS; pi++)
	{
		Player* py = &g_player[pi];
		py->curnetfr = 0;
		py->truckwage = 0;
		py->transpcost = 0;
	}

	g_gridvecs.clear();
}

#endif

void Disconnect(NetConn* nc)
{
	DisconnectPacket dp;
	dp.header.type = PACKET_DISCONNECT;
	SendData((char*)&dp, sizeof(DisconnectPacket), &nc->addr, true, false, nc, &g_sock, 0, NULL);
	nc->closed = true;
	//nc->expirein(RESEND_DELAY*2);

#if 0
	FlushPrev(&nc->addr);

	//will be removed at the end of UpdNet();.
	auto cit = g_conn.begin();
	while(cit != g_conn.end())
	{
		if(&*cit != nc)
			continue;

		cit = g_conn.erase(cit);
		break;
	}
#endif
}