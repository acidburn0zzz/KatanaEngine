/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "Game.h"

/*
	Episode One's basic rocket launcher, the SideWinder.
*/

#include "server_player.h"
#include "server_weapon.h"

#define	SIDEWINDER_MAXSPEED	600.0f
#define SIDEWINDER_MINSPEED	50.0f

EntityFrame_t SideWinderAnimation_Deploy [] =
{
	{   NULL, 30, 0.035f    },
	{   NULL, 31, 0.035f    },
	{   NULL, 32, 0.035f    },
	{   NULL, 33, 0.035f    },
	{   NULL, 34, 0.035f    },
	{   NULL, 35, 0.035f    },
	{   NULL, 36, 0.035f    },
	{   NULL, 37, 0.035f    },
	{   NULL, 38, 0.035f    },
	{   NULL, 39, 0.035f    },
	{   NULL, 40, 0.035f    },
	{   NULL, 41, 0.035f    },
	{   NULL, 42, 0.035f    },
	{   NULL, 43, 0.035f    },
	{   NULL, 44, 0.035f    },
	{   NULL, 45, 0.035f    },
	{   NULL, 46, 0.035f    },
	{   NULL, 47, 0.035f    },
	{   NULL, 48, 0.035f    },
	{   NULL, 49, 0.035f    },
	{   NULL, 50, 0.035f    },
	{   NULL, 51, 0.035f    },
	{   NULL, 52, 0.035f    },
	{   NULL, 53, 0.035f    },
	{   NULL, 54, 0.035f    },
	{   NULL, 55, 0.035f    },
	{   NULL, 56, 0.035f		},
	{   NULL, 57, 0.035f, true	}
};

EntityFrame_t SideWinderAnimation_Fire [] =
{
	{   NULL, 1, 0.035f    },
	{   NULL, 2, 0.035f    },
	{   NULL, 3, 0.035f    },
	{   NULL, 4, 0.035f    },
	{   NULL, 5, 0.035f    },
	{   NULL, 6, 0.035f    },
	{   NULL, 7, 0.035f    },
	{   NULL, 8, 0.035f    },
	{   NULL, 9, 0.035f    },
	{   NULL, 10, 0.035f    },
	{   NULL, 11, 0.035f    },
	{   NULL, 12, 0.035f    },
	{   NULL, 13, 0.035f    },
	{   NULL, 14, 0.035f    },
	{   NULL, 15, 0.035f    },
	{   NULL, 16, 0.035f    },
	{   NULL, 17, 0.035f    },
	{   NULL, 18, 0.035f    },
	{   NULL, 19, 0.035f    },
	{   NULL, 20, 0.035f    },
	{   NULL, 21, 0.035f    },
	{   NULL, 22, 0.035f    },
	{   NULL, 23, 0.035f    },
	{   NULL, 24, 0.035f    },
	{   NULL, 25, 0.035f    },
	{   NULL, 26, 0.035f    },
	{   NULL, 27, 0.035f    },
	{   NULL, 28, 0.035f    },
	{   NULL, 29, 0.035f, true    }
};

void SideWinder_Think(edict_t *ent);

void SideWinder_Deploy(edict_t *ent)
{
	// [31/12/2013] TODO: Deploy sound? ~hogsy

	WEAPON_Animate(ent,SideWinderAnimation_Deploy);
}

// [2/8/2012] Renamed to SideWinder_MissileExplode ~hogsy
void SideWinder_MissileExplode(edict_t *ent,edict_t *other)
{
	vec3_t	vVelocity;

	if(other && (other == ent->local.eOwner))
	{
		SideWinder_Think(ent);
		return;
	}

	Entity_RadiusDamage(ent,MONSTER_RANGE_NEAR,30);

	// [4/8/2012] Simplified ~hogsy
	Math_VectorNegate(ent->v.velocity,vVelocity);

	// [4/8/2012] TODO: Underwater explosion too! ~hogsy
	Sound(ent,CHAN_AUTO,va("fx/explosion%i.wav",rand()%6+1),255,ATTN_NORM);

	Engine.Particle(ent->v.origin,vVelocity,1.0f,"spark2",25);

	Entity_Remove(ent);
}

// [4/8/2012] Added SideWinder_Think ~hogsy
void SideWinder_Think(edict_t *eSideWinder)
{
	// [31/8/2012] Complete rewrite ~hogsy
	int	iContents = Engine.Server_PointContents(eSideWinder->v.origin);

	// [4/8/2012] Check if the contents have changed ~hogsy
	if(iContents ==	(BSP_CONTENTS_WATER || CONTENT_SLIME	|| CONTENT_LAVA))
		eSideWinder->local.speed = SIDEWINDER_MINSPEED;
	// [31/8/2012] Don't let us explode in the sky... ~hogsy
	else if(iContents == CONTENT_SKY)
	{
		Entity_Remove(eSideWinder);
		// [15/8/2013] Oops! Return here! ~hogsy
		return;
	}
	else
		eSideWinder->local.speed = SIDEWINDER_MAXSPEED;

	if(eSideWinder->local.count > 12)
		eSideWinder->local.count = 0;

//	eSideWinder->v.velocity[X]	*= ((float)sin(Server.dTime*1.5f)*5.5f)/10.0f;
//	eSideWinder->v.velocity[Y]	*= -((float)sin(Server.dTime*1.5f)*5.5f)/10.0f;
//	eSideWinder->v.velocity[Z]	*= eSideWinder->v.velocity[X]/eSideWinder->v.velocity[Y];

	if(Server.dTime >= eSideWinder->local.fSpawnDelay)
	{
		SideWinder_MissileExplode(eSideWinder,NULL);
		return;
	}

	eSideWinder->v.dNextThink = Server.dTime+0.05;
}

// [4/8/2012] Renamed to SideWinder_SpawnMissle ~hogsy
void SideWinder_SpawnMissle(edict_t *ent,float fSpeed,float ox)
{
	// [26/2/2012] Revised and fixed ~hogsy
	vec3_t	vOrg;
	edict_t *eMissile = Spawn();

	/*	TODO:
			Spawn a flare at our position too
		~hogsy
	*/

	eMissile->v.cClassname	= "sidewindermissile";
	eMissile->v.movetype	= MOVETYPE_FLYMISSILE;
	eMissile->v.effects		= EF_PARTICLE_SMOKE|EF_DIMLIGHT;

	eMissile->Physics.iSolid	= SOLID_BBOX;
	eMissile->Physics.eIgnore	= ent;

	eMissile->local.speed	= SIDEWINDER_MAXSPEED;
	eMissile->local.eOwner	= ent;
	eMissile->local.count	= 0;
	// [4/8/2012] Change our speed depending on what contents we're within ~hogsy
	eMissile->local.speed	= fSpeed;

	Math_VectorScale(Engine.Aim(ent),eMissile->local.speed,eMissile->v.velocity);

	Math_AngleVectors(ent->v.v_angle,
		// [4/8/2012] Set up our angle vectors ~hogsy
		eMissile->v.vForward,
		eMissile->local.vRight,
		eMissile->local.vUp);
	Math_VectorCopy(ent->v.v_angle,eMissile->v.angles);

	Entity_SetModel(eMissile,"models/sidewinder_missile.md2");

	Math_VectorCopy(ent->v.origin,vOrg);

	vOrg[0] += eMissile->v.vForward[0]*8+eMissile->local.vRight[0]*ox;
	vOrg[1] += eMissile->v.vForward[1]*8+eMissile->local.vRight[1]*ox;
	vOrg[2] += eMissile->v.vForward[2]*24;

	Entity_SetSizeVector(eMissile,vec3_origin,vec3_origin);
	Entity_SetOrigin(eMissile,vOrg);

	// [4/8/2012] Time at which we'll be removed if nothing hit ~hogsy
	eMissile->local.fSpawnDelay = (float)(Server.dTime+8.0);

	eMissile->v.TouchFunction	= SideWinder_MissileExplode;
	eMissile->v.dNextThink		= Server.dTime+0.05;
	eMissile->v.think			= SideWinder_Think;

	// [4/8/2012] Moved so we do this last! ~hogsy
	Engine.LinkEntity(eMissile,false);
}

// [4/8/2012] Renamed to SideWinder_PrimaryAttack ~hogsy
void SideWinder_PrimaryAttack(edict_t *eOwner)
{
	float	fSpeed	= SIDEWINDER_MAXSPEED;
	char	*cSound = "weapons/sidewinder/sidewinderfire.wav";

	// [31/12/2013] Only do this once when spawning, here! ~hogsy
	if(Engine.Server_PointContents(eOwner->v.origin) ==
		(BSP_CONTENTS_WATER || CONTENT_SLIME || CONTENT_LAVA))
	{
		cSound = "weapons/sidewinder/sidewinderunderwaterfire.wav";

		fSpeed = SIDEWINDER_MINSPEED;
	}

	Sound(eOwner,CHAN_WEAPON,cSound,255,ATTN_NORM);

	WEAPON_Animate(eOwner,SideWinderAnimation_Fire);

	// [1/10/2012] Readded ~hogsy
	eOwner->v.iPrimaryAmmo	= eOwner->local.sidewinder_ammo -= 2;

	eOwner->v.punchangle[PITCH] -= (float)(rand()%5+1);

	SideWinder_SpawnMissle(eOwner,fSpeed,10);
	SideWinder_SpawnMissle(eOwner,fSpeed,-10);

	eOwner->local.dAttackFinished = Server.dTime+1.2;
}
