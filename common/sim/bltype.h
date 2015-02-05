#ifndef BUILDINGTYPE_H
#define BUILDINGTYPE_H

#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "resources.h"
#include "../render/billboard.h"
#include "../render/particle.h"
#include "conduit.h"

#define MAX_B_EMITTERS	10

#define BLSND_PROD	0	//production sound
#define BLSND_FINI	1	//finished construction sound
#define BLSND_PLACE	2	//placed new construction project sound
#define BL_SOUNDS	3

class BlType
{
public:
	// width in tiles
	int widthx;
	int widthy;

	int model;
	int cmodel;

	char name[64];
	std::string desc;

	int foundation;

	int input[RESOURCES];
	int output[RESOURCES];

	int conmat[RESOURCES];

	int reqdeposit;

	EmitterPlace emitterpl[MAX_B_EMITTERS];

	bool hugterr;

	int sprite;
	
	std::list<unsigned char> manuf;

	short sound[BL_SOUNDS];
	int maxhp;

	int visrange;

	BlType();
};

#define FOUNDATION_LAND			0
#define FOUNDATION_COASTAL		1
#define FOUNDATION_SEA			2

#define BL_NONE				-1
#define BL_HOUSE			0
#define BL_STORE			1
#define BL_FACTORY			2
#define BL_FARM				3
#define BL_MINE				4
#define BL_SMELTER			5
#define BL_OILWELL			6
#define BL_REFINERY			7
#define BL_NUCPOW			8
#define BL_COALPOW			9
#define BL_CEMPLANT			10
#define BL_CHEMPLANT		11
#define BL_ELECPLANT		12
#define BL_GASSTATION		13
#define BL_TYPES			14

#define BL_ROAD				(BL_TYPES+CONDUIT_ROAD)
#define BL_POWL				(BL_TYPES+CONDUIT_POWL)
#define BL_CRPIPE			(BL_TYPES+CONDUIT_CRPIPE)
//#define BL_WATERPIPE		(BL_TYPES+4)

#define TOTAL_BUILDABLES	(BL_TYPES+CONDUIT_TYPES)

extern BlType g_bltype[BL_TYPES];

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
		  int visrange);
void BMat(int type, int res, int amt);
void BIn(int type, int res, int amt);
void BOut(int type, int res, int amt);
void BEmitter(int type, int emitterindex, int ptype, Vec3f offset);
void BDesc(int type, const char* desc);
void BSound(int type, int stype, const char* relative);
void BMan(int type, unsigned char utype);

#endif
