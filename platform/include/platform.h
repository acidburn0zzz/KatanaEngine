/*	Copyright (C) 2013-2014 OldTimes Software
*/
#ifndef __PLATFORM__
#define __PLATFORM__

#define gVERSION	"0.4"

// Shared headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef _MSC_VER	// [14/1/2014] MSVC doesn't support stdint, great... ~hogsy
#include "platform_inttypes.h"
#else
#include <stdint.h>
#endif
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>

#define PLATFORM_SYSTEM_HEADERS // temp

#ifdef _WINDOWS
#ifdef PLATFORM_SYSTEM_HEADERS
#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#endif

// [4/3/2014] TODO: At some point, ditch these in the main header since it's the opposite of what this library should be doing! ~hogsy
#ifdef _WIN32   // x32
// Platform information
#define	gPLATFORM_NAME	    "WIN32"	// Platform name.
#define	gPLATFORM_MAX_PATH	260		// Maximum path length.

#define gINSTANCE	HINSTANCE       // Instance definition.
#else   // x64
#define gPLATFORM_NAME  "WIN64"
#endif
#else	// Linux
#ifdef PLATFORM_SYSTEM_HEADERS
#include <dirent.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#endif

// Platform information
#define	gPLATFORM_NAME		"LINUX" // Platform name.
#define	gPLATFORM_MAX_PATH	256		// Maximum path length.

#define gINSTANCE	void *
#endif

#define gPLATFORM_CPU   "x64"

#ifdef __cplusplus
extern "C" {
#else
typedef int	bool;

#define	true	1
#define	false	0
#endif

#ifndef BOOL
#define BOOL    bool
#endif

#ifndef TRUE
#define	TRUE	true
#endif

#ifndef FALSE
#define FALSE	false
#endif

#define gTRUE   true
#define gFALSE  false

typedef	bool			gBOOL;
typedef unsigned int	gUINT;
typedef	unsigned char	gBYTE;	// TODO: Or just define as uint8_t?

extern void	GIPL_ResetError(void);
extern void	GIPL_SetError(const char *ccMessage,...);
extern void gError_SetFunction(const char *ccFunction,...);

extern char	*GIPL_GetError(void);

#ifdef __cplusplus
}
#endif

#endif
