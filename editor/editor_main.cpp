/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "editor_main.h"

EngineImport_t  *Editor;

pINSTANCE   iEngineInstance;

int main(int argc,char *argv[])
{
	Engine = (EngineExport_t*)gModule_Load(
		iEngineInstance,
		"./"PATH_ENGINE"/"MODULE_ENGINE,
		"Engine_Main",
		&Editor);
	if(!Engine)
	{
		// [1/6/2013] Updated to use new Platform function ~hogsy
		gWindow_MessageBox("Editor",GIPL_GetError());
		return -1;
	}
	else if(Engine->iVersion != ENGINE_INTERFACE)
	{
		// [1/6/2013] Updated to use new Platform function ~hogsy
		gWindow_MessageBox("Editor","Editor is outdated, please rebuild!");

		Editor_Shutdown();
	}

	Engine->Initialize(argc,argv);

	if(!Engine->CreateWindow())
    {
        // todo: warning message
    }

    // [7/3/2014] Begin our main loop ~hogsy
	for(;;)
	{

	}

    Editor_Shutdown();

	return -1;
}

void Editor_Shutdown(void)
{
    if(iEngineInstance)
        gModule_Unload(iEngineInstance);

	exit(1);
}
