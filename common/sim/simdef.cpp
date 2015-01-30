#include "simdef.h"
#include "../script/console.h"
#include "conduit.h"
#include "road.h"
#include "building.h"
#include "truck.h"
#include "labourer.h"
#include "../sound/sound.h"

// Queue all the game resources and define objects
void Queue()
{
	// Cursor types

#if 0
#define CU_NONE		0	//cursor off?
#define CU_DEFAULT	1
#define CU_MOVE		2	//move window
#define CU_RESZL	3	//resize width (horizontal) from left side
#define CU_RESZR	4	//resize width (horizontal) from right side
#define CU_RESZT	5	//resize height (vertical) from top side
#define CU_RESZB	6	//resize height (vertical) from bottom side
#define CU_RESZTL	7	//resize top left corner
#define CU_RESZTR	8	//resize top right corner
#define CU_RESZBL	9	//resize bottom left corner
#define CU_RESZBR	10	//resize bottom right corner
#define CU_WAIT		11	//shows a hourglass?
#define CU_DRAG		12	//drag some object between widgets?
#define CU_STATES	13
#endif

	DefS("gui/transp.png", &g_cursor[CU_NONE], 0, 0);
	DefS("gui/cursors/default.png", &g_cursor[CU_DEFAULT], 0, 0);
	DefS("gui/cursors/move.png", &g_cursor[CU_MOVE], 16, 16);
	DefS("gui/cursors/reszh.png", &g_cursor[CU_RESZL], 16, 16);
	DefS("gui/cursors/reszh.png", &g_cursor[CU_RESZR], 16, 16);
	DefS("gui/cursors/reszv.png", &g_cursor[CU_RESZT], 16, 16);
	DefS("gui/cursors/reszv.png", &g_cursor[CU_RESZB], 16, 16);
	DefS("gui/cursors/reszd2.png", &g_cursor[CU_RESZTL], 16, 16);
	DefS("gui/cursors/reszd1.png", &g_cursor[CU_RESZTR], 16, 16);
	DefS("gui/cursors/reszd1.png", &g_cursor[CU_RESZBL], 16, 16);
	DefS("gui/cursors/reszd2.png", &g_cursor[CU_RESZBR], 16, 16);
	DefS("gui/cursors/default.png", &g_cursor[CU_WAIT], 16, 16);
	DefS("gui/cursors/default.png", &g_cursor[CU_DRAG], 16, 16);


	// Icons

#if 0
#define ICON_DOLLARS		0
#define ICON_PESOS			1
#define ICON_EUROS			2
#define ICON_POUNDS			3
#define ICON_FRANCS			4
#define ICON_YENS			5
#define ICON_RUPEES			6
#define ICON_ROUBLES		7
#define ICON_LABOUR			8
#define ICON_HOUSING		9
#define ICON_FARMPRODUCT	10
#define ICON_FOOD			11
#define ICON_CHEMICALS		12
#define ICON_ELECTRONICS	13
#define ICON_RESEARCH		14
#define ICON_PRODUCTION		15
#define ICON_IRONORE		16
#define ICON_URANIUMORE		17
#define ICON_STEEL			18
#define ICON_CRUDEOIL		19
#define ICON_WSFUEL			20
#define ICON_STONE			21
#define ICON_CEMENT			22
#define ICON_ENERGY			23
#define ICON_ENRICHEDURAN	24
#define ICON_COAL			25
#define ICON_TIME			26
#define ICON_RETFUEL		27
#define ICON_LOGS			28
#define ICON_LUMBER			29
#define ICON_WATER			30
#define ICONS				31
#endif

	DefI(ICON_DOLLARS, "gui/icons/dollars.png", "\\$");
	DefI(ICON_PESOS, "gui/icons/pesos.png", "\\peso");
	DefI(ICON_EUROS, "gui/icons/euros.png", "\\euro");
	DefI(ICON_POUNDS, "gui/icons/pounds.png", "\\pound");
	DefI(ICON_FRANCS, "gui/icons/francs.png", "\\franc");
	DefI(ICON_YENS, "gui/icons/yens.png", "\\yen");
	DefI(ICON_RUPEES, "gui/icons/rupees.png", "\\rupee");
	DefI(ICON_ROUBLES, "gui/icons/roubles.png", "\\ruble");
	DefI(ICON_LABOUR, "gui/icons/labour.png", "\\labour");
	DefI(ICON_HOUSING, "gui/icons/housing.png", "\\housing");
	DefI(ICON_FARMPRODUCT, "gui/icons/farmproducts.png", "\\farmprod");
	DefI(ICON_WSFOOD, "gui/icons/wsfood.png", "\\wsfood");
	DefI(ICON_RETFOOD, "gui/icons/retfood.png", "\\retfood");
	DefI(ICON_CHEMICALS, "gui/icons/chemicals.png", "\\chemicals");
	DefI(ICON_ELECTRONICS, "gui/icons/electronics.png", "\\electronics");
	DefI(ICON_RESEARCH, "gui/icons/research.png", "\\research");
	DefI(ICON_PRODUCTION, "gui/icons/production.png", "\\production");
	DefI(ICON_IRONORE, "gui/icons/ironore.png", "\\ironore");
	DefI(ICON_URANIUMORE, "gui/icons/uraniumore.png", "\\uraniumore");
	DefI(ICON_STEEL, "gui/icons/steel.png", "\\steel");
	DefI(ICON_CRUDEOIL, "gui/icons/crudeoil.png", "\\crudeoil");
	DefI(ICON_WSFUEL, "gui/icons/fuelwholesale.png", "\\wsfuel");
	DefI(ICON_STONE, "gui/icons/stone.png", "\\stone");
	DefI(ICON_CEMENT, "gui/icons/cement.png", "\\cement");
	DefI(ICON_ENERGY, "gui/icons/energy.png", "\\energy");
	DefI(ICON_ENRICHEDURAN, "gui/icons/uranium.png", "\\enricheduran");
	DefI(ICON_COAL, "gui/icons/coal.png", "\\coal");
	DefI(ICON_TIME, "gui/icons/time.png", "\\time");
	DefI(ICON_RETFUEL, "gui/icons/fuelretail.png", "\\retfuel");
	DefI(ICON_LOGS, "gui/icons/logs.png", "\\logs");
	DefI(ICON_LUMBER, "gui/icons/lumber.png", "\\lumber");
	DefI(ICON_WATER, "gui/icons/water.png", "\\water");
	DefI(ICON_EXCLAMATION, "gui/icons/exclamation.png", "\\exclam");
	DefI(ICON_CENTS, "gui/icons/cents.png", "\\cent");

	//return;

	// Resource types

#if 0
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_CHEMICALS		5
#define RES_ELECTRONICS		6
#define RES_IRONORE			7
#define RES_METAL			8
#define RES_STONE			9
#define RES_CEMENT			10
#define RES_COAL			11
#define RES_URANIUM			12
#define RES_PRODUCTION		13
#define RES_CRUDEOIL		15
#define RES_FUEL			16
#define RES_ENERGY			17
#define RESOURCES			18
#endif

#if 0
	void DefR(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a)
#endif

	DefR(RES_DOLLARS,		"Funds",				"",							ICON_DOLLARS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_NONE);
	DefR(RES_LABOUR,		"Labour",				"",							ICON_LABOUR,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_NONE);
	DefR(RES_HOUSING,		"Housing",				"",							ICON_HOUSING,		true,	true,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_FARMPRODUCTS,	"Farm Products",		"Fertile",					ICON_FARMPRODUCT,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_PRODUCTION,	"Production",			"",							ICON_PRODUCTION,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_RETFOOD,		"Retail Food",			"",							ICON_RETFOOD,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_CRUDEOIL,		"Crude Oil",			"Oil Deposit",				ICON_CRUDEOIL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_CRPIPE);
	DefR(RES_FUEL,			"Fuel",					"",							ICON_WSFUEL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_ENERGY,		"Energy",				"",							ICON_ENERGY,		false,	true,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_POWL);
	DefR(RES_CHEMICALS,		"Chemicals",			"",							ICON_CHEMICALS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_ELECTRONICS,	"Electronics",			"",							ICON_ELECTRONICS,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_IRONORE,		"Iron Ore",				"",							ICON_IRONORE,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_METAL,			"Metal",				"",							ICON_STEEL,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_STONE,			"Stone",				"",							ICON_STONE,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_CEMENT,		"Cement",				"",							ICON_CEMENT,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_COAL,			"Coal",					"",							ICON_COAL,			true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);
	DefR(RES_URANIUM,		"Uranium",				"",							ICON_ENRICHEDURAN,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f,	CONDUIT_ROAD);


	// Various environment textures

	QueueTexture(&g_tiletexs[TILE_SAND], "textures/terrain/default/sand.jpg", false, false);
	QueueTexture(&g_tiletexs[TILE_GRASS], "textures/terrain/default/grass.png", false, false);
	//QueueTexture(&g_tiletexs[TILE_ROCK], "textures/terrain/default/rock.png", false, false);
	QueueTexture(&g_tiletexs[TILE_ROCK], "textures/terrain/default/rock.jpg", false, true);
	QueueTexture(&g_tiletexs[TILE_ROCK_NORM], "textures/terrain/default/rock.norm.jpg", false, false);
	QueueTexture(&g_tiletexs[TILE_CRACKEDROCK], "textures/terrain/default/crackedrock.jpg", false, false);
	QueueTexture(&g_tiletexs[TILE_CRACKEDROCK_NORM], "textures/terrain/default/crackedrock.norm.jpg", false, false);

	QueueTexture(&g_rimtexs[TEX_DIFF], "textures/terrain/default/underground.jpg", false, false);
	QueueTexture(&g_rimtexs[TEX_SPEC], "textures/terrain/default/underground.spec.jpg", false, false);
	QueueTexture(&g_rimtexs[TEX_NORM], "textures/terrain/default/underground.norm.jpg", false, false);

	//QueueTexture(&g_water, "textures/terrain/default/water.png", false);
	QueueTexture(&g_watertex[WATER_TEX_GRADIENT], "textures/terrain/default/water.gradient.png", false, false);
	QueueTexture(&g_watertex[WATER_TEX_DETAIL], "textures/terrain/default/water.detail.jpg", false, false);
	//QueueTexture(&g_watertex[WATER_TEX_DETAIL], "textures/terrain/default/water2.png", false, true);
	QueueTexture(&g_watertex[WATER_TEX_SPECULAR], "textures/terrain/default/water.spec.jpg", false, false);
	//QueueTexture(&g_watertex[WATER_TEX_NORMAL], "textures/terrain/default/water.norm.jpg", false, true);
	QueueTexture(&g_watertex[WATER_TEX_NORMAL], "textures/terrain/default/water5.norm.jpg", false, false);

	QueueTexture(&g_circle, "gui/circle.png", true, true);

	LoadParticles();
	LoadSkyBox("textures/terrain/default/skydome");

	//return;

	// Players

#if 1
	QueueModel(&g_playerm, "models/brain/brain.ms3d", Vec3f(50, 50, 50), Vec3f(0,0,0), true);

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];
		PlayerColor* pyc = &g_pycols[i];

		char name[64];
		sprintf(name, "Player %d (%s)", i, pyc->name);

		DefP(i, pyc->color[0]/255.0f, pyc->color[1]/255.0f, pyc->color[2]/255.0f, 1, RichText(name));

		SubmitConsole(&p->name);
	}
#endif

	// Unit types

	DefU(UNIT_BATTLECOMP, "models/battlecomp2011simp/battlecomp.ms3d", Vec3f(1,1,1)*182.0f/72.0f, Vec3f(0,0,0)*182.0f/72.0f, Vec3i(125, 250, 125), "Droid", 100, true, true, false, false, false, 6, true);
	UCost(UNIT_BATTLECOMP, RES_PRODUCTION, 10);
	
	DefU(UNIT_CARLYLE, "models/carlyle/carlyle.ms3d", Vec3f(1,1,1)*220.0f/72.0f, Vec3f(0,0,0)*182.0f/72.0f, Vec3i(250, 250, 250), "Tank", 100, true, true, false, false, false, 16, true);
	UCost(UNIT_CARLYLE, RES_PRODUCTION, 15);
	
	//DefU(UNIT_LABOURER, "models/labourer/labourer.ms3d", Vec3f(1,1,1)*182.0f/100.0f, Vec3f(0,0,0)*182.0f/100.0f, Vec3i(125, 250, 125), "Labourer", 100, true, true, false, false, false, 6, false);
	DefU(UNIT_LABOURER, "models/labourer/labourer.ms3d", Vec3f(1,1,1)*182.0f/100.0f, Vec3f(0,0,0)*182.0f/100.0f, Vec3i(50, 150, 50), "Labourer", 100, true, true, false, false, false, 6, false);
	
	//DefU(UNIT_TRUCK, "models/truck/truck.ms3d", Vec3f(1,1,1)*30.0f, Vec3f(0,0,0), Vec3i(125, 250, 125), "Truck", 100, true, false, true, false, false, 30, false);
	DefU(UNIT_TRUCK, "models/truck/truck.ms3d", Vec3f(1,1,1)*30.0f, Vec3f(0,0,0), Vec3i(100, 250, 100), "Truck", 100, true, false, true, false, false, 30, false);
	UCost(UNIT_TRUCK, RES_PRODUCTION, 1);

	// Foliage types

#if 0
	DefF(FL_TREE1, "models/pine/pine.ms3d", Vec3f(200,200,200), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefF(FL_TREE2, "models/pine/pine.ms3d", Vec3f(200,200,200), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefF(FL_TREE3, "models/pine/pine.ms3d", Vec3f(200,200,200), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
#elif 1
	DefF(FL_TREE1, "models/spruce1/spruce1.ms3d", Vec3f(20,20,20), Vec3f(0,0,0), Vec3i(125, 200, 125)*3);
	DefF(FL_TREE2, "models/spruce1/spruce1.ms3d", Vec3f(20,20,20), Vec3f(0,0,0), Vec3i(125, 200, 125)*3);
	DefF(FL_TREE3, "models/spruce1/spruce1.ms3d", Vec3f(20,20,20), Vec3f(0,0,0), Vec3i(125, 200, 125)*3);
#elif 1
	DefF(FL_TREE1, "models/trees/tree1/tree1.ms3d", Vec3f(20,20,20)*8, Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefF(FL_TREE2, "models/trees/tree2/tree2.ms3d", Vec3f(20,20,20)*8, Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefF(FL_TREE3, "models/trees/tree3/tree3.ms3d", Vec3f(20,20,20)*8, Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
#endif

	// Building types

#if 0
#define BL_HOUSE		0
#define BL_STORE		1
#define BL_FACTORY		2
#define BL_FARM			3
#define BL_MINE			4
#define BL_SMELTER		5
#define BL_OILWELL		6
#define BL_REFINERY		7
#define BL_NUCPOW			8
#define BL_COALPOW		9
#define BL_QUARRY			10
#define BL_CEMPLANT		11
#define BL_CHEMPLANT		12
#define BL_ELECPLANT		13
#define BL_TYPES			14
#endif

#if 0	//from old corpstates
		//		Building	Name					Model									Construction model							wX wZ	HP		Qta
	BDefine(APARTMENT,	"Apartment",		"models\\apartment\\apartment.ms3d",		"models\\apartment\\apartment_c.ms3d",		2, 2,	100,	1);
	BCost(LABOUR, 4);
	BCost(CEMENT, 4);
	BCost(STONE, 2);
	//BIn(ELECTRICITY, 1);
	BOut(HOUSING, 8);	// = 1
    
	BDefine(SHOPCPLX,	"Shopping Complex",	"models\\shopcplx\\shopcplx.ms3d",			"models\\shopcplx\\shopcplx_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 2);
	BIn(LABOUR, 10);		//+10
	BIn(PRODUCE, 5);		//+70
	BIn(PRODUCTION, 5);		//+15
	BIn(ELECTRICITY, 3);	//+6
	BOut(CONSUMERGOODS, 39);	// 101/10+1+2*12 = 35.1
    
	BDefine(FACTORY,	"Factory",			"models\\factory\\factory.ms3d",			"models\\factory\\factory_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 2);
	BCost(CEMENT, 3);
	BCost(ELECTRONICS, 10);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 10);	//+20
	BIn(CHEMICALS, 3);		//+12
	BIn(METAL, 3);
	BOut(PRODUCTION, 18);	// 32/20+1+1*12 = 15
	BBuildable(TRUCK);
	BBuildable(BATTLECOMP);
	BBuildable(CARLYLE);
    
	BDefine(FARM,		"Farm",				"models\\farm\\farm.ms3d",					"models\\farm\\farm_c.ms3d",				4, 2,	100,	10);
	BCost(LABOUR, 4);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 1);	//+2
	BIn(CHEMICALS, 1);		//+4
	BOut(PRODUCE, 5);		//16/5+1+1*12 = 16
    
	BDefine(MINE,		"Mine",				"models\\mine\\mine.ms3d",					"models\\mine\\mine_c.ms3d",				2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	//BCost(METAL, 4);
	BIn(LABOUR, 10);		//+10
	BOut(ORE, 5);			// 10/15+1 = 2
	BOut(URONIUM, 5);
	BOut(COHL, 5);
    
	BDefine(SMELTER,	"Smelter",			"models\\smelter\\smelter.ms3d",			"models\\smelter\\smelter_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 10);	//+20
	BIn(ORE, 5);			//+10
	BIn(CHEMICALS, 10);		//+40
	BOut(METAL, 5);			// 80/5+1+2*12 = 41
	BEmitter(EXHAUST, Vec3f(-7.0f, 23.2f, -11.5f));
	BEmitter(EXHAUST, Vec3f(-12.1f, 23.2f, -6.0f));
    
	BDefine(DERRICK,	"Derrick",			"models\\derrick\\derrick.ms3d",			"models\\derrick\\derrick_c.ms3d",			1, 1,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BIn(LABOUR, 10);	//+10
	BOut(CRUDE, 3);		// 10/3+1 = 4
    
	BDefine(REFINERY,	"Refinery",			"models\\refinery\\refinery.ms3d",			"models\\refinery\\refinery_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BIn(LABOUR, 1);			//+1
	BIn(ELECTRICITY, 1);	//+2
	BIn(CHEMICALS, 1);		//+4
	BIn(CRUDE, 3);			//+4
	BOut(ZETROL, 21);		// 11/7+1+1*12 = 17
	BEmitter(EXHAUST, Vec3f(-4.0f, 22.4f, -6.5f));
	BEmitter(EXHAUST, Vec3f(8.9f, 22.4f, 9.0f));
    
	BDefine(REACTOR,	"Reactor",			"models\\reactor\\reactor.ms3d",			"models\\reactor\\reactor_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 3);
	BIn(LABOUR, 10);	//+10
	BIn(URONIUM, 1);	//+2
	BOut(ELECTRICITY, 10);	// 12/10+1+1*12 = 14
	BEmitter(EXHAUSTBIG, Vec3f(1.8f, 15.69f, -7.99f));
	BEmitter(EXHAUSTBIG, Vec3f(-8.0f, 15.69f, 2.35f));
    
	BDefine(COMBUSTOR,	"Combustor",		"models\\combustor\\combustor.ms3d",		"models\\combustor\\combustor_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 1);
	BIn(LABOUR, 10);	//+10
	BIn(COHL, 1);		//+2
	BOut(ELECTRICITY, 10);	// 12/10+1+1*12 = 14
	BEmitter(EXHAUST, Vec3f(-8.7f, 23.1f, -11.3f));
	BEmitter(EXHAUST, Vec3f(-10.7f, 23.1f, -8.8f));
	BEmitter(EXHAUST, Vec3f(-10.3f, 23.1f, 9.6f));
	BEmitter(EXHAUST, Vec3f(-8.4f, 23.1f, 11.8f));
    
	BDefine(QUARRY,		"Quarry",			"models\\quarry\\quarry.ms3d",				"models\\quarry\\quarry_c.ms3d",			2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BIn(LABOUR, 10);	//+10
	BOut(STONE, 5);		// 10/5+1 = 3

	BDefine(CEMPLANT,	"Cement Plant",		"models\\cemplant\\cemplant.ms3d",			"models\\cemplant\\cemplant_c.ms3d",		1, 1,	100,	10);
	BCost(LABOUR, 4);
	BCost(METAL, 3);
	BIn(LABOUR, 10);	//+10
	BIn(STONE, 5);		//+15
	BOut(CEMENT, 5);	//  (25+1*12)/5+1 = 18
    
	BDefine(CHEMPLANT,	"Chemical Plant",	"models\\chemplant\\chemplant.ms3d",		"models\\chemplant\\chemplant_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 2);
	BCost(STONE, 1);
	BCost(ELECTRONICS, 1);
	BCost(METAL, 1);
	BIn(LABOUR, 10);		//+10
	BIn(ELECTRICITY, 10);	//+20
	BOut(CHEMICALS, 13);	// 30/10+1 = 4
    
	BDefine(ELECPLANT,	"Electronics Plant", "models\\elecplant\\elecplant.ms3d",		"models\\elecplant\\elecplant_c.ms3d",		2, 2,	100,	10);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(METAL, 3);
	BIn(LABOUR, 1);		//+10
	BIn(ELECTRICITY, 1);	//+20
	BIn(METAL, 1);			//+27
	BIn(CHEMICALS, 1);		//+40
	BOut(ELECTRONICS, 25);	// 97/5+1+2*12 = 45
    
	BDefine(RNDFACILITY,	"R&D Facility",		"models\\rndfac\\rndfac.ms3d",				"models\\rndfac\\rndfac_c.ms3d",			2, 2,	100,	1);
	BCost(LABOUR, 4);
	BCost(CEMENT, 3);
	BCost(STONE, 1);
	BCost(METAL, 1);
	BIn(LABOUR, 1);			//+10
	BIn(ELECTRICITY, 1);		//+20
	BIn(METAL, 1);				//+27
	BIn(ELECTRONICS, 1);		//+21
	BIn(CHEMICALS, 1);			//+40
	BOut(RESEARCH, 1);			// 118/1+1+3*12 = 155
#endif

	//DefB(BL_HOUSE, "Apartments", Vec2i(2,1), "models/apartment1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/apartment1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
#if 0
	DefB(BL_HOUSE, "Apartments", 
		Vec2i(2,2),  false, 
		"models/apartment2/b1911", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/apartment2/b1911", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#elif 1
	DefB(BL_HOUSE, "House", 
		Vec2i(1,1),  false, 
		"models/naping1/naping1", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/naping1/naping1", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_HOUSE, "Apartments", 
		Vec2i(2,2),  false, 
		"models/box/apartment/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		"models/box/apartment_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_HOUSE, RES_CEMENT, 4);
	BMat(BL_HOUSE, RES_LABOUR, 4);
	BMat(BL_HOUSE, RES_STONE, 2);
	BOut(BL_HOUSE, RES_HOUSING, 15);
	BDesc(BL_HOUSE, "Apartments collect rent from labourers. They are required by the labourers to regenerate labour power.");
	BSound(BL_HOUSE, BLSND_FINI, "sounds/notif/beep-22.wav");

#if 1
	DefB(BL_FACTORY, "Factory", 
		Vec2i(1,1),  false, 
		"models/factory3/factory3", 
		Vec3f(1,1,1)/2, Vec3f(0,0,0), 
		"models/factory3/factory3", 
		Vec3f(1,1,1)/2, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_FACTORY, "Factory", 
		Vec2i(1,1),  false, 
		"models/box/factory/basebuilding.ms3d", 
		Vec3f(1,1,1)/200, Vec3f(0,0,0), 
		"models/box/factory_c/basebuilding.ms3d", 
		Vec3f(1,1,1)/200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_FACTORY, RES_LABOUR, 4);
	BMat(BL_FACTORY, RES_METAL, 2);
	BMat(BL_FACTORY, RES_CEMENT, 3);
	BMat(BL_FACTORY, RES_ELECTRONICS, 10);
	BIn(BL_FACTORY, RES_LABOUR, 10);
	BIn(BL_FACTORY, RES_ENERGY, 10);
	BIn(BL_FACTORY, RES_CHEMICALS, 3);
	BIn(BL_FACTORY, RES_METAL, 3);
	BOut(BL_FACTORY, RES_PRODUCTION, 18);
	BDesc(BL_FACTORY, "Factories produce units. They generate production necessary for the processing and packaging of farm products to create retail food.");
	BSound(BL_FACTORY, BLSND_PROD, "sounds/notif/button-39.wav");
	BSound(BL_FACTORY, BLSND_FINI, "sounds/notif/beep-22.wav");
	BMan(BL_FACTORY, UNIT_TRUCK);
	BMan(BL_FACTORY, UNIT_BATTLECOMP);
	BMan(BL_FACTORY, UNIT_CARLYLE);
	
#if 0
	DefB(BL_REFINERY, "Oil Refinery", 
		Vec2i(2,2),  false, 
		"models/refinery2/refinery2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/refinery2/refinery2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#elif 1
	DefB(BL_REFINERY, "Oil Refinery", 
		Vec2i(1,1),  false, 
		"models/oilref/oilref", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/oilref/oilref", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#elif 0
	DefB(BL_REFINERY, "Oil Refinery", 
		Vec2i(2,2),  false, 
		"models/box/refinery/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		"models/box/refinery_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_REFINERY, RES_CEMENT, 5);
	BMat(BL_REFINERY, RES_LABOUR, 10);
	BIn(BL_REFINERY, RES_LABOUR, 10);
	BIn(BL_REFINERY, RES_ENERGY, 5);
	BIn(BL_REFINERY, RES_CRUDEOIL, 100);
	BOut(BL_REFINERY, RES_FUEL, 6000);
	BEmitter(BL_REFINERY, 0, PARTICLE_EXHAUST, Vec3f(TILE_SIZE*0.4f, TILE_SIZE*4, TILE_SIZE*-0.2f));
	//BEmitter(BL_REFINERY, 1, PARTICLE_EXHAUST2, Vec3f(TILE_SIZE*5.7/10, TILE_SIZE*3/2, TILE_SIZE*-5/10));
	//BEmitter(BL_REFINERY, 2, PARTICLE_EXHAUST, Vec3f(TILE_SIZE*-4.5/10, TILE_SIZE*1.75, TILE_SIZE*3.0f/10));
	//BEmitter(BL_REFINERY, 3, PARTICLE_EXHAUST2, Vec3f(TILE_SIZE*-4.5/10, TILE_SIZE*1.75, TILE_SIZE*3.0f/10));
	BDesc(BL_REFINERY, "Turn crude oil into wholesale fuel.");
	BSound(BL_REFINERY, BLSND_PROD, "sounds/notif/beep-23.wav");
	BSound(BL_REFINERY, BLSND_FINI, "sounds/notif/beep-22.wav");
	
#if 0
	DefB(BL_REFINERY, "Gas Station", 
		Vec2i(2,2),  false, "models/refinery2/refinery2", 
		Vec3f(1,1,1), Vec3f(0,0,0), "models/refinery2/refinery2", 
		Vec3f(1,1,1), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE, 1000);
	BMat(BL_REFINERY, RES_CEMENT, 5);
	BMat(BL_REFINERY, RES_LABOUR, 10);
	BIn(BL_REFINERY, RES_ENERGY, 50);
	BIn(BL_REFINERY, RES_WSFUEL, 5);
	BOut(BL_REFINERY, RES_RETFUEL, 5);
	BDesc(BL_REFINERY, "Turn wholesale fuel into retail fuel, generated at refineries.");
#endif

#if 1
	DefB(BL_COALPOW, "Coal Powerplant", 
		Vec2i(2,2), false, 
		"models/coalpow/combustor.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		"models/coalpow/combustor.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_COALPOW, "Coal Powerplant", 
		Vec2i(2,2), false, 
		"models/box/coalpow/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		"models/box/coalpow_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_COALPOW, RES_CEMENT, 5);
	BMat(BL_COALPOW, RES_LABOUR, 10);
	BIn(BL_COALPOW, RES_COAL, 5);
	BIn(BL_COALPOW, RES_LABOUR, 5);
	BOut(BL_COALPOW, RES_ENERGY, 45);
	//BEmitter(BL_COALPOW, 0, PARTICLE_EXHAUST2, Vec3f(-9.5f + 1, 23.4f, 10.6f + 1)*TILE_SIZE/32.0f*2.0f);
	//BEmitter(BL_COALPOW, 1, PARTICLE_EXHAUST2, Vec3f(-9.9f - 1, 23.4f, -10.0f + 1)*TILE_SIZE/32.0f*2.0f);
	//BEmitter(BL_COALPOW, 2, PARTICLE_EXHAUST2, Vec3f(-9.5f - 1, 23.4f, 10.6f - 1)*TILE_SIZE/32.0f*2.0f);
	//BEmitter(BL_COALPOW, 3, PARTICLE_EXHAUST2, Vec3f(-9.9f + 1, 23.4f, -10.0f - 1)*TILE_SIZE/32.0f*2.0f);
	BDesc(BL_COALPOW, "Generates electricity from coal.");
	BSound(BL_COALPOW, BLSND_PROD, "sounds/notif/beep-24.wav");
	BSound(BL_COALPOW, BLSND_FINI, "sounds/notif/beep-22.wav");
	
#if 0
	DefB(BL_CHEMPLANT, "Chemical Plant", 
		Vec2i(2,2), true, 
		"models/chemplant/chemplant.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		"models/chemplant/chemplant.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#elif 1
	DefB(BL_CHEMPLANT, "Chemical Plant", 
		Vec2i(1,1), false, 
		"models/chempl/chempl", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/chempl/chempl", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_CHEMPLANT, "Chemical Plant", 
		Vec2i(2,2), false, 
		"models/box/chemplant/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		"models/box/chemplant_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_CHEMPLANT, RES_CEMENT, 5);
	BMat(BL_CHEMPLANT, RES_LABOUR, 10);
	BIn(BL_CHEMPLANT, RES_IRONORE, 5);
	BIn(BL_CHEMPLANT, RES_ENERGY, 5);
	BIn(BL_CHEMPLANT, RES_LABOUR, 5);
	BOut(BL_CHEMPLANT, RES_CHEMICALS, 10);
	BDesc(BL_CHEMPLANT, "Generates chemicals necessary for farming and petrol refining.");
	BSound(BL_CHEMPLANT, BLSND_PROD, "sounds/notif/beep-25.wav");
	BSound(BL_CHEMPLANT, BLSND_FINI, "sounds/notif/beep-22.wav");
	
#if 1
	DefB(BL_ELECPLANT, "Electronics Plant", 
		Vec2i(2,2), true, 
		"models/elecplant/elecplant.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		"models/elecplant/elecplant.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_ELECPLANT, "Electronics Plant", 
		Vec2i(2,2), false, 
		"models/box/elecplant/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		"models/box/elecplant_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_ELECPLANT, RES_CEMENT, 5);
	BMat(BL_ELECPLANT, RES_LABOUR, 10);
	BIn(BL_ELECPLANT, RES_IRONORE, 5);
	BIn(BL_ELECPLANT, RES_ENERGY, 5);
	BIn(BL_ELECPLANT, RES_LABOUR, 5);
	BOut(BL_ELECPLANT, RES_ELECTRONICS, 10);
	BDesc(BL_ELECPLANT, "Produces electronics necessary for units.");
	BSound(BL_ELECPLANT, BLSND_PROD, "sounds/notif/beep-26.wav");
	BSound(BL_ELECPLANT, BLSND_FINI, "sounds/notif/beep-22.wav");

#if 1
	DefB(BL_CEMPLANT, "Cement Plant", 
		Vec2i(1,1), false, 
		"models/cemplant/cemplant.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		"models/cemplant/cemplant.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_CEMPLANT, "Cement Plant", 
		Vec2i(1,1), false, 
		"models/box/cemplant/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		"models/box/cemplant_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_CEMPLANT, RES_CEMENT, 5);
	BMat(BL_CEMPLANT, RES_LABOUR, 10);
	BIn(BL_CEMPLANT, RES_STONE, 5);
	BIn(BL_CEMPLANT, RES_ENERGY, 5);
	BIn(BL_CEMPLANT, RES_LABOUR, 5);
	BOut(BL_CEMPLANT, RES_CEMENT, 10);
	BDesc(BL_CEMPLANT, "Produces cement from stone.");
	BSound(BL_CEMPLANT, BLSND_PROD, "sounds/notif/beep-027.wav");
	BSound(BL_CEMPLANT, BLSND_FINI, "sounds/notif/beep-22.wav");
	
#if 0
#if 1
	DefB(BL_QUARRY, "Quarry", 
		Vec2i(1,1), true, 
		"models/quarry/quarry.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE, Vec3f(0,0,0), 
		"models/quarry/quarry.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_QUARRY, "Quarry", 
		Vec2i(1,1), false, 
		"models/box/quarry/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		"models/box/quarry_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_QUARRY, RES_CEMENT, 5);
	BMat(BL_QUARRY, RES_LABOUR, 10);
	BIn(BL_QUARRY, RES_ENERGY, 5);
	BIn(BL_QUARRY, RES_LABOUR, 5);
	BOut(BL_QUARRY, RES_STONE, 10);
	BDesc(BL_QUARRY, "Extracts stone.");
	BSound(BL_QUARRY, BLSND_PROD, "sounds/notif/beep-28.wav");
	BSound(BL_QUARRY, BLSND_FINI, "sounds/notif/beep-22.wav");
#endif

#if 0
	DefB(BL_SMELTER, "Smelter", 
		Vec2i(2,2), true, 
		"models/smelter/smelter.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		"models/smelter/smelter.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE*2, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#elif 1
	DefB(BL_SMELTER, "Iron Smelter", 
		Vec2i(1,1), false, 
		"models/ironsm/ironsm", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/ironsm/ironsm", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_SMELTER, "Smelter", 
		Vec2i(2,2), false, 
		"models/box/smelter/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		"models/box/smelter_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_SMELTER, RES_LABOUR, 4);
	BMat(BL_SMELTER, RES_CEMENT, 3);
	BMat(BL_SMELTER, RES_STONE, 1);
	BIn(BL_SMELTER, RES_LABOUR, 10);
	BIn(BL_SMELTER, RES_IRONORE, 5);
	BIn(BL_SMELTER, RES_CHEMICALS, 10);
	BOut(BL_SMELTER, RES_METAL, 10);
	BDesc(BL_SMELTER, "Turns iron ore into metal.");
	BSound(BL_SMELTER, BLSND_PROD, "sounds/notif/beep-29.wav");
	BSound(BL_SMELTER, BLSND_FINI, "sounds/notif/beep-22.wav");

#if 1
	DefB(BL_NUCPOW, "Nuclear Powerplant", 
		Vec2i(2,2), false, 
		"models/nucpow2/nucpow2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/nucpow2/nucpow2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_NUCPOW, "Nuclear Powerplant", 
		Vec2i(2,2), false, 
		"models/box/nucpow/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		"models/box/nucpow_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_NUCPOW, RES_CEMENT, 5);
	BMat(BL_NUCPOW, RES_LABOUR, 10);
	BIn(BL_NUCPOW, RES_URANIUM, 5);
	BIn(BL_NUCPOW, RES_LABOUR, 5);
	BOut(BL_NUCPOW, RES_ENERGY, 45);
	//BEmitter(BL_NUCPOW, 0, PARTICLE_EXHAUSTBIG, Vec3f(TILE_SIZE*-0.63f, TILE_SIZE*1.5f, TILE_SIZE*0));
	//BEmitter(BL_NUCPOW, 1, PARTICLE_EXHAUSTBIG, Vec3f(TILE_SIZE*0.17f, TILE_SIZE*1.5f, TILE_SIZE*-0.64f));
	BDesc(BL_NUCPOW, "Generates electricity from uranium.");
	BSound(BL_NUCPOW, BLSND_PROD, "sounds/notif/beep-30b.wav");
	BSound(BL_NUCPOW, BLSND_FINI, "sounds/notif/beep-22.wav");

#if 0
	DefB(BL_FARM, "Farm", 
		Vec2i(4,2), true, 
		"models/farm2/farm2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/farm2/farm2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#elif 1
	DefB(BL_FARM, "Farm", 
		Vec2i(1,1), true, 
		"models/farm3/farm", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/farm3/farm", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_FARM, "Farm", 
		Vec2i(4,2), false, 
		"models/box/farm/basebuilding.ms3d", 
		Vec3f(2,1,1)*200, Vec3f(0,0,0), 
		"models/box/farm_c/basebuilding.ms3d", 
		Vec3f(2,1,1)*200, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_FARM, RES_LABOUR, 4);
	BIn(BL_FARM, RES_LABOUR, 10);
	BIn(BL_FARM, RES_ENERGY, 1);
	BIn(BL_FARM, RES_CHEMICALS, 1);
	BOut(BL_FARM, RES_FARMPRODUCTS, 1900);
	BDesc(BL_FARM, "Produces farm products, necessary for the production of retail food.");
	BSound(BL_FARM, BLSND_PROD, "sounds/notif/button-16.wav");
	BSound(BL_FARM, BLSND_FINI, "sounds/notif/beep-22.wav");

#if 1
	DefB(BL_STORE, "Store", 
		Vec2i(2,1), true, 
		"models/store1/hugterr.ms3d", 
		Vec3f(100,100,100), Vec3f(0,0,0), 
		"models/store1/hugterr.ms3d", 
		Vec3f(100,100,100), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#else
	DefB(BL_STORE, "Store", 
		Vec2i(2,1), false, 
		"models/box/store/basebuilding.ms3d", 
		Vec3f(2,1,1)*100, Vec3f(0,0,0), 
		"models/box/store_c/basebuilding.ms3d", 
		Vec3f(2,1,1)*100, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_NONE, 1000);
#endif
	BMat(BL_STORE, RES_CEMENT, 5);
	BMat(BL_STORE, RES_LABOUR, 10);
	BIn(BL_STORE, RES_LABOUR, 5);
	BIn(BL_STORE, RES_ENERGY, 10);
	//BIn(BL_STORE, RES_FARMPRODUCTS, 3600);
	BIn(BL_STORE, RES_FARMPRODUCTS, 1600);
	BIn(BL_STORE, RES_PRODUCTION, 5);
	//BOut(BL_STORE, RES_RETFOOD, 3600);
	BOut(BL_STORE, RES_RETFOOD, 3600);
	BDesc(BL_STORE, "Generates retail food from farm products and production, necessary for labourers to survive and multiply.");
	BSound(BL_STORE, BLSND_PROD, "sounds/notif/button-19.wav");
	BSound(BL_STORE, BLSND_FINI, "sounds/notif/beep-22.wav");

#if 1
	DefB(BL_OILWELL, "Oil Well", 
		Vec2i(1,1), false, 
		"models/oilwell2/oilwell2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/oilwell2/oilwell2", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_CRUDEOIL, 1000);
#else
	DefB(BL_OILWELL, "Oil Well", 
		Vec2i(1,1), false, 
		"models/box/oilwell/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		"models/box/oilwell_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		FOUNDATION_LAND, RES_CRUDEOIL, 1000);
#endif
	BMat(BL_OILWELL, RES_CEMENT, 5);
	BMat(BL_OILWELL, RES_LABOUR, 10);
	BIn(BL_OILWELL, RES_ENERGY, 5);
	BIn(BL_OILWELL, RES_LABOUR, 5);
	BOut(BL_OILWELL, RES_CRUDEOIL, 100);
	BDesc(BL_OILWELL, "Pumps up crude oil, necessary for fuel, which is consumed by all road vehicles.");
	BSound(BL_OILWELL, BLSND_PROD, "sounds/notif/button-31.wav");
	BSound(BL_OILWELL, BLSND_FINI, "sounds/notif/beep-22.wav");

#if 0
	DefB(BL_MINE, "Mine", 
		Vec2i(1,1), false, 
		"models/mine/nobottom.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE, Vec3f(0,0,0), 
		"models/mine/nobottom.ms3d", 
		Vec3f(1,1,1)/32.0f*TILE_SIZE, Vec3f(0,0,0), 
		FOUNDATION_LAND, -1, 1000);
#elif 1
	DefB(BL_MINE, "Shaft Mine", 
		Vec2i(1,1), false, 
		"models/shmine/shmine", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		"models/shmine/shmine", 
		Vec3f(1,1,1), Vec3f(0,0,0), 
		FOUNDATION_LAND, -1, 1000);
#else
	DefB(BL_MINE, "Mine", 
		Vec2i(1,1), false, 
		"models/box/mine/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		"models/box/mine_c/basebuilding.ms3d", 
		Vec3f(1,1,1)*100, Vec3f(0,0,0), 
		FOUNDATION_LAND, -1, 1000);
#endif
	BMat(BL_MINE, RES_LABOUR, 4);
	BMat(BL_MINE, RES_CEMENT, 3);
	BIn(BL_MINE, RES_LABOUR, 10);
	BIn(BL_MINE, RES_LABOUR, 5);
	BOut(BL_MINE, RES_IRONORE, 7);
	BOut(BL_MINE, RES_STONE, 10);
	BOut(BL_MINE, RES_URANIUM, 5);
	BOut(BL_MINE, RES_COAL, 5);
	BDesc(BL_MINE, "Digs up minerals necessary for production at factories, stone, and uranium, necessary for electricity generation at nuclear powerplants.");
	BSound(BL_MINE, BLSND_PROD, "sounds/notif/button-32.wav");
	BSound(BL_MINE, BLSND_FINI, "sounds/notif/beep-22.wav");


	// Conduit types

	Vec3f scale(TILE_SIZE/16.0f, TILE_SIZE/16.0f, TILE_SIZE/16.0f);
	Vec3f trans(0,0,0);

	DefCo(CONDUIT_ROAD, "Road", offsetof(Building,roadnetw), offsetof(Selection,roads), ROAD_MAX_FOREW_INCLINE, ROAD_MAX_SIDEW_INCLINE, false, false, Vec2i(TILE_SIZE/2, TILE_SIZE/2), Vec3f(TILE_SIZE/2, 0, TILE_SIZE/2), "gui/hover/noroad.png");
	CoDesc(CONDUIT_ROAD, "Necessary for the transportation of all physical resources between buildings.");
	CoConMat(CONDUIT_ROAD, RES_LABOUR, 1);
	CoConMat(CONDUIT_ROAD, RES_CEMENT, 1);
	DefConn(CONDUIT_ROAD, CONNECTION_NOCONNECTION, CONSTRUCTION, "models/road/1_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTH, CONSTRUCTION, "models/road/n_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EAST, CONSTRUCTION, "models/road/e_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_SOUTH, CONSTRUCTION, "models/road/s_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_WEST, CONSTRUCTION, "models/road/w_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEAST, CONSTRUCTION, "models/road/ne_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/road/ns_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EASTSOUTH, CONSTRUCTION, "models/road/es_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHWEST, CONSTRUCTION, "models/road/nw_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EASTWEST, CONSTRUCTION, "models/road/ew_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_SOUTHWEST, CONSTRUCTION, "models/road/sw_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/road/esw_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/road/nsw_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/road/new_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/road/nes_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/road/nesw_c.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NOCONNECTION, FINISHED, "models/road/1.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTH, FINISHED, "models/road/n.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EAST, FINISHED, "models/road/e.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_SOUTH, FINISHED, "models/road/s.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_WEST, FINISHED, "models/road/w.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEAST, FINISHED, "models/road/ne.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHSOUTH, FINISHED, "models/road/ns.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EASTSOUTH, FINISHED, "models/road/es.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHWEST, FINISHED, "models/road/nw.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EASTWEST, FINISHED, "models/road/ew.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_SOUTHWEST, FINISHED, "models/road/sw.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_EASTSOUTHWEST, FINISHED, "models/road/esw.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHSOUTHWEST, FINISHED, "models/road/nsw.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEASTWEST, FINISHED, "models/road/new.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEASTSOUTH, FINISHED, "models/road/nes.ms3d", scale, trans);
	DefConn(CONDUIT_ROAD, CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/road/nesw.ms3d", scale, trans);

	DefCo(CONDUIT_POWL, "Powerline", offsetof(Building,pownetw), offsetof(Selection,powls), TILE_SIZE*2, TILE_SIZE*2, true, true, Vec2i(0, 0), Vec3f(0, 0, 0), "gui/hover/noelec.png");
	CoDesc(CONDUIT_POWL, "Necessary to conduct electricity between buildings.");
	CoConMat(CONDUIT_POWL, RES_LABOUR, 1);
	CoConMat(CONDUIT_POWL, RES_CEMENT, 1);
	DefConn(CONDUIT_POWL, CONNECTION_NOCONNECTION, CONSTRUCTION, "models/powl/1_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTH, CONSTRUCTION, "models/powl/n_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EAST, CONSTRUCTION, "models/powl/e_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_SOUTH, CONSTRUCTION, "models/powl/s_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_WEST, CONSTRUCTION, "models/powl/w_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEAST, CONSTRUCTION, "models/powl/ne_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/powl/ns_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EASTSOUTH, CONSTRUCTION, "models/powl/es_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHWEST, CONSTRUCTION, "models/powl/nw_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EASTWEST, CONSTRUCTION, "models/powl/ew_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_SOUTHWEST, CONSTRUCTION, "models/powl/sw_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/powl/esw_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/powl/nsw_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/powl/new_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/powl/nes_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/powl/nesw_c.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NOCONNECTION, FINISHED, "models/powl/1.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTH, FINISHED, "models/powl/n.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EAST, FINISHED, "models/powl/e.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_SOUTH, FINISHED, "models/powl/s.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_WEST, FINISHED, "models/powl/w.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEAST, FINISHED, "models/powl/ne.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHSOUTH, FINISHED, "models/powl/ns.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EASTSOUTH, FINISHED, "models/powl/es.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHWEST, FINISHED, "models/powl/nw.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EASTWEST, FINISHED, "models/powl/ew.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_SOUTHWEST, FINISHED, "models/powl/sw.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_EASTSOUTHWEST, FINISHED, "models/powl/esw.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHSOUTHWEST, FINISHED, "models/powl/nsw.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEASTWEST, FINISHED, "models/powl/new.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEASTSOUTH, FINISHED, "models/powl/nes.ms3d", scale, trans);
	DefConn(CONDUIT_POWL, CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/powl/nesw.ms3d", scale, trans);

	trans = Vec3f(-TILE_SIZE/2, 0, TILE_SIZE/2);
	DefCo(CONDUIT_CRPIPE, "Crude Oil Pipeline", offsetof(Building,crpipenetw), offsetof(Selection,crpipes), TILE_SIZE*2, TILE_SIZE*2, true, true, Vec2i(0, 0), Vec3f(0, 0, 0), "gui/hover/nocrude.png");
	CoDesc(CONDUIT_CRPIPE, "Pumps crude oil between oil wells and refineries.");
	CoConMat(CONDUIT_CRPIPE, RES_LABOUR, 1);
	CoConMat(CONDUIT_CRPIPE, RES_CEMENT, 1);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NOCONNECTION, CONSTRUCTION, "models/crpipe/1_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTH, CONSTRUCTION, "models/crpipe/n_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EAST, CONSTRUCTION, "models/crpipe/e_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_SOUTH, CONSTRUCTION, "models/crpipe/s_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_WEST, CONSTRUCTION, "models/crpipe/w_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEAST, CONSTRUCTION, "models/crpipe/ne_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/crpipe/ns_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EASTSOUTH, CONSTRUCTION, "models/crpipe/es_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHWEST, CONSTRUCTION, "models/crpipe/nw_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EASTWEST, CONSTRUCTION, "models/crpipe/ew_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_SOUTHWEST, CONSTRUCTION, "models/crpipe/sw_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/crpipe/esw_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/crpipe/nsw_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/crpipe/new_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/crpipe/nes_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/crpipe/nesw_c.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NOCONNECTION, FINISHED, "models/crpipe/1.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTH, FINISHED, "models/crpipe/n.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EAST, FINISHED, "models/crpipe/e.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_SOUTH, FINISHED, "models/crpipe/s.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_WEST, FINISHED, "models/crpipe/w.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEAST, FINISHED, "models/crpipe/ne.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHSOUTH, FINISHED, "models/crpipe/ns.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EASTSOUTH, FINISHED, "models/crpipe/es.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHWEST, FINISHED, "models/crpipe/nw.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EASTWEST, FINISHED, "models/crpipe/ew.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_SOUTHWEST, FINISHED, "models/crpipe/sw.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_EASTSOUTHWEST, FINISHED, "models/crpipe/esw.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHSOUTHWEST, FINISHED, "models/crpipe/nsw.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEASTWEST, FINISHED, "models/crpipe/new.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEASTSOUTH, FINISHED, "models/crpipe/nes.ms3d", scale, trans);
	DefConn(CONDUIT_CRPIPE, CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/crpipe/nesw.ms3d", scale, trans);


	// Sounds

#if 0
#define TRSND_NEWJOB	0
#define TRSND_DONEJOB	1
#define TRSND_WORK		2
#define TR_SOUNDS		3

extern short g_trsnd[TR_SOUNDS];
#endif

	LoadSound("sounds/notif/button-33a.wav", &g_trsnd[TRSND_NEWJOB]);
	LoadSound("sounds/notif/button-34.wav", &g_trsnd[TRSND_DONEJOB]);
	LoadSound("sounds/notif/button-35.wav", &g_trsnd[TRSND_WORK]);
	
	//LoadSound("sounds/notif/button-37.wav", &g_labsnd[LABSND_WORK]);
	LoadSound("sounds/notif/button-41.wav", &g_labsnd[LABSND_WORK]);
	LoadSound("sounds/notif/button-38.wav", &g_labsnd[LABSND_SHOP]);
	LoadSound("sounds/notif/beep-22.wav", &g_labsnd[LABSND_REST]);

#if 0
	g_ordersnd.clear();
	g_ordersnd.push_back(Sound("sounds/aaa000/gogogo.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/moveout2.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/spreadout.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/wereunderattack3.wav"));
	//g_zpainSnd.push_back(Sound("sounds/zpain.wav"));
#endif
#if 0
	g_ordersnd.clear();
	g_ordersnd.push_back(Sound());
	g_ordersnd[0] = Sound("sounds/aaa000/gogogo.wav");
#endif
}
