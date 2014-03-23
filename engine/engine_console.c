/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_console.h"

#include <fcntl.h>

// TODO: Rewrite and clean

#define CONSOLE_MAX_GAMEDIRLEN	1000

void Console_Initialize(void)
{
	char temp[CONSOLE_MAX_GAMEDIRLEN+1],*t2 = "log.txt";

	if(strlen(com_gamedir) < (CONSOLE_MAX_GAMEDIRLEN-strlen(t2)))
	{
		sprintf(temp,"%s",t2);

		unlink(temp);
	}

//	Cmd_AddCommand("dir",Console_DirCommand);
}

void Console_ErrorMessage(bool bCrash,const char *ccFile,char *reason)
{
	if(bCrash)
		Sys_Error("Failed to load %s\nReason: %s",ccFile,reason);
	else
		Con_Error("Failed to load %s\nReason: %s",ccFile,reason);
}

void Console_WriteToLog(char *file,char *fmt,...)
{
	va_list		    argptr;
	static  char	scData[1024];
	int			    fd,iData;

	va_start(argptr, fmt);
	vsprintf(scData, fmt, argptr);
	va_end(argptr);

    iData = strlen(scData);

	fd = open(file,O_WRONLY|O_CREAT|O_APPEND,0666);
	if(write(fd,scData,iData) != iData)
        Sys_Error("Failed to write to log!\n");

	close(fd);
}
