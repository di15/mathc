
#include "../platform.h"
#include "net.h"
#include "netconn.h"
#include "readpackets.h"
#include "sendpackets.h"
#include "packets.h"
#include "../utils.h"
#ifndef MATCHMAKER
#include "../sim/player.h"
#include "../gui/gui.h"
#include "../gui/widgets/spez/svlist.h"
#endif

unsigned long long g_lastS;  //last sent
unsigned long long g_lastR;  //last recieved

#ifndef MATCHMAKER
int g_netmode = NETM_SINGLE;
#else
int g_netmode = NETM_HOST;
unsigned int g_transmitted = 0;
#endif

char g_mapname[MAPNAME_LEN+1] = "";
char g_svname[SVNAME_LEN+1] = "";

#ifndef MATCHMAKER
unsigned int g_transmitted = 0;
#endif

unsigned short NextAck(unsigned short ack)
{
	//if(ack == UINT_MAX)
	//	ack = 0;
	//else
	//	ack ++;

	ack++;

	return ack;
}

unsigned short PrevAck(unsigned short ack)
{
	//if(ack == 0)
	//	ack = UINT_MAX;
	//else
	//	ack --;

	ack--;

	return ack;
}

bool PastAck(unsigned short test, unsigned short current)
{
	return ((current >= test) && (current - test <= USHRT_MAX/2))
	       || ((test > current) && (test - current > USHRT_MAX/2));
}

bool PastFr(unsigned int test, unsigned int current)
{
	return ((current >= test) && (current - test <= UINT_MAX/2))
	       || ((test > current) && (test - current > UINT_MAX/2));
}


#ifndef MATCHMAKER
//add us to sv list of matchmaker,
//check sv hosts for info
//(two different things, one for hosts, one for clients).
void CheckAddSv()
{
	//check sv hosts for info
#if 0
	for(auto cit=g_conn.begin(); cit!=g_conn.end(); cit++)
	{
		NetConn* nc = &*cit;

		if(!nc->ishostinfo)
			continue;
	}
#else
	//check sv hosts for info
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
	
	auto sit = v->m_svlist.begin();
	while(sit != v->m_svlist.end())
	{
		if(sit->replied)
		{
			sit++;
			continue;
		}

		NetConn* nc = Match(&sit->addr);

		if(!nc)
		{
			//timeout?
			if(v->m_selsv == &*sit)
				v->m_selsv = NULL;

			sit = v->m_svlist.erase(sit);
			continue;
		}
		//update ping
		else
		{
			char pingstr[16];
			sprintf(pingstr, "%d", (int)nc->ping);
			sit->ping = nc->ping;
			sit->pingrt = RichText(pingstr);
		}

		sit++;
	}
#endif

	//add us to sv list of matchmaker
	//if(g_netmode == NETM_HOST && !g_sentsvinfo)
	if(false)	//disabled, since no matchmaker server up
	{
		if(g_mmconn)
		{
			//check if we've got an outgoing packet in queue
			if(g_mmconn->handshook)
			{
				bool outgoing = false;

				for(auto pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
				{
					OldPacket* op = (OldPacket*)&*pi;
					
					//if(memcmp(&op->addr, &g_mmconn->addr, sizeof(IPaddress)) != 0)
					if(!Same(&op->addr, &g_mmconn->addr))
						continue;
					
					PacketHeader* ph = (PacketHeader*)op->buffer;

					if(ph->type != PACKET_ADDSV)
						continue;

					outgoing = true;
					break;
				}

				if(!outgoing)
				{
					//InfoMess("se", "sen asv");
					AddSvPacket asp;
					asp.header.type = PACKET_ADDSV;
#if 0
					//actually, this goes into a SvInfoPacket
					memcpy(asp.svinfo.mapname, g_mapname, sizeof(char)*MAPNAME_LEN);
					memcpy(asp.svinfo.svname, g_svname, sizeof(char)*SVNAME_LEN);
					asp.svinfo.nplayers = 1;	//TO DO
					memset(&asp.svinfo.addr, 0, sizeof(IPaddress));
#endif
					SendData((char*)&asp, sizeof(AddSvPacket), &g_mmconn->addr, true, false, g_mmconn, &g_sock, 0);
					g_sentsvinfo = true;
				}
			}
			// !g_mmconn->handshook
			else
			{
#if 0	//we've got this connection so we've already sent a connect packet. code below only creates 
		//problems as multiple connect packets get ack'd and resent with new acks before the connectpacket back arrives.
				bool connecting = false;

				for(auto pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
				{
					OldPacket* op = (OldPacket*)&*pi;
					
					//if(memcmp(&op->addr, &g_mmconn->addr, sizeof(IPaddress)) != 0)
					if(!Same(&op->addr, &g_mmconn->addr))
						continue;
					
					PacketHeader* ph = (PacketHeader*)op->buffer;

					if(ph->type != PACKET_CONNECT)
						continue;

					connecting = true;
					break;
				}

				if(!connecting)
				{
					Connect(SV_ADDR, PORT, true, false, false, false);
					g_log<<"con mm 1 g_outgo.size() = "<<g_outgo.size()<<std::endl;
				}
#endif
			}
		}
		// !g_mmconn
		else
		{
			Connect(SV_ADDR, PORT, true, false, false, false);
			g_log<<"con mm 2"<<std::endl;
		}
	}
}
#endif

#ifdef MATCHMAKER
#define SV_LIST_RATE		5	//limit 127

//send out server lists
void SendSvs()
{
	//check which clients need to send to
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
	{
		
	//g_log<<"send sv l 1"<<std::endl;
	//g_log.flush();

		//if we're not sending to this client, or 
		//if we've already sent all our sv's.
		if(ci->svlistoff < 0)
			continue;

		//if it's not a client.
		if(!ci->isclient)
			continue;

		//how many we're already sending (outgoing) to this cl.
		int sending = 0;

		//have to go through all outgoing packets to match them up.
		for(auto pi=g_outgo.begin(); pi!=g_outgo.end(); pi++)
		{
			//same address as this client?
			if(!Same(&pi->addr, &ci->addr))
				continue;

			PacketHeader* ph = (PacketHeader*)pi->buffer;

			//an sv addr packet?
			if(ph->type != PACKET_SVADDR)
				continue;

			sending++;
		}

		signed char tosend = SV_LIST_RATE - sending;

		if(tosend <= 0)
			continue;

		auto ait = g_conn.begin();	//address iterator
		int ain = 0;	//address index

		while(true)
		{
			//if we've reached the end of our connections,
			//we've sent all of our sv list
			if(ait == g_conn.end())
			{
				ci->svlistoff = -1;
				tosend = 0;
				break;
			}

			//if this isn't a game host, increment the iterator
			//and go to next.
			if(!ait->ishostinfo)
			{
				ait++;
				continue;
			}

			//if we've reached where we've left off on last send,
			//send this sv addr.
			if(ain >= ci->svlistoff)
			{
				SvAddrPacket sap;
				sap.header.type = PACKET_SVADDR;
				memcpy(&sap.addr, &ait->addr, sizeof(IPaddress));
				SendData((char*)&sap, sizeof(SvAddrPacket), &ci->addr, true, false, &*ci, &g_sock, 0);
				tosend--;
				ci->svlistoff = ain;

	g_log<<"send sv l 2 g_conn.size()="<<g_conn.size()<<std::endl;
	g_log.flush();

	unsigned int svipaddr = SDL_SwapBE32(ait->addr.host);
	unsigned short svport = SDL_SwapBE16(ait->addr.port);
	unsigned int svipaddr2 = SDL_SwapBE32(ci->addr.host);
	unsigned short svport2 = SDL_SwapBE16(ci->addr.port);

	g_log<<"send to "<<svipaddr2<<":"<<svport2<<" about sv "<<svipaddr<<":"<<svport<<std::endl;
	g_log.flush();
			}

			//increment index and iterator, because this is a game host.
			ain++;
			ait++;
		}
	}
}
#endif

#ifndef MATCHMAKER
//check getting sv list info's
void CheckGetSvs()
{
	if(!g_reqsvlist)
		return;

	//if already getting next host
	if(g_reqdnexthost)
		return;

	//check if it's time to queue up next sv addr
	//if the last reply from any of the previous was 200 ms
	//and we have < NETCONN_TIMEOUT/RESEND_DELAY or <50 connections
	const int maxconn = imax(NETCONN_TIMEOUT/RESEND_DELAY, 50);
	int numconn = 0;
	unsigned long long latest = 0;

	for(auto sit=g_conn.begin(); sit!=g_conn.end(); sit++)
	{
		if(!sit->ishostinfo)
			continue;

		numconn ++;

		NetConn* nc = Match(&sit->addr);

		if(!nc)
			continue;

		if(nc->lastrecv > latest)
			latest = nc->lastrecv;
	}

	//if there's more hosts than limit
	//or if latest reply was <200ms ago
	if(numconn >= maxconn && 
		GetTickCount64() - latest <= RESEND_DELAY)
		return;

	if(!g_mmconn)
	{
		//need to be connected to matchmaker
		Connect(SV_ADDR, PORT, true, false, false, false);
		return;
	}

	if(!g_mmconn->handshook)
		return;

	SendNextHostPacket snhp;
	snhp.header.type = PACKET_SENDNEXTHOST;
	SendData((char*)&snhp, sizeof(SendNextHostPacket), &g_mmconn->addr, true, false, g_mmconn, &g_sock, 0);

	//prevent us from sending out more nexthost requests
	//until we get the current one.
	g_reqdnexthost = true;
}

//check for unresponsive clients
void CheckCls()
{
	if(g_netmode != NETM_HOST)
		return;

	const unsigned long long now = GetTickCount64();

	for(auto cit=g_conn.begin(); cit!=g_conn.end(); cit++)
	{
		if(!cit->isclient)
			continue;

		if(now - cit->lastrecv < NETCONN_UNRESP)
			continue;

		//unresponsive, what to do?

		if(cit->isunresponsive)
			continue;

		cit->isunresponsive = true;

		ClStatePacket csp;
		csp.header.type = PACKET_CLSTATE;
		csp.chtype = CLCH_UNRESP;
		csp.client = cit->client;
		SendAll((char*)&csp, sizeof(ClStatePacket), true, false, NULL);
	}
	
	//unresponsive becoming responsive again
	for(auto cit=g_conn.begin(); cit!=g_conn.end(); cit++)
	{
		if(!cit->isclient)
			continue;
		
		if(now - cit->lastrecv >= NETCONN_UNRESP)
			continue;

		if(!cit->isunresponsive)
			continue;

		cit->isunresponsive = false;

		ClStatePacket csp;
		csp.header.type = PACKET_CLSTATE;
		csp.chtype = CLCH_RESP;
		csp.client = cit->client;
		SendAll((char*)&csp, sizeof(ClStatePacket), true, false, NULL);
	}

	//TODO send this as well to newly joined clients on join?
}
#endif

//Check if we can quit. There's also a countdown to 
//make sure we quit eventually either way.
bool NetQuit()
{
#ifndef MATCHMAKER
	for(auto oit=g_outgo.begin(); oit!=g_outgo.end(); oit++)
	{
		PacketHeader* ph = (PacketHeader*)oit->buffer;

		if(ph->type != PACKET_DISCONNECT)
			continue;
		
		//Sending out disconnect packet, so can't quit yet.
		return false;
	}
#endif

	return true;
}

void EndConns()
{
	auto cit = g_conn.begin();
	unsigned long long now = GetTickCount64();

	while(cit != g_conn.end())
	{
		if(!cit->closed
			|| now - cit->lastrecv < NETCONN_TIMEOUT)
		{
			cit++;
			continue;
		}

		FlushPrev(&cit->addr);
		cit = g_conn.erase(cit);
	}
}

//Net input
void UpdNet()
{
	//struct sockaddr_in from;
	//socklen_t fromlen = sizeof(struct sockaddr_in);
	//char buffer[1024];
	int bytes;

	UDPpacket *in;

	//IPaddress* addr = &ns->addr;

	//unsigned int svipaddr = SDL_SwapBE32(addr->host);
	//unsigned short svport = SDL_SwapBE16(addr->port);

	UDPsocket* sock = &g_sock;

	if(!sock)
		return;
	
	in = SDLNet_AllocPacket(65535);

	do
	{
		in->data[0] = 0;
		//bytes = recvfrom(g_socket, buffer, 1024, 0, (struct addr *)&from, &fromlen);
		bytes = SDLNet_UDP_Recv(*sock, in);

		IPaddress ip;

		memcpy(&ip, &in->address, sizeof(IPaddress));
		//const char* host = SDLNet_ResolveIP(&ip);
		//unsigned int ipaddr = SDL_SwapBE32(ip.host);
		unsigned short port = SDL_SwapBE16(ip.port);

		//if(port < PORT || port > PORT+10)
		//	continue;

		if(bytes > 0)
		{
//#ifdef MATCHMAKER
#if 1
			g_transmitted += bytes;
#endif
			//if(memcmp((void*)&from, (void*)&g_sockaddr, sizeof(struct sockaddr_in)) != 0)
			//if(ipaddr != svipaddr)
			//	continue;
			
			//g_log<<"r t"<<((PacketHeader*)in->data)->type<<std::endl;
			//g_log.flush();
			
			//for(int i=0; i<bytes; i++)
			//for(int i=0; i<4; i++)
			//{
			//	g_log<<"r"<<i<<": "<<(unsigned int)(in->data[i])<<std::endl;
			//	g_log.flush();
			//}

#if 0
			if(((PacketHeader*)in->data)->type == PACKET_NETTURN)
			{
				char msg[128];
				sprintf(msg, "recv ack%u t%d", (unsigned int)((PacketHeader*)in->data)->ack, ((PacketHeader*)in->data)->type);
				InfoMess("recv", msg);
			}
#endif

			//TranslatePacket(buffer, bytes, true);
			TranslatePacket((char*)in->data, bytes, true, &g_sock, &ip);
		}
	} while(bytes > 0);

	SDLNet_FreePacket(in);

	KeepAlive();
	CheckConns();
	ResendPacks();

#ifndef MATCHMAKER
	CheckAddSv();
	CheckGetSvs();
	CheckCls();
#else
	SendSvs();
#endif

	//EndConns();

#if 1
	static long long lastlog = GetTickCount64();
	
#ifdef MATCHMAKER
	if(GetTickCount64() - lastlog > (1000 * 60 * 60))
#else
	if(GetTickCount64() - lastlog > (1000 * 60))
#endif
	{
		lastlog = GetTickCount64();
		g_log<<"transmitted "<<DateTime()<<" = "<<g_transmitted<<" bytes"<<std::endl;
		g_log.flush();
		g_transmitted = 0;
	}
#endif
}

bool Same(IPaddress* a, IPaddress* b)
{
	if(a->host != b->host)
		return false;

	if(a->port != b->port)
		return false;

	return true;
}

#if 1

void ClearPackets()
{
#if 0
	for(auto i=g_outgo.begin(); i!=g_outgo.end(); i++)
		i->freemem();

	for(auto i=g_recv.begin(); i!=g_recv.end(); i++)
		i->freemem();
#else
	g_outgo.clear();
	g_recv.clear();
#endif
}

#endif