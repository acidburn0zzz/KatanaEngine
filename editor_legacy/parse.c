
#include "qe3.h"

char	token[MAXTOKEN];
bool	unget;
char	*script_p;
int		scriptline;

void	StartTokenParsing (char *data)
{
	scriptline = 1;
	script_p = data;
	unget = FALSE;
}

bool GetToken (bool crossline)
{
	char    *token_p;

	if (unget)                         // is a token allready waiting?
		return TRUE;

//
// skip space
//
skipspace:
	while (*script_p <= 32)
	{
		if (!*script_p)
		{
			if (!crossline)
				Error ("Line %i is incomplete",scriptline);
			return FALSE;
		}
		if (*script_p++ == '\n')
		{
			if (!crossline)
				Error ("Line %i is incomplete",scriptline);
			scriptline++;
		}
	}

	if (script_p[0] == '/' && script_p[1] == '/')	// comment field
	{
		if (!crossline)
			Error ("Line %i is incomplete\n",scriptline);
		while (*script_p++ != '\n')
			if (!*script_p)
			{
				if (!crossline)
					Error ("Line %i is incomplete",scriptline);
				return FALSE;
			}
		goto skipspace;
	}

//
// copy token
//
	token_p = token;

	if (*script_p == '"')
	{
		script_p++;
		while ( *script_p != '"' )
		{
			if (!*script_p)
				Error ("EOF inside quoted token");
			*token_p++ = *script_p++;
			if (token_p == &token[MAXTOKEN])
				Error ("Token too large on line %i",scriptline);
		}
		script_p++;
	}
	else while ( *script_p > 32 )
	{
		*token_p++ = *script_p++;
		if (token_p == &token[MAXTOKEN])
			Error ("Token too large on line %i",scriptline);
	}

	*token_p = 0;
	
	return TRUE;
}

/*	Returns TRUE if there is another token on the line
*/
bool TokenAvailable (void)
{
	char    *search_p;

	search_p = script_p;

	while ( *search_p <= 32)
	{
		if (*search_p == '\n')
			return FALSE;
		if (*search_p == 0)
			return FALSE;
		search_p++;
	}

	if (*search_p == ';')
		return FALSE;

	return TRUE;
}

