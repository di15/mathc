

#ifndef TILEPATH_H
#define TILEPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2s.h"
#include "../platform.h"

class PathJob;
class PathNode;
class Unit;
class Building;

extern unsigned int pathnum;

void TilePath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype, signed char cdtype,
               std::list<Vec2s> *tpath, Unit* thisu, Unit* ignoreu, Building* ignoreb,
               int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
               int maxsearch);

void Expand_T(PathJob* pj, PathNode* node);

void UpdJams();

#endif