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
	bool			bActive;                // Is the server active?

	// [10/4/2013] Changed from a float to a double ~hogsy
	double			dTime,					// Server time.
					dWaypointSpawnDelay;	// Delay before spawning another waypoint.

	edict_t			*eEntity,				// Current player.
					*eWorld;

	char			*cMapAuthor,            // Map author.
					*cMapTitle;             // Map title.

	int				iLastGameMode,          // The last active gamemode.
					iClients,               // Number of connected clients.
					iMonsters;				// Number of monsters within the level.

	// Gamemode
	bool			bRoundStarted,			// Has the round started yet?
					bPlayersSpawned;		// Have the players been spawned for the current mode?

#ifdef GAME_ADAMAS
	int				iLives;					// Players current number of lives (shared).
#endif
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

#define	DEAD_NO				0	// Entity isn't dead
#define DEAD_DEAD			2	// Entity is dead
#define DEAD_RESPAWNABLE	3	// Entity can respawn

/*
	Weapons
*/

#define	WEAPON_NONE		-1

// [5/8/2013] Made these standard defines since maps rely on these now ~hogsy
#ifdef OPENKATANA
#define WEAPON_LASERS		1	// NPC weapon
#define WEAPON_KATANA		2	// Simple Katana / Melee
#define	WEAPON_DAIKATANA	3	// The Daikatana / Melee+
#define	WEAPON_IONBLASTER	4	// Basic projectile-based weapon
#define	WEAPON_C4VIZATERGO	5	// Explosive projectile weapon
#define	WEAPON_SHOTCYCLER	6
#define	WEAPON_SIDEWINDER	7	// Explosive projectile weapon (+)
#define	WEAPON_SHOCKWAVE	8	// Powerful Shockwave Rifle
#define	AMMO_IONBALLS		9
#define	AMMO_C4BOMBS		10
#define	AMMO_SLUGS			11
#define	AMMO_WINDROCKET		12
#define	AMMO_SHOCKWAVE		13
#define	ITEM_ATTACKBOOST	14
#define	ITEM_POWERBOOST		15
#define	ITEM_VITABOOST		16
#define	ITEM_SPEEDBOOST		17
#define	ITEM_ACROBOOST		18
#define	ITEM_HEALTHKIT		19
#define	ITEM_PLASTEELARMOR	20
#define	ITEM_ENVIROSUIT		21
#define	ITEM_OXYLUNG		22
// Episode Four
#define	WEAPON_GLOCK		50
#define	WEAPON_IONRIFLE		70	// Talon Brave's weapon
#elif GAME_ADAMAS
#define WEAPON_BLAZER	1
#define	ITEM_LIFE		2
#endif
#define ITEM_FLAG			1000

int		iRedScore,iBlueScore;

char *va(char *format,...);

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
