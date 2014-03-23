#include "Game.h"

#include "server_weapon.h"

EntityFrame_t BallistaAnimation_Deploy [] =
{
	{   NULL, 35, 0.1f    },
	{   NULL, 36, 0.1f    },
	{   NULL, 37, 0.1f    },
	{   NULL, 38, 0.1f    },
	{   NULL, 39, 0.1f, TRUE    }
};

void Ballista_Deploy(edict_t *ent)
{
	WEAPON_Animate(ent,BallistaAnimation_Deploy);
}

void Ballista_LogTouch(edict_t *ent, edict_t *other)
{
	// [7/4/2012] Cleaned up ~hogsy
	char	snd[64];

	if(other == ent->local.eOwner)
		return;

	// We hit an enemy! Stick with 'em
	if(other->v.bTakeDamage)
	{
		// [23/5/2012] Cleaned up ~hogsy
		sprintf(snd,"weapons/ballista/logwetimpact%i.wav",rand()%4);

		MONSTER_Damage(other,ent,25);

		ent->v.think		= WEAPON_StickThink;
		ent->v.dNextThink	= Server.dTime+0.1;
	}
	// [23/5/2012] We didn't hit anything, so NOW we set up our impact sound ~hogsy
	else
		sprintf(snd,"weapons/ballista/logimpact%i.wav",rand()%5);

	Sound(ent,CHAN_ITEM,snd,255,ATTN_NORM);

	Math_VectorClear(ent->v.velocity);
	Math_VectorClear(ent->v.avelocity);

	ent->v.enemy = other;
}

void Ballista_SpawnLogProjectile(edict_t *ent)
{
	// [11/2/2012] Revised and fixed ~hogsy
	float	*dir;
	edict_t *log = Spawn();

	log->local.eOwner = ent;

	log->v.movetype = MOVETYPE_FLY;
	log->Physics.iSolid = SOLID_BBOX;

	dir = Engine.Aim(ent);

	log->v.velocity[0] = dir[0]*2000.0f;
	log->v.velocity[1] = dir[1]*2000.0f;
	log->v.velocity[2] = dir[2]*2000.0f;

	Engine.MakeVectors(log->v.v_angle);

	Entity_SetModel(log,"models/log.md2");
	Entity_SetSizeVector(log,vec3_origin,vec3_origin);

	// [1/7/2012] Simplified ~hogsy
	Math_VectorCopy(ent->v.origin,log->v.origin);
	log->v.origin[2] += 15.0f;

    Math_MVToVector(Math_VectorToAngles(log->v.velocity),log->v.angles);

	log->v.TouchFunction = Ballista_LogTouch;
}

void Ballista_PrimaryAttack(edict_t *ent)
{
	//if (ent->v.waterlevel >= 2)
	//	Sound(ent,CHAN_WEAPON,"weapons/ballista/cbwaterfire.wav",255,ATTN_NORM);
	//else
	//	Sound(ent,CHAN_WEAPON,"weapons/ballista/cbfire.wav",255,ATTN_NORM);

	ent->v.punchangle[0] -= 5.0f;

	ent->v.iPrimaryAmmo	= ent->local.ballista_ammo--;

	Ballista_SpawnLogProjectile(ent);

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+1.0;
	else
		ent->local.dAttackFinished = Server.dTime+2.0;
}
