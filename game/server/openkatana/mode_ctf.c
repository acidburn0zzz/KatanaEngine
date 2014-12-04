/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "server_mode.h"

/*	
	This is my little "Capture The Flag" gamemode which is pretty much
	based around a single entity (item_flag), this code mainly served as
	a good study for states and such for entities which I hope to take
	advantage of for the AI in OpenKatana. You can pretty much do as you
	please with this code, it should translate back into QuakeC quite easily
	for those who understand it well enough ;)
	It's worth mentioning that there may be some flaws/mistakes, I blame the
	lack of sleep really but I've cleaned and revised it a number of times
	so if you find anything wrong let me know and I'll sort it out in future
	releases. ~hogsy
*/

#include "server_monster.h"
#include "server_item.h"

#define	RESPAWN_DELAY		1000

#define STATE_FLAG_DROPPED	0
#define STATE_FLAG_CARRIED	1
#define STATE_FLAG_IDLE		3
#define STATE_FLAG_CAPTURED	4

void CTF_FlagTouch(edict_t *ent,edict_t *other);

void CTF_FlagReset(edict_t *ent)
{
	Entity_SetOrigin(ent,ent->local.pos1);
	SetAngle(ent,ent->local.pos2);

	// [2/4/2012] Make sure our owner no longer thinks he has us ~hogsy
	if(ent->local.eOwner)
	{
		ent->local.eOwner->local.flag	= NULL;
		ent->local.eOwner				= NULL;
	}

	ent->local.state	= STATE_FLAG_IDLE;

	ent->v.TouchFunction	= CTF_FlagTouch;
	// [5/6/2012] Just clear the effects out... ~hogsy
	ent->v.effects			= 0;
	ent->v.dNextThink		= Server.dTime+0.5;
}

void CTF_FlagThink(edict_t *ent)
{
	switch(ent->local.state)
	{
	case STATE_FLAG_IDLE:
//		MONSTER_Animate(ent,FALSE,0,9,0.1f,0);
		break;
	case STATE_FLAG_CARRIED:
		if(ent->local.eOwner->v.iHealth <= 0)
		{
			ent->local.hit = RESPAWN_DELAY;
			ent->local.state = STATE_FLAG_DROPPED;
//			MONSTER_Animate(ent,FALSE,0,9,0.1f,0);
			break;
		}

		Math_VectorCopy(ent->local.eOwner->v.origin,ent->v.origin);

		ent->v.angles[0] = ent->local.eOwner->v.angles[0]+20;
		ent->v.angles[1] = ent->local.eOwner->v.angles[1];

//		MONSTER_Animate(ent,FALSE,15,15,0.01f,15);
		break;
	case STATE_FLAG_CAPTURED:
		CTF_FlagReset(ent);
		break;
	case STATE_FLAG_DROPPED:
		if(ent->local.hit <= 0)
		{
			CTF_FlagReset(ent);
			break;
		}
		else if(ent->local.hit == RESPAWN_DELAY)
		{
			Sound(ent,CHAN_ITEM,"ctf/flagdropped.wav",255,ATTN_NORM);

			ent->local.eOwner->local.flag	= NULL;
			ent->local.eOwner				= NULL;

			ent->v.TouchFunction	= CTF_FlagTouch;
			ent->v.angles[0]		= ent->v.angles[0]+10;
			// [10/5/2012] Floating motion is now done client-side ~hogsy
			ent->v.effects			|= EF_MOTION_ROTATE|EF_MOTION_FLOAT;
		}
#if 1
		else if(ent->local.hit == 70	||
				ent->local.hit == 40	||
				ent->local.hit == 10)
			Sound(ent,CHAN_ITEM,"ctf/flagdropped.wav",255,ATTN_NORM);	// "VWOOM"
#endif

		ent->local.hit--;

		Engine.Con_DPrintf("Flag respawn in... %i\n",ent->local.hit);

//		MONSTER_Animate(ent,FALSE,0,9,0.1f,0);
		break;
	default:
		// [5/6/2012] Currently this gives us our think time so we need to call it here ~hogsy
//		MONSTER_Animate(ent,FALSE,0,9,0.1f,0);
		Engine.Con_Warning("Unknown flag state (%i)!\n",ent->local.state);
	}
}

void CTF_FlagTouch(edict_t *ent,edict_t *other)
{
	// [20/2/2012] Revised ~hogsy
	// [23/2/2012] Revised ~hogsy
	/*	Don't let us pick this up if we're either not a client
		or we are dead.											*/
	if(other->monster.iType != MONSTER_PLAYER || other->v.iHealth <= 0)
		return;
	else if(other->local.flag && other->local.flag->local.style != ent->local.style)
	{
		if(other->local.flag->local.state == STATE_FLAG_CAPTURED)
			return;

		switch(other->local.pTeam)
		{
		case TEAM_RED:
			iRedScore++;
			Sound(ent,CHAN_ITEM,"ctf/redscore.wav",255,ATTN_NORM);
			// TODO: Update score HUD
			break;
		case TEAM_BLUE:
			iBlueScore++;
			Sound(ent,CHAN_ITEM,"ctf/bluescore.wav",255,ATTN_NORM);
			// TODO: Update score HUD
			break;
		default:
			other->v.iScore++;
		}

		// [8/6/2012] Check our scores... ~hogsy
		if(iRedScore > cvServerMaxScore.value)
		{
		}
		else if(iBlueScore > cvServerMaxScore.value)
		{
		}

		other->local.flag->local.state = STATE_FLAG_CAPTURED;
	}
	else if(!other->local.flag && other->local.pTeam != ent->local.style)
	{
		other->local.flag					= ent;
		other->local.flag->local.state		= STATE_FLAG_CARRIED;
		other->local.flag->local.eOwner		= other;
		other->local.flag->v.TouchFunction	= NULL;

		// Get the flags style and set our effect...
		switch(other->local.flag->local.style)
		{
		case TEAM_RED:
			other->local.flag->v.effects = EF_GLOW_RED;
			break;
		case TEAM_BLUE:
			other->local.flag->v.effects = EF_GLOW_BLUE;
			break;
		default:
			other->local.flag->v.effects = EF_GLOW_WHITE;
			break;
		}

		Engine.CenterPrint(other,va("%s stole the %s!\n",other->v.netname,other->local.flag->v.netname));

		Sound(ent,CHAN_ITEM,"ctf/flagtaken.wav",255,ATTN_NORM);
		Sound(ent,CHAN_ITEM,other->local.flag->v.noise,255,ATTN_STATIC);		//	"RED FLAG TAKEN!"
	}
}

/*	Spawns the flag for CTF mode!
*/
void CTF_FlagSpawn(edict_t *eFlag)
{
	if(!cvServerGameMode.iValue == MODE_CAPTURETHEFLAG)
		return;

	eFlag->v.movetype	= MOVETYPE_NONE;
	eFlag->v.items		= ITEM_FLAG;

	eFlag->Physics.iSolid	= SOLID_TRIGGER;

	eFlag->local.state = STATE_FLAG_IDLE;

	switch(eFlag->local.style)
	{
	/*	Once we have support for setting
		our models skin we'll do that here
		instead ;)							*/
	case TEAM_RED:
		Engine.Server_PrecacheResource(RESOURCE_SOUND,"ctf/redscore.wav");

		eFlag->v.model		= "models/ctf/flag_red.md2";
		eFlag->v.noise		= "ctf/redflagtaken.wav";
		eFlag->v.netname	= "red flag";
		break;
	case TEAM_BLUE:
		Engine.Server_PrecacheResource(RESOURCE_SOUND,"ctf/bluescore.wav");

		eFlag->v.model		= "models/ctf/flag_blue.md2";
		eFlag->v.noise		= "ctf/blueflagtaken.wav";
		eFlag->v.netname	= "blue flag";
		break;
	default:
		eFlag->v.model		= "models/ctf/flag_neutral.md2";
		eFlag->v.noise		= "ctf/neutralflagtaken.wav";
		eFlag->v.netname	= "neutral flag";
		break;
	}

	Engine.Server_PrecacheResource(RESOURCE_MODEL,eFlag->v.model);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,eFlag->v.noise);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"ctf/flagtaken.wav");

	Entity_SetModel(eFlag,eFlag->v.model);
	Entity_SetSize(eFlag,-10,-10,0,10,10,65);
	Entity_SetOrigin(eFlag,eFlag->v.origin);

	// [5/6/2012] Save our original position ~hogsy
	Math_VectorCopy(eFlag->v.angles,eFlag->local.pos1);
	Math_VectorCopy(eFlag->v.angles,eFlag->local.pos2);

	eFlag->v.bTakeDamage	= false;
	eFlag->v.TouchFunction	= CTF_FlagTouch;
	eFlag->v.think			= CTF_FlagThink;
	eFlag->v.dNextThink		= Server.dTime+0.01;
}
