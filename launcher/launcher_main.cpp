/*	Copyright (C) 2011-2014 OldTimes Software
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

// [21/5/2013] Moved down here for consistency ~hogsy
EngineExport_t	*Engine;
EngineImport_t	*Launcher;

gINSTANCE hEngineInstance;

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
	Engine = (EngineExport_t*)gModule_Load(
		hEngineInstance,
		"./"PATH_ENGINE"/"MODULE_ENGINE,
		"Engine_Main",
		&Launcher);
	if(!Engine)
	{
		// [1/6/2013] Updated to use new Platform function ~hogsy
		gWindow_MessageBox("Launcher",GIPL_GetError());
		return -1;
	}
	else if(Engine->iVersion != ENGINE_INTERFACE)
	{
		// [1/6/2013] Updated to use new Platform function ~hogsy
		gWindow_MessageBox("Launcher","Launcher is outdated, please rebuild!");

		gModule_Unload(hEngineInstance);
		return -1;
	}

	// [8/11/2012] Initialize the engine and begin our main loop ~hogsy
	Engine->Initialize(argc,argv);

	// [8/11/2012] Because the loop is within the initialize function ~hogsy
	gModule_Unload(hEngineInstance);

	return -1;
}
