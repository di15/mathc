#include "../math/matrix.h"
#include "transaction.h"
#include "../gui/font.h"
#include "../math/vec4f.h"
#include "../window.h"
#include "../utils.h"
#include "../sim/player.h"
#include "../math/frustum.h"
#include "fogofwar.h"
#include "heightmap.h"

std::list<Transaction> g_transx;
bool g_drawtransx = false;

void DrawTransactions(Matrix projmodlview)
{
	//return;

	Player* py = &g_player[g_localP];

	Vec3f* pos;
	Vec4f screenpos;
	int size = (int)g_font[MAINFONT8].gheight;
	float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	auto triter = g_transx.begin();

	while(triter != g_transx.end())
	{
		triter->drawpos.y += TRANSACTION_RISE * g_drawfrinterval;
		triter->life -= TRANSACTION_DECAY * g_drawfrinterval;

		if(triter->life <= 0.0f || _isnan(triter->life))
		{
			triter = g_transx.erase( triter );
			continue;
		}

		triter ++;
	}

	if(!g_drawtransx)
		return;

	triter = g_transx.begin();

	while(triter != g_transx.end())
	{
		pos = &triter->drawpos;

		if(!g_frustum.pointin(pos->x, pos->y, pos->z))
		{
			triter++;
			continue;
		}
		
		int tx = pos->x / TILE_SIZE;
		int ty = pos->y / TILE_SIZE;

		if(!IsTileVis(g_localP, tx, ty))
		{
			triter++;
			continue;
		}

		screenpos.x = pos->x;
		screenpos.y = pos->y;
		screenpos.z = pos->z;
#if	1 //if ortho projection
		screenpos.w = 1;
#endif

		screenpos.transform(projmodlview);
		screenpos = screenpos / screenpos.w;
		screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
		screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

#if 0
		if(_isnan(screenpos.x))
			goto next;
		
		if(_isnan(screenpos.y))
			goto next;
#endif

		int x1 = (int)( screenpos.x - triter->halfwidth );
		int y1 = (int)screenpos.y;
		color[3] = triter->life * 0.9f;

#if 0
		if(x1 < 0)
			goto next;
		if(y1 < 0)
			goto next;
		if(x1 > g_width)
			goto next;
		if(y1 > g_height)
			goto next;
#endif

		//DrawShadowedText(MAINFONT8, x1, y1, &triter->rtext, color);
		DrawBoxShadText(MAINFONT8, x1, y1, g_width, g_height, &triter->rtext, color, 0, -1);
		//DrawCenterShadText(MAINFONT8, x1, y1, &triter->rtext, color, -1);

next:
		triter ++;
	}
}

void NewTransx(Vec3f pos, const RichText* rtext)
{
	Transaction t;
	t.life = 1;
	t.drawpos = pos;
	t.rtext = *rtext;
	t.halfwidth = TextWidth(MAINFONT8, rtext) / 2.0f;
	g_transx.push_back(t);

#if 0
	auto titer = g_transx.rbegin();

	g_log<<"raw str = "<<titer->rtext.rawstr()<<std::endl;
	g_log.flush();
#endif
}

void FreeTransx()
{
	g_transx.clear();
}