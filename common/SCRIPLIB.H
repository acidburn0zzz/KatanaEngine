#ifndef __SHAREDSCRIPT__
#define __SHAREDSCRIPT__

#include "cmdlib.h"

#define	MAXTOKEN	1024

extern	char	cToken[MAXTOKEN];
extern	int		iScriptLine;

void Script_StartTokenParsing(char *cData);

bool	Script_GetToken(bool bCrossline);
bool	Script_TokenAvaliable(void);

#endif
