

#include "transport.h"
#include "building.h"
#include "unit.h"
#include "bltype.h"
#include "utype.h"
#include "conduit.h"
#include "resources.h"
#include "player.h"
#include "job.h"
#include "../render/transaction.h"
#include "../ai/ai.h"
#include "simdef.h"
#include "../econ/utility.h"
#include "../path/jpspath.h"
#include "../path/astarpath.h"
#include "../path/pathnode.h"
#include "../platform.h"
#include "truck.h"
#include "../sound/sound.h"
#include "../path/pathjob.h"
#include "../path/anypath.h"
#include "../path/fillbodies.h"
#include "../math/frustum.h"
#include "../render/fogofwar.h"

//not engine
#include "../../game/gui/chattext.h"

static std::list<int> truckpathd;

//#define TRANSPORT_DEBUG
//optimized pathing (checks sorted list before prepathing), 
//less accurate dist (doesn't traverse the path, but uses Magnitude() or Manhattan()).
//this is now the default, disabling this define leaves it broken. will clean it up.
#define TBID2

//i would base the utility of a truck as a transporter for a given job based on a function of its distance from the supplier and the distance of 
//the supplier to the demander, and the charge of the transport owner, but especially in the early building phase of the game,
//there might be disconnected pockets of the economy that might be inaccessible to each other, or even road tiles that might not be accessable 
//because the road leading up to them hasn't been finished, meaning a truck might be repeatedly be given a transport job it can't fulfill.
//so i'm using the complicated, bidding and pathfinding-to-everything method i used in the previous version.

#if 0
int BestSup(Vec2i demcmpos, int rtype, int* retutil, Vec2i* retcmpos)
{
	int bestsup = -1;
	int bestutil = -1;
	Resource* r = &g_resource[rtype];

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];
		BlType* bt = &g_bltype[b->type];

		if(bt->output[rtype] <= 0)
			continue;

		Player* supp = &g_player[b->owner];

		if(supp->global[rtype] + b->stocked[rtype] <= 0)
			continue;

		//check if distance is better or if there's no best yet

		Vec2i bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

		int dist = Magnitude(bcmpos - demcmpos);

		//if(dist > bestdist && bestdemb)
		//	continue;

		int margpr = b->price[rtype];

		int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

		if(util <= bestutil && bestsup >= 0)
			continue;

		bestsup = bi;
		bestutil = util;
		*retutil = util;
		*retcmpos = bcmpos;
	}

	return bestsup;
}
#endif

//warning: "arrive" must be set to starting position before calling this function, or else "dist" will be wrong.
void StepBeforeArrival(int utype, Vec2i* arrive, std::list<Vec2i>* path, int* dist, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy)
{
	UType* ut = &g_utype[utype];

	for(auto pit=path->begin(); pit!=path->end(); pit++)
	{
#if 0
		int newuminx = pit->x - ut->size.x/2;
		int newuminy = pit->y - ut->size.z/2;
		int newumaxx = newuminx + ut->size.x - 1;
		int newumaxy = newuminy + ut->size.z - 1;

		if(newuminx <= cmgoalmaxx && newuminy <= cmgoalmaxy && newumaxx >= cmgoalminx && newumaxy >= cmgoalminy)
			return;
		
		*dist += Magnitude(*arrive - *pit);
		//only advance here once we know we're still out of potential collision area, so that next pathfind won't start in a solid
		*arrive = *pit;
#else
		//break up long moves into PATHNODE_SIZE steps
		Vec2i delta = *pit - *arrive;	//arrive is previous step here
		//assuming only diagonal or straight moves
		int len = imax(delta.x, delta.y);

		if(len <= 0)
			continue;

		delta = delta / len;
		Vec2i step = delta * PATHNODE_SIZE;
		int steps = len / PATHNODE_SIZE;

		for(int si=0; si<steps; si++)
		{
			Vec2i newpos = *arrive + step;
			
			int newuminx = newpos.x - ut->size.x/2;
			int newuminy = newpos.y - ut->size.z/2;
			int newumaxx = newuminx + ut->size.x - 1;
			int newumaxy = newuminy + ut->size.z - 1;

			if(newuminx <= cmgoalmaxx && newuminy <= cmgoalmaxy && newumaxx >= cmgoalminx && newumaxy >= cmgoalminy)
				return;
			
			*dist += Magnitude(*arrive - newpos);
			*arrive = newpos;
		}
#endif
	}
}

// comparison, not case sensitive.
bool CompareJobs(const TransportJob& a, const TransportJob& b)
{
	return a.jobutil > b.jobutil;
}

// bid for transportation of resources to X
void TBid(int target, int target2, int targtype, int umode, int cdtype, int res, int amt)
{
#ifdef TRUCK_DEBUG
	g_log<<"-------------------"<<endl;
	g_log<<"TBid("<<target<<", "<<target2<<", "<<targtype<<", "<<res<<", "<<amt<<");"<<endl;
	g_log.flush();
#endif

#ifdef TRANSPORT_DEBUG
	if(res != RES_FARMPRODUCTS)
		return;
#endif

	Vec2i dempos;
	int demcmminx;	//"demander centimeter minimum x coordinate"
	int demcmminy;
	int demcmmaxx;
	int demcmmaxy;
	int demplayer = -1;
	UType* ut = &g_utype[UNIT_TRUCK];
	Building* demb = NULL;

	//if(amt >0 && res == RES_CEMENT)
		//InfoMess("req c", "hreq c2");

	if(targtype == TARG_BL)
	{
		//char msg[128];
		//sprintf(msg, "bid %s", g_bltype[g_building[target].type].name);
		//Chat(msg);
		demb = &g_building[target];
		demplayer = demb->owner;
		dempos = demb->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		BlType* bt = &g_bltype[demb->type];
		int demtminx = demb->tilepos.x - bt->widthx/2;
		int demtminy = demb->tilepos.y - bt->widthy/2;
#if 1
		demcmminx = demtminx * TILE_SIZE;
		demcmminy = demtminy * TILE_SIZE;
		//"pixel perfect" centimeter coordinates. 
		//we subtract 1 because we start count at 0. this is important. 
		//you can use <= for collision checks this way instead of <, 
		//which wouldn't detect objects that are overlapping by 1 cm.
		//so an object of width 10 has the range [0,9]. 
		//0 + 10 - 1 = 9
		//the next object will occupy [10,19].
		demcmmaxx = demcmminx + bt->widthx * TILE_SIZE - 1;	
		demcmmaxy = demcmminy + bt->widthy * TILE_SIZE - 1;
#else
		demcmminx = dempos.x;
		demcmminy = dempos.y;
		demcmmaxx = dempos.x;
		demcmmaxy = dempos.y;
#endif

#ifdef TRUCK_DEBUG
		g_log<<"demander b = "<<g_bltype[g_building[target].type].name<<endl;
		g_log.flush();
#endif
	}
	else if(targtype == TARG_CD)
	{
		//Chat("bid road");
		CdTile* ctile = GetCd(cdtype, target, target2, false);
		demplayer = ctile->owner;
		CdType* ctype = &g_cdtype[cdtype];
		dempos = Vec2i(target,target2)*TILE_SIZE + ctype->physoff;
		demcmminx = dempos.x - TILE_SIZE/2;
		demcmminy = dempos.y - TILE_SIZE/2;
		demcmmaxx = demcmminx + TILE_SIZE - 1;
		demcmmaxy = demcmminy + TILE_SIZE - 1;
	}

	// no available sources/paths?

#if 0
	bool found = false;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[res] <= 0)
			continue;

		Player* bp = &g_player[b->owner];

		if(b->stocked[res] + bp->global[res] <= 0)
			continue;

		found = true;
	}

	if(!found)
		return;
#elif 0
	int suputil;
	Vec2i suppos;
	bestsup = BestSup(dempos, res, &suputil, &suppos);

	if(bestsup < 0)
		return;
#endif

	int bestsup = -1;
	int bestutil = -1;
	int bestunit = -1;
	Vec2i bestsuppos;

	std::list<TransportJob> jobs;

	//try each truck
	for(int ui=0; ui<UNITS; ui++)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(u->type != UNIT_TRUCK)
			continue;

		if(u->mode != UMODE_NONE)
			continue;

		if(u->jobframes < TBID_DELAY)
			continue;

		truckpathd.push_back(ui);

		//if(amt >0 && res == RES_CEMENT)
		//	InfoMess("req c", "hreq c3");

		if(Trapped(u, NULL))
		{
			short tin = (u->cmpos.x/TILE_SIZE) + (u->cmpos.y/TILE_SIZE)*g_hmap.m_widthx;
			TileNode* tn = &g_tilenode[tin];
			tn->jams = imin(tn->jams + 3, 6);
			continue;
		}

		Player* up = &g_player[u->owner];

		//for each truck, try each supplier too
		int subbestsup = -1;	//sub best supplier: used to evalute the best supplier given this truck (and the demander of course), so as to assign the best supplier and truck combo
		int subbestutil = -1;	//used to figure out the supplier with best util
		Vec2i subbestsuppos;
		Resource* r = &g_resource[res];

		//...with each supplier
		for(int bi=0; bi<BUILDINGS; bi++)
		{
			Building* potsupb = &g_building[bi];	//potential supplier building

			if(!potsupb->on)//how could I have missed this?
				continue;

			if(!potsupb->finished)
				continue;

			BlType* bt = &g_bltype[potsupb->type];

			if(bt->output[res] <= 0)
				continue;
			
			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("req c", "hreq c1");

			Player* supp = &g_player[potsupb->owner];

			if(supp->global[res] + potsupb->stocked[res] <= 0)
				continue;

			//check if distance is better or if there's no best yet

			Vec2i supcmpos = potsupb->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

			
			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("req c", "hreq c");

#if 0
			//warning: possibility of int overflow on large maps, definitly on 255x255 tiles, using Magnitude()
			int dist = Magnitude(supcmpos - dempos);

			//if(dist > bestdist && bestdemb)
			//	continue;

			int margpr = potsupb->price[res];

			int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

			if(util <= subbestutil && subbestsup >= 0)
				continue;
#endif

			int supcmminx = (potsupb->tilepos.x - bt->widthx/2) * TILE_SIZE;
			int supcmminy = (potsupb->tilepos.y - bt->widthy/2) * TILE_SIZE;	//TO DO: change all the z's to y's and vice versa, for Id coordinate system, which makes more sense for top-down RTS/strategy games
			int supcmmaxx = supcmminx + bt->widthx * TILE_SIZE - 1;
			int supcmmaxy = supcmminy + bt->widthy * TILE_SIZE - 1;

#ifndef TBID2
			//now check if a path is available from the transporter to this supplier - a very expensive op
			std::list<Vec2i> path;
			Vec2i subgoal;

#if 0
			JPSPath(u->type, UMODE_GOSUP, u->cmpos.x, u->cmpos.y, bi, -1, UMODE_GOSUP, &path, &subgoal, u, NULL, potsupb, supcmpos.x, supcmpos.y, supcmminx, supcmminy, supcmmaxx, supcmmaxy);
#elif 0
			AStarPath(
				        u->type, UMODE_GOSUP,
				        u->cmpos.x, u->cmpos.y, target, target2, TARG_BL, &path, &subgoal,
				        u, NULL, potsupb,
				        supcmpos.x, supcmpos.y,
				        supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y, SHRT_MAX-1);
#elif 1
			JPSPath(
				        u->type, UMODE_GOSUP,
				        u->cmpos.x, u->cmpos.y, -1, -1, TARG_BL, cdtype, &path, &subgoal,
				        u, NULL, potsupb,
				        supcmpos.x, supcmpos.y,
				        supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y,
						0, 0, g_pathdim.x-1, g_pathdim.y-1);
#else
			
			JPSPath(
				        UNIT_LABOURER, UMODE_GOSUP,
				        supcmpos.x-100, supcmpos.y-100, target, target2, TARG_BL, &path, &subgoal,
				        u, NULL, potsupb,
				        supcmpos.x, supcmpos.y,
				        supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y);
#endif
			if(path.size() == 0)
			{
#ifdef TRANSPORT_DEBUG
				{
					char msg[1280];
					sprintf(msg, "no path 1 sup%d ", (int)(potsupb-g_building));
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				continue;	//no path
			}

			//continue;

			int trucktosup = 0;
			Vec2i arrive = u->cmpos;

			//calculate distance from truck to supplier and determine arrival spot (the path given leads to the center of the supplier, but we stop at some point before).
			StepBeforeArrival(u->type, &arrive, &path, &trucktosup, supcmminx, supcmminy, supcmmaxx, supcmmaxy);

			//now, given the position of the transporter at the supplier, find a path from there to the demander - another expensive op
			path.clear();
			
			JPSPath(u->type, umode, 
				arrive.x, arrive.y, target, target2, targtype, cdtype,
				&path, &subgoal, 
				u, NULL, demb, 
				dempos.x, dempos.y, 
				demcmminx, demcmminy, demcmmaxx, demcmmaxy,
				0, 0, g_pathdim.x-1, g_pathdim.y-1);
			//JPSPath(u->type, umode, u->cmpos.x, u->cmpos.y, target, target2, targtype, &path, &subgoal, u, NULL, demb, dempos.x, dempos.y, demcmminx, demcmminy, demcmmaxx, demcmmaxy);

			//calculate distance from supplier arrival position to supplier and determine arrival spot (the path given leads to the center of the supplier, but we stop at some point before).
			int suptodem = 0;
			StepBeforeArrival(u->type, &arrive, &path, &suptodem, demcmminx, demcmminy, demcmmaxx, demcmmaxy);

			if(path.size() == 0)
			{
#ifdef TRANSPORT_DEBUG
				{
					char msg[128];
					sprintf(msg, "no path 2 dembi%d", demb ? (int)(demb-g_building) : -1);
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				continue;	//no path
			}

#if 1
			//warning: possibility of int overflow on large maps, definitly on 255x255 tiles, using Magnitude()
			int dist = trucktosup + suptodem;

			int margpr = potsupb->price[res];

			int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

			if(util <= subbestutil && subbestsup >= 0)
				continue;
#endif

			subbestsup = bi;
			subbestutil = util;
			subbestsuppos = supcmpos;
			
#else
			int trucktosup = TRANHEUR(u->cmpos - supcmpos);
			int suptodem = TRANHEUR(supcmpos - dempos);

			int dist = trucktosup + suptodem;

			int margpr = potsupb->price[res];

			int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);
			
#if 0
			if(util <= subbestutil && subbestsup >= 0)
				continue;
			
			//don't prepath until the end, when we check off a sorted list
			if(!AnyPath(
				u->type, UMODE_GOSUP,
				u->cmpos.x, u->cmpos.y, -1, -1, TARG_BL, cdtype,
				u, NULL, potsupb,
				supcmpos.x, supcmpos.y,
				supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y,
				0, 0, g_pathdim.x-1, g_pathdim.y-1))
				continue;

			if(!AnyPath(u->type, umode, 
				u->cmpos.x, u->cmpos.y, target, target2, targtype, cdtype,
				u, NULL, demb, 
				dempos.x, dempos.y, 
				demcmminx, demcmminy, demcmmaxx, demcmmaxy,
				0, 0, g_pathdim.x-1, g_pathdim.y-1))
				continue;
#endif

#if 0
			if(amt >0 && res == RES_CEMENT)
			{
				char msg[128];
				sprintf(msg, "c hu %d", util);
				InfoMess("req c", msg);
			}
#endif

			jobs.push_back(TransportJob());
			TransportJob* j = &*jobs.rbegin();
			j->suputil = util;
			j->jobutil = util;
			j->supbi = bi;
			j->supcmpos = supcmpos;
			j->truckui = ui;
			j->potsupb = potsupb;
			//TO DO: add transportjob and later go through 
			//list in sorted order, pathing to each.

			//TO DO: use binheap instead of std::list,
			//for this, for JobOpp, for other uses?
			
#if 0
			subbestsup = bi;
			subbestutil = util;
			subbestsuppos = supcmpos;
#endif

#endif
		}

#if 0
		//no best supplier? maybe there's no path between this truck and any supplier.
		if(subbestsup < 0)
			continue;

		if(subbestutil < bestutil)
			continue;

		bestutil = subbestutil;
		bestsup = subbestsup;
		bestunit = ui;
		bestsuppos = subbestsuppos;
#endif
	}

#ifdef TRANSPORT_DEBUG
	{
		char msg[128];
		sprintf(msg, "success TBid?");
		RichText debugrt;
		debugrt.m_part.push_back(msg);
		AddChat(&debugrt);
	}
#endif

#if 0
	// no transporters?
	if(bestunit < 0)
		return;

	if(bestsup < 0)
		return;
#endif

#ifdef TBID2
	
	if(jobs.size() <= 0)
		return;
	
#if 0
	if(amt >0 && res == RES_CEMENT)
	{
		char msg[128];
		sprintf(msg, "j %d", (int)jobs.size());
		InfoMess("req c", msg);
	}
#endif

	jobs.sort(CompareJobs);

#ifdef TRANSPORT_DEBUG
	{
		char msg[128];
		sprintf(msg, "success TBid!");
		RichText debugrt;
		debugrt.m_part.push_back(msg);
		AddChat(&debugrt);
	}
#endif

	
	for(auto jit=jobs.begin(); jit!=jobs.end(); jit++)
	{
		TransportJob* j = &*jit;

		if(j->jobutil <= 0)
			return;

		//numpaths++;

		Unit* u = &g_unit[j->truckui];
		//TODO

		//first pathable job

#if 1
#if 0
		if(!AnyPath(u->type, j->jobtype, u->cmpos.x, u->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype,
			u, j->ignoreu, j->ignoreb,
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			nminx, nminy, nmaxx, nmaxy))
			continue;
#else	
		if(!AnyPath(
			u->type, UMODE_GOSUP,
			u->cmpos.x, u->cmpos.y, -1, -1, TARG_BL, cdtype,
			u, NULL, j->potsupb,
			j->supcmpos.x, j->supcmpos.y,
			j->supcmpos.x, j->supcmpos.y, j->supcmpos.x, j->supcmpos.y,
			0, 0, g_pathdim.x-1, g_pathdim.y-1))
		{
			
			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("bl","bl");

			continue;
		}

		if(!AnyPath(u->type, umode, 
			u->cmpos.x, u->cmpos.y, target, target2, targtype, cdtype,
			u, NULL, demb, 
			dempos.x, dempos.y, 
			demcmminx, demcmminy, demcmmaxx, demcmmaxy,
			0, 0, g_pathdim.x-1, g_pathdim.y-1))
		{
			
			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("bl","bl2");

			continue;
		}
#endif
#else
		Vec2i subgoal;
		std::list<Vec2i> path;

#ifdef RANDOM8DEBUG
		if(j->ignoreu - g_unit == 12)
			thatunit = u - g_unit;
#endif

		JPSPath(u->type, j->jobtype, u->cmpos.x, u->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype, 
			&path, &subgoal, 
			u, j->ignoreu, j->ignoreb, 
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			nminx, nminy, nmaxx, nmaxy);
		
#ifdef RANDOM8DEBUG
		thatunit = -1;
#endif

		if(path.size() == 0)
		{
			//InfoMess("np","np");
			continue;
		}
#endif

		//if(amt >0 && res == RES_CEMENT)
		//	InfoMess("bl","suc");

		u->mode = UMODE_GOSUP;
		u->goal = j->supcmpos;
		u->cargoreq = amt;
		u->cargotype = res;
		u->cargoamt = 0;
		u->target = target;
		u->target2 = target2;
		u->targtype = targtype;
		u->cdtype = cdtype;
		u->supplier = j->supbi;
		
		//if(amt >0 && res == RES_CEMENT)
		//	InfoMess("req c", "req c");
	
		if(targtype == TARG_BL)
		{
			Building* b = &g_building[target];
			b->transporter[res] = j->truckui;
		}
		else if(targtype == TARG_CD)
		{
			CdTile* ctile = GetCd(cdtype, target, target2, false);
			ctile->transporter[res] = j->truckui;
		}

		NewJob(UMODE_GODRIVE, j->truckui, -1, CONDUIT_NONE);
		if(g_frustum.pointin(u->drawpos.x, u->drawpos.y, u->drawpos.z) && 
			IsTileVis(g_localP, u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE))
			PlaySound(g_trsnd[TRSND_NEWJOB]);

		//InfoMess("fj", "fj");

		//g_log<<"job paths success "<<numpaths<<std::endl;

		return;
	}
	
#else	//ifndef TBID2

	Unit* u = &g_unit[bestunit];
	
#if 0
	if(res == RES_URANIUM)
	{
		char msg[1280];
		sprintf(msg, "ur del \n bestunit=%d \n bef u->mode=%d \n target=%d \n target2=%d \n cdtype=%d \n bestsup=%d \d amt=%d \d res=%d u->cargotype=%d", 
			bestunit, 
			(int)u->mode,
			target,
			target2,
			cdtype, 
			bestsup,
			amt,
			res,
			u->cargotype);
		InfoMess(msg, msg);
	}
#endif

	u->mode = UMODE_GOSUP;
	u->goal = bestsuppos;
	u->cargoreq = amt;
	u->cargotype = res;
	u->cargoamt = 0;
	u->target = target;
	u->target2 = target2;
	u->targtype = targtype;
	u->cdtype = cdtype;
	u->supplier = bestsup;
	
	if(targtype == TARG_BL)
	{
		Building* b = &g_building[target];
		b->transporter[res] = bestunit;
	}
	else if(targtype == TARG_CD)
	{
		CdTile* ctile = GetCd(cdtype, target, target2, false);
		ctile->transporter[res] = bestunit;
	}

	NewJob(UMODE_GODRIVE, bestunit, -1, CONDUIT_NONE);
	PlaySound(g_trsnd[TRSND_NEWJOB]);
	
#if 0
	if(res == RES_URANIUM)
	{
		char msg[1280];
		sprintf(msg, "ur del 2 \n bestunit=%d \n bef u->mode=%d \n target=%d \n target2=%d \n cdtype=%d \n bestsup=%d \d amt=%d \d res=%d u->cargotype=%d", 
			bestunit, 
			(int)u->mode,
			target,
			target2,
			cdtype, 
			bestsup,
			amt,
			res,
			u->cargotype);
		InfoMess(msg, msg);
	}
#endif

#endif
}

void ManageTrips()
{
	//g_freetrucks.clear();
	//return;

	//LastNum("pre cheap fuel");

	StartTimer(TIMER_MANAGETRIPS);

	//LastNum("pre buildings trip");
#ifdef TRANSPORT_DEBUG
	{
		char msg[128];
		sprintf(msg, "pre buildings trip");
		RichText debugrt;
		debugrt.m_part.push_back(msg);
		AddChat(&debugrt);
	}
#endif

#if 1
	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_TRUCK)
			continue;

		if(u->mode == UMODE_NONE)
			continue;

		//this unit must be stuck
		if(u->jobframes > CYCLE_FRAMES*5)
		{
			ResetMode(u);
			u->jobframes = -u->jobframes;	//let somebody else get this job
		}
	}
#endif

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		BlType* t = &g_bltype[b->type];
		//Player* p = &g_player[b->owner];

		// bid for transportation of resources to finished buildings
		if(b->finished)
		{
			//if(b->type == FACTORY)
			{
				//	char msg[128];
				//	sprintf(msg, "bid Bf 1 %s", t->name);
				//	Chat(msg);
			}
			//if(b->excessoutput())
			//	continue;
			//if(b->type == FACTORY)
			//	Chat("bid 1.1");
			//if(b->prodquota <= 0.0f)
			//	continue;
			//if(b->type == FACTORY)
			//	Chat("bid 1.2");
			for(int ri=0; ri<RESOURCES; ri++)
			{
				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 1");
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				if(t->input[ri] <= 0)
					continue;
				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 2");
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				Resource* r = &g_resource[ri];

				if(r->capacity)
					continue;

				if(r->conduit != CONDUIT_ROAD)
					continue;
				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 3");
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				if(!r->physical)
					continue;
				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 4");
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				if(ri == RES_LABOUR)
					continue;

				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 5");
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				//if(b->type == FACTORY)
				//	Chat("bid Bf 2");

				if(b->transporter[ri] >= 0)
					continue;

				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 6");
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				//if(b->type == FACTORY)
				//	Chat("bid Bf 3");

				int netreq = b->netreq(ri);

				if(netreq <= 0)
					continue;

				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 7");
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				//if(b->type == FACTORY)
				{
					//	char msg[128];
					//	sprintf(msg, "bid Bf 5 %s", t->name);
					//	Chat(msg);
				}
				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS)
				{
					char msg[128];
					sprintf(msg, "Tbid fini i%d, ri%d, netreq%d", i, ri, netreq);
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				TBid(i, -1, TARG_BL, UMODE_GODEMB, -1, ri, netreq);
			}
		}
		// bid for transportation of resources to building construction project
		else
		{
			for(int ri=0; ri<RESOURCES; ri++)
			{
				if(t->conmat[ri] <= 0)
					continue;

				Resource* r = &g_resource[ri];
				//Chat("1");

				if(r->capacity)
					continue;

				//Chat("2");

				if(!r->physical)
					continue;

				if(r->conduit != CONDUIT_ROAD)
					continue;

				if(ri == RES_LABOUR)
					continue;

				//if(ri == CEMENT)
				//	Chat("3");

				if(b->transporter[ri] >= 0)
					continue;

				//if(ri == CEMENT)
				//	Chat("4");

				//int netreq = b->netreq(ri);
				int netreq = t->conmat[ri] - b->conmat[ri];

				//if(ri == CEMENT)
				//	Chat("5");

				if(netreq <= 0)
					continue;

				//if(ri == CEMENT)
				//	Chat("6");
				
#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS)
				{
					char msg[128];
					sprintf(msg, "Tbid cst i%d, ri%d, netreq%d", i, ri, netreq);
					RichText debugrt;
					debugrt.m_part.push_back(msg);
					AddChat(&debugrt);
				}
#endif

				//if(netreq >0 && ri == RES_CEMENT)
				//	InfoMess("req c", "req c");

				TBid(i, -1, TARG_BL, UMODE_GODEMB, -1, ri, netreq);
			}
		}
	}

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int y=0; y<g_hmap.m_widthy; y++)
			for(unsigned char cti=0; cti<CONDUIT_TYPES; cti++)
			{
				CdTile* ctile = GetCd(cti, x, y, false);
				CdType* ctype = &g_cdtype[cti];

				if(!ctile->on)
					continue;
				if(ctile->finished)
					continue;

#ifdef TRUCK_DEBUG
				g_log<<"road bid 0"<<endl;
				g_log.flush();
#endif
				for(int ri=0; ri<RESOURCES; ri++)
				{
					if(ctype->conmat[ri] <= 0)
						continue;

					Resource* r = &g_resource[ri];
#ifdef TRUCK_DEBUG
					g_log<<"road bid 1"<<endl;
					g_log.flush();
#endif
					if(r->capacity)
						continue;

					if(r->conduit != CONDUIT_ROAD)
						continue;
#ifdef TRUCK_DEBUG
					g_log<<"road bid 2"<<endl;
					g_log.flush();
#endif
					if(ri == RES_LABOUR)
						continue;
#ifdef TRUCK_DEBUG
					g_log<<"road bid 3"<<endl;
					g_log.flush();
#endif
					if(ctile->transporter[ri] >= 0)
						continue;
#ifdef TRUCK_DEBUG
					g_log<<"road bid 4 ri="<<g_resource[ri].name<<endl;
					g_log.flush();
#endif
					int netreq = ctile->netreq(ri, cti);
					if(netreq <= 0)
						continue;
#ifdef TRUCK_DEBUG
					g_log<<"road bid 5"<<endl;
					g_log.flush();
#endif
					
#ifdef TRANSPORT_DEBUG
					//if(ri == RES_FARMPRODUCTS)
					{
						char msg[128];
						sprintf(msg, "Tbid cdt xy%d,%d cti%d, ri%d, netreq%d", x, y, cti, ri, netreq);
						RichText debugrt;
						debugrt.m_part.push_back(msg);
						AddChat(&debugrt);
					}
#endif
					
					//if(netreq >0 && ri == RES_CEMENT)
					//	InfoMess("req c", "req c");

					//Chat("bid r");
					TBid(x, y, TARG_CD, UMODE_GODEMCD, cti, ri, netreq);
				}
			}

	//LastNum("pre pick up bids");

	// pick which bid to take up

	//bool newjobs = false;

	//if(newjobs)
	//	NewJob();

	for(auto it=truckpathd.begin(); it!=truckpathd.end(); it++)
	{
		Unit* u = &g_unit[*it];
		u->jobframes = 0;
	}

	truckpathd.clear();
	

	StopTimer(TIMER_MANAGETRIPS);
}

#undef TRANSPORT_DEBUG