
#include "bsp5.h"

int		firstface,
		*planemapping;

//===========================================================================

/*	Used to find plane index numbers for clip nodes read from child processes.
*/
int FindFinalPlane( vec3_t normal, vec_t dist, int type )
{
	int			i;
	BSPPlane_t	*dplane;

	for( i = 0, dplane = dplanes; i < numplanes; i++, dplane++ ) {
		if( dplane->iType != type
			|| dplane->fDist != dist
			|| dplane->fNormal[0] != normal[0]
			|| dplane->fNormal[1] != normal[1]
			|| dplane->fNormal[2] != normal[2])
			continue;
		return i;
	}

	// new plane
	if( numplanes == BSP_MAX_PLANES )
		Error( "numplanes == BSP_MAX_PLANES" );

	dplane->iType = type;
	dplane->fDist = dist;
	VectorCopy( normal, dplane->fNormal );

	return numplanes++;
}

static void EmitNodePlanes_r( node_t *node )
{
	if( node->planenum == PLANENUM_LEAF )
		return;

	if( planemapping[node->planenum] == -1 ) {	// a new plane
		plane_t *plane;

		plane = &mapplanes[node->planenum];
		planemapping[node->planenum] = FindFinalPlane( plane->normal, plane->dist, plane->type );
	}

	node->outputplanenum = planemapping[node->planenum];

	EmitNodePlanes_r( node->children[0] );
	EmitNodePlanes_r( node->children[1] );
}

void EmitNodePlanes( node_t *nodes )
{
	EmitNodePlanes_r( nodes );
}

static int EmitClipNodes_r( node_t *node )
{
	int				i, c;
	BSPClipNode_t	*cn;
	int				num;

	// FIXME: free more stuff?
	if( node->planenum == PLANENUM_LEAF ) 
	{
		num = node->contents;
		FreeNode( node );
		return num;
	}

	c = numclipnodes;
	cn = &dclipnodes[numclipnodes++];	// emit a clipnode
	cn->iPlaneNum = node->outputplanenum;
	for( i = 0; i < 2; i++ )
		cn->iChildren[i] = EmitClipNodes_r( node->children[i] );

	FreeNode( node );

	return c;
}

/*
==================
EmitClipNodes

Called after the clipping hull is completed. Generates a disk format
representation and frees the original memory.
==================
*/
void EmitClipNodes( node_t *nodes, int modnum, int hullnum )
{
//	qprintf( "--- EmitClipNodes ---\n" );

	dmodels[modnum].iHeadNode[hullnum] = numclipnodes;
	EmitClipNodes_r( nodes );
}

void EmitVertex( vec3_t point )
{
	BSPVertex_t	*vert;

	if( numvertexes == BSP_MAX_VERTS )
		Error( "numvertexes == BSP_MAX_VERTS" );

	vert = &dvertexes[numvertexes++];	// emit a vertex

	VectorCopy(point,vert->fPoint);
}

void EmitEdge( int v1, int v2 )
{
	BSPEdge_t	*edge;

	if( numedges == BSP_MAX_EDGES )
		Error( "numedges == BSP_MAX_EDGES" );

	edge = &dedges[numedges++];	// emit an edge
	edge->v[0] = v1;
	edge->v[1] = v2;
}

static void EmitLeaf( node_t *node )
{
	face_t		**fp, *f;
	BSPLeaf_t	*leaf_p;

	if( numleafs == BSP_MAX_LEAFS )
		Error( "numleafs == BSP_MAX_LEAFS" );

	leaf_p = &dleafs[numleafs++];	// emit a leaf
	leaf_p->iContents = node->contents;
	leaf_p->iVisibilityOffset = -1;			// no vis info yet

	// write bounding box info
	VectorCopy( node->mins, leaf_p->fMins );
	VectorCopy( node->maxs, leaf_p->fMaxs );

	// write the marksurfaces
	leaf_p->uiFirstMarkSurface = nummarksurfaces;

	for( fp = node->markfaces; *fp ; fp++ ) 
	{
		for( f = *fp; f; f = f->original ) 
		{	
			// grab tjunction split faces
			if( nummarksurfaces == BSP_MAX_MARKSURFACES )
				Error( "nummarksurfaces == BSP_MAX_MARKSURFACES" );
			if( f->outputnumber == -1 )
				Error( "f->outputnumber == -1" );
			dmarksurfaces[nummarksurfaces++] = f->outputnumber;		// emit a marksurface
		}
	}

	leaf_p->uiNumMarkSurfaces = nummarksurfaces - leaf_p->uiFirstMarkSurface;
}

void EmitDrawNodes_r( node_t *node )
{
	int			i;
	BSPNode_t	*n;

	if( numnodes == BSP_MAX_NODES )
		Error( "numnodes == BSP_MAX_NODES" );

	n = &dnodes[numnodes++];		// emit a node
	n->iPlaneNum	= node->outputplanenum;
	n->usFirstFace	= node->firstface;
	n->usNumFaces	= node->numfaces;
	VectorCopy( node->mins, n->fMins );
	VectorCopy( node->maxs, n->fMaxs );

	// recursively output the other nodes
	for( i = 0; i < 2; i++ ) 
	{
		if( node->children[i]->planenum == PLANENUM_LEAF ) 
		{
			n->iChildren[i] = -1;

			if( node->children[i]->contents != BSP_CONTENTS_SOLID ) {
				n->iChildren[i] = -(numleafs + 1);
				EmitLeaf( node->children[i] );
			}
		} 
		else 
		{
			n->iChildren[i] = numnodes;
			EmitDrawNodes_r( node->children[i] );
		}
	}
}

void EmitDrawNodes( node_t *headnode )
{
	int			i;
	BSPModel_t	*bm;
	int			firstleaf;

//	qprintf( "--- EmitDrawNodes ---\n" );

	if( nummodels == BSP_MAX_MODELS )
		Error( "nummodels == BSP_MAX_MODELS" );

	bm = &dmodels[nummodels++];		// emit a model
	bm->iHeadNode[0] = numnodes;
	bm->iFirstFace = firstface;
	bm->iNumFaces = numfaces - firstface;
	firstface = numfaces;

	firstleaf = numleafs;
	if( headnode->contents < 0 )
		EmitLeaf( headnode );
	else
		EmitDrawNodes_r( headnode );
	bm->iVisLeafs = numleafs - firstleaf;

	for( i = 0; i < 3; i++ ) {
		bm->fMins[i] = headnode->mins[i] + SIDESPACE + 1;	// remove the padding
		bm->fMaxs[i] = headnode->maxs[i] - SIDESPACE - 1;
	}
}

void BeginBSPFile( void )
{
	qprintf( "--- BeginBSPFile ---\n" );

	planemapping = (int*)qmalloc( sizeof( *planemapping ) * BSP_MAX_PLANES );
	memset( planemapping, -1, sizeof( *planemapping ) * BSP_MAX_PLANES );

	// edge 0 is not used, because 0 can't be negated
	numedges = 1;

	// leaf 0 is common solid with no faces
	numleafs = 1;
	dleafs[0].iContents = BSP_CONTENTS_SOLID;
	dleafs[0].iVisibilityOffset = -1; // thanks to Vic for suggesting this line

	firstface = 0;
}

void FinishBSPFile( void )
{
	printf( "--- BSP file information ---\n" );
	PrintBSPFileSizes ();

	qprintf( "--- WriteBSPFile: %s ---\n", filename_bsp );
	WriteBSPFile(filename_bsp);

	qfree( planemapping );
}
