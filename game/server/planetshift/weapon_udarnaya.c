#include "server_weapon.h"

void Udarnaya_Touch(edict_t *ent,edict_t *other)
{
	if(other->v.iHealth <= 0)
		return;

	Entity_Remove(ent);
}

void Udarnaya_Spawn(edict_t *ent)
{
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/weapons/w_udarnaya.md2");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"items/weapons/udarnayapickup.wav");

	if(bIsMultiplayer)
		ent->v.effects = EF_MOTION_ROTATE;

	ent->v.netname			= "Udarnaya";
	ent->Physics.iSolid		= SOLID_TRIGGER;
	ent->v.TouchFunction	= Udarnaya_Touch;

	Entity_SetModel(ent,"models/weapons/w_udarnaya.md2");
	Entity_SetSize(ent,-16.0f,-16.0f,-24.0f,16.0f,16.0f,32.0f);
}