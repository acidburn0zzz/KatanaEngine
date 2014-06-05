#include "server_main.h"

void GreekFire_Deploy(edict_t *ent)
{
	//WEAPON_Animate(ent,FALSE,1,7,0.1f,0,0,0,FALSE);
}

void GreekfireTouch(edict_t *ent, edict_t *other)
{
	vec3_t vel;

	if (other == ent->local.eOwner)
		return;

	if(other->v.bTakeDamage)
		MONSTER_Damage(other,ent,50,0);

    // [25/6/2012] Simplified ~hogsy
	Math_VectorCopy(ent->v.velocity,vel);
	Math_VectorInverse(vel);

	Engine.Particle(ent->v.origin,vel,5,"spark",17);

	// [25/6/2012] Simplified ~hogsy
	Math_VectorClear(ent->v.velocity);

	RemoveEntity(ent);
}

void GreekFire_Throw(edict_t *ent)
{
	float	*dir;
	edict_t *greekfire = Spawn();

	greekfire->v.cClassname	= "greekfire";
	greekfire->v.movetype	= MOVETYPE_BOUNCE;
	greekfire->v.effects	= EF_DIMLIGHT;

	greekfire->Physics.iSolid = SOLID_BBOX;

	greekfire->local.eOwner = ent;

	dir = Engine.Aim(ent);
	greekfire->v.velocity[0] = dir[0]*800;
	greekfire->v.velocity[1] = dir[1]*800;
	greekfire->v.velocity[2] = dir[2]*800;

	Engine.MakeVectors(greekfire->v.v_angle);

	Entity_SetModel(greekfire,"models/w_greekfire.md2");
	Math_MVToVector(Math_VectorToAngles(greekfire->v.velocity),greekfire->v.angles);

	// [4/7/2012] Simplified ~hogsy
	Math_VectorCopy(ent->v.origin,greekfire->v.origin);

	Entity_SetSizeVector(greekfire,vec3_origin,vec3_origin);

	Engine.LinkEntity(greekfire,false);

	greekfire->v.TouchFunction = GreekfireTouch;
}

// [4/7/2012] Renamed to GreekFire_PrimaryAttack ~hogsy
void GreekFire_PrimaryAttack(edict_t *ent)
{
	//WEAPON_Animate(ent,FALSE,8,25,0.08f,0,0,0,FALSE);

	ent->v.iPrimaryAmmo = ent->local.greekfire_ammo--;

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.75;
	else
		ent->local.dAttackFinished = Server.dTime+1.5;
}
