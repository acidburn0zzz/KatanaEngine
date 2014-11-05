/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef	__SHAREDFORMATS__
#define	__SHAREDFORMATS__

typedef enum
{
	MODEL_TYPE_MD2,		// MD2 support.
	MODEL_TYPE_IQM,		// IQM support.
	MODEL_TYPE_KMDL,	// Extended MD2 support.
	MODEL_TYPE_OBJ,		// OBJ support.
	MODEL_TYPE_SPRITE,	// Just a flat-plane with a texture.
	MODEL_TYPE_BSP,		// BSP support.

	MODEL_NONE
} ModelType_t;

/*
	KMDL Format
	Extension of the already existing MD2 format.
*/

#define	KMDL_VERSION	9

typedef struct
{
	vec3_t	index_xyz[3],
			index_st[3];
} KMDLTriangle_t;

/*
	MD2 Format
*/

#define MD2_HEADER		(('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define	MD2_EXTENSION	".md2"

#define	MD2_VERSION	8

#define	MD2_MAX_SIZE		(1024*4200)
#define	MD2_MAX_FRAMES		1024
#define MD2_MAX_SKINS		32
#define MD2_MAX_TRIANGLES	4096
#define	MD2_MAX_VERTICES	12288

typedef struct
{
	short	index_xyz[3];
	short	index_st[3];
} MD2Triangle_t;

typedef struct
{
	byte	v[3];			// scaled byte to fit in frame mins/maxs
	byte	lightnormalindex;
} MD2TriangleVertex_t;

typedef struct
{
	float				scale[3];		// multiply byte verts by this
	float				translate[3];	// then add this
	char				name[16];		// frame name from grabbing
	MD2TriangleVertex_t	verts[1];		// variable sized
} MD2Frame_t;

typedef struct
{
	short	S,T;
} MD2TextureCoordinate_t;

typedef struct
{
	int			    ident;
	int			    version;
	unsigned    int	skinwidth;
	unsigned    int	skinheight;
	int			    framesize;		// Byte size of each frame.
	int			    num_skins;
	int			    num_xyz;
	int			    num_st;			// Greater than num_xyz for seams.
	int			    numtris;
	int			    num_glcmds;		// Dwords in strip/fan command list.
	int			    num_frames;
	int			    ofs_skins;		// Each skin is a MAX_SKINNAME string.
	int			    ofs_st;			// Byte offset from start for stverts.
	int			    ofs_tris;		// Offset for dtriangles.
	int			    ofs_frames;		// Offset for first frame.
	int			    ofs_glcmds;
	int			    ofs_end;		// End of file.

	MD2TextureCoordinate_t	*mtcTextureCoord;
} MD2_t;

/*
	OBJ Format
*/

#define	OBJ_EXTENSION	".obj"

typedef struct
{
	vec3_t	vVertex;
} OBJVertex_t;

typedef struct
{
	int	iTriangles;

	OBJVertex_t	*ovVertex;
} OBJ_t;

/*
	IQM Format
*/

#define IQM_HEADER		"INTERQUAKEMODEL"
#define	IQM_EXTENSION	".iqm"

#define	IQM_VERSION	2

#define IQM_LOOP	1<<0

enum
{
	IQM_POSITION,
	IQM_TEXCOORD,
	IQM_NORMAL,
	IQM_TANGENT,
	IQM_BLENDINDEXES,
	IQM_BLENDWEIGHTS,
	IQM_COLOR,
	IQM_CUSTOM = 0x10
};

enum
{
	IQM_BYTE,
	IQM_UBYTE,
	IQM_SHORT,
	IQM_USHORT,
	IQM_INT,
	IQM_UINT,
	IQM_HALF,
	IQM_FLOAT,
	IQM_DOUBLE
};

typedef struct
{
	char			cMagic[16];
	unsigned int	uiVersion;
	unsigned int	uiFileSize;
	unsigned int	uiFlags;
	unsigned int	uiNumText,uiOfsText;
	unsigned int	uiNumMeshes,uiOfsMeshes;
	unsigned int	uiNumVertexArrays,uiNumVertexes,uiOfsVertexArrays;
	unsigned int	uiNumTriangles,uiOfsTriangles,uiOfsAdjacency;
	unsigned int	uiNumJoints,uiOfsJoints;
	unsigned int	uiNumPoses,uiOfsPoses;
	unsigned int	uiNumAnims,uiOfsAnims;
	unsigned int	uiNumFrames,uiNumFrameChannels,uiOfsFrames,uiOfsBounds;
	unsigned int	uiNumComment,uiOfsComment;
	unsigned int	uiNumExtensions,uiOfsExtensions;
} IQMHeader_t;

typedef struct
{
	unsigned int	uiName;
	unsigned int	uiMaterial;
	unsigned int	uiFirstVertex,uiNumVertexes;
	unsigned int	uiFirstTriangle,uiNumTriangles;
} IQMMesh_t;

typedef struct
{
	unsigned int	uiVertex[3];
} IQMTriangle_t;

typedef struct
{
	unsigned int	uiName;
	int				iParent;
	// [21/8/2012] TODO: Change over to vec3_t etc? ~hogsy
	float			fTranslate[3],fRotate[4],fScale[3];
} IQMJoint_t;

typedef struct
{
	int				iParent;
	unsigned int	uiMask;
	float			fChannelOffset[10];
	float			fChannelScale[10];
} IQMPose_t;

typedef struct
{
	unsigned int	uiName;
	unsigned int	uiFirstFrame,uiNumFrames;
	float			fFrameRate;
	unsigned int	uiFlags;
} IQMAnim_t;

typedef struct
{
	unsigned int	uiType;
	unsigned int	uiFlags;
	unsigned int	uiFormat;
	unsigned int	uiSize;
	unsigned int	uiOffset;
} IQMVertexArray_t;

typedef struct
{
	// [21/8/2012] TODO: Change to vec3_t? ~hogsy
	float	fBBMin[3],fBBMax[3];
	float	fXYRadius,fRadius;
} IQMBounds_t;

/*
	LVL	Format
*/

#define LVL_HEADER      (('4'<<24)+('L'<<16)+('V'<<8)+'L')
#define	LVL_EXTENSION	".level"

/*
	BSP Format
*/

#define	BSP_VERSION	4
#define	BSP_VERSION_4	// Enable support for version 4 feature set.

#define	BSP_HEADER		(('4'<<24)+('L'<<16)+('V'<<8)+'L')	// For easy identification.
#define	BSP_HEADER_SIZE	8									// "BSP" followed by version number.
#define	BSP_EXTENSION	".bsp"

#define	BSP_MIP_LEVELS	4

#define	BSP_MAX_HULLS			4
#define	BSP_MAX_LEAFS			32768			//0x400000
#define	BSP_MAX_ENTITIES		32768
#define	BSP_MAX_VERTS			65535			//0x100000
#define	BSP_MAX_MODELS			4096
#define	BSP_MAX_BRUSHES			32768			//0x100000
#define	BSP_MAX_ENTSTRING		0x100000		//0x400000
#define	BSP_MAX_PLANES			65536			//0x200000
#define	BSP_MAX_NODES			32767			//0x200000
#define	BSP_MAX_CLIPNODES		32767			//0x800000
#define	BSP_MAX_FACES			65536			//0x200000
#define	BSP_MAX_MARKSURFACES	65535			//0x400000
#define	BSP_MAX_TEXINFO			BSP_MAX_FACES	//0x100000
#define	BSP_MAX_EDGES			0x100000		//0x400000
#define	BSP_MAX_SURFEDGES		0x200000		//0x800000
#define	BSP_MAX_MIPTEX			0x800000		//0x1000000
#define	BSP_MAX_LIGHTING		0x400000		//0x1000000
#define	BSP_MAX_LIGHTMAPS		4
#define	BSP_MAX_VISIBILITY		0x400000		//0x1000000

enum
{
	BSP_TEXTURE_SPECIAL = 1,	// Used for sky or water.
	BSP_TEXTURE_SKIP,			// Surface to be skipped during rendering (and lighting).

	BSP_TEXTURE_MISSING			// Indicates that the texture is missing.
};

/*	Different content types
	set for brushes by Katana Level.
	Probably best not to touch
	unless you're planning on
	updating the tools yourself.
*/
#define	BSP_CONTENTS_EMPTY	-1	// Empty space.
#define	BSP_CONTENTS_SOLID	-2	// Solid brush.
#define	BSP_CONTENTS_WATER	-3	// Water
#define BSP_CONTENTS_SLIME  -4
#define BSP_CONTENTS_LAVA	-5	// Lava content.
#define	BSP_CONTENTS_SKY	-7	// Sky content.
#define	BSP_CONTENTS_0		-9
#define	BSP_CONTENTS_DOWN	-14	// Down current.
#define	BSP_CONTENTS_ORIGIN	-15	// Origin brush, removed during CSG time.

enum
{
	LUMP_ENTITIES,
	LUMP_PLANES,
	LUMP_TEXTURES,
	LUMP_VERTEXES,
	LUMP_VISIBILITY,
	LUMP_NODES,
	LUMP_TEXINFO,
	LUMP_FACES,
	LUMP_LIGHTING,
	LUMP_CLIPNODES,
	LUMP_LEAFS,
	LUMP_MARKSURFACES,
	LUMP_EDGES,
	LUMP_SURFEDGES,
	LUMP_MODELS,

	HEADER_LUMPS
};

typedef enum
{
	BSP_AMBIENT_WATER,
	BSP_AMBIENT_SKY,
	BSP_AMBIENT_SLIME,
	BSP_AMBIENT_LAVA,

	BSP_AMBIENT_END
} BSPAmbient_t;

typedef struct
{
	int	iFileOffset,iFileLength;
} BSPLump_t;

typedef struct
{
	int			iIdent,iVersion;
	BSPLump_t	bLumps[HEADER_LUMPS];
} BSPHeader_t;

typedef struct
{
	float	fMins[3],fMaxs[3],
			fOrigin[3];

	int		iHeadNode[BSP_MAX_HULLS],
			iVisLeafs;

	unsigned int	iFirstFace,
					iNumFaces;
} BSPModel_t;

typedef struct
{
	float	fPoint[3];
} BSPVertex_t;

typedef struct
{
	float	fNormal[3];
	float	fDist;
	int		iType;
} BSPPlane_t;

typedef struct
{
	int					iPlaneNum,
						iChildren[2];

	float				fMins[3],fMaxs[3];

	unsigned	int		usFirstFace,
						usNumFaces;
} BSPNode_t;

typedef struct
{
	int		iPlaneNum,
			iChildren[2];
} BSPClipNode_t;

typedef struct
{
	float	v[2][4];
	int		iMipTex,
			iFlags;
} BSPTextureInfo_t;

typedef struct
{
	unsigned	int	v[2];
} BSPEdge_t;

typedef struct
{
	int		iPlaneNum,
			iSide,
			iFirstEdge,
			iNumEdges,
			iTexInfo;

	byte	bStyles[4];

	int		iLightOffset;
} BSPFace_t;

typedef struct
{
	int					iContents,
						iVisibilityOffset;

	float				fMins[3],fMaxs[3];

	unsigned	int		uiFirstMarkSurface,
						uiNumMarkSurfaces;

	byte				bAmbientLevel[BSP_AMBIENT_END];
} BSPLeaf_t;

// Obsolete

typedef struct
{
	int			nummiptex;
	int			dataofs[4];		// [nummiptex]
} dmiptexlump_t;

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

#endif
