/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_monster.h"

/*
	Base code for the AI. This contains various basic functions
	that handle animation, movement and other small features
	that are used for the various monsters that can be found
	in both OpenKatana and future projects.
*/

#include "server_player.h"
#include "server_item.h"
#include "server_weapon.h"

/*	TODO:
		- Move out player specific code
		- Parse a document to find waypoints in the map
		- Add in tick stuff, if an entity is trying to
		run but hasn't moved from its last position and
		we're stuck then we'll get angry or if we're idle
		and doing nothing we might decide to wander around
		the environment a little.
		- Each monster should have its own list of "known"
		monsters which it has based sets for, if it doesn't
		know a monster such as the "player" and the player
		attacks it then we'll become an enemy.
		If we don't "know" an entity then our interest in
		it will increase, this will stimulate our wish to
		approach that entity. Depending on the response
		we'll update our memory table to reflect it so
		we can react on that.
		Also known as a relationship table fagget!
		- Implement enemy FOV system. Based on origin of
		the head position and then triangulated to four
		different points in-front of the creature which
		should be based on their maxview distance.
		Trace onto the world surface at the same time?
		 /a(a2)
	    O - - -
		 \b(b2)

	This is our basic monster/ai platform which
	we'll be using for OpenKatana and other future
	projects which will be using the same code
	base...
	Worth noting that most of these will be shared
	between other entities, in other words if they're
	here then they must	function globally!
	Though that does not mean moving literally everything
	in here obviously.
	Entities must NOT make their own changes to any "tick"
	variables outside of here!
*/

#define MONSTER_STEPSIZE	18

// [28/6/2013] Global relationship table ~hogsy
MonsterRelationship_t MonsterRelationship[]=
{
#ifdef GAME_OPENKATANA
	// LaserGat
	{	MONSTER_LASERGAT,	MONSTER_LASERGAT,	RELATIONSHIP_LIKE		},
	{	MONSTER_LASERGAT,	MONSTER_PLAYER,		RELATIONSHIP_HATE		},
	{	MONSTER_LASERGAT,	MONSTER_INMATER,	RELATIONSHIP_LIKE		},
	{	MONSTER_LASERGAT,	MONSTER_PRISONER,	RELATIONSHIP_NEUTRAL	},

	// Inmater
	{	MONSTER_INMATER,	MONSTER_INMATER,	RELATIONSHIP_LIKE		},
	{	MONSTER_INMATER,	MONSTER_LASERGAT,	RELATIONSHIP_LIKE		},
	{	MONSTER_INMATER,	MONSTER_PLAYER,		RELATIONSHIP_HATE		},
	{	MONSTER_INMATER,	MONSTER_PRISONER,	RELATIONSHIP_NEUTRAL	},

	// Prisoner
	{	MONSTER_PRISONER,	MONSTER_PRISONER,	RELATIONSHIP_LIKE	},
	{	MONSTER_PRISONER,	MONSTER_PLAYER,		RELATIONSHIP_LIKE	},
	{	MONSTER_PRISONER,	MONSTER_LASERGAT,	RELATIONSHIP_HATE	},
	{	MONSTER_PRISONER,	MONSTER_INMATER,	RELATIONSHIP_HATE	},
#else
	// Hurler
	{	MONSTER_HURLER,		MONSTER_PLAYER,		RELATIONSHIP_HATE	},
	{	MONSTER_HURLER,		MONSTER_HURLER,		RELATIONSHIP_LIKE	},
#endif

	{	MONSTER_NONE	}
};

/**/

// [30/7/2012] Added Monster_CheckBottom ~hogsy
bool Monster_CheckBottom(edict_t *ent)
{
	vec3_t	mins,maxs,start,stop;
	trace_t	trace;
	int		x,y;
	float	mid,bottom;

	Math_VectorAdd(ent->v.origin,ent->v.mins,mins);
	Math_VectorAdd(ent->v.origin,ent->v.maxs,maxs);

	/*	If all of the points under the corners are solid world, don't bother
		with the tougher checks
		the corners must be within 16 of the midpoint
	*/
	start[2] = mins[2]-1;
	for(x = 0; x <= 1; x++)
		for(y = 0; y <= 1; y++)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if(Engine.Server_PointContents(start) != BSP_CONTENTS_SOLID)
			{
				start[2] = mins[2];

				// The midpoint must be within 16 of the bottom
				start[0]	= stop[0]	= (mins[0]+maxs[0])*0.5f;
				start[1]	= stop[1]	= (mins[1]+maxs[1])*0.5f;

				stop[2]	= start[2]-2*MONSTER_STEPSIZE;

				trace = Engine.Server_Move(start,vec3_origin,vec3_origin,stop,true,ent);
				if(trace.fraction == 1.0)
					return false;

				mid = bottom = trace.endpos[2];

				// The corners must be within 16 of the midpoint
				for(x = 0; x <= 1; x++)
					for(y = 0; y <= 1; y++)
					{
						start[0] = stop[0] = x ? maxs[0] : mins[0];
						start[1] = stop[1] = x ? maxs[1] : mins[1];

						trace = Engine.Server_Move(start,vec3_origin,vec3_origin,stop,true,ent);
						if(trace.fraction != 1.0 && trace.endpos[2] > bottom)
							bottom = trace.endpos[2];
						if(trace.fraction == 1.0 || mid-trace.endpos[2] > MONSTER_STEPSIZE)
							return false;
					}
			}
		}

	return true;
}

// [30/7/2012] Added Monster_MoveStep ~hogsy
// [30/7/2012] TODO: Revised ~hogsy
bool Monster_MoveStep(edict_t *ent,vec3_t move,bool bRelink)
{
	float	dz;
	vec3_t	oldorg,vNewOrigin,end;
	trace_t	trace;
	int		i;

	Math_VectorClear(vNewOrigin);

	// Flying monsters don't step up
	if(ent->v.flags & (FL_SWIM|FL_FLY))
	{
		// Try one move with vertical motion, then one without
		for(i = 0; i < 2; i++)
		{
			Math_VectorAdd(ent->v.origin,move,vNewOrigin);

			if(i == 0)
			{
				dz = ent->v.origin[2]-ent->monster.vTarget[2];
				if(dz > 40)
					vNewOrigin[2] -= 8;
				else if(dz < 30)
					vNewOrigin[2] += 8;
			}

			trace = Engine.Server_Move(ent->v.origin,ent->v.mins,ent->v.maxs,vNewOrigin,false,ent);
			if(trace.fraction == 1.0f)
			{
				if((ent->v.flags & FL_SWIM) && (Engine.Server_PointContents(trace.endpos) == BSP_CONTENTS_EMPTY))
					return false;

				Math_VectorCopy(trace.endpos,ent->v.origin);

				if(bRelink)
					Engine.LinkEntity(ent,true);

				return true;
			}
		}

		return false;
	}

	// Push down from a step height above the wished position
	vNewOrigin[2] += MONSTER_STEPSIZE;

	Math_VectorCopy(vNewOrigin,end);

	end[2] -= MONSTER_STEPSIZE*2;

	trace = Engine.Server_Move(vNewOrigin,ent->v.mins,ent->v.maxs,end,false,ent);
	if(trace.bAllSolid)
		return false;
	else if(trace.bStartSolid)
	{
		vNewOrigin[2] -= MONSTER_STEPSIZE;

		trace = Engine.Server_Move(vNewOrigin,ent->v.mins,ent->v.maxs,end,false,ent);
		if(trace.bAllSolid || trace.bStartSolid)
			return false;
	}

	if(trace.fraction == 1)
	{
		// If monster had the ground pulled out, go ahead and fall
		if(ent->v.flags & FL_PARTIALGROUND)
		{
			Math_VectorAdd(ent->v.origin,move,ent->v.origin);

			if(bRelink)
				Engine.LinkEntity(ent,true);

			ent->v.flags &= ~FL_ONGROUND;

			return true;
		}

		// Walked off an edge
		return false;
	}

	// Check point traces down for dangling corners
	Math_VectorCopy(trace.endpos,ent->v.origin);

	if(!Monster_CheckBottom(ent))
	{
		if(ent->v.flags & FL_PARTIALGROUND)
		{
			/*	Entity had floor mostly
				pulled out from underneath
				it and is trying to correct
			*/
			if(bRelink)
				Engine.LinkEntity(ent,true);

			return true;
		}

		Math_VectorCopy(oldorg,ent->v.origin);

		return false;
	}

	if(ent->v.flags & FL_PARTIALGROUND)
		ent->v.flags &= ~FL_PARTIALGROUND;

	ent->v.groundentity = trace.ent;

	if(bRelink)
		Engine.LinkEntity(ent,true);

	return true;
}

// [28/7/2012] Added Monster_StepDirection ~hogsy
bool Monster_StepDirection(edict_t *ent,float yaw,float dist)
{
	vec3_t	move,oldorg;
	float	delta;

	ent->v.ideal_yaw	= yaw;
	ChangeYaw(ent);

	yaw	*= (float)pMath_PI*2/360;
	move[0] = (vec_t)cos(yaw)*dist;
	move[1] = (vec_t)sin(yaw)*dist;
	move[2] = 0;

	Math_VectorCopy(ent->v.origin,oldorg);
	if(Monster_MoveStep(ent,move,false))
	{
		delta = ent->v.angles[YAW]-ent->v.ideal_yaw;
		if(delta > 45 && delta < 315)
			Math_VectorCopy(oldorg,ent->v.origin);

		Engine.LinkEntity(ent,true);
		return true;
	}

	Engine.LinkEntity(ent,true);
	return false;
}

// [30/7/2012] Added Monster_NewChaseDirection ~hogsy
void Monster_NewChaseDirection(edict_t *ent,vec3_t target,float dist)
{
	float	deltax,deltay,d[3],tdir,olddir,turnaround;

	olddir		= Math_AngleMod((ent->v.ideal_yaw/45.0f)*45.0f);
	turnaround	= Math_AngleMod(olddir-180);

	deltax	= target[0]-ent->v.origin[0];
	deltay	= target[1]-ent->v.origin[1];
	if(deltax > 10)
		d[1] = 0;
	else if(deltax < -10)
		d[1] = 180;
	else
		d[1] = -1;

	if(deltay < -10)
		d[2] = 270;
	else if(deltay > 10)
		d[2] = 90;
	else
		d[2] = -1;

	// Try direct route
	if(d[1] != -1 && d[2] != -1)
	{
		if(!d[1])
			tdir = d[2] == 90.0f ? 45.0f:315.0f;
		else
			tdir = d[2] == 90.0f ? 135.0f:215.0f;

		if(tdir != turnaround && Monster_StepDirection(ent,tdir,dist))
			return;
	}

	// Try other directions
	if(((rand()&3)&1) || abs((int)deltay) > abs((int)deltax))
	{
		tdir	= d[1];
		d[1]	= d[2];
		d[2]	= tdir;
	}

	if(	d[1] != -1 && d[1] != turnaround	&&
		Monster_StepDirection(ent,d[1],dist))
		return;

	if(	d[2] != -1 && d[2] != turnaround	&&
		Monster_StepDirection(ent,d[2],dist))
		return;

	// There is no direct path to the player, so pick another direction
	if(olddir != -1 && Monster_StepDirection(ent,olddir,dist))
		return;

	// Randomly determine direction of search
	if(rand()&1)
	{
		for(tdir = 0; tdir <= 315; tdir += 45)
			if(tdir != turnaround && Monster_StepDirection(ent,tdir,dist))
				return;
	}
	else
	{
		for(tdir = 315; tdir >= 0; tdir -= 45)
			if(tdir!=turnaround && Monster_StepDirection(ent,tdir,dist))
				return;
	}

	if(turnaround != -1 && Monster_StepDirection(ent,turnaround,dist))
		return;

	ent->v.ideal_yaw = olddir;		// can't move

	// If a bridge was pulled out from underneath a monster, it may not have
	// a valid standing position at all
	if(!Monster_CheckBottom(ent))
		ent->v.flags |= FL_PARTIALGROUND;
}

// [26/7/2012] Added Monster_SetState to deal with conflicting states ~hogsy
// [8/2/2013] Revised and simplified ~hogsy
bool Monster_SetThink(edict_t *eMonster,MonsterThink_t mtThink)
{
	if(eMonster->monster.iThink == mtThink)
		// [4/2/2013] Return false, then we might decide it's time for a different state ~hogsy
		return false;
	else if(eMonster->monster.iState >= STATE_NONE)
	{
		Engine.Con_Warning("Attempted to set a think without a state for %s!\n",eMonster->v.cClassname);
		return false;
	}

	eMonster->monster.iThink = mtThink;

	return true;
}

/*	Automatically sets the state for the monster.
*/
bool Monster_SetState(edict_t *eMonster,MonsterState_t msState)
{
	if(eMonster->monster.iState == msState)
		return true;

	switch(msState)
	{
	case STATE_AWAKE:
		if(eMonster->monster.iState == STATE_DEAD)
			return false;

		eMonster->monster.iState = STATE_AWAKE;
		break;
	case STATE_ASLEEP:
		if(eMonster->monster.iState == STATE_DEAD)
			return false;

		eMonster->monster.iState = STATE_ASLEEP;
		break;
	case STATE_DEAD:
		eMonster->monster.iState = STATE_DEAD;
		break;
	default:
		Engine.Con_Warning("Tried to set an unknown state for %s (%i)!\n",eMonster->v.cClassname,msState);
		return false;
	}

	return true;
}

/*	Called when a monster/entity gets killed.
*/
void Monster_Killed(edict_t *eTarget,edict_t *eAttacker)
{
	if(eTarget->monster.iState == STATE_DEAD)
		return;

	if(Entity_IsMonster(eTarget))
	{
		WriteByte(MSG_ALL,SVC_KILLEDMONSTER);

		eAttacker->v.iScore++;
	}
	else if(Entity_IsPlayer(eAttacker) && bIsMultiplayer)
	{
		char *cDeathMessage = "%s was killed by %s\n";

		if(eTarget == eAttacker)
		{
			cDeathMessage = "%s killed himself!\n";

			eAttacker->v.iScore--;
		}
		else if(Entity_IsPlayer(eTarget) && bIsCooperative)
		{
			cDeathMessage = "%s was tk'd by %s (what a dick, huh?)";

			eAttacker->v.iScore--;
		}
		// [2/9/2012] Did we kill someone while dead? ~hogsy
		else
		{
			eAttacker->v.iScore++;

			// [15/12/2013] Extra points! ~hogsy
			if(eAttacker->v.iHealth <= 0)
			{
				// [3/10/2012] TODO: Play sound ~hogsy
				Engine.CenterPrint(eAttacker,"FROM BEYOND THE GRAVE!\n");

				cDeathMessage = "%s was killed from beyond the grave by %s\n";

				eAttacker->v.iScore += 2;
			}

			// [15/12/2013] Extra points! ~hogsy
			if(!(eTarget->v.flags & FL_ONGROUND))
			{
				// [25/6/2012] TODO: Play sound ~hogsy
				Engine.CenterPrint(eAttacker,"WATCH THEM DROP!\n");

				cDeathMessage = "%s was shot out of the air by %s\n";

				eAttacker->v.iScore += 2;
			}
		}

		// Update number of frags for client.
		Engine.SetMessageEntity(eAttacker);
		Engine.WriteByte(MSG_ONE,SVC_UPDATESTAT);
		Engine.WriteByte(MSG_ONE,STAT_FRAGS);
		Engine.WriteByte(MSG_ONE,eAttacker->v.iScore);

		// TODO: move Kill messages into mode_deathmatch (?) Create Weapon specific kill messages and more variations! ~eukos
		Engine.Server_BroadcastPrint(cDeathMessage,eTarget->v.netname,eAttacker->v.netname);
	}
	else
		eTarget->v.bTakeDamage = false;

#ifdef GAME_OPENKATANA
    // [22/4/2014] Drop the currently equipped item for the player to pick up! ~hogsy
    {
        Weapon_t *wActive = Weapon_GetCurrentWeapon(eTarget);
        if(wActive && (wActive->iItem != WEAPON_LASERS))
        {
            edict_t *eDroppedItem = Entity_Spawn();

            Math_VectorCopy(eTarget->v.origin,eDroppedItem->v.origin);

            eDroppedItem->local.style = wActive->iItem;

            Item_Spawn(eDroppedItem);
        }
    }
#endif

	if(eTarget->monster.think_die)
		eTarget->monster.think_die(eTarget,eAttacker);
}

void MONSTER_Damage(edict_t *target,edict_t *inflictor,int iDamage, int iDamageType)
{
	/*	TODO
		If the new inflicted damage is greater
		than the last amount, we're a monster
		and	the attacker is client (or other)
		then we should change our target to this
		other entity as it's most likely more
		dangerous!
		Suggested that amount of damage should
		multiply on each call and then slowly
		decrease after sometime.
	*/
	if(!target->v.bTakeDamage || (target->v.flags & FL_GODMODE) || !Entity_CanDamage(inflictor, target, iDamageType))
		return;

	// [28/8/2012] Blood is now handled here :) ~hogsy
	if(target->local.bBleed)
	{
		char	cBlood[6];

		PARTICLE_BLOOD(cBlood);

		Engine.Particle(target->v.origin,target->v.velocity,10.0f,cBlood,20);
	}

	// [3/10/2012] Only do this for clients ~hogsy
	if(Entity_IsPlayer(inflictor))
	{
#ifdef GAME_OPENKATANA
		if(inflictor->local.power_finished > Server.dTime)
			iDamage *= 3;
#endif

		// [3/10/2012] Half the amount of damage we can inflict in hard ~hogsy
		// [15/11/2012] Removed vita check here... Vita should not be acting like armor! ~hogsy
		if(cvServerSkill.value >= 3)
			iDamage /= 2;
	}
	else if(Entity_IsMonster(inflictor))
	{
		// [3/10/2012] Double if we're a monster ~hogsy
		if(cvServerSkill.value >= 3)
			iDamage *= 2;
	}

	if(Entity_IsMonster(target))
		// [27/4/2014] Automatically wake us up if asleep ~hogsy
		if(target->monster.iState == STATE_ASLEEP)
			Monster_SetState(target,STATE_AWAKE);

	target->v.iHealth -= iDamage;
	if(target->v.iHealth <= 0)
		Monster_Killed(target,inflictor);
	else if(target->monster.think_pain)
		target->monster.think_pain(target,inflictor);
}

void MONSTER_WaterMove(edict_t *ent)
{
	if(ent->v.iHealth < 0 || ent->v.movetype == MOVETYPE_NOCLIP)
		return;

	if(ent->v.waterlevel != 3)
	{
		ent->local.dAirFinished = Server.dTime+12.0;
		ent->local.iDamage		= 2;
	}
	else if(ent->local.dAirFinished < Server.dTime && ent->local.dPainFinished < Server.dTime)
	{
		ent->local.iDamage += 2;
		if(ent->local.iDamage > 15)
			ent->local.iDamage = 10;

		MONSTER_Damage(ent,Server.eWorld,ent->local.iDamage,ent->local.iDamageType);

		ent->local.dPainFinished = Server.dTime+1.0;
	}

	if((ent->v.watertype == CONTENT_LAVA) && ent->local.damage_time < Server.dTime)
	{
		ent->local.damage_time = Server.dTime+0.2;
	}
	else if((ent->v.watertype == CONTENT_SLIME) && ent->local.damage_time < Server.dTime)
	{
		ent->local.damage_time = Server.dTime+1.0;
	}

	if(!(ent->v.flags & FL_WATERJUMP))
	{
		ent->v.velocity[0] = ent->v.velocity[1] = ent->v.velocity[2] =
			ent->v.velocity[2]-0.8f*ent->v.waterlevel*((float)Server.dTime)*(ent->v.velocity[0]+ent->v.velocity[1]+ent->v.velocity[2]);
	}
}

void Monster_MoveToGoal(edict_t *ent,vec3_t goal,float distance)
{
	int i;

	if(!(ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM)))
		return;

	// [9/12/2012] Updated so this works better with our point based shit ~hogsy
	for(i = 0; i < 3; i++)
	{
		if(goal[i] > ent->v.absmax[i]+distance)
			return;
		if(goal[i] < ent->v.absmin[i]+distance)
			return;
	}

	if((rand()&3) == 1 || !Monster_StepDirection(ent,ent->v.ideal_yaw,distance))
		Monster_NewChaseDirection(ent,goal,distance);	// Change to goal at some point
}

/*	Returns the range from an entity to a target.
*/
float MONSTER_GetRange(edict_t *ent,vec3_t target)
{
	// [12/4/2012] Revised ~hogsy
	vec3_t spot,spot1,spot2;

	spot1[0] = ent->v.origin[0]+ent->v.view_ofs[0];
	spot1[1] = ent->v.origin[1]+ent->v.view_ofs[1];
	spot1[2] = ent->v.origin[2]+ent->v.view_ofs[2];
	spot2[0] = target[0]; //+ target->v.view_ofs[0];
	spot2[1] = target[1]; //+ target->v.view_ofs[1];
	spot2[2] = target[2]; //+ target->v.view_ofs[2];

	Math_VectorSubtract(spot1,spot2,spot);

	// [4/2/2013] Shouldn't this be a float rather than a double? Oh well, whatever you say, Carmack ~hogsy
	return (float)Math_VectorLength(spot);
}

bool Monster_IsVisible(edict_t *ent,edict_t *target)
{
	trace_t	tTrace;
	vec3_t	vStart,vEnd;

	// [10/4/2013] TODO: Rework for new FOV system ~hogsy
	// [2/1/2013] Simplified ~hogsy
	Math_VectorAdd(ent->v.origin,ent->v.view_ofs,vStart);
	Math_VectorAdd(target->v.origin,target->v.view_ofs,vEnd);

	tTrace = Traceline(ent,vStart,vEnd,true);
	// [10/4/2013] Simplified ~hogsy
	if(!(tTrace.bOpen && tTrace.bWater) && tTrace.fraction == 1.0f)
		return true;

	return false;
}

/*	Checks our relationship against a table and returns how	we'll
	treat the current target.
*/
int	Monster_GetRelationship(edict_t *eMonster,edict_t *eTarget)
{
	int	i;

	if(!eMonster->monster.iType)
	{
		Engine.Con_Warning("Attempted to get a relationship, but no monster type set! (%s)\n",eMonster->v.cClassname);
		return RELATIONSHIP_NEUTRAL;
	}

	for(i = 0; i < sizeof(MonsterRelationship); i++)
	{
		if(!MonsterRelationship[i].iFirstType)
			break;

		// [15/12/2013] Fixed a little mistake here ~hogsy
		if( (eMonster->monster.iType == MonsterRelationship[i].iFirstType)	&&
			(eTarget->monster.iType == MonsterRelationship[i].iSecondType))
			return MonsterRelationship[i].iRelationship;
	}

	return RELATIONSHIP_NEUTRAL;
}

/*	Returns view target.
	TODO:
		- Base this on if we're currently facing a target or not.
*/
edict_t *Monster_GetTarget(edict_t *eMonster)
{
	edict_t	*eTargets;

	eTargets = Engine.Server_FindRadius(eMonster->v.origin,10000.0f);	//eMonster->monster.fViewDistance);

	do
	{
		// [6/6/2013] Only return if it's a new target and a monster type! ~hogsy
		if(	(eMonster != eTargets)																&&
			(eTargets != eMonster->monster.eTarget && eTargets != eMonster->monster.eOldTarget) &&
			(eTargets->monster.iType != MONSTER_NONE)											&&
			(eTargets->monster.iType != eMonster->monster.iType))
			// [11/6/2013] Quick crap thrown in to check if the target is visible or not... ~hogsy
			if(Monster_IsVisible(eMonster,eTargets))
				return eTargets;

		eTargets = eTargets->v.chain;
	} while(eTargets);

	return NULL;
}

void Monster_SetTargets(edict_t *eMonster)
{
	int	iRelationship;

	if(!eMonster->monster.eTarget)
	{
		eMonster->monster.eTarget = Monster_GetTarget(eMonster);
		if(eMonster->monster.eTarget)
		{
			if((eMonster->monster.eTarget == eMonster->monster.eFriend) || (eMonster->monster.eTarget == eMonster->monster.eEnemy))
				return;

			iRelationship = Monster_GetRelationship(eMonster,eMonster->monster.eTarget);
			if(iRelationship == RELATIONSHIP_LIKE)
			{
				eMonster->monster.eFriend = eMonster->monster.eTarget;
				eMonster->monster.eTarget = NULL;
			}
			else if(iRelationship == RELATIONSHIP_HATE)
			{
				eMonster->monster.eEnemy	= eMonster->monster.eTarget;
				eMonster->monster.eTarget	= NULL;
			}
		}
	}
}

/*	Sets the current targetted entity and checks if it's a valid enemy then
	updates eEnemy and eOldTarget.
	eTarget needs to be set first!
	Assumes that the target is visible...
	~hogsy
*/
bool Monster_SetEnemy(edict_t *eMonster)
{
	if(eMonster->monster.eTarget == eMonster)
		return false;

	if(eMonster->monster.eTarget != eMonster->monster.eOldTarget)
	{
		if(!eMonster->monster.eTarget->v.iHealth)
			return false;

		if(Monster_GetRelationship(eMonster,eMonster->monster.eTarget) == RELATIONSHIP_HATE)
		{
			eMonster->monster.eEnemy		=
			eMonster->monster.eOldTarget	= eMonster->monster.eTarget;
			eMonster->monster.eTarget		= NULL;
			return true;
		}
	}

	return false;
}

// [6/8/2012] Rewrite of the below functions ~hogsy
/*	Used to go over each monster state then update it, and then calls the monsters
	assigned think function.
*/
void Monster_Frame(edict_t *eMonster)
{
	int i;

	// [19/2/2013] If it's not a monster then return! ~hogsy
	if(eMonster->monster.iType < MONSTER_VEHICLE)
		return;

	Entity_CheckFrames(eMonster);

	// [26/9/2012] Handle jumping (this is copied over from sv_player.c) ~hogsy
	if(	(eMonster->local.jump_flag < -300.0f)	&&
		(eMonster->v.flags & FL_ONGROUND))
	{
		// [26/9/2012] TODO: Add in think for land! ~hogsy
		// [26/9/2012] Set our jump_flag to 0 ~hogsy
		eMonster->local.jump_flag = 0;
	}
	else if(!(eMonster->v.flags & FL_ONGROUND))
		eMonster->local.jump_flag = eMonster->v.velocity[2];

	switch(eMonster->monster.iState)
	{
	case STATE_DEAD:
		// [6/8/2012] Dead creatures don't have emotions... ~hogsy
		// [23/9/2012] Simplified ~hogsy
		for(i = 0; i < EMOTION_NONE; i++)
			eMonster->monster.fEmotion[i] = 0;

		// [20/9/2012] TODO: Check if we should gib? ~hogsy
		break;
	case STATE_ASLEEP:
		eMonster->monster.fEmotion[EMOTION_BOREDOM]++;
		break;
	case STATE_AWAKE:
		switch(eMonster->monster.iThink)
		{
		case THINK_WANDERING:
			eMonster->monster.fEmotion[EMOTION_BOREDOM]--;
			eMonster->monster.fEmotion[EMOTION_ANGER]--;
			break;
		case THINK_ATTACKING:
			eMonster->monster.fEmotion[EMOTION_BOREDOM]--;
			eMonster->monster.fEmotion[EMOTION_ANGER]++;
			break;
		case THINK_FLEEING:
			eMonster->monster.fEmotion[EMOTION_BOREDOM]--;
			if(eMonster->monster.fEmotion[EMOTION_ANGER] >= 50.0f)
				eMonster->monster.fEmotion[EMOTION_FEAR]--;
			else
				eMonster->monster.fEmotion[EMOTION_FEAR]++;
			break;
		case THINK_PURSUING:
			// [1/9/2013] TODO: Handle emotions... ~hogsy
			break;
		case THINK_IDLE:
			eMonster->monster.fEmotion[EMOTION_BOREDOM]++;
			break;
		default:
			Engine.Con_Warning("No think was set for %s at spawn!\n",eMonster->v.cClassname);

			Monster_SetThink(eMonster,THINK_IDLE);
			return;
		}
		break;
	default:
		Engine.Con_Warning("Unknown state set for monster! (%s)\n",eMonster->v.cClassname);
	}

	// [6/8/2012] Don't let the emotion value get too high or low ~hogsy
	// [30/6/2013] Moved down here ~hogsy
	for(i = 0; i < EMOTION_NONE; i++)
	{
		// [4/2/2013] Made a quick fix to these, derp! ~hogsy
		if(eMonster->monster.fEmotion[i] > 100.0f)
			eMonster->monster.fEmotion[i] = 100.0f;
		else if(eMonster->monster.fEmotion[i] < -100.0f)
			eMonster->monster.fEmotion[i] = -100.0f;

		//Engine.Con_DPrintf("EMOTION %i: %f (%s)\n",i,eMonster->monster.fEmotion[i],eMonster->v.cClassname);
	}

	if(eMonster->monster.Think)
		eMonster->monster.Think(eMonster);
}

// [30/7/2012] Added Monster_WalkMove ~hogsy
bool Monster_WalkMove(edict_t *ent,float yaw,float dist)
{
	vec3_t move;

	if(!(ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM)))
		return false;

	yaw *= (float)pMath_PI*2.0f/360.0f;

	move[0] = (vec_t)cos(yaw)*dist;
	move[1] = (vec_t)sin(yaw)*dist;
	move[2] = 0;

	return Monster_MoveStep(ent,move,true);
}

/*
	Movement
*/

void Monster_MoveForward(edict_t *eMonster)
{
	if(!(eMonster->v.flags & FL_ONGROUND))
		return;
}

void Monster_MoveBackward(void)
{}

void Monster_MoveLeft(void)
{}

void Monster_MoveRight(void)
{}

/*	Allows a monster to jump with the given velocity.
*/
void Monster_Jump(edict_t *eMonster,float fVelocity)
{
	/*	TODO: Check if where we're
		jumping into is safe or
		that it's a reasonable
		height that doesn't kill
		us
	*/
	if(eMonster->v.iHealth <= 0 || eMonster->v.velocity[2] || !(eMonster->v.flags & FL_ONGROUND))
		return;

	eMonster->v.flags		-= FL_ONGROUND;
	eMonster->v.velocity[2]	= fVelocity;
}

/*
*/
