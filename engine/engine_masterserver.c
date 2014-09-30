/*	Copyright (C) 2008-2011  Mathieu Olivier
	Copyright (C) 2011-2013 OldTimes Software

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "quakedef.h"

#include <assert.h>

#ifdef _WIN32
#include <WS2tcpip.h>

/*	KatMasterServer.c	(Based on dpmaster by Mathieu Olivier)
	This is planned to be our own internal
	master server for future projects.
	TODO:
		- Move into its own module?

	[9/9/2012]
	Added KatMasterServer.c
	Added function MasterServer_Init.
	Added struct AddressMap_t.
	Added function MasterServer_ResolveIPv4Address.
*/

typedef struct AddressMap_s
{
	struct	AddressMap_s*	aNext;
	struct	sockaddr_in		sFrom;
	struct	sockaddr_in		sTo;

	char					*cFromString;
	char					*cToString;
} AddressMap_t;

typedef struct
{
	SOCKET							sSocket;

	socklen_t						sLocalAddressLength;

	const		char				*ccLocalAddressName;

	struct		sockaddr_storage	ssLocalAddress;

	BOOL							bOptional;
} ListenSocket_t;

static	AddressMap_t *aAddressMaps = NULL;

unsigned	int	iNumSockets = 0;

ListenSocket_t	lListenSockets[8];

void MasterServer_ResolveIPv4Address(const char *cName,struct sockaddr_in *sAddress)
{
	char			*cNameCopy,*cPort;
	struct	hostent	*hHost;

	cNameCopy = strdup(cName);
	if(!cNameCopy)
		Sys_Error("Can't allocate enough memory to resolve %s!",cName);

	if((cPort = strchr(cNameCopy,':')) != NULL)
		*cPort++ = '\0';

	hHost = gethostbyname(cNameCopy);
	if(!hHost)
	{
		free(cNameCopy);

		Sys_Error("Can't resolve %s!",cNameCopy);
	}
	else if(hHost->h_addrtype != AF_INET)
	{
		free(cNameCopy);

		Sys_Error("%s is not an IPv4 address!",cNameCopy);
	}

	memset(sAddress,0,sizeof(*sAddress));
	sAddress->sin_family = AF_INET;
	memcpy(&sAddress->sin_addr.s_addr,hHost->h_addr,sizeof(sAddress->sin_addr.s_addr));
	if(cPort)
	{
		char				*cEndPtr;
		unsigned	short	usPortNumber;

		usPortNumber = (unsigned short)strtol(cPort,&cEndPtr,0);
		if(cEndPtr == cPort || *cEndPtr != '\0' || !usPortNumber)
		{
			free(cNameCopy);

			Sys_Error("%s is not a valid port number!",cPort);
		}

		sAddress->sin_port = htons(usPortNumber);
	}

	Con_DPrintf(" \"%s\" resolved to %s:%hu\n",cName,inet_ntoa(sAddress->sin_addr),ntohs(sAddress->sin_port));

	free(cNameCopy);
}

void MasterServer_Init(void)
{
	WSADATA			wData;
	AddressMap_t	*aAddressMap;
	size_t			sAddressLength;
	char			cFromAddress[16],cAddressBuffer[128];
	const	char	*ccPortName		= NULL,
					*ccAddressEnd	= NULL,
					*ccAddressStart;
	int				iAddressFamily	= AF_UNSPEC;

	Con_Printf("Initializing master server...\n");

	if(WSAStartup(MAKEWORD(1,1),&wData))
		Sys_Error("Failed to initialize winsock!\n");

	for(aAddressMap = aAddressMaps; aAddressMap != NULL; aAddressMap = aAddressMap->aNext)
	{
		MasterServer_ResolveIPv4Address(aAddressMap->cFromString,&aAddressMap->sFrom);
		MasterServer_ResolveIPv4Address(aAddressMap->cToString,&aAddressMap->sTo);

		if(!aAddressMap->sFrom.sin_addr.s_addr || !aAddressMap->sTo.sin_addr.s_addr)
			Sys_Error("Mapping from or to 0.0.0.0 is forbidden!");

		if((ntohl(aAddressMap->sTo.sin_addr.s_addr) >> 24) == 127)
			Sys_Error("Mapping to a loopback address is forbidden!");
	}

	aAddressMap		= aAddressMaps;
	aAddressMaps	= NULL;
	while(aAddressMap != NULL)
	{
		AddressMap_t	*aNextAddressMap	= aAddressMap->aNext,
						*aNewAddressMap		= aAddressMaps,
						**aPrevious			= &aAddressMaps;

		while(	aNewAddressMap != NULL &&
				aNewAddressMap->sFrom.sin_addr.s_addr <= aAddressMap->sFrom.sin_addr.s_addr)
		{
			if(	aNewAddressMap->sFrom.sin_addr.s_addr == aAddressMap->sFrom.sin_addr.s_addr &&
				aNewAddressMap->sFrom.sin_port >= aAddressMap->sFrom.sin_port)
			{
				if(aNewAddressMap->sFrom.sin_port == aAddressMap->sFrom.sin_port)
					Sys_Error
					(
						"Several mappings are declared for address %s:%hu!",
						inet_ntoa(aAddressMap->sFrom.sin_addr),
						ntohs(aAddressMap->sFrom.sin_port)
					);

				break;
			}

			aPrevious		= &aNewAddressMap->aNext;
			aNewAddressMap	= aNewAddressMap->aNext;
		}

		aAddressMap->aNext	= *aPrevious;
		*aPrevious			= aAddressMap;

		strncpy(cFromAddress,inet_ntoa(aAddressMap->sFrom.sin_addr),sizeof(cFromAddress)-1);
		cFromAddress[sizeof(cFromAddress)-1] = '\0';
		Con_Printf
		(
			"Address \"%s\" (%s:%hu) mapped to \"%s\" (%s:%hu)\n",
			aAddressMap->cFromString,
			cFromAddress,
			ntohs(aAddressMap->sFrom.sin_port),
			aAddressMap->cToString,
			inet_ntoa(aAddressMap->sTo.sin_addr),
			ntohs(aAddressMap->sTo.sin_port)
		);

		aAddressMap = aNextAddressMap;
	}

	if(!iNumSockets)
	{
	}
	else
	{
		unsigned int	uiSockInd;

		for(uiSockInd = 0; uiSockInd < iNumSockets; uiSockInd++)
		{
			ListenSocket_t	*lListenSocket = &lListenSockets[uiSockInd];

			if(lListenSocket->ccLocalAddressName[0] == '[')
			{
				const char *ccEndBracket = strchr(lListenSocket->ccLocalAddressName,']');

				if(!ccEndBracket)
					Sys_Error
					(
						"IPv6 address has no closing bracket (%s)!",
						lListenSocket->ccLocalAddressName
					);
				else if(ccEndBracket[1] != ':' && ccEndBracket[1] != '\0')
					Sys_Error
					(
						"Invalid end of bracketed IPv6 address (%s)!",
						lListenSocket->ccLocalAddressName
					);
				else if(ccEndBracket[1] == ':')
					ccPortName = ccEndBracket+2;

				iAddressFamily	= AF_INET6;

				ccAddressStart	= &lListenSocket->ccLocalAddressName[1];
				ccAddressEnd	= ccEndBracket;
			}
			else
			{
				const char	*ccFirstColon;

				ccAddressStart	= lListenSocket->ccLocalAddressName;

				ccFirstColon = strchr(lListenSocket->ccLocalAddressName,':');
				if(ccFirstColon != NULL)
				{
					const char *ccLastColon = strrchr(ccFirstColon+1,':');

					if(!ccLastColon)
					{
						ccAddressEnd	= ccFirstColon;
						ccPortName		= ccFirstColon+1;
					}
					else
						iAddressFamily = AF_INET6;
				}
			}

			if(ccAddressEnd != NULL)
			{
				assert(ccAddressEnd >= ccAddressStart);
				sAddressLength = ccAddressEnd-ccAddressStart;
			}
			else
				sAddressLength = strlen(ccAddressStart);

			if(sAddressLength >= sizeof(cAddressBuffer))
				Sys_Error
				(
					"Address too long to be resolved (%s)!",
					lListenSocket->ccLocalAddressName
				);

			memcpy(cAddressBuffer,ccAddressStart,sAddressLength);

			cAddressBuffer[sAddressLength] = '\0';

			// [9/9/2012] TODO: SYS_BUILDSOCKADDR
		}
	}
}
#endif
