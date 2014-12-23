/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2015 OldTimes Software

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

#include "engine_modgame.h"
#include "engine_physics.h"
#include "engine_edict.h"

#include "shared_server.h"

server_t		sv;
server_static_t	svs;

char	localmodels[MAX_MODELS][5];			// inline model names for precache

void SV_Init (void)
{
	int				i;
	extern	cvar_t	cvPhysicsNoStep;
	extern	cvar_t	sv_edgefriction;
	extern	cvar_t	cvPhysicsStopSpeed;
	extern	cvar_t	sv_maxspeed;
	extern	cvar_t	sv_accelerate;
	extern	cvar_t	sv_idealpitchscale;
	extern	cvar_t	sv_aim;
	extern	cvar_t	sv_altnoclip; //johnfitz
	extern	cvar_t	cvPhysicsStepSize;

	Cvar_RegisterVariable(&sv_edgefriction, NULL);
	Cvar_RegisterVariable(&cvPhysicsStopSpeed, NULL);
	Cvar_RegisterVariable(&sv_maxspeed, NULL);
	Cvar_RegisterVariable(&sv_accelerate, NULL);
	Cvar_RegisterVariable(&sv_idealpitchscale, NULL);
	Cvar_RegisterVariable(&sv_aim,NULL);
	Cvar_RegisterVariable(&cvPhysicsNoStep, NULL);
	Cvar_RegisterVariable(&sv_altnoclip,NULL); //johnfitz
	Cvar_RegisterVariable(&cvPhysicsStepSize,NULL);

	for(i = 0; i < MAX_MODELS; i++)
		sprintf(localmodels[i],"*%i",i);
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*	Each entity can have eight independant sound sources, like voice,
	weapon, feet, etc.

	Channel 0 is an auto-allocate channel, the others override anything
	allready running on that entity/channel pair.

	An attenuation of 0 will play full volume everywhere in the level.
	Larger attenuations will drop off.  (max 4 attenuation)
*/
void SV_StartSound(edict_t *entity,int channel,char *sample,int volume,float attenuation)
{
	int	sound_num,field_mask,i,ent;

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if(attenuation < 0 || attenuation > 4.0f)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

// find precache number for sound
	for (sound_num=1 ; sound_num<MAX_SOUNDS
		&& sv.sound_precache[sound_num] ; sound_num++)
		if (!strcmp(sample, sv.sound_precache[sound_num]))
			break;

	if ( sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num] )
	{
		Con_Printf ("SV_StartSound: %s not precacheed\n", sample);
		return;
	}

	ent = NUM_FOR_EDICT(entity);

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;
	if (ent >= 8192)
		field_mask |= SND_LARGEENTITY;
	if (sound_num >= 256 || channel >= 8)
		field_mask |= SND_LARGESOUND;

// directed messages go only to the entity the are targeted on
	MSG_WriteByte (&sv.datagram, svc_sound);
	MSG_WriteByte (&sv.datagram, field_mask);
	if (field_mask & SND_VOLUME)
		MSG_WriteByte (&sv.datagram, volume);
	if (field_mask & SND_ATTENUATION)
		MSG_WriteByte (&sv.datagram, attenuation*64);

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (field_mask & SND_LARGEENTITY)
	{
		MSG_WriteShort (&sv.datagram, ent);
		MSG_WriteByte (&sv.datagram, channel);
	}
	else
		MSG_WriteShort (&sv.datagram, (ent<<3) | channel);
	if (field_mask & SND_LARGESOUND)
		MSG_WriteShort (&sv.datagram, sound_num);
	else
		MSG_WriteByte (&sv.datagram, sound_num);
	//johnfitz

	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.datagram, entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i]));
}

/*
==============================================================================

CLIENT SPAWNING

==============================================================================
*/

/*	Sends the first message from the server to a connected client.
	This will be sent on the initial connection and upon each server load.
*/
void SV_SendServerinfo(client_t *client)
{
	char	**s,message[2048];
	int		i; //johnfitz

	MSG_WriteByte(&client->message,svc_print);
	sprintf(message,"%c\n%s Server\n   Release %i\n   Build %i\n",2,Game->Name,ENGINE_VERSION_MAJOR,ENGINE_VERSION_BUILD); //johnfitz -- include fitzquake version
	MSG_WriteString(&client->message,message);

	MSG_WriteByte(&client->message,svc_serverinfo);
	MSG_WriteLong(&client->message,sv.protocol);
	MSG_WriteByte(&client->message,svs.maxclients);

	if(!coop.value && deathmatch.value)
		MSG_WriteByte(&client->message,GAME_DEATHMATCH);
	else
		MSG_WriteByte(&client->message,GAME_COOP);

	if(sv.edicts->v.message)
	{
		sprintf(message,sv.edicts->v.message);

		MSG_WriteString(&client->message,message);
	}
	else
		MSG_WriteString(&client->message,0);

	//johnfitz -- only send the first 256 model and sound precaches if protocol is 15
	for(i = 0,s = sv.model_precache+1; *s; s++,i++)
		if(i < 256)
			MSG_WriteString(&client->message,*s);

	MSG_WriteByte (&client->message, 0);

	for (i=0,s = sv.sound_precache+1 ; *s ; s++,i++)
		if (i < 256)
			MSG_WriteString(&client->message,*s);

	MSG_WriteByte(&client->message,0);
	//johnfitz

	// [6/6/2013] Send over textures ~hogsy
	for(i = 0,s = sv.cParticlePrecache+1; *s; s++,i++)
		MSG_WriteString(&client->message,*s);

	MSG_WriteByte(&client->message,0);

	// [16/1/2014] Send over lights ~hogsy
	{
		edict_t *eLight = NEXT_EDICT(sv.edicts);

		for(i = 1; i < sv.num_edicts; i++,eLight = NEXT_EDICT(eLight))
		{
			if(eLight->free)
				continue;

			if(eLight->v.cClassname && !strcmp("light",eLight->v.cClassname))
			{
				int	j;

				MSG_WriteByte(&client->message,1);

				for(j = 0; j < 3; j++)
					MSG_WriteCoord(&client->message,eLight->v.origin[j]);

				for(j = 0; j < 4; j++)
					MSG_WriteCoord(&client->message,eLight->v.vLight[j]);
			}
		}

		MSG_WriteByte(&client->message,0);
	}

	// Send music
	MSG_WriteByte(&client->message,svc_cdtrack);
	MSG_WriteByte(&client->message,sv.edicts->v.sounds);
	MSG_WriteByte(&client->message,sv.edicts->v.sounds);

	// Set the view
	MSG_WriteByte(&client->message,svc_setview);
	MSG_WriteShort(&client->message,NUM_FOR_EDICT(client->edict));

	MSG_WriteByte(&client->message,svc_signonnum);
	MSG_WriteByte(&client->message,1);

	client->sendsignon	= true;
	client->bSpawned	= false;		// need prespawn, spawn, etc
}

/*	Initializes a client_t for a new net connection.  This will only be called
	once for a player each game, not once for each level change.
*/
void SV_ConnectClient (int clientnum)
{
	edict_t			*ent;
	client_t		*client;
	int				edictnum;
	struct qsocket_s *netconnection;
	int				i;
	float			spawn_parms[NUM_SPAWN_PARMS];

	client = svs.clients + clientnum;

	Con_DPrintf ("Client %s connected\n", client->netconnection->address);

	edictnum = clientnum+1;

	ent = EDICT_NUM(edictnum);

	// set up the client_t
	netconnection = client->netconnection;

	if (sv.loadgame)
		memcpy (spawn_parms, client->spawn_parms, sizeof(spawn_parms));
	memset (client, 0, sizeof(*client));
	client->netconnection = netconnection;

	strcpy(client->name,"unconnected");
	client->active					= true;
	client->bSpawned				= false;
	client->edict					= ent;
	client->message.data			= client->msgbuf;
	client->message.maxsize			= sizeof(client->msgbuf);
	client->message.allowoverflow	= true;		// we can catch it
	client->privileged				= false;

	if(sv.loadgame)
		memcpy(client->spawn_parms, spawn_parms, sizeof(spawn_parms));
	else
	{
		// Call the progs to get default spawn parms for the new client
		Game->Game_Init(SERVER_SETNEWPARMS,client->edict,sv.time);
		for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
			client->spawn_parms[i] = (&pr_global_struct.parm1)[i];
	}

	SV_SendServerinfo(client);
}

void SV_CheckForNewClients (void)
{
	struct qsocket_s	*ret;
	int				i;

	// check for new connections
	for(;;)
	{
		ret = NET_CheckNewConnections ();
		if (!ret)
			break;

		// init a new client structure
		for (i=0 ; i<svs.maxclients ; i++)
			if (!svs.clients[i].active)
				break;
		if (i == svs.maxclients)
			Sys_Error("Host_CheckForNewClients: no free clients");

		svs.clients[i].netconnection = ret;
		SV_ConnectClient (i);

		iActiveNetConnections++;
	}
}

/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

void SV_ClearDatagram (void)
{
	SZ_Clear (&sv.datagram);
}

/*
=============================================================================

The PVS must include a small area around the client to allow head bobbing
or other small motion on the client side.  Otherwise, a bob might cause an
entity that should be visible to not show up, especially when the bob
crosses a waterline.

=============================================================================
*/

int		fatbytes;
byte	fatpvs[BSP_MAX_LEAFS/8];

void SV_AddToFatPVS (vec3_t org, mnode_t *node, model_t *worldmodel) //johnfitz -- added worldmodel as a parameter
{
	int		i;
	byte	*pvs;
	mplane_t	*plane;
	float	d;

	for(;;)
	{
		// If this is a leaf, accumulate the pvs bits
		if (node->contents < 0)
		{
			if (node->contents != BSP_CONTENTS_SOLID)
			{
				pvs = Mod_LeafPVS ( (mleaf_t *)node, worldmodel); //johnfitz -- worldmodel as a parameter
				for (i=0 ; i<fatbytes ; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}

		plane = node->plane;
		d = Math_DotProduct (org, plane->normal) - plane->dist;
		if (d > 8)
			node = node->children[0];
		else if (d < -8)
			node = node->children[1];
		else
		{	// go down both
			SV_AddToFatPVS (org, node->children[0], worldmodel); //johnfitz -- worldmodel as a parameter
			node = node->children[1];
		}
	}
}

/*	Calculates a PVS that is the inclusive or of all leafs within 8 pixels of the
	given point.
*/
byte *SV_FatPVS (vec3_t org, model_t *worldmodel) //johnfitz -- added worldmodel as a parameter
{
	fatbytes = (worldmodel->numleafs+31)>>3;
	Q_memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVS (org, worldmodel->nodes, worldmodel); //johnfitz -- worldmodel as a parameter
	return fatpvs;
}

/*	PVS test encapsulated in a nice function
*/
bool SV_VisibleToClient(edict_t *client,edict_t *test,model_t *worldmodel)
{
	byte	*pvs;
	vec3_t	org;
	int		i;

	Math_VectorAdd (client->v.origin, client->v.view_ofs, org);
	pvs = SV_FatPVS (org, worldmodel);

	for (i=0 ; i < test->num_leafs ; i++)
		if (pvs[test->leafnums[i] >> 3] & (1 << (test->leafnums[i]&7) ))
			return true;

	return false;
}

void SV_WriteEntitiesToClient (edict_t	*clent, sizebuf_t *msg)
{
	int		e,i,bits;
	byte	*pvs;
	vec3_t	org;
	float	miss;
	edict_t	*ent;

// find the client's PVS
	Math_VectorAdd(clent->v.origin,clent->v.view_ofs,org);
	pvs = SV_FatPVS (org, sv.worldmodel);

// send over all entities (excpet the client) that touch the pvs
	ent = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
	{
		if (ent != clent)	// clent is ALLWAYS sent
		{
			// ignore ents without visible models
			if (!ent->v.modelindex || !ent->v.model)
				continue;

			// ignore if not touching a PV leaf
			for (i=0 ; i < ent->num_leafs ; i++)
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i]&7) ))
					break;
			if (i == ent->num_leafs)
				continue;		// not visible
		}

		if (msg->cursize + 24 > msg->maxsize)
		{
			//johnfitz -- less spammy overflow message
			if (!dev_overflows.packetsize || dev_overflows.packetsize + CONSOLE_RESPAM_TIME < realtime )
			{
				Con_Printf ("Packet overflow!\n");
				dev_overflows.packetsize = realtime;
			}
			goto stats;
			//johnfitz
		}

// send an update
		bits = 0;

		for (i=0 ; i<3 ; i++)
		{
			miss = ent->v.origin[i] - ent->baseline.origin[i];
			if ( miss < -0.1 || miss > 0.1 )
				bits |= U_ORIGIN1<<i;
		}

		if(ent->baseline.fScale != ent->Model.fScale)
			bits |= U_SCALE;
		if ( ent->v.angles[0] != ent->baseline.angles[0] )
			bits |= U_ANGLE1;
		if ( ent->v.angles[1] != ent->baseline.angles[1] )
			bits |= U_ANGLE2;
		if ( ent->v.angles[2] != ent->baseline.angles[2] )
			bits |= U_ANGLE3;
		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_STEP;	// don't mess up the step animation
		if (ent->baseline.colormap != ent->v.colormap)
			bits |= U_COLORMAP;

		if (ent->baseline.skin != ent->Model.iSkin)
			bits |= U_SKIN;

		if (ent->baseline.frame != ent->v.frame)
			bits |= U_FRAME;

		if (ent->baseline.effects != ent->v.effects)
			bits |= U_EFFECTS;

		if (ent->baseline.modelindex != ent->v.modelindex)
			bits |= U_MODEL;

		//don't send invisible entities unless they have effects
		if (ent->alpha == ENTALPHA_ZERO && !ent->v.effects)
			continue;

		//johnfitz
		if (ent->baseline.alpha != ent->alpha)
			bits |= U_ALPHA;
		if (bits & U_FRAME && (int)ent->v.frame & 0xFF00)
			bits |= U_FRAME2;
		if (bits & U_MODEL && (int)ent->v.modelindex & 0xFF00)
			bits |= U_MODEL2;
		if (ent->bSendInterval)
			bits |= U_LERPFINISH;
		if (bits >= 65536)
			bits |= U_EXTEND1;
		if (bits >= 16777216)
			bits |= U_EXTEND2;
		//johnfitz

		if (e >= 256)
			bits |= U_LONGENTITY;

		if (bits >= 256)
			bits |= U_MOREBITS;

	//
	// write the message
	//
		MSG_WriteByte (msg, bits | U_SIGNAL);

		if (bits & U_MOREBITS)
			MSG_WriteByte (msg, bits>>8);

		//johnfitz -- PROTOCOL_FITZQUAKE
		if (bits & U_EXTEND1)
			MSG_WriteByte(msg, bits>>16);
		if (bits & U_EXTEND2)
			MSG_WriteByte(msg, bits>>24);
		//johnfitz

		if (bits & U_LONGENTITY)
			MSG_WriteShort (msg,e);
		else
			MSG_WriteByte (msg,e);

		if (bits & U_MODEL)
			MSG_WriteByte (msg,	ent->v.modelindex);

		if (bits & U_FRAME)
			MSG_WriteByte (msg, ent->v.frame);

		if(bits & U_SCALE)
			MSG_WriteFloat(msg,ent->Model.fScale);

		if (bits & U_COLORMAP)
			MSG_WriteByte (msg, ent->v.colormap);

		if (bits & U_SKIN)
			MSG_WriteByte(msg, ent->Model.iSkin);

		if (bits & U_EFFECTS)
			// [21/12/2013] Fixed this not being sent over in full form ~hogsy
			MSG_WriteLong(msg,ent->v.effects);

		if (bits & U_ORIGIN1)
			MSG_WriteCoord (msg, ent->v.origin[0]);
		if (bits & U_ANGLE1)
			MSG_WriteAngle(msg, ent->v.angles[0]);
		if (bits & U_ORIGIN2)
			MSG_WriteCoord (msg, ent->v.origin[1]);
		if (bits & U_ANGLE2)
			MSG_WriteAngle(msg, ent->v.angles[1]);
		if (bits & U_ORIGIN3)
			MSG_WriteCoord (msg, ent->v.origin[2]);
		if (bits & U_ANGLE3)
			MSG_WriteAngle(msg, ent->v.angles[2]);
		if (bits & U_ALPHA)
			MSG_WriteByte(msg, ent->alpha);
		if (bits & U_FRAME2)
			MSG_WriteByte(msg, (int)ent->v.frame >> 8);
		if (bits & U_MODEL2)
			MSG_WriteByte(msg, (int)ent->v.modelindex >> 8);
		if (bits & U_LERPFINISH)
			MSG_WriteByte(msg,(byte)(pMath_RINT((ent->v.dNextThink-sv.time)*255)));
	}

	//johnfitz -- devstats
stats:
	dev_stats.packetsize = msg->cursize;
	dev_peakstats.packetsize = Math_Max(msg->cursize,dev_peakstats.packetsize);
	//johnfitz
}

void SV_CleanupEnts (void)
{
	int		e;
	edict_t	*ent;

	ent = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
		ent->v.effects &= ~EF_MUZZLEFLASH;
}

void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg)
{
	int		bits;
	int		i;
	edict_t	*other;
	int		items;
	eval_t	*val;

	// Send a damage message
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = ent->v.eDamageInflictor;

		MSG_WriteByte(msg,SVC_DAMAGE);
		MSG_WriteByte(msg,ent->v.dmg_save);
		MSG_WriteByte(msg,ent->v.dmg_take);

		for (i=0 ; i<3 ; i++)
			MSG_WriteCoord (msg, other->v.origin[i] + 0.5*(other->v.mins[i] + other->v.maxs[i]));

		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

	// Send the current viewpos offset from the view entity
	SV_SetIdealPitch ();		// how much to look up / down ideally

	// A fixangle might get lost in a dropped packet.  Oh well.
	if ( ent->v.fixangle )
	{
		MSG_WriteByte (msg, svc_setangle);
		for(i = 0; i < 3; i++)
			MSG_WriteAngle (msg, ent->v.angles[i] );
		ent->v.fixangle = 0;
	}

	bits = 0;

	if (ent->v.view_ofs[2] != DEFAULT_VIEWHEIGHT)
		bits |= SU_VIEWHEIGHT;
	if (ent->v.idealpitch)
		bits |= SU_IDEALPITCH;

	// stuff the sigil bits into the high bits of items for sbar, or else
	// mix in items2
	val = GetEdictFieldValue(ent, "items2");
	if (val)
		items = ent->v.items | ((int)val->_float << 23);
	else
		items = ent->v.items | ((int)pr_global_struct.serverflags << 28);

	bits |= SU_ITEMS;

	if(ent->v.flags & FL_ONGROUND)
		bits |= SU_ONGROUND;
	if(ent->v.waterlevel >= 2)
		bits |= SU_INWATER;

	for (i=0 ; i<3 ; i++)
	{
		if (ent->v.punchangle[i])
			bits |= (SU_PUNCH1<<i);
		if (ent->v.velocity[i])
			bits |= (SU_VELOCITY1<<i);
	}

	if(ent->v.iWeaponFrame)
		bits |= SU_WEAPONFRAME;

	if(ent->v.iArmorValue)
		bits |= SU_ARMOR;

	bits |= SU_WEAPON;

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (bits & SU_WEAPON && SV_ModelIndex(ent->v.cViewModel) & 0xFF00)
		bits |= SU_WEAPON2;
	if ((int)ent->v.iArmorValue & 0xFF00)
		bits |= SU_ARMOR2;
	if(ent->v.iPrimaryAmmo & 0xFF00)
		bits |= SU_AMMO2;
	if (bits & SU_WEAPONFRAME && ent->v.iWeaponFrame & 0xFF00)
		bits |= SU_WEAPONFRAME2;
	if (bits & SU_WEAPON && ent->alpha != ENTALPHA_DEFAULT)
		bits |= SU_WEAPONALPHA; //for now, weaponalpha = client entity alpha
	if (bits >= 65536)
		bits |= SU_EXTEND1;
	if (bits >= 16777216)
		bits |= SU_EXTEND2;
	//johnfitz

	// Send the data
	MSG_WriteByte(msg,svc_clientdata);
	MSG_WriteShort(msg,bits);

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (bits & SU_EXTEND1)
		MSG_WriteByte(msg,bits>>16);
	if (bits & SU_EXTEND2)
		MSG_WriteByte(msg,bits>>24);
	//johnfitz

	if (bits & SU_VIEWHEIGHT)
		MSG_WriteChar (msg, ent->v.view_ofs[2]);

	if (bits & SU_IDEALPITCH)
		MSG_WriteChar (msg, ent->v.idealpitch);

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i))
			MSG_WriteChar (msg, ent->v.punchangle[i]);
		if (bits & (SU_VELOCITY1<<i))
			MSG_WriteChar (msg, ent->v.velocity[i]/16);
	}

	MSG_WriteLong(msg,items);

	if (bits & SU_WEAPONFRAME)
		MSG_WriteByte(msg,ent->v.iWeaponFrame);
	if (bits & SU_ARMOR)
		MSG_WriteByte(msg,ent->v.iArmorValue);
	if (bits & SU_WEAPON)
		MSG_WriteByte (msg,SV_ModelIndex(ent->v.cViewModel));

	MSG_WriteShort(msg,ent->v.iHealth);
	MSG_WriteByte(msg,ent->v.iPrimaryAmmo);
	MSG_WriteByte(msg,ent->v.iActiveWeapon);

	//johnfitz -- PROTOCOL_FITZQUAKE
	if (bits & SU_WEAPON2)
		MSG_WriteByte (msg, SV_ModelIndex(ent->v.cViewModel) >> 8);
	if (bits & SU_ARMOR2)
		MSG_WriteByte (msg,ent->v.iArmorValue >> 8);
	if (bits & SU_AMMO2)
		MSG_WriteByte (msg,ent->v.iPrimaryAmmo >> 8);
	if (bits & SU_WEAPONFRAME2)
		MSG_WriteByte (msg,ent->v.iWeaponFrame >> 8);
	if (bits & SU_WEAPONALPHA)
		MSG_WriteByte (msg, ent->alpha); //for now, weaponalpha = client entity alpha
	//johnfitz
}

bool SV_SendClientDatagram (client_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	//johnfitz -- if client is nonlocal, use smaller max size so packets aren't fragmented
	if (Q_strcmp (client->netconnection->address, "LOCAL") != 0)
		msg.maxsize = DATAGRAM_MTU;
	//johnfitz

	MSG_WriteByte(&msg, svc_time);
	MSG_WriteFloat(&msg, sv.time);

// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client->edict, &msg);

	SV_WriteEntitiesToClient (client->edict, &msg);

// copy the server datagram if there is space
	if (msg.cursize + sv.datagram.cursize < msg.maxsize)
		SZ_Write (&msg, sv.datagram.data, sv.datagram.cursize);

// send the datagram
	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
	{
		SV_DropClient(true);// if the message couldn't send, kick off

		return false;
	}

	return true;
}

void SV_UpdateToReliableMessages (void)
{
	int			i, j;
	client_t *client;

// check for changes to be sent over the reliable streams
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (host_client->old_frags != host_client->edict->v.iScore)
		{
			for(j = 0,client = svs.clients; j < svs.maxclients; j++,client++)
			{
				if(!client->active)
					continue;

				MSG_WriteByte (&client->message, svc_updatefrags);
				MSG_WriteByte (&client->message, i);
				MSG_WriteShort (&client->message, host_client->edict->v.iScore);
			}

			host_client->old_frags = host_client->edict->v.iScore;
		}
	}

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		SZ_Write (&client->message, sv.reliable_datagram.data, sv.reliable_datagram.cursize);
	}

	SZ_Clear (&sv.reliable_datagram);
}

/*	Send a nop message without trashing or sending the accumulated client
	message buffer
*/
void SV_SendNop (client_t *client)
{
	sizebuf_t	msg;
	byte		buf[4];

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;

	MSG_WriteChar(&msg,SVC_NOP);

	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
		SV_DropClient(true);	// If the message couldn't send, kick off

	client->last_message = realtime;
}

void SV_SendClientMessages (void)
{
	int	i;

// update frags, names, etc
	SV_UpdateToReliableMessages ();

// build individual updates
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if(!host_client->active)
			continue;

		if(host_client->bSpawned)
		{
			if (!SV_SendClientDatagram (host_client))
				continue;
		}
		else
		{
		// the player isn't totally in the game yet
		// send small keepalive messages if too much time has passed
		// send a full message when the next signon stage has been requested
		// some other message data (name changes, etc) may accumulate
		// between signon stages
			if (!host_client->sendsignon)
			{
				if (realtime - host_client->last_message > 5)
					SV_SendNop (host_client);
				continue;	// don't send out non-signon messages
			}
		}

		// check for an overflowed message.  Should only happen
		// on a very fucked up connection that backs up a lot, then
		// changes level
		if (host_client->message.overflowed)
		{
			SV_DropClient (true);
			host_client->message.overflowed = false;
			continue;
		}

		if(host_client->message.cursize || host_client->dropasap)
		{
			if (!NET_CanSendMessage (host_client->netconnection))
				continue;

			if(host_client->dropasap)
				SV_DropClient(false);	// went to another level
			else
			{
				if (NET_SendMessage (host_client->netconnection
				, &host_client->message) == -1)
					SV_DropClient(true);	// if the message couldn't send, kick off
				SZ_Clear (&host_client->message);

				host_client->last_message	= realtime;
				host_client->sendsignon		= false;
			}
		}
	}

	// clear muzzle flashes
	SV_CleanupEnts();
}

int SV_ModelIndex (char *name)
{
	int		i;

	if(!name || !name[0])
		return 0;

	for(i = 0; i < MAX_MODELS && sv.model_precache[i]; i++)
		if(!strcmp(sv.model_precache[i],name))
			return i;

#if 0
	if (i==MAX_MODELS || !sv.model_precache[i])
		Sys_Error ("SV_ModelIndex: model %s not precached", name);
#else
	// [4/8/2012] Check that we're not over our limit first... ~hogsy
	if(i == MAX_MODELS)
		Host_Error("Failed to precache %i, model limit reached!",name);
	else if(!sv.model_precache[i])
	{
		// [4/8/2012] Handle missing content a little more gracefully ~hogsy
		Con_Warning("%s not precached!\n",name);

		// [4/8/2012] So we don't go through this again ~hogsy
		sv.model_precache[i] = name;
		return i;
	}
#endif

	return i;
}

void SV_CreateBaseline (void)
{
	int		i;
	edict_t	*svent;
	int		entnum,bits; //johnfitz -- PROTOCOL_FITZQUAKE

	for (entnum = 0; entnum < sv.num_edicts ; entnum++)
	{
	// get the current server version
		svent = EDICT_NUM(entnum);
		if (svent->free)
			continue;
		if (entnum > svs.maxclients && !svent->v.modelindex)
			continue;

		// Create entity baseline
		Math_VectorCopy(svent->v.origin,svent->baseline.origin);
		Math_VectorCopy(svent->v.angles,svent->baseline.angles);

		svent->baseline.frame		= svent->v.frame;
		svent->baseline.skin		= svent->Model.iSkin;
		svent->baseline.colormap	= 0;
		svent->baseline.modelindex	= SV_ModelIndex(svent->v.model);
		svent->baseline.fScale		= 1.0f;

		if(entnum > 0 && entnum <= svs.maxclients)
			svent->baseline.alpha = ENTALPHA_DEFAULT;
		else
			svent->baseline.alpha = svent->alpha; //johnfitz -- alpha support

		//johnfitz -- PROTOCOL_FITZQUAKE
		bits = 0;
		if (svent->baseline.modelindex & 0xFF00)
			bits |= B_LARGEMODEL;
		if (svent->baseline.frame & 0xFF00)
			bits |= B_LARGEFRAME;
		if (svent->baseline.alpha != ENTALPHA_DEFAULT)
			bits |= B_ALPHA;
		//johnfitz

	//
	// add to the message
	//
		//johnfitz -- PROTOCOL_FITZQUAKE
		if (bits)
			MSG_WriteByte (&sv.signon, svc_spawnbaseline2);
		else
			MSG_WriteByte (&sv.signon, svc_spawnbaseline);
		//johnfitz

		MSG_WriteShort (&sv.signon,entnum);

		//johnfitz -- PROTOCOL_FITZQUAKE
		if (bits)
			MSG_WriteByte (&sv.signon, bits);

		if (bits & B_LARGEMODEL)
			MSG_WriteShort (&sv.signon, svent->baseline.modelindex);
		else
			MSG_WriteByte (&sv.signon, svent->baseline.modelindex);

		if (bits & B_LARGEFRAME)
			MSG_WriteShort (&sv.signon, svent->baseline.frame);
		else
			MSG_WriteByte (&sv.signon, svent->baseline.frame);
		//johnfitz

		MSG_WriteFloat(&sv.signon,svent->baseline.fScale);
		MSG_WriteByte(&sv.signon,svent->baseline.colormap);
		MSG_WriteByte(&sv.signon,svent->baseline.skin);
		for (i=0 ; i<3 ; i++)
		{
			MSG_WriteCoord(&sv.signon, svent->baseline.origin[i]);
			MSG_WriteAngle(&sv.signon, svent->baseline.angles[i]);
		}

		//johnfitz -- PROTOCOL_FITZQUAKE
		if (bits & B_ALPHA)
			MSG_WriteByte (&sv.signon, svent->baseline.alpha);
		//johnfitz
	}
}

/*	Tell all the clients that the server is changing levels
*/
void SV_SendReconnect (void)
{
	char		data[128];
	sizebuf_t	msg;

	msg.data	= (byte*)data;
	msg.cursize = 0;
	msg.maxsize = sizeof(data);

	MSG_WriteChar(&msg,SVC_STUFFTEXT);
	MSG_WriteString(&msg,"reconnect\n");

	NET_SendToAll(&msg,5);

	if(cls.state != ca_dedicated)
		Cmd_ExecuteString ("reconnect\n", src_command);
}

/*	Grabs the current state of each client for saving across the
	transition to another level
*/
void SV_SaveSpawnparms (void)
{
	int		i, j;

	svs.serverflags = pr_global_struct.serverflags;

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

	// call the progs to get default spawn parms for the new client
		pr_global_struct.self = EDICT_TO_PROG(host_client->edict);
		Game->Game_Init(SERVER_SETCHANGEPARMS,host_client->edict,sv.time);
		for (j=0 ; j<NUM_SPAWN_PARMS ; j++)
			host_client->spawn_parms[j] = (&pr_global_struct.parm1)[j];
	}
}

extern float scr_centertime_off;

void SV_SpawnServer(char *server)
{
	edict_t	*ent;
	int		i;

	// Let's not have any servers with no name
	if(hostname.string[0] == 0)
		// [19/3/2013] Updated so this isn't specific for OpenKatana anymore ~hogsy
		Cvar_Set("hostname",va("%s %f (%i) Server",Game->Name,ENGINE_VERSION_MAJOR,ENGINE_VERSION_BUILD));

	scr_centertime_off = 0;

	Con_DPrintf("SpawnServer: %s\n",server);
	svs.bChangingLevel = false;		// now safe to issue another

	// tell all connected clients that we are going to a new level
	if(sv.active)
		SV_SendReconnect();

	current_skill = (int)(skill.value+0.5f);
	if (current_skill < 0)
		current_skill = 0;
	else if(current_skill > 3)
		current_skill = 3;

	Cvar_SetValue ("skill", (float)current_skill);

	// Set up the new server
	Host_ClearMemory ();

	memset(&sv,0,sizeof(sv));

	strcpy(sv.name,server);

	sv.protocol	= SERVER_PROTOCOL;

	// Allocate server memory
	sv.max_edicts = Math_Clamp(MIN_EDICTS, (int)max_edicts.value, MAX_EDICTS); //johnfitz -- max_edicts cvar
	sv.edicts		= (edict_t*)Hunk_AllocName(sv.max_edicts*sizeof(edict_t),"edicts");

	sv.datagram.maxsize = sizeof(sv.datagram_buf);
	sv.datagram.cursize = 0;
	sv.datagram.data	= sv.datagram_buf;

	sv.reliable_datagram.maxsize	= sizeof(sv.reliable_datagram_buf);
	sv.reliable_datagram.cursize	= 0;
	sv.reliable_datagram.data		= sv.reliable_datagram_buf;

	sv.signon.maxsize	= sizeof(sv.signon_buf);
	sv.signon.cursize	= 0;
	sv.signon.data		= sv.signon_buf;

	// Leave slots at start for clients only
	sv.num_edicts = svs.maxclients+1;
	for(i = 0; i < svs.maxclients; i++)
	{
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;
	}

	sv.state	= SERVER_STATE_LOADING;
	sv.paused	= false;
	sv.time		= 1.0;

	Q_strcpy(sv.name,server);
	sprintf(sv.modelname,"%s%s"BSP_EXTENSION,Global.cLevelPath,server);
	sv.worldmodel = Mod_ForName(sv.modelname);
	if(!sv.worldmodel)
	{
		Con_Warning("Couldn't spawn server! (%s)\n",sv.modelname);

		sv.active = false;
		return;
	}
	sv.models[1] = sv.worldmodel;

	// Clear world interaction links
	SV_ClearWorld ();

	sv.sound_precache[0]	= "";
	sv.cParticlePrecache[0]	= "";
	sv.model_precache[0]	= "";
	sv.model_precache[1]	= sv.modelname;

	for (i=1 ; i<sv.worldmodel->numsubmodels ; i++)
	{
		sv.model_precache[1+i]	= localmodels[i];
		sv.models[i+1]			= Mod_ForName(localmodels[i]);
	}

	// load the rest of the entities
	ent = EDICT_NUM(0);

	Edict_Clear(ent,false);

	ent->v.model		= sv.worldmodel->name;
	ent->v.modelindex	= 1;					// World model
	ent->Physics.iSolid	= SOLID_BSP;
	ent->v.movetype		= MOVETYPE_PUSH;

	pr_global_struct.mapname		= sv.name;
	pr_global_struct.serverflags	= svs.serverflags;	// Serverflags are for cross level information (sigils)

	ED_LoadFromFile(sv.worldmodel->entities);

	sv.active	= true;
	sv.state	= SERVER_STATE_ACTIVE;	// All setup is completed, any further precache statements are errors

	// Run two frames to allow everything to settle
	host_frametime = 0.1;
	Physics_ServerFrame();
	Physics_ServerFrame();

	// create a baseline for more efficient communications
	SV_CreateBaseline();

	// Send serverinfo to all connected clients
	for (i = 0,host_client = svs.clients; i < svs.maxclients; i++,host_client++)
		if (host_client->active)
			SV_SendServerinfo(host_client);

	Con_DPrintf("Server spawned.\n");
}
