#include "Game.h"

#include "server_weapon.h"

void PulseRifle_Deploy(edict_t *ent)
{
	//WEAPON_Animate(ent,FALSE,1,15,0.1f,0,0,0,FALSE);
}

void PulseRifle_PrimaryAttack(edict_t *ent)
{
	Sound(ent,CHAN_WEAPON,"weapons/pulserifle/pulsefire2.wav",255,ATTN_NORM);

#if 0
	if(rand()%2 == 1)
		//WEAPON_Animate(ent,FALSE,0,15,0.05f,10,19,0,FALSE);
	else
		//WEAPON_Animate(ent,FALSE,16,27,0.05f,10,19,0,FALSE);
#endif

	ent->v.effects |= EF_MUZZLEFLASH;

	ent->v.iPrimaryAmmo	= ent->local.glock_ammo--;
	ent->local.glock_ammo2--;

	ent->local.dAttackFinished	= Server.dTime+0.2;

	// [23/5/2012] Damage set to 5 due to high rate of fire ~hogsy
//	Weapon_BulletProjectile(ent,0.5f,5);
}

// [17/7/2012] Revised ~hogsy
void CorditeExplode(edict_t *ent)
{
	vec3_t vel;

	// [25/6/2012] Simplified ~hogsy
	Math_VectorCopy(ent->v.velocity,vel);
	Math_VectorInverse(vel);

	Engine.Particle(ent->v.origin,vel,5,"spark",17);

	// [25/6/2012] Simplified ~hogsy
	Math_VectorClear(ent->v.velocity);

	RemoveEntity(ent);
}

// [17/7/2012] Revised ~hogsy
void CorditeTouch(edict_t *ent, edict_t *other)
{
	if(other == ent->local.eOwner)
		return;

	CorditeExplode(ent);

	MONSTER_Damage(other,ent,50);
}

void throw_cordite(edict_t *ent)
{
	float	*dir;
	edict_t *greekfire = Spawn();

	greekfire->v.cClassname	= "cordite";
	greekfire->v.movetype	= MOVETYPE_BOUNCE;
	greekfire->Physics.iSolid		= SOLID_BBOX;

	greekfire->local.eOwner = ent;

	dir = Engine.Aim(ent);
	greekfire->v.velocity[0] = dir[0]*2000.0f;
	greekfire->v.velocity[1] = dir[1]*2000.0f;
	greekfire->v.velocity[2] = dir[2]*2000.0f;

	Entity_SetModel(greekfire,"models/grenade.mdl");

    Math_MVToVector(Math_VectorToAngles(greekfire->v.velocity),greekfire->v.angles);

	Entity_SetSize(greekfire,0,0,0,0,0,0);

	// [25/6/2012] Simplified ~hogsy
	Math_VectorCopy(ent->v.origin,greekfire->v.origin);

	greekfire->v.think		= CorditeExplode;
	greekfire->v.dNextThink	= Server.dTime+3.0;

	Engine.LinkEntity(greekfire,false);

	greekfire->v.TouchFunction = CorditeTouch;
}

void PulseRifle_SecondaryAttack(edict_t *ent)
{
	// [23/5/2012] Revised ~hogsy
	// [25/6/2012] Revised ~hogsy
	char *noise;

	if(ent->local.dAttackFinished > Server.dTime)
		return;

	switch(ent->local.iFireMode)
	{
	case 1:
		ent->local.iFireMode = 0;
		noise = "weapons/pulserifle/pulsechange2.wav";
		break;
	default:
		ent->local.iFireMode = 1;
		noise = "weapons/pulserifle/pulsechange1.wav";
	}

	Sound(ent,CHAN_WEAPON,noise,255,ATTN_NORM);

	ent->local.dAttackFinished = Server.dTime+1.0;
}
