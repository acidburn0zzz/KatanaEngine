/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SERVERPHYSICS__
#define __SERVERPHYSICS__

#include "server_main.h"

void Physics_CheckVelocity(edict_t *eEntity);
void Physics_CheckWaterTransition(edict_t *eEntity);
void Physics_SetGravity(edict_t *eEntity);

#endif
