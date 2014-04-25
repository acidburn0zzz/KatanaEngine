/*  Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_main.h"

void Deco_Push(edict_t *ent,edict_t *other)
{
	ent->v.velocity[0] = ent->v.velocity[0]+other->v.velocity[0];
	ent->v.velocity[1] = ent->v.velocity[0]+other->v.velocity[1];
	ent->v.velocity[2] = ent->v.velocity[0]+other->v.velocity[2];

	DropToFloor(ent);
}

void Deco_VaseBreak(edict_t *ent,edict_t *other)
{
	int		i;
	vec3_t	vDirection;

	for(i = 0; i < 3; i++)
		vDirection[i] = ent->v.velocity[i]-(rand()%10-rand()%10);

	Sound(ent,CHAN_VOICE,"fx/vasebrk.wav",255,ATTN_NORM);

	ThrowGib(ent->v.origin,vDirection,"models/gibs/vgib1.md2",(float)(rand()%30),false);
	ThrowGib(ent->v.origin,vDirection,"models/gibs/vgib2.md2",(float)(rand()%30),false);

	Entity_Remove(ent);
}

void deco_vase(edict_t *ent)
{
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/deco/e2/vase01.md2");
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/deco/e2/vgib1.md2");
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/deco/e2/vgib2.md2");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/vasebrk.wav");

	ent->v.movetype		= MOVETYPE_BOUNCE;
	ent->v.iHealth		= 10;
	ent->Physics.iSolid	= SOLID_BBOX;
	ent->v.bTakeDamage	= true;

	ent->monster.think_die = Deco_VaseBreak;

	Entity_SetOrigin(ent,ent->v.origin);
	Entity_SetModel(ent,"models/deco/e2/vase01.md2");
	Entity_SetSize(ent,-11,-13,0,11,12,31);

//	DropToFloor(ent);
}
