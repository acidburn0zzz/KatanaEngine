/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform.h"

/*
	Generic functions for platform, such as
	error handling.
*/

#define	MAX_FUNCTION_LENGTH	64
#define	MAX_ERROR_LENGTH	1024

char	cErrorMessage[MAX_ERROR_LENGTH]		= "null",
		cLastFunction[MAX_FUNCTION_LENGTH]	= "null";

/*	Sets the name of the currently entered function.
*/
void pError_SetFunction(const char *ccFunction,...)
{
	char	cOut[MAX_FUNCTION_LENGTH];
	va_list vlArguments;

	va_start(vlArguments,ccFunction);
	vsprintf(cOut,ccFunction,vlArguments);
	va_end(vlArguments);

	strncpy(cLastFunction,ccFunction,sizeof(ccFunction));
}

void pError_Reset(void)
{
	sprintf(cErrorMessage,"null");
}

void pError_Set(const char *ccMessage,...)
{
	char	cOut[MAX_ERROR_LENGTH];
	va_list vlArguments;

	va_start(vlArguments,ccMessage);
	vsprintf(cOut,ccMessage,vlArguments);
	va_end(vlArguments);

	strncpy(cErrorMessage,ccMessage,sizeof(cErrorMessage));
}

char *pError_Get(void)
{
	return cErrorMessage;
}
