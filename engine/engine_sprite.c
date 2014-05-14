/*	Copyright (C) 2011-2014 OldTimes Software
*/

#include "engine_video.h"

/*
	Sprite Management
*/

typedef struct
{
    bool    bFullbright;    // If set to true, the sprite isn't effected by lighting.
} Sprite_t;

Sprite_t *sSprites;

/*  Allocate a new sprite.
*/
Sprite_t *Sprite_Allocate()
{
    Sprite_t    *sAllocated;

}

void Sprite_Process(void)
{}

void Sprite_Draw(entity_t *eEntity)
{
	VideoObject_t	voSprite[4];

    Video_ResetCapabilities(false);

	voSprite[0].vTextureCoord[0][0]	= 0;

	Video_DrawFill(voSprite);

	Video_ResetCapabilities(true);
}
