/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "quakedef.h"

#include "engine_script.h"

void Material_Initialize(void)
{
}

typedef struct
{
	char	*cKey;

	void	(*Function)(model_t *mModel,char *cArg);
} MaterialKey_t;

int	iMaterialCount;

void _Material_SetDiffuseTexture(model_t *mModel,char *cArg)
{
	byte *bDiffuseMap = Image_LoadImage(cArg,&mModel->iTextureWidth[iMaterialCount],&mModel->iTextureHeight[iMaterialCount]);
	if(bDiffuseMap)
		mModel->gDiffuseTexture[iMaterialCount] = TexMgr_LoadImage(
		mModel,
		cArg,
		mModel->iTextureWidth[iMaterialCount],
		mModel->iTextureHeight[iMaterialCount],
		SRC_RGBA,
		bDiffuseMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

void _Material_SetFullbrightTexture(model_t *mModel,char *cArg)
{
	byte *bFullbrightMap = Image_LoadImage(cArg,&mModel->iTextureWidth[iMaterialCount],&mModel->iTextureHeight[iMaterialCount]);
	if(bFullbrightMap)
		mModel->gFullbrightTexture[iMaterialCount] = TexMgr_LoadImage(
		mModel,
		cArg,
		mModel->iTextureWidth[iMaterialCount],
		mModel->iTextureHeight[iMaterialCount],
		SRC_RGBA,
		bFullbrightMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

void _Material_SetSphereTexture(model_t *mModel,char *cArg)
{
	byte *bSphereMap = Image_LoadImage(cArg,&mModel->iTextureWidth[iMaterialCount],&mModel->iTextureHeight[iMaterialCount]);
	if(bSphereMap)
		mModel->gSphereTexture[iMaterialCount] = TexMgr_LoadImage(
		mModel,
		cArg,
		mModel->iTextureWidth[iMaterialCount],
		mModel->iTextureHeight[iMaterialCount],
		SRC_RGBA,
		bSphereMap,
		cArg,
		0,
		TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

void _Material_AddSkin(model_t *mModel,char *cArg)
{
}

MaterialKey_t	mkMaterialKey[]=
{
	{	"AddSkin",		_Material_AddSkin				},	// Adds a new skin instance.
	{	"diffuse",		_Material_SetDiffuseTexture		},	// Sets the diffuse texture.
	{	"sphere",		_Material_SetSphereTexture		},	// Sets the spheremap texture.
	{	"fullbright",	_Material_SetFullbrightTexture	},	// Sets the fullbright texture.

	{	0	}
};

/*	Loads and parses material.
	Returns false on fail.
*/
bool Material_Load(model_t *mModel,const char *ccPath)
{
	char *cData;

	if(LoadFile(ccPath,(void**)&cData) == -1)
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

			// Set defaults...
			mModel->gDiffuseTexture[iMaterialCount]		= notexture;
			mModel->gSphereTexture[iMaterialCount]		= NULL;
			mModel->gFullbrightTexture[iMaterialCount]	= NULL;

			while(true)
			{
				if(!Script_GetToken(true))
				{
					Con_Warning("End of field without closing brace! (%s) (%i)\n",ccPath,iScriptLine);
					break;
				}
			}
		}
	}

	return false;
}