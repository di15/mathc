

#ifndef NETCONN_H
#define NETCONN_H

#include "../platform.h"
#include "net.h"
#include "packets.h"

extern UDPsocket g_sock;
extern bool g_sentsvinfo;
//extern bool g_needsvlist;
//extern bool g_reqdsvlist;

extern std::list<OldPacket> g_outgo;
extern std::list<OldPacket> g_recv;

class NetConn
{
public:
	unsigned short sendack;
	unsigned short recvack;
	bool handshook;
	IPaddress addr;
	//TODO change these to flags
	bool isclient;	//is this a hosted game's client? or for MATCHMAKER, is this somebody requesting sv list?
	bool isourhost;	//is this the currently joined game's host? cannot be a host from a server list or something. for MATCHMAKER, it can be a host getting added to sv list.
	bool ismatcher;	//matchmaker?
	bool ishostinfo;	//is this a host we're just getting info from for our sv list?
	bool isunresponsive;
	unsigned long long lastsent;
	unsigned long long lastrecv;
	short client;
	float ping;
	bool closed;
	
	void expirein(int millis);

#ifdef MATCHMAKER
	int svlistoff;	//offset in server list, sending a few at a time
	SendSvInfo svinfo;
#endif
	//void (*chcallback)(NetConn* nc, bool success);	//connection state change callback - did we connect successfully or time out?

	NetConn()
	{
		client = -1;
		handshook = false;
		sendack = 0;
		//important - reply ConnectPacket with ack=0 will be 
		//ignored as copy (even though it is original) if new NetConn's recvack=0.
		recvack = USHRT_MAX;
		isclient = false;
		isourhost = false;
		ismatcher = false;
		ishostinfo = false;
		isunresponsive = false;
		lastrecv = GetTickCount64();
		lastsent = GetTickCount64();
		//chcallback = NULL;
#ifdef MATCHMAKER
		svlistoff = -1;
#endif
		ping = 1;
		closed = false;
	}
};

extern std::list<NetConn> g_conn;
extern NetConn* g_svconn;
extern NetConn* g_mmconn;	//matchmaker

NetConn* Match(IPaddress* addr);
void EndSess();	//used to clear cmd queues
void BegSess();
NetConn* Connect(const char* addrstr, unsigned short port, bool ismatcher, bool isourhost, bool isclient, bool ishostinfo);
NetConn* Connect(IPaddress* ip, bool ismatcher, bool isourhost, bool isclient, bool ishostinfo);
void CheckConns();
void FlushPrev(IPaddress* from);
void KeepAlive();
void OpenSock();
void Disconnect(NetConn* nc);

#endif