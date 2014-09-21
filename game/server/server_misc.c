/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_misc.h"

void UseTargets(edict_t *ent, edict_t *other);

#if 0
edict_t *SpawnField(edict_t *ent,vec3_t fmins,vec3_t fmaxs)
{
	edict_t	*trigger = Entity_Spawn();

	trigger->v.movetype		= MOVETYPE_NONE;
	trigger->Physics.iSolid	= SOLID_TRIGGER;
	trigger->v.touch		= Touch_DoorTrigger;

	trigger->local.eOwner = ent;

	Entity_SetSize(trigger,fmins[0]-60,fmins[1]-60,fmins[2]-8,fmaxs[0]+60,fmaxs[1]+60,fmaxs[2]+8);

	return(trigger);
}
#endif

void Misc_DelayThink(edict_t *ent)
{
	edict_t *other = ent->v.enemy;

	UseTargets(ent, other);

	Entity_Remove(ent);
}

void UseTargets(edict_t *ent, edict_t *other)
{
	edict_t *t;

	ent->local.activator = other;

	if(ent->local.delay)
	{
		t = Entity_Spawn();
		if(t)
		{
			t->v.cClassname	= "DelayedUse";
			t->v.dNextThink	= Server.dTime+ent->local.delay;
			t->v.think		= Misc_DelayThink;
			t->v.enemy		= other;
		}

		return;
	}

	if (ent->local.killtarget)
	{
		t = Engine.Server_FindEntity(Server.eWorld,ent->local.killtarget,FALSE);
		if(!t)
			return;

		Entity_Remove(t);
	}

	if(ent->v.targetname)
	{
		t = Engine.Server_FindEntity(Server.eWorld,ent->v.targetname,FALSE);
		if(t)
		{
			t->local.activator = other;
			if(t->v.use)
				t->v.use(t);
			return;
		}
	}
}

void WEAPON_StickThink(edict_t *ent)
{
	ent->v.think			= WEAPON_StickThink;
	ent->v.TouchFunction	= NULL;

	if(((ent->v.enemy != Server.eWorld) && (ent->v.enemy->v.iHealth > 1)))
	{
		Entity_SetOrigin(ent,ent->v.enemy->v.origin);
		if(!ent->v.velocity)
			Math_VectorClear(ent->v.avelocity);
	}
	else
		Entity_Remove(ent);

	ent->v.dNextThink = (Server.dTime+0.100);
}

// [17/7/2012] Renamed to Misc_GibThink ~hogsy
void Misc_GibThink(edict_t *ent)
{
	Entity_Remove(ent);
}

void ThrowGib(vec3_t origin,vec3_t velocity,char *model,float damage,bool bleed)
{
	edict_t *gib = Entity_Spawn();

	gib->v.cClassname	= "gib";

	Entity_SetOrigin(gib,origin);

	if(bleed)
		gib->v.effects = EF_PARTICLE_BLOOD;

	gib->v.velocity[0]	= velocity[0]*((damage+rand()%20)/2.0f);
	gib->v.velocity[1]	= velocity[1]*((damage+rand()%20)/2.0f);
	gib->v.velocity[2]	= velocity[2]*((damage+rand()%20)/2.0f);
	gib->v.movetype		= MOVETYPE_BOUNCE;

	gib->Physics.iSolid	= SOLID_NOT;

	Math_VectorSet((float)(rand()%10*damage),gib->v.avelocity);

	Entity_SetModel(gib,model);
	Entity_SetSizeVector(gib,vec3_origin,vec3_origin);

	gib->v.think		= Misc_GibThink;
	gib->v.dNextThink	= Server.dTime+20.0f;
}

bool EntitiesTouching(edict_t *e1,edict_t *e2)
{
	if(	e1->v.mins[0] > e2->v.maxs[0] ||
		e1->v.mins[1] > e2->v.maxs[1] ||
		e1->v.mins[2] > e2->v.maxs[2] ||
		e1->v.maxs[0] < e2->v.mins[0] ||
		e1->v.maxs[1] < e2->v.mins[1] ||
		e1->v.maxs[2] < e2->v.mins[2])
		return false;

	return true;
}

bool DropToFloor(edict_t *ent)
{
	vec3_t	end;
	trace_t	trace;

	Math_VectorCopy(ent->v.origin,end);

	end[2] -= 256;

	trace = Engine.Server_Move(ent->v.origin,ent->v.mins,ent->v.maxs,end,false,ent);
	if(trace.fraction == 1 || trace.bAllSolid)
	{
		Engine.Con_Warning("%s is stuck (%i %i %i)!\n",ent->v.cClassname,
			// [17/11/2013] Give us integers instead ~hogsy
			(int)ent->v.origin[0],
			(int)ent->v.origin[1],
			(int)ent->v.origin[2]);
		return false;
	}

	Math_VectorCopy(trace.endpos,ent->v.origin);

	Engine.LinkEntity(ent,false);

	ent->v.flags		|= FL_ONGROUND;
	ent->v.groundentity = trace.ent;

	return true;
}
