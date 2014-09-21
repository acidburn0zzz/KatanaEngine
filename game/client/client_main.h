/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __CLIENTMAIN__
#define __CLIENTMAIN__

#include "game_main.h"

#include "shared_client.h"

void Client_Initialize(void);
void Client_Draw(void);
void Client_ParseTemporaryEntity(void);

// View
void Client_ViewFrame(void);

#endif
