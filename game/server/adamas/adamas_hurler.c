/*	Copyright (C) 2014 OldTimes Software / Mark Elsworth Sowden
*/
#include "server_monster.h"

/*
	Small little blob thing that throws itself around.
*/

#define	HURLER_MAX_HEALTH	250

int	iHurlers = 0;

void Hurler_Pain(edict_t *eHurler,edict_t *eOther);
void Hurler_Think(edict_t *eHurler);
void Hurler_Touch(edict_t *eHurler,edict_t *eOther);
void Hurler_Die(edict_t *eHurler,edict_t *eOther);

void Hurler_Spawn(edict_t *eHurler)
{
	Engine.Server_PrecacheResource(RESOURCE_MODEL,HURLER_MODEL_BODY);

	Entity_SetModel(eHurler,HURLER_MODEL_BODY);

	eHurler->monster.iType		= MONSTER_HURLER;
	eHurler->monster.think_pain	= Hurler_Pain;
	eHurler->monster.think_die	= Hurler_Die;
	eHurler->monster.Think		= Hurler_Think;

	eHurler->Physics.fMass		= 0.5f;
	eHurler->Physics.iSolid		= SOLID_SLIDEBOX;
	eHurler->Physics.fFriction	= 10.0f;
	eHurler->Physics.fGravity	= SERVER_GRAVITY;

	eHurler->v.bTakeDamage		= true;
	eHurler->v.movetype			= MOVETYPE_BOUNCE;
	eHurler->v.iHealth			= HURLER_MAX_HEALTH;
	eHurler->v.iMaxHealth		= HURLER_MAX_HEALTH;
	eHurler->v.netname			= "Hurler";
	eHurler->v.TouchFunction	= Hurler_Touch;

	Monster_SetState(eHurler,STATE_AWAKE);
	Monster_SetThink(eHurler,THINK_IDLE);

	// Add to the count of Hurlers currently in the game.
	iHurlers++;

	DropToFloor(eHurler);
}

void Hurler_Think(edict_t *eHurler)
{
	edict_t *eTarget;

	switch(eHurler->monster.iThink)
	{
	case THINK_IDLE:
		Monster_Jump(eHurler,70.0f);

		if(rand()%50 == 0)
			eHurler->v.angles[1] = (float)(rand()%360);

		if(!eHurler->monster.eEnemy)
		{
			eHurler->monster.eTarget = Monster_GetTarget(eHurler);
			if(!eHurler->monster.eTarget)
				return;

			if(Monster_SetEnemy(eHurler))
				Monster_SetThink(eHurler,THINK_PURSUING);
		}
		break;
	case THINK_PURSUING:
		if(!eHurler->monster.eEnemy || eHurler->monster.eEnemy->v.iHealth <= 0)
		{
			Monster_SetThink(eHurler,THINK_IDLE);
			return;
		}

		// Throw us in the direction of the enemy.
		if(eHurler->v.flags & FL_ONGROUND)
		{
			vec3_t	vOrigin;

			Monster_Jump(eHurler,120.0f);

			Math_VectorSubtract(eHurler->monster.eEnemy->v.origin,eHurler->v.origin,vOrigin);
			Math_VectorDivide(vOrigin,2.0f,vOrigin);
			Math_VectorAdd(eHurler->v.velocity,vOrigin,eHurler->v.velocity);
		}

		{
			vec3_t	vAngles;

			// Update angles.
			Math_VectorSubtract(eHurler->v.origin,eHurler->monster.eEnemy->v.origin,vAngles);
			Math_VectorNormalize(vAngles);
			Math_VectorInverse(vAngles);
			Math_MVToVector(Math_VectorToAngles(vAngles),eHurler->v.angles);

			eHurler->v.angles[0] = 0;
		}
		break;
	}
}

void Hurler_Touch(edict_t *eHurler,edict_t *eOther)
{
	if(Entity_IsPlayer(eOther))
	{
		vec3_t vOrigin;

		MONSTER_Damage(eOther,eHurler,15,DAMAGE_TYPE_NONE);

		// Throw it back.
		Math_VectorSubtract(eHurler->monster.eEnemy->v.origin,eHurler->v.origin,vOrigin);
		Math_VectorScale(vOrigin,4.0f,vOrigin);
		Math_VectorInverse(vOrigin);
		Math_VectorAdd(eHurler->v.velocity,vOrigin,eHurler->v.velocity);
	}
}

void Hurler_Pain(edict_t *eHurler,edict_t *eOther)
{
}

void Hurler_Die(edict_t *eHurler,edict_t *eOther)
{
	iHurlers--;
	if(!iHurlers)
		Engine.Server_ChangeLevel()
}