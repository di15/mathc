

#include "unit.h"
#include "../render/shader.h"
#include "unittype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../render/model.h"
#include "../math/hmapmath.h"
#include "unitmove.h"
#include "sim.h"
#include "labourer.h"
#include "../debug.h"

Unit g_unit[UNITS];

Unit::Unit()
{
	on = false;
	threadwait = false;
}

Unit::~Unit()
{
	destroy();
}

void Unit::destroy()
{
	on = false;
	threadwait = false;
}

void Unit::resetpath()
{
	path.clear();
	subgoal = cmpos;
	goal = cmpos;
	pathblocked = false;
}

#if 0
void DrawUnits()
{
	Vec3f vertical = c->up2();
	Vec3f horizontal = c->m_strafe;
	Vec3f a, b, c, d;
	Vec3f vert, horiz;

	Shader* s = &g_shader[g_curS];
	
	glActiveTextureARB(GL_TEXTURE0);
	glUniform1iARB(s->m_slot[SSLOT_TEXTURE0], 0);

	//glDisable(GL_CULL_FACE);

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UnitT* t = &g_unitT[u->type];
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_texture[t->texindex].texname);
		glUniform1iARB(s->m_slot[SSLOT_TEXTURE0], 0);

		Vec2i* size = &t->bilbsize;

		vert = vertical*size->y;
		horiz = horizontal*(size->x/2);

		a = u->drawpos + horiz + vert;
		b = u->drawpos + horiz;
		c = u->drawpos - horiz;
		d = u->drawpos - horiz + vert;

		float vertices[] =
		{
			//posx, posy posz   texx, texy
			a.x, a.y, a.z,          1, 0,
			b.x, b.y, b.z,          1, 1,
			c.x, c.y, c.z,          0, 1,

			c.x, c.y, c.z,          0, 1,
			d.x, d.y, d.z,          0, 0,
			a.x, a.y, a.z,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	
	//glEnable(GL_CULL_FACE);
}
#endif

void DrawUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];
		
		if(!u->on)
		{
			continue;
		}

		UnitT* t = &g_unitT[u->type];

		Model* m = &g_model[t->model];
		
		m->draw(u->frame[BODY_LOWER], u->drawpos, u->rotation.y);
	}
}

int NewUnit()
{
	for(int i=0; i<UNITS; i++)
		if(!g_unit[i].on)
			return i;

	return -1;
}

void StartingBelongings(Unit* u)
{
	Zero(u->belongings);
	u->car = -1;
	u->home = -1;

	if(u->type == UNIT_LABOURER)
	{
		if(u->owner >= 0)
		{
			u->belongings[ RES_FUNDS ] = 100;
		}

		u->belongings[ RES_RETFOOD ] = STARTING_RETFOOD;
		u->belongings[ RES_LABOUR ] = STARTING_LABOUR;
	}
}

bool PlaceUnit(int type, Vec3i cmpos, int owner)
{
	int i = NewUnit();

	if(i < 0)
		return false;

#if 0
	bool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec3i cmpos;
	Vec3f facing;
	Vec2f rotation;

	deque<Vec2i> path;
	Vec2i goal;

	int step;
	int target;
	int target2;
	bool targetu;
	bool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	bool passive;
	Vec2i prevpos;
	int taskframe;
	bool pathblocked;
	int frameslookjobago;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//vector<TransportJob> bids;

	float frame[2];
#endif

	Unit* u = &g_unit[i];
	UnitT* t = &g_unitT[type];

	u->on = true;
	u->type = type;
	u->cmpos = Vec2i(cmpos.x, cmpos.z);
	u->drawpos = Vec3f(cmpos.x, cmpos.y, cmpos.z);
	u->owner = owner;
	u->path.clear();
	u->goal = Vec2i(cmpos.x, cmpos.z);
	u->target = -1;
	u->target2 = -1;
	u->targetu = false;
	u->underorder = false;
	u->fuelstation = -1;
	StartingBelongings(u);
	u->hp = t->starthp;
	u->passive = false;
	u->prevpos = u->cmpos;
	u->taskframe = 0;
	u->pathblocked = false;
	u->frameslookjobago = 0;
	u->supplier = -1;
	u->reqamt = 0;
	u->targtype = -1;
	u->frame[BODY_LOWER] = 0;
	u->frame[BODY_UPPER] = 0;
	u->subgoal = u->goal;

	u->mode = -1;
	u->pathdelay = 0;
	u->lastpath = g_simframe;

	u->fillcollider();

	return true;
}

void FreeUnits()
{
	for(int i=0; i<UNITS; i++)
	{
		g_unit[i].destroy();
		g_unit[i].on = false;
	}
}

bool Unit::hidden() const
{
	return false;
}

void AnimateUnit(Unit* u)
{
	UnitT* t = &g_unitT[u->type];

	if(u->type == UNIT_ROBOSOLDIER || u->type == UNIT_LABOURER)
	{
		if(u->prevpos == u->cmpos)
		{
			u->frame[BODY_LOWER] = 0;
			return;
		}
		
		PlayAnimation(u->frame[BODY_LOWER], 0, 29, true, 1.0f);
	}
}

void UpdateAI(Unit* u)
{
	if(u->type == UNIT_LABOURER)
		UpdateLabourer(u);
}

void UpdateUnits()
{
	for(int i = 0; i < UNITS; i++)
	{

		StartTimer(UPDUONCHECK);

		Unit* u = &g_unit[i];

		if(!u->on)
		{
			StopTimer(UPDUONCHECK);
			continue;
		}

		StopTimer(UPDUONCHECK);

		StartTimer(UPDUNITAI);
		UpdateAI(u);
		StopTimer(UPDUNITAI);
		StartTimer(MOVEUNIT);
		MoveUnit(u);
		StopTimer(MOVEUNIT);
		StartTimer(ANIMUNIT);
		AnimateUnit(u);
		StopTimer(ANIMUNIT);
	}
}