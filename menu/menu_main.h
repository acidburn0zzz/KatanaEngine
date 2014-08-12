/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __MENUMAIN__
#define __MENUMAIN__

#include "platform.h"

#include "shared_flags.h"
#include "shared_menu.h"
#include "shared_module.h"
#include "shared_math.h"
#include "shared_engine.h"
#include "shared_game.h"

#define	MENU_BASE_PATH	"textures/interface/"
#define MENU_HUD_PATH	"textures/interface/hud/"

extern ModuleImport_t	Engine;

extern	cvar_t	cvShowMenu,
				cvShowHealth,
				cvShowAmmo,
				cvShowCrosshair,
				cvCrosshairScale;

int	iMenuState,				// Global menu state.
	iMenuWidth,iMenuHeight;	// Menu scale.

void	Menu_AddState(int iState);
void	Menu_SetState(int iState);
void	Menu_RemoveState(int iState);
void	Menu_GetScreenSize(void);

char *va(char *format,...);	// Temporary convenience function, please don't get comfortable with this :(

#endif
