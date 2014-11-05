#ifndef __ENGINEMAIN__
#define __ENGINEMAIN__

#include "quakedef.h"

#include "shared_module.h"
#include "shared_engine.h"

typedef struct
{
	// Host Information
	char	cLocalName[128];	// Current system username.	

	char	cMaterialPath[PLATFORM_MAX_PATH],
			cTexturePath[PLATFORM_MAX_PATH];
} Global_t;

Global_t	Global;

// System
double System_DoubleTime(void);

#endif