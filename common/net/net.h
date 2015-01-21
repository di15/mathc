

#ifndef NET_H
#define NET_H

#include "../platform.h"
#include "packets.h"

#define PORT		50400
//#define SV_ADDR		"corpstates.com"	//live server
#define SV_ADDR			"23.226.224.175"		//vps
//#define SV_ADDR		"54.221.229.124"	//corp1 aws
//#define SV_ADDR			"192.168.1.100"		//home local server ip
//#define SV_ADDR			"192.168.1.103"		//home local server ip
//#define SV_ADDR			"174.6.61.178"		//home public server ip

#define RESEND_DELAY	200
#define RESEND_EXPIRE	(5000)
#ifdef MATCHMAKER
#define NETCONN_TIMEOUT		(60*1000)
#else
#define NETCONN_TIMEOUT		(60*1000)
#endif
#define NETCONN_UNRESP	(2*1000)
#define QUIT_DELAY		(10*1000)

unsigned short NextAck(unsigned short ack);
unsigned short PrevAck(unsigned short ack);
bool PastAck(unsigned short test, unsigned short current);
bool PastFr(unsigned int test, unsigned int current);

extern unsigned long long g_lastS;  //last sent
extern unsigned long long g_lastR;  //last recieved

extern int g_netmode;

extern char g_mapname[MAPNAME_LEN+1];
extern char g_svname[SVNAME_LEN+1];

#ifdef MATCHMAKER
extern unsigned int g_transmitted;
#endif

#define NETM_SINGLE			0	//single player
#define NETM_HOST			1	//hosting
#define NETM_CLIENT			2	//client
//#define NET_SPECT			3	//spectator client

#define NET_DEBUG	//debug messages for packets recvd

void UpdNet();
void ClearPackets();
void CheckAddSv();
bool Same(IPaddress* a, IPaddress* b);
bool NetQuit();

#ifndef MATCHMAKER
extern unsigned int g_transmitted;
#endif

#endif

