/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SERVERMODE__
#define __SERVERMODE__

#include "server_main.h"

enum
{
	MODE_SINGLEPLAYER,			// Just one player playing
	MODE_COOPERATIVE,			// Two players working together through the main game
	MODE_DEATHMATCH,			// Competitive online fighting
	MODE_CAPTURETHEFLAG,		// Competitive online capture the flag
	MODE_VEKTAR,				// Vektar mode!

	MODE_NONE					// No gametype, used for checks
};

// Deathmatch
void Deathmatch_Spawn(edict_t *ePlayer);
void Deathmatch_Frame(void);

bool	bIsMultiplayer,
		bIsCooperative,
		bIsDeathmatch;

#endif