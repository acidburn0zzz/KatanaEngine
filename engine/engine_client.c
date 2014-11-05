/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_client.h"

/*
	Client-side stuff!
*/

#include "engine_console.h"
#include "engine_videoshadow.h"
#include "engine_video.h"	// [7/8/2013] Included for cvLitParticles ~hogsy

#include "shared_server.h"
#include "shared_menu.h"

/*
	Utilities
*/

entity_t *Client_GetPlayerEntity(void)
{
	return &cl_entities[cl.viewentity];
}

entity_t *Client_GetViewEntity(void)
{
	return &cl.viewent;
}

/*
	Particles
	TODO:
		Move into its own document "engine_particles" :)
*/

Particle_t	*active_particles,*pFreeParticles,*particles;

void Client_ParseParticleEffect(void)
{
	vec3_t	vOrigin,vDirection;
	int		i,iCount,iTexture;
	float	fScale;

	for(i = 0; i < 3; i++)
		vOrigin[i] = MSG_ReadCoord();

	for(i = 0; i < 3; i++)
		vDirection[i] = MSG_ReadChar()*(1.0f/16.0f);

	fScale		= MSG_ReadFloat();
	iTexture	= MSG_ReadByte();
	iCount		= MSG_ReadByte();

	Client_CreateParticleEffect(
		PARTICLE_TYPE_DEFAULT,
		PARTICLE_BEHAVIOUR_SLOWGRAVITY,
		vOrigin,
		vDirection,
		0.5f,
		fScale,
		iCount,
		iTexture);
}

Particle_t *Client_AllocateParticle(void)
{
	Particle_t *pParticle;

	if(cl.bIsPaused || ((key_dest == key_console) && svs.maxclients == 1))
		return NULL;

	if(!pFreeParticles)
	{
		Con_Warning("Failed to allocate particle!\n");
		return NULL;
	}

	pParticle			= pFreeParticles;
	pFreeParticles		= pParticle->next;
	pParticle->next		= active_particles;
	active_particles	= pParticle;

	return pParticle;
}

void Client_CreateParticleEffect(
	ParticleType_t		pType,
	ParticleBehaviour_t	pBehaviour,
	vec3_t				vOrigin,
	vec3_t				vDirection,
	float				fVelocity,
	float				fScale,
	int					iCount,
	int					iTextureID)
{
	Particle_t	*pParticle;
	int			i,j,k;

	switch(pType)
	{
	case PARTICLE_TYPE_LAVASPLASH:
		for(i = -16; i < 16; i++)
			for(j = -16; j < 16; j++)
				for(k = 0; k < 1; k++)
				{
					pParticle = Client_AllocateParticle();
					if(!pParticle)
						return;

					vDirection[0]	= ((float)j)*8.0f+(float)(rand()&7);
					vDirection[1]	= ((float)i)*8.0f+(float)(rand()&7);
					vDirection[2]	= 256.0f;

					// [5/9/2012] Keep this pre-defined? ~hogsy
					fVelocity = 50.0f+(float)(rand()&63);

					pParticle->die			= cl.time+2.0f+(rand()&31)*0.02f;
					pParticle->pBehaviour	= PARTICLE_BEHAVIOUR_SLOWGRAVITY;
					pParticle->texture		= iTextureID;

					Math_VectorAdd(vOrigin,vDirection,pParticle->org);
					Math_VectorNormalize(vDirection);
					Math_VectorScale(vDirection,fVelocity,pParticle->vel);
				}
		break;
	case PARTICLE_TYPE_TELEPORTSPLASH:
		for(i = -16; i < 16; i += 4)
			for(j = -16; j < 16; j += 4)
				for(k = -24; k < 32; k += 4)
				{
					pParticle = Client_AllocateParticle();
					if(!pParticle)
						return;

					vDirection[0]	= j*8;
					vDirection[1]	= i*8;
					vDirection[2]	= k*8;

					fVelocity = 50+(rand()&63);

					pParticle->die			= cl.time+0.2f+(rand()&7)*0.02f;
					pParticle->pBehaviour	= PARTICLE_BEHAVIOUR_SLOWGRAVITY;
					pParticle->org[0]		= vOrigin[0]+i+(rand()&3);
					pParticle->org[1]		= vOrigin[1]+j+(rand()&3);
					pParticle->org[2]		= vOrigin[2]+k+(rand()&3);

					Math_VectorNormalize(vDirection);
					Math_VectorScale(vDirection,fVelocity,pParticle->vel);
				}
		break;
	case PARTICLE_TYPE_DEFAULT:
		for(i = 0; i < iCount; i++)
		{
			pParticle = Client_AllocateParticle();
			if(!pParticle)
				return;

			pParticle->texture		= iTextureID;
			pParticle->scale		= fScale;
			pParticle->die			= cl.time+0.1f*(rand()%5);
			pParticle->pBehaviour	= pBehaviour;

			// [5/9/2012] TODO: Simplify this ~hogsy
			for(j = 0; j < 3; j++)
				pParticle->org[j] = vOrigin[j]+((rand()&15)-8);

			Math_VectorScale(vDirection,15,pParticle->vel);
		}
		break;
	default:
		Con_Warning("Unknown particle type (%i)!\n",pType);
	}
}

#if 0
#define NUMVERTEXNORMALS	162

vec3_t	avelocities[NUMVERTEXNORMALS],avelocity = {23, 7, 3};

float	beamlength = 16;

void Client_EntityParticles(entity_t *ent)
{
	int			i;
	Particle_t	*p;
	float		angle,
				sr,sp,sy,cr,cp,cy,
				dist=64.0f;
	vec3_t		forward;

	if(!avelocities[0][0])
		for(i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255)*0.01f;

	for (i=0;i<NUMVERTEXNORMALS;i++)
	{
		angle = cl.time*avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = cl.time*avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = cl.time*avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		if(!pFreeParticles)
			return;

		p = pFreeParticles;
		pFreeParticles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->die			= cl.time+0.01f;
		p->pBehaviour	= PARTICLE_BEHAVIOUR_EXPLODE;
		p->org[0]		= ent->origin[0]+r_avertexnormals[i][0]*dist+forward[0]*beamlength;
		p->org[1]		= ent->origin[1]+r_avertexnormals[i][1]*dist+forward[1]*beamlength;
		p->org[2]		= ent->origin[2]+r_avertexnormals[i][2]*dist+forward[2]*beamlength;
	}
}
#endif

// [4/9/2012] Moved over here from KatGL ~hogsy
void Client_ProcessParticles(void)
{
	int					i;
	float				time1,time2,time3,
						dvel,frametime,
						fActualGravity,fGravity,fSlowGravity;
	Particle_t			*pParticle,*kill;

	// [19/3/2013] Moved up here since we were still going through the below... *sighs* ~hogsy
	// [7/8/2013] Don't process if there aren't any active particles! ~hogsy
	if(!active_particles || (cl.bIsPaused || ((key_dest == key_console) && svs.maxclients == 1)))
		return;

	frametime		= cl.time-cl.oldtime;
	time3			= frametime*15.0f;
	time2			= frametime*10.0f;
	time1			= frametime*5.0f;
	// [19/3/2013] TODO: No! Don't use Cvar_VariableValue!! ~hogsy
	fActualGravity	= (	Cvar_VariableValue("server_gravity")*
						Cvar_VariableValue("server_gravityamount"));
	fGravity		= frametime*fActualGravity;
	fSlowGravity	= frametime*fActualGravity*0.05f;
	dvel			= 4.0f*frametime;

	for(;;)
	{
		kill = active_particles;
		if (kill && kill->die < cl.time)
		{
			active_particles	= kill->next;
			kill->next			= pFreeParticles;
			pFreeParticles		= kill;
			continue;
		}
		break;
	}

	for (pParticle=active_particles ; pParticle ; pParticle=pParticle->next)
	{
		for(;;)
		{
			kill = pParticle->next;
			if (kill && kill->die < cl.time)
			{
				pParticle->next	= kill->next;
				kill->next		= pFreeParticles;
				pFreeParticles	= kill;
				continue;
			}
			break;
		}

		for(i = 0; i < 3; i++)
			pParticle->org[i] += pParticle->vel[i]*frametime;

		// [14/8/2013] Set the alpha here ~hogsy
		pParticle->color[3] = CLAMP(0,pParticle->die-cl.time,1.0f);

		// [16/6/2012] Ripped from gl_alias.c ~hogsy
		// [3/9/2012] Moved here ~hogsy
		if(cvLitParticles.value)
			Math_MVToVector(Light_GetSample(pParticle->org),pParticle->color);
		else
			// [22/4/2013] Simplified ~hogsy
			Math_VectorSet(1.0f,pParticle->color);

		switch(pParticle->pBehaviour)
		{
		case PARTICLE_BEHAVIOUR_STATIC:
			break;
		case PARTICLE_SMOKE:
			// [2/9/2012] Smoke particles get larger overtime ~hogsy
			pParticle->scale	+= time3;
			pParticle->vel[2]	+= fSlowGravity/10.0f;
			break;
		case PARTICLE_BEHAVIOUR_FIRE:
			pParticle->ramp += time1;
			if(pParticle->ramp >= 6)
				pParticle->die = -1;

			pParticle->vel[2] += fGravity;
			break;
		case PARTICLE_BEHAVIOUR_EXPLODE:
			pParticle->ramp += time2;
			if(pParticle->ramp >= 8)
				pParticle->die = -1;

			for(i=0 ; i<3 ; i++)
				pParticle->vel[i] += pParticle->vel[i]*dvel;

			pParticle->vel[2] -= fSlowGravity;
			break;
		case PARTICLE_EXPLODE2:
			pParticle->ramp += time3;
			if (pParticle->ramp >=8)
				pParticle->die = -1;

			for (i=0 ; i<3 ; i++)
				pParticle->vel[i] -= pParticle->vel[i]*frametime;

			pParticle->vel[2] -= fSlowGravity;
			break;
		case PARTICLE_BLOB:
			for (i=0 ; i<3 ; i++)
				pParticle->vel[i] += pParticle->vel[i]*dvel;
			pParticle->vel[2] -= fGravity;
			break;
		case PARTICLE_BLOB2:
			for (i=0 ; i<2 ; i++)
				pParticle->vel[i] -= pParticle->vel[i]*dvel;
			pParticle->vel[2] -= fGravity;
			break;
		case PARTICLE_BEHAVIOUR_GRAVITY:
			pParticle->vel[2] -= fGravity;
			break;
		case PARTICLE_BEHAVIOUR_SLOWGRAVITY:
			pParticle->vel[2] -= fSlowGravity;
			break;
		}
	}
}

/*	Gets an effect texture.
*/
int	Client_GetEffect(const char *cPath)
{
	int i;

	for(i = 0; i < MAX_EFFECTS; i++)
		if(!gEffectTexture[i])
		{
			Con_Warning("Failed to find effect texture! (%s)\n",cPath);
			break;
		}
		else if(!strcmp(cPath,gEffectTexture[i]->name))
			return i;

	return 0;
}

/*	Clear out any effects such as particles or flares.
*/
void Client_ClearEffects(void)
{
	int	i;

	pFreeParticles		= &particles[0];
	active_particles	= NULL;

	for(i = 0; i < MAX_PARTICLES; i++)
		particles[i].next = &particles[i+1];

	particles[MAX_PARTICLES-1].next = NULL;

/*
	free_flares=&flares[0];
	active_flares=NULL;
	for(i=0;i<MAX_FLARES;i++)
		flares[i].next=&flares[i+1];
	flares[MAX_FLARES-1].next=NULL;*/
}

// [30/7/2013] "Fixed" so that VS doesn't give us any trouble ~hogsy
gltexture_t	*gMenuTexture[128];

/*	Precache client-side resources.
*/
void Client_PrecacheResource(int iType,char *cResource)
{
	char	cPath[MAX_OSPATH];
	int		i;
	unsigned int    w,h;
	model_t	*mClientModel;
	byte	*bData;

	switch(iType)
	{
	case RESOURCE_FONT:
		break;
	case RESOURCE_MODEL:
		for(i = 0; i < MAX_MODELS; i++)
			if(!cl.model_precache[i])
			{
				mClientModel = Mod_ForName(cResource);
				if(!mClientModel)
				{
					Console_ErrorMessage(false,cResource,"Either the file is corrupt or does not exist.");
					return;
				}

				cl.model_precache[i] = mClientModel;
				return;
			}
			else if(!strcmp(cl.model_precache[i]->name,cResource))
				return;

		Console_ErrorMessage(false,cResource,"Overflow!");
		break;
	// [26/1/2013] Precache for effect types ~hogsy
	case RESOURCE_SPRITE:
		sprintf(cPath,PATH_SPRITES"%s",cResource);

		for(i = 0; i < MAX_EFFECTS; i++)
			if(!gEffectTexture[i])
			{
				bData = Image_LoadImage(cPath,&w,&h);
				if(!bData)
				{
					Con_Warning("Failed to load %s!\n",cPath);
					return;
				}

				gEffectTexture[i] = TexMgr_LoadImage(NULL,cResource,w,h,SRC_RGBA,bData,cPath,0,TEXPREF_ALPHA);
				return;
			}
			else if(!strcmp(cPath,gEffectTexture[i]->name))
				return;

		Console_ErrorMessage(false,cPath,"Overflow!");
		break;
	case RESOURCE_TEXTURE:
		for(i = 0; i < sizeof(gMenuTexture); i++)
			if(!gMenuTexture[i])
			{
				bData = Image_LoadImage(cResource,&w,&h);
				if(!bData)
				{
					Con_Warning("Failed to load %s!\n",cResource);
					return;
				}

				gMenuTexture[i] = TexMgr_LoadImage(NULL,cResource,w,h,SRC_RGBA,bData,cResource,0,TEXPREF_ALPHA);
				return;
			}
			else if(!strcmp(cResource,gMenuTexture[i]->name))
				return;

		Console_ErrorMessage(false,cResource,"Overflow!");
		break;
	default:
		Con_Warning("Attempted to precache an undefined type! (%s)\n",cResource);
	}
}

int Client_GetStat(ClientStat_t csStat)
{
	return cl.stats[csStat];
}
