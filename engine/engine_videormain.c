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
#include "quakedef.h"

#include "engine_editor.h"
#include "engine_console.h"
#include "engine_videoshadow.h"
#include "engine_video.h"
#include "KatAlias.h"

vec3_t		modelorg, r_entorigin;
entity_t	*currententity;

int	r_visframecount,	// bumped when going to a new PVS
	r_framecount;		// used for dlight push checking

mplane_t	frustum[4];

//johnfitz -- rendering statistics
int rs_brushpolys, rs_aliaspolys, rs_skypolys, rs_particles, rs_fogpolys;
int rs_dynamiclightmaps, rs_brushpasses, rs_aliaspasses, rs_skypasses;
float rs_megatexels;

bool	bEnvironmentMap;				// True during envmap command capture

// view origin
vec3_t	vup,vpn,vright,
		r_origin;

float	r_world_matrix[16],r_base_world_matrix[16];

float r_fovx, r_fovy; //johnfitz -- rendering fov may be different becuase of r_waterwarp and r_stereo

// screen size info
refdef_t	r_refdef;

mleaf_t		*r_viewleaf,*r_oldviewleaf;

int		d_lightstylevalue[256];	// 8.8 fraction of base light value

bool		bMirror;
mplane_t	*mpMirrorPlane;

cvar_t	r_norefresh				= {	"r_norefresh",			"0"					};
cvar_t	r_drawentities			= {	"r_drawentities",		"1"					};
cvar_t	r_drawviewmodel			= {	"r_drawviewmodel",		"1"					},
		cvVideoDrawBrushes		= {	"video_drawbrushes",	"1",		false,	true,	"Toggles whether brushes are drawn in the level."				};
cvar_t	r_speeds				= {	"r_speeds",				"0"					};
cvar_t	r_fullbright			= {	"r_fullbright",			"0"					};
cvar_t	r_lightmap				= {	"r_lightmap",			"0"					};
// [23/7/2012] Defaults to 2 (both classic and blob) ~hogsy
cvar_t	r_shadows				= {	"r_shadows",			"2",		true,	false,	"0 = Disabled\n1 = Blob shadows\n3 = Blob and planar shadows"	};
cvar_t	r_mirroralpha			= {	"r_mirroralpha",		"0.5"				};
cvar_t	r_wateralpha			= {	"r_wateralpha",			"0.98",		true,	false,	"Changes the level of alpha for water surfaces."				};
cvar_t	r_dynamic				= {	"r_dynamic",			"1"					};
cvar_t	r_novis					= {	"r_novis",				"0"					};
cvar_t	r_nocull				= {	"r_nocull",				"0"					};
cvar_t	gl_finish				= {	"gl_finish",			"0"					};
cvar_t	gl_cull					= {	"gl_cull",				"1"					};
cvar_t	gl_smoothmodels			= {	"gl_smoothmodels",		"1"					};
cvar_t	gl_polyblend			= {	"gl_polyblend",			"1"					};
cvar_t	gl_flashblend			= {	"gl_flashblend",		"0"					};
cvar_t	r_stereo				= {	"r_stereo",				"0"					};
cvar_t	r_stereodepth			= {	"r_stereodepth",		"128"				};
cvar_t	r_drawflat				= {	"r_drawflat",			"0"					};
cvar_t	r_flatlightstyles		= {	"r_flatlightstyles",	"0"					};
cvar_t	gl_fullbrights			= {	"gl_fullbrights",		"1",		true	};
cvar_t	gl_farclip				= {	"gl_farclip",			"16384",	true	};
cvar_t	gl_overbright			= {	"gl_overbright",		"1",		true	};
cvar_t	r_oldskyleaf			= {	"r_oldskyleaf",			"0"					};
cvar_t	r_drawworld				= {	"r_drawworld",			"1"					};
cvar_t	r_showtris				= {	"r_showtris",			"0"					};
cvar_t	r_showbboxes			= {	"r_showbboxes",			"0"					};
cvar_t	r_lerpmodels			= {	"r_lerpmodels",			"1"					};
cvar_t	r_lerpmove				= {	"r_lerpmove",			"1"					};

/*	Returns true if the box is completely outside the frustum
*/
bool R_CullBox (vec3_t emins, vec3_t emaxs)
{
	int			i;
	mplane_t	*p;

	for (i = 0;i < 4;i++)
	{
		p = frustum + i;
		switch(p->signbits)
		{
		default:
		case 0:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 1:
			if (p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 2:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 3:
			if (p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2] < p->dist)
				return true;
			break;
		case 4:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		case 5:
			if (p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		case 6:
			if (p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		case 7:
			if (p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2] < p->dist)
				return true;
			break;
		}
	}
	return false;
}

bool R_CullModelForEntity(entity_t *e)
{
	vec3_t mins, maxs;

	if(r_nocull.value == 1 || e == &cl.viewent)
		return false;

	if(e->angles[PITCH] || e->angles[ROLL])
	{
		Math_VectorAdd (e->origin, e->model->rmins, mins);
		Math_VectorAdd (e->origin, e->model->rmaxs, maxs);
	}
	else if(e->angles[YAW])
	{
		Math_VectorAdd (e->origin, e->model->ymins, mins);
		Math_VectorAdd (e->origin, e->model->ymaxs, maxs);
	}
	else //no rotation
	{
		Math_VectorAdd(e->origin,e->model->mins,mins);
		Math_VectorAdd(e->origin,e->model->maxs,maxs);
	}

	return R_CullBox(mins, maxs);
}

void R_RotateForEntity (vec3_t origin, vec3_t angles)
{
	glTranslatef(origin[0],origin[1],origin[2]);
	glRotatef(angles[1],0,0,1);
	glRotatef(-angles[0],0,1,0);
	glRotatef(angles[2],1,0,0);
}

/*	Negative offset moves polygon closer to camera
*/
void GL_PolygonOffset (int offset)
{
	if (offset > 0)
	{
		glEnable (GL_POLYGON_OFFSET_FILL);
		glEnable (GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(1, offset);
	}
	else if (offset < 0)
	{
		glEnable (GL_POLYGON_OFFSET_FILL);
		glEnable (GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1, offset);
	}
	else
	{
		glDisable (GL_POLYGON_OFFSET_FILL);
		glDisable (GL_POLYGON_OFFSET_LINE);
	}
}

int SignbitsForPlane (mplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test
	bits = 0;
	for (j=0 ; j<3 ; j++)
		if (out->normal[j] < 0)
			bits |= 1<<j;

	return bits;
}

/*	turn forward towards side on the plane defined by forward and side
	if angle = 90, the result will be equal to side
	assumes side and forward are perpendicular, and normalized
	to turn away from side, use a negative angle
*/
void TurnVector (vec3_t out, const vec3_t forward, const vec3_t side, float angle)
{
	float scale_forward, scale_side;

	scale_forward = cos(DEG2RAD(angle));
	scale_side = sin(DEG2RAD(angle));

	out[0] = scale_forward*forward[0] + scale_side*side[0];
	out[1] = scale_forward*forward[1] + scale_side*side[1];
	out[2] = scale_forward*forward[2] + scale_side*side[2];
}

void R_SetFrustum (float fovx, float fovy)
{
	int		i;

	if (r_stereo.value)
		fovx += 10; //silly hack so that polygons don't drop out becuase of stereo skew

	TurnVector(frustum[0].normal, vpn, vright, fovx/2 - 90); //left plane
	TurnVector(frustum[1].normal, vpn, vright, 90 - fovx/2); //right plane
	TurnVector(frustum[2].normal, vpn, vup, 90 - fovy/2); //bottom plane
	TurnVector(frustum[3].normal, vpn, vup, fovy/2 - 90); //top plane

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = Math_DotProduct(r_origin, frustum[i].normal); //FIXME: shouldn't this always be zero?
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}

#define NEARCLIP 4
float frustum_skew = 0.0; //used by r_stereo
void GL_SetFrustum(float fovx, float fovy)
{
	float xmax, ymax;
	xmax = NEARCLIP * tan( fovx * pMath_PI / 360.0 );
	ymax = NEARCLIP * tan( fovy * pMath_PI / 360.0 );
	glFrustum(-xmax + frustum_skew, xmax + frustum_skew, -ymax, ymax, NEARCLIP, gl_farclip.value);
}

void R_RenderScene(void);

void R_Mirror(void)
{
	float		d;
	entity_t	*eEntity;

	if(!bMirror)
		return;

	memcpy(r_base_world_matrix,r_world_matrix,sizeof(r_base_world_matrix));

	d = Math_DotProduct(r_refdef.vieworg,mpMirrorPlane->normal)-mpMirrorPlane->dist;
	Math_VectorMA(r_refdef.vieworg, -2*d, mpMirrorPlane->normal, r_refdef.vieworg);

	d = Math_DotProduct(vpn,mpMirrorPlane->normal);
	Math_VectorMA(vpn,-2*d,mpMirrorPlane->normal,vpn);

	r_refdef.viewangles[0] = -asin(vpn[2])/pMath_PI*180;
	r_refdef.viewangles[1] = atan2(vpn[1],vpn[0])/pMath_PI*180;
	r_refdef.viewangles[2] = -r_refdef.viewangles[2];

	eEntity = &cl_entities[cl.viewentity];
	if (cl_numvisedicts < MAX_VISEDICTS)
	{
		cl_visedicts[cl_numvisedicts] = eEntity;
		cl_numvisedicts++;
	}

	glDepthRange(0,0.5);
	glDepthFunc(GL_LEQUAL);

	R_RenderScene();

	Video_ResetCapabilities(false);

	Video_EnableCapabilities(VIDEO_BLEND);

	glDepthRange(0,1);
	glDepthFunc(GL_LEQUAL);
	glMatrixMode(GL_PROJECTION);

	if(mpMirrorPlane->normal[2])
		glScalef(1.0f,-1.0f,1.0f);
	else
		glScalef(-1.0f,1.0f,1.0f);

	glCullFace(GL_FRONT);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(r_base_world_matrix);
	glColor4f(1.0f,1.0f,1.0f,r_mirroralpha.value);

	Video_ResetCapabilities(true);
}

void R_MarkSurfaces(void);          // [25/11/2013] See r_world.c ~hogsy
void R_CullSurfaces(void);          // [25/11/2013] See r_world.c ~hogsy
void R_UpdateWarpTextures(void);    // [25/11/2013] See gl_warp.c ~hogsy

void R_SetupView (void)
{
	Fog_SetupFrame (); //johnfitz

// build the transformation matrix for the given view angles
	Math_VectorCopy(r_refdef.vieworg,r_origin);
	Math_AngleVectors(r_refdef.viewangles,vpn,vright,vup);

	// Current viewleaf
	r_oldviewleaf	= r_viewleaf;
	r_viewleaf		= Mod_PointInLeaf (r_origin, cl.worldmodel);

	V_SetContentsColor(r_viewleaf->contents);

	View_CalculateBlend();

	//johnfitz -- calculate r_fovx and r_fovy here
	r_fovx = r_refdef.fov_x;
	r_fovy = r_refdef.fov_y;
	if (r_waterwarp.value)
	{
		int contents = Mod_PointInLeaf (r_origin, cl.worldmodel)->contents;
		if (contents == BSP_CONTENTS_WATER || contents == BSP_CONTENTS_SLIME || contents == BSP_CONTENTS_LAVA)
		{
			//variance is a percentage of width, where width = 2 * tan(fov / 2) otherwise the effect is too dramatic at high FOV and too subtle at low FOV.  what a mess!
			r_fovx = atan(tan(DEG2RAD(r_refdef.fov_x) / 2) * (0.97 + sin(cl.time * 1.5) * 0.03)) * 2 / M_PI_DIV_180;
			r_fovy = atan(tan(DEG2RAD(r_refdef.fov_y) / 2) * (1.03 - sin(cl.time * 1.5) * 0.03)) * 2 / M_PI_DIV_180;
		}
	}
	//johnfitz

	R_SetFrustum(r_fovx, r_fovy); //johnfitz -- use r_fov* vars
	R_MarkSurfaces(); //johnfitz -- create texture chains from PVS
	R_CullSurfaces(); //johnfitz -- do after R_SetFrustum and R_MarkSurfaces
	R_UpdateWarpTextures(); //johnfitz -- do this before R_Clear

	Video_ClearBuffer();

	//johnfitz -- cheat-protect some draw modes
	r_drawflat_cheatsafe = r_fullbright_cheatsafe = r_lightmap_cheatsafe = false;
	r_drawworld_cheatsafe = true;
	if (cl.maxclients == 1)
	{
		if (!r_drawworld.value)
			r_drawworld_cheatsafe = false;

		if (r_drawflat.value)
			r_drawflat_cheatsafe = true;
		else if (r_fullbright.value || !cl.worldmodel->lightdata)
			r_fullbright_cheatsafe = true;
		else if (r_lightmap.value)
			r_lightmap_cheatsafe = true;
	}
	//johnfitz
}

void R_DrawEntitiesOnList(bool bAlphaPass) //johnfitz -- added parameter
{
	int i;

	if(!r_drawentities.value)
		return;

	//johnfitz -- sprites are not a special case
	for(i = 0; i < cl_numvisedicts; i++)
	{
		currententity = cl_visedicts[i];

		//johnfitz -- if alphapass is true, draw only alpha entites this time
		//if alphapass is false, draw only nonalpha entities this time
		if ((ENTALPHA_DECODE(currententity->alpha) < 1 && !bAlphaPass) ||
			(ENTALPHA_DECODE(currententity->alpha) == 1 && bAlphaPass))
			continue;

		//johnfitz -- chasecam
		if (currententity == &cl_entities[cl.viewentity])
			currententity->angles[0] *= 0.3f;
		//johnfitz

		switch(currententity->model->mType)
		{
			case MODEL_TYPE_MD2:
				Alias_Draw(currententity);
				break;
			case MODEL_TYPE_OBJ:
				Model_DrawOBJ(currententity);
				break;
			case MODEL_TYPE_BSP:
				Brush_Draw(currententity);
				break;
			case MODEL_TYPE_SPRITE:
                Sprite_Draw(currententity);
                break;
			default:
				Console_ErrorMessage(false,currententity->model->name,"Unrecognised model type.");
		}
	}
}

void R_DrawViewModel(void)
{
	if(!cvVideoDrawModels.value || !r_drawviewmodel.value || !r_drawentities.value || chase_active.value || bEnvironmentMap)
		return;
	else if(cl.items & IT_INVISIBILITY || cl.stats[STAT_HEALTH] <= 0)
		return;

	currententity = &cl.viewent;
	if(!currententity->model || currententity->model->mType != MODEL_TYPE_MD2)
		return;

	// hack the depth range to prevent view model from poking into walls
	glDepthRange(0,0.3);

	Alias_Draw(currententity);

	glDepthRange(0,1);
}

void R_EmitWirePoint (vec3_t origin)
{
	glBegin(GL_LINES);
	glVertex3f(origin[0]-4.0f,origin[1],origin[2]);
	glVertex3f(origin[0]+4.0f,origin[1],origin[2]);
	glVertex3f(origin[0],origin[1]-4.0f,origin[2]);
	glVertex3f(origin[0],origin[1]+4.0f,origin[2]);
	glVertex3f(origin[0],origin[1],origin[2]-4.0f);
	glVertex3f(origin[0],origin[1],origin[2]+4.0f);
	glEnd();
}

void R_EmitWireBox(vec3_t mins,vec3_t maxs)
{
	glColor4f(0.2f,0,0.5f,0.5f);

	glBegin(GL_QUADS);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glVertex3fv(maxs);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glVertex3fv(mins);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glVertex3fv(maxs);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glVertex3fv(mins);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glVertex3fv(maxs);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glVertex3fv(mins);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glEnd();

	glColor4f(0.2f,0,0.5f,1.0f);

	glBegin(GL_LINES);
	glVertex3fv(mins);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3fv(mins);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glVertex3fv(maxs);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],maxs[1],maxs[2]);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glVertex3fv(mins);
	glVertex3f(mins[0],mins[1],maxs[2]);
	glVertex3f(maxs[0],maxs[1],mins[2]);
	glVertex3fv(maxs);
	glVertex3f(mins[0],maxs[1],mins[2]);
	glVertex3f(mins[0],maxs[1],maxs[2]);
	glVertex3f(maxs[0],mins[1],mins[2]);
	glVertex3f(maxs[0],mins[1],maxs[2]);
	glEnd();
}

/*	draw bounding boxes -- the server-side boxes, not the renderer cullboxes
*/
void R_ShowBoundingBoxes(void)
{
	extern		edict_t *sv_player;
	vec3_t				mins,maxs;
	edict_t				*ed;
	int					i;

	if(!r_showbboxes.value || cl.maxclients > 1 || !r_drawentities.value || !sv.active)
		return;

    Video_DisableCapabilities(VIDEO_DEPTH_TEST|VIDEO_TEXTURE_2D);
    Video_EnableCapabilities(VIDEO_BLEND);

	for (i=0, ed=NEXT_EDICT(sv.edicts) ; i<sv.num_edicts ; i++, ed=NEXT_EDICT(ed))
	{
		if(ed == sv_player && !chase_active.value)
			continue;

		if(ed->v.mins[0] == ed->v.maxs[0] && ed->v.mins[1] == ed->v.maxs[1] && ed->v.mins[2] == ed->v.maxs[2])
		{
			glColor3f(1.0f,1.0f,1.0f);

			R_EmitWirePoint (ed->v.origin);
		}
		else
		{
			Math_VectorAdd(ed->v.mins,ed->v.origin,mins);
			Math_VectorAdd(ed->v.maxs,ed->v.origin,maxs);

			R_EmitWireBox(mins,maxs);
		}
	}

	glColor3f(1.0f,1.0f,1.0f);

    Video_DisableCapabilities(VIDEO_BLEND);
	Video_EnableCapabilities(VIDEO_TEXTURE_2D|VIDEO_DEPTH_TEST);
}

void R_DrawShadows (void)
{
	int i;

	if(!r_shadows.value || !r_drawentities.value || r_drawflat_cheatsafe || r_lightmap_cheatsafe)
		return;

	for(i = 0; i < cl_numvisedicts; i++)
	{
		currententity = cl_visedicts[i];

		if(currententity == &cl.viewent)
			return;

		Draw_Shadow(currententity);
	}
}

void R_RenderScene(void)
{
	R_PushDlights();
	Light_Animate();

	r_framecount++;

	// [15/2/2014] Setup OpenGL; moved this over here since it's not used anywhere else ~hogsy
	{
		//johnfitz -- rewrote this section
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity ();
		glViewport (glx + r_refdef.vrect.x,
					gly + glheight - r_refdef.vrect.y - r_refdef.vrect.height,
					r_refdef.vrect.width,
					r_refdef.vrect.height);
		//johnfitz

		GL_SetFrustum(r_fovx,r_fovy); //johnfitz -- use r_fov* vars

		if(bMirror)
		{
			if(mpMirrorPlane->normal[2])
				glScalef(1,-1,1);
			else
				glScalef(-1,1,1);

			glCullFace(GL_BACK);
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);	    // put Z going up
		glRotatef(90,0,0,1);	    // put Z going up
		glRotatef (-r_refdef.viewangles[2],  1, 0, 0);
		glRotatef (-r_refdef.viewangles[0],  0, 1, 0);
		glRotatef (-r_refdef.viewangles[1],  0, 0, 1);
		glTranslatef (-r_refdef.vieworg[0],  -r_refdef.vieworg[1],  -r_refdef.vieworg[2]);
		glGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);

		// set drawing parms
		if (gl_cull.value)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

        Video_DisableCapabilities(VIDEO_BLEND|VIDEO_ALPHA_TEST);
        Video_EnableCapabilities(VIDEO_DEPTH_TEST);
	}

	Fog_EnableGFog(); //johnfitz

	Sky_Draw();		//johnfitz
	World_Draw();
	Editor_Draw();

	S_ExtraUpdate(); // don't let sound get messed up if going slow

	R_DrawShadows();
	R_DrawEntitiesOnList(false);
	World_DrawWaterTextureChains();
	R_DrawEntitiesOnList(true);
	Draw_Particles();
	R_DrawFlares();
	Light_Draw();

	Fog_DisableGFog();

	R_DrawViewModel();
	R_ShowBoundingBoxes();
	R_Mirror();
}

void R_RenderView (void)
{
	double	time1 = 0,
			time2;

	if (r_norefresh.value)
		return;

	if (!cl.worldmodel)
		Sys_Error ("R_RenderView: NULL worldmodel");

	if(r_speeds.value)
	{
		glFinish ();
		time1 = System_DoubleTime();

		//johnfitz -- rendering statistics
		rs_brushpolys = rs_aliaspolys = rs_skypolys = rs_particles = rs_fogpolys = rs_megatexels =
		rs_dynamiclightmaps = rs_aliaspasses = rs_skypasses = rs_brushpasses = 0;
	}
	else if (gl_finish.value)
		glFinish ();

	R_SetupView (); //johnfitz -- this does everything that should be done once per frame

	//johnfitz -- stereo rendering -- full of hacky goodness
	if (r_stereo.value)
	{
		float	eyesep = CLAMP(-8.0f,r_stereo.value,8.0f),
				fdepth = CLAMP(32.0f,r_stereodepth.value,1024.0f);

		Math_AngleVectors(r_refdef.viewangles, vpn, vright, vup);

		// Render left eye (red)
		glColorMask(true,false,false,true);
		Math_VectorMA (r_refdef.vieworg, -0.5f * eyesep, vright, r_refdef.vieworg);
		frustum_skew = 0.5f*eyesep*NEARCLIP/fdepth;
		srand((int) (cl.time * 1000)); //sync random stuff between eyes

		R_RenderScene();

		// Render right eye (cyan)
		glClear (GL_DEPTH_BUFFER_BIT);
		glColorMask(0, 1, 1, 1);
		Math_VectorMA (r_refdef.vieworg, 1.0f * eyesep, vright, r_refdef.vieworg);
		frustum_skew = -frustum_skew;
		srand((int) (cl.time * 1000)); //sync random stuff between eyes

		R_RenderScene();

		// Restore
		glColorMask(true,true,true,true);
		Math_VectorMA (r_refdef.vieworg, -0.5f * eyesep, vright, r_refdef.vieworg);
		frustum_skew = 0.0f;
	}
	else
		R_RenderScene();
	//johnfitz

	//johnfitz -- modified r_speeds output
	time2 = System_DoubleTime();
	if(r_speeds.value == 2)
		Con_Printf(	"%3i ms  %4i/%4i wpoly %4i/%4i epoly %3i lmap %4i/%4i sky %1.1f mtex\n",
					(int)((time2-time1)*1000),
					rs_brushpolys,
					rs_brushpasses,
					rs_aliaspolys,
					rs_aliaspasses,
					rs_dynamiclightmaps,
					rs_skypolys,
					rs_skypasses,
					TexMgr_FrameUsage ());
	else if(r_speeds.value)
		Con_Printf ("%3i ms  %4i wpoly %4i epoly %3i lmap\n",
					(int)((time2-time1)*1000),
					rs_brushpolys,
					rs_aliaspolys,
					rs_dynamiclightmaps);
	//johnfitz
}

