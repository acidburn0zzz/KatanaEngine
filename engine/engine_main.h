#ifndef __ENGINEMAIN__
#define __ENGINEMAIN__

#include "quakedef.h"

#include "shared_module.h"
#include "shared_engine.h"

typedef struct
{
	// Host Information
	char	cLocalName[128];	// Current system username.	
} Global_t;

Global_t	Global;

#endif