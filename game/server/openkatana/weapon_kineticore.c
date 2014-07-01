#include "server_main.h"

void Kineticore_Deploy(edict_t *ent)
{
	//Weapon_Animate(ent,FALSE,1,14,0.1f,0,0,0,FALSE);
}

void Kineticore_IceballExplode(edict_t *ent)
{
	vec3_t vel;

	vel[0] = ent->v.velocity[0]*-1;
	vel[1] = ent->v.velocity[1]*-1;
	vel[2] = ent->v.velocity[2]*-1;

	Engine.Particle(ent->v.origin,vel,5,"ice",17);

	Math_VectorClear(ent->v.velocity);

	Entity_Remove(ent);
}
void IceballTouch(edict_t *ent, edict_t *other)
{
	// [26/2/2012] Revised and fixed ~hogsy
	// [25/6/2012] Revised ~hogsy
	vec3_t vel;

	// [25/6/2012] Simplified ~hogsy
	Math_VectorCopy(ent->v.velocity,vel);
	Math_VectorInverse(vel);

	if(other->v.bTakeDamage)
		MONSTER_Damage(other,ent,50,DAMAGE_TYPE_NONE);
	else if (ent->local.hit < 9)
	{
		if(!ent->v.angles[1] || !ent->v.angles[0])
			ent->v.velocity[2] += 200;

		ent->v.angles[2] -= 180;

		ent->local.hit += 1;

		Engine.Particle(ent->v.origin,vel,5,"ice",13);

		return;
	}

	Engine.Particle(ent->v.origin,vel,5,"ice",17);
	Sound(ent,CHAN_ITEM,"weapons/freeze.wav",255,ATTN_NORM);

	// [25/6/2012] Simplified ~hogsy
	Math_VectorClear(ent->v.velocity);

	RemoveEntity(ent);
}

void projectile_iceball(edict_t *ent, vec3_t orig)
{
	// [26/2/2012] Revised and fixed ~hogsy
	float   *dir;
	edict_t *ionball = Spawn();

	ionball->v.cClassname	= "iceball";
	ionball->v.movetype		= MOVETYPE_FLYBOUNCE;
	ionball->Physics.iSolid		= SOLID_BBOX;

	ionball->local.eOwner = ent;

	dir = Engine.Aim(ent);
	ionball->v.velocity[0] = dir[0]*2500.0f;
	ionball->v.velocity[1] = dir[1]*2500.0f;
	ionball->v.velocity[2] = dir[2]*2500.0f;

	Engine.MakeVectors(ionball->v.v_angle);

	Entity_SetModel(ionball,"models/iceball.md2");

	Math_MVToVector(Math_VectorToAngles(ionball->v.velocity),ionball->v.angles);

	Entity_SetSize(ionball,0,0,0,0,0,0);
	SetOrigin(ionball, orig);

	Engine.LinkEntity(ionball,false);

	ionball->v.TouchFunction	= IceballTouch;
	ionball->v.dNextThink		= Server.dTime+3.0;
	ionball->v.think			= Kineticore_IceballExplode;
}

void Kineticore_PrimaryAttack(edict_t *ent)
{
	if(!ent->local.kineticore_ammo2)
	{
		ent->local.kineticore_ammo2 = 5;
		ent->local.dAttackFinished = Server.dTime+0.7;
		return;
	}

	//Weapon_Animate(ent,FALSE,27,37,0.3f,10,19,0,FALSE);

	ent->v.iPrimaryAmmo = ent->local.kineticore_ammo--;
	ent->local.kineticore_ammo2--;

	projectile_iceball(ent, ent->v.origin);

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.1;
	else
		ent->local.dAttackFinished = Server.dTime+0.25;
}
