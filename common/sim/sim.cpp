#include "sim.h"
#include "../script/console.h"

long long g_simframe = 0;

// Queue all the game resources and define objects
void Queue()
{
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

	DefI(ICON_DOLLARS, "gui/icons/dollars.png", ":dollar:");
	DefI(ICON_PESOS, "gui/icons/pesos.png", ":peso:");
	DefI(ICON_EUROS, "gui/icons/euros.png", ":euro:");
	DefI(ICON_POUNDS, "gui/icons/pounds.png", ":pound:");
	DefI(ICON_FRANCS, "gui/icons/francs.png", ":franc:");
	DefI(ICON_YENS, "gui/icons/yens.png", ":yen:");
	DefI(ICON_RUPEES, "gui/icons/rupees.png", ":rupee:");
	DefI(ICON_ROUBLES, "gui/icons/roubles.png", ":rouble:");
	DefI(ICON_LABOUR, "gui/icons/labour.png", ":labour:");
	DefI(ICON_HOUSING, "gui/icons/housing.png", ":housing:");
	DefI(ICON_FARMPRODUCT, "gui/icons/farmproducts.png", ":farmprod:");
	DefI(ICON_WSFOOD, "gui/icons/wsfood.png", ":wsfood:");
	DefI(ICON_RETFOOD, "gui/icons/retfood.png", ":retfood:");
	DefI(ICON_CHEMICALS, "gui/icons/chemicals.png", ":chemicals:");
	DefI(ICON_ELECTRONICS, "gui/icons/electronics.png", ":electronics:");
	DefI(ICON_RESEARCH, "gui/icons/research.png", ":research:");
	DefI(ICON_PRODUCTION, "gui/icons/production.png", ":production:");
	DefI(ICON_IRONORE, "gui/icons/ironore.png", ":ironore:");
	DefI(ICON_URANIUMORE, "gui/icons/uraniumore.png", ":uraniumore:");
	DefI(ICON_STEEL, "gui/icons/steel.png", ":steel:");
	DefI(ICON_CRUDEOIL, "gui/icons/crudeoil.png", ":crudeoil:");
	DefI(ICON_WSFUEL, "gui/icons/fuelwholesale.png", ":wsfuel:");
	DefI(ICON_STONE, "gui/icons/stone.png", ":stone:");
	DefI(ICON_CEMENT, "gui/icons/cement.png", ":cement:");
	DefI(ICON_ENERGY, "gui/icons/energy.png", ":energy:");
	DefI(ICON_ENRICHEDURAN, "gui/icons/uranium.png", ":enricheduran:");
	DefI(ICON_COAL, "gui/icons/coal.png", ":coal:");
	DefI(ICON_TIME, "gui/icons/time.png", ":time:");
	DefI(ICON_RETFUEL, "gui/icons/fuelretail.png", ":retfuel:");
	DefI(ICON_LOGS, "gui/icons/logs.png", ":logs:");
	DefI(ICON_LUMBER, "gui/icons/lumber.png", ":lumber:");
	DefI(ICON_WATER, "gui/icons/water.png", ":water:");
	DefI(ICON_EXCLAMATION, "gui/icons/exclamation.png", ":exclam:");

#if 0
#define RES_NONE			-1
#define RES_FUNDS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_PRODUCTION		5
#define RES_MINERALS		6
#define RES_CRUDEOIL		7
#define RES_WSFUEL			8
#define RES_RETFUEL			9
#define RES_ENERGY			10
#define RES_URANIUM			11
#define RESOURCES			12
#endif

#if 0
	void DefR(int resi, const char* n, const char* depn, int iconindex, bool phys, bool cap, bool glob, float r, float g, float b, float a)
#endif

	DefR(RES_FUNDS,		"Funds",				"",							ICON_DOLLARS,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_LABOUR,		"Labour",				"",							ICON_LABOUR,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_HOUSING,		"Housing",				"",							ICON_HOUSING,		true,	true,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_FARMPRODUCTS,	"Farm Products",		"Fertile",					ICON_FARMPRODUCT,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_PRODUCTION,	"Production",			"",							ICON_PRODUCTION,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_RETFOOD,		"Retail Food",			"",							ICON_RETFOOD,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_MINERALS,		"Minerals",				"Iron Ore Deposit",			ICON_IRONORE,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_CRUDEOIL,		"Crude Oil",			"Oil Deposit",				ICON_CRUDEOIL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_WSFUEL,		"Wholesale Fuel",		"",							ICON_WSFUEL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_RETFUEL,		"Retail Fuel",			"",							ICON_RETFUEL,		true,	false,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_ENERGY,		"Energy",				"",							ICON_ENERGY,		true,	true,	false,	1.0f,1.0f,1.0f,1.0f);
	DefR(RES_URANIUM,		"Uranium",		"",							ICON_ENRICHEDURAN,	true,	false,	false,	1.0f,1.0f,1.0f,1.0f);

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

	QueueModel(&g_playerm, "models/brain/brain.ms3d", Vec3f(50, 50, 50), Vec3f(0,0,0), true);

	//QueueModel(&themodel, "models/battlecomp/battlecomp.ms3d", Vec3f(0.1f,0.1f,0.1f) * 100 / 64, Vec3f(0,100,0));

#if 0
#define RES_DOLLARS			0
#define RES_PESOS			1
#define RES_EUROS			2
#define RES_POUNDS			3
#define RES_FRANCS			4
#define RES_YENS			5
#define RES_RUPEES			6
#define RES_ROUBLES			7
#endif

#if 0
	DefP(0, 1, 0, 0, 1, RichText("Player 0 (Red)"));
	DefP(1, 0, 1, 0, 1, RichText("Player 1 (Green)"));
	DefP(2, 0, 0, 1, 1, RichText("Player 2 (Blue)"));
	DefP(3, 1, 0, 1, 1, RichText("Player 3 (Purple)"));
	DefP(4, 1, 1, 0, 1, RichText("Player 4 (Yellow)"));
	DefP(5, 0, 1, 1, 1, RichText("Player 5 (Cyan)"));
	DefP(6, 150.0f/255.0f, 249.0f/255.0f, 123.0f/255.0f, 1, RichText("Player 6 (Light Green)"));
	DefP(7, 3.0f/255.0f, 53.0f/255.0f, 0.0f/255.0f, 1, RichText("Player 7 (Dark Green)"));
#endif

	struct PlayerColor
	{
		unsigned char color[3];
		char name[32];
	};
	
	PlayerColor pcs[] =
	{
		{{0x7e, 0x1e, 0x9c}, "Purple"},
		{{0x15, 0xb0, 0x1a}, "Green"},
		{{0x03, 0x43, 0xdf}, "Blue"},
		{{0xff, 0x81, 0xc0}, "Pink"},
		{{0x65, 0x37, 0x00}, "Brown"},
		{{0xe5, 0x00, 0x00}, "Red"},
		{{0x95, 0xd0, 0xfc}, "Light Blue"},
		{{0x02, 0x93, 0x86}, "Teal"},
		{{0xf9, 0x73, 0x06}, "Orange"},
		{{0x96, 0xf9, 0x7b}, "Light Green"},
		{{0xc2, 0x00, 0x78}, "Magenta"},
		{{0xff, 0xff, 0x14}, "Yellow"},
		{{0x75, 0xbb, 0xfd}, "Sky Blue"},
		{{0x92, 0x95, 0x91}, "Grey"},
		{{0x89, 0xfe, 0x05}, "Lime Green"},
		{{0xbf, 0x77, 0xf6}, "Light Purple"},
		{{0x9a, 0x0e, 0xea}, "Violet"},
		{{0x33, 0x35, 0x00}, "Dark Green"},
		{{0x06, 0xc2, 0xac}, "Turquoise"},
		{{0xc7, 0x9f, 0xef}, "Lavender"},
		{{0x00, 0x03, 0x5b}, "Dark Blue"},
		{{0xd1, 0xb2, 0x6f}, "Tan"},
		{{0x00, 0xff, 0xff}, "Cyan"},
		{{0x13, 0xea, 0xc9}, "Aqua"},
		{{0x06, 0x47, 0x0c}, "Forest Green"},
		{{0xae, 0x71, 0x81}, "Mauve"},
		{{0x35, 0x06, 0x3e}, "Dark Purple"},
		{{0x01, 0xff, 0x07}, "Bright Green"},
		{{0x65, 0x00, 0x21}, "Maroon"},
		{{0x6e, 0x75, 0x0e}, "Olive"},
		{{0xff, 0x79, 0x6c}, "Salmon"},
		{{0xe6, 0xda, 0xa6}, "Beige"},
		{{0x05, 0x04, 0xaa}, "Royal Blue"},
		{{0x00, 0x11, 0x46}, "Navy Blue"},
		{{0xce, 0xa2, 0xfd}, "Lilac"},
		{{0x00, 0x00, 0x00}, "Black"},
		{{0xff, 0x02, 0x8d}, "Hot Pink"},
		{{0xad, 0x81, 0x50}, "Light Brown"},
		{{0xc7, 0xfd, 0xb5}, "Pale Green"},
		{{0xff, 0xb0, 0x7c}, "Peach"},
		{{0x67, 0x7a, 0x04}, "Olive Green"},
		{{0xcb, 0x41, 0x6b}, "Dark Pink"},
		{{0x8e, 0x82, 0xfe}, "Periwinkle"},
		{{0x53, 0xfc, 0xa1}, "Sea Green"},
		{{0xaa, 0xff, 0x32}, "Lime"},
		{{0x38, 0x02, 0x82}, "Indigo"},
		{{0xce, 0xb3, 0x01}, "Mustard"},
		{{0xff, 0xd1, 0xdf}, "Light Pink"}
	};

#if 1
	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		char name[64];
		sprintf(name, "Player %d (%s)", i, pcs[i].name);
		
		DefP(i, pcs[i].color[0]/255.0f, pcs[i].color[1]/255.0f, pcs[i].color[2]/255.0f, 1, RichText(name));

		SubmitConsole(&p->name);
	}
#endif

	DefU(UNIT_ROBOSOLDIER, "models/battlecomp2011simp/battlecomp.ms3d", Vec3f(1,1,1)*182.0f/70.0f, Vec3f(0,0,0)*182.0f/70.0f, Vec3i(125, 250, 125), "Robot Soldier", 100, true, true, false, false, false, 6, true);
	DefU(UNIT_LABOURER, "models/labourer/labourer.ms3d", Vec3f(1,1,1)*182.0f/70.0f, Vec3f(0,0,0)*182.0f/70.0f, Vec3i(125, 250, 125), "Labourer", 100, true, true, false, false, false, 6, false);
	DefU(UNIT_TRUCK, "models/truck/truck.ms3d", Vec3f(1,1,1)*30.0f, Vec3f(0,0,0), Vec3i(125, 250, 125), "Truck", 100, true, false, true, false, false, 30, false);

	DefF(FOLIAGE_TREE1, "models/pine/pine.ms3d", Vec3f(200,200,200), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefF(FOLIAGE_TREE2, "models/pine/pine.ms3d", Vec3f(200,200,200), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);
	DefF(FOLIAGE_TREE3, "models/pine/pine.ms3d", Vec3f(200,200,200), Vec3f(0,0,0), Vec3i(40, 60, 500)*20);

#if 0
#define BUILDING_NONE			-1
#define BUILDING_APARTMENT		0
#define BUILDING_HOUSE			1
#define BUILDING_STORE		2
#define BUILDING_FARM			3
#define BUILDING_GASSTATION		4
#define BUILDING_BANK			5
#define BUILDING_CARDEALERSHIP	6
#define BUILDING_CARFACTORY		7
#define BUILDING_MILITARYBASE	8
#define BUILDING_TRUCKFACTORY	9
#define BUILDING_CONCGTRFAC	10
#define BUILDING_TANKERTRFAC	11
#define BUILDING_CURREXCENTER	12
#define BUILDING_COALMINE		13
#define BUILDING_HOUR		14
#define BUILDING_LUMBERMILL		15
#define BUILDING_FOREQFACTORY	16
#define BUILDING_CEMENTPLANT	17
#define BUILDING_CHEMPLANT		18
#define BUILDING_COALPOW		19
#define BUILDING_ELECPLANT		20
#define BUILDING_IRONMINE		21
#define BUILDING_URANMINE		22
#define BUILDING_NUCPOW			23
#define BUILDING_OILREFINERY	24
#define BUILDING_OILWELL		25
#define BUILDING_RESEARCHFAC	26
#define BUILDING_STEELMILL		27
#define BUILDING_STONEQUARRY	28
#define BUILDING_URANENRICHPL	29
#define BUILDING_WATERTOWER		30
#define BUILDING_WATERPUMPSTN	31
#define BUILDING_OFFSHOREOILRIG	32
#endif

	//DefB(BUILDING_APARTMENT, "Apartment Building", Vec2i(2,1), "models/apartment1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/apartment1/basebuilding.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	DefB(BUILDING_APARTMENT, "Apartment Building", Vec2i(2,2),  false, "models/apartment2/b1911", Vec3f(1,1,1), Vec3f(0,0,0), "models/apartment2/b1911", Vec3f(1,1,1), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BConMat(BUILDING_APARTMENT, RES_MINERALS, 5);
	BConMat(BUILDING_APARTMENT, RES_LABOUR, 10);
	BInput(BUILDING_APARTMENT, RES_ENERGY, 5);
	BOutput(BUILDING_APARTMENT, RES_HOUSING, 5);

	DefB(BUILDING_FACTORY, "Factory", Vec2i(2,2),  false, "models/factory3/factory3", Vec3f(1,1,1), Vec3f(0,0,0), "models/factory3/factory3", Vec3f(1,1,1), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BConMat(BUILDING_FACTORY, RES_MINERALS, 5);
	BConMat(BUILDING_FACTORY, RES_LABOUR, 10);
	BInput(BUILDING_FACTORY, RES_ENERGY, 5);

	DefB(BUILDING_REFINERY, "Refinery", Vec2i(2,2),  false, "models/refinery2/refinery2", Vec3f(1,1,1), Vec3f(0,0,0), "models/refinery2/refinery2", Vec3f(1,1,1), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BConMat(BUILDING_REFINERY, RES_MINERALS, 5);
	BConMat(BUILDING_REFINERY, RES_LABOUR, 10);
	BInput(BUILDING_REFINERY, RES_ENERGY, 5);
	BInput(BUILDING_REFINERY, RES_CRUDEOIL, 5);
	BOutput(BUILDING_REFINERY, RES_WSFUEL, 5);
	BEmitter(BUILDING_REFINERY, 0, PARTICLE_EXHAUST, Vec3f(TILE_SIZE*5.7/10, TILE_SIZE*3/2, TILE_SIZE*-5/10));
	BEmitter(BUILDING_REFINERY, 1, PARTICLE_EXHAUST2, Vec3f(TILE_SIZE*5.7/10, TILE_SIZE*3/2, TILE_SIZE*-5/10));
	BEmitter(BUILDING_REFINERY, 2, PARTICLE_EXHAUST, Vec3f(TILE_SIZE*-4.5/10, TILE_SIZE*1.75, TILE_SIZE*3.0f/10));
	BEmitter(BUILDING_REFINERY, 3, PARTICLE_EXHAUST2, Vec3f(TILE_SIZE*-4.5/10, TILE_SIZE*1.75, TILE_SIZE*3.0f/10));

	DefB(BUILDING_NUCPOW, "Nuclear Powerplant", Vec2i(2,2), false, "models/nucpow2/nucpow2", Vec3f(1,1,1), Vec3f(0,0,0), "models/nucpow2/nucpow2", Vec3f(1,1,1), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BConMat(BUILDING_NUCPOW, RES_MINERALS, 5);
	BConMat(BUILDING_NUCPOW, RES_LABOUR, 10);
	BInput(BUILDING_NUCPOW, RES_URANIUM, 5);
	BOutput(BUILDING_NUCPOW, RES_ENERGY, 500);
	BEmitter(BUILDING_NUCPOW, 0, PARTICLE_EXHAUSTBIG, Vec3f(TILE_SIZE*-0.63f, TILE_SIZE*1.5f, TILE_SIZE*0));
	BEmitter(BUILDING_NUCPOW, 1, PARTICLE_EXHAUSTBIG, Vec3f(TILE_SIZE*0.17f, TILE_SIZE*1.5f, TILE_SIZE*-0.64f));

	DefB(BUILDING_FARM, "Farm", Vec2i(4,2), true, "models/farm2/farm2", Vec3f(1,1,1), Vec3f(0,0,0), "models/farm2/farm2", Vec3f(1,1,1), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BConMat(BUILDING_FARM, RES_MINERALS, 5);
	BConMat(BUILDING_FARM, RES_LABOUR, 10);
	BInput(BUILDING_FARM, RES_ENERGY, 5);
	BOutput(BUILDING_FARM, RES_FARMPRODUCTS, 5);

	DefB(BUILDING_STORE, "Store", Vec2i(2,1), true, "models/store1/hugterr.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), "models/store1/hugterr.ms3d", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BConMat(BUILDING_STORE, RES_MINERALS, 5);
	BConMat(BUILDING_STORE, RES_LABOUR, 10);
	BInput(BUILDING_STORE, RES_ENERGY, 5);
	BInput(BUILDING_STORE, RES_FARMPRODUCTS, 5);
	BOutput(BUILDING_STORE, RES_RETFOOD, 5);

	DefB(BUILDING_HOUR, "Harbour", Vec2i(2,2), false, "models/harbour2/harbour2", Vec3f(100,100,100), Vec3f(0,0,0), "models/harbour2/harbour2", Vec3f(100,100,100), Vec3f(0,0,0), FOUNDATION_COASTAL, RES_NONE);
	BConMat(BUILDING_HOUR, RES_MINERALS, 5);
	BConMat(BUILDING_HOUR, RES_LABOUR, 10);
	BInput(BUILDING_HOUR, RES_ENERGY, 5);

	DefB(BUILDING_OILWELL, "Oil Well", Vec2i(1,1), false, "models/oilwell2/oilwell2", Vec3f(1,1,1), Vec3f(0,0,0), "models/oilwell2/oilwell2", Vec3f(1,1,1), Vec3f(0,0,0), FOUNDATION_LAND, RES_CRUDEOIL);
	BConMat(BUILDING_OILWELL, RES_MINERALS, 5);
	BConMat(BUILDING_OILWELL, RES_LABOUR, 10);
	BInput(BUILDING_OILWELL, RES_ENERGY, 5);
	BOutput(BUILDING_OILWELL, RES_CRUDEOIL, 5);

	DefB(BUILDING_MINE, "Mine", Vec2i(1,1), false, "models/mine/nobottom.ms3d", Vec3f(1,1,1)/32.0f*TILE_SIZE, Vec3f(0,0,0), "models/mine/nobottom.ms3d", Vec3f(1,1,1)/32.0f*TILE_SIZE, Vec3f(0,0,0), FOUNDATION_LAND, -1);
	BConMat(BUILDING_MINE, RES_MINERALS, 5);
	BConMat(BUILDING_MINE, RES_LABOUR, 10);
	BInput(BUILDING_MINE, RES_ENERGY, 5);

	DefB(BUILDING_GASSTATION, "Gas Station", Vec2i(1,1), true, "models/gasstation2/gasstation2.ms3d", Vec3f(1,1,1)/80.0f*TILE_SIZE, Vec3f(0,0,0), "models/gasstation2/gasstation2.ms3d", Vec3f(1,1,1)/80.0f*TILE_SIZE, Vec3f(0,0,0), FOUNDATION_LAND, RES_NONE);
	BConMat(BUILDING_GASSTATION, RES_MINERALS, 5);
	BConMat(BUILDING_GASSTATION, RES_LABOUR, 10);
	BInput(BUILDING_GASSTATION, RES_ENERGY, 5);
	BOutput(BUILDING_GASSTATION, RES_RETFUEL, 5);

	Zero(g_roadcost);
	g_roadcost[RES_LABOUR] = 1;
	g_roadcost[RES_MINERALS] = 1;

	DefineRoad(CONNECTION_NOCONNECTION, CONSTRUCTION, "models/road/1_c.ms3d");
	DefineRoad(CONNECTION_NORTH, CONSTRUCTION, "models/road/n_c.ms3d");
	DefineRoad(CONNECTION_EAST, CONSTRUCTION, "models/road/e_c.ms3d");
	DefineRoad(CONNECTION_SOUTH, CONSTRUCTION, "models/road/s_c.ms3d");
	DefineRoad(CONNECTION_WEST, CONSTRUCTION, "models/road/w_c.ms3d");
	DefineRoad(CONNECTION_NORTHEAST, CONSTRUCTION, "models/road/ne_c.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/road/ns_c.ms3d");
	DefineRoad(CONNECTION_EASTSOUTH, CONSTRUCTION, "models/road/es_c.ms3d");
	DefineRoad(CONNECTION_NORTHWEST, CONSTRUCTION, "models/road/nw_c.ms3d");
	DefineRoad(CONNECTION_EASTWEST, CONSTRUCTION, "models/road/ew_c.ms3d");
	DefineRoad(CONNECTION_SOUTHWEST, CONSTRUCTION, "models/road/sw_c.ms3d");
	DefineRoad(CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/road/esw_c.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/road/nsw_c.ms3d");
	DefineRoad(CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/road/new_c.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/road/nes_c.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/road/nesw_c.ms3d");
	DefineRoad(CONNECTION_NOCONNECTION, FINISHED, "models/road/1.ms3d");
	//DefineRoad(CONNECTION_NOCONNECTION, FINISHED, "models/road/flat.ms3d");
	DefineRoad(CONNECTION_NORTH, FINISHED, "models/road/n.ms3d");
	DefineRoad(CONNECTION_EAST, FINISHED, "models/road/e.ms3d");
	DefineRoad(CONNECTION_SOUTH, FINISHED, "models/road/s.ms3d");
	DefineRoad(CONNECTION_WEST, FINISHED, "models/road/w.ms3d");
	DefineRoad(CONNECTION_NORTHEAST, FINISHED, "models/road/ne.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTH, FINISHED, "models/road/ns.ms3d");
	DefineRoad(CONNECTION_EASTSOUTH, FINISHED, "models/road/es.ms3d");
	DefineRoad(CONNECTION_NORTHWEST, FINISHED, "models/road/nw.ms3d");
	DefineRoad(CONNECTION_EASTWEST, FINISHED, "models/road/ew.ms3d");
	DefineRoad(CONNECTION_SOUTHWEST, FINISHED, "models/road/sw.ms3d");
	DefineRoad(CONNECTION_EASTSOUTHWEST, FINISHED, "models/road/esw.ms3d");
	DefineRoad(CONNECTION_NORTHSOUTHWEST, FINISHED, "models/road/nsw.ms3d");
	DefineRoad(CONNECTION_NORTHEASTWEST, FINISHED, "models/road/new.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTH, FINISHED, "models/road/nes.ms3d");
	DefineRoad(CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/road/nesw.ms3d");

	Zero(g_powlcost);
	g_powlcost[RES_LABOUR] = 1;
	g_powlcost[RES_MINERALS] = 1;

	DefinePowl(CONNECTION_NOCONNECTION, CONSTRUCTION, "models/powerline/1_c.ms3d");
	DefinePowl(CONNECTION_NORTH, CONSTRUCTION, "models/powerline/n_c.ms3d");
	DefinePowl(CONNECTION_EAST, CONSTRUCTION, "models/powerline/e_c.ms3d");
	DefinePowl(CONNECTION_SOUTH, CONSTRUCTION, "models/powerline/s_c.ms3d");
	DefinePowl(CONNECTION_WEST, CONSTRUCTION, "models/powerline/w_c.ms3d");
	DefinePowl(CONNECTION_NORTHEAST, CONSTRUCTION, "models/powerline/ne_c.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/powerline/ns_c.ms3d");
	DefinePowl(CONNECTION_EASTSOUTH, CONSTRUCTION, "models/powerline/es_c.ms3d");
	DefinePowl(CONNECTION_NORTHWEST, CONSTRUCTION, "models/powerline/nw_c.ms3d");
	DefinePowl(CONNECTION_EASTWEST, CONSTRUCTION, "models/powerline/ew_c.ms3d");
	DefinePowl(CONNECTION_SOUTHWEST, CONSTRUCTION, "models/powerline/sw_c.ms3d");
	DefinePowl(CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/powerline/esw_c.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/powerline/nsw_c.ms3d");
	DefinePowl(CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/powerline/new_c.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/powerline/nes_c.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/powerline/nesw_c.ms3d");
	DefinePowl(CONNECTION_NOCONNECTION, FINISHED, "models/powerline/1.ms3d");
	DefinePowl(CONNECTION_NORTH, FINISHED, "models/powerline/n.ms3d");
	DefinePowl(CONNECTION_EAST, FINISHED, "models/powerline/e.ms3d");
	DefinePowl(CONNECTION_SOUTH, FINISHED, "models/powerline/s.ms3d");
	DefinePowl(CONNECTION_WEST, FINISHED, "models/powerline/w.ms3d");
	DefinePowl(CONNECTION_NORTHEAST, FINISHED, "models/powerline/ne.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTH, FINISHED, "models/powerline/ns.ms3d");
	DefinePowl(CONNECTION_EASTSOUTH, FINISHED, "models/powerline/es.ms3d");
	DefinePowl(CONNECTION_NORTHWEST, FINISHED, "models/powerline/nw.ms3d");
	DefinePowl(CONNECTION_EASTWEST, FINISHED, "models/powerline/ew.ms3d");
	DefinePowl(CONNECTION_SOUTHWEST, FINISHED, "models/powerline/sw.ms3d");
	DefinePowl(CONNECTION_EASTSOUTHWEST, FINISHED, "models/powerline/esw.ms3d");
	DefinePowl(CONNECTION_NORTHSOUTHWEST, FINISHED, "models/powerline/nsw.ms3d");
	DefinePowl(CONNECTION_NORTHEASTWEST, FINISHED, "models/powerline/new.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTH, FINISHED, "models/powerline/nes.ms3d");
	DefinePowl(CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/powerline/nesw.ms3d");

	Zero(g_crpipecost);
	g_crpipecost[RES_LABOUR] = 1;
	g_crpipecost[RES_MINERALS] = 1;

	DefineCrPipe(CONNECTION_NOCONNECTION, CONSTRUCTION, "models/crpipeline/1_c.ms3d");
	DefineCrPipe(CONNECTION_NORTH, CONSTRUCTION, "models/crpipeline/n_c.ms3d");
	DefineCrPipe(CONNECTION_EAST, CONSTRUCTION, "models/crpipeline/e_c.ms3d");
	DefineCrPipe(CONNECTION_SOUTH, CONSTRUCTION, "models/crpipeline/s_c.ms3d");
	DefineCrPipe(CONNECTION_WEST, CONSTRUCTION, "models/crpipeline/w_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEAST, CONSTRUCTION, "models/crpipeline/ne_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTH, CONSTRUCTION, "models/crpipeline/ns_c.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTH, CONSTRUCTION, "models/crpipeline/es_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHWEST, CONSTRUCTION, "models/crpipeline/nw_c.ms3d");
	DefineCrPipe(CONNECTION_EASTWEST, CONSTRUCTION, "models/crpipeline/ew_c.ms3d");
	DefineCrPipe(CONNECTION_SOUTHWEST, CONSTRUCTION, "models/crpipeline/sw_c.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTHWEST, CONSTRUCTION, "models/crpipeline/esw_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTHWEST, CONSTRUCTION, "models/crpipeline/nsw_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTWEST, CONSTRUCTION, "models/crpipeline/new_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTH, CONSTRUCTION, "models/crpipeline/nes_c.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTHWEST, CONSTRUCTION, "models/crpipeline/nesw_c.ms3d");
	DefineCrPipe(CONNECTION_NOCONNECTION, FINISHED, "models/crpipeline/1.ms3d");
	DefineCrPipe(CONNECTION_NORTH, FINISHED, "models/crpipeline/n.ms3d");
	DefineCrPipe(CONNECTION_EAST, FINISHED, "models/crpipeline/e.ms3d");
	DefineCrPipe(CONNECTION_SOUTH, FINISHED, "models/crpipeline/s.ms3d");
	DefineCrPipe(CONNECTION_WEST, FINISHED, "models/crpipeline/w.ms3d");
	DefineCrPipe(CONNECTION_NORTHEAST, FINISHED, "models/crpipeline/ne.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTH, FINISHED, "models/crpipeline/ns.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTH, FINISHED, "models/crpipeline/es.ms3d");
	DefineCrPipe(CONNECTION_NORTHWEST, FINISHED, "models/crpipeline/nw.ms3d");
	DefineCrPipe(CONNECTION_EASTWEST, FINISHED, "models/crpipeline/ew.ms3d");
	DefineCrPipe(CONNECTION_SOUTHWEST, FINISHED, "models/crpipeline/sw.ms3d");
	DefineCrPipe(CONNECTION_EASTSOUTHWEST, FINISHED, "models/crpipeline/esw.ms3d");
	DefineCrPipe(CONNECTION_NORTHSOUTHWEST, FINISHED, "models/crpipeline/nsw.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTWEST, FINISHED, "models/crpipeline/new.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTH, FINISHED, "models/crpipeline/nes.ms3d");
	DefineCrPipe(CONNECTION_NORTHEASTSOUTHWEST, FINISHED, "models/crpipeline/nesw.ms3d");

	g_ordersnd.clear();

	g_ordersnd.push_back(Sound("sounds/aaa000/gogogo.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/moveout2.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/spreadout.wav"));
	g_ordersnd.push_back(Sound("sounds/aaa000/wereunderattack3.wav"));
	//g_zpainSnd.push_back(Sound("sounds/zpain.wav"));
}
