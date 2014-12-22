/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "engine_clientinput.h"

/*
	Input System

	TODO:
		Somewhat merge this with the platform library so it's more unified here...
		Add in XInput support again for windows.

	Credits to raynorpat for his example.
*/

#include "engine_video.h"
#include "engine_modmenu.h"

#define INPUT_MAX_CONTROLLERS	3
#define INPUT_MAX_VIBRATION		65535
#define INPUT_MAX_ZONE			32767
#define INPUT_MIN_ZONE			3000

// [29/7/2012] Changing mouselook over to a cvar ~hogsy
cvar_t	cvInputMouseLook	= {	"input_mouselook",		"1",	true,   false,  "Enables and disables the ability to use the mouse to look around."	    },
        cvInputMouseFilter	= {	"input_mousefilter",	"0",	true,   false,  "Filters out mouse input so it responds more smoothly."	                },
        cvInputMouseGrab    = { "input_mousegrab",      "1",    true,   false,  "If enabled, the mouse is automatically hidden and limited to window."  };

extern cvar_t	joy_pitchsensitivity;

typedef struct
{
	SDL_Joystick		*sJoystick;
	SDL_GameController	*sGameController;

	float			fLeftMagnitude,
					fRightMagnitude;
} Controller_t;

Controller_t	cController[INPUT_MAX_CONTROLLERS];

int		iNumControllers;
int		iMousePosition[2],
		iOldMousePosition[2];	// [14/7/2013] For filtering ~hogsy

bool bMouseActive = false;

// [30/1/2013] Found in engine_video.c ~hogsy
extern SDL_Window	*sMainWindow;

SDL_Event	sEvent;

void Input_OpenTweakMenu(void);

void Input_Initialize(void)
{
	int i;

	Con_Printf("Initializing input...\n");

	// [29/7/2012] Register input cvars ~hogsy
	Cvar_RegisterVariable(&cvInputMouseLook,NULL);
	Cvar_RegisterVariable(&cvInputMouseFilter,NULL);
	Cvar_RegisterVariable(&cvInputMouseGrab,NULL);

	if(SDL_Init(SDL_INIT_JOYSTICK) < 0)
	{
		Con_Warning("Failed to initialize input!\n%s",SDL_GetError());
		// [2/7/2012] Not a serious issue if input ain't working ~hogsy
		return;
	}

	SDL_JoystickEventState(SDL_ENABLE);
	SDL_GameControllerEventState(SDL_ENABLE);

	for(i = 0; i < SDL_NumJoysticks(); i++)
	{
		Con_Printf(" Opening ");

		if(SDL_IsGameController(i))
		{
			cController[i].sGameController = SDL_GameControllerOpen(i);

			Con_Printf("Controller ");
        }
		else
		{
			cController[i].sJoystick = SDL_JoystickOpen(i);

			Con_Printf("Joystick ");
		}

		Con_Printf("(%i): ",i);

		if(cController[i].sJoystick || cController[i].sGameController)
		{
            Con_Printf("Success\n");

			iNumControllers++;
        }
        else
            Con_Printf("Failed!\n");
	}

	if(iNumControllers > 0)
		Con_Printf(" Found %i input device(s)\n",iNumControllers);
	else
	{
		// [30/1/2013] No controllers found, disable event processing ~hogsy
		SDL_JoystickEventState(SDL_IGNORE);
		SDL_GameControllerEventState(SDL_IGNORE);
    }
}

/*	Convert the given key over to what our engine uses.
	This is copied straight from raynorpat's example...
*/
int Input_ConvertKey(int iKey)
{
	switch(iKey)
	{
		case SDLK_PAGEUP:		return K_PGUP;
		case SDLK_PAGEDOWN:		return K_PGDN;
		case SDLK_HOME:			return K_HOME;
		case SDLK_END:			return K_END;
		case SDLK_LEFT:			return K_LEFTARROW;
		case SDLK_RIGHT:		return K_RIGHTARROW;
		case SDLK_DOWN:			return K_DOWNARROW;
		case SDLK_UP:			return K_UPARROW;
		case SDLK_ESCAPE:		return K_ESCAPE;
		case SDLK_RETURN:
		case SDLK_KP_ENTER:		return INPUT_KEY_ENTER;
		case SDLK_TAB:			return INPUT_KEY_TAB;
		case SDLK_F1:			return K_F1;
		case SDLK_F2:			return K_F2;
		case SDLK_F3:			return K_F3;
		case SDLK_F4:			return K_F4;
		case SDLK_F5:			return K_F5;
		case SDLK_F6:			return K_F6;
		case SDLK_F7:			return K_F7;
		case SDLK_F8:			return K_F8;
		case SDLK_F9:			return K_F9;
		case SDLK_F10:			return K_F10;
		case SDLK_F11:			return K_F11;
		case SDLK_F12:			return K_F12;
		case SDLK_DELETE:		return K_DEL;
		case SDLK_BACKSPACE:	return K_BACKSPACE;
		case SDLK_PAUSE:		return K_PAUSE;
		case SDLK_CAPSLOCK:		return K_CAPSLOCK;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:		return K_SHIFT;
		case SDLK_LCTRL:
		case SDLK_RCTRL:		return K_CTRL;
		case SDLK_LALT:
		case SDLK_RALT:			return K_ALT;
		case SDLK_BACKQUOTE:	return '`';
		case SDLK_KP_0:			return '0';
		case SDLK_KP_1:			return '1';
		case SDLK_KP_2:			return '2';
		case SDLK_KP_3:			return '3';
		case SDLK_KP_4:			return '4';
		case SDLK_KP_5:			return '5';
		case SDLK_KP_6:			return '6';
		case SDLK_KP_7:			return '7';
		case SDLK_KP_8:			return '8';
		case SDLK_KP_9:			return '9';
		case SDLK_a:			return 'a';
		case SDLK_b:			return 'b';
		case SDLK_c:			return 'c';
		case SDLK_d:			return 'd';
		case SDLK_e:			return 'e';
		case SDLK_f:			return 'f';
		case SDLK_g:            return 'g';
		case SDLK_h:            return 'h';
		case SDLK_i:			return 'i';
		case SDLK_j:			return 'j';
		case SDLK_k:			return 'k';
		case SDLK_l:			return 'l';
		case SDLK_m:			return 'm';
		case SDLK_n:			return 'n';
		case SDLK_o:			return 'o';
		case SDLK_p:			return 'p';
		case SDLK_q:			return 'q';
		case SDLK_r:			return 'r';
		case SDLK_s:			return 's';
		case SDLK_t:			return 't';
		case SDLK_u:			return 'u';
		case SDLK_v:			return 'v';
		case SDLK_w:			return 'w';
		case SDLK_x:			return 'x';
		case SDLK_y:			return 'y';
		case SDLK_z:			return 'z';
		case SDLK_INSERT:		return K_INS;
		case SDLK_KP_MULTIPLY:	return '*';
		case SDLK_KP_PLUS:		return '+';
		case SDLK_KP_MINUS:		return '-';
		case SDLK_KP_DIVIDE:	return '/';
		case SDLK_COLON:        return ':';
		case SDLK_SEMICOLON:    return ';';
		case SDLK_LESS:         return '<';
		case SDLK_EQUALS:       return '=';
		case SDLK_GREATER:      return '>';
		case SDLK_QUESTION:     return '?';
		case SDLK_AT:           return '@';
		case SDLK_LEFTBRACKET:  return '[';
		case SDLK_BACKSLASH:    return '\\';
		case SDLK_RIGHTBRACKET:	return ']';
		case SDLK_CARET:        return '^';
		case SDLK_UNDERSCORE:   return '_';
		case SDLK_EXCLAIM:      return '!';
		case SDLK_QUOTEDBL:     return '"';
		case SDLK_HASH:         return '#';
		case SDLK_PERCENT:      return '%';
		case SDLK_DOLLAR:       return '$';
		case SDLK_AMPERSAND:    return '&';
		case SDLK_QUOTE:        return '\'';
		case SDLK_LEFTPAREN:    return '(';
		case SDLK_RIGHTPAREN:   return ')';
		case SDLK_ASTERISK:		return '*';
		case SDLK_PLUS:			return '+';
		case SDLK_COMMA:		return ',';
		case SDLK_MINUS:		return '-';
		case SDLK_PERIOD:		return '.';
		case SDLK_SLASH:		return '/';
		default:    			return iKey;
	}
}

// [13/7/2013] Ripped from Raynorpat's code ~hogsy
static int InputMouseRemap[18]=
{
	K_MOUSE1,
	K_MOUSE3,
	K_MOUSE2,
	K_MWHEELUP,
	K_MWHEELDOWN
};

void Input_Process(void)
{
	static	bool sbOldMouseState = false;

	while(SDL_PollEvent(&sEvent))
	{
            switch(sEvent.type)
            {
            case SDL_WINDOWEVENT:
                switch(sEvent.window.event)
                {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    Video.bActive = true;

                    // [6/11/2013] Then restore it :) ~hogsy
                    if(sbOldMouseState)
                        Input_ActivateMouse();
                    else
                        Input_DeactivateMouse();

                    // [30/7/2013] Window isn't active! Clear states ~hogsy
                    Key_ClearStates();
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    Video.bActive = false;

                    // [6/11/2013] Save our old mouse state ~hogsy
                    sbOldMouseState = bMouseActive;

                    Input_DeactivateMouse();

                    // [30/7/2013] Window isn't active! Clear states ~hogsy
                    Key_ClearStates();
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    Video_UpdateWindow();
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    Sys_Quit();
                    break;
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
				Key_Event(Input_ConvertKey(sEvent.key.keysym.sym),(sEvent.key.state == SDL_PRESSED));
                break;
            case SDL_MOUSEMOTION:
				if (bIsDedicated || !bMouseActive)
                    return;

                // [30/7/2013] Originally handled this differently for fullscreen but this works fine apparently ~hogsy
                if(((unsigned)sEvent.motion.x != (Video.iWidth/2)) || ((unsigned)sEvent.motion.y != (Video.iHeight/2)))
                {
                    iMousePosition[pX]	= sEvent.motion.xrel*5;
                    iMousePosition[pY]	= sEvent.motion.yrel*5;
                    // [22/12/2013] TODO: This currently isn't accounting for any frame-loss... Ugh ~hogsy
                    if(	((unsigned)sEvent.motion.x < Video.iWidth)	||
                        ((unsigned)sEvent.motion.x > Video.iWidth)	||
                        ((unsigned)sEvent.motion.y < Video.iHeight)	||
                        ((unsigned)sEvent.motion.y > Video.iHeight))
                        SDL_WarpMouseInWindow(sMainWindow,Video.iWidth/2,Video.iHeight/2);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
				if(sEvent.button.button <= 18)
					Key_Event(InputMouseRemap[sEvent.button.button-1],(sEvent.button.state == SDL_PRESSED));
                break;
            case SDL_MOUSEWHEEL:
                if(sEvent.wheel.y > 0)
                {
                    Key_Event(K_MWHEELUP,true);
                    Key_Event(K_MWHEELUP,false);
                }
                else
                {
                    Key_Event(K_MWHEELDOWN,true);
                    Key_Event(K_MWHEELDOWN,false);
                }
                break;
            /*
                Controller Input
            */
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                break;
            /*
                Joystick Input
            */
            case SDL_JOYAXISMOTION:
                if((sEvent.jaxis.value > INPUT_MAX_ZONE) || (sEvent.jaxis.value < INPUT_MIN_ZONE))
                {
                }
                break;
            case SDL_JOYBALLMOTION:
            case SDL_JOYHATMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                break;
            case SDL_QUIT:
                Sys_Quit();
                break;
            }
        }
}

extern cvar_t	cl_maxpitch,
				cl_minpitch;

/*	Process client-specific movement which
	will be sent to the server.
*/
void Input_ProcessClient(usercmd_t *ucCommand)
{
	// [16/7/2013] Don't bother if the application isn't active ~hogsy
	if(!bMouseActive)
		return;

	if(cvInputMouseFilter.value)
	{
		iMousePosition[pX] += iOldMousePosition[pX]*cvInputMouseFilter.value;
		iMousePosition[pY] += iOldMousePosition[pY]*cvInputMouseFilter.value;

		iOldMousePosition[pX]	= iMousePosition[pX];
		iOldMousePosition[pY]	= iMousePosition[pY];
	}

	iMousePosition[pX] *= sensitivity.value;
	iMousePosition[pY] *= sensitivity.value;

	if(!cvInputMouseLook.value)
	{
		// [13/7/2013] Copied from Raynorpat's code ~hogsy
		if(in_strafe.state & 1)
			ucCommand->sidemove += m_side.value*iMousePosition[pX];

		ucCommand->forwardmove -= m_forward.value*iMousePosition[pY];
	}
	else
	{
		// [16/7/2013] Prevent the camera from resetting ~hogsy
		V_StopPitchDrift();

		cl.viewangles[YAW]		-= m_yaw.value*iMousePosition[pX];
		cl.viewangles[PITCH]	+= m_pitch.value*iMousePosition[pY];

		if(cl.viewangles[PITCH] > cl_maxpitch.value)
			cl.viewangles[PITCH] = cl_maxpitch.value;
		if(cl.viewangles[PITCH] < cl_minpitch.value)
			cl.viewangles[PITCH] = cl_minpitch.value;
	}

	iMousePosition[pX] = iMousePosition[pY] = 0;
}

void Input_ActivateMouse(void)
{
	if(bMouseActive)
		return;

    if(cvInputMouseGrab.bValue)
    {
        SDL_ShowCursor(false);
        SDL_SetWindowGrab(sMainWindow,SDL_TRUE);
        SDL_WarpMouseInWindow(sMainWindow,Video.iWidth/2,Video.iHeight/2);
	}

	bMouseActive = true;
}

void Input_DeactivateMouse(void)
{
	if(!bMouseActive)
		return;

	SDL_ShowCursor(true);
	SDL_SetWindowGrab(sMainWindow,SDL_FALSE);

	bMouseActive = false;
}

void Input_Shutdown(void)
{
	int i;

	for(i = 0; i < iNumControllers; i++)
	{
		// [29/7/2013] Function originally used here is obsolete. Just do a silly check here instead for safety ~hogsy
		if(cController[i].sJoystick)
			SDL_JoystickClose(cController[i].sJoystick);
		else if(cController[i].sGameController)
			SDL_GameControllerClose(cController[i].sGameController);
	}

	Input_DeactivateMouse();
}
