#include "widget.h"
#include "gui.h"
#include "font.h"
#include "../window.h"
#include "icon.h"


void Widget::frameupd()
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->frameupd();
}

void Widget::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

#if 1
	if(m_parent)
	{
		float parentframe[4];
		m_parent->subframe(parentframe);
		SubFrame(parentframe, m_pos, m_frame);
	}
#endif

#if 0	//only use when add windows widgets, and then fix parent bounds of "zoom text" and "max elev" labels
	if(m_parent)
	{
		float* parp = m_parent->m_pos;
		
		//must be bounded by the parent's frame

		m_pos[0] = fmax(parp[0], m_pos[0]);
		m_pos[0] = fmin(parp[2], m_pos[0]);
		m_pos[2] = fmax(parp[0], m_pos[2]);
		m_pos[2] = fmin(parp[2], m_pos[2]);
		m_pos[1] = fmax(parp[1], m_pos[1]);
		m_pos[1] = fmin(parp[3], m_pos[1]);
		m_pos[3] = fmax(parp[1], m_pos[3]);
		m_pos[3] = fmin(parp[3], m_pos[3]);

		m_pos[1] = fmin(m_pos[1], m_pos[3]);
		m_pos[0] = fmin(m_pos[0], m_pos[2]);
	}
#endif

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();
}

void Widget::tofront()
{
	//return;

	if(!m_parent)
		return;

	//return;

	auto subs = &m_parent->m_subwidg;

	for(auto wi=subs->begin(); wi!=subs->end(); wi++)
	{
		if(*wi == this)
		{
			subs->erase(wi);
			subs->push_back(this);
			break;
		}
	}
}

void CenterLabel(Widget* w)
{
	Font* f = &g_font[w->m_font];

	int texwidth = TextWidth(w->m_font, &w->m_label);

#if 0
	char msg[128];
	sprintf(msg, "tw %d, tl %d, fn %d, gh %f", texwidth, w->m_label.texlen(), w->m_font, f->gheight);
	if(g_netmode == NETM_CLIENT)
	InfoMess(msg, msg);
#endif

	w->m_tpos[0] = (w->m_pos[2]+w->m_pos[0])/2 - texwidth/2;
	w->m_tpos[1] = (w->m_pos[3]+w->m_pos[1])/2 - f->gheight/2;
}

Widget* Widget::get(const char* name)
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		if(stricmp((*i)->m_name.c_str(), name) == 0)
			return *i;

	return NULL;
}

void Widget::add(Widget* neww)
{
	if(!neww)
		OutOfMem(__FILE__, __LINE__);

	m_subwidg.push_back(neww);
}

void Widget::close()
{
	m_opened = false;

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->close();
}

void Widget::open()
{
	m_opened = true;
	tofront();	//can't break list iterator, might shift
}

void Widget::chcall(Widget* ch, int type, void* data)
{
}

//free subwidget children
void Widget::freech()
{
	auto witer = m_subwidg.begin();
	while(witer != m_subwidg.end())
	{
		delete *witer;
		witer = m_subwidg.erase(witer);
	}
}

void SubFrame(float *a, float *b, float *c)
{
	c[0] = fmax(a[0], b[0]);
	c[1] = fmax(a[1], b[1]);
	c[2] = fmin(a[2], b[2]);
	c[3] = fmin(a[3], b[3]);

	if(c[0] > c[2])
		c[0] = c[2];

	if(c[1] > c[3])
		c[1] = c[3];
}
