/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2015 OldTimes Software

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
#include "engine_edict.h"

#include "engine_modgame.h"

ddef_t				*pr_fielddefs,
					*pr_globaldefs;
GlobalVariables_t	pr_global_struct;	//*pr_global_struct;
float				*pr_globals;			// same as pr_global_struct

#define	MAX_FIELD_LEN	128

/*	Sets everything to null for the specified entity.
*/
void Edict_Clear(edict_t *eEntity,bool bFreeEntity)
{
	memset(&eEntity->v,0,sizeof(eEntity->v));
	// [10/6/2013] Clear everything else that's shared... ~hogsy
	memset(&eEntity->Model,0,sizeof(eEntity->Model));
	memset(&eEntity->Physics,0,sizeof(eEntity->Physics));

	eEntity->free = bFreeEntity;
}

/*	Either finds a free edict, or allocates a new one.
	Try to avoid reusing an entity that was recently freed, because it
	can cause the client to think the entity morphed into something else
	instead of being removed and recreated, which can cause interpolated
	angles and bad trails.
*/
edict_t *ED_Alloc (void)
{
	int			i;
	edict_t		*e;

	for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && ( e->freetime < 2 || sv.time - e->freetime > 0.5 ) )
		{
			Edict_Clear(e,false);

			return e;
		}
	}

	if (i == sv.max_edicts) //johnfitz -- use sv.max_edicts instead of MAX_EDICTS
		Host_Error("ED_Alloc: no free edicts (max_edicts is %i)", sv.max_edicts); //johnfitz -- was Sys_Error

	sv.num_edicts++;

	e = EDICT_NUM(i);

	Edict_Clear(e,false);

	return e;
}

/*	Marks the edict as free
	FIXME: walk all entities and NULL out references to this entity
*/
void ED_Free (edict_t *ed)
{
	SV_UnlinkEdict(ed);		// unlink from world bsp

	ed->free = true;

	ed->v.model			= 0;
	ed->v.bTakeDamage	= false;
	ed->v.modelindex	= 0;
	ed->v.colormap		= 0;
	ed->Model.iSkin		= 0;
	ed->v.frame			= 0;
	ed->v.dNextThink	= -1;
	ed->Physics.iSolid	= 0;
	ed->Model.fScale	= 1.0f;
	ed->alpha			= ENTALPHA_DEFAULT; //johnfitz -- reset alpha for next entity

	Math_VectorCopy(mv3Origin,ed->v.origin);
	Math_VectorCopy(mv3Origin,ed->v.angles);

	ed->freetime = sv.time;
}

eval_t *GetEdictFieldValue(edict_t *ed, char *field)
{
#if 0
	ddef_t			*def = NULL;
	int				i;
	static int		rep = 0;

	for(i = 0; i < GEFV_CACHESIZE; i++)
	{
		if(!strcmp(field, gefvCache[i].field))
		{
			def = gefvCache[i].pcache;
			goto Done;
		}
	}

	def = ED_FindField (field);

	if(strlen(field) < MAX_FIELD_LEN)
	{
		gefvCache[rep].pcache = def;
		strcpy (gefvCache[rep].field, field);
		rep ^= 1;
	}

Done:
	if (!def)
		return NULL;

	return (eval_t *)((char *)&ed->v + def->ofs*4);
#endif
	return NULL;
}

/*	Returns a string describing *data in a type specific manner
*/
char *PR_ValueString (EntityType_t type, eval_t *val)
{
#if 0
	static char	line[256];

	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case EV_STRING:
		sprintf(line, "%s", val->string);
		break;
	case ev_entity:
		sprintf(line,"entity %i",NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)) );
		break;
	case EV_FLOAT:
		sprintf(line, "%5.1f", val->_float);
		break;
	case EV_VECTOR:
		sprintf(line, "'%5.1f %5.1f %5.1f'", val->vector[0], val->vector[1], val->vector[2]);
		break;
	default:
		sprintf(line, "bad type %i", type);
		break;
	}

	return line;
#else
	return "null";
#endif
}

/*	Returns a string describing *data in a type specific manner
	Easier to parse than PR_ValueString
*/
char *PR_UglyValueString(EntityType_t type, eval_t *val)
{
	static char	line[256];

	type &= ~DEF_SAVEGLOBAL;

#if 0
	switch (type)
	{
	case EV_STRING:
		sprintf (line, "%s", pr_strings + val->string);
		break;
	case ev_entity:
		sprintf (line, "%i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)));
		break;
	case EV_FLOAT:
		sprintf (line, "%f", val->_float);
		break;
	case EV_VECTOR:
		sprintf (line, "%f %f %f", val->vector[0], val->vector[1], val->vector[2]);
		break;
	default:
		sprintf (line, "bad type %i", type);
		break;
	}
#endif

	return line;
}

/*	For debugging
*/
void Edict_Print(edict_t *eEntity)
{
#if 0
	int		l;
	ddef_t	*d;
	int		*v,i,j,type;
	char	*name;

	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if(name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars

		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;

		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;

		Con_Printf("%s",name); //johnfitz -- was Con_Printf
		l = Q_strlen(name);
		while (l++ < 15)
			Con_Printf(" "); //johnfitz -- was Con_Printf

		Con_Printf("%s\n",PR_ValueString((EntityType_t)d->type,(eval_t *)v)); //johnfitz -- was Con_Printf
	}
#else
	int	i;

	if(eEntity->free)
	{
		Con_Printf("Entity is free!\n");
		return;
	}

	Con_Printf("\nEDICT %i:\n",NUM_FOR_EDICT(eEntity));

	for(i = 1; i < sizeof(edict_t); i++)
	{
	}
#endif
}

/*	For savegames
	[7/2/2013] TODO: Rewrite! ~hogsy
*/
void ED_Write(FILE *f,edict_t *ed)
{
//	ddef_t	*d;
//	int		*v;
	int		i;
//	char	*name;
//	int		type;

	fprintf (f, "{\n");

	if (ed->free)
	{
		fprintf (f, "}\n");
		return;
	}

#if 0
	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars

		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;
		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;

		fprintf (f,"\"%s\" ",name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(d->type, (eval_t *)v));
	}
#else	// [10/3/2013] Rewrite... ~hogsy
	for(i = 1; i < sizeof(edict_t); i++)
	{
//		fprintf(f,"\"%s\" ");
	}
#endif

	fprintf (f, "}\n");
}

void ED_PrintNum (int ent)
{
	Edict_Print(EDICT_NUM(ent));
}

/*	For debugging, prints all the entities in the current server
*/
void ED_PrintEdicts (void)
{
	int		i;

	Con_Printf ("%i entities\n", sv.num_edicts);
	for (i=0 ; i<sv.num_edicts ; i++)
		ED_PrintNum (i);
}

/*	For debugging, prints a single edicy
*/
void ED_PrintEdict_f (void)
{
	int		i;

	i = Q_atoi (Cmd_Argv(1));
	if (i >= sv.num_edicts)
	{
		Con_Printf("Bad edict number\n");
		return;
	}
	ED_PrintNum (i);
}

/*	For debugging
*/
void ED_Count (void)
{
	int		i;
	edict_t	*ent;
	int		active,models,solid,step;

	active = models = solid = step = 0;
	for(i = 0; i < sv.num_edicts; i++)
	{
		ent = EDICT_NUM(i);
		if(!ent)
			return;

		if(ent->free)
			continue;

		active++;

		if (ent->Physics.iSolid)
			solid++;

		if (ent->v.model)
			models++;

		if(ent->v.movetype == MOVETYPE_STEP)
			step++;
	}

	Con_Printf ("num_edicts:%3i\n", sv.num_edicts);
	Con_Printf ("active    :%3i\n", active);
	Con_Printf ("view      :%3i\n", models);
	Con_Printf ("touch     :%3i\n", solid);
	Con_Printf ("step      :%3i\n", step);
}

/*
==============================================================================

					ARCHIVING GLOBALS

FIXME: need to tag constants, doesn't really work
==============================================================================
*/

void ED_WriteGlobals (FILE *f)
{
#if 0
	ddef_t		*def;
	int			i;
	char		*name;
	int			type;

	fprintf (f,"{\n");
	for(i = 0; i < progs->numglobaldefs; i++)
	{
		def = &pr_globaldefs[i];
		type = def->type;
		if ( !(def->type & DEF_SAVEGLOBAL) )
			continue;
		type &= ~DEF_SAVEGLOBAL;

		if(type != EV_STRING && type != EV_FLOAT && type != ev_entity)
			continue;

		name = pr_strings + def->s_name;
		fprintf (f,"\"%s\" ", name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(type, (eval_t *)&pr_globals[def->ofs]));
	}
	fprintf (f,"}\n");
#else
	Con_Printf("ED_WriteGlobals\n");
#endif
}

void ED_ParseGlobals (char *data)
{
	char keyname[64];

	for(;;)
	{
	// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		strcpy (keyname, com_token);

	// parse value
		data = COM_Parse (data);
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEntity: closing brace without data");

		Con_Printf ("'%s' is not a global\n", keyname);

#if 0
		if(!ED_ParseEpair((void*)pr_globals,com_token))
			Host_Error ("ED_ParseGlobals: parse error");
#else

#endif
	}
}

//============================================================================

char *ED_NewString (char *string)
{
	char	*cNew,*new_p;
	int		i,l;

	l = strlen(string) + 1;
	cNew = (char*)Hunk_Alloc(l);
	new_p = cNew;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}

	return cNew;
}

void Server_ParseEntityField(char *cKey,char *cValue,edict_t *eEntity);

/*	Parses an edict out of the given string, returning the new position
	ed should be a properly initialized empty edict.
	Used for initial level load and for savegames.
	[19/11/2012]
	Revised to work for our needs. ~hogsy
*/
char *ED_ParseEdict(char *data, edict_t *ent)
{
	bool	bInit = false;
	char	keyname[256];
	int		n;

	if(ent != sv.edicts)	// hack
		Edict_Clear(ent,false);

	// Go through all the dictionary pairs
	for(;;)
	{
		// Parse key
		data = COM_Parse(data);
		if(com_token[0] == '}')
			break;

		if(!data)
			Sys_Error("ED_ParseEntity: EOF without closing brace");

		strncpy(keyname,com_token,sizeof(keyname)-1);

		// Hack to fix keynames with trailing spaces
#ifdef _MSC_VER
#pragma warning(suppress: 6053)	// This is intended.
#endif
		n = strlen(keyname);
		if(n > 0)
			while(n && keyname[n-1] == ' ')
			{
				keyname[n-1] = 0;
				n--;
			}

		// Parse value
		data = COM_Parse(data);
		if(!data)
			Sys_Error("ED_ParseEntity: EOF without closing brace");
		else if(com_token[0] == '}')
			Sys_Error("ED_ParseEntity: closing brace without data");

		bInit = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if(keyname[0] == '_')
			continue;

		Server_ParseEntityField(keyname,com_token,ent);
	}

	if(!bInit)
		ent->free = true;

	return data;
}

/*	The entities are directly placed in the array, rather than allocated with
	ED_Alloc, because otherwise an error loading the map would have entity
	number references out of order.

	Creates a server's entity / program execution context by
	parsing textual entity definitions out of an ent file.

	Used for both fresh maps and savegame loads.  A fresh map would also need
	to call ED_CallSpawnFunctions () to let the objects initialize themselves.
*/
void ED_LoadFromFile(char *data)
{
	edict_t	*eEntity = NULL;

	for(;;)
	{
		// parse the opening brace
		data = COM_Parse(data);
		if(!data)
			break;

		if(com_token[0] != '{')
			Sys_Error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if(!eEntity)
			eEntity = EDICT_NUM(0);
		else
			eEntity = ED_Alloc();

		data = ED_ParseEdict(data,eEntity);

		// Immediately call spawn function
		if(!Game->Server_SpawnEntity(eEntity))
		{
			if(developer.value)
				Edict_Print(eEntity);

			ED_Free(eEntity);
			continue;
		}
	}
}

void PR_Init (void)
{
	Cmd_AddCommand("edict",ED_PrintEdict_f);
	Cmd_AddCommand("edicts",ED_PrintEdicts);
	Cmd_AddCommand("edictcount",ED_Count);
}

edict_t *EDICT_NUM(int n)
{
	if (n < 0 || n >= sv.max_edicts)
		Sys_Error ("EDICT_NUM: bad number %i", n);

	return (edict_t*)((byte*)sv.edicts+sizeof(edict_t)*(n));
}

int NUM_FOR_EDICT(edict_t *e)
{
	int		b;

	b = ((byte*)e-(byte*)sv.edicts)/sizeof(edict_t);
	if(b < 0 || b >= sv.num_edicts)
		Sys_Error ("NUM_FOR_EDICT: bad pointer");

	return b;
}
