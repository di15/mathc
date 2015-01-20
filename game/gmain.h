#ifndef GMAIN_H
#define GMAIN_H

#include "../common/window.h"

#define VERSION				3
#define CONFIGFILE			"config.ini"
#define TITLE				"Economy Planner"
#define SERVER_ADDR			"corpstates.com"

//#define TRUCK_DEBUG
//#define TRUCK_DEBUG2
//#define PATH_DEBUG

//#define LOCAL_TRANSX	//define this if you want only player-specific transaction hovering notifications to appear

class Heightmap;

#define APPMODE_LOGO		0
#define APPMODE_INTRO		1
#define APPMODE_LOADING		2
#define APPMODE_RELOADING	3
#define APPMODE_MENU		4
#define APPMODE_PLAY		5
#define APPMODE_PAUSE		6
#define APPMODE_EDITOR		7
#define APPMODE_JOINING		8
extern int g_mode;
extern int g_reStage;

void WriteConfig();
void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float lightpos[3], float lightdir[3]);
void DrawSceneDepth();

#endif
