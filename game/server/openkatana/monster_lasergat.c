/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_monster.h"

#include "server_waypoint.h"
#include "server_weapon.h"
#include "server_item.h"

enum
{
	COMMAND_ATTACK_WAIT,	// Delay before checking for a target (and thus attacking).

	COMMAND_IDLE_WAIT,		// Delay before switching to idle from attack.

	COMMAND_LOOK_PITCH,		// Should we look up or down?
	COMMAND_LOOK_YAW,		// Should we look left or right?
	COMMAND_LOOK_WAIT		// Delay until we start turning again.
};

#define LASERGAT_LOOK_RIGHT	0	// Look right.
#define	LASERGAT_LOOK_LEFT	1	// Look left.
#define	LASERGAT_LOOK_DOWN	0	// Look down.
#define	LASERGAT_LOOK_UP	1	// Look up.
#define LASERGAT_LOOK_NONE	2	// Don't do either.

#define	LASERGAT_MAX_WAIT	rand()%50

// [11/6/2013] So we don't have to set this up a billion times ~hogsy
Weapon_t *wLaserWeapon;

void LaserGat_AimTarget(edict_t *eLaserGat,edict_t *eTarget)
{
	vec3_t	vOrigin;

	Math_VectorSubtract(eLaserGat->v.origin,eTarget->v.origin,vOrigin);
	Math_VectorNormalize(vOrigin);
	Math_MVToVector(Math_VectorToAngles(vOrigin),eLaserGat->v.angles);
	Math_VectorInverse(eLaserGat->v.angles);
}

void LaserGat_HandleAim(edict_t *eLaserGat)
{
	if(eLaserGat->monster.iThink == THINK_IDLE)
	{
		if(!eLaserGat->monster.eTarget)
		{
		}
		else
			LaserGat_AimTarget(eLaserGat,eLaserGat->monster.eTarget);
    }
    else if(eLaserGat->monster.iThink == THINK_ATTACKING)
		LaserGat_AimTarget(eLaserGat,eLaserGat->monster.eEnemy);
}

void LaserGat_Touch(edict_t *eLaserGat,edict_t *eOther)
{
	// [12/3/2013] TODO: Explode ~hogsy
	Sound(eLaserGat,CHAN_BODY,PHYSICS_SOUND_METAL2,255,ATTN_STATIC);
}

void LaserGat_Explode(edict_t *eLaserGat)
{
	Sound(eLaserGat,CHAN_AUTO,va("fx/explosion%i.wav",rand()%6+1),255,ATTN_NORM);

	Entity_RadiusDamage(eLaserGat,300.0f,25,DAMAGE_TYPE_EXPLOSIVE);
	Entity_Remove(eLaserGat);
}

void LaserGat_Die(edict_t *eLaserGat,edict_t *eOther)
{
	eLaserGat->v.think		= LaserGat_Explode;
	eLaserGat->v.dNextThink	= Server.dTime+5.0;
}

void LaserGat_Think(edict_t *eLaserGat)
{
	switch(eLaserGat->monster.iThink)
	{
	case THINK_IDLE:
        // [22/4/2014] Check if there are any targets nearby... ~hogsy
		if(!eLaserGat->monster.iCommandList[COMMAND_ATTACK_WAIT])
		{
			eLaserGat->monster.eTarget = Monster_GetTarget(eLaserGat);
			if(eLaserGat->monster.eTarget)
			{
				// [11/6/2013] Try setting the target as an enemy... ~hogsy
				if(Monster_SetEnemy(eLaserGat))
				{
					// [6/4/2013] Set think and state for next frame ~hogsy
					Monster_SetThink(eLaserGat,THINK_ATTACKING);
					Monster_SetState(eLaserGat,STATE_AWAKE);
					return;
				}
				else
				{
					eLaserGat->monster.eOldTarget	= eLaserGat->monster.eTarget;
					eLaserGat->monster.eTarget		= NULL;
				}
			}
			// [6/4/2013] TODO: Check if it's neutral and worth damaging? ~hogsy

			eLaserGat->monster.iCommandList[COMMAND_ATTACK_WAIT] = 5;
		}
		else
            eLaserGat->monster.iCommandList[COMMAND_ATTACK_WAIT]--;

        if(!eLaserGat->monster.iCommandList[COMMAND_LOOK_WAIT])
        {
			if(eLaserGat->monster.iCommandList[COMMAND_LOOK_PITCH] == LASERGAT_LOOK_DOWN)
				eLaserGat->v.angles[0] -= 0.5f;
			else if(eLaserGat->monster.iCommandList[COMMAND_LOOK_YAW] == LASERGAT_LOOK_UP)
				eLaserGat->v.angles[0] += 0.5f;

			if(eLaserGat->monster.iCommandList[COMMAND_LOOK_YAW] == LASERGAT_LOOK_LEFT)
				eLaserGat->v.angles[1] += 0.5f;
			else if(eLaserGat->monster.iCommandList[COMMAND_LOOK_YAW] == LASERGAT_LOOK_RIGHT)
				eLaserGat->v.angles[1] -= 0.5f;

			if(rand()%125 == 1)
			{
				eLaserGat->monster.iCommandList[COMMAND_LOOK_WAIT] = LASERGAT_MAX_WAIT;
				if(rand()%2 == 1)
					eLaserGat->monster.iCommandList[COMMAND_LOOK_PITCH]	= LASERGAT_LOOK_UP;
				else
					eLaserGat->monster.iCommandList[COMMAND_LOOK_PITCH]	= LASERGAT_LOOK_DOWN;

				if(rand()%2 == 1)
					eLaserGat->monster.iCommandList[COMMAND_LOOK_YAW]	= LASERGAT_LOOK_LEFT;
				else
					eLaserGat->monster.iCommandList[COMMAND_LOOK_YAW]	= LASERGAT_LOOK_RIGHT;
			}
        }
		break;
	case THINK_ATTACKING:
		{
			if(!eLaserGat->monster.eEnemy)
			{
				Monster_SetThink(eLaserGat,THINK_IDLE);
				return;
			}
			else if(!eLaserGat->monster.eEnemy->v.iHealth)
			{
				// [6/4/2013] TODO: Add a delay here until we actually stop shooting? Means we might gib it which would be kewl... ~hogsy
				eLaserGat->monster.eEnemy = NULL;
				break;
			}

            // [22/4/2014] Switched over to use this instead since it manages delay etc for us :) ~hogsy
			Weapon_PrimaryAttack(eLaserGat);
		}
		break;
	default:
		Monster_SetThink(eLaserGat,THINK_IDLE);
	}

    // [22/4/2014] Make sure our pitch doesn't go crazy, heh ~hogsy
    if(eLaserGat->v.angles[0] > 10.0f)
		eLaserGat->v.angles[0] = 10.0f;
	else if(eLaserGat->v.angles[0] < -60.0f)
		eLaserGat->v.angles[0] = -60.0f;
}

void LaserGat_BasePain(edict_t *eBase,edict_t *eOther)
{
	Sound(eBase,CHAN_BODY,PHYSICS_SOUND_METAL2,255,ATTN_STATIC);
}

void LaserGat_BaseDie(edict_t *eBase,edict_t *eOther)
{
	Entity_SetModel(eBase,LASERGAT_MODEL_BROKEN);

	// [6/4/2013] Check that we still have our owner... ~hogsy
	if(eBase->local.eOwner)
	{
		// [6/4/2013] Clear out the owners owner ~hogsy
		eBase->local.eOwner->local.eOwner = NULL;

		eBase->local.eOwner->v.movetype			= MOVETYPE_BOUNCE;
		eBase->local.eOwner->v.TouchFunction	= LaserGat_Touch;

		Math_VectorScale(
			eBase->local.eOwner->v.avelocity,
			(float)eBase->v.iHealth,
			eBase->local.eOwner->v.avelocity);

		Monster_Killed(eBase->local.eOwner,eOther);
	}
}

void LaserGat_Spawn(edict_t *eLaserGat)
{
	Engine.Server_PrecacheResource(RESOURCE_MODEL,LASERGAT_MODEL_BASE);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,LASERGAT_MODEL_BROKEN);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,LASERGAT_MODEL_HEAD);

	// [11/6/2013] Get our weapon ~hogsy
	Item_AddInventory(Item_GetItem(WEAPON_LASERS),eLaserGat);

	eLaserGat->v.netname		= "Laser Turret";
	eLaserGat->v.movetype		= MOVETYPE_NONE;
	eLaserGat->v.iHealth		= 150;
	eLaserGat->v.bTakeDamage	= true;

	eLaserGat->monster.think_die		= LaserGat_Die;
//	eLaserGat->monster.think_pain		= LaserGat_Pain;
	eLaserGat->monster.Think			= LaserGat_Think;
	eLaserGat->monster.fViewDistance	= MONSTER_RANGE_MEDIUM;
	eLaserGat->monster.iType			= MONSTER_LASERGAT;

	// [28/6/2013] Make this random so not all turrets start looking in the same directions ~hogsy
	eLaserGat->monster.iCommandList[COMMAND_LOOK_PITCH]	= rand()%2;
	eLaserGat->monster.iCommandList[COMMAND_LOOK_YAW]	= rand()%2;

	// [6/6/2013] Set our physical properties ~hogsy
	eLaserGat->Physics.iSolid		= SOLID_BBOX;
	eLaserGat->Physics.fGravity		= cvServerGravity.value;
	eLaserGat->Physics.fMass		= 3.5f;

	Entity_SetModel(eLaserGat,LASERGAT_MODEL_HEAD);
	Entity_SetSize(eLaserGat,-6.20f,-18.70f,-8.0f,19.46f,18.71f,7.53f);
	Entity_SetOrigin(eLaserGat,eLaserGat->v.origin);

	Monster_SetState(eLaserGat,STATE_AWAKE);
	Monster_SetThink(eLaserGat,THINK_IDLE);

	{
		// [19/2/2013] Now set up the base... ~hogsy
		edict_t	*eBase = Spawn();
		if(eBase)
		{
			eBase->v.cClassname		= "lasergat_base";
			eBase->v.iHealth		= 100;
			eBase->v.movetype		= MOVETYPE_NONE;
			eBase->v.bTakeDamage	= true;

			// Physical properties
			eBase->Physics.iSolid		= SOLID_BBOX;
			eBase->Physics.fGravity		= 0;
			eBase->Physics.fFriction	= 0;
			eBase->Physics.fMass		= 0;

			// [10/3/2013] This was pain rather than die, fixed ~hogsy
			eBase->monster.think_die	= LaserGat_BaseDie;
			eBase->monster.think_pain	= LaserGat_BasePain;

			Entity_SetModel(eBase,LASERGAT_MODEL_BASE);
			Entity_SetSize(eBase,-22.70f,-19.60f,-18.70f,19.66f,19.28f,3.71f);
			Entity_SetOrigin(eBase,eLaserGat->v.origin);

			eLaserGat->v.origin[2] -= 30.0f;

			/*	[19/2/2013]
				To be debated, but we'll set the base as the turrets owner
				so we can destroy it on the turrets death... For now... ~hogsy
			*/
			eLaserGat->local.eOwner	= eBase;
			eBase->local.eOwner		= eLaserGat;

			eBase->Physics.eIgnore	= eLaserGat;
		}
	}
}
