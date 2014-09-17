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

/*
	Client-side Temporary Entities
*/

#include "KatGL.h"

int			num_temp_entities;
entity_t	cl_temp_entities[MAX_TEMP_ENTITIES];
beam_t		cl_beams[MAX_BEAMS];

sfx_t		*sExplosionSound;

void CL_InitTEnts(void)
{
	sExplosionSound = S_PrecacheSound("weapons/r_exp3.wav");
}

void CL_ParseTEnt (void)
{
	int				type,i,j,k,iParticleTexture;
	float			fVelocity;
	vec3_t			pos,vDirection;
	Particle_t		*pParticle;

	type = MSG_ReadByte ();
	switch (type)
	{
	case TE_EXPLOSION:			// rocket explosion
		for(i = 0; i < 3; i++)
			pos[i] = MSG_ReadCoord();

		iParticleTexture = Client_GetEffect(DIR_PARTICLES"smoke0");

		for(i = 0; i < 1024; i++)
		{
			pParticle = Client_AllocateParticle();
			if(!pParticle)
				return;

			pParticle->texture		= iParticleTexture;
			pParticle->scale		= 30.0f;
			pParticle->die			= cl.time+10.0f;
			pParticle->ramp			= rand()&3;
			pParticle->pBehaviour	= PARTICLE_SMOKE;

			for(j = 0; j < 3; j++)
			{
				pParticle->org[j]	= pos[j]+((rand()&32)-16);
				pParticle->vel[j]	= (rand()%128)-256;
			}
		}

		{
			DynamicLight_t *dlExplodeLight = Client_AllocDlight(0);

			Math_VectorCopy(pos,dlExplodeLight->origin);

			dlExplodeLight->radius		= 300.0f;
			dlExplodeLight->color[0]	= 255.0f;
			dlExplodeLight->color[1]	= 255.0f;
			dlExplodeLight->color[2]	= 50.0f;
			dlExplodeLight->minlight	= 32.0f;
			dlExplodeLight->die			= cl.time+0.5f;
			dlExplodeLight->bLightmap	= true;
		}

		S_StartSound(-1,0,sExplosionSound,pos,255,1);
		break;
	case TE_LAVASPLASH:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		// [6/9/2012] TODO: Find a more appropriate texture, or scrap completely? ~hogsy
		iParticleTexture = Client_GetEffect(DIR_PARTICLES"smoke0");

		for(i = -16; i < 16; i++)
			for(j = -16; j < 16; j++)
				for(k = 0; k < 1; k++)
				{
					pParticle = Client_AllocateParticle();
					if(!pParticle)
						return;

					vDirection[0]	= j*8.0f+(rand()&7);
					vDirection[1]	= i*8.0f+(rand()&7);
					vDirection[2]	= 256;

					pParticle->texture		= iParticleTexture;
					pParticle->scale		= 7.0f;
					pParticle->die			= cl.time+2.0f+(rand()&31)*0.02f;
					pParticle->pBehaviour	= PARTICLE_BEHAVIOUR_SLOWGRAVITY;
					pParticle->org[0]		= pos[0]+vDirection[0];
					pParticle->org[1]		= pos[1]+vDirection[1];
					pParticle->org[2]		= pos[2]+(rand()&63);

					fVelocity	= 50+(rand()&63);

					Math_VectorNormalize(vDirection);
					Math_VectorScale(vDirection,fVelocity,pParticle->vel);
				}
		break;
	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		// [6/9/2012] TODO: Find a more appropriate texture, or scrap completely? ~hogsy
		iParticleTexture = Client_GetEffect(DIR_PARTICLES"smoke0");

		for(i = -16; i < 16; i += 4)
			for(j = -16; j < 16; j += 4)
				for(k = -24; k < 32; k += 4)
				{
					pParticle = Client_AllocateParticle();
					if(!pParticle)
						return;

					vDirection[0]	= j*8.0f;
					vDirection[1]	= i*8.0f;
					vDirection[2]	= k*8.0f;

					pParticle->texture		= iParticleTexture;
					pParticle->scale		= 7.0f;
					pParticle->die			= cl.time+2.0f+(rand()&7)*0.02f;
					pParticle->pBehaviour	= PARTICLE_BEHAVIOUR_SLOWGRAVITY;
					pParticle->org[0]		= pos[0]+i+(rand()&3);
					pParticle->org[1]		= pos[1]+j+(rand()&3);
					pParticle->org[2]		= pos[2]+k+(rand()&3);

					fVelocity	= 50+(rand()&63);

					Math_VectorNormalize(vDirection);
					Math_VectorScale(vDirection,fVelocity,pParticle->vel);
				}
		break;
	default:
		Con_Error("CL_ParseTEnt: Unknown temporary entity type!\n");
	}
}

entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

	if(cl_numvisedicts == MAX_VISEDICTS ||	num_temp_entities == MAX_TEMP_ENTITIES)
		return NULL;
	ent = &cl_temp_entities[num_temp_entities];
	memset (ent, 0, sizeof(*ent));
	num_temp_entities++;
	cl_visedicts[cl_numvisedicts] = ent;
	cl_numvisedicts++;

	return ent;
}

void CL_UpdateTEnts (void)
{
	int			i, j; //johnfitz -- use j instead of using i twice, so we don't corrupt memory
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	entity_t	*ent;
	float		yaw, pitch;
	float		forward;

	num_temp_entities = 0;

	srand((int)(cl.time*1000)); //johnfitz -- freeze beams when paused

	// update lightning
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
			continue;

		// if coming from the player, update the start position
		if (b->entity == cl.viewentity)
			Math_VectorCopy (cl_entities[cl.viewentity].origin, b->start);

		// calculate pitch and yaw
		Math_VectorSubtract(b->end,b->start,dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(dist[1], dist[0]) * 180 / pMath_PI);
			if (yaw < 0)
				yaw += 360;

			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int) (atan2(dist[2], forward) * 180 / pMath_PI);
			if (pitch < 0)
				pitch += 360;
		}

		// Add new entities for the lightning
		Math_VectorCopy (b->start, org);
		d = Math_VectorNormalize(dist);
		while (d > 0)
		{
			ent = CL_NewTempEntity ();
			if (!ent)
				return;

			Math_VectorCopy (org, ent->origin);
			ent->model		= b->model;
			ent->angles[0]	= pitch;
			ent->angles[1]	= yaw;
			ent->angles[2]	= rand()%360;

			//johnfitz -- use j instead of using i twice, so we don't corrupt memory
			for (j=0 ; j<3 ; j++)
				org[j] += dist[j]*30;
			d -= 30;
		}
	}
}
