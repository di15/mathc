#include "unit.h"
#include "../render/shader.h"
#include "utype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../render/model.h"
#include "../math/hmapmath.h"
#include "unitmove.h"
#include "simdef.h"
#include "simflow.h"
#include "labourer.h"
#include "../debug.h"
#include "../math/frustum.h"
#include "building.h"
#include "labourer.h"
#include "truck.h"
#include "../econ/demand.h"
#include "../math/vec4f.h"
#include "../path/pathjob.h"
#include "../render/fogofwar.h"

#ifdef RANDOM8DEBUG
int thatunit = -1;
#endif

Unit g_unit[UNITS];

Unit::Unit()
{
	on = false;
	threadwait = false;
}

Unit::~Unit()
{
	destroy();
}

void Unit::destroy()
{
	on = false;
	threadwait = false;

	if(home >= 0)
		Evict(this);

	if(mode == UMODE_GOSUP
		//|| mode == GOINGTOREFUEL
			//|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
				)
	{
		if(supplier >= 0)
		{
			//Building* b = &g_building[supplier];
			//b->transporter[cargotype] = -1;
		}
	}

	if(type == UNIT_TRUCK)
	{
		bool targbl = false;
		bool targcd = false;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_BL )
			targbl = true;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_CD )
			targbl = true;

		if(mode == UMODE_GODEMB)
			targbl = true;

		if(mode == UMODE_ATDEMB)
			targbl = true;

		if(mode == UMODE_GODEMCD)
			targcd = true;

		if(mode == UMODE_ATDEMCD)
			targcd = true;

		if(targtype == TARG_BL)
			targbl = true;
		
		if(targtype == TARG_CD)
			targcd = true;

		if( targbl )
		{
			if(target >= 0)
			{
				Building* b = &g_building[target];
				b->transporter[cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(target >= 0 && target2 >= 0 && cdtype >= 0)
			{
				CdTile* ctile = GetCd(cdtype, target, target2, false);
				ctile->transporter[cargotype] = -1;
			}
		}
	}
}

void Unit::resetpath()
{
	path.clear();
	subgoal = cmpos;
	goal = cmpos;
	pathblocked = false;
}

void DrawUnits()
{
	Shader* s = &g_shader[g_curS];

	for(int i=0; i<UNITS; i++)
	{
		StartTimer(TIMER_DRAWUMAT);

		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hidden())
			continue;

		int tx = u->cmpos.x / TILE_SIZE;
		int ty = u->cmpos.y / TILE_SIZE;

		if(!IsTileVis(g_localP, tx, ty))
			continue;

		UType* t = &g_utype[u->type];

		Vec3f vmin(u->drawpos.x - t->size.x/2, u->drawpos.y, u->drawpos.z - t->size.x/2);
		Vec3f vmax(u->drawpos.x + t->size.x/2, u->drawpos.y + t->size.y, u->drawpos.z + t->size.x/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		Player* py = &g_player[u->owner];
		float* color = py->color;
		glUniform4f(s->m_slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

		Model* m = &g_model[t->model];

		StopTimer(TIMER_DRAWUMAT);

		m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.y);
	}
}

int NewUnit()
{
	for(int i=0; i<UNITS; i++)
		if(!g_unit[i].on)
			return i;

	return -1;
}

// starting belongings for labourer
void StartBel(Unit* u)
{
	Zero(u->belongings);
	u->car = -1;
	u->home = -1;

	if(u->type == UNIT_LABOURER)
	{
		//if(u->owner >= 0)
		{
			//u->belongings[ RES_DOLLARS ] = 100;
			//u->belongings[ RES_DOLLARS ] = CYCLE_FRAMES * LABOURER_FOODCONSUM * 30;
			u->belongings[ RES_DOLLARS ] = CYCLE_FRAMES/SIM_FRAME_RATE * LABOURER_FOODCONSUM * 10;
		}

		u->belongings[ RES_RETFOOD ] = STARTING_RETFOOD;
		u->belongings[ RES_LABOUR ] = STARTING_LABOUR;
	}
	else if(u->type == UNIT_TRUCK)
	{
		u->belongings[ RES_FUEL ] = STARTING_FUEL;
	}
}

bool PlaceUnit(int type, Vec2i cmpos, int owner, int *reti)
{
	int i = NewUnit();

	if(i < 0)
		return false;

	if(reti)
		*reti = i;

#if 0
	bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec3i cmpos;
	Vec3f facing;
	Vec2f rotation;

	deque<Vec2i> path;
	Vec2i goal;

	int step;
	int target;
	int target2;
	bool targetu;
	bool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	bool passive;
	Vec2i prevpos;
	int taskframe;
	bool pathblocked;
	int jobframes;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//std::vector<TransportJob> bids;

	float frame[2];
#endif

	Unit* u = &g_unit[i];
	UType* t = &g_utype[type];

	u->on = true;
	u->type = type;
	u->cmpos = cmpos;
	u->drawpos = Vec3f(cmpos.x, g_hmap.accheight(cmpos.x, cmpos.y), cmpos.y);
	u->owner = owner;
	u->path.clear();
	u->goal = cmpos;
	u->target = -1;
	u->target2 = -1;
	u->targetu = false;
	u->underorder = false;
	u->fuelstation = -1;
	u->targtype = TARG_NONE;
	//u->home = -1;
	StartBel(u);
	u->hp = t->starthp;
	u->passive = false;
	u->prevpos = u->cmpos;
	u->taskframe = 0;
	u->pathblocked = false;
	u->jobframes = 0;
	u->supplier = -1;
	u->reqamt = 0;
	u->targtype = -1;
	u->frame[BODY_LOWER] = 0;
	u->frame[BODY_UPPER] = 0;
	u->subgoal = u->goal;

	u->mode = UMODE_NONE;
	u->pathdelay = 0;
	u->lastpath = g_simframe;

	u->cdtype = CONDUIT_NONE;
	u->driver = -1;
	//u->framesleft = 0;
	u->cyframes = WORK_DELAY-1;
	u->cargoamt = 0;
	u->cargotype = -1;

	u->fillcollider();
	AddVis(u);
	Explore(u);

	return true;
}

void FreeUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		g_unit[i].destroy();
		g_unit[i].on = false;
	}
}

bool Unit::hidden() const
{
	switch(mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		return true;
	default:break;
	}

	return false;
}

void AnimUnit(Unit* u)
{
	UType* t = &g_utype[u->type];

	if(u->type == UNIT_BATTLECOMP || u->type == UNIT_LABOURER)
	{
		if(u->prevpos == u->cmpos)
		{
			u->frame[BODY_LOWER] = 0;
			return;
		}

		PlayAni(u->frame[BODY_LOWER], 0, 29, true, 1.0f);
	}
}

void UpdAI(Unit* u)
{
	//return;	//do nothing for now?

	if(u->type == UNIT_LABOURER)
		UpdLab(u);
	else if(u->type == UNIT_TRUCK)
		UpdTruck(u);
}

void UpdCheck(Unit* u)
{
	if(u->type == UNIT_LABOURER)
		UpdLab2(u);
}

void UpdUnits()
{
	for(int i = 0; i < UNITS; i++)
	{
		StartTimer(TIMER_UPDUONCHECK);

		Unit* u = &g_unit[i];

		if(!u->on)
		{
			StopTimer(TIMER_UPDUONCHECK);
			continue;
		}

		StopTimer(TIMER_UPDUONCHECK);

		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
		StartTimer(TIMER_MOVEUNIT);
		MoveUnit(u);
		StopTimer(TIMER_MOVEUNIT);
		//must be called after Move... labs without paths are stuck
		UpdCheck(u);
		StartTimer(TIMER_ANIMUNIT);
		AnimUnit(u);
		StopTimer(TIMER_ANIMUNIT);
	}
}

void ResetPath(Unit* u)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		g_log<<"the 13th unit:"<<std::endl;
		g_log<<"path reset"<<std::endl;
		InfoMess("pr", "pr");
	}
#endif

	u->path.clear();
	u->tpath.clear();

#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		g_log<<"ResetPath u=thatunit"<<std::endl;
	}
#endif
}

void ResetGoal(Unit* u)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		g_log<<"the 13th unit:"<<std::endl;
		g_log<<"g reset"<<std::endl;
		InfoMess("rg", "rg");
	}
#endif

	u->goal = u->subgoal = u->cmpos;
	ResetPath(u);
}

void ResetMode(Unit* u)
{
#ifdef RANDOM8DEBUG
	if(u - g_unit == thatunit)
	{
		g_log<<"\tResetMode u=thatunit"<<std::endl;
	}
#endif

#ifdef HIERDEBUG
	if(u - g_unit == 5 && u->mode == UMODE_GOBLJOB && u->target == 5)
	{
		InfoMess("rsu5", "rsu5");
	}
#endif

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_unit == 19)
	{
		g_log<<"the 13th unit:"<<std::endl;
		g_log<<"mode reset"<<std::endl;
		char msg[128];
		sprintf(msg, "rm %s prevm=%d", g_utype[u->type].name, (int)u->mode);
		InfoMess("rm", msg);
	}
#endif

#if 0
	switch(u->mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		u->freecollider();
		PlaceUAb(u->type, u->cmpos, &u->cmpos);
		u->fillcollider();
	default:break;
	}
#else
	if(u->hidden())
	{
		u->freecollider();
		RemVis(u);
		PlaceUAb(u->type, u->cmpos, &u->cmpos);
		u->drawpos.x = u->cmpos.x;
		u->drawpos.z = u->cmpos.y;
		u->drawpos.y = g_hmap.accheight(u->cmpos.x, u->cmpos.y);
		u->fillcollider();
		AddVis(u);
		Explore(u);
	}
#endif

#if 0
	//URAN_DEBUG
	if(u-g_unit == 19)
	{
		Building* b = &g_building[5];
		char msg[1280];
		sprintf(msg, "ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d", 
			(int)b->transporter[RES_URANIUM], 
			(int)g_unit[b->transporter[RES_URANIUM]].mode,
			(int)g_unit[b->transporter[RES_URANIUM]].target,
			5,
			(int)g_unit[b->transporter[RES_URANIUM]].targtype,
			(int)g_unit[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 1");
	if(u->type == UNIT_LABOURER)
	{
		//LastNum("resetmode 1a");
		if(u->mode == UMODE_BLJOB)
			RemWorker(u);

		//if(hidden())
		//	relocate();
	}
	else if(u->type == UNIT_TRUCK)
	{
#if 1
		if(u->mode == UMODE_GOSUP
		                //|| mode == GOINGTOREFUEL
		                //|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
		  )
		{
			if(u->supplier >= 0)
			{
				//necessary?
				Building* b = &g_building[u->supplier];
				b->transporter[u->cargotype] = -1;
			}
		}

		bool targbl = false;
		bool targcd = false;

		if( (u->mode == UMODE_GOSUP || u->mode == UMODE_ATSUP || u->mode == UMODE_GOREFUEL || u->mode == UMODE_REFUELING) && u->targtype == TARG_BL )
			targbl = true;
		
		if( (u->mode == UMODE_GOSUP || u->mode == UMODE_ATSUP || u->mode == UMODE_GOREFUEL || u->mode == UMODE_REFUELING) && u->targtype == TARG_CD )
			targbl = true;
		
		if(u->mode == UMODE_GODEMB)
			targbl = true;

		if(u->mode == UMODE_ATDEMB)
			targbl = true;

		if(u->mode == UMODE_GODEMCD)
			targcd = true;

		if(u->mode == UMODE_ATDEMCD)
			targcd = true;
		
		if(u->targtype == TARG_BL)
			targbl = true;
		
		if(u->targtype == TARG_CD)
			targcd = true;

		if( targbl )
		{
			if(u->target >= 0)
			{
				Building* b = &g_building[u->target];
				b->transporter[u->cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(u->target >= 0 && u->target2 >= 0 && u->cdtype >= 0)
			{
				CdTile* ctile = GetCd(u->cdtype, u->target, u->target2, false);
				ctile->transporter[u->cargotype] = -1;
			}
		}
#endif
		u->targtype = TARG_NONE;

		if(u->driver >= 0)
		{
			//LastNum("resetmode 1b");
			//g_unit[u->driver].Disembark();
			u->driver = -1;
		}
	}

	//LastNum("resetmode 2");

	//transportAmt = 0;
	u->targtype = TARG_NONE;
	u->target = u->target2 = -1;
	u->supplier = -1;
	u->mode = UMODE_NONE;
	ResetGoal(u);

	
#if 0
	//URAN_DEBUG
	if(u-g_unit == 19)
	{
		Building* b = &g_building[5];
		char msg[1280];
		sprintf(msg, "/ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n cargty%d", 
			(int)b->transporter[RES_URANIUM], 
			(int)g_unit[b->transporter[RES_URANIUM]].mode,
			(int)g_unit[b->transporter[RES_URANIUM]].target,
			5,
			(int)g_unit[b->transporter[RES_URANIUM]].targtype,
			(int)g_unit[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 3");
}

void ResetTarget(Unit* u)
{
	ResetMode(u);
	u->target = -1;
}
