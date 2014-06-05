/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "server_weapon.h"

/*
	The secret Ion Rifle. 
	Inspired by Talon Brave's weapon in Prey '98!
*/

#define	IONRIFLE_MAX_RANGE	2000.0f

#define	IONRIFLE_IDLEA_FRAME	0
#define	IONRIFLE_IDLEB_FRAME	30

// [17/11/2013] Animation for the primary attack ~hogsy
EntityFrame_t	efIonRiflePulseFireA[] =
{
	{	NULL,	10,	0.02f			},
	{	NULL,	11,	0.02f			},
	{	NULL,	12,	0.02f,	true	}
};

EntityFrame_t	efIonRiflePulseFireB[] =
{
	{	NULL,	13,	0.02f			},
	{	NULL,	14,	0.02f			},
	{	NULL,	15,	0.02f,	true	}
};

EntityFrame_t	efIonRiflePulseFireC[] =
{
	{	NULL,	16,	0.02f			},
	{	NULL,	17,	0.02f			},
	{	NULL,	18,	0.02f,	true	}
};

EntityFrame_t	efIonRiflePulseFireD[] =
{
	{	NULL,	19,	0.02f			},
	{	NULL,	20,	0.02f			},
	{	NULL,	21,	0.02f,	true	}
};

EntityFrame_t	efIonRiflePulseFireE[] =
{
	{	NULL,	22,	0.02f			},
	{	NULL,	23,	0.02f			},
	{	NULL,	24,	0.02f,	true	}
};

// [17/11/2013] Animation for switching to pulse mode (arms fold in) ~hogsy
EntityFrame_t	efIonRiflePulseMode[] = 
{
	{	NULL,	29,	0.03f			},
	{	NULL,	28,	0.03f			},
	{	NULL,	27,	0.03f			},
	{	NULL,	26,	0.03f			},
	{	NULL,	25,	0.03f			},
	{	NULL,	24,	0.03f,	true	}
};

EntityFrame_t	efIonRifleBlastFire[] = 
{
	{	NULL,	32,	0.03f			},
	{	NULL,	33,	0.03f			},
	{	NULL,	34,	0.03f			},
	{	NULL,	35,	0.03f			},
	{	NULL,	36,	0.03f,	true	}
};

// [17/11/2013] Should be just a reverse of the above (arms fold out) ~hogsy
EntityFrame_t	efIonRifleBlastMode[] = 
{
	{	NULL,	24,	0.03f			},
	{	NULL,	25,	0.03f			},
	{	NULL,	26,	0.03f			},
	{	NULL,	27,	0.03f			},
	{	NULL,	28,	0.03f			},
	{	NULL,	29,	0.03f,	true	}
};

EntityFrame_t	efIonRifleDeploy[] = 
{
	{	NULL,	1,	0.02f			},
	{	NULL,	2,	0.02f			},
	{	NULL,	3,	0.02f			},
	{	NULL,	4,	0.02f			},
	{	NULL,	5,	0.02f			},
	{	NULL,	6,	0.02f			},
	{	NULL,	7,	0.02f			},
	{	NULL,	8,	0.02f			},
	{	NULL,	9,	0.02f,	true	}
};

void IonRifle_IonBallTouch(edict_t *eIonBall,edict_t *eOther)
{
	if(eOther == eIonBall->local.eOwner)
		return;	// [29/1/2014] Should never even happen, but it does. Shit. ~hogsy
	else if(Engine.Server_PointContents(eIonBall->v.origin) == CONTENT_SKY)
	{
		Entity_Remove(eIonBall);
		return;
	}

	if(eOther->v.bTakeDamage)
	{
		if(!eIonBall->local.style)
			MONSTER_Damage(eOther,eIonBall->local.eOwner,25,0);
		else
			MONSTER_Damage(eOther,eIonBall->local.eOwner,50,0);
	}

	Entity_Remove(eIonBall);
}

/*	Do the deploy animation.
*/
void IonRifle_Deploy(edict_t *eOwner)
{
	// [29/1/2014] Remember to reset everything we're going to use ~hogsy
	eOwner->local.iBarrelCount = 0;

	WEAPON_Animate(eOwner,efIonRifleDeploy);
}

/*	Uses both primary burst and mega burst!
*/
void IonRifle_PrimaryAttack(edict_t *eOwner)
{
	switch(eOwner->local.iFireMode)
	{
	case 1:
		WEAPON_Animate(eOwner,efIonRifleBlastFire);

		eOwner->v.punchangle[0] -= (float)(((rand()%10)+5));

		eOwner->local.ionblaster_ammo -= 5;

		{
			edict_t	*eIonBall = Spawn();
			if(eIonBall)
			{
				vec3_t	vOrigin;

				eIonBall->v.cClassname		= "ionball";
				eIonBall->v.movetype		= MOVETYPE_FLY;
				eIonBall->v.effects			= EF_LIGHT_GREEN;
				eIonBall->v.TouchFunction	= IonRifle_IonBallTouch;

				eIonBall->Model.fScale = 2.0f;

				eIonBall->Physics.iSolid	= SOLID_BBOX;
				eIonBall->Physics.eIgnore	= eOwner;

				eIonBall->local.eOwner	= eOwner;
				eIonBall->local.style	= 1;		// [29/1/2014] Preserve our firemode ~hogsy

				Math_VectorCopy(eOwner->v.origin,vOrigin);

				vOrigin[2] += 25.0f;

				Entity_SetModel(eIonBall,"models/ionball.md2");
				Entity_SetSizeVector(eIonBall,vec3_origin,vec3_origin);
				Entity_SetOrigin(eIonBall,vOrigin);

				{
					vec_t	*vAim = Engine.Aim(eOwner);

					Math_VectorScale(vAim,IONRIFLE_MAX_RANGE,eIonBall->v.velocity);
					Math_VectorCopy(vAim,eIonBall->v.angles);
				}

				Engine.LinkEntity(eIonBall,false);
			}
		}

		if(eOwner->local.attackb_finished > Server.dTime)
			eOwner->local.dAttackFinished = Server.dTime+0.10;
		else
			eOwner->local.dAttackFinished = Server.dTime+0.5;
		break;
	default:	// Simple bursts
		Sound(eOwner,CHAN_AUTO,"weapons/laser.wav",255,ATTN_NORM);

		switch(eOwner->local.iBarrelCount)
		{
		case 0:
			WEAPON_Animate(eOwner,efIonRiflePulseFireA);
			break;
		case 1:
			WEAPON_Animate(eOwner,efIonRiflePulseFireB);
			break;
		case 2:
			WEAPON_Animate(eOwner,efIonRiflePulseFireC);
			break;
		case 3:
			WEAPON_Animate(eOwner,efIonRiflePulseFireD);
			break;
		case 4:
			WEAPON_Animate(eOwner,efIonRiflePulseFireE);
		}

		// [25/9/2013] Punch the view back ~hogsy
		eOwner->v.punchangle[0] -= (float)(((rand()%5)+1)/10.0f);

		eOwner->local.ionblaster_ammo--;

		// [29/1/2014] Let us cycle through each barrel on an individual basis ~hogsy
		eOwner->local.iBarrelCount++;
		if(eOwner->local.iBarrelCount >= 4)
			eOwner->local.iBarrelCount = 0;

		{
			edict_t	*eIonBall = Spawn();
			if(eIonBall)
			{
				vec3_t	vOrigin;

				eIonBall->v.cClassname		= "ionball";
				eIonBall->v.movetype		= MOVETYPE_FLY;
				eIonBall->v.effects			= EF_LIGHT_GREEN;
				eIonBall->v.TouchFunction	= IonRifle_IonBallTouch;

				eIonBall->Model.fScale	= 0.3f;

				eIonBall->Physics.iSolid	= SOLID_BBOX;
				eIonBall->Physics.eIgnore	= eOwner;

				eIonBall->local.eOwner	= eOwner;
				eIonBall->local.style	= 0;		// [29/1/2014] Preserve our firemode ~hogsy

				Math_VectorCopy(eOwner->v.origin,vOrigin);

				vOrigin[2] += 25.0f;

				Entity_SetModel(eIonBall,"models/ionball.md2");
				Entity_SetSizeVector(eIonBall,vec3_origin,vec3_origin);
				Entity_SetOrigin(eIonBall,vOrigin);

				{
					vec_t	*vAim = Engine.Aim(eOwner);

					Math_VectorScale(vAim,IONRIFLE_MAX_RANGE,eIonBall->v.velocity);
					Math_VectorCopy(vAim,eIonBall->v.avelocity);
				}

				Engine.LinkEntity(eIonBall,false);
			}
		}

		if(eOwner->local.attackb_finished > Server.dTime)
			eOwner->local.dAttackFinished = Server.dTime+0.5;
		else
			eOwner->local.dAttackFinished = Server.dTime+0.3;
	}

	// [17/11/2013] Update ammo counts... ~hogsy
	eOwner->v.iPrimaryAmmo = eOwner->local.ionblaster_ammo;
}

/*	Switch fire modes.
*/
void IonRifle_SecondaryAttack(edict_t *eOwner)
{
	if(eOwner->local.iFireMode)
	{
		WEAPON_Animate(eOwner,efIonRiflePulseMode);

		eOwner->local.iFireMode			=
		eOwner->local.iWeaponIdleFrame	= IONRIFLE_IDLEA_FRAME;
	}
	else
	{
		WEAPON_Animate(eOwner,efIonRifleBlastMode);

		eOwner->local.iFireMode			= 1;
		eOwner->local.iWeaponIdleFrame	= IONRIFLE_IDLEB_FRAME;
	}

	eOwner->local.dAttackFinished = Server.dTime+1.2;
}
