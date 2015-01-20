
#include "chattext.h"
#include "../../common/gui/gui.h"
#include "../../common/sim/player.h"

void Resize_ChatLine(Widget* thisw)
{
	Font* f = &g_font[thisw->m_font];
	Player* py = &g_player[g_localP];

	int i = 0;
	sscanf(thisw->m_name.c_str(), "%d", &i);

	float topy = g_height - 200 - CHAT_LINES * f->gheight;

	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = topy + f->gheight * i;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = topy + f->gheight * (i+1);
}

void Resize_ChatPrompt(Widget* thisw)
{
	Font* f = &g_font[thisw->m_font];
	Player* py = &g_player[g_localP];
	int i = CHAT_LINES;
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 30 + f->gheight * i;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = 30 + f->gheight * (i+1);
}

void AddChat(ViewLayer* playview)
{
	for(int i=0; i<CHAT_LINES; i++)
	{
		char name[32];
		sprintf(name, "%d", i);
		playview->add(new Text(playview, name, RichText(), MAINFONT16, Resize_ChatLine, true, 1.0f, 1.0f, 1.0f, 1.0f));
		//TODO get rid of warnings
	}
}

void AddChat(RichText* newl)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* playview = (ViewLayer*)gui->get("play");

	for(int i=0; i<CHAT_LINES-1; i++)
	{
		char name[32];
		sprintf(name, "%d", i);
		
		char name2[32];
		sprintf(name2, "%d", i+1);
		
		Text* text = (Text*)playview->get(name);
		Text* text2 = (Text*)playview->get(name2);

		text->m_text = text2->m_text;
	}

	char name[32];
	sprintf(name, "%d", CHAT_LINES-1);
	Text* text = (Text*)playview->get(name);
	std::string datetime = DateTime();
	//text->m_text = RichText(UString("[")) + RichText(UString(datetime.c_str())) + RichText(UString("] ")) + *newl;
	text->m_text = *newl;
}