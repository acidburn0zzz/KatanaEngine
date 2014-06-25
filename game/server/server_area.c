/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_main.h"

/*
	Brush-based entities that span across an area.
*/

#include "server_waypoint.h"

/*	[14/10/2013]
		Eukos you could at least try to make this look uncopied!
		Cleaned this up.
	~hogsy
*/
void Area_SetMoveDirection(vec3_t vAngles,vec3_t vMoveDirection)
{
	vec3_t	vUp			= { 0,	-1, 0	},
			vMoveUp		= {	0,	0,	1	},
			vDown		= { 0,	-2, 0	},
			vMoveDown	= { 0,	0,	-1	};

	Engine.Con_Printf("Area_SetMoveDirection\n");

	if(Math_VectorCompare(vAngles,vUp))
		Math_VectorCopy(vMoveUp,vMoveDirection);
	else if(Math_VectorCompare(vAngles,vDown))
		Math_VectorCopy(vMoveDown,vMoveDirection);
	else
		Math_AngleVectors(vAngles,vMoveDirection,NULL,NULL);

	Math_VectorClear(vAngles);
}

void Area_CalculateMovementDone(edict_t *eArea)
{
	Engine.Con_Printf("Area_CalculateMovementDone\n");

	Entity_SetOrigin(eArea,eArea->local.finaldest);

	// [17/7/2012] Simplified ~hogsy
	Math_VectorClear(eArea->v.velocity);

	if(eArea->local.think1)
		eArea->local.think1(eArea,eArea);
}

void Area_CalculateMovement(edict_t *eArea,vec3_t vTDest,float fSpeed,void (*Function)(edict_t *eArea,edict_t *eOther))
{
	vec3_t	vdestdelta;
	float	fTravelTime;

	Engine.Con_Printf("Area_CalculateMovement\n");

	Math_VectorCopy(vTDest,eArea->local.finaldest);

	Engine.Con_Printf("TDEST: %i(X) %i(Y) %i(Z)\nORIGIN: %i(X) %i(Y) %i(Z)\n",
					  vTDest[0],vTDest[1],vTDest[2],
					  eArea->v.origin[0],eArea->v.origin[1],eArea->v.origin[2]);

	// [14/10/2013] Simplified ~hogsy
	Math_VectorSubtract(vTDest,eArea->v.origin,vdestdelta);

	Engine.Con_Printf("VDEST: %i(X) %i(Y) %i(Z)\n",
		(int)vdestdelta[0],
		(int)vdestdelta[1],
		(int)vdestdelta[2]);

	fTravelTime = (float)Math_VectorLength(vdestdelta)/fSpeed;

	Engine.Con_Printf("traveltime: %f\n",fTravelTime);

	// [13/12/2013] Simplified ~hogsy
	Math_VectorScale(vdestdelta,1.0f/fTravelTime,eArea->v.velocity);

	eArea->local.think1	= Function;

	eArea->v.think		= Area_CalculateMovementDone;
	eArea->v.dNextThink	= eArea->v.ltime+fTravelTime;
}

/*
	Spawn
*/

/*	Area in which players can be randomly spawned.
	TODO: Finish this!
*/
void Area_PlayerSpawn(edict_t *eArea)
{
	// [10/9/2013] Create a new waypoint here so monsters know to keep back... ~hogsy
	Waypoint_Spawn(eArea->v.origin,WAYPOINT_SPAWNAREA);
}

/*
	Breakable
*/

// [10/9/2013] Made these static since entities specifically rely on them ~hogsy
#define BREAKABLE_GLASS	0
#define	BREAKABLE_WOOD	1
#define	BREAKABLE_ROCK	2
#define	BREAKABLE_METAL	3

void Area_BreakableBounce(edict_t *eGib,edict_t *eOther)
{
	char cSound[24];

	if(eGib->v.flags & FL_ONGROUND)
		return;

	switch(eGib->local.style)
	{
	case BREAKABLE_GLASS:
		PHYSICS_SOUND_GLASS(cSound);
		break;
	case BREAKABLE_WOOD:
		PHYSICS_SOUND_WOOD(cSound);
		break;
	case BREAKABLE_ROCK:
		PHYSICS_SOUND_ROCK(cSound);
		break;
	case BREAKABLE_METAL:
		PHYSICS_SOUND_METAL(cSound);
		break;
	default:
		Engine.Con_Warning("Unknown breakable type! (%i)\n",eGib->local.style);
		return;
	}

	Sound(eGib,CHAN_AUTO,cSound,30,ATTN_NORM);
}

/**/

void Area_CreateGib(edict_t *eArea,const char *cModel)
{
	int		j;
	edict_t	*eGib = Spawn();

	if(eGib)
	{
		eGib->v.cClassname		= "area_gib";
		eGib->v.movetype		= MOVETYPE_BOUNCE;
		eGib->v.TouchFunction	= Area_BreakableBounce;
		eGib->v.think			= Entity_Remove;
		eGib->v.dNextThink		= Server.dTime+20.0f;

		eGib->Physics.iSolid = SOLID_TRIGGER;

		// [7/6/2013] Needed to check what we are when bouncing ~hogsy
		eGib->local.style = eArea->local.style;

		for(j = 0; j < 3; j++)
		{
			// [7/6/2013] Corrected these (were set to i rather than j) ~hogsy
			eGib->v.velocity[j]		=
			eGib->v.avelocity[j]	= (float)(rand()%5*eArea->v.iHealth*5);
		}

		Entity_SetModel(eGib,(char*)cModel);
		Entity_SetOrigin(eGib,eArea->v.oldorigin);
		Entity_SetSizeVector(eGib,vec3_origin,vec3_origin);
	}
}

void Area_BreakableDie(edict_t *eArea,edict_t *eOther)
{
	int		i;
	char	cSound[24],cModel[64];

	switch(eArea->local.style)
	{
	case BREAKABLE_GLASS:
		PHYSICS_SOUND_GLASS(cSound);
		sprintf(cModel,"models/gibs/glass_gibs%i.md2",rand()%3+1);
		break;
	case BREAKABLE_WOOD:
		PHYSICS_SOUND_WOOD(cSound);
		sprintf(cModel,"models/gibs/wood_gibs%i.md2",rand()%3+1);
		break;
	case BREAKABLE_ROCK:
		PHYSICS_SOUND_ROCK(cSound);
		sprintf(cModel,"models/gibs/rock_gibs%i.md2",rand()%3+1);
		break;
	case BREAKABLE_METAL:
		PHYSICS_SOUND_METAL(cSound);
		sprintf(cModel,"models/gibs/metal_gibs%i.md2",rand()%3+1);
		break;
	}

	Sound(eArea,CHAN_AUTO,cSound,255,ATTN_STATIC);

	for(i = 0; i < eArea->local.count; i++)
		Area_CreateGib(eArea,cModel);

	if(eArea->v.targetname) // Trigger doors, etc. ~eukos
		UseTargets(eArea,eOther);

	Entity_Remove(eArea);
}

void Area_BreakableUse(edict_t *eArea)
{
	Area_BreakableDie(eArea,eArea->local.activator);
}

void Area_BreakableSpawn(edict_t *eArea)
{
	if(eArea->v.iHealth <= 0)
		eArea->v.iHealth = 1;

	switch(eArea->local.style)
	{
		case BREAKABLE_GLASS:
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_GLASS0);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_GLASS1);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_GLASS2);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GLASS0);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GLASS1);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GLASS2);
			break;
		case BREAKABLE_WOOD:
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_WOOD0);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_WOOD1);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_WOOD2);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_WOOD0);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_WOOD1);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_WOOD2);
			break;
		case BREAKABLE_ROCK:
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_ROCK0);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_ROCK1);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_ROCK2);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/rock_gibs1.md2");
			Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/rock_gibs2.md2");
			Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/rock_gibs3.md2");
			break;
		case BREAKABLE_METAL:
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_METAL0);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_METAL1);
			Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_METAL2);
			Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/metal_gibs1.md2");
			Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/metal_gibs2.md2");
			Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/metal_gibs3.md2");
			break;
		default:
			Engine.Con_Warning("area_breakable: Unknown style\n");
	}

	// UNDONE: This compiled and all and it works fine but do it a cleaner way. ~eukos
	// [31/12/2013] How much cleaner could you even do this? Seems fine to me. ~hogsy
	if(eArea->v.cName)
		eArea->v.use = Area_BreakableUse;

	eArea->Physics.iSolid = SOLID_BSP;

	eArea->v.movetype		= MOVETYPE_PUSH;
	eArea->v.bTakeDamage	= true;

	eArea->local.bBleed = false;

	eArea->monster.think_die = Area_BreakableDie;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
	Entity_SetSizeVector(eArea,eArea->v.mins,eArea->v.maxs);

	eArea->v.oldorigin[0]	= (eArea->v.mins[0]+eArea->v.maxs[0])*0.5f;
	eArea->v.oldorigin[1]	= (eArea->v.mins[1]+eArea->v.maxs[1])*0.5f;
	eArea->v.oldorigin[2]	= (eArea->v.mins[2]+eArea->v.maxs[2])*0.5f;
}

void Area_RotateBlocked(edict_t *eArea,edict_t *eOther)
{
	if(eOther->v.iHealth <= 0)
		return;

	MONSTER_Damage(eOther,eArea,eArea->local.iDamage, 0);
}

void Area_RotateTouch(edict_t *eArea,edict_t *eOther)
{
	if(eArea->local.dMoveFinished	> Server.dTime)
		return;
}

#define STYLE_ROTATE_DOOR	1

#define	SPAWNFLAG_ROTATE_X			2
#define	SPAWNFLAG_ROTATE_Y			4
#define	SPAWNFLAG_ROTATE_Z			8
#define	SPAWNFLAG_ROTATE_REVERSE	64

void Area_RotateSpawn(edict_t *eArea)
{
	if(!eArea->local.iDamage)
		eArea->local.iDamage = 2;

	if(!eArea->local.speed)
		eArea->local.speed = 100.0f;

	if(!eArea->local.distance)
		eArea->local.distance = 5;

	// [26/7/2012] Check our spawn flags ~hogsy
	if(eArea->local.style == STYLE_ROTATE_DOOR)
	{
		if(eArea->v.spawnflags & SPAWNFLAG_ROTATE_REVERSE)
		{
		}

		if(eArea->v.spawnflags & SPAWNFLAG_ROTATE_X)
			eArea->v.movedir[0] = 1.0f;
		else if(eArea->v.spawnflags & SPAWNFLAG_ROTATE_Y)
			eArea->v.movedir[1] = 1.0f;
		else
			eArea->v.movedir[2] = 1.0f;

		Math_VectorCopy(eArea->v.angles,eArea->local.pos1);
		eArea->local.pos2[0] = eArea->local.pos1[0]+eArea->v.movedir[0]*eArea->local.distance;

		eArea->v.TouchFunction = Area_RotateTouch;

		eArea->local.dMoveFinished = 0;
	}
	else
	{
		if(eArea->v.spawnflags & SPAWNFLAG_ROTATE_REVERSE)
			eArea->local.speed *= -1;

		if(eArea->v.spawnflags & SPAWNFLAG_ROTATE_X)
			eArea->v.avelocity[0] = eArea->local.speed;

		if(eArea->v.spawnflags & SPAWNFLAG_ROTATE_Y)
			eArea->v.avelocity[1] = eArea->local.speed;

		if(eArea->v.spawnflags & SPAWNFLAG_ROTATE_Z)
			eArea->v.avelocity[2] = eArea->local.speed;
	}

	eArea->v.movetype	= MOVETYPE_PUSH;
	eArea->v.blocked	= Area_RotateBlocked;

	eArea->Physics.iSolid = SOLID_BSP;

#if 0
	if(eArea->v.targetname)
	{
		eOriginEntity = Engine.Server_FindEntity(Server.eWorld,eArea->v.targetname,false);
		if(eOriginEntity)
		{
			Entity_SetOrigin(eArea,eOriginEntity->v.origin);
			return;
		}
	}
#endif

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetSizeVector(eArea,eArea->v.mins,eArea->v.maxs);
	Entity_SetOrigin(eArea,eArea->v.origin);
}

/*
	AREA_DOOR
*/

// Various states for doors.
#define STATE_TOP			0	// Brush is at the top.
#define STATE_BOTTOM		1	// Brush is at the bottom.
#define STATE_UP			2	// Brush is moving up.
#define STATE_DOWN			3	// Brush is moving down.

void Area_DoorDone(edict_t *eArea, edict_t *eOther)
{
	eArea->local.iValue = 0;

	if(eArea->local.cSoundStop)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundStop,255,ATTN_NORM);
}

void Area_DoorReturn(edict_t *eArea)
{
	eArea->local.state = STATE_DOWN;

	Area_CalculateMovement(eArea,eArea->local.pos1,eArea->local.speed,Area_DoorDone);

	if(eArea->local.cSoundReturn)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundReturn,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_DoorWait(edict_t *eArea,edict_t *eOther)
{
	UseTargets(eArea, eOther);

	eArea->local.state	= STATE_TOP;
	eArea->local.iValue = 1;

	if(eArea->local.dWait >= 0)
		eArea->v.dNextThink	= Server.dTime+eArea->local.dWait;

	eArea->v.think = Area_DoorReturn;

	if(eArea->local.cSoundStop)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundStop,255,ATTN_NORM);
}

void Area_DoorTouch(edict_t *eArea, edict_t *eOther)
{
	if (eArea->local.state == STATE_UP || eArea->local.state == STATE_TOP)
		return;
	if((eOther->monster.iType != MONSTER_PLAYER) && eOther->v.iHealth <= 0)
		return;

	eArea->local.state = STATE_UP;
	Area_CalculateMovement(eArea,eArea->local.pos2,eArea->local.speed,Area_DoorWait);

	if(eArea->local.cSoundStart)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundStart,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_DoorUse(edict_t *eArea)
{
	if(eArea->local.state == STATE_UP || eArea->local.state == STATE_TOP)
		return;

	eArea->local.state = STATE_UP;

	Area_CalculateMovement(eArea,eArea->local.pos2,eArea->local.speed,Area_DoorWait);

	if(eArea->local.cSoundStart)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundStart,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_DoorBlocked(edict_t *eArea, edict_t *eOther)
{
	MONSTER_Damage(eOther,eArea,eArea->local.iDamage,0);
}

void Area_DoorSpawn(edict_t *eArea)
{
	int		i;
	float	fDist;
	vec3_t	vMoveDir;

	if(!eArea->v.spawnflags)
		eArea->v.spawnflags = 0;

	if(eArea->local.cSoundStart)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundStart);
	if(eArea->local.cSoundStop)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundStop);
	if(eArea->local.cSoundMoving)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundMoving);
	if(eArea->local.cSoundReturn)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundReturn);

	eArea->v.movetype = MOVETYPE_PUSH;

	eArea->Physics.iSolid = SOLID_BSP;

	eArea->local.iValue = 0;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
	Entity_SetSizeVector(eArea,eArea->v.mins,eArea->v.maxs);

	if(eArea->local.lip == 0.0f)
		eArea->local.lip = 4.0f;

	Math_VectorCopy(eArea->v.origin,eArea->local.pos1);

	Area_SetMoveDirection(eArea->v.angles, eArea->v.movedir);

	for(i = 0; i < 3; i++)
		vMoveDir[i] = (float)fabs(eArea->v.movedir[i]);

	fDist = vMoveDir[0]*eArea->v.size[0]+vMoveDir[1]*eArea->v.size[1]+vMoveDir[2]*eArea->v.size[2]-eArea->local.lip;

	Math_VectorMake(eArea->local.pos1,fDist,eArea->v.movedir,eArea->local.pos2);

	eArea->local.state = STATE_BOTTOM;

	if(eArea->v.spawnflags != 32) // Toggle
		eArea->v.TouchFunction = Area_DoorTouch;

	if(eArea->local.iDamage)
		eArea->v.blocked = Area_DoorBlocked;

	eArea->v.use = Area_DoorUse;
}

/*
	Change Level
*/

void Area_ChangeLevelTouch(edict_t *eArea,edict_t *eOther)
{
	// [2/1/2013] TODO: If coop wait for eOther players? ~hogsy

	if((eOther->monster.iType != MONSTER_PLAYER))
		return;

#if 0
	// [2/1/2013] Because we don't want to trigger it multiple times within the delay!!! ~hogsy
	eArea->v.solid		= SOLID_NOT;
	eArea->v.think		= Area_ChangelevelStart;
	eArea->v.dNextThink	= Server.dTime+eArea->local.delay;
#else
	eArea->Physics.iSolid		= SOLID_NOT;

	// [2/1/2013] Change the level! ~hogsy
	Engine.Server_ChangeLevel(eArea->v.targetname);
#endif
}

void Area_ChangeLevel(edict_t *eArea)
{
	if(!eArea->v.targetname)
	{
		Engine.Con_Warning("No targetname set for area_changelevel! (%i %i %i)\n",
			(int)eArea->v.origin[0],
			(int)eArea->v.origin[1],
			(int)eArea->v.origin[2]);
		return;
	}

	eArea->v.TouchFunction	= Area_ChangeLevelTouch;

	eArea->Physics.iSolid	= SOLID_TRIGGER;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);

	eArea->v.model = 0;
}

/*
	Trigger
*/

void Area_TriggerTouch(edict_t *eArea,edict_t *eOther)
{
	if(eArea->v.dNextThink || ((eArea->monster.iType != MONSTER_PLAYER) && eOther->v.iHealth <= 0))
		return;

	eArea->v.dNextThink = Server.dTime+eArea->local.dWait;

	UseTargets(eArea,eOther);

	if(eArea->local.dWait < 0)
		Entity_Remove(eArea);
}

void Area_TriggerSpawn(edict_t *eArea)
{
	if(!eArea->v.targetname)
	{
		Engine.Con_Warning("'targetname' not set for trigger! (%i %i %i)\n",
			(int)eArea->v.origin[0],
			(int)eArea->v.origin[1],
			(int)eArea->v.origin[2]);
		return;
	}

	eArea->v.TouchFunction = Area_TriggerTouch;

	eArea->Physics.iSolid = SOLID_TRIGGER;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);

	eArea->v.model = 0;
}

/*
	Pushable
*/

void Area_PushableTouch(edict_t *eArea,edict_t *eOther)
{
	int i;

	if(	eOther->v.velocity[0] > eArea->Physics.fMass ||
		eOther->v.velocity[1] > eArea->Physics.fMass)
		for(i = 0; i < 2; i++)
			eArea->v.origin[i] += (eOther->v.velocity[i]/5.0f);
	else if(	eOther->v.velocity[0] < eArea->Physics.fMass ||
				eOther->v.velocity[1] < eArea->Physics.fMass)
		for(i = 0; i < 2; i++)
			eArea->v.origin[i] -= (eOther->v.velocity[i]/5.0f);
}

void Area_PushableUse(edict_t *eArea)
{
	eArea->Physics.iSolid	= SOLID_NOT;
	eArea->v.movetype		= MOVETYPE_FLYBOUNCE;

	// [8/11/2013] Placeholder values! ~hogsy
//	Math_VectorSet(30.0f,eArea->v.avelocity);
	//Math_VectorSet(10.0f,eArea->v.velocity);
}

void Area_PushableSpawn(edict_t *eArea)
{
	eArea->v.TouchFunction	= Area_PushableTouch;
	eArea->v.use			= Area_PushableUse;
	eArea->v.movetype		= MOVETYPE_PUSH;

	eArea->Physics.iSolid	= SOLID_BSP;
	eArea->Physics.fGravity	= SERVER_GRAVITY;
	eArea->Physics.fMass	= 2.0f;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
	Entity_SetSizeVector(eArea,eArea->v.mins,eArea->v.maxs);
}

/*
	Wall
*/

void Area_WallUse(edict_t *eArea)
{
	if(eArea->Physics.iSolid == SOLID_BSP)
	{
		eArea->local.cOldModel	= eArea->v.model;
		eArea->local.iValue		= 0;

		eArea->Physics.iSolid = SOLID_NOT;

		eArea->v.model = 0;
		return;
	}

	eArea->Physics.iSolid = SOLID_BSP;

	Entity_SetModel(eArea,eArea->local.cOldModel);
}

void Area_WallSpawn(edict_t *eArea)
{
	if(!eArea->v.model)
	{
		Engine.Con_Warning("Area entity with no model!\n");
		ENTITY_REMOVE(eArea);
	}

	if(eArea->v.cName)
		eArea->v.use = Area_WallUse;

	eArea->v.movetype = MOVETYPE_PUSH;

	eArea->local.iValue	= 1;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
}

/*
	Button
*/

void Area_ButtonDone(edict_t *eArea, edict_t *eOther)
{
	eArea->local.state	= STATE_DOWN;
	eArea->local.iValue = 0;

	if(eArea->local.cSoundStop)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundStop,255,ATTN_NORM);
}

void Area_ButtonReturn(edict_t *eArea)
{
	Area_CalculateMovement(eArea,eArea->local.pos1,eArea->local.speed,Area_ButtonDone);

	if(eArea->local.cSoundReturn)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundReturn,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_ButtonWait(edict_t *eArea, edict_t *eOther)
{
	UseTargets(eArea, eOther);

	eArea->local.state = STATE_TOP;
	eArea->local.iValue = 1;

	eArea->v.think		= Area_ButtonReturn;

//	if(eArea->local.dWait >= 0)
	eArea->v.dNextThink	= eArea->v.ltime + 4;

	if(eArea->local.cSoundStop)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundStop,255,ATTN_NORM);
}

void Area_ButtonTouch(edict_t *eArea,edict_t *eOther)
{
	if(eArea->local.state == STATE_UP || eArea->local.state == STATE_TOP)
		return;
	if((eOther->monster.iType != MONSTER_PLAYER) && eOther->v.iHealth <= 0)
		return;

	eArea->local.state = STATE_UP;

	Area_CalculateMovement(eArea,eArea->local.pos2,eArea->local.speed,Area_ButtonWait);

	if(eArea->local.cSoundStart)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundStart,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_ButtonUse(edict_t *eArea)
{
	if(eArea->local.state == STATE_UP || eArea->local.state == STATE_TOP)
		return;

	eArea->local.state = STATE_UP;

	Area_CalculateMovement(eArea,eArea->local.pos2,eArea->local.speed,Area_ButtonWait);

	if(eArea->local.cSoundStart)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundStart,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_ButtonBlocked(edict_t *eArea, edict_t *eOther)
{
	MONSTER_Damage(eOther,eArea,eArea->local.iDamage,0);
}

void Area_ButtonSpawn(edict_t *eArea)
{
	int		i;
	float	fDist;
	vec3_t	vMoveDir;

	if(!eArea->v.spawnflags)
		eArea->v.spawnflags = 0;

	if(eArea->local.cSoundStart)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundStart);
	if(eArea->local.cSoundStop)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundStop);
	if(eArea->local.cSoundMoving)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundMoving);
	if(eArea->local.cSoundReturn)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundReturn);

	eArea->v.movetype = MOVETYPE_PUSH;

	eArea->Physics.iSolid = SOLID_BSP;

	// [18/5/2013] Changed to use ! check instead since it's safer here ~hogsy
	if(eArea->local.lip == 0.0f)
		eArea->local.lip = 4.0f;

	eArea->local.iValue = 0;
	eArea->local.state	= STATE_BOTTOM;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
	Entity_SetSizeVector(eArea,eArea->v.mins,eArea->v.maxs);

	Math_VectorCopy(eArea->v.origin,eArea->local.pos1);

	Area_SetMoveDirection(eArea->v.angles,eArea->v.movedir);

	for(i = 0; i < 3; i++)
		vMoveDir[i] = (float)fabs(eArea->v.movedir[i]);

	fDist = vMoveDir[0]*eArea->v.size[0]+
			vMoveDir[1]*eArea->v.size[1]+
			vMoveDir[2]*eArea->v.size[2]-
			eArea->local.lip;

	Math_VectorMake(eArea->local.pos1,fDist,eArea->v.movedir,eArea->local.pos2);

	if(eArea->v.spawnflags != 32) // Toggle
		eArea->v.TouchFunction = Area_ButtonTouch;

	if(eArea->local.iDamage)
		eArea->v.blocked = Area_ButtonBlocked;

	eArea->v.use = Area_ButtonUse;
}

/*
	Platform
*/

void Area_PlatformTouch(edict_t *eArea, edict_t *eOther);

void Area_PlatformSpawnTouchboxTouch(edict_t *eArea, edict_t *eOther)
{
	Area_PlatformTouch(eArea->v.enemy, eOther);
}

void Area_PlatformSpawnTouchbox(edict_t *eArea)
{
	edict_t *eTrigger;
	vec3_t vTrigMin, vTrigMax;

	eTrigger = Spawn();
	eTrigger->v.TouchFunction = Area_PlatformSpawnTouchboxTouch;
	eTrigger->v.movetype = MOVETYPE_NONE;
	eTrigger->v.enemy = eArea;

	eTrigger->Physics.iSolid = SOLID_TRIGGER;

	vTrigMin[0] = eArea->v.mins[0] - 25;
	vTrigMin[1] = eArea->v.mins[1] - 25;
	vTrigMin[2] = eArea->v.mins[2] + -8;

	vTrigMax[0] = eArea->v.maxs[0] + 25;
	vTrigMax[1] = eArea->v.maxs[1] + 25;
	vTrigMax[2] = eArea->v.maxs[2] - 8;

	vTrigMin[2] = vTrigMax[2] - (eArea->local.pos1[2] - eArea->local.pos2[2] -8);

	if (eArea->v.size[0] <= 50)
	{
		vTrigMin[0] = (vTrigMin[0] + vTrigMax[0]) / 2;
		vTrigMax[0] = vTrigMin[0] + 1;
	}
	if (eArea->v.size[1] <= 50)
	{
		vTrigMin[1] = (vTrigMin[1] + vTrigMax[1]) / 2;
		vTrigMax[1] = vTrigMin[1] + 1;
	}

	// [24/9/2013] Simplified ~hogsy
	Entity_SetSizeVector(eTrigger,vTrigMin,vTrigMax);
}

void Area_PlatformDone(edict_t *eArea, edict_t *eOther)
{
	eArea->local.iValue = 0;

	if(eArea->local.cSoundStop)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundStop,255,ATTN_NORM);
}

void Area_PlatformReturn(edict_t *eArea)
{
	eArea->local.state = STATE_DOWN;

	Area_CalculateMovement(eArea,eArea->local.pos1,eArea->local.speed,Area_PlatformDone);

	if(eArea->local.cSoundReturn)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundReturn,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_PlatformWait(edict_t *eArea, edict_t *eOther)
{
	UseTargets(eArea, eOther);

	eArea->local.state	= STATE_TOP;
	eArea->local.iValue = 1;

	eArea->v.think = Area_PlatformReturn;

	if(eArea->local.dWait >= 0)
		eArea->v.dNextThink	= Server.dTime+eArea->local.dWait;

	if(eArea->local.cSoundStop)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundStop,255,ATTN_NORM);
}

void Area_PlatformTouch(edict_t *eArea, edict_t *eOther)
{
	if(eArea->local.state == STATE_UP || eArea->local.state == STATE_TOP)
		return;
	if((eOther->monster.iType != MONSTER_PLAYER) && eOther->v.iHealth <= 0)
		return;

	eArea->local.state = STATE_UP;
	Area_CalculateMovement(eArea,eArea->local.pos2,eArea->local.speed,Area_PlatformWait);

	if(eArea->local.cSoundStart)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundStart,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_PlatformUse(edict_t *eArea)
{
	if(eArea->local.state == STATE_UP || eArea->local.state == STATE_TOP)
		return;

	eArea->local.state = STATE_UP;

	Area_CalculateMovement(eArea,eArea->local.pos2,eArea->local.speed,Area_PlatformWait);

	if(eArea->local.cSoundStart)
		Sound(eArea,CHAN_BODY,eArea->local.cSoundStart,255,ATTN_NORM);
	if(eArea->local.cSoundMoving)
		Sound(eArea,CHAN_VOICE,eArea->local.cSoundMoving,255,ATTN_NORM);
}

void Area_PlatformBlocked(edict_t *eArea, edict_t *eOther)
{
	MONSTER_Damage(eOther,eArea,eArea->local.iDamage,0);
}

void Area_PlatformSpawn(edict_t *eArea)
{
	float dist;

	if(!eArea->v.spawnflags)
		eArea->v.spawnflags = 0;

	if(eArea->local.cSoundStart)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundStart);
	if(eArea->local.cSoundStop)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundStop);
	if(eArea->local.cSoundMoving)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundMoving);
	if(eArea->local.cSoundReturn)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eArea->local.cSoundReturn);

	eArea->v.movetype = MOVETYPE_PUSH;

	eArea->Physics.iSolid = SOLID_BSP;

	eArea->local.iValue = 0;
	eArea->local.state	= STATE_BOTTOM;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
	Entity_SetSizeVector(eArea,eArea->v.mins,eArea->v.maxs);

	if(!eArea->local.style)
		eArea->local.style = 0;
	if(eArea->local.lip == 0.0f)
		eArea->local.lip = 4.0f;

	Math_VectorCopy(eArea->v.origin,eArea->local.pos1);

	Area_SetMoveDirection(eArea->v.angles, eArea->v.movedir);

	dist = (float)eArea->local.count;

	Math_VectorMake(eArea->local.pos1, dist, eArea->v.movedir, eArea->local.pos2);

	if(eArea->local.style == 1)
	{
		Entity_SetOrigin(eArea, eArea->local.pos2);

		Math_VectorCopy(eArea->local.pos2, eArea->local.pos1);
		Math_VectorCopy(eArea->v.origin, eArea->local.pos2);
	}

	if(eArea->v.spawnflags != 32) // Toggle
		eArea->v.TouchFunction = Area_PlatformTouch;

	if(eArea->local.iDamage)
		eArea->v.blocked = Area_PlatformBlocked;

	Area_PlatformSpawnTouchbox(eArea);

	eArea->v.use = Area_PlatformUse;
}

/*
	Climb / Ladders

	TODO:
		Only push the player up when he is actually trying to move forward. ~eukos
*/

void Area_ClimbTouch(edict_t *eArea,edict_t *eOther)
{
	if(eOther->v.movetype != MOVETYPE_WALK || !eOther->v.iHealth) // LADDER MESSY ~EUKOS
		return;

#if 1
	eOther->v.movetype = MOVETYPE_FLY;
#else	// [24/9/2013] Mockup of replacement ~hogsy
	eOther->v.movetype = MOVETYPE_CLIMB;
#endif
}

void Area_ClimbSpawn(edict_t *eArea)
{
	eArea->v.TouchFunction = Area_ClimbTouch;

	eArea->Physics.iSolid	= SOLID_TRIGGER;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);

	eArea->v.model = 0;
}

/*
	Noclip
*/

void Area_NoclipSpawn(edict_t *eArea)
{
	eArea->v.movetype	= MOVETYPE_PUSH;
	eArea->Physics.iSolid	= SOLID_NOT;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
}

/*
	Pusher
*/

#define PUSH_ONCE 32

void Area_PushTouch(edict_t *eArea, edict_t *eOther)
{
	// [9/12/2013] TODO: Make this optional? Would be cool to throw monsters and other crap around... ~hogsy
	if(!Entity_IsPlayer(eOther))
		return;

	Math_VectorScale(eArea->v.movedir,eArea->local.speed*10,eOther->v.velocity);

	// [9/12/2013] Corrected a mistake that was made here. ~hogsy
	if(eArea->v.spawnflags & PUSH_ONCE)
		Entity_Remove(eArea);
}

void Area_PushSpawn(edict_t *eArea)
{
	if(!eArea->local.speed)
		eArea->local.speed = 500.0f;

	Area_SetMoveDirection(eArea->v.angles, eArea->v.movedir);

	eArea->v.TouchFunction	= Area_PushTouch;
	eArea->Physics.iSolid	= SOLID_TRIGGER;

	Entity_SetModel(eArea,eArea->v.model);
	Entity_SetOrigin(eArea,eArea->v.origin);
	Entity_SetSizeVector(eArea,eArea->v.mins,eArea->v.maxs);

	eArea->v.model = 0;
}
