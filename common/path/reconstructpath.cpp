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
#include "pathjob.h"
#include "reconstructpath.h"

void ReconstructPath(PathJob* pj, PathNode* endnode)
{
	pj->path->clear();

#if 1
	// Reconstruct the path, following the path steps
	for(PathNode* n = endnode; n; n = n->previous)
	{
		Vec2i npos = PathNodePos(n);
		
#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		g_log<<"Reconstruct "<<npos.x<<","<<npos.y<<" pjtype="<<(int)pj->pjtype<<std::endl;
		g_log.flush();
	}
#endif

		Vec2i cmpos( npos.x * PATHNODE_SIZE + PATHNODE_SIZE/2, npos.y * PATHNODE_SIZE + PATHNODE_SIZE/2 );
		pj->path->push_front(cmpos);
	}
#endif

	
#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		g_log<<"Reconstruct start "<<(pj->cmstartx/PATHNODE_SIZE)<<","<<(pj->cmstarty/PATHNODE_SIZE)<<" pjtype="<<(int)pj->pjtype<<std::endl;
		g_log.flush();
	}
#endif

#if 1	//necessary for exact point
	//pj->path->push_back(Vec2i(pj->goalx, pj->goalz)*PATHNODE_SIZE + PATHNODE_SIZE/2);
	if(pj->capend)
		pj->path->push_back(pj->cmgoal);
#endif

	if(pj->path->size() > 0)
		*pj->subgoal = *pj->path->begin();
}