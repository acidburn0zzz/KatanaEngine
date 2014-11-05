#include "bsp5.h"

static node_t *Bsp2Prt_BuildTree_r( int nodenum )
{
	node_t		*n;

	if( nodenum < 0 ) 
	{
		BSPLeaf_t *leaf = &dleafs[-1-nodenum];

		n = AllocNode();
		n->planenum = PLANENUM_LEAF;
		n->contents = leaf->iContents;
	} 
	else 
	{
		int		side;
		plane_t plane;
		BSPNode_t *node = &dnodes[nodenum];

		n = AllocNode ();
		plane.dist = dplanes[node->iPlaneNum].fDist;
		VectorCopy( dplanes[node->iPlaneNum].fNormal, plane.normal );
		n->planenum = FindPlane( &plane, &side );

		if( side )
			Error( "Bad node plane" );

		n->children[0] = Bsp2Prt_BuildTree_r( node->iChildren[0] );
		n->children[1] = Bsp2Prt_BuildTree_r( node->iChildren[1] );
	}

	return n;
}

static void Bsp2Prt_GetWorldBounds( vec3_t mins, vec3_t maxs )
{
	int				j, e;
	unsigned int	i;
	BSPFace_t		*face;
	BSPVertex_t		*v;
	vec3_t			point;

	ClearBounds( mins, maxs );

	for( i = 0, face = dfaces; i < dmodels[0].iNumFaces; i++, face++ ) 
	{
		for( j = 0; j < face->iNumEdges; j++ ) 
		{
			e = dsurfedges[face->iFirstEdge + j];
			if( e >= 0 )
				v = dvertexes + dedges[e].v[0];
			else
				v = dvertexes + dedges[-e].v[1];

			VectorCopy(v->fPoint,point);
			AddPointToBounds(point,mins,maxs);
		}
	}
}

static void Bsp2Prt_ProcessFile(void)
{
	tree_t		*tree;

	LoadBSPFile( filename_bsp );

	tree = AllocTree ();

	Bsp2Prt_GetWorldBounds( tree->mins, tree->maxs );

	tree->headnode = Bsp2Prt_BuildTree_r( 0 );

	PortalizeTree( tree );
	WritePortalfile( tree );
	FreeTreePortals( tree );

	FreeTree( tree );
}

int Bsp2Prt_Main( int argc, char **argv )
{
	int			i;
	double		start, end;

	transwater = true;

	for( i = 1; i < argc; i++ ) {
		if (!strcmp (argv[i],"-nowater"))
			transwater = false;
		else if( argv[i][0] == '-' )
			Error( "Unknown option \"%s\"", argv[i] );
		else
			break;
	}

	if( i != argc - 1 )
		Error( "%s",
"usage: hmap2 -bsp2prt [options] sourcefile\n"
"Makes a .prt file from a "BSP_EXTENSION", to allow it to be vised\n"
"\n"
"What the options do:\n"
"-nowater    disable watervis; r_wateralpha in glquake will not work right\n"
		);

	// init memory
	Q_InitMem ();

	// do it!
	start = I_DoubleTime ();

	end = I_DoubleTime ();
	Bsp2Prt_ProcessFile ();
	printf( "%5.2f seconds elapsed\n\n", end - start );

	// print memory stats
	Q_PrintMem ();

	// free allocated memory
	Q_ShutdownMem ();

	return 0;
}
