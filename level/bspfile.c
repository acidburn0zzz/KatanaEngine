/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "mem.h"

int			nummodels;
BSPModel_t	dmodels[BSP_MAX_MODELS];

int			visdatasize;
byte		dvisdata[BSP_MAX_VISIBILITY];

int			lightdatasize;
byte		dlightdata[BSP_MAX_LIGHTING];

// LordHavoc: stored in .lit file
int			rgblightdatasize;
byte		drgblightdata[BSP_MAX_LIGHTING*3];

int			texdatasize;
byte		dtexdata[BSP_MAX_MIPTEX]; // (dmiptexlump_t)

int			entdatasize;
char		dentdata[BSP_MAX_ENTSTRING];

int			numleafs;
BSPLeaf_t	dleafs[BSP_MAX_LEAFS];

int			numplanes;
BSPPlane_t	dplanes[BSP_MAX_PLANES];

int			numvertexes;
BSPVertex_t	dvertexes[BSP_MAX_VERTS];

int			numnodes;
BSPNode_t	dnodes[BSP_MAX_NODES];

int					numtexinfo;
BSPTextureInfo_t	texinfo[BSP_MAX_TEXINFO];

int			numfaces;
BSPFace_t	dfaces[BSP_MAX_FACES];

int				numclipnodes;
BSPClipNode_t	dclipnodes[BSP_MAX_CLIPNODES];

int			numedges;
BSPEdge_t	dedges[BSP_MAX_EDGES];

int				nummarksurfaces;
unsigned int	dmarksurfaces[BSP_MAX_MARKSURFACES];

int			numsurfedges;
int			dsurfedges[BSP_MAX_SURFEDGES];

hullinfo_t	hullinfo;

//=============================================================================

typedef struct
{
	int		initialized;
	int		maxsize;
	byte	*start;
	byte	*index;
} swappedbuffer_t;

void SB_LoadFile (swappedbuffer_t *sbuf, char *filename)
{
	FILE	*f;

	f = SafeOpenRead (filename);
	sbuf->maxsize = Q_filelength (f);
	sbuf->start = qmalloc (sbuf->maxsize + 1);
	sbuf->start[sbuf->maxsize] = 0;
	SafeRead (f, sbuf->start, sbuf->maxsize);
	fclose (f);

	sbuf->index = sbuf->start;
	sbuf->initialized = true;
}

void SB_Alloc (swappedbuffer_t *sbuf, int maxsize)
{
	sbuf->maxsize = maxsize;
	sbuf->start = qmalloc (maxsize);
	sbuf->index = sbuf->start;
	sbuf->initialized = true;
}

void SB_Free (swappedbuffer_t *sbuf)
{
	if (!sbuf->initialized)
		return;

	qfree (sbuf->start);
	memset (sbuf, 0, sizeof(*sbuf));
}

byte SB_ReadByte (swappedbuffer_t *sbuf)
{
	byte b;
	b = *sbuf->index++;
	return b;
}

short SB_ReadShort (swappedbuffer_t *sbuf)
{
	short s;
	s = sbuf->index[0] + 256*sbuf->index[1];
	sbuf->index += 2;
	return s;
}

int SB_ReadInt (swappedbuffer_t *sbuf)
{
	int i;
	i = sbuf->index[0] + 256*sbuf->index[1] + 65536*sbuf->index[2] + 16777216*sbuf->index[3];
	sbuf->index += 4;
	return i;
}

float SB_ReadFloat (swappedbuffer_t *sbuf)
{
	union
	{
		int		i;
		float	f;
	} u;

	u.i = SB_ReadInt (sbuf);
	return u.f;
}

void SB_ReadData (swappedbuffer_t *sbuf, void *d, int n)
{
	memcpy (d, sbuf->index, n);
	sbuf->index += n;
}

void SB_ZeroFill (swappedbuffer_t *sbuf, int n)
{
	while (n--)
		*sbuf->index++ = 0;
}

void SB_WriteByte (swappedbuffer_t *sbuf, byte b)
{
	*sbuf->index++ = b;
}

void SB_WriteShort (swappedbuffer_t *sbuf, short i)
{
	sbuf->index[0] = i & 0xFF;
	sbuf->index[1] = (i >> 8) & 0xFF;
	sbuf->index += 2;
}

void SB_WriteInt (swappedbuffer_t *sbuf, int i)
{
	sbuf->index[0] = i & 0xFF;
	sbuf->index[1] = (i >> 8) & 0xFF;
	sbuf->index[2] = (i >> 16) & 0xFF;
	sbuf->index[3] = (i >> 24) & 0xFF;
	sbuf->index += 4;
}

void SB_WriteFloat (swappedbuffer_t *sbuf, float f)
{
	union
	{
		int		i;
		float	f;
	} u;

	u.f = f;
	SB_WriteInt (sbuf, u.i);
}

void SB_WriteData (swappedbuffer_t *sbuf, void *d, int n)
{
	memcpy (sbuf->index, d, n);
	sbuf->index += n;
}

void SB_WriteString (swappedbuffer_t *sbuf, char *s)
{
	while (*s)
		*sbuf->index++ = *s++;
	*sbuf->index++ = 0;
}

void SB_SeekAbsolute (swappedbuffer_t *sbuf, int index)
{
	sbuf->index = sbuf->start + index;
}

int SB_Tell (swappedbuffer_t *sbuf)
{
	return sbuf->index - sbuf->start;
}

void	LoadBSPFile (char *filename)
{
	int				i, j, headerend;
	swappedbuffer_t	sb;
	BSPLump_t		lumps[HEADER_LUMPS], *lump;

// load file into buffer
	SB_LoadFile(&sb, filename);

// check header
	i = SB_ReadInt(&sb);
	if(i != BSP_HEADER)
		Error("Invalid file type!");

	i = SB_ReadInt(&sb);
	if(i != BSP_VERSION)
		Error ("%s is version %i, should be %i!", filename, i, BSP_VERSION);

// hull 0 is always point-sized
	VectorClear (hullinfo.hullsizes[0][0]);
	VectorClear (hullinfo.hullsizes[0][1]);

	hullinfo.numhulls = 3;
	hullinfo.filehulls = 4;
	VectorSet (hullinfo.hullsizes[1][0], -16, -16, -24);
	VectorSet (hullinfo.hullsizes[1][1], 16, 16, 32);
	VectorSet (hullinfo.hullsizes[2][0], -32, -32, -24);
	VectorSet (hullinfo.hullsizes[2][1], 32, 32, 64);

	// parse extended header (up to 128 lumps) by keeping track of first offset
	headerend = 4+sizeof(lumps);
	for (i = 0; i < HEADER_LUMPS; i++)
	{
		if(headerend >= 4+(i+1)*sizeof(lumps[0]))
		{
			lumps[i].iFileOffset = SB_ReadInt (&sb);
			lumps[i].iFileLength = SB_ReadInt (&sb);
			if (headerend > lumps[i].iFileOffset && lumps[i].iFileLength)
				headerend = lumps[i].iFileOffset;
		}
		else
			lumps[i].iFileOffset = lumps[i].iFileLength = 0;
	}

// read lumps (sigh...)
	lump = &lumps[LUMP_PLANES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numplanes = lump->iFileLength / 20;
	for (i = 0; i < numplanes; i++)
	{
		dplanes[i].fNormal[0]	= SB_ReadFloat (&sb);
		dplanes[i].fNormal[1]	= SB_ReadFloat (&sb);
		dplanes[i].fNormal[2]	= SB_ReadFloat (&sb);
		dplanes[i].fDist		= SB_ReadFloat (&sb);
		dplanes[i].iType		= SB_ReadInt (&sb);
	}

	lump = &lumps[LUMP_LEAFS];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numleafs = lump->iFileLength / 44;
	for (i = 0; i < numleafs; i++)
	{
		dleafs[i].iContents = SB_ReadInt (&sb);
		dleafs[i].iVisibilityOffset = SB_ReadInt (&sb);
		dleafs[i].fMins[0] = SB_ReadFloat (&sb);
		dleafs[i].fMins[1] = SB_ReadFloat (&sb);
		dleafs[i].fMins[2] = SB_ReadFloat (&sb);
		dleafs[i].fMaxs[0] = SB_ReadFloat (&sb);
		dleafs[i].fMaxs[1] = SB_ReadFloat (&sb);
		dleafs[i].fMaxs[2] = SB_ReadFloat (&sb);
		dleafs[i].uiFirstMarkSurface = SB_ReadInt (&sb);
		dleafs[i].uiNumMarkSurfaces = SB_ReadInt (&sb);
		for (j = 0; j < BSP_AMBIENT_END; j++)
			dleafs[i].bAmbientLevel[j] = SB_ReadByte (&sb);
	}

	lump = &lumps[LUMP_VERTEXES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numvertexes = lump->iFileLength / 12;
	for (i = 0; i < numvertexes; i++)
	{
		dvertexes[i].fPoint[0] = SB_ReadFloat (&sb);
		dvertexes[i].fPoint[1] = SB_ReadFloat (&sb);
		dvertexes[i].fPoint[2] = SB_ReadFloat (&sb);
	}

	lump = &lumps[LUMP_NODES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numnodes = lump->iFileLength / 44;
	for (i = 0; i < numnodes; i++)
	{
		dnodes[i].iPlaneNum = SB_ReadInt (&sb);
		dnodes[i].iChildren[0] = SB_ReadInt (&sb);
		dnodes[i].iChildren[1] = SB_ReadInt (&sb);
		dnodes[i].fMins[0] = SB_ReadFloat (&sb);
		dnodes[i].fMins[1] = SB_ReadFloat (&sb);
		dnodes[i].fMins[2] = SB_ReadFloat (&sb);
		dnodes[i].fMaxs[0] = SB_ReadFloat (&sb);
		dnodes[i].fMaxs[1] = SB_ReadFloat (&sb);
		dnodes[i].fMaxs[2] = SB_ReadFloat (&sb);
		dnodes[i].usFirstFace = SB_ReadInt (&sb);
		dnodes[i].usNumFaces = SB_ReadInt (&sb);
	}

	lump = &lumps[LUMP_TEXINFO];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numtexinfo = lump->iFileLength / 40;
	for (i = 0; i < numtexinfo; i++)
	{
		texinfo[i].v[0][0]	= SB_ReadFloat (&sb);
		texinfo[i].v[0][1]	= SB_ReadFloat (&sb);
		texinfo[i].v[0][2]	= SB_ReadFloat (&sb);
		texinfo[i].v[0][3]	= SB_ReadFloat (&sb);
		texinfo[i].v[1][0]	= SB_ReadFloat (&sb);
		texinfo[i].v[1][1]	= SB_ReadFloat (&sb);
		texinfo[i].v[1][2]	= SB_ReadFloat (&sb);
		texinfo[i].v[1][3]	= SB_ReadFloat (&sb);
		texinfo[i].iMipTex	= SB_ReadInt (&sb);
		texinfo[i].iFlags	= SB_ReadInt (&sb);
	}

	lump = &lumps[LUMP_FACES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numfaces = lump->iFileLength / 28;
	for (i = 0; i < numfaces; i++)
	{
		dfaces[i].iPlaneNum = SB_ReadInt (&sb);
		dfaces[i].iSide = SB_ReadInt (&sb);
		dfaces[i].iFirstEdge = SB_ReadInt (&sb);
		dfaces[i].iNumEdges = SB_ReadInt (&sb);
		dfaces[i].iTexInfo = SB_ReadInt (&sb);
		for (j = 0; j < BSP_MAX_LIGHTMAPS; j++)
			dfaces[i].bStyles[j] = SB_ReadByte (&sb);
		dfaces[i].iLightOffset = SB_ReadInt (&sb);
	}

	lump = &lumps[LUMP_CLIPNODES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numclipnodes = lump->iFileLength / 12;
	for (i = 0; i < numclipnodes; i++)
	{
		dclipnodes[i].iPlaneNum = SB_ReadInt (&sb);
		dclipnodes[i].iChildren[0] = SB_ReadInt (&sb);
		dclipnodes[i].iChildren[1] = SB_ReadInt (&sb);
	}

	lump = &lumps[LUMP_MARKSURFACES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	nummarksurfaces = lump->iFileLength / 4;
	for (i = 0; i < nummarksurfaces; i++)
		dmarksurfaces[i] = SB_ReadInt (&sb);

	lump = &lumps[LUMP_SURFEDGES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numsurfedges = lump->iFileLength / 4;
	for (i = 0; i < numsurfedges; i++)
		dsurfedges[i] = SB_ReadInt (&sb);

	lump = &lumps[LUMP_EDGES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	numedges = lump->iFileLength / 8;
	for (i = 0; i < numedges; i++)
	{
		dedges[i].v[0] = SB_ReadInt (&sb);
		dedges[i].v[1] = SB_ReadInt (&sb);
	}

	lump = &lumps[LUMP_MODELS];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	nummodels = lump->iFileLength / (48+4*hullinfo.filehulls);
	for (i = 0; i < nummodels; i++)
	{
		dmodels[i].fMins[0] = SB_ReadFloat (&sb);
		dmodels[i].fMins[1] = SB_ReadFloat (&sb);
		dmodels[i].fMins[2] = SB_ReadFloat (&sb);
		dmodels[i].fMaxs[0] = SB_ReadFloat (&sb);
		dmodels[i].fMaxs[1] = SB_ReadFloat (&sb);
		dmodels[i].fMaxs[2] = SB_ReadFloat (&sb);
		dmodels[i].fOrigin[0] = SB_ReadFloat (&sb);
		dmodels[i].fOrigin[1] = SB_ReadFloat (&sb);
		dmodels[i].fOrigin[2] = SB_ReadFloat (&sb);
		for (j = 0; j < hullinfo.filehulls; j++)
			dmodels[i].iHeadNode[j] = SB_ReadInt (&sb);
		dmodels[i].iVisLeafs = SB_ReadInt (&sb);
		dmodels[i].iFirstFace = SB_ReadInt (&sb);
		dmodels[i].iNumFaces = SB_ReadInt (&sb);
	}

	lump = &lumps[LUMP_LIGHTING];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	rgblightdatasize = lump->iFileLength;
	SB_ReadData (&sb, drgblightdata, rgblightdatasize);

	lump = &lumps[LUMP_VISIBILITY];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	visdatasize = lump->iFileLength;
	SB_ReadData (&sb, dvisdata, visdatasize);

	lump = &lumps[LUMP_ENTITIES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	entdatasize = lump->iFileLength;
	SB_ReadData (&sb, dentdata, entdatasize);

	lump = &lumps[LUMP_TEXTURES];
	SB_SeekAbsolute (&sb, lump->iFileOffset);
	texdatasize = lump->iFileLength;
	SB_ReadData (&sb, dtexdata, texdatasize);

// finish up
	SB_Free(&sb);
}

/*	Swaps the bsp file in place, so it should not be referenced again
*/
void WriteBSPFile (char *filename)
{
	int				i, j;
	FILE			*f;
	swappedbuffer_t	sb;
	int				index;
	int				bspsize;
	BSPLump_t		lumps[HEADER_LUMPS],*lump;

	bspsize = BSP_HEADER_SIZE;
	bspsize += sizeof(lumps)+20*numplanes+(40+BSP_AMBIENT_END)*numleafs+12*numvertexes;
	bspsize += 44*numnodes+40*numtexinfo+(24+BSP_MAX_LIGHTMAPS)*numfaces+12*numclipnodes;
	bspsize += 4*nummarksurfaces+4*numsurfedges+8*numedges;
	bspsize += (48+4*BSP_MAX_HULLS)*nummodels+rgblightdatasize;
	bspsize += visdatasize+entdatasize+texdatasize;
	bspsize += 512;	// extra case for safety and to compensate for the 4-byte padding of the lumps

	SB_Alloc (&sb, bspsize);
	printf ("Allocated %f MB (%d bytes) for file buffer\n", bspsize*(1.0f/(1024.0f*1024.0f)), bspsize);

	// write header
	SB_ZeroFill(&sb,BSP_HEADER_SIZE); // filled in later
	SB_ZeroFill(&sb,sizeof(lumps));	// filled in later

	// write lumps and pad each one to a multiple of 4 bytes
	lump = &lumps[LUMP_PLANES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numplanes; i++)
	{
		SB_WriteFloat(&sb,dplanes[i].fNormal[0]);
		SB_WriteFloat(&sb,dplanes[i].fNormal[1]);
		SB_WriteFloat(&sb,dplanes[i].fNormal[2]);
		SB_WriteFloat(&sb,dplanes[i].fDist);
		SB_WriteInt(&sb,dplanes[i].iType);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill(&sb,((lump->iFileLength+3)&~3)-lump->iFileLength);

	lump = &lumps[LUMP_LEAFS];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numleafs; i++)
	{
		SB_WriteInt (&sb, dleafs[i].iContents);
		SB_WriteInt (&sb, dleafs[i].iVisibilityOffset);
		SB_WriteFloat (&sb, dleafs[i].fMins[0]);
		SB_WriteFloat (&sb, dleafs[i].fMins[1]);
		SB_WriteFloat (&sb, dleafs[i].fMins[2]);
		SB_WriteFloat (&sb, dleafs[i].fMaxs[0]);
		SB_WriteFloat (&sb, dleafs[i].fMaxs[1]);
		SB_WriteFloat (&sb, dleafs[i].fMaxs[2]);
		SB_WriteInt (&sb, dleafs[i].uiFirstMarkSurface);
		SB_WriteInt (&sb, dleafs[i].uiNumMarkSurfaces);
		for (j = 0; j < BSP_AMBIENT_END; j++)
			SB_WriteByte (&sb, dleafs[i].bAmbientLevel[j]);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_VERTEXES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numvertexes; i++)
	{
		SB_WriteFloat (&sb, dvertexes[i].fPoint[0]);
		SB_WriteFloat (&sb, dvertexes[i].fPoint[1]);
		SB_WriteFloat (&sb, dvertexes[i].fPoint[2]);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_NODES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numnodes; i++)
	{
		SB_WriteInt (&sb, dnodes[i].iPlaneNum);
		SB_WriteInt (&sb, dnodes[i].iChildren[0]);
		SB_WriteInt (&sb, dnodes[i].iChildren[1]);
		SB_WriteFloat (&sb, dnodes[i].fMins[0]);
		SB_WriteFloat (&sb, dnodes[i].fMins[1]);
		SB_WriteFloat (&sb, dnodes[i].fMins[2]);
		SB_WriteFloat (&sb, dnodes[i].fMaxs[0]);
		SB_WriteFloat (&sb, dnodes[i].fMaxs[1]);
		SB_WriteFloat (&sb, dnodes[i].fMaxs[2]);
		SB_WriteInt (&sb, dnodes[i].usFirstFace);
		SB_WriteInt (&sb, dnodes[i].usNumFaces);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_TEXINFO];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numtexinfo; i++)
	{
		SB_WriteFloat (&sb, texinfo[i].v[0][0]);
		SB_WriteFloat (&sb, texinfo[i].v[0][1]);
		SB_WriteFloat (&sb, texinfo[i].v[0][2]);
		SB_WriteFloat (&sb, texinfo[i].v[0][3]);
		SB_WriteFloat (&sb, texinfo[i].v[1][0]);
		SB_WriteFloat (&sb, texinfo[i].v[1][1]);
		SB_WriteFloat (&sb, texinfo[i].v[1][2]);
		SB_WriteFloat (&sb, texinfo[i].v[1][3]);
		SB_WriteInt (&sb, texinfo[i].iMipTex);
		SB_WriteInt (&sb, texinfo[i].iFlags);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_FACES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numfaces; i++)
	{
		SB_WriteInt (&sb, dfaces[i].iPlaneNum);
		SB_WriteInt (&sb, dfaces[i].iSide);
		SB_WriteInt (&sb, dfaces[i].iFirstEdge);
		SB_WriteInt (&sb, dfaces[i].iNumEdges);
		SB_WriteInt (&sb, dfaces[i].iTexInfo);
		for (j = 0; j < BSP_MAX_LIGHTMAPS; j++)
			SB_WriteByte (&sb, dfaces[i].bStyles[j]);
		SB_WriteInt (&sb, dfaces[i].iLightOffset);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_CLIPNODES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numclipnodes; i++)
	{
		SB_WriteInt (&sb, dclipnodes[i].iPlaneNum);
		SB_WriteInt (&sb, dclipnodes[i].iChildren[0]);
		SB_WriteInt (&sb, dclipnodes[i].iChildren[1]);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_MARKSURFACES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < nummarksurfaces; i++)
		SB_WriteInt (&sb, dmarksurfaces[i]);
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_SURFEDGES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numsurfedges; i++)
		SB_WriteInt (&sb, dsurfedges[i]);
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_EDGES];
	lump->iFileOffset = SB_Tell(&sb);
	for (i = 0; i < numedges; i++)
	{
		SB_WriteInt (&sb, dedges[i].v[0]);
		SB_WriteInt (&sb, dedges[i].v[1]);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_MODELS];
	lump->iFileOffset = SB_Tell (&sb);
	for (i = 0; i < nummodels; i++)
	{
		SB_WriteFloat (&sb, dmodels[i].fMins[0]);
		SB_WriteFloat (&sb, dmodels[i].fMins[1]);
		SB_WriteFloat (&sb, dmodels[i].fMins[2]);
		SB_WriteFloat (&sb, dmodels[i].fMaxs[0]);
		SB_WriteFloat (&sb, dmodels[i].fMaxs[1]);
		SB_WriteFloat (&sb, dmodels[i].fMaxs[2]);
		SB_WriteFloat (&sb, dmodels[i].fOrigin[0]);
		SB_WriteFloat (&sb, dmodels[i].fOrigin[1]);
		SB_WriteFloat (&sb, dmodels[i].fOrigin[2]);
		for (j = 0; j < hullinfo.filehulls; j++)
			SB_WriteInt (&sb, dmodels[i].iHeadNode[j]);
		SB_WriteInt (&sb, dmodels[i].iVisLeafs);
		SB_WriteInt (&sb, dmodels[i].iFirstFace);
		SB_WriteInt (&sb, dmodels[i].iNumFaces);
	}
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_LIGHTING];
	lump->iFileOffset = SB_Tell (&sb);
	SB_WriteData (&sb, drgblightdata, rgblightdatasize);
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_VISIBILITY];
	lump->iFileOffset = SB_Tell (&sb);
	SB_WriteData (&sb, dvisdata, visdatasize);
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_ENTITIES];
	lump->iFileOffset = SB_Tell (&sb);
	SB_WriteData (&sb, dentdata, entdatasize);
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	lump = &lumps[LUMP_TEXTURES];
	lump->iFileOffset = SB_Tell (&sb);
	SB_WriteData (&sb, dtexdata, texdatasize);
	lump->iFileLength = SB_Tell(&sb) - lump->iFileOffset;
	SB_ZeroFill (&sb, ((lump->iFileLength + 3) & ~3) - lump->iFileLength);

	// Go back and update the header.
	index = SB_Tell (&sb);
	SB_SeekAbsolute (&sb, 0);

	SB_WriteInt(&sb,BSP_HEADER);
	SB_WriteInt(&sb,BSP_VERSION);

	// always write full BSP2 lumps, this is for future-proofing
	for (i = 0; i < HEADER_LUMPS; i++)
	{
		SB_WriteInt(&sb,lumps[i].iFileOffset);
		SB_WriteInt(&sb,lumps[i].iFileLength);
	}

	SB_SeekAbsolute (&sb, index);

	// open a file and dump the buffer into it
	f = SafeOpenWrite (filename);
	SafeWrite (f, sb.start, (sb.index - sb.start));
	fclose (f);

	// finish up
	SB_Free (&sb);
}

//============================================================================

/*	Dumps info about current file.
*/
void PrintBSPFileSizes (void)
{
	printf ("%5i models       %6i\n", nummodels, (int)(nummodels*64));
	printf ("%5i planes       %6i\n", numplanes, (int)(numplanes*sizeof(BSPPlane_t)));
	printf ("%5i vertexes     %6i\n", numvertexes, (int)(numvertexes*sizeof(BSPVertex_t)));
	printf ("%5i nodes        %6i\n", numnodes, (int)(numnodes*sizeof(BSPNode_t)));
	printf ("%5i texinfo      %6i\n", numtexinfo, (int)(numtexinfo*sizeof(BSPTextureInfo_t)));
	printf ("%5i faces        %6i\n", numfaces, (int)(numfaces*sizeof(BSPFace_t)));
	printf ("%5i clipnodes    %6i\n", numclipnodes, (int)(numclipnodes*sizeof(BSPClipNode_t)));
	printf ("%5i leafs        %6i\n", numleafs, (int)(numleafs*sizeof(BSPLeaf_t)));
	printf ("%5i marksurfaces %6i\n", nummarksurfaces, (int)(nummarksurfaces*sizeof(dmarksurfaces[0])));
	printf ("%5i surfedges    %6i\n", numsurfedges, (int)(numsurfedges*sizeof(dmarksurfaces[0])));
	printf ("%5i edges        %6i\n", numedges, (int)(numedges*sizeof(BSPEdge_t)));
	if (!texdatasize)
		printf ("    0 textures          0\n");
	else
		printf ("%5i textures     %6i\n", ((dmiptexlump_t*)dtexdata)->nummiptex, texdatasize);
	printf ("      lightdata    %6i\n", lightdatasize);
	printf ("      visdata      %6i\n", visdatasize);
	printf ("      entdata      %6i\n", entdatasize);
}

//============================================================================

int CompressVis (byte *vis, byte *dest, int visrow)
{
	int		j;
	int		rep;
	byte	*dest_p;

	dest_p = dest;

	for (j=0 ; j<visrow ; j++)
	{
		*dest_p++ = vis[j];
		if (vis[j])
			continue;

		rep = 1;
		for (j++; j<visrow ; j++)
			if (vis[j] || rep == 255)
				break;
			else
				rep++;
		*dest_p++ = rep;
		j--;
	}

	return dest_p - dest;
}

void DecompressVis(byte *in, byte *out, int size)
{
	byte *end = out + size;
	int n;
	while (out < end)
	{
		n = *in++;
		if (n)
			*out++ = n;
		else
		{
			n = *in++;
			while (n--)
				*out++ = 0;
		}
	}
}

//============================================================================

int			num_entities = 0;
entity_t	entities[BSP_MAX_ENTITIES];

void PrintEntity (entity_t *ent)
{
	epair_t	*ep;

	for (ep=ent->epairs ; ep ; ep=ep->next)
		printf ("%20s : %s\n", ep->key, ep->value);
}


char *ValueForKey (entity_t *ent, char *key)
{
	epair_t	*ep;

	for (ep=ent->epairs ; ep ; ep=ep->next)
		if (!strcmp (ep->key, key) )
			return ep->value;
		return "";
}

vec_t FloatForKey (entity_t *ent, char *key)
{
	epair_t	*ep;

	for (ep=ent->epairs ; ep ; ep=ep->next)
		if (!strcmp (ep->key, key) )
			return atof( ep->value );

	return 0;
}

void SetKeyValue (entity_t *ent, char *key, char *value)
{
	epair_t	*ep;

	for (ep=ent->epairs ; ep ; ep=ep->next)
		if (!strcmp (ep->key, key) )
		{
			qfree (ep->value);
			ep->value = copystring(value);
			return;
		}
	ep = qmalloc (sizeof(*ep));
	ep->next = ent->epairs;
	ent->epairs = ep;
	ep->key = copystring(key);
	ep->value = copystring(value);
}

epair_t *ParseEpair (void)
{
	epair_t	*e;

	e = qmalloc (sizeof(epair_t));
	memset (e, 0, sizeof(epair_t));

	if (strlen(token) >= MAX_KEY-1)
		Error ("ParseEpair: token too long");
	e->key = copystring(token);
	GetToken (false);
	if (strlen(token) >= MAX_VALUE-1)
		Error ("ParseEpair: token too long");
	e->value = copystring(token);

	return e;
}

entity_t *FindEntityWithKeyPair( char *key, char *value )
{
	entity_t *ent;
	epair_t	*ep;
	int i;

	for (i=0 ; i<num_entities ; i++)
	{
		ent = &entities[ i ];
		for (ep=ent->epairs ; ep ; ep=ep->next)
		{
			if (!strcmp (ep->key, key) )
			{
				if ( !strcmp( ep->value, value ) )
					return ent;
				break;
			}
		}
	}
	return NULL;
}

void GetVectorForKey (entity_t *ent, char *key, vec3_t vec)
{
	char	*k;
	double	v1, v2, v3;

	k = ValueForKey (ent, key);
	v1 = v2 = v3 = 0;
	// scanf into doubles, then assign, so it is vec_t size independent
	sscanf (k, "%lf %lf %lf", &v1, &v2, &v3);
	vec[0] = v1;
	vec[1] = v2;
	vec[2] = v3;
}

void ParseEntities (void)
{
	char 		*data;
	entity_t	*entity;
	char		key[MAX_KEY];
	epair_t		*epair;

	data = dentdata;

	//
	// start parsing
	//
	num_entities = 0;

	// go through all the entities
	for(;;)
	{
		// parse the opening brace
		data = COM_Parse (data);
		if (!data)
			break;
		if (com_token[0] != '{')
			Error ("LoadEntities: found %s when expecting {", com_token);

		if (num_entities == BSP_MAX_ENTITIES)
			Error ("LoadEntities: MAX_MAP_ENTITIES");

		entity = &entities[num_entities++];

		// go through all the keys in this entity
		for(;;)
		{
			int		c;

			// parse key
			data = COM_Parse (data);
			if (!data)
				Error ("LoadEntities: EOF without closing brace");
			if (!strcmp(com_token,"}"))
				break;

			strcpy (key, com_token);

			// parse value
			data = COM_Parse (data);
			if (!data)
				Error ("LoadEntities: EOF without closing brace");
			c = com_token[0];
			if (c == '}')
				Error ("LoadEntities: closing brace without data");

			epair = malloc (sizeof(epair_t));
			epair->key = copystring( key );
			epair->value = copystring( com_token );
			epair->next = entity->epairs;
			entity->epairs = epair;
		}
	}
}

static void UnparseEntity( entity_t *ent, char *buf, char **end )
{
	epair_t		*ep;
	char		line[16384];

	ep = ent->epairs;
	if( !ep )
		return;

	strcat (*end,"{\n");
	*end += 2;

	for ( ; ep ; ep=ep->next)
	{
		sprintf (line, "\"%s\" \"%s\"\n", ep->key, ep->value);
		strcat (*end, line);
		*end += strlen(line);
	}
	strcat (*end,"}\n");
	*end += 2;

	if (*end > buf + BSP_MAX_ENTSTRING)
		Error ("Entity text too long");
}

void UnparseEntities (void)
{
	int			i;
	entity_t	*ent;
	char		*buf, *end;

	buf = dentdata;
	end = buf;
	*end = 0;

	// Vic: write "worldspawn" as the very first entity (engines might depend on it)
	ent = FindEntityWithKeyPair( "classname", "worldspawn" );
	if( ent )
		UnparseEntity( ent, buf, &end );

	for (i=0 ; i<num_entities ; i++) {
		if( &entities[i] != ent )
			UnparseEntity( &entities[i], buf, &end );
	}

	entdatasize = end - buf + 1;
}
