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
#include "engine_video.h"

int	r_dlightframecount;

extern cvar_t r_flatlightstyles; //johnfitz

void Light_Animate(void)
{
	int	i,j,k;

	// Light animations
	// 'm' is normal light, 'a' is no light, 'z' is double bright
	i = (int)(cl.time*10);

	for(j = 0; j < MAX_LIGHTSTYLES; j++)
	{
		if(!cl_lightstyle[j].length)
		{
			d_lightstylevalue[j] = 256;
			continue;
		}

		//johnfitz -- r_flatlightstyles
		if(r_flatlightstyles.value == 2)
			k = cl_lightstyle[j].peak-'a';
		else if(r_flatlightstyles.value == 1)
			k = cl_lightstyle[j].average-'a';
		else
		{
			k = i % cl_lightstyle[j].length;
			k = cl_lightstyle[j].cMap[k]-'a';
		}

		d_lightstylevalue[j] = k*22;
		//johnfitz
	}
}

void Light_Draw(void)
{
	int				i;
	DynamicLight_t	*dLight;

	if(!gl_flashblend.value)
		return;

	// Because the count hasn't advanced yet for this frame.
	r_dlightframecount = r_framecount+1;

	glDepthMask(false);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);

	dLight = cl_dlights;
	for(i = 0; i < MAX_DLIGHTS; i++,dLight++)
	{
		// [5/5/2012] Ugh some lights are inverted... ~hogsy
		if(((dLight->die < cl.time) && dLight->die) || !dLight->radius)
			continue;

		{
			int		i,j;
			float	a,a2,b,rad;
			vec3_t	v;

			Math_VectorSubtract(dLight->origin,r_origin,v);

			rad = dLight->radius*0.35f;
			if(Math_Length(v) < rad)
			{
				// view is inside the dlight
				a2 = dLight->radius*0.0003f;
				vViewBlend[3] = b = vViewBlend[3]+a2*(1-vViewBlend[3]);
				a2 = a2/b;

				// [7/5/2012] Make sure it shows the lights colour ~hogsy
				vViewBlend[0] = vViewBlend[1]*(1-a2)+((1.0f/255)*dLight->color[RED])*a2;
				vViewBlend[1] = vViewBlend[1]*(1-a2)+((1.0f/255)*dLight->color[GREEN])*a2;
				vViewBlend[2] = vViewBlend[2]*(1-a2)+((1.0f/255)*dLight->color[BLUE])*a2;
				return;
			}

			glBegin(GL_TRIANGLE_FAN);
			// [7/5/2012] Make sure it shows the lights colour ~hogsy
			glColor3ub(dLight->color[RED],dLight->color[GREEN],dLight->color[BLUE]);

			for(i = 0; i < 3; i++)
				v[i] = dLight->origin[i]-vpn[i]*rad;

			glVertex3fv(v);
			glColor3f(0,0,0);

			for(i = 16; i >= 0; i--)
			{
				a = i/16.0f*M_PI*2.0f;

				for(j = 0; j < 3; j++)
					v[j] = dLight->origin[j]+vright[j]*cos(a)*rad+vup[j]*sin(a)*rad;

				glVertex3fv(v);
			}

			glEnd();
		}
	}

	glColor3f(1.0f,1.0f,1.0f);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(true);
}

void Light_MarkLights(DynamicLight_t *light,int bit,mnode_t *node)
{
	mplane_t	*splitplane;
	msurface_t	*surf;
	vec3_t		impact;
	float		dist, l, maxdist;
	int			i, j, s, t;

START:
	if (node->contents < 0)
		return;

	splitplane = node->plane;
	if (splitplane->type < 3)
		dist = light->transformed[splitplane->type] - splitplane->dist;
	else
		dist = Math_DotProduct(light->transformed, splitplane->normal) - splitplane->dist;

	if(dist > light->radius)
	{
		node = node->children[0];
		goto START;
	}
	else if(dist < -light->radius)
	{
		node = node->children[1];
		goto START;
	}

	maxdist = light->radius*light->radius;

	// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		for (j=0 ; j<3 ; j++)
			impact[j] = light->transformed[j] - surf->plane->normal[j]*dist;

		// clamp center of light to corner and check brightness
		l = Math_DotProduct(impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l+0.5;if (s < 0) s = 0;else if (s > surf->extents[0]) s = surf->extents[0];
		s = l - s;
		l = Math_DotProduct(impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l+0.5;if (t < 0) t = 0;else if (t > surf->extents[1]) t = surf->extents[1];
		t = l - t;

		// Compare to minimum light.
		if ((s*s+t*t+dist*dist) < maxdist)
		{
			if (surf->dlightframe != r_dlightframecount) // not dynamic until now
			{
				surf->dlightbits = bit;
				surf->dlightframe = r_dlightframecount;
			}
			else // already dynamic
				surf->dlightbits |= bit;
		}
	}

	if(node->children[0]->contents >= 0)
		Light_MarkLights(light,bit,node->children[0]);
	if(node->children[1]->contents >= 0)
		Light_MarkLights(light,bit,node->children[1]);
}

void R_PushDlights(void)
{
	int				i;
	DynamicLight_t	*dLight;

	if (gl_flashblend.value)
		return;

	// Because the count hasn't advanced yet for this frame.
	r_dlightframecount = r_framecount + 1;

	dLight = cl_dlights;
	for(i = 0; i < MAX_DLIGHTS; i++,dLight++)
	{
		if(((dLight->die < cl.time) && dLight->die) || !dLight->radius)
			continue;

		Math_VectorCopy(dLight->origin,dLight->transformed);

		Light_MarkLights(dLight,1 << i,cl.worldmodel->nodes);
	}
}

/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

msurface_t	*lightsurface;
mplane_t	*lightplane;
vec3_t		lightspot;

bool RecursiveLightPoint(vec3_t color,mnode_t *node,vec3_t start,vec3_t end)
{
	float	front,back,
			frac;
	vec3_t	mid;

loc0:
	if (node->contents < 0)
		return false;		// didn't hit anything

// calculate mid point
	if (node->plane->type < 3)
	{
		front = start[node->plane->type] - node->plane->dist;
		back = end[node->plane->type] - node->plane->dist;
	}
	else
	{
		front	= Math_DotProduct(start, node->plane->normal) - node->plane->dist;
		back	= Math_DotProduct(end, node->plane->normal) - node->plane->dist;
	}

	// LordHavoc: optimized recursion
	if((back < 0) == (front < 0))
	{
		node = node->children[front < 0];
		goto loc0;
	}

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

	// Go down front side
	if(RecursiveLightPoint (color, node->children[front < 0], start, mid))
		return true;	// hit something
	else
	{
		int i, ds, dt;
		msurface_t *surf;

		// Check for impact on this node
		Math_VectorCopy(mid,lightspot);
		lightplane = node->plane;

		surf = cl.worldmodel->surfaces + node->firstsurface;
		for (i = 0;i < node->numsurfaces;i++, surf++)
		{
			if(surf->flags & SURF_DRAWTILED)
				continue;	// no lightmaps

			ds = (int)((float)Math_DotProduct(mid,surf->texinfo->vecs[0])+surf->texinfo->vecs[0][3]);
			dt = (int)((float)Math_DotProduct(mid,surf->texinfo->vecs[1])+surf->texinfo->vecs[1][3]);

			if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
				continue;

			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];

			if (ds > surf->extents[0] || dt > surf->extents[1])
				continue;

			if (surf->samples)
			{
				// LordHavoc: enhanced to interpolate lighting
				byte *lightmap;
				int maps,line3,dsfrac = ds & 15,dtfrac = dt & 15,
					r00 = 0,g00 = 0,b00 = 0,
					r01 = 0,g01 = 0,b01 = 0,
					r10 = 0,g10 = 0,b10 = 0,
					r11 = 0,g11 = 0,b11 = 0,
					smax = (surf->extents[0] >> 4)+1,
					tmax = (surf->extents[1] >> 4)+1;
				float scale;
				line3 = smax*3;
				lightmap = surf->samples+((dt>>4)*smax+(ds>>4))*3; // LordHavoc: *3 for color

				for (maps = 0;maps < MAXLIGHTMAPS && surf->styles[maps] != 255;maps++)
				{
					scale = (float) d_lightstylevalue[surf->styles[maps]] * 1.0 / 256.0;
					r00 += (float)lightmap[0]*scale;
					g00 += (float)lightmap[1]*scale;
					b00 += (float)lightmap[2]*scale;
					r01 += (float)lightmap[3]*scale;
					g01 += (float)lightmap[4]*scale;
					b01 += (float)lightmap[5]*scale;
					r10 += (float)lightmap[line3+0] * scale;g10 += (float) lightmap[line3+1] * scale;b10 += (float) lightmap[line3+2] * scale;
					r11 += (float)lightmap[line3+3] * scale;g11 += (float) lightmap[line3+4] * scale;b11 += (float) lightmap[line3+5] * scale;

					lightmap += smax*tmax*3; // LordHavoc: *3 for colored lighting
				}

				color[0] += (float)((int)((((((((r11-r10)*dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)));
				color[1] += (float)((int)((((((((g11-g10)*dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)));
				color[2] += (float)((int)((((((((b11-b10)*dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)));
			}
			return true; // success
		}

		// Go down back side
		return RecursiveLightPoint (color, node->children[front >= 0], mid, end);
	}
}

/*	Returns a colour sample from the lightmap.
*/
MathVector_t Light_GetSample(vec3_t vPoint)
{
	vec3_t			vEnd,vLightColour;
	MathVector_t	mvLightColour;

	if(!cl.worldmodel->lightdata)
		Math_VectorSet(255.0f,vLightColour);

	vEnd[0] = vPoint[0];
	vEnd[1] = vPoint[1];
	vEnd[2] = vPoint[2]-2048.0f;

	// [1/7/2012] Simplified ~hogsy
	Math_VectorClear(vLightColour);

	RecursiveLightPoint(vLightColour,cl.worldmodel->nodes,vPoint,vEnd);

	Math_VectorToMV(vLightColour,mvLightColour);

	return mvLightColour;
}

/*	Updated alternative to the above.
	Gets the closest static/dynamic light source, but also checks the lightmap
	sample. If the lightmap sample comes back as dark, then we assume
	there shouldn't be an actual light source.
*/
DynamicLight_t *Light_GetDynamic(vec3_t vPoint)
{
	int				i;
	bool			bStaticLights = true;
	float			fLightAmount;
	DynamicLight_t	dlClosestLight = cl_dlights[0];
	vec3_t			vLightColour;

	if(!cl.worldmodel->lightdata)
		Math_VectorSet(255.0f,vLightColour);
	else
		Math_MVToVector(Light_GetSample(vPoint),vLightColour);

	// [30/10/2013] Check that we're actually being effected by a lightsource before anything else ~hogsy
	fLightAmount = (vLightColour[0]+vLightColour[1]+vLightColour[2])/100.0f;
	if(!fLightAmount)
		bStaticLights = false;

	for(i = 0; i < MAX_DLIGHTS; i++)
	{
		// [24/2/2014] If we're not being effected by the lightmap then ignore static light sources ~hogsy
		if(!bStaticLights && ((cl_dlights[i].bLightmap == false) && cl_dlights[i].radius))
			continue;

		if(cl_dlights[i].die >= cl.time || (!cl_dlights[i].bLightmap && cl_dlights[i].radius))
		{
			float	fDistance[1];
			vec3_t	vDistance;

			Math_VectorSubtract(vPoint,cl_dlights[i].origin,vDistance);

			fDistance[0] = cl_dlights[i].radius-Math_Length(vDistance);
			if(fDistance[0] > 0)
			{
				Math_VectorSubtract(dlClosestLight.origin,cl_dlights[i].origin,vDistance);

				fDistance[1] = cl_dlights[i].radius-Math_Length(vDistance);
				if(fDistance[1] < fDistance[0])
					dlClosestLight = cl_dlights[i];
			}
		}
	}

	return NULL;
}

extern unsigned	blocklights[128*128*3];

void R_AddDynamicLights(msurface_t *surf)
{
	int			lnum,sd,td,s,t,i,smax,tmax;
	float		dist,rad,minlight,cred,cgreen,cblue,brightness;
	//johnfitz -- lit support via lordhavoc
	unsigned	*bl;
	//johnfitz
	vec3_t		impact, local;
	mtexinfo_t	*tex;

	smax	= (surf->extents[0]>>4)+1;
	tmax	= (surf->extents[1]>>4)+1;
	tex		= surf->texinfo;

	for(lnum = 0; lnum < MAX_DLIGHTS; lnum++)
	{
		if(!(surf->dlightbits & (1<<lnum)) || !cl_dlights[lnum].bLightmap)
			continue;		// not lit by this light

		rad = cl_dlights[lnum].radius;
		dist = Math_DotProduct(cl_dlights[lnum].transformed, surf->plane->normal) - surf->plane->dist;

		rad -= fabs(dist);
		minlight = cl_dlights[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad-minlight;

		for (i=0 ; i<3 ; i++)
			impact[i] = cl_dlights[lnum].transformed[i]-surf->plane->normal[i]*dist;

		local[0] = Math_DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = Math_DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		bl = blocklights;

		cred	= cl_dlights[lnum].color[RED];
		cgreen	= cl_dlights[lnum].color[GREEN];
		cblue	= cl_dlights[lnum].color[BLUE];

		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
				{
					brightness = minlight-dist;
					bl[0] += (unsigned int)(brightness*cred);
					bl[1] += (unsigned int)(brightness*cgreen);
					bl[2] += (unsigned int)(brightness*cblue);
				}
				bl += 3;
			}
		}
	}
}
