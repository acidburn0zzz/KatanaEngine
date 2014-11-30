/*  Copyright (C) 1996-2001 Id Software, Inc.
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

#include "common/SCRIPLIB.H"

#include "engine_console.h"

#include "platform_filesystem.h"

void Cmd_ForwardToServer(void);

#define	MAX_ALIAS_NAME	32

/*	Sub-commands.
*/
#define COMMAND_USER			"$user"			// Replaces "$user" with current username.

 //johnfitz -- mirrored in common.c

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;

cmdalias_t	*cmd_alias;

int trashtest,*trashspot;

bool bCmdWait;

//=============================================================================

/*	Causes execution of the remainder of the command buffer to be delayed until
	next frame.  This allows commands like:
	bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
*/
void Cmd_Wait_f(void)
{
	bCmdWait = true;
}

/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

sizebuf_t	cmd_text;

void Cbuf_Init (void)
{
	SZ_Alloc(&cmd_text,8192);		// space for commands and script files
}

/*	Adds command text at the end of the buffer
*/
void Cbuf_AddText (char *text)
{
	int		l;

	l = strlen(text);

	if (cmd_text.cursize + l >= cmd_text.maxsize)
	{
		Con_Printf ("Cbuf_AddText: overflow\n");
		return;
	}

	SZ_Write(&cmd_text,text,strlen(text));
}


/*	Adds command text immediately after the current command
	Adds a \n to the text
	FIXME: actually change the command buffer to do less copying
*/
void Cbuf_InsertText (char *text)
{
	char	*temp = NULL;
	int		templen;

	// Copy off any commands still remaining in the exec buffer
	templen = cmd_text.cursize;
	if (templen)
	{
		temp = (char*)Z_Malloc (templen);
		Q_memcpy (temp, cmd_text.data, templen);
		SZ_Clear (&cmd_text);
	}

	// Add the entire text of the file
	Cbuf_AddText (text);

	// Add the copied off data
	if (templen)
	{
		SZ_Write (&cmd_text, temp, templen);
		Z_Free (temp);
	}
}

void Cbuf_Execute (void)
{
	int		i,quotes;
	char	*text,line[1024];

	while(cmd_text.cursize)
	{
		// find a \n or ; line break
		text = (char*)cmd_text.data;

		quotes = 0;
		for(i = 0; i < cmd_text.cursize; i++)
		{
			if(text[i] == '"')
				quotes++;
			if( !(quotes&1) &&  text[i] == ';')
				break;	// Don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}

		memcpy(line,text,i);
		line[i] = 0;

		/*	Delete the text from the command buffer and move remaining commands down
			this is necessary because commands (exec, alias) can insert data at the
			beginning of the text buffer.												*/
		if (i == cmd_text.cursize)
			cmd_text.cursize = 0;
		else
		{
			i++;
			cmd_text.cursize -= i;
			Q_memcpy(text,text+i,cmd_text.cursize);
		}

		// Execute the command line
		Cmd_ExecuteString(line,src_command);

		if(bCmdWait)
		{
			/*	Skip out while text still remains in buffer, leaving it
				for next frame
			*/
			bCmdWait = false;
			break;
		}
	}
}

/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/

/*	Adds command line parameters as script statements
	Commands lead with a +, and continue until a - or another +
	quake +prog jctest.qp +cmd amlev1
	quake -nosound +cmd amlev1
*/
void Cmd_StuffCmds_f(void)
{
	char	cCommands[CMDLINE_LENGTH];
	int		i, iLength = 0, iCommands = 0;
	bool	bIsPlus		= false;

	for(i = 1; i < com_argc; i++)
	{
		if(com_argv[i][0] == '+')
		{
			int j;

			if((iLength > 1) && cCommands[iLength-1] == ' ')
				cCommands[iLength++] = ';';

			for(j = 1; j < com_argv[i][j]; j++)
				cCommands[iLength++] = com_argv[i][j];

			// [1/1/2014] Add a space :) ~hogsy
			cCommands[iLength++] = ' ';

			iCommands++;

			bIsPlus = true;
		}
		else if(com_argv[i][0] == '-')
			bIsPlus = false;
		else if(bIsPlus)
		{
			int j;

			for(j = 0; j < com_argv[i][j]; j++)
				cCommands[iLength++] = com_argv[i][j];

			// [1/1/2014] Add a space :) ~hogsy
			cCommands[iLength++] = ' ';
		}
	}

	// Don't, unless we have some commands to actually send over.
	if (iCommands > 0)
		Cbuf_InsertText(cCommands);
}

// [3/5/2012] Revised ~hogsy
void Cmd_Exec_f(void)
{
	char	*f;
	int		mark;

	if(Cmd_Argc() != 2)
	{
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}

	mark = Hunk_LowMark();

	f = (char*)COM_LoadHunkFile(Cmd_Argv(1));
	if(!f)
	{
		Con_Warning("Couldn't exec %s\n",Cmd_Argv(1));
		return;
	}

	Con_Printf("Execing %s\n",Cmd_Argv(1));

	Cbuf_InsertText(f);
	Hunk_FreeToLowMark(mark);
}

/*	Just prints the rest of the line to the console
*/
void Cmd_Echo_f (void)
{
	int		i;

	for(i = 1; i < Cmd_Argc(); i++)
		Con_Printf("%s ",Cmd_Argv(i));

	Con_Printf("\n");
}

char *CopyString (char *in)
{
	char *out;

	out = (char*)Z_Malloc (strlen(in)+1);
	strcpy (out, in);
	return out;
}

/*	Creates a new command that executes a command string (possibly ; seperated)
*/
void Cmd_Alias_f (void)
{
	cmdalias_t	*a;
	char		cmd[1024];
	int			i, c;
	char		*s;

	switch (Cmd_Argc())
	{
	case 1: //list all aliases
		for (a = cmd_alias, i = 0; a; a=a->next, i++)
			Con_SafePrintf ("   %s: %s", a->name, a->value);
		if (i)
			Con_SafePrintf ("%i alias command(s)\n", i);
		else
			Con_SafePrintf ("no alias commands found\n");
		break;
	case 2: //output current alias string
		for (a = cmd_alias ; a ; a=a->next)
			if (!strcmp(Cmd_Argv(1), a->name))
				Con_Printf ("   %s: %s", a->name, a->value);
		break;
	default: //set alias string
		s = Cmd_Argv(1);
		if (strlen(s) >= MAX_ALIAS_NAME)
		{
			Con_Printf ("Alias name is too long\n");
			return;
		}

		// if the alias allready exists, reuse it
		for (a = cmd_alias ; a ; a=a->next)
			if (!strcmp(s, a->name))
			{
				Z_Free (a->value);
				break;
			}

		if (!a)
		{
			a = (cmdalias_t*)Z_Malloc (sizeof(cmdalias_t));
			a->next = cmd_alias;
			cmd_alias = a;
		}
		strcpy (a->name, s);

		// copy the rest of the command line
		cmd[0] = 0;		// start out with a null string
		c = Cmd_Argc();
		for (i=2 ; i< c ; i++)
		{
			strcat (cmd, Cmd_Argv(i));
			if (i != c)
				strcat (cmd, " ");
		}
		strcat (cmd, "\n");

		a->value = CopyString (cmd);
		break;
	}
}

void Cmd_Unalias_f (void)
{
	cmdalias_t	*a, *prev;

	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("unalias <name> : delete alias\n");
		break;
	case 2:
		for (prev = a = cmd_alias; a; a = a->next)
		{
			if (!strcmp(Cmd_Argv(1), a->name))
			{
				prev->next = a->next;
				Z_Free (a->value);
				Z_Free (a);
				prev = a;
				return;
			}
			prev = a;
		}
		break;
	}
}

void Cmd_Unaliasall_f (void)
{
	cmdalias_t	*blah;

	while (cmd_alias)
	{
		blah = cmd_alias->next;
		Z_Free(cmd_alias->value);
		Z_Free(cmd_alias);
		cmd_alias = blah;
	}
}

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
} cmd_function_t;

#define	MAX_ARGS		80

static	int			cmd_argc;
static	char		*cmd_argv[MAX_ARGS];
static	char		*cmd_null_string = "";
static	char		*cmd_args = NULL;

cmd_source_t	cmd_source;

//johnfitz -- better tab completion
cmd_function_t	*cmd_functions;		// possible commands to execute
//johnfitz

void Cmd_List_f (void)
{
	cmd_function_t	*cmd;
	char 			*partial;
	int				len, count;

	if (Cmd_Argc() > 1)
	{
		partial = Cmd_Argv (1);
		len = Q_strlen(partial);
	}
	else
	{
		partial = NULL;
		len = 0;
	}

	count=0;
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (partial && Q_strncmp (partial,cmd->name, len))
			continue;
		Con_SafePrintf ("   %s\n", cmd->name);
		count++;
	}

	Con_SafePrintf ("%i commands", count);
	if (partial)
		Con_SafePrintf (" beginning with \"%s\"", partial);
	Con_SafePrintf ("\n");
}

void Cmd_Init(void)
{
	Cmd_AddCommand("cmds", Cmd_List_f); //johnfitz
	Cmd_AddCommand("unalias", Cmd_Unalias_f); //johnfitz
	Cmd_AddCommand("unaliasall", Cmd_Unaliasall_f); //johnfitz
	Cmd_AddCommand("stuffcmds",Cmd_StuffCmds_f);
	Cmd_AddCommand("exec",Cmd_Exec_f);
	Cmd_AddCommand("echo",Cmd_Echo_f);
	Cmd_AddCommand("alias",Cmd_Alias_f);
	Cmd_AddCommand("cmd", Cmd_ForwardToServer);
	Cmd_AddCommand("wait",Cmd_Wait_f);
}

int	Cmd_Argc(void)
{
	return cmd_argc;
}

char *Cmd_Argv(int arg)
{
	if(arg >= cmd_argc )
		return cmd_null_string;

	return cmd_argv[arg];
}

char *Cmd_Args (void)
{
	return cmd_args;
}

/*	Parses the given string into command line tokens.
*/
void Cmd_TokenizeString (char *text)
{
	int		i;

// clear the args from the last string
	for (i=0 ; i<cmd_argc ; i++)
		Z_Free (cmd_argv[i]);

	cmd_argc = 0;
	cmd_args = NULL;

	for(;;)
	{
		// skip whitespace up to a /n
		while(*text && *text <= ' ' && *text != '\n')
		{
			text++;
		}

		if (*text == '\n')
		{
			// a newline seperates commands in the buffer
			text++;
			break;
		}

		if(!*text)
			return;

		if (cmd_argc == 1)
			 cmd_args = text;

		text = COM_Parse(text);
		if (!text)
			return;

		if (cmd_argc < MAX_ARGS)
		{
			cmd_argv[cmd_argc] = (char*)Z_Malloc (Q_strlen(com_token)+1);
			strcpy(cmd_argv[cmd_argc], com_token);
			cmd_argc++;
		}
	}
}

void Cmd_AddCommand (char *cmd_name, xcommand_t function)
{
	cmd_function_t	*cmd;
	cmd_function_t	*cursor,*prev; //johnfitz -- sorted list insert

	if(bHostInitialized)	// because hunk allocation would get stomped
		Sys_Error("Cmd_AddCommand after host_initialized");

	// Fail if the command is a variable name
	if (Cvar_VariableString(cmd_name)[0])
	{
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}

	// Fail if the command already exists
	for(cmd=cmd_functions ; cmd ; cmd=cmd->next)
		if(!strcmp (cmd_name, cmd->name))
		{
			Con_Printf ("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}

	cmd = (cmd_function_t*)Hunk_Alloc (sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;

	//johnfitz -- insert each entry in alphabetical order
	if (cmd_functions == NULL || strcmp(cmd->name, cmd_functions->name) < 0) //insert at front
	{
		cmd->next = cmd_functions;
		cmd_functions = cmd;
	}
	else //insert later
	{
		prev = cmd_functions;
		cursor = cmd_functions->next;
		while ((cursor != NULL) && (strcmp(cmd->name, cursor->name) > 0))
		{
			prev = cursor;
			cursor = cursor->next;
		}
		cmd->next = prev->next;
		prev->next = cmd;
	}
	//johnfitz
}

bool Cmd_Exists (char *cmd_name)
{
	cmd_function_t	*cmd;

	for(cmd = cmd_functions; cmd; cmd = cmd->next)
		if(!strcmp(cmd_name,cmd->name))
			return true;

	return false;
}

char *Cmd_CompleteCommand (char *partial)
{
	cmd_function_t	*cmd;
	int				iLength;

	iLength = strlen(partial);
	if(!iLength)
		return NULL;

	// Check functions
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
		if (!Q_strncmp (partial,cmd->name, iLength))
			return cmd->name;

	return NULL;
}

/*  Temporary duplicate of Script_UpdateString!
*/
char *Cmd_UpdateString(char *cString,const char *cFind,const char *cReplace)
{
	static char scBuffer[4096];
	char		*p;

	p = strstr(cString,cFind);
	if(!p)
		return cString;

	if(strlen(cFind) >= strlen(cReplace))
	{
		strncpy(scBuffer,cString,p-cString);

		scBuffer[p-cString]	= '\0';
	}
	else
	{
		strncpy(scBuffer,cString,p-cString+strlen(cFind));

		scBuffer[p-cString+strlen(cFind)] = '\0';
	}

	sprintf(scBuffer+(p-cString),"%s%s",cReplace,p+strlen(cFind));

	return scBuffer;
}

/*	A complete command line has been parsed, so try to execute it
	FIXME: lookupnoadd the token to speed search?
*/
void Cmd_ExecuteString(char *text,cmd_source_t src)
{
	cmd_function_t	*cmd;
	cmdalias_t		*a;

	// [5/8/2012] Check if we're trying to use a modifier ~hogsy
	if(strstr(text,COMMAND_USER))
		text = Cmd_UpdateString(text,COMMAND_USER,Global.cLocalName);

	cmd_source = src;
	Cmd_TokenizeString(text);

	// Execute the command line
	if(!Cmd_Argc())
		return;		// No tokens

	// Check functions
	for(cmd=cmd_functions ; cmd ; cmd=cmd->next)
		if(!Q_strcasecmp(cmd_argv[0],cmd->name))
		{
			cmd->function ();
			return;
		}

	// Check alias
	for(a = cmd_alias; a; a = a->next)
		if(!Q_strcasecmp(cmd_argv[0], a->name))
		{
			Cbuf_InsertText(a->value);
			return;
		}

	// Check cvars
	if(!Cvar_Command())
		Con_Printf ("Unknown command \"%s\"\n", Cmd_Argv(0));
}

/*	Sends the entire command line over to the server
*/
void Cmd_ForwardToServer (void)
{
	if(cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}
	else if(cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte(&cls.message,clc_stringcmd);

	if(Q_strcasecmp(Cmd_Argv(0),"cmd") != 0)
	{
		SZ_Print(&cls.message,Cmd_Argv(0));
		SZ_Print(&cls.message," ");
	}

	if (Cmd_Argc() > 1)
		SZ_Print (&cls.message,Cmd_Args());
	else
		SZ_Print (&cls.message,"\n");
}

/*	Returns the position (1 to argc-1) in the command's argument list
	where the given parameter apears, or 0 if not present
*/
int Cmd_CheckParm (char *parm)
{
	int i;

	if (!parm)
		Sys_Error ("Cmd_CheckParm: NULL");

	for (i = 1; i < Cmd_Argc (); i++)
		if(!Q_strcasecmp (parm, Cmd_Argv (i)))
			return i;

	return 0;
}
