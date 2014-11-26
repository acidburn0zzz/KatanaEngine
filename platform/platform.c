/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform.h"

/*
	Generic functions for platform, such as
	error handling.
*/

#include "platform_system.h"

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

	strncpy(cLastFunction, cOut, sizeof(ccFunction));
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

	strncpy(cErrorMessage, cOut, sizeof(cErrorMessage));
}

char *pError_Get(void)
{
	return cErrorMessage;
}

/*
	System Error Management
*/

char	*cSystemError;

/*	Returns a system-side error message.
	Requires SystemReset to be called afterwards.
*/
char *pError_SystemGet(void)
{
#ifdef _WIN32
	char	*cBuffer = NULL;
	int		iError; 
	
	iError = GetLastError();
	if (iError == 0)
		return "Unknown system error!";
	
	if (!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		iError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&cBuffer,
		0, NULL))
		return "Failed to get system error details!";

	cSystemError = _strdup(cBuffer);

	LocalFree(cBuffer);

	return cSystemError;
#else
	cSystemError = dlerror();

	return cSystemError;
#endif
}

/*	Only really does anything on Windows, and it's probably my fault since I'm an idiot.
*/
void pError_SystemReset(void)
{
#ifdef _WIN32
	free(cSystemError);
#endif
}
