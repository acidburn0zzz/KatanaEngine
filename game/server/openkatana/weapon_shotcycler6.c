/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "server_weapon.h"

/*
	Episode One's shotgun
*/

#include "server_player.h"

void Shotcycler_SoundPull(edict_t *eShotcycler);
void Shotcycler_SoundClick(edict_t *eShotcycler);
void Shotcycler_SoundSlap(edict_t *eShotcycler);

EntityFrame_t ShotcyclerAnimation_Deploy [] =
{
    {   NULL, 1, 0.05f    },
    {   NULL, 2, 0.05f    },
    {   NULL, 3, 0.05f    },
    {   NULL, 4, 0.05f    },
    {   NULL, 5, 0.05f    },
    {   NULL, 6, 0.05f    },
    {   NULL, 7, 0.05f    },
    {   NULL, 8, 0.05f, true    }
};

EntityFrame_t ShotcyclerAnimation_Fire [] =
{
    {   NULL, 8, 0.03f	},
    {   NULL, 9, 0.08f	},
    {   NULL, 10, 0.08f			},
    {   NULL, 11, 0.08f			},
    {	Shotcycler_PrimaryAttack, 12, 0.08f, true   }
};

EntityFrame_t ShotcyclerAnimation_Reload [] =
{
	{   NULL, 21, 0.05f},
	{   NULL, 22, 0.05f},
	{   NULL, 23, 0.05f},
	{   NULL, 24, 0.05f            },
	{   NULL, 25, 0.05f            },
	{   NULL, 26, 0.05f            },
	{   NULL, 27, 0.05f            },
	{   NULL, 28, 0.05f            },
	{   NULL, 29, 0.05f},
	{   NULL, 30, 0.05f            },
	{   NULL, 31, 0.05f            },
	{   NULL, 32, 0.05f            },
	{   NULL, 33, 0.05f            },
	{   NULL, 34, 0.05f            },
	{   NULL, 35, 0.05f            },
	{   NULL, 36, 0.05f            },
	{   NULL, 37, 0.05f            },
	{   NULL, 29, 0.05f},
	{   NULL, 30, 0.05f            },
	{   NULL, 31, 0.05f            },
	{   NULL, 32, 0.05f            },
	{   NULL, 33, 0.05f            },
	{   NULL, 34, 0.05f            },
	{   NULL, 35, 0.05f            },
	{   NULL, 36, 0.05f            },
	{   NULL, 37, 0.05f            },
	{   NULL, 29, 0.05f},
	{   NULL, 30, 0.05f            },
	{   NULL, 31, 0.05f            },
	{   NULL, 32, 0.05f            },
	{   NULL, 33, 0.05f            },
	{   NULL, 34, 0.05f            },
	{   NULL, 35, 0.05f            },
	{   NULL, 36, 0.05f            },
	{   NULL, 37, 0.05f            },
	{   NULL, 29, 0.05f},
	{   NULL, 30, 0.05f            },
	{   NULL, 31, 0.05f            },
	{   NULL, 32, 0.05f            },
	{   NULL, 33, 0.05f            },
	{   NULL, 34, 0.05f            },
	{   NULL, 35, 0.05f            },
	{   NULL, 36, 0.05f            },
	{   NULL, 37, 0.05f            },
	{   NULL, 29, 0.05f},
	{   NULL, 30, 0.05f            },
	{   NULL, 31, 0.05f            },
	{   NULL, 32, 0.05f            },
	{   NULL, 33, 0.05f            },
	{   NULL, 34, 0.05f            },
	{   NULL, 35, 0.05f            },
	{   NULL, 36, 0.05f            },
	{   NULL, 37, 0.05f            },
	{   NULL, 29, 0.05f},
	{   NULL, 30, 0.05f            },
	{   NULL, 31, 0.05f            },
	{   NULL, 32, 0.05f            },
	{   NULL, 33, 0.05f            },
	{   NULL, 34, 0.05f            },
	{   NULL, 35, 0.05f            },
	{   NULL, 36, 0.05f            },
	{   NULL, 37, 0.05f            },
	{   NULL, 29, 0.05f},
	{   NULL, 30, 0.05f            },
	{   NULL, 31, 0.05f            },
	{   NULL, 32, 0.05f            },
	{   NULL, 33, 0.05f            },
	{   NULL, 34, 0.05f            },
	{   NULL, 35, 0.05f            },
	{   NULL, 36, 0.05f            },
	{   NULL, 37, 0.05f            },
	{   NULL, 38, 0.05f},
	{   NULL, 39, 0.05f            },
	{   NULL, 40, 0.05f            },
	{   NULL, 41, 0.05f            },
	{   NULL, 42, 0.05f            },
	{   NULL, 43, 0.05f            },
	{   NULL, 44, 0.05f            },
	{   NULL, 45, 0.05f            },
	{   NULL, 46, 0.05f            },
	{   NULL, 47, 0.05f},
	{   NULL, 48, 0.05f            },
	{   NULL, 49, 0.05f            },
	{   NULL, 50, 0.05f            },
	{   NULL, 51, 0.05f            },
	{   NULL, 52, 0.05f            },
	{   NULL, 53, 0.05f            },
	{   NULL, 54, 0.05f            },
	{   NULL, 55, 0.05f            },
	{   NULL, 56, 0.05f            },
	{   NULL, 57, 0.05f            },
	{   NULL, 58, 0.05f            },
	{   NULL, 59, 0.05f            },
	{   NULL, 60, 0.05f,	      },
	{   NULL, 61, 0.05f, true	}
};

void Shotcycler_SoundClick(edict_t *eShotcycler)
{
	Sound(eShotcycler,CHAN_WEAPON,"weapons/shotcycler6/shotclick.wav",255,ATTN_NORM);
}

void Shotcycler_SoundPull(edict_t *eShotcycler)
{
	Sound(eShotcycler,CHAN_WEAPON,"weapons/shotcycler6/shotpull.wav",255,ATTN_NORM);
}

void Shotcycler_SoundSlap(edict_t *eShotcycler)
{
	Sound(eShotcycler,CHAN_WEAPON,"weapons/shotcycler6/shotslap.wav",255,ATTN_NORM);
}

void Shotcycler_Deploy(edict_t *ent)
{
	ent->local.iShotCycle = 6;

	// [24/8/2012] TODO: Call our deploy sound ~hogsy
	Sound(ent,CHAN_WEAPON,"weapons/shotcycler6/shotcyclerready.wav",255,ATTN_NORM);

	Weapon_Animate(ent,ShotcyclerAnimation_Deploy);
}

void Shotcycler_Reload(edict_t *ent)
{
	ent->local.iShotCycle = 6;

	Weapon_Animate(ent,ShotcyclerAnimation_Reload);

	ent->local.dAttackFinished = Server.dTime+2;
}

void Shotcycler_PrimaryAttack(edict_t *ent)
{
	vec3_t	vForward,vTemp;

	Math_AngleVectors(ent->v.v_angle,vForward,vTemp,vTemp);

	if(!ent->local.iShotCycle)
	{
		Shotcycler_Reload(ent);
		return;
	}
	else if(!ent->local.shotcycler_ammo)
	{
		Sound(ent,CHAN_WEAPON,"weapons/shotcycler6/shotcyclerclick.wav",255,ATTN_NORM);
		return;
	}

	Sound(ent,CHAN_WEAPON,va("weapons/shotcycler6/shotcyclerfire%i.wav",rand()%9+1),255,ATTN_NORM);

	ent->v.punchangle[PITCH]	-= (float)(rand()%5+1);
	ent->v.effects				|= EF_MUZZLEFLASH;
	// [8/8/2013] Fixed ~hogsy
	ent->v.iPrimaryAmmo			= ent->local.shotcycler_ammo -= 2;

	// [24/8/2012] Simplified ~hogsy
	ent->local.iShotCycle--;

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+ 2.5f;
	else
		ent->local.dAttackFinished = Server.dTime+ 5.0f;


#if 0
    // [25/8/2012] TODO: Obviously this isn't how it really works, should shoot all shots one after the other instead ~hogsy
    {
        int i;

        for(i = 0; i < 4; i++)
            Weapon_BulletProjectile(ent,15.0f,5, vForward);
    }
#else // [8/8/2013] Shouldn't it work more like this? ~hogsy
	Weapon_BulletProjectile(ent,5.0f,5,vForward);
#endif

	Weapon_Animate(ent,ShotcyclerAnimation_Fire);
}
