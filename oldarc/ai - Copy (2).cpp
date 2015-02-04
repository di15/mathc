#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../sim/build.h"
#include "../sim/simdef.h"
#include "../econ/utility.h"
#include "../sim/unit.h"

//#define AI_FRAMES	CYCLE_FRAMES/30
//#define AI_FRAMES	CYCLE_FRAMES
#define AI_FRAMES	1

void UpdAI()
{
	//return;	//do nothing for now
#if 0
	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		if(!p->on)
			continue;

		if(!p->ai)
			continue;
		
		CalcDem2(p);
		AdjPr(p);
	}
#endif

	static long long lastthink = -AI_FRAMES;

	if(g_simframe - lastthink < AI_FRAMES)
		return;

	//CalcDem1();

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		if(!p->on)
			continue;

		if(!p->ai)
			continue;

		UpdAI(p);
	}

	if(g_simframe - lastthink >= AI_FRAMES)
		lastthink = g_simframe;
}

//build buildings
bool Build(Player* p)
{
	//DemTree* dm = &g_demtree;
	int pi = p - g_player;
	DemTree* dm = &g_demtree2[pi];

	for(auto biter = dm->supbpcopy.begin(); biter != dm->supbpcopy.end(); biter ++)
	{
		DemsAtB* demb = (DemsAtB*)*biter;

		if(demb->bi >= 0)
			continue;

		int btype = demb->btype;

		Vec2i tpos;

		if(!PlaceBAb(btype, Vec2i(g_hmap.m_widthx/2, g_hmap.m_widthz/2), &tpos))
			continue;
		
#if 0
		{
			char msg[256];
			int cmdist = -1;
			Vec2i demcmpos = g_unit[0].cmpos;
			Vec2i supcmpos = tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
			cmdist = Magnitude(demcmpos - supcmpos);
			sprintf(msg, "placebl p%d bt:%s margpr%d profit%d minutil%d cmdist%d", pi, g_bltype[btype].name, demb->bid.marginpr, demb->bid.maxbid, demb->bid.minutil, cmdist);
			InfoMessage("r", msg);
		}
#endif

		if(!PlaceBl(btype, tpos, false, pi, &demb->bi))
			continue;

#if 0
		BlType* bt = &g_bltype[btype];
		g_log<<"place opp "<<bt->name<<" capitalization $"<<demb->bid.maxbid<<std::endl;

		int crtype = -1;

		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->output[ri] <= 0)
				continue;

			crtype = ri;
			break;
		}

		for(auto riter=dm->rdemcopy.begin(); riter!=dm->rdemcopy.end(); riter++)
		{
			RDemNode* rdem = (RDemNode*)*riter;

			if(rdem->rtype != crtype)
				continue;

			g_log<<"\t constr "<<g_resource[crtype].name.c_str()<<" dem minutil="<<rdem->bid.minutil<<"  maxbid="<<rdem->bid.maxbid<<" rdem->ramt="<<rdem->ramt<<std::endl;

			if(rdem->parent)
			{
				g_log<<"\t\t parenttype="<<rdem->parent->demtype<<std::endl;
			}
		}

		g_log<<"housing capit"<<std::endl;
		int htotal = 0;
		for(auto riter=dm->rdemcopy.begin(); riter!=dm->rdemcopy.end(); riter++)
		{
			RDemNode* rdem = (RDemNode*)*riter;

			if(rdem->rtype != RES_HOUSING)
				continue;

			htotal += rdem->bid.maxbid;

			g_log<<"\t housing "<<g_resource[RES_HOUSING].name.c_str()<<" dem minutil="<<rdem->bid.minutil<<"  maxbid="<<rdem->bid.maxbid<<" rdem->ramt="<<rdem->ramt<<std::endl;

			if(rdem->parent)
			{
				g_log<<"\t\t parenttype="<<rdem->parent->demtype<<std::endl;
			}
		}
		g_log<<"\t housing maxbid="<<htotal<<std::endl;

		g_log.flush();
#endif

		Building* b = &g_building[demb->bi];
		AdjPr(b);
#if 1
		//react to new price

		bool chpr;
		int nch = 0;

		do
		{
			chpr = false;

			for(int i=pi+1; i<PLAYERS; i++)
			{
				Player* p2 = &g_player[i];

				if(!p2->on)
					continue;

				if(!p2->ai)
					continue;
		
				CalcDem2(p2, false);
				chpr = AdjPr(p2) ? true : chpr;
			}

			for(int i=0; i<imin(pi+1,PLAYERS); i++)
			{
				Player* p2 = &g_player[i];

				if(!p2->on)
					continue;

				if(!p2->ai)
					continue;
		
				CalcDem2(p2, false);
				chpr = AdjPr(p2) ? true : chpr;
			}

			nch++;
		}while(chpr /* && nch < 5 */);
#endif

		return true;
	}

	return false;
}

//manufacture units
void Manuf(Player* p)
{
}

//adjust prices at building
bool AdjPr(Building* b)
{
	int pi = b->owner;
	Player* p = &g_player[pi];
	DemTree* dm = &g_demtree2[pi];
	BlType* bt = &g_bltype[b->type];
	Vec2i supcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	bool change = false;

	//for each output resource adjust price
	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;

		Resource* r = &g_resource[ri];
		std::list<DemNode*> rdems;

		int maxramt = 0;

		for(auto diter=dm->rdemcopy.begin(); diter!=dm->rdemcopy.end(); diter++)
		{
			RDemNode* rdem = (RDemNode*)*diter;

			if(rdem->rtype != ri)
				continue;

			rdems.push_back(rdem);
			maxramt += rdem->ramt;

			int cmdist = -1;
			Vec2i demcmpos;
			//Vec2i supcmpos;

			bool havedem = false;
			//bool havesup = false;

#if 0
			//does this rdem have a supplier bl?
			if(rdem->bi >= 0)
			{
				Building* b2 = &g_building[rdem->bi];
				supcmpos = b2->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
				havesup = true;
			}
			else
			{

			}
#endif

			DemNode* pardem = rdem->parent;
			havedem = DemCmPos(pardem, &demcmpos);

			if(havedem /* && havesup */)
				cmdist = Magnitude(demcmpos - supcmpos);
			else
				cmdist = MAX_UTIL;	//willingness to go anywhere

			int requtil = rdem->bid.minutil < 0 ? -1 : rdem->bid.minutil + 1;

			//if this is owned by the same player, we don't want to decrease price to compete
			if(rdem->bi >= 0)
			{
				Building* b2 = &g_building[rdem->bi];
				if(b2->owner == pi)
					//requtil = rdem->bid.minutil;
					continue;
			}

			//we need price in this case
			//we have a given building that we're optimizing, so distance is given

			int margpr = 0;

			//if there's no utility limit and only a limit on price
			if(requtil < 0)
			{
				margpr = rdem->bid.maxbid;
				
#if 0
				//if(ri == RES_HOUSING)
				//if(ri == RES_RETFOOD)
				{
					char msg[256];
					sprintf(msg, "adjpr?1 from$%d p%d %s b%d margpr%d minutil%d ramt%d cmdist%d", b->prodprice[ri], pi, g_resource[ri].name.c_str(), b-g_building, margpr, requtil, rdem->ramt, cmdist);
					InfoMessage("ap1", msg);
				}
#endif
			}
			else
			{
				margpr = r->physical ? InvPhUtilP(requtil, cmdist) : InvGlUtilP(requtil);
				
#if 0
				//if(ri == RES_HOUSING)
				//if(ri == RES_RETFOOD)
				{
					char msg[256];
					sprintf(msg, "adjpr?2 from$%d p%d %s b%d margpr%d minutil%d ramt%d cmdist%d", b->prodprice[ri], pi, g_resource[ri].name.c_str(), b-g_building, margpr, requtil, rdem->ramt, cmdist);
					InfoMessage("ap2", msg);
				}
#endif

				if(margpr <= 0)
					continue;

				while((r->physical ? PhUtil(margpr, cmdist) : GlUtil(margpr)) < requtil)
					margpr--;
			
				if(margpr <= 0)
					continue;
				
				//also, need to stay within budget, otherwise this generates wildly large prices
				if(margpr > rdem->bid.maxbid)
					margpr = rdem->bid.maxbid;
			}

#if 0
			//if(ri == RES_HOUSING)
			//if(ri == RES_RETFOOD)
			{
				char msg[256];
				sprintf(msg, "p%d %s b%d margpr%d minutil%d ramt%d cmdist%d", pi, g_resource[ri].name.c_str(), b-g_building, margpr, rdem->bid.minutil, rdem->ramt, cmdist);
				InfoMessage("ap3", msg);
			}
#endif

			rdem->bid.marginpr = margpr;
		}

		Bid bid;

		CombCo(b->type, &bid, ri, maxramt);

		int prevprc = -1;
		bool dupdm = false;
		int bestprofit = -1;
		int bestmaxr = -1;
		int bestprc = -1;
		int blmaxr = bt->output[ri];
		int maxbudg = 0;	//max consumer budget

		//evalute max projected revenue at tile and bltype
		//try all the price levels from smallest to greatest
		while(true)
		{
			int leastnext = prevprc;
			maxbudg = 0;

			//while there's another possible price, see if it will generate more total profit

			for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
				if(leastnext < 0 || ((*diter)->bid.marginpr < leastnext && (*diter)->bid.marginpr > prevprc))
					leastnext = (*diter)->bid.marginpr;

			if(leastnext == prevprc)
				break;

			prevprc = leastnext;

			//see how much profit this price level will generate

			int demramt = 0;	//how much will be demanded at this price level

			for(auto diter=rdems.begin(); diter!=rdems.end(); diter++)
				if((*diter)->bid.marginpr >= leastnext)
				{
					RDemNode* rdem = (RDemNode*)*diter;
					//curprofit += diter->ramt * leastnext;
					demramt += rdem->ramt;
					maxbudg += rdem->bid.maxbid;
				}

			//if demanded exceeds bl's max out
			if(demramt > blmaxr)
				demramt = blmaxr;

			int proramt = 0;	//how much is most profitable to produce in this case
			int curprofit;
			int currev;
			//find max profit based on cost composition and price
			MaxPro(bid.costcompo, leastnext, demramt, &proramt, maxbudg, &currev, &curprofit);

			//int ofmax = Ceili(proramt * RATIO_DENOM, bestmaxr);	//how much of max demanded is
			//curprofit += ofmax * bestrecur / RATIO_DENOM;	//bl recurring costs, scaled to demanded qty

			if(curprofit <= bestprofit)
				continue;

			//if(curprofit > maxbudg)
			//	curprofit = maxbudg;

			bestprofit = curprofit;
			bestmaxr = blmaxr;
			bestprc = leastnext;
		}

		if(bestprofit <= 0)
		{
			b->prodprice[ri] = 1;
			continue;
		}

#if 0
		if(ri == RES_HOUSING)
		//if(ri == RES_RETFOOD)
		//if(ri == RES_ENERGY)
		{
			char msg[1280];
			int bi = b - g_building;

			sprintf(msg, "adjpr %s b%d to$%d from$%d", g_resource[ri].name.c_str(), bi, bestprc, b->prodprice[ri]);
			
			for(int bi=0; bi<BUILDINGS; bi++)
			{
				Building* b2 = &g_building[bi];

				if(!b2->on)
					continue;

				BlType* b2t = &g_bltype[b2->type];

				if(b2t->output[ri] <= 0)
					continue;

				char submsg[128];
				sprintf(submsg, "\n p%d b%d pr$%d", b2->owner, bi, b2->prodprice[ri]);
				strcat(msg, submsg);
			}

			//InfoMessage("info", msg);
			g_log<<"----"<<std::endl<<msg<<std::endl;
			g_log.flush();
		}
#endif

		if(b->prodprice[ri] == bestprc)
			continue;

		b->prodprice[ri] = bestprc;

		change = true;
	}

	return change;
}

//adjust prices
bool AdjPr(Player* p)
{
	int pi = p - g_player;
	bool change = false;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(b->owner != pi)
			continue;

		change = AdjPr(b) ? true : change;
	}

	return change;
}

void UpdAI(Player* p)
{
	//if(p != g_player)
	//	return;

#if 0
	static bool once = false;

	if(once)
		return;

	once = true;
#endif
	
#if 0
	//OpenLog("log.txt", 123);
	g_log<<"a p "<<(int)(p-g_player)<<std::endl;
	g_log.flush();
#endif

	CalcDem2(p, true);
	
#if 0
	//OpenLog("log.txt", 123);
	g_log<<"b p "<<(int)(p-g_player)<<std::endl;
	g_log.flush();
#endif

	if(Build(p))
		return;
	
#if 0
	//OpenLog("log.txt", 123);
	g_log<<"c p "<<(int)(p-g_player)<<std::endl;
	g_log.flush();
#endif

	Manuf(p);
	
#if 0
	//OpenLog("log.txt", 123);
	g_log<<"d p "<<(int)(p-g_player)<<std::endl;
	g_log.flush();
#endif

	AdjPr(p);
	
#if 0
	//OpenLog("log.txt", 123);
	g_log<<"e p "<<(int)(p-g_player)<<std::endl;
	g_log.flush();
#endif
}