#include "../platform.h"
#include "../math/3dmath.h"
#include "model.h"
#include "../texture.h"
#include "../utils.h"
#include "../gui/gui.h"
#include "shader.h"
#include "../debug.h"
#include "vertexarray.h"
#include "dmdmodel.h"
#include "shadow.h"

Model g_model[MODELS];
std::vector<ModelToLoad> g_modelsToLoad;
int g_lastmodelload = -1;

Model::~Model()
{
	destroy();
}

void Model::destroy()
{
	m_on = false;

	if(m_va == NULL)
		return;

	delete [] m_va;
	m_va = NULL;

	m_ms3d.destroy();
}

int NewModel()
{
	for(int i=0; i<MODELS; i++)
		if(!g_model[i].m_on)
			return i;

	return -1;
}

void QueueModel(int* id, const char* relative, Vec3f scale, Vec3f translate, bool blendnorm)
{
	ModelToLoad toLoad;
	toLoad.id = id;
	strcpy(toLoad.filepath, relative);
	toLoad.scale = scale;
	toLoad.translate = translate;
	toLoad.blendnorm = blendnorm;

	g_modelsToLoad.push_back(toLoad);
}

bool Load1Model()
{

	if(g_lastmodelload+1 < g_modelsToLoad.size())
		Status(g_modelsToLoad[g_lastmodelload+1].filepath);

	if(g_lastmodelload >= 0)
	{
		int id = NewModel();
		g_model[id].load(g_modelsToLoad[g_lastmodelload].filepath, g_modelsToLoad[g_lastmodelload].scale, g_modelsToLoad[g_lastmodelload].translate, false, g_modelsToLoad[g_lastmodelload].blendnorm);
		(*g_modelsToLoad[g_lastmodelload].id) = id;
	}

	g_lastmodelload ++;

	if(g_lastmodelload >= g_modelsToLoad.size())
	{
		g_modelsToLoad.clear();
		return false;	// Done loading all models
	}

	return true;	// Not finished loading models
}

void DrawVA(VertexArray* va, Vec3f pos)
{
	Shader* s = &g_shader[g_curS];

	Matrix modelmat;
	modelmat.translation((const float*)&pos);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

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

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);

#if 0
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);

	if(s->m_slot[SSLOT_NORMAL] != -1)
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#else
	glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_POSITION]);
	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_TEXCOORD]);
	//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	//if(s->m_slot[SSLOT_NORMAL] != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_NORMAL]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, 0);
		glNormalPointer(GL_FLOAT, 0, 0);
	}
#endif

	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void Model::usetex() const
{
	Shader* s = &g_shader[g_curS];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_diffusem ].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_specularm ].texname);
	glUniform1i(s->m_slot[SSLOT_SPECULARMAP], 1);

	//if(s->m_slot[SSLOT_NORMAL] != -1)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, g_texture[ m_normalm ].texname);
		glUniform1i(s->m_slot[SSLOT_NORMALMAP], 2);
	}

	//if(s->m_slot[SSLOT_OWNERMAP] != -1)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, g_texture[ m_ownerm ].texname);
		glUniform1i(s->m_slot[SSLOT_OWNERMAP], 3);
	}
}

void Model::draw(int frame, Vec3f pos, float yaw)
{
	if(g_curS == SHADER_UNIT)
		StartTimer(TIMER_DRAWUMAT);

	Shader* s = &g_shader[g_curS];

	float pitch = 0;
	Matrix modelmat;
	float radians[] = {(float)DEGTORAD(pitch), (float)DEGTORAD(yaw), 0};
	modelmat.translation((const float*)&pos);
	Matrix rotation;
	rotation.rotrad(radians);
	modelmat.postmult(rotation);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

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

	VertexArray* va = &m_va[frame];


	if(g_curS == SHADER_UNIT)
	{
		StopTimer(TIMER_DRAWUMAT);
		StartTimer(TIMER_DRAWUTEXBIND);
	}

	usetex();

	if(g_curS == SHADER_UNIT)
	{
		StopTimer(TIMER_DRAWUTEXBIND);
		StartTimer(TIMER_DRAWUGL);
	}

#if 0
	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);

	if(s->m_slot[SSLOT_NORMAL] != -1)
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#else
	glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_POSITION]);
	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_TEXCOORD]);
	//glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

#if 0
	if(s->m_slot[SSLOT_NORMAL] != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_NORMAL]);
		glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, 0);
	}
#else
	glBindBuffer(GL_ARRAY_BUFFER, va->vbo[VBO_NORMAL]);
	glNormalPointer(GL_FLOAT, 0, 0);
#endif
#endif

	glDrawArrays(GL_TRIANGLES, 0, va->numverts);

	if(g_curS == SHADER_UNIT)
		StopTimer(TIMER_DRAWUGL);
}

int FindModel(const char* relative)
{
	char full[MAX_PATH+1];
	FullPath(relative, full);
	char corrected[MAX_PATH+1];
	strcpy(corrected, full);
	CorrectSlashes(corrected);

	for(int i=0; i<MODELS; i++)
	{
		Model* m = &g_model[i];

		if(!m->m_on)
			continue;

		if(stricmp(m->m_fullpath.c_str(), corrected) == 0)
			return i;
	}

	return -1;
}


int LoadModel(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue, bool blendnorm)
{
	int i = FindModel(relative);

	if(i >= 0)
		return i;

	i = NewModel();

	if(i < 0)
		return i;

	if(g_model[i].load(relative, scale, translate, dontqueue, blendnorm))
		return i;

	return -1;
}

bool Model::load(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue, bool blendnorm)
{
	m_diffusem = 0;
	m_specularm = 0;
	m_normalm = 0;
	m_ownerm = 0;

	if(LoadDMDModel(&m_va, m_diffusem, m_specularm, m_normalm, m_ownerm, scale, translate, relative, dontqueue))
	{
		m_on = true;
		char full[MAX_PATH+1];
		FullPath(relative, full);
		char corrected[MAX_PATH+1];
		strcpy(corrected, full);
		CorrectSlashes(corrected);
		m_fullpath = corrected;
		m_nframes = 1;
		genvbo();
		return true;
	}

	if(m_ms3d.load(relative, m_diffusem, m_specularm, m_normalm, m_ownerm, dontqueue))
	{
		m_on = true;
		m_ms3d.genva(&m_va, scale, translate, relative, blendnorm);
		char full[MAX_PATH+1];
		FullPath(relative, full);
		char corrected[MAX_PATH+1];
		strcpy(corrected, full);
		CorrectSlashes(corrected);
		m_fullpath = corrected;
		m_nframes = m_ms3d.m_totalFrames;
		genvbo();
		return true;
	}

	return false;
}

void Model::genvbo()
{
	delvbo();

	for(int i=0; i<m_nframes; i++)
		m_va[i].genvbo();
}

void Model::delvbo()
{
	for(int i=0; i<m_nframes; i++)
		m_va[i].delvbo();
}

bool PlayAni(float& frame, int first, int last, bool loop, float rate)
{
	if(frame < first || frame > last+1)
	{
		frame = first;
		return false;
	}

	frame += rate;

	if(frame > last)
	{
		if(loop)
			frame = first;
		else
			frame = last;

		return true;
	}

	return false;
}

//Play animation backwards
bool PlayAniB(float& frame, int first, int last, bool loop, float rate)
{
	if(frame < first-1 || frame > last)
	{
		frame = last;
		return false;
	}

	frame -= rate;

	if(frame < first)
	{
		if(loop)
			frame = last;
		else
			frame = first;

		return true;
	}

	return false;
}

void FreeModels()
{
	for(int i=0; i<MODELS; i++)
	{
		Model* m = &g_model[i];

		if(!m->m_on)
			continue;

		m->destroy();
	}
}


void ReloadModels()
{

}