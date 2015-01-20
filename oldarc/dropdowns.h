
#ifndef DROPDOWNS_H
#define DROPDOWNS_H

#include "../widget.h"

class DropDownS : public Widget
{
public:

	int m_mousedown[2];

	DropDownS(Widget* parent, const char* n, int f, void (*reframef)(Widget* thisw), void (*change)());

	void draw();
	void drawover();
	void inev(InEv* ev);

	int rowsshown();
	int square();
	void erase(int which);

	float topratio()
	{
		return m_scroll[1] / (float)(m_options.size());
	}

	float bottomratio()
	{
		return (m_scroll[1]+rowsshown()) / (float)(m_options.size());
	}

	float scrollspace();
};

#endif
