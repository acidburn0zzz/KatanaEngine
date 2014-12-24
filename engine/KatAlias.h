#ifndef __ENGINEALIAS__
#define __ENGINEALIAS__

#include "quakedef.h"

extern bool	bShading;

typedef struct
{
	short	pose1,pose2;

	float	blend;

	vec3_t	origin,angles;
} lerpdata_t;

void Alias_SetupFrame(MD2_t *mModel,lerpdata_t *ldLerp);
void Alias_Draw(entity_t *eEntity);
void Alias_DrawModelFrame(MD2_t *mModel,entity_t *eEntity,lerpdata_t lLerpData);

#endif
