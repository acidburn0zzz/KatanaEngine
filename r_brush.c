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

/*
	Brush model rendering.
*/

#include "engine_video.h"

extern cvar_t gl_fullbrights, r_drawflat, gl_overbright, r_oldwater; //johnfitz

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	BLOCK_WIDTH

unsigned	blocklights[BLOCK_WIDTH*BLOCK_HEIGHT*3]; //johnfitz -- was 18*18, added lit support (*3) and loosened surface extents maximum (BLOCK_WIDTH*BLOCK_HEIGHT)

typedef struct glRect_s {
	unsigned char l,t,w,h;
} glRect_t;

glpoly_t	*lightmap_polys[MAX_LIGHTMAPS];
bool	bLightmapModified[MAX_LIGHTMAPS];
glRect_t	lightmap_rectchange[MAX_LIGHTMAPS];

int			allocated[MAX_LIGHTMAPS][BLOCK_WIDTH];

// the lightmap texture data needs to be kept in
// main memory so texsubimage can update properly
byte		lightmaps[LIGHTMAP_BYTES*MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];

void R_RenderDynamicLightmaps (msurface_t *fa);
void R_BuildLightMap (msurface_t *surf, byte *dest, int stride);
void R_UploadLightmap (int lmap);

/*	johnfitz -- added "frame" param to eliminate use of "currententity" global
*/
texture_t *R_TextureAnimation (texture_t *base, int frame)
{
	int		relative;
	int		count;

	if (frame)
		if (base->alternate_anims)
			base = base->alternate_anims;

	if (!base->anim_total)
		return base;

	relative = (int)(cl.time*10) % base->anim_total;

	count = 0;
	while (base->anim_min > relative || base->anim_max <= relative)
	{
		base = base->anim_next;
		if (!base)
			Sys_Error ("R_TextureAnimation: broken cycle");
		if (++count > 100)
			Sys_Error ("R_TextureAnimation: infinite cycle");
	}

	return base;
}

void DrawGLPoly(glpoly_t *p)
{
	float	*v;
	int		i;

	glBegin(GL_TRIANGLE_FAN);

	v = p->verts[0];
	for(i = 0; i < p->numverts; i++, v += VERTEXSIZE)
	{
        if(!r_showtris.value)
            glTexCoord2f(v[3],v[4]);
		glVertex3fv(v);
	}

	glEnd();
}

/*
=============================================================

	BRUSH MODELS

=============================================================
*/

void R_DrawSequentialPoly(msurface_t *s)
{
#if 1
    int         i;
    float       fAlpha;
    texture_t   *tAnimation;

    fAlpha      = ENTALPHA_DECODE(currententity->alpha);
    tAnimation  = R_TextureAnimation(s->texinfo->texture,currententity->frame);

    Video_ResetCapabilities(false);

	if(s->flags & SURF_DRAWSKY)
		return;
    else if(s->flags & SURF_NOTEXTURE)
	{
		if(fAlpha < 1.0f)
		{
            Video_SetBlend(VIDEO_DEPTH_FALSE,VIDEO_BLEND_IGNORE);
            Video_EnableCapabilities(VIDEO_BLEND);

			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			glColor4f(1.0f,1.0f,1.0f,fAlpha);
		}

		Video_SetTexture(tAnimation->gltexture);

		DrawGLPoly(s->polys);

		rs_brushpasses++;

		if(fAlpha < 1.0f)
		{
			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			glColor3f(1.0f,1.0f,1.0f);
		}
	}
    else if(s->flags & SURF_DRAWTURB)
	{
        glpoly_t *pBrushPoly;

		if(currententity->alpha == ENTALPHA_DEFAULT)
			fAlpha = CLAMP(0.0,r_wateralpha.value,1.0f);

		if(fAlpha < 1.0f)
		{
            Video_SetBlend(VIDEO_BLEND_IGNORE,VIDEO_DEPTH_FALSE);
            Video_EnableCapabilities(VIDEO_BLEND);

			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			glColor4f(1.0f,1.0f,1.0f,fAlpha);
		}

        Video_SetTexture(s->texinfo->texture->gltexture);

        for(pBrushPoly = s->polys->next; pBrushPoly; pBrushPoly = pBrushPoly->next)
        {
            Warp_DrawWaterPoly(pBrushPoly);

            rs_brushpasses++;
        }

        rs_brushpasses++;

#if 0   // New water
		{
			Video_SetTexture(s->texinfo->texture->warpimage);
			s->texinfo->texture->update_warp = true; // FIXME: one frame too late!
			DrawGLPoly (s->polys);
			rs_brushpasses++;
		}
#endif

		if(fAlpha < 1.0f)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);
			glColor3f(1.0f,1.0f,1.0f);
		}
	}
	else
	{
        float   *fVert;

        if(fAlpha < 1.0f)
		{
            Video_SetBlend(VIDEO_BLEND_IGNORE,VIDEO_DEPTH_FALSE);
            Video_EnableCapabilities(VIDEO_BLEND);

			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			glColor4f(1.0f,1.0f,1.0f,fAlpha);
		}
		else
            Video_EnableCapabilities(VIDEO_ALPHA_TEST);

        Video_SetTexture(tAnimation->gltexture);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        Video_SelectTexture(1);
        Video_EnableCapabilities(VIDEO_TEXTURE_2D);
        Video_SetTexture(lightmap_textures[s->lightmaptexturenum]);

        R_RenderDynamicLightmaps(s);
        R_UploadLightmap(s->lightmaptexturenum);

        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE);
        glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB,GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE0_RGB,GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV,GL_SOURCE1_RGB,GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV,GL_RGB_SCALE,4);

        glBegin(GL_TRIANGLE_FAN);

        fVert = s->polys->verts[0];
        for(i = 0; i < s->polys->numverts; i++,fVert += VERTEXSIZE)
        {
            glMultiTexCoord2fv(VIDEO_TEXTURE0,fVert+3);
            glMultiTexCoord2fv(VIDEO_TEXTURE1,fVert+5);
            glVertex3fv(fVert);
        }

        glEnd();

        glTexEnvf(GL_TEXTURE_ENV,GL_RGB_SCALE,1.0f);
        glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);

        Video_SelectTexture(0);

        if(fAlpha < 1.0f)
        {
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);
			glColor3f(1.0f,1.0f,1.0f);
        }

        rs_brushpasses++;
	}

	Video_ResetCapabilities(true);
#else
	glpoly_t	*p;
	texture_t	*t;
	float		*v;
	float		entalpha;
	int			i;

	t = R_TextureAnimation (s->texinfo->texture, currententity->frame);
	entalpha = ENTALPHA_DECODE(currententity->alpha);

// drawflat
	if (r_drawflat_cheatsafe)
	{
		if ((s->flags & SURF_DRAWTURB) && r_oldwater.value)
		{
			for (p = s->polys->next; p; p = p->next)
			{
				srand((unsigned int) p);
				glColor3f (rand()%256/255.0, rand()%256/255.0, rand()%256/255.0);
				DrawGLPoly (p);
				rs_brushpasses++;
			}
			return;
		}

		srand((unsigned int) s->polys);
		glColor3f (rand()%256/255.0, rand()%256/255.0, rand()%256/255.0);
		DrawGLPoly (s->polys);
		rs_brushpasses++;
		return;
	}

// fullbright
	if((r_fullbright_cheatsafe) && !(s->flags & SURF_DRAWTILED))
	{
		if(entalpha < 1.0f)
		{
			Video_EnableCapabilities(VIDEO_BLEND);
			Video_SetBlend(VIDEO_BLEND_IGNORE,VIDEO_DEPTH_FALSE);

			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			glColor4f(1.0f,1.0f,1.0f,entalpha);
		}

		Video_SetTexture(t->gltexture);
		DrawGLPoly (s->polys);
		rs_brushpasses++;

		if(entalpha < 1.0f)
		{
			Video_DisableCapabilities(VIDEO_BLEND);
			Video_SetBlend(VIDEO_BLEND_IGNORE,VIDEO_DEPTH_TRUE);

			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			glColor3f(1.0f,1.0f,1.0f);
		}

		goto fullbrights;
	}

// r_lightmap
	if (r_lightmap_cheatsafe)
	{
		if (s->flags & SURF_DRAWTILED)
		{
			Video_DisableCapabilities(VIDEO_TEXTURE_2D);

			DrawGLPoly (s->polys);

			Video_EnableCapabilities(VIDEO_TEXTURE_2D);

			rs_brushpasses++;
			return;
		}

		R_RenderDynamicLightmaps(s);

		Video_SetTexture(lightmap_textures[s->lightmaptexturenum]);

		R_UploadLightmap(s->lightmaptexturenum);

		if (!gl_overbright.value)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor3f(0.5f,0.5f,0.5f);
		}

		glBegin(GL_TRIANGLE_FAN);

		v = s->polys->verts[0];
		for (i=0 ; i<s->polys->numverts ; i++, v+= VERTEXSIZE)
		{
			glTexCoord2f (v[5], v[6]);
			glVertex3fv (v);
		}

		glEnd();

		if(!gl_overbright.value)
		{
			glColor3f(1.0f,1.0f,1.0f);
			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		}

		rs_brushpasses++;

		return;
	}

// missing texture


// lightmapped poly
	if(entalpha < 1.0f)
	{
		glDepthMask(false);
		glEnable(GL_BLEND);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4f(1.0f,1.0f,1.0f,entalpha);
	}
	else
		glColor3f(5.0f,1.0f,1.0f);

	if(gl_overbright.value)
	{
		else if (entalpha < 1.0f) //case 2: can't do multipass if entity has alpha, so just draw the texture
		{
			Video_SetTexture(t->gltexture);
			DrawGLPoly (s->polys);
			rs_brushpasses++;
		}
		else //case 3: texture in one pass, lightmap in second pass using 2x modulation blend func, fog in third pass
		{
			//first pass -- texture with no fog
			Fog_DisableGFog ();
			Video_SetTexture(t->gltexture);
			DrawGLPoly (s->polys);

			Fog_EnableGFog ();

			rs_brushpasses++;

			//second pass -- lightmap with black fog, modulate blended
			R_RenderDynamicLightmaps (s);

			Video_SetTexture(lightmap_textures[s->lightmaptexturenum]);

			R_UploadLightmap(s->lightmaptexturenum);

			glDepthMask(false);
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //2x modulate

			Fog_StartAdditive ();

			glBegin(GL_POLYGON);

			v = s->polys->verts[0];
			for (i=0 ; i<s->polys->numverts ; i++, v+= VERTEXSIZE)
			{
				glTexCoord2f (v[5], v[6]);
				glVertex3fv (v);
			}

			glEnd ();

			Fog_StopAdditive ();

			rs_brushpasses++;

			//third pass -- black geo with normal fog, additive blended
			if (Fog_GetDensity() > 0)
			{
				glBlendFunc(GL_ONE, GL_ONE); //add
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glColor3f(0,0,0);
				DrawGLPoly (s->polys);
				glColor3f(1,1,1);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				rs_brushpasses++;
			}

			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable (GL_BLEND);
			glDepthMask (GL_TRUE);
		}
	}
	else
	{
		if(Video.bMultitexture) //case 4: texture and lightmap in one pass, regular modulation
		{
			Video_DisableMultitexture(); // selects TEXTURE0
			Video_SetTexture(t->gltexture);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			Video_EnableMultitexture(); // selects TEXTURE1
			Video_SetTexture(lightmap_textures[s->lightmaptexturenum]);

			R_RenderDynamicLightmaps (s);
			R_UploadLightmap(s->lightmaptexturenum);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBegin(GL_POLYGON);

			v = s->polys->verts[0];
			for (i=0 ; i<s->polys->numverts ; i++, v+= VERTEXSIZE)
			{
				glMultiTexCoord2fARB(VIDEO_TEXTURE0,v[3],v[4]);
				glMultiTexCoord2fARB(VIDEO_TEXTURE1,v[5],v[6]);
				glVertex3fv (v);
			}
			glEnd ();

			Video_DisableMultitexture();

			rs_brushpasses++;
		}
		else if (entalpha < 1) //case 5: can't do multipass if entity has alpha, so just draw the texture
		{
			Video_SetTexture(t->gltexture);
			DrawGLPoly (s->polys);
			rs_brushpasses++;
		}
		else //case 6: texture in one pass, lightmap in a second pass, fog in third pass
		{
			//first pass -- texture with no fog
			Fog_DisableGFog ();
			Video_SetTexture(t->gltexture);
			DrawGLPoly (s->polys);
			Fog_EnableGFog ();
			rs_brushpasses++;

			//second pass -- lightmap with black fog, modulate blended
			R_RenderDynamicLightmaps(s);
			Video_SetTexture(lightmap_textures[s->lightmaptexturenum]);
			R_UploadLightmap(s->lightmaptexturenum);

			glDepthMask (GL_FALSE);
			glEnable (GL_BLEND);
			glBlendFunc (GL_ZERO, GL_SRC_COLOR); //modulate
			Fog_StartAdditive ();

			glBegin(GL_POLYGON);
			v = s->polys->verts[0];
			for (i=0 ; i<s->polys->numverts ; i++, v+= VERTEXSIZE)
			{
				glTexCoord2f (v[5], v[6]);
				glVertex3fv (v);
			}
			glEnd();

			Fog_StopAdditive ();
			rs_brushpasses++;

			//third pass -- black geo with normal fog, additive blended
			if (Fog_GetDensity() > 0)
			{
				glBlendFunc(GL_ONE, GL_ONE); //add
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glColor3f(0,0,0);
				DrawGLPoly (s->polys);
				glColor3f(1,1,1);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				rs_brushpasses++;
			}

            Video_DisableCapabilities(VIDEO_BLEND);
            Video_SetBlend(VIDEO_BLEND_TWO,VIDEO_DEPTH_TRUE);
		}
	}

	if(entalpha < 1.0f)
	{
        Video_SetBlend(VIDEO_BLEND_IGNORE,VIDEO_DEPTH_TRUE);
        Video_DisableCapabilities(VIDEO_BLEND);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor3f(1.0f,1.0f,1.0f);
	}

fullbrights:
	if (gl_fullbrights.value && t->fullbright)
	{
		Video_SetBlend(VIDEO_BLEND_ONE,VIDEO_DEPTH_FALSE);
		Video_EnableCapabilities(VIDEO_BLEND);

		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glColor3f(entalpha,entalpha,entalpha);

		Video_SetTexture(t->fullbright);

		Fog_StartAdditive ();
		DrawGLPoly (s->polys);
		Fog_StopAdditive ();

		glColor3f(1.0f,1.0f,1.0f);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

        Video_DisableCapabilities(VIDEO_BLEND);
		Video_SetBlend(VIDEO_BLEND_TWO,VIDEO_DEPTH_TRUE);

		rs_brushpasses++;
	}
#endif
}

void Brush_Draw(entity_t *e)
{
	int			k,i;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	model_t		*clmodel;

	if (R_CullModelForEntity(e))
		return;

	currententity = e;
	clmodel = e->model;

	Math_VectorSubtract(r_refdef.vieworg,e->origin,modelorg);
	if(e->angles[0] || e->angles[1] || e->angles[2])
	{
		vec3_t	temp,forward,right,up;

		Math_VectorCopy (modelorg, temp);
		Math_AngleVectors(e->angles, forward, right, up);
		modelorg[0] = Math_DotProduct (temp, forward);
		modelorg[1] = -Math_DotProduct (temp, right);
		modelorg[2] = Math_DotProduct (temp, up);
	}

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];

	// calculate dynamic lighting for bmodel if it's not an
	// instanced model
	if (clmodel->firstmodelsurface != 0 && !gl_flashblend.value)
	{
		float matrix[16];

		glPushMatrix();
		glLoadIdentity();
		glRotatef(-e->angles[2],1,0,0);
		glRotatef(-e->angles[0],0,1,0);
		glRotatef(-e->angles[1],0,0,1);
		glTranslatef(-e->origin[0],-e->origin[1],-e->origin[2]);
		glGetFloatv(GL_MODELVIEW_MATRIX,matrix);
		glPopMatrix();

		for(k = 0; k < MAX_DLIGHTS; k++)
		{
			if(((cl_dlights[k].die < cl.time) && cl_dlights[k].die) || (!cl_dlights[k].radius))
				continue;

			cl_dlights[k].transformed[0] =
				cl_dlights[k].origin[0]*
				matrix[0]+
				cl_dlights[k].origin[1]*
				matrix[4]+
				cl_dlights[k].origin[2]*
				matrix[8]+
				matrix[12];
			cl_dlights[k].transformed[1] =
				cl_dlights[k].origin[0]*
				matrix[1]+
				cl_dlights[k].origin[1]*
				matrix[5]+
				cl_dlights[k].origin[2]*
				matrix[9]+
				matrix[13];
			cl_dlights[k].transformed[2] =
				cl_dlights[k].origin[0]*
				matrix[2]+
				cl_dlights[k].origin[1]*
				matrix[6]+
				cl_dlights[k].origin[2]*
				matrix[10]+
				matrix[14];

			Light_MarkLights(&cl_dlights[k],1<<k,clmodel->nodes+clmodel->hulls[0].firstclipnode);
		}
	}

	glPushMatrix ();
	e->angles[0] = -e->angles[0];	// stupid quake bug
	R_RotateForEntity(e->origin,e->angles);
	e->angles[0] = -e->angles[0];	// stupid quake bug

	//
	// draw it
	//

	if (r_drawflat_cheatsafe) //johnfitz
		Video_DisableCapabilities(VIDEO_TEXTURE_2D);

	for(i = 0; i < clmodel->nummodelsurfaces; i++,psurf++)
	{
		pplane = psurf->plane;
		dot = Math_DotProduct (modelorg, pplane->normal) - pplane->dist;
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			R_DrawSequentialPoly(psurf);
			rs_brushpolys++;
		}
	}

	glPopMatrix();

	if(r_drawflat_cheatsafe)
        Video_EnableCapabilities(VIDEO_TEXTURE_2D);
}

void R_DrawBrushModel_ShowTris (entity_t *e)
{
	int			i;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	model_t		*clmodel;
	glpoly_t	*p;

	if (R_CullModelForEntity(e))
		return;

	currententity = e;
	clmodel = e->model;

	Math_VectorSubtract (r_refdef.vieworg, e->origin, modelorg);
	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		Math_VectorCopy (modelorg, temp);
		Math_AngleVectors(e->angles, forward, right, up);
		modelorg[0] = Math_DotProduct (temp, forward);
		modelorg[1] = -Math_DotProduct (temp, right);
		modelorg[2] = Math_DotProduct (temp, up);
	}

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];

	glPushMatrix ();
	e->angles[0] = -e->angles[0];	// stupid quake bug
	R_RotateForEntity (e->origin, e->angles);
	e->angles[0] = -e->angles[0];	// stupid quake bug

	// draw it
	for (i=0 ; i<clmodel->nummodelsurfaces ; i++, psurf++)
	{
		pplane = psurf->plane;
		dot = Math_DotProduct(modelorg, pplane->normal) - pplane->dist;
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			if ((psurf->flags & SURF_DRAWTURB) && r_oldwater.value)
				for (p = psurf->polys->next; p; p = p->next)
					DrawGLPoly(p);
			else
				DrawGLPoly(psurf->polys);
		}
	}

	glPopMatrix ();
}

/*
=============================================================

	LIGHTMAPS

=============================================================
*/

void R_RenderDynamicLightmaps (msurface_t *fa)
{
	byte		*base;
	int			maps;
	glRect_t    *theRect;
	int smax, tmax;

	if(fa->flags & SURF_DRAWTILED) //johnfitz -- not a lightmapped surface
		return;

	// add to lightmap chain
	fa->polys->chain = lightmap_polys[fa->lightmaptexturenum];
	lightmap_polys[fa->lightmaptexturenum] = fa->polys;

	// check for lightmap modification
	for(maps = 0; maps < BSP_MAX_LIGHTMAPS && fa->styles[maps] != 255; maps++)
		if(d_lightstylevalue[fa->styles[maps]] != fa->cached_light[maps])
			goto dynamic;

	if(fa->dlightframe == r_framecount	// dynamic this frame
		|| fa->cached_dlight)			// dynamic previously
	{
dynamic:
		if (r_dynamic.value)
		{
			bLightmapModified[fa->lightmaptexturenum] = true;
			theRect = &lightmap_rectchange[fa->lightmaptexturenum];
			if (fa->light_t < theRect->t)
			{
				if (theRect->h)
					theRect->h += theRect->t - fa->light_t;
				theRect->t = fa->light_t;
			}
			if (fa->light_s < theRect->l)
			{
				if (theRect->w)
					theRect->w += theRect->l - fa->light_s;
				theRect->l = fa->light_s;
			}
			smax = (fa->extents[0]>>4)+1;
			tmax = (fa->extents[1]>>4)+1;
			if ((theRect->w + theRect->l) < (fa->light_s + smax))
				theRect->w = (fa->light_s-theRect->l)+smax;
			if ((theRect->h + theRect->t) < (fa->light_t + tmax))
				theRect->h = (fa->light_t-theRect->t)+tmax;
			base = lightmaps + fa->lightmaptexturenum*LIGHTMAP_BYTES*BLOCK_WIDTH*BLOCK_HEIGHT;
			base += fa->light_t * BLOCK_WIDTH * LIGHTMAP_BYTES + fa->light_s * LIGHTMAP_BYTES;
			R_BuildLightMap(fa,base,BLOCK_WIDTH*LIGHTMAP_BYTES);
		}
	}
}

int AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_LIGHTMAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (allocated[texnum][i+j] >= best)
					break;
				if (allocated[texnum][i+j] > best2)
					best2 = allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	Con_Warning("AllocBlock: full\n");
	return 0;
}


BSPVertex_t	*r_pcurrentvertbase;
model_t		*currentmodel;

int	nColinElim;

void GL_CreateSurfaceLightmap (msurface_t *surf)
{
	int		smax, tmax;
	byte	*base;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;

	surf->lightmaptexturenum = AllocBlock (smax, tmax, &surf->light_s, &surf->light_t);
	base = lightmaps + surf->lightmaptexturenum*LIGHTMAP_BYTES*BLOCK_WIDTH*BLOCK_HEIGHT;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;
	R_BuildLightMap (surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
}

void BuildSurfaceDisplayList (msurface_t *fa)
{
	int			i,lindex,lnumverts;
	medge_t		*pedges, *r_pedge;
	float		*vec;
	float		s, t;
	glpoly_t	*poly;

	// Reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = fa->numedges;

	// Draw texture
	poly = (glpoly_t*)Hunk_Alloc (sizeof(glpoly_t) + (lnumverts-4) * VERTEXSIZE*sizeof(float));
	poly->next		= fa->polys;
	fa->polys		= poly;
	poly->numverts	= lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = currentmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = r_pcurrentvertbase[r_pedge->v[0]].fPoint;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = r_pcurrentvertbase[r_pedge->v[1]].fPoint;
		}
		s = Math_DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->texture->width;

		t = Math_DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->texture->height;

		Math_VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		// Lightmap texture coordinates
		s = Math_DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s*16;
		s += 8;
		s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = Math_DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t*16;
		t += 8;
		t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}

	poly->numverts = lnumverts;
}

/*	Builds the lightmap texture	with all the surfaces from all brush models.
*/
void GL_BuildLightmaps (void)
{
	char	name[16];
	byte	*data;
	int		i, j;
	model_t	*m;

	memset(allocated,0,sizeof(allocated));
	// [2/10/2013] TODO: Use MAX_LIGHTMAPS for size instead? Quicker and safer? ~hogsy
	memset(lightmap_textures,0,sizeof(lightmap_textures));

	r_framecount = 1; // no dlightcache

	for (j=1 ; j<MAX_MODELS ; j++)
	{
		m = cl.model_precache[j];
		if (!m)
			break;
		if (m->name[0] == '*')
			continue;
		r_pcurrentvertbase = m->vertexes;
		currentmodel = m;
		for (i=0 ; i<m->numsurfaces ; i++)
		{
			//johnfitz -- rewritten to use SURF_DRAWTILED instead of the sky/water flags
			if (m->surfaces[i].flags & SURF_DRAWTILED)
				continue;

			GL_CreateSurfaceLightmap (m->surfaces + i);
			BuildSurfaceDisplayList (m->surfaces + i);
			//johnfitz
		}
	}

	//
	// upload all lightmaps that were filled
	//
	for(i = 0; i < MAX_LIGHTMAPS; i++)
	{
		if (!allocated[i][0])
			break;		// no more used

		bLightmapModified[i] = false;

		lightmap_rectchange[i].l = BLOCK_WIDTH;
		lightmap_rectchange[i].t = BLOCK_HEIGHT;
		lightmap_rectchange[i].w = 0;
		lightmap_rectchange[i].h = 0;

		//johnfitz -- use texture manager
		sprintf(name, "lightmap%03i",i);
		data = lightmaps+i*BLOCK_WIDTH*BLOCK_HEIGHT*LIGHTMAP_BYTES;
		lightmap_textures[i] = TexMgr_LoadImage(
			cl.worldmodel,
			name,
			BLOCK_WIDTH,BLOCK_HEIGHT,
			SRC_LIGHTMAP,
			data,
			"",
			(unsigned)data,
			TEXPREF_LINEAR|TEXPREF_NOPICMIP);
		//johnfitz
	}
}

extern void R_AddDynamicLights(msurface_t *surf);

void R_BuildLightMap (msurface_t *surf, byte *dest, int stride)
{
	int			smax, tmax;
	int			t;
	int			i, j, size;
	byte		*lightmap;
	unsigned	scale;
	int			maps;
	unsigned	*bl;

	surf->cached_dlight = (surf->dlightframe == r_framecount);

	smax		= (surf->extents[0]>>4)+1;
	tmax		= (surf->extents[1]>>4)+1;
	size		= smax*tmax;
	lightmap	= surf->samples;

	if (cl.worldmodel->lightdata)
	{
		// Clear to no light
		memset(&blocklights[0],0,size*3*sizeof(unsigned int)); //johnfitz -- lit support via lordhavoc

		// Add all the lightmaps
		if (lightmap)
			for(maps = 0; maps < BSP_MAX_LIGHTMAPS && surf->styles[maps] != 255; maps++)
			{
				// [16/5/2013] Let us scale the lightmap ~hogsy
				scale = d_lightstylevalue[surf->styles[maps]];
				surf->cached_light[maps] = scale;	// 8.8 fraction
				//johnfitz -- lit support via lordhavoc
				bl = blocklights;
				for(i = 0; i < size; i++)
				{
					*bl++ += *lightmap++*scale;
					*bl++ += *lightmap++*scale;
					*bl++ += *lightmap++*scale;
				}
				//johnfitz
			}

		// Add all the dynamic lights
		if (surf->dlightframe == r_framecount)
			R_AddDynamicLights (surf);
	}
	else
		// Set to full bright if no light data
		memset(&blocklights[0],255,size*3*sizeof(unsigned int)); //johnfitz -- lit support via lordhavoc

	stride -= smax*4;
	bl = blocklights;
	for(i = 0; i < tmax; i++,dest += stride)
	{
		for(j = 0; j < smax; j++,dest += 4)
		{
			if (gl_overbright.value)
			{
				t = *bl++ >> 8;if (t > 255) t = 255;dest[2] = t;
				t = *bl++ >> 8;if (t > 255) t = 255;dest[1] = t;
				t = *bl++ >> 8;if (t > 255) t = 255;dest[0] = t;
			}
			else
			{
				t = *bl++ >> 7;if (t > 255) t = 255;dest[2] = t;
				t = *bl++ >> 7;if (t > 255) t = 255;dest[1] = t;
				t = *bl++ >> 7;if (t > 255) t = 255;dest[0] = t;
			}

			dest[3] = 255;
		}
	}
}

// Assumes lightmap texture is already bound
void R_UploadLightmap(int lmap)
{
	glRect_t	*theRect;

	if(!bLightmapModified[lmap])
		return;

	bLightmapModified[lmap] = false;

	theRect = &lightmap_rectchange[lmap];
	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,0,
		theRect->t,
		BLOCK_WIDTH,
		theRect->h,
		// [16/6/2013] MH's suggestions ~hogsy
		GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV,
		lightmaps+(lmap* BLOCK_HEIGHT + theRect->t)*BLOCK_WIDTH*LIGHTMAP_BYTES);
	theRect->l = BLOCK_WIDTH;
	theRect->t = BLOCK_HEIGHT;
	theRect->h = 0;
	theRect->w = 0;

	rs_dynamiclightmaps++;
}

void R_RebuildAllLightmaps (void)
{
	int			i, j;
	model_t		*mod;
	msurface_t	*fa;
	byte		*base;

	if(!cl.worldmodel) // is this the correct test?
		return;

	// For each surface in each model, rebuild lightmap with new scale
	for(i = 1; i < MAX_MODELS; i++)
	{
		if (!(mod = cl.model_precache[i]))
			continue;

		fa = &mod->surfaces[mod->firstmodelsurface];
		for(j = 0; j < mod->nummodelsurfaces; j++,fa++)
		{
			if (fa->flags & SURF_DRAWTILED)
				continue;
			base = lightmaps + fa->lightmaptexturenum*LIGHTMAP_BYTES*BLOCK_WIDTH*BLOCK_HEIGHT;
			base += fa->light_t * BLOCK_WIDTH*LIGHTMAP_BYTES+fa->light_s*LIGHTMAP_BYTES;
			R_BuildLightMap (fa, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
		}
	}

	// For each lightmap, upload it
	for(i = 0; i < MAX_LIGHTMAPS; i++)
	{
		if(!allocated[i][0])
			break;

		Video_SetTexture(lightmap_textures[i]);

		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,0,0,
			BLOCK_WIDTH,BLOCK_HEIGHT,
			// [16/6/2013] MH's suggestions ~hogsy
			GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV,
			lightmaps+i*BLOCK_WIDTH*BLOCK_HEIGHT*LIGHTMAP_BYTES);
	}
}
