/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "quakedef.h"

#include "engine_videomaterial.h"

#include "engine_script.h"

bool	bInitialized = false;

Material_t	mMaterials[MATERIALS_MAX_ALLOCATED];	// Global array.

MaterialType_t	MaterialTypes[]=
{
	{	MATERIAL_TYPE_NONE,		"default"	},
	{	MATERIAL_TYPE_METAL,	"metal"		},
	{	MATERIAL_TYPE_GLASS,	"glass"		},
	{	MATERIAL_TYPE_CONCRETE,	"concrete"	},
	{	MATERIAL_TYPE_WOOD,		"wood"		},
	{	MATERIAL_TYPE_DIRT,		"dirt"		},
	{	MATERIAL_TYPE_RUBBER,	"rubber"	},
	{	MATERIAL_TYPE_WATER,	"water"		},
	{	MATERIAL_TYPE_FLESH,	"flesh"		},
	{	MATERIAL_TYPE_MUD,		"mud"		}
};

int	iMaterialCount = 0;

cvar_t	cvMaterialDraw = { "material_draw", "1", false, false, "Enables and disables the drawing of materials." };

Material_t *Material_Allocate(void);

void Material_Initialize(void)
{
	Material_t *mDummy;

	if(bInitialized)
		return;

	Con_Printf("Initializing material system...\n");

	Cvar_RegisterVariable(&cvMaterialDraw, NULL);

#if 0
	Con_DPrintf(" Allocating materials array... ");

	mMaterials = Hunk_AllocName(MATERIALS_MAX_ALLOCATED*sizeof(Material_t),"materials");
	if (!mMaterials)
		Sys_Error("Failed to allocate material array!\n");

	Con_DPrintf("DONE!\n");
#endif

	// Must be set to initialized before anything else.
	bInitialized = true;

	Con_DPrintf(" Creating dummy material... ");

	{
		// Add dummy material.
		mDummy = Material_Allocate();
		if (!mDummy)
			Sys_Error("Failed to create dummy material!\n");

#ifdef _MSC_VER // This is false, since the function above shuts us down, but MSC doesn't understand that.
#pragma warning(suppress: 6011)
#endif
		sprintf(mDummy->cName, "dummy");
		mDummy->iSkins							= 1;
		mDummy->iType							= MATERIAL_TYPE_NONE;
		mDummy->iFlags							= MATERIAL_FLAG_PRESERVE;
		mDummy->msSkin[0].gDiffuseTexture		= notexture;
		mDummy->msSkin[0].gFullbrightTexture	= NULL;
		mDummy->msSkin[0].gSphereTexture		= NULL;
		mDummy->msSkin[0].iTextureHeight		= notexture->height;
		mDummy->msSkin[0].iTextureWidth			= notexture->width;
	}

	Con_DPrintf("DONE!\n");
}

Material_t *Material_Allocate(void)
{
	int	i;

	// In the case of allocation, we go through the entire array.
	for (i = 0; i < MATERIALS_MAX_ALLOCATED; i++)
		if (!(mMaterials[i].iFlags & MATERIAL_FLAG_PRESERVE))
			if (!mMaterials[i].iIdentification || !mMaterials[i].iSkins)
			{
				//memset(mMaterial, 0, sizeof(*mMaterial));

				mMaterials[i].iIdentification = i + 1;

				iMaterialCount++;

				return &mMaterials[i];
			}

	return NULL;
}

MaterialSkin_t *Material_GetSkin(Material_t *mMaterial,int iSkin)
{
	// Don't let us spam the console; silly but whatever.
	static	int	iPasses = 0;

	if(iSkin < 0 || iSkin > MODEL_MAX_TEXTURES)
	{
		if (iPasses < 50)
		{
			Con_Warning("Invalid skin identification, should be greater than 0 and less than %i! (%i)\n", MODEL_MAX_TEXTURES, iSkin);
			iPasses++;
		}
		return NULL;
	}
	else if(!mMaterial)
	{
		if (iPasses < 50)
		{
			Con_Warning("Invalid material!\n");
			iPasses++;
		}
		return NULL;
	}
	else if(!mMaterial->iSkins)
	{
		if (iPasses < 50)
		{
			Con_Warning("Material with no valid skins! (%s)\n", mMaterial->cName);
			iPasses++;
		}
		return NULL;
	}
	else if (iSkin > (mMaterial->iSkins - 1))
	{
		if (iPasses < 50)
		{
			Con_Warning("Attempted to get an invalid skin! (%i) (%s)\n", iSkin, mMaterial->cName);
			iPasses++;
		}
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
		Con_Warning("Invalid material ID! (%i)\n",iMaterialID);
		return NULL;
	}

	for (i = 0; i < iMaterialCount; i++)
		if(mMaterials[i].iIdentification == iMaterialID)
			return &mMaterials[i];

	return NULL;
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

	for (i = 0; i < iMaterialCount; i++)
		// If the material has no name, then it's not valid.
		if (mMaterials[i].cName[0])
			if (!strncmp(mMaterials[i].cName, ccMaterialName, sizeof(mMaterials[i].cName)))
				return &mMaterials[i];

	return NULL;
}

Material_t *Material_GetByPath(const char *ccPath)
{
	int i;

	if(ccPath[0] == ' ')
	{
		Con_Warning("Attempted to find material, but recieved invalid path!\n");
		return NULL;
	}

	for (i = 0; i < iMaterialCount; i++)
		if (mMaterials[i].cPath[0])
			if (!strncmp(mMaterials[i].cPath, ccPath, sizeof(mMaterials[i].cPath)))
				return &mMaterials[i];

	return NULL;
}

extern cvar_t gl_fullbrights;

/*	Routine for applying each of our materials.
*/
void Material_Draw(Material_t *mMaterial, int iSkin, VideoObject_t *voObject, int iSize)
{
	int				i,iLayers = 1;
	MaterialSkin_t	*msCurrentSkin;

	if (!cvMaterialDraw.bValue)
		return;
	else if(!mMaterial)
	{
		Con_Warning("Invalid material, either not found or wasn't cached!\n");
		return;
	}
	// If we're drawing flat, then don't apply textures.
	else if(r_drawflat_cheatsafe)
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

#if 0	// TODO: Finish implementing this at some point... Meh ~hogsy
	if (msCurrentSkin->gSpecularTexture)
	{
		Video_SelectTexture(iLayers);
		Video_SetTexture(msCurrentSkin->gSpecularTexture);
		Video_EnableCapabilities(VIDEO_TEXTURE_2D|VIDEO_BLEND);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);

		iLayers++;
	}
#endif

	if(msCurrentSkin->gSphereTexture)
	{
		Video_SelectTexture(iLayers);
		Video_SetTexture(msCurrentSkin->gSphereTexture);
		Video_GenerateSphereCoordinates();
		Video_EnableCapabilities(VIDEO_TEXTURE_2D | VIDEO_BLEND | VIDEO_TEXTURE_GEN_S | VIDEO_TEXTURE_GEN_T);

#if 0
		if (msCurrentSkin->gSpecularTexture)
			Video_SetBlend(VIDEO_BLEND_ONE, VIDEO_DEPTH_IGNORE);
#endif

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

		iLayers++;
	}

	if(msCurrentSkin->gFullbrightTexture && gl_fullbrights.bValue)
	{
		Video_SelectTexture(iLayers);
		Video_EnableCapabilities(VIDEO_TEXTURE_2D|VIDEO_BLEND);
		Video_SetTexture(msCurrentSkin->gFullbrightTexture);

		for (i = 0; i < iSize; i++)
		{
			// Texture coordinates remain the same for fullbright layers.
			voObject[i].vTextureCoord[iLayers][0] = voObject[i].vTextureCoord[0][0];
			voObject[i].vTextureCoord[iLayers][1] = voObject[i].vTextureCoord[0][1];
		}

		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_ADD);

		iLayers++;
	}
}

/*
	Scripting
*/

void _Material_SetType(Material_t *mCurrentMaterial,char *cArg)
{
	int	iMaterialType = Q_atoi(cArg);

	// Ensure that the given type is valid.
	if((iMaterialType < MATERIAL_TYPE_NONE) || (iMaterialType >= MATERIAL_TYPE_MAX))
		Con_Warning("Invalid material type! (%i)\n",iMaterialType);

	mCurrentMaterial->iType = iMaterialType;
}

void _Material_SetDiffuseTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bDiffuseMap = Image_LoadImage(cArg,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureWidth,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureHeight);
	if (bDiffuseMap)
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].gDiffuseTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureWidth,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureHeight,
		SRC_RGBA,
		bDiffuseMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
	{
		Con_Warning("Failed to load texture %s!\n", cArg);

		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].gDiffuseTexture = notexture;
	}
}

void _Material_SetFullbrightTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bFullbrightMap = Image_LoadImage(cArg,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureWidth,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureHeight);
	if(bFullbrightMap)
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].gFullbrightTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureWidth,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureHeight,
		SRC_RGBA,
		bFullbrightMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
	{
		Con_Warning("Failed to load texture %s!\n", cArg);

		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].gFullbrightTexture = notexture;
	}
}

void _Material_SetSphereTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bSphereMap = Image_LoadImage(cArg,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureWidth,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureHeight);
	if(bSphereMap)
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].gSphereTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureWidth,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins-1].iTextureHeight,
		SRC_RGBA,
		bSphereMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
	{
		Con_Warning("Failed to load texture %s!\n", cArg);

		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].gSphereTexture = notexture;
	}
}

void _Material_SetSpecularTexture(Material_t *mCurrentMaterial, char *cArg)
{
	byte *bSpecularMap = Image_LoadImage(cArg,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].iTextureWidth,
		&mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].iTextureHeight);
	if (bSpecularMap)
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].gSpecularTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].iTextureWidth,
		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].iTextureHeight,
		SRC_RGBA,
		bSpecularMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
	{
		Con_Warning("Failed to load texture %s!\n", cArg);

		mCurrentMaterial->msSkin[mCurrentMaterial->iSkins - 1].gSpecularTexture = notexture;
	}
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
	{	"SetType",				_Material_SetType				},	// Sets the type of material.
	{	"SetDiffuseTexture",	_Material_SetDiffuseTexture		},	// Sets the diffuse texture.
	{ "SetSpecularTexture", _Material_SetSpecularTexture },		// Sets the specular map.
	{	"SetSphereTexture",		_Material_SetSphereTexture		},	// Sets the spheremap texture.
	{	"SetFullbrightTexture",	_Material_SetFullbrightTexture	},	// Sets the fullbright texture.
	{	"SetFlags",				_Material_SetFlags				},	// Sets seperate flags for the material; e.g. persist etc.

	{	0	}
};

/*	Loads and parses material.
	Returns false on complete failure.
*/
Material_t *Material_Load(/*const */char *ccPath)
{
    Material_t  *mNewMaterial;
	byte        *cData;
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
	
	cData = COM_LoadTempFile(cPath);
	if(!cData)
	{
		Con_Warning("Failed to load material! (%s) (%s)\n", cPath, ccPath);
		return false;
	}

	Script_StartTokenParsing((char*)cData);

	if(!Script_GetToken(true))
	{
		Con_Warning("Failed to get initial token! (%s) (%i)\n",ccPath,iScriptLine);
		return false;
	}
	else if(cToken[0] != '{')
	{
		Con_Warning("Missing '{'! (%s) (%i)\n",ccPath,iScriptLine);
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

		// End
		if(cToken[0] == '}')
			break;
		// Start
		else if(cToken[0] == '{')
		{
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

					// Find the related function.
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
					Con_Warning("Invalid field! (%s) (%i)\n", ccPath, iScriptLine);
					break;
				}
			}
		}
	}

	return mNewMaterial;
}

/*	Returns default dummy material.
*/
Material_t *Material_GetDummy(void)
{
	Material_t *mDummy;

	mDummy = Material_GetByName("dummy");
	if (!mDummy)
		Sys_Error("Failed to assign dummy material!\n");

	return mDummy;
}

/**/

void Material_Shutdown(void)
{
	Con_Printf("Shutting down material system...\n");
}