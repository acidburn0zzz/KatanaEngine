/*	Copyright (C) 2014 OldTimes Software / Mark Elsworth Sowden
*/
#include "server_weapon.h"

void Blazer_Deploy(edict_t *eOwner)
{
}

void Blazer_PrimaryAttack(edict_t *eOwner)
{
	vec3_t	vForward,vTemp;

	Math_AngleVectors(eOwner->v.v_angle,vForward,vTemp,vTemp);

	eOwner->v.effects |= EF_MUZZLEFLASH;

	Weapon_BulletProjectile(eOwner,0.5f,25,vForward);

	eOwner->local.dAttackFinished = Server.dTime+0.3;

	eOwner->local.iBulletAmmo--;
	eOwner->v.iPrimaryAmmo = eOwner->local.iBulletAmmo;
}