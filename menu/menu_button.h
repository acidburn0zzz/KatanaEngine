#ifndef __MENUBUTTON__
#define	__MENUBUTTON__

#include "menu_main.h"

typedef struct
{
	bool	bActive,                                // Is the button active?
			bMouseInside;                           // Is the mouse within the buttons area?

    Menu_t  *mParent;                               // Parent of the button. The button will move with this.

	void	(*Click)(Menu_t *mParent);
	void	(*Draw)(Menu_t *mParent);
} MenuButton_t;

MenuButton_t	mbButtons[1024];

#endif
