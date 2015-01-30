


#include "net.h"
#include "sendpackets.h"
#include "../sim/player.h"
#include "packets.h"
#include "../sim/simdef.h"
#include "lockstep.h"

void SendAll(char* data, int size, bool reliable, bool expires, IPaddress* exception)
{
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
	{
		//if(ci->ctype != CONN_CLIENT)
		//	continue;

		if(!ci->isclient)
			continue;

		if(exception &&
			//memcmp(&ci->addr, exception, sizeof(IPaddress)) == 0
				Same(&ci->addr, exception))
			continue;

		//InfoMess("sa", "sa");

		SendData(data, size, &ci->addr, reliable, expires, &*ci, &g_sock, 0, NULL);
	}
}

void SendData(char* data, int size, IPaddress * paddr, bool reliable, bool expires, NetConn* nc, UDPsocket* sock, int msdelay, void (*onackfunc)(OldPacket* p, NetConn* nc))
{
	UDPpacket *out = SDLNet_AllocPacket(65535);

	g_lastS = GetTickCount64();

#if 0
	if(((PacketHeader*)data)->type == PACKET_NETTURN)
	{
		char msg[128];
		sprintf(msg, "send netturn for netfr%u load=%u", ((NetTurnPacket*)data)->fornetfr, (unsigned int)((NetTurnPacket*)data)->loadsz);
		//InfoMess("s", msg);
		g_log<<msg<<std::endl;
		FILE* fp = fopen("sntp.txt", "wb");
		fwrite(msg, strlen(msg)+1, 1, fp);
		fclose(fp);
	}
#endif

	if(reliable)
	{
		//InfoMess("sa", "s2");
		((PacketHeader*)data)->ack = nc->sendack;
		OldPacket p;
		p.buffer = new char[ size ];
		p.len = size;
		memcpy(p.buffer, data, size);
		memcpy((void*)&p.addr, (void*)paddr, sizeof(IPaddress));
		//in msdelay milliseconds, p.last will be RESEND_DELAY millisecs behind GetTickCount64()
		p.last = GetTickCount64() + msdelay - RESEND_DELAY;
		p.first = p.last;
		p.expires = expires;
		p.onackfunc = onackfunc;
		g_outgo.push_back(p);
		nc->sendack = NextAck(nc->sendack);
	}

#ifdef NET_DEBUG
	
	//unsigned int ipaddr = SDL_SwapBE32(ip.host);
	//unsigned short port = SDL_SwapBE16(ip.port);
	if(paddr)
	{
		g_log<<"send to "<<SDL_SwapBE32(paddr->host)<<":"<<SDL_SwapBE16(paddr->port)<<" at tick "<<SDL_GetTicks()<<" ack"<<((PacketHeader*)data)->ack<<" t"<<((PacketHeader*)data)->type<<std::endl;
		g_log.flush();
	}

	int nhs = 0;
	for(auto ci=g_conn.begin(); ci!=g_conn.end(); ci++)
		if(ci->handshook)
			nhs++;

	g_log<<"g_conn.sz = "<<g_conn.size()<<" numhandshook="<<nhs<<std::endl;
	g_log.flush();
#endif

	if(reliable && msdelay > 0)
		return;

	memcpy(out->data, data, size);
	out->len = size;
	//out->data[size] = 0;


	//if(bindaddr)
	{
		SDLNet_UDP_Unbind(*sock, 0);
		if(SDLNet_UDP_Bind(*sock, 0, (const IPaddress*)paddr) == -1)
		{
			char msg[1280];
			sprintf(msg, "SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
			ErrMess("Error", msg);
			//printf("SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
			//exit(7);
		}
	}

#if 0
	g_log<<"send t"<<((PacketHeader*)data)->type<<std::endl;
	g_log.flush();

			char msg[128];
			sprintf(msg, "send ack%u t%d", (unsigned int)((PacketHeader*)out->data)->ack, ((PacketHeader*)out->data)->type);
			InfoMess("send", msg);
#endif

	//sendto(g_socket, data, size, 0, (struct addr *)paddr, sizeof(struct sockaddr_in));
	SDLNet_UDP_Send(*sock, 0, out);
	
//#ifdef MATCHMAKER
#if 1
	g_transmitted += size;
#endif

	SDLNet_FreePacket(out);
}

void ResendPacks()
{
	OldPacket* p;
	unsigned long long now = GetTickCount64();
	//unsigned long long due = now - RESEND_DELAY;
	//unsigned long long expire = now - RESEND_EXPIRE;

	auto i=g_outgo.begin();
	while(i!=g_outgo.end())
	{
		p = &*i;

		unsigned long long passed = now - p->last;
		
		NetConn* nc = Match(&p->addr);

#if 1
		//increasing resend delay for the same outgoing packet

		unsigned int nextdelay = RESEND_DELAY;
		unsigned long long firstpassed = now - p->first;

		if(nc && firstpassed >= RESEND_DELAY)
		{
			nextdelay = firstpassed;
		}
#endif

		//if(p->last > due)
		//if((nc && passed > (unsigned int)(0.8f * nc->ping)) || (!nc && passed > RESEND_DELAY))
		//if(passed > RESEND_DELAY)
		if(passed > nextdelay)
		{
			i++;
			continue;
		}
		
		//if(p->expires && p->first < expire)
		if(p->expires && now - p->first > RESEND_EXPIRE)
		{
			//i->freemem();
			i = g_outgo.erase(i);

#if 0
			g_log<<"expire at "<<DateTime()<<" dt="<<expire<<"-"<<p->first<<"="<<(expire-p->first)<<"(>"<<RESEND_EXPIRE<<") left = "<<g_outgo.size()<<endl;
			g_log.flush();
#endif

			continue;
		}

#ifdef NET_DEBUG
		g_log<<"\t resend...";
		g_log.flush();
#endif

		SendData(p->buffer, p->len, &p->addr, false, p->expires, nc, &g_sock, 0, NULL);

		p->last = now;
#ifdef _IOS
		NSLog(@"Resent at %lld", now);
#endif

#if 0
		g_log<<"resent at "<<DateTime()<<" left = "<<g_outgo.size()<<endl;
		g_log.flush();
#endif

		i++;
	}
}

void Acknowledge(unsigned short ack, NetConn* nc, IPaddress* addr, UDPsocket* sock, char* buffer, int bytes)
{
#if 0	//actually, don't mess with RUDP protocol; build it on top of packets
	//if it's a NetTurnPacket, we have to do something special...
	if(((PacketHeader*)buffer)->type == PACKET_NETTURN)
	{
		NetTurnPacket* ntp = (NetTurnPacket*)buffer;
		
		//if it's not for the next turn, drop the ack so the server waits for us to complete up to the necessary turn
		if(ntp->fornetfr != (g_netframe / NETTURN + 1) * NETTURN)
		{
			char msg[128];
			sprintf(msg, "faulty turn fr%u", ntp->fornetfr);
			InfoMess("Er", msg);
			return;
		}
	}
#endif

	AckPacket p;
	p.header.type = PACKET_ACKNOWLEDGMENT;
	p.header.ack = ack;

#if 0
	g_log<<"ack "<<ack<<std::endl;
	g_log.flush();
#endif

	SendData((char*)&p, sizeof(AckPacket), addr, false, true, nc, sock, 0, NULL);
}




