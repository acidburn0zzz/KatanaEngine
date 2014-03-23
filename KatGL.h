#ifndef __KATGL__
#define __KATGL__

#include "quakedef.h"

// [3/9/2012] Included purely for ptype_t ~hogsy
#include "engine_client.h"

#define PARTICLE_DEFAULT		0
#define PARTICLE_LAVASPLASH		1
#define PARTICLE_TELEPORTSPLASH	2

struct	gltexture_s	*gEffectTexture[MAX_EFFECTS];

/*	TODO:
		Merge with sprite stuff?	*/
typedef struct flare_s
{
	vec3_t	org;
	float	scale;
	float	r;
	float	g;
	float	b;
	float	alpha;
	int		texture;
	struct	flare_s	*next;
} flare_t;
flare_t	*active_flares,*free_flares,*flares;

void	R_InitExperimental(void);
void	R_DrawString(int x,int y,char *msg);
void	Draw_Shadow(entity_t *ent);
void	R_DrawFlares(void);

#endif