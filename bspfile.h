#ifndef	__BSPFILESUCKS__
#define	__BSPFILESUCKS__

#define	MAX_MAP_HULLS	4

typedef struct
{
	int		fileofs, filelen;
} lump_t;

typedef struct
{
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[MAX_MAP_HULLS];
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} dmodel_t;

typedef struct
{
	int			version;
	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct
{
	int			nummiptex;
	int			dataofs[4];		// [nummiptex]
} dmiptexlump_t;

#define	MIPLEVELS	4
typedef struct miptex_s
{
	char		name[16];
	unsigned	width,height;
} miptex_t;

typedef struct
{
	float	point[3];
} dvertex_t;

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest
#define	PLANE_ANYX		3
#define	PLANE_ANYY		4
#define	PLANE_ANYZ		5

typedef struct
{
	float	normal[3];
	float	dist;
	int		type;		// PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} dplane_t;

#define	CONTENTS_SLIME			-4
#define	CONTENTS_LAVA			-5
#define	CONTENTS_SKY			-6
#define	CONTENTS_ORIGIN			-7		// removed at csg time
#define	CONTENTS_CLIP			-8		// changed to contents_solid
#define	CONTENTS_CURRENT_0		-9
#define	CONTENTS_CURRENT_90		-10
#define	CONTENTS_CURRENT_180	-11
#define	CONTENTS_CURRENT_270	-12
#define	CONTENTS_CURRENT_UP		-13
#define	CONTENTS_CURRENT_DOWN	-14

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
	int				planenum;
	short			children[2];	// negative numbers are -(leafs+1), not nodes
	short			mins[3];		// for sphere culling
	short			maxs[3];
	unsigned short	firstface;
	unsigned short	numfaces;	// counting both sides
} dnode_t;

typedef struct
{
	int	planenum;
	short	children[2];	// negative numbers are contents
} dclipnode_t;

typedef struct texinfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			miptex;
	int			flags;
} texinfo_t;
#define	TEX_SPECIAL		1		// sky or slime, no lightmap or 256 subdivision
#define TEX_MISSING		2		// johnfitz -- this texinfo does not have a texture

// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
typedef struct
{
	unsigned short	v[2];		// vertex numbers
} dedge_t;

#define	MAXLIGHTMAPS	4
typedef struct
{
	short		planenum;
	short		side;
	int			firstedge;		// we must support > 64k edges
	short		numedges;
	short		texinfo;

// lighting info
	byte		styles[MAXLIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
} dface_t;

enum
{
	AMBIENT_WATER,
	AMBIENT_SKY,
	AMBIENT_SLIME,
	AMBIENT_LAVA,

	NUM_AMBIENTS
};

// leaf 0 is the generic CONTENTS_SOLID leaf, used for all solid areas
// all other leafs need visibility info
typedef struct
{
	int			contents;
	int			visofs;				// -1 = no visibility info

	short		mins[3];			// for frustum culling
	short		maxs[3];

	unsigned short		firstmarksurface;
	unsigned short		nummarksurfaces;

	byte		ambient_level[NUM_AMBIENTS];
} dleaf_t;

#endif