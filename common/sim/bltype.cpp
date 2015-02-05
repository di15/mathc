#include "../render/model.h"
#include "bltype.h"
#include "../sound/sound.h"

BlType g_bltype[BL_TYPES];

BlType::BlType()
{
}

void DefB(int type, 
		  const char* name, 
		  Vec2i size, 
		  bool hugterr, 
		  const char* modelrelative, 
		  Vec3f scale, 
		  Vec3f translate, 
		  const char* cmodelrelative,  
		  Vec3f cscale, 
		  Vec3f ctranslate, 
		  int foundation, 
		  int reqdeposit,
		  int maxhp,
		  int visrange)
{
	BlType* t = &g_bltype[type];
	t->widthx = size.x;
	t->widthy = size.y;
	sprintf(t->name, name);
	QueueModel(&t->model, modelrelative, scale, translate);
	QueueModel(&t->cmodel, cmodelrelative, cscale, ctranslate);
	t->foundation = foundation;
	t->hugterr = hugterr;
	t->visrange = visrange;

	Zero(t->input);
	Zero(t->output);
	Zero(t->conmat);

	t->reqdeposit = reqdeposit;

	for(int i=0; i<BL_SOUNDS; i++)
		t->sound[i] = -1;

	t->maxhp = maxhp;
	t->manuf.clear();
}

void BMan(int type, unsigned char utype)
{
	BlType* t = &g_bltype[type];
	t->manuf.push_back(utype);
}

void BSound(int type, int stype, const char* relative)
{
	BlType* t = &g_bltype[type];
	LoadSound(relative, &t->sound[stype]);
}

void BDesc(int type, const char* desc)
{
	BlType* t = &g_bltype[type];
	t->desc = desc;
}

void BMat(int type, int res, int amt)
{
	BlType* t = &g_bltype[type];
	t->conmat[res] = amt;
}

void BIn(int type, int res, int amt)
{
	BlType* t = &g_bltype[type];
	t->input[res] = amt;
}

void BOut(int type, int res, int amt)
{
	BlType* t = &g_bltype[type];
	t->output[res] = amt;
}

void BEmitter(int type, int emitterindex, int ptype, Vec3f offset)
{
	BlType* t = &g_bltype[type];
	EmitterPlace* e = &t->emitterpl[emitterindex];
	*e = EmitterPlace(ptype, offset);
}
