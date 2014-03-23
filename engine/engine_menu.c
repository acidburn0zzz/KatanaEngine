/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_menu.h"

#include "KatGL.h"
#include "engine_video.h"
#include "engine_client.h"

#include "shared_module.h"

#include "../platform/include/platform_window.h"
#include "../platform/include/platform_module.h"

gINSTANCE	hMenuInstance;

int	Menu_GetScreenWidth(void)
{
	return Video.iWidth;
}

int Menu_GetScreenHeight(void)
{
	return Video.iHeight;
}

/*	Toggles whether the menu is active or not.
*/
void Menu_Toggle(void)
{
	if(Menu->GetState() & MENU_STATE_MENU)
		Menu->RemoveState(MENU_STATE_MENU);
	else
		Menu->AddState(MENU_STATE_MENU);
}

int Menu_GetState(MenuType_t mtType)
{
	switch(mtType)
	{
	case MENU_TYPE_HUD:
		break;
	case MENU_TYPE_MENU:
		break;
	default:
		Con_Warning("Unknown menu type! (%i)\n",mtType);
	}

	return 0;
}

void Game_AddCommand(char *c,void *f);	// [21/5/2013] TEMP: See engine_game.c ~hogsy

void Menu_Initialize(void)
{
	bool			bMenuLoaded = false;
	ModuleImport_t	mImport;

	if(Menu)
		gModule_Unload(hMenuInstance);

	mImport.Con_Printf				= Con_Printf;
	mImport.Con_DPrintf				= Con_DPrintf;
	mImport.Con_Warning				= Con_Warning;
	mImport.Sys_Error				= Sys_Error;
	mImport.Cvar_RegisterVariable	= Cvar_RegisterVariable;
	mImport.DrawPic					= Draw_ExternPic;
	mImport.DrawString				= R_DrawString;
	mImport.DrawFill				= Draw_Fill;
	mImport.GetScreenWidth			= Menu_GetScreenWidth;
	mImport.GetScreenHeight			= Menu_GetScreenHeight;
	mImport.ShowCursor				= gWindow_ShowCursor;
	mImport.GetCursorPosition		= gWindow_GetCursorPosition;
	mImport.Cmd_AddCommand			= Game_AddCommand;

	mImport.Client_GetStat			= Client_GetStat;
	mImport.Client_PrecacheResource	= Client_PrecacheResource;
	mImport.Client_SetMenuCanvas	= GL_SetCanvas;

	Menu = (MenuExport_t*)gModule_Load(hMenuInstance,va("%s/%s",com_gamedir,MODULE_MENU),"Menu_Main",&mImport);
	if(!Menu)
		Con_Warning(GIPL_GetError(),com_gamedir,MODULE_MENU);
	else if(Menu->iVersion != MODULE_VERSION)
		Con_Warning("Size mismatch (recieved %i, expected %i)!\n",Menu->iVersion,MODULE_VERSION);
	else
		bMenuLoaded = true;

	if(!bMenuLoaded)
	{
		// [4/5/2013] Unload our module ~hogsy
		gModule_Unload(hMenuInstance);

		Sys_Error("Failed to load %s/%s"gMODULE_EXTENSION"!\n",com_gamedir,MODULE_MENU);
	}
}
