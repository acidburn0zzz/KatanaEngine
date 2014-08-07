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
#ifndef QUAKEDEF_H
#define QUAKEDEF_H

#include "platform.h"

#ifdef _MSC_VER
#pragma warning(disable:4115)
#pragma warning(disable:4996)	// [7/7/2013] Annoying VS warnings... Sigh ~hogsy
#pragma warning(disable:4229)   // mgraph gets this
#pragma warning(disable:4305)
#endif

#define MINIMUM_MEMORY	0x2000000	// 32MB

#define MAX_NUM_ARGVS	50

#define	ON_EPSILON		0.1			// point on plane side epsilon

#define	MAX_MSGLEN		32000	// max length of a reliable message
#define	MAX_DATAGRAM	32000	// max length of unreliable message

#define DATAGRAM_MTU	1400	// johnfitz -- actual limit for unreliable messages to nonlocal clients

// per-level limits
#define MIN_EDICTS		256			// johnfitz -- lowest allowed value for max_edicts cvar
#define MAX_EDICTS		32000		// johnfitz -- highest allowed value for max_edicts cvar
									// ents past 8192 can't play sounds in the standard protocol
#define	MAX_LIGHTSTYLES	64
#define	MAX_MODELS		2048		// johnfitz -- was 256
#define	MAX_SOUNDS		2048		// johnfitz -- was 256
#define MAX_PARTICLES	32000		// Default max # of particles at one time
#define MAX_FLARES		2048		// Max number of flares
#define MAX_EFFECTS		1024		// Max textures for particles, sprites and flares

#define	SAVEGAME_COMMENT_LENGTH	39

#define	IT_INVISIBILITY		524288

#define	MAX_SCOREBOARD		64

#include "shared_flags.h"

#include "cmdlib.h"
#include "common.h"
#include "vid.h"
#include "sys.h"
#include "zone.h"

#include "shared_math.h"
#include "shared_client.h"
#include "shared_engine.h"
#include "shared_formats.h"

#include "engine_main.h"
//#include "bspfile.h"
#include "draw.h"
#include "cvar.h"
#include "screen.h"
#include "net.h"
#include "protocol.h"
#include "cmd.h"
#include "sound.h"
#include "render.h"
#include "client.h"
#include "progs.h"
#include "server.h"
#include "gl_model.h"
#include "image.h" //johnfitz
#include "gl_texmgr.h" //johnfitz
#include "world.h"
#include "keys.h"
#include "console.h"
#include "view.h"
#include "menu.h"
#include "crc.h"
#include "glquake.h"

//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct
{
	char	*basedir,
			*cachedir;		// For development over ISDN lines.
	int		argc;
	char	**argv;
	void	*membase;
	int		memsize;

	char	cBasePath[MAX_QPATH];	// Base directory for game assets.
} EngineParameters_t;


//=============================================================================

extern bool noclip_anglehack;

// host
extern	EngineParameters_t host_parms;

extern	cvar_t		sys_ticrate;
extern	cvar_t		sys_nostdout;
extern	cvar_t		developer;
extern	cvar_t		max_edicts; //johnfitz

extern	bool		bHostInitialized;		// True if into command execution
extern	double		host_frametime;
extern	int			host_framecount;	// incremented every frame, never reset
extern	double		realtime;			// not bounded in any way, changed at
										// start of every frame, never reset

void Host_ClearMemory (void);
void Host_ServerFrame (void);
void Host_InitCommands (void);
void Host_Initialize(EngineParameters_t *parms);
void Host_Shutdown(void);
void Host_Error (char *error, ...);
void Host_EndGame (char *message, ...);
void Host_Frame (float time);
void Host_Quit_f (void);
void Host_ClientCommands (char *fmt, ...);
void Host_ShutdownServer (bool crash);

extern bool		msg_suppress_1;		// suppresses resolution and cache size console output
										//  an fullscreen DIB focus gain/loss
extern int			current_skill;		// skill level for currently loaded level (in case
										//  the user changes the cvar while the level is
										//  running, this reflects the level actually in use)

extern bool	bIsDedicated;

// chase
extern	cvar_t	chase_active;

void TraceLine(vec3_t start,vec3_t end,vec3_t impact);

void Chase_Init(void);
void Chase_UpdateForDrawing(void); //johnfitz

#endif //QUAKEDEF_H
