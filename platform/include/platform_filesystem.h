/*	Copyright (C) 2013-2014 OldTimes Software
*/
#ifndef __PLATFORMFILESYSTEM__
#define	__PLATFORMFILESYSTEM__

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

extern void gFileSystem_GetUserName(char *cOut);
extern void gFileSystem_ScanDirectory(const char *ccPath,void (*vFunction)(const char *ccFile));
extern void gFileSystem_GetWorkingDirectory(char *cOutput);

extern bool gFileSystem_CreateDirectory(const char *ccPath);

#ifdef __cplusplus
}
#endif

#endif
