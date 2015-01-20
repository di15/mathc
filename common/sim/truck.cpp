

#include "truck.h"
#include "unit.h"
#include "transport.h"
#include "utype.h"
#include "building.h"
#include "bltype.h"
#include "conduit.h"
#include "simdef.h"
#include "../econ/utility.h"
#include "unitmove.h"
#include "../render/transaction.h"
#include "conduit.h"
#include "../sim/job.h"
#include "../econ/demand.h"
#include "../path/pathjob.h"

//not engine
#include "../../game/gui/chattext.h"

short g_trsnd[TR_SOUNDS] = {-1,-1,-1};

bool FindFuel(Unit* u)
{
	Vec2i bcmpos;
	int bestbi = -1;
	int bestutil = -1;

	//InfoMess("ff", "ff?");

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_FUEL] <= 0)
			continue;

		Player* py = &g_player[b->owner];

		if(b->stocked[RES_FUEL] + py->global[RES_FUEL] <= 0)
			continue;

		//if(b->price[RES_FUEL] > u->belongings[RES_DOLLARS])
		//	continue;

		bcmpos = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = Magnitude(pos - camera.Position()) * p->price[CONSUMERGOODS];
		int cmdist = FUELHEUR(bcmpos - u->cmpos);
		int thisutil = PhUtil(b->price[RES_FUEL], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return false;

	ResetGoal(u);
	u->fuelstation = bestbi;
	u->mode = UMODE_GOREFUEL;
	Building* b = &g_building[u->fuelstation];
	u->goal = b->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	//Pathfind();
	//float pathlen = pathlength();
	//g_log<<"found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)"<<endl;
	//g_log.flush();

	return true;
}

bool NeedFuel(Unit* u)
{
	if(u->belongings[RES_FUEL] < STARTING_FUEL*3/4)
		return true;

	return false;
}

void GoSup(Unit* u)
{
	//nothing here
}

void GoDemB(Unit* u)
{
#if 0
	if(mode == GOINGTODEMANDERB)
	{
		CBuilding* b = &g_building[target];

		if(!g_diplomacy[b->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMROAD)
	{
		CRoad* road = RoadAt(target, target2);

		if(!g_diplomacy[road->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPOWL)
	{
		CPowerline* powl = PowlAt(target, target2);

		if(!g_diplomacy[powl->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPIPE)
	{
		CPipeline* pipe = PipeAt(target, target2);

		if(!g_diplomacy[pipe->owner][owner])
			ResetMode();
	}
#endif

	//for building's we check for arrival upon a collision in MoveUnit()
	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
	//if(CheckIfArrived(u))
	//	OnArrived(u);
}

void GoRefuel(Unit* u)
{
#if 0
	CBuilding* b = &g_building[fuelStation];

	if(!g_diplomacy[b->owner][owner])
		ResetMode();
#endif

	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
}

void DoneRefuel(Unit* u)
{
	//g_log<<"after bid done refueling";
	//g_log.flush();

	u->fuelstation = -1;
	/*
	if(target2 >= 0)
	{
	mode = GOINGTOSUPPLIER;
	goal = g_building[target].pos;
	}
	else if(target >= 0)
	{
	mode = GOINGTODEMANDERB;
	goal = g_building[target].pos;
	}*//*
	if(transportAmt > 0)
	{
	if(targtype == GOINGTODEMANDERB)
	{
	mode = GOINGTODEMANDERB;
	goal = g_building[target].pos;
	}
	else if(targtype == GOINGTODEMROAD)
	{
	mode = GOINGTODEMROAD;
	goal = RoadPosition(target, target2);
	}
	else if(targtype == GOINGTODEMPIPE)
	{
	mode = GOINGTODEMPIPE;
	goal = PipelinePhysPos(target, target2);
	}
	else if(targtype == GOINGTODEMPOWL)
	{
	mode = GOINGTODEMPOWL;
	goal = PowerlinePosition(target, target2);
	}
	else
	{
	mode = targtype;
	ResetGoal();
	Pathfind();
	}
	}
	else if(targtype != NONE)
	{
	mode = GOINGTOSUPPLIER;
	goal = g_building[supplier].pos;
	}
	else*/
	{
		ResetMode(u);
		PlaySound(g_trsnd[TRSND_DONEJOB]);
	}
	//else
	//	ResetMode();
}

void DoRefuel(Unit* u)
{
	//InfoMess("do ref", "refling");

	if(u->belongings[RES_FUEL] >= STARTING_FUEL)
	{
		DoneRefuel(u);
		return;
	}

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(u->cyframes != 0)
		return;

	//last = GetTickCount();

	Building* b = &g_building[u->fuelstation];
	Player* up = &g_player[u->owner];
	Player* bp = &g_player[b->owner];

	if(b->stocked[RES_FUEL] + bp->global[RES_FUEL] < 0)
	{
		DoneRefuel(u);
		return;
	}

	int req = STARTING_FUEL - u->belongings[RES_FUEL];
	int got = req;

	if(b->stocked[RES_FUEL] + bp->global[RES_FUEL] < req)
		got = b->stocked[RES_FUEL] + bp->global[RES_FUEL];

	int paid;

	if(u->owner != b->owner)
	{
		//LogTransx(owner, -bp->price[ZETROL], "buy fuel");
		//LogTransx(b->owner, bp->price[ZETROL], "buy fuel");

		if(up->global[RES_DOLLARS] < b->price[RES_FUEL])
		{
			Bankrupt(u->owner, "fuel costs");
			DoneRefuel(u);
			return;
		}

		int canafford = got;

		if(b->price[u->cargotype] > 0)
			canafford = up->global[RES_DOLLARS] / b->price[RES_FUEL];

		if(canafford < got)
			got = canafford;

		paid = got * b->price[RES_FUEL];

		up->global[RES_DOLLARS] -= paid;
		bp->global[RES_DOLLARS] += paid;

#if 0
		up->recentth.consumed[CURRENC] += paid;
		bp->recentph[ZETROL].earnings += paid;
#endif

		if(up->global[RES_DOLLARS] <= 0)
			Bankrupt(u->owner, "fuel costs");
	}

	if(bp->global[RES_FUEL] >= got)
		bp->global[RES_FUEL] -= got;
	else
	{
		int subgot = got;
		subgot -= bp->global[RES_FUEL];
		bp->global[RES_FUEL] = 0;

		b->stocked[RES_FUEL] -= got;
		bp->local[RES_FUEL] -= got;
	}

#if 0
	b->recenth.consumed[RES_FUEL] += got;
#endif
	u->belongings[RES_FUEL] += got;

	//b->Emit(BARREL);
#ifdef LOCAL_TRANSX
	if(
		b->owner == g_localP &&
		b->owner != u->owner)
		NewTransx(b->drawpos, CURRENC, paid, ZETROL, -got);
#endif
	if(
#ifdef LOCAL_TRANSX
		owner == g_localP &&
#endif
		b->owner != u->owner)
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_FUEL];
			char numpart[128];
			sprintf(numpart, "%+d", got);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}
		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -paid);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(u->drawpos, &transx);

		//NewTransx(b->drawpos, ZETROL, got, CURRENC, -paid);
	}

	DoneRefuel(u);
}

void AtDemB(Unit* u)
{
	//LastNum("at demander b");

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(u->cyframes != 0)
		return;

	Building* b = &g_building[u->target];

	if(b->finished)
		//b->stock[u->cargotype] += 1.0f;
			b->stocked[u->cargotype] += u->cargoamt;
	else
	{
		//b->conmat[u->cargotype] += 1.0f;
		b->conmat[u->cargotype] += u->cargoamt;
		b->checkconstruction();
	}

#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[u->cargotype];
			char numpart[128];
			sprintf(numpart, "%+d", u->cargoamt);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(u->drawpos, &transx);

		//NewTransx(b->pos, transportRes, 1.0f);
		//NewTransx(b->pos, transportRes, transportAmt);
	}

	//u->cargoamt -= 1;
	u->cargoamt = 0;

	//char msg[128];
	//sprintf(msg, "unloaded %0.2f/%0.2f %s", 1.0f, transportAmt, g_resource[transportRes].name);
	//Chat(msg);

	if(u->cargoamt <= 0)
	{
		b->transporter[u->cargotype] = -1;
		ResetMode(u);
	}

	PlaySound(g_trsnd[TRSND_DONEJOB]);
}

void DoneAtSup(Unit* u)
{
	//g_log<<"after bid done at sup";
	//g_log.flush();

	//LastNum("dat sup 1");

	if(u->cargoamt <= 0)
	{
		ResetMode(u);
		return;
	}
	//LastNum("dat sup 2");

	Building* supb = &g_building[u->supplier];
	//supb->transporter[u->cargotype] = -1;
	//u->mode = u->targtype;

	//LastNum("dat sup 3");
	if(u->targtype == TARG_BL)
	{
		Building* demb = &g_building[u->target];
		u->goal = demb->tilepos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		u->mode = UMODE_GODEMB;
	}
	else if(u->targtype == TARG_CD)
	{
		CdType* ctype = &g_cdtype[u->cdtype];
		//CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
		u->goal = Vec2i(u->target, u->target2) * TILE_SIZE + ctype->physoff;
		u->mode = UMODE_GODEMCD;
	}
	//LastNum("dat sup 4");

	NewJob(UMODE_GODRIVE, (int)(u-g_unit), -1, CONDUIT_NONE);
	//LastNum("dat sup 5");

	PlaySound(g_trsnd[TRSND_DONEJOB]);
}

void AtSup(Unit* u)
{
	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	//LastNum("at sup 1");

	if(u->cyframes != 0)
		return;

	Building* supb = &g_building[u->supplier];
	Building* demb;
	Player* demp = NULL;
	Player* supp = &g_player[supb->owner];
	BlType* demt;
	CdTile* demcd;
	//LastNum("at sup 2");

	//char msg[128];
	//sprintf(msg, "at sup targtype=%d @%d,%d", targtype, target, target2);
	//LastNum(msg);

	int demplayer = -1;

	if(u->targtype == TARG_BL)
	{
		demb = &g_building[u->target];
		demplayer = demb->owner;
		demp = &g_player[demb->owner];
		demt = &g_bltype[demb->type];
	}
	else if(u->targtype == TARG_CD)
	{
		demcd = GetCd(u->cdtype, u->target, u->target2, false);
		demplayer = demcd->owner;
		demp = &g_player[demcd->owner];
	}

	//LastNum("at sup 3");
	//if(supb->stock[transportRes] + supp->global[transportRes] <= 0.0f)
	//	DoneAtSupplier();

	//LastNum("at sup 1");

	//if(demb->stock[transportRes] + demp->global[transportRes] + transportAmt >= demt->input[transportRes])
	if(u->cargoamt >= u->cargoreq)
	{
		DoneAtSup(u);
		return;
	}

	//LastNum("at sup 2");
	//LastNum("at sup 4");

	//float required = demt->input[transportRes] - transportAmt - demb->stock[transportRes] - demp->global[transportRes];
	int reqnow = u->cargoreq - u->cargoamt;
	int got = reqnow;

	if(supb->stocked[u->cargotype] + supp->global[u->cargotype] < reqnow)
		got = supb->stocked[u->cargotype] + supp->global[u->cargotype];

	int paid;

	//LastNum("at sup 5");
	//LastNum("at sup 3");
	//if(targtype == GOINGTODEMANDERB)
	{
		//if(demb->owner != supb->owner)
		if(demp != supp)
		{
			if(demp->global[RES_DOLLARS] < supb->price[u->cargotype])
			{
				char reason[64];
				Resource* r = &g_resource[u->cargotype];
				sprintf(reason, "buying %s", r->name.c_str());
				Bankrupt(demplayer, reason);
				DoneAtSup(u);
				return;
			}

			int canafford = got;

			//LastNum("at sup 5.2");
			if(supb->price[u->cargotype] > 0)
				canafford = demp->global[RES_DOLLARS] / supb->price[u->cargotype];

			if(canafford < got)
				got = canafford;
			//LastNum("at sup 4");

			paid = got * supb->price[u->cargotype];

			//LastNum("at sup 5.3");
			demp->global[RES_DOLLARS] -= paid;
			supp->global[RES_DOLLARS] += paid;

#if 0
			supb->recenth.produced[CURRENC] += paid;
			if(targtype == GOINGTODEMANDERB)
				demb->recenth.consumed[CURRENC] += paid;
			else
				demp->recentth.consumed[CURRENC] += paid;
			supp->recentph[transportRes].earnings += paid;
#endif

			//LastNum("at sup 5.4");
			//char msg[128];
			//sprintf(msg, "purchase %s", g_resource[transportRes].name);
			//LogTransx(supb->owner, paid, msg);
			//LogTransx(demplayer, -paid, msg);
		}
	}
	//LastNum("at sup 6");

	//LastNum("at sup 5");
	if(supp->global[u->cargotype] >= got)
		supp->global[u->cargotype] -= got;
	else
	{
		//LastNum("at sup 6");
		float subgot = got;
		subgot -= supp->global[u->cargotype];
		supp->global[u->cargotype] = 0;

		supb->stocked[u->cargotype] -= subgot;
		supp->local[u->cargotype] -= subgot;
	}

	//LastNum("at sup 7");
	//LastNum("at sup 7");
	u->cargoamt += got;
#if 0
	supb->recenth.consumed[u->cargotype] += got;
#endif

	Vec3f transxpos = supb->drawpos;

#ifdef LOCAL_TRANSX
	if(supb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, CURRENC, paid, transportRes, -got);
	if(demb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, transportRes, got, CURRENC, -paid);
#else
	{
		RichText transx;

		{
			Resource* r = &g_resource[u->cargotype];
			char numpart[128];
			sprintf(numpart, "%+d", -got);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(transxpos, &transx);

		//NewTransx(transxpos, u->cargotype, -got);
	}
#endif

	//LastNum("at sup 8");
	DoneAtSup(u);
	//LastNum("at sup 9");
}

void GoDemCd(Unit* u)
{
	//transporters can only deliver a certain resource to a building/conduit one at a time
	//(two transporters can't deliver the same res type). so we don't need to check if somebody
	//already delivered enough. we just need to check if we've arrived at a conduit because
	//we don't collide with them.
	if(CheckIfArrived(u))
		OnArrived(u);
}

void AtDemCd(Unit* u)
{
	//LastNum("at demander b");

	//if(GetTickCount() - last < WORK_DELAY)
	//	return;

	if(u->cyframes != 0)
		return;

	//last = GetTickCount();

	CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);

	ctile->conmat[u->cargotype] += 1;
	u->cargoamt -= 1;

#ifdef LOCAL_TRANSX
	if(r->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[u->cargotype];
			char numpart[128];
			//sprintf(numpart, "%+d", u->cargoamt);
			sprintf(numpart, "%+d", 1);
			transx.m_part.push_back( RichPart( numpart ) );
			transx.m_part.push_back( RichPart( RICHTEXT_ICON, r->icon ) );
			//transx.m_part.push_back( RichPart( r->name.c_str() ) );
			transx.m_part.push_back( RichPart( " \n" ) );
		}

		NewTransx(ctile->drawpos, &transx);

		//NewTransx(b->pos, transportRes, 1.0f);
		//NewTransx(b->pos, transportRes, transportAmt);

		PlaySound(g_trsnd[TRSND_DONEJOB]);
	}

	if(u->cargoamt <= 0)
	{
		ctile->transporter[u->cargotype] = -1;
		ResetMode(u);
	}

	ctile->checkconstruction(u->cdtype);
}

void UpdTruck(Unit* u)
{
	StartTimer(TIMER_UPDTRUCK);
	//return;	//do nothing for now

	//u->jobframes ++;

	if(u->mode != UMODE_NONE)
	{
		u->cyframes--;

		if(u->cyframes < 0)
			u->cyframes = WORK_DELAY-1;

		if(u->cyframes % SIM_FRAME_RATE == 0 && u->cmpos != u->prevpos)
			u->belongings[RES_FUEL] -= TRUCK_CONSUMPRATE;

		//prevent job from expiring from inactivity ("stuck")
		if(u->cmpos != u->prevpos)
			u->jobframes = TBID_DELAY;

		Unit* ignoreu = NULL;

		if(u->driver >= 0)
			ignoreu = &g_unit[u->driver];

#if 0
		//Doesn't work, in situations where 
		//approaching driver blocks path.
		if(Trapped(u, ignoreu))
		{
			ResetMode(u);
			return;
		}
#endif
	}

	if(u->belongings[RES_FUEL] <= 0)
	{
		char msg[128];
		sprintf(msg, "%s Truck out of fuel! (Remaining %d)", Time().c_str(), CountU(UNIT_TRUCK));
		g_log<<msg<<std::endl;
		g_log<<"\tRemain: "<<CountU(UNIT_TRUCK)<<std::endl;
		RichText sr;
		sr.m_part.push_back(UString(msg));
		//SubmitConsole(&sr);
		AddChat(&sr);
		//SubmitConsole(&sr);
		AddChat(&sr);
		u->destroy();
		return;
	}

#if 0
	if(u->path.size() > 0)
	{
		InfoMess("trp", "trp");
	}
#endif

	//if(NeedFuel(u))
	//	InfoMess("nf", "nf");

	switch(u->mode)
	{
	case UMODE_NONE:
		{
			if(NeedFuel(u) && FindFuel(u))
			{
				//InfoMess("ff", "ff");
				//g_log<<"find fuel"<<endl;
				//g_log.flush();
			}
			else if(u->mode == UMODE_NONE && u->target < 0)
			{
				//	FindDemander();
				if(u->jobframes >= TBID_DELAY)
				{
					//g_freetrucks.push_back(UnitID(this));
					//g_log<<"free truck"<<endl;
					//g_log.flush();
				}
				u->jobframes ++;
			}
		} break;

	case UMODE_GOSUP:
		GoSup(u);
		break;

	case UMODE_GODEMB:
		GoDemB(u);
		break;

	case UMODE_GOREFUEL:
		GoRefuel(u);
		break;

	case UMODE_REFUELING:
		DoRefuel(u);
		break;

	case UMODE_ATDEMB:
		AtDemB(u);
		break;

	case UMODE_ATSUP:
		AtSup(u);
		break;

	case UMODE_GODEMCD:
		GoDemCd(u);
		break;

	case UMODE_ATDEMCD:
		AtDemCd(u);
		break;

	default:
		break;
	}

	StopTimer(TIMER_UPDTRUCK);
}
