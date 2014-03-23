/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2014 OldTimes Software

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "pr_comp.h"			// defs shared with qcc

#include "KatShared.h"

typedef union eval_s
{
	string_t		string;
	float			_float;
	float			vector[3];
	func_t			function;
	int				_int;
	int				edict;
} eval_t;

#define	EDICT_FROM_AREA(l) STRUCT_FROM_LINK(l,edict_t,area)

//============================================================================

extern	ddef_t				*pr_globaldefs;
extern	GlobalVariables_t	pr_global_struct;
extern	float				*pr_globals;			// same as pr_global_struct

//============================================================================

void PR_Init (void);

edict_t *ED_Alloc (void);
void ED_Free (edict_t *ed);

char	*ED_NewString (char *string);
// returns a copy of the string allocated from the server's string heap

void Edict_Print(edict_t *eEntity);
void ED_Write (FILE *f, edict_t *ed);

char *ED_ParseEdict (char *data, edict_t *ent);

void ED_WriteGlobals (FILE *f);
void ED_ParseGlobals (char *data);
void ED_LoadFromFile (char *data);

edict_t *EDICT_NUM(int n);
int NUM_FOR_EDICT(edict_t *e);

#define	NEXT_EDICT(e) ((edict_t *)( (byte *)e + sizeof(edict_t)))

#define	EDICT_TO_PROG(e) ((byte*)e-(byte*)sv.edicts)
#define PROG_TO_EDICT(e) ((edict_t *)((byte *)sv.edicts + e))

//============================================================================

#define	G_INT(o) (*(int *)&pr_globals[o])

typedef void (*builtin_t) (void);

void ED_PrintEdicts (void);
void ED_PrintNum (int ent);

eval_t *GetEdictFieldValue(edict_t *ed, char *field);
