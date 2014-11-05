/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "editor_main.h"

/*
	Launches the editor interface.
*/

// Platform Library
#include "platform_module.h"
#include "platform_window.h"

// Shared Library
#include "shared_flags.h"
#include "shared_module.h"
#include "shared_engine.h"

EngineExport_t	*Engine;
EngineImport_t	*Editor;

pINSTANCE hEngineInstance;

/*	TODO:
		Handle problems as gracefully as possible.

		List of things I want access to from the engine...
			Engine->GetGamePath()			(Returns current game path)
			Engine->CreateEntity()			(Returns an entity instance)
			Engine->LoadMap(file)			(Loads the specified level)
			Engine->CreateBrush(xyz,w,h)	(Creates a brush)
			Engine->CreateWindow(x,y,w,h)	(Creates a new window... Need to handle different window instances somehow... Sigh)
*/

int main(int argc,char *argv[])
{
	Engine = (EngineExport_t*)pModule_LoadInterface(
		hEngineInstance,
		"./"PATH_ENGINE"/"MODULE_ENGINE,
		"Engine_Main",
		&Editor);
	if(!Engine)
	{
		gWindow_MessageBox("Launcher",pError_Get());
		return -1;
	}
	else if(Engine->iVersion != ENGINE_INTERFACE)
	{
		gWindow_MessageBox("Launcher","Launcher is outdated, please rebuild!");

		pModule_Unload(hEngineInstance);
		return -1;
	}

	return -1;
}
