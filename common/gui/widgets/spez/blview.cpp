#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "cstrview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../../../game/gui/gviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/bltype.h"
#include "../../../sim/road.h"
#include "../../../sim/crpipe.h"
#include "../../../sim/powl.h"
#include "../../../sim/unit.h"
#include "../../../sim/utype.h"
#include "../../../sim/player.h"
#include "blview.h"
#include "blpreview.h"
#include "../../icon.h"
#include "../../../math/fixmath.h"
#include "../../../net/lockstep.h"

//Need a better way to organize buttons, text, etc.

// change bl view prod price
#if 0
void Change_BV_PP(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(ri >= parm)
			break;
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* pp = (EditBox*)bv->get(wn);
	std::string sval = pp->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	ival = imax(ival, 0);
	b->price[parm] = ival;
}
#endif

//resize bl view column item
void Resize_BV_Cl(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	int row;
	int col;
	sscanf(thisw->m_name.c_str(), "%d %d", &row, &col);
	
	thisw->m_pos[0] = parw->m_pos[0] + 64*(imax(col-1,0));
	thisw->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	thisw->m_pos[2] = imin(parw->m_pos[0] + 150 + 64*(col+1), parw->m_pos[2]);
	thisw->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);

	//first column is extra large (label text)
	if(col > 0)
		thisw->m_pos[0] += 200;
}

//resize bl view stocked goods list
//TODO regenvals
void Resize_BV_SL(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}
	
	if(bt->input[RES_LABOUR] > 0)
		row++;

#if 0
	for(int ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#else
	if(b->type == BL_FACTORY)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#endif

	//bank space
	row++;

	//set button
	row++;

	//blank space
	row++;
	row++;

	//stocked goods list
	
	Widget* parw = thisw->m_parent;
	int col = 0; 
	
	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[3];
}

//resize bl view cancel button
void Resize_BV_Close(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;

	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}
	
	if(bt->input[RES_LABOUR] > 0)
		row++;

#if 0
	for(int ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#else
	if(b->type == BL_FACTORY)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#endif

	//blank space
	row++;

	//set, cancel button
	row++;
	
	Widget* parw = thisw->m_parent;
	int col = 1; 
	
	thisw->m_pos[0] = parw->m_pos[0] + col * 60;
	thisw->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	thisw->m_pos[2] = parw->m_pos[0] + (col+1) * 60;
	thisw->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);

	//blank space
	row++;
	row++;
	
	//stocked goods list
	//...
}

//resize bl view set button
void Resize_BV_Set(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}
	
	if(bt->input[RES_LABOUR] > 0)
		row++;

#if 0
	for(int ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}
#else
	if(b->type == BL_FACTORY)
	{
		row++;	//truck line
		row++;	//battlecomp line
		row++;	//carlyle line
	}
#endif

	//blank space
	row++;

	//set, cancel button
	row++;
	
	Widget* parw = thisw->m_parent;
	int col = 3; 
	
	thisw->m_pos[0] = parw->m_pos[0] + col * 60;
	thisw->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	thisw->m_pos[2] = parw->m_pos[0] + (col+1) * 60;
	thisw->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);

	//blank space
	row++;
	row++;
	
	//stocked goods list
	//...
}

// change bl view manuf unit price
#if 0
void Change_BV_MP(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}
	
	if(bt->input[RES_LABOUR] > 0)
		row++;

	for(int ui=0; ui<UNIT_TYPES; ui++)
	{
		//todo when buildables set in bltype
	}

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* cw = (EditBox*)bv->get(wn);
	std::string sval = cw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	ival = imax(ival, 0);
	b->manufprc[parm] = ival;
}
#endif

// change bl view (op) wage
#if 0
void Change_BV_OW(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	//if(bt->input[RES_LABOUR] > 0)
	//	row++;

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* cw = (EditBox*)bv->get(wn);
	std::string sval = cw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	ival = imax(ival, 0);
	b->opwage = ival;
}
#endif

// change bl view production level
#if 0
void Change_BV_PL(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	if(bt->input[RES_LABOUR] > 0)
		row++;

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* cw = (EditBox*)bv->get(wn);
	std::string sval = cw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	ival = imin(ival, RATIO_DENOM);
	ival = imax(ival, 0);
	b->prodlevel = ival;
}
#endif

//bl view: build truck (at factory only)
void Click_BV_Truck()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	//TO DO: multiplayer lockstep netturn packet
	ManufJob mj;
	mj.owner = g_localP;
	mj.utype = UNIT_TRUCK;
	b->manufjob.push_back(mj);
	b->trymanuf();
	b->manufjob.clear();

	//crash as button is erased while in use
	//py = &g_player[g_localP];
	//bv->regen(&g_sel);
}

void Click_BV_BComp()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	//TO DO: multiplayer lockstep netturn packet
	ManufJob mj;
	mj.owner = g_localP;
	mj.utype = UNIT_BATTLECOMP;
	b->manufjob.push_back(mj);
	b->trymanuf();
	b->manufjob.clear();

	//crash as button is erased while in use
	//py = &g_player[g_localP];
	//bv->regen(&g_sel);
}

void Click_BV_Carlyle()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	//TO DO: multiplayer lockstep netturn packet
	ManufJob mj;
	mj.owner = g_localP;
	mj.utype = UNIT_CARLYLE;
	b->manufjob.push_back(mj);
	b->trymanuf();
	b->manufjob.clear();

	//crash as button is erased while in use
	//py = &g_player[g_localP];
	//bv->regen(&g_sel);
}

void Click_BV_Close()
{
	GUI* gui = &g_gui;
	gui->close("bl view");
}

void Click_BV_Set()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	BlView* bv = (BlView*)gui->get("bl view");
	Selection* sel = &g_sel;
	
	if(sel->buildings.size() <= 0)
		return;

	int bi = *sel->buildings.begin();
	Building* b = &g_building[ bi ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;
	char wn[32];

	// change bl view prod price

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;

		sprintf(wn, "%d 1", row);
		EditBox* pp = (EditBox*)bv->get(wn);
		std::string sval = pp->m_value.rawstr();

		if(sval.length() <= 0)
			return;

		int ival;
		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		ival = imax(ival, 0);
		//b->price[ri] = ival;

		if(b->price[ri] != ival)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_BLPRICE;
			cvp.bi = bi;
			cvp.player = g_localP;
			cvp.res = ri;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp, sizeof(ChValPacket));
		}

		row++;
	}
	
	EditBox* cw;
	std::string sval;
	int ival;

	if(bt->input[RES_LABOUR] > 0)
	{
		// change bl view (op) wage

		sprintf(wn, "%d 1", row);
		cw = (EditBox*)bv->get(wn);
		sval = cw->m_value.rawstr();

		if(sval.length() <= 0)
			return;

		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		ival = imax(ival, 0);
		//b->opwage = ival;

		if(b->opwage != ival)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_BLWAGE;
			cvp.bi = bi;
			cvp.player = g_localP;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp, sizeof(ChValPacket));
		}
	
		row++;
	}

	// change bl view production level

	sprintf(wn, "%d 1", row);
	cw = (EditBox*)bv->get(wn);
	sval = cw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	ival = imin(ival, RATIO_DENOM);
	ival = imax(ival, 0);
	//b->prodlevel = ival;

	//char msg[128];
	//sprintf(msg, "prodv %d", ival);
	//InfoMess(msg, msg);
	
	if(b->prodlevel != ival)
	{
		ChValPacket cvp;
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_PRODLEV;
		cvp.bi = bi;
		cvp.player = g_localP;
		cvp.value = ival;
		LockCmd((PacketHeader*)&cvp, sizeof(ChValPacket));
	}
	
	row++;

	// change bl view manuf unit price

	for(int ui=0; ui<UNIT_TYPES; ui++)
	{
		//buildables set in bltype
		bool found = false;

		for(auto mit=bt->manuf.begin(); mit!=bt->manuf.end(); mit++)
		{
			if(*mit != ui)
				continue;

			found = true;
			break;
		}

		if(!found)
			continue;

		char wn[32];
		sprintf(wn, "%d 1", row);
		EditBox* cw = (EditBox*)bv->get(wn);
		std::string sval = cw->m_value.rawstr();

		if(sval.length() <= 0)
			return;

		int ival;
		sscanf(sval.c_str(), "%d", &ival);

		//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
		//Also needs to be sent to server and only executed when received back.
		ival = imax(ival, 0);
		//b->manufprc[ui] = ival;

		if(b->manufprc[ui] != ival)
		{
			ChValPacket cvp;
			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_MANPRICE;
			cvp.bi = bi;
			cvp.player = g_localP;
			cvp.utype = ui;
			cvp.value = ival;
			LockCmd((PacketHeader*)&cvp, sizeof(ChValPacket));
		}

		row++;
	}
}

//TODO mouseover manuf button resource costs
BlView::BlView(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_BUILDINGVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	
	m_close = Button(this, "close", "gui/transp.png", RichText("Close"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Close, Click_BV_Close, NULL, NULL, NULL, NULL, -1);
	m_set = Button(this, "set", "gui/transp.png", RichText("Set"), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Set, Click_BV_Set, NULL, NULL, NULL, NULL, -1);

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void BlView::regvals(Selection* sel)
{

}

void BlView::subdraw()
{
	m_close.draw();
	m_set.draw();
}

void BlView::subdrawover()
{
	m_close.drawover();
	m_set.drawover();
}

void BlView::subreframe()
{
	m_close.reframe();
	m_set.reframe();
}

void BlView::subinev(InEv* ie)
{
	m_close.inev(ie);
	m_set.inev(ie);
}

void BlView::regen(Selection* sel)
{
	RichText bname;
	int* price;

	Player* py = &g_player[g_localP];
	BlType* bt;
	Building* b;
	bool owned = false;	//owned by current player?
	Player* opy;
	int bi;

	if(sel->buildings.size() > 0)
	{
		bi = *sel->buildings.begin();
		b = &g_building[bi];
		bt = &g_bltype[b->type];

		if(b->owner == g_localP)
			owned = true;

		g_bptype = b->type;
		price = b->price;

		bname = RichText(UString(bt->name));

#if 0
		if(b->type == BL_NUCPOW)
		{
			char msg[1280];
			sprintf(msg, "blview \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d", 
				(int)b->transporter[RES_URANIUM], 
				(int)g_unit[b->transporter[RES_URANIUM]].mode,
				(int)g_unit[b->transporter[RES_URANIUM]].target,
				bi,
				(int)g_unit[b->transporter[RES_URANIUM]].targtype,
				(int)g_unit[b->transporter[RES_URANIUM]].cargotype);
			InfoMess(msg, msg);
		}
#endif

		opy = &g_player[b->owner];
	}
#if 0
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		conmat = g_roadcost;
		qty = sel->roads.size();
		Vec2i xz = *sel->roads.begin();
		RoadTile* road = RoadAt(xz.x, xz.y);
		maxcost = road->maxcost;
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		conmat = g_powlcost;
		qty = sel->powls.size();
		Vec2i xz = *sel->powls.begin();
		PowlTile* powl = PowlAt(xz.x, xz.y);
		maxcost = powl->maxcost;
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		qty = sel->crpipes.size();
		conmat = g_crpipecost;
		Vec2i xz = *sel->crpipes.begin();
		CrPipeTile* crpipe = CrPipeAt(xz.x, xz.y);
		maxcost = crpipe->maxcost;
	}
#endif

	freech();

	m_subwidg.push_back(new Viewport(this, "viewport", Resize_BP_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	m_subwidg.push_back(new Text(this, "owner", opy->name, FONT_EUROSTILE16, Resize_BP_Ow, true, opy->color[0], opy->color[1], opy->color[2], opy->color[3]));
	m_subwidg.push_back(new Text(this, "title", bname, MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;

		Resource* r = &g_resource[ri];

		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		m_subwidg.push_back(new Text(this, rowname, RichText(UString("Price of ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString(r->name.c_str())) + RichText(UString(":")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", b->price[ri]);

		if(owned)
			//m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, Change_BV_PP, NULL, ri));
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, NULL, NULL, ri));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	if(bt->input[RES_LABOUR] > 0)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		Resource* r = &g_resource[RES_DOLLARS];

		m_subwidg.push_back(new Text(this, rowname, RichText(UString("Wage ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString(":")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", b->opwage);

		if(owned)
			//m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, Change_BV_OW, NULL, RES_LABOUR));
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl, false, 8, NULL, NULL, RES_LABOUR));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	//production level input field
	if(owned)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);

		char label[64];
		sprintf(label, "Production level (max %d):", RATIO_DENOM);

		m_subwidg.push_back(new Text(this, rowname, RichText(UString(label)), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cplstr[32];
		sprintf(cplstr, "%d", b->prodlevel);

		if(owned)
			//m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cplstr)), MAINFONT16, Resize_BV_Cl, false, 6, Change_BV_PL, NULL, RES_LABOUR));
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cplstr)), MAINFONT16, Resize_BV_Cl, false, 6, NULL, NULL, RES_LABOUR));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(cplstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	//TODO buildables set in bltype
	if(b->type == BL_FACTORY)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		Resource* r = &g_resource[RES_DOLLARS];
#if 0
		Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip,int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm);
#endif
		m_subwidg.push_back(new Button(this, rowname, "gui/transp.png", RichText(UString("Order Truck for ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString(":")), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Cl, Click_BV_Truck, NULL, NULL, NULL, NULL, UNIT_TRUCK));
	
		sprintf(rowname, "%d %d", row, 1);
		char clabel[64];
		sprintf(clabel, "%d", b->manufprc[UNIT_TRUCK]);

		if(owned)
			//m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, Change_BV_MP, NULL, UNIT_TRUCK));
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, NULL, NULL, UNIT_TRUCK));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	if(b->type == BL_FACTORY)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		Resource* r = &g_resource[RES_DOLLARS];
#if 0
		Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip,int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm);
#endif
		m_subwidg.push_back(new Button(this, rowname, "gui/transp.png", RichText(UString("Order Battle Computer for ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString(":")), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Cl, Click_BV_BComp, NULL, NULL, NULL, NULL, UNIT_TRUCK));
	
		sprintf(rowname, "%d %d", row, 1);
		char clabel[64];
		sprintf(clabel, "%d", b->manufprc[UNIT_BATTLECOMP]);

		if(owned)
			//m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, Change_BV_MP, NULL, UNIT_BATTLECOMP));
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, NULL, NULL, UNIT_BATTLECOMP));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl));

		row++;
	}
	
	if(b->type == BL_FACTORY)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		Resource* r = &g_resource[RES_DOLLARS];
#if 0
		Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip,int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm);
#endif
		m_subwidg.push_back(new Button(this, rowname, "gui/transp.png", RichText(UString("Order Carlyle Tank for ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString(":")), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BV_Cl, Click_BV_Carlyle, NULL, NULL, NULL, NULL, UNIT_TRUCK));
	
		sprintf(rowname, "%d %d", row, 1);
		char clabel[64];
		sprintf(clabel, "%d", b->manufprc[UNIT_CARLYLE]);

		if(owned)
			//m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, Change_BV_MP, NULL, UNIT_CARLYLE));
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl, false, 6, NULL, NULL, UNIT_CARLYLE));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(clabel)), MAINFONT16, Resize_BV_Cl));

		row++;
	}
	
	//stocked goods list
	RichText sl;
	sl.m_part.push_back(RichPart("Stocked goods list:\n"));
	bool somestock = false;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		Resource* r = &g_resource[ri];

		if(ri == RES_LABOUR || !r->capacity)
		{
			if(b->stocked[ri] <= 0)
				continue;

			somestock = true;

			char num[32];
			std::string stocked = iform(b->stocked[ri]);
			sprintf(num, "%s\n", stocked.c_str());

			Resource* r = &g_resource[ri];
		
			sl.m_part.push_back(RichPart("      "));
			sl.m_part.push_back(RichPart(r->name.c_str()));
			sl.m_part.push_back(RichPart(" "));
			sl.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));
			sl.m_part.push_back(RichPart(": "));
			sl.m_part.push_back(RichPart(num));
		}
		else
		{
			int num = 0;
			int denom = 0;

			if(bt->output[ri] > 0)
			{
				int minlevel = b->maxprod();
				num = bt->output[ri] * b->cymet / RATIO_DENOM;
				num = imax(num, bt->output[ri] * minlevel / RATIO_DENOM);
				denom = num;
			}
			
			if(ri == RES_HOUSING && 
				bt->output[ri] > 0)
			{
				num -= b->occupier.size();
			}

			for(auto csit=b->capsup.begin(); csit!=b->capsup.end(); csit++)
			{
				if(csit->rtype != ri)
					continue;

#if 0
				if(b->type == BL_CHEMPLANT)
				{
					char msg[128];
					sprintf(msg, ">>dst:%d src:%d r:%s this:%d", (int)csit->dst, (int)csit->src, g_resource[ri].name.c_str(), bi);
					InfoMess("...",msg);
				}
#endif

				if(csit->dst == bi)
				{
					num += csit->amt;
					denom += csit->amt;

#if 0
					char msg[128];
					sprintf(msg, "numdenum+=%d", csit->amt);
					if(b->type == BL_CHEMPLANT)
						InfoMess("...",msg);
#endif
				}

				if(csit->src == bi)
				{
					num -= csit->amt;
				}
			}

			if(num == 0 && denom == 0 && bt->output[ri] <= 0)
				continue;

			somestock = true;

			char str[32];
			if(bt->output[ri] <= 0)
			{
				std::string nums = iform(num);
				sprintf(str, "%s\n", nums.c_str());
			}
			else
			{
				std::string nums = iform(num);
				std::string denoms = iform(denom);
				sprintf(str, "%s/%s\n", nums.c_str(), denoms.c_str());
			}

			Resource* r = &g_resource[ri];
		
			sl.m_part.push_back(RichPart("      "));
			sl.m_part.push_back(RichPart(r->name.c_str()));
			sl.m_part.push_back(RichPart(" "));
			sl.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));
			sl.m_part.push_back(RichPart(": "));
			sl.m_part.push_back(RichPart(str));
		}
	}

	if(!somestock)
		sl.m_part.push_back(RichPart("      (None)"));

	//InfoMess("sl", sl.rawstr().c_str());

	m_subwidg.push_back(new TextBlock(this, "req list", sl, MAINFONT16, Resize_BV_SL, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

	reframe();
}
