/*  Copyright (C) 2011-2014 OldTimes Software
*/
#include "menu_button.h"

/*
	Button Elements
*/

void Button_Draw(Menu_t *mParent,MenuButton_t *mbButton)
{
}

void Button_Click(Menu_t *mParent,MenuButton_t *mbButton)
{
	if(!mbButton->bMouseInside || !mbButton->bActive)
		return;

    //Menu_MakeActive(bButton->mParent);
}

MenuButton_t *Button_Create(void)
{
	MenuButton_t	*mbOut;

	// Default handlers...
	mbOut->Click	= Button_Click;
	mbOut->Draw		= Button_Draw;

    return mbOut;
}