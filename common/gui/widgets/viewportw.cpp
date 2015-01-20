#include "../widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "droplist.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"
#include "viewportw.h"
#include "../../platform.h"
#include "../../window.h"
#include "../../render/shader.h"
#include "../gui.h"

#include "../../debug.h"

Viewport::Viewport()
{
	m_parent = NULL;
	m_type = WIDGET_VIEWPORT;
	m_name = "";
	reframefunc = NULL;
	m_ldown = false;
	m_param = -1;
	drawfunc = NULL;
	ldownfunc = NULL;
	lupfunc = NULL;
	mousemovefunc = NULL;
	rdownfunc = NULL;
	rupfunc = NULL;
	mousewfunc = NULL;
	//reframe();
}

Viewport::Viewport(Widget* parent, const char* n, void (*reframef)(Widget* thisw),
                     void (*drawf)(int p, int x, int y, int w, int h),
                     bool (*ldownf)(int p, int x, int y, int w, int h),
                     bool (*lupf)(int p, int x, int y, int w, int h),
                     bool (*mousemovef)(int p, int x, int y, int w, int h),
                     bool (*rdownf)(int p, int relx, int rely, int w, int h),
                     bool (*rupf)(int p, int relx, int rely, int w, int h),
                     bool (*mousewf)(int p, int d),
                     int parm)
{
	m_parent = parent;
	m_type = WIDGET_VIEWPORT;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_param = parm;
	drawfunc = drawf;
	ldownfunc = ldownf;
	lupfunc = lupf;
	mousemovefunc = mousemovef;
	rdownfunc = rdownf;
	rupfunc = rupf;
	mousewfunc = mousewf;
	reframe();
}

void Viewport::draw()
{
	//g_applog<<m_pos[0]<<","<<m_pos[1]<<","<<m_pos[2]<<","<<m_pos[3]<<std::endl;

	int w = (int)( m_pos[2] - m_pos[0] );
	int h = (int)( m_pos[3] - m_pos[1] );

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(m_pos[0], g_height-m_pos[3], w, h);
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_WIDTH], (float)w);
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_HEIGHT], (float)h);

	EndS();

	CheckGLError(__FILE__, __LINE__);

	if(drawfunc != NULL)
		drawfunc(m_param, m_pos[0], m_pos[1], w, h);

	CheckGLError(__FILE__, __LINE__);

	//glViewport(0, 0, g_width, g_height);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	//glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_WIDTH], (float)g_width);
	//glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_HEIGHT], (float)g_height);

	CheckGLError(__FILE__, __LINE__);
	Ortho(g_width, g_height, 1, 1, 1, 1);
}

void Viewport::inev(InEv* ie)
{
	if(ie->type == INEV_MOUSEMOVE)
	{
		if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
		{
			if(!ie->intercepted)
				m_over = true;
		}
		else
			m_over = false;
	}
	
	if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
			m_over = true;

		if(mousemovefunc != NULL)
		{
			int relx = g_mouse.x - (int)m_pos[0];
			int rely = g_mouse.y - (int)m_pos[1];
			int w = (int)( m_pos[2] - m_pos[0] );
			int h = (int)( m_pos[3] - m_pos[1] );
			ie->intercepted = mousemovefunc(m_param, relx, rely, w, h);
		}

		return;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//InfoMess("vpld", "vpld");

		if(!m_over)
			return;

		if(ldownfunc != NULL)
		{
			int relx = g_mouse.x - (int)m_pos[0];
			int rely = g_mouse.y - (int)m_pos[1];
			int w = (int)( m_pos[2] - m_pos[0] );
			int h = (int)( m_pos[3] - m_pos[1] );
			ie->intercepted = ldownfunc(m_param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//InfoMess("vplu", "vplu");

		if(!m_over)
			return;

		if(lupfunc != NULL)
		{
			int relx = g_mouse.x - (int)m_pos[0];
			int rely = g_mouse.y - (int)m_pos[1];
			int w = (int)( m_pos[2] - m_pos[0] );
			int h = (int)( m_pos[3] - m_pos[1] );
			ie->intercepted = lupfunc(m_param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_RIGHT && !ie->intercepted)
	{
		if(!m_over)
			return;

		if(rdownfunc != NULL)
		{
			int relx = g_mouse.x - (int)m_pos[0];
			int rely = g_mouse.y - (int)m_pos[1];
			int w = (int)( m_pos[2] - m_pos[0] );
			int h = (int)( m_pos[3] - m_pos[1] );
			ie->intercepted = rdownfunc(m_param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_RIGHT && !ie->intercepted)
	{
		if(!m_over)
			return;

		if(rupfunc != NULL)
		{
			int relx = g_mouse.x - (int)m_pos[0];
			int rely = g_mouse.y - (int)m_pos[1];
			int w = (int)( m_pos[2] - m_pos[0] );
			int h = (int)( m_pos[3] - m_pos[1] );
			ie->intercepted = rupfunc(m_param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEWHEEL && !ie->intercepted)
	{
		if(!m_over)
			return;

		if(mousewfunc != NULL)
		{
			ie->intercepted = mousewfunc(m_param, ie->amount);
		}
	}
}
