#include "../texture.h"
#include "foliage.h"
#include "water.h"
#include "shader.h"
#include "../math/camera.h"
#include "model.h"
#include "../math/frustum.h"
#include "vertexarray.h"
#include "heightmap.h"
#include "../utils.h"
#include "../window.h"
#include "../sim/player.h"
#include "../render/shadow.h"
#include "../path/pathnode.h"
#include "../path/collidertile.h"
#include "../phys/collision.h"
#include "../math/hmapmath.h"
#include "../sim/simflow.h"
#include "../path/fillbodies.h"
#include "fogofwar.h"

FlType g_fltype[FL_TYPES];
Foliage g_foliage[FOLIAGES];
Matrix g_folmodmat[FOLIAGES];
int g_folonsw[FOLIAGES];
unsigned char g_drawframe = 255;	//unsigned char

Foliage::Foliage()
{
	on = false;
	lastdraw = 0;
}

void Foliage::reinstance()
{
	int i = this - g_foliage;
	g_folonsw[i] = on ? 1 : 0;
	Matrix* m = &g_folmodmat[i];

	if(on)
	{
		float pitch = 0;
		m->reset();
		float radians[] = {(float)DEGTORAD(pitch), (float)DEGTORAD(yaw), 0};
		m->translation((const float*)&drawpos);
		Matrix rotation;
		rotation.rotrad(radians);
		m->postmult(rotation);
	}
	else
	{
		memset(m->m_matrix, 0, sizeof(float)*16);
	}
}

void DefF(int type, const char* modelrelative, Vec3f scale, Vec3f translate, Vec3i size)
{
	FlType* t = &g_fltype[type];
	//QueueTexture(&t->texindex, texrelative, true);
	QueueModel(&t->model, modelrelative, scale, translate);
	t->size = size;
}

int NewFoliage()
{
	for(int i=0; i<FOLIAGES; i++)
	{
		if(!g_foliage[i].on)
			return i;
	}

	return -1;
}

#if 0
void PlaceFol()
{
	if(g_scT < 0)
		return;

	int i = NewFoliage();

	if(i < 0)
		return;

	Foliage* s = &g_foliage[i];
	s->on = true;
	s->pos = g_vMouse;
	s->type = g_scT;
	s->yaw = DEGTORAD((rand()%360));

	if(s->pos.y <= WATER_LEVEL)
		s->on = false;
}
#endif

void DrawFol(Vec3f zoompos, Vec3f vertical, Vec3f horizontal)
{
	//return;

	Vec3f a, b, c, d;
	Vec3f vert, horiz;

	Shader* s = &g_shader[g_curS];

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);

	//glUniform1f(s->m_slot[SSLOT_MIND], MIN_DISTANCE);
	//glUniform1f(s->m_slot[SSLOT_MAXD], MAX_DISTANCE / g_zoom);

	FlType* t = &g_fltype[FL_TREE1];
	Vec3i* size = &t->size;
	Model* m = &g_model[t->model];
	VertexArray* va = &m->m_va[0];
	m->usetex();
	Matrix im;

	Player* py = &g_player[g_localP];
	Camera* cam = &g_cam;

	Vec3f viewdir = Normalize(cam->m_view - zoompos);
	Vec3f horizontal2 = horizontal;

#if 0
	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];
		
		if(!f->on)
			continue;
#else
	//get a list of visible f's
	//based on pathnode tiles.
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;
	
	//return;

	MapFrust(&interTopLeft,
		&interTopRight,
		&interBottomLeft,
		&interBottomRight);

	float fminx = fmin(interTopLeft.x,
		fmin(interTopRight.x,
		fmin(interBottomLeft.x, interBottomRight.x)));
	float fminy = fmin(interTopLeft.z,
		fmin(interTopRight.z,
		fmin(interBottomLeft.z, interBottomRight.z)));
	float fmaxx = fmax(interTopLeft.x,
		fmax(interTopRight.x,
		fmax(interBottomLeft.x, interBottomRight.x)));
	float fmaxy = fmax(interTopLeft.z,
		fmax(interTopRight.z,
		fmax(interBottomLeft.z, interBottomRight.z)));
	
	int tminx = imax(0, imin(g_pathdim.x-1, (int)(fminx) / PATHNODE_SIZE));
	int tminy = imax(0, imin(g_pathdim.y-1, (int)(fminy) / PATHNODE_SIZE));
	int tmaxx = imax(0, imin(g_pathdim.x-1, (int)(fmaxx) / PATHNODE_SIZE));
	int tmaxy = imax(0, imin(g_pathdim.y-1, (int)(fmaxy) / PATHNODE_SIZE));

	std::list<unsigned short> vf;

	//return;

	for(int nx=tminx; nx<=tmaxx; nx++)
		for(int ny=tminy; ny<=tmaxy; ny++)
		{
			ColliderTile* c = ColliderAt(nx, ny);

			if(c->foliage == USHRT_MAX)
				continue;

			Foliage* f = &g_foliage[c->foliage];

			if(f->lastdraw == g_drawframe)
				continue;

			f->lastdraw = g_drawframe;
			vf.push_back(c->foliage);
		}

	g_drawframe++;

	for(auto it=vf.begin(); it!=vf.end(); it++)
	{
		unsigned short i = *it;
		Foliage* f = &g_foliage[i];
		
		if(!f->on)
			continue;
#endif
		
		t = &g_fltype[f->type];
		size = &t->size;

		Vec3f vmin(f->drawpos.x - t->size.x/2, f->drawpos.y, f->drawpos.z - t->size.x/2);
		Vec3f vmax(f->drawpos.x + t->size.x/2, f->drawpos.y + t->size.y, f->drawpos.z + t->size.x/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;
		
		int tx = f->cmpos.x / TILE_SIZE;
		int ty = f->cmpos.y / TILE_SIZE;
		
		if(!IsTileVis(g_localP, tx, ty))
		{
			if(!Explored(g_localP, tx, ty))
				continue;
			else
				glUniform4f(s->m_slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, 1.0f);
		}
		else
			glUniform4f(s->m_slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
		
		m = &g_model[t->model];
		va = &m->m_va[0];
		m->usetex();

		if(Magnitude2( f->drawpos - zoompos ) > 8000*8000*2*2)
		//if(false)
		{
#if 0
			horizontal2 = horizontal;
			
			if(Magnitude2( f->pos - zoompos ) < 8000*8000*2*2)
			{
				viewdir = Normalize( f->pos - zoompos );
				//horizontal = Cross( vertical, viewdir );
				horizontal2 = Cross( viewdir, vertical );
			}
#endif

			Vec3f vert = vertical*size->y*1.6f;
			Vec3f horiz = horizontal2*(size->x/2)*1.6f;

			Vec3f a = f->drawpos + horiz + vert;
			Vec3f b = f->drawpos + horiz;
			Vec3f c = f->drawpos - horiz;
			Vec3f d = f->drawpos - horiz + vert;

			float vertices[] =
			{
				//posx, posy posz   texx, texy		normx, normy, normz
				a.x, a.y, a.z,          1, 0,		horiz.x, horiz.y, horiz.z,
				b.x, b.y, b.z,          1, 1,		horiz.x, horiz.y, horiz.z,
				c.x, c.y, c.z,          0, 1,		horiz.x, horiz.y, horiz.z,

				c.x, c.y, c.z,          0, 1,		-horiz.x, -horiz.y, -horiz.z,
				d.x, d.y, d.z,          0, 0,		-horiz.x, -horiz.y, -horiz.z,
				a.x, a.y, a.z,          1, 0,		horiz.x, horiz.y, horiz.z
			};

			//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
			//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
			
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, im.m_matrix);

			Matrix modelview;
	#ifdef SPECBUMPSHADOW
			modelview.set(g_camview.m_matrix);
	#endif
			modelview.postmult(im);
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

			Matrix mvp;
	#if 0
			mvp.set(modelview.m_matrix);
			mvp.postmult(g_camproj);
	#elif 0
			mvp.set(g_camproj.m_matrix);
			mvp.postmult(modelview);
	#else
			mvp.set(g_camproj.m_matrix);
			mvp.postmult(g_camview);
			mvp.postmult(im);
	#endif
			glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

			Matrix modelviewinv;
			Transpose(modelview, modelview);
			Inverse2(modelview, modelviewinv);
			//Transpose(modelviewinv, modelviewinv);
			glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);
			
			//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			glVertexPointer(3, GL_FLOAT, sizeof(float) * 8, &vertices[0]);
			//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 8, &vertices[3]);
			//if(s->m_slot[SSLOT_NORMAL] != -1)
			//	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
			glNormalPointer(GL_FLOAT, sizeof(float) * 8, &vertices[5]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		else
		{
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, g_folmodmat[i].m_matrix);

			Matrix modelview;
	#ifdef SPECBUMPSHADOW
			modelview.set(g_camview.m_matrix);
	#endif
			modelview.postmult(g_folmodmat[i]);
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

			Matrix mvp;
	#if 0
			mvp.set(modelview.m_matrix);
			mvp.postmult(g_camproj);
	#elif 0
			mvp.set(g_camproj.m_matrix);
			mvp.postmult(modelview);
	#else
			mvp.set(g_camproj.m_matrix);
			mvp.postmult(g_camview);
			mvp.postmult(g_folmodmat[i]);
	#endif
			glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

			Matrix modelviewinv;
			Transpose(modelview, modelview);
			Inverse2(modelview, modelviewinv);
			//Transpose(modelviewinv, modelviewinv);
			glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

			//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
			glVertexPointer(3, GL_FLOAT, 0, va->vertices);
			//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
			glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			//if(s->m_slot[SSLOT_NORMAL] != -1)
			//	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
			glNormalPointer(GL_FLOAT, 0, va->normals);
			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
		}
	}
}

bool PlaceFol(int type, Vec3i ipos)
{
	int i = NewFoliage();

	if(i < 0)
		return false;

	int nx = ipos.x / PATHNODE_SIZE;
	int nz = ipos.z / PATHNODE_SIZE;
	ColliderTile* c = ColliderAt(nx, nz);

	if(c->foliage != USHRT_MAX)
		return false;

	Foliage* f = &g_foliage[i];
	f->on = true;
	f->type = type;
	f->cmpos = Vec2i(ipos.x, ipos.z);
	f->drawpos = Vec3f(ipos.x, ipos.y, ipos.z);
	f->yaw = rand()%360;
	f->reinstance();
	f->fillcollider();

	return true;
}

void ClearFol(int cmminx, int cmminy, int cmmaxx, int cmmaxy)
{
#if 0
	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		if(f->pos.x >= minx && f->pos.x <= maxx && f->pos.z >= minz && f->pos.z <= maxz)
		{
			f->on = false;
			f->reinstance();
		}
	}
#endif

	//c = cell position
	int cminx = cmminx / PATHNODE_SIZE;
	int cminy = cmminy / PATHNODE_SIZE;
	int cmaxx = cmmaxx / PATHNODE_SIZE;
	int cmaxy = cmmaxy / PATHNODE_SIZE;

	for(int ny = cminy; ny <= cmaxy; ny++)
		for(int nx = cminx; nx <= cmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);

			if(c->foliage < 0)
				continue;

			Foliage* f = &g_foliage[c->foliage];
			FlType* ft = &g_fltype[f->type];
			
			int cmminx2 = f->cmpos.x - ft->size.x/2;
			int cmminy2 = f->cmpos.y - ft->size.z/2;
			int cmmaxx2 = cmminx2 + ft->size.x - 1;
			int cmmaxy2 = cmminy2 + ft->size.z - 1;

			//if(f->cmpos.x >= cmminx && f->cmpos.x <= cmmaxx && f->cmpos.y >= cmminz && f->cmpos.y <= cmmaxz)
			if(cmminx2 <= cmmaxx && cmminy2 <= cmmaxy && cmmaxx2 >= cmminx && cmmaxy2 >= cmminy)
			{
				f->on = false;
				f->freecollider();
				f->reinstance();
			}
		}

	FillBodies();
}

void FillForest()
{
#if 0
	for(int tx = 0; tx < g_hmap.m_widthx; tx++)
		for(int tz = 0; tz < g_hmap.m_widthy; tz++)
		{
			int x = tz*TILE_SIZE + TILE_SIZE/2;
			int z = tz*TILE_SIZE + TILE_SIZE/2;

			Vec3f norm = g_hmap.getnormal(tx, tz);

			float y = g_hmap.accheight2(x, z);

			if(y >= ELEV_SANDONLYMAXY && y <= ELEV_GRASSONLYMAXY && 1.0f - norm.y <= 0.3f)
			{
				Forest* f = ForestAt(tx, tz);
				f->on = true;
				f->remesh();
			}
		}
#endif
	//for(int condensation = 0; condensation < sqrt(g_hmap.m_widthx * g_hmap.m_widthy); condensation++)
	{

		int maxfoliage = FOLIAGES*g_hmap.m_widthx*g_hmap.m_widthy/MAX_MAP/MAX_MAP;
		maxfoliage = imin(FOLIAGES, maxfoliage);

		Vec2i last = Vec2i((rand()%g_hmap.m_widthx)*TILE_SIZE + rand()%TILE_SIZE, (rand()%g_hmap.m_widthy)*TILE_SIZE + rand()%TILE_SIZE);

		for(int i=0; i<maxfoliage; i++)
		{
			//break;
			for(int j=0; j<300; j++)
			{
				//new forest?
				if(rand()%300 == 0)
					last = Vec2i((rand()%g_hmap.m_widthx)*TILE_SIZE + rand()%TILE_SIZE, (rand()%g_hmap.m_widthy)*TILE_SIZE + rand()%TILE_SIZE);

				//move from last seedling
				int x = last.x + rand()%TILE_SIZE - TILE_SIZE/2;
				int z = last.y + rand()%TILE_SIZE - TILE_SIZE/2;

				if(OffMap(x, z, x, z))
					continue;

				last = Vec2i(x,z);

				int tx = x / TILE_SIZE;
				int tz = z / TILE_SIZE;

				float y = g_hmap.accheight2(x, z);

				Vec3f norm = g_hmap.getnormal(x/TILE_SIZE, z/TILE_SIZE);

				float offequator = fabs( (float)g_hmap.m_widthy*TILE_SIZE/2.0f - z );

				if(y >= ELEV_SANDONLYMAXY && y <= ELEV_GRASSONLYMAXY && 1.0f - norm.y <= 0.3f)
				{
					//int type = rand()%10 == 1 ? UNIT_MECH : UNIT_LABOURER;
#if 0
					int type = UNIT_LABOURER;

					if(rand()%10 == 1)
						type = UNIT_MECH;
					if(rand()%10 == 1)
						type = UNIT_GEPARDAA;

					PlaceUnit(type, Vec3i(x, y, z), -1, -1);
#endif
#if 1
					
					//int type = FL_TREE1;
					int type = rand()%FL_TYPES;

					if(PlaceFol(type, Vec3i(x, y, z)))
						break;
#endif
					//break;
				}
			}
		}

		//CondenseForest(0, 0, g_hmap.m_widthx-1, g_hmap.m_widthy-1);
	}
}

void FreeFol()
{
	for(int i=0; i<FOLIAGES; i++)
	{
		g_foliage[i].on = false;
		g_foliage[i].reinstance();
	}
}
