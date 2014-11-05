/*  Copyright (C) 2011-2014 OldTimes Software
*/
#include "menu_button.h"

/*
	Button Elements
	All this is currently in the conceptial phases, and should be implemented soon ~hogsy
*/

void Button_Draw(MenuButton_t *mbButton)
{
/*
	Basic drawing routine

	apply materials

	throw at the rendering pipeline...
	Engine.Video_DrawRectangle();
*/
}

void Button_Click(MenuButton_t *mbButton)
{
	if(!mbButton->bMouseInside || !mbButton->bActive)
		return;

    //Menu_MakeActive(bButton->mParent);
}

void Button_Frame(MenuButton_t *mbButton)
{
/*
	// Allow button to override before the frame occurs.
	if(mbButton->PreFrame)
		mbButton->PreFrame();

	// Allow the button to override the entire frame.
	if(mbButton->Frame)
		mbButton->Frame();
	// Otherwise only bother if the button is currently active.
	else if(mbButton->bActive)
	{
		if(mouse is inside button area)
		{
			Button_MouseHover(mbButton);

			if(mouse hasn't been inside before)
				Button_MouseEnter(mbButton);	[Set us up so we remember we're inside]
		}
		else if(mouse still thinks it's inside)
			Button_MouseLeave(mbButton);	[Set us up so we know we're not inside anymore]
	}

	// Allow the button to override after the frame occurs.
	if(mbButton->PostFrame)
		mbButton->PostFrame();
*/
}

MenuButton_t *Button_Create(void)
{
	MenuButton_t	*mbOut;

	// Default handlers...
	mbOut->Click	= Button_Click;
	mbOut->Draw		= Button_Draw;

    return mbOut;
}