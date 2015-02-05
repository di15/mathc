

#include "../platform.h"
#include "shader.h"
#include "../gui/font.h"
#include "../math/matrix.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "../sim/player.h"
#include "../math/vec4f.h"
#include "../sim/simflow.h"
#include "../sim/simdef.h"
#include "../math/frustum.h"
#include "fogofwar.h"

//draw unit/bl info overlay
void DrawOv(Matrix* mvp)
{
	glDisable(GL_DEPTH_TEST);
	const float color[] = {1,1,1,1};

	Player* py = &g_player[g_localP];

	//show lack-of-conduit-access hovering icons
	if(g_zoom > 0.1f)
		for(int i=0; i<BUILDINGS; i++)
		{
			Building* b = &g_building[i];

			if(!b->on)
				continue;

			//if(!b->finished)
			//	continue;

			if(!g_frustum.pointin(b->drawpos.x, b->drawpos.y, b->drawpos.z))
				continue;
			
			int tx = b->tilepos.x;
			int ty = b->tilepos.y;

			if(!IsTileVis(g_localP, tx, ty))
				continue;

			Vec4f screenpos = ScreenPos(mvp, b->drawpos, g_width, g_height, true);
		
			bool showcd[CONDUIT_TYPES];
			memset(showcd, -1, sizeof(showcd));
			int showcnt = CONDUIT_TYPES;
			BlType* bt = &g_bltype[b->type];

			//now generic and moddable

			for(int ci=0; ci<CONDUIT_TYPES; ci++)
			{
				//does this bltype use this cdtype?
				bool usescd = false;

				for(int ri=0; ri<RESOURCES; ri++)
				{
					Resource* r = &g_resource[ri];

					if(r->conduit != ci)
						continue;

					if(b->finished &&
						bt->input[ri] <= 0)
						continue;
				
					if(!b->finished &&
						bt->conmat[ri] <= 0)
						continue;

					usescd = true;
					break;
				}

				if(!usescd)
				{
					showcnt--;
					showcd[ci] = false;
					continue;
				}

				CdType* ct = &g_cdtype[ci];

				if(ct->blconduct)
				{
					short& netw = *(short*)(((char*)b)+ct->netwoff);
					if(netw >= 0)
					{
						showcnt--;
						showcd[ci] = false;
					}
				}
				else
				{
					std::list<short>* netw = (std::list<short>*)(((char*)b)+ct->netwoff);
					if(netw->size() > 0)
					{
						showcnt--;
						showcd[ci] = false;
					}
				}
			}

			if(showcnt <= 0)
				continue;

			int x = screenpos.x - (25 * showcnt)/2;
			int y = screenpos.y + 25;

			for(int ci=0; ci<CONDUIT_TYPES; ci++)
			{
				if(!showcd[ci])
					continue;

				CdType* ct = &g_cdtype[ci];
				Texture* tex = &g_texture[ct->lacktex];
				DrawImage(tex->texname, x, y, x+25, y+25);

				x += 25;
			}
		}

	EndS();
	
	UseS(SHADER_COLOR2D);
	Shader* s = &g_shader[g_curS];
	glUniform1f(s->m_slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->m_slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(s->m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	//show production met % and next cycle count down timer and HP
	if(g_zoom > 0.09f)
		for(int i=0; i<BUILDINGS; i++)
		{
			Building* b = &g_building[i];

			if(!b->on)
				continue;

			if(!b->finished)
				continue;

			if(!g_frustum.pointin(b->drawpos.x, b->drawpos.y, b->drawpos.z))
				continue;
			
			int tx = b->tilepos.x;
			int ty = b->tilepos.y;

			if(!IsTileVis(g_localP, tx, ty))
				continue;

			Vec4f screenpos = ScreenPos(mvp, b->drawpos, g_width, g_height, true);
		
#if 0
			std::string t;
			char add[64];
			sprintf(add, "%%%d met\n", b->cymet);
			t += add;
			sprintf(add, "%0.1f next\n", (CYCLE_FRAMES - (g_simframe - b->lastcy))/(float)SIM_FRAME_RATE);
			t += add;
			RichText rt(UString(t.c_str()));
			//DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, &rt);

			if(g_zoom > 0.1f)
				DrawBoxShadText(MAINFONT16, screenpos.x - 16, screenpos.y - 16, 128, 128, &rt, color, 0, -1);
			else if(g_zoom > 0.04f)
				DrawBoxShadText(MAINFONT8, screenpos.x - 16, screenpos.y - 8, 128, 128, &rt, color, 0, -1);
#else
			BlType* bt = &g_bltype[b->type];
			
			float pos[4];

			//HP
			pos[0] = screenpos.x - bt->maxhp / 100 / 2;
			pos[1] = screenpos.y;
			pos[2] = pos[0] + bt->maxhp / 100;
			pos[3] = pos[1] + 2;
			DrawSquare(0, 0, 0, 1, pos[0], pos[1], pos[2], pos[3]);
			pos[2] = pos[0] + b->hp / 100;
			DrawSquare(1.0f, 0.2f, 0.2f, 1.0f, pos[0], pos[1], pos[2], pos[3]);

			//production met %
			pos[1] = pos[3] + 1;
			pos[3] = pos[1] + 2;
			pos[0] = screenpos.x - b->prodlevel / 2 / 5;
			pos[2] = pos[0] + b->prodlevel / 5;
			DrawSquare(0, 0, 0, 1, pos[0], pos[1], pos[2], pos[3]);
			pos[2] = pos[0] + b->cymet / 5;
			DrawSquare(0.2f, 1.0f, 0.2f, 1.0f, pos[0], pos[1], pos[2], pos[3]);

			//cycle count down timer
			pos[1] = pos[3] + 1;
			pos[3] = pos[1] + 2;
			pos[0] = screenpos.x - 30;
			pos[2] = pos[0] + 60;
			DrawSquare(0, 0, 0, 1, pos[0], pos[1], pos[2], pos[3]);
			pos[2] = pos[0] + (g_simframe-b->lastcy)/SIM_FRAME_RATE;
			DrawSquare(0.2f, 0.2f, 1.0f, 1.0f, pos[0], pos[1], pos[2], pos[3]);

#endif
		}

	EndS();
	Ortho(g_width, g_height, color[0], color[1], color[2], color[3]);

	if(g_zoom < 0.6f)
		goto end;

	//show close up unit stats
	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->type != UNIT_LABOURER && u->type != UNIT_TRUCK)
			continue;
		
		if(u->hidden())
			continue;

		if(!g_frustum.pointin(u->drawpos.x, u->drawpos.y, u->drawpos.z))
			continue;
		
		int tx = u->cmpos.x / TILE_SIZE;
		int ty = u->cmpos.y / TILE_SIZE;

		if(!IsTileVis(g_localP, tx, ty))
			continue;

		Vec4f screenpos = ScreenPos(mvp, u->drawpos, g_width, g_height, true);
		RichText rt;

		std::string mode;
		Building* b;
		BlType* bt;
		CdType* ct;

		switch(u->mode)
		{
		case UMODE_NONE:
			mode = "Idle";	
			break;
		case UMODE_GOBLJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "To job (" + std::string(bt->name) + ")";	
			break;
		case UMODE_BLJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "At job (" + std::string(bt->name) + ")";	
			break;
		case UMODE_GOCSTJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "To job (" + std::string(bt->name) + " construction)";
			break;
		case UMODE_CSTJOB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "At job (" + std::string(bt->name) + " construction)";
			break;
		case UMODE_GOCDJOB:
			ct = &g_cdtype[u->cdtype];
			mode = "To job (" + std::string(ct->name) + " construction)";
			break;
		case UMODE_CDJOB:
			ct = &g_cdtype[u->cdtype];
			mode = "At job (" + std::string(ct->name) + " construction)";
			break;
		case UMODE_GOSHOP:
			mode = "To store";
			break;
		case UMODE_SHOPPING:
			mode = "At store";
			break;
		case UMODE_GOREST:
			mode = "To home";
			break;
		case UMODE_RESTING:
			mode = "At home";
			break;
		case UMODE_GODRIVE:
			mode = "To job (Trucking)";
			break;
		case UMODE_DRIVE:
			mode = "At job (Trucking)";
			break;
		case UMODE_GOSUP:
			b = &g_building[u->supplier];
			bt = &g_bltype[b->type];
			mode = "To supplier (" + std::string(bt->name) + ")";
			break;
		case UMODE_GODEMB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "To demander (" + std::string(bt->name) + ")";
			break;
		case UMODE_GOREFUEL:
			mode = "To refuel";
			break;
		case UMODE_REFUELING:
			mode = "Refueling";
			break;
		case UMODE_ATDEMB:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "Offloading (" + std::string(bt->name) + ")";
			break;
		case UMODE_ATSUP:
			b = &g_building[u->target];
			bt = &g_bltype[b->type];
			mode = "Loading up (" + std::string(bt->name) + ")";
			break;
		case UMODE_GODEMCD:
			ct = &g_cdtype[u->cdtype];
			mode = "To demander (" + std::string(ct->name) + ")";
			break;
		case UMODE_ATDEMCD:
			ct = &g_cdtype[u->cdtype];
			mode = "Offloading (" + std::string(ct->name) + ")";
			break;
		default: 
			mode = "?";
			break;
		}

		//mode += "\n";

		rt.m_part.push_back(RichPart(UString(mode.c_str())));
		DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y, &rt);
		//DrawBoxShadText(MAINFONT8, screenpos.x, screenpos.y, g_width, g_height, &rt, color, 0, -1);
		
		RichText rt2;
		
		if(u->type == UNIT_LABOURER)
		{
			char food[16];
			char labour[16];
			char funds[16];

			Resource* foodr = &g_resource[RES_RETFOOD];
			Resource* labourr = &g_resource[RES_LABOUR];
			Resource* fundsr = &g_resource[RES_DOLLARS];
			
			sprintf(food, "%d \n", u->belongings[RES_RETFOOD]);
			sprintf(labour, "%d \n", u->belongings[RES_LABOUR]);
			sprintf(funds, "%d \n", u->belongings[RES_DOLLARS]);
			
			rt2.m_part.push_back(RichPart(RICHTEXT_ICON, foodr->icon));
			rt2.m_part.push_back(RichPart(UString(food)));
			rt2.m_part.push_back(RichPart(RICHTEXT_ICON, labourr->icon));
			rt2.m_part.push_back(RichPart(UString(labour)));
			rt2.m_part.push_back(RichPart(RICHTEXT_ICON, fundsr->icon));
			rt2.m_part.push_back(RichPart(UString(funds)));
			
#if 0
			char add[32];
			sprintf(add, "t%d,t%d fr%d,%d\n", (int)(u->target), u->target2, u->cmpos.x/TILE_SIZE, u->cmpos.y/TILE_SIZE);
			rt2.m_part.push_back(RichPart(UString(add)));
#endif
		}
		else if(u->type == UNIT_TRUCK)
		{	
			char fuel[16];
		
			Resource* fuelr = &g_resource[RES_FUEL];

			sprintf(fuel, "%d \n", u->belongings[RES_FUEL]);
			
			rt2.m_part.push_back(RichPart(RICHTEXT_ICON, fuelr->icon));
			rt2.m_part.push_back(RichPart(UString(fuel)));

			if(u->cargotype >= 0 && u->cargoamt > 0)
			{
				char carry[16];
				
				Resource* carryr = &g_resource[u->cargotype];

				sprintf(carry, "%d \n", u->cargoamt);

				rt2.m_part.push_back(RichPart(RICHTEXT_ICON, carryr->icon));
				rt2.m_part.push_back(RichPart(UString(carry)));
			}

#if 0
			char add[16];
			sprintf(add, "this%d\n", (int)(u-g_unit));
			rt2.m_part.push_back(RichPart(UString(add)));
#endif
		}

		Font* f = &g_font[MAINFONT8];
		//DrawCenterShadText(MAINFONT8, screenpos.x, screenpos.y + f->gheight, &rt2);
		DrawBoxShadText(MAINFONT8, screenpos.x - f->gheight*2, screenpos.y + f->gheight, g_width, g_height, &rt2, color, 0, -1);
	}

	//RichText rt("lkajslkdlads");
	//DrawCenterShadText(MAINFONT32, 100,100, &rt);

end:
	
	glEnable(GL_DEPTH_TEST);
}