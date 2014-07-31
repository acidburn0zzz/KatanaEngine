/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_main.h"

/*
	General entity treatment.
*/

/*	Create a new entity instance.
*/
edict_t *Entity_Spawn(void)
{
	return Engine.Spawn();
}

/*	Alternative to SetOrigin.
	This is the only valid way to
	move an object without using
	the physics of the world
	(setting velocity and waiting).
	Directly changing origin will
	not set internal links correctly,
	so clipping would be messed up.
	This should be called when an
	object is spawned, and then only
	if it is teleported.
*/
void Entity_SetOrigin(edict_t *eEntity,vec3_t vOrigin)
{
	Math_VectorCopy(vOrigin,eEntity->v.origin);

	Engine.LinkEntity(eEntity,false);
}

/*	Alternative to SetModel.
*/
void Entity_SetModel(edict_t *eEntity,char *cModelPath)
{
	Engine.SetModel(eEntity,cModelPath);
}

/*	Alternative to SetSize.
*/
void Entity_SetSizeVector(edict_t *eEntity,vec3_t vMin,vec3_t vMax)
{
	int	i;

	// [13/9/2013] Check if the model is set yet, if not give us a little warning ~hogsy
	if(!eEntity->v.model)
		Engine.Con_Warning("Setting entity size before model! (%s)\n",eEntity->v.cClassname);

	for(i = 0; i < 3; i++)
		if(vMin[i] > vMax[i])
		{
			Engine.Con_Warning("Backwards mins/maxs! (%s)\n",eEntity->v.cClassname);
			return;
		}

	Math_VectorCopy(vMin,eEntity->v.mins);
	Math_VectorCopy(vMax,eEntity->v.maxs);
	Math_VectorSubtract(vMax,vMin,eEntity->v.size);

	Engine.LinkEntity(eEntity,false);
}

/*	Set the size of the bounding box for the entity.
	Should be called AFTER setting model!
*/
void Entity_SetSize(edict_t *eEntity,
	float fMinA,float fMinB,float fMinC,
	float fMaxA,float fMaxB,float fMaxC)
{
	vec3_t	vMin,vMax;

	vMin[0] = fMinA; vMin[1] = fMinB; vMin[2] = fMinC;
	vMax[0] = fMaxA; vMax[1] = fMaxB; vMax[2] = fMaxC;

	// [12/9/2013] Meh, just do it through our other func :) ~hogsy
	Entity_SetSizeVector(eEntity,vMin,vMax);
}

/*	Can be used for convenience.
*/
void Entity_AddEffects(edict_t *eEntity,int iEffects)
{
	eEntity->v.effects |= iEffects;
}

/*	Can be used for convenience.
*/
void Entity_RemoveEffects(edict_t *eEntity,int iEffects)
{
	eEntity->v.effects &= ~iEffects;
}

/*	Can be used for convenience.
*/
void Entity_ClearEffects(edict_t *eEntity)
{
	eEntity->v.effects = 0;
}

/*	Find a random spawn point for the entity (point_start).
	TODO: Rename!
*/
edict_t *Entity_SpawnPoint(edict_t *eEntity,int iType)
{
	edict_t	*eSpawnPoint;
	char	*cStartName;

	switch(iType)
	{
	case INFO_PLAYER_DEATHMATCH:
		cStartName = "point_deathmatch";
		break;
	case INFO_PLAYER_CTF:
		if(eEntity->local.pTeam == TEAM_RED)
			cStartName = "point_start_red";
		else if(eEntity->local.pTeam == TEAM_BLUE)
			cStartName = "point_start_blue";
		else
		{
			Engine.Con_Warning("Unknown team type for spawn point! (%i)",eEntity->local.pTeam);
			return NULL;
		}
		// [27/4/2014] Oops, forgot to break here ~hogsy
		break;
	case INFO_PLAYER_SUPERFLY:
	case INFO_PLAYER_MIKIKO:
	default:
		cStartName = "point_start";
	}

	eSpawnPoint = Engine.Server_FindEntity(Server.eWorld,cStartName,true);
	if(eSpawnPoint)
		if(iType == eSpawnPoint->local.style)
			return eSpawnPoint;

	return NULL;
}

/*	Simple function for checking if an entity can be damaged or not.
*/
bool Entity_CanDamage(edict_t *eEntity,edict_t *eTarget, int iDamageType)
{
	// Can't damage people on the same team.
	if(eEntity->local.pTeam && (eEntity->local.pTeam == eTarget->local.pTeam))
		return false;

	if(eTarget->v.bTakeDamage && (!eTarget->local.iDamageType || (eTarget->local.iDamageType == iDamageType)))
		return true;

	return false;
}

/*	Damage entities within a specific radius.
*/
void Entity_RadiusDamage(edict_t *eInflictor,float fRadius,int iDamage,int iDamageType)
{
	int		i;
	float	fDistance;
	vec3_t	vOrigin;
	edict_t *eTarget = Engine.Server_FindRadius(eInflictor->v.origin,fRadius);

	Engine.WriteByte(MSG_BROADCAST,SVC_TEMPENTITY);
	Engine.WriteByte(MSG_BROADCAST,CTE_EXPLOSION);

	for(i = 0; i < 3; i++)
		Engine.WriteCoord(MSG_BROADCAST,eInflictor->v.origin[i]);

	do
	{
		if(eTarget->v.bTakeDamage)
		{
			int i;

			for(i = 0; i < 3; i++)
				vOrigin[i] = eTarget->v.origin[i]+(eTarget->v.mins[i]+eTarget->v.maxs[i])*0.5f;

			Math_VectorSubtract(eInflictor->v.origin,vOrigin,vOrigin);

			fDistance = 0.5f*(float)Math_VectorLength(vOrigin);
			if(fDistance > 0)
			{
				Math_VectorInverse(vOrigin);
				Math_VectorAdd(eTarget->v.velocity,vOrigin,eTarget->v.velocity);

				// [15/7/2013] Reduce the damage by distance ~hogsy
				fDistance = (float)iDamage-(100.0f/fDistance);

				// [15/8/2013] Less damage for the inflictor ~hogsy
				if(eTarget == eInflictor)
					fDistance = fDistance/2.0f;

				if(fDistance > 0)
					if(Entity_CanDamage(eInflictor,eTarget, iDamageType))
						MONSTER_Damage(eTarget,eInflictor,(int)fDistance,iDamageType);
			}
		}

		eTarget = eTarget->v.chain;
	}
	while(eTarget);
}

/*	Remove an entity from the world.
*/
void Entity_Remove(edict_t *eEntity)
{
	Engine.FreeEntity(eEntity);
}

/*
	Animation
*/

/*	Checks and updates the entity's current frame.
*/
void Entity_CheckFrames(edict_t *eEntity)
{
	if(!eEntity->local.iAnimationEnd || Server.dTime < eEntity->local.dAnimationTime)	// If something isn't active and Animationtime is over
		return;
	else if(eEntity->local.iAnimationCurrent > eEntity->local.iAnimationEnd)
	{
		eEntity->local.dAnimationTime = 0;
		return;
	}

	eEntity->v.frame = eEntity->local.iFrames[eEntity->local.iAnimationCurrent].iFrame;

	eEntity->local.dAnimationTime = Server.dTime+((double)eEntity->local.iFrames[eEntity->local.iAnimationCurrent].fSpeed);

	if(eEntity->local.iFrames[eEntity->local.iAnimationCurrent].Function)
		eEntity->local.iFrames[eEntity->local.iAnimationCurrent].Function(eEntity);

	eEntity->local.iAnimationCurrent++;
}

/*	Resets the currently active animation.
*/
void Entity_ResetAnimation(edict_t *eEntity)
{
	eEntity->v.frame					=
	eEntity->local.iAnimationCurrent	=
	eEntity->local.iAnimationEnd		= 0;

	// [7/2/2013] Split away since this is a float and not an integer ~hogsy
	eEntity->local.dAnimationTime = 0;
}

/*	Start animating the entity.
*/
void Entity_Animate(edict_t *eEntity,EntityFrame_t *efAnimation)
{
	/*	TODO:
		Something in here was fucked up... Can't remember what but it needs looking over.
		~hogsy
	*/
	int i = 0;

	Entity_ResetAnimation(eEntity);

	eEntity->local.iAnimationCurrent = 0;

	// [20/10/2013] We were skipping the first frame? Fixed ~hogsy
	eEntity->v.frame = efAnimation[0].iFrame;

	for(;;)
	{
		if(efAnimation[i].bIsEnd)
		{
			eEntity->local.iAnimationEnd = i;
			break;
		}

		i++;
	}

	eEntity->local.dAnimationTime	= Server.dTime+((double)efAnimation[1].fSpeed);
	eEntity->local.iFrames			= efAnimation;
}

/*
	Type Checks
*/

/*	Is the given monster a player?
*/
bool Entity_IsPlayer(edict_t *eEntity)
{
	if(eEntity->monster.iType == MONSTER_PLAYER)
		return true;

	return false;
}

/*	Is the given monster a monster? (this is dumb...)
*/
bool Entity_IsMonster(edict_t *eEntity)
{
	if((eEntity->monster.iType != MONSTER_PLAYER) && (eEntity->monster.iType > MONSTER_VEHICLE))
		return true;

	return false;
}
