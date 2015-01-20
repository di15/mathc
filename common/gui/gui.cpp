#include "gui.h"
#include "../render/shader.h"
#include "../texture.h"
#include "font.h"
#include "../math/3dmath.h"
#include "../platform.h"
#include "../window.h"
#include "draw2d.h"
#include "../render/shadow.h"
#include "../render/heightmap.h"
#include "../../game/gmain.h"
#include "cursor.h"
#include "../sim/player.h"
#include "../debug.h"

GUI g_gui;

void GUI::draw()
{
	Player* py = &g_player[g_localP];

	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	CheckGLError(__FILE__, __LINE__);
	Ortho(g_width, g_height, 1, 1, 1, 1);
	CheckGLError(__FILE__, __LINE__);

#if 0
	DrawImage(g_texture[0].texname, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
	{
#ifdef DEBUG
		g_log<<"draw "<<(*i)->m_name<<" "<<__FILE__<<" "<<__LINE__<<std::endl;
		g_log.flush();
#endif

		(*i)->draw();
	}

	CheckGLError(__FILE__, __LINE__);

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->drawover();

#if 0
	DrawImage(g_depth, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

#if 0
	if(g_depth != -1)
		DrawImage(g_depth, 0, 0, 150, 150, 0, 1, 1, 0);
#endif

#if 0
	if(g_mode == APPMODE_PLAY)
		DrawImage(g_tiletexs[TILE_PRERENDER], 0, 0, 150, 150, 0, 1, 1, 0);
#endif

	Sprite* sp = &g_cursor[g_curst];
	DrawImage(g_texture[sp->texindex].texname, g_mouse.x-sp->offset[0], g_mouse.y-sp->offset[1], g_mouse.x-sp->offset[0]+32, g_mouse.y-sp->offset[1]+32);

	CheckGLError(__FILE__, __LINE__);

	EndS();
	CheckGLError(__FILE__, __LINE__);

	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	//glEnable(GL_DEPTH_TEST);
	//DrawSelector();
	DrawMarquee();

	CheckGLError(__FILE__, __LINE__);
	EndS();
	CheckGLError(__FILE__, __LINE__);

	glEnable(GL_DEPTH_TEST);
}

void GUI::inev(InEv* ie)
{
#if 0
	for(auto w=m_subwidg.rbegin(); w!=m_subwidg.rend(); w++)
		(*w)->inev(ie);
#else
	//safe, may shift during call
	int win = 0;
	while(win < m_subwidg.size())
	{
		int win2 = 0;
		for(auto wit=m_subwidg.rbegin(); wit!=m_subwidg.rend(); wit++, win2++)
		{
			if(win2 < win)
				continue;

			(*wit)->inev(ie);
			break;
		}
		win++;
	}
#endif


	if(!ie->intercepted)
	{
		//if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT) g_log<<"mouse up l"<<std::endl;

		if(ie->type == INEV_MOUSEMOVE && mousemovefunc) mousemovefunc();
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && lbuttondownfunc) lbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && lbuttonupfunc) lbuttonupfunc();
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_MIDDLE && mbuttondownfunc) mbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_MIDDLE && mbuttonupfunc) mbuttonupfunc();
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_RIGHT && rbuttondownfunc) rbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_RIGHT && rbuttonupfunc) rbuttonupfunc();
		else if(ie->type == INEV_MOUSEWHEEL && mousewheelfunc) mousewheelfunc(ie->amount);
		else if(ie->type == INEV_KEYDOWN && keydownfunc[ie->scancode]) keydownfunc[ie->scancode]();
		else if(ie->type == INEV_KEYUP && keyupfunc[ie->scancode]) keyupfunc[ie->scancode]();
	}
}

void GUI::closeall()
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->close();
}

void GUI::close(const char* name)
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		if(stricmp((*i)->m_name.c_str(), name) == 0)
		{
			(*i)->close();
		}
}

void GUI::open(const char* name)
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		if(stricmp((*i)->m_name.c_str(), name) == 0)
		{
			(*i)->open();
			break;	//important - list may shift after open() and tofront() call
		}
}

void GUI::reframe()
{
	Player* py = &g_player[g_localP];

	m_pos[0] = 0;
	m_pos[1] = 0;
	m_pos[2] = g_width-1;
	m_pos[3] = g_height-1;

	for(auto w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->reframe();
}

void Status(const char* status, bool logthis)
{
	if(logthis)
	{
		g_log<<status<<std::endl;
		g_log.flush();
	}

#if 0
	g_log<<status<<std::endl;
	g_log.flush();
#endif
	/*
	char upper[1024];
	int i;
	for(i=0; i<strlen(status); i++)
	{
	upper[i] = toupper(status[i]);
	}
	upper[i] = '\0';*/

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	//gui->get("load")->get("status", WIDGET_TEXT)->m_text = upper;
	ViewLayer* loadingview = (ViewLayer*)gui->get("loading");

	if(!loadingview)
		return;

	Widget* statustext = loadingview->get("status");

	if(!statustext)
		return;

	statustext->m_text = RichText(UString(status));
}

bool MousePosition()
{
	Player* py = &g_player[g_localP];

	Vec2i old = g_mouse;
	SDL_GetMouseState(&g_mouse.x, &g_mouse.y);

	if(g_mouse.x == old.x && g_mouse.y == old.y)
		return false;

	return true;
}

void CenterMouse()
{
	Player* py = &g_player[g_localP];

	g_mouse.x = g_width/2;
	g_mouse.y = g_height/2;
	SDL_WarpMouseInWindow(g_window, g_mouse.x, g_mouse.y);
}

void Ortho(int width, int height, float r, float g, float b, float a)
{
	CheckGLError(__FILE__, __LINE__);
	Player* py = &g_player[g_localP];
	UseS(SHADER_ORTHO);
	Shader* s = &g_shader[g_curS];
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_WIDTH], (float)width);
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_HEIGHT], (float)height);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], r, g, b, a);
	//glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(g_shader[SHADER_ORTHO].m_slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(g_shader[SHADER_ORTHO].m_slot[SSLOT_NORMAL]);
	g_currw = width;
	g_currh = height;
	CheckGLError(__FILE__, __LINE__);
}
