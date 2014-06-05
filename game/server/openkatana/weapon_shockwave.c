/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_weapon.h"

#include "server_player.h"

void Shockwave_SpawnProjectile(edict_t *ent);

EntityFrame_t ShockwaveAnimation_Deploy [] =
{
	{   NULL, 37, 0.06f    },
	{   NULL, 38, 0.06f    },
	{   NULL, 39, 0.06f    },
	{   NULL, 40, 0.06f    },
	{   NULL, 41, 0.06f    },
	{   NULL, 42, 0.06f    },
	{   NULL, 43, 0.06f    },
	{   NULL, 44, 0.06f    },
	{   NULL, 45, 0.06f    },
	{   NULL, 46, 0.06f    },
	{   NULL, 47, 0.06f, TRUE    }
};
EntityFrame_t ShockwaveAnimation_Fire [] =
{
	{   NULL, 1, 0.06f    },
	{   NULL, 2, 0.06f    },
	{   NULL, 3, 0.06f    },
	{   NULL, 4, 0.06f    },
	{   NULL, 5, 0.06f    },
	{   NULL, 6, 0.06f    },
	{   Shockwave_SpawnProjectile, 7, 0.06f    },
	{   NULL, 8, 0.06f    },
	{   NULL, 9, 0.06f    },
	{   NULL, 10, 0.06f    },
	{   NULL, 11, 0.06f    },
	{   NULL, 12, 0.06f    },
	{   NULL, 13, 0.06f    },
	{   NULL, 14, 0.06f    },
	{   NULL, 15, 0.06f    },
	{   NULL, 16, 0.06f    },
	{   NULL, 17, 0.06f    },
	{   NULL, 18, 0.06f    },
	{   NULL, 19, 0.06f    },
	{   NULL, 20, 0.06f    },
	{   NULL, 21, 0.06f    },
	{   NULL, 22, 0.06f    },
	{   NULL, 23, 0.06f    },
	{   NULL, 24, 0.06f    },
	{   NULL, 25, 0.06f    },
	{   NULL, 26, 0.06f    },
	{   NULL, 27, 0.06f, TRUE    }
};

void Shockwave_Deploy(edict_t *ent)
{
	WEAPON_Animate(ent,ShockwaveAnimation_Deploy);
	Sound(ent,CHAN_WEAPON,"weapons/shockwave/ready.wav",255,ATTN_NORM);
}

void ShockLaser_Touch(edict_t *ent, edict_t *other)
{
	char	*cSound;

	if(other && other == ent->local.eOwner)
		return;

	if(other->v.bTakeDamage)
	{
		// burning flesh sound
		cSound = "weapons/shockwave/burn.wav";

		MONSTER_Damage(other,ent,100,0);
	}
	else
		cSound = "weapons/shockwave/fade.wav";

	Sound(ent,CHAN_ITEM,cSound,255,ATTN_NORM);

	Entity_Remove(ent);
}

// [19/8/2012] Renamed to Shockwave_SpawnProjectile ~hogsy
void Shockwave_SpawnProjectile(edict_t *ent)
{
	edict_t *eLaser;

	Sound(ent,CHAN_WEAPON,"weapons/shockwave/fire.wav",255,ATTN_NORM);

	// [27/1/2013] Moved punch here so that it happens when we actually throw out the projectile ~hogsy
	ent->v.punchangle[0] -= 10.0f;
	
	eLaser = Spawn();
	if(eLaser)
	{
		Engine.MakeVectors(ent->v.v_angle);

		Math_VectorScale(Engine.Aim(ent),2000.0f,eLaser->v.velocity);

		Engine.MakeVectors(eLaser->v.v_angle);

		Math_VectorCopy(ent->v.origin,eLaser->v.origin);
		Math_MVToVector(Math_VectorToAngles(eLaser->v.velocity),eLaser->v.angles);

		eLaser->local.eOwner = ent;

		eLaser->v.movetype		= MOVETYPE_FLY;
		eLaser->v.TouchFunction = ShockLaser_Touch;
		eLaser->v.origin[2]		+= 25.0f;

		eLaser->Physics.iSolid	= SOLID_BBOX;

		Entity_SetModel(eLaser,"models/slaser.md2");
		Entity_SetSizeVector(eLaser,vec3_origin,vec3_origin);
	}

	// This fixes the ammo bug ~eukos [05/08/2013]
	ent->local.shockwave_ammo--;
	ent->v.iPrimaryAmmo = ent->local.shockwave_ammo;
}

void Shockwave_PrimaryAttack(edict_t *ent)
{
	Sound(ent,CHAN_WEAPON,"weapons/shockwave/warmup.wav",255,ATTN_NORM);

	WEAPON_Animate(ent,ShockwaveAnimation_Fire);

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.65;
	else
		ent->local.dAttackFinished = Server.dTime+1.3;
}
