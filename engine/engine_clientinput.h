#ifndef __KATINPUT__
#define __KATINPUT__

#include "quakedef.h"

extern bool bMouseActive;

void Input_Initialize(void);
void Input_Process(void);
void Input_ProcessClient(usercmd_t *ucCommand);
void Input_ActivateMouse(void);
void Input_DeactivateMouse(void);
void Input_Shutdown(void);

#endif
