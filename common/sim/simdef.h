#ifndef SIMDEF_H
#define SIMDEF_H

#ifndef MATCHMAKER
#include "../render/heightmap.h"
#include "../platform.h"
#include "../texture.h"
#include "../render/model.h"
#include "../sim/resources.h"
#include "../gui/icon.h"
#include "../sim/bltype.h"
#include "../sim/utype.h"
#include "../render/foliage.h"
#include "../sim/road.h"
#include "../sim/powl.h"
#include "../sim/crpipe.h"
#include "../render/water.h"
#include "../sim/selection.h"
#include "../sound/sound.h"
#include "../render/particle.h"
#include "../render/skybox.h"
#include "../gui/sprite.h"
#include "../gui/cursor.h"
#include "../sim/player.h"
#endif

#define SIM_FRAME_RATE		30

#define AVG_DIST		(TILE_SIZE*6)
#define CYCLE_FRAMES	(SIM_FRAME_RATE*60)

#define MAXJOBDIST		(TILE_SIZE*20)

//labourer unit starting items
//#define STARTING_LABOUR		1000	//lasts for 33.333 seconds
#define STARTING_LABOUR			10		//lasts for 33.333 seconds
#define LABOURER_FOODCONSUM		1
#define LABOURER_ENERGYCONSUM	1
//#define STARTING_RETFOOD		9000	//lasts for 5 minutes
#define STARTING_RETFOOD		(5 * CYCLE_FRAMES/SIM_FRAME_RATE * LABOURER_FOODCONSUM)	//lasts for 5 minutes
#define MUL_RETFOOD				(STARTING_RETFOOD*2)
#define DRIVE_WORK_DELAY		(SIM_FRAME_RATE*1)	//in sim frames
#define WORK_DELAY				(SIM_FRAME_RATE*1)	//in sim frames
#define LOOKJOB_DELAY_MAX		(SIM_FRAME_RATE*5)
#define FUEL_DISTANCE			(TILE_SIZE)		//(TILE_SIZE*2.0f)
#define	SHOP_RATE				200
#define TRUCK_CONSUMPRATE		1
#define TBID_DELAY				200	//truck job bid delay
#define STARTING_FUEL			(10 * CYCLE_FRAMES/SIM_FRAME_RATE * TRUCK_CONSUMPRATE)	//lasts for 10 minute(s)

void Queue();

#endif
