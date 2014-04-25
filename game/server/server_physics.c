/*	Copyright (C) 2011-2013 OldTimes Software
*/
#include "server_physics.h"

/*
	Server-side physics.
*/

#define PHYSICS_MAXVELOCITY	2000.0f

void Server_CheckVelocity(edict_t *ent)
{
	int i;

	for(i = 0; i < 3; i++)
	{
		if(IS_NAN(ent->v.velocity[i]))
		{
			Engine.Con_DPrintf("Got a NaN velocity on %s\n",ent->v.cClassname);
			ent->v.velocity[i] = 0;
		}

		if(IS_NAN(ent->v.origin[i]))
		{
			Engine.Con_DPrintf("Got a NaN origin on %s\n",ent->v.cClassname);
			ent->v.origin[i] = 0;
		}

		if(ent->v.velocity[i] > PHYSICS_MAXVELOCITY)
			ent->v.velocity[i] = PHYSICS_MAXVELOCITY;
		else if(ent->v.velocity[i] < -PHYSICS_MAXVELOCITY)
			ent->v.velocity[i] = -PHYSICS_MAXVELOCITY;
	}
}

void Server_CheckWaterTransition(edict_t *ent)
{
	int	iCont = Engine.Server_PointContents(ent->v.origin);

	if(!ent->v.watertype)
	{
		ent->v.watertype	= iCont;
		ent->v.waterlevel	= 1.0f;
		return;
	}

	if(iCont <= BSP_CONTENTS_WATER)
	{
		ent->v.watertype	= iCont;
		ent->v.waterlevel	= 1.0f;

		Sound(ent,CHAN_AUTO,PHYSICS_SOUND_SPLASH,15,ATTN_NORM);
		return;
	}
	else
	{
		ent->v.watertype	= BSP_CONTENTS_EMPTY;
		ent->v.waterlevel	= (float)iCont;
	}
	
	Sound(ent,CHAN_AUTO,PHYSICS_SOUND_BODY,15,ATTN_NORM);
}

// [5/7/2012] Was SV_Impact ~hogsy
void Physics_EntityImpact(edict_t *eEntity,edict_t *eOther)
{
	if(eEntity->v.TouchFunction && eEntity->Physics.iSolid != SOLID_NOT)
		eEntity->v.TouchFunction(eEntity,eOther);

	if(eOther->v.TouchFunction && eOther->Physics.iSolid != SOLID_NOT)
		eOther->v.TouchFunction(eOther,eEntity);
}

trace_t	Server_PushEntity(edict_t *ent,vec3_t push)
{
	trace_t	trace;
	vec3_t	end;

	Math_VectorAdd(ent->v.origin,push,end);

	if(ent->v.movetype == MOVETYPE_FLYMISSILE)
		trace = Engine.Server_Move(ent->v.origin,ent->v.mins,ent->v.maxs,end,MOVE_MISSILE,ent);
	else if(ent->Physics.iSolid == SOLID_TRIGGER || ent->Physics.iSolid == SOLID_NOT)
		trace = Engine.Server_Move(ent->v.origin,ent->v.mins,ent->v.maxs,end,MOVE_NOMONSTERS,ent);
	else
		trace = Engine.Server_Move(ent->v.origin,ent->v.mins,ent->v.maxs,end,MOVE_NORMAL,ent);

	Math_VectorCopy(trace.endpos,ent->v.origin);

	Engine.LinkEntity(ent,true);

	if(trace.ent)
		Physics_EntityImpact(ent,trace.ent);

	return trace;
}