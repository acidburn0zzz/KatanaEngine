/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "mode_vektar.h"

#include "server_waypoint.h"

/*	Notes:
	At first I wanted to simply create a game mode that was similar to classic board games,
	in which people actually paid attention to what your other playmates are up to.
	And instead of being guided by rules and responsibilities limiting you, I wanted you to play the way you want.
	You can either be an ass and play fair, or be a dick to others - or just be yourself.

	In this gamemode I originally wanted players to fight for something, in the earliest concepts
	I was thinking of a Golem like creature that would warp around the map and has to be destroyed
	with limited amounts of resources given to the players so they would have to help eachother out
	at certain points, but that slightly defeated the purpose of my original intention and so
	I came up with the idea of simply replacing the Golem with a statue that finds a random place in the map
	to hide. The first one who finds and kills it gets 10 points.
	The concept was still to simple for me and I wanted to give others a chance aswell to get to the statue.
	So i made that the statue drops an item (the so called VEKTAR QUAD) which will give you the 10 points when
	touched instead.
	Because of the fear that some people might steal the item away and to provoke the players to fight a bit
	more I made a delay in which the item can be picked up aswell as an explosion that should kill off any nearby campers.
	-eukos

	TODO:
		Move over to state-based system. ~hogsy
		Visibility check before spawning the Vektar. ~hogsy
*/

void Vektar_Spawn(void);

/*	VEKBALL
*/
// Merge? - eukos
void Vektar_VekBallDie(edict_t *ent)
{
	Sound(ent,CHAN_ITEM,VEKTAR_SOUND_GOTIT,255,ATTN_NORM);

	Vektar_Spawn();

	Entity_Remove(ent);
}

void Vektar_VekBallTouch(edict_t *ent, edict_t *other)
{
	if(other->monster.iType != MONSTER_PLAYER)
		return;

	Engine.Server_BroadcastPrint("- VEKTAR WAS DESTROYED BY %s -\n", other->v.netname);

	other->v.iScore += 10;

	Vektar_VekBallDie(ent);
}

void Vektar_VekBallStart(edict_t *ent)
{
	ent->v.TouchFunction	= Vektar_VekBallTouch;
	ent->v.think			= Vektar_VekBallDie;
	ent->v.dNextThink		= Server.dTime+20.0;
}

void Vektar_VekBallSpawn(vec_t *org)
{
	edict_t *eVekball = Entity_Spawn();

	if(eVekball)
	{
		Entity_SetModel(eVekball,DAIKATANA_MODEL_WORLD);
		Entity_SetSize(eVekball,-16.0f,-16.0f,-24.0f,16.0f,16.0f,40.0f);
		Entity_SetOrigin(eVekball, org);

		eVekball->v.velocity[2]	= eVekball->v.velocity[2]*20.0f;
		eVekball->v.movetype	= MOVETYPE_BOUNCE;
		eVekball->Physics.iSolid		= SOLID_TRIGGER;
		eVekball->v.think		= Vektar_VekBallStart;
		eVekball->v.dNextThink	= Server.dTime+3.0;

		Sound(eVekball,CHAN_ITEM,VEKTAR_SOUND_GRABIT,255,ATTN_NORM);
	}
	else
		Engine.Con_Warning("Failed to spawn vekball!\n");
}
// =================================================================================

void Vektar_Die(edict_t *eVektar,edict_t *other)
{
	Engine.Server_BroadcastPrint("- %s OPENED THE VEKTAR -\n",other->v.netname);

	Vektar_VekBallSpawn(eVektar->v.origin);

	Sound(eVektar,CHAN_VOICE,VEKTAR_SOUND_FANFARE,255,ATTN_NONE);
	Sound(eVektar,CHAN_AUTO,"weapons/explosion1.wav",255,ATTN_NORM);

	//Monster_DamageRadius

	Engine.WriteByte(MSG_BROADCAST,SVC_TEMPENTITY);
	Engine.WriteByte(MSG_BROADCAST,CTE_EXPLOSION);
	Engine.WriteCoord(MSG_BROADCAST,eVektar->v.origin[0]);
	Engine.WriteCoord(MSG_BROADCAST,eVektar->v.origin[1]);
	Engine.WriteCoord(MSG_BROADCAST,eVektar->v.origin[2]);

	Entity_Remove(eVektar);
}

void Vektar_Move(edict_t *eVektar)
{
	Waypoint_t	*wWaypoint;

	// [20/12/2012] Vektar's positioning shit ~hogsy
	// [20/12/2012] BUG: Pretty sure this will always return the first players position... ~hogsy
	wWaypoint = Waypoint_GetByType(Engine.Server_FindEntity(Server.eWorld,"player",true),WAYPOINT_DEFAULT,MONSTER_RANGE_FAR);
	if(!wWaypoint)
	{
		eVektar->v.think		= Vektar_Move;
		eVektar->v.dNextThink	= Server.dTime+10.0;
		return;
	}
	// [25/1/2013] TODO: Is position empty? ~hogsy

	eVektar->Physics.iSolid = SOLID_SLIDEBOX;

	Entity_SetOrigin(eVektar,wWaypoint->position);

	DropToFloor(eVektar);

	Sound(eVektar,CHAN_ITEM,VEKTAR_SOUND_FINDING,255,ATTN_NORM);

	Engine.Server_BroadcastPrint("- THE VEKTAR HAS BEEN SPAWNED -\n");
}

void Vektar_Spawn(void)
{
	edict_t	*eVektar = Entity_Spawn();

	// [20/12/2012] Also precache our resources ~hogsy
	Engine.Server_PrecacheResource(RESOURCE_MODEL,DAIKATANA_MODEL_WORLD);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,VEKTAR_MODEL_STATUE);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,VEKTAR_SOUND_GRABIT);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,VEKTAR_SOUND_FINDING);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,VEKTAR_SOUND_GOTIT);
	Engine.Server_PrecacheResource(RESOURCE_SOUND,VEKTAR_SOUND_FANFARE);

	eVektar->v.cClassname	= "mpm_vektar";
	eVektar->v.movetype		= MOVETYPE_STEP;
	eVektar->v.bTakeDamage	= true;
	eVektar->v.iHealth		= 1000;
	eVektar->v.effects		= EF_INVISIBLE;

	eVektar->Physics.iSolid	= SOLID_NOT;

	eVektar->monster.think_die = Vektar_Die;

	Entity_SetModel(eVektar,VEKTAR_MODEL_STATUE);
	Entity_SetSize(eVektar,-16.0f,-16.0f,-24.0f,16.0f,16.0f,40.0f);

	eVektar->v.think		= Vektar_Move;
	eVektar->v.dNextThink	= Server.dTime+30.0;

	Engine.Con_Printf("Vektar spawned, waiting to place...\n");
}
