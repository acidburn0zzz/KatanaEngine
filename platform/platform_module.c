/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform_module.h"

/*
	Module System
*/

void *gModule_FindFunction(gINSTANCE hModule,const char *cEntryFunction)
{
	if(hModule)
	{
#ifdef _WIN32
		FARPROC vFunc;

		vFunc = GetProcAddress(hModule,cEntryFunction);
#else   // Linux
		void *vFunc;

		vFunc = dlsym(hModule,cEntryFunction);
#endif
		if(vFunc)
			return (void*)vFunc;
	}

	return (NULL);
}

void gModule_Unload(gINSTANCE hModule)
{
	if(hModule)
	{
#ifdef _WIN32
		FreeLibrary(hModule);
#else   // Linux
		dlclose(hModule);
#endif

		// [12/10/2012] Set the instance to null ~hogsy
		hModule = NULL;
	}
}

void *gModule_Load(gINSTANCE hModule,const char *cPath,const char *cEntryFunction,void *vPoint)
{
	char	cUpdatedPath[gPLATFORM_MAX_PATH];
	void	*(*vMain)(void*);

	sprintf(cUpdatedPath,"%s%s",cPath,gMODULE_EXTENSION);

	// [19/5/2013] TODO: Automatically handle extensions here ~hogsy
#ifdef _WIN32
	hModule = LoadLibrary(cUpdatedPath);
#else   // Linux
	hModule = dlopen(cUpdatedPath,RTLD_NOW);
#endif
	if(!hModule)
	{
		GIPL_SetError("Failed to load library! (%s)\n",
#ifdef _WIN32
			cUpdatedPath);
#else
			dlerror());
#endif
		return NULL;
	}

	vMain = (void*(*)(void*))gModule_FindFunction(hModule,cEntryFunction);
	if(!vMain)
	{
		GIPL_SetError("Failed to find entry function! (%s)\n",cEntryFunction);
		return NULL;
	}

	return (vMain(vPoint));
}
