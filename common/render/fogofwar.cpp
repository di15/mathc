

#include "fogofwar.h"
#include "../sim/utype.h"
#include "../sim/unit.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "heightmap.h"
#include "../utils.h"
#include "../math/fixmath.h"

VisTile* g_vistile = NULL;


void AddVis(Unit* u)
{
	UType* ut = &g_utype[u->type];
	int vr = ut->visrange;
	int cmminx = u->cmpos.x - vr;
	int cmminy = u->cmpos.y - vr;
	int cmmaxx = u->cmpos.x + vr;
	int cmmaxy = u->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_hmap.m_widthx-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_hmap.m_widthy-1, cmmaxy / TILE_SIZE);

	///g_log<<"cymmp "<<u->cmpos.y<<","<<vr<<std::endl;
	//g_log<<"cymm "<<cmminy<<","<<cmmaxy<<std::endl;
	//g_log<<"tymm "<<tminy<<","<<tmaxy<<std::endl;

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the 
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;
			
			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			int ui = u - g_unit;
			VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];
			v->uvis.push_back( ui );

			//g_log<<"add vis "<<tx<<","<<ty<<std::endl;
		}
}

void RemVis(Unit* u)
{
	UType* ut = &g_utype[u->type];
	int vr = ut->visrange;
	int cmminx = u->cmpos.x - vr;
	int cmminy = u->cmpos.y - vr;
	int cmmaxx = u->cmpos.x + vr;
	int cmmaxy = u->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_hmap.m_widthx-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_hmap.m_widthy-1, cmmaxy / TILE_SIZE);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the 
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;
			
			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];
			int ui = u - g_unit;

			auto vit=v->uvis.begin();
			while(vit!=v->uvis.end())
			{
				if(*vit == ui)
				{
					vit = v->uvis.erase(vit);
					//g_log<<"rem vis "<<tx<<","<<ty<<std::endl;
					continue;
				}

				vit++;
			}
		}
}

void AddVis(Building* b)
{
	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tilepos.x * TILE_SIZE + ((bt->widthx % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tilepos.y * TILE_SIZE + ((bt->widthy % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_hmap.m_widthx-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_hmap.m_widthy-1, cmmaxy / TILE_SIZE);

	//g_log<<"bav "<<bcmpos.x<<","<<bcmpos.y<<std::endl;

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the 
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - bcmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - bcmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - bcmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - bcmpos.y;
			
			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			int bi = b - g_building;
			VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];
			v->bvis.push_back( bi );

			//g_log<<"bav "<<tx<<","<<ty<<std::endl;
		}
}

void RemVis(Building* b)
{
	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tilepos.x * TILE_SIZE + ((bt->widthx % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tilepos.y * TILE_SIZE + ((bt->widthy % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_hmap.m_widthx-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_hmap.m_widthy-1, cmmaxy / TILE_SIZE);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the 
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - bcmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - bcmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - bcmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - bcmpos.y;
			
			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];
			int bi = b - g_building;

			auto vit=v->bvis.begin();
			while(vit!=v->bvis.end())
			{
				if(*vit == bi)
				{
					vit = v->bvis.erase(vit);
					continue;
				}

				vit++;
			}
		}
}

void Explore(Unit* u)
{
	UType* ut = &g_utype[u->type];
	int vr = ut->visrange;
	int cmminx = u->cmpos.x - vr;
	int cmminy = u->cmpos.y - vr;
	int cmmaxx = u->cmpos.x + vr;
	int cmmaxy = u->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_hmap.m_widthx-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_hmap.m_widthy-1, cmmaxy / TILE_SIZE);

	///g_log<<"cymmp "<<u->cmpos.y<<","<<vr<<std::endl;
	//g_log<<"cymm "<<cmminy<<","<<cmmaxy<<std::endl;
	//g_log<<"tymm "<<tminy<<","<<tmaxy<<std::endl;

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the 
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - u->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - u->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - u->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - u->cmpos.y;
			
			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			int ui = u - g_unit;
			VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];
			v->explored[u->owner] = true;

			//g_log<<"add vis "<<tx<<","<<ty<<std::endl;
		}
}

void Explore(Building* b)
{	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tilepos.x * TILE_SIZE + ((bt->widthx % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tilepos.y * TILE_SIZE + ((bt->widthy % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_hmap.m_widthx-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_hmap.m_widthy-1, cmmaxy / TILE_SIZE);

	//g_log<<"bav "<<bcmpos.x<<","<<bcmpos.y<<std::endl;

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the 
			//tile we're measuring from.
			int dcmx1 = labs(tx * TILE_SIZE) - bcmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - bcmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - bcmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - bcmpos.y;
			
			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);

			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;

			//visible
			int bi = b - g_building;
			VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];
			v->explored[b->owner] = true;

			//g_log<<"bav "<<tx<<","<<ty<<std::endl;
		}
}

bool IsTileVis(short py, short tx, short ty)
{
	VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];

	for(auto vit=v->bvis.begin(); vit!=v->bvis.end(); vit++)
	{
		Building* b = &g_building[ *vit ];

		if(b->owner == py)
			return true;
	}

	for(auto vit=v->uvis.begin(); vit!=v->uvis.end(); vit++)
	{
		Unit* u = &g_unit[ *vit ];

		//InfoMess("v","v");
		if(u->owner == py)
			return true;
	}

	return false;
}

bool Explored(short py, short tx, short ty)
{	
	VisTile* v = &g_vistile[ tx + ty * g_hmap.m_widthx ];
	return v->explored[py];
}