#include "engine_loader.h"

/*
	ENGINE LOADER
	Unfinished replacement for gl_model.
*/

#if 0
model_t *Loader_LoadModel(LoaderType_t ltFileType,char *ccPath)
{
	unsigned	*uBuffer;
	char		cOutName[MAX_OSPATH];
	byte		bStackBuffer[1024];
	model_t		*mModel;

	uBuffer = (unsigned*)COM_LoadStackFile(ccPath,bStackBuffer,sizeof(bStackBuffer));
	if(!uBuffer)
	{
		Con_Warning("Failed to find file! (%s)\n",ccPath);

		return NULL;
	}

	COM_FileBase(ccPath,cOutName);

	switch(ltFileType)
	{
	case LOADERTYPE_LMPO:
		break;
	case LOADERTYPE_MAP:
		break;
	default:
		Con_Warning("Unknown file type! (%i) (%s)\n",ltFileType,ccPath);
	}

	return mModel;
}

void Loader_UnloadModel()
{
}
#endif