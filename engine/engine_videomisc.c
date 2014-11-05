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
#include "quakedef.h"

#include "engine_videoshadow.h"
#include "engine_video.h"
#include "engine_client.h"

extern cvar_t r_stereo;
extern cvar_t r_stereodepth;
extern cvar_t r_drawflat;
extern cvar_t r_flatlightstyles;
extern cvar_t gl_fullbrights;
extern cvar_t gl_farclip;
extern cvar_t gl_overbright;
extern cvar_t r_waterquality;
extern cvar_t r_oldwater;
extern cvar_t r_waterwarp;
extern cvar_t r_oldskyleaf;
extern cvar_t r_drawworld;
extern cvar_t r_showtris;
extern cvar_t r_showbboxes;
extern cvar_t r_lerpmodels;
extern cvar_t r_lerpmove;

extern float load_subdivide_size; //johnfitz -- remember what subdivide_size value was when this map was loaded

extern cvar_t gl_subdivide_size; //johnfitz -- moved here from gl_model.c

gltexture_t *playertextures[MAX_SCOREBOARD]; //johnfitz

void GL_BeginRendering(int *x,int *y,int *width,int *height)
{
	*x = *y = 0;
	*width	= Video.iWidth;
	*height = Video.iHeight;
}

void GL_EndRendering(void)
{
	if(!Video.bSkipUpdate)
		SDL_GL_SwapWindow(sMainWindow);
}

void GL_Overbright_f(void)
{
	R_RebuildAllLightmaps ();
}

void GL_Fullbrights_f (void)
{
	TexMgr_ReloadNobrightImages ();
}

void R_Novis_f (void)
{
	extern bool bVisibilityChanged;
	bVisibilityChanged = true;
}

void R_OldSkyLeaf_f (void)
{
	extern bool bVisibilityChanged;
	bVisibilityChanged = true;
}

/*	Grab six views for environment mapping tests
*/
void R_Envmap_f(void)
{
	byte	buffer[256*256*4];

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	bEnvironmentMap = true;

	r_refdef.vrect.x		=
	r_refdef.vrect.y		= 0;
	r_refdef.vrect.width	=
	r_refdef.vrect.height	= 256;

	Math_VectorClear(r_refdef.viewangles);

	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	FileSystem_WriteFile ("env0.rgb", buffer, sizeof(buffer));

	r_refdef.viewangles[1] = 90;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	FileSystem_WriteFile ("env1.rgb", buffer, sizeof(buffer));

	r_refdef.viewangles[1] = 180;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	FileSystem_WriteFile ("env2.rgb", buffer, sizeof(buffer));

	r_refdef.viewangles[1] = 270;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	FileSystem_WriteFile ("env3.rgb", buffer, sizeof(buffer));

	r_refdef.viewangles[0] = -90.0f;
	r_refdef.viewangles[1] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	FileSystem_WriteFile("env4.rgb", buffer, sizeof(buffer));

	r_refdef.viewangles[0] = 90;
	r_refdef.viewangles[1] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	FileSystem_WriteFile("env5.rgb",buffer,sizeof(buffer));

	bEnvironmentMap = false;

	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);

	GL_EndRendering();
}

void R_Init (void)
{
	extern cvar_t	gl_finish,
					r_norefresh;

	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);
	Cmd_AddCommand ("envmap", R_Envmap_f);

	Cvar_RegisterVariable (&r_norefresh, NULL);
	Cvar_RegisterVariable (&r_lightmap, NULL);
	Cvar_RegisterVariable (&r_fullbright, NULL);
	Cvar_RegisterVariable (&r_drawentities, NULL);
	Cvar_RegisterVariable(&cvVideoDrawBrushes,NULL);
	Cvar_RegisterVariable (&r_drawviewmodel, NULL);
	Cvar_RegisterVariable (&r_shadows, NULL);
	Cvar_RegisterVariable (&r_mirroralpha, NULL);
	Cvar_RegisterVariable (&r_wateralpha, NULL);
	Cvar_RegisterVariable (&r_dynamic, NULL);
	Cvar_RegisterVariable (&r_novis, R_Novis_f);
	Cvar_RegisterVariable (&r_nocull, NULL);
	Cvar_RegisterVariable (&r_speeds, NULL);
	Cvar_RegisterVariable (&gl_finish, NULL);
	Cvar_RegisterVariable (&gl_cull, NULL);
	Cvar_RegisterVariable (&gl_smoothmodels, NULL);
	Cvar_RegisterVariable (&gl_polyblend, NULL);
	Cvar_RegisterVariable (&gl_flashblend, NULL);
	Cvar_RegisterVariable (&r_stereo, NULL);
	Cvar_RegisterVariable (&r_stereodepth, NULL);
	Cvar_RegisterVariable (&r_waterquality, NULL);
	Cvar_RegisterVariable (&r_oldwater, NULL);
	Cvar_RegisterVariable (&r_waterwarp, NULL);
	Cvar_RegisterVariable (&r_drawflat, NULL);
	Cvar_RegisterVariable (&r_flatlightstyles, NULL);
	Cvar_RegisterVariable (&r_oldskyleaf, R_OldSkyLeaf_f);
	Cvar_RegisterVariable (&r_drawworld, NULL);
	Cvar_RegisterVariable (&r_showtris, NULL);
	Cvar_RegisterVariable (&r_showbboxes, NULL);
	Cvar_RegisterVariable (&gl_farclip, NULL);
	Cvar_RegisterVariable (&gl_fullbrights, GL_Fullbrights_f);
	Cvar_RegisterVariable (&gl_overbright, GL_Overbright_f);
	Cvar_RegisterVariable (&r_lerpmodels, NULL);
	Cvar_RegisterVariable (&r_lerpmove, NULL);
	Cvar_RegisterVariable (&gl_subdivide_size, NULL);

	R_InitExperimental();

	Sky_Init();
	Fog_Init();
}

void R_NewMap (void)
{
	int	i;

	for(i = 0; i < 256; i++)
		d_lightstylevalue[i] = 264;		// normal light value

// clear out efrags in case the level hasn't been reloaded
// FIXME: is this one short?
	for(i = 0; i < cl.worldmodel->numleafs; i++)
		cl.worldmodel->leafs[i].efrags = NULL;

	r_viewleaf = NULL;

	Client_ClearEffects();

	GL_BuildLightmaps ();

	r_framecount = 0; //johnfitz -- paranoid?
	r_visframecount = 0; //johnfitz -- paranoid?

	Sky_NewMap (); //johnfitz -- skybox in worldspawn
	Fog_NewMap (); //johnfitz -- global fog in worldspawn

	load_subdivide_size = gl_subdivide_size.value; //johnfitz -- is this the right place to set this?
}

/*	For program optimization
*/
void R_TimeRefresh_f (void)
{
	int			i;
	float		start, stop, time;

	glDrawBuffer  (GL_FRONT);
	glFinish ();

	start = System_DoubleTime ();
	for (i=0 ; i<128 ; i++)
	{
		r_refdef.viewangles[1] = i/128.0f*360.0f;
		R_RenderView ();
	}

	glFinish ();
	stop = System_DoubleTime ();
	time = stop-start;
	Con_Printf("%f seconds (%f fps)\n",time,128/time);

	glDrawBuffer (GL_BACK);
	GL_EndRendering ();
}
