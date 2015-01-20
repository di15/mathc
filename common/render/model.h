#ifndef MODEL_H
#define MODEL_H

#include "../platform.h"
#include "ms3d.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"

class VertexArray;
class MS3DModel;
class Shader;

class Model
{
public:
	bool m_on;
	MS3DModel m_ms3d;
	VertexArray* m_va;
	int m_nframes;
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	unsigned int m_ownerm;
	std::string m_fullpath;

	Model()
	{
		m_va = NULL;
		m_on = false;
	}

	~Model();

	bool load(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue, bool blendnorm);
	void genvbo();
	void delvbo();
	void usetex() const;
	void draw(int frame, Vec3f pos, float yaw);
	void destroy();
};

#define MODELS	512
extern Model g_model[MODELS];

struct ModelToLoad
{
	int* id;
	char filepath[MAX_PATH];
	Vec3f scale;
	Vec3f translate;
	bool blendnorm;
};

extern int g_lastmodelload;

int NewModel();
int FindModel(const char* relative);
void QueueModel(int* id, const char* relative, Vec3f scale, Vec3f translate, bool blendnorm=false);
bool Load1Model();
void DrawVA(VertexArray* va, Vec3f pos);
void BeginVertexArrays();
void EndVertexArrays();
bool PlayAni(float& frame, int first, int last, bool loop, float rate);
bool PlayAniB(float& frame, int first, int last, bool loop, float rate);	//Play animation backwards
void FreeModels();
int LoadModel(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue, bool blendnorm);
void ReloadModels();

#endif
