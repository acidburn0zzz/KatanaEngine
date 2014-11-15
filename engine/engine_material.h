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
	struct gltexture_s	*gDiffuseTexture,			// Diffuse texture.
						*gFullbrightTexture,		// Texture used for fullbright layer.
						*gSphereTexture;			// Texture used for sphere mapping.

	unsigned int	iTextureWidth,iTextureHeight;	// Size of the skin.
} MaterialSkin_t;

typedef struct
{
	int		iIdentification,
			iType,				// The type of properties.
			iSkins;				// Number of skins provided by this material.

	char	*cPath,	// Path of the material.
			*cName;	// Name of the material.

	MaterialSkin_t	msSkin[MODEL_MAX_TEXTURES];
} Material_t;

Material_t	*mMaterials;	// Global pointer

void	Material_Initialize(void);
void	Material_Draw(Material_t *mMaterial,int iSkin);

Material_t *Material_Load(const char *ccPath);
Material_t *Material_Get(int iMaterialID);
Material_t *Material_GetByPath(const char *ccPath);

#endif // __ENGINEMATERIAL__
