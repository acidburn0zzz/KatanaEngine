/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_script.h"

/*
	Python scripting system that's used for both
	game-logic additions and for the creation
	of menu elements.
	TODO:
		Move the following out into its own shared module.
*/

#include "engine_modgame.h"

#if 0
#include "Python.h"

bool bInitialized = false;

static PyObject *Script_SystemError(PyObject *poSelf,PyObject *poArguments)
{
}

/*
	Server Functions.
*/

/*
	Client Functions.
*/

/**/

static struct PyMethodDef pmdKatanaMethods[]=
{
	{
		"SystemError",	Script_SystemError,	METH_VARARGS,
		"Causes the engine to give a system error."
	},

	{	NULL,	NULL	}
};

/*	Initialize scripting system.
*/
void Script_Initialize(void)
{
	PyObject *poModule;

	if(bInitialized)
		return;

	Con_Printf("Initializing python...\n");

	Py_SetProgramName(com_argv[0]);
	Py_Initialize();

	poModule = Py_InitModule("Katana",pmdKatanaMethods);
	if(!poModule)
		Sys_Error("Failed to initialize python module!\n");

	bInitialized = true;
}

/*	Execute a script.
*/
void Script_Execute(char *ccPath)
{
	byte *bScriptFile;

	if(!bInitialized)
		return;

	bScriptFile = COM_LoadTempFile(ccPath);
	if(!bScriptFile)
	{
		Con_Warning("Failed to load script! (%s)\n",ccPath);
		return;
	}

	PyRun_SimpleFile((FILE*)bScriptFile,ccPath);
}

/*	Shutdown scripting system.
*/
void Script_Shutdown(void)
{
	if(!bInitialized)
		return;

	if(Py_IsInitialized())
		Py_Finalize();

	bInitialized = false;
}
#endif

/*
	Old script stuff...
*/

/*
	Script Library
*/

char	cToken[MAXTOKEN],
		*cScriptParse;

int		iScriptLine;

void Script_StartTokenParsing(char *cData)
{
	iScriptLine		= 1;
	cScriptParse	= cData;
}

bool Script_GetToken(bool bCrossline)
{
	char *cTokenParse;

SKIPSPACE:
	while(*cScriptParse <= 32)
	{
		if(!*cScriptParse)
		{
			if(!bCrossline)
				Error("Line is incomplete! (%i)",iScriptLine);

			return false;
		}

		if(*cScriptParse++ == '\n')
		{
			if(!bCrossline)
				Error("Line is incomplete! (%i)",iScriptLine);

			iScriptLine++;
		}
	}

	// [2/4/2013] Standard comments ~hogsy
	if(cScriptParse[0] == '/' && cScriptParse[1] == '/')
	{
		if(bCrossline)
			Error("Line is incomplete! (%i)",iScriptLine);

		while(*cScriptParse++ != '\n')
			if(!*cScriptParse)
			{
				if(!bCrossline)
					Error("Line is incomplete! (%i)",iScriptLine);

				return false;
			}

		goto SKIPSPACE;
	}
	// [2/4/2013] Multiple line comments ~hogsy
	else if((cScriptParse[0] == '/') && (cScriptParse[1] == '*'))
	{
		if(bCrossline)
			Error("Line is incomplete! (%i)",iScriptLine);

		while((*cScriptParse++ != '*'))	//cScriptParse[0] && !(cScriptParse[0] == '*' && cScriptParse[1] == '/'))
			if(!*cScriptParse)
			{
				if(!bCrossline)
					Error("Line is incomplete! (%i)",iScriptLine);

				return false;
			}

		goto SKIPSPACE;
	}

	cTokenParse = cToken;

	if(*cScriptParse == '"')
	{
		cScriptParse++;
		while(*cScriptParse != '"')
		{
			if(!*cScriptParse)
				Error("End of field inside quoted token! (%i)",iScriptLine);

			*cTokenParse++ = *cScriptParse++;

			if(cTokenParse == &cToken[MAXTOKEN])
				Error("Token too large! (%i)\n",iScriptLine);
		}

		cScriptParse++;
	}
	else if(*cScriptParse == '(')
	{
		cScriptParse++;
		cScriptParse++;
	}
	else while(*cScriptParse > 32)
	{
		*cTokenParse++ = *cScriptParse++;
		if(cTokenParse == &cToken[MAXTOKEN])
			Error("Token too large! (%i)",iScriptLine);
	}

	*cTokenParse = 0;

	return true;
}

bool Script_TokenAvaliable(void)
{
	char	*cSearchParse;

	cSearchParse = cScriptParse;
	while(*cSearchParse <= 32)
	{
		if(*cSearchParse == '\n' || *cSearchParse == 0)
			return false;

		cSearchParse++;
	}

	if(*cSearchParse == ';')
		return false;

	return true;
}

// [18/5/2012] Code by itsme86 @ http://www.linuxquestions.org ~hogsy
// [11/5/2013] Added fixes by paulhope123 ~hogsy
// [3/7/2013] Revised ~hogsy
// [6/7/2013] Cleaned up ~hogsy
char *Script_UpdateString(char *cString,const char *cFind,const char *cReplace)
{
	static char scBuffer[4096];
	char		*p;

	p = strstr(cString,cFind);
	if(!p)
		return cString;

	if(strlen(cFind) >= strlen(cReplace))
	{
		strncpy(scBuffer,cString,p-cString);

		scBuffer[p-cString]	= '\0';
	}
	else
	{
		strncpy(scBuffer,cString,p-cString+strlen(cFind));

		scBuffer[p-cString+strlen(cFind)] = '\0';
	}

	sprintf(scBuffer+(p-cString),"%s%s",cReplace,p+strlen(cFind));

	return scBuffer;
}

/*
    New stuff
*/

typedef struct
{
	char	*cKey;

	void	(*Function)(char *cArg);
} ScriptKey_t;

void _FileSystem_SetBasePath(char *cArg);		// common.c
void _FileSystem_SetTexturePath(char *cArg);
void _FileSystem_SetMaterialPath(char *cArg);
void _FileSystem_AddGameDirectory(char *cArg);	// common.c

/*	Allows for console variables to be set from scripts.
*/
void _Script_SetConsoleVariable(char *cArguments)
{
}

ScriptKey_t	skScriptKeys[]=
{
	{	"basepath",			_FileSystem_SetBasePath			},
	{	"SetTexturePath",	_FileSystem_SetTexturePath		},
	{	"SetMaterialPath",	_FileSystem_SetMaterialPath		},
	{	"AddGameDirectory",	_FileSystem_AddGameDirectory	},

	{	0	}
};

/*	Loads a script.
*/
bool Script_Load(/*const */char *ccPath)
{
	char *cData;

	if(!LoadFile(ccPath,(void**)&cData))
	{
		cData = (char*)COM_LoadTempFile(ccPath);
		if(!cData)
		{
			Con_Warning("Failed to load script! (%s)\n",ccPath);
			return false;
		}
	}

	Script_StartTokenParsing(cData);

	if(!Script_GetToken(true))
	{
		Con_Warning("Failed to get initial token! (%s) (%i)\n",ccPath,iScriptLine);
		free(cData);
		return false;
	}
	else if(Q_strcmp(cToken,"{"))
	{
		Con_Warning("Missing '{'! (%s) (%i)\n",ccPath,iScriptLine);
		free(cData);
		return false;
	}

	do
	{
		if(!Script_GetToken(true))
		{
			Con_Warning("End of field without closing brace! (%s) (%i)\n",ccPath,iScriptLine);
			break;
		}
		else if(!Q_strcmp(cToken,"}"))
			break;
		else if(Q_strcmp(cToken,"{"))
		{
			ScriptKey_t	*sKey;

			// '$' declares that the following is a function.
			if(cToken[0] == SCRIPT_SYMBOL_FUNCTION)
			{
				for(sKey = skScriptKeys; sKey->cKey; sKey++)
					// Remain case sensitive.
					if(!Q_strcasecmp(sKey->cKey,cToken+1))
					{
						Script_GetToken(false);

						sKey->Function(cToken);
						break;
					}
			}
			else
			{
				Con_Warning("Invalid function call!\n");
				break;
			}

			Con_Warning("Invalid field! (%s) (%i)\n",ccPath,iScriptLine);
		}
	} while(true);

	free(cData);

	return true;
}