/*	Copyright (C) 2011-2013 OldTimes Software
*/
#include "client_main.h"

/*
	Only functions that are called by the engine should
	exist in here.
*/

/*	Initialize any client-side specific variables.
*/
void Client_Initialize(void)
{
}

/*	Crappy the sticky renderer loop!
	This lets us draw images and text to the screen... Forcefully.
*/
void Client_Draw(void)
{
}

/*	Parse temporary entity types, useful for particles and other small crap.
*/
void Client_ParseTemporaryEntity(void)
{
	int	iType;

	// Set iType, so if the type is missing we can mention it below.
	iType = Engine.ReadByte();

	switch(iType)
	{
	case CTE_EXPLOSION:
		{
			int			i,j;
			vec3_t		vPosition;
			Particle_t	*pExplosion;

			for(i = 0; i < 3; i++)
				vPosition[i] = Engine.ReadCoord();

			for(i = 0; i < 512; i++)
			{
				pExplosion = Engine.Client_AllocateParticle();
				if(!pExplosion)
					return;

				pExplosion->texture		= Engine.Client_GetEffect(va("smoke%i",rand()%4));
				pExplosion->scale		= 30.0f;
				pExplosion->die			= (float)(Client.time+5.0);
				pExplosion->ramp		= (float)(rand()&3);
				pExplosion->pBehaviour	= PARTICLE_EXPLODE2;

				for(j = 0; j < 3; j++)
				{
					pExplosion->org[j]	= vPosition[j]+((rand()&32)-16);
					pExplosion->vel[j]	= (float)((rand()%512)-256);
				}
			}

			{
				DynamicLight_t *dExplosionLight = Engine.Client_AllocateDlight(0);

				Math_VectorCopy(vPosition,dExplosionLight->origin);

				dExplosionLight->bLightmap		= true;
				dExplosionLight->radius			= 300.0f;
				dExplosionLight->color[RED]		= 255.0f;
				dExplosionLight->color[GREEN]	= 255.0f;
				dExplosionLight->color[BLUE]	= 50.0f;
				dExplosionLight->minlight		= 32.0f;
				dExplosionLight->die			= (float)(Client.time+0.5);
			}
		}
		break;
	case CTE_BLOODSPRAY:
		{
			int			i,j;
			char		cBlood[12];
			vec3_t		vPosition;
			Particle_t	*pBloodSpray;

			for(i = 0; i < 3; i++)
				vPosition[i] = Engine.ReadCoord();

			for(i = 0; i < 24; i++)
			{
				pBloodSpray = Engine.Client_AllocateParticle();
				if(!pBloodSpray)
					return;

				// Keep the textures random.
				PARTICLE_BLOOD(cBlood);

				Math_VectorSet(1.0f,pBloodSpray->color);
				Math_VectorCopy(vPosition,pBloodSpray->org);

				pBloodSpray->die		= (float)(Client.time+5.0);
				pBloodSpray->pBehaviour	= PARTICLE_BEHAVIOUR_SLOWGRAVITY;
				pBloodSpray->ramp		= (float)(rand()&3);
				pBloodSpray->scale		= (float)(rand()%8+1);
				pBloodSpray->texture	= Engine.Client_GetEffect(cBlood);
				
				for(j = 0; j < 3; j++)
					pBloodSpray->vel[j] = (float)((rand()%512)-256);
			}
		}
		break;
	case CTE_PARTICLE_FIELD:
		break;
	default:
		Engine.Con_Warning("Unknown temporary entity type (%i)!\n",iType);
		// [19/6/2013] TODO: Need to disconnect here! ~hogsy
	}
}

/*	Called by the engine.
*/
void Client_RelinkEntities(entity_t *eClient,int i,double dTime)
{
	vec3_t			f,r,u;
	DynamicLight_t	*dLight;

	// [6/5/2012] Keep client time updated (temp) ~hogsy
	// [5/6/2012] TODO: Move over to somewhere more appropriate please ~hogsy
	Client.time = dTime;

	if(eClient->effects & EF_MOTION_FLOAT)
		eClient->origin[2] += ((float)sin(Client.time*2.0f))*5.0f;

	if(eClient->effects & EF_MOTION_ROTATE)
		eClient->angles[YAW] = Math_AngleMod((float)(100.0*Client.time));

	if(eClient->effects & EF_PARTICLE_SMOKE)
	{
		int k,j;

		for(k = 0; k < 2; k++)
		{
			Particle_t	*pParticle;
			
			pParticle = Engine.Client_AllocateParticle();
			if(!pParticle)
				return;

			pParticle->texture		= Engine.Client_GetEffect(va("smoke%i",rand()%4));
			pParticle->ramp			= (float)(rand()&3);
			pParticle->scale		= ((float)(rand()%20)*2);
			pParticle->die			= (float)(Client.time+(rand()%5));
			pParticle->pBehaviour	= PARTICLE_SMOKE;
			
			// [5/9/2012] TODO: Simplify this ~hogsy
			for(j = 0; j < 3; j++)
				pParticle->org[j] = eClient->origin[j]+((rand()&15)-5.0f);

			Math_VectorClear(pParticle->vel);
		}
	}

	// [23/11/2013] Simple bloody particle effect, which is now done client-side! Yay! ~hogsy
	if(eClient->effects & EF_PARTICLE_BLOOD)
	{
		char	cBlood[12];
		int		k,j;

		for(k = 0; k < 2; k++)
		{
			Particle_t *pParticle = Engine.Client_AllocateParticle();
			if(!pParticle)
				return;

			// Keep the textures random.
			PARTICLE_BLOOD(cBlood);

			pParticle->texture		= Engine.Client_GetEffect(cBlood);
			pParticle->ramp			= (float)(rand()&4);
			pParticle->scale		= (float)(rand()%2+2);
			pParticle->die			= (float)(Client.time+0.3*(rand()%5));
			pParticle->pBehaviour	= PARTICLE_BEHAVIOUR_GRAVITY;
			
			for(j = 0; j < 3; j++)
				pParticle->org[j] = eClient->origin[j]+((rand()&15)-5.0f);

			pParticle->vel[2] = 15.0f;
			//Math_VectorScale(eClient->angles,15.0f,pParticle->vel);
		}
	}

	if(eClient->effects & EF_LIGHT_GREEN)
	{
		int k,j;

		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);

		dLight->radius			= (float)(rand()%20)*10;
		dLight->color[RED]		= 0;
		dLight->color[GREEN]	= 255.0f;
		dLight->color[BLUE]		= 0;
		dLight->minlight		= 16.0f;
		dLight->die				= (float)(Client.time+0.01);
		dLight->bLightmap		= true;

		for(k = 0; k < 128; k++)
		{
			Particle_t *pParticle = Engine.Client_AllocateParticle();
			if(!pParticle)
				return;

			pParticle->texture		= Engine.Client_GetEffect("spark2");
			pParticle->ramp			= (float)(rand()&3);
			pParticle->scale		= 0.5f+(float)(rand()%15/10);
			pParticle->die			= (float)(Client.time+(double)(rand()%2));
			pParticle->pBehaviour	= PARTICLE_BEHAVIOUR_SLOWGRAVITY;
			
			// [5/9/2012] TODO: Simplify this ~hogsy
			for(j = 0; j < 3; j++)
				pParticle->org[j] = eClient->origin[j]+((rand()&15)-5.0f);

			Math_VectorClear(pParticle->vel);
		}
	}

	if(eClient->effects & EF_LIGHT_BLUE)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);

		dLight->radius			= 120.0f;
		dLight->color[RED]		= 0;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 255.0f;
		dLight->minlight		= 32.0f;
		dLight->die				= (float)(Client.time+0.01);
		dLight->bLightmap		= true;
	}

	if(eClient->effects & EF_LIGHT_RED)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);

		dLight->radius			= 120.0f;
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 0;
		dLight->minlight		= 32.0f;
		dLight->die				= (float)(Client.time+0.01);
		dLight->bLightmap		= true;
	}

	if(eClient->effects & EF_MUZZLEFLASH)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);

		dLight->origin[2] += 16.0f;

		Math_AngleVectors(eClient->angles,f,r,u);
		Math_VectorMA(dLight->origin,18,f,dLight->origin);
		
		dLight->radius			= 170.0f+(rand()&31);
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 255.0f;
		dLight->color[BLUE]		= 50.0f;
		dLight->minlight		= 32.0f;
		dLight->die				= ((float)Client.time)+0.1f;
		dLight->bLightmap		= true;
	}

	if(eClient->effects & EF_DIMLIGHT)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);

		dLight->radius			= 200.0f;
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 255.0f;
		dLight->color[BLUE]		= 255.0f;
		dLight->minlight		= 32.0f;
		dLight->die				= (float)(Client.time+0.01);
		dLight->bLightmap		= true;
	}

	if(eClient->effects & EF_BRIGHTLIGHT)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);
		// [22/4/2013] Simplified ~hogsy
		Math_VectorSet(255.0f,dLight->color);

		dLight->origin[2]		+= 16.0f;
		dLight->radius			= 300.0f+(rand()&31);
		dLight->minlight		= 32.0f;
		dLight->die				= ((float)Client.time)+0.001f;
		dLight->bLightmap		= true;
	}

	if(eClient->effects & EF_GLOW_RED)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);

		dLight->radius			= (float)sin(Client.time*2.0f)*100.0f;
		dLight->color[RED]		= 255.0f;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 0;
		dLight->minlight		= 16.0f;
		dLight->die				= ((float)Client.time)+0.01f;
		dLight->bLightmap		= true;
	}

/*	if(ent->effects & EF_GLOW_GREEN)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(ent->origin,dLight->origin);

		dLight->radius			= (float)sin(Client.time*2.0f)*100.0f;
		dLight->color[RED]		= 0;
		dLight->color[GREEN]	= 255.0f;
		dLight->color[BLUE]		= 0;
		dLight->minlight		= 16.0f;
		dLight->die				= Client.time+0.01f;
		dLight->bLightmap		= true;
	}*/

	if(eClient->effects & EF_GLOW_BLUE)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);

		dLight->radius			= (float)sin(Client.time*2.0f)*100.0f;
		dLight->color[RED]		= 0;
		dLight->color[GREEN]	= 0;
		dLight->color[BLUE]		= 255.0f;
		dLight->minlight		= 16.0f;
		dLight->die				= (float)(Client.time+0.01);
		dLight->bLightmap		= true;
	}

	if(eClient->effects & EF_GLOW_WHITE)
	{
		dLight = Engine.Client_AllocateDlight(i);

		Math_VectorCopy(eClient->origin,dLight->origin);
		// [22/4/2013] Simplified ~hogsy
		Math_VectorSet(255.0f,dLight->color);

		dLight->radius		= (float)sin(Client.time*2.0f)*100.0f;
		dLight->minlight	= 16.0f;
		dLight->die			= (float)(Client.time+0.01);
		dLight->bLightmap	= true;
	}

	eClient->bForceLink = false;
}