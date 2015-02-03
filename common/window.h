#ifndef WINDOW_H
#define WINDOW_H

#include "platform.h"
#include "math/3dmath.h"
#include "math/vec2i.h"

#ifndef MATCHMAKER
#include "sim/selection.h"
#include "math/camera.h"
#endif

#define INI_WIDTH			800
#define INI_HEIGHT			600
#define INI_BPP				32
#define DRAW_FRAME_RATE			30
//#define DRAW_FRAME_RATE			5
#define MIN_DISTANCE		1.0f
//#define MAX_DISTANCE		(100 * 1000 * 10.0f) //10km
//#define MAX_DISTANCE		(10 * 1000 * 10.0f) //10km
#define MAX_DISTANCE		(5.0f * 1000.0f * 10.0f) //10km
#define FIELD_OF_VIEW		45.0f
//#define PROJ_LEFT			(-16*4)
#define PROJ_RIGHT			600 //(500)	//(30*12.5f)
//#define PROJ_TOP			(16*4)
//#define PROJ_BOTTOM			(-16*4)
//#define INI_ZOOM			0.025f
//#define MIN_ZOOM		0.25f
#define MIN_ZOOM		0.15f
//#define MIN_ZOOM		0.025f
//#define MIN_ZOOM		0.005f
//#define MIN_ZOOM		0.05f
//#define MAX_ZOOM		0.7f
#define MAX_ZOOM		0.3f
//#define MAX_ZOOM		0.1f
#define INI_ZOOM			MIN_ZOOM
//#define INI_ZOOM			0.05f

extern double g_drawfrinterval;
extern bool g_quit;
extern bool g_active;
extern bool g_fullscreen;
#if 0
extern double g_currentTime;
extern double g_lastTime;
extern double g_framesPerSecond;
#endif
extern double g_instantdrawfps;
extern double g_instantupdfps;
extern double g_updfrinterval;

struct Resolution
{
	int width;
	int height;
};

extern Resolution g_selectedRes;
extern std::vector<Resolution> g_resolution;
extern std::vector<int> g_bpps;

#ifndef MATCHMAKER
extern Camera g_cam;
extern int g_currw;
extern int g_currh;
extern int g_width;
extern int g_height;
extern int g_bpp;
extern Vec2i g_mouse;
extern Vec2i g_mousestart;
extern bool g_keyintercepted;
extern bool g_keys[SDL_NUM_SCANCODES];
extern bool g_mousekeys[5];
extern float g_zoom;
extern bool g_mouseout;
extern bool g_moved;
extern bool g_canplace;
extern int g_bpcol;
extern int g_build;
extern Vec3f g_vdrag[2];
extern Camera g_bpcam;
extern int g_bptype;
extern float g_bpyaw;
extern Selection g_sel;
extern bool g_mouseoveraction;
extern int g_curst;	//cursor state
extern int g_kbfocus;	//keyboad focus counter

/*
Determines if the cursor is over an actionable widget, like a drop-down selector.
If it is, we don't want to scroll if the mouse is at the edge of the screen because
the user is trying to do something.
*/
//bool g_mouseoveraction = false;
#endif

void AddRes(int w, int h);
void CalcDrawRate();
bool DrawNextFrame(int desiredfps);
void CalcUpdRate();
bool UpdNextFrame(int desiredfps);
void EnumerateDisplay();
void Resize(int width, int height);
void DestroyWindow(const char* title);
bool MakeWindow(const char* title);

#endif
