#include "server_weapon.h"

void Crossbow_Deploy(edict_t *ent)
{
	//Weapon_Animate(ent,FALSE,53,64,0.1f,0,0,0,FALSE);
}

void arrow_touch(edict_t *ent, edict_t *other)
{
	// [25/6/2012] Cleaned up ~hogsy
	// [7/4/2012] Cleaned up ~hogsy
	char	snd[64];

	if(other == ent->local.eOwner)
		return;

	// We hit an enemy! Stick with 'em
	if(other->v.bTakeDamage)
	{
		// Fleshy sound plz
		sprintf(snd,"weapons/crossbow/arrowwetimpact%i.wav",rand()%5+1);

		MONSTER_Damage(other,ent,25,0);

		ent->v.think		= WEAPON_StickThink;
		ent->v.dNextThink	= Server.dTime+0.1;
	}
	// [25/6/2012] Moved so we don't set this before checking what we're hitting ~hogsy
	else
		sprintf(snd,"weapons/crossbow/arrowimpact%i.wav",rand()%5+1);

	Sound(ent,CHAN_ITEM,snd,255,ATTN_NORM);

	ent->v.velocity[0] = ent->v.velocity[1] = ent->v.velocity[2] = 0;
	ent->v.avelocity[0] = ent->v.avelocity[1] = ent->v.avelocity[2] = 0;
	ent->v.enemy = other;
}

void projectile_arrow(edict_t *ent)
{
	// [11/2/2012] Revised and fixed ~hogsy
	float	*dir;
	edict_t *eArrow;

	eArrow = Entity_Spawn();
	if(eArrow)
	{
		eArrow->local.eOwner = ent;

		eArrow->v.movetype	= MOVETYPE_FLY;

		eArrow->Physics.iSolid	= SOLID_BBOX;

		dir = Engine.Aim(ent);
		eArrow->v.velocity[0] = dir[0]*2000.0f;
		eArrow->v.velocity[1] = dir[1]*2000.0f;
		eArrow->v.velocity[2] = dir[2]*2000.0f;

		Engine.MakeVectors(eArrow->v.v_angle);

		Entity_SetModel(eArrow,"models/arrow.md2");
		Entity_SetSizeVector(eArrow,mv3Origin,mv3Origin);

		// [25/6/2012] Simplified ~hogsy
		Math_VectorCopy(ent->v.origin,eArrow->v.origin);
		eArrow->v.origin[2] += 15.0f;

		Math_MVToVector(Math_VectorToAngles(ent->v.velocity),ent->v.angles);

		eArrow->v.TouchFunction = arrow_touch;
	}
}

void Crossbow_PrimaryAttack(edict_t *ent)
{
	if (ent->v.waterlevel >= 2)
		Sound(ent,CHAN_WEAPON,"weapons/crossbow/cbwaterfire.wav",255,ATTN_NORM);
	else
		Sound(ent,CHAN_WEAPON,"weapons/crossbow/cbfire.wav",255,ATTN_NORM);

	ent->v.punchangle[0] -= 5.0f;
	ent->v.iPrimaryAmmo = ent->local.crossbow_ammo -= 1;

	//Weapon_Animate(ent,FALSE,1,52,0.043f,10,19,0,FALSE);

	projectile_arrow(ent);

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+1.0;
	else
		ent->local.dAttackFinished = Server.dTime+2.0;
}
