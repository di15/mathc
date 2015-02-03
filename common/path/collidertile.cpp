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
#include "pathnode.h"
#include "../math/vec2i.h"
#include "pathdebug.h"
#include "pathjob.h"
#include "../debug.h"
#include "../sim/conduit.h"
#include "../sim/transport.h"
#include "../phys/collision.h"
#include "fillbodies.h"
#include "tilepath.h"

//not engine
#include "../../game/gui/chattext.h"

ColliderTile *g_collidertile = NULL;

ColliderTile::ColliderTile()
{
#if 0
	bool hasroad;
	bool hasland;
	bool haswater;
	bool abrupt;	//abrupt incline?
	std::list<int> units;
	std::list<int> foliage;
	int building;
#endif

	//hasroad = false;
	//hasland = false;
	//haswater = false;
	//abrupt = false;
	flags = 0;
	building = -1;
	for(int i=0; i<MAX_COLLIDER_UNITS; i++)
		units[i] = -1;
	foliage = USHRT_MAX;
}

inline Vec2i PathNodePos(int cmposx, int cmposz)
{
	return Vec2i(cmposx/PATHNODE_SIZE, cmposz/PATHNODE_SIZE);
}

void FreePathGrid()
{
	g_log<<"free path gr"<<std::endl;

	if(g_collidertile)
	{
		delete [] g_collidertile;
		g_collidertile = NULL;
	}

	g_pathdim = Vec2i(0,0);

	if(g_pathnode)
	{
		delete [] g_pathnode;
		g_pathnode = NULL;
	}

	g_openlist.freemem();

	if(g_tilepass)
	{
		delete [] g_tilepass;
		g_tilepass = NULL;
	}

	if(g_tregs)
	{
		delete [] g_tregs;
		g_tregs = NULL;
	}

	if(g_tilenode)
	{
		delete [] g_tilenode;
		g_tilenode = NULL;
	}
}

void AllocPathGrid(int cmwx, int cmwz)
{
	FreePathGrid();
	g_pathdim.x = cmwx / PATHNODE_SIZE;
	g_pathdim.y = cmwz / PATHNODE_SIZE;
	g_collidertile = new ColliderTile [ g_pathdim.x * g_pathdim.y ];

	g_log<<"path gr allc "<<g_pathdim.x<<","<<g_pathdim.y<<std::endl;

	int cwx = g_pathdim.x;
	int cwz = g_pathdim.y;

	g_pathnode = new PathNode [ cwx * cwz ];
	if(!g_pathnode) OutOfMem(__FILE__, __LINE__);

	g_openlist.alloc( cwx * cwz );

	for(int x=0; x<cwx; x++)
		for(int z=0; z<cwz; z++)
		{
			PathNode* n = PathNodeAt(x, z);
			//n->nx = x;
			//n->nz = z;
			n->opened = false;
			n->closed = false;
		}

	g_tilepass = new TileRegs [ (cmwx / TILE_SIZE) * (cmwz / TILE_SIZE) ];
	if(!g_tilepass) OutOfMem(__FILE__, __LINE__);

	g_tregs = new unsigned char [ g_pathdim.x * g_pathdim.y ];
	if(!g_tregs) OutOfMem(__FILE__, __LINE__);

	g_tilenode = new TileNode [ (cmwx / TILE_SIZE) * (cmwz / TILE_SIZE) ];
	if(!g_tilenode) OutOfMem(__FILE__, __LINE__);

	//g_lastpath = g_simframe;
}

ColliderTile* ColliderAt(int nx, int nz)
{
	return &g_collidertile[ PathNodeIndex(nx, nz) ];
}

void FillColliderGrid()
{
	const int cwx = g_pathdim.x;
	const int cwz = g_pathdim.y;

	//g_log<<"path gr "<<cwx<<","<<cwz<<std::endl;

	for(int x=0; x<cwx; x++)
		for(int z=0; z<cwz; z++)
		{
			int cmx = x*PATHNODE_SIZE + PATHNODE_SIZE/2;
			int cmz = z*PATHNODE_SIZE + PATHNODE_SIZE/2;
			ColliderTile* cell = ColliderAt(x, z);

			//g_log<<"cell "<<x<<","<<z<<" cmpos="<<cmx<<","<<cmz<<" y="<<g_hmap.accheight(cmx, cmz)<<std::endl;

			if(AtLand(cmx, cmz))
			{
				//cell->hasland = true;
				cell->flags |= FLAG_HASLAND;
				//g_log<<"land "<<(cmx/TILE_SIZE)<<","<<(cmz/TILE_SIZE)<<" flag="<<(cell->flags & FLAG_HASLAND)<<"="<<(unsigned int)cell->flags<<std::endl;
				//g_log<<"land"<<std::endl;
			}
			else
			{
				//cell->hasland = false;
				cell->flags &= ~FLAG_HASLAND;
			}

#if 0
			if(AtWater(cmx, cmz))
				cell->haswater = true;
			else
				cell->haswater = false;
#endif

			if(TileUnclimable(cmx, cmz) && (cell->flags & FLAG_HASLAND))
			{
				//cell->abrupt = true;
				cell->flags |= FLAG_ABRUPT;
			}
			else
			{
				//cell->abrupt = false;
				cell->flags &= ~FLAG_ABRUPT;
			}

			int tx = cmx/TILE_SIZE;
			int tz = cmz/TILE_SIZE;

			CdTile* r = GetCd(CONDUIT_ROAD, tx, tz, false);

#if 0
			//if(r->on /* && r->finished */ )
			if(r->on && r->finished)
			{
				//cell->hasroad = true;
				cell->flags |= FLAG_HASROAD;
			}
			else
			{
				//cell->hasroad = false;
				cell->flags &= ~FLAG_HASROAD;
			}
#endif
		}


	for(int x=0; x<LARGEST_UNIT_NODES; x++)
		for(int z=0; z<cwz; z++)
		{
			ColliderTile* cell = ColliderAt(x, z);
			cell->flags |= FLAG_ABRUPT;
		}

	for(int x=cwx-LARGEST_UNIT_NODES-1; x<cwx; x++)
		for(int z=0; z<cwz; z++)
		{
			ColliderTile* cell = ColliderAt(x, z);
			cell->flags |= FLAG_ABRUPT;
		}

	for(int x=0; x<cwx; x++)
		for(int z=0; z<LARGEST_UNIT_NODES; z++)
		{
			ColliderTile* cell = ColliderAt(x, z);
			cell->flags |= FLAG_ABRUPT;
		}

	for(int x=0; x<cwx; x++)
		for(int z=cwz-LARGEST_UNIT_NODES-1; z<cwz; z++)
		{
			ColliderTile* cell = ColliderAt(x, z);
			cell->flags |= FLAG_ABRUPT;
		}

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hidden())
			continue;

		u->fillcollider();
	}

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		b->fillcollider();
	}

	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		f->fillcollider();
	}

	ResetPathNodes();
	FillBodies();
}

void Foliage::fillcollider()
{	
	FlType* ft = &g_fltype[type];

	//cm = centimeter position
	int cmminx = cmpos.x - ft->size.x/2;
	int cmminz = cmpos.y - ft->size.z/2;
	int cmmaxx = cmminx + ft->size.x - 1;
	int cmmaxz = cmminz + ft->size.z - 1;

	//c = cell position
	int cminx = imax(0, cmminx / PATHNODE_SIZE);
	int cminz = imax(0, cmminz / PATHNODE_SIZE);
	int cmaxx = imin(g_pathdim.x-1, cmmaxx / PATHNODE_SIZE);
	int cmaxz = imin(g_pathdim.y-1, cmmaxz / PATHNODE_SIZE);

	for(int nz = cminz; nz <= cmaxz; nz++)
		for(int nx = cminx; nx <= cmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, nz);
			c->foliage = this - g_foliage;
		}
}

void Foliage::freecollider()
{
	FlType* ft = &g_fltype[type];

	//cm = centimeter position
	int cmminx = cmpos.x - ft->size.x/2;
	int cmminz = cmpos.y - ft->size.z/2;
	int cmmaxx = cmminx + ft->size.x - 1;
	int cmmaxz = cmminz + ft->size.z - 1;

	//c = cell position
	int cminx = imax(0, cmminx / PATHNODE_SIZE);
	int cminz = imax(0, cmminz / PATHNODE_SIZE);
	int cmaxx = imin(g_pathdim.x-1, cmmaxx / PATHNODE_SIZE);
	int cmaxz = imin(g_pathdim.y-1, cmmaxz / PATHNODE_SIZE);

	for(int nz = cminz; nz <= cmaxz; nz++)
		for(int nx = cminx; nx <= cmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, nz);
			c->foliage = USHRT_MAX;
		}
}

void Unit::fillcollider()
{
	UType* t = &g_utype[type];
	int ui = this - g_unit;

	//cm = centimeter position
	int cmminx = cmpos.x - t->size.x/2;
	int cmminz = cmpos.y - t->size.z/2;
	int cmmaxx = cmminx + t->size.x - 1;
	int cmmaxz = cmminz + t->size.z - 1;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int nz = cminz; nz <= cmaxz; nz++)
		for(int nx = cminx; nx <= cmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, nz);

			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				if(c->units[uiter] < 0)
				{
					c->units[uiter] = ui;
					break;
				}
			}
		}
}

void Building::fillcollider()
{
	BlType* t = &g_bltype[type];
	int bi = this - g_building;

	//t = tile position
	int tminx = tilepos.x - t->widthx/2;
	int tminz = tilepos.y - t->widthy/2;
	int tmaxx = tminx + t->widthx;
	int tmaxz = tminz + t->widthy;

	//cm = centimeter position
	int cmminx = tminx*TILE_SIZE;
	int cmminz = tminz*TILE_SIZE;
	int cmmaxx = tmaxx*TILE_SIZE - 1;
	int cmmaxz = tmaxz*TILE_SIZE - 1;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int nz = cminz; nz <= cmaxz; nz++)
		for(int nx = cminx; nx <= cmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, nz);
			c->building = bi;
		}
}

void Unit::freecollider()
{
	UType* t = &g_utype[type];
	int ui = this - g_unit;

	//cm = centimeter position
	int cmminx = cmpos.x - t->size.x/2;
	int cmminz = cmpos.y - t->size.z/2;
	int cmmaxx = cmminx + t->size.x - 1;
	int cmmaxz = cmminz + t->size.z - 1;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int nz = cminz; nz <= cmaxz; nz++)
		for(int nx = cminx; nx <= cmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, nz);

			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				if(c->units[uiter] == ui)
					c->units[uiter] = -1;
			}
		}
}

void Building::freecollider()
{
	BlType* t = &g_bltype[type];
	int bi = this - g_building;

	//t = tile position
	int tminx = tilepos.x - t->widthx/2;
	int tminz = tilepos.y - t->widthy/2;
	int tmaxx = tminx + t->widthx;
	int tmaxz = tminz + t->widthy;

	//cm = centimeter position
	int cmminx = tminx*TILE_SIZE;
	int cmminz = tminz*TILE_SIZE;
	int cmmaxx = tmaxx*TILE_SIZE - 1;
	int cmmaxz = tmaxz*TILE_SIZE - 1;

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminz = cmminz / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxz = cmmaxz / PATHNODE_SIZE;

	for(int nz = cminz; nz <= cmaxz; nz++)
		for(int nx = cminx; nx <= cmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, nz);

			if(c->building == bi)
				c->building = -1;
		}
}

// Uses cm pos instead of pathnode pos
// Uses cm-accurate intersection checks
bool Standable2(const PathJob* pj, int cmposx, int cmposz)
{
	const int nx = cmposx / PATHNODE_SIZE;
	const int nz = cmposz / PATHNODE_SIZE;

#if 0
	if(nx < 0 || nz < 0 || nx >= g_pathdim.x || nz >= g_pathdim.y)
		return false;
#endif

	ColliderTile* cell = ColliderAt( nx, nz );

#if 1
	if(cell->flags & FLAG_ABRUPT)
	{
		//g_log<<"abrupt"<<std::endl;
		return false;
	}

	if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{
		//g_log<<"!land flag="<<(cell->flags & FLAG_HASLAND)<<"="<<(unsigned int)cell->flags<<std::endl;
		return false;
	}

	if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		//g_log<<"!sea"<<std::endl;
		return false;
	}
#endif

	UType* ut = &g_utype[pj->utype];

	int cmminx = cmposx - ut->size.x/2;
	int cmminz = cmposz - ut->size.z/2;
	int cmmaxx = cmminx + ut->size.x - 1;
	int cmmaxz = cmminz + ut->size.z - 1;

	int cminx = cmminx/PATHNODE_SIZE;
	int cminz = cmminz/PATHNODE_SIZE;
	int cmaxx = cmmaxx/PATHNODE_SIZE;
	int cmaxz = cmmaxz/PATHNODE_SIZE;

#if 0
	if(cminx < 0 || cminz < 0 || cmaxx >= g_pathdim.x || cmaxz >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;

	for(int z=cminz; z<=cmaxz; z++)
		for(int x=cminx; x<=cmaxx; x++)
		{
			cell = ColliderAt(x, z);
			
			if(cell->foliage != USHRT_MAX)
				collided = true;

			if(cell->building >= 0)
			{
				Building* b = &g_building[cell->building];
				BlType* t2 = &g_bltype[b->type];

				int tminx = b->tilepos.x - t2->widthx/2;
				int tminz = b->tilepos.y - t2->widthy/2;
				int tmaxx = tminx + t2->widthx;
				int tmaxz = tminz + t2->widthy;

				int minx2 = tminx*TILE_SIZE;
				int minz2 = tminz*TILE_SIZE;
				int maxx2 = tmaxx*TILE_SIZE - 1;
				int maxz2 = tmaxz*TILE_SIZE - 1;

				if(cmminx <= maxx2 && cmminz <= maxz2 && cmmaxx >= minx2 && cmmaxz >= minz2)
				{
					//g_log<<"bld"<<std::endl;
					collided = true;

					if(cell->building != pj->ignoreb)
						return false;
					else
						ignoredb = true;
				}
			}

			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u = &g_unit[uindex];

				if(uindex != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{
#if 1
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminz2 = u->cmpos.y - t->size.z/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxz2 = cmminz2 + t->size.z - 1;

					if(cmmaxx >= cmminx2 && cmmaxz >= cmminz2 && cmminx <= cmmaxx2 && cmminz <= cmmaxz2)
					{
						//g_log<<"u"<<std::endl;
						//return false;
						collided = true;

						if(uindex == pj->ignoreu)
							ignoredu = true;
					}
#else
					return false;
#endif
				}
			}
		}

	bool arrived = false;

	if(cmminx < pj->goalmaxx &&
		cmminz < pj->goalmaxy &&
		cmmaxx > pj->goalminx &&
		cmmaxz > pj->goalminy)
		arrived = true;
		
	if(pj->roaded)
	{
		CdTile* cdtile = GetCd(CONDUIT_ROAD, cmposx / TILE_SIZE, cmposz / TILE_SIZE, false);
		if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu && !arrived)
		//g_log<<"!road"<<std::endl;
			return false;
	}

	if(collided && !ignoredb && !ignoredu && !arrived)
		return false;

	return true;
}

//#define TRANSPORT_DEBUG
//#define POWCD_DEBUG

bool Standable(const PathJob* pj, const int nx, const int nz)
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

#if 1
	if(cell->flags & FLAG_ABRUPT)
		return false;

	if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{
		
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t !land"<<std::endl;
		char add[1280] = "\t !land \r\n";
		powcdstr += add;
	}
#endif

		return false;
	}

	if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		return false;
	}
#endif

	const UType* ut = &g_utype[pj->utype];

	const int cmposx = nx * PATHNODE_SIZE + PATHNODE_SIZE/2;
	const int cmposz = nz * PATHNODE_SIZE + PATHNODE_SIZE/2;

	const int cmminx = cmposx - ut->size.x/2;
	const int cmminz = cmposz - ut->size.z/2;
	const int cmmaxx = cmminx + ut->size.x - 1;
	const int cmmaxz = cmminz + ut->size.z - 1;

	const int cminx = cmminx/PATHNODE_SIZE;
	const int cminz = cmminz/PATHNODE_SIZE;
	const int cmaxx = cmmaxx/PATHNODE_SIZE;
	const int cmaxz = cmmaxz/PATHNODE_SIZE;

#if 0
	//Done more efficiently in PathJob::process()
	if(pj->pjtype == PATHJOB_BOUNDJPS ||
		pj->pjtype == PATHJOB_BOUNDASTAR)
	{
		if(nx < pj->nminx ||
			nx > pj->nmaxx ||
			nz < pj->nminy ||
			nz > pj->nmaxy)
			return false;
	}
#endif

#if 0
	if(cminx < 0 || cminz < 0 || cmaxx >= g_pathdim.x || cmaxz >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;

	for(int z=cminz; z<=cmaxz; z++)
		for(int x=cminx; x<=cmaxx; x++)
		{
			cell = ColliderAt(x, z);

			if(cell->foliage != USHRT_MAX)
				collided = true;

			if(cell->building >= 0)
			{
				if(cell->building != pj->ignoreb)
				{
					
					
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t collB"<<std::endl;
		char add[1280] = "\t colB \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "Bhit bt=%s b%d ib%d d(%d,%d) u%d", g_bltype[g_building[cell->building].type].name, (int)cell->building, (int)pj->ignoreb, (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goalz-nz)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						g_log<<t<<std::endl;
						AddChat(&rt);
					}
#endif

					//return false;
					collided = true;
				}
				else
				{
					ignoredb = true;

						
#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "ignoreB d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goalz-nz)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						g_log<<t<<std::endl;
						AddChat(&rt);
					}
#endif
				}
			}

			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u = &g_unit[uindex];

				if(uindex == pj->ignoreu)
				{
					ignoredu = true;
				}

				if(uindex != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{
							
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t collU"<<std::endl;
		char add[1280] = "\t colU \r\n";
		powcdstr += add;
	}
#endif

#if 0
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminz2 = u->cmpos.y - t->size.z/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxz2 = cmminz2 + t->size.z - 1;

					if(cmmaxx >= cmminx2 && cmmaxz >= cmminz2 && cmminx <= cmmaxx2 && cmminz <= cmmaxz2)
					{

						return false;
					}
#else
					//return false;
					collided = true;
#endif
				}
			}
		}

	bool arrived = false;

	if(cmminx < pj->goalmaxx &&
		cmminz < pj->goalmaxy &&
		cmmaxx > pj->goalminx &&
		cmmaxz > pj->goalminy)
		arrived = true;

	if(pj->roaded)
	{
		CdTile* cdtile = GetCd(CONDUIT_ROAD, cmposx / TILE_SIZE, cmposz / TILE_SIZE, false);
		if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu && !arrived)
		//g_log<<"!road"<<std::endl;
				
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t !road"<<std::endl;
		char add[1280] = "\t !road \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[pj->thisu];

		RichText rt;
		char t[1280];
		sprintf(t, "!roaded d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goalz-nz)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
		rt.m_part.push_back(t);
		AddChat(&rt);
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		g_log<<"\t !r"<<std::endl;
	}
#endif

		return false;
	}

	if(collided && !ignoredu && !ignoredb && !arrived)
		return false;

	return true;
}

bool TileStandable(const PathJob* pj, const int nx, const int nz)
{
#if 0
	if(nx < 0 || nz < 0 || nx >= g_pathdim.x || nz >= g_pathdim.y)
		return false;
#endif
	
#ifdef HIERDEBUG
	if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && nz == 68*TILE_SIZE/PATHNODE_SIZE)
	{
		g_log<<"\t TileStandable? nxy "<<nx<<","<<nz<<" (t "<<(nx*PATHNODE_SIZE/TILE_SIZE)<<","<<(nz*PATHNODE_SIZE/TILE_SIZE)<<")"<<std::endl;
	}
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

#if 1
	if(cell->flags & FLAG_ABRUPT)
		return false;

	if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{
		
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t !land"<<std::endl;
		char add[1280] = "\t !land \r\n";
		powcdstr += add;
	}
#endif

		return false;
	}

	if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		return false;
	}
#endif

	const UType* ut = &g_utype[pj->utype];

	const int cmposx = nx * PATHNODE_SIZE + PATHNODE_SIZE/2;
	const int cmposz = nz * PATHNODE_SIZE + PATHNODE_SIZE/2;

#if 0
	const int cmminx = cmposx - ut->size.x/2;
	const int cmminz = cmposz - ut->size.z/2;
	const int cmmaxx = cmminx + ut->size.x - 1;
	const int cmmaxz = cmminz + ut->size.z - 1;

	const int cminx = cmminx/PATHNODE_SIZE;
	const int cminz = cmminz/PATHNODE_SIZE;
	const int cmaxx = cmmaxx/PATHNODE_SIZE;
	const int cmaxz = cmmaxz/PATHNODE_SIZE;
#endif

#if 0
	if(cminx < 0 || cminz < 0 || cmaxx >= g_pathdim.x || cmaxz >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;

#if 0
	for(int z=cminz; z<=cmaxz; z++)
		for(int x=cminx; x<=cmaxx; x++)
#else
	int z = nz;
	int x = nx;
#endif
		{
			cell = ColliderAt(x, z);

			//if(cell->foliage != USHRT_MAX)
			//	collided = true;

			
#ifdef HIERDEBUG
			if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && nz == 68*TILE_SIZE/PATHNODE_SIZE)
				g_log<<"cb = "<<cell->building<<std::endl;
#endif

			if(cell->building >= 0)
			{
				
#ifdef HIERDEBUG
				if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && nz == 68*TILE_SIZE/PATHNODE_SIZE)
					g_log<<"ib = "<<(pj->ignoreb)<<" cb = "<<cell->building<<std::endl;
#endif

				if(cell->building != pj->ignoreb)
				{
					
					
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t collB"<<std::endl;
		char add[1280] = "\t colB \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "Bhit bt=%s b%d ib%d d(%d,%d) u%d", g_bltype[g_building[cell->building].type].name, (int)cell->building, (int)pj->ignoreb, (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goalz-nz)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						g_log<<t<<std::endl;
						AddChat(&rt);
					}
#endif

					//return false;
					collided = true;
				}
				else
				{
					ignoredb = true;

						
#ifdef TRANSPORT_DEBUG
					if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
					{
						Unit* u = &g_unit[pj->thisu];

						RichText rt;
						char t[1280];
						sprintf(t, "ignoreB d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goalz-nz)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
						rt.m_part.push_back(t);
						g_log<<t<<std::endl;
						AddChat(&rt);
					}
#endif
				}
			}

#if 0
			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u = &g_unit[uindex];

				if(uindex == pj->ignoreu)
				{
					ignoredu = true;
				}

				if(uindex != pj->thisu && uindex != pj->ignoreu && !u->hidden())
				{
							
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t collU"<<std::endl;
		char add[1280] = "\t colU \r\n";
		powcdstr += add;
	}
#endif

#if 0
					UType* t = &g_utype[u->type];

					int cmminx2 = u->cmpos.x - t->size.x/2;
					int cmminz2 = u->cmpos.y - t->size.z/2;
					int cmmaxx2 = cmminx2 + t->size.x - 1;
					int cmmaxz2 = cmminz2 + t->size.z - 1;

					if(cmmaxx >= cmminx2 && cmmaxz >= cmminz2 && cmminx <= cmmaxx2 && cmminz <= cmmaxz2)
					{

						return false;
					}
#else
					//return false;
					collided = true;
#endif
				}
			}
#endif
		}

	if(pj->roaded)
	{
		CdTile* cdtile = GetCd(CONDUIT_ROAD, cmposx / TILE_SIZE, cmposz / TILE_SIZE, false);
		if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu)
		//g_log<<"!road"<<std::endl;
			return false;
				
#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"\t !road"<<std::endl;
		char add[1280] = "\t !road \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[pj->thisu];

		RichText rt;
		char t[1280];
		sprintf(t, "!roaded d(%d,%d) u%d", (int)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int)(pj->goalz-nz)*PATHNODE_SIZE/TILE_SIZE, (int)pj->thisu);
		rt.m_part.push_back(t);
		AddChat(&rt);
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		g_log<<"\t !r"<<std::endl;
	}
#endif

		//return false;
	}

	if(collided && !ignoredu && !ignoredb)
		return false;
	
#ifdef HIERDEBUG
	//if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && nz 
#endif

	return true;
}
