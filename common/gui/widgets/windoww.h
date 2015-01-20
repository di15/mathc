#ifndef WINDOWW_H
#define WINDOWW_H

#include "../widget.h"
#include "image.h"
#include "../cursor.h"
#include "vscrollbar.h"

class WindowW : public Widget
{
public:

	Image top_image;
	Image topleft_image;
	Image topright_image;
	Image left_image;
	Image right_image;
	Image bottom_image;
	Image bottomleft_image;
	Image bottomright_image;
	Image bg_image;
	Image bg_logo_image;

	Text title_text;

	Image inner_top_mid_image;
	Image inner_top_leftdiag_image;
	Image inner_top_rightdiag_image;
	Image inner_top_leftstrip_image;
	Image inner_top_rightstrip_image;
	Image inner_top_leftstripblur_image;
	Image inner_top_rightstripblur_image;
	Image inner_top_leftvblur_image;
	Image inner_top_rightvblur_image;

	Image inner_bottom_mid_image;
	Image inner_bottom_leftdiag_image;
	Image inner_bottom_rightdiag_image;
	Image inner_bottom_leftstrip_image;
	Image inner_bottom_rightstrip_image;
	Image inner_bottom_leftstripblur_image;
	Image inner_bottom_rightstripblur_image;
	Image inner_bottom_leftvblur_image;
	Image inner_bottom_rightvblur_image;

	VScroll m_vscroll;

	float m_minsz[2];
	int m_mousedown[2];
	
	float innerleft;
	float innertop;
	float innerright;
	float innerbottom;

	WindowW();
	WindowW(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

	void inev(InEv* ie);
	void draw();
	void drawover();
	void reframe();
	void chcall(Widget* ch, int type, void* data);
	void subframe(float* fr);
};

#endif
