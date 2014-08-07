#include "quakedef.h"

#include "engine_game.h"
#include "engine_input.h"
#include "engine_menu.h"

// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// these two are not intended to be set directly
cvar_t	cl_name		= {"_cl_name",		"player",	true	};
cvar_t	cl_color	= {"_cl_color",		"0",		true	};
cvar_t	cl_shownet	= {"cl_shownet",	"0"					};	// can be 0, 1, or 2
cvar_t	cl_nolerp	= {"cl_nolerp",		"0"					};
cvar_t	cl_maxpitch = {"cl_maxpitch",	"90",		true	}; //johnfitz -- variable pitch clamping
cvar_t	cl_minpitch = {"cl_minpitch",	"-90",		true	}; //johnfitz -- variable pitch clamping

cvar_t	lookspring	= {"lookspring",	"0", true	};
cvar_t	lookstrafe	= {"lookstrafe",	"0", true	};
cvar_t	sensitivity = {"sensitivity",	"3", true	};

cvar_t	m_pitch		= {"m_pitch",	"0.022",	true	};
cvar_t	m_yaw		= {"m_yaw",		"0.022",	true	};
cvar_t	m_forward	= {"m_forward",	"1",		true	};
cvar_t	m_side		= {"m_side",	"0.8",		true	};

client_static_t	cls;
client_state_t	cl;
// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
DynamicLight_t	cl_dlights[MAX_DLIGHTS];

entity_t		*cl_entities; //johnfitz -- was a static array, now on hunk
int				cl_max_edicts; //johnfitz -- only changes when new map loads

int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];

extern cvar_t	r_lerpmodels, r_lerpmove; //johnfitz

void CL_ClearState (void)
{
	int i;

	if(!sv.active)
		Host_ClearMemory();

// wipe the entire cl structure
	memset (&cl, 0, sizeof(cl));

	SZ_Clear (&cls.message);

	// Clear other arrays
	memset(cl_efrags,0,sizeof(cl_efrags));
	memset(cl_dlights,0,sizeof(cl_dlights));
	memset(cl_lightstyle,0,sizeof(cl_lightstyle));
	memset(cl_temp_entities,0,sizeof(cl_temp_entities));
	memset(cl_beams,0,sizeof(cl_beams));

	//johnfitz -- cl_entities is now dynamically allocated
	cl_max_edicts	= CLAMP(MIN_EDICTS,(int)max_edicts.value,MAX_EDICTS);
	cl_entities		= (entity_t*)Hunk_AllocName(cl_max_edicts*sizeof(entity_t),"cl_entities");
	//johnfitz

	// allocate the efrags and chain together into a free list
	cl.free_efrags = cl_efrags;
	for(i = 0; i < MAX_EFRAGS-1; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];

	cl.free_efrags[i].entnext = NULL;
}

/*	Sends a disconnect message to the server
	This is also called on Host_Error, so it shouldn't cause any errors
*/
void CL_Disconnect (void)
{
	// Stop sounds (especially looping!)
	S_StopAllSounds();

	// If running a local server, shut it down
	if (cls.demoplayback)
		CL_StopPlayback ();
	else if(cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer(false);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
}

void CL_Disconnect_f (void)
{
	CL_Disconnect();

	if(sv.active)
		Host_ShutdownServer(false);

	Menu->RemoveState(MENU_STATE_HUD);
}

/*	Host should be either "local" or a net address to be passed on
*/
void CL_EstablishConnection (char *host)
{
	if(cls.state == ca_dedicated || cls.demoplayback)
		return;

	CL_Disconnect ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon)
		Host_Error ("CL_Connect: connect failed\n");

	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host);

	cls.demonum = -1;			// not in the demo loop now
	cls.state	= ca_connected;
	cls.signon	= 0;			// need all the signon messages before playing

	// [11/7/2012] Baker's NAT fix ~hogsy
	MSG_WriteByte(&cls.message,CLC_NOP);
}

/*	An svc_signonnum has been received, perform a client side setup
*/
void CL_SignonReply (void)
{
	char 	str[8192];

	Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "prespawn");
		break;
	case 2:
		MSG_WriteByte(&cls.message,clc_stringcmd);
		MSG_WriteString(&cls.message,va("name \"%s\"\n",cl_name.string));

		MSG_WriteByte(&cls.message,clc_stringcmd);
		MSG_WriteString(&cls.message,va("color %i %i\n",((int)cl_color.value)>>4,((int)cl_color.value)&15));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		sprintf (str, "spawn %s", cls.spawnparms);
		MSG_WriteString (&cls.message, str);
		break;
	case 3:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory
		break;
	case 4:
		SCR_EndLoadingPlaque ();		// allow normal screen updates
		break;
	}
}

/*	Called to play the next demo in the demo loop
*/
void CL_NextDemo (void)
{
	char	str[1024];

	if (cls.demonum == -1)
		return;		// don't play demos

	SCR_BeginLoadingPlaque ();

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
			Con_Printf ("No demos listed with startdemos\n");
			cls.demonum = -1;
			return;
		}
	}

	sprintf (str,"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}

void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int			i;

	for(i = 0,ent = cl_entities; i < cl.num_entities; i++,ent++)
	{
		Con_Printf ("%3i:",i);

		if(!ent->model)
		{
			Con_Printf("EMPTY\n");
			continue;
		}

		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n",
			ent->model->name,
			ent->frame,
			ent->origin[0],ent->origin[1],
			ent->origin[2],ent->angles[0],
			ent->angles[1],ent->angles[2]);
	}
}

DynamicLight_t *Client_AllocDlight(int key)
{
	int				i;
	DynamicLight_t	*dlLight;

	// First look for an exact key match
	if(key)
	{
		dlLight = cl_dlights;
		for(i = 0; i < MAX_DLIGHTS; i++,dlLight++)
		{
			if(dlLight->key == key)
			{
				memset(dlLight,0,sizeof(*dlLight));

				dlLight->key		= key;
				dlLight->bLightmap	= true;

				Math_VectorSet(1.0f,dlLight->color);
				return dlLight;
			}
		}
	}

	// Then look for anything else
	dlLight = cl_dlights;
	for(i = 0; i < MAX_DLIGHTS; i++,dlLight++)
	{
		if(!dlLight->radius || (dlLight->die < cl.time && dlLight->die))
		{
			memset(dlLight,0,sizeof(*dlLight));

			dlLight->key		= key;
			dlLight->bLightmap	= true;

			Math_VectorSet(1.0f,dlLight->color);
			return dlLight;
		}
	}

	dlLight = &cl_dlights[0];

	memset(dlLight,0,sizeof(*dlLight));

	dlLight->key		= key;
	dlLight->bLightmap	= true;

	Math_VectorSet(1.0f,dlLight->color);

	return dlLight;
}

void CL_DecayLights (void)
{
	int				i;
	DynamicLight_t	*dl;
	float			time;

	time = cl.time - cl.oldtime;

	dl = cl_dlights;
	for(i = 0; i < MAX_DLIGHTS; i++,dl++)
	{
		if(((dl->die < cl.time) && dl->die) || !dl->radius)
			continue;

		dl->radius -= time*dl->decay;
		if(dl->radius < 0)
			dl->radius = 0;
	}
}

/*	Determines the fraction between the last two messages that the objects
	should be put at.
*/
float CL_LerpPoint(void)
{
	float	f, frac;

	f = cl.mtime[0] - cl.mtime[1];
	if(!f || cls.timedemo || sv.active)
	{
		cl.time = cl.mtime[0];
		return 1;
	}

	if (f > 0.1) // dropped packet, or start of demo
	{
		cl.mtime[1] = cl.mtime[0] - 0.1;
		f = 0.1f;
	}

	frac = (cl.time-cl.mtime[1])/f;
	if(frac < 0)
	{
		if (frac < -0.01)
			cl.time = cl.mtime[1];
		frac = 0;
	}
	else if (frac > 1.0f)
	{
		if (frac > 1.01f)
			cl.time = cl.mtime[0];
		frac = 1;
	}

	//johnfitz -- better nolerp behavior
	if(cl_nolerp.value)
		return 1;
	//johnfitz

	return frac;
}

void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int			i, j;
	float		frac, f, d;
	vec3_t		delta;

	if(cl.bIsPaused)
		return;

	// determine partial update time
	frac = CL_LerpPoint();

	cl_numvisedicts = 0;

	// interpolate player info
	for (i=0 ; i<3 ; i++)
		cl.velocity[i] = cl.mvelocity[1][i] +
			frac*(cl.mvelocity[0][i] - cl.mvelocity[1][i]);

	if(cls.demoplayback)
	{
		// interpolate the angles
		for (j=0 ; j<3 ; j++)
		{
			d = cl.mviewangles[0][j] - cl.mviewangles[1][j];
			if (d > 180.0f)
				d -= 360.0f;
			else if (d < -180.0f)
				d += 360.0f;
			cl.viewangles[j] = cl.mviewangles[1][j] + frac*d;
		}
	}

// start on the entity after the world
	for (i=1,ent=cl_entities+1 ; i<cl.num_entities ; i++,ent++)
	{
#if 1
		if(!ent->model)
		{
			// Empty slot
			if (ent->bForceLink)
				R_RemoveEfrags (ent);	// just became empty
			continue;
		}
#endif

		// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime[0])
		{
			ent->model		= NULL;
			ent->lerpflags	|= LERP_RESETMOVE|LERP_RESETANIM; //johnfitz -- next time this entity slot is reused, the lerp will need to be reset
			continue;
		}

		if (ent->bForceLink)
		{
			// the entity was not updated in the last message
			// so move to the final spot
			Math_VectorCopy (ent->msg_origins[0], ent->origin);
			Math_VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			for (j=0 ; j<3 ; j++)
			{
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];
				if (delta[j] > 100 || delta[j] < -100)
				{
					f = 1.0f;		// assume a teleportation, not a motion
					ent->lerpflags |= LERP_RESETMOVE; //johnfitz -- don't lerp teleports
				}
			}

			//johnfitz -- don't cl_lerp entities that will be r_lerped
			if (r_lerpmove.value && (ent->lerpflags & LERP_MOVESTEP))
				f = 1.0f;
			//johnfitz

		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
			{
				ent->origin[j] = ent->msg_origins[1][j] + f*delta[j];

				d = ent->msg_angles[0][j] - ent->msg_angles[1][j];
				if (d > 180.0f)
					d -= 360.0f;
				else if (d < -180.0f)
					d += 360.0f;

				ent->angles[j] = ent->msg_angles[1][j] + f*d;
			}
		}

		Game->Client_RelinkEntities(ent,i,cl.time);

		if(i == cl.viewentity && !chase_active.value)
			continue;

		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}

/*	Read all incoming data from the server
*/
int CL_ReadFromServer (void)
{
	int				ret;
	extern int		num_temp_entities;	//johnfitz
	int				num_beams	= 0;	//johnfitz
	int				num_dlights = 0;	//johnfitz
	beam_t			*b; //johnfitz
	DynamicLight_t	*l; //johnfitz
	int				i; //johnfitz

	cl.oldtime = cl.time;
	cl.time += host_frametime;

	do
	{
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;

		cl.last_received_message = realtime;
		CL_ParseServerMessage();
	} while (ret && cls.state == ca_connected);

	if (cl_shownet.value)
		Con_Printf ("\n");

	CL_RelinkEntities ();
	CL_UpdateTEnts ();

//johnfitz -- devstats

	//visedicts
	dev_stats.visedicts = cl_numvisedicts;
	dev_peakstats.visedicts = Math_Max(cl_numvisedicts, dev_peakstats.visedicts);

	//temp entities
	dev_stats.tempents = num_temp_entities;
	dev_peakstats.tempents = Math_Max(num_temp_entities, dev_peakstats.tempents);

	//beams
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->model && b->endtime >= cl.time)
			num_beams++;
	dev_stats.beams		= num_beams;
	dev_peakstats.beams = Math_Max(num_beams, dev_peakstats.beams);

	//dlights
	for(i = 0,l = cl_dlights; i < MAX_DLIGHTS; i++,l++)
		if((l->die >= cl.time || !l->die) && l->radius)
			num_dlights++;

	dev_stats.dlights		= num_dlights;
	dev_peakstats.dlights	= Math_Max(num_dlights, dev_peakstats.dlights);

//johnfitz

	// bring the links up to date
	return 0;
}

void CL_SendCmd(void)
{
	usercmd_t	cmd;

	if(cls.state != ca_connected)
		return;

	if (cls.signon == SIGNONS)
	{
		// Get basic movement from keyboard
		CL_BaseMove (&cmd);

		// [10/7/2013] Process client input ~hogsy
		Input_ProcessClient(&cmd);

		// Send the unreliable message
		CL_SendMove (&cmd);
	}

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

	// Send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all

	if (!NET_CanSendMessage (cls.netcon))
	{
		Con_DPrintf ("CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1)
		Host_Error ("CL_WriteToServer: lost server connection");

	SZ_Clear (&cls.message);
}

/*	Display impact point of trace along VPN
*/
void CL_Tracepos_f (void)
{
	vec3_t	v, w;

	Math_VectorScale(vpn, 8192.0, v);
	TraceLine(r_refdef.vieworg, v, w);

	if(Math_Length(w) == 0)
		Con_Printf ("Tracepos: trace didn't hit anything\n");
	else
		Con_Printf ("Tracepos: (%i %i %i)\n", (int)w[0], (int)w[1], (int)w[2]);
}

void CL_Viewpos_f (void)
{
#if 1
	//camera position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)r_refdef.vieworg[0],
		(int)r_refdef.vieworg[1],
		(int)r_refdef.vieworg[2],
		(int)r_refdef.viewangles[PITCH],
		(int)r_refdef.viewangles[YAW],
		(int)r_refdef.viewangles[ROLL]);
#else
	//player position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)cl_entities[cl.viewentity].origin[0],
		(int)cl_entities[cl.viewentity].origin[1],
		(int)cl_entities[cl.viewentity].origin[2],
		(int)cl.viewangles[PITCH],
		(int)cl.viewangles[YAW],
		(int)cl.viewangles[ROLL]);
#endif
}

void CL_Init (void)
{
	SZ_Alloc(&cls.message,1024);

	CL_InitInput();
	CL_InitTEnts();

	Cvar_RegisterVariable (&cl_name, NULL);
	Cvar_RegisterVariable (&cl_color, NULL);
	Cvar_RegisterVariable (&cl_upspeed, NULL);
	Cvar_RegisterVariable (&cl_forwardspeed, NULL);
	Cvar_RegisterVariable (&cl_backspeed, NULL);
	Cvar_RegisterVariable (&cl_sidespeed, NULL);
	Cvar_RegisterVariable (&cl_movespeedkey, NULL);
	Cvar_RegisterVariable (&cl_yawspeed, NULL);
	Cvar_RegisterVariable (&cl_pitchspeed, NULL);
	Cvar_RegisterVariable (&cl_anglespeedkey, NULL);
	Cvar_RegisterVariable (&cl_shownet, NULL);
	Cvar_RegisterVariable (&cl_nolerp, NULL);
	Cvar_RegisterVariable (&lookspring, NULL);
	Cvar_RegisterVariable (&lookstrafe, NULL);
	Cvar_RegisterVariable (&sensitivity, NULL);
	Cvar_RegisterVariable (&m_pitch, NULL);
	Cvar_RegisterVariable (&m_yaw, NULL);
	Cvar_RegisterVariable (&m_forward, NULL);
	Cvar_RegisterVariable (&m_side, NULL);
	Cvar_RegisterVariable (&cl_maxpitch, NULL); //johnfitz -- variable pitch clamping
	Cvar_RegisterVariable (&cl_minpitch, NULL); //johnfitz -- variable pitch clamping

	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
	Cmd_AddCommand ("tracepos", CL_Tracepos_f); //johnfitz
	Cmd_AddCommand ("viewpos", CL_Viewpos_f); //johnfitz
}

