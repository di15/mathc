

#include "client.h"

Client g_client[CLIENTS];
int g_localC;
RichText g_name = RichText("Player");

void ResetClients()
{
	g_localC = -1;
	g_localP = -1;

	for(int i=0; i<CLIENTS; i++)
	{
		Client* c = &g_client[i];

		c->on = false;
		c->name = RichText("Player");
		//c->color = 0;
		c->nc = NULL;
	}

	for(int i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];

		py->client = -1;
	}
}

int NewClient()
{
	for(int i=0; i<CLIENTS; i++)
		if(!g_client[i].on)
			return i;

	return -1;
}

bool AddClient(NetConn* nc, RichText name, int* retci)
{
	int ci = NewClient();

	if(ci < 0)
		return false;

	if(nc)
		nc->client = ci;

	Client* c = &g_client[ci];

	c->on = true;
	c->player = -1;
	c->name = name;
	c->nc = nc;

	if(retci)
		*retci = ci;

	return true;
}