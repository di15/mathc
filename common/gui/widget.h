#ifndef WIDGET_H
#define WIDGET_H

#include "../utils.h"
#include "../texture.h"
#include "font.h"
#include "../render/shader.h"
#include "../window.h"
#include "draw2d.h"
#include "richtext.h"
#include "inevent.h"

#define MAX_OPTIONS_SHOWN	7

#define WIDGET_IMAGE				1
#define WIDGET_BUTTON				2
#define WIDGET_TEXT					3
#define WIDGET_LINK					4
#define WIDGET_DROPLIST				5
#define WIDGET_EDITBOX				6
#define WIDGET_BARBUTTON			7
#define WIDGET_HSCROLLER			8
#define WIDGET_TOUCHLISTENER		9
#define WIDGET_TEXTBLOCK			10
#define WIDGET_CHECKBOX				11
#define WIDGET_INSDRAW				12
#define WIDGET_LISTBOX				13
#define WIDGET_TEXTAREA				14
#define WIDGET_VIEWPORT				15
#define WIDGET_FRAME				16
#define WIDGET_RESTICKER			17
#define WIDGET_BOTTOMPANEL			18
#define WIDGET_BUILDPREVIEW			19
#define WIDGET_CONSTRUCTIONVIEW		20
#define WIDGET_GUI					21
#define WIDGET_VIEWLAYER			22
#define WIDGET_WINDOW				23
#define WIDGET_VSCROLLBAR			24
#define WIDGET_HSCROLLBAR			25
#define WIDGET_BUILDINGVIEW			26
#define WIDGET_SVLISTVIEW			27
#define WIDGET_NEWHOST				28
#define WIDGET_TRUCKMGR				29
#define WIDGET_SAVEVIEW				30
#define WIDGET_LOADVIEW				31
#define WIDGET_LOBBY				32

#define CHCALL_VSCROLL				0
#define CHCAlL_HSCROLL				1

class Widget
{
public:
	int m_type;
	Widget* m_parent;
	float m_pos[4];
	float m_frame[4];
	float m_texc[4];	//texture coordinates
	float m_tpos[4];	//text pos
	unsigned int m_tex;
	unsigned int m_bgtex;
	unsigned int m_bgovertex;
	bool m_over;
	bool m_ldown;	//was the left mouse button pressed while over this (i.e. drag)?
	std::string m_name;
	RichText m_text;
	int m_font;
	unsigned int m_frametex, m_filledtex, m_uptex, m_downtex;
	bool m_opened;
	std::vector<RichText> m_options;
	int m_selected;
	float m_scroll[2];
	bool m_mousescroll;
	float m_vel[2];
	int m_param;
	float m_rgba[4];
	RichText m_value;
	int m_caret;
	bool m_passw;
	int m_maxlen;
	bool m_shadow;
	std::list<Widget*> m_subwidg;
	int m_lines;
	int m_alignment;
	RichText m_label;
	bool m_popup;

	void (*clickfunc)();
	void (*clickfunc2)(int p);
	void (*overfunc)();
	void (*overfunc2)(int p);
	void (*outfunc)();
	void (*changefunc)();
	void (*changefunc2)(int p);
	void (*reframefunc)(Widget* thisw);

	Widget()
	{
		clickfunc = NULL;
		clickfunc2 = NULL;
		overfunc = NULL;
		overfunc2 = NULL;
		outfunc = NULL;
		changefunc = NULL;
		changefunc2 = NULL;
		m_caret = 0;
		m_parent = NULL;
		m_opened = false;
		m_ldown = false;
		reframefunc = NULL;
	}

	virtual ~Widget()
	{
		freech();
	}

	virtual void draw()	{}
	virtual void drawover()	{}
	virtual void inev(InEv* ie) {}
	virtual void frameupd();
	virtual void reframe();	//resized or moved
	virtual void subframe(float* fr)
	{
		memcpy((void*)fr, (void*)m_pos, sizeof(float)*4);
	}
	virtual Widget* get(const char* name);
	virtual void add(Widget* neww);
	virtual void close();
	virtual void open();
	virtual void chcall(Widget* ch, int type, void* data);	//child callback
	virtual void freech();	//free subwidget children
	virtual void tofront();	//only used for windows. edit: needed for everything since droplist uses it on parent tree.
	virtual void subdraw(){}
	virtual void subdrawover(){}
	virtual void subreframe(){}
	virtual void subinev(InEv* ie){}
};

void CenterLabel(Widget* w);
void SubFrame(float *a, float *b, float *c);
#endif
