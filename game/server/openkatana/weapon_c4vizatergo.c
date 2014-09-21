/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_weapon.h"

#include "server_player.h"
#include "server_physics.h"

#define C4VIZATERGO_MAX_RANGE	700

EntityFrame_t C4Animation_Deploy[] =
{
	{   NULL, 1, 0.08f    },
	{   NULL, 2, 0.08f    },
	{   NULL, 3, 0.08f    },
	{   NULL, 4, 0.08f    },
	{   NULL, 5, 0.08f    },
	{   NULL, 6, 0.08f    },
	{   NULL, 7, 0.08f    },
	{   NULL, 8, 0.08f    },
	{   NULL, 9, 0.08f    },
	{   NULL, 10, 0.08f, true    }
};

EntityFrame_t C4Animation_Fire [] =
{
	{   NULL, 11, 0.08f    },
	{   NULL, 12, 0.08f    },
	{   NULL, 13, 0.08f    },
	{   NULL, 14, 0.08f    },
	{   NULL, 15, 0.08f    },
	{   NULL, 16, 0.08f    },
	{   NULL, 17, 0.08f    },
	{   NULL, 18, 0.08f    },
	{   NULL, 19, 0.08f    },
	{   NULL, 20, 0.08f    },
	{   NULL, 21, 0.08f    },
	{   NULL, 22, 0.08f, true    }
};

EntityFrame_t C4Animation_Trigger [] =
{
	{   NULL, 24, 0.08f    },
	{   NULL, 25, 0.08f    },
	{   NULL, 26, 0.08f    },
	{   NULL, 27, 0.08f    },
	{   NULL, 28, 0.08f    },
	{   NULL, 29, 0.08f    },
	{   NULL, 30, 0.08f    },
	{   NULL, 31, 0.08f    },
	{   NULL, 32, 0.08f    },
	{   NULL, 33, 0.08f    },
	{   NULL, 34, 0.08f    },
	{   NULL, 35, 0.08f, true    }
};

void C4Vizatergo_Deploy(edict_t *ent)
{
	Sound(ent,CHAN_WEAPON,"weapons/c4/c4cock.wav",255,ATTN_NORM);

	Weapon_Animate(ent,C4Animation_Deploy);
}

void C4Vizatergo_Explode(edict_t *eVizatergo)
{
	Sound(eVizatergo,CHAN_AUTO,va("fx/explosion%i.wav",rand()%6+1),255,ATTN_NORM);

	eVizatergo->local.eOwner->local.bomb = 0;

	Entity_RadiusDamage(eVizatergo,MONSTER_RANGE_NEAR,40,DAMAGE_TYPE_EXPLODE);
	Entity_Remove(eVizatergo);
}

void C4Vizatergo_C4BallTouch(edict_t *ent,edict_t *eOther)
{
#if 0
	// [12/8/2012] Don't touch our owner ~hogsy
	if(other == ent->local.eOwner)
		return;

	// [12/8/2012] Play the stick sound! ~hogsy
	Sound(ent,CHAN_ITEM,"weapons/c4/c4stick.wav",255,ATTN_NORM);

	if(other->Physics.iSolid == SOLID_BSP && ent->v.movetype != MOVETYPE_NONE)
		ent->v.movetype = MOVETYPE_NONE;
	else
	{
		ent->v.TouchFunction	= NULL;
		ent->v.enemy			= other;
		ent->v.think			= WEAPON_StickThink;
		ent->v.dNextThink		= Server.time;
	}
#else
	// [12/12/2012] New method. Explode if touches monster / another player. ~hogsy

	if(	eOther != ent->local.eOwner && eOther->monster.iType)
		C4Vizatergo_Explode(ent);
	else if(eOther->Physics.iSolid == SOLID_BSP && ent->v.movetype != MOVETYPE_NONE)
	{
		// [12/8/2012] Play the stick sound! ~hogsy
		Sound(ent,CHAN_ITEM,"weapons/c4/c4stick.wav",255,ATTN_NORM);

		ent->v.movetype	= MOVETYPE_NONE;

		ent->local.hit = true;
	}
#endif
}

void C4Vizatergo_Think(edict_t *ent)
{
#if 0	// [12/12/2012] TODO: Finish... ~hogsy
	float	fDistance;
	vec3_t	vDistance;
	edict_t *eDistantEnt = Game.Server_FindRadius(ent->v.origin,512.0f);

	if(eDistantEnt)
	{
		Math_VectorSubtract(eDistantEnt->v.origin,ent->v.origin,vDistance);
		fDistance = 512.0f-Math_Length(vDistance);

		Game.Con_Printf("Distance: %f\n",fDistance);
		if(fDistance > 0)
			Sound(ent,CHAN_AUTO,"weapons/c4/c4beep.wav",255,ATTN_NORM);
	}
	else
		fDistance = 0;

	ent->v.dNextThink = Server.dTime+(2.5f*(fDistance/100.0f));
#else
	Sound(ent,CHAN_AUTO,"weapons/c4/c4beep.wav",120,ATTN_NORM);

	if(!ent->local.eOwner && ent->local.hit)
	{
		ent->v.movetype = MOVETYPE_BOUNCE;
	}

	ent->v.dNextThink = Server.dTime+2.5;
#endif
}

void C4Vizatergo_PrimaryAttack(edict_t *eOwner)
{
	// [26/2/2012] Revised and fixed ~hogsy
	vec3_t	vOrigin;
	edict_t *c4ball = Entity_Spawn();

	Sound(eOwner,CHAN_AUTO,"weapons/c4/c4fire.wav",255,ATTN_NORM);
	Sound(eOwner,CHAN_AUTO,"weapons/c4/c4cock.wav",255,ATTN_NORM);

	Weapon_Animate(eOwner,C4Animation_Fire);

	// [11/8/2013] Fixed ~hogsy
	eOwner->v.iPrimaryAmmo = eOwner->local.iC4Ammo -= 1;

	c4ball->v.cClassname	= "c4ball";
	c4ball->v.movetype		= MOVETYPE_BOUNCE;

	c4ball->local.style		= AMMO_C4BOMBS;		// Cleaner way to tell if this can explode or not :V ~hogsy
	c4ball->local.iC4Ammo	= 1;				// [11/8/2013] Since style is used for other shit too LAWL ~hogsy
	c4ball->local.eOwner	= eOwner;

	// [30/5/2013] Set the gravity properties ~hogsy
	c4ball->Physics.iSolid		= SOLID_BBOX;
	c4ball->Physics.fMass		= 0.9f;
	c4ball->Physics.eIgnore		= eOwner;
	c4ball->Physics.fGravity	= SERVER_GRAVITY;

	eOwner->local.bomb = c4ball;

	// [26/8/2012] Simplified ~hogsy
	Math_VectorScale(Engine.Aim(eOwner),C4VIZATERGO_MAX_RANGE,c4ball->v.velocity);

	c4ball->v.velocity[Y] += 20.0f;

	Engine.MakeVectors(c4ball->v.v_angle);

	Math_MVToVector(Math_VectorToAngles(c4ball->v.velocity),c4ball->v.angles);
	Math_VectorCopy(eOwner->v.origin,vOrigin);

	Entity_SetModel(c4ball,"models/c4ammo.md2");
	Entity_SetSizeVector(c4ball,vec3_origin,vec3_origin);
	Entity_SetOrigin(c4ball,vOrigin);

	c4ball->v.TouchFunction	= C4Vizatergo_C4BallTouch;
	c4ball->v.think			= C4Vizatergo_Think;
	c4ball->v.dNextThink	= Server.dTime+2.5;

	Engine.LinkEntity(c4ball,false);

	if(eOwner->local.attackb_finished > Server.dTime)	// No attack boost...
		eOwner->local.dAttackFinished = Server.dTime+0.6;
	else
		eOwner->local.dAttackFinished = Server.dTime+1.2;
}

void C4Vizatergo_SecondaryAttack(edict_t *eOwner)
{
	edict_t *eExplodable = Engine.Server_FindRadius(eOwner->v.origin,10000.0f);

	// [30/5/2013] Fixed ~hogsy
	do
	{
		// [11/8/2013] Fixed ~hogsy
		if((eExplodable->local.style == AMMO_C4BOMBS) && (eExplodable->local.iC4Ammo == 1) && (eExplodable->local.eOwner == eOwner))
		{
			eExplodable->v.think		= C4Vizatergo_Explode;
			eExplodable->v.dNextThink	= Server.dTime;
		}

		eExplodable = eExplodable->v.chain;
	} while(eExplodable);

	Weapon_Animate(eOwner,C4Animation_Trigger);
}
