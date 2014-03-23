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

#include "engine_main.h"
#include "engine_console.h"
#include "engine_game.h"
#include "engine_menu.h"
#include "KatEditor.h"
#include "engine_input.h"
#include "engine_video.h"
#include "engine_physics.h"
#include "engine_client.h"
#include "engine_audio.h"

#include "shared_server.h"

#include "platform/include/platform_filesystem.h"

/*	A server can allways be started, even if the system started out as a client
	to a remote system.

	A client can NOT be started if the system started as a dedicated server.

	Memory is cleared / released when a server or client begins, not when they end.
*/

EngineParameters_t host_parms;

bool	bHostInitialized;		// True if into command execution

double		host_frametime,
			dServerTime,
			realtime,			// Without any filtering or bounding
			dOldRealTime;		// Last frame run

int	host_framecount,host_hunklevel;

client_t	*host_client;			// current client

jmp_buf 	host_abortserver;

cvar_t	host_speeds		= {	"host_speeds",		"0"						};	// Set for running times
cvar_t	host_maxfps		= {	"host_maxfps",		"72",	true			};	//johnfitz
cvar_t	host_timescale	= {	"host_timescale",	"0"						};	//johnfitz
cvar_t	max_edicts		= {	"max_edicts",		"2048",	true			};	//johnfitz
cvar_t	sys_ticrate		= {	"sys_ticrate",		"0.05"					};	// dedicated server
cvar_t	serverprofile	= {	"serverprofile",	"0"						};
cvar_t	fraglimit		= {	"fraglimit",		"0",	false,	true	};
cvar_t	timelimit		= {	"timelimit",		"0",	false,	true	};
cvar_t	teamplay		= {	"teamplay",			"0",	false,	true	};
cvar_t	samelevel		= {	"samelevel",		"0"						};
cvar_t	noexit			= {	"noexit",			"0",	false,	true	};
cvar_t	skill			= {	"skill",			"1"						};	// 0 - 3
cvar_t	deathmatch		= {	"deathmatch",		"0"						};	// 0, 1, or 2
cvar_t	coop			= {	"coop",				"0"						};	// 0 or 1
cvar_t	pausable		= {	"pausable",			"1"						};
cvar_t	developer		= {	"developer",		"0"						};
cvar_t	devstats		= {	"devstats",			"0"						};	//johnfitz -- track developer statistics that vary every frame

void Max_Edicts_f(void)
{
	static float oldval = 2048;

	if(max_edicts.value >= MAX_EDICTS)
	{
		Con_Warning("Max_Edicts_f: max_edicts set too high, reverting to default value.\n");
		Cvar_Set("max_edicts","2048");
		return;
	}
	else if(max_edicts.value == oldval)
		return;

	if (cls.state == ca_connected || sv.active)
		Con_Printf ("Changes will not take effect until the next level load.\n");

	oldval = max_edicts.value;
}

void Host_EndGame (char *message, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,message);
	vsprintf (string,message,argptr);
	va_end (argptr);
	Con_DPrintf ("Host_EndGame: %s\n",string);

	if (sv.active)
		Host_ShutdownServer(false);

	if (cls.state == ca_dedicated)
		Sys_Error ("Host_EndGame: %s\n",string);	// dedicated servers exit

	if (cls.demonum != -1)
		CL_NextDemo ();
	else
		CL_Disconnect ();

	longjmp (host_abortserver, 1);
}

/*	This shuts down both the client and server
*/
void Host_Error (char *error, ...)
{
	va_list			argptr;
	char			string[1024];
	static	bool	inerror = false;

	if (inerror)
		Sys_Error ("Host_Error: recursively entered");
	inerror = true;

	SCR_EndLoadingPlaque ();		// reenable screen updates

	va_start (argptr,error);
	vsprintf (string,error,argptr);
	va_end (argptr);
	Con_Printf ("Host_Error: %s\n",string);

	if (sv.active)
		Host_ShutdownServer(false);

	if (cls.state == ca_dedicated)
		Sys_Error ("Host_Error: %s\n",string);	// dedicated servers exit

	CL_Disconnect();
	cls.demonum = -1;
	cl.intermission = 0; //johnfitz -- for errors during intermissions (changelevel with no map found, etc.)

	inerror = false;

	longjmp (host_abortserver, 1);
}

void Host_FindMaxClients(void)
{
	int		i;

	svs.maxclients = 1;

	i = COM_CheckParm ("-dedicated");
	if (i)
	{
		cls.state = ca_dedicated;
		if (i != (com_argc - 1))
			svs.maxclients = Q_atoi (com_argv[i+1]);
		else
			svs.maxclients = 8;
	}
	else
		cls.state = ca_disconnected;

	i = COM_CheckParm ("-listen");
	if (i)
	{
		if (cls.state == ca_dedicated)
			Sys_Error ("Only one of -dedicated or -listen can be specified");
		if (i != (com_argc - 1))
			svs.maxclients = Q_atoi(com_argv[i+1]);
		else
			svs.maxclients = 8;
	}

	if(svs.maxclients < 1)
		svs.maxclients = 8;
	else if(svs.maxclients > MAX_SCOREBOARD)
		svs.maxclients = MAX_SCOREBOARD;

	svs.maxclientslimit = svs.maxclients;
	if(svs.maxclientslimit < 4)
		svs.maxclientslimit = 4;
	svs.clients = (client_t*)Hunk_AllocName(svs.maxclientslimit*sizeof(client_t),"clients");

	if(svs.maxclients > 1)
		Cvar_SetValue("deathmatch",1.0f);
	else
		Cvar_SetValue("deathmatch",0);
}

void Host_InitLocal (void)
{
	Host_InitCommands();

	Cvar_RegisterVariable(&host_speeds, NULL);
	Cvar_RegisterVariable(&host_maxfps, NULL); //johnfitz
	Cvar_RegisterVariable(&host_timescale, NULL); //johnfitz
	Cvar_RegisterVariable(&max_edicts, Max_Edicts_f); //johnfitz
	Cvar_RegisterVariable(&devstats, NULL); //johnfitz
	Cvar_RegisterVariable(&sys_ticrate, NULL);
	Cvar_RegisterVariable(&serverprofile, NULL);
	Cvar_RegisterVariable(&fraglimit, NULL);
	Cvar_RegisterVariable(&timelimit, NULL);
	Cvar_RegisterVariable(&teamplay, NULL);
	Cvar_RegisterVariable(&samelevel, NULL);
	Cvar_RegisterVariable(&noexit, NULL);
	Cvar_RegisterVariable(&skill, NULL);
	Cvar_RegisterVariable(&developer, NULL);
	Cvar_RegisterVariable(&deathmatch, NULL);
	Cvar_RegisterVariable(&coop, NULL);
	Cvar_RegisterVariable(&pausable, NULL);

	// [16/2/2013] For my sanity... ~hogsy
	if(COM_CheckParm("-developer"))
		Cvar_SetValue("developer",1.0f);

	// [28/6/2013] Set the global username ~hogsy
	gFileSystem_GetUserName(Global.cLocalName);

	Host_FindMaxClients();

	dServerTime = 1.0;		// so a think at time 0 won't get called
}

/*	Writes key bindings and archived cvars to config.cfg
*/
void Host_WriteConfiguration(void)
{
	/*	Dedicated servers initialize the host but don't parse and set the
		config.cfg cvars.
	*/
	if(bHostInitialized & !bIsDedicated)
	{
		FILE	*fConfig;
		// [2/5/2013] Updated to use the new platform function ~hogsy
		char	cString[256];

		sprintf(cString,"%s/config/%s.cfg",com_gamedir,Global.cLocalName);

		fConfig = fopen(cString,"w");
		if(!fConfig)
		{
			Con_Warning("Couldn't write %s.cfg!\n",Global.cLocalName);
			return;
		}

//		VID_SyncCvars(); //johnfitz -- write actual current mode to config file, in case cvars were messed with

		Key_WriteBindings(fConfig);
		Cvar_WriteVariables(fConfig);

		//johnfitz -- extra commands to preserve state
		fprintf(fConfig,"video_restart\n");
		//johnfitz

		fclose(fConfig);
	}
}

/*	Sends text across to be displayed
	FIXME: make this just a stuffed echo?
*/
void SV_ClientPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	vsprintf (string, fmt,argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_print);
	MSG_WriteString (&host_client->message, string);
}

/*	Sends text to all active clients
*/
void SV_BroadcastPrintf(char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	int			i;

	va_start(argptr,fmt);
	vsprintf(string,fmt,argptr);
	va_end(argptr);

	for (i=0 ; i<svs.maxclients ; i++)
		if (svs.clients[i].active && svs.clients[i].bSpawned)
		{
			MSG_WriteByte (&svs.clients[i].message, svc_print);
			MSG_WriteString (&svs.clients[i].message, string);
		}
}

/*	Send text over to the client to be executed
*/
void Host_ClientCommands (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	vsprintf (string, fmt,argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_stufftext);
	MSG_WriteString (&host_client->message, string);
}

/*	Called when the player is getting totally kicked off the host
	if (crash = true), don't bother sending signofs
*/
void SV_DropClient(bool crash)
{
	int			iSaveSelf,
				i;
	client_t	*client;

	if(!crash)
	{
		// Send any final messages (don't check for errors)
		if (NET_CanSendMessage (host_client->netconnection))
		{
			MSG_WriteByte(&host_client->message,SVC_DISCONNECT);

			NET_SendMessage(host_client->netconnection,&host_client->message);
		}

		if(host_client->edict && host_client->bSpawned)
		{
			/*	Call the prog function for removing a client
				this will set the body to a dead frame, among other things	*/
			iSaveSelf = pr_global_struct.self;
			pr_global_struct.self = EDICT_TO_PROG(host_client->edict);

			Game->Game_Init(SERVER_CLIENTDISCONNECT,host_client->edict,sv.time);

			pr_global_struct.self = iSaveSelf;
		}

		Sys_Printf("Client %s removed\n",host_client->name);
	}

	// Break the net connection
	NET_Close (host_client->netconnection);
	host_client->netconnection = NULL;

	// Free the client (the body stays around)
	host_client->active		= false;
	host_client->name[0]	= 0;
	host_client->old_frags	= -999999;

	iActiveNetConnections--;

	// Send notification to all clients
	for(i = 0,client = svs.clients; i < svs.maxclients; i++,client++)
	{
		if(!client->active)
			continue;

		MSG_WriteByte (&client->message, svc_updatename);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteString (&client->message, "");
		MSG_WriteByte (&client->message, svc_updatefrags);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteShort (&client->message, 0);
		MSG_WriteByte (&client->message, svc_updatecolors);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteByte (&client->message, 0);
	}
}

/*	This only happens at the end of a game, not between levels
*/
void Host_ShutdownServer(bool crash)
{
	int			i,count;
	sizebuf_t	buf;
	char		message[4];
	double		start;

	if (!sv.active)
		return;

	sv.active = false;

// stop all client sounds immediately
	if (cls.state == ca_connected)
		CL_Disconnect ();

// flush any pending messages - like the score!!!
	start = Sys_FloatTime();
	do
	{
		count = 0;
		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->active && host_client->message.cursize)
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					NET_SendMessage(host_client->netconnection, &host_client->message);
					SZ_Clear (&host_client->message);
				}
				else
				{
					NET_GetMessage(host_client->netconnection);
					count++;
				}
			}
		}
		if ((Sys_FloatTime() - start) > 3.0)
			break;
	}
	while (count);

	// Make sure all the clients know we're disconnecting
	buf.data	= (byte*)message;
	buf.maxsize = 4;
	buf.cursize = 0;

	MSG_WriteByte(&buf,SVC_DISCONNECT);

	count = NET_SendToAll(&buf, 5);
	if (count)
		Con_Printf("Host_ShutdownServer: NET_SendToAll failed for %u clients\n", count);

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_DropClient(crash);

	// Clear structures
	memset (&sv, 0, sizeof(sv));
	memset (svs.clients, 0, svs.maxclientslimit*sizeof(client_t));
}

/*	This clears all the memory used by both the client and server, but does
	not reinitialize anything.
*/
void Host_ClearMemory(void)
{
	Con_DPrintf("Clearing memory\n");

	Model_ClearAll();

	if(host_hunklevel)
		Hunk_FreeToLowMark (host_hunklevel);

	cls.signon = 0;
	memset(&sv,0,sizeof(sv));
	memset(&cl,0,sizeof(cl));
}

/*	Returns false if the time is too short to run a frame
*/
bool Host_FilterTime(float time)
{
	float fMaxFrameRate; //johnfitz

	realtime += time;

	//johnfitz -- max fps cvar
	fMaxFrameRate = CLAMP(10.0f,host_maxfps.value,1000.0f);
	if(!cls.timedemo && realtime-dOldRealTime < 1.0f/fMaxFrameRate)
		return false; // framerate is too high
	//johnfitz

	host_frametime	= realtime-dOldRealTime;
	dOldRealTime	= realtime;

	if (host_timescale.value > 0)
		host_frametime *= host_timescale.value;
	else // Don't allow really long or short frames
		host_frametime = CLAMP(0.001f,host_frametime,0.1f); //johnfitz -- use CLAMP

	return true;
}

/*	Add them exactly as if they had been typed at the console
*/
void Host_GetConsoleCommands(void)
{
	char	*cmd;

	for(;;)
	{
		cmd = Sys_ConsoleInput();
		if(!cmd)
			break;

		Cbuf_AddText (cmd);
	}
}

void Host_ServerFrame (void)
{
	int		i,active; //johnfitz

	// Run the world state
	pr_global_struct.frametime = host_frametime;

	// Set the time and clear the general datagram
	SV_ClearDatagram ();

	// Check for new clients
	SV_CheckForNewClients ();

	// Read client messages
	SV_RunClients ();

	// move things around and think
	// always pause in single player if in console or menus
	if(!sv.paused && (svs.maxclients > 1 || key_dest == key_game))
		Physics_ServerFrame();

//johnfitz -- devstats
	if(cls.signon == SIGNONS)
	{
		edict_t *eEnttiy;

		for (i=0, active=0; i<sv.num_edicts; i++)
		{
			eEnttiy = EDICT_NUM(i);
			if(!eEnttiy->free)
				active++;
		}

		dev_stats.edicts        = active;
		dev_peakstats.edicts    = Math_Max(active,dev_peakstats.edicts);
	}
//johnfitz

	// Send all messages to the clients
	SV_SendClientMessages ();
}

/*	Runs all active servers
*/
void _Host_Frame (float time)
{
	static double	time1 = 0,
					time2 = 0,
					time3 = 0;
	int				pass1,pass2,pass3;

	if(setjmp(host_abortserver))
		return;			// Something bad happened, or the server disconnected

	// decide the simulation time
	if(!Host_FilterTime (time))
		return;			// don't run too fast, or packets will flood out

	Input_Process();

	// Process console commands
	Cbuf_Execute();
	NET_Poll();

// if running the server locally, make intentions now
	if (sv.active)
		CL_SendCmd ();

//-------------------
//
// server operations
//
//-------------------

// check for commands typed to the host
	Host_GetConsoleCommands ();

	if(sv.active)
		Host_ServerFrame ();

//-------------------
//
// client operations
//
//-------------------

// if running the server remotely, send intentions now after
// the incoming messages have been read
	if (!sv.active)
		CL_SendCmd ();

	dServerTime += host_frametime;

// fetch results from server
	if(cls.state == ca_connected)
	{
		CL_ReadFromServer();
		Client_ProcessParticles(); //johnfitz -- seperated from rendering
	}

	if(host_speeds.value)
		time1 = Sys_FloatTime();

	Video_Process();
	Audio_Process();

	if(host_speeds.value)
		time2 = Sys_FloatTime ();

	if(cls.signon == SIGNONS)
	{
		S_Update(r_origin,vpn,vright,vup);

		CL_DecayLights();
	}
	else
		S_Update(vec3_origin,vec3_origin,vec3_origin,vec3_origin);

	if (host_speeds.value)
	{
		pass1 = (time1-time3)*1000;
		time3 = Sys_FloatTime();
		pass2 = (time2-time1)*1000;
		pass3 = (time3-time2)*1000;
		Con_Printf("%3i Total\n%3i Server\n%3i Graphics\n%3i Sound\n",
					pass1+pass2+pass3, pass1, pass2, pass3);
	}

	host_framecount++;
}

void Host_Frame (float time)
{
	double			time1,time2;
	static	double	timetotal;
	static	int		timecount;
	int				i,c,m;

	if (!serverprofile.value)
	{
		_Host_Frame (time);
		return;
	}

	time1 = Sys_FloatTime ();
	_Host_Frame (time);
	time2 = Sys_FloatTime ();

	timetotal += time2 - time1;
	timecount++;

	if (timecount < 1000)
		return;

	m = timetotal*1000/timecount;
	timecount = 0;
	timetotal = 0;
	c = 0;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active)
			c++;
	}

	Con_Printf ("serverprofile: %2i clients %2i msec\n",  c,  m);
}

void Host_Version(void);	// [31/3/2013] See host_cmd.c ~hogsy

void Host_Initialize(EngineParameters_t *epParameters)
{
	if(COM_CheckParm("-minmemory"))
		epParameters->memsize = MINIMUM_MEMORY;

	host_parms = *epParameters;

	if(epParameters->memsize < MINIMUM_MEMORY)
		Sys_Error("Only %4.1f megs of memory available, can't execute game!\n",epParameters->memsize/(float)0x100000);

	com_argc = epParameters->argc;
	com_argv = epParameters->argv;

	Memory_Init(epParameters->membase,epParameters->memsize);
	Cbuf_Init();
	Cmd_Init();
	Cvar_Init(); //johnfitz
	V_Init();
	Chase_Init();
	FileSystem_Initialize();
	Host_InitLocal();
	Key_Init();
	Console_Initialize();
	Con_Init();
	Menu_Initialize();	// [23/9/2013] Initialize the menu first! ~hogsy
	Game_Initialize();
	PR_Init();
	Model_Initialize();
	Network_Initialize();

	Game->Server_Initialize();
	SV_Init();

	Con_Printf("\n");

	Host_Version();

	Con_Printf("\n%4.1f megabyte heap\n\n",epParameters->memsize/(1024*1024.0));

	if(cls.state != ca_dedicated)
	{
		Video_Initialize();
		Input_Initialize();
		Audio_Initialize();

		TexMgr_Init(); //johnfitz
		Draw_Init();
		SCR_Init();
		Menu->Initialize();
		R_Init();
#ifndef KATANA_AUDIO_OPENAL
		S_Init();
#endif

		Game->Client_Initialize();
		CL_Init();

		Editor_Initialize();
	}

	Cbuf_InsertText("exec config/katana.rc");

	Hunk_AllocName(0,"-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	bHostInitialized = true;
}

/*	FIXME: this is a callback from Sys_Quit and Sys_Error.  It would be better
	to run quit through here before the final handoff to the sys code.
*/
void Host_Shutdown(void)
{
	static bool bIsClosing = false;

	if(bIsClosing)
	{
		printf("Recursive shutdown!\n");
		return;
	}
	else
		bIsClosing = true;

	// Keep Con_Printf from trying to update the screen
	scr_disabled_for_loading = true;

	Host_WriteConfiguration ();

	// [3/8/2012] Shutdown the menu! ~hogsy
	if(Menu)
		Menu->Shutdown();

#if 0   // [28/12/2013] TODO: Secure shutdown for Game! ~hogsy
	if(Game)
		Game->Shutdown();
#endif

	Audio_Shutdown();
	Input_Shutdown();

	NET_Shutdown();
	S_Shutdown();

	if(cls.state != ca_dedicated)
		Video_Shutdown();
}

