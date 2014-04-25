/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SERVERPLAYER__
#define __SERVERPLAYER__

#include "server_main.h"

int	iPlayerModelIndex;

extern	EntityFrame_t	PlayerAnimation_Death1[];
extern	EntityFrame_t	PlayerAnimation_Death2[];
extern	EntityFrame_t	PlayerAnimation_Fire[];
extern	EntityFrame_t	PlayerAnimation_Idle[];
extern	EntityFrame_t	PlayerAnimation_Jump[];
extern	EntityFrame_t	PlayerAnimation_RunJump[];
extern	EntityFrame_t	PlayerAnimation_Walk[];
extern	EntityFrame_t	PlayerAnimation_KatanaAttack1[];
extern	EntityFrame_t	PlayerAnimation_KatanaAttack2[];

void Player_PostThink(edict_t *ePlayer);
void Player_PreThink(edict_t *ent);
void Player_Spawn(edict_t *self);
void Player_Jump(edict_t *self);
#ifdef OPENKATANA
void Player_CheckPowerups(edict_t *self);
#endif
void Player_DeathThink(edict_t *ent);
void Player_Use(edict_t *ePlayer);

#endif