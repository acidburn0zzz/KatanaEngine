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

#include "engine_game.h"
#include "engine_server.h"

extern cvar_t	pausable;

int	current_skill;

void Model_PrintMemoryCache(void);

void Host_Quit_f (void)
{
#if 0
	if(key_dest != key_console && cls.state != ca_dedicated)
	{
		M_Menu_Quit_f();
		return;
	}
#endif

	CL_Disconnect();

	Host_ShutdownServer(false);

	Sys_Quit();
}

//==============================================================================
//johnfitz -- dynamic gamedir stuff
//==============================================================================

// Declarations shared with common.c:
typedef struct
{
	char    name[MAX_QPATH];
	int             filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char    filename[MAX_OSPATH];
	int             handle;
	int             numfiles;
	packfile_t      *files;
} pack_t;

typedef struct searchpath_s
{
	char    filename[MAX_OSPATH];
	pack_t  *pack;          // only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

extern searchpath_t *com_searchpaths;
pack_t *FileSystem_LoadPackage(char *packfile);

// Kill all the search packs until the game path is found. Kill it, then return
// the next path to it.
void KillGameDir(searchpath_t *search)
{
	searchpath_t *search_killer;

	while (search)
	{
		if (*search->filename)
		{
			com_searchpaths = search->next;
			Z_Free(search);
			return; //once you hit the dir, youve already freed the paks
		}

		Sys_FileClose (search->pack->handle); //johnfitz

		search_killer = search->next;

		Z_Free(search->pack->files);
		Z_Free(search->pack);
		Z_Free(search);

		search = search_killer;
	}
}

// Return the number of games in memory
int NumGames(searchpath_t *search)
{
	int found = 0;
	while (search)
	{
		if (*search->filename)
			found++;
		search = search->next;
	}
	return found;
}

void ExtraMaps_NewGame(void);
void Draw_NewGame(void);		    // [20/9/2012] See gl_draw.c ~hogsy
void Host_WriteConfiguration(void); // [11/11/2013] See host.c ~hogsy

void Host_Game_f (void)
{
	int				i;
	searchpath_t	*search = com_searchpaths;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH]; //FIXME: it's confusing to use this string for two different things

	if(Cmd_Argc() > 1)
	{
		if(strstr(Cmd_Argv(1),".."))
		{
			Con_Warning("Relative pathnames are not allowed!\n");
			return;
		}

		Q_strcpy(pakfile,va("%s/%s",host_parms.basedir,Cmd_Argv(1)));
		if(!strncmp(pakfile,com_gamedir,sizeof(com_gamedir))) //no change
		{
			Con_Printf("\"game\" is already \"%s\"\n",FileSystem_SkipPath(com_gamedir));
			return;
		}

		// Kill the server
		CL_Disconnect ();
		Host_ShutdownServer(true);

		// Write config file
		Host_WriteConfiguration();

		strcpy (com_gamedir, pakfile);

		if(Q_strcasecmp(Cmd_Argv(1),host_parms.cBasePath)) //game is not id1
		{
			search = (searchpath_t*)Z_Malloc(sizeof(searchpath_t));
			strcpy (search->filename, pakfile);
			search->next = com_searchpaths;
			com_searchpaths = search;

			//Load the paks if any are found:
			for (i = 0; ; i++)
			{
				sprintf (pakfile, "%s/pak%i.pak", com_gamedir, i);
				pak = FileSystem_LoadPackage(pakfile);
				if(!pak)
					break;
				search = (searchpath_t*)Z_Malloc(sizeof(searchpath_t));
				search->pack = pak;
				search->next = com_searchpaths;
				com_searchpaths = search;
			}
		}

		//clear out and reload appropriate data
		Cache_Flush ();

		/*	TODO:
			Anything initialized by the old modules
			won't be thrown out... Yikes! Main worry
			actually is for commands.
		*/
		Game_Initialize();

		if(!bIsDedicated)
		{
			// Delete all textures where TEXPREF_PERSIST is unset
			TexMgr_FreeTextures(0,TEXPREF_PERSIST);

			Draw_NewGame();

			// [20/9/2012] Initialize the client ~hogsy
			// [27/9/2012] Updated to use the new Client_Init function ~hogsy
			Game->Client_Initialize();
		}

		// [20/9/2012] Initialize the server ~hogsy
		Game->Server_Initialize();

		ExtraMaps_NewGame();

		Con_Printf("\"game\" changed to \"%s\"\n",FileSystem_SkipPath(com_gamedir));
	}
	else //Diplay the current gamedir
		Con_Printf("\"game\" is \"%s\"\n",FileSystem_SkipPath(com_gamedir));
}

//==============================================================================
//johnfitz -- extramaps management
//==============================================================================

typedef struct extralevel_s
{
	char				name[32];
	struct extralevel_s	*next;
} extralevel_t;

extralevel_t	*extralevels;

void ExtraMaps_Add (char *name)
{
	extralevel_t	*level,*cursor,*prev;

	// Ignore duplicates
	for(level = extralevels; level; level = level->next)
		if(!strcmp(name,level->name))
			return;

	level = (extralevel_t*)Z_Malloc(sizeof(extralevel_t));
	strcpy(level->name, name);

	// Insert each entry in alphabetical order
	if (extralevels == NULL || Q_strcasecmp(level->name,extralevels->name) < 0) //insert at front
	{
		level->next = extralevels;
		extralevels = level;
	}
	else //insert later
	{
		prev = extralevels;
		cursor = extralevels->next;
		while(cursor && (Q_strcasecmp(level->name,cursor->name) > 0))
		{
			prev = cursor;
			cursor = cursor->next;
		}
		level->next = prev->next;
		prev->next = level;
	}
}

void ExtraMaps_Init(void)
{
    char            cMapName[128],
                    filestring[MAX_OSPATH],
					ignorepakdir[64];
	searchpath_t    *search;
	pack_t          *pak;
	int             i;

	//we don't want to list the maps in id1 pakfiles, becuase these are not "add-on" levels
	sprintf(ignorepakdir,"/%s/",host_parms.cBasePath);

	for (search = com_searchpaths ; search ; search = search->next)
	{
		if(*search->filename) //directory
		{
#ifdef _WIN32
            {
                WIN32_FIND_DATA	FindFileData;
                HANDLE			Find;

                sprintf(filestring,"%s/maps/*.bsp",search->filename);

                Find = FindFirstFile(filestring,&FindFileData);
                if(Find == INVALID_HANDLE_VALUE)
                    continue;
				
                do
                {
                    COM_StripExtension(FindFileData.cFileName,cMapName);

                    ExtraMaps_Add(cMapName);
                } while(FindNextFile(Find,&FindFileData));
			}
#else // linux
            {
                DIR             *dDirectory;
                struct  dirent  *dEntry;

                sprintf(filestring,"%s/maps",search->filename);

                dDirectory = opendir(filestring);
                if(dDirectory)
                {
                    while((dEntry = readdir(dDirectory)))
                    {
                        if(strstr(dEntry->d_name,".bsp"))
                        {
                            COM_StripExtension(dEntry->d_name,cMapName);

                            ExtraMaps_Add(cMapName);
                        }
                    }

                    closedir(dDirectory);
                }
            }
#endif
		}
		else //pakfile
		{
			if(!strstr(search->pack->filename,ignorepakdir)) // Don't list standard id maps
				for(i = 0,pak = search->pack; i < pak->numfiles; i++)
					if(strstr(pak->files[i].name,".bsp"))
						if(pak->files[i].filelen > 32*1024) // Don't list files under 32k (ammo boxes etc)
						{
							COM_StripExtension(pak->files[i].name+5,cMapName);
							ExtraMaps_Add(cMapName);
						}
		}
	}
}

void ExtraMaps_Clear (void)
{
	extralevel_t *blah;

	while (extralevels)
	{
		blah = extralevels->next;
		Z_Free(extralevels);
		extralevels = blah;
	}
}

void ExtraMaps_NewGame (void)
{
	ExtraMaps_Clear();
	ExtraMaps_Init();
}

void Host_Maps_f (void)
{
	int						i;
	static			bool	bListUpdated = false;
	extralevel_t			*level;

	if(!bListUpdated)
	{
		ExtraMaps_Init();

		bListUpdated = true;
	}

	for (level=extralevels, i=0; level; level=level->next, i++)
		Con_SafePrintf ("   %s\n", level->name);

	if (i)
		Con_SafePrintf ("%i map(s)\n", i);
	else
		Con_SafePrintf ("no maps found\n");
}

//==============================================================================
//johnfitz -- modlist management
//==============================================================================

typedef struct mod_s
{
	char			name[MAX_OSPATH];
	struct mod_s	*next;
} mod_t;

mod_t	*modlist;

void Modlist_Init(void) //TODO: move win32 specific stuff to sys_win.c
{
#ifdef _WIN32
	WIN32_FIND_DATA	FindFileData,FindChildData;
	HANDLE			Find,FindGame;
	mod_t			*mod,*cursor,*prev;
	char			filestring[MAX_OSPATH],childstring[MAX_OSPATH];

	sprintf(filestring,"%s/*",host_parms.basedir);
	Find = FindFirstFile(filestring,&FindFileData);
	if(Find == INVALID_HANDLE_VALUE)
	{
		FindClose(Find);
		return;
	}

	do
	{
		sprintf (childstring,"%s/%s/%s",host_parms.basedir,FindFileData.cFileName,MODULE_GAME);
		FindGame = FindFirstFile(childstring, &FindChildData);

		if(FindGame != INVALID_HANDLE_VALUE)
		{
			//ingore duplicate
			for(mod = modlist; mod; mod = mod->next)
				if(!Q_strcmp(FindFileData.cFileName, mod->name))
					return;

			mod = (mod_t*)Z_Malloc(sizeof(mod_t));
			strcpy (mod->name,FindFileData.cFileName);

			//insert each entry in alphabetical order
			if (modlist == NULL || _stricmp(mod->name, modlist->name) < 0) //insert at front
			{
				mod->next = modlist;
				modlist = mod;
			}
			else //insert later
			{
				prev = modlist;
				cursor = modlist->next;
				while (cursor && (_stricmp(mod->name, cursor->name) > 0))
				{
					prev = cursor;
					cursor = cursor->next;
				}

				mod->next = prev->next;
				prev->next = mod;
			}
		}

		FindClose(FindGame);
	} while (FindNextFile(Find,&FindFileData));

	FindClose (Find);
#endif
}

/*	List all potential mod directories (contain either a pak file or a progs.dat)
*/
void Host_Mods_f(void)
{
	int			i;
	mod_t		*mod;
	static bool	bListUpdated = false;

	if(!bListUpdated)
	{
		Modlist_Init();
		bListUpdated = true;
	}

	for (mod = modlist, i=0; mod; mod = mod->next, i++)
		Con_SafePrintf ("   %s\n", mod->name);

	if (i)
		Con_SafePrintf ("%i mod(s)\n", i);
	else
		Con_SafePrintf ("no mods found\n");
}

//==============================================================================

void Host_Mapname_f (void)
{
	char name[MAX_QPATH];

	if (sv.active)
	{
		COM_StripExtension (sv.worldmodel->name + 5, name);
		Con_Printf ("\"mapname\" is \"%s\"\n", name);
		return;
	}

	if (cls.state == ca_connected)
	{
		COM_StripExtension (cl.worldmodel->name + 5, name);
		Con_Printf ("\"mapname\" is \"%s\"\n", name);
		return;
	}

	Con_Printf ("no map loaded\n");
}

void Host_Status_f (void)
{
	client_t	*client;
	int			seconds;
	int			minutes;
	int			hours = 0;
	int			j;
	void		(*print) (char *fmt, ...);

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
		print = Con_Printf;
	}
	else
		print = SV_ClientPrintf;

	print("host:    %s\n", Cvar_VariableString ("hostname"));
	print("Version: %i\n",ENGINE_VERSION_BUILD);
	if (tcpipAvailable)
		print ("tcp/ip:  %s\n", my_tcpip_address);
	print("map:     %s\n", sv.name);
	print("players: %i active (%i max)\n\n", iActiveNetConnections, svs.maxclients);
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = (int)(net_time - client->netconnection->connecttime);
		minutes = seconds/60;
		if (minutes)
		{
			seconds -= (minutes*60);
			hours = minutes/60;
			if (hours)
				minutes -= (hours*60);
		}
		else
			hours = 0;
		print ("#%-2u %-16.16s  %3i  %2i:%02i:%02i\n",j+1,client->name,client->edict->v.iScore,hours,minutes,seconds);
		print ("   %s\n", client->netconnection->address);
	}
}

/*	Sets client to godmode
*/
void Host_God_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

#if 0
	// [8/6/2012] TODO: Add server-side game_cheats cvar and check it here... ~hogsy
	if(!host_client->privileged)
		return;
#endif

	//johnfitz -- allow user to explicitly set god mode to on or off
	switch (Cmd_Argc())
	{
	case 1:
		sv_player->v.flags = sv_player->v.flags ^ FL_GODMODE;
		if (!(sv_player->v.flags & FL_GODMODE) )
			SV_ClientPrintf ("godmode OFF\n");
		else
			SV_ClientPrintf ("godmode ON\n");
		break;
	case 2:
		if(atof(Cmd_Argv(1)))
		{
			sv_player->v.flags = sv_player->v.flags | FL_GODMODE;
			SV_ClientPrintf ("godmode ON\n");
		}
		else
		{
			sv_player->v.flags = sv_player->v.flags & ~FL_GODMODE;
			SV_ClientPrintf ("godmode OFF\n");
		}
		break;
	default:
		Con_Printf("god [value] : toggle god mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}

void Host_Notarget_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	// [8/6/2012] TODO: Add server-side game_cheats cvar and check it here... ~hogsy
	if(!host_client->privileged)
		return;

	//johnfitz -- allow user to explicitly set notarget to on or off
	switch (Cmd_Argc())
	{
	case 1:
		sv_player->v.flags = sv_player->v.flags ^ FL_NOTARGET;
		if (!(sv_player->v.flags & FL_NOTARGET) )
			SV_ClientPrintf ("notarget OFF\n");
		else
			SV_ClientPrintf ("notarget ON\n");
		break;
	case 2:
		if (Q_atof(Cmd_Argv(1)))
		{
			sv_player->v.flags = (int)sv_player->v.flags | FL_NOTARGET;
			SV_ClientPrintf ("notarget ON\n");
		}
		else
		{
			sv_player->v.flags = (int)sv_player->v.flags & ~FL_NOTARGET;
			SV_ClientPrintf ("notarget OFF\n");
		}
		break;
	default:
		Con_Printf("notarget [value] : toggle notarget mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}

bool noclip_anglehack;

void Host_Noclip_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer();
		return;
	}

	// [8/6/2012] TODO: Add server-side game_cheats cvar and check it here... ~hogsy
#if 0
	if(!host_client->privileged)
		return;
#endif

	//johnfitz -- allow user to explicitly set noclip to on or off
	switch (Cmd_Argc())
	{
	case 1:
		if (sv_player->v.movetype != MOVETYPE_NOCLIP)
		{
			noclip_anglehack = true;
			sv_player->v.movetype = MOVETYPE_NOCLIP;
			SV_ClientPrintf ("noclip ON\n");
		}
		else
		{
			noclip_anglehack = false;
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("noclip OFF\n");
		}
		break;
	case 2:
		if (Q_atof(Cmd_Argv(1)))
		{
			noclip_anglehack = true;
			sv_player->v.movetype = MOVETYPE_NOCLIP;
			SV_ClientPrintf ("noclip ON\n");
		}
		else
		{
			noclip_anglehack = false;
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("noclip OFF\n");
		}
		break;
	default:
		Con_Printf("noclip [value] : toggle noclip mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}

/*	Sets client to flymode
*/
void Host_Fly_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	// [8/6/2012] TODO: Add server-side game_cheats cvar and check it here... ~hogsy
#if 0
	if(!host_client->privileged)
		return;
#endif

	//johnfitz -- allow user to explicitly set noclip to on or off
	switch (Cmd_Argc())
	{
	case 1:
		if (sv_player->v.movetype != MOVETYPE_FLY)
		{
			sv_player->v.movetype = MOVETYPE_FLY;
			SV_ClientPrintf ("flymode ON\n");
		}
		else
		{
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("flymode OFF\n");
		}
		break;
	case 2:
		if (Q_atof(Cmd_Argv(1)))
		{
			sv_player->v.movetype = MOVETYPE_FLY;
			SV_ClientPrintf ("flymode ON\n");
		}
		else
		{
			sv_player->v.movetype = MOVETYPE_WALK;
			SV_ClientPrintf ("flymode OFF\n");
		}
		break;
	default:
		Con_Printf("fly [value] : toggle fly mode. values: 0 = off, 1 = on\n");
		break;
	}
	//johnfitz
}

void Host_Ping_f (void)
{
	int		i, j;
	float	total;
	client_t	*client;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	SV_ClientPrintf ("Client ping times:\n");
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		total = 0;
		for (j=0 ; j<NUM_PING_TIMES ; j++)
			total+=client->ping_times[j];
		total /= NUM_PING_TIMES;
		SV_ClientPrintf ("%4i %s\n", (int)(total*1000), client->name);
	}
}

/*
===============================================================================

SERVER TRANSITIONS

===============================================================================
*/

#include "engine_input.h"

/*	handle a
	map <servername>
	command from the console.  Active clients are kicked off.
*/
void Host_Map_f (void)
{
	int		i;
	char	name[MAX_QPATH];

	if (cmd_source != src_command)
		return;

	cls.demonum = -1;		// stop demo loop in case this fails

	CL_Disconnect ();
	Host_ShutdownServer(false);

	key_dest = key_game;			// remove console or menu

	Input_ActivateMouse();

	SCR_BeginLoadingPlaque();

	cls.mapstring[0] = 0;
	for (i=0 ; i<Cmd_Argc() ; i++)
	{
		strcat (cls.mapstring, Cmd_Argv(i));
		strcat (cls.mapstring, " ");
	}
	strcat (cls.mapstring, "\n");

	svs.serverflags = 0;			// haven't completed an episode yet
	strcpy (name, Cmd_Argv(1));

	SV_SpawnServer (name);

	if (!sv.active)
		return;

	if (cls.state != ca_dedicated)
	{
		strcpy (cls.spawnparms, "");

		for (i=2 ; i<Cmd_Argc() ; i++)
		{
			strcat (cls.spawnparms, Cmd_Argv(i));
			strcat (cls.spawnparms, " ");
		}

		Cmd_ExecuteString ("connect local", src_command);
	}
}

/*	Goes to a new map, taking all clients along
*/
void Host_Changelevel_f (void)
{
	char	level[MAX_QPATH];
	int		i; //johnfitz

	if(Cmd_Argc() != 2)
	{
		Con_Printf ("changelevel <levelname> : continue game on a new level\n");
		return;
	}
	else if(!sv.active || cls.demoplayback)
	{
		Con_Printf("Only the server may changelevel\n");
		return;
	}

	//johnfitz -- check for client having map before anything else
	sprintf (level,PATH_MAPS"/%s.bsp", Cmd_Argv(1));
	if (COM_OpenFile (level, &i) == -1)
		Host_Error ("cannot find map %s", level);
	//johnfitz

	SV_SaveSpawnparms ();
	strcpy (level, Cmd_Argv(1));
	SV_SpawnServer (level);
}

/*	Restarts the current server for a dead player
*/
void Host_Restart_f (void)
{
	char	mapname[MAX_QPATH];

	if (cls.demoplayback || !sv.active)
		return;
	else if (cmd_source != src_command)
		return;

	Q_strcpy(mapname,sv.name);	// must copy out, because it gets cleared
								// in sv_spawnserver
	SV_SpawnServer (mapname);
}

/*	This command causes the client to wait for the signon messages again.
	This is sent just before a server changes levels
*/
void Host_Reconnect_f (void)
{
	SCR_BeginLoadingPlaque ();
	cls.signon = 0;		// need new connection messages
}

/*	User command to connect to server
*/
void Host_Connect_f (void)
{
	char	name[MAX_QPATH];

	cls.demonum = -1;		// stop demo loop in case this fails
	if (cls.demoplayback)
	{
		CL_StopPlayback ();
		CL_Disconnect ();
	}
	Q_strcpy(name,Cmd_Argv(1));
	CL_EstablishConnection (name);
	Host_Reconnect_f ();
}


/*
===============================================================================

LOAD / SAVE GAME

===============================================================================
*/

#define	SAVEGAME_VERSION	5

/*	Writes a SAVEGAME_COMMENT_LENGTH character comment describing the current
*/
void Host_SavegameComment (char *text)
{
	int		i;
	char	kills[20];

	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		text[i] = ' ';
	memcpy(text,cl.levelname,Math_Min(strlen(cl.levelname),22)); //johnfitz -- only copy 22 chars.
	sprintf(kills,"kills:%3i/%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
	memcpy(text+22,kills,strlen(kills));

	// Convert space to _ to make stdio happy
	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		if (text[i] == ' ')
			text[i] = '_';
	text[SAVEGAME_COMMENT_LENGTH] = '\0';
}

void Host_Savegame_f (void)
{
	char	name[256];
	FILE	*f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];

	if (cmd_source != src_command)
		return;

	if(!sv.active)
	{
		Con_Printf ("Not playing a local game.\n");
		return;
	}
	else if(cl.intermission)
	{
		Con_Printf ("Can't save in intermission.\n");
		return;
	}
	else if(svs.maxclients != 1)
	{
		Con_Printf ("Can't save multiplayer games.\n");
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("save <savename> : save a game\n");
		return;
	}

	if (strstr(Cmd_Argv(1), ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return;
	}

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.iHealth <= 0) )
		{
			Con_Printf ("Can't savegame with a dead player\n");
			return;
		}
	}

	sprintf (name, "%s/%s", com_gamedir, Cmd_Argv(1));
	COM_DefaultExtension (name, ".sav");

	Con_Printf ("Saving game to %s...\n", name);
	f = fopen (name, "w");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);
	Host_SavegameComment (comment);
	fprintf (f, "%s\n", comment);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
	fprintf (f, "%d\n", current_skill);
	fprintf (f, "%s\n", sv.name);
	fprintf (f, "%f\n",sv.time);

// write the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		if (sv.lightstyles[i])
			fprintf (f, "%s\n", sv.lightstyles[i]);
		else
			fprintf (f,"m\n");
	}


	ED_WriteGlobals (f);
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ED_Write (f, EDICT_NUM(i));
		fflush (f);
	}
	fclose (f);
	Con_Printf ("done.\n");
}

void Host_Loadgame_f (void)
{
	char	name[MAX_OSPATH];
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	time, tfloat;
	char	str[32768], *start;
	int		i, r;
	edict_t	*ent;
	int		entnum,version;
	float	spawn_parms[NUM_SPAWN_PARMS];

	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("load <savename> : load a game\n");
		return;
	}

	cls.demonum = -1;		// stop demo loop in case this fails

	sprintf (name, "%s/%s", com_gamedir, Cmd_Argv(1));
	COM_DefaultExtension (name, ".sav");

// we can't call SCR_BeginLoadingPlaque, because too much stack space has
// been used.  The menu calls it before stuffing loadgame command
//	SCR_BeginLoadingPlaque ();

	Con_Printf ("Loading game from %s...\n", name);
	f = fopen (name, "r");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	fscanf (f, "%i\n", &version);
	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return;
	}
	fscanf (f, "%s\n", str);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fscanf (f, "%f\n", &spawn_parms[i]);
// this silliness is so we can load 1.06 save files, which have float skill values
	fscanf (f, "%f\n", &tfloat);
	current_skill = (int)(tfloat + 0.1);
	Cvar_SetValue ("skill", (float)current_skill);

	fscanf (f, "%s\n",mapname);
	fscanf (f, "%f\n",&time);

	CL_Disconnect_f ();

	SV_SpawnServer (mapname);

	if (!sv.active)
	{
		Con_Printf ("Couldn't load map\n");
		return;
	}

	sv.paused	= true;		// Pause until all clients connect
	sv.loadgame = true;

	// Load the light styles
	for(i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		fscanf(f,"%s\n",str);

		sv.lightstyles[i] = (char*)Hunk_Alloc(strlen(str)+1);

		strcpy(sv.lightstyles[i],str);
	}

// load the edicts out of the savegame file
	entnum = -1;		// -1 is the globals
	while (!feof(f))
	{
		for (i=0 ; i<sizeof(str)-1 ; i++)
		{
			r = fgetc (f);
			if (r == EOF || !r)
				break;
			str[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i == sizeof(str)-1)
			Sys_Error ("Loadgame buffer overflow");
		str[i] = 0;
		start = str;
		start = COM_Parse(str);
		if (!com_token[0])
			break;		// end of file
		if (strcmp(com_token,"{"))
			Sys_Error ("First token isn't a brace");

		if (entnum == -1)
			ED_ParseGlobals(start);
		else
		{	// parse an edict

			ent = EDICT_NUM(entnum);
			memset (&ent->v, 0,sizeof(ent->v));
			ent->free = false;
			ED_ParseEdict (start, ent);

		// link it into the bsp tree
			if(!ent->free)
				SV_LinkEdict(ent,false);
		}

		entnum++;
	}

	sv.num_edicts = entnum;
	sv.time = time;

	fclose (f);

	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		svs.clients->spawn_parms[i] = spawn_parms[i];

	if (cls.state != ca_dedicated)
	{
		CL_EstablishConnection ("local");
		Host_Reconnect_f ();
	}
}

//============================================================================

void Host_Name_f (void)
{
	char	*newName;

	if (Cmd_Argc () == 1)
	{
		Con_Printf ("\"name\" is \"%s\"\n", cl_name.string);
		return;
	}

	if (Cmd_Argc () == 2)
		newName = Cmd_Argv(1);
	else
		newName = Cmd_Args();
	newName[15] = 0;

	if (cmd_source == src_command)
	{
		if (Q_strcmp(cl_name.string, newName) == 0)
			return;
		Cvar_Set ("_cl_name", newName);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer ();
		return;
	}

	if(host_client->name[0] && strcmp(host_client->name, "unconnected") )
		if(strcmp(host_client->name, newName) != 0)
			Con_Printf("%s renamed to %s\n",host_client->name,newName);
	strcpy(host_client->name, newName);
	host_client->edict->v.netname = host_client->name;

	// Send notification to all clients
	MSG_WriteByte(&sv.reliable_datagram,svc_updatename);
	MSG_WriteByte(&sv.reliable_datagram,host_client - svs.clients);
	MSG_WriteString(&sv.reliable_datagram,host_client->name);
}

void Host_Version(void)
{
	Con_Printf("Katana %i.%i.%i ("PLATFORM_NAME""PLATFORM_CPU")\n",
		ENGINE_VERSION_MAJOR,
		ENGINE_VERSION_MINOR,
		ENGINE_VERSION_BUILD);
	Con_Printf("Compiled: "__TIME__" "__DATE__"\n");
}

void Host_Say(bool bTeamOnly)
{
	client_t		*client,*save;
	int				j;
	char			*p;
	unsigned char	text[64];
	bool			bFromServer = false;

	if (cmd_source == src_command)
	{
		if (cls.state == ca_dedicated)
		{
			bFromServer	= true;
			bTeamOnly	= false;
		}
		else
		{
			Cmd_ForwardToServer ();
			return;
		}
	}

	if (Cmd_Argc () < 2)
		return;

	save = host_client;

	p = Cmd_Args();
	// Remove quotes if present
	if(*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

	// Turn on color set 1
	if(!bFromServer)
		sprintf((char*)text, "%c%s: ", 1, save->name);
	else
		sprintf((char*)text, "%c<%s> ", 1, hostname.string);

	j = sizeof(text)-2-strlen((char*)text);  // -2 for /n and null terminator
	if(strlen(p) > j)
		p[j] = 0;

	strcat((char*)text,p);
	strcat((char*)text,"\n");

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client || !client->active || !client->bSpawned)
			continue;
		if (teamplay.value && bTeamOnly && client->edict->local.pTeam != save->edict->local.pTeam)
			continue;

		host_client = client;
		SV_ClientPrintf("%s", text);
	}
	host_client = save;

	Sys_Printf("%s", &text[1]);
}

void Host_Say_f(void)
{
	Host_Say(false);
}

void Host_Say_Team_f(void)
{
	Host_Say(true);
}

void Host_Tell_f(void)
{
	client_t	*client,*save;
	int			j;
	char		*p,text[64];

	if(cmd_source == src_command)
	{
		Cmd_ForwardToServer();
		return;
	}
	else if(Cmd_Argc() < 3)
		return;

	strcpy(text,host_client->name);
	strcat(text,": ");

	p = Cmd_Args();

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[Q_strlen(p)-1] = 0;
	}

// check length & truncate if necessary
	j = sizeof(text)-2-strlen(text);  // -2 for /n and null terminator
	if(strlen(p) > j)
		p[j] = 0;

	strcat (text, p);
	strcat (text, "\n");

	save = host_client;
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if(!client->active || !client->bSpawned)
			continue;
		if(Q_strcasecmp(client->name, Cmd_Argv(1)))
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
		break;
	}
	host_client = save;
}

void Host_Color_f(void)
{
	int		top, bottom;
	int		playercolor;

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"color\" is \"%i %i\"\n", ((int)cl_color.value) >> 4, ((int)cl_color.value) & 0x0f);
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else
	{
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}

	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;

	playercolor = top*16 + bottom;

	if (cmd_source == src_command)
	{
		Cvar_SetValue ("_cl_color", playercolor);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer ();
		return;
	}

	host_client->colors	= playercolor;

	// Send notification to all clients
	MSG_WriteByte(&sv.reliable_datagram,SVC_UPDATECOLORS);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, host_client->colors);
}

void Host_Kill_f (void)
{
	if(cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	pr_global_struct.self = EDICT_TO_PROG(sv_player);

	// [11/7/2013] Cleaned up and given its own function ~hogsy
	Game->Server_KillClient(sv_player);
}

void Host_Pause_f(void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (!pausable.value)
		SV_ClientPrintf ("Pause not allowed.\n");
	else
	{
		sv.paused ^= 1;

		if (sv.paused)
			SV_BroadcastPrintf ("%s paused the game\n",sv_player->v.netname);
		else
			SV_BroadcastPrintf ("%s unpaused the game\n",sv_player->v.netname);

		// Send notification to all clients
		MSG_WriteByte (&sv.reliable_datagram, svc_setpause);
		MSG_WriteByte (&sv.reliable_datagram, sv.paused);
	}
}

//===========================================================================

void Host_PreSpawn_f (void)
{
	if(cmd_source == src_command)
	{
		Con_Printf ("prespawn is not valid from the console\n");
		return;
	}
	else if(host_client->bSpawned)
	{
		Con_Printf ("prespawn not valid -- allready spawned\n");
		return;
	}

	SZ_Write (&host_client->message, sv.signon.data, sv.signon.cursize);
	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 2);

	host_client->sendsignon = true;
}

void Host_Spawn_f(void)
{
	int			i;
	client_t	*client;
	edict_t		*eClient;

	if (cmd_source == src_command)
	{
		Con_Printf("Spawn is not valid from the console\n");
		return;
	}
	else if (host_client->bSpawned)
	{
		Con_Printf("Spawn not valid -- allready spawned\n");
		return;
	}

	// Run the entrance script
	if (sv.loadgame)
		// loaded games are fully inited allready
		// if this is the last client to be connected, unpause
		sv.paused = false;
	else
	{
		// Set up the edict
		eClient = host_client->edict;

		memset(&eClient->v,0,sizeof(eClient->v));
		eClient->v.colormap = NUM_FOR_EDICT(eClient);
		eClient->v.netname	= host_client->name;

		// copy spawn parms out of the client_t
		for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
			(&pr_global_struct.parm1)[i] = host_client->spawn_parms[i];

		// call the spawn function

		pr_global_struct.self = EDICT_TO_PROG(sv_player);

		Game->Game_Init(SERVER_CLIENTCONNECT,sv_player,sv.time);

		if ((Sys_FloatTime() - host_client->netconnection->connecttime) <= sv.time)
			Sys_Printf ("%s entered the game\n", host_client->name);

		Game->Server_SpawnPlayer(eClient);
	}

	// send all current names, colors, and frag counts
	SZ_Clear (&host_client->message);

	// send time of update
	MSG_WriteByte (&host_client->message,SVC_TIME);
	MSG_WriteFloat (&host_client->message,sv.time);

	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		MSG_WriteByte (&host_client->message, svc_updatename);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteString (&host_client->message, client->name);
		MSG_WriteByte (&host_client->message, svc_updatefrags);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteShort (&host_client->message, client->old_frags);
		MSG_WriteByte (&host_client->message, svc_updatecolors);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteByte (&host_client->message, client->colors);
	}

	// Send all current light styles
	for(i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		MSG_WriteByte(&host_client->message,svc_lightstyle);
		MSG_WriteByte(&host_client->message,(char)i);
		MSG_WriteString(&host_client->message,sv.lightstyles[i]);
	}

//
// send some stats
//
	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALSECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct.total_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->message, pr_global_struct.total_monsters);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_SECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct.found_secrets);

	MSG_WriteByte(&host_client->message,svc_updatestat);
	MSG_WriteByte(&host_client->message,STAT_MONSTERS);
	MSG_WriteLong(&host_client->message,pr_global_struct.iKilledMonsters);

//
// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	eClient = EDICT_NUM(1+(host_client-svs.clients));
	MSG_WriteByte(&host_client->message,SVC_SETANGLE);
	for(i = 0; i < 2; i++)
		MSG_WriteAngle(&host_client->message,eClient->v.angles[i]);
	MSG_WriteAngle(&host_client->message,0);

	SV_WriteClientdataToMessage (sv_player, &host_client->message);

	MSG_WriteByte(&host_client->message,svc_signonnum);
	MSG_WriteByte(&host_client->message,3);

	host_client->sendsignon = true;
}

void Host_Begin_f(void)
{
	if(cmd_source == src_command)
	{
		Con_Printf("Begin is not valid from the console.\n");
		return;
	}

	host_client->bSpawned = true;
}

//===========================================================================

void Host_Kick_f (void)
{
	char		*who,
				*message = NULL;
	client_t	*save;
	int			i;
	bool		bByNumber = false;

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
	}
	else if(!host_client->privileged)
		return;

	save = host_client;

	if(Cmd_Argc() > 2 && Q_strcmp(Cmd_Argv(1), "#") == 0)
	{
		i = Q_atof(Cmd_Argv(2)) - 1;
		if(i < 0 || i >= svs.maxclients)
			return;
		else if(!svs.clients[i].active)
			return;

		host_client = &svs.clients[i];

		bByNumber = true;
	}
	else
	{
		for(i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if(!host_client->active)
				continue;

			if(Q_strcasecmp(host_client->name,Cmd_Argv(1)) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (cmd_source == src_command)
			if (cls.state == ca_dedicated)
				who = "Console";
			else
				who = cl_name.string;
		else
			who = save->name;

		// Can't kick yourself!
		if (host_client == save)
			return;

		if (Cmd_Argc() > 2)
		{
			message = COM_Parse(Cmd_Args());
			if(bByNumber)
			{
				message++;							// skip the #
				while (*message == ' ')				// skip white space
					message++;
				message += strlen(Cmd_Argv(2));	// skip the number
			}
			while (*message && *message == ' ')
				message++;
		}

		if (message)
			SV_ClientPrintf ("Kicked by %s: %s\n", who, message);
		else
			SV_ClientPrintf ("Kicked by %s\n", who);

		SV_DropClient(false);
	}

	host_client = save;
}

/*
===============================================================================

DEBUGGING TOOLS

===============================================================================
*/

void Host_Give_f (void)
{
#if 0
	char	*t;
	int		v;
	eval_t	*val;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct.deathmatch && !host_client->privileged)
		return;

	t = Cmd_Argv(1);
	v = atoi (Cmd_Argv(2));

	switch (t[0])
	{
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
		 if (t[0] >= '2')
			sv_player->v.items = sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
		break;
	case 'l':
		break;
	case 'r':
		sv_player->v.ammo_rockets = v;
		break;
	case 'm':
		break;
	case 'h':
		sv_player->v.health = v;
		break;
	case 'c':
		sv_player->v.ammo_cells = v;
		break;
	//johnfitz -- give armour
	case 'a':
		if (v > 150)
		{
			sv_player->v.armortype = 0.8f;
			sv_player->v.armorvalue = v;
			sv_player->v.items = sv_player->v.items - ((sv_player->v.items) & (int)(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3)) + IT_ARMOR3;
		}
		else if (v > 100)
		{
			sv_player->v.armortype = 0.6f;
			sv_player->v.armorvalue = v;
			sv_player->v.items = sv_player->v.items - ((sv_player->v.items) & (int)(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3)) + IT_ARMOR2;
		}
		else if (v >= 0)
		{
			sv_player->v.armortype = 0.3f;
			sv_player->v.armorvalue = v;
			sv_player->v.items = sv_player->v.items - ((sv_player->v.items) & (int)(IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3)) + IT_ARMOR1;
		}
		break;
	//johnfitz
	}

	//johnfitz -- update currentammo to match new ammo (so statusbar updates correctly)
	switch((sv_player->v.iActiveWeapon))
	{
	case IT_SHOTGUN:
	case IT_SUPER_SHOTGUN:
		sv_player->v.currentammo = sv_player->v.ammo_shells;
		break;
	case IT_NAILGUN:
	case IT_SUPER_NAILGUN:
	case RIT_LAVA_SUPER_NAILGUN:
		sv_player->v.currentammo = sv_player->v.ammo_nails;
		break;
	case IT_GRENADE_LAUNCHER:
	case IT_ROCKET_LAUNCHER:
	case RIT_MULTI_GRENADE:
	case RIT_MULTI_ROCKET:
		sv_player->v.currentammo = sv_player->v.ammo_rockets;
		break;
	case IT_LIGHTNING:
	case HIT_LASER_CANNON:
	case HIT_MJOLNIR:
		sv_player->v.currentammo = sv_player->v.ammo_cells;
		break;
	}
	//johnfitz
#endif
}

edict_t	*FindViewthing (void)
{
	int		i;
	edict_t	*e;

	for (i=0 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		if (!strcmp(e->v.cClassname, "viewthing"))
			return e;
	}
	Con_Printf ("No viewthing on map\n");
	return NULL;
}

void Host_Viewmodel_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = Mod_ForName (Cmd_Argv(1));
	if (!m)
	{
		Con_Printf ("Can't load %s\n", Cmd_Argv(1));
		return;
	}

	e->v.frame = 0;
	cl.model_precache[(int)e->v.modelindex] = m;
}

void Host_Viewframe_f (void)
{
	edict_t	*e;
	int		f;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	f = atoi(Cmd_Argv(1));
	if (f >= m->numframes)
		f = m->numframes-1;

	e->v.frame = f;
}

void PrintFrameName(model_t *m,int frame)
{
	// [16/9/2012] TODO: Quickly threw this together, needs to be tested (pretty sure md2 models don't even use frame names anymore) ~hogsy
	MD2_t				*mModel;
	MD2Frame_t			*mFrame;

	mModel = (MD2_t*)Mod_Extradata(m);
	if(!mModel)
		return;

	mFrame = (MD2Frame_t*)((int)mModel+mModel->ofs_frames+(mModel->framesize*currententity->draw_pose));

	Con_Printf("Frame %i (%s)\n",frame,mFrame->name);
}

void Host_Viewnext_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame + 1;
	if (e->v.frame >= m->numframes)
		e->v.frame = m->numframes - 1;

	PrintFrameName (m, e->v.frame);
}

void Host_Viewprev_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame - 1;
	if (e->v.frame < 0)
		e->v.frame = 0;

	PrintFrameName (m, e->v.frame);
}

/*
===============================================================================

DEMO LOOP CONTROL

===============================================================================
*/

void Host_Startdemos_f (void)
{
	int		i, c;

	if (cls.state == ca_dedicated)
	{
		if (!sv.active)
			Cbuf_AddText ("map start\n");
		return;
	}

	c = Cmd_Argc() - 1;
	if (c > MAX_DEMOS)
	{
		Con_Printf ("Max %i demos in demoloop\n", MAX_DEMOS);
		c = MAX_DEMOS;
	}
	Con_Printf ("%i demo(s) in loop\n", c);

	for (i=1 ; i<c+1 ; i++)
		strncpy (cls.demos[i-1], Cmd_Argv(i), sizeof(cls.demos[0])-1);

	if (!sv.active && cls.demonum != -1 && !cls.demoplayback)
	{
		cls.demonum = 0;
		CL_NextDemo ();
	}
	else
		cls.demonum = -1;
}


/*	Return to looping demos
*/
void Host_Demos_f (void)
{
	if (cls.state == ca_dedicated)
		return;
	if (cls.demonum == -1)
		cls.demonum = 1;
	CL_Disconnect_f ();
	CL_NextDemo ();
}

/*	Return to looping demos
*/
void Host_Stopdemo_f (void)
{
	if (cls.state == ca_dedicated)
		return;
	if (!cls.demoplayback)
		return;
	CL_StopPlayback ();
	CL_Disconnect ();
}

//=============================================================================

void Host_InitCommands (void)
{
	Cmd_AddCommand("maps",Host_Maps_f); //johnfitz
	Cmd_AddCommand("game",Host_Game_f); //johnfitz
	Cmd_AddCommand("mods",Host_Mods_f); //johnfitz
	Cmd_AddCommand("mapname",Host_Mapname_f); //johnfitz
	Cmd_AddCommand("status",Host_Status_f);
	Cmd_AddCommand("quit",Host_Quit_f);
	Cmd_AddCommand("god",Host_God_f);
	Cmd_AddCommand("notarget",Host_Notarget_f);
	Cmd_AddCommand("fly",Host_Fly_f);
	Cmd_AddCommand("map",Host_Map_f);
	Cmd_AddCommand("restart",Host_Restart_f);
	Cmd_AddCommand("changelevel",Host_Changelevel_f);
	Cmd_AddCommand("connect",Host_Connect_f);
	Cmd_AddCommand("reconnect",Host_Reconnect_f);
	Cmd_AddCommand("name",Host_Name_f);
	Cmd_AddCommand("noclip",Host_Noclip_f);
	Cmd_AddCommand("version",Host_Version);
	Cmd_AddCommand("say",Host_Say_f);
	Cmd_AddCommand("say_team",Host_Say_Team_f);
	Cmd_AddCommand("tell",Host_Tell_f);
	Cmd_AddCommand("color",Host_Color_f);
	Cmd_AddCommand("kill",Host_Kill_f);
	Cmd_AddCommand("pause",Host_Pause_f);
	Cmd_AddCommand("spawn",Host_Spawn_f);
	Cmd_AddCommand("begin",Host_Begin_f);
	Cmd_AddCommand("prespawn",Host_PreSpawn_f);
	Cmd_AddCommand("kick",Host_Kick_f);
	Cmd_AddCommand("ping",Host_Ping_f);
	Cmd_AddCommand("load",Host_Loadgame_f);
	Cmd_AddCommand("save",Host_Savegame_f);
	Cmd_AddCommand("give",Host_Give_f);
	Cmd_AddCommand("startdemos",Host_Startdemos_f);
	Cmd_AddCommand("demos",Host_Demos_f);
	Cmd_AddCommand("stopdemo",Host_Stopdemo_f);
	Cmd_AddCommand("viewmodel",Host_Viewmodel_f);
	Cmd_AddCommand("viewframe",Host_Viewframe_f);
	Cmd_AddCommand("viewnext",Host_Viewnext_f);
	Cmd_AddCommand("viewprev",Host_Viewprev_f);
	Cmd_AddCommand("mcache",Model_PrintMemoryCache);
}
