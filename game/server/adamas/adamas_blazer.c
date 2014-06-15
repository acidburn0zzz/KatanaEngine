/*	Copyright (C) 2014 OldTimes Software
*/
#include "server_weapon.h"

EntityFrame_t efBlazerDeploy[]=
{
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	0,	0.02f			},
	{	NULL,	13,	0.02f,	true	}
};

void Blazer_Deploy(edict_t *eOwner)
{
	Weapon_Animate(eOwner,efBlazerDeploy);
}

void Blazer_PrimaryAttack(edict_t *eOwner)
{
	eOwner->local.dAttackFinished = Server.dTime+0.3;

	eOwner->local.iBlazerAmmo--;
	eOwner->v.iPrimaryAmmo = eOwner->local.iBlazerAmmo;
}