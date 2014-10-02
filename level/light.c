#include "light.h"

#include "shared_editor.h"

bool	lightvis,
		relight,
		verbose;

int			extrasamplesbit;	// power of 2 extra sampling (0 = 1x1 sampling, 1 = 2x2 sampling, 2 = 4x4 sampling, etc)
vec_t		extrasamplesscale;	// 1.0 / pointspersample (extrasamples related)

vec_t		globallightscale;
vec_t		globallightradiusscale;

int			harshshade;

lighttype_t	defaultlighttype;
int			overridelighttypes;

int			minlight;
int			ambientlight;

byte		currentvis[(BSP_MAX_LEAFS+7)/8];

int			c_occluded;

int			num_directlights;
directlight_t directlights[MAP_DIRECTLIGHTS];

/*
==============================================================================

ENTITY FILE PARSING

If a light has a targetname, generate a unique style in the 32-63 range
==============================================================================
*/

int		numlighttargets;
char	lighttargets[32][128];

vec3_t	vTextureReflectivity[BSP_MAX_TEXINFO];

// [22/7/2012] Taken from Quake 2's BSP tools ~hogsy
void Light_CalculateTextureReflectivity(void)
{
#if 0
	vec3_t		vColor;
	int			i,j,k,texels,texel;
	char		path[1024];
	float		r,scale;
	miptex_t	*mt;

	// Always set index 0 even if no textures
	VectorSet(vTextureReflectivity[0],0.5,0.5,0.5);

	for(i = 0; i < numtexinfo; i++)
	{
		for(j = 0; j < i; j++)
			if(!strcmp(miptex[i],miptex[j]))
			{
				VectorCopy(vTextureReflectivity[j],vTextureReflectivity[i]);
				break;
			}

		if(j != i)
			continue;

		// [1/8/2012] TODO: Load our texture ~hogsy

		texels = LittleLong(mt->width)*LittleLong(mt->height);

		VectorClear(vColor);

		// [1/8/2012] TODO: How to do for 32bpp images :/ ~hogsy
		for(j = 0; j < texels; j++)
		{
			mt->
			texel = ((byte*)mt)[LittleLong(mt->offsets[o])+j];
			for(k = 0; k < 3; k++)
				vColor[k] += palette
		}

		for(j = 0; j < 3; j++)
		{
			r = vColor[j]/texels/255.0f;
			vTextureReflectivity[i][j] = r;
		}

		// [1/8/2012] TODO: Add in our math library ~hogsy
		scale = Math_ColorNormalize(vTextureReflectivity[i],vTextureReflectivity[i]);
		if(scale < 0.5f)
		{
			scale *= 2.0f;
			VectorScale(vTextureReflectivity[i],scale,vTextureReflectivity[i]);
		}
	}
#endif
}

int LightStyleForTargetname( char *targetname )
{
	int		i;

	for( i = 0; i < numlighttargets; i++ )
		if( !strcmp( lighttargets[i], targetname ) )
			return 32 + i;

	if( numlighttargets == 32 )
		Error( "LightStyleForTargetname: numlighttargets == 32" );

	strcpy( lighttargets[numlighttargets], targetname );
	numlighttargets++;
	return numlighttargets - 1 + 32;
}

#ifndef __linux
#include <Windows.h>
#endif

void ParseLightEntities( void )
{
	int	i, j;
	entity_t *ent;
	char *value, *targetname, *style;
	directlight_t *l;
	double vec[4], color2[3];
	BOOL isLight;

	num_directlights = 0;
	for( i = 0, ent = entities; i < num_entities; i++, ent++ )
	{
		value = ValueForKey(ent,"classname");

        if(strncmp(value,"light",5))
			continue;

        if(!strcmp(value,"light"))
			isLight = true;
		else
			isLight = false;

		if( num_directlights == MAP_DIRECTLIGHTS )
			Error( "numdirectlights == MAP_DIRECTLIGHTS" );

		l = &directlights[num_directlights++];
		memset( l, 0, sizeof (*l) );

		color2[0] = color2[1] = color2[2] = 1.0f;
		l->color[0] = l->color[1] = l->color[2] = 1.0f;
		GetVectorForKey(ent,"origin",l->origin);

		l->type = defaultlighttype;
		j = FloatForKey(ent,"delay");
		if(!overridelighttypes && j)
		{
			if (j < 0 || j >= LIGHTTYPE_TOTAL)
				Error("error in light at %.0f %.0f %.0f:\nunknown light type \"delay\" \"%s\"\n", l->origin[0], l->origin[1], l->origin[2], ValueForKey( ent, "delay" ));
			l->type = (lighttype_t)j;
		}

		l->style = FloatForKey( ent, "style" );
		if( (unsigned)l->style > 254 )
			Error( "error in light at %.0f %.0f %.0f:\nBad light style %i (must be 0-254)", l->origin[0], l->origin[1], l->origin[2], l->style );

		l->angle = FloatForKey(ent,"angle");

		value = ValueForKey(ent,"color");
		if( !value[0] )
			value = ValueForKey(ent,"_color");
		if( value[0] )
			if( sscanf(value,"%lf %lf %lf",&color2[0],&color2[1],&color2[2]) != 3 )
				Error("error in light at %.0f %.0f %.0f:\ncolor must be given 3 values", l->origin[0], l->origin[1], l->origin[2] );

        value = ValueForKey(ent,"light");
		if( !value[0] )
			value = ValueForKey(ent,"_light");

		if( value[0] )
		{
			j = sscanf(value,"%lf %lf %lf %lf",&vec[0],&vec[1],&vec[2],&vec[3]);

			switch( j )
			{
				case 4:// HalfLife light
					l->color[0] = vec[0];
					l->color[1] = vec[1];
					l->color[2] = vec[2];
					l->radius	= vec[3];
					break;
				case 1: // quake light
					l->radius	= vec[0];
					l->color[0] = vec[0];
					l->color[1] = vec[0];
					l->color[2] = vec[0];
					break;
				default:
					Error( "error in light at %.0f %.0f %.0f:\n_light (or light) key must be 1 (Quake) or 4 (HalfLife) numbers, \"%s\" is not valid\n", l->origin[0], l->origin[1], l->origin[2], value );
			}
		}

		if( !l->radius )
		{
			l->radius	= DEFAULTLIGHTLEVEL;
			l->color[0] = DEFAULTLIGHTLEVEL;
			l->color[1] = DEFAULTLIGHTLEVEL;
			l->color[2] = DEFAULTLIGHTLEVEL;
		}

		// for some reason this * 0.5 is needed to match quake light
		VectorScale(l->color, 0.5, l->color);

		// fix tyrlite darklight radius value (but color remains negative)
		l->radius = fabs(l->radius);

		// apply scaling to radius
		vec[0] = FloatForKey( ent, "wait" );
		if( !vec[0] )
			vec[0] = 1;
		if (vec[0] != 1)
		{
			if (vec[0] <= 0)
			{
				l->type		= LIGHTTYPE_NONE;
				l->radius	= BOGUS_RANGE;
			}
			else
				l->radius /= vec[0];
        }

		l->radius *= globallightradiusscale;
		l->clampradius = l->radius;

		switch (l->type)
		{
		case LIGHTTYPE_RECIPX:
			l->clampradius = BOGUS_RANGE;
			break;
		case LIGHTTYPE_RECIPXX:
			l->clampradius = BOGUS_RANGE;
			break;
		default:
			break;
		}

		l->color[0] *= color2[0] * globallightscale;
		l->color[1] *= color2[1] * globallightscale;
		l->color[2] *= color2[2] * globallightscale;

		// this confines the light to a specified radius (typically used on RECIPX and RECIPXX)
		vec[0] = FloatForKey( ent, "_lightradius" );
		if (vec[0])
			l->clampradius = vec[0];

		if( isLight )
		{
			value = ValueForKey( ent, "targetname" );

			if( value[0] && !l->style )
			{
				char s[16];

				l->style = LightStyleForTargetname( value );

				memset( s, 0, sizeof(s) );
				sprintf( s, "%i", l->style );
				SetKeyValue( ent, "style", s );
			}
		}

		value = ValueForKey( ent, "target" );
		if( !value[0] )
		{
			if (l->type == LIGHTTYPE_SUN)
				Error("error in light at %.0f %.0f %.0f:\nLIGHTTYPE_SUN (delay 4) requires a target for the sun direction\n", l->origin[0], l->origin[1], l->origin[2]);
			continue;
		}

		for( j = 0; j < num_entities; j++ )
		{
			if( i == j )
				continue;

			targetname = ValueForKey(&entities[j],"targetname");
			if(!strcmp(targetname,value))
			{
				vec3_t origin;

				GetVectorForKey( &entities[j], "origin", origin );

				// set up spotlight values for lighting code to use
				VectorSubtract( origin, l->origin, l->spotdir );
				VectorNormalize( l->spotdir );

				if(!l->angle)
					l->spotcone = -cos(20.0f*Q_PI/180.0f);
				else
					l->spotcone = -cos(l->angle/2.0f*Q_PI/180.0f);
				break;
			}
		}

		if( j == num_entities )
		{
			printf( "warning in light at %.0f %.0f %.0f:\nunmatched spotlight target\n", l->origin[0], l->origin[1], l->origin[2]);
			if (l->type == LIGHTTYPE_SUN)
				Error("error in light at %.0f %.0f %.0f:\nLIGHTTYPE_SUN (delay 4) requires a target for the sun direction\n", l->origin[0], l->origin[1], l->origin[2]);
			continue;
		}

		// set the style on the source ent for switchable lights
		style = ValueForKey( &entities[j], "style" );
		if( style[0] && atof (style) )
		{
			char s[16];

			l->style = atof( style );
			if( (unsigned)l->style > 254 )
				Error( "error in light at %.0f %.0f %.0f:\nBad target light style %i (must be 0-254)", l->origin[0], l->origin[1], l->origin[2], l->style );

			memset( s, 0, sizeof(s) );
			sprintf( s, "%i", l->style );
			SetKeyValue( ent, "style", s );
		}
	}
}

#define LIGHTCHAINS (BSP_MAX_FACES * 64)

lightchain_t *surfacelightchain[BSP_MAX_FACES];
lightchain_t lightchainbuf[LIGHTCHAINS];
byte surfacehit[BSP_MAX_FACES];
const directlight_t *novislight[BSP_MAX_ENTITIES];
const directlight_t *alllight[BSP_MAX_ENTITIES];
int novislights, alllights;
int lightchainbufindex;

void LightWorld( void )
{
	int			i, k, n, m, count, pass, surfacesdone, lightvisibilitydone;
	unsigned int	*mark;
	time_t		lightstarttime, oldtime, newtime;
	directlight_t *light;
	BSPLeaf_t		*leaf;
	int			lightcount = 0, castcount = 0, emptycount = 0, solidcount = 0, watercount = 0, slimecount = 0, lavacount = 0, skycount = 0, misccount = 0, ignorevis;
	vec3_t		org;
	char		name[8];
	entity_t	*ent;

	lightdatasize = 0;
	rgblightdatasize = 0;
	lightstarttime = time( NULL );

	lightchainbufindex = 0;
	novislights = alllights = 0;
	memset( surfacelightchain, 0, sizeof( surfacelightchain ) );

	// LordHavoc: find the right leaf for each entity
	lightvisibilitydone = 0;
	oldtime = time( NULL );
	for( i = 0, light = directlights; i < num_directlights; i++, light++ )
	{
		lightcount++;
		alllight[alllights++] = light;
		leaf = Light_PointInLeaf(light->origin);
		ignorevis = false;

		switch( leaf->iContents )
		{
			case BSP_CONTENTS_EMPTY:
				emptycount++;
				break;
			case BSP_CONTENTS_SOLID:
				solidcount++;
				ignorevis = true;
				break;
			case BSP_CONTENTS_WATER:
				watercount++;
				break;
			case BSP_CONTENTS_SLIME:
				slimecount++;
				break;
			case BSP_CONTENTS_LAVA:
				lavacount++;
				break;
			case BSP_CONTENTS_SKY:
				skycount++;
				ignorevis = true;
				break;
			default:
				misccount++;
				break;
		}

		if( ignorevis )
			printf( "light at origin '%f %f %f' is in solid or sky, ignoring vis\n", light->origin[0], light->origin[1], light->origin[2] );

		if( leaf->iVisibilityOffset == -1 || ignorevis || !lightvis || light->type == LIGHTTYPE_SUN )
		{
			castcount += numfaces;
			novislight[novislights++] = light;
		}
		else
		{
			DecompressVis( dvisdata + leaf->iVisibilityOffset, currentvis, (dmodels[0].iVisLeafs + 7) >> 3 );
			memset( surfacehit, 0, numfaces );

			for( n = 0, leaf = dleafs + 1; n < numleafs; n++, leaf++ )
			{
				// leafs begin at 1
				if( !leaf->uiNumMarkSurfaces )
					continue;
				if( !(currentvis[n >> 3] & (1 << (n & 7))) )
					continue;

				if( (lightchainbufindex + leaf->uiNumMarkSurfaces) > LIGHTCHAINS )
					Error( "LightWorld: ran out of light chains!  complain to maintainer of hlight\n" );

				for( m = 0, mark = dmarksurfaces + leaf->uiFirstMarkSurface; m < leaf->uiNumMarkSurfaces; m++, mark++ )
				{
					if( surfacehit[*mark] )
						continue;
					surfacehit[*mark] = true;
					castcount++;
					lightchainbuf[lightchainbufindex].light = light;
					lightchainbuf[lightchainbufindex].next = surfacelightchain[*mark];
					surfacelightchain[*mark] = &lightchainbuf[lightchainbufindex++];
				}
			}
		}
		lightvisibilitydone++;
		newtime = time(NULL);
		if (newtime != oldtime)
		{
			printf("\rvisibility for light %5i of %5i (%3i%%), estimated time left: %5i ", lightvisibilitydone, num_directlights, (int) (lightvisibilitydone*100)/num_directlights, (int) (((num_directlights-lightvisibilitydone)*(newtime-lightstarttime))/lightvisibilitydone));
			fflush(stdout);
			oldtime = newtime;
		}
	}

	printf( "%4i lights, %4i air, %4i solid, %4i water, %4i slime, %4i lava, %4i sky, %4i unknown\n", lightcount, emptycount, solidcount, watercount, slimecount, lavacount, skycount, misccount );

	i = 0;
	for( m = 0; m < numfaces; m++ )
		if( surfacelightchain[m] )
			i++;

	printf( "%5i faces, %5i (%3i%%) may receive light\n", numfaces, i, i * 100 / numfaces );

	if( solidcount || skycount )
		printf( "warning: %4i lights of %4i lights (%3i%%) were found in sky or solid and will not be accelerated using vis, move them out of the solid or sky to accelerate compiling\n", solidcount+skycount, lightcount, (solidcount+skycount) * 100 / lightcount );

	printf( "%4i lights will be cast onto %5i surfaces, %10i casts will be performed\n", lightcount, numfaces, castcount );

	// LordHavoc: let there be light
	count = dmodels[0].iNumFaces;
	VectorClear(org);
	oldtime = time( NULL );

	c_occluded = 0;
	surfacesdone = 0;
	for (pass = 0;pass < 256;pass++)
		for (m = pass; m < count;m += 256)
		{
			LightFace(dfaces + m + dmodels[0].iFirstFace, surfacelightchain[m + dmodels[0].iFirstFace], novislight, novislights, org);
			surfacesdone++;
			newtime = time(NULL);
			if (newtime != oldtime)
			{
				printf("\rworld face %5i of %5i (%3i%%), estimated time left: %5i ", surfacesdone, count, (int) (surfacesdone*100)/count, (int) (((count-surfacesdone)*(newtime-lightstarttime))/surfacesdone));
				fflush(stdout);
				oldtime = newtime;
			}
		}

	printf( "\n%5i faces done\nlightdatasize: %i\n", surfacesdone, lightdatasize );
	printf( "c_occluded: %i\n", c_occluded );

	printf( "\nlighting %5i submodels:\n", nummodels );
	fflush( stdout );

	c_occluded = 0;
	// LordHavoc: light bmodels
	for( k = 1; k < nummodels; k++ )
	{
		newtime = time( NULL );
		if( newtime != oldtime )
		{
			m = k;
			count = nummodels;
			printf( "\rsubmodel %3i of %3i (%3i%%), estimated time left: %5i ", m, count, (int) (m*100)/count, (int) (((count-m)*(newtime-lightstarttime))/m) );
			fflush( stdout );
			oldtime = newtime;
		}

		sprintf( name, "*%d", k );
		ent = FindEntityWithKeyPair( "model", name );
		if( !ent )
			Error( "FindFaceOffsets: Couldn't find entity for model %s.\n", name );

		// LordHavoc: changed this to support origins on all submodels
		GetVectorForKey( ent, "origin", org );

		for( m = 0; m < dmodels[k].iNumFaces; m++ )
			LightFace( dfaces + m + dmodels[k].iFirstFace, NULL, alllight, alllights, org );
	}

	printf( "\n%5i submodels done\nlightdatasize: %i\n", nummodels, lightdatasize );
	printf( "c_occluded: %i\n", c_occluded );
}

int Light_Main( int argc, char **argv )
{
	double start, end;
	int i;

	extrasamplesbit         = 0;
	lightvis                = true;
	relight                 = false;
	globallightscale        = 1.0;
	globallightradiusscale  = 1.0;
	minlight                = 0;
	ambientlight            = 0;
	defaultlighttype        = LIGHTTYPE_MINUSX;
	overridelighttypes      = false;

	for( i = 1; i < argc; i++ )
	{
		if( !strcmp( argv[i],"-extra" ) )
		{
			i++;
			if(i >= argc)
				Error("No value given to -extra!");

			extrasamplesbit = atoi(argv[i]);
			printf("Antialiased lighting enabled (mode %i)\n",extrasamplesbit);
		}
		else if( !strcmp( argv[i],"-nolightvis" ) )
		{
			printf( "use of vis data to optimize lighting disabled\n" );
			lightvis = false;
		}
		else if( !strcmp( argv[i],"-intensity" ) )
		{
			i++;
			if( i >= argc )
				Error( "no value was given to -intensity\n" );
			globallightscale = atof( argv[i] );
			if( globallightscale < 0.01 )
				globallightscale = 0.01;
		}
		else if( !strcmp( argv[i],"-radiusscale" ) )
		{
			i++;
			if( i >= argc )
				Error( "no value was given to -radiusscale\n" );
			globallightradiusscale = atof( argv[i] );
			if( globallightradiusscale < 0.01 )
				globallightradiusscale = 0.01;
		}
		else if( !strcmp( argv[i],"-minlight" ) )
		{
			i++;
			if( i >= argc )
				Error( "no value was given to -minlight\n" );
			minlight = atof( argv[i] );
			if( minlight < 0 )
				minlight = 0;
		}
		else if( !strcmp( argv[i], "-ambientlight" ) )
		{
			i++;
			if( i >= argc )
				Error( "no value was given to -ambientlight\n" );
			ambientlight = atof( argv[i] );
			if( ambientlight < 0 )
				ambientlight = 0;
		}
		else if( !strcmp( argv[i],"-defaulttype" ) )
		{
			i++;
			if( i >= argc )
				Error( "no value was given to -defaulttype\n" );
			defaultlighttype = atoi( argv[i] );
			if( defaultlighttype < 0 || defaultlighttype >= LIGHTTYPE_TOTAL )
				Error( "invalid value given to -defaulttype\n" );
			printf( "defaulting all lights to type %i\n", defaultlighttype);
		}
		else if( !strcmp( argv[i],"-overridetypes" ) )
		{
			printf( "overriding all light types with current default (%i)\n", defaultlighttype );
			overridelighttypes = true;
		}
		else if( !strcmp( argv[i],"-harshshade" ) )
		{
			harshshade = 1;
			printf( "harsh shading enabled\n" );
		}
		else if( argv[i][0] == '-' )
			Error( "Unknown option \"%s\"", argv[i] );
		else
			break;
	}

	extrasamplesscale = 1.0f / (1 << (extrasamplesbit * 2));

	if( i != argc - 1 )
		Error( "%s",
"usage: hmap2 -light [options] bspfile\n"
"Compiles lighting data in a .bsp and also makes .lit colored lighting data\n"
"\n"
"Quick usage notes for entities: (place these in key/value pairs)\n"
"wait - falloff rate: 1.0 default, 0.5 = double radius, 2 = half radius\n"
"_color - red green blue, specifies color of light, example 1 0.6 0.3 is orange\n"
"_lightradius - forces light to be this radius (useful with 1/ types)\n"
"delay - light type: (x = distance of surface from light, r = radius)\n"
"0: 1-(x/r)    fast, quake lighting, the default, tyrlite compatible\n"
"1: 1/(x)      slow, tyrlite compatible\n"
"2: 1/(x*x)    slow, realistic, tyrlite compatible\n"
"3: 1          fast, no fade, useful for sky lights, tyrlite compatible\n"
"4: sun        slow, directional sunlight, uses target direction like spotlights\n"
"5: 1-x/r*x/r  fast, looks like darkplaces/tenebrae lights\n"
"\n"
"What the options do:\n"
"-extra <number> antialiased lighting, takes much longer, higher quality\n"
"-nolightvis   disables use of visibility data to optimize lighting\n"
"-harshshade   harsh shading rather than the normal soft shading\n"
"Options from here on are incompatible with darkplaces realtime lighting mode\n"
"(it does not know if these options are used, and will require manual rtlights\n"
" editing to look good)\n"
"-intensity    scales brightness of all lights\n"
"-radiusscale  scales size of all lights (including 1/ types)\n"
"-defaulttype <number> sets default light type by number, see delay above\n"
"-overridetypes forces all lights to use the -defaulttype setting\n"
"-minlight     raises darkest areas of the map to this light level (0-255)\n"
"-ambientlight raises all of the map by this light level (0-255)\n"
		);

	// init memory
	Q_InitMem ();

	printf( "----- LightFaces ----\n" );

//	InitThreads ();

	start = I_DoubleTime ();

	LoadBSPFile( filename_bsp );

	memset(dlightdata,0,sizeof(dlightdata));
	memset(drgblightdata,0,sizeof(drgblightdata));

	if( !visdatasize )
	{
		printf( "no visibility data found (run -vis before -light to compile faster)\n" );
		lightvis = false;
	}

	ParseEntities ();
	printf( "%i entities read\n", num_entities );

	ParseLightEntities ();

	LightWorld ();

	UnparseEntities ();

	WriteBSPFile(filename_bsp);

	end = I_DoubleTime ();
	printf( "%5.2f seconds elapsed\n\n", end - start );

	Q_PrintMem ();
	Q_ShutdownMem ();

	return 0;
}
