/*	Copyright (C) 2011-2013 OldTimes Software
*/
#include "server_physics.h"

/*
	Server-Side Physics
*/

#define PHYSICS_MAXVELOCITY	2000.0f

/*	Sets the amount of gravity for the object.
*/
void Physics_SetGravity(edict_t *eEntity)
{	
	eEntity->v.velocity[2] -=
		// [30/5/2013] Slightly more complex gravity management ~hogsy
		(eEntity->Physics.fGravity*cvServerGravityTweak.value)*
		eEntity->Physics.fMass*
		(float)Engine.Server_GetFrameTime();
}

void Physics_CheckVelocity(edict_t *eEntity)
{
	int i;

	for(i = 0; i < 3; i++)
	{
		if (pMath_ISNAN(eEntity->v.velocity[i]))
		{
			Engine.Con_DPrintf("Got a NaN velocity on %s\n",eEntity->v.cClassname);
			eEntity->v.velocity[i] = 0;
		}

		if (pMath_ISNAN(eEntity->v.origin[i]))
		{
			Engine.Con_DPrintf("Got a NaN origin on %s\n",eEntity->v.cClassname);
			eEntity->v.origin[i] = 0;
		}

		if(eEntity->v.velocity[i] > PHYSICS_MAXVELOCITY)
			eEntity->v.velocity[i] = PHYSICS_MAXVELOCITY;
		else if(eEntity->v.velocity[i] < -PHYSICS_MAXVELOCITY)
			eEntity->v.velocity[i] = -PHYSICS_MAXVELOCITY;
	}
}

void Physics_CheckWaterTransition(edict_t *eEntity)
{
	int	iCont = Engine.Server_PointContents(eEntity->v.origin);

	if(!eEntity->v.watertype)
	{
		eEntity->v.watertype	= iCont;
		eEntity->v.waterlevel	= 1.0f;
		return;
	}

	if(iCont <= BSP_CONTENTS_WATER)
	{
		eEntity->v.watertype	= iCont;
		eEntity->v.waterlevel	= 1.0f;

		Sound(eEntity,CHAN_AUTO,PHYSICS_SOUND_SPLASH,15,ATTN_NORM);
		return;
	}
	else
	{
		eEntity->v.watertype	= BSP_CONTENTS_EMPTY;
		eEntity->v.waterlevel	= (float)iCont;
	}
	
	Sound(eEntity,CHAN_AUTO,PHYSICS_SOUND_BODY,15,ATTN_NORM);
}

// [5/7/2012] Was SV_Impact ~hogsy
void Physics_EntityImpact(edict_t *eEntity,edict_t *eOther)
{
	if(eEntity->v.TouchFunction && eEntity->Physics.iSolid != SOLID_NOT)
		eEntity->v.TouchFunction(eEntity,eOther);

	if(eOther->v.TouchFunction && eOther->Physics.iSolid != SOLID_NOT)
		eOther->v.TouchFunction(eOther,eEntity);
}

trace_t	Physics_PushEntity(edict_t *eEntity,vec3_t vPush)
{
	trace_t	trace;
	vec3_t	end;

	Math_VectorAdd(eEntity->v.origin,vPush,end);

	if(eEntity->v.movetype == MOVETYPE_FLYMISSILE)
		trace = Engine.Server_Move(eEntity->v.origin,eEntity->v.mins,eEntity->v.maxs,end,MOVE_MISSILE,eEntity);
	else if(eEntity->Physics.iSolid == SOLID_TRIGGER || eEntity->Physics.iSolid == SOLID_NOT)
		trace = Engine.Server_Move(eEntity->v.origin,eEntity->v.mins,eEntity->v.maxs,end,MOVE_NOMONSTERS,eEntity);
	else
		trace = Engine.Server_Move(eEntity->v.origin,eEntity->v.mins,eEntity->v.maxs,end,MOVE_NORMAL,eEntity);

	Math_VectorCopy(trace.endpos,eEntity->v.origin);

	Engine.LinkEntity(eEntity,true);

	if(trace.ent)
		Physics_EntityImpact(eEntity,trace.ent);

	return trace;
}