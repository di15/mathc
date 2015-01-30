#include "../gmain.h"
#include "../../common/gui/gui.h"
#include "../keymap.h"
#include "../../common/render/heightmap.h"
#include "../../common/math/hmapmath.h"
#include "../../common/math/camera.h"
#include "../../common/render/shadow.h"
#include "../../common/render/screenshot.h"
#include "../../common/save/savemap.h"
#include "../../common/sim/unit.h"
#include "../../common/sim/utype.h"
#include "../../common/sim/building.h"
#include "../../common/sim/bltype.h"
#include "../../common/sim/selection.h"
#include "../../common/sim/road.h"
#include "../../common/sim/powl.h"
#include "../../common/sim/crpipe.h"
#include "../../common/render/water.h"
#include "../../common/sim/order.h"
#include "../../common/path/pathdebug.h"
#include "../../common/path/pathnode.h"
#include "../../common/sim/build.h"
#include "../../common/sim/player.h"
#include "../../common/gui/widgets/windoww.h"
#include "../../common/debug.h"
#include "../../common/script/console.h"
#include "../../common/window.h"
#include "../../common/net/lockstep.h"
#include "../../common/net/sendpackets.h"
#include "../../common/gui/widgets/spez/svlist.h"
#include "../../common/gui/widgets/spez/newhost.h"
#include "../../common/gui/widgets/spez/loadview.h"
#include "../../common/gui/widgets/spez/lobby.h"
#include "../../common/sim/simdef.h"

//not engine
#include "editorgui.h"
#include "playgui.h"
#include "ggui.h"

//bool g_canselect = true;

char g_lastsave[MAX_PATH+1];

void Resize_Fullscreen(Widget* thisw)
{
	Player* py = &g_player[g_localP];

	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = (float)g_width-1;
	thisw->m_pos[3] = (float)g_height-1;
}

void Click_LoadMapButton()
{
#ifdef PLATFORM_WIN
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("map projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("map projects/map project", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize     = sizeof ( ofn );
	ofn.hwndOwner       = NULL;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile       = filepath;
	//ofn.lpstrFile[0]  = '\0';
	ofn.nMaxFile        = sizeof( filepath );
	//ofn.lpstrFilter   = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter     = "All\0*.*\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFileTitle  = NULL;
	ofn.nMaxFileTitle   = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags           = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	FreeMap();

	if(LoadMap(filepath))
		strcpy(g_lastsave, filepath);
#endif //PLATFORM_WIN
}

void Click_SaveMapButton()
{
#ifdef PLATFORM_WIN
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps/map", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	SaveMap(filepath);
	strcpy(g_lastsave, filepath);
#endif //PLATFORM_WIN
}

void Click_QSaveMapButton()
{
	if(g_lastsave[0] == '\0')
	{
		Click_SaveMapButton();
		return;
	}

	SaveMap(g_lastsave);
}

void Resize_MenuItem(Widget* thisw)
{
	int row;
	sscanf(thisw->m_name.c_str(), "%d", &row);
	Font* f = &g_font[thisw->m_font];

	Player* py = &g_player[g_localP];
	
	//thisw->m_pos[0] = g_width/2 - f->gheight*4;
	thisw->m_pos[0] = f->gheight*4;
	thisw->m_pos[1] = g_height/2 - f->gheight*4 + f->gheight*1.5f*row;
	thisw->m_pos[2] = thisw->m_pos[0] + f->gheight * 6;
	thisw->m_pos[3] = thisw->m_pos[1] + f->gheight;
}

void Resize_LoadingStatus(Widget* thisw)
{
	Player* py = &g_player[g_localP];

	thisw->m_pos[0] = g_width/2 - 64;
	thisw->m_pos[1] = g_height/2;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
	thisw->m_tpos[0] = g_width/2;
	thisw->m_tpos[1] = g_height/2;
}

void Resize_WinText(Widget* thisw)
{
	Widget* parent = thisw->m_parent;

	thisw->m_pos[0] = parent->m_pos[0];
	thisw->m_pos[1] = parent->m_pos[1];
	thisw->m_pos[2] = thisw->m_pos[0] + 400;
	thisw->m_pos[3] = thisw->m_pos[1] + 2000;
}

void Click_HostGame()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->open("new host");
}

void Click_ListHosts()
{
	//g_mode = APPMODE_PLAY;

	//g_netmode = NETM_CLIENT;
	//g_needsvlist = true;
	//g_reqdsvlist = false;

	//Connect("localhost", PORT, false, true, false, false);
	//Connect(SV_ADDR, PORT, true, false, false, false);
	
	//BegSess();

	//g_canturn = true;	//temp, assuming no clients were handshook to server when netfr0 turn happened and server didn't have any cl's to send NetTurnPacket to

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->open("sv list");
}

void Click_EndGame()
{
	EndSess();
}

void Click_NewGame()
{
	CheckGLError(__FILE__, __LINE__);
#if 0
	//LoadJPGMap("heightmaps/heightmap0e2s.jpg");
	//LoadJPGMap("heightmaps/heightmap0e2.jpg");
	//LoadJPGMap("heightmaps/heightmap0e.jpg");
	LoadJPGMap("heightmaps/heightmap0e4.jpg");
	//LoadJPGMap("heightmaps/water.jpg");
#elif 1
	//LoadJPGMap("heightmaps/heightmap0c.jpg");
	LoadJPGMap("heightmaps/heightmap0d.jpg");
#else
	char fullpath[MAX_PATH+1];
	FullPath("maps/testmap", fullpath);
	//FullPath("maps/bigmap256", fullpath);
	LoadMap(fullpath);
#endif

	//return;

	CheckGLError(__FILE__, __LINE__);



	for(int i=0; i<10; i++)
		//for(int j=0; j<10; j++)
	//for(int i=0; i<15; i++)
	//	for(int j=0; j<1; j++)
	//for(int i=0; i<1; i++)
		for(int j=0; j<1; j++)
		{

			//Vec3i cmpos((g_hmap.m_widthx+4)*TILE_SIZE/2 + (i+2)*PATHNODE_SIZE, 0, g_hmap.m_widthy*TILE_SIZE/2 + (j+2)*PATHNODE_SIZE);
			//cmpos.y = g_hmap.accheight(cmpos.x, cmpos.z);
			Vec2i cmpos((g_hmap.m_widthx+4)*TILE_SIZE/2 + (i+2)*PATHNODE_SIZE*4, g_hmap.m_widthy*TILE_SIZE/2 + (j+2)*PATHNODE_SIZE*4);

			//if(rand()%2 == 1)
			//	PlaceUnit(UNIT_BATTLECOMP, cmpos, 0);
			//else
			PlaceUnit(UNIT_LABOURER, cmpos, 0, NULL);
			//PlaceUnit(UNIT_TRUCK, cmpos, rand()%PLAYERS, NULL);

			ClearFol(cmpos.x - TILE_SIZE, cmpos.y - TILE_SIZE, cmpos.x + TILE_SIZE, cmpos.y + TILE_SIZE);
		}

	//return;

	CheckGLError(__FILE__, __LINE__);

	for(int i=0; i<PLAYERS; i++)
	{
		Player* p = &g_player[i];

		p->on = true;
		p->ai = (i == g_localP) ? false : true;

		p->global[RES_DOLLARS] = 40 * 1000 * 1000;
		p->global[RES_FARMPRODUCTS] = 4000;
		p->global[RES_RETFOOD] = 4000;
		p->global[RES_CEMENT] = 150;
		p->global[RES_STONE] = 4000;
		p->global[RES_FUEL] = 4000;
		p->global[RES_URANIUM] = 4000;
		p->global[RES_ELECTRONICS] = 4000;
		p->global[RES_METAL] = 150;
		p->global[RES_PRODUCTION] = 40;

#if 0
#define RES_DOLLARS			0
#define RES_LABOUR			1
#define RES_HOUSING			2
#define RES_FARMPRODUCTS	3
#define RES_RETFOOD			4
#define RES_PRODUCTION		5
#define RES_CEMENT		6
#define RES_CRUDEOIL		7
#define RES_FUEL			8
#define RES_FUEL			9
#define RES_ENERGY			10
#define RES_URANIUM			11
#define RESOURCES			12
#endif
	}

#if 0
	PlaceBl(BL_HARBOUR, Vec2i(g_hmap.m_widthx/2-1, g_hmap.m_widthy/2-3), true, 0);
	PlaceBl(BL_HOUSE, Vec2i(g_hmap.m_widthx/2+2, g_hmap.m_widthy/2-2), true, 0);
	PlaceBl(BL_HOUSE, Vec2i(g_hmap.m_widthx/2+4, g_hmap.m_widthy/2-3), true, 0);
	PlaceBl(BL_HOUSE, Vec2i(g_hmap.m_widthx/2+6, g_hmap.m_widthy/2-3), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+1, g_hmap.m_widthy/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+2, g_hmap.m_widthy/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+3, g_hmap.m_widthy/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+3, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+4, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+5, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+6, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthy/2-3, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthy/2-4, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthy/2-5, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthy/2-6, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+7, g_hmap.m_widthy/2-7, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+8, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+9, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthy/2-2, 1, false);
	PlaceBl(BL_FACTORY, Vec2i(g_hmap.m_widthx/2+9, g_hmap.m_widthy/2-3), true, 0);
	PlaceBl(BL_REFINERY, Vec2i(g_hmap.m_widthx/2+11, g_hmap.m_widthy/2-3), true, 0);
	PlaceBl(BL_NUCPOW, Vec2i(g_hmap.m_widthx/2+13, g_hmap.m_widthy/2-3), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+11, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+12, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthy/2-2, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthy/2-3, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthy/2-4, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthy/2-5, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthy/2-6, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+14, g_hmap.m_widthy/2-7, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+15, g_hmap.m_widthy/2-2, 1, false);
	PlaceBl(BL_FARM, Vec2i(g_hmap.m_widthx/2+6, g_hmap.m_widthy/2-0), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthy/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthy/2-0, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+10, g_hmap.m_widthy/2+1, 1, false);
	PlaceBl(BL_STORE, Vec2i(g_hmap.m_widthx/2+9, g_hmap.m_widthy/2-1), true, 0);
	PlaceBl(BL_OILWELL, Vec2i(g_hmap.m_widthx/2+9, g_hmap.m_widthy/2-0), true, 0);
	PlaceBl(BL_MINE, Vec2i(g_hmap.m_widthx/2+11, g_hmap.m_widthy/2-0), true, 0);
	PlaceBl(BL_MINE, Vec2i(g_hmap.m_widthx/2+12, g_hmap.m_widthy/2-0), true, 0);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthy/2-1, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthy/2-0, 1, false);
	PlaceRoad(g_hmap.m_widthx/2+13, g_hmap.m_widthy/2+1, 1, false);
	CheckGLError(__FILE__, __LINE__);
	PlaceBl(BL_GASSTATION, Vec2i(g_hmap.m_widthx/2+14, g_hmap.m_widthy/2-1), true, 0);
	g_hmap.genvbo();
#endif

	CheckGLError(__FILE__, __LINE__);

	g_mode = APPMODE_PLAY;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	//return;

	gui->closeall();
	gui->open("play");
	gui->open("play right opener");

	CheckGLError(__FILE__, __LINE__);

#if 0
	gui->add(new WindowW(gui, "window", Resize_Window));
	//gui->open("window");

	WindowW* win = (WindowW*)gui->get("window");

	RichText bigtext;

	for(int i=0; i<2000; i++)
	{
		bigtext.m_part.push_back(RichPart((unsigned int)(rand()%3000+16000)));

		if(rand()%10 == 1)
			bigtext.m_part.push_back(RichPart((unsigned int)'\n'));
	}

	win->add(new TextBlock(win, "text block", bigtext, MAINFONT16, Resize_WinText));
#endif

	g_lastsave[0] = '\0';
	BegSess();
	g_netmode = NETM_SINGLE;
}

void Click_OpenEditor()
{
	CheckGLError(__FILE__, __LINE__);
	LoadJPGMap("heightmaps/heightmap0e2.jpg");
	CheckGLError(__FILE__, __LINE__);

	g_mode = APPMODE_EDITOR;


	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	gui->closeall();
	gui->open("editor gui");

	g_lastsave[0] = '\0';
}

void Resize_CenterWin(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	
	thisw->m_pos[0] = g_width/2 - 150;
	thisw->m_pos[1] = g_height/2 - 150;
	thisw->m_pos[2] = g_width/2 + 200;
	thisw->m_pos[3] = g_height/2 + 150;
}

void Resize_CenterWin2(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	
	thisw->m_pos[0] = g_width/2 - 150 + 60;
	thisw->m_pos[1] = g_height/2 - 150 + 30;
	thisw->m_pos[2] = g_width/2 + 200 + 60;
	thisw->m_pos[3] = g_height/2 + 150 + 30;
}

void Click_LoadGame()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	
	gui->open("load");
	((LoadView*)gui->get("load"))->regen();
}

void Click_Options()
{
	GUI* gui = &g_gui;
	gui->closeall();
	gui->open("options");
}

void Click_Quit()
{
	EndSess();
	FreeMap();
	FreeGrid();
	g_quit = true;
}

void Resize_JoinCancel(Widget* thisw)
{
	thisw->m_pos[0] = g_width/2 - 75;
	thisw->m_pos[1] = g_height/2 + 100 - 30;
	thisw->m_pos[2] = g_width/2 + 75;
	thisw->m_pos[3] = g_height/2 + 100 - 0;
	CenterLabel(thisw);
}
	
void Click_JoinCancel()
{

}

void Click_SetOptions()
{
	GUI* gui = &g_gui;
	ViewLayer* opview = (ViewLayer*)gui->get("options");
	DropList* winmodes = (DropList*)opview->get("0");
	DropList* reslist = (DropList*)opview->get("1");
	bool changed = false;

	int w;
	int h;
	bool fs;
	std::string resname = reslist->m_options[ reslist->m_selected ].rawstr();
	fs = winmodes->m_selected == 1 ? true : false;
	sscanf(resname.c_str(), "%dx%d", &w, &h);

	if(w != g_selectedRes.width)
		changed = true;

	if(h != g_selectedRes.height)
		changed = true;

	if(fs != g_fullscreen)
		changed = true;

	gui->closeall();
	gui->open("main");

	if(!changed)
		return;
	
	g_selectedRes.width = w;
	g_selectedRes.height = h;
	g_width = w;
	g_height = h;
	g_fullscreen = fs;

	WriteConfig();
	g_mode = APPMODE_RELOADING;
}

void FillMenu()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	// Main ViewLayer
	gui->add(new ViewLayer(gui, "main"));
	ViewLayer* mainview = (ViewLayer*)gui->get("main");
	mainview->add(new Image(mainview, "gui/mmbg.jpg", true, Resize_Fullscreen));
#if 0
	mainview->add(new Link(mainview, "0", RichText("New Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_NewGame));
	mainview->add(new Link(mainview, "1", RichText("Load Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_LoadGame));
	mainview->add(new Link(mainview, "2", RichText("Host Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_HostGame));
	mainview->add(new Link(mainview, "3", RichText("Join Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_ListHosts));
	mainview->add(new Link(mainview, "4", RichText("Options"), FONT_EUROSTILE16, Resize_MenuItem, Click_Options));
	mainview->add(new Link(mainview, "5", RichText("Quit"), FONT_EUROSTILE16, Resize_MenuItem, Click_Quit));
#else
	mainview->add(new Link(mainview, "0", RichText("New Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_NewGame));
	mainview->add(new Link(mainview, "1", RichText("Load Game"), FONT_EUROSTILE16, Resize_MenuItem, Click_LoadGame));
	mainview->add(new Link(mainview, "2", RichText("Options"), FONT_EUROSTILE16, Resize_MenuItem, Click_Options));
	mainview->add(new Link(mainview, "3", RichText("Quit"), FONT_EUROSTILE16, Resize_MenuItem, Click_Quit));
#endif

	//Options ViewLayer
	gui->add(new ViewLayer(gui, "options"));
	ViewLayer* opview = (ViewLayer*)gui->get("options");
	opview->add(new Image(opview, "gui/mmbg.jpg", true, Resize_Fullscreen));
	opview->add(new DropList(opview, "0", MAINFONT16, Resize_MenuItem, NULL));
	opview->add(new DropList(opview, "1", MAINFONT16, Resize_MenuItem, NULL));
	opview->add(new Link(opview, "2", RichText("Done"), FONT_EUROSTILE16, Resize_MenuItem, Click_SetOptions));
	DropList* winmodes = (DropList*)opview->get("0");
	winmodes->m_options.push_back(RichText("Windowed"));
	winmodes->m_options.push_back(RichText("Fullscreen"));
	winmodes->m_selected = g_fullscreen ? 1 : 0;
	DropList* reslist = (DropList*)opview->get("1");
	g_resolution.clear();
	int uniquei = 0;
	for(int i=0; i<SDL_GetNumDisplayModes(0); i++)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, i, &mode);

		bool found = false;

		for(auto rit=g_resolution.begin(); rit!=g_resolution.end(); rit++)
		{
			if(rit->width == mode.w &&
				rit->height == mode.h)
			{
				found = true;
				break;
			}
		}

		if(found)
			continue;

		Resolution res;
		res.width = mode.w;
		res.height = mode.h;
		g_resolution.push_back(res);

		if(mode.w == g_selectedRes.width &&
			mode.h == g_selectedRes.height)
			reslist->m_selected = uniquei;

		char resname[32];
		sprintf(resname, "%dx%d", mode.w, mode.h);
		reslist->m_options.push_back(RichText(resname));

		uniquei++;
	}


	gui->add(new SvList(gui, "sv list", Resize_CenterWin));
	gui->add(new NewHost(gui, "new host", Resize_CenterWin2));
	gui->add(new Lobby(gui, "lobby"));
	
	gui->add(new ViewLayer(gui, "join"));
	ViewLayer* joinview = (ViewLayer*)gui->get("join");
	joinview->add(new Image(joinview, "gui/mmbg.jpg", true, Resize_Fullscreen));
	joinview->add(new Text(joinview, "status", RichText("Joining..."), MAINFONT16, Resize_LoadingStatus));
	joinview->add(new Button(joinview, "cancel", "gui/transp.png", RichText("Cancel"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_JoinCancel, Click_JoinCancel, NULL, NULL, NULL, NULL, -1));
}

void StartRoadPlacement()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[0] = Vec3f(-1,-1,-1);
	g_vdrag[1] = Vec3f(-1,-1,-1);

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[0] = intersection;
	g_vdrag[1] = intersection;
}

void MouseLDown()
{
	if(g_mode == APPMODE_EDITOR)
	{
		int edtool = GetEdTool();

		if(edtool == EDTOOL_PLACEROADS ||
		                edtool == EDTOOL_PLACECRUDEPIPES ||
		                edtool == EDTOOL_PLACEPOWERLINES)
		{
			StartRoadPlacement();
		}
	}
	else if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_localP];
		g_mousestart = g_mouse;
	}
}

void EdPlaceUnit()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	int type = GetPlaceUnitType();

	int country = GetPlaceUnitCountry();
	int company = GetPlaceUnitCompany();

	//PlaceUnit(type, Vec3i(intersection.x, intersection.y, intersection.z), country, company, -1);
	PlaceUnit(type, Vec2i(intersection.x, intersection.z), country, NULL);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdPlaceBuilding()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	if(intersection.y < WATER_LEVEL && !GetMapIntersection2(&g_hmap, line, &intersection))
		return;


	int type = GetPlaceBuildingType();

	Vec2i tilepos (intersection.x/TILE_SIZE, intersection.z/TILE_SIZE);

	if(CheckCanPlace(type, tilepos))
		PlaceBl(type, tilepos, true, -1, NULL);
#if 0
	g_hmap.setheight( intersection.x / TILE_SIZE, intersection.z / TILE_SIZE, 0);
	g_hmap.remesh();
#endif
}

void EdDeleteObject()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	Selection sel = DoSel(c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos);

	for(auto unititer = sel.units.begin(); unititer != sel.units.end(); unititer++)
	{
		g_unit[ *unititer ].destroy();
		return;
	}
}

void MouseLUp()
{
	if(g_mode == APPMODE_EDITOR)
	{
		int edtool = GetEdTool();

		if(edtool == EDTOOL_PLACEUNITS)
			EdPlaceUnit();
		else if(edtool == EDTOOL_PLACEBUILDINGS)
			EdPlaceBuilding();
		else if(edtool == EDTOOL_DELETEOBJECTS)
			EdDeleteObject();
		else if(edtool == EDTOOL_PLACEROADS)
			PlaceCo(CONDUIT_ROAD);
		else if(edtool == EDTOOL_PLACECRUDEPIPES)
			PlaceCo(CONDUIT_CRPIPE);
		else if(edtool == EDTOOL_PLACEPOWERLINES)
			PlaceCo(CONDUIT_POWL);
	}
	else if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_localP];
		Camera* c = &g_cam;
		GUI* gui = &g_gui;

		if(g_build == BL_NONE)
		{
			gui->close("cstr view");
			gui->close("bl view");
			gui->close("truck mgr");
			gui->close("save");
			gui->close("load");
			g_sel = DoSel(c->zoompos(), c->m_strafe, c->up2(), Normalize(c->m_view - c->zoompos()));
			AfterSel(&g_sel);
		}
		else if(g_build < BL_TYPES)
		{
			if(g_canplace)
			{
				//int bid;
				//PlaceBl(g_build, Vec2i(g_vdrag[0].x/TILE_SIZE, g_vdrag[0].z/TILE_SIZE), false, g_localP, &bid);

				//InfoMess("pl", "pl");

				//if(g_netmode != NETM_SINGLE)
				{
					PlaceBlPacket pbp;
					pbp.header.type = PACKET_PLACEBL;
					pbp.player = g_localP;
					pbp.btype = g_build;
					pbp.tpos = Vec2i(g_vdrag[0].x/TILE_SIZE, g_vdrag[0].z/TILE_SIZE);
					
					LockCmd((PacketHeader*)&pbp, sizeof(PlaceBlPacket));
				}
			}
			else
			{
			}

			g_build = -1;
		}
		else if(g_build >= BL_TYPES && g_build < BL_TYPES+CONDUIT_TYPES)
		{
			//g_log<<"place r"<<std::endl;
			PlaceCo(g_build - BL_TYPES);
			g_build = -1;
		}
	}
}

void RotateAbout()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	float dx = (float)( g_mouse.x - g_width/2 );
	float dy = (float)( g_mouse.y - g_height/2 );

	Camera oldcam = g_cam;
	Vec3f line[2];
	line[0] = c->zoompos();

	//c->rotateabout(c->m_view, dy / 100.0f, c->m_strafe.x, c->m_strafe.y, c->m_strafe.z);
	c->rotateabout(c->m_view, dx / 100.0f, c->m_up.x, c->m_up.y, c->m_up.z);

	line[1] = c->zoompos();

	Vec3f ray = Normalize(line[1] - line[0]) * TILE_SIZE;

	//line[0] = line[0] - ray;
	line[1] = line[1] + ray;

	Vec3f clip;

#if 0
	if(GetMapIntersection(&g_hmap, line, &clip))
#else
	if(FastMapIntersect(&g_hmap, line, &clip))
#endif
		g_cam = oldcam;
	//else
	//	CalcMapView();

}

void UpdateRoadPlans()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_ROAD, 0, g_vdrag[0], g_vdrag[1]);
}

void UpdateCrPipePlans()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_CRPIPE, 0, g_vdrag[0], g_vdrag[1]);
}

void UpdatePowlPlans()
{
	Player* py = &g_player[g_localP];
	Camera* c = &g_cam;

	g_vdrag[1] = g_vdrag[0];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_strafe, c->up2(), c->m_view - c->m_pos, FIELD_OF_VIEW);

	Vec3f intersection;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;

#if 1
	if(!FastMapIntersect(&g_hmap, line, &intersection))
#else
	if(!GetMapIntersection(&g_hmap, line, &intersection))
#endif
		return;

	g_vdrag[1] = intersection;

	UpdCoPlans(CONDUIT_POWL, 0, g_vdrag[0], g_vdrag[1]);
}

void MouseMove()
{
	if(g_mode == APPMODE_PLAY || g_mode == APPMODE_EDITOR)
	{
		Player* py = &g_player[g_localP];

		if(g_mousekeys[MOUSE_MIDDLE])
		{
			RotateAbout();
			CenterMouse();
		}

		UpdSBl();

		if(g_mousekeys[MOUSE_LEFT])
		{
			int edtool = GetEdTool();

			if(edtool == EDTOOL_PLACEROADS)
			{
				UpdateRoadPlans();
			}
			else if(edtool == EDTOOL_PLACECRUDEPIPES)
			{
				UpdateCrPipePlans();
			}
			else if(edtool == EDTOOL_PLACEPOWERLINES)
			{
				UpdatePowlPlans();
			}
		}
	}
}

void MouseRDown()
{
}

void MouseRUp()
{
	if(g_mode == APPMODE_PLAY)
	{
		Player* py = &g_player[g_localP];
		Camera* c = &g_cam;

		//if(!g_keyintercepted)
		{
			Order(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->m_view, Normalize(c->m_view - c->zoompos()), c->m_strafe, c->up2());
		}
	}
}

void FillGUI()
{
	//DrawSceneFunc = DrawScene;
	//DrawSceneDepthFunc = DrawSceneDepth;

	g_log<<"2.1"<<std::endl;
	g_log.flush();

	for(int i=0; i<PLAYERS; i++)
	{
		g_log<<"2.1.1"<<std::endl;
		g_log.flush();

		Player* py = &g_player[i];
		GUI* gui = &g_gui;

		g_log<<"2.1.2"<<std::endl;
		g_log.flush();

		gui->assignkey(SDL_SCANCODE_F1, SaveScreenshot, NULL);
		gui->assignkey(SDL_SCANCODE_F2, LogPathDebug, NULL);
		gui->assignlbutton(MouseLDown, MouseLUp);
		gui->assignrbutton(MouseRDown, MouseRUp);
		gui->assignmousemove(MouseMove);

		g_log<<"2.1.3"<<std::endl;
		g_log.flush();
	}

	g_log<<"2.2"<<std::endl;
	g_log.flush();

	// Loading ViewLayer
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	g_log<<"2.2"<<std::endl;
	g_log.flush();

	gui->add(new ViewLayer(gui, "loading"));
	ViewLayer* loadview = (ViewLayer*)gui->get("loading");

	g_log<<"2.3"<<std::endl;
	g_log.flush();

	loadview->add(new Image(loadview, "gui/mmbg.jpg", true, Resize_Fullscreen));
	loadview->add(new Text(NULL, "status", RichText("Loading..."), MAINFONT8, Resize_LoadingStatus));

	gui->closeall();
	gui->open("loading");

	g_log<<"2.4"<<std::endl;
	g_log.flush();

	FillMenu();

	g_log<<"2.5"<<std::endl;
	g_log.flush();

	FillEd();

	g_log<<"2.6"<<std::endl;
	g_log.flush();

	FillPlay();

	g_log<<"2.7"<<std::endl;
	g_log.flush();

	FillConsole();
}
