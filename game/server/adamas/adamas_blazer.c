/*	Copyright (C) 2014 OldTimes Software / Mark Elsworth Sowden
*/
#include "server_weapon.h"

void Blazer_Deploy(edict_t *eOwner)
{
}

void Blazer_PrimaryAttack(edict_t *eOwner)
{
	Engine.MakeVectors(eOwner->v.v_angle);
	Engine.Aim(eOwner);

	eOwner->v.effects |= EF_MUZZLEFLASH;

	Weapon_BulletProjectile(eOwner,1.0f,25,eOwner->v.vForward);

	eOwner->local.dAttackFinished = Server.dTime+0.3;

	eOwner->local.iBlazerAmmo--;
	eOwner->v.iPrimaryAmmo = eOwner->local.iBlazerAmmo;
}