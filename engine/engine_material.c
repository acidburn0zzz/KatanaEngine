/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_material.h"

#include "engine_script.h"

int	iMaterialCount;

Material_t *Material_Allocate(void)
{
	Material_t *mAllocated;

	// TODO: Do stuff...

	return mAllocated;
}

void _Material_SetDiffuseTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bDiffuseMap = Image_LoadImage(cArg,&mCurrentMaterial->iTextureWidth,&mCurrentMaterial->iTextureHeight);
	if(bDiffuseMap)
		mCurrentMaterial->gDiffuseTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->iTextureWidth,
		mCurrentMaterial->iTextureHeight,
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
	byte *bFullbrightMap = Image_LoadImage(cArg,&mCurrentMaterial->iTextureWidth,&mCurrentMaterial->iTextureHeight);
	if(bFullbrightMap)
		mCurrentMaterial->gFullbrightTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->iTextureWidth,
		mCurrentMaterial->iTextureHeight,
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
	byte *bSphereMap = Image_LoadImage(cArg,&mCurrentMaterial->iTextureWidth,&mCurrentMaterial->iTextureHeight);
	if(bSphereMap)
		mCurrentMaterial->gSphereTexture = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->iTextureWidth,
		mCurrentMaterial->iTextureHeight,
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
Material_t *Material_Load(model_t *mModel,const char *ccPath)
{
    Material_t  *mNewMaterial;
	char        *cData;

	if(LoadFile(ccPath,(void**)&cData) == -1)
	{
		Con_Warning("Failed to load material! (%s)\n",ccPath);
		return false;
	}

    // Reset the global material count.
	iMaterialCount = -1;

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
			Material_t *mNewMaterial;

			iMaterialCount++;
			if(iMaterialCount > MODEL_MAX_TEXTURES)
			{
                Con_Warning("Reached max number of allowed materials!\n");
				// Return true, since we haven't entirely failed.
                return true;
			}

#if 0
			mNewMaterial = Material_Allocate();
			if(!mNewMaterial)
			{
				Con_Warning("Failed to allocate material! (%s) (%i)\n",ccPath,iMaterialCount);

				// If we've managed to load a material already, then we haven't entirely failed.
				if(iMaterialCount > 0)
					return true;
				
				// Otherwise if it's our first, then we've failed.
				return false;
			}
#endif

			// Set defaults...
			mModel->mMaterials[iMaterialCount].gDiffuseTexture		= notexture;
			mModel->mMaterials[iMaterialCount].gFullbrightTexture	= NULL;
			mModel->mMaterials[iMaterialCount].gSphereTexture		= NULL;

			while(true)
			{
				if(!Script_GetToken(true))
				{
					Con_Warning("End of field without closing brace! (%s) (%i)\n",ccPath,iScriptLine);
					break;
				}

				// '$' declares that the following is a function.
				if(cToken[0] == '$')
				{
					MaterialKey_t *mKey;

					for(mKey = mkMaterialFunctions; mKey->cKey; mKey++)
						// Remain case sensitive.
						if(!Q_strcasecmp(mKey->cKey,cToken+1))
						{
							Script_GetToken(false);

							mKey->Function(&mModel->mMaterials[iMaterialCount],cToken);
							break;
						}
				}
				// '%' declares that the following is a function.
				else if(cToken[0] == '%')
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
