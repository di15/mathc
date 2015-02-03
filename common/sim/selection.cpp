#include "selection.h"
#include "../math/matrix.h"
#include "../window.h"
#include "../math/plane3f.h"
#include "../math/frustum.h"
#include "../math/brush.h"
#include "utype.h"
#include "unit.h"
#include "bltype.h"
#include "building.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../math/hmapmath.h"
#include "../utils.h"
#include "../../game/gmain.h"
#include "build.h"
#include "player.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../gui/widgets/spez/blview.h"
#include "../gui/widgets/spez/truckmgr.h"
#include "../gui/gui.h"
#include "powl.h"
#include "crpipe.h"
#include "road.h"

// Selection circle texture index
unsigned int g_circle = 0;

void Selection::clear()
{
	units.clear();
	buildings.clear();
	roads.clear();
	powls.clear();
	crpipes.clear();
}

// Selection frustum for drag/area-selection
static Vec3f normalLeft;
static Vec3f normalTop;
static Vec3f normalRight;
static Vec3f normalBottom;
static float distLeft;
static float distTop;
static float distRight;
static float distBottom;

static Frustum g_selfrust;	//selection frustum

void DrawMarquee()
{
	Player* py = &g_player[g_localP];

	if(!g_mousekeys[0] || g_keyintercepted || g_mode != APPMODE_PLAY || g_build != BL_NONE)
		return;

#if 0
	EndS();
	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].m_slot[SSLOT_TEXCOORD0]);
#endif

	float vertices[] =
	{
		//posx, posy    texx, texy
		(float)g_mousestart.x,	(float)g_mousestart.y, 0,			0, 0,
		(float)g_mousestart.x,	(float)g_mouse.y,0,				1, 0,
		(float)g_mouse.x,			(float)g_mouse.y,0,				1, 1,

		(float)g_mouse.x,			(float)g_mousestart.y,0,			1, 1,
		(float)g_mousestart.x,	(float)g_mousestart.y,0,			0, 1
	};

	//glVertexAttribPointer(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
	glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);

	glDrawArrays(GL_LINE_STRIP, 0, 5);
}

#if 1
void DrawSel(Matrix* projection, Matrix* modelmat, Matrix* viewmat)
{
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	UseS(SHADER_COLOR3D);
	Shader* s = &g_shader[g_curS];
	glUniformMatrix4fv(s->m_slot[SSLOT_PROJECTION], 1, 0, projection->m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat->m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_VIEWMAT], 1, 0, viewmat->m_matrix);

	Matrix mvp;
#if 0
	mvp.set(modelview.m_matrix);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(modelview);
#else
	mvp.set(projection->m_matrix);
	mvp.postmult(*viewmat);
	mvp.postmult(*modelmat);
#endif
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	float* color = g_player[g_localP].color;
	glUniform4f(s->m_slot[SSLOT_COLOR], color[0], color[1], color[2], 0.5f);

	Player* py = &g_player[g_localP];

	glLineWidth(3);

	for(auto seliter = g_sel.buildings.begin(); seliter != g_sel.buildings.end(); seliter++)
	{
		const int bi = *seliter;
		const Building* b = &g_building[bi];
		const BlType* t = &g_bltype[b->type];

		const int tminx = b->tilepos.x - t->widthx/2;
		const int tminz = b->tilepos.y - t->widthy/2;
		const int tmaxx = tminx + t->widthx;
		const int tmaxz = tminz + t->widthy;

		const int cmminx = tminx*TILE_SIZE;
		const int cmminz = tminz*TILE_SIZE;
		const int cmmaxx = tmaxx*TILE_SIZE;
		const int cmmaxz = tmaxz*TILE_SIZE;

		const int off = TILE_SIZE/100;

		const float y1 = g_hmap.accheight(cmmaxx + off, cmminz - off) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(cmmaxx + off, cmmaxz + off) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(cmminx - off, cmmaxz + off) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(cmminx - off, cmminz - off) + TILE_SIZE/20;

		const float vertices[] =
		{
			//posx, posy posz
			(float)(cmmaxx + off), y1, (float)(cmminz - off),
			(float)(cmmaxx + off), y2, (float)(cmmaxz + off),
			(float)(cmminx - off), y3, (float)(cmmaxz + off),

			(float)(cmminx - off), y4, (float)(cmminz - off),
			(float)(cmmaxx + off), y1, (float)(cmminz - off)
		};

		//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
		//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}


	glLineWidth(1);
	EndS();

	//if(g_projtype == PROJ_PERSPECTIVE)
	UseS(SHADER_BILLBOARD);
	//else
	//	UseS(SHADER_BILLBOAR);

	s = &g_shader[g_curS];

	glUniformMatrix4fv(s->m_slot[SSLOT_PROJECTION], 1, 0, projection->m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat->m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_VIEWMAT], 1, 0, viewmat->m_matrix);

	color = g_player[g_localP].color;
	glUniform4f(s->m_slot[SSLOT_COLOR], color[0], color[1], color[2], 1.0f);

	//glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_circle ].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);

	for(auto seliter = g_sel.units.begin(); seliter != g_sel.units.end(); seliter++)
	{
		Unit* u = &g_unit[ *seliter ];
		//Entity* e = g_entity[ 0 ];
		Vec3f p = u->drawpos;
		UType* t = &g_utype[ u->type ];

		//Vec3f p = c->m_pos + Vec3f(0, t->vmin.y, 0) + Vec3f(0, 1.0f, 0);

		const float r = t->size.x * 1.0f;

#if 0
		float y1 = Bilerp(&g_hmap, p.x + r, p.z - r);
		float y2 = Bilerp(&g_hmap, p.x + r, p.z + r);
		float y3 = Bilerp(&g_hmap, p.x - r, p.z + r);
		float y4 = Bilerp(&g_hmap, p.x - r, p.z - r);
#elif 1
		const float y1 = g_hmap.accheight(p.x + r, p.z - r) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(p.x + r, p.z + r) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(p.x - r, p.z + r) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(p.x - r, p.z - r) + TILE_SIZE/20;
#else
		float y1 = p.y;
		float y2 = p.y;
		float y3 = p.y;
		float y4 = p.y;
#endif

		const float vertices[] =
		{
			//posx, posy posz   texx, texy
			p.x + r, y1, p.z - r,          1, 0,
			p.x + r, y2, p.z + r,          1, 1,
			p.x - r, y3, p.z + r,          0, 1,

			p.x - r, y3, p.z + r,          0, 1,
			p.x - r, y4, p.z - r,          0, 0,
			p.x + r, y1, p.z - r,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	EndS();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
#endif

int SelectOneBuilding(Vec3f *line)
{
	int sel = -1;
	float closestd = -1;

	for(int i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		BlType* t = &g_bltype[ b->type ];

		const int tminx = b->tilepos.x - t->widthx/2;
		const int tminz = b->tilepos.y - t->widthy/2;
		const int tmaxx = tminx + t->widthx;
		const int tmaxz = tminz + t->widthy;

		const int cmminx = tminx*TILE_SIZE;
		const int cmminz = tminz*TILE_SIZE;
		const int cmmaxx = tmaxx*TILE_SIZE;
		const int cmmaxz = tmaxz*TILE_SIZE;

		const int cmx = (cmminx+cmmaxx)/2;
		const int cmz = (cmminz+cmmaxz)/2;

		const float y = g_hmap.accheight(cmx, cmz);

		const int maxy = (int)y + imax(t->widthx, t->widthy) * TILE_SIZE;

		Vec3f normals[6];
		float dists[6];

		Vec3f vmin = Vec3f(cmminx, y, cmminz);
		Vec3f vmax = Vec3f(cmmaxx, maxy, cmmaxz);

		//MakeHull(normals, dists, b->drawpos, vmin, vmax);
		MakeHull(normals, dists, Vec3f(0,0,0), vmin, vmax);

		Vec3f intersection;

		if(!LineInterHull(line, normals, dists, 6, &intersection))
			continue;
		bool ispolyinter = false;

		const VertexArray* va = &b->drawva;
		Vec3f poly[3];

		for(int v=0; v<va->numverts; v+=3)
		{
			poly[0] = va->vertices[v+0] + b->drawpos;
			poly[1] = va->vertices[v+1] + b->drawpos;
			poly[2] = va->vertices[v+2] + b->drawpos;

			Vec3f polyinter;

			if(InterPoly(poly, line, 3, &polyinter))
			{
				ispolyinter = true;

				float thisd = Magnitude(line[0] - polyinter);

				if(thisd < closestd || closestd < 0)
					intersection = polyinter;
			}
		}

		if(!ispolyinter)
			continue;

		float thisd = Magnitude(line[0] - intersection);

		if(thisd < closestd || closestd < 0)
		{
			sel = i;
			closestd = thisd;
		}
	}

	return sel;
}

int SelectOneUnit(Vec3f *line)
{
	int unitsel = -1;
	float closestd = -1;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UType* t = &g_utype[ u->type ];

		Vec3f normals[6];
		float dists[6];

		Vec3f vmin = Vec3f(-t->size.x/2, 0, -t->size.z/2);
		Vec3f vmax = Vec3f(t->size.x/2, t->size.y, t->size.z/2);

		MakeHull(normals, dists, u->drawpos, vmin, vmax);

		Vec3f intersection;

		if(!LineInterHull(line, normals, dists, 6, &intersection))
			continue;

		float thisd = Magnitude(line[0] - intersection);

		if(thisd < closestd || closestd < 0)
		{
			unitsel = i;
			closestd = thisd;
		}
	}

	return unitsel;
}

Selection SelectOne(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	Player* py = &g_player[g_localP];

	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f line[2];
	line[0] = campos;
	line[1] = campos + (ray * 1000000.0f);

	Selection selection;

	int sel = SelectOneUnit(line);
#if 0
	InfoMess("sel one", "a");
#endif

	if(sel >= 0)
	{
#if 0
		InfoMess("sel one", "b");
#endif
		selection.units.push_back( sel );
	}

	sel = SelectOneBuilding(line);

	if(sel >= 0)
	{
		selection.buildings.push_back( sel );
	}

	return selection;
}

bool PointBehindPlane(Vec3f p, Vec3f normal, float dist)
{
	float result = p.x*normal.x + p.y*normal.y + p.z*normal.z + dist;

	if(result <= EPSILON)
		return true;

	return false;
}

bool PointInsidePlanes(Vec3f p)
{
	if(!PointBehindPlane(p, normalLeft, distLeft))
		return false;

	if(!PointBehindPlane(p, normalTop, distTop))
		return false;

	if(!PointBehindPlane(p, normalRight, distRight))
		return false;

	if(!PointBehindPlane(p, normalBottom, distBottom))
		return false;

	return true;
}

std::list<int> SelectAreaUnits()
{
	std::list<int> unitsel;

	bool haveowned = false;
	bool haveowmili = false;

	for(int i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		UType* t = &g_utype[ u->type ];

		Vec3f vmin = u->drawpos + Vec3f(-t->size.x/2, 0, -t->size.z/2);
#if 1
		Vec3f vmax = u->drawpos + Vec3f(t->size.x/2, t->size.y, t->size.z/2);
#else
		Vec3f vmax = u->drawpos + Vec3f(t->size.x/2, 0, t->size.z/2);
#endif

		if(!g_selfrust.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		unitsel.push_back(i);

		if(u->owner == g_localP && u->type != UNIT_LABOURER)
		{
			haveowned = true;

			if(t->military)
				haveowmili = true;
		}
	}

	//filter units....

	if(haveowmili)
	{
		//only owned military

		auto uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Unit* u = &g_unit[*uit];

			if(u->type == UNIT_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(u->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			UType* t = &g_utype[u->type];

			if(!t->military)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}
	else if(haveowned)
	{
		//only owned (no labourers)
		
		auto uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Unit* u = &g_unit[*uit];

			if(u->type == UNIT_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(u->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}

	return unitsel;
}

Selection SelectAreaPersp(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	Player* py = &g_player[g_localP];

	int minx = imin(g_mousestart.x, g_mouse.x);
	int maxx = imax(g_mousestart.x, g_mouse.x);
	int miny = imin(g_mousestart.y, g_mouse.y);
	int maxy = imax(g_mousestart.y, g_mouse.y);

	//Vec3f campos = c->m_pos;
	//Vec3f camside = c->m_strafe;
	//Vec3f camup2 = c->up2();
	//Vec3f viewdir = Normalize( c->m_view - c->m_pos );

	Vec3f topLeftRay = ScreenPerspRay(minx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopLeft[2];
	lineTopLeft[0] = campos;
	lineTopLeft[1] = campos + (topLeftRay * 10000.0f);

	Vec3f topRightRay = ScreenPerspRay(maxx, miny, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineTopRight[2];
	lineTopRight[0] = campos;
	lineTopRight[1] = campos + (topRightRay * 10000.0f);

	Vec3f bottomLeftRay = ScreenPerspRay(minx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomLeft[2];
	lineBottomLeft[0] = campos;
	lineBottomLeft[1] = campos + (bottomLeftRay * 10000.0f);

	Vec3f bottomRightRay = ScreenPerspRay(maxx, maxy, g_width, g_height, campos, camside, camup2, viewdir, FIELD_OF_VIEW);
	Vec3f lineBottomRight[2];
	lineBottomRight[0] = campos;
	lineBottomRight[1] = campos + (bottomRightRay * 10000.0f);

#if 0
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	GetMapIntersection2(lineTopLeft, &interTopLeft);
	GetMapIntersection2(lineTopRight, &interTopRight);
	GetMapIntersection2(lineBottomLeft, &interBottomLeft);
	GetMapIntersection2(lineBottomRight, &interBottomRight);
#elif 0
	Vec3f interTopLeft;
	Vec3f interTopRight;
	Vec3f interBottomLeft;
	Vec3f interBottomRight;

	GetMapIntersection(&g_hmap, lineTopLeft, &interTopLeft);
	GetMapIntersection(&g_hmap, lineTopRight, &interTopRight);
	GetMapIntersection(&g_hmap, lineBottomLeft, &interBottomLeft);
	GetMapIntersection(&g_hmap, lineBottomRight, &interBottomRight);

	PlaceUnit(UNIT_BATTLECOMP, Vec3i(interTopLeft.x, interTopLeft.y, interTopLeft.z), 2, -1, -1);
	PlaceUnit(UNIT_BATTLECOMP, Vec3i(interTopRight.x, interTopRight.y, interTopRight.z), 2, -1, -1);
	PlaceUnit(UNIT_BATTLECOMP, Vec3i(interBottomLeft.x, interBottomLeft.y, interBottomLeft.z), 2, -1, -1);
	PlaceUnit(UNIT_BATTLECOMP, Vec3i(interBottomRight.x, interBottomRight.y, interBottomRight.z), 2, -1, -1);
#endif

	Vec3f leftPoly[3];
	Vec3f topPoly[3];
	Vec3f rightPoly[3];
	Vec3f bottomPoly[3];
	Vec3f frontPoly[3];
	Vec3f backPoly[3];

	//Assemble polys in clockwise order so that their normals face outward

	leftPoly[0] = campos;
#if 0
	leftPoly[1] = interBottomLeft;
	leftPoly[2] = interTopLeft;
#endif
	leftPoly[1] = lineBottomLeft[1];
	leftPoly[2] = lineTopLeft[1];

	topPoly[0] = campos;
#if 0
	topPoly[1] = interTopLeft;
	topPoly[2] = interTopRight;
#endif
	topPoly[1] = lineTopLeft[1];
	topPoly[2] = lineTopRight[1];

	rightPoly[0] = campos;
#if 0
	rightPoly[1] = interTopRight;
	rightPoly[2] = interBottomRight;
#endif
	rightPoly[1] = lineTopRight[1];
	rightPoly[2] = lineBottomRight[1];

	bottomPoly[0] = campos;
#if 0
	bottomPoly[1] = interBottomRight;
	bottomPoly[2] = interBottomLeft;
#endif
	bottomPoly[1] = lineBottomRight[1];
	bottomPoly[2] = lineBottomLeft[1];

	frontPoly[0] = lineBottomLeft[1];
	frontPoly[1] = lineBottomRight[1];
	frontPoly[2] = lineTopRight[1];

	backPoly[0] = lineTopRight[0] + topLeftRay;
	backPoly[1] = lineBottomRight[0] + bottomRightRay;
	backPoly[2] = lineBottomLeft[0] + bottomLeftRay;

	// Normals will actually face inward
	// (using counter-clockwise function Normal2)

	normalLeft = Normal2(leftPoly);
	normalTop = Normal2(topPoly);
	normalRight = Normal2(rightPoly);
	normalBottom = Normal2(bottomPoly);
	Vec3f normalFront = Normal(frontPoly);
	Vec3f normalBack = Normal(backPoly);

	distLeft = PlaneDistance(normalLeft, leftPoly[0]);
	distTop = PlaneDistance(normalTop, topPoly[0]);
	distRight = PlaneDistance(normalRight, rightPoly[0]);
	distBottom = PlaneDistance(normalBottom, bottomPoly[0]);
	float distFront = PlaneDistance(normalFront, frontPoly[0]);
	float distBack = PlaneDistance(normalBack, backPoly[0]);

	g_selfrust.construct(
	        Plane3f(normalLeft.x, normalLeft.y, normalLeft.z, distLeft),
	        Plane3f(normalRight.x, normalRight.y, normalRight.z, distRight),
	        Plane3f(normalTop.x, normalTop.y, normalTop.z, distTop),
	        Plane3f(normalBottom.x, normalBottom.y, normalBottom.z, distBottom),
	        Plane3f(normalFront.x, normalFront.y, normalFront.z, distFront),
	        Plane3f(normalBack.x, normalBack.y, normalBack.z, distBack));

	Selection selection;

	selection.units = SelectAreaUnits();

	return selection;
}

Selection DoSel(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir)
{
	Selection sel;
	Player* py = &g_player[g_localP];

	if(g_mousestart.x == g_mouse.x && g_mousestart.y == g_mouse.y)
		sel = SelectOne(campos, camside, camup2, viewdir);
	else
		sel = SelectAreaPersp(campos, camside, camup2, viewdir);

	return sel;
}

void ClearSel(Selection* s)
{
	s->buildings.clear();
	s->crpipes.clear();
	s->powls.clear();
	s->roads.clear();
	s->units.clear();
}

void AfterSel(Selection* s)
{
	bool haveconstr = false;
	bool havefini = false;
	bool havetruck = false;

	for(auto seliter = s->buildings.begin(); seliter != s->buildings.end(); seliter++)
	{
		int bi = *seliter;
		Building* b = &g_building[bi];

		if(!b->finished)
		{
			haveconstr = true;
			break;
		}
		else
		{
			havefini = true;
			break;
		}
	}

	if(!havefini && !haveconstr && s->units.size() > 0)
	{
		auto seliter = s->units.begin();
		int ui = *seliter;
		Unit* u = &g_unit[ui];
		if(u->type == UNIT_TRUCK)
			havetruck = true;
	}

#if 0
	for(auto seliter = s->powls.begin(); seliter != s->powls.end(); seliter++)
	{
		PowlTile* p = PowlAt(seliter->x, seliter->y);

		if(!p->finished)
		{
			haveconstr = true;
			break;
		}
	}

	for(auto seliter = s->crpipes.begin(); seliter != s->crpipes.end(); seliter++)
	{
		CrPipeTile* p = CrPipeAt(seliter->x, seliter->y);

		if(!p->finished)
		{
			haveconstr = true;
			break;
		}
	}


	for(auto seliter = s->roads.begin(); seliter != s->roads.end(); seliter++)
	{
		RoadTile* r = RoadAt(seliter->x, seliter->y);

		if(!r->finished)
		{
			haveconstr = true;
			break;
		}
	}
#endif
	
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	if(haveconstr)
	{
		CstrView* cv = (CstrView*)gui->get("cstr view");
		cv->regen(s);
		gui->open("cstr view");
	}
	else if(havefini)
	{
		BlView* bv = (BlView*)gui->get("bl view");
		bv->regen(s);
		gui->open("bl view");
	}
	else if(havetruck)
	{
		TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
		tm->regen(s);
		gui->open("truck mgr");
	}

#if 0
	//35
	//2
	//15
	//39
	//12
	if(s->units.size() > 0)
	{
		char msg[128];
		sprintf(msg, "sel %d u \n target=%d \n dgoal=%d,%d", 
			*s->units.begin(), 
			g_unit[*s->units.begin()].target,
			g_unit[*s->units.begin()].goal.x - g_unit[*s->units.begin()].cmpos.x,
			g_unit[*s->units.begin()].goal.y - g_unit[*s->units.begin()].cmpos.y);
		InfoMess(msg, msg);
	}
	if(s->buildings.size() > 0)
	{
		char msg[128];
		sprintf(msg, "sel %d b \n transporter cement = %d",
			*s->buildings.begin(),
			g_building[*s->buildings.begin()].transporter[RES_CEMENT]);
		InfoMess(msg, msg);
	}
#endif
}
