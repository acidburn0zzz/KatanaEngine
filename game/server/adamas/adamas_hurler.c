/*	Copyright (C) 2014 OldTimes Software / Mark Elsworth Sowden
*/
#include "server_monster.h"

/*
	Small little blob thing that throws itself around.
*/

#define	HURLER_MAX_HEALTH	150

void Hurler_Pain(edict_t *eHurler,edict_t *eOther);
void Hurler_Think(edict_t *eHurler);

void Hurler_Spawn(edict_t *eHurler)
{
	eHurler->monster.iType		= MONSTER_HURLER;
	eHurler->monster.think_pain	= Hurler_Pain;
	eHurler->monster.Think		= Hurler_Think;

	eHurler->Physics.fMass	= 0.5f;
	eHurler->Physics.iSolid	= SOLID_SLIDEBOX;

	eHurler->v.bTakeDamage	= true;
	eHurler->v.movetype		= MOVETYPE_BOUNCE;
	eHurler->v.iHealth		= HURLER_MAX_HEALTH;
	eHurler->v.iMaxHealth	= HURLER_MAX_HEALTH;
	eHurler->v.netname		= "Hurler";

	DropToFloor(eHurler);
}

void Hurler_Think(edict_t *eHurler)
{}

void Hurler_Pain(edict_t *eHurler,edict_t *eOther)
{
}
