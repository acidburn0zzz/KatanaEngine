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

#ifdef GAME_OPENKATANA
// [20/12/2012] Include the Vektar stuff for spawning ~hogsy
#include "openkatana/mode_vektar.h"
#endif

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
	{	"worldspawn",	Server_Spawn		},
	{	"light",		Point_LightSpawn	},	// TODO: This should be made obsolete ~hogsy

	// Area/Group Entities
	{	"area_breakable",	Area_BreakableSpawn	},
	{	"area_button",		Area_ButtonSpawn	},
	{	"area_changelevel",	Area_ChangeLevel	},
	{	"area_climb",		Area_ClimbSpawn		},
	{	"area_door",		Area_DoorSpawn		},
	{	"area_door_rotate",	Area_DoorSpawn		},
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
	{	"point_light",			Point_LightSpawn		},
	{	"point_ambient",		Point_AmbientSpawn		},
	{	"point_bot",			Bot_Spawn				},
	{	"point_damage",			Point_DamageSpawn		},
	{	"point_effect",			Point_EffectSpawn		},
	{	"point_flare",			Point_FlareSpawn		},
	{	"point_item",			Item_Spawn				},
	{	"point_lightstyle",		Point_LightstyleSpawn	},
	{	"point_logic",			Point_LogicSpawn		},
	{	"point_message",		Point_MessageSpawn		},
	{	"point_monster",		Point_MonsterSpawn		},
	{	"point_multitrigger",	Point_MultiTriggerSpawn	},
	{	"point_vehicle",		Point_VehicleSpawn		},
	{	"point_null",			Point_NullSpawn			},
	{	"point_particle",		Point_ParticleSpawn		},
	{	"point_prop",			Point_PropSpawn			},
	{	"point_sound",			Point_SoundSpawn		},
	{	"point_start",			Point_Start				},
	{	"point_shake",			Point_ShakeSpawn		},
	{	"point_teleport",		Point_TeleportSpawn		},
	{	"point_timedtrigger",	Point_TimedTriggerSpawn	},
	{	"point_waypoint",		Point_WaypointSpawn		},

#ifdef GAME_OPENKATANA
	{	"point_decoration",	Point_DecorationSpawn	},	// TODO: Isn't this made redundant by point_prop? ~hogsy
#elif GAME_ADAMAS
#endif

	{	NULL,	NULL	}
};

cvar_t	cvServerPlayerModel		= { "server_playermodel",	"models/player.md2",	false,	true,   "Sets the main server-side player model."	            },
		cvServerRespawnDelay	= { "server_respawndelay",	"40",					false,	true,	"Sets the amount of time until a player respawns."      },
		cvServerSkill			= { "server_skill",			"1",					false,	true,   "The level of difficulty."	                            },
		cvServerSelfDamage		= { "server_selfdamage",	"0",					false,	true,	"If enabled, your weapons can damage you."              },
// [7/12/2012] Changed default maxhealth to 200 ~hogsy
		cvServerMaxHealth		= { "server_maxhealth",		"200",					false,	true,   "Sets the max amount of health."	                    },
		cvServerDefaultHealth	= {	"server_defaulthealth",	"100",					false,	true,   "Changes the default amount of health."	                },
		cvServerMonsters		= { "server_monsters",		"1",					false,	true,   "If enabled, monsters can spawn."	                    },
		cvServerMaxScore		= { "server_maxscore",		"20",					false,	true,	"Max score before the round ends."						},
		cvServerGameMode		= { "server_gamemode",		"0",					false,	true,   "Sets the active mode of play."	                        },
		cvServerGameTime		= {	"server_gametime",		"120",					false,	true,	"Time before the round ends."							},
		cvServerGameClients		= { "server_gameclients",	"2",					false,	true,	"Number of clients before round starts."				},
		cvServerWaypointDelay	= { "server_waypointdelay",	"5.0",					false,	true,   "Delay before attempting to spawn another waypoint."	},
		cvServerWaypointSpawn	= { "server_waypointspawn",	"0",					false,	true,	"if enabled, waypoints autospawn."						},
		cvServerWaypointParse	= { "server_waypointparse",	"",						false,	true,	"Overrides the default path to load waypoints from."	};
// [19/3/2013] Replacement for the engine-side variable ~hogsy
cvar_t	cvServerGravityTweak	= {	"server_gravityamount",	"1.0",					false,	true,	"Gravity modifier."										};
cvar_t	cvServerGravity			= {	"server_gravity",		"600.0",				false,	true,	"Overall gravity."										};
#ifdef GAME_OPENKATANA
// [20/1/2013] By default bots spawn in OpenKatana for both SP and MP ~hogsy
cvar_t	cvServerBots			= {	"server_bots",			"1",					false,	true,	"Can enable and disable bots."							};
#else
cvar_t	cvServerBots			= {	"server_bots",			"0",					false,	true	};
#endif

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

	Server.bActive			= false;						// [7/12/2012] We're not active when we've only just initialized ~hogsy
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

	// Set defaults.
	Server.dWaypointSpawnDelay	= ((double)cvServerWaypointDelay.value);
	Server.bRoundStarted		=
	Server.bPlayersSpawned		= false; // [5/9/3024] Players have no been spawned yet ~hogsy
	Server.iMonsters			= 0;
#ifdef GAME_ADAMAS
	Server.iLives				= 2;
#endif

	Waypoint_Initialize();

	// [19/3/2013] This is... Ugly... ~hogsy
	bIsDeathmatch	=
	bIsCooperative	= false;
	bIsMultiplayer	= true;

	// [19/3/2013] Set up our gamemode ~hogsy
	if(cvServerGameMode.iValue == MODE_DEATHMATCH)
		bIsDeathmatch = true;
	else if(cvServerGameMode.iValue == MODE_COOPERATIVE)
		bIsCooperative = true;
	else if(cvServerGameMode.iValue == MODE_SINGLEPLAYER)
	{
		bIsMultiplayer = false;

		// [5/9/2013] Ooooopsey!!!! Round always starts on singleplayer from spawn ~hogsy
		Server.bRoundStarted = true;
	}

	Engine.Server_PrecacheResource(RESOURCE_SOUND,"misc/deny.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,BASE_SOUND_TALK0);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,BASE_SOUND_TALK1);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,BASE_SOUND_TALK2);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"misc/gib1.wav");

	// Physics
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_SPLASH);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_BODY);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET0);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET1);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET2);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET3);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET4);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET5);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET6);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET7);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET8);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,PHYSICS_SOUND_RICOCHET9);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion3.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion4.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion5.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"fx/explosion6.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND, PHYSICS_SOUND_CONCRETESTEP0);
	Engine.Server_PrecacheResource(RESOURCE_SOUND, PHYSICS_SOUND_CONCRETESTEP1);
	Engine.Server_PrecacheResource(RESOURCE_SOUND, PHYSICS_SOUND_CONCRETESTEP2);
	Engine.Server_PrecacheResource(RESOURCE_SOUND, PHYSICS_SOUND_CONCRETESTEP3);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GIB0);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GIB1);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GIB2);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,PHYSICS_MODEL_GIB3);
	
	// Player
	Engine.Server_PrecacheResource(RESOURCE_MODEL,cvServerPlayerModel.string);

	// Particles
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_BLOOD0);
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_BLOOD1);
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_BLOOD2);
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_BLOOD3);
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_SMOKE0);
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_SMOKE1);
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_SMOKE2);
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,PARTICLE_SMOKE3);

#ifdef GAME_OPENKATANA	// [22/4/2013] OpenKatana specific stuff is now here instead ~hogsy
	// Player
	Engine.Server_PrecacheResource(RESOURCE_SOUND, "player/jump0.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND, "player/jump1.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND, "player/jump2.wav");
	Engine.Server_PrecacheResource(RESOURCE_SOUND, "player/jump3.wav");

	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/blip.md2");

	Engine.Server_PrecacheResource(RESOURCE_SPRITE,"poison");
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,"spark");
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,"spark2");
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,"ice");
	Engine.Server_PrecacheResource(RESOURCE_SPRITE,"zspark");
#endif

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
#ifdef GAME_OPENKATANA
		// [31/7/2012] TODO: We need an md2 version of this! ~hogsy
//		Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/mikiko.mdl");
		Engine.Server_PrecacheResource(RESOURCE_SOUND,"items/respawn.wav");

		// [20/12/2012] If we're in Vektar mode, then spawn the Vektar! ~hogsy
		if(cvServerGameMode.iValue == MODE_VEKTAR)
			Vektar_Spawn();
#endif

		// [31/7/2013] Precache custom player models ~hogsy
		// [31/7/2013] TODO: Get actual current game directory from the engine ~hogsy
		//gFileSystem_ScanDirectory("data/models/player/*.md2",Server_PrecachePlayerModel);
	}

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
*/
void Server_StartFrame(void)
{
#ifdef GAME_OPENKATANA
	Deathmatch_Frame();
#elif GAME_ADAMAS
	// This is stupid... ~hogsy
	if(!Server.iMonsters && Server.bRoundStarted)
	{
		if(strstr(Engine.Server_GetLevelName(),"0"))
			Engine.Server_ChangeLevel("room1");
		else if(strstr(Engine.Server_GetLevelName(),"1"))
			Engine.Server_ChangeLevel("room2");
		else
		{
			Engine.Server_BroadcastPrint("You Win!!\n");
			Engine.Server_ChangeLevel("room0");
		}
	}
#endif
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
	if(eClient->monster.iState != STATE_DEAD)
		MONSTER_Damage(eClient,eClient,eClient->v.iHealth,0);
}

/*	General function for globally updating the HUD for clients.
*/
void Server_UpdateClientMenu(edict_t *eClient,int iMenuState,bool bShow)
{
	// [5/8/2013] This is who we're telling to hide/show their HUD ~hogsy
	Engine.SetMessageEntity(eClient);

	Engine.WriteByte(MSG_ONE,SVC_UPDATEMENU);
	Engine.WriteByte(MSG_ONE,iMenuState);
	Engine.WriteByte(MSG_ONE,bShow);
}
