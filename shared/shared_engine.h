/*	Copyright (C) 2011-2013 OldTimes Software
*/
#ifndef __SHAREDENGINE__
#define __SHAREDENGINE__

#include "../platform/include/platform.h"

#include "shared_flags.h"

//johnfitz -- stuff for 2d drawing control
typedef enum
{
	CANVAS_NONE,
	CANVAS_DEFAULT,
	CANVAS_CONSOLE,
	CANVAS_MENU,
	CANVAS_SBAR,
	CANVAS_WARPIMAGE,
	CANVAS_CROSSHAIR,
	CANVAS_BOTTOMLEFT,
	CANVAS_BOTTOMRIGHT,
	CANVAS_TOPRIGHT,
} canvastype;

// [12/10/2012] Engine import functions ~hogsy
typedef struct
{
	bool    (*Initialize)(int argc,char *argv[]);                   // Initializes the engine.
    bool    (*CreateWindow)(int X,int Y,int iWidth,int iHeight);    // Creates a new window.

    char    *(*GetGamePath)(void);  // Gets the currently active game path.

    void    (*Shutdown)(void);

	int		iVersion;
} EngineExport_t;

// [12/10/2012] Engine export functions ~hogsy
typedef struct
{
	int		iVersion;
} EngineImport_t;

#define ENGINE_INTERFACE	(sizeof(EngineImport_t)+sizeof(EngineExport_t))

#endif
