#ifndef	__EDITORCONSOLE__
#define	__EDITORCONSOLE__

#include "qe3.h"

HWND	hConsoleWindow;

void	Console_CreateWindow(void);
void	Console_Print(BOOL bWarning,const char *cMessage,...);

#endif