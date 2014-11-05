#ifndef __MENUBUTTON__
#define	__MENUBUTTON__

#include "menu_main.h"

typedef struct MenuButton_s
{
	bool	bActive,                                // Is the button active?
			bMouseInside;                           // Is the mouse within the buttons area?

    Menu_t  *mParent;                               // Parent of the button. The button will move with this.

	void	(*Click)(struct MenuButton_s *mbButton);
	void	(*Draw)(struct MenuButton_s *mbButton);
} MenuButton_t;

MenuButton_t	mbButtons[1024];

#endif
