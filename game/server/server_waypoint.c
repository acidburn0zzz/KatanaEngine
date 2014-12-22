/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "server_waypoint.h"

/*
	Used for both navigation and as basic reference points within levels.

	TODO:
		Save our waypoints to a map specific document.
*/

#define	WAYPOINT_MAX_ALLOCATED	2048

Waypoint_t	wWaypoints[WAYPOINT_MAX_ALLOCATED];

int	iWaypointCount,
	iWaypointAllocated;

void Waypoint_Shutdown(void);

/*	Called per server spawn to reset waypoints and set everything up.
*/
void Waypoint_Initialize(void)
{
	memset(wWaypoints,0,sizeof(wWaypoints));

	iWaypointCount = 0;
}

bool Waypoint_IsSafe(edict_t *eMonster,Waypoint_t *wPoint)
{
	edict_t *eMonsters;

	eMonsters = Engine.Server_FindRadius(wPoint->position,MONSTER_RANGE_NEAR);
	while(eMonsters)
	{
		// [6/6/2013] Check this by relationship rather than type ~hogsy
		if(Monster_GetRelationship(eMonster,eMonsters) == RELATIONSHIP_HATE)
			return false;

		eMonsters = eMonsters->v.chain;
	}
	return true;
}

/*	Allocate a new waypoint so that we can use it.
	(Originally relied on dynamic allocation, but not anymore)
*/
Waypoint_t *Waypoint_Allocate(void)
{
	if((iWaypointCount+1) > WAYPOINT_MAX_ALLOCATED)
	{
		Engine.Con_Warning("Failed to allocate waypoint! (%i / %i)\n",iWaypointCount,WAYPOINT_MAX_ALLOCATED);
		return NULL;
	}

	return &wWaypoints[iWaypointCount++];
}

void Waypoint_Delete(Waypoint_t *wPoint)
{
	if(!wPoint)
		return;

#ifdef	DEBUG_WAYPOINT
	// [23/3/2013] If we have an entity set for debugging, then remove it! ~hogsy
	if(wPoint->eDebug)
		Entity_Remove(wPoint->eDebug);
#endif

	free((void*)wPoint);

	iWaypointCount--;
	iWaypointAllocated--;
}

// [25/6/2012] Added Monster_FindClosestVisibleWaypoint ~hogsy
// [20/12/2012] Renamed to Monster_GetVisibleWaypoint for consistency ~hogsy
// [30/1/2013] Renamed to Waypoint_GetByVisibility ~hogsy
Waypoint_t *Waypoint_GetByVisibility(vec3_t vOrigin)
{
	Waypoint_t	*wPoint;
	trace_t		tTrace;

	for(wPoint = wWaypoints; wPoint->number < iWaypointCount; wPoint++)
	{
		tTrace = Traceline(NULL,vOrigin,wPoint->position,0);
		// [16/1/2014] Given point cannot be in the same place as the given origin ~hogsy
		if(	Math_VectorCompare(tTrace.endpos,wPoint->position) && 
			!Math_VectorCompare(vOrigin,wPoint->position))
			return wPoint;
	}
	return NULL;
}

Waypoint_t *Waypoint_GetByNumber(int iWaypointNumber)
{
	Waypoint_t	*wPoint;

	for(wPoint = wWaypoints; wPoint->number < iWaypointCount; wPoint++)
		if(wPoint->number == iWaypointNumber)
			return wPoint;

	return NULL;
}

// [20/12/2012] Let us find a waypoint by type! ~hogsy
Waypoint_t *Waypoint_GetByType(edict_t *eMonster,int iType,float fMaxDistance)
{
	Waypoint_t	*wPoint;
	vec3_t		vDistance;

	// [5/1/2013] Very unlikely but this is a temporary precaution ~hogsy
	if(!eMonster)
	{
		Engine.Con_Warning("Failed to get entity when finding waypoint!\n");
		return NULL;
	}

	for(wPoint = wWaypoints; wPoint->number < iWaypointCount; wPoint++)
		if(wPoint->wType == iType)
		{
			// [20/9/2012] TODO: Needs testing :[ ~hogsy
			Math_VectorSubtract(eMonster->v.origin,wPoint->position,vDistance);
			if(Math_Length(vDistance) < fMaxDistance)
				return wPoint;
		}

	// [20/9/2012] Welp we didn't find anything, return null ~hogsy
	return NULL;
}

// [20/9/2012] Lets us find a specific waypoint by name ~hogsy
Waypoint_t *Waypoint_GetByName(edict_t *eMonster,char *cName,float fMaxDistance)
{
	Waypoint_t	*wPoint;
	vec3_t		vDistance;

	for(wPoint = wWaypoints; wPoint->number < iWaypointCount; wPoint++)
		if(strcmp(wPoint->cName,cName))
		{
			// [20/9/2012] TODO: Needs testing :[ ~hogsy
			Math_VectorSubtract(eMonster->v.origin,wPoint->position,vDistance);
			if(Math_Length(vDistance) < fMaxDistance)
				return wPoint;
		}

	// [20/9/2012] Welp we didn't find anything, return null ~hogsy
	return NULL;
}

void Waypoint_Frame(edict_t *eEntity)
{
#if 0
	Waypoint_t	*wPoint;

	for(wPoint = wWaypoints; wPoint->number < iWaypointCount; wPoint++)
	{
		if(!wPoint->next)
			wPoint->next = Waypoint_GetByNumber(wPoint->number+1);
		// Is someone there!?
		// Yeah, we're occupied folks :)
		// Oh they left, we're not occupied!
	}
#endif
}

void Waypoint_Spawn(vec3_t vOrigin,WaypointType_t type)
{
#ifdef	DEBUG_WAYPOINT
	char		*cModelName = WAYPOINT_MODEL_BASE;
#endif
	int			iPointContents;
	Waypoint_t	*wPoint;

	/*	TODO
		If we're between two other waypoints
		and they can be seen then slot ourselves
		in so that we act as the last waypoint
		instead.
	*/

	iPointContents = Engine.Server_PointContents(vOrigin);
	// [17/6/2012] Check that this area is safe ~hogsy
	if(iPointContents == BSP_CONTENTS_SOLID)
	{
		Engine.Con_Warning("Failed to place waypoint, position is within a solid!\n");
		return;
	}

	{
		Waypoint_t *wVisibleWaypoint = Waypoint_GetByVisibility(vOrigin);
		// [30/1/2013] Oops! Check we actually have a visible waypoint!! ~hogsy
		if(wVisibleWaypoint)
		{
			vec3_t	vDistance;

			Math_VectorSubtract(wVisibleWaypoint->position,vOrigin,vDistance);
			if(Math_VectorLength(vDistance) < MONSTER_RANGE_MEDIUM)
			{
				Engine.Con_Printf("Invalid waypoint position!\n");
				return;
			}
		}
	}

	wPoint = Waypoint_Allocate();
	if(!wPoint)
	{
		Engine.Con_Warning("Failed to allocate waypoint!\n");
		return;
	}

	Math_VectorCopy(vOrigin,wPoint->position);

	wPoint->number	= iWaypointCount;
	wPoint->bOpen	= false;
	wPoint->next	= Waypoint_GetByNumber(wPoint->number+1);
	wPoint->last	= Waypoint_GetByNumber(wPoint->number-1);
	wPoint->wType	= type;

	switch(type)
	{
	case WAYPOINT_ITEM:
		wPoint->cName = "item";
#ifdef DEBUG_WAYPOINT
		cModelName	= WAYPOINT_MODEL_ITEM;
#endif
		break;
	case WAYPOINT_CLIMB:
		wPoint->cName = "climb";
		// [27/12/2012] TODO: Check that there's a ladder nearby ~hogsy
#ifdef DEBUG_WAYPOINT
		cModelName	= WAYPOINT_MODEL_CLIMB;
#endif
		break;
	case WAYPOINT_COVER:
		wPoint->cName = "cover";
		// [27/12/2012] TODO: Check that this is actually cover ~hogsy
		break;
	case WAYPOINT_JUMP:
		wPoint->cName = "jump";
		// [27/12/2012] TODO: Check if this is actually a jump by tracing out ahead ~hogsy
#ifdef DEBUG_WAYPOINT
		cModelName	= WAYPOINT_MODEL_JUMP;
#endif
		break;
	case WAYPOINT_SWIM:
		if(iPointContents != BSP_CONTENTS_WATER)
		{
			Engine.Con_Warning("Waypoint with type swim not within water contents (%i %i %i)!",
				(int)vOrigin[0],
				(int)vOrigin[1],
				(int)vOrigin[2]);

			Waypoint_Delete(wPoint);
			return;
		}

		wPoint->cName = "swim";
#ifdef DEBUG_WAYPOINT
		cModelName	= WAYPOINT_MODEL_SWIM;
#endif
		break;
	case WAYPOINT_DEFAULT:
		wPoint->cName = "default";
		break;
	case WAYPOINT_SPAWN:
		wPoint->cName = "spawn";
		break;
	default:
		Engine.Con_Warning("Unknown waypoint type (%i)!\n",type);

		Waypoint_Delete(wPoint);
		return;
	}

	// [30/1/2013] Pathetic reordering... Ugh ~hogsy
	if(!wPoint->last)
	{
		wPoint->last = Waypoint_GetByVisibility(vOrigin);
		if(!wPoint->last)
		{
			Engine.Con_Warning("Failed to get another visible waypoint! (%i)\n",wPoint->number);
			return;
		}
	}
	else if(wPoint->last != wPoint && wPoint->last->next)
		wPoint->last->next = wPoint;

#ifdef DEBUG_WAYPOINT
	wPoint->eDebug = Entity_Spawn();
	if(wPoint->eDebug)
	{
		wPoint->eDebug->v.effects = EF_MOTION_ROTATE;

		Entity_SetModel(wPoint->eDebug,cModelName);
		Entity_SetSizeVector(wPoint->eDebug,mv3Origin,mv3Origin);
		Entity_SetOrigin(wPoint->eDebug,wPoint->position);
	}

	Engine.Con_DPrintf("Waypoint placed (%i %i %i)\n",
		(int)wPoint->position[0],
		(int)wPoint->position[1],
		(int)wPoint->position[2]);
	Engine.Con_DPrintf(" number: %i\n",wPoint->number);
	Engine.Con_DPrintf(" type:   %i\n",wPoint->wType);
#endif
}
