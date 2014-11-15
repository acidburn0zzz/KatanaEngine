/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "quakedef.h"

#include "engine_material.h"

#include "engine_script.h"
#include "engine_video.h"

bool	bInitialized = false;

#define	MATERIALS_MAX_ALLOCATED	2048

int	iMaterialCount;

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
	int			i;
	Material_t	*mAllocated;

	// TODO: Do stuff...
	for(i = 0; i < MATERIALS_MAX_ALLOCATED; i++)
	{
		if(mMaterials[i].iIdentification <= 0)
			break;

	}

	mAllocated = &mMaterials[i];
	if(!mAllocated)
	{
		Con_Warning("Failed to allocate material!\n");
		return NULL;
	}
	
	mAllocated->iIdentification = i;
	mAllocated->iSkins			= 0;

	return mAllocated;
}

void Material_Initialize(void)
{
	Material_t *mDummy;

	if(bInitialized)
		return;

	Con_Printf("Initializing material system...\n");

	mMaterials = Hunk_AllocName(MATERIALS_MAX_ALLOCATED*sizeof(Material_t),"materials");

	bInitialized = true;

	// Add dummy material.
	mDummy = Material_Allocate();
	if(!mDummy)
		Sys_Error("Failed to create dummy material!\n");

#ifdef _MSC_VER // This is false, since the function above shuts us down, but MSC doesn't understand that.
#pragma warning(suppress: 6011)
#endif
	mDummy->cName							= "dummy";
	mDummy->cPath							= "";
	mDummy->iSkins							= 1;
	mDummy->iType							= MATERIAL_TYPE_NONE;
	mDummy->msSkin[0].gDiffuseTexture		= notexture;
	mDummy->msSkin[0].gFullbrightTexture	= NULL;
	mDummy->msSkin[0].gSphereTexture		= NULL;
	mDummy->msSkin[0].iTextureHeight		= notexture->height;
	mDummy->msSkin[0].iTextureWidth			= notexture->width;
}

MaterialSkin_t *Material_GetSkin(Material_t *mMaterial,int iSkin)
{
	if(iSkin <= 0 || iSkin > MODEL_MAX_TEXTURES)
	{
		Con_Warning("Invalid skin identification, should be greater than 0 and less than %i! (%i)\n",MODEL_MAX_TEXTURES,iSkin);
		return NULL;
	}
	else if(!mMaterial)
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

	// The identification would never be less than 0, and never more than our maximum.
	if(iMaterialID < 0 || iMaterialID > MATERIALS_MAX_ALLOCATED)
	{
		Con_Warning("Invalid material id! (%i)\n",iMaterialID);
		return NULL;
	}

	for(i = 0; i < MATERIALS_MAX_ALLOCATED; i++)
		if(mMaterials[i].iIdentification == iMaterialID)
			return &mMaterials[i];

	return NULL;
}

/*	Returns true on success.
	Unfinished
*/
Material_t *Material_GetByName(const char *ccMaterialName)
{
	Material_t	*mMaterial;

	if(ccMaterialName[0] == ' ')
	{
		Con_Warning("Attempted to find material, but recieved invalid material name!\n");
		return NULL;
	}

	for(mMaterial = mMaterials; mMaterial; mMaterial++)
	{
		// If the material has no name, then it's not valid.
		if(mMaterial->cName[0] != ' ')
			if(!strncmp(mMaterial->cName,ccMaterialName,sizeof(mMaterial->cName)))
				return mMaterial;
	}

	return NULL;
}

Material_t *Material_GetByPath(const char *ccPath)
{
	Material_t	*mMaterial;

	if(ccPath[0] == ' ')
	{
		Con_Warning("Attempted to find material, but recieved invalid path!\n");
		return NULL;
	}

	for(mMaterial = mMaterials; mMaterial; mMaterial++)
	{
		if(mMaterial->cPath[0] != ' ')
			if(!strncmp(mMaterial->cPath,ccPath,sizeof(mMaterial->cPath)))
				return mMaterial;
	}

	return NULL;
}

extern cvar_t gl_fullbrights;

/*	Routine for applying each of our materials.
*/
void Material_Draw(Material_t *mMaterial,int iSkin)
{
	MaterialSkin_t	*msCurrentSkin;

	if(!mMaterial)
	{
		Con_Warning("Invalid material, either not found or wasn't cached!\n");
		return;
	}

	// If we're drawing flat, then don't apply textures.
	if(r_drawflat_cheatsafe)
	{
		Video_DisableCapabilities(VIDEO_TEXTURE_2D);
		return;
	}

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
	else if(msCurrentSkin->gFullbrightTexture && gl_fullbrights.bValue)
	{
		Video_EnableMultitexture();
        Video_EnableCapabilities(VIDEO_BLEND);
		Video_SetTexture(msCurrentSkin->gFullbrightTexture);

		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_ADD);
	}
}

/*
	Scripting
*/

void _Material_SetType(Material_t *mCurrentMaterial,char *cArg)
{
	int	iMaterialType = Q_atoi(cArg);

	if(iMaterialType < MATERIAL_TYPE_NONE)
		Con_Warning("Invalid material type! (%i)\n",iMaterialType);

	mCurrentMaterial->iType = iMaterialType;
}

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

void _Material_SetFlags(Material_t *mCurrentMaterial,char *cArg)
{
}

typedef struct
{
	char	*cKey;

	void	(*Function)(Material_t *mCurrentMaterial,char *cArg);
} MaterialKey_t;

MaterialKey_t	mkMaterialFunctions[]=
{
	{	"type",			_Material_SetType				},	// Sets the type of material.
	{	"diffuse",		_Material_SetDiffuseTexture		},	// Sets the diffuse texture.
	{	"sphere",		_Material_SetSphereTexture		},	// Sets the spheremap texture.
	{	"fullbright",	_Material_SetFullbrightTexture	},	// Sets the fullbright texture.
	{	"SetFlags",		_Material_SetFlags				},	// Sets seperate flags for the material; e.g. persist etc.

	{	0	}
};

/*	Loads and parses material.
	Returns false on complete failure.
*/
Material_t *Material_Load(const char *ccPath)
{
    Material_t  *mNewMaterial;
	void        *cData;
	char		cPath[PLATFORM_MAX_PATH];

	if(!bInitialized)
	{
		Con_Warning("Attempted to load material, before initialization! (%s)\n",ccPath);
		return false;
	}

	// Update the given path with the base path plus extension.
	sprintf(cPath,"%s%s.material",Global.cMaterialPath,ccPath);

	// Check if it's been cached already...
	mNewMaterial = Material_GetByPath(cPath);
	if(mNewMaterial)
		return mNewMaterial;

	if(LoadFile(cPath,&cData) == -1)
	{
		Con_Warning("Failed to load material! (%s)\n",ccPath);
		return false;
	}

	Script_StartTokenParsing((char*)cData);

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

	// Assume that the material hasn't been cached yet, so allocate a new copy of one.
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

	free(cData);

	return NULL;
}

/**/

void Material_Shutdown(void)
{
	Con_Printf("Shutting down material system...\n");
}