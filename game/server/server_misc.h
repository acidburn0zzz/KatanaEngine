/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SERVERMISC__
#define __SERVERMISC__

#include "server_main.h"

/*	Spawn point types.
*/
enum
{
	INFO_PLAYER_START,
	INFO_PLAYER_COOP,
	INFO_PLAYER_DEATHMATCH,
	INFO_PLAYER_CTF,

	// Companions
	INFO_PLAYER_MIKIKO,
	INFO_PLAYER_SUPERFLY
};

void Bot_Spawn(edict_t *ent);

void CTF_FlagSpawn(edict_t *eFlag);

void Area_PlayerSpawn(edict_t *eArea);
void Area_BreakableSpawn(edict_t *eArea);
void Area_ButtonSpawn(edict_t *eArea);
void Area_ChangeLevel(edict_t *eArea);
void Area_ClimbSpawn(edict_t *eArea);
void Area_DoorSpawn(edict_t *eArea);
void Area_NoclipSpawn(edict_t *eArea);
void Area_PlatSpawn(edict_t *eArea);
void Area_PushSpawn(edict_t *eArea);
void Area_PushableSpawn(edict_t *eArea);
void Area_PlatformSpawn(edict_t *eArea);
void Area_RotateSpawn(edict_t *eArea);
void Area_TriggerSpawn(edict_t *eArea);
void Area_WallSpawn(edict_t *eArea);

void Point_AmbientSpawn(edict_t *eEntity);
void Point_DecorationSpawn(edict_t *eDecoration);
void Point_DynamicLight(edict_t *ent);
void Point_DamageSpawn(edict_t *ent);
void Point_EffectSpawn(edict_t *eFlare);
void Point_FlareSpawn(edict_t *eFlare);
void Point_MessageSpawn(edict_t *eEntity);
void Point_LightstyleSpawn(edict_t *eEntity);
void Point_LogicSpawn(edict_t *eEntity);
void Point_VehicleSpawn(edict_t *eVehicle);
void Point_MonsterSpawn(edict_t *eMonster);
void Point_MultiTriggerSpawn(edict_t *eEntity);
void Point_NullSpawn(edict_t *eEntity);
void Point_ParticleSpawn(edict_t *ent);
void Point_PropSpawn(edict_t *eEntity);
void Point_SoundSpawn(edict_t *eEntity);
void Point_Start(edict_t *ent);
void Point_ShakeSpawn(edict_t *ent);
void Point_TeleportSpawn(edict_t *ent);
void Point_TimedTriggerSpawn(edict_t *eEntity);
void Point_WaypointSpawn(edict_t *eEntity);
void Point_LightSpawn(edict_t *eLight);

// Weapons
void WEAPON_StickThink(edict_t *ent);

void ThrowGib(vec3_t origin,vec3_t velocity,char *model,float damage,bool bleed);
void Item_Respawn(edict_t *ent);

bool DropToFloor(edict_t *ent);

bool EntitiesTouching(edict_t *e1,edict_t *e2);

void Client_RelinkEntities(entity_t *ent,int i,double dTime);

#endif
