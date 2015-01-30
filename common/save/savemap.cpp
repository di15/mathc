#include "../render/heightmap.h"
#include "../texture.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../window.h"
#include "../math/camera.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../math/hmapmath.h"
#include "../render/foliage.h"
#include "../sim/unit.h"
#include "../render/foliage.h"
#include "savemap.h"
#include "../sim/building.h"
#include "../sim/road.h"
#include "../sim/powl.h"
#include "../sim/crpipe.h"
#include "../sim/deposit.h"
#include "../render/water.h"
#include "../sim/player.h"
#include "../sim/selection.h"
#include "../path/collidertile.h"
#include "../path/pathjob.h"
#include "../path/pathnode.h"
#include "../debug.h"
#include "../sim/simflow.h"
#include "../sim/transport.h"
#include "../render/particle.h"
#include "../render/transaction.h"
#include "../render/graph.h"
#include "../path/fillbodies.h"
#include "../path/tilepath.h"

float ConvertHeight(unsigned char brightness)
{
#if 0
	// Apply exponential scale to height data.
	float y = (float)brightness*TILE_Y_SCALE/255.0f - TILE_Y_SCALE/2.0f;
	y = y / fabs(y) * pow(fabs(y), TILE_Y_POWER) * TILE_Y_AFTERPOW;

	if(y <= WATER_LEVEL)
		y -= TILE_SIZE;
	else if(y > WATER_LEVEL && y < WATER_LEVEL + TILE_SIZE/100)
		y += TILE_SIZE/100;

	return y;
#else
	return brightness / 25 * TILE_SIZE / 4;
#endif
}

void PlaceUnits()
{
	UType* ut = &g_utype[UNIT_LABOURER];
	PathJob pj;
	pj.airborne = ut->airborne;
	pj.landborne = ut->landborne;
	pj.seaborne = ut->seaborne;
	pj.roaded = ut->roaded;
	pj.ignoreb = 0;
	pj.ignoreu = 0;
	pj.thisu = 0;
	pj.utype = UNIT_LABOURER;

	for(int li=0; li<6*PLAYERS; li++)
	{
		int ntries = 0;

		for(; ntries < 100; ntries++)
		{
			Vec2i cmpos(rand()%g_hmap.m_widthx*TILE_SIZE, rand()%g_hmap.m_widthy*TILE_SIZE);
			Vec2i tpos = cmpos / TILE_SIZE;
			Vec2i npos = cmpos / PATHNODE_SIZE;

			pj.cmstartx = cmpos.x;
			pj.cmstarty = cmpos.y;

			if(Standable2(&pj, cmpos.x, cmpos.y))
			{
				PlaceUnit(UNIT_LABOURER, cmpos, -1, NULL);
				//g_log<<"placed "<<li<<" at"<<tpos.x<<","<<tpos.y<<std::endl;
				break;
			}
			//else
			//g_log<<"not placed at"<<tpos.x<<","<<tpos.y<<std::endl;
		}
	}
}

void LoadJPGMap(const char* relative)
{
	g_hmap.destroy();
	g_hmap2.destroy();
	g_hmap4.destroy();
	g_hmap8.destroy();

	CheckGLError(__FILE__, __LINE__);

	LoadedTex *pImage = NULL;

	char full[1024];
	FullPath(relative, full);

	pImage = LoadJPG(full);

	if(!pImage)
		return;

	CheckGLError(__FILE__, __LINE__);

	g_hmap.allocate((pImage->sizeX-1), (pImage->sizeY-1));
#if 0
	g_hmap2.allocate((pImage->sizeX-1)/2, (pImage->sizeY-1)/2);
	g_hmap4.allocate((pImage->sizeX-1)/4, (pImage->sizeY-1)/4);
	g_hmap8.allocate((pImage->sizeX-1)/8, (pImage->sizeY-1)/8);
#endif

	CheckGLError(__FILE__, __LINE__);
	for(int x=0; x<pImage->sizeX; x++)
	{
		for(int z=0; z<pImage->sizeY; z++)
		{
			float r = (float)(unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 0 ];
			float g = (float)(unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 1 ];
			float b = (float)(unsigned int)pImage->data[ z*pImage->sizeX*3 + x*3 + 2 ];

			// Apply exponential scale to height data.
			float y = ConvertHeight((r+g+b)/3.0f);

			g_hmap.setheight(x, z, y);

#if 0
			if(x%2 == 0 && z%2 == 0)
				g_hmap2.setheight(x/2, z/2, y);

			if(x%4 == 0 && z%4 == 0)
				g_hmap4.setheight(x/4, z/4, y);

			if(x%8 == 0 && z%8 == 0)
				g_hmap8.setheight(x/8, z/8, y);
#endif
		}
	}

	CheckGLError(__FILE__, __LINE__);
	AllocWater(g_hmap.m_widthx, g_hmap.m_widthy);
	CheckGLError(__FILE__, __LINE__);

	g_hmap.remesh(1);
#if 0
	g_hmap2.remesh(2);
	g_hmap4.remesh(4);
	g_hmap8.remesh(8);
#endif
	CheckGLError(__FILE__, __LINE__);

#if 1
	AllocPathGrid(g_hmap.m_widthx*TILE_SIZE, g_hmap.m_widthy*TILE_SIZE);
	AllocGrid((pImage->sizeX-1), (pImage->sizeY-1));
	FillColliderGrid();
#endif

	CheckGLError(__FILE__, __LINE__);
	FillForest();
	//PlaceUnits();

	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f center = Vec3f( g_hmap.m_widthx * TILE_SIZE/2.0f, g_hmap.getheight(g_hmap.m_widthx/2, g_hmap.m_widthy/2), g_hmap.m_widthy * TILE_SIZE/2.0f );
	Vec3f delta = center - c->m_view;
	c->move(delta);
	Vec3f viewvec = c->m_view - c->m_pos;
	viewvec = Normalize(viewvec) * imax(g_hmap.m_widthx, g_hmap.m_widthy) * TILE_SIZE;
	c->m_pos = c->m_view - viewvec;
	g_zoom = INI_ZOOM;

	if(pImage)
	{
		//free(pImage);								// Free the image structure
		delete pImage;

		g_log<<relative<<"\n\r";
		g_log.flush();
	}

	c->position(
	        -1000.0f/3, 1000.0f/3 + 5000, -1000.0f/3,
	        0, 5000, 0,
	        0, 1, 0);

	c->position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	c->move( Vec3f(g_hmap.m_widthx*TILE_SIZE/2, 1000, g_hmap.m_widthy*TILE_SIZE/2) );

	//CalcMapView();
}

void FreeMap()
{
	FreeBls();	//bl's must be freed before units, because of Building::destroy because of how it clears ->worker and ->occupier lists
	FreeUnits();
	FreeFol();
	FreeDeposits();
	g_hmap.destroy();
	FreeGrid();
	Player* py = &g_player[g_localP];
	g_sel.clear();
	FreeParts();
	FreeTransx();
	FreeGraphs();
}

void SaveHmap(FILE *fp)
{
	fwrite(&g_hmap.m_widthx, sizeof(int), 1, fp);
	fwrite(&g_hmap.m_widthy, sizeof(int), 1, fp);

	fwrite(g_hmap.m_heightpoints, sizeof(float), (g_hmap.m_widthx+1)*(g_hmap.m_widthy+1), fp);
	//fwrite(g_hmap.m_countryowner, sizeof(int), g_hmap.m_widthx*g_hmap.m_widthy, fp);
}

void ReadHmap(FILE *fp)
{
	int widthx=0, widthy=0;

	fread(&widthx, sizeof(int), 1, fp);
	fread(&widthy, sizeof(int), 1, fp);

	g_hmap.allocate(widthx, widthy);
#if 0
	g_hmap2.allocate(widthx/2, widthy/2);
	g_hmap4.allocate(widthx/4, widthy/4);
	g_hmap8.allocate(widthx/8, widthy/8);
#endif

	fread(g_hmap.m_heightpoints, sizeof(float), (g_hmap.m_widthx+1)*(g_hmap.m_widthy+1), fp);
	//fread(g_hmap.m_countryowner, sizeof(int), g_hmap.m_widthx*g_hmap.m_widthy, fp);

#if 0
	for(int x=0; x<widthx+1; x++)
		for(int z=0; z<widthy+1; z++)
		{
			if(x%2 == 0 && z%2 == 0)
				g_hmap2.setheight(x/2, z/2, g_hmap.getheight(x, z));
			if(x%4 == 0 && z%4 == 0)
				g_hmap4.setheight(x/4, z/4, g_hmap.getheight(x, z));
			if(x%8 == 0 && z%8 == 0)
				g_hmap8.setheight(x/8, z/8, g_hmap.getheight(x, z));
		}
#endif

	g_hmap.remesh(1);
#if 0
	g_hmap2.remesh(2);
	g_hmap4.remesh(4);
	g_hmap8.remesh(8);
#endif

	AllocGrid(widthx, widthy);

	AllocPathGrid(widthx*TILE_SIZE, widthy*TILE_SIZE);
	//CalcMapView();
}

void SaveFol(FILE *fp)
{
	for(int i=0; i<FOLIAGES; i++)
	{
#if 0
		bool on;
		unsigned char type;
		Vec3f pos;
		float yaw;
#endif

		Foliage *f = &g_foliage[i];

		fwrite(&f->on, sizeof(bool), 1, fp);

		if(!f->on)
			continue;

		fwrite(&f->type, sizeof(unsigned char), 1, fp);
		fwrite(&f->cmpos, sizeof(Vec2i), 1, fp);
		fwrite(&f->drawpos, sizeof(Vec3f), 1, fp);
		fwrite(&f->yaw, sizeof(float), 1, fp);
	}
}

void ReadFol(FILE *fp)
{
	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage *f = &g_foliage[i];

		fread(&f->on, sizeof(bool), 1, fp);

		if(!f->on)
		{
			f->reinstance();
			continue;
		}

		fread(&f->type, sizeof(unsigned char), 1, fp);
		fread(&f->cmpos, sizeof(Vec2i), 1, fp);
		fread(&f->drawpos, sizeof(Vec3f), 1, fp);
		fread(&f->yaw, sizeof(float), 1, fp);

		f->reinstance();
	}
}

void SaveDeps(FILE *fp)
{
	for(int i=0; i<DEPOSITS; i++)
	{
#if 0
		bool on;
		bool occupied;
		int restype;
		int amount;
		Vec2i tilepos;
		Vec3f drawpos;
#endif

		Deposit *d = &g_deposit[i];

		fwrite(&d->on, sizeof(bool), 1, fp);

		if(!d->on)
			continue;

		fwrite(&d->occupied, sizeof(bool), 1, fp);
		fwrite(&d->restype, sizeof(int), 1, fp);
		fwrite(&d->amount, sizeof(int), 1, fp);
		fwrite(&d->tilepos, sizeof(Vec2i), 1, fp);
		fwrite(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void ReadDeps(FILE *fp)
{
	for(int i=0; i<DEPOSITS; i++)
	{
		Deposit *d = &g_deposit[i];

		fread(&d->on, sizeof(bool), 1, fp);

		if(!d->on)
			continue;

		fread(&d->occupied, sizeof(bool), 1, fp);
		fread(&d->restype, sizeof(int), 1, fp);
		fread(&d->amount, sizeof(int), 1, fp);
		fread(&d->tilepos, sizeof(Vec2i), 1, fp);
		fread(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void SaveUnits(FILE *fp)
{
	for(int i=0; i<UNITS; i++)
	{
#if 0
		bool on;
		int type;
		int stateowner;
		int corpowner;
		int unitowner;

		/*
		The f (floating-point) position vectory is used for drawing.
		*/
		Vec3f fpos;

		/*
		The real position is stored in integers.
		*/
		Vec2i cmpos;
		Vec3f facing;
		Vec2f rotation;

		std::list<Vec2i> path;
		Vec2i goal;
#endif

		Unit *u = &g_unit[i];

		fwrite(&u->on, sizeof(bool), 1, fp);

		if(!u->on)
			continue;

		fwrite(&u->type, sizeof(int), 1, fp);
#if 0
		fwrite(&u->stateowner, sizeof(int), 1, fp);
		fwrite(&u->corpowner, sizeof(int), 1, fp);
		fwrite(&u->unitowner, sizeof(int), 1, fp);
#else
		fwrite(&u->owner, sizeof(int), 1, fp);
#endif

		fwrite(&u->drawpos, sizeof(Vec3f), 1, fp);
		fwrite(&u->cmpos, sizeof(Vec2i), 1, fp);
		fwrite(&u->rotation, sizeof(Vec2f), 1, fp);

		int pathsz = u->path.size();

		fwrite(&pathsz, sizeof(int), 1, fp);

		for(auto pathiter = u->path.begin(); pathiter != u->path.end(); pathiter++)
			fwrite(&*pathiter, sizeof(Vec2i), 1, fp);

		fwrite(&u->goal, sizeof(Vec2i), 1, fp);

#if 0
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

		Vec2i subgoal;
#endif

		fwrite(&u->target, sizeof(int), 1, fp);
		fwrite(&u->target2, sizeof(int), 1, fp);
		fwrite(&u->targetu, sizeof(bool), 1, fp);
		fwrite(&u->underorder, sizeof(bool), 1, fp);
		fwrite(&u->fuelstation, sizeof(int), 1, fp);
		fwrite(u->belongings, sizeof(int), RESOURCES, fp);
		fwrite(&u->hp, sizeof(int), 1, fp);
		fwrite(&u->passive, sizeof(bool), 1, fp);
		fwrite(&u->prevpos, sizeof(Vec2i), 1, fp);
		fwrite(&u->taskframe, sizeof(int), 1, fp);
		fwrite(&u->pathblocked, sizeof(bool), 1, fp);
		fwrite(&u->jobframes, sizeof(int), 1, fp);
		fwrite(&u->supplier, sizeof(int), 1, fp);
		fwrite(&u->reqamt, sizeof(int), 1, fp);
		fwrite(&u->targtype, sizeof(int), 1, fp);
		fwrite(&u->home, sizeof(int), 1, fp);
		fwrite(&u->car, sizeof(int), 1, fp);
		fwrite(&u->frame, sizeof(float), 2, fp);
		fwrite(&u->subgoal, sizeof(Vec2i), 1, fp);

#if 0
	unsigned char mode;
	int pathdelay;
	unsigned long long lastpath;

	bool threadwait;

	// used for debugging - don't save to file
	bool collided;

	unsigned char cdtype;	//conduit type for mode (going to construction)
	int driver;
	//short framesleft;
	short cyframes;	//cycle frames (unit cycle of consumption and work)
	int opwage;	//transport driver wage
	//std::list<TransportJob> bids;	//for trucks
#endif

		fwrite(&u->mode, sizeof(unsigned char), 1, fp);
		fwrite(&u->pathdelay, sizeof(int), 1, fp);
		fwrite(&u->lastpath, sizeof(unsigned long long), 1, fp);
		fwrite(&u->threadwait, sizeof(bool), 1, fp);
		fwrite(&u->collided, sizeof(bool), 1, fp);
		fwrite(&u->cdtype, sizeof(signed char), 1, fp);
		fwrite(&u->driver, sizeof(int), 1, fp);
		fwrite(&u->cyframes, sizeof(short), 1, fp);
		fwrite(&u->opwage, sizeof(int), 1, fp);

#if 0
	int cargoamt;
	int cargotype;
	int cargoreq;	//req amt
#endif
	
		fwrite(&u->cargoamt, sizeof(int), 1, fp);
		fwrite(&u->cargotype, sizeof(int), 1, fp);
		fwrite(&u->cargoreq, sizeof(int), 1, fp);

		int ntpath = u->tpath.size();
		fwrite(&ntpath, sizeof(int), 1, fp);
		//u->tpath.clear();
		for(auto ti=u->tpath.begin(); ti!=u->tpath.end(); ti++)
			fwrite(&*ti, sizeof(Vec2s), 1, fp);
	}
}

void ReadUnits(FILE *fp)
{
	for(int i=0; i<UNITS; i++)
	{
		Unit *u = &g_unit[i];

		fread(&u->on, sizeof(bool), 1, fp);

		if(!u->on)
			continue;

		fread(&u->type, sizeof(int), 1, fp);
		
#ifdef TRANSPORT_DEBUG
		if(u->type == UNIT_LABOURER || i==10 || i == 11)
		//if(i!=0)
			u->on = false;
#endif

#if 0
		fread(&u->stateowner, sizeof(int), 1, fp);
		fread(&u->corpowner, sizeof(int), 1, fp);
		fread(&u->unitowner, sizeof(int), 1, fp);
#else
		fread(&u->owner, sizeof(int), 1, fp);
#endif

		fread(&u->drawpos, sizeof(Vec3f), 1, fp);
		fread(&u->cmpos, sizeof(Vec2i), 1, fp);
		fread(&u->rotation, sizeof(Vec2f), 1, fp);

#ifdef RANDOM8DEBUG
		//u->drawpos = Vec3f(u->cmpos.x, g_hmap.accheight(u->cmpos.x, u->cmpos.y), u->cmpos.y);
#endif

		int pathsz = 0;

		fread(&pathsz, sizeof(int), 1, fp);
		u->path.clear();

		for(int pathindex=0; pathindex<pathsz; pathindex++)
		{
			Vec2i waypoint;
			fread(&waypoint, sizeof(Vec2i), 1, fp);
			u->path.push_back(waypoint);
		}

		fread(&u->goal, sizeof(Vec2i), 1, fp);

		fread(&u->target, sizeof(int), 1, fp);
		fread(&u->target2, sizeof(int), 1, fp);
		fread(&u->targetu, sizeof(bool), 1, fp);
		fread(&u->underorder, sizeof(bool), 1, fp);
		fread(&u->fuelstation, sizeof(int), 1, fp);
		fread(u->belongings, sizeof(int), RESOURCES, fp);
		fread(&u->hp, sizeof(int), 1, fp);
		fread(&u->passive, sizeof(bool), 1, fp);
		fread(&u->prevpos, sizeof(Vec2i), 1, fp);
		fread(&u->taskframe, sizeof(int), 1, fp);
		fread(&u->pathblocked, sizeof(bool), 1, fp);
		fread(&u->jobframes, sizeof(int), 1, fp);
		fread(&u->supplier, sizeof(int), 1, fp);
		fread(&u->reqamt, sizeof(int), 1, fp);
		fread(&u->targtype, sizeof(int), 1, fp);
		fread(&u->home, sizeof(int), 1, fp);
		fread(&u->car, sizeof(int), 1, fp);
		fread(&u->frame, sizeof(float), 2, fp);
		fread(&u->subgoal, sizeof(Vec2i), 1, fp);
		
		fread(&u->mode, sizeof(unsigned char), 1, fp);
		fread(&u->pathdelay, sizeof(int), 1, fp);
		fread(&u->lastpath, sizeof(unsigned long long), 1, fp);
		fread(&u->threadwait, sizeof(bool), 1, fp);
		fread(&u->collided, sizeof(bool), 1, fp);
		fread(&u->cdtype, sizeof(signed char), 1, fp);
		fread(&u->driver, sizeof(int), 1, fp);
		fread(&u->cyframes, sizeof(short), 1, fp);
		fread(&u->opwage, sizeof(int), 1, fp);
		
		fread(&u->cargoamt, sizeof(int), 1, fp);
		fread(&u->cargotype, sizeof(int), 1, fp);
		fread(&u->cargoreq, sizeof(int), 1, fp);

#if 1
		int ntpath = 0;
		fread(&ntpath, sizeof(int), 1, fp);
		u->tpath.clear();
		for(int ti=0; ti<ntpath; ti++)
		{
			Vec2s tpos;
			fread(&tpos, sizeof(Vec2s), 1, fp);
			u->tpath.push_back(tpos);
		}
#endif

		//u->fillcollider();
	}
}

void SaveCapSup(CapSup* cs, FILE* fp)
{
#if 0
	unsigned char rtype;
	int amt;
	int src;
	int dst;
#endif

	fwrite(&cs->rtype, sizeof(unsigned char), 1, fp);
	fwrite(&cs->amt, sizeof(int), 1, fp);
	fwrite(&cs->src, sizeof(int), 1, fp);
	fwrite(&cs->dst, sizeof(int), 1, fp);
}

void ReadCapSup(CapSup* cs, FILE* fp)
{
	fread(&cs->rtype, sizeof(unsigned char), 1, fp);
	fread(&cs->amt, sizeof(int), 1, fp);
	fread(&cs->src, sizeof(int), 1, fp);
	fread(&cs->dst, sizeof(int), 1, fp);
}

void SaveManufJob(ManufJob* mj, FILE* fp)
{
#if 0
	int utype;
	int owner;
#endif

	fwrite(&mj->utype, sizeof(int), 1, fp);
	fwrite(&mj->owner, sizeof(int), 1, fp);
}

void ReadManufJob(ManufJob* mj, FILE* fp)
{
	fread(&mj->utype, sizeof(int), 1, fp);
	fread(&mj->owner, sizeof(int), 1, fp);
}

void SaveBls(FILE *fp)
{
	for(int i=0; i<BUILDINGS; i++)
	{
#if 0
	bool on;
	int type;
	int owner;

	Vec2i tilepos;	//position in tiles
	Vec3f drawpos;	//drawing position in centimeters

	bool finished;

	short pownetw;
	short crpipenetw;
	std::list<short> roadnetw;
#endif

		Building *b = &g_building[i];

		fwrite(&b->on, sizeof(bool), 1, fp);

		if(!b->on)
			continue;

		fwrite(&b->type, sizeof(int), 1, fp);
#if 0
		fwrite(&b->stateowner, sizeof(int), 1, fp);
		fwrite(&b->corpowner, sizeof(int), 1, fp);
		fwrite(&b->unitowner, sizeof(int), 1, fp);
#else
		fwrite(&b->owner, sizeof(int), 1, fp);
#endif

		fwrite(&b->tilepos, sizeof(Vec2i), 1, fp);
		fwrite(&b->drawpos, sizeof(Vec3f), 1, fp);

		fwrite(&b->finished, sizeof(bool), 1, fp);

		fwrite(&b->pownetw, sizeof(short), 1, fp);
		fwrite(&b->crpipenetw, sizeof(short), 1, fp);
		int nroadnetw = b->roadnetw.size();
		fwrite(&nroadnetw, sizeof(short), 1, fp);
		for(auto rnetit = b->roadnetw.begin(); rnetit != b->roadnetw.end(); rnetit++)
			fwrite(&*rnetit, sizeof(short), 1, fp);

#if 0
	int stocked[RESOURCES];
	int inuse[RESOURCES];

	EmitterCounter emitterco[MAX_B_EMITTERS];

	VertexArray drawva;

	int conmat[RESOURCES];
	bool inoperation;

	int price[RESOURCES];	//price of produced goods
	int propprice;	//price of this property

	std::list<int> occupier;
	std::list<int> worker;
	int conwage;
	int opwage;
	int cydelay;	//the frame delay between production cycles, when production target is renewed
	short prodlevel;	//production target level of max RATIO_DENOM
	short cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	unsigned int lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	int manufprc[UNIT_TYPES];
	std::list<ManufJob> manufjob;
	short transporter[RESOURCES];
#endif

		fwrite(b->stocked, sizeof(int), RESOURCES, fp);
		fwrite(b->inuse, sizeof(int), RESOURCES, fp);

		fwrite(b->conmat, sizeof(int), RESOURCES, fp);
		fwrite(&b->inoperation, sizeof(bool), 1, fp);
		fwrite(b->price, sizeof(int), RESOURCES, fp);
		fwrite(&b->propprice, sizeof(int), 1, fp);

		unsigned char noc = b->occupier.size();
		fwrite(&noc, sizeof(unsigned char), 1, fp);
		for(auto ocit=b->occupier.begin(); ocit!=b->occupier.end(); ocit++)
			fwrite(&*ocit, sizeof(int), 1, fp);
		
		unsigned char nwk = b->worker.size();
		fwrite(&nwk, sizeof(unsigned char), 1, fp);
		for(auto wkit=b->worker.begin(); wkit!=b->worker.end(); wkit++)
			fwrite(&*wkit, sizeof(int), 1, fp);

		fwrite(&b->conwage, sizeof(int), 1, fp);
		fwrite(&b->opwage, sizeof(int), 1, fp);
		fwrite(&b->cydelay, sizeof(int), 1, fp);
		fwrite(&b->prodlevel, sizeof(short), 1, fp);
		fwrite(&b->cymet, sizeof(short), 1, fp);
		fwrite(&b->lastcy, sizeof(unsigned int), 1, fp);

		int ncs = b->capsup.size();
		fwrite(&ncs, sizeof(int), 1, fp);
		for(auto ncit=b->capsup.begin(); ncit!=b->capsup.end(); ncit++)
			SaveCapSup(&*ncit, fp);

		fwrite(b->manufprc, sizeof(int), UNIT_TYPES, fp);

		int nmj = b->manufjob.size();
		fwrite(&nmj, sizeof(int), 1, fp);
		for(auto mjit=b->manufjob.begin(); mjit!=b->manufjob.end(); mjit++)
			SaveManufJob(&*mjit, fp);

		fwrite(b->transporter, sizeof(short), RESOURCES, fp);

#if 0
		int hp;
#endif

		fwrite(&b->hp, sizeof(int), 1, fp);
	}
}

void ReadBls(FILE *fp)
{

	for(int i=0; i<BUILDINGS; i++)
	{
		
		//g_log<<"\t read bl"<<i<<std::endl;
		//g_log.flush();

		Building *b = &g_building[i];

		fread(&b->on, sizeof(bool), 1, fp);

		if(!b->on)
			continue;
		
		//g_log<<"\t\t on"<<i<<std::endl;
		//g_log.flush();

		fread(&b->type, sizeof(int), 1, fp);
#if 0
		fread(&b->stateowner, sizeof(int), 1, fp);
		fread(&b->corpowner, sizeof(int), 1, fp);
		fread(&b->unitowner, sizeof(int), 1, fp);
#else
		fread(&b->owner, sizeof(int), 1, fp);
#endif

		fread(&b->tilepos, sizeof(Vec2i), 1, fp);
		fread(&b->drawpos, sizeof(Vec3f), 1, fp);
		
		BlType* t = &g_bltype[b->type];
		Vec2i tmin;
		Vec2i tmax;
		tmin.x = b->tilepos.x - t->widthx/2;
		tmin.y = b->tilepos.y - t->widthy/2;
		tmax.x = tmin.x + t->widthx;
		tmax.y = tmin.y + t->widthy;
		b->drawpos = Vec3f(b->tilepos.x*TILE_SIZE, Lowest(tmin.x, tmin.y, tmax.x, tmax.y), b->tilepos.y*TILE_SIZE);
		if(t->foundation == FOUNDATION_SEA)
			b->drawpos.y = WATER_LEVEL;
		if(t->widthx % 2 == 1)
			b->drawpos.x += TILE_SIZE/2;
		if(t->widthy % 2 == 1)
			b->drawpos.z += TILE_SIZE/2;

		fread(&b->finished, sizeof(bool), 1, fp);

		//g_log<<"\t\t netws..."<<i<<std::endl;
		//g_log.flush();

		fread(&b->pownetw, sizeof(short), 1, fp);
		fread(&b->crpipenetw, sizeof(short), 1, fp);
		short nroadnetw = -1;
		fread(&nroadnetw, sizeof(short), 1, fp);
		for(int rni=0; rni<nroadnetw; rni++)
		{
			short roadnetw = -1;
			fread(&roadnetw, sizeof(short), 1, fp);
			b->roadnetw.push_back(roadnetw);
		}

		fread(b->stocked, sizeof(int), RESOURCES, fp);
		fread(b->inuse, sizeof(int), RESOURCES, fp);

		fread(b->conmat, sizeof(int), RESOURCES, fp);
		fread(&b->inoperation, sizeof(bool), 1, fp);
		fread(b->price, sizeof(int), RESOURCES, fp);
		fread(&b->propprice, sizeof(int), 1, fp);
		
		//g_log<<"\t\t ocs..."<<i<<std::endl;
		//g_log.flush();

		unsigned char noc = b->occupier.size();
		fread(&noc, sizeof(unsigned char), 1, fp);
		for(int oci=0; oci<noc; oci++)
		{
			int oc;
			fread(&oc, sizeof(int), 1, fp);
			b->occupier.push_back(oc);
		}
		
		unsigned char nwk = b->worker.size();
		fread(&nwk, sizeof(unsigned char), 1, fp);
		for(int wki=0; wki<nwk; wki++)
		{
			int wk;
			fread(&wk, sizeof(int), 1, fp);
			b->worker.push_back(wk);
		}

		fread(&b->conwage, sizeof(int), 1, fp);
		fread(&b->opwage, sizeof(int), 1, fp);
		fread(&b->cydelay, sizeof(int), 1, fp);
		fread(&b->prodlevel, sizeof(short), 1, fp);
		fread(&b->cymet, sizeof(short), 1, fp);
		fread(&b->lastcy, sizeof(unsigned int), 1, fp);

		int ncs = 0;
		fread(&ncs, sizeof(int), 1, fp);
		for(int ci=0; ci<ncs; ci++)
		{
			CapSup cs;
			ReadCapSup(&cs, fp);
			b->capsup.push_back(cs);
		}

		fread(b->manufprc, sizeof(int), UNIT_TYPES, fp);

		int nmj = 0;
		fread(&nmj, sizeof(int), 1, fp);
		for(int mji=0; mji<nmj; mji++)
		{
			ManufJob mj;
			ReadManufJob(&mj, fp);
			b->manufjob.push_back(mj);
		}

		fread(b->transporter, sizeof(short), RESOURCES, fp);

		fread(&b->hp, sizeof(int), 1, fp);

		//b->fillcollider();
		b->remesh();
	}
}

void SaveView(FILE *fp)
{
	Player* py = &g_player[g_localP];
	fwrite(&g_cam, sizeof(Camera), 1, fp);
	fwrite(&g_zoom, sizeof(float), 1, fp);
}

void ReadView(FILE *fp)
{
	Player* py = &g_player[g_localP];
	fread(&g_cam, sizeof(Camera), 1, fp);
	fread(&g_zoom, sizeof(float), 1, fp);
}

void SaveCo(FILE* fp)
{
	for(unsigned char ctype=0; ctype<CONDUIT_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthy; i++)
		{
#if 0
	bool on;
	unsigned char conntype;
	bool finished;
	unsigned char owner;
	int conmat[RESOURCES];
	short netw;	//network
	VertexArray drawva;
	//bool inaccessible;
	short transporter[RESOURCES];
	Vec3f drawpos;
	//int maxcost[RESOURCES];
	int conwage;
#endif

			CdTile* ctile = &ct->cdtiles[(int)false][i];

			fwrite(&ctile->on, sizeof(bool), 1, fp);

			if(!ctile->on)
				continue;

			fwrite(&ctile->conntype, sizeof(char), 1, fp);
			fwrite(&ctile->finished, sizeof(bool), 1, fp);
			fwrite(&ctile->owner, sizeof(char), 1, fp);
			fwrite(ctile->conmat, sizeof(int), RESOURCES, fp);
			fwrite(&ctile->netw, sizeof(short), 1, fp);
			fwrite(ctile->transporter, sizeof(short), RESOURCES, fp);
			fwrite(&ctile->drawpos, sizeof(Vec3f), 1, fp);
			fwrite(&ctile->conwage, sizeof(int), 1, fp);
		}
	}
}

void ReadCo(FILE* fp)
{
	for(unsigned char ctype=0; ctype<CONDUIT_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int i=0; i<g_hmap.m_widthx*g_hmap.m_widthy; i++)
		{
			CdTile* ctile = &ct->cdtiles[(int)false][i];

			fread(&ctile->on, sizeof(bool), 1, fp);

			if(!ctile->on)
				continue;

			fread(&ctile->conntype, sizeof(char), 1, fp);
			fread(&ctile->finished, sizeof(bool), 1, fp);
			fread(&ctile->owner, sizeof(char), 1, fp);
			fread(ctile->conmat, sizeof(int), RESOURCES, fp);
			fread(&ctile->netw, sizeof(short), 1, fp);
			fread(ctile->transporter, sizeof(short), RESOURCES, fp);
			fread(&ctile->drawpos, sizeof(Vec3f), 1, fp);
			fread(&ctile->conwage, sizeof(int), 1, fp);
		}

		for(int x=0; x<g_hmap.m_widthx; x++)
			for(int y=0; y<g_hmap.m_widthy; y++)
			{
				if(!GetCd(ctype, x, y, false)->on)
					continue;

				if(!ct->cornerpl)
					g_hmap.hidetile(x, y);

				RemeshCd(ctype, x, y, false);
			}
	}

	g_hmap.genvbo();	//call once
}

void ReadPys(FILE* fp)
{
	for(int i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];

#if 0
	bool on;
	bool ai;

	int local[RESOURCES];	// used just for counting; cannot be used
	int global[RESOURCES];
	int resch[RESOURCES];	//resource changes/deltas
	int truckwage;	//truck driver wage per second
	int transpcost;	//transport cost per second

#endif

		fread(&py->on, sizeof(bool), 1, fp);

		if(!py->on)
			continue;

		fread(&py->ai, sizeof(bool), 1, fp);
		fread(py->local, sizeof(int), RESOURCES, fp);
		fread(py->global, sizeof(int), RESOURCES, fp);
		fread(py->resch, sizeof(int), RESOURCES, fp);
		fread(&py->truckwage, sizeof(int), 1, fp);
		fread(&py->transpcost, sizeof(int), 1, fp);
		
#if 0
	float color[4];
	RichText name;
#endif

		fread(py->color, sizeof(float), 4, fp);

		int slen = 0;
		fread(&slen, sizeof(int), 1, fp);
		char* name = new char[slen];
		fread(name, sizeof(char), slen, fp);
		py->name = RichText(name);
		delete [] name;
	}
}

void SavePys(FILE* fp)
{
	for(int i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];

		fwrite(&py->on, sizeof(bool), 1, fp);

		if(!py->on)
			continue;

		fwrite(&py->ai, sizeof(bool), 1, fp);
		fwrite(py->local, sizeof(int), RESOURCES, fp);
		fwrite(py->global, sizeof(int), RESOURCES, fp);
		fwrite(py->resch, sizeof(int), RESOURCES, fp);
		fwrite(&py->truckwage, sizeof(int), 1, fp);
		fwrite(&py->transpcost, sizeof(int), 1, fp);
		fwrite(py->color, sizeof(float), 4, fp);

		std::string name = py->name.rawstr();
		int slen = name.length() + 1;
		fwrite(&slen, sizeof(int), 1, fp);
		fwrite(name.c_str(), sizeof(char), slen, fp);
	}
}

void ReadGr(FILE* fp)
{
	for(int i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];

#if 0
		std::list<float> points;
		unsigned int startframe;
		unsigned int cycles;
#endif
		
		fread(&g->startframe, sizeof(unsigned int), 1, fp);
		fread(&g->cycles, sizeof(unsigned int), 1, fp);

		unsigned int np = 0;
		fread(&np, sizeof(unsigned int), 1, fp);

		for(int pi=0; pi<np; pi++)
		{
			float p;
			fread(&p, sizeof(float), 1, fp);
			g->points.push_back(p);
		}
	}
}

void SaveGr(FILE* fp)
{
	for(int i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];

		fwrite(&g->startframe, sizeof(unsigned int), 1, fp);
		fwrite(&g->cycles, sizeof(unsigned int), 1, fp);

		unsigned int np = g->points.size();
		fwrite(&np, sizeof(unsigned int), 1, fp);

		for(auto pit=g->points.begin(); pit!=g->points.end(); pit++)
		{
			fwrite(&*pit, sizeof(float), 1, fp);
		}
	}
}

void ReadJams(FILE* fp)
{
	//return;

	for(short x=0; x<g_hmap.m_widthx; x++)
		for(short y=0; y<g_hmap.m_widthy; y++)
		{
			int tin = x + y * g_hmap.m_widthx;
			TileNode* tn = &g_tilenode[tin];
			fread(&tn->jams, sizeof(unsigned char), 1, fp);
		}
}

void SaveJams(FILE* fp)
{
	for(short x=0; x<g_hmap.m_widthx; x++)
		for(short y=0; y<g_hmap.m_widthy; y++)
		{
			int tin = x + y * g_hmap.m_widthx;
			TileNode* tn = &g_tilenode[tin];
			fwrite(&tn->jams, sizeof(unsigned char), 1, fp);
		}
}

bool SaveMap(const char* name)
{
	char fullpath[MAX_PATH+1];
	FullPath(name, fullpath);

	FILE *fp = NULL;

	fp = fopen(fullpath, "wb");

	if(!fp)
		return false;

	char tag[] = MAP_TAG;
	int version = MAP_VERSION;

	fwrite(&tag, sizeof(tag), 1, fp);
	fwrite(&version, sizeof(version), 1, fp);

	fwrite(&g_simframe, sizeof(g_simframe), 1, fp);

	SaveView(fp);
	SavePys(fp);
	SaveHmap(fp);
	SaveDeps(fp);
	SaveFol(fp);
	SaveBls(fp);
	SaveUnits(fp);
	SaveCo(fp);
	SaveGr(fp);
	SaveJams(fp);

	fclose(fp);

	return true;
}

//consistency check
void ConsistCh()
{
	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Building* b = &g_building[bi];

		if(!b->on)
			continue;

		for(int ri=0; ri<RESOURCES; ri++)
		{
			int ui = b->transporter[ri];

			if(ui < 0)
				continue;

			Unit* u = &g_unit[ui];

			bool fail = false;
			unsigned int flag = 0;

			if(!u->on)
			{
				flag |= 1;
				fail = true;
			}

			if(u->type != UNIT_TRUCK)
			{
				flag |= 2;
				fail = true;
			}

			if(u->mode != UMODE_ATDEMB &&
				u->mode != UMODE_GODEMB &&
				u->mode != UMODE_GOSUP && 
				u->mode != UMODE_ATSUP)
			{
				flag |= 4;
				fail = true;
			}

			if(u->target != bi)
			{
				flag |= 8;
				fail = true;
			}

			if(!fail)
				continue;

			BlType* bt = &g_bltype[b->type];

			char msg[1280];
			sprintf(msg, "Consistency check failed #%u:\ntransporter to %s umode=%d ucargotype=%d ucargoamt=%d", flag, bt->name, (int)u->mode, u->cargotype, u->cargoamt);
			InfoMess("csfail", msg);
		}
	}
}

bool LoadMap(const char* name)
{
	FreeMap();
	char fullpath[MAX_PATH+1];
	FullPath(name, fullpath);

	FILE *fp = NULL;

	fp = fopen(fullpath, "rb");

	if(!fp)
		return false;

	char realtag[] = MAP_TAG;
	int version = MAP_VERSION;
	char tag[ sizeof(realtag) ];

	fread(&tag, sizeof(tag), 1, fp);

	if(memcmp(tag, realtag, sizeof(tag)) != 0)
	{
		ErrMess("Error", "Incorrect header tag in map file.");
		fclose(fp);
		return false;
	}

	fread(&version, sizeof(version), 1, fp);

	if(version != MAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		sprintf(msg, "Map file version (%i) doesn't match %i.", version, MAP_VERSION);
		ErrMess("Error", msg);
		return false;
	}

	fread(&g_simframe, sizeof(g_simframe), 1, fp);

	g_log<<"read view"<<std::endl;
	g_log.flush();
	ReadView(fp);
	g_log<<"read pys"<<std::endl;
	g_log.flush();
	ReadPys(fp);
	g_log<<"read hmap"<<std::endl;
	g_log.flush();
	ReadHmap(fp);
	g_log<<"read deps"<<std::endl;
	g_log.flush();
	ReadDeps(fp);
	g_log<<"read fol"<<std::endl;
	g_log.flush();
	ReadFol(fp);
	g_log<<"read bls"<<std::endl;
	g_log.flush();
	ReadBls(fp);
	g_log<<"read units"<<std::endl;
	g_log.flush();
	ReadUnits(fp);
	g_log<<"read co"<<std::endl;
	g_log.flush();
	ReadCo(fp);
	g_log<<"fill col grd"<<std::endl;
	g_log.flush();

	ReadGr(fp);
	ReadJams(fp);

	FillColliderGrid();
	g_log<<"loaded m"<<std::endl;
	g_log.flush();

	fclose(fp);

	ConsistCh();

	return true;
}
