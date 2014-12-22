/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "game_main.h"

/*
	Main Entry
*/

#include "platform_module.h"

#include "server_player.h"

#include "client_main.h"

GameExport_t	Export;	// Game exports.
ModuleImport_t	Engine;	// Engine imports.

int	iOldGameMode;

// [2/8/2012] TODO: Why are we doing this!? Should be using the one from the lib ~hogsy
char *va(char *format,...)
{
	va_list		argptr;
	static char	string[1024];

	va_start (argptr, format);
	vsprintf (string, format, argptr);
	va_end (argptr);

	return string;
}

void SetAngle(edict_t *ent,vec3_t vAngle)
{
	// [21/3/2012] Updated ~hogsy
	Math_VectorCopy(vAngle,ent->v.angles);

	Engine.LinkEntity(ent,false);
}

// [8/2/2012] Brought ChangeYaw over from the engine ~hogsy
void ChangeYaw(edict_t *ent)
{
	float ideal,current,move,speed;

	current	= Math_AngleMod(ent->v.angles[1]);
	ideal	= ent->v.ideal_yaw;
	speed	= ent->v.yaw_speed;

	if(current == ideal)
		return;

	move = ideal - current;
	if(ideal > current)
	{
		if(move >= 180)
			move = move-360;
	}
	else if(move <= -180)
		move = move+360;

	if(move > 0)
	{
		if(move > speed)
			move = speed;
	}
	else if(move < -speed)
		move = -speed;

	ent->v.angles[1] = Math_AngleMod(current+move);
}

/*	Used for use tracing and shot
	targeting. Traces are blocked by
	bbox and exact bsp entities, and
	also slide box entities	if the
	tryents flag is set.
*/
trace_t Traceline(edict_t *ent,vec3_t vStart,vec3_t vEnd,int type)
{
	return Engine.Server_Move(vStart, mv3Origin, mv3Origin, vEnd, type, ent);
}

/*	Each entity can have eight
	independant sound sources, like
	voice, weapon, feet, etc.
	Channel 0 is an auto-allocate
	channel, the others override
	anything allready running on that
	entity/channel pair.
	An attenuation of 0 will play full
	volume everywhere in the level.
	Larger attenuations will drop off.
*/
void Sound(edict_t *ent,int channel,char *sound,int volume,float attenuation)
{
	Engine.Sound(ent,channel,sound,volume,attenuation);

	/* TODO
	// [21/3/2012] Revised ~hogsy
	#define SOUND_DEFAULT	0
	#define SOUND_REFERENCE	1

	edict_t *sound;

	if(type)
		sound = Entity_Spawn();
	else
		return;

	switch(type)
	{
	case SOUND_REFERENCE:
		if(volume >= 0.5f)
		{
			sound->v.cClassname = "sound_reference";

			SetOrigin(sound,ent->v.origin);

			sound->v.think		= RemoveEntity(ref);
			sound->v.dNextThink	= (Server.time+0.01)*volume;
		}
		else
			RemoveEntity(sound);
		break;
	default:
		RemoveEntity(sound);
	}
	*/
}

void WriteByte(int mode,int c)
{
	Engine.WriteByte(mode,c);
}

// OBSOLETE
bool Game_Init(int state,edict_t *ent,double dTime)
{
	Server.dTime = dTime;

	switch(state)
	{
	case SERVER_CLIENTPOSTTHINK:
		// [8/6/2012] Revised ~hogsy
		Player_PostThink(ent);
		break;
	case SERVER_PLAYERPRETHINK:
		// [8/6/2012] Revised ~hogsy
		Player_PreThink(ent);
		break;
	case SERVER_CLIENTCONNECT:
		Server.iClients++;

		Engine.Server_BroadcastPrint("%s connected\n",ent->v.netname);
		break;
	case SERVER_CLIENTDISCONNECT:
		Server.iClients--;

		Engine.Server_BroadcastPrint("%s disconnected\n",ent->v.netname);

		Entity_Remove(ent);
		break;
	case SERVER_SETCHANGEPARMS:
		if(ent->v.iHealth <= 0)
			break;
		else if(ent->v.iHealth < 50)
			ent->v.iHealth = 50;

		// TODO: Set all necessary parms...
		break;
	case SERVER_SETNEWPARMS:
		if(!Server.bActive)
			Server.bActive = true;

		iRedScore	=
		iBlueScore	= 0;
		break;
	}

	return true;
}

void	Server_StartFrame(void);	// server_main
void	Game_Shutdown(void);

pMODULE_EXPORT GameExport_t *Game_Main(ModuleImport_t *Import)
{
	Engine.Con_Printf				= Import->Con_Printf;
	Engine.Con_DPrintf				= Import->Con_DPrintf;
	Engine.Con_Warning				= Import->Con_Warning;
	Engine.Sys_Error				= Import->Sys_Error;
	Engine.SetMessageEntity			= Import->SetMessageEntity;
	Engine.SetModel					= Import->SetModel;
	Engine.Sound					= Import->Sound;
	Engine.LinkEntity				= Import->LinkEntity;
	Engine.FreeEntity				= Import->FreeEntity;
	Engine.Spawn					= Import->Spawn;
	Engine.Cvar_RegisterVariable	= Import->Cvar_RegisterVariable;
	Engine.Cvar_SetValue			= Import->Cvar_SetValue;
	Engine.ReadByte					= Import->ReadByte;
	Engine.ReadCoord				= Import->ReadCoord;
	Engine.WriteByte				= Import->WriteByte;
	Engine.WriteCoord				= Import->WriteCoord;
	// [28/7/2012] Added WriteAngle ~hogsy
	Engine.WriteAngle				= Import->WriteAngle;
	// [28/7/2012] Added WriteEntity ~hogsy
	Engine.WriteEntity				= Import->WriteEntity;
	Engine.MakeVectors				= Import->MakeVectors;
	Engine.Server_Move				= Import->Server_Move;
	Engine.LightStyle				= Import->LightStyle;
	Engine.Aim						= Import->Aim;
	Engine.Particle					= Import->Particle;
	Engine.Flare					= Import->Flare;
	Engine.CenterPrint				= Import->CenterPrint;
	Engine.Cmd_AddCommand			= Import->Cmd_AddCommand;
	Engine.GetLightSample			= Import->GetLightSample;

	Engine.Client_GetEffect			= Import->Client_GetEffect;
	Engine.Client_AllocateDlight	= Import->Client_AllocateDlight;
	Engine.Client_AllocateParticle	= Import->Client_AllocateParticle;
	Engine.Client_PrecacheResource	= Import->Client_PrecacheResource;
	Engine.Client_GetPlayerEntity	= Import->Client_GetPlayerEntity;
	Engine.Client_GetViewEntity		= Import->Client_GetViewEntity;

	Engine.Server_PointContents		= Import->Server_PointContents;
	Engine.Server_MakeStatic		= Import->Server_MakeStatic;
	Engine.Server_PrecacheResource	= Import->Server_PrecacheResource;
	Engine.Server_BroadcastPrint	= Import->Server_BroadcastPrint;
	Engine.Server_SinglePrint		= Import->Server_SinglePrint;
	Engine.Server_FindRadius		= Import->Server_FindRadius;
	Engine.Server_FindEntity		= Import->Server_FindEntity;
	Engine.Server_Restart			= Import->Server_Restart;
	Engine.Server_ChangeLevel		= Import->Server_ChangeLevel;
	Engine.Server_AmbientSound		= Import->Server_AmbientSound;
	Engine.Server_GetLevelName		= Import->Server_GetLevelName;
	Engine.Server_GetFrameTime		= Import->Server_GetFrameTime;

	Export.iVersion = GAME_VERSION;
#ifdef GAME_OPENKATANA
	Export.Name							= "OpenKatana";
#elif GAME_ADAMAS
	Export.Name							= "Adamas";
#elif GAME_CLASP
	Export.Name							= "Clasp";
#elif GAME_ICTUS
	Export.Name							= "Ictus";
#else
	Export.Name							= "Katana";
#endif
	Export.ChangeYaw					= ChangeYaw;
	Export.SetSize						= Entity_SetSize;
	Export.Draw							= Client_Draw;
	Export.Game_Init					= Game_Init;
	Export.Shutdown						= Game_Shutdown;

	// Client
	Export.Client_RelinkEntities		= Client_RelinkEntities;
	Export.Client_Initialize			= Client_Initialize;
	Export.Client_ParseTemporaryEntity	= Client_ParseTemporaryEntity;
	Export.Client_ViewFrame				= Client_ViewFrame;

	// Server
	Export.Server_Initialize			= Server_Initialize;
	Export.Server_StartFrame			= Server_StartFrame;
	Export.Server_SpawnEntity			= Server_SpawnEntity;
	Export.Server_EntityFrame			= Server_EntityFrame;
	Export.Server_KillClient			= Server_KillClient;
	Export.Server_SetSizeVector			= Entity_SetSizeVector;
	Export.Server_SpawnPlayer			= Player_Spawn;

	Export.Physics_SetGravity			= Physics_SetGravity;
	Export.Physics_CheckWaterTransition	= Physics_CheckWaterTransition;
	Export.Physics_CheckVelocity		= Physics_CheckVelocity;

	return &Export;
}

/* Called upon engine shutdown.
*/
void Game_Shutdown(void)
{
}
