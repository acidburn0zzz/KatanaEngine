/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform.h"

/*
	Network
*/

#include "platform_system.h"

#ifdef _WIN32
HRESULT	(*gURLDownloadToFile)(LPUNKNOWN pCaller,LPCTSTR szURL,LPCTSTR szFileName,DWORD dwReserved,LPBINDSTATUSCALLBACK lpfnCB);
#endif

/*  Initialization.
*/
void pNetwork_Initialize(void)
{
#ifdef _WIN32
//	gURLDownloadToFile = GetProcAddress(LoadLibrary("urlmon.dll"),"URLDownloadToFileA");
#endif
}

/*	Download a resource from an online source.
*/
bool gNetwork_Download(const char *ccURL,const char *ccCache)
{
	pERROR_UPDATE;

#ifdef _WIN32
	{
		HRESULT	hResult;

		hResult = URLDownloadToFile(0,ccURL,ccCache,0,0);
		switch(hResult)
		{
		case E_OUTOFMEMORY:
			pError_Set("There is insufficient memory to complete the operation!\n");
			return false;
		case INET_E_DOWNLOAD_FAILURE:
			pError_Set("Failed to find the specified file!\n");
			return false;
		}
	}
#else	// Linux
#endif

	return true;
}
