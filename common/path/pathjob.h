#ifndef PATHJOB_H
#define PATHJOB_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"

class Unit;
class Building;
class PathNode;

#define PATH_DELAY		200
//#define PATH_DELAY		0

#define PATHJOB_JPS					0
#define PATHJOB_QUICKPARTIAL		1
#define PATHJOB_ASTAR				2
#define PATHJOB_JPSPART				3
#define PATHJOB_ANYPATH				4
#define PATHJOB_TILE				5
#define PATHJOB_BOUNDJPS			6
#define PATHJOB_BOUNDASTAR			7

#define PATHHEUR Magnitude
#define JOBHEUR Manhattan
#define FUELHEUR Manhattan
#define TRANHEUR Manhattan	//transport heuristic

//#define HIERDEBUG	//hierarchical pathfinding debug output

//#define POWCD_DEBUG
#ifdef POWCD_DEBUG
extern std::string powcdstr;
#endif

// byte-align structures
#pragma pack(push, 1)

class PathJob
{
public:
	//TO DO figure out which things can be shorts/ushorts/schars
	unsigned char utype;
	unsigned char umode;
	int cmstartx;
	int cmstarty;
	short target;
	short target2;
	signed char targtype;
	signed char cdtype;
	std::list<Vec2i> *path;
	std::list<Vec2s> *tpath;
	Vec2i *subgoal;
	short thisu;
	short ignoreu;
	short ignoreb;
	//int cmgoalx;
	//int cmgoalz;
	//might be in pathnodes, 
	//might be in centimeters,
	//depending on pathjob type.
	//TO DO set goalx,z to unsigned short once i test with map below MAX_MAP width tiles
	int goalx;
	int goalz;
	int goalminx;
	int goalminy;
	int goalmaxx;
	int goalmaxy;
	bool roaded;
	bool landborne;
	bool seaborne;
	bool airborne;
	int maxsearch;
	unsigned char pjtype;
	PathNode* closestnode;
	int closest;
	int searchdepth;
#if 0
	int maxsubsearch;
	int maxsubstraight;
	int maxsubdiag;
	int maxsubdiagstraight;
	int subsearchdepth;
#endif
	
	//search bounds nodes
	int nminx;
	int nminy;
	int nmaxx;
	int nmaxy;

	Vec2i cmgoal;
	bool capend;	//append cmgoal to path?
	bool allowpart;	//allow incomplete (closest node) path?

	void (*callback)(bool result, PathJob* pj);

	virtual bool process();
};
#pragma pack(pop)

extern std::list<PathNode*> g_toclear;

void Callback_UnitPath(bool result, PathJob* pj);
void ClearNodes(std::list<PathNode*> &toclear);
bool Trapped(Unit* u, Unit* ignoreu);

#endif
