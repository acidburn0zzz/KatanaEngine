/*  Copyright (C) 2011-2013 OldTimes Software
*/
#include "menu_button.h"

/*
	Button Elements
*/

MenuButton_t *Button_Allocate(void)
{
    return NULL;
}

void Button_Click(MenuButton_t *bButton)
{
	if(!bButton->bMouseInside || !bButton->bActive)
		return;

    //Menu_MakeActive(bButton->mParent);
}
