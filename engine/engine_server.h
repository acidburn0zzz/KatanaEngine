/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef	__ENGINESERVER__
#define	__ENGINESERVER__

#include "quakedef.h"

#include "shared_server.h"

edict_t *Server_FindEntity(edict_t *eStartEntity,char *cName,bool bClassname);

void	Server_SinglePrint(edict_t *eEntity,char *cMessage);
void	Server_PrecacheResource(int iType,const char *ccResource);

double	Server_GetFrameTime(void);

#endif
