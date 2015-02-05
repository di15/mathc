

#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include "../platform.h"
#include "../sim/player.h"

class VisTile
{
public:
	std::list<short> uvis;
	std::list<short> bvis;
	bool explored[PLAYERS];

	VisTile()
	{
		for(int i=0; i<PLAYERS; i++)
			explored[i] = false;
	}
};

extern VisTile* g_vistile;

class Unit;
class Building;

void RemVis(Unit* u);
void AddVis(Unit* u);
void RemVis(Building* b);
void AddVis(Building* b);
void Explore(Unit* u);
void Explore(Building* b);
bool IsTileVis(short py, short tx, short ty);
bool Explored(short py, short tx, short ty);

#endif