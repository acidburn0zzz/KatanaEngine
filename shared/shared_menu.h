/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SHAREDMENU__
#define	__SHAREDMENU__

typedef enum
{
	MENU_IMAGE,
	MENU_MODEL,		// [19/12/2012] Necessary? Could just compare extensions... ~hogsy
	MENU_WINDOW,
	MENU_BUTTON,
		MENU_BUTTON_OK,
		MENU_BUTTON_CANCEL,
		MENU_BUTTON_YESNO,

	// Special types
	MENU_TYPE_HUD,
	MENU_TYPE_MENU
} MenuType_t;

// Different menu states.
#define	MENU_STATE_LOADING		1	// Display loading menu.
#define	MENU_STATE_HUD			2	// Display HUD.
#define	MENU_STATE_SCOREBOARD	4	// Display player score.
#define	MENU_STATE_MENU			8	// Display the menu.
#define	MENU_STATE_PAUSED		16	// Display pause menu.

typedef struct Menu_s
{
	char			*cName,			// The name of the menu element.
					*cResource,		// If based on a model/image this is the path of the resource it needs.
					*cContent;		// Content, meaning the text that could be inside the window.

	// [3/8/2012] Parent window ~hogsy
	struct	Menu_s	*mParent;		// The parent of the element. If an element has a parent and the parent moves then this will move with it.

	int				iPosition[1];	// Position (X & Y) of the element.
	// [3/8/2012] Added vScale variable ~hogsy
	int				iScale[1];		// Scale (width & height) of the element.

	MenuType_t		mMenuType;		// The type of element.

	// [2/8/2012] Added visible and active flags ~hogsy
	// [4/8/2013] Removed bVisible since bActive makes it obsolete ~hogsy
	bool			bActive,		// Is it active?
					bMoveable;		// Can the element be moved?

	// [3/8/2012] Added in alpha variable ~hogsy
	float			fAlpha;
} Menu_t;

// Exported functions
typedef struct
{
	int		iVersion;					// Inteface version.

	void	(*Initialize)(void);		// Called when we initialize the module.
	void	(*Draw)(void);				// Called whenever we draw the menu.
	void	(*Input)(int iKey);			// Called whenever we press a key while in menu mode.
	void	(*AddState)(int iState);	// Add onto the current menu state.
	void	(*RemoveState)(int iState);	// Remove a state from the menus current state set.
	void	(*SetState)(int iState);	// Forcefully set a new state.
	void	(*Shutdown)(void);			// Called upon shutdown.

	int		(*GetState)(void);			// Get the menus current state so we can check it.
} MenuExport_t;

#endif
