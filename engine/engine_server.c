/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_server.h"

#include "KatGL.h"
#include "engine_console.h"

typedef struct
{
	const char		*cFieldName;

	int				iOffset;

	EntityType_t	eDataType;

	int				iFlags;
} EntityField_t;

EntityField_t	GlobalFields[] =
{
	{	"wad",			0,	EV_STRING		},
	{	"ambient",		0,	EV_NONE			},
	{	"sky",			0,	EV_STRING		},

	{	0	}
};

EntityField_t	EntityFields[] =
{
	{	"classname",	FIELD(v.cClassname),		EV_STRING	},
	{	"name",			FIELD(v.cName),				EV_STRING	},
	{	"model",		FIELD(v.model),				EV_STRING	},
	{	"targetname",	FIELD(v.targetname),		EV_STRING	},
	{	"noise",		FIELD(v.noise),				EV_STRING	},
	{	"message",		FIELD(v.message),			EV_STRING	},
	{	"origin",		FIELD(v.origin),			EV_VECTOR	},
	{	"angles",		FIELD(v.angles),			EV_VECTOR	},
	{	"light",		FIELD(v.vLight),			EV_VECTOR4	},
	{	"health",		FIELD(v.iHealth),			EV_INTEGER	},
	{	"spawnflags",	FIELD(v.spawnflags),		EV_INTEGER	},
	{	"bTakeDamage",	FIELD(v.bTakeDamage),		EV_BOOLEAN	},
	{	"scale",		FIELD(Model.fScale),		EV_FLOAT	},
	{	"alpha",		FIELD(alpha),				EV_FLOAT	},

	// Physical properties
	{	"physics_solid",	FIELD(Physics.iSolid),		EV_INTEGER	},
	{	"physics_mass",		FIELD(Physics.fMass),		EV_FLOAT	},
	{	"physics_gravity",	FIELD(Physics.fGravity),	EV_FLOAT	},

	// Local (move these at some point)
	{	"sound",			FIELD(local.sound),				EV_STRING	},
	{	"soundstart",		FIELD(local.cSoundStart),		EV_STRING	},
	{	"soundstop",		FIELD(local.cSoundStop),		EV_STRING	},
	{	"soundmoving",		FIELD(local.cSoundMoving),		EV_STRING	},
	{	"soundreturn",		FIELD(local.cSoundReturn),		EV_STRING	},
	{	"target1",			FIELD(local.cTarget1),			EV_STRING	},
	{	"target2",			FIELD(local.cTarget2),			EV_STRING	},
	{	"speed",			FIELD(local.speed),				EV_FLOAT	},
	{	"delay",			FIELD(local.delay),				EV_FLOAT	},
	{	"lip",				FIELD(local.lip),				EV_FLOAT	},
	{	"wait",				FIELD(local.dWait),				EV_DOUBLE	},
	{	"damage",			FIELD(local.iDamage),			EV_INTEGER	},
	{	"volume",			FIELD(local.volume),			EV_INTEGER	},
	{	"style",			FIELD(local.style),				EV_INTEGER	},
	{	"count",			FIELD(local.count),				EV_INTEGER	},
	{	"pTeam",			FIELD(local.pTeam),				EV_INTEGER	},
	{	"attack_finished",	FIELD(local.dAttackFinished),	EV_DOUBLE	},

	// hacks
	{	"angle",	FIELD(v.angles),	EV_VECTOR,	FL_ANGLEHACK	},

    {	"wad",			0,	EV_NONE		},
	{	"ambient",		0,	EV_NONE		},
	{	"sky",			0,	EV_NONE		},

	{	0	}
};

/*	[20/11/2012]
	Based on ED_ParseField from Quake 2. ~hogsy
	TODO:
		Better error handling?
*/
void Server_ParseEntityField(char *cKey,char *cValue,edict_t *eEntity)
{
	EntityField_t	*eField;
	vec4_t			vVector;

	for(eField = EntityFields; eField->cFieldName; eField++)
	{
		if(!Q_strcasecmp((char*)eField->cFieldName,cKey))
		{
			switch(eField->eDataType)
			{
				case EV_STRING:
					*(char**)((byte*)eEntity+eField->iOffset) = ED_NewString(cValue);
					break;
				case EV_VECTOR:
					switch(eField->iFlags)
					{
					case FL_ANGLEHACK:
						((float*)((byte*)eEntity+eField->iOffset))[0] = 0;
						((float*)((byte*)eEntity+eField->iOffset))[1] = (float)atof(cValue);
						((float*)((byte*)eEntity+eField->iOffset))[2] = 0;
						break;
					default:
						sscanf(cValue,"%f %f %f",&vVector[0],&vVector[1],&vVector[2]);
						((float*)((byte*)eEntity+eField->iOffset))[0] = vVector[0];
						((float*)((byte*)eEntity+eField->iOffset))[1] = vVector[1];
						((float*)((byte*)eEntity+eField->iOffset))[2] = vVector[2];
					}
					break;
				case EV_VECTOR4:
					sscanf(cValue,"%f %f %f %f",&vVector[0],&vVector[1],&vVector[2],&vVector[3]);
					((float*)((byte*)eEntity+eField->iOffset))[0] = vVector[0];
					((float*)((byte*)eEntity+eField->iOffset))[1] = vVector[1];
					((float*)((byte*)eEntity+eField->iOffset))[2] = vVector[2];
					((float*)((byte*)eEntity+eField->iOffset))[3] = vVector[3];
					break;
				case EV_FLOAT:
					*(float*)((byte*)eEntity+eField->iOffset) = (float)atof(cValue);
					break;
				case EV_DOUBLE:
					*(double*)((byte*)eEntity+eField->iOffset) = strtod(cValue,NULL);
					break;
				case EV_BOOLEAN:
					if(!Q_strcasecmp(cValue,"true"))
						cValue = "1";
					else if(!Q_strcasecmp(cValue,"false"))
						cValue = "0";
					// [2/1/2013] Booleans are handled in the same way as integers, so don't break here! ~hogsy
				case EV_INTEGER:
					*(int*)((byte*)eEntity+eField->iOffset) = atoi(cValue);
					break;
				// [22/11/2012] Just ignore anything that has skip set! ~hogsy
				case EV_NONE:
					break;
				default:
					Con_Warning("Unknown entity field type! (%i)\n",(int)eField->eDataType);
			}
			return;
		}
	}

	Con_Warning("Invalid field! (%s)\n",cKey);
}

edict_t *Server_FindEntity(edict_t *eStartEntity,char *cName,bool bClassname)
{
	int		i;
	edict_t *eEntity = eStartEntity;

	for(i = 0; i < sv.num_edicts; i++,eEntity = NEXT_EDICT(eEntity))
	{
		if(eEntity->free)
			continue;

		if(bClassname)
		{
			if(eEntity->v.cClassname)
				if(!Q_strcmp(cName,eEntity->v.cClassname))
					return eEntity;
		}
		else
		{
			if(eEntity->v.cName)
				if(!Q_strcmp(cName,eEntity->v.cName))
					return eEntity;
		}
	}

	return NULL;
}

// [9/7/2012] Added Server_PrecacheResource ~hogsy
void Server_PrecacheResource(int iType,const char *ccResource)
{
	char	name[MAX_OSPATH];
	int		i;

	/*	[13/7/2012] TODO:
			Set extension automatically based on type.

		~hogsy
	*/

	if(sv.state != SERVER_STATE_LOADING)
	{
		Con_Warning("Server attempted to precache resource during runtime! (%s)\n",ccResource);
		return;
	}

	switch(iType)
	{
	case RESOURCE_MODEL:
		for (i = 0; i < MAX_MODELS; i++)
			if(!sv.model_precache[i])
			{
				sv.model_precache[i]	= (char*)ccResource;
				sv.models[i]			= Mod_ForName((char*)ccResource);
				return;
			}
			else if(!strcmp(sv.model_precache[i],ccResource))
				return;

		Console_ErrorMessage(false,ccResource,"Overflow!");
		break;
	case RESOURCE_PARTICLE:
	case RESOURCE_FLARE:
		sprintf(name,"%s%s",DIR_PARTICLES,ccResource);
		for(i = 0; i < MAX_PARTICLES; i++)
			if(!sv.cParticlePrecache[i])
			{
				sv.cParticlePrecache[i]	= (char*)ccResource;
				return;
			}
			else if(!strcmp(name,sv.cParticlePrecache[i]))
				return;

		Console_ErrorMessage(false,ccResource,"Overflow!");
		break;
	case RESOURCE_SOUND:
		for(i = 0; i < MAX_SOUNDS; i++)
			if(!sv.sound_precache[i])
			{
				sv.sound_precache[i] = (char*)ccResource;
				return;
			}
			else if(!strcmp(sv.sound_precache[i],ccResource))
				return;

		Console_ErrorMessage(false,(char*)ccResource,"Overflow!");
		break;
	default:
		Con_Warning("Attempted to precache an undefined type! (%s) (%i)\n",ccResource,iType);
	}
}

void Server_SinglePrint(edict_t *eEntity,char *cMessage)
{
	client_t	*cClient;
	int			iEntity	= NUM_FOR_EDICT(eEntity);

	if(iEntity > svs.maxclients)
	{
		Con_Warning("Attempted to send a message to a non-client!\n");
		return;
	}

	cClient = &svs.clients[iEntity-1];

	MSG_WriteChar(&cClient->message,SVC_PRINT);
	MSG_WriteString(&cClient->message,cMessage);
}
