/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __GAMEMAIN__
#define __GAMEMAIN__

/*	
	Main header for our game module.
*/

#include "platform.h"

#include "shared_flags.h"
#include "shared_module.h"
#include "shared_math.h"
#include "shared_menu.h"
#include "shared_formats.h"
#include "shared_game.h"

#include "game_resources.h"

extern ModuleImport_t	Engine;

typedef struct
{
	bool		bActive;                // Is the server active?

	double		dTime,			// Server time.
			dWaypointSpawnDelay;	// Delay before spawning another waypoint.

	edict_t		*eEntity,		// Current player.
			*eWorld;

	char		*cMapAuthor,            // Map author.
			*cMapTitle;             // Map title.

	int		iLastGameMode,          // The last active gamemode.
			iClients;               // Number of connected clients.

	// Gamemode
	bool		bRoundStarted,		// Has the round started yet?
			bPlayersSpawned;	// Have the players been spawned for the current mode?
} GameServer_t;

typedef struct
{
	double		time;

	entity_t	*ent;
} GameClient_t;

GameServer_t Server;
GameClient_t Client;

// [29/7/2013] Moved mode types into server_mode.h ~hogsy

// [28/7/2013] Moved door states into sv_area.c ~hogsy

#define	DEAD_NO			0	// Entity isn't dead
#define DEAD_DEAD		2	// Entity is dead
#define DEAD_RESPAWNABLE	3	// Entity can respawn

/*
	Weapons
*/

// [5/8/2013] Made these standard defines since maps rely on these now ~hogsy
#ifdef GAME_EXAMPLE
#define WEAPON_LASERS		1	// NPC weapon
#endif

int		iRedScore,iBlueScore;

void Flare(vec3_t org,float r,float g,float b,float a,float scale,char *texture);

void Sound(edict_t *ent,int channel,char *sound,int volume,float attenuation);

void SetAngle(edict_t *ent,vec3_t vAngle);

void DrawPic(char *texture,float alpha,int x,int y,int h,int w);
void PutClientInServer(edict_t *ent);
void WriteByte(int mode,int c);
void ChangeYaw(edict_t *ent);

trace_t Traceline(edict_t *ent,vec3_t vStart,vec3_t vEnd,int type);

void UseTargets(edict_t *ent, edict_t *other);

edict_t *Spawn(void);

#endif
