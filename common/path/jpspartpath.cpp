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

// Calculates a path.
void JPSPartPath(int utype, int umode, int cmstartx, int cmstarty, int target, int target2, int targtype,
                 std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* ignoreu, Building* ignoreb,
                 int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy,
                 int maxsearch)
{
	UType* ut = &g_utype[utype];

	PathJob* pj = new PathJob;
	pj->utype = utype;
	pj->umode = umode;
	pj->cmstartx = cmstartx;
	pj->cmstarty = cmstarty;
	pj->target = target;
	pj->target2 = target2;
	pj->targtype = targtype;
	pj->path = path;
	pj->subgoal = subgoal;
	pj->thisu = thisu ? thisu - g_unit : -1;
	pj->ignoreu = ignoreu ? ignoreu - g_unit : -1;
	pj->ignoreb = ignoreb ? ignoreb - g_building : -1;
	pj->goalx = (cmgoalminx+cmgoalmaxx)/2;
	pj->goalz = (cmgoalminy+cmgoalmaxy)/2;
	pj->goalx = pj->goalx / PATHNODE_SIZE;
	pj->goalz = pj->goalz / PATHNODE_SIZE;
	pj->goalminx = cmgoalminx;
	pj->goalminy = cmgoalminy;
	pj->goalmaxx = cmgoalmaxx;
	pj->goalmaxy = cmgoalmaxy;
	pj->roaded = ut->roaded;
	pj->landborne = ut->landborne;
	pj->seaborne = ut->seaborne;
	pj->airborne = ut->airborne;
	pj->callback = Callback_UnitPath;
	pj->pjtype = PATHJOB_JPSPART;
	pj->maxsearch = maxsearch;
	pj->cmgoal = Vec2i(cmgoalx, cmgoalz);

#if 0
	int sqmax = sqrt(maxsearch);
	int diagw = (sqmax - 1) / 2;

	pj->maxsubdiag = diagw;
	pj->maxsubdiagstraight = diagw;
	pj->maxsubstraight = diagw;
#endif

	// Returns the path from location `<startX, startY>` to location `<endX, endY>`.
	//return function(finder, startNode, endNode, clearance, toClear)

	pj->process();
	delete pj;
}
