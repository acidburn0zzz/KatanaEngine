#include "server_main.h"

/*
	Point entities.
*/

#include "server_waypoint.h"
#include "server_vehicle.h"
#include "server_physics.h"

enum
{
	PARTICLE_DEFAULT,
	PARTICLE_STACK
};

void Point_NullSpawn(edict_t *eEntity)
{
	// [3/3/2013] Remove if there is no name given! ~hogsy
	if(!eEntity->v.cName)
		ENTITY_REMOVE(eEntity);

	Entity_SetOrigin(eEntity,eEntity->v.origin);
}

#ifdef GAME_OPENKATANA
void Prisoner_Spawn(edict_t *ePrisoner);	// [2/10/2012] See monster_prisoner.c ~hogsy
void LaserGat_Spawn(edict_t *eLaserGat);	// [14/2/2013] See monster_lasergat.c ~hogsy
void Inmater_Spawn(edict_t *eInmater);		// [3/3/2013] See monster_inmater.c ~hogsy
#elif GAME_ADAMAS
void Hurler_Spawn(edict_t *eHurler);
#endif

void Point_MonsterSpawn(edict_t *eMonster)
{
	if(cvServerMonsters.value <= 0)
		ENTITY_REMOVE(eMonster);

	Server.iMonsters++;

	switch(eMonster->local.style)
	{
#ifdef GAME_OPENKATANA
	case MONSTER_PRISONER:
		eMonster->v.cClassname = "monster_prisoner";
		Prisoner_Spawn(eMonster);
		break;
	case MONSTER_LASERGAT:
		eMonster->v.cClassname = "monster_lasergat";
		LaserGat_Spawn(eMonster);
		break;
	case MONSTER_INMATER:
		eMonster->v.cClassname = "monster_inmater";
		Inmater_Spawn(eMonster);
		break;
#elif GAME_ADAMAS
	case MONSTER_HURLER:
		eMonster->v.cClassname = "monster_hurler";
		Hurler_Spawn(eMonster);
		break;
#endif
	default:
		Engine.Con_Warning("Invalid monster type (%i)!\n",eMonster->local.style);

		// Reduce the monster count. ~hogsy
		Server.iMonsters--;

		Entity_Remove(eMonster);
	}
}

#ifdef KATANA_GAME_ICTUS
void Rover_Spawn(edict_t *eRover);
#endif

void Point_VehicleSpawn(edict_t *eVehicle)
{
	switch(eVehicle->local.style)
	{
#ifdef KATANA_GAME_ICTUS
	case VEHICLE_TYPE_ROVER:
		Rover_Spawn(eVehicle);
		break;
#endif
	default:
		Engine.Con_Warning("Invalid vehicle type! (%)\n",eVehicle->local.style);
		Entity_Remove(eVehicle);
	}
}

/*
	Player Start
*/

void Point_Start(edict_t *ent)
{
	// [2/1/2013] Revised ~hogsy
	switch((int)cvServerGameMode.value)
	{
	case MODE_SINGLEPLAYER:
		if(	(ent->local.style != INFO_PLAYER_START)		&&
			(ent->local.style != INFO_PLAYER_MIKIKO)	&&
			(ent->local.style != INFO_PLAYER_SUPERFLY))
        {
            Engine.Con_Warning("Invalid start style! (%i)\n",ent->local.style);

			ENTITY_REMOVE(ent);
        }

        if(ent->local.style == INFO_PLAYER_SUPERFLY)
        {
            ent->local.style = BOT_SUPERFLY;

            Bot_Spawn(ent);
        }
        else if(ent->local.style == INFO_PLAYER_MIKIKO)
        {
            ent->local.style = BOT_MIKIKO;

            Bot_Spawn(ent);
        }
		break;
#ifdef OPENKATANA
	case MODE_CAPTURETHEFLAG:
		if((ent->local.style != INFO_PLAYER_CTF) || !ent->local.pTeam)
        {
            Engine.Con_Warning("Invalid start style! (%i)\n",ent->local.style);

			ENTITY_REMOVE(ent);
        }

		if(ent->local.pTeam == TEAM_RED)
			ent->v.cClassname = "point_start_red";
		else
			ent->v.cClassname = "point_start_blue";
		break;
	case MODE_COOPERATIVE:
		if(	(ent->local.style != INFO_PLAYER_START)		&&
			(ent->local.style != INFO_PLAYER_MIKIKO)	&&
			(ent->local.style != INFO_PLAYER_SUPERFLY)	&&
			(ent->local.style != INFO_PLAYER_COOP))
        {
            Engine.Con_Warning("Invalid start style! (%i)\n",ent->local.style);

			ENTITY_REMOVE(ent);
        }
		break;
	case MODE_DEATHMATCH:
	case MODE_VEKTAR:
		if(ent->local.style != INFO_PLAYER_DEATHMATCH)
        {
            Engine.Con_Warning("Invalid start style! (%i)\n",ent->local.style);

			ENTITY_REMOVE(ent);
        }

		ent->v.cClassname = "point_start_deathmatch";
		break;
#endif
	default:
		// [16/1/2014] Show the gamemode, not the entity style! ~hogsy
		Engine.Con_Warning("Failed to set up spawn points, unknown game type! (%i)\n",cvServerGameMode.value);
		ENTITY_REMOVE(ent);
	}

	// [19/7/2012] Create a waypoint here so bots can try to avoid ~hogsy
	// [16/1/2014] This was getting set outside the world, derp ~hogsy
	Waypoint_Spawn(ent->v.origin,WAYPOINT_SPAWN);
}

/*
	Particle Emitter
*/
// [17/7/2012] Renamed to Point_ParticleEmit ~hogsy
void Point_ParticleEmit(edict_t *ent)
{
	Engine.Particle(ent->v.origin,ent->v.velocity,ent->Model.fScale,ent->v.model,ent->local.count);

	ent->v.think		= Point_ParticleEmit;
	ent->v.dNextThink	= Server.dTime+ent->local.dAttackFinished;
}

void Point_ParticleTrigger(edict_t *ent)
{
	Engine.Particle(ent->v.origin,ent->v.velocity,ent->Model.fScale,ent->v.model,ent->local.count);
}

void Point_ParticleSpawn(edict_t *ent)
{
	// [10/3/2012] Revised ~hogsy
	if(ent->local.count <= 0)
		ent->local.count = 1;

	// [9/3/2012] Revised ~hogsy
	if(ent->Model.fScale <= 0)
		ent->Model.fScale = 7.0f;

	// [21/3/2012] Updated ~hogsy
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,ent->v.model);

	Entity_SetOrigin(ent,ent->v.origin);

	switch(ent->local.style)
	{
	case PARTICLE_DEFAULT:
		break;
	case PARTICLE_STACK:
		ent->v.velocity[2] += 15.0f;
		break;
	default:
		Engine.Con_Warning("Unknown particle type (%i)!\n",ent->local.style);
	}

	// [9/3/2012] Revised ~hogsy
	if(ent->local.dAttackFinished > 0)
	{
		ent->v.think		= Point_ParticleEmit;
		ent->v.dNextThink	= Server.dTime+ent->local.dAttackFinished;
	}
	else
		ent->v.use = Point_ParticleTrigger;
}

/*
	Flare
*/

void Point_FlareSpawn(edict_t *eFlare)
{
	Engine.Server_PrecacheResource(RESOURCE_FLARE,eFlare->v.model);

	Entity_SetOrigin(eFlare,eFlare->v.origin);

//	Flare(ent->v.origin,ent->local.red,ent->local.green,ent->local.blue,ent->alpha,ent->local.scale,ent->v.model);
}

/*
	Dynamic Light
*/

void Point_DynamicLightThink(edict_t *ent)
{
	ent->v.origin[2] = (float)sin(Server.dTime*2.0)*10.0f;

	ent->v.dNextThink = Server.dTime+0.1;
}

void Point_DynamicLight(edict_t *ent)
{
	Entity_SetOrigin(ent,ent->v.origin);

	ent->v.effects = EF_LIGHT_GREEN;

	ent->v.think		= Point_DynamicLightThink;
	ent->v.dNextThink	= Server.dTime+0.1;
}

/*
	Ambient
*/

enum
{
	NONE,
	AMBIENT_RADIUS_SMALL,
	AMBIENT_RADIUS_MEDIUM,
	AMBIENT_RADIUS_LARGE,
	AMBIENT_RADIUS_EVERYWHERE
};

void Point_AmbientSpawn(edict_t *eEntity)
{
	int iAttenuation;

	if(!eEntity->local.volume)
		eEntity->local.volume = 255;

	if(!eEntity->v.noise)
	{
		Engine.Con_Warning("No sound set for point_ambient! (%i %i %i)\n",
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	switch(eEntity->local.style)
	{
		case AMBIENT_RADIUS_EVERYWHERE:
			iAttenuation = ATTN_NONE;
			break;
		case AMBIENT_RADIUS_SMALL:
			iAttenuation = ATTN_IDLE;
			break;
		case AMBIENT_RADIUS_MEDIUM:
			iAttenuation = ATTN_STATIC;
			break;
		case AMBIENT_RADIUS_LARGE:
			iAttenuation = ATTN_NORM;
			break;
		default:
			iAttenuation = ATTN_NORM;
	}

	Engine.Server_PrecacheResource(RESOURCE_SOUND,eEntity->v.noise);

	Engine.Server_AmbientSound(eEntity->v.origin,eEntity->v.noise,eEntity->local.volume,iAttenuation);
}

/*
	Camera
*/

// [28/7/2012] Spawn flags ~hogsy
#define	CAMERA_SPAWN_ACTIVE	0	// Defines if this camera is active after spawning

void Point_CameraUse(edict_t *eEntity, edict_t *eOther)
{
		Engine.SetMessageEntity(eOther);

		Engine.WriteByte(	MSG_ONE,	SVC_SETVIEW	);
		Engine.WriteEntity(	MSG_ONE,	eEntity		);

		// [28/7/2012] Set up the camera angle ~hogsy
		Engine.WriteByte(	MSG_ONE,	SVC_SETANGLE			);
		Engine.WriteAngle(	MSG_ONE,	eEntity->v.angles[0]	);
		Engine.WriteAngle(	MSG_ONE,	eEntity->v.angles[1]	);
		Engine.WriteAngle(	MSG_ONE,	eEntity->v.angles[2]	);
}

void Point_CameraSpawn(edict_t *eEntity)
{
	if(eEntity->v.spawnflags & CAMERA_SPAWN_ACTIVE)
	{
		//Engine.SetMessageEntity(sv_player);

		Engine.WriteByte(	MSG_ONE,	SVC_SETVIEW	);
		Engine.WriteEntity(	MSG_ONE,	eEntity		);

		// [28/7/2012] Set up the camera angle ~hogsy
		Engine.WriteByte(	MSG_ONE,	SVC_SETANGLE			);
		Engine.WriteAngle(	MSG_ONE,	eEntity->v.angles[0]	);
		Engine.WriteAngle(	MSG_ONE,	eEntity->v.angles[1]	);
		Engine.WriteAngle(	MSG_ONE,	eEntity->v.angles[2]	);
	}
	else if(!eEntity->local.dWait)
		eEntity->local.dWait = 3.0;

	Entity_SetOrigin(eEntity,eEntity->v.origin);

	//eEntity->v.use = Point_CameraUse;
}

/*
	Waypoint
*/

// [20/9/2012] Waypoints placed inside a level ~hogsy
void Point_WaypointSpawn(edict_t *eEntity)
{
	Waypoint_Spawn(eEntity->v.origin,(WaypointType_t)eEntity->local.style);

	Entity_Remove(eEntity);
}

/*
	Sound
*/

// [2/2/2013] Is this needed or could it be extended in any way? ~eukos
enum
{
	POINT_SND_LOCAL,
	POINT_SND_ENTITY,
	POINT_SND_GLOBAL
};

void Point_SoundUse(edict_t *eEntity)
{
	switch(eEntity->local.style)
	{
	case POINT_SND_ENTITY:
		Sound(eEntity->local.activator,CHAN_AUTO,eEntity->v.noise,eEntity->local.volume,ATTN_NORM);
		break;
	case POINT_SND_GLOBAL:
		Sound(eEntity,CHAN_AUTO,eEntity->v.noise,eEntity->local.volume,ATTN_NONE);
		break;
	default:
		Sound(eEntity,CHAN_AUTO,eEntity->v.noise,eEntity->local.volume,ATTN_NORM);
	}

}

// [2/2/2013] Renamed from Point_PlaySound to Point_SoundSpawn for consistency ~hogsy
void Point_SoundSpawn(edict_t *eEntity)
{
	if(!eEntity->v.noise)
	{
		Engine.Con_Warning("No noise not set for %i (%i %i %i)!\n",
			eEntity->v.cClassname,
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}
	else if(!eEntity->local.volume)
		eEntity->local.volume = 255;

	Engine.Server_PrecacheResource(RESOURCE_SOUND,eEntity->v.noise);

	Entity_SetOrigin(eEntity,eEntity->v.origin);

	eEntity->v.use = Point_SoundUse;
}

/*
	Decoration
*/

#define DECORATION_DROPTOFLOOR	0

void Point_DecorationSpawn(edict_t *eDecoration)
{
	if(!eDecoration->v.model)
		ENTITY_REMOVE(eDecoration);

	Engine.Server_PrecacheResource(RESOURCE_MODEL,eDecoration->v.model);

	Entity_SetModel(eDecoration,eDecoration->v.model);

	if(eDecoration->v.spawnflags & DECORATION_DROPTOFLOOR)
		DropToFloor(eDecoration);
}

/*
	Message
*/

enum
{
	POINT_MSG_CENTER,
	POINT_MSG_LOCALMESSAGE,
	POINT_MSG_SERVERMESSAGE,
	POINT_MSG_INFOMESSAGE
};

void Point_MessageLocal(edict_t *eEntity)
{
	// [4/8/2013] Simplified ~hogsy
	if(!eEntity->local.activator || ((eEntity->monster.iType != MONSTER_PLAYER) && eEntity->local.activator->v.iHealth <= 0))
		return;

	Engine.Server_SinglePrint(eEntity->local.activator,eEntity->v.message);
}

void Point_MessageCenter(edict_t *eEntity)
{
	// [4/8/2013] Simplified ~hogsy
	if(!eEntity->local.activator || ((eEntity->monster.iType != MONSTER_PLAYER) && eEntity->local.activator->v.iHealth <= 0))
		return;

	Engine.CenterPrint(eEntity->local.activator,eEntity->v.message);
}

void Point_MessageServer(edict_t *eEntity)
{
	Engine.Server_BroadcastPrint(eEntity->v.message);
}

void Point_InfoMessage(edict_t *eEntity)
{
	if(!eEntity->local.activator)
		return;
	else if((eEntity->monster.iType != MONSTER_PLAYER) && eEntity->local.activator->v.iHealth <= 0)
		return;

	Engine.Server_SinglePrint(eEntity->local.activator,"New info message received");
	eEntity->local.activator->local.cInfoMessage = eEntity->v.message;
}

void Point_MessageSpawn(edict_t *eEntity)
{
	if(!eEntity->v.message)
	{
		Engine.Con_Warning("Parameter 'message' not set! (%s) (%i %i %i)\n",
			eEntity->v.cClassname,
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	Entity_SetOrigin(eEntity,eEntity->v.origin);

	switch(eEntity->local.style)
	{
	case POINT_MSG_CENTER:
		eEntity->v.use = Point_MessageCenter;
		break;
	case POINT_MSG_LOCALMESSAGE:
		eEntity->v.use = Point_MessageLocal;
		break;
	case POINT_MSG_SERVERMESSAGE:
		eEntity->v.use = Point_MessageServer;
		break;
	case POINT_MSG_INFOMESSAGE:
		eEntity->v.use = Point_InfoMessage;
		break;
	default:
		Engine.Con_Warning("Parameter 'style' not set! (%s) (%i %i %i)\n",
			eEntity->v.cClassname,
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		Entity_Remove(eEntity);
	}
}

/*
	Teleport
*/

void Point_TeleportUse(edict_t *eEntity)
{
	Entity_SetOrigin(eEntity->local.activator,eEntity->v.origin);

	// [4/8/2013] Simplified ~hogsy
	Math_VectorCopy(eEntity->v.angles,eEntity->local.activator->v.angles);
}

void Point_TeleportSpawn(edict_t *eEntity)
{
	Entity_SetOrigin(eEntity,eEntity->v.origin);

	eEntity->v.use = Point_TeleportUse;
}

/*
	Prop
*/

enum
{
	BREAKABLE_GLASS,
	BREAKABLE_WOOD,
	BREAKABLE_ROCK,
	BREAKABLE_METAL
};

void Area_BreakableDie(edict_t *eArea,edict_t *eOther);

void Point_PropTouch(edict_t *eEntity, edict_t *eOther)
{
	// [28/5/2013] Why can't monsters move props too? Removed check. ~hogsy
	if(!eOther->v.iHealth)
		return;

	// [28/5/2013] Simplified all this ~hogsy
	Math_VectorClear(eEntity->v.velocity);
	Math_VectorScale(eOther->v.velocity,0.25f,eEntity->v.velocity);
}

void Point_PropSpawn(edict_t *eEntity)
{
	if(!eEntity->v.model)
		Entity_Remove(eEntity);

	eEntity->v.iHealth = 10;

	if(eEntity->v.iHealth)
	{
		switch(eEntity->local.style)
		{
			case BREAKABLE_GLASS:
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_GLASS0);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_GLASS1);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_GLASS2);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GLASS0);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GLASS1);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GLASS2);
				break;
			case BREAKABLE_WOOD:
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_WOOD0);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_WOOD1);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_WOOD2);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_WOOD0);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_WOOD1);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_WOOD2);
				break;
			case BREAKABLE_ROCK:
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_ROCK0);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_ROCK1);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_ROCK2);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/rock_gibs1.md2");
				Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/rock_gibs2.md2");
				Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/rock_gibs3.md2");
				break;
			case BREAKABLE_METAL:
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_METAL0);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_METAL1);
				Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_METAL2);
				Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/metal_gibs1.md2");
				Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/metal_gibs2.md2");
				Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/metal_gibs3.md2");
				break;
			default:
				// [4/8/2013] Updated for consistency ~hogsy
				Engine.Con_Warning("Prop with unknown style! (%i)\n",eEntity->local.style);
		}

		eEntity->v.bTakeDamage		= true;
		eEntity->local.bBleed		= false;
		eEntity->monster.think_die	= Area_BreakableDie;
	}

	Engine.Server_PrecacheResource(RESOURCE_MODEL,eEntity->v.model);

	eEntity->v.movetype			= MOVETYPE_BOUNCE;
	eEntity->v.TouchFunction	= Point_PropTouch;

	eEntity->Physics.iSolid		= SOLID_BBOX;
	eEntity->Physics.fGravity	= cvServerGravity.value;
	eEntity->Physics.fMass		= 0.5f;

	Entity_SetModel(eEntity,eEntity->v.model);
	Entity_SetOrigin(eEntity,eEntity->v.origin);
	Entity_SetSize(eEntity,-16.0f,-16.0f,-24.0f,16.0f,16.0f,32.0f);

	eEntity->v.bTakeDamage		= true;
	eEntity->local.bBleed		= false;
	eEntity->monster.think_die	= Area_BreakableDie;
}

/*
	Shake
*/

void Point_ShakeThink (edict_t *eEntity)
{
	edict_t *eEnts = Engine.Server_FindRadius(eEntity->v.origin,10000.0f);

	if(eEntity->local.dAttackFinished < Server.dTime)
		return;

	do
	{
		if(!(eEnts->v.movetype == MOVETYPE_NONE) && !(eEnts->v.movetype == MOVETYPE_FLY))
		{
			eEnts->v.velocity[0] += Math_CRandom()*250;
			eEnts->v.velocity[1] += Math_CRandom()*250;

			eEnts->v.punchangle[0] += Math_CRandom()*8;
			eEnts->v.punchangle[2] += Math_CRandom()*8;

			if(eEnts->v.flags & FL_ONGROUND)
				eEnts->v.velocity[2] = eEntity->local.speed;
		}

		eEnts = eEnts->v.chain;

	} while(eEnts);

	eEntity->v.dNextThink = Server.dTime+eEntity->local.delay;
}

void Point_ShakeUse (edict_t *eEntity)
{
	eEntity->v.dNextThink			= Server.dTime;
	eEntity->local.dAttackFinished	= Server.dTime+eEntity->local.dWait;
}

void Point_ShakeSpawn (edict_t *eEntity)
{
	if (!eEntity->local.delay)
		eEntity->local.delay = 0.2f;

	if (!eEntity->local.speed)
		eEntity->local.speed = 300;

	if (!eEntity->local.dWait)
		eEntity->local.dWait = 5.0;

	eEntity->v.use		= Point_ShakeUse;
	eEntity->v.think	= Point_ShakeThink;
}

/*
	Effect
*/

void Point_EffectUse(edict_t *eEntity)
{
	switch(eEntity->local.style)
	{
		case 1:
			// [15/8/2013] Updated ~hogsy
			Entity_RadiusDamage(eEntity,MONSTER_RANGE_MEDIUM,eEntity->local.iDamage, eEntity->local.iDamageType);
			break;
		case 2:
			Engine.WriteByte(MSG_BROADCAST,SVC_TEMPENTITY);
			Engine.WriteByte(MSG_BROADCAST,CTE_GUNSHOT);
			Engine.WriteCoord(MSG_BROADCAST,eEntity->v.origin[0]);
			Engine.WriteCoord(MSG_BROADCAST,eEntity->v.origin[1]);
			Engine.WriteCoord(MSG_BROADCAST,eEntity->v.origin[2]);
			break;
		case 3:
			Engine.WriteByte(MSG_BROADCAST,SVC_TEMPENTITY);
			Engine.WriteByte(MSG_BROADCAST,CTE_TELEPORT);
			Engine.WriteCoord(MSG_BROADCAST,eEntity->v.origin[0]);
			Engine.WriteCoord(MSG_BROADCAST,eEntity->v.origin[1]);
			Engine.WriteCoord(MSG_BROADCAST,eEntity->v.origin[2]);
			break;
		default:
			// [24/6/2013] Made this more informative ~hogsy
			Engine.Con_Warning("Unknown effect style! (%i)\n",eEntity->local.style);
	}

	if(eEntity->v.noise)
		Sound(eEntity,CHAN_ITEM,eEntity->v.noise,255,ATTN_NORM);
}

void Point_EffectSpawn(edict_t *eEntity)
{
	if(eEntity->v.noise)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eEntity->v.noise);

	eEntity->v.use = Point_EffectUse;

	Entity_SetOrigin(eEntity,eEntity->v.origin);
}

/*
	Damage
*/

void Point_DamageUse(edict_t *eEntity)
{
	MONSTER_Damage(eEntity->local.activator,eEntity,eEntity->local.iDamage,eEntity->local.style);
}

void Point_DamageSpawn(edict_t *eEntity)
{
	if(!eEntity->local.iDamage)
		eEntity->local.iDamage = 10;

	if(!eEntity->local.style)
		eEntity->local.style = DAMAGE_TYPE_NORMAL;

	eEntity->v.use = Point_DamageUse;

	Entity_SetOrigin(eEntity,eEntity->v.origin);
}

/*
	Lightstyle Switcher
*/

void Point_LightstyleDie(edict_t *eEntity)
{
	Engine.LightStyle(eEntity->local.style,"a");

	if(eEntity->v.targetname)
		UseTargets(eEntity, eEntity);
}

void Point_LightstyleUse(edict_t *eEntity)
{
	if(eEntity->v.noise)
		Sound(eEntity,CHAN_ITEM,eEntity->v.noise,255,ATTN_NORM);

	Engine.LightStyle(eEntity->local.style,eEntity->v.message);

	if(eEntity->local.dWait > 0)
	{
		eEntity->v.dNextThink	= Server.dTime+eEntity->local.dWait;
		eEntity->v.think		= Point_LightstyleDie;
	}
}

void Point_LightstyleSpawn(edict_t *eEntity)
{
	if(eEntity->v.noise)
		Engine.Server_PrecacheResource(RESOURCE_SOUND,eEntity->v.noise);

	if(!eEntity->v.message)
		eEntity->v.message = "a";

	if(!eEntity->local.style)
	{
		Engine.Con_Warning("No style set for point_lightstyle! (%i %i %i)\n",
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	eEntity->v.use = Point_LightstyleUse;

	Entity_SetOrigin(eEntity,eEntity->v.origin);
}

/*
	Multi-Trigger / Random
*/

void Point_MultiTriggerUse(edict_t *eEntity)
{
	int iRand;

	if(eEntity->local.style)
	{
		iRand = rand();

		if(iRand < 0.5)
			UseTargets(eEntity, eEntity);
		else
		{
			strcpy (eEntity->v.targetname,eEntity->v.message);
			UseTargets(eEntity, eEntity);
		}
	}
	else
	{
		UseTargets(eEntity, eEntity);
		strcpy (eEntity->v.targetname,eEntity->v.message);
		UseTargets(eEntity, eEntity);
	}
}

void Point_MultiTriggerSpawn(edict_t *eEntity)
{
	if(!eEntity->v.targetname)
	{
		Engine.Con_Warning("No targetname set for point_multitrigger! (%i %i %i)\n",
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	if(!eEntity->v.message)
	{
		Engine.Con_Warning("No message set for point_multitrigger! (%i %i %i)\n",
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	if(!eEntity->local.style)
		eEntity->local.style = 0;

	eEntity->v.use = Point_MultiTriggerUse;

	Entity_SetOrigin(eEntity,eEntity->v.origin);
}

/*
	Timed Trigger
*/

void Point_TimedTriggerThink(edict_t *eEntity)
{
	UseTargets(eEntity, eEntity);
}

void Point_TimedTriggerUse(edict_t *eEntity)
{
	eEntity->v.dNextThink	= Server.dTime+eEntity->local.dWait;
	eEntity->v.think		= Point_TimedTriggerThink;
}

void Point_TimedTriggerSpawn(edict_t *eEntity)
{
	if(!eEntity->v.targetname)
	{
		Engine.Con_Warning("No targetname set for point_timedtrigger (%f %f %f)!\n",
			eEntity->v.origin[0],
			eEntity->v.origin[1],
			eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	eEntity->v.use = Point_TimedTriggerUse;

	if(eEntity->v.spawnflags & 32)
		eEntity->v.use(eEntity);

	Entity_SetOrigin(eEntity,eEntity->v.origin);
}

/*
	Logic
*/

void Point_LogicThink(edict_t *eEntity)
{
	// [2/1/2013] Set eEnt2 as NULL here to avoid an VC warning *sighs* ~hogsy
	edict_t *eEnt1,*eEnt2 = NULL;

	eEnt1 = Engine.Server_FindEntity(Server.eWorld,eEntity->local.cTarget1,false);
	if(!eEnt1)
	{
		Engine.Con_Warning("Point_Logic: Can't find Target1!\n");
		return;
	}

	if(eEntity->local.style < 3)
	{
		eEnt2 = Engine.Server_FindEntity(Server.eWorld,eEntity->local.cTarget2,false);
		if(!eEnt2)
		{
			Engine.Con_Warning("Point_Logic: Can't find Target2!\n");
			return;
		}
	}

	switch(eEntity->local.style)
	{
	case 1:
		if(eEnt1->local.iValue && eEnt2->local.iValue)
		{
			if(eEntity->v.targetname && !eEntity->local.iValue)
				UseTargets(eEntity,eEntity->local.activator);

			eEntity->local.iValue = 1;
		}
		else
			eEntity->local.iValue = 0;
		break;
	case 2:
		if(eEnt1->local.iValue || eEnt2->local.iValue)
		{
			if(eEntity->v.targetname && eEntity->local.iValue)
				UseTargets(eEntity,eEntity->local.activator);

			eEntity->local.iValue = 1;
		}
		else
			eEntity->local.iValue = 0;
		break;
	case 3:
		if(!eEnt1->local.iValue)
			eEntity->local.iValue = 1;
		else
			eEntity->local.iValue = 0;
		break;
	}

	eEntity->v.dNextThink = Server.dTime+eEntity->local.dWait;
}

void Point_LogicSpawn(edict_t *eEntity)
{
	if(!eEntity->local.cTarget1)
	{
		Engine.Con_Warning("No target1 set for point_logic! (%i %i %i)\n",
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	if(!eEntity->local.cTarget2 && eEntity->local.style != 3)
	{
		Engine.Con_Warning("No target2 set for point_logic! (%i %i %i)\n",
			(int)eEntity->v.origin[0],
			(int)eEntity->v.origin[1],
			(int)eEntity->v.origin[2]);

		ENTITY_REMOVE(eEntity);
	}

	if(!eEntity->local.style)
		eEntity->local.style = 1;

	if(!eEntity->local.dWait)
		eEntity->local.dWait = 1;

	if(eEntity->local.style != 3)
		eEntity->local.iValue = 0;
	else
		eEntity->local.iValue = 1;

	Entity_SetOrigin(eEntity,eEntity->v.origin);

	eEntity->v.think		= Point_LogicThink;
	eEntity->v.dNextThink	= Server.dTime+1.0f;
}
