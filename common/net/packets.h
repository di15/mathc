

#ifndef PACKETS_H
#define PACKETS_H

#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../math/vec2i.h"
#include "../math/camera.h"

class NetConn;

class OldPacket
{
public:
	char* buffer;
	int len;
	unsigned long long last;		//last time resent
	unsigned long long first;	//first time sent
	bool expires;

	//sender/reciever
	IPaddress addr;
	void (*onackfunc)(OldPacket* op, NetConn* nc);

	void freemem()
	{
		if(len <= 0)
			return;

		if(buffer != NULL)
			delete [] buffer;
		buffer = NULL;
	}

	OldPacket()
	{
		len = 0;
		buffer = NULL;
		onackfunc = NULL;
	}
	~OldPacket()
	{
		freemem();
	}

	OldPacket(const OldPacket& original)
	{
		len = 0;
		buffer = NULL;
		*this = original;
	}

	OldPacket& operator=(const OldPacket &original)
	{
		if(original.buffer && original.len > 0)
		{
			len = original.len;
			if(len > 0)
			{
				buffer = new char[len];
				memcpy((void*)buffer, (void*)original.buffer, len);
			}
			last = original.last;
			first = original.first;
			expires = original.expires;
			addr = original.addr;
			onackfunc = original.onackfunc;
#ifdef MATCHMAKER
			//ipaddr = original.ipaddr;
			//port = original.port;
			//memcpy((void*)&addr, (void*)&original.addr, sizeof(struct sockaddr_in));
#endif
		}
		else
		{
			buffer = NULL;
			len = 0;
			onackfunc = NULL;
		}

		return *this;
	}
};

//TODO merge some of these into multi-purpose packet types

#define	PACKET_NULL						0
#define PACKET_DISCONNECT				1
#define PACKET_CONNECT					2
#define	PACKET_ACKNOWLEDGMENT			3
#define PACKET_PLACEBL					4
#define PACKET_NETTURN					5
#define PACKET_DONETURN					6
#define PACKET_JOIN						7
#define PACKET_ADDSV					8
#define PACKET_ADDEDSV					9
#define PACKET_KEEPALIVE				10
#define PACKET_GETSVLIST				11
#define PACKET_SVADDR					12
#define PACKET_SVINFO					13
#define PACKET_GETSVINFO				14
#define PACKET_SENDNEXTHOST				15
#define PACKET_NOMOREHOSTS				16
#define PACKET_ADDCLIENT				17
#define PACKET_SELFCLIENT				18
#define PACKET_SETCLNAME				19
#define PACKET_CLIENTLEFT				20
#define PACKET_CLIENTROLE				21
#define PACKET_DONEJOIN					22
#define PACKET_TOOMANYCL				23
#define PACKET_MAPCHANGE				24
#define PACKET_CHVAL					25
#define PACKET_CLDISCONNECTED			26
#define PACKET_CLSTATE					27
#define PACKET_NOCONN					28

// byte-align structures
#pragma pack(push, 1)

struct PacketHeader
{
	unsigned short type;
	unsigned short ack;
};

struct BasePacket
{
	PacketHeader header;
};

typedef BasePacket NoConnectionPacket;
typedef BasePacket DoneJoinPacket;
typedef BasePacket TooManyClPacket;
typedef BasePacket SendNextHostPacket;
typedef BasePacket NoMoreHostsPacket;
typedef BasePacket GetSvInfoPacket;
typedef BasePacket GetSvListPacket;
typedef BasePacket KeepAlivePacket;
typedef BasePacket AddSvPacket;
typedef BasePacket AddedSvPacket;
typedef BasePacket AckPacket;

#define CLCH_UNRESP			0	//client became unresponsive
#define CLCH_RESP			1	//became responsive again
#define CLCH_PING			2

struct ClStatePacket
{
	PacketHeader header;
	unsigned char chtype;
	short client;
	float ping;
};

struct ClDisconnectedPacket
{
	PacketHeader header;
	short client;
};

#define CHVAL_BLPRICE					0
#define CHVAL_BLWAGE					1
#define CHVAL_TRWAGE					2
#define CHVAL_TRPRICE					3
#define CHVAL_CSTWAGE					4
#define CHVAL_PRODLEV					5
#define CHVAL_CDWAGE					6
#define CHVAL_MANPRICE					7

struct ChValPacket
{
	PacketHeader header;
	unsigned char chtype;
	int value;
	unsigned char player;
	unsigned char res;
	unsigned short bi;
	unsigned char x;
	unsigned char y;
	unsigned char cdtype;
	unsigned char utype;
};

//not counting null terminator
#define MAPNAME_LEN		63
#define SVNAME_LEN		63
#define PYNAME_LEN		63

struct MapChangePacket
{
	PacketHeader header;
	char map[MAPNAME_LEN+1];
};

struct JoinPacket
{
	PacketHeader header;
	char name[PYNAME_LEN+1];
};

struct AddClientPacket
{
	PacketHeader header;
	signed char client;
	signed char player;
	char name[PYNAME_LEN+1];
};

struct SelfClientPacket
{
	PacketHeader header;
	int client;
};

struct SetClNamePacket
{
	PacketHeader header;
	int client;
	char name[PYNAME_LEN+1];
};

struct ClientLeftPacket
{
	PacketHeader header;
	int client;
};

struct ClientRolePacket
{
	PacketHeader header;
	signed char client;
	signed char player;
};

struct SvAddrPacket
{
	PacketHeader header;
	IPaddress addr;
};

class SendSvInfo	//sendable
{
public:
	IPaddress addr;
	char svname[SVNAME_LEN+1];
	short nplayers;
	char mapname[MAPNAME_LEN+1];
};

struct SvInfoPacket
{
	PacketHeader header;
	SendSvInfo svinfo;
};

struct NetTurnPacket
{
	PacketHeader header;
	unsigned int fornetfr;	//for net fr #..
	unsigned short loadsz;
	//commands go after
};

struct DoneTurnPacket
{
	PacketHeader header;
	unsigned int fornetfr;	//for net fr #..
	short player;	//should match sender
};

struct PlaceBlPacket
{
	PacketHeader header;
	int btype;
	Vec2i tpos;
	int player;
};

struct ConnectPacket
{
	PacketHeader header;
	bool reply;
};

struct DisconnectPacket
{
	PacketHeader header;
	bool reply;
};

// Default alignment
#pragma pack(pop)

#endif



