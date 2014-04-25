/*	Copyright (C) 2011-2013 OldTimes Software
*/
#include "server_main.h"

/*
	Katana Bot
	Used as companions in single-player
	and as opponents in multi-player.
	Server admins can modify any of this
	to their desire.

	TODO:
		Parse a file for phrases and names?

		Create a WAYPOINT_SPAWN at the point
		we spawn if there isn't one already.
		We'll check for this whenever we're
		moving later on to make sure we don't
		risk getting "spawn gibbed" by someone.
	~hogsy
*/

#include "server_monster.h"

#include "server_waypoint.h"
#include "server_weapon.h"
#include "server_player.h"

#define	BOT_MAX_HEALTH	100
#define	BOT_MAX_SIGHT	900.0f
#define BOT_MIN_HEALTH	-20		// [2/1/2013] Same as in server_player ~hogsy

// [15/7/2012] List of death phrases ~hogsy
char *BotDeathPhrases[] =
{
	"You got lucky, punk.\n",
	"%s is a dirty hacker!\n",
	"Looks like %s is cheating!\n",
	"Lucky shot, %s.\n",
	"I'll get you for this!\n",
	"%s should be banned.\n",
	"That was unfair, %s!\n",
	"%s has an unfair advantage.\n",
	"Please ban %s!\n",
	"I hit you like 5 times, %s!\n"
};

// [15/7/2012] List of kill phrases ~hogsy
char *BotKillPhrases[] =
{
	"Suck it down!\n",
	"Messed %s up, hah!\n",
	"I just made %s my bitch.\n",
	"Looks like it's not your day, %s.\n",
	"Baw, %s is dead... Oh well.\n",
	"Owned.\n",
	"%s licked my shoe!\n",
	"%s just lost his piece.\n",
	"Learn to play, %s!\n",
	"You all suck, especially %s.\n"
};

// [17/7/2012] Emotion based phrases ~hogsy
char *BotBoredPhrases[] =
{
	"Ugh this is boring...\n",
	"Are we playing yet?\n"
};

// [17/7/2012] Emotion based phrases ~hogsy
char *BotAngryPhrases[] =
{
	"Argh!!\n",
	"I'm going to kill you!!!\n"
};

// [4/10/2012] Emotion based phrases ~hogsy
char *BotFearPhrases[] =
{
	"Hold up! Wait!! Go easy on me, %s\n",
	"You're too strong! I can't do this...\n"
};

// [4/10/2012] Emotion based phrases ~hogsy
char *BotSurprisePhrases[] =
{
	"Woah! Snuck up on me there, %s ;)\n",
	"Haha you almost scared me for a moment there.\n"
};

// [4/10/2012] Emotion based phrases ~hogsy
char *BotJoyPhrases[] =
{
	"This is pretty fun!\n",
	"Nothing can stop me :)\n"
};

// [15/7/2012] List of bot names ~hogsy
// [29/7/2012] Reorganised ~hogsy
char *BotNames[] =
{
	"[BOT] Mercury",
	"[BOT] Atlas",
	"[BOT] Apollo",
	"[BOT] Hector",
	"[BOT] Prometheus",
	"[BOT] Athena",
	"[BOT] Styx",
	"[BOT] Erebos",
	"[BOT] Hecate",
	"[BOT] Oceanus",
	"[BOT] Chaos",
	"[BOT] Odysseus",
	"[BOT] Theseus",
	"[BOT] Castor"
};

enum
{
	BOT_DEFAULT,
#ifdef OPENKATANA
	BOT_MIKIKO,
	BOT_SUPERFLY
#endif
};

void Bot_Run(edict_t *ent);
void Bot_Pain(edict_t *ent,edict_t *other);
void Bot_Die(edict_t *ent,edict_t *other);
void Bot_Stand(edict_t *eBot);
void Bot_Walk(edict_t *eBot);
void Bot_Think(edict_t *eBot);

/*	style
		0	Mikiko
		1	Superfly
*/
void Bot_Spawn(edict_t *eBot)
{
	int		iSpawnType;
	edict_t	*eSpawnPoint;

	// [20/1/2013] Don't spawn bots unless it's allowed by admin ~hogsy
	if(!cvServerBots.value)
		return;

	Math_VectorClear(eBot->v.velocity);

	// [29/7/2012] Names are now set here, the otherway was dumb ~hogsy
	switch(eBot->local.style)
	{
	case BOT_DEFAULT:
		iSpawnType = INFO_PLAYER_DEATHMATCH;

		eBot->v.model	= cvServerPlayerModel.string;
		eBot->v.netname	= BotNames[rand()%sizeof(BotNames)];
		break;
#ifdef OPENKATANA
	case BOT_MIKIKO:
		iSpawnType = INFO_PLAYER_MIKIKO;

		eBot->v.model	= "models/mikiko.md2";
		eBot->v.netname	= "Mikiko Ebihara";
		break;
	case BOT_SUPERFLY:
		iSpawnType = INFO_PLAYER_SUPERFLY;

		Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/superfly/superflydeath1.wav");
		Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/superfly/superflydeath2.wav");
		Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/superfly/superflydeath3.wav");
		Engine.Server_PrecacheResource(RESOURCE_SOUND,"player/superfly/superflydeath4.wav");

		eBot->v.model	= "models/sprfly.md2";
		eBot->v.netname	= "Superfly Johnson";
		break;
#endif
	default:
		// [22/3/2013] Removed multiplayer support ~hogsy
		Engine.Con_Warning("Attempted to spawn unknown bot type! (%i) (%i %i %i)\n",
			eBot->local.style,
			(int)eBot->v.origin[0],
			(int)eBot->v.origin[1],
			(int)eBot->v.origin[2]);

		ENTITY_REMOVE(eBot);
	}

	eBot->v.cClassname	= "bot";
	eBot->v.iHealth		= 100;
	eBot->v.iMaxHealth	= (int)cvServerMaxHealth.value;
	eBot->v.movetype	= MOVETYPE_STEP;

	eBot->Physics.iSolid = SOLID_SLIDEBOX;

	eBot->local.bBleed	= true;

	// [16/7/2012] Must be set after teams are set up ~hogsy
	eSpawnPoint = Entity_SpawnPoint(eBot,iSpawnType);
	if(!eSpawnPoint)
	{
		Engine.Con_Warning("%s failed to find spawnpoint!\n",eBot->v.netname);
		ENTITY_REMOVE(eBot);
	}

	// [15/7/2012] Set the initial state to awake ~hogsy
	eBot->monster.think_die		= Bot_Die;
	eBot->monster.think_pain	= Bot_Pain;
	eBot->monster.Think			= Bot_Think;
	eBot->monster.iType			= MONSTER_PLAYER;

	Entity_SetModel(eBot,eBot->v.model);
	Entity_SetOrigin(eBot,eSpawnPoint->v.origin);
	SetAngle(eBot,eSpawnPoint->v.angles);
	Entity_SetSize(eBot,-16.0f,-16.0f,-24.0f,16.0f,16.0f,32.0f);

	Monster_SetState(eBot,STATE_AWAKE);
	Monster_SetThink(eBot,THINK_IDLE);

	// [6/8/2012] Make sure we're not in the air ~hogsy
	DropToFloor(eBot);
}

void Bot_Think(edict_t *eBot)
{
	switch(eBot->monster.iThink)
	{
	case THINK_IDLE:
#if 0
		char	cJumpSound[128];

		// [4/2/2013] TEMP: Some test stuff for monster jumping... LOL ~hogsy
		if(!(eBot->v.flags & FL_ONGROUND) || eBot->v.iHealth <= 0)
			return;

		Monster_Jump(eBot,200.0f);

		sprintf(cJumpSound,"player/playerjump%i.wav",5+(rand()%3));

		Sound(eBot,CHAN_VOICE,cJumpSound,255,ATTN_NORM);
#else
		Monster_SetTargets(eBot);
#endif
		break;
	case THINK_WANDERING:
		{
			edict_t		*eTarget;
			Waypoint_t	*wPoint;
//			Weapon_t	*wMyWeapon;
//			vec3_t		vAngle;

			eTarget = Monster_GetTarget(eBot);
			if(eTarget)
			{
				if(Monster_GetRelationship(eBot,eTarget) == RELATIONSHIP_HATE)
				{
					// [22/3/2013] Begin attacking next frame ~hogsy
					Monster_SetThink(eBot,THINK_ATTACKING);
					return;
				}
			}

			if(!eBot->monster.vTarget)
			{
				// [28/7/2012] TODO: Find specific waypoint such as an item ~hogsy
				wPoint = Waypoint_GetByVisibility(eBot->v.origin);
				if(wPoint)
				{
					if(wPoint->bOpen)
					{
						// [22/3/2013] TODO: Tell that current entity it's time to move... ~hogsy
					}

					Math_VectorCopy(wPoint->position,eBot->monster.vTarget);
				}
			}

#if 0
			wMyWeapon = Weapon_GetCurrentWeapon(eBot);
			if(MONSTER_GetRange(eBot,eBot->v.enemy->v.origin) > 4000)
				return;
			else if(wMyWeapon->iPrimaryType == AM_MELEE && MONSTER_GetRange(eBot,eBot->v.enemy->v.origin) > MONSTER_RANGE_MELEE)
				return;
			else if(Monster_IsVisible(eBot,eBot->v.enemy))
			{
				// [5/8/2012] No ammo and it's not a melee weapon? ~hogsy
				if(!Weapon_CheckPrimaryAmmo(eBot) && wMyWeapon->iPrimaryType != AM_MELEE)
				{
					// [5/8/2012] Should probably flee ~hogsy
					Monster_SetThink(eBot,THINK_FLEEING);
					return;
				}

				Math_VectorSubtract(eBot->v.enemy->v.origin,eBot->v.origin,vAngle);

		//		ent->v.ideal_yaw	= VectorToAngles(vAngle);

				ChangeYaw(eBot);
			}
#endif
		}
		break;
	}
}

void Bot_BroadcastMessage(edict_t *eBot,edict_t *other)
{
	char *cPhrase;

	if(!bIsMultiplayer || rand()%5 == 5)
		return;

	if(eBot->v.iHealth <= 0)
		cPhrase = BotDeathPhrases[(rand()%sizeof(BotDeathPhrases))];
	else
	{
		if(eBot->monster.fEmotion[EMOTION_ANGER] > 5.0f)
			cPhrase = BotAngryPhrases[(rand()%sizeof(BotAngryPhrases))];
		else if(eBot->monster.fEmotion[EMOTION_BOREDOM] > 5.0f)
			cPhrase = BotBoredPhrases[(rand()%sizeof(BotBoredPhrases))];
		else if(eBot->monster.fEmotion[EMOTION_FEAR] > 5.0f)
			cPhrase = BotFearPhrases[(rand()%sizeof(BotFearPhrases))];
		else if(eBot->monster.fEmotion[EMOTION_JOY] > 5.0f)
			cPhrase = BotJoyPhrases[(rand()%sizeof(BotJoyPhrases))];
		else
			// [22/3/2013] Emotions don't give us anything worth saying... ~hogsy
			return;
	}

	// [17/7/2012] Temporary until we can simulate chat ~hogsy
	Engine.Server_BroadcastPrint
	(
		"%s: %s\n",
		eBot->v.netname,
		cPhrase,
		other->v.netname
	);
}

void Bot_Pain(edict_t *ent,edict_t *other)
{
	char		sound[MAX_QPATH];
	Weapon_t	*wMyWeapon,*wHisWeapon;

	if(ent->monster.iState == STATE_ASLEEP)
	{
		// [5/8/2012] Wake us up ~hogsy
		Monster_SetState(ent,STATE_AWAKE);
		// [14/9/2012] Immediately run if we were asleep since we'd be in shock! ~hogsy
		Monster_SetThink(ent,THINK_FLEEING);
		return;
	}

	// [4/10/2012] Let the player know how we're feeling :) ~hogsy
	Bot_BroadcastMessage(ent,other);

	// [14/9/2012] Get both mine and our enemies weapon ~hogsy
	wMyWeapon	= Weapon_GetCurrentWeapon(ent);
	wHisWeapon	= Weapon_GetCurrentWeapon(other);
	if(!wMyWeapon || (!Weapon_CheckPrimaryAmmo(wMyWeapon,ent) && wMyWeapon->iPrimaryType != AM_MELEE))
	{
		Monster_SetThink(ent,THINK_FLEEING);
		return;
	}
	// [14/9/2012] Otherwise check what we can see our enemy having (don't check ammo since it's unrealistic) ~hogsy
	else if(!wHisWeapon || wHisWeapon->iPrimaryType == AM_MELEE)
	{
		// [14/9/2012] We see you!! ~hogsy
		if(Monster_IsVisible(ent,other))
		{
			Monster_SetEnemy(other);
			Monster_SetThink(ent,THINK_ATTACKING);
			return;
		}
	}

#if 0
	// [16/7/2012] Moved to avoid a situation where we try fleeing without a target ~hogsy
	if(	(ent->v.enemy && other->v.iHealth > ent->v.enemy->v.iHealth) ||
		!ent->v.enemy)
		// [15/7/2012] Reckless but change our target to this new enemy ~hogsy
		ent->v.enemy = other;

	mywep	= Weapon_GetCurrentWeapon(ent);
	wHisWeapon	= Weapon_GetCurrentWeapon(other);

	// [15/7/2012] Check that I have a weapon and ammo ~hogsy
	if(!mywep || ent->v.currentammo <= 0)
	{
		// [15/7/2012] I don't! Find the closest waypoint to move back to a little ~hogsy
		point = Waypoint_GetByVisibility(ent->v.origin);
POINTCHECK:
		if(!point)
			return;
		if(	point->type	== WAYPOINT_WEAPON	||
			point->type == WAYPOINT_COVER	||
			point->type == WAYPOINT_DEFAULT)
		{
			Monster_SetThink(ent,MONSTER);

			// [15/7/2012] Set the position we'll move to next ~hogsy
			Math_VectorCopy(ent->monster.target,point->position);
		}
		else
		{
			// [15/7/2012] Keep on checking until we get something ~hogsy
			if(pointchecks >= 5)
			{
				// [15/7/2012] Didn't get anything... ~hogsy
				return;
			}
			pointchecks++;

			point = Waypoint_GetByVisibility(point->position);
			goto POINTCHECK;
		}

		return;
	}
	// [15/7/2012] Okay, he doesn't have a weapon ~hogsy
	else if(!wHisWeapon)
		// [15/7/2012] Since it's probably not even a player ~hogsy
		return;
#endif

#ifdef OPENKATANA
	// [26/7/2012] Character-based sounds ~hogsy
	// [29/7/2012] Revised ~hogsy
	switch(ent->local.style)
	{
	case BOT_MIKIKO:
		if(ent->v.watertype == BSP_CONTENTS_WATER && ent->v.waterlevel == 3)
		{
		}
		else
			sprintf(sound,"player/mikiko/mikikodeath%i.wav",rand()%5+1);
		break;
	case BOT_SUPERFLY:
		if(ent->v.watertype == BSP_CONTENTS_WATER && ent->v.waterlevel == 3)
		{
		}
		else
			sprintf(sound,"player/superfly/superflydeath%i.wav",rand()%4+1);
		break;
	default:
#endif
		if(ent->v.watertype == BSP_CONTENTS_WATER && ent->v.waterlevel == 3)
		{
			if(rand()%2 == 1)
				sprintf(sound,"player/playerchoke1.wav");
			else
				sprintf(sound,"player/playerchoke3.wav");
		}
		else
			sprintf(sound,"player/playerpain%i.wav",rand()%4+1);
#ifdef OPENKATANA
	}
#endif

	Sound(ent,CHAN_VOICE,sound,255,ATTN_NORM);
}

void Bot_Walk(edict_t *eBot)
{
	eBot->v.velocity[0] -= 5.0f;
	eBot->v.velocity[1] -= 10.0f;

	if(eBot->monster.fEmotion[EMOTION_BOREDOM] <= 0)
		Monster_SetThink(eBot,THINK_IDLE);
}

void Bot_Die(edict_t *eBot,edict_t *eOther)
{
	char sound[MAX_QPATH];

	if(eBot->monster.iState == STATE_DEAD)
		return;

	// [15/7/2012] He's dead, Jim. ~hogsy
	Monster_SetState(eBot,STATE_DEAD);

	eBot->v.movetype	= MOVETYPE_TOSS;
	eBot->v.flags		-= (eBot->v.flags & FL_ONGROUND);
	eBot->v.angles[0]	= eBot->v.angles[2] = 0;

	eBot->Physics.iSolid	= SOLID_NOT;

	if(rand()%2)
		Entity_Animate(eBot,PlayerAnimation_Death1);
	else
		Entity_Animate(eBot,PlayerAnimation_Death2);

	// [4/10/2012] Let the player know how we're feeling :) ~hogsy
	Bot_BroadcastMessage(eBot,eOther);

#ifdef OPENKATANA
	// [26/7/2012] Character-based sounds ~hogsy
	switch(eBot->local.style)
	{
	case BOT_MIKIKO:
		sprintf(sound,"player/mikiko/mikikodeath%i.wav",rand()%5+1);
		break;
	case BOT_SUPERFLY:
		sprintf(sound,"player/superfly/superflydeath%i.wav",rand()%4+1);
		break;
	default:
#endif
		sprintf(sound,"player/playerdeath%i.wav",rand()%4+1);
#ifdef OPENKATANA
	}
#endif

	Sound(eBot,CHAN_VOICE,sound,255,ATTN_NORM);

	if(eBot->v.iHealth < BOT_MIN_HEALTH)
	{
		Sound(eBot,CHAN_VOICE,"misc/gib1.wav",255,ATTN_NORM);

		ThrowGib(eBot->v.origin,eBot->v.velocity,"models/gibs/gib0.md2",(float)(eBot->v.iHealth*-1),true);
		ThrowGib(eBot->v.origin,eBot->v.velocity,"models/gibs/gib1.md2",(float)(eBot->v.iHealth*-1),true);
		ThrowGib(eBot->v.origin,eBot->v.velocity,"models/gibs/gib2.md2",(float)(eBot->v.iHealth*-1),true);

		Engine.Particle(eBot->v.origin,eBot->v.velocity,1.0f,"blood",5);
	}
}
