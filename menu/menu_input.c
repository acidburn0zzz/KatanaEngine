/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "menu_main.h"

/*	Called by the engine.
	Handles all input given by the engine which is sent when our main-menu is open.
*/
void Input_Key(int iKey)
{
	switch(iKey)
	{}
}

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