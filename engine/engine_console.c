/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_console.h"

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
	FILE			*fLog;
	va_list		    argptr;
	static  char	scData[1024];
	char			cPath[MAX_OSPATH];
	unsigned int	iData;
	
	sprintf(cPath,PATH_LOGS"/%s",ccFile);

	COM_DefaultExtension(cPath,".txt");

	va_start(argptr,fmt);
	vsprintf(scData,fmt,argptr);
	va_end(argptr);

    iData = strlen(scData);

	fLog = fopen(cPath,"a");
	if(fwrite(scData,sizeof(char),iData,fLog) != iData)
        Sys_Error("Failed to write to log! (%s)\n",ccFile);

	fclose(fLog);
}

void Console_ClearLog(const char *ccFile)
{
	char cPath[MAX_OSPATH];

	sprintf(cPath,PATH_LOGS"/%s",ccFile);

	COM_DefaultExtension(cPath,".txt");

	unlink(cPath);
}
