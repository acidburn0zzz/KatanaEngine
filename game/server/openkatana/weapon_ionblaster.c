/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_weapon.h"

/*
	The Ion Blaster. Hiro's starter weapon.
*/

#include "server_player.h"

#define IONBLASTER_MAX_RANGE	2000

EntityFrame_t IonBlasterAnimation_Deploy [] =
{
	{   NULL, 1, 0.02f    },
	{   NULL, 2, 0.02f    },
	{   NULL, 3, 0.02f    },
	{   NULL, 4, 0.02f    },
	{   NULL, 5, 0.02f    },
	{   NULL, 6, 0.02f    },
	{   NULL, 7, 0.02f    },
	{   NULL, 8, 0.02f    },
	{   NULL, 9, 0.02f    },
	{   NULL, 10, 0.02f	  },
	{   NULL, 11, 0.02f    },
	{   NULL, 12, 0.02f    },
	{   NULL, 13, 0.02f    },
	{   NULL, 14, 0.02f    },
	{   NULL, 15, 0.02f    },
	{   NULL, 16, 0.02f	   },
	{   NULL, 17, 0.02f    },
	{   NULL, 18, 0.02f    },
	{   NULL, 19, 0.02f    },
	{   NULL, 20, 0.02f    },
	{   NULL, 21, 0.02f    },
	{   NULL, 22, 0.02f    },
	{   NULL, 23, 0.02f    },
	{   NULL, 24, 0.02f    },
	{   NULL, 25, 0.02f    },
	{   NULL, 26, 0.02f, TRUE    }
};

EntityFrame_t IonBlasterAnimation_Fire1 [] =
{
	{   NULL, 27, 0.02f    },
	{   NULL, 28, 0.02f    },
	{   NULL, 29, 0.02f    },
	{   NULL, 30, 0.02f    },
	{   NULL, 31, 0.02f    },
	{   NULL, 32, 0.02f    },
	{   NULL, 33, 0.02f    },
	{   NULL, 34, 0.02f    },
	{   NULL, 35, 0.02f    },
	{   NULL, 36, 0.02f    },
	{   NULL, 37, 0.02f, TRUE    }
};

EntityFrame_t IonBlasterAnimation_Fire2 [] =
{
	{   NULL, 38, 0.02f    },
	{   NULL, 39, 0.02f    },
	{   NULL, 40, 0.02f    },
	{   NULL, 41, 0.02f    },
	{   NULL, 42, 0.02f    },
	{   NULL, 43, 0.02f    },
	{   NULL, 44, 0.02f    },
	{   NULL, 45, 0.02f    },
	{   NULL, 46, 0.02f    },
	{   NULL, 47, 0.02f    },
	{   NULL, 48, 0.02f    },
	{   NULL, 49, 0.02f    },
	{   NULL, 50, 0.02f    },
	{   NULL, 51, 0.02f, TRUE    }
};

void IonBlaster_Deploy(edict_t *ent)
{
	Sound(ent,CHAN_WEAPON,"weapons/ionblaster/ionready.wav",255,ATTN_NORM);

	WEAPON_Animate(ent,IonBlasterAnimation_Deploy);
}

void IonBlaster_IonBallExplode(edict_t *ent)
{
	vec3_t vel;

	// [18/5/2013] This is was the wrong way, fixed now! ~hogsy
	Math_VectorCopy(ent->v.velocity,vel);
	Math_VectorInverse(vel);

	Sound(ent,CHAN_ITEM,"weapons/ionblaster/ionexplode3.wav",255,ATTN_NORM);

	Engine.Particle(ent->v.origin,vel,3.0f,"spark2",20);

	Entity_Remove(ent);
}

void IonBlaster_IonBallTouch(edict_t *eIonBall,edict_t *other)
{
	// [26/2/2012] Revised and fixed ~hogsy
	// [25/6/2012] Revised ~hogsy
	// [9/12/2012] Revised and fixed ~hogsy
	vec3_t	vInversed;

	// [14/2/2013] Moved ~hogsy
	if(Engine.Server_PointContents(eIonBall->v.origin) == CONTENT_SKY)
	{
		Entity_Remove(eIonBall);
		return;
	}

	// [12/8/2012] Don't collide with owner ~hogsy
	// [26/8/2012] Don't collide on at least the first hit with the owner (otherwise we'll kill them upon fire) ~hogsy
	if((other == eIonBall->local.eOwner) && !eIonBall->local.hit)
		return;
	else
		eIonBall->local.hit = true;

	Math_MVToVector(Math_VectorToAngles(eIonBall->v.velocity),eIonBall->v.angles);
	Math_VectorCopy(eIonBall->v.velocity,vInversed);
	Math_VectorInverse(vInversed);

	if(eIonBall->local.hit)
	{
		if(other->v.bTakeDamage && other != eIonBall->local.eOwner)
			// [9/12/2012] IonBall just explodes if selfharm is false ~hogsy
			// [11/1/2013] Revised ~hogsy
			MONSTER_Damage(other,eIonBall,15);
		else
		{
			if(!eIonBall->v.angles[1] || !eIonBall->v.angles[0])
				eIonBall->v.velocity[2] *= 200.0f;

			eIonBall->v.angles[1] -= 180.0f;

			Sound(eIonBall,CHAN_ITEM,"weapons/ionblaster/ionwallbounce.wav",255,ATTN_NORM);

			Engine.Particle(eIonBall->v.origin,vInversed,5.0f,"spark2",25);

			// [26/8/2012] TODO: Keep the direction we're facing updated! ~hogsy

			return;
		}
	}

//	Sound(ent,CHAN_ITEM,"weapons/ionblaster/ionexplode3.wav",255,ATTN_NORM);

	Engine.Particle(eIonBall->v.origin,vInversed,5.0f,"spark2",17);

	Entity_Remove(eIonBall);
}

void IonBlaster_PrimaryAttack(edict_t *ent)
{
	// [26/2/2012] Revised and fixed ~hogsy
	float	*dir;
	edict_t *eIonBall;
	vec3_t	orig;

	Sound(ent,CHAN_WEAPON,"weapons/ionblaster/ionfire.wav",255,ATTN_NORM);

	ent->v.punchangle[0] -= (float)(rand()%5+2);

	if(rand()%2 == 1)
		WEAPON_Animate(ent,IonBlasterAnimation_Fire1);
	else
		WEAPON_Animate(ent,IonBlasterAnimation_Fire2);

	// This fixes the ammo bug ~eukos [05/08/2013]
	ent->local.ionblaster_ammo--;
	ent->v.iPrimaryAmmo = ent->local.ionblaster_ammo;

	eIonBall = Spawn();
	if(eIonBall)
	{
		eIonBall->v.cClassname	= "ionball";
		eIonBall->v.movetype	= MOVETYPE_FLYBOUNCE;
		eIonBall->v.effects		= EF_LIGHT_GREEN;

		eIonBall->Physics.iSolid	= SOLID_BBOX;

		eIonBall->local.hit		= false;
		eIonBall->local.eOwner	= ent;

		dir = Engine.Aim(ent);
		// [26/8/2012] Simplified ~hogsy
		Math_VectorScale(dir,IONBLASTER_MAX_RANGE,eIonBall->v.velocity);
		// [26/8/2012] Start us off facing in the direction we're being fired from ~hogsy
		Math_VectorCopy(dir,eIonBall->v.avelocity);

		Engine.MakeVectors(eIonBall->v.v_angle);

		Math_MVToVector(Math_VectorToAngles(eIonBall->v.velocity),eIonBall->v.angles);
		Math_VectorCopy(ent->v.origin,orig);

		orig[2] += 25.0f;

		Entity_SetModel(eIonBall,"models/ionball.md2");
		Entity_SetSizeVector(eIonBall,vec3_origin,vec3_origin);
		Entity_SetOrigin(eIonBall,orig);

		eIonBall->v.TouchFunction	= IonBlaster_IonBallTouch;
		eIonBall->v.dNextThink		= Server.dTime+3.0;
		eIonBall->v.think			= IonBlaster_IonBallExplode;

		Engine.LinkEntity(eIonBall,false);
	}

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.25;
	else
		ent->local.dAttackFinished = Server.dTime+0.5;
}
