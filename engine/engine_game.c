/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_game.h"

/*
	Game Interface
*/

#include "engine_console.h"
#include "KatGL.h"			// [23/9/2013] TODO: Rename to engine_draw.h? ~hogsy
#include "engine_menu.h"
#include "engine_server.h"

#include "shared_module.h"

#include "../platform/include/platform_module.h"

/*	TODO:
		Move Server_ functions into KatServer
*/

pINSTANCE hGameInstance;

ModuleExport_t *Game;

void Server_ChangeLevel(const char *ccNewLevel)
{
	// Make sure we don't issue two changelevels
	if(svs.bChangingLevel)
		return;

	svs.bChangingLevel = true;

	Cbuf_AddText(va("changelevel %s\n",ccNewLevel));
}

char *Server_GetLevelName(void)
{
	return sv.name;
}

edict_t *Server_FindRadius(vec3_t origin,float radius)
{
	int		i,j;
	edict_t *eEntity,*eChain;
	vec3_t	eorg;

	eChain = sv.edicts;

	eEntity = NEXT_EDICT(sv.edicts);
	for(i = 1; i < sv.num_edicts; i++,eEntity = NEXT_EDICT(eEntity))
	{
		if(eEntity->free && eEntity->Physics.iSolid == SOLID_NOT)
			continue;

		for(j = 0; j < 3; j++)
			eorg[j] = origin[j]-(eEntity->v.origin[j]+(eEntity->v.mins[j]+eEntity->v.maxs[j])*0.5f);

		if(Math_Length(eorg) > radius)
			continue;

		eEntity->v.chain	= eChain;
		eChain				= eEntity;
	}

	return eChain;
}

int SV_ModelIndex(char *name);

void Server_MakeStatic(edict_t *ent)
{
	int	i,bits=0;

	if(ent->alpha == ENTALPHA_ZERO)
	{
		ED_Free(ent);
		return;
	}

	if(SV_ModelIndex(ent->v.model) & 0xFF00)
		bits |= B_LARGEMODEL;
	if((int)(ent->v.frame) & 0xFF00)
		bits |= B_LARGEFRAME;
	if(ent->alpha != ENTALPHA_DEFAULT)
		bits |= B_ALPHA;

	if(bits)
	{
		MSG_WriteByte(&sv.signon, svc_spawnstatic2);
		MSG_WriteByte(&sv.signon, bits);
	}
	else
		MSG_WriteByte(&sv.signon, svc_spawnstatic);

	if(bits & B_LARGEMODEL)
		MSG_WriteShort(&sv.signon, SV_ModelIndex(ent->v.model));
	else
		MSG_WriteByte(&sv.signon, SV_ModelIndex(ent->v.model));

	if(bits & B_LARGEFRAME)
		MSG_WriteShort(&sv.signon,ent->v.frame);
	else
		MSG_WriteByte(&sv.signon,ent->v.frame);

	MSG_WriteByte(&sv.signon,ent->Model.fScale);
	MSG_WriteByte(&sv.signon,ent->v.colormap);
	MSG_WriteByte(&sv.signon,ent->v.skin);
	for (i=0 ; i<3 ; i++)
	{
		MSG_WriteCoord(&sv.signon, ent->v.origin[i]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[i]);
	}

	if (bits & B_ALPHA)
		MSG_WriteByte (&sv.signon, ent->alpha);

	ED_Free (ent);
}

cvar_t	sv_aim = {	"sv_aim",	"1"	};

float *Game_Aim(edict_t *ent)
{
	edict_t			*check, *bestent;
	vec3_t			start, dir, end, bestdir;
	int				i, j;
	trace_t			tr;
	float			dist, bestdist;
	static float	result[3];

	Math_VectorCopy (ent->v.origin, start);
	start[2] += 20;

	// try sending a trace straight
	Math_VectorCopy(pr_global_struct.v_forward,dir);
	Math_VectorMA(start,2048.0f,dir,end);

#if 1
	// [5/3/2013] BUG: Results in fucked up aim vectors ~hogsy
	tr = SV_Move(start,vec3_origin,vec3_origin,end,false,ent);
	if(tr.ent && tr.ent->v.bTakeDamage	&&
	(!teamplay.value || ent->local.pTeam <= 0 || ent->local.pTeam != tr.ent->local.pTeam) )
	{
		Math_VectorCopy(pr_global_struct.v_forward,result);
		return result;
	}
#endif

	// try all possible entities
	Math_VectorCopy (dir, bestdir);
	bestdist = sv_aim.value;
	bestent = NULL;

	check = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, check = NEXT_EDICT(check) )
	{
//		if (check->v.takedamage != DAMAGE_AIM)
//			continue;
		if (check == ent)
			continue;
		if (teamplay.value && ent->local.pTeam > 0 && ent->local.pTeam == check->local.pTeam)
			continue;	// don't aim at teammate

		for (j=0 ; j<3 ; j++)
			end[j] = check->v.origin[j]	+ 0.5f*(check->v.mins[j] + check->v.maxs[j]);

		Math_VectorSubtract(end,start,dir);
		Math_VectorNormalize(dir);

		dist = Math_DotProduct(dir,ent->v.vForward);		//pr_global_struct.v_forward);
		if (dist < bestdist)
			continue;	// to far to turn

		tr = SV_Move(start,vec3_origin,vec3_origin,end,false,ent);
		if (tr.ent == check)
		{
			// Can shoot at this one
			bestdist = dist;
			bestent = check;
		}
	}

	if (bestent)
	{
		Math_VectorSubtract(bestent->v.origin,ent->v.origin,dir);
		dist = Math_DotProduct(dir,ent->v.vForward);	//pr_global_struct.v_forward);
		Math_VectorScale(ent->v.vForward,dist,end);		//pr_global_struct.v_forward,dist,end);
		end[2] = dir[2];
		Math_VectorNormalize(end);
		Math_VectorCopy(end,result);
	}
	else
		Math_VectorCopy(bestdir,result);

	return result;
}

/*	Sets the model for the specified entity.
*/
void Server_SetModel(edict_t *ent,char *m)
{
	char	**check;
	model_t	*mod;
	int		i;

	for(i = 0,check = sv.model_precache; *check; i++,check++)
		if(!strcmp(*check, m))
			break;

	// [27/1/2013] Removed dynamic caching. This should only be done client-side derp! ~hogsy
	if(!*check)
		Console_ErrorMessage(true,m,"Model was not registered.");

	ent->v.model		= m;
	ent->v.modelindex	= i;

	mod = sv.models[(int)ent->v.modelindex];
	// [13/4/2013] Oops, made this more secure! ~hogsy
	if(mod && mod->mType == MODEL_TYPE_BSP)
		Game->Server_SetSizeVector(ent,mod->clipmins,mod->clipmaxs);
	else
		Game->Server_SetSizeVector(ent,vec3_origin,vec3_origin);
}

void Game_AmbientSound(vec_t *vPosition,const char *cPath,int iVolume,int iAttenuation)
{
	char		**cCheck;
	int			i,iSoundNumber;
	bool		bLarge = false;

	for(iSoundNumber = 0,cCheck = sv.sound_precache; *cCheck; cCheck++,iSoundNumber++)
		if(!strcmp(*cCheck,cPath))
			break;

	if(!*cCheck)
		Console_ErrorMessage(false,(char*)cPath,"Sound was not registered.");

	if(iSoundNumber > 255)
		bLarge = true;

	if(bLarge)
		MSG_WriteByte(&sv.signon,svc_spawnstaticsound2);
	else
		MSG_WriteByte(&sv.signon,svc_spawnstaticsound);

	for(i = 0; i < 3; i++)
		MSG_WriteCoord(&sv.signon,vPosition[i]);

	if(bLarge)
		MSG_WriteShort(&sv.signon,iSoundNumber);
	else
		MSG_WriteByte(&sv.signon,iSoundNumber);

	MSG_WriteByte(&sv.signon,iVolume);
	MSG_WriteByte(&sv.signon,iAttenuation*64);
}

// [18/7/2012] Renamed to Server_Sound ~hogsy
void Server_Sound(edict_t *ent,int channel,char *sample,int volume,float attenuation)
{
	int sound_num, field_mask, i, e;

	if(!sample)
	{
		Con_Warning("Bad sample name (%s)!\n",ent->v.cClassname);
		return;
	}
	else if(volume < 0 || volume > 255)
	{
		Con_Warning("Sound: volume = %i\n", volume);
		return;
	}
	else if(attenuation < 0 || attenuation > 4)
	{
		Con_Warning("Sound: attenuation = %f\n", attenuation);
		return;
	}
	else if(channel < 0 || channel > 7)
	{
		Con_Warning("Sound: channel = %i\n", channel);
		return;
	}
	else if(sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

	for(sound_num=1 ; sound_num<MAX_SOUNDS && sv.sound_precache[sound_num] ; sound_num++)
		if(!strcmp(sample,sv.sound_precache[sound_num]))
			break;

	if(sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num])
	{
		// [19/8/2012] Just return a simple warning ~hogsy
		Con_Warning("%s was not precached!\n",sample);
		return;
	}

	e = NUM_FOR_EDICT(ent);

	field_mask = 0;
	if(volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;

	if(attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;

	if(e >= 8192)
		field_mask |= SND_LARGEENTITY;

	if(sound_num >= 256 || channel >= 8)
		field_mask |= SND_LARGESOUND;

	MSG_WriteByte(&sv.datagram,svc_sound);
	MSG_WriteByte(&sv.datagram,field_mask);
	if(field_mask & SND_VOLUME)
		MSG_WriteByte(&sv.datagram,volume);
	if(field_mask & SND_ATTENUATION)
		MSG_WriteByte(&sv.datagram,attenuation*64);

	if(field_mask & SND_LARGEENTITY)
	{
		MSG_WriteShort(&sv.datagram,e);
		MSG_WriteByte(&sv.datagram,channel);
	}
	else
		MSG_WriteShort(&sv.datagram,(e<<3)|channel);

	if(field_mask & SND_LARGESOUND)
		MSG_WriteShort(&sv.datagram,sound_num);
	else
		MSG_WriteByte(&sv.datagram,sound_num);

	for(i=0 ; i<3 ; i++)
		MSG_WriteCoord(&sv.datagram,ent->v.origin[i]+0.5*(ent->v.mins[i]+ent->v.maxs[i]));
}

/*
	Flares
*/

void Server_Flare(vec3_t org,float r,float g,float b,float a,float scale,char *texture)
{
	int		i;
	char	name[MAX_OSPATH];

	if(!texture)
	{
		Con_Warning("Tried to spawn a flare with no texture.\n");
		return;
	}
	else if(sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

	sprintf(name,PATH_SPRITES"%s",texture);

	if(!org || !scale)
	{
		Console_ErrorMessage(FALSE,name,"Illegal parameters used for flare.");
		return;
	}

	MSG_WriteByte(&sv.datagram,SVC_FLARE);
	for(i=0;i<3;i++)
		MSG_WriteCoord(&sv.datagram,org[i]);
	MSG_WriteFloat(&sv.datagram,r);
	MSG_WriteFloat(&sv.datagram,g);
	MSG_WriteFloat(&sv.datagram,b);
	MSG_WriteFloat(&sv.datagram,a);
	MSG_WriteFloat(&sv.datagram,scale);
	for(i=0;i<MAX_EFFECTS;i++)
		if(gEffectTexture[i])
			if(!Q_strcmp(name,gEffectTexture[i]->name))
			{
				MSG_WriteByte(&sv.datagram,i);
				break;
			}
}

void Client_RunFlareEffect(vec3_t org,int scale,float r,float g,float b,float a,int texture)
{
	int		i;
	flare_t *f;

	if(!free_flares)
		return;
	f = free_flares;
	free_flares = f->next;
	f->next = active_flares;
	active_flares = f;

	f->texture = texture;
	if(f->scale)
		f->scale = scale*2;
	else
		f->scale = 1.0;
	for(i=0;i<3;i++)
		f->org[i] = org[i];
	if(r || g || b || a)
	{
		f->r = r;
		f->g = g;
		f->b = b;
		f->alpha = a;
	}
}

void Client_ParseFlareEffect(void)
{
	vec3_t	org;
	int		i,texture,scale;
	float	r,g,b,a;

	for(i=0;i<3;i++)
		org[i] = MSG_ReadCoord();

	r		= MSG_ReadFloat();
	g		= MSG_ReadFloat();
	b		= MSG_ReadFloat();
	a		= MSG_ReadFloat();
	scale	= MSG_ReadFloat();
	texture = MSG_ReadByte();

	Client_RunFlareEffect(org,scale,r,g,b,a,texture);
}

/*
	Particles
*/

void Particle(vec3_t org,vec3_t dir,float scale,char *texture,int count)
{
	int	i,v;

	if(sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

	MSG_WriteByte(&sv.datagram,SVC_PARTICLE);

	for(i = 0; i < 3; i++)
		MSG_WriteCoord(&sv.datagram,org[i]);

	for(i=0 ; i<3 ; i++)
	{
		v = dir[i]*16;
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;

		MSG_WriteChar(&sv.datagram,v);
	}

	MSG_WriteFloat(&sv.datagram,scale);

	for(i = 0; i < MAX_EFFECTS; i++)
		if(gEffectTexture[i])
		{
			if(!strcmp(texture,gEffectTexture[i]->name))
			{
				MSG_WriteByte(&sv.datagram,i);
				break;
			}
		}
		else
		{
			// Otherwise give the texture the initial slot (this points to the notexture).
			MSG_WriteByte(&sv.datagram,0);
			break;
		}

	MSG_WriteByte(&sv.datagram,count);
}

/**/

void LightStyle(int style,char *val)
{
	client_t	*client;
	int			j;

	sv.lightstyles[style] = val;

	if (sv.state != SERVER_STATE_ACTIVE)
		return;

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
		if (client->active || client->bSpawned)
		{
			MSG_WriteChar (&client->message, svc_lightstyle);
			MSG_WriteChar (&client->message,style);
			MSG_WriteString (&client->message, val);
		}
}

void Game_MakeVectors(vec3_t angles)
{
	Math_AngleVectors(angles,pr_global_struct.v_forward,pr_global_struct.v_right,pr_global_struct.v_up);
}

edict_t	*eMessageEntity;

sizebuf_t *Game_WriteDest(int dest)
{
	int	entnum;

	switch(dest)
	{
	case MSG_BROADCAST:
		return &sv.datagram;
	case MSG_ONE:
		entnum = NUM_FOR_EDICT(eMessageEntity);
		if(entnum < 1 || entnum > svs.maxclients)
			Con_Error("WriteDest: not a client");

		return &svs.clients[entnum-1].message;
	case MSG_ALL:
		return &sv.reliable_datagram;
	case MSG_INIT:
		return &sv.signon;
	default:
		Con_Error("WriteDest: bad destination");
		break;
	}

	return NULL;
}

void Game_WriteByte(int mode,int command)
{
	MSG_WriteByte(Game_WriteDest(mode),command);
}

void Game_WriteCoord(int mode,float f)
{
	MSG_WriteCoord(Game_WriteDest(mode),f);
}

void Game_WriteAngle(int mode,float f)
{
	MSG_WriteAngle(Game_WriteDest(mode),f);
}

void Game_WriteEntity(int mode,edict_t *ent)
{
	MSG_WriteShort(Game_WriteDest(mode),NUM_FOR_EDICT(ent));
}

void Game_AddCommand(char *c,void (*Function)(void))
{
	Cmd_AddCommand(c,(xcommand_t)Function);
}

// [28/7/2012] Added Game_SetMessageEntity ~hogsy
void Game_SetMessageEntity(edict_t *eEntity)
{
	eMessageEntity = eEntity;
}

void Host_Restart_f(void);
void Server_CenterPrint(edict_t *ent,char *msg);	// See engine_server.

void Game_Initialize(void)
{
	bool			bGameLoaded = false;
	ModuleImport_t	Import;

	if(Game)
		// [4/5/2013] Oops forgot to put this here ~hogsy
		pModule_Unload(hGameInstance);

	Import.Con_Printf				= Con_Printf;
	Import.Con_DPrintf				= Con_DPrintf;
	Import.Con_Warning				= Con_Warning;
	Import.Sys_Error				= Sys_Error;
	Import.SetModel					= Server_SetModel;
	Import.Particle					= Particle;
	Import.Flare					= Server_Flare;
	Import.Sound					= Server_Sound;
	Import.UnlinkEntity				= SV_UnlinkEdict;
	Import.LinkEntity				= SV_LinkEdict;
	Import.Server_Move				= SV_Move;
	Import.FreeEntity				= ED_Free;
	Import.DrawPic					= Draw_ExternPic;
	Import.Spawn					= ED_Alloc;
	Import.Cvar_RegisterVariable	= Cvar_RegisterVariable;
	Import.Cvar_SetValue			= Cvar_SetValue;
	Import.LightStyle				= LightStyle;
	Import.CenterPrint				= Server_CenterPrint;
	// [28/7/2012] Updated to use the new function ~hogsy
	Import.Cmd_AddCommand			= Game_AddCommand;
	Import.MakeVectors				= Game_MakeVectors;
	Import.Aim						= Game_Aim;
	Import.ReadByte					= MSG_ReadByte;
	Import.ReadCoord				= MSG_ReadCoord;
	Import.WriteByte				= Game_WriteByte;
	Import.WriteCoord				= Game_WriteCoord;
	Import.WriteAngle				= Game_WriteAngle;
	Import.WriteEntity				= Game_WriteEntity;
	Import.SetMessageEntity			= Game_SetMessageEntity;

	// [6/5/2012] Client-side functions ~hogsy
	Import.Client_AllocateDlight	= Client_AllocDlight;
	Import.Client_AllocateParticle	= Client_AllocateParticle;
	Import.Client_PrecacheResource	= Client_PrecacheResource;
	Import.Client_GetStat			= Client_GetStat;
	Import.Client_GetEffect			= Client_GetEffect;
	Import.Client_GetPlayerEntity	= Client_GetPlayerEntity;
	Import.Client_GetViewEntity		= Client_GetViewEntity;

	if(Menu)
	{
		Import.Client_AddMenuState		= Menu->AddState;
		Import.Client_RemoveMenuState	= Menu->RemoveState;
	}

	Import.Server_PointContents		= SV_PointContents;
	Import.Server_MakeStatic		= Server_MakeStatic;
	Import.Server_BroadcastPrint	= SV_BroadcastPrintf;
	Import.Server_SinglePrint		= Server_SinglePrint;
	Import.Server_PrecacheResource	= Server_PrecacheResource;
	Import.Server_FindRadius		= Server_FindRadius;
	Import.Server_FindEntity		= Server_FindEntity;
	Import.Server_Restart			= Host_Restart_f;
	Import.Server_ChangeLevel		= Server_ChangeLevel;
	Import.Server_AmbientSound		= Game_AmbientSound;
	Import.Server_GetLevelName		= Server_GetLevelName;
	Import.Server_GetFrameTime		= Server_GetFrameTime;

	Game = (ModuleExport_t*)pModule_LoadInterface(hGameInstance,va("%s/"MODULE_GAME,com_gamedir),"Game_Main",&Import);
	if(!Game)
		Con_Warning("Failed to find %s/"MODULE_GAME"!\n",com_gamedir,MODULE_GAME);
	else if(Game->Version != MODULE_VERSION2)
		Con_Warning("Size mismatch (recieved %i, expected %i)!\n",Game->Version,MODULE_VERSION2);
	else
		bGameLoaded = true;

	if(!bGameLoaded)
	{
		pModule_Unload(hGameInstance);

        // Let the user know the module failed to load. ~hogsy
		Sys_Error("Failed to load %s/%s."PLATFORM_CPU""pMODULE_EXTENSION"!\nCheck log for details.",com_gamedir,MODULE_GAME);
	}
}
