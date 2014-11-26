/*	Copyright (C) 2011-2015 OldTimes Software
*/
#ifndef __SHAREDCLIENT__
#define __SHAREDCLIENT__

/*	Client->Server type
	defines. Currently these
	are all still engine-side
	so it's wise not to touch!
*/
enum
{
	CLC_BAD,								// A bad message which won't be read by the server
	CLC_NOP,
	CLC_DISCONNECT,							// Disconnects from the current server
	CLC_MOVE,								// Lets the server know we're moving
	CLC_STRINGCMD,
/*	Anything beyond this point
	is local.					*/

	CLC_NONE
};

/*	Temporary Entity Types
	CTE stands for Client Temporary Entity.
*/
enum
{
	CTE_EXPLOSION,
	CTE_GUNSHOT,
	CTE_TELEPORT,
	CTE_BLOODSPRAY,
	CTE_PARTICLE_FIELD
};

typedef enum
{
	STAT_HEALTH,
	STAT_FRAGS,
	STAT_WEAPON,
	STAT_AMMO,
	STAT_ARMOR,
	STAT_WEAPONFRAME,
	STAT_ACTIVEWEAPON,
	STAT_TOTALSECRETS,
	STAT_TOTALMONSTERS,
	STAT_SECRETS,		// Bumped on client side by svc_foundsecret.
	STAT_MONSTERS,		// Bumped by svc_killedmonster.

	STAT_NONE
} ClientStat_t;

typedef struct
{
	vec3_t	origin;
	float	radius,
			die,			// Stop lighting after this time
			decay,			// Drop this each second
			minlight;		// Don't add when contributing less
	int		key;
	vec3_t	color;			//johnfitz -- lit support via lordhavoc
	vec3_t	transformed;

	bool	bLightmap;		// Defines whether this light effects the lightmap.
} DynamicLight_t;

typedef void (*xcommand_t)(void);

typedef struct cvar_s
{
	char		*name;				// Name of the variable
	char		*string;			// Default value
	bool		archive;			// Variable will be archived
	bool	    server;
	char        *cDescription;
	float		value;
	struct		cvar_s *next;
	char		*default_string;
	void		(*callback)(void);

	// [27/5/2014] Different value types ~hogsy
	bool	bValue;
	int		iValue;
} cvar_t;

typedef struct
{
	vec3_t			origin,
					angles;
	unsigned short 	modelindex,
					frame;
	unsigned char 	colormap,
					skin,
					alpha;
	int				effects;

	float			fScale;
} entity_state_t;

typedef struct entity_s
{
	bool			bForceLink;		// Model changed

	int				update_type;
	entity_state_t	baseline;		// to fill in defaults in updates
	double			msgtime;		// time of last update
	vec3_t			msg_origins[2];	// last two updates (0 is newest)
	vec3_t			origin;
	vec3_t			msg_angles[2];	// last two updates (0 is newest)
	vec3_t			angles;
	int				draw_lastpose,
					draw_pose;
	float			draw_lerpstart;
	struct model_s	*draw_lastmodel;
	struct model_s	*model;			// NULL = no model
	struct efrag_s	*efrag;			// linked list of efrags
	int				frame;
	byte			*colormap;          // Obsolete
	int				effects;		// light, particles, etc
	int				skinnum;		// for Alias models
	int				visframe;		// last frame this entity was
											//  found in an active leaf
	int				dlightframe;	// dynamic lighting
	int				dlightbits;

// FIXME: could turn these into a union
	int				trivial_accept;
	struct mnode_s	*topnode;		// for bmodels, first world node
											//  that splits bmodel, or NULL if
											//  not split
	byte			alpha;			//johnfitz -- alpha
	byte			lerpflags;		//johnfitz -- lerping
	float			lerpstart;		//johnfitz -- animation lerping
	float			lerpfinish;		//johnfitz -- lerping -- server sent us a more accurate interval, use it instead of 0.1
	short			previouspose;	//johnfitz -- animation lerping
	short			currentpose;	//johnfitz -- animation lerping
	float			movelerpstart;	//johnfitz -- transform lerping
	vec3_t			previousorigin;	//johnfitz -- transform lerping
	vec3_t			currentorigin;	//johnfitz -- transform lerping
	vec3_t			previousangles;	//johnfitz -- transform lerping
	vec3_t			currentangles;	//johnfitz -- transform lerping

	float			scale;			// Sets the model scale.
} entity_t;

//////////////////////////////////////////////////////////////////////////
// PARTICLES															//
//////////////////////////////////////////////////////////////////////////
typedef enum
{
	PARTICLE_BEHAVIOUR_STATIC,
	PARTICLE_BEHAVIOUR_GRAVITY,
	PARTICLE_BEHAVIOUR_SLOWGRAVITY,
	PARTICLE_BEHAVIOUR_FIRE,
	PARTICLE_BEHAVIOUR_EXPLODE,
	PARTICLE_EXPLODE2,
	PARTICLE_BLOB,
	PARTICLE_BLOB2,
	PARTICLE_SMOKE
} ParticleBehaviour_t;

typedef enum
{
	PARTICLE_TYPE_DEFAULT,
	PARTICLE_TYPE_LAVASPLASH,
	PARTICLE_TYPE_TELEPORTSPLASH
} ParticleType_t;

typedef struct particle_s
{
	vec3_t					org;		// The origin of the particle.
	vec3_t					vel;
	vec4_t					color;		// RGBA colour of the particle.

	int						texture;	// The texture id of the particle.
	ParticleBehaviour_t		pBehaviour;	// Type of behaviour.

	struct		particle_s	*next;

	float					ramp,
							die,
							scale;		// Size/scale of the particle.
} Particle_t;
//////////////////////////////////////////////////////////////////////////

#endif
