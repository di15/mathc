#include "building.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "bltype.h"
#include "../render/model.h"
#include "../render/foliage.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../render/shader.h"
#include "../math/barycentric.h"
#include "../math/physics.h"
#include "player.h"
#include "../render/transaction.h"
#include "selection.h"
#include "../gui/gui.h"
#include "../gui/widget.h"
#include "../gui/widgets/spez/cstrview.h"
#include "powl.h"
#include "road.h"
#include "crpipe.h"
#include "../../game/gmain.h"
#include "../render/shadow.h"
#include "../math/frustum.h"
#include "unit.h"
#include "simdef.h"
#include "labourer.h"
#include "../econ/demand.h"
#include "../math/fixmath.h"
#include "../ai/ai.h"
#include "simflow.h"
#include "../econ/utility.h"
#include "job.h"

//not engine
#include "../../game/gui/chattext.h"

Building g_building[BUILDINGS];

void Building::destroy()
{
	while(occupier.size() > 0)
	{
		int i = *occupier.begin();
		//Evict(&g_unit[i]);
		Unit* u = &g_unit[i];
		u->home = -1;
		occupier.erase( occupier.begin() );
	}

	while(worker.size() > 0)
	{
		int i = *worker.begin();
		//ResetMode(&g_unit[i]);
		Unit* u = &g_unit[i];
		u->mode = UMODE_NONE;
		Vec2i cmplace;
		if(PlaceUAb(u->type, u->cmpos, &cmplace))
		{
			u->cmpos = cmplace;
			u->fillcollider();
			u->drawpos.x = (float)u->cmpos.x;
			u->drawpos.z = (float)u->cmpos.y;
			u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);
		}
		worker.erase( worker.begin() );
	}

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(transporter[ri] < 0)
			continue;

		int ui = transporter[ri];
		Unit* u = &g_unit[ui];
		ResetMode(u);
	}

	int bi = this - g_building;

	for(auto csit=capsup.begin(); csit!=capsup.end(); csit++)
	{
		if(csit->src == bi)
			continue;

		Building* b2 = &g_building[csit->dst];
		
		auto csit2=b2->capsup.begin();
		
		while(csit2!=b2->capsup.end())
		{
			if(csit2->src == bi)
			{
				csit2 = b2->capsup.erase(csit2);
				continue;
			}

			csit2++;
		}
	}

	capsup.clear();

	on = false;
}

Building::Building()
{
	on = false;
}

Building::~Building()
{
	if(!on)
		return;

	destroy();
}

int Building::netreq(int rtype)
{
	//int prodleft = prodlevel - cymet;

	//leave reserves for next cycle
	int prodleft = (prodlevel * 3) - cymet;
	//int prodleft = (prodlevel * 2) - cymet;
	//int prodleft = (prodlevel * 4) - cymet;

	if(prodleft <= 0)
		return 0;

	BlType* bt = &g_bltype[type];

#if 0
	int lowr = -1;
	int lowamt = -1;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		if(lowr >= 0 && lowamt <= bt->input[ri])
			continue;

		lowr = ri;
		lowamt = bt->input[ri];
	}

	if(lowr < 0)
		return 0;
#endif

	int net = iceil(bt->input[rtype] * prodleft, RATIO_DENOM);
	
	Resource* r = &g_resource[rtype];

	if(!r->capacity)
		net -= stocked[rtype];
	else //don't use this for adjcaps(); as cymet reaches prodlevel, this will become negative as 
		//cap sources are no longer required, which makes them spike only at certain times, 
		//not be continuous like electricity cap usage,
	{
		int bi = this - g_building;

		for(auto csit=capsup.begin(); csit!=capsup.end(); csit++)
		{
			if(csit->src == bi)
				continue;

			if(csit->rtype != rtype)
				continue;

			net -= csit->amt;
		}
	}

	Player* py = &g_player[owner];

	//important: resources must be obtained from outlet bl's, 
	//even if they're globally available to the owner player.
	//if(r->physical)
	//	net -= py->global[rtype];

	return net;
}

bool Building::excin(int rtype)
{
#if 0
	//CBuildingType* bt = &g_buildingType[type];
	Player* py = &g_player[owner];

	int got = stocked[rtype];

	Resource* r = &g_resource[rtype];

	if(rtype != RES_LABOUR && r->physical)
		got += py->global[rtype];

	//if(got >= prodquota * bt->input[res])
	if(got >= netreq(rtype))
		return true;
#endif

	if(netreq(rtype) <= 0)
		return true;

	return false;
}

bool Building::metout()
{
	if(cymet >= prodlevel)
		return true;

	return false;
}

bool Building::hasworker(int ui)
{
	for(auto witer=worker.begin(); witer!=worker.end(); witer++)
		if(*witer == ui)
			return true;

	return false;
}


void Building::spawn(int utype, int uowner)
{
	Vec2i cmplace;

	if(!PlaceUAb(utype, tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2, &cmplace))
		return;

	PlaceUnit(utype, cmplace, uowner, NULL);
}

//try to manufacture a unit if one is in the production queue
bool Building::trymanuf()
{
	if(manufjob.size() <= 0)
		return false;

	ManufJob* mj = &*manufjob.begin();

	Player* py = &g_player[ owner ];
	UType* t = &g_utype[ mj->utype ];

	int netch[RESOURCES];
	Zero(netch);

	if(!TrySub(t->cost, py->global, stocked, py->local, netch, NULL))
		return false;

#if 0

	for(int i=0; i<RESOURCES; i++)
	{
		recenth.consumed[i] += netch[i];
	}

#endif

	spawn(mj->utype, mj->owner);
	manufjob.erase( manufjob.begin() );

	////if(owner == g_localP)
	{
		//if(g_selection.size() > 0 && g_selectType == SELECT_BUILDING && g_selection[0] == BuildingID(this))
		//	UpdateQueueCount(mj.utype);

		if(mj->owner == g_localP)
		{
			char finmsg[128];
			sprintf(finmsg, "%s manufactured.", t->name);
			RichText rt(finmsg);
			AddChat(&rt);
			//Chat(finmsg);
			//WaiFoO();
			// OnBuildU(mj.utype);
		}
	}

	return true;
}

//find a supplier building of a capacity resource type "rtype" and register the usage.
//if amt is negative, get rid of that much usage.
void Building::morecap(int rtype, int amt)
{
	int dembi = this - g_building;

	Resource* r = &g_resource[rtype];
	CdType* ct = &g_cdtype[r->conduit];
	short netw;
	std::list<short>* netwlist;

	//can't do anything if we're not connected to the grid of whatever conduit type this capacity resource is supplied by
	if(ct->blconduct)
	{
		netw = *(short*)(((char*)this)+ct->netwoff);

		if(netw < 0)
			return;
	}
	else
	{
		netwlist = (std::list<short>*)(((char*)this)+ct->netwoff);

		if(netwlist->size() <= 0)
			return;
	}

	//Vec2i demcmpos = tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	bool foundsup = false;

	//InfoMess(g_resource[rtype].name.c_str(), "supcap?");

	do
	{
		foundsup = false;
		//repeatedly find the best priced/distanced (based on utilty score) supplier of this cap resource,
		//until we either cannot get anymore supplied or we don't need anymore.

		int bestsrc = -1;
		int bestutil = -1;
		int bestamt = -1;

		for(int bi=0; bi<BUILDINGS; bi++)
		{
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(!b->finished)
				continue;

			BlType* bt = &g_bltype[b->type];

			if(bt->output[rtype] <= 0)
				continue;

			short netw2;
			std::list<short>* netwlist2;

			//must be connected by conduit
			if(ct->blconduct)
			{
				netw2 = *(short*)(((char*)b)+ct->netwoff);

				if(netw != netw2)
					continue;
			}
			else
			{
				netwlist2 = (std::list<short>*)(((char*)b)+ct->netwoff);

				bool foundnetw = false;

				//matching road networks? (might be connected to more than one seperate, isolated road grid.)
				for(auto nit=netwlist->begin(); nit!=netwlist->end(); nit++)
					for(auto nit2=netwlist2->begin(); nit2!=netwlist2->end(); nit2++)
					{
						if(*nit != *nit2)
							continue;

						foundnetw = true;
						break;
					}

				if(!foundnetw)
					continue;
			}

			int minlevel = b->maxprod();
			//minlevel is not enough to determine ability to supply capacity type resources.
			//as soon as the inputs are used up, they into the "cymet" counter, and won't be resupplied
			//until cymet is reset to 0, next cycle. so use whichever is greater.
			int suppliable = bt->output[rtype] * minlevel / RATIO_DENOM;
			suppliable = imax(suppliable, b->cymet * bt->output[rtype] / RATIO_DENOM);

			int supplying = 0;

			//check how much this supplier is already supplying to other demanders
			for(auto csit=b->capsup.begin(); csit!=b->capsup.end(); csit++)
			{
				if(csit->src != bi)
					continue;

				if(csit->rtype != rtype)
					continue;

				supplying += csit->amt;
			}

			suppliable -= supplying;

			if(suppliable <= 0)
				continue;

			//assuming here all capacity resource types are non-physical (thus global, as long as we're connected to the right conduit)
			int util = GlUtil(b->price[rtype]);

			if(bestutil >= 0 && bestutil > util)
				continue;

			bestutil = util;
			bestsrc = bi;
			bestamt = suppliable;
			foundsup = true;
		}

		if(!foundsup)
			break;

		int supplied = imin(amt, bestamt);

		Building* supb = &g_building[bestsrc];
		CapSup newcs;
		newcs.amt = supplied;
		newcs.dst = dembi;
		newcs.src = bestsrc;
		newcs.rtype = rtype;
		supb->capsup.push_back(newcs);
		this->capsup.push_back(newcs);

		amt -= supplied;

#if 0
		char msg[128];
		sprintf(msg, "supcap to:%s amt:%d", g_bltype[this->type].name, supplied);
		InfoMess(g_resource[rtype].name.c_str(), msg);
#endif

	}while(foundsup && amt > 0);
}

void Building::lesscap(int rtype, int amt)
{
	int dembi = this - g_building;

	//need to get rid of excess usage? (e.g., we've recently lowered the production level).

	//assuming price is the only factor to judge because of the assumption that all capacity resource types
	//are non-physical (thus global, as long as we're connected to the right conduit).

	int worstprc = -1;
	int worstsrc = -1;

	//InfoMess(g_resource[rtype].name.c_str(), "less cap");

	do
	{
		//repeatedly get rid of the more expensive sources until we've gotten rid of enough
		for(auto csit=capsup.begin(); csit!=capsup.end(); csit++)
		{
			if(csit->src == dembi)
				continue;

			if(csit->rtype != rtype)
				continue;

			Building* supb = &g_building[csit->src];

			if(worstprc >= 0 && worstprc > supb->price[rtype])
				continue;

			worstprc = supb->price[rtype];
			worstsrc = csit->src;
		}

		if(worstsrc < 0)
			break;

		//there might be more than one CapSup associated with this source for this demander,
		//so cycle through until we've got them all or amt == 0.
		auto csit=capsup.begin();
		while(csit!=capsup.end())
		{
			if(csit->src != worstsrc ||
				csit->rtype != rtype)
			{
				csit++;
				continue;
			}

			Building* supb = &g_building[csit->src];
			auto csit2=supb->capsup.begin();
			while(csit2!=supb->capsup.end())
			{
				if(csit2->src != csit->src ||
					csit2->rtype != csit->rtype ||
					csit2->amt != csit->amt ||
					csit2->dst != csit->dst)
				{
					csit2++;
					continue;
				}

				if(csit2->amt > -amt)
					csit2->amt += amt;
				else
					csit2 = supb->capsup.erase(csit2);
				break;
			}

			if(csit->amt > -amt)
			{
				csit->amt += amt;
				amt = 0;
				break;
			}
			else
			{
				amt += csit->amt;
				csit = capsup.erase(csit);
			}
		}
	}while(amt < 0);

	//InfoMess(g_resource[rtype].name.c_str(), "/less cap");
}

//update capacities - produce electricity, buy
void Building::adjcaps()
{
	int bi = this - g_building;
	BlType* bt = &g_bltype[type];

	//cap the supply to other buildings of this building's cap output resource types
	int minlevel = maxprod();

	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(!r->capacity)
			continue;

		int produced = minlevel * bt->output[ri] / RATIO_DENOM;
		//see note in function "adjcap" about "suppliable", also applies to "produced".
		produced = imax(produced, cymet * bt->output[ri] / RATIO_DENOM);
		int used = 0;

		auto csit=capsup.begin();
		while(csit!=capsup.end())
		{
			CapSup* cs = &*csit;

			//get users of this bl of this res

			if(cs->rtype != ri)
			{
				csit++;
				continue;
			}

			if(cs->src != bi)
			{
				csit++;
				continue;	//this must NOT be an indicator of a user of this bl
			}

			//remove this user
			if(used >= produced)
			{
				Building* b2 = &g_building[cs->dst];
				auto csit2=b2->capsup.begin();
		
				while(csit2!=b2->capsup.end())
				{
					//adjust the corresponding CapSup entry in the demander bl
					if(csit2->rtype == ri &&
						csit2->src == bi &&
						csit2->amt == csit->amt)
					{
						//InfoMess(g_bltype[b2->type].name, "cap erase");
						csit2 = b2->capsup.erase(csit2);
						break;
					}

					csit2++;
				}

				csit = capsup.erase(csit);
				continue;
			}
			//cap this user?
			else if(used + cs->amt > produced)
			{
				int suphere = produced - used;

				Building* b2 = &g_building[cs->dst];
				auto csit2=b2->capsup.begin();
		
				//adjust the corresponding CapSup entry in the demander bl
				while(csit2!=b2->capsup.end())
				{
					if(csit2->rtype == ri &&
						csit2->src == bi && 
						csit2->amt == csit->amt)
					{
						//InfoMess(g_bltype[b2->type].name, "cap erase");
						csit2->amt = suphere;
						break;
					}

					csit2++;
				}

				cs->amt = suphere;
				used = produced;
			}
			else
				used += cs->amt;

			csit++;
		}
	}

	//obtain capacity resource type sources...

#if 0
	int total[RESOURCES];
	Zero(total);
	
	for(auto csit=capsup.begin(); csit!=capsup.end(); csit++)
	{
		CapSup* cs = &*csit;

		if(cs->src == bi)
			continue;	//this must be an indicator of a user of this bl

		total[cs->rtype] += cs->amt;
	}

	for(int ri=0; ri<RESOURCES; ri++)
	{

	}
#else
	
	//repeat: obtain capacity resource type sources

	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(!r->capacity)
			continue;

		int req = iceil(bt->input[ri] * prodlevel, RATIO_DENOM);

		if(req <= 0)
			continue;

		//if(type == BL_CHEMPLANT)
		//	InfoMess(g_resource[ri].name.c_str(), "req cap >?");

		//see how much requisite amt is already supplied
		for(auto csit=capsup.begin(); csit!=capsup.end(); csit++)
		{
			if(csit->src == bi)
				continue;

			if(csit->rtype != ri)
				continue;

			req -= csit->amt;
		}

		if(req == 0)
		{
			//if(type == BL_CHEMPLANT)
			//	InfoMess(g_resource[ri].name.c_str(), "supd cap");

			continue;
		}
		
		//if(type == BL_CHEMPLANT)
		//	InfoMess(g_resource[ri].name.c_str(), "not supd cap");

		if(req < 0)
		{
			//if(type == BL_CHEMPLANT)
			//	InfoMess(g_resource[ri].name.c_str(), "less cap");

			lesscap(ri, req);
		}
		else if(req > 0)
		{
			//if(type == BL_CHEMPLANT)
			//	InfoMess(g_resource[ri].name.c_str(), "more cap");

			morecap(ri, req);
		}
	}

#endif

}

//requisition non-physical (hence "ethereal") resource types that aren't capacity types (e.g. crude oil).
//non-physical resources are probably all obtained by conduits other than roads.
//all capacity resource types are non-physical (ethereal), but not all non-physical resources are capacity types.
//edit: nevermind, crude oil is physical. I don't want to abuse the term. so I will check if it is transported by conduits or not.
void Building::getether()
{
	int bi = this - g_building;
	BlType* bt = &g_bltype[type];

	//repeat: obtain capacity resource type sources

	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(r->capacity)
			continue;

		//if(r->physical)
		//	continue;

		if(bt->input[ri] <= 0)
			continue;
		
		if(r->conduit == CONDUIT_ROAD)
			continue;

		int req = bt->input[ri] * prodlevel / RATIO_DENOM;

		req -= stocked[ri];

		if(req <= 0)
			continue;

		getether(ri, req);
	}
}

//buy through conduits
void Building::getether(int rtype, int amt)
{
	Player* py = &g_player[owner];
	int dembi = this - g_building;

	Resource* r = &g_resource[rtype];
	CdType* ct = &g_cdtype[r->conduit];
	short netw;
	std::list<short>* netwlist;

	//g_log<<"? get ether r"<<r->name<<" amt"<<amt<<std::endl;

	//can't do anything if we're not connected to the grid of whatever conduit type this capacity resource is supplied by
	if(ct->blconduct)
	{
		netw = *(short*)(((char*)this)+ct->netwoff);

		if(netw < 0)
			return;
	}
	else
	{
		netwlist = (std::list<short>*)(((char*)this)+ct->netwoff);

		if(netwlist->size() <= 0)
			return;
	}

	//Vec2i demcmpos = tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	bool found = false;

	//g_log<<"get ether r"<<r->name<<" amt"<<amt<<std::endl;

	do
	{
		//repeatedly find the best priced/distanced (based on utilty score) supplier of this cap resource,
		//until we either cannot get anymore supplied or we don't need anymore.

		int bestsrc = -1;
		int bestutil = -1;
		int bestamt = -1;

		for(int bi=0; bi<BUILDINGS; bi++)
		{
			Building* b = &g_building[bi];

			if(!b->on)
				continue;

			if(!b->finished)
				continue;

			BlType* bt = &g_bltype[b->type];

			if(bt->output[rtype] <= 0)
				continue;

			short netw2;
			std::list<short>* netwlist2;

			if(ct->blconduct)
			{
				netw2 = *(short*)(((char*)b)+ct->netwoff);

				if(netw != netw2)
					continue;
			}
			else
			{
				netwlist2 = (std::list<short>*)(((char*)b)+ct->netwoff);

				bool found = false;

				//matching road networks? (might be connected to more than one seperate, isolated road grid.)
				for(auto nit=netwlist->begin(); nit!=netwlist->end(); nit++)
					for(auto nit2=netwlist2->begin(); nit2!=netwlist2->end(); nit2++)
					{
						if(*nit != *nit2)
							continue;

						found = true;
						break;
					}

					if(!found)
						continue;
			}

			int suppliable = b->stocked[rtype];

			if(suppliable <= 0)
				continue;

			if(b->price[rtype] > py->global[RES_DOLLARS])
				continue;

			//assuming here all capacity resource types are non-physical (thus global, as long as we're connected to the right conduit)
			int util = GlUtil(b->price[rtype]);

			if(bestutil >= 0 && bestutil > util)
				continue;

			bestutil = util;
			bestsrc = bi;
			bestamt = suppliable;
			found = true;

			//g_log<<"found ether"<<std::endl;
		}

		if(!found)
			break;

		Building* b = &g_building[bestsrc];

		int supplied = imin(amt, bestamt);
		int price = imax(1, b->price[rtype]);
		supplied = imin(supplied, py->global[RES_DOLLARS] / price);

		if(supplied <= 0)
		{
			//no more suppliable, out of money?
			return;
		}

		int cost = supplied * b->price[rtype];
		py->global[RES_DOLLARS] -= cost;
		Player* supp = &g_player[b->owner];
		supp->global[RES_DOLLARS] += cost;
		stocked[rtype] += supplied;

		RichText transx;
		
		{
			r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", cost);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		{
			r = &g_resource[rtype];
			char numpart[128];
			sprintf(numpart, "%+d", -supplied);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
		&& b->owner == g_localP
#endif
		)
			NewTransx(b->drawpos, &transx);

		amt -= supplied;

	}while(found && amt > 0);
}

//get the maximum production level we can attain given our target and input levels
int Building::maxprod()
{
	BlType* bt = &g_bltype[type];
	int bi = this - g_building;

	int total[RESOURCES];
	Zero(total);
	Player* py = &g_player[owner];

	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		total[ri] += stocked[ri];

		//must still be obtained from a local resource, even if available globally (that local source can obtain it globally as an output)
		//if(r->physical)
		//	total[ri] += py->global[ri];
	}

	for(auto csit=capsup.begin(); csit!=capsup.end(); csit++)
	{
		CapSup* cs = &*csit;

		if(cs->src == bi)
			continue;	//this must be an indicator of a user of this bl

		total[cs->rtype] += cs->amt;
	}

	int minbund[RESOURCES];
	Zero(minbund);
	
	int minr = -1;
	int minamt = -1;
	int minlevel = prodlevel;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->input[ri] <= 0)
			continue;

		//if(minr >= 0 && bt->input[ri] >= minamt)
		//	continue;
		
		//the minimum level of production afforded by the bottleneck input 
		//resource (whichever one is restricting us from producing more).
		//this gives us a ratio out of max of RATIO_DENOM.
		//warning: there's potential for signed int overflow if total[minr] is some big number,
		//if the user is measuring resources in large amounts.
		int subminlevel = total[ri] * RATIO_DENOM / bt->input[ri];

		if(subminlevel > minlevel)
			continue;

		minlevel = subminlevel;
		minr = ri;
		minamt = bt->input[ri];
	}

	return minlevel;
}

//try to produce outputs if a minimum bundle of input resources is had
bool Building::tryprod()
{
	//TO DO: produce, update cymet
	if(g_simframe - lastcy >= CYCLE_FRAMES)
	{
		cymet = 0;
		lastcy = g_simframe;
		//return false;
	}

#ifdef PROD_DEBUG
	if(type != BL_NUCPOW)
		return false;
#endif

	BlType* bt = &g_bltype[type];
	int bi = this - g_building;

	//if we just completed a cycle and need labour, announce the new job
	if(g_simframe-lastcy == 0 &&
		netreq(RES_LABOUR) > 0)
		NewJob(UMODE_GOBLJOB, bi, -1, CONDUIT_NONE);

	if(cymet >= prodlevel)
		return false;

	int minlevel = maxprod();

#ifdef PROD_DEBUG
	g_log<<"min=r "<<minr<<" "<<g_resource[minr].name<<std::endl;
#endif

	
#ifdef PROD_DEBUG
	g_log<<"minlevel "<<minlevel<<" "<<g_resource[minr].name<<std::endl;
#endif

	
	int minbund[RESOURCES];
	Zero(minbund);
	bool somecon = false;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		minbund[ri] = bt->input[ri] * minlevel / RATIO_DENOM;

		Resource* r = &g_resource[ri];

		//prevent constant sound effect of production
		//for bl's that don't consume anything or have a 
		//constant capacity consumption of a single input
		if(minbund[ri] > 0 &&
			!r->capacity)
			somecon = true;
	}

	if(!somecon)
		return false;

	//enough to produce, go ahead

	cymet += minlevel;
	cymet = imin(cymet, RATIO_DENOM);
	RichText transx;

	//create output resources
	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(r->capacity)
		{
			//TO DO: check
			continue;
		}

		//add an amount of output based on the minimum level of production afforded by the 
		//bottleneck input resource (whichever one is restricting us from producing more).
		int add = bt->output[ri] * minlevel / RATIO_DENOM;
		stocked[ri] += add;

		//TO DO: capacity, what if not enough supplied

		if(add > 0)
		{
			char numpart[128];
			sprintf(numpart, "%+d", add);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}
	}

	Player* py = &g_player[owner];

	//subtract used raw inputs
	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		int take = minbund[ri];

#if 0	//only local subtractions. only outputs can be subtracted globally by demanders/transporters.
		//subtract from global stock as higher priority. only local when global runs out.
		if(r->physical)
		{
			int takeg = imin(take, py->global[ri]);
			py->global[ri] -= takeg;
			take -= takeg;
		}
#endif
		
		if(!r->capacity)
		{
			//remainder from local
			int takel = imin(take, stocked[ri]);
			py->local[ri] -= takel;
			stocked[ri] -= takel;
			take -= takel;
		}

		if(take > 0)
		{
			char numpart[128];
			sprintf(numpart, "%+d", -take);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}
	}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
	{
		NewTransx(drawpos, &transx);
		PlaySound(bt->sound[BLSND_PROD]);
	}

	//TO DO: capacity resources like electricity have to be handled completely differently
	//amount input depends on required output, depends on duration of cycle, must be upward limited by max gen capacity of that res for that bl type
	//so lower bound of cycle at 60 sec, and upper bound? for those bl's that output capacity? or just disallow change cycle delay for those types of bl's?

	//cymet = cymet % RATIO_DENOM;

	return true;
}

void RemWorker(Unit* w)
{
	Building* b = &g_building[w->target];

	int ui = w - g_unit;

	for(auto witer=b->worker.begin(); witer!=b->worker.end(); witer++)
		if(*witer == ui)
		{
			b->worker.erase(witer);
			return;
		}
}

void FreeBls()
{
	for(int i=0; i<BUILDINGS; i++)
	{
		g_building[i].destroy();
		g_building[i].on = false;
	}
}

int NewBl()
{
	for(int i=0; i<BUILDINGS; i++)
		if(!g_building[i].on)
			return i;

	return -1;
}

float CompletPct(int* cost, int* current)
{
	int totalreq = 0;

	for(int i=0; i<RESOURCES; i++)
	{
		totalreq += cost[i];
	}

	int totalhave = 0;

	for(int i=0; i<RESOURCES; i++)
	{
		totalhave += imin(cost[i], current[i]);
	}

	return (float)totalhave/(float)totalreq;
}

void Building::allocres()
{
	BlType* t = &g_bltype[type];
	Player* py = &g_player[owner];

	int alloc;

	RichText transx;

	for(int i=0; i<RESOURCES; i++)
	{
		if(t->conmat[i] <= 0)
			continue;

		if(i == RES_LABOUR)
			continue;

		alloc = t->conmat[i] - conmat[i];

		if(py->global[i] < alloc)
			alloc = py->global[i];

		conmat[i] += alloc;
		py->global[i] -= alloc;

		Resource* r = &g_resource[i];

		if(alloc > 0)
		{
			char numpart[128];
			sprintf(numpart, "%+d", -alloc);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( "\n" ) );
		}
	}

	if(transx.m_part.size() > 0
#ifdef LOCAL_TRANSX
		&& owner == g_localP
#endif
		)
		NewTransx(drawpos, &transx);

	checkconstruction();
}

void Building::remesh()
{
	BlType* t = &g_bltype[type];

	if(finished)
		CopyVA(&drawva, &g_model[t->model].m_va[0]);
	else
	{
		float pct = 0.1f + 0.9f * CompletPct(t->conmat, conmat);
		StageCopyVA(&drawva, &g_model[t->cmodel].m_va[0], pct);
	}

	if(t->hugterr)
		HugTerrain(&drawva, drawpos);

	drawva.genvbo();
}

bool Building::checkconstruction()
{
	BlType* t = &g_bltype[type];

	bool haveall = true;

	for(int i=0; i<RESOURCES; i++)
		if(conmat[i] < t->conmat[i])
		{
			haveall = false;
			break;
		}

#if 0
	if(owner == g_localP)
	{
		char msg[128];
		sprintf(msg, "%s construction complete.", t->name);
		Chat(msg);
		ConCom();
	}
#endif

	if(haveall && !finished)
		for(char ctype=0; ctype<CONDUIT_TYPES; ctype++)
			ReNetw(ctype);

	//if(owner == g_localP)
	//	OnFinishedB(type);

	finished = haveall;

	remesh();

	if(finished)
	{
		NewJob(UMODE_GOBLJOB, (int)(this-g_building), -1, CONDUIT_NONE);
		PlaySound(t->sound[BLSND_FINI]);
		hp = t->maxhp;

		Vec2i tmin;
		Vec2i tmax;

		tmin.x = tilepos.x - t->widthx/2;
		tmin.y = tilepos.y - t->widthy/2;
		tmax.x = tmin.x + t->widthx;
		tmax.y = tmin.y + t->widthy;

		int cmminx = tmin.x*TILE_SIZE;
		int cmminy = tmin.y*TILE_SIZE;
		int cmmaxx = cmminx + t->widthx*TILE_SIZE - 1;
		int cmmaxy = cmminy + t->widthy*TILE_SIZE - 1;

		ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);
	}

	return finished;
}

void DrawBl()
{
	Shader* s = &g_shader[g_curS];

	//return;
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		const BlType* t = &g_bltype[b->type];
		//const BlType* t = &g_bltype[BL_HOUSE];
		Model* m = &g_model[ t->model ];

		Vec3f vmin(b->drawpos.x - t->widthx*TILE_SIZE/2, b->drawpos.y, b->drawpos.z - t->widthy*TILE_SIZE/2);
		Vec3f vmax(b->drawpos.x + t->widthx*TILE_SIZE/2, b->drawpos.y + (t->widthx+t->widthy)*TILE_SIZE/2, b->drawpos.z + t->widthy*TILE_SIZE/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		if(!b->finished)
			m = &g_model[ t->cmodel ];

		/*
		m->draw(0, b->drawpos, 0);
		*/

		Player* py = &g_player[b->owner];
		float* color = py->color;
		glUniform4f(s->m_slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

		float pitch = 0;
		float yaw = 0;
		Matrix modelmat;
		float radians[] = {(float)DEGTORAD(pitch), (float)DEGTORAD(yaw), 0};
		modelmat.translation((const float*)&b->drawpos);
		Matrix rotation;
		rotation.rotrad(radians);
		modelmat.postmult(rotation);
		glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

		Matrix modelview;
#ifdef SPECBUMPSHADOW
		modelview.set(g_camview.m_matrix);
#endif
		modelview.postmult(modelmat);
		glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

		Matrix mvp;
#if 0
		mvp.set(modelview.m_matrix);
		mvp.postmult(g_camproj);
#elif 0
		mvp.set(g_camproj.m_matrix);
		mvp.postmult(modelview);
#else
		mvp.set(g_camproj.m_matrix);
		mvp.postmult(g_camview);
		mvp.postmult(modelmat);
#endif
		glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

		Matrix modelviewinv;
		Transpose(modelview, modelview);
		Inverse2(modelview, modelviewinv);
		//Transpose(modelviewinv, modelviewinv);
		glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

		VertexArray* va = &b->drawva;

		m->usetex();

		//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
		glVertexPointer(3, GL_FLOAT, 0, va->vertices);
		//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
		glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);

		//if(s->m_slot[SSLOT_NORMAL] != -1)
		//	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
		glNormalPointer(GL_FLOAT, 0, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, va->numverts);
	}
}

void UpdBls()
{
	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

#if 0
		if(rand()%100 == 1)
		{
			Explode(b);
			b->on = false;
			continue;
		}
#endif

		BlType* t = &g_bltype[b->type];
		EmitterPlace* ep;
		PlType* pt;

		if(!b->finished)
			continue;

#ifdef FREEZE_DEBUG
		g_log<<"upcaps"<<std::endl;
		g_log.flush();
#endif

		b->adjcaps();
		
#ifdef FREEZE_DEBUG
		g_log<<"getether"<<std::endl;
		g_log.flush();
#endif

		b->getether();
		
#ifdef FREEZE_DEBUG
		g_log<<"tryprod "<<std::endl;
		g_log.flush();
#endif

		b->tryprod();

#ifdef FREEZE_DEBUG
		g_log<<"trymanuf"<<std::endl;
		g_log.flush();
#endif

		b->trymanuf();

		//emit decorations
		if(b->cymet > 0)
			for(int j=0; j<MAX_B_EMITTERS; j++)
			{
				//first = true;

				//if(completion < 1)
				//	continue;

				ep = &t->emitterpl[j];

				if(!ep->on)
					continue;

				pt = &g_particleT[ep->type];

				if(b->emitterco[j].emitnext(pt->delay))
					EmitParticle(ep->type, b->drawpos + ep->offset);
			}
	}
}

void StageCopyVA(VertexArray* to, VertexArray* from, float completion)
{
	CopyVA(to, from);

	float maxy = 0;
	for(int i=0; i<to->numverts; i++)
		if(to->vertices[i].y > maxy)
			maxy = to->vertices[i].y;

	float limity = maxy*completion;

	for(int tri=0; tri<to->numverts/3; tri++)
	{
		Vec3f* belowv = NULL;
		Vec3f* belowv2 = NULL;

		for(int v=tri*3; v<tri*3+3; v++)
		{
			if(to->vertices[v].y <= limity)
			{
				if(!belowv)
					belowv = &to->vertices[v];
				else if(!belowv2)
					belowv2 = &to->vertices[v];

				//break;
			}
		}

		Vec3f prevt[3];
		prevt[0] = to->vertices[tri*3+0];
		prevt[1] = to->vertices[tri*3+1];
		prevt[2] = to->vertices[tri*3+2];

		Vec2f prevc[3];
		prevc[0] = to->texcoords[tri*3+0];
		prevc[1] = to->texcoords[tri*3+1];
		prevc[2] = to->texcoords[tri*3+2];

		for(int v=tri*3; v<tri*3+3; v++)
		{
			if(to->vertices[v].y > limity)
			{
				float prevy = to->vertices[v].y;
				to->vertices[v].y = limity;
#if 0
#if 0
				void barycent(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2,
					double vx, double vy, double vz,
					double *u, double *v, double *w)
#endif

					double ratio0 = 0;
				double ratio1 = 0;
				double ratio2 = 0;

				barycent(prevt[0].x, prevt[0].y, prevt[0].z,
					prevt[1].x, prevt[1].y, prevt[1].z,
					prevt[2].x, prevt[2].y, prevt[2].z,
					to->vertices[v].x, to->vertices[v].y, to->vertices[v].z,
					&ratio0, &ratio1, &ratio2);

				to->texcoords[v].x = ratio0 * prevc[0].x + ratio1 * prevc[1].x + ratio2 * prevc[2].x;
				to->texcoords[v].y = ratio0 * prevc[0].y + ratio1 * prevc[1].y + ratio2 * prevc[2].y;
#elif 0
				Vec3f* closebelowv = NULL;

				if(belowv)
					closebelowv = belowv;

				if(belowv2 && (!closebelowv || Magnitude2((*closebelowv) - to->vertices[v]) > Magnitude2((*belowv2) - to->vertices[v])))
					closebelowv = belowv2;

				float yratio = (closebelowv->y - prevy);
#endif
			}
		}
	}
}


void HeightCopyVA(VertexArray* to, VertexArray* from, float completion)
{
	CopyVA(to, from);

	float maxy = 0;
	for(int i=0; i<to->numverts; i++)
		if(to->vertices[i].y > maxy)
			maxy = to->vertices[i].y;

	float dy = maxy*(1.0f-completion);

	for(int tri=0; tri<to->numverts/3; tri++)
	{
		Vec3f* belowv = NULL;
		Vec3f* belowv2 = NULL;

		for(int v=tri*3; v<tri*3+3; v++)
		{
			to->vertices[v].y -= dy;
		}
	}
}

void HugTerrain(VertexArray* va, Vec3f pos)
{
	for(int i=0; i<va->numverts; i++)
	{
		va->vertices[i].y += Bilerp(&g_hmap, pos.x + va->vertices[i].x, pos.z + va->vertices[i].z) - pos.y;
	}
}

void Explode(Building* b)
{
	BlType* t = &g_bltype[b->type];
	float hwx = t->widthx*TILE_SIZE/2.0f;
	float hwz = t->widthy*TILE_SIZE/2.0f;
	Vec3f p;

	for(int i=0; i<5; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(PARTICLE_FIREBALL, p + b->drawpos);
	}

	for(int i=0; i<10; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(PARTICLE_FIREBALL2, p + b->drawpos);
	}
	/*
	for(int i=0; i<5; i++)
	{
	p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
	p.y = 8;
	p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
	EmitParticle(SMOKE, p + pos);
	}

	for(int i=0; i<5; i++)
	{
	p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
	p.y = 8;
	p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
	EmitParticle(SMOKE2, p + pos);
	}
	*/
	for(int i=0; i<20; i++)
	{
		p.x = hwx * (float)(rand()%1000 - 500)/500.0f;
		p.y = TILE_SIZE/2.5f;
		p.z = hwz * (float)(rand()%1000 - 500)/500.0f;
		EmitParticle(PARTICLE_DEBRIS, p + b->drawpos);
	}
}
