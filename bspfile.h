#ifndef	__BSPFILESUCKS__
#define	__BSPFILESUCKS__

#define	MAX_MAP_HULLS	4

typedef struct
{
	int		fileofs, filelen;
} lump_t;

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

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest
#define	PLANE_ANYX		3
#define	PLANE_ANYY		4
#define	PLANE_ANYZ		5

#define	CONTENTS_LAVA			-5
#define	CONTENTS_SKY			-6
#define	CONTENTS_ORIGIN			-7		// removed at csg time
#define	CONTENTS_CURRENT_0		-9
#define	CONTENTS_CURRENT_90		-10
#define	CONTENTS_CURRENT_180	-11
#define	CONTENTS_CURRENT_270	-12
#define	CONTENTS_CURRENT_UP		-13
#define	CONTENTS_CURRENT_DOWN	-14

enum
{
	AMBIENT_WATER,
	AMBIENT_SKY,
	AMBIENT_SLIME,
	AMBIENT_LAVA,

	NUM_AMBIENTS
};

#endif
