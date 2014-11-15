/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "menu_main.h"

/*
	Main menu entry!
	Functions called by the engine can be found here.
*/

// Platform library
#include "platform_module.h"

/*	TODO:
		#	Get menu elements to be handled like objects.
		#	Let us handle models (for 3D menu interface).
		#	Get basic window environment done, move windows
			if being clicked and mousepos changing, allow
			windows to be resized, let us show a "virtual"
			window within another window, "Katana Menu Scripts",
			allow us to manipulate windows within a 3D space
		#	Allow for texture maps!
		etc
		etc
*/

enum
{
	MENU_MAIN,										// Main Menu
		MENU_NEW,									// New Game
			MENU_NEW_EPISODE1,
			MENU_NEW_EPISODE2,
			MENU_NEW_EPISODE3,
			MENU_NEW_EPISODE4,
		MENU_NEW_END,

		MENU_LOAD,
		MENU_LOAD_END,

		MENU_MULTI,									// Multiplayer
			MENU_MULTI_JOIN,						// Join Game
				MENU_MULTI_JOIN_NETWORK,			// Network
				MENU_MULTI_JOIN_INTERNET,			// Internet
				MENU_MULTI_JOIN_MODEM,				// Modem
				MENU_MULTI_JOIN_DIRECT,				// Direct Connect
			MENU_MULTI_NEW,							// New Game
				MENU_MULTI_NEW_NETWORK,				// Network
				MENU_MULTI_NEW_MODEM,				// Modem
				MENU_MULTI_NEW_DIRECT,				// Direct Connect
			MENU_MULTI_SETUP,						// Player Setup
				MENU_MULTI_SETUP_GAME,				// Game Name
				MENU_MULTI_SETUP_NAME,				// Player Name
				MENU_MULTI_SETUP_PORT,				// Network Port
		MENU_MULTI_END,

		MENU_OPTIONS,								// Options
			MENU_OPTIONS_SOUND,						// Options / Sound Volume
			MENU_OPTIONS_CD,						// Options / CD Volume
			MENU_OPTIONS_CONTROL,					// Options / Customize Control
			MENU_OPTIONS_MOUSE,						// Mouse Config
			MENU_OPTIONS_VIDEO,						// Video Mode
			MENU_OPTIONS_MISC,						// Misc Settings
				MENU_OPTIONS_MISC_SCREEN,			// Screen Size
				MENU_OPTIONS_MISC_CROSSHAIR,		// Options / Misc / Crosshair
				MENU_OPTIONS_MISC_CROSSHAIR_SCALE,	// Options / Misc / Crosshair Scale
				MENU_OPTIONS_MISC_BRIGHTNESS,		// Brightness
				MENU_OPTIONS_MISC_RUN,				// Always Run
				MENU_OPTIONS_MISC_LOOKSPRING,		// Lookspring
				MENU_OPTIONS_MISC_LOOKSTRAFE,		// Lookstrafe
		MENU_OPTIONS_END,

		MENU_QUIT,
	MENU_MAIN_END
};

MenuExport_t	Export;
ModuleImport_t	Engine;

Menu_t	*mMenuElements;

int	iMenuElements,
	iMenuAllocated			= 512,
	iMenuState				= 0,
	iMenuDisplaySelection	= MENU_NEW;

cvar_t	cvShowMenu			= {	"menu_show",			"1",    false,  false,  "Toggle the display of any menu elements."	        },
		cvShowHealth		= {	"menu_showhealth",		"2",	true,   false,  "Toggle the health HUD."	                        },
		cvShowCrosshair		= {	"menu_showcrosshair",	"1",	true,	false,	"Toggle the display of the crosshair."				},
		cvShowAmmo			= {	"menu_showammo",		"2",	true,	false,  "Toggle the ammo HUD."                              },
		cvDebugMenu			= { "menu_debug",			"0",    false,  false,  "Toggle the display of any debugging information."  },
		cvCrosshairScale	= { "menu_crosshair_scale",	"1",	true,	false,	"Changes the scale of the crosshair."				};

// [13/8/2013] Fixed ~hogsy
int	iMousePosition[2];

bool	bInventoryOpen	= false,
		bScoreboardOpen	= false;

// [2/8/2012] TODO: Why are we doing this!? Should be using the one from the lib ~hogsy
char *va(char *format,...)
{
	va_list		argptr;
	static char	string[1024];

	va_start(argptr,format);
	vsprintf(string,format,argptr);
	va_end(argptr);

	return string;
}

void Input_ToggleMenu(void);		// menu_input.c
void Input_ToggleScoreboard(void);	// menu_input.c

void Menu_GetScreenSize(void);

void Menu_Initialize(void)
{
	int	i;

	Engine.Con_Printf("Initializing menu...\n");

	Engine.Cvar_RegisterVariable(&cvShowMenu,NULL);
	// [5/5/2012] Added cvar menu_showhealth and menu_showammo ~hogsy
	Engine.Cvar_RegisterVariable(&cvShowHealth,NULL);
	Engine.Cvar_RegisterVariable(&cvShowAmmo,NULL);
	Engine.Cvar_RegisterVariable(&cvShowCrosshair,NULL);
	// [2/8/2012] Added menu_debug ~hogsy
	Engine.Cvar_RegisterVariable(&cvDebugMenu,NULL);
	Engine.Cvar_RegisterVariable(&cvCrosshairScale,NULL);

	Engine.Cmd_AddCommand("menu_togglescores",Input_ToggleScoreboard);
	Engine.Cmd_AddCommand("menu_toggle",Input_ToggleMenu);

//	Script_Initialize();

#ifdef GAME_OPENKATANA
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_BASE_PATH"topbar");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_BASE_PATH"topclose");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_BASE_PATH"loading");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_BASE_PATH"paused");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"ammo");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"health");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"messenger");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"inventory");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"cross");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"armor");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"crosshair0");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"crosshair1");
#elif GAME_ADAMAS
    Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"health");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"ammo");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"armor");
	Engine.Client_PrecacheResource(RESOURCE_TEXTURE,MENU_HUD_PATH"crosshair");
#endif

	// Allocate global elements.
	mMenuElements = (Menu_t*)malloc(iMenuAllocated*sizeof(Menu_t));
	if(!mMenuElements)
	{
		Engine.Sys_Error("Failed to allocate menu elements!\n");
		return;
	}

	// [2/8/2012] Precache all the HUD digits ~hogsy
	for(i = 0; i < 10; i++)
		Engine.Client_PrecacheResource(RESOURCE_TEXTURE,va(MENU_HUD_PATH"num%i",i));

	Menu_GetScreenSize();

#if 0
	{
		Menu_t	mHealth,
				mTestWindow;

		mTestWindow.bActive			= true;
		mTestWindow.cContent		= 0;
		mTestWindow.cName			= "testwindow";
		mTestWindow.cResource		= 0;
		mTestWindow.fAlpha			= 1.0f;
		mTestWindow.iPosition[X]	= 320;
		mTestWindow.iPosition[Y]	= 320;
		mTestWindow.iScale[WIDTH]	= 512;
		mTestWindow.iScale[HEIGHT]	= 512;
		mTestWindow.mMenuType		= MENU_WINDOW;
		mTestWindow.mParent			= 0;

		mHealth.bActive			= false;
		mHealth.cContent		= "";
		mHealth.cName			= "health";
		mHealth.cResource		= "models/hud/hud_health.md2";
		mHealth.fAlpha			= 1.0f;
		mHealth.iPosition[0]	= 0;
		mHealth.iPosition[1]	= 0;
		mHealth.iScale[0]		= 0;
		mHealth.iScale[1]		= 0;
		mHealth.mMenuType		= MENU_MODEL;
		mHealth.mParent			= NULL;

		// [3/8/2012] Add it to the array ~hogsy
		Menu_Add(mTestWindow);
		Menu_Add(mHealth);
	}
#endif

	// [3/8/2012] Automatically precache images being used by each element ~hogsy
	for(i = 0; i < iMenuElements; i++)
	{
		if(!&mMenuElements[i] || !mMenuElements[i].cResource)
			// Break, don't return in-case we want to do here later.
			break;

		switch(mMenuElements[i].mMenuType)
		{
		default:    // [13/8/2013] By default precache it as an image ~hogsy
			Engine.Client_PrecacheResource(RESOURCE_TEXTURE,mMenuElements[i].cResource);
			break;
		case MENU_MODEL:
			Engine.Client_PrecacheResource(RESOURCE_MODEL,mMenuElements[i].cResource);
		}
	}
}

// [29/8/2012] Temporary function to deal with screensize changes. Should add Menu_UpdateState (called from engine upon display change). ~hogsy
void Menu_GetScreenSize(void)
{
	iMenuWidth	= Engine.GetScreenWidth();
	iMenuHeight	= Engine.GetScreenHeight();
}

/*
	GUI System
*/

// Elements
#include "menu_button.h"

// [2/8/2012] Dynamic allocation, which is based on this: http://fydo.net/gamedev/dynamic-arrays ~hogsy
void Menu_Add(Menu_t mWindow)
{
	/*	Stages...
		
		#1	Check that we've been given a name.
		#2	Parse a script to check that it's been initialized there. (if not then discontinue)
		#3	
	*/
	
	if(!mWindow.cName)
	{
		Engine.Con_Warning("Attempted to add menu without given name!\n");
		return;
	}

	iMenuElements++;

#if 0
	// [17/4/2013] Add to this first so we allocate the correct amount! ~hogsy
	iMenuElements++;

	{
		void *Temp = realloc(mMenuElements,(iMenuElements*sizeof(Menu_t)));
		if(!Temp)
		{
			Engine.Con_Warning("Failed to reallocate memory for menu (%s)!\n",mWindow.cName);

			iMenuElements--;

			return;
		}

		mMenuElements = (Menu_t*)Temp;
	}

	mMenuElements[iMenuElements] = mWindow;
#endif
}

void Menu_AddButton(Menu_t *mParent,MenuButton_t mButton)
{
	if(!mParent)
	{
		Engine.Con_Warning("Button parent hasn't been allocated!\n");
		return;
	}
}

/**/

void Menu_DrawSlider(Menu_t menu,float range)
{
	if(range < 0)
		range = 0;
	else if(range > 1.0f)
		range = 1.0f;

	Engine.DrawPic(MENU_BASE_PATH"i_indicator3",1.0f,menu.iPosition[WIDTH],menu.iPosition[HEIGHT],160,32);
	Engine.DrawPic(MENU_BASE_PATH"i_sliderbar2",1.0f,(menu.iPosition[WIDTH]+27)+90*(int)range,menu.iPosition[HEIGHT]+2,16,30);
}

void Menu_DrawCheckBox(int x,int y,bool bOn)
{
	if(bOn)
		Engine.DrawString(x,y,"[X]");
	else
		Engine.DrawString(x,y,"[ ]");
}

// [4/8/2012] Added Menu_IsMouseOver ~hogsy
// [13/8/2013] Fixed ~hogsy
bool Menu_IsMouseOver(int iPosition[2],int fWidth,int fHeight)
{
	if(	iMousePosition[0] >= iPosition[X] &&
		iMousePosition[0] <= (iPosition[X]+fWidth) &&
		iMousePosition[1] >= iPosition[Y] &&
		iMousePosition[1] <= (iPosition[Y]+fHeight))
		return true;

	return false;
}

void HUD_Draw(void);

void Menu_Draw(void)
{
	/*	TODO:
			Draw inactive windows last.

		~hogsy
	*/

	if(cvShowMenu.value <= 0)
		return;

	// [17/9/2013] Set the canvas to the default ~hogsy
	Engine.Client_SetMenuCanvas(CANVAS_DEFAULT);

	// [3/8/2012] Find out the current position of our mouse on each frame ~hogsy
	// [3/10/2012] Updated to use what's provided by the engine instead ~hogsy
	Engine.GetCursorPosition(&iMousePosition[X],&iMousePosition[Y]);

#if 1
	if(iMenuState & MENU_STATE_LOADING)
	{
		// [21/5/2013] TODO: Switch to element ~hogsy
		Engine.DrawPic(MENU_BASE_PATH"loading",1.0f,(iMenuWidth-256)/2,(iMenuHeight-32)/2,256,32);
		return;
	}
	else if(iMenuState & MENU_STATE_PAUSED)
	{
		Engine.DrawPic(MENU_BASE_PATH"paused",1.0f,0,0,32,64);
		return;
	}

	if((iMenuState & MENU_STATE_HUD) && (!(iMenuState & MENU_STATE_SCOREBOARD) && !(iMenuState & MENU_STATE_MENU)))
		HUD_Draw();

	if(iMenuState & MENU_STATE_SCOREBOARD && !(iMenuState & MENU_STATE_MENU))
	{
		// [10/11/2013] TEMP: Added for MP test ~hogsy
		Engine.DrawFill((iMenuWidth/2),(iMenuHeight/2),128,35,0,0,0,0.7f);
		Engine.DrawString(255,10,"Player Score:");
		Engine.DrawString(270,20,va("%d",Engine.Client_GetStat(STAT_FRAGS)));
	}

	if(iMenuState & MENU_STATE_MENU)
	{
		Engine.DrawFill(0,0,iMenuWidth,iMenuHeight,0,0,0,0.8f);

		Engine.Client_SetMenuCanvas(CANVAS_MENU);

		Engine.DrawString(110,80,">");
		Engine.DrawString(190,80,"<");

		switch(iMenuDisplaySelection)
		{
		case MENU_MAIN:
			Engine.DrawString(120,80,"New Game");
			Engine.DrawString(120,90,"Load Game");
			Engine.DrawString(120,100,"Settings");
			Engine.DrawString(120,110,"Quit");
			break;
		case MENU_NEW:
			break;
		case MENU_LOAD:
		case MENU_OPTIONS:
			break;
		case MENU_QUIT:
			// Do we even need to draw anything for this? ~hogsy
			break;
		}
	}
#if 0
	for(i = 0; i < iMenuElements; i++)
	{
		if(mMenuElements[i].bActive)
			return;

		switch((int)mMenuElements[i].mMenuType)
		{
		case MENU_IMAGE:
			Engine.DrawPic(
				mMenuElements[i].cResource,
				mMenuElements[i].fAlpha,
				mMenuElements[i].iPosition[X],
				mMenuElements[i].iPosition[Y],
				mMenuElements[i].iScale[WIDTH],
				mMenuElements[i].iScale[HEIGHT]);
		case MENU_WINDOW:
		case MENU_BUTTON:
		case MENU_MODEL:
			break;
		}

		// [3/8/2012] MUST be done last to prevent something drawing over us! ~hogsy
		if(cvDebugMenu.value >= 1)
		{
			int p = mMenuElements[i].iPosition[Y];

			Engine.DrawString(iMousePosition[X],iMousePosition[Y],va
			(
				"(%i,%i)",
				iMousePosition[X],
				iMousePosition[Y]
			));

			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"Name: %s",
				mMenuElements[i].cName
			));
			p += 12;
			// [27/8/2012] Let us know who the parent is ~hogsy
			if(mMenuElements[i].mParent)
			{
				Engine.DrawString((int)mMenuElements[i].iPosition[X],p,va
				(
					"Parent: %s",
					mMenuElements[i].mParent->cName
				));
				p += 12;
			}
			if(mMenuElements[i].mMenuType == MENU_IMAGE)
			{
				Engine.DrawString(mMenuElements[i].iPosition[X],p,va
				(
					"Image: %s",
					mMenuElements[i].cName
				));
				p += 12;
			}
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"X: %f",
				mMenuElements[i].iPosition[0]
			));
			p += 12;
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"Y: %f",
				mMenuElements[i].iPosition[1]
			));
			p += 12;
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"W: %i",
				mMenuElements[i].iScale[WIDTH]
			));
			p += 12;
			Engine.DrawString(mMenuElements[i].iPosition[X],p,va
			(
				"H: %i",
				mMenuElements[i].iScale[HEIGHT]
			));
		}
	}
#endif
#else
	for(i = 0; i < iMenuElements; i++)
	{
//		mMenuElements[i].vPos[0] = mousepos[0];
//		mMenuElements[i].vPos[1] = mousepos[1];
		// [3/8/2012] Make sure elements can't go off-screen ~hogsy
#if 0
		if(mMenuElements->.vPos[0] > (screenwidth-mMenuElements[i].vScale[WIDTH]))
			mMenuElements[i].vPos[0] = (float)(screenwidth-mMenuElements[i].vScale[WIDTH]);
		else if(mMenuElements[i].vPos[0] < (screenwidth+mMenuElements[i].vScale[WIDTH]))
			mMenuElements[i].vPos[0] = (float)(screenwidth+mMenuElements[i].vScale[WIDTH]);
		if(mMenuElements[i].vPos[1] > (iMenuHeight-mMenuElements[i].height))
			mMenuElements[i].vPos[1] = (float)(screenwidth-mMenuElements[i].height);
		else if(mMenuElements[i].vPos[1] < (screenwidth+mMenuElements[i].height))
			mMenuElements[i].vPos[1] = (float)(screenwidth+mMenuElements[i].height);
#endif

		// [3/8/2012] Don't show if this element isn't visible ~hogsy
		// [27/8/2012] Don't bother rendering if the alpha is stupidly low ~hogsy
		if(mMenuElements[i].fAlpha > 0)
		{
			// [27/8/2012] Check if we have a parent ~hogsy
			if(mMenuElements[i].mParent)
			{
				// [27/8/2012] TODO: I know this isn't right, just temporary until parents are actually in ~hogsy
				mMenuElements[i].iPosition[X] += mMenuElements[i].mParent->iPosition[0]-mMenuElements[i].iPosition[X];
				mMenuElements[i].iPosition[Y] += mMenuElements[i].mParent->iPosition[1]-mMenuElements[i].iPosition[Y];
			}

			// [3/8/2012] Set depending on the type of window ~hogsy
			switch(mMenuElements[i].mMenuType)
			{
			// [3/8/2012] Draw a window element ~hogsy
			case MENU_WINDOW:
				// [3/8/2012] Check if this window is active or not ~hogsy
				if(mMenuElements[i].bActive)
					mMenuElements[i].fAlpha = 1.0f;
				else
				{
					// [4/8/2012] Simplified ~hogsy
					if(Menu_IsMouseOver(mMenuElements[i].iPosition,mMenuElements[i].iScale[WIDTH],mMenuElements[i].iScale[HEIGHT]))
						// [3/8/2012] We're being hovered over, make us more clear ~hogsy
						mMenuElements[i].fAlpha = 1.0f;
					else
						// [4/8/2012] BUG: Anything too low and we don't even show! ~hogsy
						mMenuElements[i].fAlpha = 0.7f;
				}

				// [3/8/2012] Draw the window top ~hogsy
//				Engine.DrawPic(MENU_BASEPATH"topright",m_menu[i].fAlpha,m_menu[i].pos[0],m_menu[i].pos[1],(int)m_menu[i].vScale[WIDTH]-2,20);
//				Engine.DrawPic(MENU_BASEPATH"topleft",m_menu[i].fAlpha,m_menu[i].pos[0],m_menu[i].pos[1],(int)m_menu[i].vScale[WIDTH]-2,20);
				Engine.DrawPic(
					MENU_BASEPATH"topbar",
					mMenuElements[i].fAlpha,
					mMenuElements[i].iPosition[X],
					mMenuElements[i].iPosition[1],
					mMenuElements[i].iScale[WIDTH]-2,
					20);
				Engine.DrawPic(MENU_BASEPATH"topclose",
					mMenuElements[i].fAlpha,
					mMenuElements[i].iPosition[X]+(mMenuElements[i].iScale[WIDTH]-20),
					mMenuElements[i].iPosition[Y],
					22,
					20);

				Engine.DrawString(mMenuElements[i].iPosition[X]+5,mMenuElements[i].iPosition[Y]+5,mMenuElements[i].cName);

				// [3/8/2012] Do the fill for the contents ~hogsy
				Engine.DrawFill(
					mMenuElements[i].iPosition[X],
					mMenuElements[i].iPosition[Y]+20,
					mMenuElements[i].iScale[WIDTH],
					mMenuElements[i].iScale[HEIGHT]-20,
					0.5f,
					0.5f,
					0.5f,
					mMenuElements[i].fAlpha);
				break;
			case MENU_BUTTON:
				if(Menu_IsMouseOver(mMenuElements[i].iPosition,mMenuElements[i].iScale[WIDTH],mMenuElements[i].iScale[HEIGHT]))
				{}
			default:
				Engine.Con_Warning("Unknown menu element type (%i) for %s!\n",mMenuElements[i].mMenuType,mMenuElements[i].cName);
				// [3/8/2012] TODO: This isn't right... Clear it properly! ~hogsy
				free(mMenuElements);
				// [3/8/2012] TODO: Don't just return, we should be changing to the next slot ~hogsy
				return;
			}
		}
	}
#endif
}

int iLastOption = 0;

void Menu_AdjustSliders(int iMenu,int iOption,int iDirection)
{
	if(!iMenu || !iOption)
	{
		Engine.Con_Warning("Unknown menu or option!\n");
		return;
	}

	if(iLastOption > iOption)
	{
		// [8/5/2013] TODO: Play client-side sound ~hogsy
	}
	else
	{
		// [8/5/2013] TODO: Play client-side sound ~hogsy
	}

	switch(iMenu)
	{
	case MENU_OPTIONS:
		switch(iOption)
		{
		case MENU_OPTIONS_SOUND:
			Engine.Cvar_SetValue("volume",1.0f);
			break;
		case MENU_OPTIONS_CD:
			Engine.Cvar_SetValue("bgmvolume",1.0f);
		}
		break;
	}

	switch(iMenu)
	{
	case MENU_OPTIONS:
		switch(iOption)
		{
		case MENU_OPTIONS_MISC_CROSSHAIR_SCALE:
			cvCrosshairScale.value += iDirection*1.0f;
			if(cvCrosshairScale.value < 0.1f)
				cvCrosshairScale.value = 0.1f;
			else if(cvCrosshairScale.value > 5.0f)
				cvCrosshairScale.value = 5.0f;
			Engine.Cvar_SetValue(cvCrosshairScale.name,cvCrosshairScale.value);
			break;
		}
	}
}

/*	Called by the engine.
	Add state to the current state set.
*/
void Menu_AddState(int iState)
{
	// [17/9/2013] Return since the state is already added ~hogsy
	if(iMenuState & iState)
		return;

	iMenuState |= iState;
}

/*	Called by the engine.
	Remove a state from the menus state set.
*/
void Menu_RemoveState(int iState)
{
	// [17/9/2013] Return since the state is already removed ~hogsy
	if(!(iMenuState & iState))
		return;

	iMenuState &= ~iState;
}

/*	Called by the engine.
	Force the current menu state to something new.
*/
void Menu_SetState(int iState)
{
	iMenuState = iState;
}

/*	Called by the engine.
	Returns the menus currently active states.
*/
int Menu_GetState(void)
{
	return iMenuState;
}

// [3/8/2012] Called by the engine (see host.c) ~hogsy
void Menu_Shutdown(void)
{
	// [3/8/2012] Let us know that the menu module is being unloaded ~hogsy
	Engine.Con_Printf("Shutting down menu...\n");

	// [2/8/2012] Deallocate our array ~hogsy
	free(mMenuElements);
}

void Input_Key(int iKey);

pMODULE_EXPORT MenuExport_t *Menu_Main(ModuleImport_t *Import)
{
	Engine.Con_DPrintf				= Import->Con_DPrintf;
	Engine.Con_Printf				= Import->Con_Printf;
	Engine.Con_Warning				= Import->Con_Warning;
	Engine.Sys_Error				= Import->Sys_Error;
	Engine.Cvar_RegisterVariable	= Import->Cvar_RegisterVariable;
	Engine.DrawPic					= Import->DrawPic;
	Engine.DrawString				= Import->DrawString;
	Engine.DrawFill					= Import->DrawFill;
	Engine.Cvar_SetValue			= Import->Cvar_SetValue;
	Engine.GetScreenHeight			= Import->GetScreenHeight;
	Engine.GetScreenWidth			= Import->GetScreenWidth;
	Engine.GetCursorPosition		= Import->GetCursorPosition;
	Engine.ShowCursor				= Import->ShowCursor;
	Engine.Client_PrecacheResource	= Import->Client_PrecacheResource;
	Engine.Cmd_AddCommand			= Import->Cmd_AddCommand;
	Engine.Client_GetStat			= Import->Client_GetStat;
	Engine.Client_SetMenuCanvas		= Import->Client_SetMenuCanvas;

	Export.iVersion		= MODULE_VERSION;
	Export.Initialize	= Menu_Initialize;
	Export.Shutdown		= Menu_Shutdown;
	Export.Draw			= Menu_Draw;
	Export.Input		= Input_Key;
	Export.SetState		= Menu_SetState;
	Export.RemoveState	= Menu_RemoveState;
	Export.AddState		= Menu_AddState;
	Export.GetState		= Menu_GetState;

	return &Export;
}
