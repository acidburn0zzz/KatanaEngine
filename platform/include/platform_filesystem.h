/*	Copyright (C) 2013-2014 OldTimes Software
*/
#ifndef __PLATFORMFILESYSTEM__
#define	__PLATFORMFILESYSTEM__

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

extern	void	pFileSystem_GetUserName(char *cOut);
extern	void	pFileSystem_ScanDirectory(const char *ccPath,void (*vFunction)(const char *ccFile));
extern	void	pFileSystem_GetWorkingDirectory(char *cOut);

extern	bool	pFileSystem_CreateDirectory(const char *ccPath);

#ifdef __cplusplus
}
#endif

#endif
