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

MathVector_t Loader_CalculateNormals(vec3_t	vTriangle)
{
	MathVector_t	mvOutput[1];

/*
	Begin Function CalculateSurfaceNormal (Input Triangle) Returns Vector
 
        Set Vector U to (Triangle.p2 minus Triangle.p1)
        Set Vector V to (Triangle.p3 minus Triangle.p1)
 
        Set Normal.x to (multiply U.y by V.z) minus (multiply U.z by V.y)
        Set Normal.y to (multiply U.z by V.x) minus (multiply U.x by V.z)
        Set Normal.z to (multiply U.x by V.y) minus (multiply U.y by V.x)
 
        Returning Normal
 
	End Function
*/

	return mvOutput;
}

void Loader_UnloadModel()
{
}
#endif