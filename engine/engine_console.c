/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_console.h"

#include <fcntl.h>

// TODO: Rewrite and clean

#define CONSOLE_MAX_GAMEDIRLEN	1000

void Console_Initialize(void)
{
	Console_ClearLog("log.txt");
}

void Console_ErrorMessage(bool bCrash,const char *ccFile,char *reason)
{
	if(bCrash)
		Sys_Error("Failed to load %s\nReason: %s",ccFile,reason);
	else
		Con_Error("Failed to load %s\nReason: %s",ccFile,reason);
}

void Console_WriteToLog(const char *ccFile,char *fmt,...)
{
	va_list		    argptr;
	static  char	scData[1024];
	char			cPath[MAX_OSPATH];
	int			    fd,iData;

	sprintf(cPath,PATH_LOGS"/%s",ccFile);

	COM_DefaultExtension(cPath,".txt");

	va_start(argptr,fmt);
	vsprintf(scData,fmt,argptr);
	va_end(argptr);

    iData = strlen(scData);

	fd = open(cPath,O_WRONLY|O_CREAT|O_APPEND,0666);
	if(write(fd,scData,iData) != iData)
        Sys_Error("Failed to write to log!\n");

	close(fd);
}

void Console_ClearLog(const char *ccFile)
{
	char	cPath[MAX_OSPATH];

	sprintf(cPath,PATH_LOGS"/%s",ccFile);

	COM_DefaultExtension(cPath,".txt");

	unlink(cPath);
}
