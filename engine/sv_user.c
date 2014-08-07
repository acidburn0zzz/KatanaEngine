/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others

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
// sv_user.c -- server code for moving users

#include "quakedef.h"

edict_t	*sv_player;

cvar_t	sv_edgefriction = {"edgefriction", "2"};

extern	cvar_t	sv_stopspeed;

static	vec3_t		forward, right, up;

vec3_t	wishdir;
float	wishspeed;

// world
float	*angles,
		*origin,
		*velocity;

bool	bIsOnGround;

usercmd_t	cmd;

cvar_t	sv_idealpitchscale	= {	"sv_idealpitchscale",	"0.8"			};
cvar_t	sv_altnoclip		= {	"sv_altnoclip",			"1",	true	}; //johnfitz

#define	MAX_FORWARD	6
void SV_SetIdealPitch (void)
{
	float	angleval, sinval, cosval;
	trace_t	tr;
	vec3_t	top, bottom;
	float	z[MAX_FORWARD];
	int		i, j;
	int		step, dir, steps;

	if(!(sv_player->v.flags & FL_ONGROUND))
		return;

	angleval = sv_player->v.angles[YAW] * pMath_PI*2 / 360;
	sinval = sin(angleval);
	cosval = cos(angleval);

	for (i=0 ; i<MAX_FORWARD ; i++)
	{
		top[0] = sv_player->v.origin[0] + cosval*(i+3)*12;
		top[1] = sv_player->v.origin[1] + sinval*(i+3)*12;
		top[2] = sv_player->v.origin[2] + sv_player->v.view_ofs[2];

		bottom[0] = top[0];
		bottom[1] = top[1];
		bottom[2] = top[2] - 160;

		tr = SV_Move (top, vec3_origin, vec3_origin, bottom, 1, sv_player);
		if (tr.bAllSolid)
			return;	// looking at a wall, leave ideal the way is was

		if (tr.fraction == 1)
			return;	// near a dropoff

		z[i] = top[2] + tr.fraction*(bottom[2]-top[2]);
	}

	dir = 0;
	steps = 0;
	for (j=1 ; j<i ; j++)
	{
		step = z[j] - z[j-1];
		if (step > -ON_EPSILON && step < ON_EPSILON)
			continue;

		if (dir && ( step-dir > ON_EPSILON || step-dir < -ON_EPSILON ) )
			return;		// mixed changes

		steps++;
		dir = step;
	}

	if(!dir)
	{
		sv_player->v.idealpitch = 0;
		return;
	}

	if (steps < 2)
		return;
	sv_player->v.idealpitch = -dir * sv_idealpitchscale.value;
}

cvar_t	sv_maxspeed		= { "sv_maxspeed",		"320",	false,	true	};
cvar_t	sv_accelerate	= { "sv_accelerate",	"10"					};

void SV_AirAccelerate (vec3_t wishveloc)
{
	int			i;
	float		addspeed, wishspd, accelspeed, currentspeed;

	wishspd = Math_VectorNormalize (wishveloc);
	if (wishspd > 30.0f)
		wishspd = 30.0f;

	currentspeed = Math_DotProduct (velocity, wishveloc);
	addspeed = wishspd - currentspeed;
	if (addspeed <= 0)
		return;
//	accelspeed = sv_accelerate.value * host_frametime;
	accelspeed = sv_accelerate.value*wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i=0 ; i<3 ; i++)
		velocity[i] += accelspeed*wishveloc[i];
}

void SV_WaterMove (void)
{
	int		i;
	vec3_t	wishvel;
	float	speed, newspeed, wishspeed, addspeed, accelspeed;

//
// user intentions
//
	Math_AngleVectors(sv_player->v.v_angle, forward, right, up);

	for (i=0 ; i<3 ; i++)
		wishvel[i] = forward[i]*cmd.forwardmove + right[i]*cmd.sidemove;

	if (!cmd.forwardmove && !cmd.sidemove && !cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += cmd.upmove;

	wishspeed = Math_Length(wishvel);
	if (wishspeed > sv_maxspeed.value)
	{
		Math_VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}
	wishspeed *= 0.7f;

	// Water friction
	speed = Math_Length(velocity);
	if(speed)
	{
		newspeed = speed-host_frametime*speed*sv_player->Physics.fFriction;
		if (newspeed < 0)
			newspeed = 0;
		Math_VectorScale (velocity, newspeed/speed, velocity);
	}
	else
		newspeed = 0;

//
// water acceleration
//
	if (!wishspeed)
		return;

	addspeed = wishspeed - newspeed;
	if (addspeed <= 0)
		return;

	Math_VectorNormalize (wishvel);
	accelspeed = sv_accelerate.value * wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i=0 ; i<3 ; i++)
		velocity[i] += accelspeed * wishvel[i];
}

/*	The move fields specify an intended velocity in pix/sec.
	The angle fields specify an exact angular motion in degrees.
*/
void SV_ClientThink (void)
{
	int			i;
	float		fLength,fmove,smove,addspeed,accelspeed,currentspeed;
	vec3_t		vAngle,vWishVelocity;

	if(sv_player->v.movetype == MOVETYPE_NONE)
		return;

	bIsOnGround	= sv_player->v.flags & FL_ONGROUND;
	origin		= sv_player->v.origin;
	velocity	= sv_player->v.velocity;

	fLength = Math_VectorNormalize (sv_player->v.punchangle);

	fLength -= 10*host_frametime;
	if(fLength < 0)
		fLength = 0;

	Math_VectorScale(sv_player->v.punchangle,fLength,sv_player->v.punchangle);

	// If dead, behave differently
	if(sv_player->v.iHealth <= 0)
		return;

	// Show 1/3 the pitch angle and all the roll angle
	cmd = host_client->cmd;
	angles = sv_player->v.angles;

	Math_VectorAdd (sv_player->v.v_angle, sv_player->v.punchangle, vAngle);
	angles[ROLL] = V_CalcRoll (sv_player->v.angles, sv_player->v.velocity)*4;
	if(!sv_player->v.fixangle)
	{
		angles[PITCH]	= -vAngle[PITCH]/3.0f;
		angles[YAW]		= vAngle[YAW];
	}

	if(sv_player->v.flags & FL_WATERJUMP)
	{
		if(sv.time > sv_player->v.teleport_time || !sv_player->v.waterlevel)
		{
			sv_player->v.flags			= sv_player->v.flags & ~FL_WATERJUMP;
			sv_player->v.teleport_time	= 0;
		}

		sv_player->v.velocity[0] = sv_player->v.movedir[0];
		sv_player->v.velocity[1] = sv_player->v.movedir[1];
		return;
	}

	//johnfitz -- alternate noclip
	if(sv_player->v.movetype == MOVETYPE_NOCLIP && sv_altnoclip.value)
	{
		Math_AngleVectors(sv_player->v.v_angle,forward,right,up);

		velocity[0] = forward[0]*cmd.forwardmove+right[0]*cmd.sidemove;
		velocity[1] = forward[1]*cmd.forwardmove+right[1]*cmd.sidemove;
		velocity[2] = forward[2]*cmd.forwardmove+right[2]*cmd.sidemove;
		velocity[2] += cmd.upmove*2; //doubled to match running speed

		if(Math_Length(velocity) > sv_maxspeed.value)
		{
			Math_VectorNormalize(velocity);
			Math_VectorScale(velocity,sv_maxspeed.value,velocity);
		}
	}
	else if(sv_player->v.waterlevel >= 2 && sv_player->v.movetype != MOVETYPE_NOCLIP)
		SV_WaterMove ();
	else
	{
		Math_AngleVectors(sv_player->v.angles, forward, right, up);

		fmove = cmd.forwardmove;
		smove = cmd.sidemove;

		// Hack to not let you back into teleporter
		if(sv.time < sv_player->v.teleport_time && fmove < 0)
			fmove = 0;

		for(i = 0; i < 3; i++)
			vWishVelocity[i] = forward[i]*fmove+right[i]*smove;

		if(sv_player->v.movetype != MOVETYPE_WALK)
			vWishVelocity[2] = cmd.upmove;
		else
			vWishVelocity[2] = 0;

		Math_VectorCopy(vWishVelocity,wishdir);

		wishspeed = Math_VectorNormalize(wishdir);
		if(wishspeed > sv_maxspeed.value)
		{
			Math_VectorScale(vWishVelocity,sv_maxspeed.value/wishspeed,vWishVelocity);
			wishspeed = sv_maxspeed.value;
		}

		if(sv_player->v.movetype == MOVETYPE_NOCLIP)
			Math_VectorCopy(vWishVelocity, velocity);
		else if(bIsOnGround)
		{
			Physics_AddFriction(sv_player,velocity,origin);

			currentspeed = Math_DotProduct (velocity, wishdir);
			addspeed = wishspeed - currentspeed;
			if(addspeed <= 0)
				return;

			accelspeed = sv_accelerate.value*host_frametime*wishspeed;
			if (accelspeed > addspeed)
				accelspeed = addspeed;

			for(i = 0; i < 3; i++)
				velocity[i] += accelspeed*wishdir[i];
		}
		else
			// Not on ground, so little effect on velocity
			SV_AirAccelerate(vWishVelocity);
	}
	//johnfitz
}

void SV_ReadClientMove (usercmd_t *move)
{
	int		i;
	vec3_t	angle;
	int		bits;

	// Read ping time
	host_client->ping_times[host_client->num_pings%NUM_PING_TIMES]
		= sv.time - MSG_ReadFloat ();
	host_client->num_pings++;

	// Read current angles
	for (i=0 ; i<3 ; i++)
		angle[i] = MSG_ReadAngle16 ();

	Math_VectorCopy(angle,host_client->edict->v.v_angle);

	// Read movement
	move->forwardmove	= MSG_ReadShort ();
	move->sidemove		= MSG_ReadShort ();
	move->upmove		= MSG_ReadShort ();

	// Read buttons
	bits = MSG_ReadByte ();
	host_client->edict->v.button[0] = bits & 1;
	host_client->edict->v.button[2] = (bits & 2)>>1;

	i = MSG_ReadByte ();
	if (i)
		host_client->edict->v.impulse = i;
}

/*	Returns false if the client should be killed
*/
bool Server_ReadClientMessage(void)
{
	int		ret,cmd;
	char	*s;

	do
	{
nextmsg:
		ret = NET_GetMessage (host_client->netconnection);
		if(ret == -1)
		{
			Sys_Printf ("SV_ReadClientMessage: NET_GetMessage failed\n");
			return false;
		}
		else if(!ret)
			return true;

		MSG_BeginReading ();

		for(;;)
		{
			if(!host_client->active)
				return false;	// a command caused an error
			else if(msg_badread)
			{
				Sys_Printf ("SV_ReadClientMessage: badread\n");
				return false;
			}

			cmd = MSG_ReadChar();
			switch(cmd)
			{
			case -1:
				goto nextmsg;		// end of message
			default:
				Sys_Printf ("SV_ReadClientMessage: unknown command char\n");
				return false;
			case clc_nop:
				break;
			case clc_stringcmd:
				s = MSG_ReadString ();
				if (host_client->privileged)
					ret = 2;
				else
					ret = 0;
				if (Q_strncasecmp(s, "status", 6) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "god", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "notarget", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "fly", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "name", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "noclip", 6) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "say", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "say_team", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "tell", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "color", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "kill", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "pause", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "spawn", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "begin", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "prespawn", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "kick", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "ping", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "give", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "ban", 3) == 0)
					ret = 1;

				if (ret == 2)
					Cbuf_InsertText (s);
				else if (ret == 1)
					Cmd_ExecuteString (s, src_client);
				else
					Con_DPrintf("%s tried to %s\n", host_client->name, s);

				break;
			case clc_disconnect:
				return false;
			case clc_move:
				SV_ReadClientMove (&host_client->cmd);
				break;
			}
		}
	} while (ret == 1);

	return true;
}

void SV_RunClients (void)
{
	int	i;

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

		sv_player = host_client->edict;

		if(!Server_ReadClientMessage())
		{
			SV_DropClient(false);	// Client misbehaved...
			continue;
		}

		if(!host_client->bSpawned)
		{
			// Clear client movement until a new packet is received
			memset(&host_client->cmd,0,sizeof(host_client->cmd));
			continue;
		}

		// Always pause in single player if in console or menus
		if(!sv.paused && (svs.maxclients > 1 || key_dest == key_game) )
			SV_ClientThink();
	}
}

