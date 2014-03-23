#ifndef __KATCLIENT__
#define __KATCLIENT__

#include "quakedef.h"

#include "shared_client.h"

//////////////////////////////////////////////////////////////////////////
// PARTICLES															//
//////////////////////////////////////////////////////////////////////////
extern	Particle_t	*active_particles,*pFreeParticles,*particles;

Particle_t *Client_AllocateParticle(void);

int	Client_GetEffect(const char *cPath);
int Client_GetStat(ClientStat_t csStat);

void Client_ParseParticleEffect(void);
void Client_ClearEffects(void);
void Client_CreateParticleEffect(ParticleType_t	pType,ParticleBehaviour_t pBehaviour,vec3_t	vOrigin,vec3_t vDirection,float	fVelocity,float	fScale,int iCount,int iTextureID);
void Client_ProcessParticles(void);
//////////////////////////////////////////////////////////////////////////

void Client_PrecacheResource(int iType,char *cResource);

#endif