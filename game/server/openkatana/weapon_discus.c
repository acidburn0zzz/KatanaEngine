#include "server_main.h"

void Discus_Deploy(edict_t *ent)
{
	//Weapon_Animate(ent,FALSE,1,10,0.1f,0,0,0,FALSE);
}

void Discus_Catch(edict_t *ent, edict_t *discus)
{
	// [4/7/2012] Simplified ~hogsy
	ent->v.iPrimaryAmmo = ent->local.discus_ammo2++;

	Sound(ent,CHAN_WEAPON,"weapons/discus/discuscatch.wav",255,ATTN_NORM);

	Entity_Remove(discus);
}

void Discus_Follow(edict_t *ent)
{
	vec3_t	vtemp;

	// [23/5/2012] Quick fix for possible issue ~hogsy
	if(!ent || !ent->v.enemy)
		return;

	vtemp[0] = ent->v.enemy->v.origin[0]-ent->v.origin[0];
	vtemp[1] = ent->v.enemy->v.origin[1]-ent->v.origin[1];
	vtemp[2] = ent->v.enemy->v.origin[2]+22.0f-ent->v.origin[2];

	Math_VectorNormalize(vtemp);
	ent->v.velocity[0] = vtemp[0]*3000.0f;
	ent->v.velocity[1] = vtemp[1]*3000.0f;
	ent->v.velocity[2] = vtemp[2]*3000.0f;

	// [25/6/2012] Simplified ~hogsy
	Math_MVToVector(Math_VectorToAngles(ent->v.velocity),ent->v.angles);

	ent->v.think		= Discus_Follow;
	ent->v.dNextThink	= Server.dTime+0.01;
}

// [4/7/2012] Renamed to Discus_ProjectileTouch ~hogsy
void Discus_ProjectileTouch(edict_t *ent,edict_t *other)
{
	char	snd[64];
	vec3_t	vel;

	// [4/8/2012] Updated to use owner ~hogsy
	if(other == ent->local.eOwner)
	{
		// Don't touch the owner while being thrown
		if(ent->local.hit == 0)
			return;

		Discus_Catch(other,ent);
		return;
	}

	if(ent->local.hit == 2)
		ent->local.hit = 1;
	else if(ent->local.hit == 1)
		ent->local.hit = 2;
	else
		ent->local.hit = 1;

	if(other->v.bTakeDamage)
	{
		MONSTER_Damage(other,ent,35,0);

		Sound(ent,CHAN_BODY,"weapons/discus/discushit.wav",255,ATTN_NORM);

		return;
	}

	// [23/5/2012] Cleaned up ~hogsy
	sprintf(snd,"weapons/discus/discusclang%i.wav",rand()%5+1);
	Sound(ent,CHAN_BODY,snd,255,ATTN_NORM);

	vel[0] = vel[1] = 0;
	vel[2] = 5.0f;
	Engine.Particle(ent->v.origin,vel,4.0f,"smoke",15);

	// [4/7/2012] Simplified ~hogsy
	ent->v.angles[2] -= 180.0f;

	if(ent->local.hit == 1)
	{
		ent->v.think		= Discus_Follow;
		ent->v.dNextThink	= Server.dTime+0.01;
	}
	else
		ent->v.think		= NULL;
}

// [4/7/2012] Renamed to Discus_SpawnProjectile ~hogsy
void Discus_SpawnProjectile(edict_t *ent,vec3_t org)
{
	edict_t *discus	= Entity_Spawn();
	float	*dir;

	discus->v.cClassname	= "discus";
	discus->v.movetype		= MOVETYPE_FLYBOUNCE;
	discus->Physics.iSolid	= SOLID_BBOX;
	discus->v.effects		= EF_MOTION_ROTATE;
	discus->v.enemy			= ent;

	// [4/8/2012] Updated to use owner instead ~hogsy
	discus->local.eOwner	= ent;

	discus->local.hit	= 0;

	dir = Engine.Aim(ent);
	discus->v.velocity[0] = dir[0]*700;
	discus->v.velocity[1] = dir[1]*700;
	discus->v.velocity[2] = dir[2]*700;

	Engine.MakeVectors(discus->v.v_angle);

	Math_MVToVector(Math_VectorToAngles(discus->v.velocity),discus->v.angles);

	Entity_SetModel(discus,"models/w_daedalus.md2");
	Entity_SetSizeVector(discus,vec3_origin,vec3_origin);
	Entity_SetOrigin(discus, org);

	Engine.LinkEntity(discus,false);

	discus->v.TouchFunction = Discus_ProjectileTouch;

	Sound(ent,CHAN_WEAPON,"weapons/discus/discusthrow.wav",255,ATTN_NORM);
}

// [4/7/2012] Renamed to Discus_PrimaryAttack ~hogsy
void Discus_PrimaryAttack(edict_t *ent)
{
	//Weapon_Animate(ent,FALSE,8,24,0.035f,0,0,0,FALSE);

	Discus_SpawnProjectile(ent,ent->v.origin);

	// [4/7/2012] Simplified ~hogsy
	ent->v.iPrimaryAmmo = ent->local.discus_ammo2--;

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.35;
	else
		ent->local.dAttackFinished = Server.dTime+0.75;
}
