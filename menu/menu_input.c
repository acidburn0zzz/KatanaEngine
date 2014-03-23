/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "menu_main.h"

/*	Toggles whether the menu will be displayed or not.
*/
void Input_ToggleMenu(void)
{
	if(iMenuState & MENU_STATE_MENU)
		Menu_RemoveState(MENU_STATE_MENU);
	else
		Menu_AddState(MENU_STATE_MENU);
}

/*	Toggles whether the scoreboard will be displayed or not.
*/
void Input_ToggleScoreboard(void)
{
	if(iMenuState & MENU_STATE_SCOREBOARD)
		Menu_RemoveState(MENU_STATE_SCOREBOARD);
	else
		Menu_AddState(MENU_STATE_SCOREBOARD);
}