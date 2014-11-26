/*  Copyright (C) 2011-2015 OldTimes Software
*/
#include "server_weapon.h"

// [20/5/2013] TODO: Move into game_resources.h ~hogsy
#define GLOCK_SOUND_NOAMMO	"weapons/glock/glockclick.wav"
#define GLOCK_SOUND_FIRE	"weapons/glock/glockfire.wav"
#define GLOCK_SOUND_RELOAD1	"weapons/glock/glockload.wav"
#define GLOCK_SOUND_RELOAD2	"weapons/glock/glockload2.wav";

void Glock_Deploy(edict_t *ent)
{
	//Weapon_Animate(ent,FALSE,46,74,0.1f,0,0,0,FALSE);
}

void Glock_Reload(edict_t *ent)
{
	int		rs;
	char	*noise;

	if(ent->local.glock_ammo2 != 20)
	{
		// [20/5/2012] Fixed! ~hogsy
		rs = rand()%2;
		if (rs == 1)
			// [4/7/2012] TODO: Rename to glockload1.wav ~hogsy
			noise = GLOCK_SOUND_RELOAD1;
		else
			noise = GLOCK_SOUND_RELOAD2;

		Sound(ent,CHAN_WEAPON,noise,255,ATTN_NORM);

		//Weapon_Animate(ent,FALSE,30,74,0.05f,0,0,0,FALSE);

		ent->local.glock_ammo2		= 20;
		ent->local.dAttackFinished	= Server.dTime+2.6;
	}
}

// [4/7/2012] Renamed to Glock_PrimaryAttack ~hogsy
void Glock_PrimaryAttack(edict_t *ent)
{
	Engine.MakeVectors(ent->v.v_angle);
	Engine.Aim(ent);

	if(!ent->local.glock_ammo2)
	{
		Sound(ent,CHAN_WEAPON,GLOCK_SOUND_NOAMMO,255,ATTN_NORM);

		Glock_Reload(ent);

		return;
	}

	Sound(ent,CHAN_WEAPON,GLOCK_SOUND_FIRE,255,ATTN_NORM);

#if 0
	if(rand()%2 == 1)
		//Weapon_Animate(ent,FALSE,0,15,0.03f,10,19,0,FALSE);
	else
		//Weapon_Animate(ent,FALSE,16,27,0.03f,10,19,0,FALSE);
#endif

	// [25/8/2012] Simplified ~hogsy
	ent->v.effects |= EF_MUZZLEFLASH;

	ent->local.glock_ammo--;
	ent->v.iPrimaryAmmo	= ent->local.glock_ammo;

	Weapon_BulletProjectile(ent,2.0f,15,ent->v.vForward);

	// [4/7/2012] Simplified ~hogsy
	ent->local.glock_ammo2--;

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.2;
	else
		ent->local.dAttackFinished = Server.dTime+0.35;
}
