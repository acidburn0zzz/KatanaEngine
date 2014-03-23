#include "menu_main.h"

#include "menu_script.h"

#if 0
// [7/2/2013] Include our squirrel headers ~hogsy
#include <squirrel3\squirrel.h>
#include <squirrel3\sqstdio.h>
#include <squirrel3\sqstdaux.h>
#include <squirrel3\sqstdsystem.h>
#include <squirrel3\sqstdmath.h>
#include <squirrel3\sqstdstring.h>

void Script_PrintMessage(HSQUIRRELVM hSquirrel,const SQChar *sString,...)
{
	Engine.Con_Printf((char*)sString);
}

void Script_PrintWarning(HSQUIRRELVM hSquirrel,const SQChar *sString,...)
{
	Engine.Con_Warning((char*)sString);
}

void Script_CallFunction(char *cFunction)
{
}

SQInteger Script_RegisterFunction(HSQUIRRELVM hSquirrel,SQFUNCTION sFunction,const char *cFunctionName)
{
	sq_pushroottable(hSquirrel);
	sq_pushstring(hSquirrel,cFunctionName,-1);
	sq_newclosure(hSquirrel,sFunction,0);
	sq_newslot(hSquirrel,-3,false);
	sq_pop(hSquirrel,1);
}

void Script_Initialize(void)
{
	HSQUIRRELVM	hSquirrel = sq_open(1024);

	sq_pushroottable(hSquirrel);
	sqstd_register_iolib(hSquirrel);
	sqstd_register_systemlib(hSquirrel);
	sqstd_register_stringlib(hSquirrel);
	sqstd_register_mathlib(hSquirrel);
	sq_pop(hSquirrel,1);

	sqstd_seterrorhandlers(hSquirrel);

	sq_setprintfunc(hSquirrel,Script_PrintMessage,Script_PrintWarning);
	sq_pushroottable(hSquirrel);

	if(SQ_SUCCEEDED(sqstd_dofile(hSquirrel,_SC("data/scripts/menu.nut"),false,true)))
	{
		SQInteger	sTop = sq_gettop(hSquirrel);

		sq_pushroottable(hSquirrel);
		sq_pushstring(hSquirrel,_SC("Initialize"),-1);

		if(SQ_SUCCEEDED(sq_get(hSquirrel,-2)))
		{
			sq_pushroottable(hSquirrel);
			sq_call(hSquirrel,0,FALSE,TRUE);
		}

		sq_settop(hSquirrel,sTop);
	}
	else
		Engine.Con_Warning("Failed to load script!\n");

	sq_pop(hSquirrel,1);
	sq_close(hSquirrel);
}
#endif