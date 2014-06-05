#include "server_main.h"

void Midas_Deploy(edict_t *ent)
{
	//WEAPON_Animate(ent,FALSE,1,7,0.1f,0,0,0,FALSE);
}

void Midas_CloudThink(edict_t *ent)
{
	if(ent->local.hit == 0)
		ENTITY_REMOVE(ent);

	ent->local.hit = ent->local.hit - 2;

	Engine.Particle(ent->v.origin,vec3_origin,12.0f,"spark",8);

	ent->v.dNextThink	= Server.dTime+0.3;
	ent->v.think		= Midas_CloudThink;
}

void CloudTouch(edict_t *cloud, edict_t *other)
{
	// [13/4/2012] Fixed (please remember to use stricmp!) ~hogsy
	// [8/6/2012] Removed classname check for clients ~hogsy
	// [4/6/2013] Updated to use the new method ~hogsy
	if(Entity_IsPlayer(other))
		return;

	if(other->v.iHealth > 0 && other->v.movetype == MOVETYPE_STEP)
	{
		if(!other->local.hit) // Only play it once
			Sound(other,CHAN_VOICE,"weapons/midastouch.wav",255,ATTN_NORM);

		other->v.dNextThink = Server.dTime+15.0;

		other->local.hit = 1;
	}
	else
		// [18/4/2012] We probably hit a wall so don't hang about! ~hogsy
		Entity_Remove(cloud);
}

// [4/7/2012] Renamed to Midas_PrimaryAttack ~hogsy
void Midas_PrimaryAttack(edict_t *ent)
{
	edict_t *cloud = Spawn();
	float	*dir;

	cloud->v.cClassname	= "cloud";
	cloud->v.movetype	= MOVETYPE_FLYMISSILE;

	cloud->Physics.iSolid		= SOLID_TRIGGER;

	cloud->local.hit	= 10;
	cloud->local.eOwner = ent;

//	SetSize(cloud,-16,-16,-16,16,16,16);
	Entity_SetOrigin(cloud,ent->v.origin);

	Engine.MakeVectors(ent->v.v_angle);

	dir = Engine.Aim(ent);
	cloud->v.velocity[0]	= dir[0]*100.0f;
	cloud->v.velocity[1]	= dir[1]*100.0f;
	cloud->v.velocity[2]	= dir[2]*100.0f;
	cloud->v.dNextThink		= Server.dTime+0.3;
	cloud->v.think			= Midas_CloudThink;
	cloud->v.TouchFunction	= CloudTouch;

	Math_VectorAddValue(cloud->v.avelocity,300.0f,cloud->v.avelocity);

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.5;
	else
		ent->local.dAttackFinished = Server.dTime+1.0;

	//WEAPON_Animate(ent,FALSE,8,20,0.5,0,0,0,FALSE);
}
