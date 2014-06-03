/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "quakedef.h"

#include "engine_video.h"

/*
	Sprite Management
*/

typedef enum
{
	SPRITE_TYPE_DEFAULT,
	SPRITE_TYPE_FLARE		// No depth-test, scale by view and always fullbright.
} SpriteType_t;

typedef struct Sprite_s
{
	SpriteType_t	stType;	// Type of sprite (see SpriteType_t).

	vec3_t	vOrigin;

	vec4_t	vColour;		// Sprite colour.

    bool    bFullbright;    // If set to true, the sprite isn't effected by lighting.

	int		iTexture;		// Texture reference.

	float	fScale;			// Sprite scale.

	struct	Sprite_s	*sNext;
} Sprite_t;

Sprite_t	*sSprites,
			*sActiveSprites,
			*sFreeSprites;

/*  Allocate a new sprite.
*/
Sprite_t *Sprite_Allocate(void)
{
    Sprite_t    *sAllocated;

	if(!sFreeSprites)
		return NULL;

	sAllocated			= sFreeSprites;
	sFreeSprites		= sAllocated->sNext;
	sAllocated->sNext	= sActiveSprites;
	sActiveSprites		= sAllocated;

#if 0
	// [22/5/2014] Clear it out... ~hogsy
	memset(sAllocated,0,sizeof(sAllocated));
#else
	Math_Vector4Set(1.0f,sAllocated->vColour);
#endif

	return sAllocated;
}

void Sprite_Process(void)
{
	Sprite_t	*sSprite;

	if(!sActiveSprites)
		return;

	for(sSprite = sActiveSprites; sSprite; sSprite = sSprite->sNext)
	{
		if(!sSprite->bFullbright && (sSprite->stType != SPRITE_TYPE_FLARE))
			Math_MVToVector(Light_GetSample(sSprite->vOrigin),sSprite->vColour);

		switch(sSprite->stType)
		{
		case SPRITE_TYPE_DEFAULT:
			break;
		case SPRITE_TYPE_FLARE:
			// [22/5/2014] Scale by view ~hogsy
			sSprite->fScale *=
				(sSprite->vOrigin[0]-r_origin[0])*vpn[0]+
				(sSprite->vOrigin[1]-r_origin[1])*vpn[1]+
				(sSprite->vOrigin[2]-r_origin[2])*vpn[2];
		}
	}
}

void Sprite_Draw(entity_t *eEntity)
{
	Sprite_t	*sSprite;

    Video_ResetCapabilities(false);
	Video_DisableCapabilities(VIDEO_DEPTH_TEST);
	Video_EnableCapabilities(VIDEO_BLEND);

	for(sSprite = sActiveSprites; sSprite; sSprite = sSprite->sNext)
	{
		VideoObject_t voSprite[4];

		voSprite[0].vTextureCoord[0][0]	=
		voSprite[0].vTextureCoord[0][1]	= 0;
		voSprite[1].vTextureCoord[0][0]	= 1.0f;
		voSprite[1].vTextureCoord[0][1]	= 0;
		voSprite[2].vTextureCoord[0][0]	= 
		voSprite[2].vTextureCoord[0][1]	= 1.0f;
		voSprite[3].vTextureCoord[0][0]	= 0;
		voSprite[3].vTextureCoord[0][1]	= 1.0f;

		Math_Vector4Copy(sSprite->vColour,voSprite[0].vColour);
		Math_Vector4Copy(sSprite->vColour,voSprite[1].vColour);
		Math_Vector4Copy(sSprite->vColour,voSprite[2].vColour);
		Math_Vector4Copy(sSprite->vColour,voSprite[3].vColour);

		Math_VectorCopy(sSprite->vOrigin,voSprite[0].vVertex);
		Math_VectorMA(sSprite->vOrigin,sSprite->fScale,vup,voSprite[1].vVertex);
		Math_VectorMA(voSprite[1].vVertex,sSprite->fScale,vright,voSprite[2].vVertex);
		Math_VectorMA(sSprite->vOrigin,sSprite->fScale,vright,voSprite[3].vVertex);
		
		Video_DrawFill(voSprite);
	}

	Video_ResetCapabilities(true);
}
