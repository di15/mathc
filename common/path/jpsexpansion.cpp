#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../sim/road.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../phys/trace.h"
#include "../sys/binheap.h"
#include "jpspath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "jpsexpansion.h"
#include "pathjob.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/transport.h"

//#define TRANSPORT_DEBUG

// https://github.com/Yonaba/Jumper/blob/master/jumper/search/jps.lua

// Returns neighbours. The returned value is an array of __walkable__ nodes neighbouring a given `node`.
std::list<PathNode*> GetNeighbours(PathJob* pj, PathNode* node)
{
	std::list<PathNode*> neighbours;

	Vec2i pos = PathNodePos(node);

	for( int i=0; i<ARRSZ(straightoffsets); i++ )
	{
		PathNode* n = PathNodeAt(
			pos.x + straightoffsets[i].x,
			pos.y + straightoffsets[i].y
			);

		Vec2i p = PathNodePos(n);

		if( n && Standable(pj, p.x, p.y) )
		{
			neighbours.push_back(n);
		}
	}

	if( !1 )
	{
		return neighbours;    //if not allow diagonal
	}

	for( int i=0; i<ARRSZ(diagonaloffsets); i++ )
	{
		PathNode* n = PathNodeAt(
			pos.x + diagonaloffsets[i].x,
			pos.y + diagonaloffsets[i].y
			);

		Vec2i p = PathNodePos(n);

		bool tunnel = false;

		if( n && Standable(pj, p.x, p.y) )
		{
			if( tunnel )
			{
				neighbours.push_back(n);
			}
			else
			{
				bool skip = false;
				//PathNode* n1 = PathNodeAt(p.x+diagonaloffsets[i].x, p.y);
				//PathNode* n2 = PathNodeAt(p.x, p.y+diagonaloffsets[i].y);
				//polyf fix
				PathNode* n1 = PathNodeAt(pos.x+diagonaloffsets[i].x, pos.y);
				PathNode* n2 = PathNodeAt(pos.x, pos.y+diagonaloffsets[i].y);
				Vec2i p1 = PathNodePos(n1);
				Vec2i p2 = PathNodePos(n2);
				
				//if( (n1 && n2) && !Standable(pj, p1.x, p1.y) && !Standable(pj, p2.x, p2.y) )
				//polyf fix
				if( !n1 || !n2 || !Standable(pj, p1.x, p1.y) || !Standable(pj, p2.x, p2.y) )
				{
					skip = true;
				}
#ifdef RANDOM8DEBUG
				if(pj->thisu == thatunit)
				{
					g_log<<"diag off PATH skip="<<skip<<" off="<<diagonaloffsets[i].x<<","<<diagonaloffsets[i].y<<" Standable(pj, p1.x, p1.y)="<<Standable(pj, p1.x, p1.y)<<" p1="<<p1.x<<","<<p1.y<<" "<<std::endl;
				}
#endif

				if( !skip )
				{
					neighbours.push_back(n);
				}
			}
		}
	}

	return neighbours;
}


/*
Looks for the neighbours of a given node.
Returns its natural neighbours plus forced neighbours when the given
node has no parent (generally occurs with the starting node).
Otherwise, based on the direction of move from the parent, returns
neighbours while pruning directions which will lead to symmetric paths.

In case diagonal moves are forbidden, when the given node has no
parent, we return straight neighbours (up, down, left and right).
Otherwise, we add left and right node (perpendicular to the direction
of move) in the neighbours std::list.
*/
std::list<PathNode*> FindNeighbours(PathJob* pj, PathNode* node)
{
	if( node->previous && node->previous != node )
	{
		std::list<PathNode*> neighbours;

		Vec2i pos = PathNodePos(node);

		// Node have a parent, we will prune some neighbours
		// Gets the direction of move
		Vec2i delta = pos - PathNodePos(node->previous);
		Vec2i prevpos = PathNodePos(node->previous);
		delta.x = (pos.x-prevpos.x)/imax(abs(pos.x-prevpos.x),1);
		delta.y = (pos.y-prevpos.y)/imax(abs(pos.y-prevpos.y),1);

		// Diagonal move case
		if( delta.x!=0 && delta.y!=0 )
		{
			bool walkY = false;
			bool walkX = false;

			// Natural neighbours
			if( Standable(pj, pos.x, pos.y+delta.y) )
			{
				neighbours.push_back(PathNodeAt(pos.x, pos.y+delta.y));
				walkY = true;
			}
			if( Standable(pj, pos.x+delta.x, pos.y) )
			{
				neighbours.push_back(PathNodeAt(pos.x+delta.x, pos.y));
				walkX = true;
			}

#if 0
			if( walkX || walkY ) neighbours.push_back(PathNodeAt(pos.x+delta.x, pos.y+delta.y));
#else
			// polyf edit - trip corners
			if( walkX && walkY ) neighbours.push_back(PathNodeAt(pos.x+delta.x, pos.y+delta.y));
#endif

			// Forced neighbours
#if 0
			if( !Standable(pj, pos.x-delta.x, pos.y) && walkY ) neighbours.push_back(PathNodeAt(pos.x-delta.x, pos.y+delta.y));
			if( !Standable(pj, pos.x, pos.y-delta.y) && walkX ) neighbours.push_back(PathNodeAt(pos.x+delta.x, pos.y-delta.y));
#elif 0
			// polyf edit - trip corners
			if( !Standable(pj, pos.x-delta.x, pos.y) && walkY && walkX ) neighbours.push_back(PathNodeAt(pos.x-delta.x, pos.y+delta.y));
			if( !Standable(pj, pos.x, pos.y-delta.y) && walkX && walkY ) neighbours.push_back(PathNodeAt(pos.x+delta.x, pos.y-delta.y));
#else
			// polyf edit - trip corners

#endif
		}
		else
		{
			// Move along Y-axis case
			if( delta.x==0 )
			{
				bool walkY = false;

				if( Standable(pj, pos.x, pos.y+delta.y) )
				{
					neighbours.push_back( PathNodeAt(pos.x, pos.y+delta.y) );

#if 0
					// Forced neighbours are left and right ahead along Y
					if( !Standable(pj, pos.x+1, pos.y) ) neighbours.push_back( PathNodeAt(pos.x+1, pos.y+delta.y) );
					if( !Standable(pj, pos.x-1, pos.y) ) neighbours.push_back( PathNodeAt(pos.x-1, pos.y+delta.y) );
#elif 0
					//polyf edit - trip corners
					if( Standable(pj, pos.x, pos.y+delta.y) )
					{
						if( !Standable(pj, pos.x+1, pos.y-delta.y) && Standable(pj, pos.x+1, pos.y) ) neighbours.push_back( PathNodeAt(pos.x+1, pos.y+delta.y) );
						if( !Standable(pj, pos.x-1, pos.y-delta.y) && Standable(pj, pos.x-1, pos.y) ) neighbours.push_back( PathNodeAt(pos.x-1, pos.y+delta.y) );
					}
#else
				}
#endif
				//polyf edit - trip corners
				if( !Standable(pj, pos.x+1, pos.y-delta.y) )
				{
					neighbours.push_back( PathNodeAt(pos.x+1, pos.y) );

					if( Standable(pj, pos.x, pos.y+delta.y) && Standable(pj, pos.x+1, pos.y) )
						neighbours.push_back( PathNodeAt(pos.x+1, pos.y+delta.y) );
				}
				if( !Standable(pj, pos.x-1, pos.y-delta.y) )
				{
					neighbours.push_back( PathNodeAt(pos.x-1, pos.y) );

					if( Standable(pj, pos.x, pos.y+delta.y) && Standable(pj, pos.x-1, pos.y) )
						neighbours.push_back( PathNodeAt(pos.x-1, pos.y+delta.y) );
				}

				// In case diagonal moves are forbidden : Needs to be optimized
				if( !1 ) //if not allow diagonal
				{
					if( Standable(pj, pos.x+1, pos.y) ) neighbours.push_back( PathNodeAt(pos.x+1, pos.y) );
					if( Standable(pj, pos.x-1, pos.y) ) neighbours.push_back( PathNodeAt(pos.x-1, pos.y) );
				}
			}
			else
			{
				// Move along X-axis case
				if( Standable(pj, pos.x+delta.x, pos.y) )
				{
					neighbours.push_back( PathNodeAt(pos.x+delta.x, pos.y) );

					// Forced neighbours are up and down ahead along X
#if 0
					if( !Standable(pj, pos.x, pos.y+1) ) neighbours.push_back( PathNodeAt(pos.x+delta.x, pos.y+1) );
					if( !Standable(pj, pos.x, pos.y-1) ) neighbours.push_back( PathNodeAt(pos.x+delta.x, pos.y-1) );
#elseif 0
					//polyf edit - trip corners
					if( Standable(pj, pos.x+delta.x, pos.y) )
					{
						if( !Standable(pj, pos.x-delta.x, pos.y+1) && Standable(pj, pos.x, pos.y+1) ) neighbours.push_back( PathNodeAt(pos.x+delta.x, pos.y+1) );
						if( !Standable(pj, pos.x-delta.x, pos.y-1) && Standable(pj, pos.x, pos.y-1) ) neighbours.push_back( PathNodeAt(pos.x+delta.x, pos.y-1) );
					}
#else
				}
#endif

				//polyf edit - trip corners
				if( !Standable(pj, pos.x-delta.x, pos.y+1) )
				{
					neighbours.push_back( PathNodeAt(pos.x, pos.y+1) );

					if( Standable(pj, pos.x+delta.x, pos.y) && Standable(pj, pos.x, pos.y+1) )
						neighbours.push_back( PathNodeAt(pos.x+delta.x, pos.y+1) );
				}
				if( !Standable(pj, pos.x-delta.x, pos.y-1) )
				{
					neighbours.push_back( PathNodeAt(pos.x, pos.y-1) );

					if( Standable(pj, pos.x+delta.x, pos.y) && Standable(pj, pos.x, pos.y-1) )
						neighbours.push_back( PathNodeAt(pos.x+delta.x, pos.y-1) );
				}

				// : In case diagonal moves are forbidden
				if( !1 ) //if no diagonal
				{
					if( Standable(pj, pos.x, pos.y+1) ) neighbours.push_back( PathNodeAt(pos.x, pos.y+1) );
					if( Standable(pj, pos.x, pos.y-1) ) neighbours.push_back( PathNodeAt(pos.x, pos.y-1) );
				}
			}
		}

		return neighbours;
	}

	// Node do not have parent, we return all neighbouring nodes
	return GetNeighbours(pj, node);
}


/*
Searches for a jump point (or a turning point) in a specific direction.
This is a generic translation of the algorithm 2 in the paper:
http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf
The current expanded node is a jump point if( near a forced node

In case diagonal moves are forbidden, when lateral nodes (perpendicular to
the direction of moves are walkable, we force them to be turning points in other
to perform a straight move.
*/

PathNode* Jump(PathJob* pj, PathNode* node, PathNode* parent)
{
	if(!node) return NULL;

	Vec2i pos = PathNodePos(node);
	
#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		g_log<<"pathing at("<<pos.x<<","<<pos.y<<") dgoal("<<(pj->goalx-pos.x)<<","<<(pj->goalz-pos.y)<<std::endl;
		g_log.flush();
	}
#endif

	pj->searchdepth++;

	// If the node to be examined is unwalkable, return nil
	if( !Standable(pj, pos.x, pos.y) )
	{
		
#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		g_log<<"\t !st1 sd"<<pj->searchdepth<<" "<<std::endl;
	}
#endif

		return NULL;
	}

	// If the node to be examined is the endNode, return this node
	if( AtGoal(pj, node) )
	{
#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		g_log<<"AT GOAL SUCCESS pathing at("<<pos.x<<","<<pos.y<<") dgoal("<<(pj->goalx-pos.x)<<","<<(pj->goalz-pos.y)<<std::endl;
		g_log.flush();
	}
#endif

		return node;
	}
	
	Vec2i delta = pos - PathNodePos(parent);

	// Diagonal search case
	if( delta.x!=0 && delta.y!=0 )
	{
#if 0	//?works?
		// Current node is a jump point if one of his leftside/rightside neighbours ahead is forced
		if( (Standable(pj, pos.x-delta.x, pos.y+delta.y) && !Standable(pj, pos.x-delta.x, pos.y)) ||
			(Standable(pj, pos.x+delta.x, pos.y-delta.y) && !Standable(pj, pos.x, pos.y-delta.y)) )
		{
			return node;
		}
#elif 0
		//try fix polyf
		if( (Standable(pj, pos.x, pos.y+delta.y) && !Standable(pj, pos.x-delta.x, pos.y+delta.y)) ||
			(Standable(pj, pos.x+delta.x, pos.y) && !Standable(pj, pos.x+delta.x, pos.y-delta.y)) )
		{
			return node;
		}
		//this shouldn't return, should discover jump point with straight horizontal search.
		//this should return only when corners can be cut, which would mean a collision in our case.
#elif 0
		//try fix polyf 2
		if( (Standable(pj, pos.x, pos.y+delta.y) && !Standable(pj, pos.x-delta.x, pos.y+delta.y)) ||
			(Standable(pj, pos.x+delta.x, pos.y) && !Standable(pj, pos.x+delta.x, pos.y-delta.y)) )
		{
			return node;
		}
#endif
	}
	else
	{
		// Search along X-axis case
		if( delta.x!=0 )
		{
			if( 1 )		//allow diagonal?
			{
#if 0
				// Current node is a jump point if one of his upside/downside neighbours is forced
				if( (Standable(pj, pos.x+delta.x, pos.y+1) && !Standable(pj, pos.x, pos.y+1)) ||
					(Standable(pj, pos.x+delta.x, pos.y-1) && !Standable(pj, pos.x, pos.y-1)) )
#else
				//polyf edit - corner fix
				if( (Standable(pj, pos.x, pos.y+1) && !Standable(pj, pos.x-delta.x, pos.y+1)) ||
					(Standable(pj, pos.x, pos.y-1) && !Standable(pj, pos.x-delta.x, pos.y-1)) )
#endif
				{

					
#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		g_log<<"\t deltax!=0 if sd"<<pj->searchdepth<<" "<<std::endl;
	}
#endif

					return node;
				}
				//polyf fix
				else
					return Jump(pj, PathNodeAt(pos.x+delta.x, pos.y), node);
			}
			else
			{
				// : in case diagonal moves are forbidden
				if( Standable(pj, pos.x+1, pos.y) || Standable(pj, pos.x-1, pos.y) )
				{
								
#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		g_log<<"\t deltax!=0 else sd"<<pj->searchdepth<<" "<<std::endl;
	}
#endif

					return node;
				}
			}
		}
		else if( delta.y!=0 )
		{
			// Search along Y-axis case
			// Current node is a jump point if one of his leftside/rightside neighbours is forced
			if( 1 ) //allow diagonal?
			{
#if 0
				if( (Standable(pj, pos.x+1, pos.y+delta.y) && !Standable(pj, pos.x+1, pos.y)) ||
					(Standable(pj, pos.x-1, pos.y+delta.y) && !Standable(pj, pos.x-1, pos.y)) )
#else
				if( (Standable(pj, pos.x+1, pos.y) && !Standable(pj, pos.x+1, pos.y-delta.y)) ||
					(Standable(pj, pos.x-1, pos.y) && !Standable(pj, pos.x-1, pos.y-delta.y)) )
#endif
				{
					return node;
				}
				//polyf fix
				else
					return Jump(pj, PathNodeAt(pos.x, pos.y+delta.y), node);
			}
			else
			{
				// : in case diagonal moves are forbidden
				if( Standable(pj, pos.x, pos.y+1) || Standable(pj, pos.x, pos.y-1) )
				{
					return node;
				}
			}
		}
	}

	// Recursive horizontal/vertical search
	if( delta.x!=0 && delta.y!=0 )
	{
		PathNode* jump;
		if( jump = Jump(pj, PathNodeAt(pos.x+delta.x, pos.y), node) )
		{
			//jump->previous = node;
			return node;
		}
		if( jump = Jump(pj, PathNodeAt(pos.x, pos.y+delta.y), node) )
		{
			//jump->previous = node;
			return node;
		}

		// Recursive diagonal search
		if( 1 ) //allow diagonal?
		{
#if 0
			if( Standable(pj, pos.x+delta.x, pos.y) || Standable(pj, pos.x, pos.y+delta.y) )
			{
				return Jump(pj, PathNodeAt(pos.x+delta.x, pos.y+delta.y), node);
			}
#else
			// polyf edit - tripping on corners
			if( Standable(pj, pos.x+delta.x, pos.y) && Standable(pj, pos.x, pos.y+delta.y) )
			{
				PathNode* jump = Jump(pj, PathNodeAt(pos.x+delta.x, pos.y+delta.y), node);
				return jump;
			}
#endif
		}
	}

	return NULL;
}

/*
Searches for successors of a given node in the direction of each of its neighbours.
This is a generic translation of the algorithm 1 in the paper:
http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf

Also, we notice that processing neighbours in a reverse order producing a natural
looking path, as the pathfinder tends to keep heading in the same direction.
In case a jump point was found, and this node happened to be diagonal to the
node currently expanded in a straight mode search, we skip this jump point.
*/
void Expand_JPS(PathJob* pj, PathNode* node)
{
	PathNode* endnode = NULL;
	Vec2i pos = PathNodePos(node);

	// Gets the valid neighbours of the given node
	// Looks for a jump point in the direction of each neighbour
	std::list<PathNode*> neighbours = FindNeighbours(pj, node);

	
#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		g_log<<"find n "<<neighbours.size()<<std::endl;
	}
#endif

	for( auto niter = neighbours.rbegin(); niter != neighbours.rend(); niter++ )
	{
		bool skip = false;
		PathNode* neighbour = *niter;

		Vec2i npos = PathNodePos(neighbour);

		PathNode* jumpnode = Jump(pj, neighbour, node);

		Vec2i jumppos = PathNodePos(jumpnode);
		
#ifdef TRANSPORT_DEBUG
	//if(thisu >= 0 && g_unit[thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int)thisu);
		rt.m_part.push_back(t);
		//AddChat(&rt);
		g_log<<"jump? "<<(bool)(jumpnode != NULL)<<std::endl;
	}
#endif

		// : in case a diagonal jump point was found in straight mode, skip it.
		if( jumpnode && !1 ) // and not allow diagonal
		{
			if( jumppos.x != pos.x && jumppos.y != pos.y )
			{
				skip = true;
			}
		}

		// Performs regular A-star on a set of jump points
		if( jumpnode && !skip )
		{
			//int jumpi = jumpnode - g_pathnode;

			// Update the jump node and move it in the closed std::list if it wasn't there
			if( !jumpnode->closed )
			{
				int extraD = PATHHEUR(jumppos - pos) << 1;
				int newD = node->totalD + extraD;

				if( !jumpnode->opened || newD < jumpnode->totalD )
				{
					g_toclear.push_back(jumpnode); // Records this node to reset its properties later.
					jumpnode->totalD = newD;
					int H = PATHHEUR( jumppos - Vec2i(pj->goalx, pj->goalz) ) << 1;
					jumpnode->score = jumpnode->totalD + H;
					jumpnode->previous = node;

					if( !jumpnode->opened )
					{
						g_openlist.insert(jumpnode);
						jumpnode->opened = true;
					}
					else
					{
						g_openlist.heapify(jumpnode);
					}
				}
			}
		}
	}
}
