/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_monster.h"

/*
	The Inmater
	A horrid robot that lurks around the prisons keeping
	those locked inside in line and under control.
*/

EntityFrame_t efInmaterIdle[]=
{
	{	NULL,	1,	0.1f			},
	{	NULL,	1,	0.1f			},
	{	NULL,	2,	0.1f			},
	{	NULL,	3,	0.1f			},
	{	NULL,	4,	0.1f			},
	{	NULL,	5,	0.1f			},
	{	NULL,	6,	0.1f			},
	{	NULL,	7,	0.1f			},
	{	NULL,	8,	0.1f			},
	{	NULL,	9,	0.1f			},
	{	NULL,	10,	0.1f			},
	{	NULL,	11,	0.1f			},
	{	NULL,	12,	0.1f			},
	{	NULL,	13,	0.1f			},
	{	NULL,	14,	0.1f			},
	{	NULL,	15,	0.1f			},
	{	NULL,	16,	0.1f			},
	{	NULL,	17,	0.1f			},
	{	NULL,	18,	0.1f			},
	{	NULL,	19,	0.1f			},
	{	NULL,	20,	0.1f			},
	{	NULL,	21,	0.1f			},
	{	NULL,	22,	0.1f			},
	{	NULL,	23,	0.1f,	true	}
};

EntityFrame_t efInmaterDeath[]=
{
	{	NULL,	148,	0.1f			},
	{	NULL,	149,	0.1f			},
	{	NULL,	150,	0.1f			},
	{	NULL,	151,	0.1f			},
	{	NULL,	152,	0.1f			},
	{	NULL,	153,	0.1f			},
	{	NULL,	154,	0.1f			},
	{	NULL,	155,	0.1f			},
	{	NULL,	156,	0.1f			},
	{	NULL,	157,	0.1f			},
	{	NULL,	158,	0.1f			},
	{	NULL,	159,	0.1f			},
	{	NULL,	160,	0.1f			},
	{	NULL,	161,	0.1f			},
	{	NULL,	162,	0.1f			},
	{	NULL,	163,	0.1f			},
	{	NULL,	164,	0.1f			},
	{	NULL,	165,	0.1f			},
	{	NULL,	166,	0.1f			},
	{	NULL,	167,	0.1f,	true	}
};

#define INMATER_MAX_HEALTH	150
#define	INMATOR_MAX_BOREDOM	35
#define	INMATOR_MIN_HEALTH	-35

void Inmater_Pain(edict_t *eInmater,edict_t *eOther)
{
	if(eInmater->monster.iState == STATE_ASLEEP)
		Monster_SetState(eInmater,STATE_AWAKE);
}

void Inmater_Die(edict_t *eInmater,edict_t *eOther)
{
	if(eInmater->v.iHealth < INMATOR_MIN_HEALTH)
	{
		int	iGibs = (rand()%5)+5,
			i;

		Sound(eInmater,CHAN_VOICE,"misc/gib1.wav",255,ATTN_NORM);

		// [13/9/2012] Updated paths ~hogsy
		for(i = 0; i < iGibs; i++)
		{
			int		j;
			vec3_t	vOrigin,vGibVelocity;

			for(j = 0; j < 3; j++)
				vOrigin[j]	= eInmater->v.origin[j]+((eInmater->v.mins[j]+eInmater->v.maxs[j])*(rand()%10));

			// [26/2/2014] Add some random velocity on top of the velocity that the inmater already has ~hogsy
			Math_VectorCopy(eInmater->v.velocity,vGibVelocity);
			Math_VectorAddValue(vGibVelocity,(float)(rand()%5),vGibVelocity);

			ThrowGib(vOrigin,vGibVelocity,va("models/gibs/metal_gibs%i.md2",rand()%3+1),(float)eInmater->v.iHealth*-1,true);
		}

		Engine.Particle(eInmater->v.origin,eInmater->v.velocity,10.0f,"blood",20);

		ENTITY_REMOVE(eInmater);
	}

	Entity_Animate(eInmater,efInmaterDeath);

	Monster_SetState(eInmater,STATE_DEAD);
}

void Inmater_Think(edict_t *eInmater)
{
	switch(eInmater->monster.iState)
	{
	case STATE_ASLEEP:
		if(eInmater->monster.fEmotion[EMOTION_BOREDOM] > INMATOR_MAX_BOREDOM)
		{
			Monster_SetState(eInmater,STATE_AWAKE);
			Monster_SetThink(eInmater,THINK_WANDERING);
			return;
		}
		break;
	case STATE_AWAKE:
		switch(eInmater->monster.iThink)
		{
		case THINK_IDLE:
			if(!eInmater->local.dAnimationTime || (eInmater->local.iAnimationCurrent == eInmater->local.iAnimationEnd))
				Entity_Animate(eInmater,efInmaterIdle);
			break;
		}
		break;
	case STATE_DEAD:
		break;
	default:
		Monster_SetState(eInmater,STATE_ASLEEP);
	}
}

void Inmater_Spawn(edict_t *eInmater)
{
	Engine.Server_PrecacheResource(RESOURCE_MODEL,INMATER_MODEL_BODY);

	eInmater->Physics.iSolid	= SOLID_SLIDEBOX;
	eInmater->Physics.fGravity	= cvServerGravity.value;
	eInmater->Physics.fMass		= 4.0f;

	eInmater->monster.iType			= MONSTER_INMATER;
	eInmater->monster.Think			= Inmater_Think;
	eInmater->monster.think_pain	= Inmater_Pain;
	eInmater->monster.think_die		= Inmater_Die;

	eInmater->v.bTakeDamage	= true;
	eInmater->v.movetype	= MOVETYPE_STEP;
	eInmater->v.iHealth		= INMATER_MAX_HEALTH;
	eInmater->v.iMaxHealth	= INMATER_MAX_HEALTH;
	eInmater->v.netname		= "Inmater";
	eInmater->v.frame		= 0;

	Monster_SetState(eInmater,STATE_AWAKE);
	Monster_SetThink(eInmater,THINK_IDLE);

	Entity_SetModel(eInmater,INMATER_MODEL_BODY);
	Entity_SetSize(eInmater,-16.0f,-16.0f,-24.0f,16.0f,16.0f,32.0f);
	Entity_SetOrigin(eInmater,eInmater->v.origin);
	SetAngle(eInmater,eInmater->v.angles);

	DropToFloor(eInmater);
}
