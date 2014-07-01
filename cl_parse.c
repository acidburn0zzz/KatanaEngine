/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2014 OldTimes Software

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "quakedef.h"

/*
	Client-side parsing
*/

#include "engine_client.h"
#include "engine_console.h"
#include "engine_menu.h"

#include "shared_server.h"

char *svc_strings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",		// [long] server version
	"svc_setview",		// [short] entity number
	"svc_sound",			// <see code>
	"svc_time",			// [float] server time
	"svc_print",			// [string] null terminated string
	"svc_stufftext",		// [string] stuffed into client's console buffer
						// the string should be \n terminated
	"svc_setangle",		// [vec3] set the view angle to this absolute value

	"svc_serverinfo",		// [long] version
						// [string] signon string
						// [string]..[0]model cache [string]...[0]sounds cache
						// [string]..[0]item cache
	"svc_lightstyle",		// [byte] [string]
	"svc_updatename",		// [byte] [string]
	"svc_updatefrags",	// [byte] [short]
	"svc_clientdata",		// <shortbits + data>
	"svc_stopsound",		// <see code>
	"svc_updatecolors",	// [byte] [byte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",			// [byte] impact [byte] blood [vec3] from
	"svc_spawnstatic",
	"SVC_FLARE",
	"svc_spawnbaseline",
	"svc_temp_entity",		// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",			// [string] music [string] text
	"svc_cdtrack",			// [byte] track [byte] looptrack
	"svc_cutscene",
//johnfitz -- new server messages
	"",	// 35
	"",	// 36
	"svc_skybox", // 37					// [string] skyname
	"",								// 38
	"",								// 39
	"svc_bf",						// 40		// no data
	"svc_fog", 						// [byte] density [byte] red [byte] green [byte] blue [float] time
	"svc_spawnbaseline2", 			// support for large modelindex, large framenum, alpha, using flags
	"svc_spawnstatic2", 			// support for large modelindex, large framenum, alpha, using flags
	"svc_spawnstaticsound2",		// [coord3] [short] samp [byte] vol [byte] aten
//johnfitz
	"SVC_UPDATEMENU"
};

extern vec3_t	v_punchangles[2]; //johnfitz

//=============================================================================

/*	This error checks and tracks the total number of entities.
*/
entity_t	*CL_EntityNum (int num)
{
	//johnfitz -- check minimum number too
	if (num < 0)
		Host_Error ("CL_EntityNum: %i is an invalid number",num);
	//john

	if (num >= cl.num_entities)
	{
		if (num >= cl_max_edicts) //johnfitz -- no more MAX_EDICTS
			Host_Error ("CL_EntityNum: %i is an invalid number",num);
		while (cl.num_entities<=num)
		{
			cl_entities[cl.num_entities].colormap = 0;	// Obsolete
			cl_entities[cl.num_entities].lerpflags |= LERP_RESETMOVE|LERP_RESETANIM; //johnfitz
			cl.num_entities++;
		}
	}

	return &cl_entities[num];
}

void CL_ParseStartSoundPacket(void)
{
	vec3_t  pos;
	int 	channel,ent,sound_num,volume,field_mask;
	float 	attenuation;
	int		i;

	field_mask = MSG_ReadByte();

	if (field_mask & SND_VOLUME)
		volume = MSG_ReadByte ();
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;

	if (field_mask & SND_ATTENUATION)
		attenuation = MSG_ReadByte()/64.0f;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (field_mask & SND_LARGEENTITY)
	{
		ent = (unsigned short) MSG_ReadShort ();
		channel = MSG_ReadByte ();
	}
	else
	{
		channel = (unsigned short) MSG_ReadShort ();
		ent = channel >> 3;
		channel &= 7;
	}

	if (field_mask & SND_LARGESOUND)
		sound_num = (unsigned short) MSG_ReadShort ();
	else
		sound_num = MSG_ReadByte ();
	//johnfitz

	//johnfitz -- check soundnum
	if (sound_num >= MAX_SOUNDS)
		Host_Error ("CL_ParseStartSoundPacket: %i > MAX_SOUNDS", sound_num);
	//johnfitz

	if (ent > cl_max_edicts) //johnfitz -- no more MAX_EDICTS
		Host_Error ("CL_ParseStartSoundPacket: ent = %i", ent);

	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord();

	S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume, attenuation);
}

/*	When the client is taking a long time to load stuff, send keepalive messages
	so the server doesn't disconnect.
*/
void CL_KeepaliveMessage (void)
{
	float	time;
	static float lastmsg;
	int		ret;
	sizebuf_t	old;
	byte		olddata[8192];

	if (sv.active)
		return;		// no need if server is local
	if (cls.demoplayback)
		return;

// read messages from server, should just be nops
	old = net_message;
	memcpy (olddata, net_message.data, net_message.cursize);

	do
	{
		ret = CL_GetMessage ();
		switch (ret)
		{
		default:
			Host_Error ("CL_KeepaliveMessage: CL_GetMessage failed");
		case 0:
			break;	// nothing waiting
		case 1:
			Host_Error ("CL_KeepaliveMessage: received a message");
			break;
		case 2:
			if(MSG_ReadByte() != SVC_NOP)
				Host_Error("CL_KeepaliveMessage: datagram wasn't a nop");
			break;
		}
	} while (ret);

	net_message = old;
	memcpy (net_message.data, olddata, net_message.cursize);

// check time
	time = Sys_FloatTime ();
	if (time - lastmsg < 5)
		return;
	lastmsg = time;

// write out a nop
	Con_DPrintf ("--> client to server keepalive\n");

	MSG_WriteByte (&cls.message, clc_nop);
	NET_SendMessage (cls.netcon, &cls.message);
	SZ_Clear (&cls.message);
}

/*	Parse information given to us by the server.
*/
void Client_ParseServerInfo(void)
{
	int		i,
			nummodels, numsounds;
	char	*str,
			cModelPrecache[MAX_MODELS][MAX_QPATH],
			cSoundPrecache[MAX_SOUNDS][MAX_QPATH];

	Con_DPrintf("Parsing server information...\n");

	// Wipe the client_state_t struct
	CL_ClearState();

	// Parse protocol version number
	i = MSG_ReadLong();
	if(i != SERVER_PROTOCOL)
	{
		Con_Printf("\n");
		Host_Error("Server returned protocol version %i, not %i!\n",i,SERVER_PROTOCOL);
	}

	cl.protocol = i;

	Con_DPrintf(" PROTOCOL %i\n",i);

	// Parse maxclients
	cl.maxclients = MSG_ReadByte ();
	if (cl.maxclients < 1 || cl.maxclients > MAX_SCOREBOARD)
	{
		Con_Printf("Bad maxclients (%u) from server\n", cl.maxclients);
		return;
	}
	cl.scores = (scoreboard_t*)Hunk_AllocName(cl.maxclients*sizeof(*cl.scores), "scores");

	// Parse gametype
	cl.gametype = MSG_ReadByte();

// parse signon message
	str = MSG_ReadString ();
	strncpy(cl.levelname,str,sizeof(cl.levelname)-1);

// seperate the printfs so the server message can have a color
	Con_Printf ("\n%s\n", Con_Quakebar(40)); //johnfitz
	Con_Printf ("%c%s\n", 2, str);

// first we go through and touch all of the precache data that still
// happens to be in the cache, so precaching something else doesn't
// needlessly purge it

	// precache models
	memset(cl.model_precache,0,sizeof(cl.model_precache));
	for(nummodels = 1;; nummodels++)
	{
		str = MSG_ReadString ();
		if(!str[0])
			break;

		if(nummodels == MAX_MODELS)
		{
			Con_Warning("Server sent too many model precaches!\n");
			return;
		}

		strcpy(cModelPrecache[nummodels],str);
		Model_Touch(str);
	}

	// precache sounds
	memset (cl.sound_precache, 0, sizeof(cl.sound_precache));
	for (numsounds=1 ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;

		if (numsounds==MAX_SOUNDS)
		{
			Con_Printf ("Server sent too many sound precaches\n");
			return;
		}

		strcpy (cSoundPrecache[numsounds], str);
		S_TouchSound (str);
	}

	// [6/6/2013] Precache textures ~hogsy
	{
		int iTextures;

		for(iTextures = 1; ; iTextures++)
		{
			str = MSG_ReadString();
			if(!str[0])
				break;

			Client_PrecacheResource(RESOURCE_PARTICLE,str);
		}
	}

	// [16/1/2014] Add lights :) ~hogsy
	{
		int	i;

		for(i = 0; ; i++)
		{
			if(!MSG_ReadByte())
				break;

			{
				DynamicLight_t *dStaticLight = Client_AllocDlight(0);
				if(dStaticLight)
				{
					int j;

					for(j = 0; j < 3; j++)
						dStaticLight->origin[j] = MSG_ReadCoord();

					for(j = 0; j < 3; j++)
						dStaticLight->color[j] = MSG_ReadCoord();

					dStaticLight->decay		= 0;
					dStaticLight->bLightmap	= false;
					dStaticLight->die		= 0;
					dStaticLight->minlight	= 32.0f;
					dStaticLight->radius	= MSG_ReadCoord();
				}
			}
		}
	}

	// now we try to load everything else until a cache allocation fails
	for(i = 1; i < nummodels; i++)
	{
		cl.model_precache[i] = Mod_ForName(cModelPrecache[i]);
		if(!cl.model_precache[i])
			Console_ErrorMessage(false,cModelPrecache[i],"Either the file is corrupt or does not exist.\n");

		CL_KeepaliveMessage ();
	}

	for (i=1 ; i<numsounds ; i++)
	{
		cl.sound_precache[i] = S_PrecacheSound(cSoundPrecache[i]);
		CL_KeepaliveMessage ();
	}

	// Local state
	cl_entities[0].model = cl.worldmodel = cl.model_precache[1];

	R_NewMap ();

	//johnfitz -- clear out string; we don't consider identical
	//messages to be duplicates if the map has changed in between
	con_lastcenterstring[0] = 0;
	//johnfitz

	Hunk_Check ();		// make sure nothing is hurt

	noclip_anglehack = false;		// noclip is turned off at start

//johnfitz -- reset developer stats
	memset(&dev_stats, 0, sizeof(dev_stats));
	memset(&dev_peakstats, 0, sizeof(dev_peakstats));
	memset(&dev_overflows, 0, sizeof(dev_overflows));
}

/*
Parse an entity update message from the server
If an entities model or origin changes from frame to frame, it must be
relinked.  Other attributes can change without relinking.
*/
int	bitcounts[16];

void CL_ParseUpdate(int bits)
{
	int			i;
	model_t		*model;
	bool		bForceLink;
	entity_t	*ent;
	int			modnum,
				num,
				skin;

	if (cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		cls.signon = SIGNONS;
		CL_SignonReply ();
	}

	if (bits & U_MOREBITS)
	{
		i = MSG_ReadByte ();
		bits |= (i<<8);
	}

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (bits & U_EXTEND1)
		bits |= MSG_ReadByte() << 16;
	if (bits & U_EXTEND2)
		bits |= MSG_ReadByte() << 24;
	//johnfitz

	if (bits & U_LONGENTITY)
		num = MSG_ReadShort ();
	else
		num = MSG_ReadByte ();

	ent = CL_EntityNum(num);

	for (i=0 ; i<16 ; i++)
		if (bits&(1<<i))
			bitcounts[i]++;

	if (ent->msgtime != cl.mtime[1])
		bForceLink = true;	// no previous frame to lerp from
	else
		bForceLink = false;

	//johnfitz -- lerping
	if (ent->msgtime + 0.2 < cl.mtime[0]) //more than 0.2 seconds since the last message (most entities think every 0.1 sec)
		ent->lerpflags |= LERP_RESETANIM; //if we missed a think, we'd be lerping from the wrong frame
	//johnfitz

	ent->msgtime = cl.mtime[0];

	if (bits & U_MODEL)
	{
		modnum = MSG_ReadByte ();
		if (modnum >= MAX_MODELS)
			Host_Error ("CL_ParseModel: bad modnum");
	}
	else
		modnum = ent->baseline.modelindex;

	if (bits & U_FRAME)
		ent->frame = MSG_ReadByte ();
	else
		ent->frame = ent->baseline.frame;

	if(bits & U_SCALE)
		ent->scale	= MSG_ReadFloat();
	else
		ent->scale	= ent->baseline.fScale;

	if (bits & U_COLORMAP)
		i = MSG_ReadByte();
	else
		i = ent->baseline.colormap;

	if(i > cl.maxclients)
		Sys_Error("i >= cl.maxclients");

	if (bits & U_SKIN)
		skin = MSG_ReadByte();
	else
		skin = ent->baseline.skin;

	if(skin != ent->skinnum)
		ent->skinnum = skin;

	if (bits & U_EFFECTS)
		// [21/12/2013] Fixed this not being sent over in full form ~hogsy
		ent->effects = MSG_ReadLong();
	else
		ent->effects = ent->baseline.effects;

	// Shift the known values for interpolation
	Math_VectorCopy(ent->msg_origins[0],ent->msg_origins[1]);
	Math_VectorCopy(ent->msg_angles[0],ent->msg_angles[1]);

	if(bits & U_ORIGIN1)
		ent->msg_origins[0][0] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][0] = ent->baseline.origin[0];

	if(bits & U_ANGLE1)
		ent->msg_angles[0][0] = MSG_ReadAngle();
	else
		ent->msg_angles[0][0] = ent->baseline.angles[0];

	if(bits & U_ORIGIN2)
		ent->msg_origins[0][1] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][1] = ent->baseline.origin[1];

	if(bits & U_ANGLE2)
		ent->msg_angles[0][1] = MSG_ReadAngle();
	else
		ent->msg_angles[0][1] = ent->baseline.angles[1];

	if(bits & U_ORIGIN3)
		ent->msg_origins[0][2] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][2] = ent->baseline.origin[2];

	if (bits & U_ANGLE3)
		ent->msg_angles[0][2] = MSG_ReadAngle();
	else
		ent->msg_angles[0][2] = ent->baseline.angles[2];

	//johnfitz -- lerping for movetype_step entities
	if ( bits & U_STEP )
	{
		ent->lerpflags |= LERP_MOVESTEP;
		ent->bForceLink = true;
	}
	else
		ent->lerpflags &= ~LERP_MOVESTEP;
	//johnfitz

	if (bits & U_ALPHA)
		ent->alpha = MSG_ReadByte();
	else
		ent->alpha = ent->baseline.alpha;

	if (bits & U_FRAME2)
		ent->frame = (ent->frame & 0x00FF) | (MSG_ReadByte() << 8);

	if (bits & U_MODEL2)
		modnum = (modnum & 0x00FF) | (MSG_ReadByte() << 8);

	if (bits & U_LERPFINISH)
	{
		ent->lerpfinish = ent->msgtime + ((float)(MSG_ReadByte()) / 255);
		ent->lerpflags |= LERP_FINISH;
	}
	else
		ent->lerpflags &= ~LERP_FINISH;
	//johnfitz

	//johnfitz -- moved here from above
	model = cl.model_precache[modnum];
	if(model != ent->model)
	{
		ent->model = model;
	// automatic animation (torches, etc) can be either all together
	// or randomized
		if(!model)
			bForceLink = true;	// hack to make null model players work

		ent->lerpflags |= LERP_RESETANIM; //johnfitz -- don't lerp animation across model changes
	}
	//johnfitz

	if(bForceLink)
	{
		// Didn't have an update last message
		Math_VectorCopy(ent->msg_origins[0],ent->msg_origins[1]);
		Math_VectorCopy(ent->msg_origins[0],ent->origin);
		Math_VectorCopy(ent->msg_angles[0],ent->msg_angles[1]);
		Math_VectorCopy(ent->msg_angles[0],ent->angles);

		ent->bForceLink = true;
	}
}

void CL_ParseBaseline (entity_t *ent, int version) //johnfitz -- added argument
{
	int	i;
	int bits; //johnfitz

	//johnfitz -- PROTOCOL_FITZQUAKE
	bits = (version == 2) ? MSG_ReadByte() : 0;

	ent->baseline.modelindex	= (bits & B_LARGEMODEL) ? MSG_ReadShort() : MSG_ReadByte();
	ent->baseline.frame			= (bits & B_LARGEFRAME) ? MSG_ReadShort() : MSG_ReadByte();
	//johnfitz

	ent->baseline.fScale	= MSG_ReadFloat();
	ent->baseline.colormap	= MSG_ReadByte();
	ent->baseline.skin		= MSG_ReadByte();

	for(i = 0; i < 3; i++)
	{
		ent->baseline.origin[i] = MSG_ReadCoord ();
		ent->baseline.angles[i] = MSG_ReadAngle ();
	}

	ent->baseline.alpha = (bits & B_ALPHA) ? MSG_ReadByte() : ENTALPHA_DEFAULT; //johnfitz -- PROTOCOL_FITZQUAKE
}

/*	Server information pertaining to this client only
*/
void CL_ParseClientdata (void)
{
	int		i,j,bits; //johnfitz

	bits = (unsigned short)MSG_ReadShort (); //johnfitz -- read bits here isntead of in CL_ParseServerMessage()

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (bits & SU_EXTEND1)
		bits |= (MSG_ReadByte() << 16);
	if (bits & SU_EXTEND2)
		bits |= (MSG_ReadByte() << 24);
	//johnfitz

	if (bits & SU_VIEWHEIGHT)
		cl.viewheight = MSG_ReadChar ();
	else
		cl.viewheight = DEFAULT_VIEWHEIGHT;

	if (bits & SU_IDEALPITCH)
		cl.idealpitch = MSG_ReadChar ();
	else
		cl.idealpitch = 0;

	Math_VectorCopy(cl.mvelocity[0],cl.mvelocity[1]);
	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i) )
			cl.punchangle[i] = MSG_ReadChar();
		else
			cl.punchangle[i] = 0;

		if (bits & (SU_VELOCITY1<<i) )
			cl.mvelocity[0][i] = MSG_ReadChar()*16;
		else
			cl.mvelocity[0][i] = 0;
	}

	//johnfitz -- update v_punchangles
	if (v_punchangles[0][0] != cl.punchangle[0] || v_punchangles[0][1] != cl.punchangle[1] || v_punchangles[0][2] != cl.punchangle[2])
	{
		Math_VectorCopy(v_punchangles[0],v_punchangles[1]);
		Math_VectorCopy(cl.punchangle,v_punchangles[0]);
	}
	//johnfitz

// [always sent]	if (bits & SU_ITEMS)
	i = MSG_ReadLong ();
	if (cl.items != i)
	{
		// set flash times
		for (j=0 ; j<32 ; j++)
			if ( (i & (1<<j)) && !(cl.items & (1<<j)))
				cl.item_gettime[j] = cl.time;
		cl.items = i;
	}

	cl.bIsOnGround	= (bits & SU_ONGROUND) != 0;
	cl.bIsInWater	= (bits & SU_INWATER) != 0;

	if (bits & SU_WEAPONFRAME)
		cl.stats[STAT_WEAPONFRAME] = MSG_ReadByte ();
	else
		cl.stats[STAT_WEAPONFRAME] = 0;

	if(bits & SU_ARMOR)
		i = MSG_ReadByte ();
	else
		i = 0;

	if (cl.stats[STAT_ARMOR] != i)
		cl.stats[STAT_ARMOR] = i;

	if (bits & SU_WEAPON)
		i = MSG_ReadByte ();
	else
		i = 0;

	if (cl.stats[STAT_WEAPON] != i)
	{
		cl.stats[STAT_WEAPON] = i;

		//johnfitz -- lerping
		if (cl.viewent.model != cl.model_precache[cl.stats[STAT_WEAPON]])
			cl.viewent.lerpflags |= LERP_RESETANIM; //don't lerp animation across model changes
		//johnfitz
	}

	i = MSG_ReadShort ();
	if (cl.stats[STAT_HEALTH] != i)
		cl.stats[STAT_HEALTH] = i;

	i = MSG_ReadByte ();
	if (cl.stats[STAT_AMMO] != i)
		cl.stats[STAT_AMMO] = i;

	i = MSG_ReadByte ();
	if(cl.stats[STAT_ACTIVEWEAPON] != i)
		cl.stats[STAT_ACTIVEWEAPON] = i;

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (bits & SU_WEAPON2)
		cl.stats[STAT_WEAPON] |= (MSG_ReadByte() << 8);

	if (bits & SU_ARMOR2)
		cl.stats[STAT_ARMOR] |= (MSG_ReadByte() << 8);

	if (bits & SU_AMMO2)
		cl.stats[STAT_AMMO] |= (MSG_ReadByte() << 8);

	if (bits & SU_WEAPONFRAME2)
		cl.stats[STAT_WEAPONFRAME] |= (MSG_ReadByte() << 8);

	if (bits & SU_WEAPONALPHA)
		cl.viewent.alpha = MSG_ReadByte();
	else
		cl.viewent.alpha = ENTALPHA_DEFAULT;
	//johnfitz
}

void CL_ParseStatic (int version) //johnfitz -- added a parameter
{
	entity_t *ent;
	int		i;

	i = cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_Error ("Too many static entities");

	ent = &cl_static_entities[i];
	cl.num_statics++;
	CL_ParseBaseline (ent, version); //johnfitz -- added second parameter

// copy it to the current state

	ent->model		= cl.model_precache[ent->baseline.modelindex];
	ent->lerpflags	|= LERP_RESETANIM; //johnfitz -- lerping
	ent->frame		= ent->baseline.frame;
	ent->skinnum	= ent->baseline.skin;
	ent->effects	= ent->baseline.effects;
	ent->alpha		= ent->baseline.alpha; //johnfitz -- alpha

	Math_VectorCopy (ent->baseline.origin, ent->origin);
	Math_VectorCopy (ent->baseline.angles, ent->angles);

	R_AddEfrags (ent);
}

void CL_ParseStaticSound (int version) //johnfitz -- added argument
{
	vec3_t		org;
	int			sound_num, vol, atten;
	int			i;

	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (version == 2)
		sound_num = MSG_ReadShort ();
	else
		sound_num = MSG_ReadByte ();
	//johnfitz

	vol = MSG_ReadByte ();
	atten = MSG_ReadByte ();

	S_StaticSound (cl.sound_precache[sound_num], org, vol, atten);
}

#include "engine_game.h"

#define SHOWNET(x) if(cl_shownet.value==2)Con_Printf ("%3i:%s\n", msg_readcount-1, x);

void Sky_LoadSkyBox(char *name);

void CL_ParseServerMessage(void)
{
	int			cmd;
	int			i;
	char		*str; //johnfitz
	// [28/7/2012] Set lastcmd to 0 to resolve warning ~hogsy
	int			total,j,lastcmd = 0; //johnfitz

	// if recording demos, copy the message out
	if (cl_shownet.value == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.value == 2)
		Con_Printf ("------------------\n");

	cl.bIsOnGround = false;	// unless the server says otherwise

	// parse the message
	MSG_BeginReading();

	for(;;)
	{
		if (msg_badread)
			Host_Error("CL_ParseServerMessage: Bad server message");

		cmd = MSG_ReadByte ();

		if (cmd == -1)
		{
			SHOWNET("END OF MESSAGE");
			return;		// end of message
		}

	// if the high bit of the command byte is set, it is a fast update
		if (cmd & U_SIGNAL) //johnfitz -- was 128, changed for clarity
		{
			SHOWNET("fast update");
			CL_ParseUpdate (cmd&127);
			continue;
		}

		SHOWNET(svc_strings[cmd]);

	// other commands
		switch(cmd)
		{
		default:
			Host_Error("Illegible server message! (%s) (%i)\n",svc_strings[lastcmd],cmd); //johnfitz -- added svc_strings[lastcmd]
			break;
		case SVC_NOP:
			break;
		case svc_time:
			cl.mtime[1] = cl.mtime[0];
			cl.mtime[0] = MSG_ReadFloat ();
			break;
		case svc_clientdata:
			CL_ParseClientdata (); //johnfitz -- removed bits parameter, we will read this inside CL_ParseClientdata()
			break;
		case svc_version:
			i = MSG_ReadLong ();
			if(i != SERVER_PROTOCOL)
				Host_Error("Server returned protocol version %i, not %i!\n",i,SERVER_PROTOCOL);

			cl.protocol = i;
			break;
		case SVC_DISCONNECT:
			// [17/9/2013] Stop drawing the HUD on disconnect ~hogsy
			Menu->RemoveState(MENU_STATE_HUD);

			Host_EndGame("Server disconnected\n");
		case svc_print:
			Con_Printf ("%s", MSG_ReadString ());
			break;
		case svc_centerprint:
			//johnfitz -- log centerprints to console
			str = MSG_ReadString ();
			SCR_CenterPrint (str);
			Con_LogCenterPrint (str);
			//johnfitz
			break;
		case svc_stufftext:
			Cbuf_AddText (MSG_ReadString ());
			break;
		case SVC_DAMAGE:
			V_ParseDamage ();
			break;
		case svc_serverinfo:
			Client_ParseServerInfo();
			vid.bRecalcRefDef = true;	// leave intermission full screen
			break;
		case svc_setangle:
			for (i=0 ; i<3 ; i++)
				cl.viewangles[i] = MSG_ReadAngle ();
			break;
		case svc_setview:
			cl.viewentity = MSG_ReadShort ();
			break;
		case svc_lightstyle:
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
				Sys_Error ("svc_lightstyle > MAX_LIGHTSTYLES");
			Q_strcpy (cl_lightstyle[i].cMap,  MSG_ReadString());
			cl_lightstyle[i].length = Q_strlen(cl_lightstyle[i].cMap);
			//johnfitz -- save extra info
			if (cl_lightstyle[i].length)
			{
				total = 0;
				cl_lightstyle[i].peak = 'a';
				for (j=0; j<cl_lightstyle[i].length; j++)
				{
					total += cl_lightstyle[i].cMap[j] - 'a';
					cl_lightstyle[i].peak = Math_Max(cl_lightstyle[i].peak, cl_lightstyle[i].cMap[j]);
				}
				cl_lightstyle[i].average = total / cl_lightstyle[i].length + 'a';
			}
			else
				cl_lightstyle[i].average = cl_lightstyle[i].peak = 'm';
			//johnfitz
			break;
		case SVC_SOUND:
			CL_ParseStartSoundPacket();
			break;
		case SVC_STOPSOUND:
			i = MSG_ReadShort();
			S_StopSound(i>>3, i&7);
			break;
		case svc_updatename:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatename > MAX_SCOREBOARD");
			strcpy (cl.scores[i].name, MSG_ReadString ());
			break;
		case svc_updatefrags:
			i = MSG_ReadByte();
			if(i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatefrags > MAX_SCOREBOARD");

			cl.scores[i].frags = MSG_ReadShort();
			cl.stats[STAT_FRAGS] = cl.scores[i].frags;
			break;
		case svc_updatecolors:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatecolors > MAX_SCOREBOARD");
			cl.scores[i].colors = MSG_ReadByte ();
			break;
		case SVC_PARTICLE:
			Client_ParseParticleEffect();
			break;
		case SVC_FLARE:
			Client_ParseFlareEffect();
			break;
		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
			CL_ParseBaseline (CL_EntityNum(i), 1); // johnfitz -- added second parameter
			break;
		case svc_spawnstatic:
			CL_ParseStatic (1); //johnfitz -- added parameter
			break;
		case SVC_TEMPENTITY:
			// [19/6/2013] Now handed over directly to the game-code :) ~hogsy
			Game->Client_ParseTemporaryEntity();
			break;
		case svc_setpause:
			cl.bIsPaused = MSG_ReadByte();

			if(cl.bIsPaused)
				Menu->AddState(MENU_STATE_PAUSED);
			else
				Menu->RemoveState(MENU_STATE_PAUSED);
			break;
		case svc_signonnum:
			i = MSG_ReadByte ();
			if (i <= cls.signon)
				Host_Error ("Received signon %i when at %i", i, cls.signon);
			cls.signon = i;
			//johnfitz -- if signonnum==2, signon packet has been fully parsed, so check for excessive static ents and efrags
			if (i == 2)
				R_CheckEfrags ();
			//johnfitz
			CL_SignonReply ();
			break;
		case svc_killedmonster:
			cl.stats[STAT_MONSTERS]++;
			break;
		case svc_foundsecret:
			cl.stats[STAT_SECRETS]++;
			break;
		case SVC_UPDATESTAT:
			i = MSG_ReadByte ();
			if(i < 0 || i >= STAT_NONE)
				Sys_Error("svc_updatestat: %i is invalid",i);

			cl.stats[i] = MSG_ReadLong();
			break;
		case svc_spawnstaticsound:
			CL_ParseStaticSound (1); //johnfitz -- added parameter
			break;
		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			cl.looptrack = MSG_ReadByte ();

			// TODO: Play music
			break;
		case svc_intermission:
			cl.intermission = 1;
			cl.completed_time = cl.time;
			vid.bRecalcRefDef = true;	// go to full screen
			break;
		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = cl.time;
			vid.bRecalcRefDef = true;	// go to full screen
			//johnfitz -- log centerprints to console
			str = MSG_ReadString ();
			SCR_CenterPrint (str);
			Con_LogCenterPrint (str);
			//johnfitz
			break;
		case SVC_CUTSCENE:
			cl.intermission = 3;
			cl.completed_time = cl.time;
			vid.bRecalcRefDef = true;	// go to full screen
			//johnfitz -- log centerprints to console
			str = MSG_ReadString ();
			SCR_CenterPrint (str);
			Con_LogCenterPrint (str);
			//johnfitz
			break;
		//johnfitz -- new svc types
		case svc_skybox:
			Sky_LoadSkyBox(MSG_ReadString());
			break;
		case SVC_BF:
			Cmd_ExecuteString("bf",src_command);
			break;
		case SVC_FOG:
			Fog_ParseServerMessage ();
			break;
		case svc_spawnbaseline2:		// 666
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
			CL_ParseBaseline (CL_EntityNum(i), 2);
			break;
		case svc_spawnstatic2:			// 666
			CL_ParseStatic (2);
			break;
		case svc_spawnstaticsound2:		// 666
			CL_ParseStaticSound (2);
			break;
		//johnfitz
		case SVC_UPDATEMENU:
			{
				int iState	= MSG_ReadByte(),
					iShow	= MSG_ReadByte();

				if(iShow)
					Menu->AddState(iState);
				else
					Menu->RemoveState(iState);
			}
			break;
		}

		lastcmd = cmd; //johnfitz
	}
}
