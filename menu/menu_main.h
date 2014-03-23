/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __MENUMAIN__
#define __MENUMAIN__

#include "../platform/include/platform.h"

#include "shared_flags.h"
#include "shared_menu.h"
#include "shared_module.h"
#include "shared_math.h"
#include "shared_engine.h"

#include "KatShared.h"

#define	MENU_BASEPATH	"textures/menu/"
#define HUD_BASEPATH	"textures/interface/hud/"
#define EDITOR_BASEPATH	MENU_BASEPATH"editor/"

extern ModuleImport_t	Engine;

int	iMenuState;	// Global menu state.

void	Menu_AddState(int iState);
void	Menu_SetState(int iState);
void	Menu_RemoveState(int iState);

#endif