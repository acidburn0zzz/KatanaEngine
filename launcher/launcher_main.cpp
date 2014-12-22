/*	Copyright (C) 2011-2015 OldTimes Software
*/

/*
	Launches the engine.
	Currently can only launch the engine in one mode, but this is planned
	to also launch the engine directly into editor mode in the future.
*/

// Platform Library
#include "platform_module.h"
#include "platform_window.h"

// Shared Library
#include "shared_flags.h"
#include "shared_module.h"
#include "shared_engine.h"

EngineExport_t	*Engine;
EngineImport_t	*Launcher;

pINSTANCE hEngineInstance;

/*	TODO:
		List of things I want access to from the engine...
			Engine->GetBasePath()				(Returns current base path)
			Engine->CreateEntity()				(Returns an entity instance)
			Engine->LoadMap(file)				(Loads the specified level)
			Engine->CreateBrush(origin,size)	(Creates a brush)
			Engine->CreateWindow(x,y,w,h)		(Creates a new window... Need to handle different window instances somehow... Sigh)
*/

int main(int argc,char *argv[])
{
	Engine = (EngineExport_t*)pModule_LoadInterface(
		hEngineInstance,
		"./"PATH_ENGINE"/"MODULE_ENGINE,
		"Engine_Main",
		&Launcher);
	if(!Engine)
	{
		gWindow_MessageBox("Launcher",pError_Get());
		return -1;
	}
	else if (Engine->iVersion != ENGINE_VERSION)
	{
		gWindow_MessageBox("Launcher","Launcher is outdated, please rebuild!");

		pModule_Unload(hEngineInstance);
		return -1;
	}
	
	// Initialize and begin our main loop.
	Engine->Initialize(argc,argv);

	// Because the loop is within the initialize function.
	pModule_Unload(hEngineInstance);

	return -1;
}
