

#ifndef FILLBODIES_H
#define FILLBODIES_H

#include "pathnode.h"

void FillBodies();

class TileRegs
{
public:

	class TilePass
	{
	public:
		unsigned short from;
		unsigned short to;
	};

	std::list<TilePass> tregs[SDIRS];

	void add(unsigned char sdir, unsigned short from, unsigned short to);
};

extern TileRegs* g_tilepass;
extern unsigned char* g_tregs;

// byte-align structures
#pragma pack(push, 1)

class TileNode
{
public:
	unsigned short score;
	//short nx;
	//short nz;
	unsigned short totalD;
	//unsigned char expansion;
	TileNode* previous;
	//bool tried;
	std::list<unsigned short> tregs;
	bool opened;
	bool closed;
	unsigned char jams;
	TileNode()
	{
		//tried = false;
		previous = NULL;
		opened = false;
		closed = false;
		jams = 0;
	};
	TileNode(int startx, int startz, int endx, int endz, int nx, int nz, TileNode* prev, int totalD, int stepD, unsigned short treg);
	//PathNode(int startx, int startz, int endx, int endz, int nx, int nz, PathNode* prev, int totalD, int stepD, unsigned char expan);
};

#pragma pack(pop)

extern TileNode* g_tilenode;

#endif