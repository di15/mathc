
#include "fillbodies.h"
#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../sim/road.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../phys/trace.h"
#include "../sys/binheap.h"
#include "anypath.h"
#include "reconstructpath.h"
#include "pathdebug.h"

TileRegs* g_tilepass = NULL;
unsigned char* g_tregs = NULL;
TileNode* g_tilenode = NULL;

void TileRegs::add(unsigned char sdir, unsigned short from, unsigned short to)
{
	for(auto rit=tregs[sdir].begin(); rit!=tregs[sdir].end(); rit++)
	{
		if(rit->from == from && rit->to == to)
			return;
	}

	TileRegs::TilePass pass = {from,to};
	tregs[sdir].push_back(pass);
}

bool BodyStandable(const int nx, const int nz, const unsigned short cmsize)
{
#if 0
	if(nx < 0 || nz < 0 || nx >= g_pathdim.x || nz >= g_pathdim.y)
		return false;
#endif

	ColliderTile* cell = ColliderAt( nx, nz );

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"stand? u"<<pj->thisu<<" n"<<nx<<","<<nz<<" dn("<<(nx-pj->goalx)<<","<<(nz-pj->goalz)<<")"<<std::endl;

		char add[1280];
		sprintf(add, "std? u%d n%d,%d dn%d,%d \r\n", (int)pj->thisu, (int)nx, (int)nz, (int)(nx-pj->goalx), (int)(nz-pj->goalz));
		powcdstr += add;
	}
#endif

	if(cell->flags & FLAG_ABRUPT)
		return false;

	const int cmposx = nx * PATHNODE_SIZE + PATHNODE_SIZE/2;
	const int cmposz = nz * PATHNODE_SIZE + PATHNODE_SIZE/2;

	const int cmminx = cmposx - cmsize/2;
	const int cmminz = cmposz - cmsize/2;
	const int cmmaxx = cmminx + cmsize - 1;
	const int cmmaxz = cmminz + cmsize - 1;

	const int cminx = cmminx/PATHNODE_SIZE;
	const int cminz = cmminz/PATHNODE_SIZE;
	const int cmaxx = cmmaxx/PATHNODE_SIZE;
	const int cmaxz = cmmaxz/PATHNODE_SIZE;

#if 0
	if(cminx < 0 || cminz < 0 || cmaxx >= g_pathdim.x || cmaxz >= g_pathdim.y)
	{
		return false;
	}
#endif
	
#if 0
	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;
#endif

	for(int z=cminz; z<=cmaxz; z++)
		for(int x=cminx; x<=cmaxx; x++)
		{
			cell = ColliderAt(x, z);

			if(cell->foliage != USHRT_MAX)
				//collided = true;
				return false;

#if 0
			if(cell->building >= 0)
			{
				collided = true;
			}
#endif
		}

#if 0
	if(collided && !ignoredu && !ignoredb)
		return false;
#endif

	return true;
}

void FillBodies()
{
	//g_log<<"fillb"<<std::endl;

	//TODO use bitfield? 
	//Need to determine how many bits needed for max possible regions in node.
	//tile regions

	for(int i=0; i<g_pathdim.x*g_pathdim.y; i++)
		g_tregs[i] = UCHAR_MAX;

	for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthy; i++)
	{
		g_tilepass[i].tregs[0].clear();
		g_tilepass[i].tregs[1].clear();
		g_tilepass[i].tregs[2].clear();
		g_tilepass[i].tregs[3].clear();
	}

#define TILE_PATHNODES	(TILE_SIZE / PATHNODE_SIZE)

	//Mark flood-fill regions for each tile.
	//This must be done on a tile-by-tile basis
	//(the tile-to-tile region passability is determined next).
	for(unsigned char ty=0; ty<g_hmap.m_widthy; ty++)
		for(unsigned char tx=0; tx<g_hmap.m_widthx; tx++)
		{
			unsigned char curbody = 0;

			//For each pathnode in this tile,
			//check if it is standable for land or water.
			//Ignore buildings.
			//There must be LESS than UCHAR_MAX pathnodes in a tile.
			//UCHAR_MAX means impassable or not checked.
			//If there's 20x20 pathnodes in a tile I should use ushorts.
			for(unsigned short ny=ty*TILE_PATHNODES; ny<(ty+1)*TILE_PATHNODES; ny++)
				for(unsigned short nx=tx*TILE_PATHNODES; nx<(tx+1)*TILE_PATHNODES; nx++)
				{
					//node index
					int nin = nx + ny * g_pathdim.x;

					if(!BodyStandable(nx, ny, PATHNODE_SIZE))
						continue;

					g_tregs[nin] = curbody;
					curbody++;
				}

			bool change;

			//Merge passable regions.
			do
			{
				change = false;

				for(unsigned short ny=ty*TILE_PATHNODES; ny<(ty+1)*TILE_PATHNODES; ny++)
					for(unsigned short nx=tx*TILE_PATHNODES; nx<(tx+1)*TILE_PATHNODES; nx++)
					{
						//node index
						int nin = nx + ny * g_pathdim.x;

						if(g_tregs[nin] == UCHAR_MAX)
							continue;

						//is there a pathnode in this tile at [nx+1]?
						if(nx < (tx+1)*TILE_PATHNODES - 1)
						{
							int nin2 = (nx+1) + ny * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}

						//is there a pathnode in this tile at [nx-1]?
						if(nx > tx*TILE_PATHNODES)
						{
							int nin2 = (nx-1) + ny * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}

						//is there a pathnode in this tile at [ny+1]?
						if(ny < (ty+1)*TILE_PATHNODES - 1)
						{
							int nin2 = nx + (ny+1) * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}

						//is there a pathnode in this tile at [ny-1]?
						if(ny > ty*TILE_PATHNODES)
						{
							int nin2 = nx + (ny-1) * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}
					}
			}while(change);
		}

	//Now determine passability between regions
	//in neighbouring tiles.
	//TODO
	for(unsigned char ty=0; ty<g_hmap.m_widthy; ty++)
		for(unsigned char tx=0; tx<g_hmap.m_widthx; tx++)
		{
			//Go along each side.
			//По каждой стороне проходим.
#if 0
			for(unsigned char sdir=0; sdir<SDIRS; sdir++)
			{
				Vec2s off = straightoffsets[sdir];
				//pathnode coords
				unsigned char n[2];
				//component
				for(unsigned char c=0; c<2; c++)
				{
					if(off[c] != 0)
						continue;

					unsigned char c2 = (c+1)%2;

					//for(unsigned short ny=ty*TILE_PATHNODES; ny<(ty+1)*TILE_PATHNODES; ny++)
					//for(unsigned short nx=tx*TILE_PATHNODES; nx<(tx+1)*TILE_PATHNODES; nx++)
					{
						unsigned short nx = tx*TILE_PATHNODES;

						//node index
						int nin = nx + ny * g_pathdim.x;
						int nin2 = (nx-1) + ny * g_pathdim.x;

						if(g_tregs[nin] == UCHAR_MAX)
							continue;

						if(g_tregs[nin2] == UCHAR_MAX)
							continue;

						//tilepass index
						int tpin = tx + ty * g_hmap.m_widthx;
						g_tilepass[tpin].add(sidr, g_tregs[nin2]);
					}
				}
			}
#else
			//Go along left side.
			if(tx > 0)
				for(unsigned short ny=ty*TILE_PATHNODES; ny<(ty+1)*TILE_PATHNODES; ny++)
				//for(unsigned short nx=tx*TILE_PATHNODES; nx<(tx+1)*TILE_PATHNODES; nx++)
				{
					unsigned short nx = tx*TILE_PATHNODES;
					//unsigned short ny = (ty-off.y)*TILE_PATHNODES;

					//node index
					int nin = nx + ny * g_pathdim.x;
					int nin2 = (nx-1) + ny * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int tpin = tx + ty * g_hmap.m_widthx;
					g_tilepass[tpin].add(SDIR_W, g_tregs[nin], g_tregs[nin2]);

					//g_log<<"add1"<<std::endl;
				}

			//Go along right side.
			if(tx < g_hmap.m_widthx-1)
				for(unsigned short ny=ty*TILE_PATHNODES; ny<(ty+1)*TILE_PATHNODES; ny++)
				//for(unsigned short nx=tx*TILE_PATHNODES; nx<(tx+1)*TILE_PATHNODES; nx++)
				{
					unsigned short nx = (tx+1)*TILE_PATHNODES-1;
					//unsigned short ny = (ty-off.y)*TILE_PATHNODES;

					//node index
					int nin = nx + ny * g_pathdim.x;
					int nin2 = (nx+1) + ny * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int tpin = tx + ty * g_hmap.m_widthx;
					g_tilepass[tpin].add(SDIR_E, g_tregs[nin], g_tregs[nin2]);

					//g_log<<"add2"<<std::endl;
				}

			//Go along top side.
			if(ty > 0)
				//for(unsigned short ny=ty*TILE_PATHNODES; ny<(ty+1)*TILE_PATHNODES; ny++)
				for(unsigned short nx=tx*TILE_PATHNODES; nx<(tx+1)*TILE_PATHNODES; nx++)
				{
					//unsigned short nx = tx*TILE_PATHNODES;
					unsigned short ny = ty*TILE_PATHNODES;

					//node index
					int nin = nx + ny * g_pathdim.x;
					int nin2 = nx + (ny-1) * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int tpin = tx + ty * g_hmap.m_widthx;
					g_tilepass[tpin].add(SDIR_N, g_tregs[nin], g_tregs[nin2]);

					//g_log<<"add3"<<std::endl;
				}

			//Go along bottom side.
			if(ty < g_hmap.m_widthy-1)
				//for(unsigned short ny=ty*TILE_PATHNODES; ny<(ty+1)*TILE_PATHNODES; ny++)
				for(unsigned short nx=tx*TILE_PATHNODES; nx<(tx+1)*TILE_PATHNODES; nx++)
				{
					//unsigned short nx = (tx+1)*TILE_PATHNODES-1;
					unsigned short ny = (ty+1)*TILE_PATHNODES-1;

					//node index
					int nin = nx + ny * g_pathdim.x;
					int nin2 = nx + (ny+1) * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int tpin = tx + ty * g_hmap.m_widthx;
					g_tilepass[tpin].add(SDIR_S, g_tregs[nin], g_tregs[nin2]);

					//g_log<<"add4"<<std::endl;
				}
#endif
		}

#undef TILE_PATHNODES

	//This must be done everytime forests are changed
	//or something.
}