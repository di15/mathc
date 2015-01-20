#include "platform.h"
#include "debug.h"
#include "utils.h"
//#include "main.h"
//#include "gui.h"
//#include "unit.h"
//#include "pathfinding.h"
//#include "collision.h"
//#include "building.h"
#include "gui/gui.h"
#include "gui/widget.h"
#include "gui/widgets/spez/cstrview.h"
#include "sim/player.h"
#include "window.h"
#include "../game/gmain.h"

Timer g_profile[TIMERS];
 bool g_debuglines = false;
std::ofstream g_profF;

void StartTimer(int id)
{
	//return;

	//if(g_mode != APPMODE_PLAY)
	//	return;

	g_profile[id].starttick = GetTickCount();
}

void StopTimer(int id)
{
	//return;

	if(g_mode != APPMODE_PLAY)
		return;

#ifdef DEBUG
	if(id == TIMER_UPDATE)
	{
		g_log<<std::endl<<"upd el = "<<GetTickCount()<<" - "<<g_profile[id].starttick<<std::endl<<std::endl;
	}
#endif

	unsigned long long elapsed = GetTickCount64() - g_profile[id].starttick;
	g_profile[id].starttick = GetTickCount();
	g_profile[id].lastframeelapsed += elapsed;

#ifdef DEBUG
	if(id == TIMER_UPDATE)
	{
		g_log<<std::endl<<"upd el"<<elapsed<<" tot"<<g_profile[id].lastframeelapsed<<" avg"<<g_profile[id].averagems<<std::endl<<std::endl;
	}
#endif

	if(id == TIMER_FRAME || g_profile[id].lastframe < g_profile[TIMER_FRAME].lastframe)
	{

#ifdef DEBUG
		if(id == TIMER_UPDATE)
		{
			g_log<<std::endl<<"upd ( (double)"<<g_profile[id].lastframeelapsed<<" + "<<g_profile[id].averagems<<"*(double)"<<g_profile[id].frames<<" ) / (double)("<<g_profile[id].frames<<"+1); = ";
		}
#endif

		//g_profile[id].averagems = ( g_profile[id].lastframeaverage + g_profile[id].averagems*g_profile[id].frames ) / (g_profile[id].frames+1);
		g_profile[id].averagems = ( (double)g_profile[id].lastframeelapsed + g_profile[id].averagems*(double)g_profile[id].frames ) / (double)(g_profile[id].frames+1);
		g_profile[id].frames++;
		//g_profile[id].timescountedperframe = 0;
		g_profile[id].lastframeelapsed = 0;
		g_profile[id].lastframe = g_profile[TIMER_FRAME].lastframe;


#ifdef DEBUG
		if(id == TIMER_UPDATE)
		{
			g_log<<g_profile[id].averagems<<std::endl<<std::endl;
		}
#endif

		//g_log<<g_profile[id].name<<" "<<g_profile[id].averagems<<"ms"<<std::endl;
	}
	if(id == TIMER_FRAME)
		g_profile[id].lastframe++;

	//g_profile[id].lastframeaverage = ( elapsed + g_profile[id].lastframeaverage*g_profile[id].timescountedperframe ) / (g_profile[id].timescountedperframe+1);
	//g_profile[id].timescountedperframe+=1.0f;
}

void WriteProfiles(int in, int layer)
{
	double parentavgms;

	if(in == -1)
	{
		char fullpath[MAX_PATH+1];
		FullPath("profiles.txt", fullpath);
		g_profF.open(fullpath, std::ios_base::out);
		parentavgms = g_profile[TIMER_FRAME].averagems;
	}
	else
	{
		parentavgms = g_profile[in].averagems;
	}

	double ofparentpct;
	double totalms = 0;
	double totalofparentpct = 0;
	double percentage;
	int subprofiles = 0;

	for(int j=0; j<TIMERS; j++)
	{
		if(g_profile[j].inside != in)
			continue;

		totalms += g_profile[j].averagems;
	}

	for(int j=0; j<TIMERS; j++)
	{
		if(g_profile[j].inside != in)
			continue;

		percentage = 100.0 * g_profile[j].averagems / totalms;
		ofparentpct = 100.0 * g_profile[j].averagems / parentavgms;
		totalofparentpct += ofparentpct;
		subprofiles++;

		for(int k=0; k<layer; k++)
			g_profF<<"\t";

		g_profF<<g_profile[j].name<<"\t...\t"<<g_profile[j].averagems<<"ms per frame, "<<percentage<<"% of this level's total"<<std::endl;

		WriteProfiles(j, layer+1);
	}

	if(subprofiles > 0)
	{
		for(int k=0; k<layer; k++)
			g_profF<<"\t";

		g_profF<<"level total sum: "<<totalms<<" ms per frame, that means "<<totalofparentpct<<"% of this parent's duration underwent profiling"<<std::endl;
	}

	if(in == -1)
	{
		g_profF.flush();
		g_profF.close();
	}
}

void DefTimer(int id, int inside, const char* name)
{
	g_profile[id].inside = inside;
	strcpy(g_profile[id].name, name);
}

void InitProfiles()
{
	DefTimer(TIMER_FRAME, -1, "Frame");
	DefTimer(TIMER_EVENT, TIMER_FRAME, "EventProc");
	DefTimer(TIMER_DRAW, TIMER_FRAME, "Draw();");
	DefTimer(TIMER_DRAWSCENEDEPTH, TIMER_DRAW, "DrawSceneDepth();");
	DefTimer(TIMER_DRAWSETUP, TIMER_DRAW, "Draw(); setup");
	DefTimer(TIMER_DRAWGUI, TIMER_DRAW, "DrawGUI();");
	DefTimer(TIMER_DRAWMINIMAP, TIMER_DRAW, "DrawMinimap();");
	DefTimer(TIMER_UPDATE, TIMER_FRAME, "Update();");
	DefTimer(TIMER_RESETPATHNODES, TIMER_UPDATE, "ResetPathNodes();");
	DefTimer(TIMER_MANAGETRIPS, TIMER_UPDATE, "ManageTrips();");
	DefTimer(TIMER_UPDATEUNITS, TIMER_UPDATE, "UpdUnits();");
	DefTimer(TIMER_UPDUONCHECK, TIMER_UPDATEUNITS, "Upd U On Ch");
	DefTimer(TIMER_UPDUNITAI, TIMER_UPDATEUNITS, "Upd Unit AI");
	DefTimer(TIMER_UPDLAB, TIMER_UPDUNITAI, "UpdLab();");
	DefTimer(TIMER_UPDTRUCK, TIMER_UPDUNITAI, "UpdTruck();");
	DefTimer(TIMER_FINDJOB, TIMER_UPDLAB, "FindJob();");
	DefTimer(TIMER_JOBLIST, TIMER_FINDJOB, "Job list collection");
	DefTimer(TIMER_JOBSORT, TIMER_FINDJOB, "Job list sort");
	DefTimer(TIMER_JOBPATH, TIMER_FINDJOB, "Job prepathing");
	DefTimer(TIMER_MOVEUNIT, TIMER_UPDATEUNITS, "Move Unit");
	DefTimer(TIMER_ANIMUNIT, TIMER_UPDATEUNITS, "Anim Unit");
	DefTimer(TIMER_UPDATEBUILDINGS, TIMER_UPDATE, "UpdBls();");
	DefTimer(TIMER_DRAWBL, TIMER_DRAW, "DrawBuildings();");
	DefTimer(TIMER_DRAWUNITS, TIMER_DRAW, "DrawUnits();");
	DefTimer(TIMER_DRAWRIM, TIMER_DRAW, "DrawRim();");
	DefTimer(TIMER_DRAWWATER, TIMER_DRAW, "DrawWater();");
	DefTimer(TIMER_DRAWCRPIPES, TIMER_DRAW, "DrawCrPipes();");
	DefTimer(TIMER_DRAWPOWLS, TIMER_DRAW, "DrawPowls();");
	DefTimer(TIMER_DRAWFOLIAGE, TIMER_DRAW, "DrawFol();");
	DefTimer(TIMER_SORTPARTICLES, TIMER_DRAW, "SortParticles();");
	DefTimer(TIMER_DRAWPARTICLES, TIMER_DRAW, "DrawParticles();");
	DefTimer(TIMER_DRAWMAP, TIMER_DRAW, "DrawMap();");
	//DefTimer(SHADOWS, TIMER_DRAW, "Shadows");
	DefTimer(TIMER_DRAWSKY, TIMER_DRAW, "DrawSky();");
	DefTimer(TIMER_DRAWROADS, TIMER_DRAW, "DrawRoads();");
	//DefTimer(DRAWMODEL1, TIMER_DRAWBL, "Draw model 1");
	//DefTimer(DRAWMODEL2, TIMER_DRAWBL, "Draw model 2");
	//DefTimer(DRAWMODEL3, TIMER_DRAWBL, "Draw model 3");
	DefTimer(TIMER_DRAWMAPDEPTH, TIMER_DRAWSCENEDEPTH, "DrawMap(); depth");
	DefTimer(TIMER_DRAWUMAT, TIMER_DRAWUNITS, "CPU-side matrix math etc.");
	DefTimer(TIMER_DRAWUGL, TIMER_DRAWUNITS, "GPU-side");
	DefTimer(TIMER_DRAWUTEXBIND, TIMER_DRAWUNITS, "texture bind");
}

void LastNum(const char* l)
{
	return;

#if 1
	char fullpath[MAX_PATH+1];
	FullPath("last.txt", fullpath);
	std::ofstream last;
	last.open(fullpath, std::ios_base::out);
	last<<l;
	last.flush();
#else
	g_log<<l<<std::endl;
	g_log.flush();
#endif
}

#ifdef GLDEBUG
void CheckGLError(const char* file, int line)
{
	//char msg[2048];
	//sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	//ErrMess("Out of memory", msg);
	int error = glGetError();

	if(error == GL_NO_ERROR)
		return;

	g_log<<"GL Error #"<<error<<" in "<<file<<" on line "<<line<<" using shader #"<<g_curS<<std::endl;
}
#endif

void LogRich(const RichText* rt)
{
	g_log<<"RichText: "<<std::endl;

	for(auto rtiter = rt->m_part.begin(); rtiter != rt->m_part.end(); rtiter++)
	{
		if(rtiter->m_type == RICHTEXT_ICON)
		{
			g_log<<"[icon"<<rtiter->m_icon<<"]";
		}
		else if(rtiter->m_type == RICHTEXT_TEXT)
		{
			const UString* ustr = &rtiter->m_text;

			for(int i=0; i<ustr->m_length; i++)
			{
				g_log<<"[char#"<<ustr->m_data[i]<<"'"<<(char)ustr->m_data[i]<<"']";
			}
		}
	}

	g_log<<std::endl;
	g_log.flush();
}

#if 0
void UDebug(int i)
{
	return;

	Unit* u = &g_unit[i];
	UType* t = &g_unitType[u->type];

	g_log<<"UNIT DEBUG: "<<t->name<<" ("<<i<<")"<<std::endl;
	g_log<<"path size: "<<u->path.size()<<std::endl;

	if(u->collidesfast())
	{
		g_log<<"COLLIDES: type:"<<g_collidertype<<" ID:"<<g_lastcollider<<std::endl;

		if(g_collidertype == COLLIDER_BUILDING)
		{
			Building* b = &g_building[g_lastcollider];
			BlType* bt = &g_bltype[b->type];

			g_log<<"COLLIDER B: "<<bt->name<<std::endl;

			if(u->confirmcollision(g_collidertype, g_lastcollider))
			{
				g_log<<"CONFIRMED COLLISION"<<std::endl;

				Vec3f p = u->camera.Position();
				Vec3f p2 = b->pos;

				float r = t->radius;
				float hwx = bt->widthX*TILE_SIZE/2.0f;
				float hwz = bt->widthZ*TILE_SIZE/2.0f;

				g_log<<"COLLISION DX,DZ: "<<(fabs(p2.x-p.x)-r-hwx)<<","<<(fabs(p2.z-p.z)-r-hwz)<<std::endl;
			}
		}
	}
}
#endif

#ifdef PLATFORM_WIN
#include <windows.h>
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")
#endif

#ifdef MEMDEBUG
void CheckMem(const char* file, int line, const char* sep)
{
	return;
#ifdef PLATFORM_WIN
	PROCESS_MEMORY_COUNTERS info = {0};
	DWORD cb;
	bool b = GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
	g_log<<sep<<" "<<file<<" line"<<line<<" (?"<<b<<")info.WorkingSetSize: "<<info.WorkingSetSize<<"B / "<<(info.WorkingSetSize/1024)<<"KB "<<(info.WorkingSetSize/1024/1024)<<"MB "<<(info.WorkingSetSize/1024/1024/1024)<<"GB "<<std::endl;
	g_log.flush();
#endif
}
#endif

GLvoid APIENTRY GLMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	//ErrMess("GL Error", message);
	g_log<<"GL Message: "<<message<<std::endl;
}
