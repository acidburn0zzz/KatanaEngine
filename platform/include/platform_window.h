/*	Copyright (C) 2013-2014 OldTimes Software
*/
#ifndef __PLATFORMWINDOW__
#define __PLATFORMWINDOW__

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#else	// Linux
#include <X11/Xlib.h>
#endif

typedef struct
{
	char	*cTitle,
			*cClass;

	int		iWidth,iHeight,
			x,y;

	bool	bActive,
			bFullscreen;

#ifdef _WIN32
	HWND	hWindow;
	HDC		hDeviceContext;
#else	// Linux
	Window  wInstance;
#endif
} GIPLWindow_t;

extern void gWindow_MessageBox(const char *cTitle,const char *cMessage,...);
extern void gWindow_ShowCursor(bool bShow);
extern void gWindow_GetCursorPosition(int *iX,int *iY);
extern void gWindow_Destroy(GIPLWindow_t *gwWindow);
extern void gWindow_SwapBuffers(GIPLWindow_t *gwWindow);

#ifdef __cplusplus
}
#endif

#endif
