/*	Copyright (C) 2013-2014 OldTimes Software
*/
#ifndef __PLATFORM__
#define __PLATFORM__

/*
	Platform Library
	Version 0.6

	This library includes standard platform headers,
	gives you some standard functions to interact with
	the system and defines for basic data-types that
	you can use in your applications for easier multi-platform
	support.
*/

#define gVERSION	"0.6"

// Shared headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef _MSC_VER	// [14/1/2014] MSVC doesn't support stdint, great... ~hogsy
#	include "platform_inttypes.h"
#else
#	include <stdint.h>
#endif
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#	include <Windows.h>
#	include <WindowsX.h>
#	include <CommCtrl.h>
#	include <direct.h>

	// Platform information
#	define	PLATFORM_NAME		"WINDOWS"	// Platform name.
#	define	PLATFORM_MAX_PATH	260			// Maximum path length.

	// Other
#	define	pINSTANCE	HINSTANCE       // Instance definition.
#	define	pFARPROC	FARPROC			// Function pointer.
#else	// Linux
	// Linux Headers
#	include <dirent.h>
#	include <unistd.h>
#	include <dlfcn.h>

	// Platform information
#	define	PLATFORM_NAME		"LINUX" // Platform name.
#	define	PLATFORM_MAX_PATH	256		// Maximum path length.

	// Other
#	define	pINSTANCE	void *		// Instance definition.
#	define	pFARPROC	void *		// Function pointer.
#endif

// [25/3/2014] Quickly threw this in so we can add these extensions to modules in a clean way :) ~hogsy
#if defined(__amd64) || defined(__amd64__)
#	define PLATFORM_CPU   "x64"
#else
#	define PLATFORM_CPU   "x86"
#endif

#ifndef __cplusplus
typedef int	bool;

#define	true	1
#define	false	0
#endif

#ifndef BOOL
#	define BOOL    bool
#endif
#ifndef TRUE
#	define	TRUE	true
#endif
#ifndef FALSE
#	define FALSE	false
#endif

#define	pBOOL		bool
#define pTRUE		true
#define pFALSE		false
#ifdef _MSC_VER	// [15/5/2014] MSVC doesn't support __func__ either... ~hogsy
#	define	pFUNCTION	__FUNCTION__	// Returns the active function.
#else
#	define	pFUNCTION	__func__		// Returns the active function.
#endif

typedef unsigned int	pUINT;
typedef	unsigned char	pBYTE;

#ifdef __cplusplus
extern "C" {
#endif

extern	void	pError_Reset(void);
extern	void	pError_Set(const char *ccMessage,...);
extern	void	pError_SetFunction(const char *ccFunction,...);

extern char	*pError_Get(void);

#ifdef __cplusplus
}
#endif

#endif
