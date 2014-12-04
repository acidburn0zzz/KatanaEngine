/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform_module.h"

/*
	Module System
*/

#include "platform_system.h"

pFARPROC pModule_FindFunction(pINSTANCE hModule,const char *cEntryFunction)
{
	pERROR_UPDATE;

	if(hModule)
	{
		pFARPROC fFunc;

#ifdef _WIN32
		fFunc = GetProcAddress(hModule,cEntryFunction);
#else   // Linux
		fFunc = dlsym(hModule,cEntryFunction);
#endif
		if(fFunc)
			return fFunc;
	}

	return (NULL);
}

void pModule_Unload(pINSTANCE hModule)
{
	pERROR_UPDATE;

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

/*	Function to allow direct loading of an external module.
*/
pINSTANCE pModule_Load(const char *ccPath)
{
	pINSTANCE	iModule;
	char		cUpdatedPath[PLATFORM_MAX_PATH];

	pERROR_UPDATE;

	sprintf(cUpdatedPath,"%s"pMODULE_EXTENSION,ccPath);

	iModule	=
#ifdef _WIN32
		LoadLibrary(cUpdatedPath);
#else
		dlopen(cUpdatedPath,RTLD_NOW);
#endif
	if(!iModule)
	{
		pError_Set("Failed to load library! (%s)\n%s", cUpdatedPath, pError_SystemGet());

		pError_SystemReset();

		return NULL;
	}
	
	return iModule;
}

/*	Generic interface to allow loading of an external module.
*/
void *pModule_LoadInterface(pINSTANCE hModule,const char *cPath,const char *cEntryFunction,void *vPoint)
{
	char	cUpdatedPath[PLATFORM_MAX_PATH];
	void	*(*vMain)(void*);

	pERROR_UPDATE;

	sprintf(cUpdatedPath,"%s."PLATFORM_CPU,cPath);

	hModule = pModule_Load(cUpdatedPath);
	if(!hModule)
	{
#if 0	// Dealt with by pModule_Load.
		pError_Set("Failed to load library! (%s)\n",
#ifdef _WIN32
			cUpdatedPath);
#else
			dlerror());
#endif
#endif
		return NULL;
	}

	vMain = (void*(*)(void*))pModule_FindFunction(hModule,cEntryFunction);
	if(!vMain)
	{
		pError_Set("Failed to find entry function! (%s)\n",cEntryFunction);
		return NULL;
	}

	return (vMain(vPoint));
}
