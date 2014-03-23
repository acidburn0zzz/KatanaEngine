/*	Copyright (C) 2013-2014 OldTimes Software
*/
#ifndef __PLATFORMMODULE__
#define __PLATFORMMODULE__

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

#ifdef _WIN32
#define	gMODULE_EXTENSION	".dll"
#define	gMODULE_EXPORT		__declspec(dllexport)
#define gMODULE_IMPORT		__declspec(dllimport)
#else   // Linux
#define	gMODULE_EXTENSION	".so"
#define gMODULE_EXPORT		__attribute__((visibility("default")))
#define gMODULE_IMPORT		__attribute__((visibility("hidden")))
#endif

extern void *gModule_FindFunction(gINSTANCE hModule,const char *cEntryFunction);
extern void gModule_Unload(gINSTANCE hModule);
extern void *gModule_Load(gINSTANCE hModule,const char *cPath,const char *cEntryFunction,void *vPoint);

// Defines for legacy functions.     
#define	GIPL_EXPORT	gMODULE_EXPORT
#define GIPL_IMPORT	gMODULE_IMPORT

#ifdef __cplusplus
}
#endif

#endif
