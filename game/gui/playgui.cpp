#include "../gmain.h"
#include "../../common/gui/gui.h"
#include "../../common/gui/widgets/windoww.h"
#include "../keymap.h"
#include "../../common/render/heightmap.h"
#include "../../common/render/transaction.h"
#include "../../common/math/camera.h"
#include "../../common/render/shadow.h"
#include "../../common/render/screenshot.h"
#include "../../common/save/savemap.h"
#include "ggui.h"
#include "playgui.h"
#include "../../common/gui/icon.h"
#include "../../common/gui/widgets/spez/resticker.h"
#include "../../common/gui/widgets/spez/botpan.h"
#include "../../common/gui/widgets/spez/blpreview.h"
#include "../../common/gui/widgets/spez/blview.h"
#include "../../common/gui/widgets/spez/cstrview.h"
#include "../../common/gui/widgets/spez/svlist.h"
#include "../../common/gui/widgets/spez/saveview.h"
#include "../../common/gui/widgets/spez/loadview.h"
#include "../../common/gui/widgets/spez/truckmgr.h"
#include "gviewport.h"
#include "../../common/sim/bltype.h"
#include "../../common/sim/building.h"
#include "../../common/sim/road.h"
#include "../../common/sim/powl.h"
#include "../../common/sim/crpipe.h"
#include "../../common/sim/player.h"
#include "../../common/sim/conduit.h"
#include "../../common/sim/simflow.h"
#include "chattext.h"

void Resize_ResNamesTextBlock(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_ResAmtsTextBlock(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = 150;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_ResDeltasTextBlock(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = 250;
	thisw->m_pos[1] = 10;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Out_Build()
{
}

void Resize_ResTicker(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_font[MAINFONT16].gheight+5;
	thisw->m_tpos[0] = 0;
	thisw->m_tpos[1] = 0;
}

void UpdResTicker()
{
	//return;

	static float tickerpos = 0;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");
	ResTicker* restickerw = (ResTicker*)playview->get("res ticker");
	Widget* restickertw = &restickerw->restext;
	RichText restext;

#if 0
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_DOLLARS));
	restext.m_part.push_back(RichPart(" Funds: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_HOUSING));
	restext.m_part.push_back(RichPart(" Housing: 100/120"));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_FARMPRODUCT));
	restext.m_part.push_back(RichPart(" Farm Products: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_RETFOOD));
	restext.m_part.push_back(RichPart(" Retail Food: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_PRODUCTION));
	restext.m_part.push_back(RichPart(" Production: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_IRONORE));
	restext.m_part.push_back(RichPart(" Minerals: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_CRUDEOIL));
	restext.m_part.push_back(RichPart(" Crude Oil: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_WSFUEL));
	restext.m_part.push_back(RichPart(" Wholesale Fuel: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_RETFUEL));
	restext.m_part.push_back(RichPart(" Retail Fuel: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_ENERGY));
	restext.m_part.push_back(RichPart(" Energy: 100/120"));
	restext.m_part.push_back(RichPart("    "));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_ENRICHEDURAN));
	restext.m_part.push_back(RichPart(" Uranium: 100 +1/"));
	restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
	restext.m_part.push_back(RichPart("    "));
#else
	for(int i=0; i<RESOURCES; i++)
	{
		if(i == RES_LABOUR)
			continue;

		Resource* r = &g_resource[i];
		
		if(r->capacity && py->local[i] <= 0)
			continue;
		else if(!r->capacity && py->local[i] + py->global[i] <= 0)
			continue;

		restext.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));

		char cstr1[128];
		sprintf(cstr1, " %s: ", r->name.c_str());
		char cstr2[64];

#if 0	//with reschanges?
		if(r->capacity)
			sprintf(cstr2, "%d/%d", py->local[i], py->global[i]);
		else
			sprintf(cstr2, "%d %+d/", py->local[i] + py->global[i], py->resch[i]);

		strcat(cstr1, cstr2);
		restext.m_part.push_back(RichPart(cstr1));

		if(!r->capacity)
			restext.m_part.push_back(RichPart(RICHTEXT_ICON, ICON_TIME));
#else	
#if 0
		if(r->capacity)
			sprintf(cstr2, "%d/%d", py->local[i], py->global[i]);
		else
			sprintf(cstr2, "%d", py->local[i] + py->global[i]);
#else
		if(r->capacity)
		{
			std::string local = iform(py->local[i]);
			std::string global = iform(py->global[i]);
			sprintf(cstr2, "%s/%s", local.c_str(), global.c_str());
		}
		else
		{
			std::string total = iform(py->local[i] + py->global[i]);
			sprintf(cstr2, "%s", total.c_str());
		}
#endif

		strcat(cstr1, cstr2);
		restext.m_part.push_back(RichPart(cstr1));
#endif

		restext.m_part.push_back(RichPart("    "));
	}
#endif

	int len = restext.texlen();

	tickerpos += 0.5f * g_drawfrinterval * 20;

	if((int)tickerpos > len)
		tickerpos = 0;

	int endx = EndX(&restext, restext.rawlen(), MAINFONT16, 0, 0);

	if(endx > g_width)
	{
		RichText restext2 = restext.substr((int)tickerpos, len-(int)tickerpos) + restext.substr(0, (int)tickerpos);
		restickertw->m_text = restext2;
	}
	else
		restickertw->m_text = restext;
}

void Resize_BottomPanel(Widget* thisw)
{
	Player* py = &g_player[g_localP];

	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32);
	thisw->m_pos[2] = (float)g_width;
	thisw->m_pos[3] = (float)g_height;
}

void Out_BuildButton()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->close("bl preview");
}

void Click_BuildButton(int bwhat)
{
	Player* py = &g_player[g_localP];
	g_build = bwhat;
	//g_log<<"b "<<g_build<<std::endl;
	//char msg[128];
	//sprintf(msg, "b %d", bwhat);
	//InfoMess("t", msg);
	Out_BuildButton();
}

void Over_BuildButton(int bwhat)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	if(gui->get("cstr view")->m_opened)
		return;
	
	if(gui->get("bl view")->m_opened)
		return;

	g_bptype = bwhat;

	gui->open("bl preview");
	BuildPreview* bp = (BuildPreview*)gui->get("bl preview");
	Text* tl = (Text*)bp->get("title");

	std::string bname;
	RichText cb;	//conmat block
	RichText ib;	//inputs block
	RichText ob;	//outputs block
	RichText db;	//description block

	TextBlock* cbw = (TextBlock*)bp->get("conmat block");
	TextBlock* ibw = (TextBlock*)bp->get("input block");
	TextBlock* obw = (TextBlock*)bp->get("output block");
	TextBlock* dbw = (TextBlock*)bp->get("desc block");

	cb.m_part.push_back(RichPart(UString("CONSTRUCTION REQUISITES:")));

	if(bwhat < 0)
		;
	else if(bwhat < BL_TYPES)
	{
		ib.m_part.push_back(RichPart(UString("INPUTS:")));
		ob.m_part.push_back(RichPart(UString("OUTPUTS:")));

		BlType* bt = &g_bltype[bwhat];
		bname = bt->name;

		db.m_part.push_back(RichPart(UString(bt->desc.c_str())));

		int ns = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->conmat[ri] <= 0)
				continue;

			ns ++;
			Resource* r = &g_resource[ri];

			cb.m_part.push_back(RichPart(UString("\n")));
			cb.m_part.push_back(RichPart(UString(r->name.c_str())));
			cb.m_part.push_back(RichPart(UString(" ")));
			cb.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));
			cb.m_part.push_back(RichPart(UString(": ")));

			char num[32];
			sprintf(num, "%d", bt->conmat[ri]);
			cb.m_part.push_back(RichPart(UString(num)));
		}

		if(ns <= 0)
			cb.m_part.push_back(RichPart(UString("\nNone")));

		int ni = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->input[ri] <= 0)
				continue;

			ni ++;
			Resource* r = &g_resource[ri];

			ib.m_part.push_back(RichPart(UString("\n")));
			ib.m_part.push_back(RichPart(UString(r->name.c_str())));
			ib.m_part.push_back(RichPart(UString(" ")));
			ib.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));
			ib.m_part.push_back(RichPart(UString(": ")));

			char num[32];
			sprintf(num, "%d", bt->input[ri]);
			ib.m_part.push_back(RichPart(UString(num)));
		}

		if(ni <= 0)
			ib.m_part.push_back(RichPart(UString("\nNone")));

		int no = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(bt->output[ri] <= 0)
				continue;

			no ++;
			Resource* r = &g_resource[ri];

			ob.m_part.push_back(RichPart(UString("\n")));
			ob.m_part.push_back(RichPart(UString(r->name.c_str())));
			ob.m_part.push_back(RichPart(UString(" ")));
			ob.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));
			ob.m_part.push_back(RichPart(UString(": ")));

			char num[32];
			sprintf(num, "%d", bt->output[ri]);
			ob.m_part.push_back(RichPart(UString(num)));
		}

		if(no <= 0)
			ob.m_part.push_back(RichPart(UString("\nNone")));
	}
	else if(bwhat < BL_TYPES + CONDUIT_TYPES)
	{
		CdType* ct = &g_cdtype[bwhat - BL_TYPES];
		bname = ct->name;

		db.m_part.push_back(RichPart(UString(ct->desc.c_str())));

		int ns = 0;
		for(int ri=0; ri<RESOURCES; ri++)
		{
			if(ct->conmat[ri] <= 0)
				continue;

			ns ++;
			Resource* r = &g_resource[ri];

			cb.m_part.push_back(RichPart(UString("\n")));
			cb.m_part.push_back(RichPart(UString(r->name.c_str())));
			cb.m_part.push_back(RichPart(UString(" ")));
			cb.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));
			cb.m_part.push_back(RichPart(UString(": ")));

			char num[32];
			sprintf(num, "%d", ct->conmat[ri]);
			cb.m_part.push_back(RichPart(UString(num)));
		}

		if(ns <= 0)
			cb.m_part.push_back(RichPart(UString("\nNone")));
	}

	tl->m_text = RichText(UString(bname.c_str()));
	cbw->m_text = cb;
	ibw->m_text = ib;
	obw->m_text = ob;
	dbw->m_text = db;

	g_bpcam.position(TILE_SIZE*3, TILE_SIZE*3, TILE_SIZE*3, 0, 0, 0, 0, 1, 0);
}

void Click_NextBuildButton(int nextpage)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

	for(int i=0; i<9; i++)
		bp->bottomright_button_on[i] = false;

	if(nextpage == 1)
		BuildMenu_OpenPage1();
	else if(nextpage == 2)
		BuildMenu_OpenPage2();
	else if(nextpage == 3)
		BuildMenu_OpenPage3();
}

void BuildMenu_OpenPage1()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

#if 0
	Button(Widget* parent, const char* filepath, const RichText t, int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm)
#endif

#if 0	//with gas station
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_APARTMENT);
	bp->bottomright_button_on[0] = true;

	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/store1.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_STORE);
	bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/farm2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FARM);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/oilwell2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILWELL);
	bp->bottomright_button_on[3] = true;

	bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/refinery2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_REFINERY);
	bp->bottomright_button_on[4] = true;

	bp->bottomright_button[5] = Button(bp, "name", "gui/brbut/gasstation2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_GASSTATION);
	bp->bottomright_button_on[5] = true;

	bp->bottomright_button[6] = Button(bp, "name", "gui/brbut/mine.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_MINE);
	bp->bottomright_button_on[6] = true;

	bp->bottomright_button[7] = Button(bp, "name", "gui/brbut/factory3.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FACTORY);
	bp->bottomright_button_on[7] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 2);
	bp->bottomright_button_on[8] = true;
#else
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/apartment2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_APARTMENT);
	bp->bottomright_button_on[0] = true;

	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/store1.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_STORE);
	bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/farm2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FARM);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/oilwell2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_OILWELL);
	bp->bottomright_button_on[3] = true;

	bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/refinery2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_REFINERY);
	bp->bottomright_button_on[4] = true;

	bp->bottomright_button[5] = Button(bp, "name", "gui/brbut/mine.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_MINE);
	bp->bottomright_button_on[5] = true;

	bp->bottomright_button[6] = Button(bp, "name", "gui/brbut/factory3.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_FACTORY);
	bp->bottomright_button_on[6] = true;
	
	bp->bottomright_button[7] = Button(bp, "name", "gui/brbut/nucpow2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_NUCPOW);
	bp->bottomright_button_on[7] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 2);
	bp->bottomright_button_on[8] = true;
#endif

	bp->reframe();
}


void BuildMenu_OpenPage2()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

#if 0 //with gas station , c1
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/nucpow2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_NUCPOW);
	bp->bottomright_button_on[0] = true;

	//bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/harbour2.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_HARBOUR);
	//bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/road.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_ROAD);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/crudepipeline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CRPIPE);
	bp->bottomright_button_on[3] = true;

	bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/powerline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_POWL);
	bp->bottomright_button_on[4] = true;

	bp->bottomright_button[5] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 1);
	bp->bottomright_button_on[5] = true;
#else
	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/coalpow.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_COALPOW);
	bp->bottomright_button_on[0] = true;
	
	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/chemplant.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CHEMPLANT);
	bp->bottomright_button_on[1] = true;
	
	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/elecplant.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_ELECPLANT);
	bp->bottomright_button_on[2] = true;
	
	bp->bottomright_button[3] = Button(bp, "name", "gui/brbut/cemplant.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CEMPLANT);
	bp->bottomright_button_on[3] = true;

	bp->bottomright_button[4] = Button(bp, "name", "gui/brbut/quarry.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_QUARRY);
	bp->bottomright_button_on[4] = true;
	
	bp->bottomright_button[5] = Button(bp, "name", "gui/brbut/smelter.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_SMELTER);
	bp->bottomright_button_on[5] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 3);
	bp->bottomright_button_on[8] = true;
#endif

	bp->reframe();
}

void BuildMenu_OpenPage3()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	BotPan* bp = (BotPan*)playview->get("bottom panel");

	bp->bottomright_button[0] = Button(bp, "name", "gui/brbut/road.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_ROAD);
	bp->bottomright_button_on[0] = true;

	bp->bottomright_button[1] = Button(bp, "name", "gui/brbut/crudepipeline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_CRPIPE);
	bp->bottomright_button_on[1] = true;

	bp->bottomright_button[2] = Button(bp, "name", "gui/brbut/powerline.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_BuildButton, NULL, Over_BuildButton, Out_BuildButton, BL_POWL);
	bp->bottomright_button_on[2] = true;

	bp->bottomright_button[8] = Button(bp, "name", "gui/next.png", RichText(""), RichText(""), MAINFONT8, BUST_CORRODE, NULL, NULL, Click_NextBuildButton, NULL, NULL, NULL, 1);
	bp->bottomright_button_on[8] = true;

	bp->reframe();
}

void Resize_BuildPreview(Widget* thisw)
{
	Player* py = &g_player[g_localP];

#if 0
	int centerx = g_width/2;
	int centery = g_height/2;

	thisw->m_pos[0] = centerx-200;
	thisw->m_pos[1] = centery-200;
	thisw->m_pos[2] = centerx+200;
	thisw->m_pos[3] = centery+200;
#elif 1
	thisw->m_pos[0] = g_width - 400 - 64;
	thisw->m_pos[1] = g_height - MINIMAP_SIZE - 60 - 320;
	thisw->m_pos[2] = g_width - 64;
	thisw->m_pos[3] = g_height - MINIMAP_SIZE - 75;
#endif
}

void Resize_ConstructionView(Widget* thisw)
{
	Player* py = &g_player[g_localP];

	thisw->m_pos[0] = g_width/2 - 250;
	thisw->m_pos[1] = g_height - MINIMAP_SIZE - 32 - 300;
	thisw->m_pos[2] = g_width/2 + 100;
	thisw->m_pos[3] = g_height - MINIMAP_SIZE - 32 - 100;
}

void Click_MoveConstruction()
{
	int alloced[RESOURCES];
	Zero(alloced);
	int totalloc = 0;

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	for(auto siter = g_sel.buildings.begin(); siter != g_sel.buildings.end(); siter++)
	{
		int bi = *siter;
		Building* b = &g_building[bi];

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += b->conmat[i];
			totalloc += b->conmat[i];
		}
	}

#if 0
	for(auto siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
	{
		Vec2i tpos = *siter;
		RoadTile* r = RoadAt(tpos.x, tpos.y);

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += r->conmat[i];
			totalloc += r->conmat[i];
		}
	}

	for(auto siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
	{
		Vec2i tpos = *siter;
		PowlTile* p = PowlAt(tpos.x, tpos.y);

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += p->conmat[i];
			totalloc += p->conmat[i];
		}
	}

	for(auto siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
	{
		Vec2i tpos = *siter;
		CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);

		for(int i=0; i<RESOURCES; i++)
		{
			alloced[i] += p->conmat[i];
			totalloc += p->conmat[i];
		}
	}
#endif

	if(totalloc <= 0)
	{
		for(auto siter = g_sel.buildings.begin(); siter != g_sel.buildings.end(); siter++)
		{
			int bi = *siter;
			Building* b = &g_building[bi];
			b->on = false;
		}

#if 0
		for(auto siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
		{
			Vec2i tpos = *siter;
			RoadTile* r = RoadAt(tpos.x, tpos.y);
			r->on = false;
		}

		for(auto siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
		{
			Vec2i tpos = *siter;
			PowlTile* p = PowlAt(tpos.x, tpos.y);
			p->on = false;
		}

		for(auto siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
		{
			Vec2i tpos = *siter;
			CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);
			p->on = false;
		}
#endif

		ClearSel(&g_sel);
		gui->close("cstr view");
	}
	else
	{
		ShowMessage(RichText("You've already invested resources in this project."));
	}
}

void Click_CancelConstruction()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	for(auto siter = g_sel.buildings.begin(); siter != g_sel.buildings.end(); siter++)
	{
		int bi = *siter;
		Building* b = &g_building[bi];
		b->on = false;
	}

#if 0
	for(auto siter = g_sel.roads.begin(); siter != g_sel.roads.end(); siter++)
	{
		Vec2i tpos = *siter;
		RoadTile* r = RoadAt(tpos.x, tpos.y);
		r->on = false;
	}

	for(auto siter = g_sel.powls.begin(); siter != g_sel.powls.end(); siter++)
	{
		Vec2i tpos = *siter;
		PowlTile* p = PowlAt(tpos.x, tpos.y);
		p->on = false;
	}

	for(auto siter = g_sel.crpipes.begin(); siter != g_sel.crpipes.end(); siter++)
	{
		Vec2i tpos = *siter;
		CrPipeTile* p = CrPipeAt(tpos.x, tpos.y);
		p->on = false;
	}
#endif

	ClearSel(&g_sel);
	gui->close("cstr view");
}

void Click_ProceedConstruction()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	ClearSel(&g_sel);
	gui->close("cstr view");
}

void Click_EstimateConstruction()
{
	Player* py = &g_player[g_localP];
	ClearSel(&g_sel);
	GUI* gui = &g_gui;
	gui->close("cstr view");
}

void Resize_Message(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = (float)(g_width/2 - 200);
	thisw->m_pos[1] = (float)(g_height/2 - 100);
	thisw->m_pos[2] = (float)(g_width/2 + 200);
	thisw->m_pos[3] = (float)(g_height/2 + 100);
}

void Resize_MessageContinue(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = (float)(g_width/2 - 80);
	thisw->m_pos[1] = (float)(g_height/2 + 70);
	thisw->m_pos[2] = (float)(g_width/2 + 80);
	thisw->m_pos[3] = (float)(g_height/2 + 90);
}

void Click_MessageContinue()
{
#if 0
	auto viter = gui->view.begin();
	while(viter != gui->view.end())
	{
		if(stricmp(viter->name.c_str(), "message view") == 0)
		{
			InfoMess("f", "view found");

			viter = gui->view.erase(viter);

			continue;
		}

		viter++;
	}
#endif

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->close("message view");
}

void ShowMessage(const RichText& msg)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* msgview = (ViewLayer*)gui->get("message view");
	TextBlock* msgblock = (TextBlock*)msgview->get("message");
	msgblock->m_text = msg;
	gui->open("message view");
}

void Resize_Window(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = (float)(g_width/2 - 200);
	thisw->m_pos[1] = (float)(g_height/2 - 200);
	thisw->m_pos[2] = (float)(g_width/2 + 200);
	thisw->m_pos[3] = (float)(g_height/2 + 200);
}

void Resize_DebugLines(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	thisw->m_pos[0] = (float)(0);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Transx(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	int i = 1;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Save(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	int i = 2;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_QSave(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	int i = 3;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Load(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	int i = 4;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Pause(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	int i = 5;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Play(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	int i = 6;
	thisw->m_pos[0] = (float)(0 + 32*i);
	thisw->m_pos[1] = (float)(g_height - MINIMAP_SIZE - 32 - 32);
	thisw->m_pos[2] = (float)(32 + 32*i);
	thisw->m_pos[3] = (float)(g_height - MINIMAP_SIZE - 32);
}

void Resize_Fast(Widget* thisw)
{
	Player* py = &g_player[g_localP];
	int i = 7;
	thisw->m_pos[0] = 0 + 32*i;
	thisw->m_pos[1] = g_height - MINIMAP_SIZE - 32 - 32;
	thisw->m_pos[2] = 32 + 32*i;
	thisw->m_pos[3] = g_height - MINIMAP_SIZE - 32;
}
	
void Click_Pause()
{
	g_speed = SPEED_PAUSE;
}

void Click_Play()
{
	g_speed = SPEED_PLAY;
}

void Click_Fast()
{
	g_speed = SPEED_FAST;
}

void Click_DebugLines()
{
	g_debuglines = !g_debuglines;
}

void Click_Transx()
{
	g_drawtransx = !g_drawtransx;
}

void Click_Save()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->close("save");
	gui->close("load");
	gui->open("save");
	((SaveView*)gui->get("save"))->regen();
}

void Click_QSave()
{
	if(!g_lastsave[0])
	{
		Click_Save();
		return;
	}

	SaveMap(g_lastsave);
	
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->close("save");
	gui->close("load");
}

void Click_Load()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->close("save");
	gui->close("load");
	gui->open("load");
	((LoadView*)gui->get("load"))->regen();
}

void Click_QuitToMenu()
{
	EndSess();
	FreeMap();
	FreeGrid();
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	gui->closeall();
	gui->open("main");
	g_mode = APPMODE_MENU;
}

void FillPlay()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	g_vptype[VIEWPORT_MINIMAP] = VpType(Vec3f(0, 0, 0), Vec3f(0, 1, 0), "Minimap", true);
	g_vptype[VIEWPORT_ENTVIEW] = VpType(Vec3f(0, MAX_DISTANCE/2, 0), Vec3f(0, 1, 0), "EntView", false);

	//for(int i=0; i<25; i++)
	//Sleep(6000);

	gui->add(new ViewLayer(gui, "play"));
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	//playview->add(new Image(NULL, "gui/backg/white.png", Resize_ResTicker));
	//playview->add(new Text(NULL, "res ticker", RichText(" "), MAINFONT16, Resize_ResTicker, true, 1, 1, 1, 1));
	playview->add(new ResTicker(playview, "res ticker", Resize_ResTicker));
	playview->add(new BotPan(playview, "bottom panel", Resize_BottomPanel));

	playview->add(new Button(playview, "debug lines", "gui/debuglines.png", RichText(), RichText("Show debug info"), MAINFONT16, BUST_LEFTIMAGE, Resize_DebugLines, Click_DebugLines, NULL, NULL, NULL, NULL, -1));
	playview->add(new Button(playview, "show transactions", "gui/transx.png", RichText(), RichText("Show transactions"), MAINFONT16, BUST_LEFTIMAGE, Resize_Transx, Click_Transx, NULL, NULL, NULL, NULL, -1));
	playview->add(new Button(playview, "save", "gui/edsave.png", RichText(), RichText("Save game"), MAINFONT16, BUST_LEFTIMAGE, Resize_Save, Click_Save, NULL, NULL, NULL, NULL, -1));
	playview->add(new Button(playview, "qsave", "gui/qsave.png", RichText(), RichText("Quick save"), MAINFONT16, BUST_LEFTIMAGE, Resize_QSave, Click_QSave, NULL, NULL, NULL, NULL, -1));
	playview->add(new Button(playview, "load", "gui/edload.png", RichText(), RichText("Load game"), MAINFONT16, BUST_LEFTIMAGE, Resize_Load, Click_Load, NULL, NULL, NULL, NULL, -1));
	playview->add(new Button(playview, "pause", "gui/pause.png", RichText(), RichText("Pause"), MAINFONT16, BUST_LEFTIMAGE, Resize_Pause, Click_Pause, NULL, NULL, NULL, NULL, -1));
	playview->add(new Button(playview, "play", "gui/play.png", RichText(), RichText("Play"), MAINFONT16, BUST_LEFTIMAGE, Resize_Play, Click_Play, NULL, NULL, NULL, NULL, -1));
	playview->add(new Button(playview, "fast", "gui/fastforward.png", RichText(), RichText("Fast Forward"), MAINFONT16, BUST_LEFTIMAGE, Resize_Fast, Click_Fast, NULL, NULL, NULL, NULL, -1));

	AddChat(playview);

	//preload all the button images
	BuildMenu_OpenPage3();
	BuildMenu_OpenPage2();
	BuildMenu_OpenPage1();

	//gui->add(new ViewLayer(gui, "cstr view"));
	gui->add(new CstrView(gui, "cstr view", Resize_ConstructionView, Click_MoveConstruction, Click_CancelConstruction, Click_ProceedConstruction, Click_EstimateConstruction));
	//ViewLayer* constrview = (ViewLayer*)gui->get("cstr view");

	//constrview->add(new CstrView(NULL, "cstr view", Resize_ConstructionView, Click_MoveConstruction, Click_CancelConstruction, Click_ProceedConstruction, Click_EstimateConstruction));

	//gui->add(new ViewLayer(gui, "bl preview"));
	gui->add(new BuildPreview(gui, "bl preview", Resize_BuildPreview));
	//ViewLayer* blpreview = (ViewLayer*)gui->get("bl preview");

	//blpreview->add(new TouchListener(NULL, Resize_Fullscreen, NULL, NULL, NULL, -1));
	//blpreview->add(new BuildPreview(blpreview, "bl preview", Resize_BuildPreview));
	//blpreview->add(new WindowW(blpreview, "bl preview", Resize_BuildPreview));
	
	gui->add(new BlView(gui, "bl view", Resize_BuildPreview));
	gui->add(new TruckMgr(gui, "truck mgr", Resize_BuildPreview));

	
	gui->add(new SaveView(gui, "save", Resize_BuildPreview));
	gui->add(new LoadView(gui, "load", Resize_BuildPreview));

#if 0
	gui->add(new ViewLayer(gui, "construction estimate view"));
	ViewLayer* cev = (ViewLayer*)gui->get("construction estimate view");
#endif

	gui->add(new ViewLayer(gui, "ingame"));
	ViewLayer* ingame = (ViewLayer*)gui->get("ingame");
	ingame->add(new Image(ingame, "gui/backg/white.jpg", true, Resize_Fullscreen, 0, 0, 0, 0.5f));
	ingame->add(new Link(ingame, "0", RichText("Quit to Menu"), MAINFONT16, Resize_MenuItem, Click_QuitToMenu));
	ingame->add(new Link(ingame, "1", RichText("Cancel"), MAINFONT16, Resize_MenuItem, Escape));

	gui->add(new ViewLayer(gui, "message view"));
	ViewLayer* msgview = (ViewLayer*)gui->get("message view");

	msgview->add(new Image(NULL, "gui/backg/white.jpg", true, Resize_Message));
	msgview->add(new TextBlock(NULL, "message", RichText(""), MAINFONT16, Resize_Message));
	msgview->add(new TouchListener(NULL, Resize_Fullscreen, NULL, NULL, NULL, -1));
	msgview->add(new Button(NULL, "continue button", "gui/transp.png", RichText("Continue"), RichText(""), MAINFONT16, BUST_LEFTIMAGE, Resize_MessageContinue, Click_MessageContinue, NULL, NULL, NULL, NULL, -1));
}
