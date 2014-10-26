/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "quakedef.h"

#include "engine_material.h"

#include "engine_script.h"
#include "engine_video.h"

bool	bInitialized = false;

int	iMaterialCount,
	iAllocatedMaterials	= 1024;

typedef enum
{
	MATERIAL_TYPE_NONE,
	MATERIAL_TYPE_METAL,
	MATERIAL_TYPE_GLASS,
	MATERIAL_TYPE_CONCRETE,
	MATERIAL_TYPE_GRASS,
	MATERIAL_TYPE_WOOD,
	MATERIAL_TYPE_WATER,
	MATERIAL_TYPE_DIRT,
	MATERIAL_TYPE_RUBBER,

#if 0
	MATERIAL_TYPE_CARPET,
	MATERIAL_TYPE_SNOW,

#endif // 0
};

MaterialType_t	MaterialTypes[]=
{
	{	MATERIAL_TYPE_NONE,		"default"	},
	{	MATERIAL_TYPE_METAL,	"metal"		},
	{	MATERIAL_TYPE_GLASS,	"glass"		},
	{	MATERIAL_TYPE_CONCRETE,	"concrete"	},
	{	MATERIAL_TYPE_WOOD,		"wood"		},
	{	MATERIAL_TYPE_DIRT,		"dirt"		},
	{	MATERIAL_TYPE_RUBBER,	"rubber"	},
	{	MATERIAL_TYPE_WATER,	"water"		}
};

Material_t	*mMaterials;	// Global array.

Material_t *Material_Allocate(void)
{
	Material_t *mAllocated;

	// TODO: Do stuff...
	
	mAllocated->iSkins = 0;

	return mAllocated;
}

void Material_Initialize(void)
{
	if(bInitialized)
		return;

	Con_Printf("Initializing material system...\n");

	mMaterials = Hunk_AllocName(iAllocatedMaterials*sizeof(Material_t),"materials");

	// Add dummy material.
	{
		Material_t *mDummy;

		mDummy = Material_Allocate();
		if(!mDummy)
			Sys_Error("Failed to allocated dummy material!\n");

		mDummy->iSkins = 1;
		mDummy->msSkin[0].gDiffuseTexture		= notexture;
		mDummy->msSkin[0].gFullbrightTexture	= NULL;
		mDummy->msSkin[0].gSphereTexture		= NULL;
		mDummy->msSkin[0].iTextureHeight		= 
		mDummy->msSkin[0].iTextureWidth			= 16;
		mDummy->msSkin[0].iType					= MATERIAL_TYPE_NONE;
	}

	bInitialized = true;
}

/*	Routine for applying each of our materials.
*/
void Material_Draw(Material_t *mMaterial,int iSkin)
{
	MaterialSkin_t	*msCurrentSkin;

	msCurrentSkin = Material_GetSkin(mMaterial,iSkin);
	if(!msCurrentSkin)
	{
		Video_SetTexture(notexture);
		return;
	}

	Video_SetTexture(msCurrentSkin->gDiffuseTexture);

	if(msCurrentSkin->gSphereTexture)
	{
		Video_EnableMultitexture();
		Video_GenerateSphereCoordinates();
		Video_SetTexture(msCurrentSkin->gSphereTexture);
		Video_EnableCapabilities(VIDEO_BLEND|VIDEO_TEXTURE_GEN_S|VIDEO_TEXTURE_GEN_T);

		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	}
	// Can't have both sphere and fullbright, it's one or the other.
	else if(msCurrentSkin->gFullbrightTexture)
	{
	}
}

MaterialSkin_t *Material_GetSkin(Material_t *mMaterial,int iSkin)
{
	if(!mMaterial)
	{
		Con_Warning("Invalid material!\n");
		return NULL;
	}
	else if(!mMaterial->iSkins)
	{
		Con_Warning("Material with no valid skins! (%s)\n",mMaterial->cName);
		return NULL;
	}
	else if((iSkin > mMaterial->iSkins) || (iSkin < mMaterial->iSkins))
	{
		Con_Warning("Attempted to get an invalid skin! (%i) (%s)\n",iSkin,mMaterial->cName);
		return NULL;
	}

	return &mMaterial->msSkin[iSkin];
}

/*	Returns a material from the given ID.
*/
Material_t *Material_Get(int iMaterialID)
{
	int i;

	if(iMaterialID < 0)
	{
		Con_Warning("Invalid material id! (%i)\n",iMaterialID);
		return NULL;
	}

	for(i = 0; i < sizeof(iMaterialCount); i++)
	{
	}

	// Return the material we want.
	return &mMaterials[i];
}

/*	Returns true on success.
	Unfinished
*/
Material_t *Material_GetByName(const char *ccMaterialName)
{
	int i;

	if(ccMaterialName[0] == ' ')
	{
		Con_Warning("Attempted to find material, but recieved invalid material name!\n");
		return NULL;
	}

	for(i = 0; i < sizeof(iMaterialCount); i++)
		if(!Q_strcmp(mMaterials[i].cName,ccMaterialName))
			return &mMaterials[i];

	return NULL;
}

/*
	Scripting
*/

void _Material_SetDiffuseTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bDiffuseMap = Image_LoadImage(cArg,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureWidth,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureHeight);
	if(bDiffuseMap)
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].gDiffuseTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureWidth,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureHeight,
		SRC_RGBA,
		bDiffuseMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

void _Material_SetFullbrightTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bFullbrightMap = Image_LoadImage(cArg,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureWidth,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureHeight);
	if(bFullbrightMap)
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].gFullbrightTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureWidth,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureHeight,
		SRC_RGBA,
		bFullbrightMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

void _Material_SetSphereTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bSphereMap = Image_LoadImage(cArg,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureWidth,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureHeight);
	if(bSphereMap)
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].gSphereTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureWidth,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins].iTextureHeight,
		SRC_RGBA,
		bSphereMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

typedef struct
{
	char	*cKey;

	void	(*Function)(Material_t *mCurrentMaterial,char *cArg);
} MaterialKey_t;

MaterialKey_t	mkMaterialFunctions[]=
{
	{	"diffuse",		_Material_SetDiffuseTexture		},	// Sets the diffuse texture.
	{	"sphere",		_Material_SetSphereTexture		},	// Sets the spheremap texture.
	{	"fullbright",	_Material_SetFullbrightTexture	},	// Sets the fullbright texture.

	{	0	}
};

/*	Loads and parses material.
	Returns false on complete failure.
*/
Material_t *Material_Load(const char *ccPath)
{
    Material_t  *mNewMaterial;
	int			iSkins = 0;
	char        *cData;

	if(!bInitialized)
	{
		Con_Warning("Attempted to load material, before initialization! (%s)\n",ccPath);
		return false;
	}
	else if(LoadFile(ccPath,(void**)&cData) == -1)
	{
		Con_Warning("Failed to load material! (%s)\n",ccPath);
		return false;
	}

	Script_StartTokenParsing(cData);

	if(!Script_GetToken(true))
	{
		Con_Warning("Failed to get initial token! (%s) (%i)\n",ccPath,iScriptLine);
		free(cData);
		return false;
	}
	else if(cToken[0] != '{')
	{
		Con_Warning("Missing '{'! (%s) (%i)\n",ccPath,iScriptLine);
		free(cData);
		return false;
	}

	mNewMaterial = Material_Allocate();

	while(true)
	{
		if(!Script_GetToken(true))
		{
			Con_Warning("End of field without closing brace! (%s) (%i)\n",ccPath,iScriptLine);
			break;
		}

		if(cToken[0] == '}')
			break;
		else if(cToken[0] == '{')
		{
			// do stuff...

			mNewMaterial->iSkins++;

			while(true)
			{
				if(!Script_GetToken(true))
				{
					Con_Warning("End of field without closing brace! (%s) (%i)\n",ccPath,iScriptLine);
					break;
				}

				if(cToken[0] == '}')
					break;
				// '$' declares that the following is a function.
				else if(cToken[0] == SCRIPT_SYMBOL_FUNCTION)
				{
					MaterialKey_t *mKey;

					for(mKey = mkMaterialFunctions; mKey->cKey; mKey++)
						// Remain case sensitive.
						if(!Q_strcasecmp(mKey->cKey,cToken+1))
						{
							Script_GetToken(false);

							mKey->Function(mNewMaterial,cToken);
							break;
						}
				}
				// '%' declares that the following is a function.
				else if(cToken[0] == SCRIPT_SYMBOL_VARIABLE)
				{
					/*	TODO:
							* Collect variable
							* Check it against internal solutions
							* Otherwise declare it, figure out where/how it's used
					*/
				}
				else
				{
					Con_Warning("Invalid function call!\n");
					break;
				}

				Con_Warning("Invalid field! (%s) (%i)\n",ccPath,iScriptLine);
			}
		}
	}

	return NULL;
}

/**/

void Material_Shutdown(void)
{
	Con_Printf("Shutting down material system...\n");
}