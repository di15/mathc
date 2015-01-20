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
#include "../gui.h"

DropList::DropList() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_DROPLIST;
	m_name = "";
	m_font = MAINFONT8;
	m_opened = false;
	m_selected = 0;
	m_scroll[1] = 0;
	m_mousescroll = false;
	m_ldown = false;
	changefunc = NULL;
	reframefunc = NULL;
	CreateTexture(m_frametex, "gui/frame.jpg", true, false);
	CreateTexture(m_filledtex, "gui/filled.jpg", true, false);
	CreateTexture(m_uptex, "gui/up.jpg", true, false);
	//CreateTexture(m_downtex, "gui/down.jpg", true, false);
	reframe();
}

DropList::DropList(Widget* parent, const char* n, int f, void (*reframef)(Widget* thisw), void (*change)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_DROPLIST;
	m_name = n;
	m_font = f;
	m_opened = false;
	m_selected = 0;
	m_scroll[1] = 0;
	m_mousescroll = false;
	m_ldown = false;
	changefunc = change;
	reframefunc = reframef;
	CreateTexture(m_frametex, "gui/frame.jpg", true, false);
	CreateTexture(m_filledtex, "gui/filled.jpg", true, false);
	CreateTexture(m_uptex, "gui/up.jpg", true, false);
	//CreateTexture(m_downtex, "gui/down.jpg", true, false);
	reframe();
}

void DropList::erase(int which)
{
	m_options.erase( m_options.begin() + which );
	if(m_selected == which)
		m_selected = -1;

	if(m_scroll[1] + rowsshown() > m_options.size())
		m_scroll[1] = m_options.size() - (float)rowsshown();

	if(m_scroll[1] < 0)
		m_scroll[1] = 0;
}

int DropList::rowsshown()
{
	int rows = MAX_OPTIONS_SHOWN;

	if(rows > m_options.size())
		rows = m_options.size();

	return rows;
}

int DropList::square()
{
	return (int)g_font[m_font].gheight;
}

float DropList::scrollspace()
{
	return g_font[m_font].gheight*(rowsshown())-square();
}

void DropList::draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(g_texture[m_frametex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	if(!m_opened)
		DrawImage(g_texture[m_uptex].texname, m_pos[2]-square(), m_pos[1], m_pos[2], m_pos[1]+square(), 0, 1, 1, 0);

	if(m_options.size() <= 0)
		return;

	if(m_selected < 0)
	{
		//if(m_options.size() <= 0)
		//	return;

		//DrawShadowedText(m_font, m_pos[0]+3, m_pos[1], m_options[0].c_str());

		return;
	}

	if(m_selected >= (int)m_options.size())
		return;

	DrawShadowedTextF(m_font, m_pos[0]+3, m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &m_options[m_selected], NULL, -1);
}

void DropList::drawover()
{
	if(!m_opened)
		return;

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	Font* f = &g_font[m_font];

	DrawImage(g_texture[m_frametex].texname, m_pos[0], m_pos[1]+f->gheight, m_pos[2], m_pos[3]+f->gheight*rowsshown());
	DrawImage(g_texture[m_frametex].texname, m_pos[2]-square(), m_pos[1], m_pos[2], m_pos[3]+f->gheight*rowsshown());
	DrawImage(g_texture[m_uptex].texname, m_pos[2]-square(), m_pos[1], m_pos[2], m_pos[1]+square());
	DrawImage(g_texture[m_uptex].texname, m_pos[2]-square(), m_pos[3]+f->gheight*rowsshown()-square(), m_pos[2], m_pos[3]+f->gheight*rowsshown(), 0, 1, 1, 0);
	DrawImage(g_texture[m_filledtex].texname, m_pos[2]-square(), m_pos[3]+scrollspace()*topratio(), m_pos[2], m_pos[3]+scrollspace()*bottomratio());

	for(int i=(int)m_scroll[1]; i<(int)m_scroll[1]+rowsshown(); i++)
		//DrawShadowedText(m_font, m_pos[0]+3, m_pos[3]+f->gheight*(i-(int)m_scroll[1]), &m_options[i]);
		DrawShadowedTextF(m_font, m_pos[0]+3, m_pos[3]+f->gheight*(i-(int)m_scroll[1]), m_pos[0], m_pos[1], m_pos[2], m_pos[3] + rowsshown()*f->gheight, &m_options[i]);
}

void DropList::inev(InEv* ie)
{
	if(ie->type == INEV_MOUSEWHEEL && !ie->intercepted)
	{
		if(m_opened)
		{
			ie->intercepted = true;
			return;	// intercept mouse event
		}
	}
	else if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
			g_mouseoveraction =  true;

#if 0
		if(m_opened)
		{
			Font* f = &g_font[m_font];

			//on dropped list?
			if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[1] + f->gheight*rowsshown())
			{
				g_mouseoveraction =  true;
				m_over = true;
				ie->intercepted = true;
			}
		}
#endif

		if(m_ldown)
			ie->intercepted = true;

		if(!m_mousescroll)
			return;

		g_mouseoveraction =  true;

		int dy = g_mouse.y - m_mousedown[1];
		float topy = m_pos[3]+square()+scrollspace()*topratio();
		float newtopy = topy + dy;

		//topratio = (float)scroll / (float)(options.size());
		//topy = pos[3]+square+scrollspace*topratio
		//topy = pos[3]+square+scrollspace*((float)scroll / (float)(options.size()))
		//topy - pos[3] - square = scrollspace*(float)scroll / (float)(options.size())
		//(topy - pos[3] - square)*(float)(options.size())/scrollspace = scroll

		m_scroll[1] = (newtopy - m_pos[3] - square())*(float)(m_options.size())/scrollspace();

		if(m_scroll[1] < 0)
		{
			m_scroll[1] = 0;
			ie->intercepted = true;
			return;
		}
		else if(m_scroll[1] + rowsshown() > m_options.size())
		{
			m_scroll[1] = m_options.size() - (float)rowsshown();
			ie->intercepted = true;
			return;
		}

		m_mousedown[1] = g_mouse.y;
		ie->intercepted = true;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
	{
		//InfoMess("dlld", "dlld");

#if 0
		if(m_over)
		{
			m_ldown = true;
			ie->intercepted = true;
		}
#endif

		if(m_opened)
		{
			for(int i=(int)m_scroll[1]; i<(int)m_scroll[1]+rowsshown(); i++)
			{
				// std::list item?
				if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2]-square() && g_mouse.y >= m_pos[3]+g_font[m_font].gheight*(i-(int)m_scroll[1])
				                && g_mouse.y <= m_pos[3]+g_font[m_font].gheight*(i-(int)m_scroll[1]+1))
				{
					m_ldown = true;
					ie->intercepted = true;
					return;	// intercept mouse event
				}
			}

			// scroll bar?
			if(g_mouse.x >= m_pos[2]-square() && g_mouse.y >= m_pos[3]+scrollspace()*topratio() && g_mouse.x <= m_pos[2] &&
			                g_mouse.y <= m_pos[3]+scrollspace()*bottomratio())
			{
				m_ldown = true;
				m_mousescroll = true;
				m_mousedown[1] = g_mouse.y;
				ie->intercepted = true;
				return;	// intercept mouse event
			}

			// up button?
			if(g_mouse.x >= m_pos[2]-square() && g_mouse.y >= m_pos[1] && g_mouse.x <= m_pos[2] && g_mouse.y <= m_pos[3])
			{
				m_ldown = true;
				ie->intercepted = true;
				return;
			}

			// down button?
			if(g_mouse.x >= m_pos[2]-square() && g_mouse.y >= m_pos[3]+scrollspace() && g_mouse.x <= m_pos[2] && g_mouse.y <= m_pos[3]+scrollspace()+g_font[m_font].gheight)
			{
				m_ldown = true;
				ie->intercepted = true;
				return;
			}

			ie->intercepted = true;
		}

		if(!ie->intercepted)
		{
			if(g_mouse.x >= m_pos[0] && g_mouse.y >= m_pos[1] && g_mouse.x <= m_pos[2] && g_mouse.y <= m_pos[3])
			{
				m_ldown = true;
				ie->intercepted = true;
				return;
			}
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
	{
		//InfoMess("dllu", "dllu");

#if 0
		if(m_over)
		{
			ie->intercepted = true;
		}
#endif

		if(m_opened)
		{
			if(!m_ldown)
			{
				m_opened = false;
				return;
			}

			if(m_ldown)
			{
				ie->intercepted = true;
				m_ldown = false;
			}

			if(m_mousescroll)
			{
				m_mousescroll = false;
				ie->intercepted = true;
				return;	// intercept mouse event
			}

			for(int i=(int)m_scroll[1]; i<(int)m_scroll[1]+rowsshown(); i++)
			{
				// std::list item?
				if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2]-square() && g_mouse.y >= m_pos[3]+g_font[m_font].gheight*(i-(int)m_scroll[1])
				                && g_mouse.y <= m_pos[3]+g_font[m_font].gheight*(i-(int)m_scroll[1]+1))
				{
					m_selected = i;
					m_opened = false;
					if(changefunc != NULL)
						changefunc();

					ie->intercepted = true;
					return;	// intercept mouse event
				}
			}

			// up button?
			if(g_mouse.x >= m_pos[2]-square() && g_mouse.y >= m_pos[1] && g_mouse.x <= m_pos[2] && g_mouse.y <= m_pos[3])
			{
				m_scroll[1]--;
				if(m_scroll[1] < 0)
					m_scroll[1] = 0;

				ie->intercepted = true;
				return;
			}

			// down button?
			if(g_mouse.x >= m_pos[2]-square() && g_mouse.y >= m_pos[3]+scrollspace() && g_mouse.x <= m_pos[2] && g_mouse.y <= m_pos[3]+scrollspace()+g_font[m_font].gheight)
			{
				m_scroll[1]++;
				if(m_scroll[1]+rowsshown() > m_options.size())
					m_scroll[1] = m_options.size() - rowsshown();

				ie->intercepted = true;
				return;
			}

			m_opened = false;

			ie->intercepted = true;	// intercept mouse event
		}

		else if(!ie->intercepted)
		{
			if(!m_ldown)
				return;

			m_ldown = false;

			if(g_mouse.x >= m_pos[2]-square() && g_mouse.y >= m_pos[1] && g_mouse.x <= m_pos[2] && g_mouse.y <= m_pos[1]+square())
			{
				m_opened = true;
				ie->intercepted = true;

				//Need to bring tree to front so that drop-down list gets 
				//the mouse up event first instead of any item in the background.
				Widget* parw = m_parent;
				while(parw)
				{
					parw->tofront();
					parw = parw->m_parent;
				}
				tofront();

				return;
			}
		}
	}
}
