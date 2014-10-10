#ifndef __ENGINEMATERIAL__
#define __ENGINEMATERIAL__

#include "quakedef.h"

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
    MaterialType_t  mType;  // The type of material.

	struct gltexture_s	*gDiffuseTexture,		// Diffuse texture.
						*gFullbrightTexture,	// Texture used for fullbright layer.
						*gSphereTexture;		// Texture used for sphere mapping.

	int iTextureWidth,
		iTextureHeight;
} Material_t;

Material_t *Material_Load(model_t *mModel,const char *ccPath);

#endif // __ENGINEMATERIAL__
