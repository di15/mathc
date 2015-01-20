#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "../platform.h"
#include "../gui/richtext.h"
#include "../math/vec3f.h"

class Transaction
{
public:
	RichText rtext;
	Vec3f drawpos;
	float life;
	float halfwidth;
};

#define TRANSACTION_RISE		(15.0f*30.0f)
#define TRANSACTION_DECAY		(0.015f*30.0f)

extern std::list<Transaction> g_transx;
extern bool g_drawtransx;

class Matrix;

void DrawTransactions(Matrix projmodlview);
void NewTransx(Vec3f pos, const RichText* rtext);
void FreeTransx();

#endif
