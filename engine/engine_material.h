#ifndef __ENGINEMATERIAL__
#define __ENGINEMATERIAL__

#include "quakedef.h"

#define	MATERIAL_FLAG_SCROLLX
#define	MATERIAL_FLAG_SCROLLY

typedef enum
{
    MATERIAL_TYPE_NONE,
    MATERIAL_TYPE_METAL,
    MATERIAL_TYPE_BRICK,
    MATERIAL_TYPE_PLASTIC,
    MATERIAL_TYPE_WOOD,
    MATERIAL_TYPE_CONCRETE
} MaterialType_t;

typedef struct
{
    MaterialType_t  mType[MODEL_MAX_TEXTURES];  // The type of material.

	struct gltexture_s	*gDiffuseTexture[MODEL_MAX_TEXTURES],		// Diffuse texture.
						*gFullbrightTexture[MODEL_MAX_TEXTURES],	// Texture used for fullbright layer.
						*gSphereTexture[MODEL_MAX_TEXTURES];		// Texture used for sphere mapping.

	int iTextureWidth[MODEL_MAX_TEXTURES],iTextureHeight[MODEL_MAX_TEXTURES],	// Defines the width and height of each texture.
		iFlags[MODEL_MAX_TEXTURES];												// Various flags, such as scrolling and such.
} Material_t;

Material_t	*Material_Load(const char *ccPath);

#endif // __ENGINEMATERIAL__
