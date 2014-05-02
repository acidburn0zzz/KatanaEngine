/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef	__SHAREDFORMATS__
#define	__SHAREDFORMATS__

typedef enum
{
	MODEL_MD2,
	MODEL_SPRITE,
	MODEL_BRUSH,

	MODEL_NONE
} ModelType_t;

/*
	MD2 Format
*/

#define MD2_HEADER	(('2'<<24)+('P'<<16)+('D'<<8)+'I')

#define	MD2_VERSION	8

#define	MD2_MAX_SIZE		(1024*4200)
#define	MD2_MAX_FRAMES		1024
#define MD2_MAX_SKINS		32
#define MD2_MAX_TRIANGLES	4096
#define	MD2_MAX_VERTICES	2048

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
	int			    framesize;							// Byte size of each frame.
	int			    num_skins;
	int			    num_xyz;
	int			    num_st;								// Greater than num_xyz for seams.
	int			    numtris;
	int			    num_glcmds;							// Dwords in strip/fan command list.
	int			    num_frames;
	int			    ofs_skins;							// Each skin is a MAX_SKINNAME string.
	int			    ofs_st;								// Byte offset from start for stverts.
	int			    ofs_tris;							// Offset for dtriangles.
	int			    ofs_frames;							// Offset for first frame.
	int			    ofs_glcmds;
	int			    ofs_end;							// End of file.

	struct gltexture_s	*gDiffuseTexture[MD2_MAX_SKINS],	// Diffuse texture.
						*gFullbrightTexture[MD2_MAX_SKINS],	// Texture used for fullbright layer.
						*gSphereTexture[MD2_MAX_SKINS];		// Texture used for sphere mapping.

	MD2Triangle_t	*mtTriangles;
} MD2_t;

/*
	IQM Format
*/

#define IQM_HEADER	"INTERQUAKEMODEL"

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
	BSP Format
*/

#define	BSP_HEADER	(('P'<<16)+('S'<<8)+'B')

#define	BSP_VERSION	3

#define	BSP_MAX_HULLS		16
#define	BSP_MAX_LEAFS		32767
#define	BSP_MAX_ENTITIES	32768

/*	Different content types
	set for brushes by Katana Level.
	Probably best not to touch
	unless you're planning on
	updating the tools yourself.
*/
#define	BSP_CONTENTS_EMPTY	-1	// Empty space.
#define	BSP_CONTENTS_SOLID	-2	// Solid brush.
#define	BSP_CONTENTS_WATER	-3	// Water

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
	int			iVersion;
	BSPLump_t	bLumps[HEADER_LUMPS];
} BSPHeader_t;

typedef struct
{
	float	fMins[3],fMaxs[3];
	float	fOrigin[3];
	int		iHeadNode[BSP_MAX_HULLS];
	int		iVisLeafs;
	int		iFirstFace,iNumFaces;
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
	int					iPlaneNum;
	short				sChildren[2];
	short				sMins[3],sMaxs[3];
	unsigned	short	usFirstFace;
	unsigned	short	usNumFaces;
} BSPNode_t;

typedef struct
{
	int		iPlaneNum;
	short	sChildren[2];
} BSPClipNode_t;

typedef struct
{
	float	v[2][4];
	int		iMipTex;
	int		iFlags;
} BSPTextureInfo_t;

typedef struct
{
	unsigned	short	v[2];
} BSPEdge_t;

typedef struct
{
	short	sPlaneNum;
	short	sSide;
	int		iFirstEdge;
	short	sNumEdges;
	short	sTexInfo;
	byte	bStyles[BSP_MAX_HULLS];
	int		iLightOffset;
} BSPFace_t;

typedef struct
{
	int					iContents,
						iVisibilityOffset;

	short				sMins[3],sMaxs[3];

	unsigned	short	usFirstMarkSurface,
						usNumMarkSurfaces;

	byte				bAmbientLevel[BSP_AMBIENT_END];
} BSPLeaf_t;

#endif
