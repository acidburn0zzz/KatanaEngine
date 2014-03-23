/*	Copyright (C) 2011-2014 OldTimes Software
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
    {   NULL, 8, 0.05f    },
    {   NULL, 9, 0.05f    },
    {   NULL, 10, 0.05f    },
    {   NULL, 11, 0.05f    },
    {   NULL, 12, 0.05f    },
    {   NULL, 13, 0.05f    },
    {   NULL, 14, 0.05f    },
    {   NULL, 15, 0.05f, true    }
};

EntityFrame_t ShotcyclerAnimation_Fire [] =
{
    {   NULL, 121, 0.035f			},
    {   NULL, 122, 0.035f			},
    {   NULL, 123, 0.035f			},
    {   NULL, 124, 0.035f			},
    {   NULL, 125, 0.035f			},
    {   NULL, 126, 0.035f			},
    {   Shotcycler_PrimaryAttack, 127, 0.035f			},
    {   NULL, 128, 0.035f			},
    {   NULL, 129, 0.035f			},
    {   NULL, 130, 0.035f			},
    {   NULL, 131, 0.035f			},
    {   NULL, 132, 0.035f			},
	{   NULL, 133, 0.035f			},
    {   NULL, 134, 0.035f			},
    {   NULL, 135, 0.035f			},
	{   NULL, 136, 0.035f			},
    {   NULL, 137, 0.035f			},
    {   NULL, 138, 0.035f			},
	{   NULL, 139, 0.035f			},
	{	NULL, 140, 0.035f, true   }
};

EntityFrame_t ShotcyclerAnimation_Reload [] =
{
	{   NULL, 24, 0.02f			   },
	{   NULL, 25, 0.02f            },
	{   NULL, 26, 0.02f            },
	{   NULL, 27, 0.02f            },
	{   NULL, 28, 0.02f            },
	{   NULL, 29, 0.02f            },
	{   NULL, 30, 0.02f            },
	{   NULL, 31, 0.02f            },
	{   Shotcycler_SoundPull, 32, 0.02f},
	{   NULL, 33, 0.02f            },
	{   NULL, 34, 0.02f            },
	{   NULL, 35, 0.02f            },
	{   NULL, 36, 0.02f            },
	{   NULL, 37, 0.02f            },
	{   NULL, 38, 0.02f            },
	{   NULL, 39, 0.02f            },
	{   NULL, 40, 0.02f            },
	{   NULL, 41, 0.02f            },
	{   NULL, 42, 0.02f			   },
	{   NULL, 43, 0.02f            },
	{   NULL, 44, 0.02f            },
	{   NULL, 45, 0.02f            },
	{   NULL, 46, 0.02f            },
	{   NULL, 47, 0.02f            },
	{   NULL, 48, 0.02f            },
	{   NULL, 49, 0.02f            },
	{   NULL, 50, 0.02f            },
	{   NULL, 51, 0.02f            },
	{   NULL, 52, 0.02f            },
	{   NULL, 53, 0.02f            },
	{   NULL, 54, 0.02f            },
	{   NULL, 55, 0.02f            },
	{   NULL, 56, 0.02f            },
	{   NULL, 57, 0.02f            },
	{   NULL, 58, 0.02f            },
	{   NULL, 59, 0.02f            },
	{   NULL, 60, 0.02f            },
	{   NULL, 61, 0.02f            },
	{   NULL, 62, 0.02f            },
	{   NULL, 63, 0.02f            },
	{   NULL, 64, 0.02f            },
	{   NULL, 65, 0.02f            },
	{   NULL, 66, 0.02f            },
	{   NULL, 67, 0.02f            },
	{   NULL, 68, 0.02f            },
	{   NULL, 69, 0.02f            },
	{   NULL, 70, 0.02f            },
	{   NULL, 71, 0.02f            },
	{   NULL, 72, 0.02f            },
	{   NULL, 73, 0.02f            },
	{   NULL, 74, 0.02f            },
	{   NULL, 75, 0.02f            },
	{   NULL, 76, 0.02f            },
	{   NULL, 77, 0.02f            },
	{   NULL, 78, 0.02f            },
	{   NULL, 79, 0.02f            },
	{   NULL, 80, 0.02f            },
	{   NULL, 81, 0.02f            },
	{   NULL, 82, 0.02f            },
	{   NULL, 83, 0.02f            },
	{   NULL, 84, 0.02f            },
	{   NULL, 85, 0.02f            },
	{   NULL, 86, 0.02f            },
	{   NULL, 87, 0.02f            },
	{   NULL, 88, 0.02f            },
	{   Shotcycler_SoundClick, 89, 0.02f},
	{   NULL, 90, 0.02f            },
	{   NULL, 91, 0.02f            },
	{   NULL, 92, 0.02f            },
	{   NULL, 93, 0.02f            },
	{   NULL, 94, 0.02f            },
	{   NULL, 95, 0.02f            },
	{   NULL, 96, 0.02f            },
	{   NULL, 97, 0.02f            },
	{   NULL, 98, 0.02f            },
	{   Shotcycler_SoundSlap, 99, 0.02f            },
	{   NULL, 100, 0.02f            },
	{   NULL, 101, 0.02f            },
	{   NULL, 102, 0.02f            },
	{   NULL, 103, 0.02f            },
	{   NULL, 104, 0.02f            },
	{   NULL, 105, 0.02f            },
	{   NULL, 106, 0.02f            },
	{   NULL, 107, 0.02f            },
	{   NULL, 108, 0.02f            },
	{   NULL, 109, 0.02f            },
	{   NULL, 110, 0.02f            },
	{   NULL, 111, 0.02f            },
	{   NULL, 112, 0.02f            },
	{   NULL, 113, 0.02f            },
	{   NULL, 114, 0.02f            },
	{   NULL, 115, 0.02f            },
	{   NULL, 116, 0.02f            },
	{   NULL, 117, 0.02f            },
	{   NULL, 118, 0.02f            },
	{   NULL, 119, 0.02f            },
	{	NULL, 120, 0.02f, true		}
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

	WEAPON_Animate(ent,ShotcyclerAnimation_Deploy);
}

void Shotcycler_Reload(edict_t *ent)
{
	ent->local.iShotCycle = 6;

	WEAPON_Animate(ent,ShotcyclerAnimation_Reload);

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

	WEAPON_Animate(ent,ShotcyclerAnimation_Fire);
}
