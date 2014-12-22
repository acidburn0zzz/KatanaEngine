/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2013 OldTimes Software

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
#include "quakedef.h"

//==============================================================================
//
//  GLOBAL FOG
//
//==============================================================================

float	fog_density,
		fog_red,
		fog_green,
		fog_blue;

float	old_density,
		old_red,
		old_green,
		old_blue;

float fade_time; //duration of fade
float fade_done; //time when fade will be done

/*	Update internal variables
*/
void Fog_Update (float density, float red, float green, float blue, float time)
{
	//save previous settings for fade
	if (time > 0)
	{
		//check for a fade in progress
		if (fade_done > cl.time)
		{
			float f;

			f = (fade_done - cl.time) / fade_time;
			old_density = f * old_density + (1.0 - f) * fog_density;
			old_red = f * old_red + (1.0 - f) * fog_red;
			old_green = f * old_green + (1.0 - f) * fog_green;
			old_blue = f * old_blue + (1.0 - f) * fog_blue;
		}
		else
		{
			old_density = fog_density;
			old_red = fog_red;
			old_green = fog_green;
			old_blue = fog_blue;
		}
	}

	fog_density = density;
	fog_red = red;
	fog_green = green;
	fog_blue = blue;
	fade_time = time;
	fade_done = cl.time + time;
}

/*	Handle an SVC_FOG message from server
*/
void Fog_ParseServerMessage(void)
{
	float density, red, green, blue, time;

	density = MSG_ReadByte()/255.0;
	red		= MSG_ReadByte()/255.0;
	green	= MSG_ReadByte()/255.0;
	blue	= MSG_ReadByte()/255.0;
	time	= Math_Max(0.0,MSG_ReadShort()/100.0);

	Fog_Update (density, red, green, blue, time);
}

/*	Handle the 'fog' console command
*/
void Fog_FogCommand_f (void)
{
	switch (Cmd_Argc())
	{
	default:
	case 1:
		Con_Printf("usage:\n");
		Con_Printf("   fog <density>\n");
		Con_Printf("   fog <red> <green> <blue>\n");
		Con_Printf("   fog <density> <red> <green> <blue>\n");
		Con_Printf("current values:\n");
		Con_Printf("   \"density\" is \"%f\"\n", fog_density);
		Con_Printf("   \"red\" is \"%f\"\n", fog_red);
		Con_Printf("   \"green\" is \"%f\"\n", fog_green);
		Con_Printf("   \"blue\" is \"%f\"\n", fog_blue);
		break;
	case 2:
		Fog_Update(Math_Max(0.0,atof(Cmd_Argv(1))),
				   fog_red,
				   fog_green,
				   fog_blue,
				   0.0);
		break;
	case 3: //TEST
		Fog_Update(Math_Max(0.0, atof(Cmd_Argv(1))),
				   fog_red,
				   fog_green,
				   fog_blue,
				   atof(Cmd_Argv(2)));
		break;
	case 4:
		Fog_Update(fog_density,
			Math_Clamp(0.0, atof(Cmd_Argv(1)), 1.0),
			Math_Clamp(0.0, atof(Cmd_Argv(2)), 1.0),
			Math_Clamp(0.0, atof(Cmd_Argv(3)), 1.0),
				   0.0);
		break;
	case 5:
		Fog_Update(Math_Max(0.0,atof(Cmd_Argv(1))),
			Math_Clamp(0.0, atof(Cmd_Argv(2)), 1.0),
			Math_Clamp(0.0, atof(Cmd_Argv(3)), 1.0),
			Math_Clamp(0.0, atof(Cmd_Argv(4)), 1.0),
				   0.0);
		break;
	case 6: //TEST
		Fog_Update(Math_Max(0.0, atof(Cmd_Argv(1))),
			Math_Clamp(0.0, atof(Cmd_Argv(2)), 1.0),
			Math_Clamp(0.0, atof(Cmd_Argv(3)), 1.0),
			Math_Clamp(0.0, atof(Cmd_Argv(4)), 1.0),
				   atof(Cmd_Argv(5)));
		break;
	}
}

/*	Called at map load
*/
void Fog_ParseWorldspawn (void)
{
	char key[128], value[4096];
	char *data;

	// Initially no fog
	fog_density = 0.0;
	old_density = 0.0;
	fade_time	= 0.0;
	fade_done	= 0.0;

	data = COM_Parse(cl.worldmodel->entities);
	if (!data)
		return; // error
	if (com_token[0] != '{')
		return; // error

	for(;;)
	{
		data = COM_Parse(data);
		if (!data)
			return; // error
		if (com_token[0] == '}')
			break; // end of worldspawn
		if (com_token[0] == '_')
			strcpy(key, com_token + 1);
		else
			strcpy(key, com_token);
		while (key[strlen(key)-1] == ' ') // remove trailing spaces
			key[strlen(key)-1] = 0;
		data = COM_Parse(data);
		if (!data)
			return; // error
		strcpy(value, com_token);

		if (!strcmp("fog", key))
			sscanf(value, "%f %f %f %f", &fog_density, &fog_red, &fog_green, &fog_blue);
	}
}

/*	Calculates fog color for this frame, taking into account fade times
*/
float *Fog_GetColor (void)
{
	static float c[4];
	float f;
	int i;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		c[0] = f * old_red + (1.0 - f) * fog_red;
		c[1] = f * old_green + (1.0 - f) * fog_green;
		c[2] = f * old_blue + (1.0 - f) * fog_blue;
		c[3] = 1.0;
	}
	else
	{
		c[0] = fog_red;
		c[1] = fog_green;
		c[2] = fog_blue;
		c[3] = 1.0;
	}

	//find closest 24-bit RGB value, so solid-colored sky can match the fog perfectly
	for (i=0;i<3;i++)
		c[i] = (float)(pMath_RINT(c[i] * 255)) / 255.0f;

	return c;
}

/*	Returns current density of fog
*/
float Fog_GetDensity(void)
{
	float f;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		return f * old_density + (1.0 - f) * fog_density;
	}
	else
		return fog_density;
}

/*	Called at the beginning of each frame
*/
void Fog_SetupFrame (void)
{
	glFogfv(GL_FOG_COLOR, Fog_GetColor());
	glFogf(GL_FOG_DENSITY, Fog_GetDensity() / 64.0);
}

/*	Called before drawing stuff that should be fogged
*/
void Fog_EnableGFog (void)
{
	if (Fog_GetDensity() > 0)
		glEnable(GL_FOG);
}

/*	Called after drawing stuff that should be fogged
*/
void Fog_DisableGFog (void)
{
	if (Fog_GetDensity() > 0)
		glDisable(GL_FOG);
}

/*	Called before drawing stuff that is additive blended -- sets fog color to black
*/
void Fog_StartAdditive (void)
{
	if (Fog_GetDensity() > 0)
	{
		vec3_t vColour = {0,0,0};

		glFogfv(GL_FOG_COLOR,vColour);
	}
}

/*	Called after drawing stuff that is additive blended -- restores fog color
*/
void Fog_StopAdditive (void)
{
	if (Fog_GetDensity() > 0)
		glFogfv(GL_FOG_COLOR, Fog_GetColor());
}

//==============================================================================
//
//  VOLUMETRIC FOG
//
//==============================================================================

cvar_t r_vfog = {"r_vfog", "1"};

void Fog_DrawVFog (void){}
void Fog_MarkModels (void){}

//==============================================================================
//
//  INIT
//
//==============================================================================

/*	Called whenever a map is loaded
*/
void Fog_NewMap (void)
{
	Fog_ParseWorldspawn (); //for global fog
	Fog_MarkModels (); //for volumetric fog
}

/*	Called when we initialize
*/
void Fog_Init (void)
{
	Cmd_AddCommand("fog",Fog_FogCommand_f);

	//Cvar_RegisterVariable (&r_vfog, NULL);

	// Set up global fog
	fog_density = 0.0f;
	fog_red		= 0.3f;
	fog_green	= 0.3f;
	fog_blue	= 0.3f;

	glFogi(GL_FOG_MODE,GL_EXP2);
}
