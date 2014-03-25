/*	Copyright (C) 2011-2013 OldTimes Software
*/
#include "server_main.h"

/*
	This is where most functions between the Game and Engine can be found along
	with the main list of entities that's parsed upon spawning a server.
*/

#include "server_waypoint.h"
#include "server_weapon.h"
#include "server_item.h"

void Server_Spawn(edict_t *ent);

typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} SpawnList_t;

/*	[28/11/2012]
	Removed info_player_start for good. ~hogsy
	[29/11/2012]
	Commented out point_dynamiclit. ~hogsy
*/
SpawnList_t SpawnList[] =
{
	{	"worldspawn",	Server_Spawn	},
	{	"light",		light			},

	// Area/Group Entities
	{	"area_breakable",	Area_BreakableSpawn	},
	{	"area_button",		Area_ButtonSpawn	},
	{	"area_changelevel",	Area_ChangeLevel	},
	{	"area_climb",		Area_ClimbSpawn		},
	{	"area_door",		Area_DoorSpawn		},
	{	"area_door_rotate",	Point_NullSpawn		},
	{	"area_noclip",		Area_NoclipSpawn	},
	{	"area_push",		Area_PushSpawn		},
	{	"area_debris",		Area_PushableSpawn	},	// [25/9/2013] For compatability ~hogsy
	{	"area_pushable",	Area_PushableSpawn	},
	{	"area_platform",	Area_PlatformSpawn	},
	{	"area_rotate",		Area_RotateSpawn	},
	{	"area_trigger",		Area_TriggerSpawn	},
	{	"area_wall",		Area_WallSpawn		},
	{	"area_playerspawn",	Area_PlayerSpawn	},

	// Point Entities
	{	"point_ambient",		Point_AmbientSpawn		},
	{	"point_bot",			Bot_Spawn			},
	{	"point_damage",			Point_DamageSpawn		},
	{	"point_effect",			Point_EffectSpawn		},
	{	"point_flare",			Point_FlareSpawn		},
	{	"point_item",			Item_Spawn			},
	{	"point_lightstyle",		Point_LightstyleSpawn		},
	{	"point_logic",			Point_LogicSpawn		},
	{	"point_message",		Point_MessageSpawn		},
	{	"point_monster",		Point_MonsterSpawn		},
	{	"point_multitrigger",		Point_MultiTriggerSpawn		},
	{	"point_vehicle",		Point_VehicleSpawn		},
	{	"point_null",			Point_NullSpawn			},
	{	"point_particle",		Point_ParticleSpawn		},
	{	"point_prop",			Point_PropSpawn			},
	{	"point_sound",			Point_SoundSpawn		},
	{	"point_start",			Point_Start			},
	{	"point_shake",			Point_ShakeSpawn		},
	{	"point_teleport",		Point_TeleportSpawn		},
	{	"point_timedtrigger",		Point_TimedTriggerSpawn		},
	{	"point_waypoint",		Point_WaypointSpawn		},

	{	NULL,	NULL	}
};

cvar_t	cvServerPlayerModel	= { 	"server_playermodel",	"models/player.md2",	false,	true	};
cvar_t	cvServerRespawnDelay	= { 	"server_respawndelay",	"40",			false,	true	};
cvar_t	cvServerSkill		= { 	"server_skill",		"1",			false,	true	};
cvar_t	cvServerSelfDamage	= { 	"server_selfdamage",	"0",			false,	true	};
// [7/12/2012] Changed default maxhealth to 200 ~hogsy
cvar_t	cvServerMaxHealth	= { 	"server_maxhealth",	"200",			false,	true	};
cvar_t	cvServerDefaultHealth	= {	"server_defaulthealth",	"100",			false,	true	};
cvar_t	cvServerMonsters	= { 	"server_monsters",	"1",			false,	true	};
cvar_t	cvServerMaxScore	= { 	"server_maxscore",	"20",			false,	true	};
cvar_t	cvServerGameMode	= { 	"server_gamemode",	"0",			false,	true	};
cvar_t	cvServerGameTime	= {	"server_gametime",	"120",			false,	true	};
cvar_t	cvServerGameClients	= { 	"server_gameclients",	"2",			false,	true	};
cvar_t	cvServerWaypointDelay	= { 	"server_waypointdelay",	"5.0",			false,	true	};
cvar_t	cvServerWaypointSpawn	= { 	"server_waypointspawn",	"1",			false,	true	};
cvar_t	cvServerWaypointParse	= {	"server_waypointparse",	"",			false,	true	};
// [19/3/2013] Replacement for the engine-side variable ~hogsy
cvar_t	cvServerGravityTweak	= {	"server_gravityamount",	"1.0",			false,	true	};
cvar_t	cvServerGravity		= {	"server_gravity",	"600.0",		false,	true	};
cvar_t	cvServerBots		= {	"server_bots",		"0",			false,	true	};

void Server_SetGameMode(void)
{
	// [7/12/2012] Don't continue if we're already using this mode ~hogsy
	if(Server.iLastGameMode == (int)cvServerGameMode.value)
		return;

	if(cvServerGameMode.value >= MODE_NONE || cvServerGameMode.value < MODE_SINGLEPLAYER)
	{
		Engine.Con_Warning("Attempted to set unknown game mode! Reverting to singleplayer.\n");
		Engine.Cvar_SetValue("server_gamemode",MODE_SINGLEPLAYER);
	}

	// [5/6/2012] Moved so we give this message if the above case occurs ~hogsy
	if(Server.iLastGameMode != cvServerGameMode.value && Server.bActive)
		Engine.Con_Printf("Gamemode will be changed on next map.\n");

	// [29/3/2012] Keep OldMode up to date! ~hogsy
	Server.iLastGameMode = (int)cvServerGameMode.value;
}

/*	Called by the engine.
*/
void Server_Initialize(void)
{
	Engine.Cvar_RegisterVariable(&cvServerSkill,NULL);
	Engine.Cvar_RegisterVariable(&cvServerSelfDamage,NULL);
	Engine.Cvar_RegisterVariable(&cvServerMaxHealth,NULL);
	Engine.Cvar_RegisterVariable(&cvServerMonsters,NULL);
	Engine.Cvar_RegisterVariable(&cvServerMaxScore,NULL);
	Engine.Cvar_RegisterVariable(&cvServerGameMode,Server_SetGameMode);
	Engine.Cvar_RegisterVariable(&cvServerPlayerModel,NULL);
	Engine.Cvar_RegisterVariable(&cvServerWaypointDelay,NULL);
	Engine.Cvar_RegisterVariable(&cvServerWaypointSpawn,NULL);
	Engine.Cvar_RegisterVariable(&cvServerBots,NULL);
	Engine.Cvar_RegisterVariable(&cvServerDefaultHealth,NULL);
	Engine.Cvar_RegisterVariable(&cvServerGameTime,NULL);
	Engine.Cvar_RegisterVariable(&cvServerGameClients,NULL);
	Engine.Cvar_RegisterVariable(&cvServerGravityTweak,NULL);
	Engine.Cvar_RegisterVariable(&cvServerGravity,NULL);
	Engine.Cvar_RegisterVariable(&cvServerRespawnDelay,NULL);

	Server.bActive		= false;						// [7/12/2012] We're not active when we've only just initialized ~hogsy
	Server.iLastGameMode	= (int)cvServerGameMode.value;
}

/*	Used just for GIPL shit... Bleh...
*/
void Server_PrecachePlayerModel(const char *ccFile)
{
	Engine.Con_Printf("EX: %s\n",ccFile);

	Engine.Server_PrecacheResource(RESOURCE_MODEL,ccFile);
}

void Server_Spawn(edict_t *ent)
{
	Server.eWorld = ent;

	// Set defaults
	Server.dWaypointSpawnDelay	= ((double)cvServerWaypointDelay.value);
	Server.bRoundStarted		=
	Server.bPlayersSpawned		= false; // [5/9/3024] Players have no been spawned yet ~hogsy

	Waypoint_Initialize();

	// [19/3/2013] This is... Ugly... ~hogsy
	bIsDeathmatch	=
	bIsCooperative	= false;
	bIsMultiplayer	= true;

	// [19/3/2013] Set up our gamemode ~hogsy
	if(cvServerGameMode.value == MODE_DEATHMATCH)
		bIsDeathmatch = true;
	else if(cvServerGameMode.value == MODE_COOPERATIVE)
		bIsCooperative = true;
	else if(cvServerGameMode.value == MODE_SINGLEPLAYER)
	{
		bIsMultiplayer = false;

		// [5/9/2013] Ooooopsey!!!! Round always starts on singleplayer from spawn ~hogsy
		Server.bRoundStarted = true;
	}

	Engine.Server_PrecacheResource(RESOURCE_SOUND,"misc/deny.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"misc/talk1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"misc/talk2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"misc/talk3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"misc/gib1.wav");

	// Physics
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_SPLASH);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_BODY);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet4.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet5.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet6.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet7.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet8.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet9.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/ricochet10.wav");

	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion4.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion5.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion6.wav");

	// [15/7/2012] Precache gib models ~hogsy
	// [13/9/2012] Updated paths ~hogsy
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/gib0.md2");
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/gib1.md2");
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/gibs/gib2.md2");

#ifdef OPENKATANA	// [22/4/2013] OpenKatana specific stuff is now here instead ~hogsy
	// [29/7/2012] Player sounds ~hogsy
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerjump5.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerjump6.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerjump7.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/acroboost.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerlandhurt.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerlandhurt2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerswim1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerpain1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerpain2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerpain3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerpain4.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerdeath1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerdeath2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerdeath3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerdeath4.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerwaterdeath.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerland1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerland2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerland3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerland4.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerstep1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerstep2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerstep3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerstep4.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/playerexitwater.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/gasp2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/h2ojump.wav");

	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/blip.md2");
	// [4/2/2013] Moved here since he's used in singleplayer too :) ~hogsy
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/sprfly.md2");
#endif

	Engine.Server_PrecacheResource(RESOURCE_MODEL,cvServerPlayerModel.string);

	// [10/4/2013] Waypoint debugging models ~hogsy
#ifdef DEBUG_WAYPOINT
	Engine.Server_PrecacheResource(RESOURCE_MODEL,WAYPOINT_MODEL_BASE);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,WAYPOINT_MODEL_CLIMB);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,WAYPOINT_MODEL_ITEM);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,WAYPOINT_MODEL_JUMP);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,WAYPOINT_MODEL_SPAWN);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,WAYPOINT_MODEL_SWIM);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,WAYPOINT_MODEL_WEAPON);
#endif

	// [7/3/2012] Added mode specific precaches ~hogsy
	// [31/7/2012] Changed so we precache these if it's multiplayer ~hogsy
	if(bIsMultiplayer)
	{
		// [31/7/2013] Precache custom player models ~hogsy
		// [31/7/2013] TODO: Get actual current game directory from the engine ~hogsy
		//gFileSystem_ScanDirectory("data/models/player/*.md2",Server_PrecachePlayerModel);
	}

	// [21/3/2012] Updated ~hogsy
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"blood");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"poison");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"smoke0");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"smoke1");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"smoke2");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"smoke3");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"spark");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"spark2");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"ice");
	Engine.Server_PrecacheResource(RESOURCE_PARTICLE,"zspark");

	Weapon_Precache();

	Engine.LightStyle(0,"m");
	Engine.LightStyle(1,"mmnmmommommnonmmonqnmmo");
	Engine.LightStyle(2,"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	Engine.LightStyle(3,"mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	Engine.LightStyle(4,"mamamamamama");
	Engine.LightStyle(5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	Engine.LightStyle(6,"nmonqnmomnmomomno");
	Engine.LightStyle(7,"mmmaaaabcdefgmmmmaaaammmaamm");
	Engine.LightStyle(8,"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	Engine.LightStyle(9,"aaaaaaaazzzzzzzz");
	Engine.LightStyle(10,"mmamammmmammamamaaamammma");
	Engine.LightStyle(11,"abcdefghijklmnopqrrqponmlkjihgfedcba");
	Engine.LightStyle(32,"a");
}

/*	Called by the engine.
*/
bool Server_SpawnEntity(edict_t *ent)
{
	SpawnList_t *slSpawn;

	if(!ent->v.cClassname)
	{
		Engine.Con_Warning("Failed to get classname!\n");
		return false;
	}

	for(slSpawn = SpawnList; slSpawn->name; slSpawn++)
		if(!strcmp(slSpawn->name,ent->v.cClassname))
		{
			slSpawn->spawn(ent);
			return true;
		}

	Engine.Con_Warning("Entity doesn't have a spawn function! (%s)\n",ent->v.cClassname);
	return false;
}

/*	Called by the engine.
	Called per-frame for each entity just before physics.
*/
void Server_EntityFrame(edict_t *eEntity)
{
	Waypoint_Frame(eEntity);
	Monster_Frame(eEntity);
}

/*	Called by the engine.
	Called for the "kill" command.
*/
void Server_KillClient(edict_t *eClient)
{
	if(eClient->v.deadflag != DEAD_DEAD)
		MONSTER_Damage(eClient,eClient,eClient->v.iHealth);
}

/*	General function for globally updating the HUD for clients.
	If the entity comes back empty then we'll just tell everyone...
*/
void Server_UpdateClientMenu(edict_t *eClient,int iMenuState,bool bShow)
{
	// [5/8/2013] This is who we're telling to hide/show their HUD ~hogsy
	Engine.SetMessageEntity(eClient);

	Engine.WriteByte(MSG_ONE,SVC_UPDATEMENU);
	Engine.WriteByte(MSG_ONE,iMenuState);
	Engine.WriteByte(MSG_ONE,bShow);
}
