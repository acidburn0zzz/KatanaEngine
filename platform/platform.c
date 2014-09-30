/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform.h"

/*
	Generic functions for platform, such as
	error handling.
*/

char	cErrorMessage[2048] = "null",
		cLastFunction[1024]	= "null";

/*	Sets the name of the currently entered function.
*/
void pError_SetFunction(const char *ccFunction,...)
{
	char	cOut[2048];
	va_list vlArguments;

	va_start(vlArguments,ccFunction);
	vsprintf(cOut,ccFunction,vlArguments);
	va_end(vlArguments);

	strcpy(cLastFunction,ccFunction);
}

void pError_Reset(void)
{
	sprintf(cErrorMessage,"null");
}

void pError_Set(const char *ccMessage,...)
{
	char	cOut[2048];
	va_list	vlArguments;

	va_start(vlArguments,ccMessage);
	vsprintf(cOut,ccMessage,vlArguments);
	va_end(vlArguments);

	sprintf(cErrorMessage,cOut);

	// [9/10/2013] TEMP: Bleh just print the fucking thing to console ~hogsy
	printf("Error: %s",cOut);
}

char *pError_Get(void)
{
	return cErrorMessage;
}
