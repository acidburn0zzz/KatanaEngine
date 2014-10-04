/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_material.h"

#include "engine_script.h"

int	iMaterialCount;

void _Material_SetDiffuseTexture(Material_t *mCurrentMaterial,char *cArg)
{
	byte *bDiffuseMap = Image_LoadImage(cArg,&mCurrentMaterial->iTextureWidth[iMaterialCount],&mCurrentMaterial->iTextureHeight[iMaterialCount]);
	if(bDiffuseMap)
		mCurrentMaterial->gDiffuseTexture[iMaterialCount] = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->iTextureWidth[iMaterialCount],
		mCurrentMaterial->iTextureHeight[iMaterialCount],
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
	byte *bFullbrightMap = Image_LoadImage(cArg,&mCurrentMaterial->iTextureWidth[iMaterialCount],&mCurrentMaterial->iTextureHeight[iMaterialCount]);
	if(bFullbrightMap)
		mCurrentMaterial->gFullbrightTexture[iMaterialCount] = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->iTextureWidth[iMaterialCount],
		mCurrentMaterial->iTextureHeight[iMaterialCount],
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
	byte *bSphereMap = Image_LoadImage(cArg,&mCurrentMaterial->iTextureWidth[iMaterialCount],&mCurrentMaterial->iTextureHeight[iMaterialCount]);
	if(bSphereMap)
		mCurrentMaterial->gSphereTexture[iMaterialCount] = TexMgr_LoadImage(
		NULL,
		cArg,
		mCurrentMaterial->iTextureWidth[iMaterialCount],
		mCurrentMaterial->iTextureHeight[iMaterialCount],
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

	void	(*Function)(model_t *mModel,char *cArg);
} MaterialKey_t;

MaterialKey_t	mkMaterialKey[]=
{
	{	"diffuse",		_Material_SetDiffuseTexture		},	// Sets the diffuse texture.
	{	"sphere",		_Material_SetSphereTexture		},	// Sets the spheremap texture.
	{	"fullbright",	_Material_SetFullbrightTexture	},	// Sets the fullbright texture.

	{	0	}
};

/*	Loads and parses material.
	Returns false on fail.
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
			iMaterialCount++;
			if(iMaterialCount > MODEL_MAX_TEXTURES)
			{
                Con_Warning("Reached max number of allowed materials!\n");
                return true;
			}

			// Set defaults...
			mModel->gDiffuseTexture[iMaterialCount]		= notexture;
			mModel->gSphereTexture[iMaterialCount]		= NULL;
			mModel->gFullbrightTexture[iMaterialCount]	= NULL;

			//while(true)
			//{
            if(!Script_GetToken(true))
            {
                Con_Warning("End of field without closing brace! (%s) (%i)\n",ccPath,iScriptLine);
                break;
            }

			// '$' declares that the following is a function.
			if(cToken[0] == '$')
			{
                MaterialKey_t *mKey;

				for(mKey = skScriptKeys; mKey->cKey; mKey++)
					// Remain case sensitive.
					if(!Q_strcasecmp(mKey->cKey,cToken+1))
					{
						Script_GetToken(false);

						mKey->Function(cToken);
						break;
					}
			}
			else
			{
				Con_Warning("Invalid function call!\n");
				break;
			}

			Con_Warning("Invalid field! (%s) (%i)\n",ccPath,iScriptLine);
			//}
		}
	}

	return NULL;
}
