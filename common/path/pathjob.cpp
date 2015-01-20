#include "pathjob.h"
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
#include "jpspath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "jpsexpansion.h"
#include "../window.h"
#include "partialpath.h"
#include "../sim/transport.h"
#include "astarpath.h"
#include "anypath.h"
#include "tilepath.h"

//not engine
#include "../../game/gui/chattext.h"

long long g_lastpath = 0;
std::list<PathNode*> g_toclear;

#ifdef POWCD_DEBUG
std::string powcdstr;
#endif

bool Trapped(Unit* u, Unit* ignoreu)
{
	UType* ut = &g_utype[u->type];

	PathJob pj;
	pj.utype = u->type;
	//pj.umode = UMODE_NONE;
	pj.cmstartx = u->cmpos.x;
	pj.cmstarty = u->cmpos.y;
	//pj.target = -1;
	//pj.target2 = -1;
	//pj.targtype = TARG_NONE;
	//pj.path = path;
	//pj.subgoal = subgoal;
	pj.thisu = u - g_unit;
	pj.ignoreu = ignoreu ? (ignoreu - g_unit) : -1;
	//pj.ignoreu = -1;
	pj.ignoreb = -1;
#if 0
	//pj.goalx = cmgoalx;
	//pj.goalz = cmgoalz;
	pj.goalx = pj.goalx / PATHNODE_SIZE;
	pj.goalz = pj.goalz / PATHNODE_SIZE;
	pj.goalminx = cmgoalminx;
	pj.goalminy = cmgoalminy;
	pj.goalmaxx = cmgoalmaxx;
	pj.goalmaxy = cmgoalmaxy;
#endif
	pj.roaded = ut->roaded;
	pj.landborne = ut->landborne;
	pj.seaborne = ut->seaborne;
	pj.airborne = ut->airborne;
	//pj.callback = Callback_UnitPath;
	pj.pjtype = PATHJOB_ANYPATH;
	//pj.maxsearch = maxsearch;
	//pj.cdtype = cdtype;
	pj.goalx = u->goal.x;
	pj.goalz = u->goal.y;
	pj.goalx = pj.goalx / PATHNODE_SIZE;
	pj.goalz = pj.goalz / PATHNODE_SIZE;
	pj.goalminx = u->goal.x;
	pj.goalminy = u->goal.y;
	pj.goalmaxx = u->goal.x;
	pj.goalmaxy = u->goal.y;

	//ResetPathNodes();
	SnapToNode(&pj);

	if(!g_openlist.hasmore())
	{
		ClearNodes(g_toclear);

#if 0
		if(u - g_unit == 15)
		{
			static bool did = false;

			if(!did)
			{
				did = true;
				//if(!stand_s)
				{
					InfoMess("snap!","snap!");
				}
			}
		}
#endif

		return true;
	}

	PathNode* node = (PathNode*)g_openlist.deletemin();
	Vec2i npos = PathNodePos(node);
	//unsigned short noff = imax(1, ut->size.x / PATHNODE_SIZE);
	unsigned short noff = 1;

	bool stand_n = Standable(&pj, npos.x, npos.y - noff);
	bool stand_s = Standable(&pj, npos.x, npos.y + noff);
	bool stand_e = Standable(&pj, npos.x + noff, npos.y);
	bool stand_w = Standable(&pj, npos.x - noff, npos.y);

#if 0
	if(u - g_unit == 15)
	{
		static bool did = false;

		if(!did)
		{
			did = true;
			if(!stand_s)
			{
				InfoMess("ss!","ss!");
			}
		}
	}
#endif

	if(!stand_n && !stand_s && !stand_e && !stand_w)
	{
		ClearNodes(g_toclear);
		return true;
	}

	ClearNodes(g_toclear);

	return false;
}

void ClearNodes(std::list<PathNode*> &toclear)
{
	for(auto niter = toclear.begin(); niter != toclear.end(); niter++)
	{
		PathNode* n = *niter;
		n->opened = false;
		n->closed = false;
		n->previous = NULL;
	}

	toclear.clear();
	g_openlist.resetelems();

	//ResetPathNodes();
}

//#define TRANSPORT_DEBUG

bool PathJob::process()
{
#if 0
	long long frames = g_simframe - g_lastpath;

	int delay = frames * 1000 / DRAW_FRAME_RATE;

	if(delay < PATH_DELAY)
		return false;

	g_lastpath = g_simframe;
#endif

	
#ifdef TRANSPORT_DEBUG
	if(thisu >= 0 && g_unit[thisu].type != UNIT_TRUCK)
	{
		//return false;
	}
#endif

	ResetPathNodes();//

	SnapToNode(this);

#ifdef TRANSPORT_DEBUG
	if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280];
		int standable1 = Standable(this, u->cmpos.x/PATHNODE_SIZE, u->cmpos.y / PATHNODE_SIZE);
		int standable2 = Standable2(this, u->cmpos.x, u->cmpos.y);
		sprintf(t, "snap node ?%d u%d stdbl1:%d stdbl2:%d", (int)g_openlist.hasmore(), (int)thisu, standable1, standable2);
		rt.m_part.push_back(t);
		AddChat(&rt);
		g_log<<"truck---------------"<<thisu<<std::endl;
		g_log<<t<<std::endl;
	}
#endif

	PathNode* node;
	searchdepth = 0;
	closest = 0;
	closestnode = NULL;

	while( g_openlist.hasmore() )
	{
		searchdepth ++;

		if((pjtype == PATHJOB_QUICKPARTIAL || pjtype == PATHJOB_JPSPART) && searchdepth > maxsearch)
			break;

		// Pops the lowest score-cost node, moves it in the closed std::list
		node = (PathNode*)g_openlist.deletemin();
		//int i = node - g_pathnode;

		Vec2i npos = PathNodePos(node);

		node->closed = true;

		if(this->pjtype == PATHJOB_BOUNDJPS ||
			this->pjtype == PATHJOB_BOUNDASTAR)
		{
			if(npos.x < this->nminx ||
				npos.x > this->nmaxx ||
				npos.y < this->nminy ||
				npos.y > this->nmaxy)
				continue;
		}
		
#ifdef HIERDEBUG
		if(pathnum == 73)
		{
			g_log<<"\t trynpos "<<npos.x<<","<<npos.y<<" ie. cm "<<(npos.x*PATHNODE_SIZE)<<","<<(npos.y*PATHNODE_SIZE)<<std::endl;
		}
#endif

#ifdef TRANSPORT_DEBUG
//#if 1
	if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280];
		int standable1 = Standable(this, npos.x, npos.y);
		int standable2 = Standable2(this, npos.x * PATHNODE_SIZE, npos.y * PATHNODE_SIZE);
		g_log<<"truck node dgoal("<<(npos.x - cmgoalx/PATHNODE_SIZE)<<","<<(npos.y - cmgoalz/PATHNODE_SIZE)<<") st1:"<<standable1<<" st2:"<<standable2<<" npos("<<npos.x<<","<<npos.y<<") d:("<<(npos.x-cmstartx/PATHNODE_SIZE)<<","<<(npos.y-cmstarty/PATHNODE_SIZE)<<")"<<std::endl;
	}
#endif

		// If the popped node is the endNode, return it
		if( AtGoal(this, node) )
		{
			
#ifdef HIERDEBUG
			if(pathnum == 73)
			{
				g_log<<"\t AT GOAL trynpos "<<npos.x<<","<<npos.y<<std::endl;
			}
#endif

#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280];
		sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		AddChat(&rt);
	}
#endif
	
#ifdef RANDOM8DEBUG
	if(this->thisu == thatunit)
	{
		g_log<<"AT GOAL SUCCESS pjtype="<<pjtype<<std::endl;
		g_log.flush();
	}
#endif

//#define POWCD_DEBUG
			
#ifdef POWCD_DEBUG
	if(this->umode == UMODE_GOCDJOB && g_unit[this->thisu].cdtype == CONDUIT_POWL)
	{
		//g_log<<"===success u"<<this->thisu<<"===="<<std::endl;
		powcdstr.clear();
	}
#endif

			ReconstructPath(this, node);

			ClearNodes(g_toclear);

			if(callback)
				callback(true, this);

			return true;
		}

#if 0
		if(npos.x < nminx)
			continue;
		if(npos.y < nminy)
			continue;
		if(npos.x > nmaxx)
			continue;
		if(npos.y > nmaxy)
			continue;
#endif

		// otherwise, identify successors of the popped node
		if(pjtype == PATHJOB_JPS || 
			pjtype == PATHJOB_BOUNDJPS)
			Expand_JPS(this, node);
		//if(pjtype == PATHJOB_BOUNDJPS)
		//	Expand_BoundJPS(this, node);
		//else if(pjtype == PATHJOB_ANYPATH)
		//	Expand_AP(this, node);
		else if(pjtype == PATHJOB_QUICKPARTIAL)
			Expand_QP(this, node);
		else if(pjtype == PATHJOB_ASTAR ||
			pjtype == PATHJOB_BOUNDASTAR)
			Expand_A(this, node);
	}

	bool pathfound = false;

	if((pjtype == PATHJOB_QUICKPARTIAL || pjtype == PATHJOB_JPSPART) && 
		closestnode && allowpart)
	{
		pathfound = true;
		ReconstructPath(this, closestnode);
	}
	
#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK && !pathfound)
	{
		Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280];
		sprintf(t, "NO PATH ######## pathfound u%d searchdepth%d", (int)thisu, searchdepth);
		rt.m_part.push_back(t);
		AddChat(&rt);
	}
#endif
	
#ifdef RANDOM8DEBUG
	if(this->thisu == thatunit)
	{
		g_log<<"FAIL NO GOAL pjtype="<<pjtype<<std::endl;
		g_log.flush();
	}
#endif
	
#ifdef POWCD_DEBUG
	if(this->umode == UMODE_GOCDJOB && g_unit[this->thisu].cdtype == CONDUIT_POWL && g_unit[this->thisu].cdtype == CONDUIT_POWL)
	{
		g_log<<"===FAIL u"<<this->thisu<<"===="<<std::endl;
		g_log<<powcdstr<<std::endl;
		powcdstr.clear();
	}
#endif

	ClearNodes(g_toclear);

	if(callback)
		callback(pathfound, this);

	return true;
}

void Callback_UnitPath(bool result, PathJob* pj)
{
	short ui = pj->thisu;

	if(ui < 0)
		return;

	Unit* u = &g_unit[ui];

	u->threadwait = false;
	u->pathblocked = !result;
}
