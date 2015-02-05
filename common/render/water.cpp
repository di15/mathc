#include "water.h"
#include "shader.h"
#include "../math/vec3f.h"
#include "../texture.h"
#include "heightmap.h"
#include "../window.h"
#include "../save/savemap.h"
#include "../sim/player.h"
#include "shadow.h"
#include "../debug.h"
#include "fogofwar.h"

unsigned int g_water;
unsigned int g_watertex[WATER_TEXS];

Vec3f* g_waterverts = NULL;
Vec2f* g_watertexcos = NULL;
Vec3f* g_waternorms = NULL;

void AllocWater(int wx, int wy)
{
	FreeWater();
	g_waterverts = new Vec3f[ (wx) * (wy) * 6 ];
	g_watertexcos = new Vec2f[ (wx) * (wy) * 6 ];
	g_waternorms = new Vec3f[ (wx) * (wy) * 6 ];

	if(!g_waterverts)
		OutOfMem(__FILE__, __LINE__);

	if(!g_watertexcos)
		OutOfMem(__FILE__, __LINE__);

	if(!g_waternorms)
		OutOfMem(__FILE__, __LINE__);

	for(int x=0; x<wx; x++)
		for(int z=0; z<wy; z++)
		{
			g_waterverts[ z*(wx) * 6 + x * 6 + 0 ] = Vec3f(x*TILE_SIZE, WATER_LEVEL, z*TILE_SIZE);
			g_waterverts[ z*(wx) * 6 + x * 6 + 1 ] = Vec3f((x+1)*TILE_SIZE, WATER_LEVEL, z*TILE_SIZE);
			g_waterverts[ z*(wx) * 6 + x * 6 + 2 ] = Vec3f((x+1)*TILE_SIZE, WATER_LEVEL, (z+1)*TILE_SIZE);
			g_waterverts[ z*(wx) * 6 + x * 6 + 3 ] = Vec3f(x*TILE_SIZE, WATER_LEVEL, z*TILE_SIZE);
			g_waterverts[ z*(wx) * 6 + x * 6 + 4 ] = Vec3f((x+1)*TILE_SIZE, WATER_LEVEL, (z+1)*TILE_SIZE);
			g_waterverts[ z*(wx) * 6 + x * 6 + 5 ] = Vec3f(x*TILE_SIZE, WATER_LEVEL, (z+1)*TILE_SIZE);

			if(g_hmap.getheight(x, z) > WATER_LEVEL &&
			                g_hmap.getheight(x+1, z) > WATER_LEVEL &&
			                g_hmap.getheight(x, z+1) > WATER_LEVEL &&
			                g_hmap.getheight(x+1, z+1) > WATER_LEVEL)
			{
				g_waterverts[ z*(wx) * 6 + x * 6 + 0 ] = Vec3f(0,0,0);
				g_waterverts[ z*(wx) * 6 + x * 6 + 1 ] = Vec3f(0,0,0);
				g_waterverts[ z*(wx) * 6 + x * 6 + 2 ] = Vec3f(0,0,0);
				g_waterverts[ z*(wx) * 6 + x * 6 + 3 ] = Vec3f(0,0,0);
				g_waterverts[ z*(wx) * 6 + x * 6 + 4 ] = Vec3f(0,0,0);
				g_waterverts[ z*(wx) * 6 + x * 6 + 5 ] = Vec3f(0,0,0);
			}

			g_watertexcos[ z*(wx) * 6 + x * 6 + 0 ] = Vec2f(x, z);
			g_watertexcos[ z*(wx) * 6 + x * 6 + 1 ] = Vec2f(x+1, z);
			g_watertexcos[ z*(wx) * 6 + x * 6 + 2 ] = Vec2f(x+1, z+1);
			g_watertexcos[ z*(wx) * 6 + x * 6 + 3 ] = Vec2f(x, z);
			g_watertexcos[ z*(wx) * 6 + x * 6 + 4 ] = Vec2f(x+1, z+1);
			g_watertexcos[ z*(wx) * 6 + x * 6 + 5 ] = Vec2f(x, z+1);

			g_waternorms[ z*(wx) * 6 + x * 6 + 0 ] = Vec3f(0, 1, 0);
			g_waternorms[ z*(wx) * 6 + x * 6 + 1 ] = Vec3f(0, 1, 0);
			g_waternorms[ z*(wx) * 6 + x * 6 + 2 ] = Vec3f(0, 1, 0);
			g_waternorms[ z*(wx) * 6 + x * 6 + 3 ] = Vec3f(0, 1, 0);
			g_waternorms[ z*(wx) * 6 + x * 6 + 4 ] = Vec3f(0, 1, 0);
			g_waternorms[ z*(wx) * 6 + x * 6 + 5 ] = Vec3f(0, 1, 0);
		}
}

void FreeWater()
{
	if(g_waterverts)
	{
		delete [] g_waterverts;
		g_waterverts = NULL;
	}

	if(g_watertexcos)
	{
		delete [] g_watertexcos;
		g_watertexcos = NULL;
	}

	if(g_waternorms)
	{
		delete [] g_waternorms;
		g_waternorms = NULL;
	}
}

void DrawWater3()
{
	static float wavephase = 0;

	wavephase += g_drawfrinterval * 12.0f;
	int iwp = (int)wavephase;
	float rem = wavephase - (float)iwp;
	wavephase = (int)wavephase % 2000 + rem;

	Shader* s = &g_shader[g_curS];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_water].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_GRADIENT] ].texname);
	glUniform1i(s->m_slot[SSLOT_GRADIENTTEX], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_DETAIL] ].texname);
	glUniform1i(s->m_slot[SSLOT_DETAILTEX], 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_SPECULAR] ].texname);
	glUniform1i(s->m_slot[SSLOT_SPECULARMAP], 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_NORMAL] ].texname);
	glUniform1i(s->m_slot[SSLOT_NORMALMAP], 3);

	Player* py = &g_player[g_localP];

	glUniform1f(s->m_slot[SSLOT_MIND], MIN_DISTANCE);
	glUniform1f(s->m_slot[SSLOT_MAXD], MAX_DISTANCE / g_zoom);
	glUniform1i(s->m_slot[SSLOT_WAVEPHASE], wavephase);

	Matrix modelmat;
	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
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
	mvp.postmult(modelmat);
#endif
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

#if 0
	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);
	glPolygonOffset(1.0, 0.001/(g_zoom));
	//glPolygonOffset(1.0, 250.0);
#endif

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthy; z++)
		{
			if(!IsTileVis(g_localP, x, z))
				continue;

			//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &g_waterverts[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &g_watertexcos[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &g_waternorms[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			glVertexPointer(3, GL_FLOAT, 0, &g_waterverts[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			glTexCoordPointer(2, GL_FLOAT, 0, &g_watertexcos[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			glNormalPointer(GL_FLOAT, 0, &g_waternorms[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

#if 0
	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, g_waterverts);
	glVertexPointer(3, GL_FLOAT, 0, g_waterverts);
	//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, g_watertexcos);
	glTexCoordPointer(2, GL_FLOAT, 0, g_watertexcos);
	//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, g_waternorms);
	glNormalPointer(GL_FLOAT, 0, g_waternorms);

	glDrawArrays(GL_TRIANGLES, 0, g_hmap.m_widthx * g_hmap.m_widthy * 6 );
#endif

#if 0
	glDisable(GL_POLYGON_OFFSET_FILL);
#endif
}

void DrawWater()
{
	static float wavephase = 0;

	wavephase += g_drawfrinterval * 12.0f;
	int iwp = (int)wavephase;
	float rem = wavephase - (float)iwp;
	wavephase = (int)wavephase % 2000 + rem;

	Shader* s = &g_shader[g_curS];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_water].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_GRADIENT] ].texname);
	glUniform1i(s->m_slot[SSLOT_GRADIENTTEX], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_DETAIL] ].texname);
	glUniform1i(s->m_slot[SSLOT_DETAILTEX], 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_SPECULAR] ].texname);
	glUniform1i(s->m_slot[SSLOT_SPECULARMAP], 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_NORMAL] ].texname);
	glUniform1i(s->m_slot[SSLOT_NORMALMAP], 3);

	glUniform1i(s->m_slot[SSLOT_WAVEPHASE], wavephase);

	Matrix modelmat;
	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
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
	mvp.postmult(modelmat);
#endif
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	Vec3f a, b, c, d;

	int wx = g_hmap.m_widthx;
	int wy = g_hmap.m_widthy;

	a = Vec3f(wx * TILE_SIZE, WATER_LEVEL, wy * TILE_SIZE);
	b = Vec3f(0, WATER_LEVEL, wy * TILE_SIZE);
	c = Vec3f(0, WATER_LEVEL, 0);
	d = Vec3f(wx * TILE_SIZE, WATER_LEVEL, 0);

	const float vertices[] =
	{
		a.x, a.y, a.z,
		b.x, b.y, b.z,
		c.x, c.y, c.z,
		d.x, d.y, d.z,
		a.x, a.y, a.z,
		c.x, c.y, c.z
	};

	const float texcoords0[] =
	{
		(float)(wx+1), (float)(wy+1),
		0, (float)(wy+1),
		0, 0,
		(float)(wx+1), 0,
		(float)(wx+1), (float)(wy+1),
		0, 0
	};

	const float normals[] =
	{
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0
	};

	Player* py = &g_player[g_localP];

	glUniform1f(s->m_slot[SSLOT_MIND], MIN_DISTANCE);
	glUniform1f(s->m_slot[SSLOT_MAXD], MAX_DISTANCE / g_zoom);

#if 0
	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);
	glPolygonOffset(1.0, 0.001/(g_zoom));
	//glPolygonOffset(1.0, 250.0);
#endif

	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, texcoords0);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords0);
	//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, normals);
	glNormalPointer(GL_FLOAT, 0, normals);

	glDrawArrays(GL_TRIANGLES, 0, 6);

#if 0
	glDisable(GL_POLYGON_OFFSET_FILL);
#endif
}

void DrawWater2()
{
	Shader* s = &g_shader[g_curS];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_water].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);

	CheckGLError(__FILE__, __LINE__);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_GRADIENT] ].texname);
	glUniform1i(s->m_slot[SSLOT_GRADIENTTEX], 0);

	CheckGLError(__FILE__, __LINE__);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_DETAIL] ].texname);
	glUniform1i(s->m_slot[SSLOT_DETAILTEX], 1);

	CheckGLError(__FILE__, __LINE__);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_SPECULAR] ].texname);
	glUniform1i(s->m_slot[SSLOT_SPECULARMAP], 2);

	CheckGLError(__FILE__, __LINE__);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_NORMAL] ].texname);
	glUniform1i(s->m_slot[SSLOT_NORMALMAP], 3);

	CheckGLError(__FILE__, __LINE__);

	Vec3f a, b, c, d;

	int wx = g_hmap.m_widthx;
	int wy = g_hmap.m_widthy;

	glUniform1f(s->m_slot[SSLOT_MAPMINZ], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXZ], (float)wx*TILE_SIZE);
	glUniform1f(s->m_slot[SSLOT_MAPMINX], 0);
	glUniform1f(s->m_slot[SSLOT_MAPMAXX], (float)wy*TILE_SIZE);
	glUniform1f(s->m_slot[SSLOT_MAPMINY], ConvertHeight(0));
	glUniform1f(s->m_slot[SSLOT_MAPMAXY], ConvertHeight(255));

	CheckGLError(__FILE__, __LINE__);

	Matrix modelmat;
	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
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
	mvp.postmult(modelmat);
#endif
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

	CheckGLError(__FILE__, __LINE__);

#if 0
	a = Vec3f(wx * TILE_SIZE, WATER_LEVEL, wy * TILE_SIZE);
	b = Vec3f(0, WATER_LEVEL, wy * TILE_SIZE);
	c = Vec3f(0, WATER_LEVEL, 0);
	d = Vec3f(wx * TILE_SIZE, WATER_LEVEL, 0);

	float vertices[] =
	{
		a.x, a.y, a.z,
		b.x, b.y, b.z,
		c.x, c.y, c.z,
		d.x, d.y, d.z,
		a.x, a.y, a.z,
		c.x, c.y, c.z
	};

	float texcoords0[] =
	{
		(float)(wx+1), (float)(wy+1),
		0, (float)(wy+1),
		0, 0,
		(float)(wx+1), 0,
		(float)(wx+1), (float)(wy+1),
		0, 0
	};

	static const float normals[] =
	{
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0
	};

#endif

	Player* py = &g_player[g_localP];

	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);
	glPolygonOffset(1.0, 0.01/(g_zoom));
	//glPolygonOffset(1.0, 250.0);

	CheckGLError(__FILE__, __LINE__);

#if 0
	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	CheckGLError(__FILE__, __LINE__);
	//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, texcoords0);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords0);
	CheckGLError(__FILE__, __LINE__);
	//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, normals);
	glNormalPointer(GL_FLOAT, 0, normals);
	CheckGLError(__FILE__, __LINE__);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	CheckGLError(__FILE__, __LINE__);
#endif

	for(int x=0; x<g_hmap.m_widthx; x++)
		for(int z=0; z<g_hmap.m_widthy; z++)
		{
			if(!IsTileVis(g_localP, x, z))
				continue;
			
			//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &g_waterverts[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &g_watertexcos[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, &g_waternorms[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			glVertexPointer(3, GL_FLOAT, 0, &g_waterverts[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			glTexCoordPointer(2, GL_FLOAT, 0, &g_watertexcos[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);
			glNormalPointer(GL_FLOAT, 0, &g_waternorms[ (z * g_hmap.m_widthx + x) * 3 * 2 + 0 ]);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

	glDisable(GL_POLYGON_OFFSET_FILL);
	CheckGLError(__FILE__, __LINE__);
}
