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
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <fcntl.h>
#include "quakedef.h"
#include <stdio.h>

#include "engine_console.h"
#include "KatGL.h"

#include "engine_input.h"
#include "engine_video.h"

int 		con_linewidth;

float		con_cursorspeed = 4;

#define		CON_TEXTSIZE 65536 //johnfitz -- new default size
#define		CON_MINSIZE  16384 //johnfitz -- old default, now the minimum size

int			con_buffersize; //johnfitz -- user can now override default

bool 	con_forcedup;		// because no entities to refresh

int			con_totallines;		// total lines in console scrollback
int			con_backscroll;		// lines up from bottom to display
int			con_current;		// where next message will be printed
int			con_x;				// offset in current line for next print

char		*con_text=0;

cvar_t		con_notifytime = {"con_notifytime","3"};		//seconds
cvar_t		con_logcenterprint = {"con_logcenterprint", "1"}; //johnfitz

char		con_lastcenterstring[1024]; //johnfitz

#define	NUM_CON_TIMES 4
float		con_times[NUM_CON_TIMES];	// realtime time the line was generated
										// for transparent notify lines
int			con_vislines;

#define		MAXCMDLINE	256
extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;

bool	bConsoleInitialized;

extern void M_Menu_Main_f (void);

/*	johnfitz -- returns a bar of the desired length, but never wider than the console

	includes a newline, unless len >= con_linewidth.
*/
char *Con_Quakebar (int len)
{
	static char bar[42];
	int i;

	len = Math_Min(len,sizeof(bar)-2);
	len = Math_Min(len,con_linewidth);

	bar[0] = '\35';
	for (i = 1; i < len - 1; i++)
		bar[i] = '\36';
	bar[len-1] = '\37';

	if (len < con_linewidth)
	{
		bar[len] = '\n';
		bar[len+1] = 0;
	}
	else
		bar[len] = 0;

	return bar;
}

void Con_ToggleConsole_f (void)
{
	extern int history_line; //johnfitz

	if (key_dest == key_console)
	{
		if(cls.state == ca_connected)
		{
			key_dest				= key_game;
			key_lines[edit_line][1] = 0;	// clear any typing
			key_linepos				= 1;
			con_backscroll			= 0; //johnfitz -- toggleconsole should return you to the bottom of the scrollback
			history_line			= edit_line; //johnfitz -- it should also return you to the bottom of the command history

			Input_ActivateMouse();
		}
		else
			M_Menu_Main_f ();
	}
	else
	{
		key_dest = key_console;

		Input_DeactivateMouse();
	}

	SCR_EndLoadingPlaque();

	memset(con_times,0,sizeof(con_times));
}

void Con_Clear_f (void)
{
	if (con_text)
		Q_memset (con_text, ' ', con_buffersize); //johnfitz -- con_buffersize replaces CON_TEXTSIZE
	con_backscroll = 0; //johnfitz -- if console is empty, being scrolled up is confusing
}

void Con_ClearNotify (void)
{
	int		i;

	for (i=0 ; i<NUM_CON_TIMES ; i++)
		con_times[i] = 0;
}

extern bool team_message;

void Con_MessageMode_f (void)
{
	key_dest = key_message;
	team_message = false;
}

void Con_MessageMode2_f (void)
{
	key_dest = key_message;
	team_message = true;
}

/*	If the line width has changed, reformat the buffer.
*/
void Con_CheckResize(void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	*tbuf; //johnfitz -- tbuf no longer a static array
	int 	mark; //johnfitz

	width = (vid.conwidth >> 3)-2; //johnfitz -- use vid.conwidth instead of vid.width
	if(width == con_linewidth)
		return;

	oldwidth 		= con_linewidth;
	con_linewidth 	= width;
	oldtotallines 	= con_totallines;
	con_totallines 	= con_buffersize / con_linewidth; //johnfitz -- con_buffersize replaces CON_TEXTSIZE
	numlines 		= oldtotallines;

	if(con_totallines < numlines)
		numlines = con_totallines;

	numchars = oldwidth;
	if(con_linewidth < numchars)
		numchars = con_linewidth;

	mark = Hunk_LowMark(); //johnfitz
	tbuf = (char*)Hunk_Alloc(con_buffersize); //johnfitz

	memcpy(tbuf, con_text, con_buffersize);//johnfitz -- con_buffersize replaces CON_TEXTSIZE
	memset(con_text, ' ', con_buffersize);//johnfitz -- con_buffersize replaces CON_TEXTSIZE

	for(i = 0; i < numlines; i++)
		for (j = 0; j < numchars; j++)
		{
			con_text[(con_totallines-1-i)*con_linewidth+j] =
					tbuf[((con_current-i+oldtotallines)%oldtotallines)*oldwidth+j];
		}

	Hunk_FreeToLowMark (mark); //johnfitz

	Con_ClearNotify ();

	con_backscroll = 0;
	con_current = con_totallines - 1;
}

void Con_Init (void)
{
	//johnfitz -- user settable console buffer size
	if (COM_CheckParm("-consize"))
		con_buffersize = Math_Max(CON_MINSIZE,Q_atoi(com_argv[COM_CheckParm("-consize")+1])*1024);
	else
		con_buffersize = CON_TEXTSIZE;
	//johnfitz

	con_text = (char*)Hunk_AllocName (con_buffersize, "context");//johnfitz -- con_buffersize replaces CON_TEXTSIZE
	Q_memset (con_text, ' ', con_buffersize);//johnfitz -- con_buffersize replaces CON_TEXTSIZE

	//johnfitz -- no need to run Con_CheckResize here
	con_linewidth	= 70;
	con_totallines	= con_buffersize/con_linewidth;//johnfitz -- con_buffersize replaces CON_TEXTSIZE
	con_backscroll	= 0;
	con_current		= con_totallines-1;
	//johnfitz

	// register our commands
	Cvar_RegisterVariable (&con_notifytime, NULL);
	Cvar_RegisterVariable (&con_logcenterprint, NULL); //johnfitz

	Cmd_AddCommand("toggleconsole",Con_ToggleConsole_f);
	Cmd_AddCommand("messagemode",Con_MessageMode_f);
	Cmd_AddCommand("messagemode2",Con_MessageMode2_f);
	Cmd_AddCommand("clear",Con_Clear_f);

	bConsoleInitialized = true;
}

void Con_Linefeed (void)
{
	//johnfitz -- improved scrolling
	if (con_backscroll)
		con_backscroll++;
	if (con_backscroll > con_totallines - (glheight>>3) - 1)
		con_backscroll = con_totallines - (glheight>>3) - 1;
	//johnfitz

	con_x = 0;
	con_current++;
	Q_memset(&con_text[(con_current%con_totallines)*con_linewidth], ' ', con_linewidth);
}

/*	Handles cursor positioning, line wrapping, etc
	All console printing must go through this in order to be logged to disk
	If no console is visible, the notify window will pop up.
*/
void Con_Print (char *txt)
{
	int			mask,y,c,l;
	static int	cr;

	//con_backscroll = 0; //johnfitz -- better console scrolling

	if (txt[0] == 1)
	{
		mask = 128;		// go to colored text
		S_LocalSound("misc/talk.wav");

		// Play talk wav
		txt++;
	}
	else if (txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
		mask = 0;


	while ( *txt )
	{
		c = *txt;

		// Count word length
		for (l=0 ; l< con_linewidth ; l++)
			if ( txt[l] <= ' ')
				break;

		// Word wrap
		if (l != con_linewidth && (con_x + l > con_linewidth) )
			con_x = 0;

		txt++;

		if(cr)
		{
			con_current--;
			cr = false;
		}

		if(!con_x)
		{
			Con_Linefeed ();

			// Mark time for transparent overlay
			if (con_current >= 0)
				con_times[con_current % NUM_CON_TIMES] = realtime;
		}

		switch (c)
		{
		case '\n':
			con_x = 0;
			break;
		case '\r':
			con_x = 0;
			cr = 1;
			break;
		default:
			// Display character and advance
			y = con_current % con_totallines;
			con_text[y*con_linewidth+con_x] = c | mask;
			con_x++;
			if (con_x >= con_linewidth)
				con_x = 0;
			break;
		}
	}
}

#define	MAXPRINTMSG	4096

/*	Handles cursor positioning, line wrapping, etc
*/

// FIXME: make a buffer size safe vsprintf?
void Con_Printf (char *fmt, ...)
{
	va_list			argptr;
	char			msg[MAXPRINTMSG];
	static	bool	inupdate;

	va_start(argptr,fmt);
	vsprintf(msg,fmt,argptr);
	va_end(argptr);

	// also echo to debugging console
	Sys_Printf ("%s", msg);

	Console_WriteToLog("log.txt","%s",msg);

	// [3/10/2012] Merged check ~hogsy
	if(!bConsoleInitialized || cls.state == ca_dedicated)
		return;

	Con_Print(msg);

	if (cls.signon != SIGNONS && !scr_disabled_for_loading )
		if (!inupdate)
		{
			inupdate = true;
//			SCR_UpdateScreen ();
			inupdate = false;
		}
}

void Con_SPrintf (char *dest, int size, char *fmt, ...)
{
	int		len;
	va_list	argptr;
	char	bigbuffer[MAXPRINTMSG];

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (len >= size)
		Con_Printf("Com_sprintf: overflow of %i in %i\n", len, size);

	strncpy (dest, bigbuffer, size-1);

	Con_Printf (dest);
}

void Con_Warning (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	Con_SafePrintf("\x02Warning: ");
	Con_Printf("%s", msg);
}

void Con_Error(char *fmt,...)
{
	static	bool	bInError = false;
	va_list			argptr;
	char 			msg[MAXPRINTMSG];

	if(bInError)
		Sys_Error("Con_Error: Recursively entered\nCheck log.txt for details.");
	bInError = true;

	va_start(argptr,fmt);
	vsprintf(msg,fmt,argptr);
	va_end(argptr);

	Con_Printf("\nError: %s\n",msg);

	CL_Disconnect_f();

	bInError = false;

	longjmp(host_abortserver,1);
	/*Todo:
	 - Clear everything out*/
}

void Con_DPrintf(char *fmt,...)
{
	va_list	argptr;
	char	msg[MAXPRINTMSG];

	va_start(argptr,fmt);
	vsprintf(msg,fmt,argptr);
	va_end(argptr);

	// Don't confuse non-developers with techie stuff...
	if(!developer.value)
		Console_WriteToLog("log.txt","%s",msg);
	else
		Con_SafePrintf("%s",msg); //johnfitz -- was Con_Printf
}

/*	Okay to call even when the screen can't be updated
*/
void Con_SafePrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[1024];
	int			temp;

	va_start(argptr,fmt);
	vsprintf(msg,fmt,argptr);
	va_end(argptr);

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = TRUE;
	Con_Printf ("%s", msg);
	scr_disabled_for_loading = temp;
}

void Con_CenterPrintf (int linewidth, char *fmt, ...)
{
	va_list	argptr;
	char	msg[MAXPRINTMSG]; //the original message
	char	line[MAXPRINTMSG]; //one line from the message
	char	spaces[21]; //buffer for spaces
	char	*src, *dst;
	int		len, s;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	linewidth = Math_Min (linewidth, con_linewidth);
	for (src = msg; *src; )
	{
		dst = line;
		while (*src && *src != '\n')
			*dst++ = *src++;
		*dst = 0;
		if (*src == '\n')
			src++;

		len = strlen(line);
		if (len < linewidth)
		{
			s = (linewidth-len)/2;
			memset (spaces, ' ', s);
			spaces[s] = 0;
			Con_Printf ("%s%s\n", spaces, line);
		}
		else
			Con_Printf ("%s\n", line);
	}
}

void Con_LogCenterPrint (char *str)
{
	if(!strcmp(str,con_lastcenterstring))
		return; // Ignore duplicates

	if(cl.gametype == GAME_DEATHMATCH && con_logcenterprint.value != 2)
		return; // Don't log in deathmatch

	strcpy(con_lastcenterstring, str);

	if(con_logcenterprint.value)
	{
		Con_Printf(Con_Quakebar(40));
		Con_CenterPrintf(40,"%s\n",str);
		Con_Printf(Con_Quakebar(40));
		Con_ClearNotify();
	}
}

/*
==============================================================================

	TAB COMPLETION

==============================================================================
*/

//johnfitz -- tab completion stuff
//unique defs
char key_tabpartial[MAXCMDLINE];

typedef struct tab_s
{
	char			*name;
	char			*type;
	struct tab_s	*next;
	struct tab_s	*prev;
} tab_t;

tab_t	*tablist;

//defs from elsewhere
extern bool	keydown[256];

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
} cmd_function_t;

extern	cmd_function_t	*cmd_functions;

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;

extern	cmdalias_t	*cmd_alias;

/*	Tablist is a doubly-linked loop, alphabetized by name
*/
void AddToTabList (char *name, char *type)
{
	tab_t	*t,*insert;

	t = (tab_t*)Hunk_Alloc(sizeof(tab_t));
	t->name = name;
	t->type = type;

	if (!tablist) //create list
	{
		tablist = t;
		t->next = t;
		t->prev = t;
	}
	else if (strcmp(name, tablist->name) < 0) //insert at front
	{
		t->next = tablist;
		t->prev = tablist->prev;
		t->next->prev = t;
		t->prev->next = t;
		tablist = t;
	}
	else //insert later
	{
		insert = tablist;
		do
		{
			if (strcmp(name, insert->name) < 0)
				break;
			insert = insert->next;
		} while (insert != tablist);

		t->next = insert;
		t->prev = insert->prev;
		t->next->prev = t;
		t->prev->next = t;
	}
}

void BuildTabList (char *partial)
{
	cmdalias_t		*alias;
	cvar_t			*cvar;
	cmd_function_t	*cmd;
	int				len;

	tablist = NULL;
	len = strlen(partial);

	for (cvar=cConsoleVariables ; cvar ; cvar=cvar->next)
		if (!Q_strncmp (partial, cvar->name, len))
			AddToTabList (cvar->name, "cvar");

	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
		if (!Q_strncmp (partial,cmd->name, len))
			AddToTabList (cmd->name, "command");

	for (alias=cmd_alias ; alias ; alias=alias->next)
		if (!Q_strncmp (partial, alias->name, len))
			AddToTabList (alias->name, "alias");
}

void Con_TabComplete (void)
{
	char		partial[MAXCMDLINE],*match;
	static char	*c;
	tab_t		*t;
	int			mark, i;

// if editline is empty, return
	if (key_lines[edit_line][1] == 0)
		return;

// get partial string (space -> cursor)
	if (!Q_strlen(key_tabpartial)) //first time through, find new insert point. (Otherwise, use previous.)
	{
		//work back from cursor until you find a space, quote, semicolon, or prompt
		c = key_lines[edit_line] + key_linepos - 1; //start one space left of cursor
		while (*c!=' ' && *c!='\"' && *c!=';' && c!=key_lines[edit_line])
			c--;
		c++; //start 1 char after the seperator we just found
	}
	for (i = 0; c + i < key_lines[edit_line] + key_linepos; i++)
		partial[i] = c[i];
	partial[i] = 0;

//if partial is empty, return
	if (partial[0] == 0)
		return;

//trim trailing space becuase it screws up string comparisons
	if (i > 0 && partial[i-1] == ' ')
		partial[i-1] = 0;

// find a match
	mark = Hunk_LowMark();
	if (!Q_strlen(key_tabpartial)) //first time through
	{
		Q_strcpy (key_tabpartial, partial);
		BuildTabList (key_tabpartial);

		if (!tablist)
			return;

		//print list
		t = tablist;
		do
		{
			Con_SafePrintf("   %s (%s)\n", t->name, t->type);
			t = t->next;
		} while (t != tablist);

		//get first match
		match = tablist->name;
	}
	else
	{
		BuildTabList (key_tabpartial);

		if (!tablist)
			return;

		//find current match -- can't save a pointer because the list will be rebuilt each time
		t = tablist;
		do
		{
			if(!strcmp(t->name, partial))
				break;
			t = t->next;
		} while (t != tablist);

		//use prev or next to find next match
		match = keydown[K_SHIFT] ? t->prev->name : t->next->name;
	}
	Hunk_FreeToLowMark(mark); //it's okay to free it here becuase match is a pointer to persistent data

	// Insert new match into edit line
	strcpy(partial,match);								// First copy match string
	strcat(partial,key_lines[edit_line] + key_linepos); // Then add chars after cursor
	strcpy(c,partial);									// Now copy all of this into edit line

	key_linepos = c-key_lines[edit_line]+strlen(match); //set new cursor position

// if cursor is at end of string, let's append a space to make life easier
	if (key_lines[edit_line][key_linepos] == 0)
	{
		key_lines[edit_line][key_linepos] = ' ';
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
	}
}

/*
==============================================================================

DRAWING

==============================================================================
*/

/*	Draws the last few lines of output transparently over the game top
*/
void Con_DrawNotify(void)
{
	int				x,v;
	char			*text;
	int				i;
	float			time;
	extern	char	chat_buffer[];

	GL_SetCanvas(CANVAS_CONSOLE); //johnfitz

	v = vid.conheight; //johnfitz

	for(i = con_current-NUM_CON_TIMES+1; i <= con_current; i++)
	{
		if(i < 0)
			continue;

		time = con_times[i % NUM_CON_TIMES];
		if(time == 0)
			continue;

		time = realtime-time;
		if(time > con_notifytime.value)
			continue;

		text = con_text + (i % con_totallines)*con_linewidth;

		for(x = 0; x < con_linewidth; x++)
			Draw_Character((x+1)<<3,v,text[x]);

		v += 8;

		scr_tileclear_updates = 0; //johnfitz
	}


	if (key_dest == key_message)
	{
		char *say_prompt; //johnfitz

		x = 0;

		//johnfitz -- distinguish say and say_team
		if (team_message)
			say_prompt = "Say (team):";
		else
			say_prompt = "Say (all):";
		//johnfitz

		R_DrawString(8,v,say_prompt); //johnfitz

		while(chat_buffer[x])
		{
			Draw_Character((x+strlen(say_prompt)+2)<<3,v,chat_buffer[x]); //johnfitz
			x++;
		}
		Draw_Character((x+strlen(say_prompt)+2)<<3,v,10+((int)(realtime*con_cursorspeed)&1)); //johnfitz
		v += 8;

		scr_tileclear_updates = 0; //johnfitz
	}
}

/*	johnfitz -- modified to allow insert editing

	The input line scrolls horizontally if typing goes beyond the right edge
*/
void Con_DrawInput (void)
{
	extern double key_blinktime;
	int				i,len;
	char			c[256],*text;

	if(key_dest != key_console && !con_forcedup)
		return;		// Don't draw anything

	text = strcpy(c, key_lines[edit_line]);

	// Pad with one space so we can draw one past the string (in case the cursor is there)
	len = strlen(key_lines[edit_line]);
	text[len] = ' ';
	text[len+1] = 0;

	// Prestep if horizontally scrolling
	if (key_linepos >= con_linewidth)
		text += 1 + key_linepos - con_linewidth;

	// Draw input string
	for(i = 0; i <= (signed)strlen(text)-1; i++) //only write enough letters to go from *text to cursor
		Draw_Character ((i+1)<<3, vid.conheight - 16, text[i]);

	// johnfitz -- new cursor handling
	if(!((int)((realtime-key_blinktime)*con_cursorspeed) & 1))
		Draw_Character((key_linepos+1)<<3,vid.conheight-16,'_');
}

/*	Draws the console with the solid background
	The typing input line at the bottom should only be drawn if typing is allowed
*/
void Con_DrawConsole(int lines,bool drawinput)
{
	int		i,x,y,j,sb,rows;
	char	ver[64],*text;

	if(lines <= 0)
		return;

	con_vislines = lines*vid.conheight/glheight;

	// Draw the background
	Draw_ConsoleBackground();

	GL_SetCanvas(CANVAS_CONSOLE);

	// Draw the buffer text
	rows = (con_vislines+7)/8;
	y = vid.conheight - rows*8;
	rows -= 2; //for input and version lines
	sb = (con_backscroll) ? 2 : 0;

	for (i = con_current - rows + 1 ; i <= con_current - sb ; i++, y+=8)
	{
		j = i - con_backscroll;
		if (j<0)
			j = 0;
		text = con_text + (j % con_totallines)*con_linewidth;

		for (x=0 ; x<con_linewidth ; x++)
			Draw_Character ( (x+1)<<3, y, text[x]);
	}

	// Draw scrollback arrows
	if(con_backscroll)
	{
		y+=8; // blank line
		for (x=0 ; x<con_linewidth ; x+=4)
			Draw_Character ((x+1)<<3, y, '^');
		y+=8;
	}

	// draw the input prompt, user text, and cursor
	if (drawinput)
		Con_DrawInput ();

	// Draw version number in bottom right
	y += 8;
	sprintf(ver,"Katana %i",ENGINE_VERSION_BUILD);
	for(x = 0; x < (signed)strlen(ver); x++)
		Draw_Character((con_linewidth-strlen(ver)+x+2)<<3,y,ver[x]);
}

void Con_NotifyBox(char *text)
{
	double	t1,t2;

	// During startup for sound / cd warnings
	Con_Printf("\n\n%s",Con_Quakebar(40)); //johnfitz
	Con_Printf(text);
	Con_Printf("Press a key.\n");
	Con_Printf(Con_Quakebar(40)); //johnfitz

	key_count 	= -2;		// Wait for a key down and up
	key_dest 	= key_console;

	do
	{
		t1 = System_DoubleTime ();

		Video_Process();
		Input_Process();

		t2 = System_DoubleTime();

		realtime += t2-t1;		// make the cursor blink
	} while(key_count < 0);

	Con_Printf ("\n");
	key_dest = key_game;
	realtime = 0;				// put the cursor back to invisible
}
