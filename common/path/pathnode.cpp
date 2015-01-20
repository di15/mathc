#include "pathnode.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "pathjob.h"
#include "../sim/utype.h"
#include "collidertile.h"
#include "../sys/binheap.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/unit.h"

Vec2i g_pathdim(0,0);
PathNode* g_pathnode = NULL;
BinHeap g_openlist(CompareNodes);

bool CompareNodes(void* a, void* b)
{
	return ((PathNode*)a)->score > ((PathNode*)b)->score;
}

PathNode::PathNode(int startx, int startz, int endx, int endz, int nx, int nz, PathNode* prev, int totalD, int stepD)
{
	//this->nx = nx;
	//this->nz = nz;
	int G = totalD + stepD;
	this->totalD = G;
	//float G = Magnitude2(Vec3f(startx-x,0,startz-z));
	//multiply by 2 to give granularity
	//(needed for diagonal moves).
	int H = PATHHEUR(Vec2i(endx-nx,endz-nz)) << 1;
	//int H = Manhattan(Vec2i(endx-nx,endz-nz)) << 1;
	//int G = abs(startx-x) + abs(startz-z);
	//int H = abs(endx-x) + abs(endz-z);
	score = G + H;
	//score = H;
	previous = prev;
	//tried = false;
}

#if 0
PathNode::PathNode(int startx, int startz, int endx, int endz, int nx, int nz, PathNode* prev, int totalD, int stepD, unsigned char expan)
{
	this->nx = nx;
	this->nz = nz;
	int G = totalD + stepD;
	this->totalD = G;
	//float G = Magnitude2(Vec3f(startx-x,0,startz-z));
	//float H = Magnitude(Vec2i(endx-nx,endz-nz));
	int H = Manhattan(Vec2i(endx-nx,endz-nz));
	//int G = abs(startx-x) + abs(startz-z);
	//int H = abs(endx-x) + abs(endz-z);
	score = G + H;
	//score = H;
	previous = prev;
	//tried = false;
	expansion = expan;
}
#endif

PathNode* PathNodeAt(int nx, int nz)
{
#if 0	//safe?
	if(nx < 0)
		return NULL;

	if(nz < 0)
		return NULL;

	if(nx >= g_pathdim.x)
		return NULL;

	if(nz >= g_pathdim.y)
		return NULL;
#endif

	return &g_pathnode[ nz * g_pathdim.x + nx ];
}

int PathNodeIndex(int nx, int nz)
{
	return nz * g_pathdim.x + nx;
}

Vec2i PathNodePos(PathNode* node)
{
	if(!node)
		return Vec2i(-1,-1);

	const int i = node - g_pathnode;
	const int nz = i / g_pathdim.x;
	const int nx = i % g_pathdim.x;
	return Vec2i(nx, nz);
}

bool AtGoal(PathJob* pj, PathNode* node)
{
	Vec2i cpos = PathNodePos(node);
	int cmposx = cpos.x * PATHNODE_SIZE + PATHNODE_SIZE/2;
	int cmposz = cpos.y * PATHNODE_SIZE + PATHNODE_SIZE/2;

	UType* ut = &g_utype[pj->utype];

	int cmminx = cmposx - ut->size.x/2;
	int cmminz = cmposz - ut->size.z/2;
	int cmmaxx = cmminx + ut->size.x - 1;
	int cmmaxz = cmminz + ut->size.z - 1;

	if(cmminx <= pj->goalmaxx && cmminz <= pj->goalmaxy && cmmaxx >= pj->goalminx && cmmaxz >= pj->goalminy)
		return true;

	return false;
}

/*
This is very important not to create unwalkable paths.
If you start from the pathnode that is gotten by rounding down the cmpos,
you might be off. This is from experience of mass movement of military units
and seeing repeating movement loops that units fail to break out of.
*/
void SnapToNode(PathJob* pj)
{
#if 1
	//Is this correct? This was my first solution and I changed it to the second one for some time,
	//but seeing an unresolvable truck position that remained trapped even though it could actually move out,
	//changed it back, solving the trapped truck problem.
	//Edit: No, the second method is correct. The only solution to the truck being stuck is to use more pathnode divisions.
	//This is more realistic anyways (using PATHNODE_SIZE of 50 centimeters instead of 125, because a person has about 50 cm
	//of personal space, and a 10x10 meter road tile divides to give 20x20 people worth of room).
	//Edit: the first method must be correct, since for the npos_min we are at npos-1, and for npos_max we are at npos+1.
	Vec2i npos = Vec2i( (pj->cmstartx+PATHNODE_SIZE/2) / PATHNODE_SIZE, (pj->cmstarty+PATHNODE_SIZE/2) / PATHNODE_SIZE );
	//Vec2i npos = Vec2i( (pj->cmstartx) / PATHNODE_SIZE, (pj->cmstarty) / PATHNODE_SIZE );
	//Vec2i npos = Vec2i( (pj->cmstartx-PATHNODE_SIZE/2) / PATHNODE_SIZE, (pj->cmstarty-PATHNODE_SIZE/2) / PATHNODE_SIZE );

	npos.x = imin(g_pathdim.x-1, npos.x);
	npos.y = imin(g_pathdim.y-1, npos.y);

	Vec2i npos_min = npos - Vec2i(1,1);

	npos_min.x = imax(0, npos_min.x);
	npos_min.y = imax(0, npos_min.y);
	
	Vec2i npos_max = npos_min + Vec2i(1,1);

	Vec2i npos_nw = Vec2i( npos_min.x, npos_min.y );
	Vec2i npos_ne = Vec2i( npos_max.x, npos_min.y );
	Vec2i npos_sw = Vec2i( npos_min.x, npos_max.y );
	Vec2i npos_se = Vec2i( npos_max.x, npos_max.y );
	Vec2i npos_w = Vec2i( npos_min.x, npos.y );
	Vec2i npos_e = Vec2i( npos_max.x, npos.y );
	Vec2i npos_n = Vec2i( npos.x, npos_min.y );
	Vec2i npos_s = Vec2i( npos.x, npos_max.y );

#if 1
	PathNode* node_nw = PathNodeAt(npos_nw.x, npos_nw.y);
	PathNode* node_ne = PathNodeAt(npos_ne.x, npos_ne.y);
	PathNode* node_sw = PathNodeAt(npos_sw.x, npos_sw.y);
	PathNode* node_se = PathNodeAt(npos_se.x, npos_se.y);
	PathNode* node_cen = PathNodeAt(npos.x, npos.y);
	PathNode* node_w = PathNodeAt(npos_w.x, npos_w.y);
	PathNode* node_e = PathNodeAt(npos_e.x, npos_e.y);
	PathNode* node_n = PathNodeAt(npos_n.x, npos_n.y);
	PathNode* node_s = PathNodeAt(npos_s.x, npos_s.y);
#endif

	bool walkable_nw = Standable(pj, npos_nw.x, npos_nw.y);
	bool walkable_ne = Standable(pj, npos_ne.x, npos_ne.y);
	bool walkable_sw = Standable(pj, npos_sw.x, npos_sw.y);
	bool walkable_se = Standable(pj, npos_se.x, npos_se.y);
	bool walkable_cen = Standable(pj, npos.x, npos.y);
	bool walkable_w = Standable(pj, npos_w.x, npos_w.y);
	bool walkable_e = Standable(pj, npos_e.x, npos_e.y);
	bool walkable_n = Standable(pj, npos_n.x, npos_n.y);
	bool walkable_s = Standable(pj, npos_s.x, npos_s.y);

	Vec2i cmpos_nw = Vec2i( npos_nw.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos_nw.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );
	Vec2i cmpos_ne = Vec2i( npos_ne.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos_ne.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );
	Vec2i cmpos_sw = Vec2i( npos_sw.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos_sw.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );
	Vec2i cmpos_se = Vec2i( npos_se.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos_se.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );

	int dist_nw = Magnitude( Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_nw );
	int dist_ne = Magnitude( Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_ne );
	int dist_sw = Magnitude( Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_sw );
	int dist_se = Magnitude( Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_se );

	PathNode* startnode = NULL;

	int nearest = -1;

#if 0
	if( walkable_cen && walkable_w && !startnode )
	{
		startnode = node_cen;
	}
	if( walkable_cen && walkable_e && !startnode )
	{
		startnode = node_cen;
	}
	if( walkable_cen && walkable_s && !startnode )
	{
		startnode = node_cen;
	}
	if( walkable_cen && walkable_n && !startnode )
	{
		startnode = node_cen;
	}
#endif

	if( walkable_nw && walkable_ne && walkable_sw && (dist_nw < nearest || !startnode) )
	{
		nearest = dist_nw;
		startnode = node_nw;
		npos = npos_nw;
	}
	if( walkable_ne && walkable_nw && walkable_se && (dist_ne < nearest || !startnode) )
	{
		nearest = dist_ne;
		startnode = node_ne;
		npos = npos_ne;
	}
	if( walkable_sw && walkable_nw && walkable_se && (dist_sw < nearest || !startnode) )
	{
		nearest = dist_sw;
		startnode = node_sw;
		npos = npos_sw;
	}
	if( walkable_se && walkable_ne && walkable_sw && (dist_se < nearest || !startnode) )
	{
		nearest = dist_se;
		startnode = node_se;
		npos = npos_se;
	}

	if(!startnode)
	{
		UType* ut = &g_utype[pj->utype];

		{
			int nposx = npos_nw.x;
			int nposz = npos_nw.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + PATHNODE_SIZE/2, nposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			
			if(dir == Vec2i(0,0))
			{
				startnode = node_nw;
				goto foundnode;
			}
			
			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_nw < nearest || !startnode) )
			{
				nearest = dist_nw;
				startnode = node_nw;
			}
		}

		{
			int nposx = npos_ne.x;
			int nposz = npos_ne.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + PATHNODE_SIZE/2, nposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			
			if(dir == Vec2i(0,0))
			{
				startnode = node_ne;
				goto foundnode;
			}
			
			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_ne < nearest || !startnode) )
			{
				nearest = dist_ne;
				startnode = node_ne;
			}
		}

		{
			int nposx = npos_sw.x;
			int nposz = npos_sw.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + PATHNODE_SIZE/2, nposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;
			
			if(dir == Vec2i(0,0))
			{
				startnode = node_sw;
				goto foundnode;
			}
			
			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_sw < nearest || !startnode) )
			{
				nearest = dist_sw;
				startnode = node_sw;
			}
		}

		{
			int nposx = npos_se.x;
			int nposz = npos_se.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + PATHNODE_SIZE/2, nposz * PATHNODE_SIZE + PATHNODE_SIZE/2);
			Vec2i dir = to - from;

			if(dir == Vec2i(0,0))
			{
				startnode = node_se;
				goto foundnode;
			}
			
			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / Magnitude(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_se < nearest || !startnode) )
			{
				nearest = dist_se;
				startnode = node_se;
			}
		}

		if(!startnode)
		{
#if 0
			if(pj->thisu == 15)
			{
				static bool did = false;

				if(!did)
				{
					did = true;
					InfoMess("st!", "!st");
				}
			}
#endif

			return;
		}
	}

foundnode:

	Vec2i cmpos = Vec2i( npos.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );

	startnode->totalD = PATHHEUR( Vec2i(pj->cmstartx, pj->cmstarty)/PATHNODE_SIZE - cmpos/PATHNODE_SIZE ) << 1;
	startnode->score = startnode->totalD + PATHHEUR( Vec2i(pj->goalx, pj->goalz) - cmpos/PATHNODE_SIZE ) << 1;
	startnode->previous = NULL;

	g_openlist.insert(startnode);

	startnode->opened = true;
	g_toclear.push_back(startnode);

#else
	Vec2i npos = Vec2i( (pj->cmstartx) / PATHNODE_SIZE, (pj->cmstarty) / PATHNODE_SIZE );
	PathNode* startnode = PathNodeAt(npos.x, npos.y);

	Vec2i cmpos = Vec2i( npos.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );

	startnode->totalD = Magnitude( Vec2i(pj->cmstartx, pj->cmstarty) - cmpos );
	startnode->score = startnode->totalD + Manhattan( Vec2i(pj->goalx, pj->goalz) - cmpos );
	startnode->previous = NULL;

	g_openlist.insert(startnode);
	//pj->wt->opennode[ startnode - pj->wt->pathnode ] = pj->wt->pathcnt;
	
	startnode->opened = true;
	g_toclear.push_back(startnode);
#endif

	//startNode._opened = true
	//toClear[startNode] = true
}

//long long g_lastpath;

void ResetPathNodes()
{
	return;

	StartTimer(TIMER_RESETPATHNODES);

	for(int i = 0; i < g_pathdim.x * g_pathdim.y; i++)
	{
		PathNode* n = &g_pathnode[i];
		n->closed = false;
		n->opened = false;
		n->previous = NULL;
	}
	g_openlist.resetelems();
	
	StopTimer(TIMER_RESETPATHNODES);
}
