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
	Old platform specific loop shit. Needs to be rewritten.
*/

#include <SDL.h>

#include "platform/include/platform_window.h"

#ifdef _WIN32
#include "winquake.h"
#include "conproc.h"
#endif
#include "errno.h"
#include "resource.h"

#include "engine_main.h"
#include "KatEditor.h"      // [7/10/2013] TODO: Rename to engine_editor ~hogsy
#include "engine_console.h"
#include "engine_video.h"

#define MAXIMUM_MEMORY		0x8000000	// 128MB

#define CONSOLE_ERROR_TIMEOUT	60.0	// # of seconds to wait on Sys_Error running
										//  dedicated before exiting
#define PAUSE_SLEEP		50				// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP	20				// sleep time when not focus

int	starttime;

static double	curtime = 0.0;
static double	lastcurtime = 0.0;
bool			bIsDedicated;
static bool		sc_return_on_enter = false;
#ifdef __WIN32__
HANDLE			hinput, houtput;
#endif

void Sys_InitFloatTime (void);

volatile int sys_checksum;

#ifdef __WIN32__
void Sys_PageIn (void *ptr, int size)
{
	byte	*x;
	int		m, n;

	// touch all the memory to make sure it's there. The 16-page skip is to
	// keep Win 95 from thinking we're trying to page ourselves in (we are
	// doing that, of course, but there's no reason we shouldn't)
	x = (byte *)ptr;

	for (n=0 ; n<4 ; n++)
		for (m=0 ; m<(size - 16 * 0x1000) ; m += 4)
		{
			sys_checksum += *(int *)&x[m];
			sys_checksum += *(int *)&x[m + 16 * 0x1000];
		}
}
#endif

/*
===============================================================================

FILE IO

===============================================================================
*/

#define	MAX_HANDLES		100 //johnfitz -- was 10
FILE	*sys_handles[MAX_HANDLES];

int	findhandle (void)
{
	int		i;

	for(i = 1; i < MAX_HANDLES; i++)
		if(!sys_handles[i])
			return i;

	return -1;
}

int filelength (FILE *f)
{
	int	pos,end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE	*f;
	int		i, retval;

	i = findhandle ();
	if(i == -1)
		Sys_Error("Out of handles!");

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		retval = -1;
	}
	else
	{
		sys_handles[i] = f;
		*hndl = i;
		retval = filelength(f);
	}

	return retval;
}

int Sys_FileOpenWrite (char *path)
{
	FILE	*f;
	int		i;

	i = findhandle();
	if(i == -1)
		Sys_Error("Out of handles!");

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));

	sys_handles[i] = f;

	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead(int handle, void *dest, int count)
{
	return fread(dest,1,count,sys_handles[handle]);
}

int Sys_FileWrite(int handle, void *data, int count)
{
	return fwrite(data,1,count,sys_handles[handle]);
}

int	Sys_FileTime (char *path)
{
	FILE	*f;

	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);

		return 1;
	}
	else
		return -1;
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_Init(void)
{
#if 0
	LARGE_INTEGER	PerformanceFreq;
	unsigned int	lowpart, highpart;

	if (!QueryPerformanceFrequency (&PerformanceFreq))
		Sys_Error ("No hardware timer available");

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	lowpart		= (unsigned int)PerformanceFreq.LowPart;
	highpart	= (unsigned int)PerformanceFreq.HighPart;
	lowshift	= 0;

	while (highpart || (lowpart > 2000000.0))
	{
		lowshift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}

	pfreq = 1.0 / (double)lowpart;
#endif

	Sys_InitFloatTime();
}

void Sys_Error(char *error, ...)
{
	va_list		argptr;
	char		text[1024];
	double		starttime;
	static int	in_sys_error0 = 0,
				in_sys_error1 = 0,
				in_sys_error2 = 0,
				in_sys_error3 = 0;

	if(!in_sys_error3)
		in_sys_error3 = 1;

	va_start(argptr,error);
	vsprintf(text,error,argptr);
	va_end(argptr);

	Console_WriteToLog("log.txt","Error: %s",text);

	if(bIsDedicated)
	{
		//sprintf(text2,"ERROR: %s\n",text);

		starttime = Sys_FloatTime ();
		sc_return_on_enter = true;	// so Enter will get us out of here

		while(!Sys_ConsoleInput() && ((Sys_FloatTime()-starttime) < CONSOLE_ERROR_TIMEOUT))
		{
		}
	}
	else
	{
		// switch to windowed so the message box is visible, unless we already
		// tried that and failed
		if (!in_sys_error0)
		{
			in_sys_error0 = 1;

			// [28/10/2013] Changed to platform independent function ~hogsy
			gWindow_MessageBox("Fatal Error",text);
		}
		else
			// [28/10/2013] Changed to platform independent function ~hogsy
			gWindow_MessageBox("Double Fatal Error",text);
	}

	if (!in_sys_error1)
	{
		in_sys_error1 = 1;

		Host_Shutdown();
	}

	// Shut down QHOST hooks if necessary
	if (!in_sys_error2)
	{
		in_sys_error2 = 1;

#ifdef __WIN32__
		DeinitConProc();
#endif
	}

	exit(1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list	argptr;
	char	text[1024];

	if(bIsDedicated)
	{
		va_start(argptr,fmt);
		vsprintf(text,fmt,argptr);
		va_end(argptr);

		Console_WriteToLog("log.txt","Error: %s",text);
	}
}

void Sys_Quit(void)
{
	Host_Shutdown();

#ifdef __WIN32__
	if(bIsDedicated)
		FreeConsole();

	// shut down QHOST hooks if necessary
	DeinitConProc();
#endif

	exit(0);
}

double Sys_FloatTime(void)
{
#ifndef GIPL
	// [19/7/2013] Copied over from QuakeSpasm ~hogsy
	return SDL_GetTicks()/1000.0;
#endif
}

void Sys_InitFloatTime (void)
{
	int		j;

	Sys_FloatTime ();

	j = COM_CheckParm("-starttime");

	if (j)
		curtime = (double)(Q_atof(com_argv[j+1]));
	else
		curtime = 0.0;

	lastcurtime = curtime;
}

char *Sys_ConsoleInput (void)
{
    // [17/4/2014] Moved up here; don't worry about console input on Linux either, unless we're dedicated! ~hogsy
    if(!bIsDedicated)
		return NULL;

#ifdef __WIN32__
    {
	static char	    text[256];
	static int		len;
	INPUT_RECORD	recs[1024];
	int		        dummy,
					ch,numread,numevents;

	for(;;)
	{
		if (!GetNumberOfConsoleInputEvents(hinput,(LPDWORD)&numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if(!ReadConsoleInput(hinput,recs,1,(LPDWORD)&numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile(houtput,"\r\n",2,(LPDWORD)&dummy,NULL);

						if (len)
						{
							text[len] = 0;
							len = 0;
							return text;
						}
						else if(sc_return_on_enter)
						{
							// special case to allow exiting from the error handler on Enter
							text[0] = '\r';
							len = 0;
							return text;
						}

						break;
					case '\b':
						WriteFile(houtput,"\b \b",3,(LPDWORD)&dummy,NULL);
						if (len)
							len--;
						break;
					default:
						if (ch >= ' ')
						{
							WriteFile(houtput,&ch,1,(LPDWORD)&dummy,NULL);
							text[len] = ch;
							len = (len + 1) & 0xff;
						}

						break;

				}
			}
		}
	}
	}
#else
    {
	static char	con_text[256];
	static int	textlen;
	char		c;
	fd_set		set;
	struct timeval	timeout;

	FD_ZERO (&set);
	FD_SET (0, &set);	// stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	while(select(1,&set,NULL,NULL,&timeout))
	{
		if(!read(0,&c,1))
		{
			Con_Warning("Failed to read console input!\n");
			// [25/11/2013] Is this safe? Or even okay? Bleh ~hogsy
			return NULL;
		}

		if (c == '\n' || c == '\r')
		{
			con_text[textlen] = '\0';
			textlen = 0;
			return con_text;
		}
		else if (c == 8)
		{
			if (textlen)
			{
				textlen--;
				con_text[textlen] = '\0';
			}
			continue;
		}
		con_text[textlen] = c;
		textlen++;
		if (textlen < (int) sizeof(con_text))
			con_text[textlen] = '\0';
		else
		{
		// buffer is full
			textlen = 0;
			con_text[0] = '\0';
			Sys_Printf("\nConsole input too long!\n");
			break;
		}
	}
	}
#endif

	return NULL;
}

bool System_Main(int iArgumentCount,char *cArguments[])
{
	EngineParameters_t	epParameters;
	double				time, oldtime, newtime;
	int					t;

#ifdef _WIN32
	{
		MEMORYSTATUS    lpBuffer;

		lpBuffer.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus (&lpBuffer);

		// take the greater of all the available memory or half the total memory,
		// but at least 8 Mb and no more than 16 Mb, unless they explicitly
		// request otherwise
		epParameters.memsize = lpBuffer.dwAvailPhys;
		if(epParameters.memsize < MINIMUM_MEMORY)
			epParameters.memsize = MINIMUM_MEMORY;

		if((unsigned)epParameters.memsize < (lpBuffer.dwTotalPhys >> 1))
			epParameters.memsize = lpBuffer.dwTotalPhys >> 1;

		if(epParameters.memsize > MAXIMUM_MEMORY)
			epParameters.memsize = MAXIMUM_MEMORY;
	}
#else
	epParameters.memsize = MAXIMUM_MEMORY;
#endif

	epParameters.basedir	= ".";
	epParameters.cachedir	= NULL;
	epParameters.argc		= iArgumentCount;
	epParameters.argv		= cArguments;

	COM_InitArgv(epParameters.argc,epParameters.argv);

	epParameters.argc = com_argc;
	epParameters.argv = com_argv;

	bIsDedicated = (COM_CheckParm("-dedicated") != 0);

	if(COM_CheckParm("-heapsize"))
	{
		t = COM_CheckParm("-heapsize")+1;
		if (t < com_argc)
			epParameters.memsize = atoi(com_argv[t])*1024;
	}

	epParameters.membase = malloc(epParameters.memsize);
	if(!epParameters.membase)
		Sys_Error("Not enough memory free (%imb)!\nCheck disk space.\n",epParameters.memsize);

#ifdef _WIN32
	Sys_PageIn(epParameters.membase,epParameters.memsize);

	if(bIsDedicated)
	{
		static HANDLE   heventChild,
						heventParent,
						hFile;

		hinput	= GetStdHandle(STD_INPUT_HANDLE);
		houtput = GetStdHandle(STD_OUTPUT_HANDLE);

		// give QHOST a chance to hook into the console
		if ((t = COM_CheckParm ("-HFILE")) > 0)
			if (t < com_argc)
				hFile = (HANDLE)Q_atoi (com_argv[t+1]);

		if ((t = COM_CheckParm ("-HPARENT")) > 0)
			if (t < com_argc)
				heventParent = (HANDLE)Q_atoi(com_argv[t+1]);

		if ((t = COM_CheckParm ("-HCHILD")) > 0)
			if (t < com_argc)
				heventChild = (HANDLE)Q_atoi(com_argv[t+1]);

		InitConProc(hFile,heventParent,heventChild);
	}
#endif

	Sys_Init();

#ifndef KATANA_AUDIO_OPENAL
	// Because sound is off until we become active
	S_BlockSound();
#endif

	Sys_Printf("Host_Initialize\n");

	Host_Initialize(&epParameters);

	oldtime = Sys_FloatTime();

	for(;;)
	{
		if(bIsDedicated)
		{
			newtime = Sys_FloatTime ();
			time = newtime - oldtime;

			while(time < sys_ticrate.value)
			{
				SDL_Delay(1);

				newtime = Sys_FloatTime ();
				time = newtime-oldtime;
			}
		}
		else
		{
			if(!Video.bActive && cl.maxclients == 1)
			{
				Video.bSkipUpdate = true;

				SDL_Delay(16);
			}
			else
				Video.bSkipUpdate = false;

			newtime = Sys_FloatTime();
			time = newtime - oldtime;
		}

		Host_Frame(time);
		oldtime = newtime;
	}

	return true;
}
