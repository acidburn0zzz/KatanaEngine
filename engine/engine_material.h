#ifndef __ENGINEMATERIAL__
#define __ENGINEMATERIAL__

#include "quakedef.h"

typedef struct
{
	int	iType;

	const char *ccName;
} MaterialType_t;

typedef struct
{
	struct gltexture_s	*gDiffuseTexture,		// Diffuse texture.
						*gFullbrightTexture,	// Texture used for fullbright layer.
						*gSphereTexture;		// Texture used for sphere mapping.

	int	iTextureWidth,iTextureHeight,	// Size of the skin.
		iType;							// The type of properties.
} MaterialSkin_t;

typedef struct
{
	int		iIdentification,
			iSkins;	// Number of skins provided by this material.

	char	*cPath,	// Path of the material.
			*cName;	// Name of the material.

	MaterialSkin_t	msSkin[MODEL_MAX_TEXTURES];
} Material_t;

Material_t	*mMaterials;	// Global pointer

void	Material_Draw(Material_t *mMaterial,int iSkin);

Material_t *Material_Load(const char *ccPath);

#endif // __ENGINEMATERIAL__
