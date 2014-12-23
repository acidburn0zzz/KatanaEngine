#ifndef __ENGINEMATERIAL__
#define __ENGINEMATERIAL__

#include "quakedef.h"

typedef enum
{
	MATERIAL_TYPE_NONE,		// No assigned property, used as default.
	MATERIAL_TYPE_METAL,	// Metal property.
	MATERIAL_TYPE_GLASS,	// Glass property.
	MATERIAL_TYPE_CONCRETE,	// Concrete property.
	MATERIAL_TYPE_GRASS,	// Grass property.
	MATERIAL_TYPE_WOOD,		// Wood property.
	MATERIAL_TYPE_WATER,	// Water property.
	MATERIAL_TYPE_DIRT,		// Dirt property.
	MATERIAL_TYPE_MUD,		// Mud property.
	MATERIAL_TYPE_RUBBER,	// Rubber property.
	MATERIAL_TYPE_FLESH,	// Flesh property.

#if 0
	MATERIAL_TYPE_CARPET,	// Carpet property.
	MATERIAL_TYPE_SNOW,		// Snow property.
#endif // 0

	MATERIAL_TYPE_MAX		// This isn't valid, don't use it (just used for utility).
} MaterialProperty_t;

#define	MATERIAL_FLAG_PRESERVE	1	// Preserves the material during clear outs.

typedef struct
{
	MaterialProperty_t	iType;

	const char *ccName;
} MaterialType_t;

typedef struct
{
	struct gltexture_s	*gDiffuseTexture,		// Diffuse texture.
						*gFullbrightTexture,	// Texture used for fullbright layer.
						*gSpecularTexture,		// Texture used for specular map.
						*gSphereTexture;		// Texture used for sphere mapping.

	unsigned int	iTextureWidth,iTextureHeight;	// Size of the skin.
} MaterialSkin_t;

typedef struct
{
	int		iIdentification,
			iFlags,				// Flags for the material.
			iType,				// The type of properties.
			iSkins;				// Number of skins provided by this material.

	char	cPath[PLATFORM_MAX_PATH],	// Path of the material.
			cName[128];					// Name of the material.

	MaterialSkin_t	msSkin[MODEL_MAX_TEXTURES];
} Material_t;

#define	MATERIALS_MAX_ALLOCATED	2048

extern Material_t	mMaterials[MATERIALS_MAX_ALLOCATED];	// Global array.

void	Material_Initialize(void);
void	Material_Draw(Material_t *mMaterial,int iSkin);

Material_t *Material_Load(/*const */char *ccPath);
Material_t *Material_Get(int iMaterialID);
Material_t *Material_GetByPath(const char *ccPath);
Material_t *Material_GetDummy(void);

#endif // __ENGINEMATERIAL__
