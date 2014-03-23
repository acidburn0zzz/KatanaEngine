#include "editor_console.h"

HANDLE	hOutput,hInput;

LONG WINAPI Console_WindowProcedure(
	HWND    hWnd,
	UINT    uMsg,
	WPARAM  wParam,
	LPARAM  lParam)
{
	switch(uMsg)
	{
	}

	return (LONG)NULL;
}

void Console_Print(bool bWarning,const char *cMessage,...)
{
	DWORD			dWritten;
	static	char	scBuffer[2048];
	va_list			vMarker;

	if(bWarning)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED);
	else
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY);

	va_start(vMarker,cMessage);
	vsprintf(scBuffer,cMessage,vMarker);
	va_end(vMarker);

	WriteConsole(hOutput,scBuffer,strlen(scBuffer),&dWritten,NULL);
}

void Console_CreateWindow(void)
{
	AllocConsole();

	hOutput	= GetStdHandle(STD_OUTPUT_HANDLE);
	hInput	= GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleTitle("Console");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY);
}