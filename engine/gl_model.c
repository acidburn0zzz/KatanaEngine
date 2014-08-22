/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2014 OldTimes Software

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "quakedef.h"

/*
	Model Loading and Caching
	Models are the only shared resource between a client and server running on the same machine.
*/

#include "engine_console.h"
#include "engine_script.h"

model_t	*loadmodel;
char	loadname[32];	// for hunk tags

void Model_LoadBSP(model_t *mod, void *buffer);
void Model_LoadMD2(model_t *mod,void *buffer);
bool Model_LoadOBJ(model_t *mModel,void *Buffer);

model_t *Model_Load(model_t *mod);

byte mod_novis[BSP_MAX_LEAFS/8];

#define	MAX_MOD_KNOWN	2048 //johnfitz -- was 512
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

texture_t	*r_notexture_mip; //johnfitz -- moved here from r_main.c
texture_t	*r_notexture_mip2; //johnfitz -- used for non-lightmapped surfs with a missing texture

void Model_Initialize(void)
{
	memset(mod_novis,0xff,sizeof(mod_novis));

	//johnfitz -- create notexture miptex
	r_notexture_mip = (texture_t*)Hunk_AllocName(sizeof(texture_t),"r_notexture_mip");
	strcpy (r_notexture_mip->name, "notexture");
	r_notexture_mip->height = r_notexture_mip->width = 32;

	r_notexture_mip2 = (texture_t*)Hunk_AllocName (sizeof(texture_t), "r_notexture_mip2");
	strcpy (r_notexture_mip2->name, "notexture2");
	r_notexture_mip2->height = r_notexture_mip2->width = 32;
	//johnfitz
}

/*	Caches the data if needed
*/
void *Mod_Extradata (model_t *mod)
{
	void *r;

	r = Cache_Check(&mod->cache);
	if(r)
		return r;

	Model_Load(mod);
	if(!mod->cache.data)
		Sys_Error ("Mod_Extradata: caching failed");

	return mod->cache.data;
}

mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t		*node;
	float		d;
	mplane_t	*plane;

	if (!model || !model->nodes)
		Sys_Error ("Mod_PointInLeaf: bad model");

	node = model->nodes;
	for(;;)
	{
		if(node->contents < 0)
			// [5/3/2013] Break here so we DO reach the end and don't create dumb warnings. Seriously. ~hogsy
			break;

		plane = node->plane;
		d = Math_DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return (mleaf_t*)node;
}

byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[BSP_MAX_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->numleafs+7)>>3;
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);

	return decompressed;
}

byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if(leaf == model->leafs)
		return mod_novis;

	return Mod_DecompressVis (leaf->compressed_vis, model);
}

void Model_ClearAll(void)
{
	int		i;
	model_t	*mModel;

	for(i = 0,mModel = mod_known; i < mod_numknown; i++,mModel++)
		if(mModel->mType != MODEL_TYPE_MD2)
		{
			mModel->bNeedLoad = true;

			TexMgr_FreeTexturesForOwner(mModel); //johnfitz
		}
}

model_t *Model_FindName(char *cName)
{
	int		i;
	model_t	*mModel;

	if(!cName[0])
		Sys_Error ("Model_FindName: NULL name");

	// Search the currently loaded models
	for(i = 0,mModel = mod_known; i < mod_numknown; i++,mModel++)
		if(!strcmp(mModel->name,cName))
			break;

	if(i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			Sys_Error ("mod_numknown == MAX_MOD_KNOWN");

		strcpy(mModel->name,cName);

		mModel->bNeedLoad = true;

		mod_numknown++;
	}

	return mModel;
}

void Model_Touch(char *cName)
{
	model_t	*mModel;

	mModel = Model_FindName(cName);
	if(!mModel->bNeedLoad)
		if(mModel->mType == MODEL_TYPE_MD2)
			Cache_Check(&mModel->cache);
}

/*	Loads a model into the cache
*/
model_t *Model_Load(model_t *mModel)
{
	void		*d;
	unsigned	*buf;
	byte		stackbuf[1024];		// avoid dirtying the cache heap

	if(!mModel->bNeedLoad)
	{
		if(mModel->mType == MODEL_TYPE_MD2)
		{
			d = Cache_Check (&mModel->cache);
			if (d)
				return mModel;
		}
		else
			return mModel;
	}

	// Load the file
	buf = (unsigned*)COM_LoadStackFile(mModel->name,stackbuf,sizeof(stackbuf));
	if(!buf)
	{
		Con_Warning("Mod_LoadModel: %s not found\n",mModel->name);
		return NULL;
	}

	// Allocate a new model
	COM_FileBase(mModel->name, loadname);

	loadmodel = mModel;

	// Call the apropriate loader
	mModel->bNeedLoad = false;

	switch(LittleLong(*(unsigned*)buf))
	{
	case KMDL_HEADER:
	case MD2_HEADER:
		Model_LoadMD2(mModel,buf);
		break;
	case BSP_HEADER:
		Model_LoadBSP(mModel,buf);
		break;
	default:
		if(Model_LoadOBJ(mModel,buf))
			break;

		Con_Warning("Unsupported model type! (%s)\n",mModel->name);
		return NULL;
	}

	return mModel;
}

/*	Loads in a model for the given name
*/
model_t *Mod_ForName(char *cName)
{
	return Model_Load(Model_FindName(cName));
}

/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

byte	*mod_base;

void Model_LoadBSPTextures(BSPLump_t *blLump)
{
	int				i,j,num,max,altmax,
					nummiptex,mark;
	unsigned int	fwidth,fheight;
	miptex_t		*mt;
	texture_t		*tx,*tx2,
					*anims[10],*altanims[10];
	dmiptexlump_t	*m;
	char			texturename[64],
					filename[MAX_OSPATH],filename2[MAX_OSPATH],
					mapname[MAX_OSPATH];
	byte			*data;
	extern byte		*hunk_base;

	//johnfitz -- don't return early if no textures; still need to create dummy texture
	if(!blLump->iFileLength)
	{
		Con_Printf("No textures in BSP file...\n");
		nummiptex = 0;

		// [18/8/2012] Keep the compiler happy :S ~hogsy
		m = NULL;
	}
	else
	{
		m = (dmiptexlump_t *)(mod_base+blLump->iFileOffset);
		m->nummiptex = LittleLong (m->nummiptex);
		nummiptex = m->nummiptex;
	}
	//johnfitz

	loadmodel->numtextures	= nummiptex+2; //johnfitz -- need 2 dummy texture chains for missing textures
	loadmodel->textures		= (texture_t**)Hunk_AllocName(loadmodel->numtextures*sizeof(*loadmodel->textures),loadname);

	for(i = 0; i < nummiptex; i++)
	{
		m->dataofs[i] = LittleLong(m->dataofs[i]);
		if (m->dataofs[i] == -1)
			continue;

		mt = (miptex_t *)((byte*)m + m->dataofs[i]);
		mt->width	= LittleLong(mt->width);
		mt->height	= LittleLong(mt->height);
		if((mt->width & 15) || (mt->height & 15))
		{
			Console_ErrorMessage(false,mt->name,va("Texture is not 16 aligned (%ix%i)!",mt->width,mt->height));
			return;
		}

		tx = (texture_t*)Hunk_AllocName(sizeof(texture_t),loadname);
		loadmodel->textures[i] = tx;

		memcpy (tx->name, mt->name, sizeof(tx->name));
		tx->width	= mt->width;
		tx->height	= mt->height;

		// the pixels immediately follow the structures
		memcpy(tx+1,mt+1,sizeof(miptex_t));

		tx->update_warp	= false; //johnfitz
		tx->warpimage	= NULL; //johnfitz
		tx->fullbright	= NULL; //johnfitz

		//johnfitz -- lots of changes
		if(!bIsDedicated) //no texture uploading for dedicated server
		{
			if(tx->name[0] == '*' || tx->name[0] == '#') //warping texture
			{
				//external textures -- first look in "textures/mapname/" then look in "textures/"
				mark = Hunk_LowMark();
				COM_StripExtension(loadmodel->name+5,mapname);
				sprintf (filename, "textures/%s/#%s", mapname, tx->name+1); //this also replaces the '*' with a '#'

				data = Image_LoadImage(filename,&fwidth,&fheight);
				if(!data)
				{
					sprintf (filename, "textures/#%s", tx->name+1);
					data = Image_LoadImage (filename, &fwidth, &fheight);
					if(!data)
						Con_Warning("Failed to load %s\n",filename);
				}

				// Now load whatever we found
				if(data)
				{
					strcpy(texturename,filename);

					// [11/7/2012] No point allowing alpha for water textures ~hogsy
					tx->gltexture = TexMgr_LoadImage(loadmodel,texturename,fwidth,fheight,SRC_RGBA,data,filename,0,TEXPREF_MIPMAP);
				}

				// Now create the warpimage, using dummy data from the hunk to create the initial image
				Hunk_Alloc(gl_warpimagesize*gl_warpimagesize*4); //make sure hunk is big enough so we don't reach an illegal address
				Hunk_FreeToLowMark(mark);
				sprintf(texturename,"%s_warp",texturename);
				tx->warpimage = TexMgr_LoadImage(loadmodel,texturename,gl_warpimagesize,
					gl_warpimagesize, SRC_RGBA, hunk_base, "", (unsigned)hunk_base,TEXPREF_NOPICMIP | TEXPREF_WARPIMAGE);
				tx->update_warp = true;
			}
			else //regular texture
			{
				//external textures -- first look in "textures/mapname/" then look in "textures/"
				mark = Hunk_LowMark();

				COM_StripExtension(loadmodel->name+5,mapname);

				sprintf(filename,"textures/%s/%s",mapname,tx->name);

				data = Image_LoadImage(filename,&fwidth,&fheight);
				if(!data)
				{
					sprintf(filename,"textures/%s",tx->name);
					data = Image_LoadImage(filename,&fwidth,&fheight);
					if(!data)
						Con_Warning("Failed to load %s\n",filename);
				}

				//now load whatever we found
				if (data) //load external image
				{
					tx->gltexture = TexMgr_LoadImage(
						loadmodel,
						filename,
						fwidth,fheight,
						SRC_RGBA,
						data,
						filename,
						0,
						TEXPREF_MIPMAP|TEXPREF_ALPHA);

					//now try to load glow image from the same place
					Hunk_FreeToLowMark (mark);
					// [1/3/2014] Updated to _fbr, from _glow, to match usage for models; consistent and less confusing. ~hogsy
					sprintf (filename2, "%s_fbr", filename);
					data = Image_LoadImage(filename2,&fwidth,&fheight);
					if(data)
						tx->fullbright = TexMgr_LoadImage(
							loadmodel,
							filename2,
							fwidth,fheight,
							SRC_RGBA,
							data,
							filename,
							0,
							TEXPREF_MIPMAP|TEXPREF_ALPHA);
				}

				Hunk_FreeToLowMark (mark);
			}
		}
		//johnfitz
	}

	//johnfitz -- last 2 slots in array should be filled with dummy textures
	loadmodel->textures[loadmodel->numtextures-2] = r_notexture_mip; //for lightmapped surfs
	loadmodel->textures[loadmodel->numtextures-1] = r_notexture_mip2; //for SURF_DRAWTILED surfs

	// sequence the animations
	for (i=0 ; i<nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;

		// already sequenced
		if (tx->anim_next)
			continue;

		// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
			Sys_Error("Bad animating texture %s", tx->name);

		for (j=i+1 ; j<nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp (tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
				Sys_Error ("Bad animating texture %s", tx->name);
		}

#define	ANIM_CYCLE	2
		// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			if (!tx2)
			{
				Con_Warning("Missing frame %i of %s",j, tx->name);
				return;
			}
			tx2->anim_total = max * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[ (j+1)%max ];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
			{
				Con_Warning("Missing frame %i of %s",j, tx->name);
				return;
			}

			tx2->anim_total = altmax*ANIM_CYCLE;
			tx2->anim_min	= j*ANIM_CYCLE;
			tx2->anim_max	= (j+1)*ANIM_CYCLE;
			tx2->anim_next	= altanims[ (j+1)%altmax ];
			if(max)
				tx2->alternate_anims = anims[0];
		}
	}
}

void Model_LoadBSPLighting(BSPLump_t *blLump)
{
	if(!blLump->iFileLength)
		return;

	loadmodel->lightdata = (byte*)Hunk_Alloc(blLump->iFileLength);

	memcpy(loadmodel->lightdata,mod_base+blLump->iFileOffset,blLump->iFileLength);
}

void Model_LoadBSPVisibility(BSPLump_t *blLump)
{
	if(!blLump->iFileLength)
	{
		loadmodel->visdata = NULL;
		return;
	}

	loadmodel->visdata = (byte*)Hunk_AllocName ( blLump->iFileLength, loadname);
	memcpy(loadmodel->visdata, mod_base + blLump->iFileOffset, blLump->iFileLength);
}

void Model_LoadBSPEntities(BSPLump_t *blLump)
{
	if(!blLump->iFileLength)
	{
		loadmodel->entities = NULL;
		return;
	}

	loadmodel->entities = (char*)Hunk_AllocName(blLump->iFileLength,loadname);

	memcpy(loadmodel->entities,mod_base+blLump->iFileOffset,blLump->iFileLength);
}

void Model_LoadBSPVertexes(BSPLump_t *blLump)
{
	BSPVertex_t	*in;
	BSPVertex_t	*out;
	int			i, count;

	in = (void *)(mod_base+blLump->iFileOffset);
	if(blLump->iFileLength%sizeof(*in))
		Sys_Error ("Model_LoadBSPVertexes: funny lump size in %s",loadmodel->name);

	count	= blLump->iFileLength/sizeof(*in);
	out		= (BSPVertex_t*)Hunk_AllocName(count*sizeof(*out),loadname);

	loadmodel->vertexes		= out;
	loadmodel->numvertexes	= count;

	for(i = 0; i < count; i++,in++,out++)
	{
		out->fPoint[0]	= LittleFloat(in->fPoint[0]);
		out->fPoint[1]	= LittleFloat(in->fPoint[1]);
		out->fPoint[2]	= LittleFloat(in->fPoint[2]);
	}
}

void Model_LoadBSPEdges(BSPLump_t *blLump)
{
	BSPEdge_t	*beEdge;
	medge_t		*out;
	int 		i,count;

	beEdge = (void*)(mod_base+blLump->iFileOffset);
	if(blLump->iFileLength%sizeof(*beEdge))
		Sys_Error ("Model_LoadBSPEdges: funny lump size in %s",loadmodel->name);

	count	= blLump->iFileLength/sizeof(*beEdge);
	out		= (medge_t*)Hunk_AllocName((count+1)*sizeof(*out),loadname);

	loadmodel->edges	= out;
	loadmodel->numedges = count;

	for(i = 0; i < count; i++,beEdge++,out++)
	{
		out->v[0] = (unsigned short)LittleShort(beEdge->v[0]);
		out->v[1] = (unsigned short)LittleShort(beEdge->v[1]);
	}
}

void Model_LoadBSPTextureInfo(BSPLump_t *blLump)
{
	BSPTextureInfo_t	*in;
	mtexinfo_t			*out;
	int 				i, j, count, miptex;
	float				len1, len2;
	int					missing = 0; //johnfitz

	in = (void *)(mod_base + blLump->iFileOffset);
	if (blLump->iFileLength % sizeof(*in))
		Sys_Error ("Model_LoadBSPTextureInfo: funny lump size in %s",loadmodel->name);
	count = blLump->iFileLength / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->texinfo		= out;
	loadmodel->numtexinfo	= count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
			out->vecs[0][j] = LittleFloat (in->v[0][j]);

		len1 = Math_Length(out->vecs[0]);
		len2 = Math_Length(out->vecs[1]);
		len1 = (len1 + len2)/2;
		if (len1 < 0.32)
			out->mipadjust = 4;
		else if (len1 < 0.49)
			out->mipadjust = 3;
		else if (len1 < 0.99)
			out->mipadjust = 2;
		else
			out->mipadjust = 1;
#if 0
		if (len1 + len2 < 0.001)
			out->mipadjust = 1;		// don't crash
		else
			out->mipadjust = 1 / floor( (len1+len2)/2 + 0.1 );
#endif

		miptex = LittleLong (in->iMipTex);
		out->flags = LittleLong (in->iFlags);

		//johnfitz -- rewrote this section
		if (miptex >= loadmodel->numtextures-1 || !loadmodel->textures[miptex])
		{
			if(out->flags & BSP_TEXTURE_SPECIAL)
				out->texture = loadmodel->textures[loadmodel->numtextures-1];
			else
				out->texture = loadmodel->textures[loadmodel->numtextures-2];
			out->flags |= BSP_TEXTURE_MISSING;
			missing++;
		}
		else
			out->texture = loadmodel->textures[miptex];
		//johnfitz
	}

	//johnfitz: report missing textures
	if (missing && loadmodel->numtextures > 1)
		// [11/7/2012] Updated ~hogsy
		Con_Warning("Textures missing from BSP file! (%i)\n",missing);
	//johnfitz
}

/*	Fills in s->texturemins[] and s->extents[]
*/
void CalcSurfaceExtents (msurface_t *s)
{
	float	mins[2], maxs[2];
	double	val;
	int		i,j, e;
	BSPVertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;

	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];

		for (j=0 ; j<2 ; j++)
		{
			// [24/11/2013] Double cast, suggestion from LordHavoc ~hogsy
			val =	(double)	v->fPoint[0]*tex->vecs[j][0]+
								v->fPoint[1]*tex->vecs[j][1]+
								v->fPoint[2]*tex->vecs[j][2]+
								tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{
		bmins[i] = floor(mins[i]/16);
		bmaxs[i] = ceil(maxs[i]/16);

		s->texturemins[i] = bmins[i]*16;
		s->extents[i] = (bmaxs[i]-bmins[i])*16;

		if(!(tex->flags & BSP_TEXTURE_SPECIAL) && s->extents[i] > 2000) //johnfitz -- was 512 in glquake, 256 in winquake
			Con_Warning("Bad surface extents\n");
	}
}

/*
================
Mod_PolyForUnlitSurface -- johnfitz -- creates polys for unlightmapped surfaces (sky and water)

TODO: merge this into BuildSurfaceDisplayList?
================
*/
void Mod_PolyForUnlitSurface (msurface_t *fa)
{
	vec3_t		verts[64];
	int			numverts, i, lindex;
	float		*vec;
	glpoly_t	*poly;
	float		texscale;

	if (fa->flags & (SURF_DRAWTURB | SURF_DRAWSKY))
		texscale = (1.0/128.0); //warp animation repeats every 128
	else
		texscale = (1.0/32.0); //to match r_notexture_mip

	// convert edges back to a normal polygon
	numverts = 0;
	for (i=0 ; i<fa->numedges ; i++)
	{
		lindex = loadmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].fPoint;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].fPoint;
		Math_VectorCopy(vec,verts[numverts]);
		numverts++;
	}

	//create the poly
	poly = (glpoly_t*)Hunk_Alloc (sizeof(glpoly_t) + (numverts-4) * VERTEXSIZE*sizeof(float));
	poly->next = NULL;
	fa->polys = poly;
	poly->numverts = numverts;

	for (i=0, vec=(float *)verts; i<numverts; i++, vec+= 3)
	{
		Math_VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = Math_DotProduct(vec, fa->texinfo->vecs[0]) * texscale;
		poly->verts[i][4] = Math_DotProduct(vec, fa->texinfo->vecs[1]) * texscale;
	}
}

void Mod_CalcSurfaceBounds(msurface_t *s)
{
	int			i, e;
	BSPVertex_t	*v;

	s->mins[0] = s->mins[1] = s->mins[2] = 9999;
	s->maxs[0] = s->maxs[1] = s->maxs[2] = -9999;

	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];

		if (s->mins[0] > v->fPoint[0])
			s->mins[0] = v->fPoint[0];
		if (s->mins[1] > v->fPoint[1])
			s->mins[1] = v->fPoint[1];
		if (s->mins[2] > v->fPoint[2])
			s->mins[2] = v->fPoint[2];

		if (s->maxs[0] < v->fPoint[0])
			s->maxs[0] = v->fPoint[0];
		if (s->maxs[1] < v->fPoint[1])
			s->maxs[1] = v->fPoint[1];
		if (s->maxs[2] < v->fPoint[2])
			s->maxs[2] = v->fPoint[2];
	}
}

void GL_SubdivideSurface(msurface_t *fa);

void Model_LoadBSPFaces(BSPLump_t *blLump)
{
	BSPFace_t	*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;

	in = (void *)(mod_base + blLump->iFileOffset);
	if (blLump->iFileLength % sizeof(*in))
		Sys_Error ("Model_LoadBSPFaces: funny lump size in %s",loadmodel->name);
	count = blLump->iFileLength / sizeof(*in);
	out = (msurface_t*)Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge	= LittleLong(in->iFirstEdge);
		out->numedges	= LittleLong(in->iNumEdges);
		out->flags = 0;

		planenum = LittleLong(in->iPlaneNum);

		side = LittleLong(in->iSide);
		if (side)
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;

		out->texinfo = loadmodel->texinfo+LittleLong(in->iTexInfo);

		CalcSurfaceExtents (out);

		Mod_CalcSurfaceBounds (out); //johnfitz -- for per-surface frustum culling

	// lighting info

		for (i=0 ; i<BSP_MAX_LIGHTMAPS ; i++)
			out->styles[i] = in->bStyles[i];
		i = LittleLong(in->iLightOffset);
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata+(i*3); //johnfitz -- lit support via lordhavoc (was "+ i")

		//johnfitz -- this section rewritten
		if(!Q_strncasecmp(out->texinfo->texture->name,"sky",3)) // sky surface //also note -- was Q_strncmp, changed to match qbsp
		{
			out->flags |= (SURF_DRAWSKY | SURF_DRAWTILED);
			Mod_PolyForUnlitSurface (out); //no more subdivision
		}
		else if (out->texinfo->texture->name[0] == '*') // warp surface
		{
			out->flags |= (SURF_DRAWTURB | SURF_DRAWTILED);

			if((!out->samples && SURF_DRAWTURB) || SURF_DRAWTILED)
				Mod_PolyForUnlitSurface(out);

			GL_SubdivideSurface (out);
		}
		else if(out->texinfo->flags & BSP_TEXTURE_MISSING) // texture is missing from bsp
		{
			if(out->samples) //lightmapped
				out->flags |= SURF_NOTEXTURE;
			else // not lightmapped
			{
				out->flags |= (SURF_NOTEXTURE | SURF_DRAWTILED);
				Mod_PolyForUnlitSurface (out);
			}
		}
		//johnfitz
	}
}

void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if(node->contents < 0)
		return;

	Mod_SetParent(node->children[0],node);
	Mod_SetParent(node->children[1],node);
}

void Model_LoadBSPNodes(BSPLump_t *blLump)
{
	int			i, j, count, p;
	BSPNode_t	*in;
	mnode_t 	*out;

	in = (void *)(mod_base + blLump->iFileOffset);
	if (blLump->iFileLength % sizeof(*in))
		Sys_Error ("Model_LoadBSPNodes: funny lump size in %s",loadmodel->name);

	count	= blLump->iFileLength / sizeof(*in);
	out		= (mnode_t*)Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j]		= LittleFloat(in->fMins[j]);
			out->minmaxs[3+j]	= LittleFloat(in->fMaxs[j]);
		}

		p = LittleLong(in->iPlaneNum);
		out->plane = loadmodel->planes + p;

		out->firstsurface	= LittleLong (in->usFirstFace); //johnfitz -- explicit cast as unsigned short
		out->numsurfaces	= LittleLong (in->usNumFaces); //johnfitz -- explicit cast as unsigned short

		for (j=0 ; j<2 ; j++)
		{
			//johnfitz -- hack to handle nodes > 32k, adapted from darkplaces
			p = (unsigned short)LittleLong(in->iChildren[j]);
			if (p < count)
				out->children[j] = loadmodel->nodes + p;
			else
			{
				p = 65535 - p; //note this uses 65535 intentionally, -1 is leaf 0
				if (p < loadmodel->numleafs)
					out->children[j] = (mnode_t *)(loadmodel->leafs + p);
				else
				{
					Con_Printf("Model_LoadBSPNodes: invalid leaf index %i (file has only %i leafs)\n", p, loadmodel->numleafs);
					out->children[j] = (mnode_t *)(loadmodel->leafs); //map it to the solid leaf
				}
			}
			//johnfitz
		}
	}

	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

void Model_LoadBSPLeafs(BSPLump_t *blLump)
{
	BSPLeaf_t 	*in;
	mleaf_t 	*out;
	int			i, j, count, p;

	in = (void *)(mod_base + blLump->iFileOffset);
	if (blLump->iFileLength % sizeof(*in))
		Sys_Error ("Model_LoadBSPLeafs: funny lump size in %s",loadmodel->name);
	count = blLump->iFileLength / sizeof(*in);
	out = (mleaf_t*)Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->leafs	= out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j]		= LittleFloat(in->fMins[j]);
			out->minmaxs[3+j]	= LittleFloat(in->fMaxs[j]);
		}

		p = LittleLong(in->iContents);
		out->contents = p;

		out->firstmarksurface	= loadmodel->marksurfaces+LittleLong(in->uiFirstMarkSurface);
		out->nummarksurfaces	= LittleLong(in->uiNumMarkSurfaces);

		p = LittleLong(in->iVisibilityOffset);
		if (p == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = loadmodel->visdata + p;
		out->efrags = NULL;

		for (j=0 ; j<4 ; j++)
			out->ambient_sound_level[j] = in->bAmbientLevel[j];

		//johnfitz -- removed code to mark surfaces as SURF_UNDERWATER
	}
}

void Model_LoadBSPClipNodes(BSPLump_t *blLump)
{
	BSPClipNode_t *in;
	BSPClipNode_t *out; //johnfitz -- was dclipnode_t
	int			i, count;
	hull_t		*hull;

	in = (void *)(mod_base + blLump->iFileOffset);
	if (blLump->iFileLength % sizeof(*in))
		Sys_Error ("Model_LoadBSPClipNodes: funny lump size in %s",loadmodel->name);
	count = blLump->iFileLength / sizeof(*in);
	out = (BSPClipNode_t*)Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->clipnodes = out;
	loadmodel->numclipnodes = count;

	hull = &loadmodel->hulls[1];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 32;

	hull = &loadmodel->hulls[2];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -32;
	hull->clip_mins[1] = -32;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 32;
	hull->clip_maxs[1] = 32;
	hull->clip_maxs[2] = 64;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->iPlaneNum = LittleLong(in->iPlaneNum);

		//johnfitz -- bounds check
		if (out->iPlaneNum < 0 || out->iPlaneNum >= loadmodel->numplanes)
			Host_Error ("Model_LoadBSPClipNodes: planenum out of bounds! (%i)",out->iPlaneNum);
		//johnfitz

		//johnfitz -- support clipnodes > 32k
		out->iChildren[0]	= LittleLong(in->iChildren[0]);
		out->iChildren[1]	= LittleLong(in->iChildren[1]);
		if (out->iChildren[0] >= count)
			out->iChildren[0] -= 65536;
		if (out->iChildren[1] >= count)
			out->iChildren[1] -= 65536;
		//johnfitz
	}
}

/*	Duplicate the drawing hull structure as a clipping hull
*/
void Mod_MakeHull0 (void)
{
	mnode_t			*in, *child;
	BSPClipNode_t	*out; //johnfitz -- was dclipnode_t
	int			i, j, count;
	hull_t		*hull;

	hull = &loadmodel->hulls[0];

	in = loadmodel->nodes;
	count = loadmodel->numnodes;
	out = (BSPClipNode_t*)Hunk_AllocName ( count*sizeof(*out), loadname);

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->iPlaneNum = in->plane - loadmodel->planes;
		for (j=0 ; j<2 ; j++)
		{
			child = in->children[j];
			if (child->contents < 0)
				out->iChildren[j] = child->contents;
			else
				out->iChildren[j] = child - loadmodel->nodes;
		}
	}
}

void Model_LoadBSPMarkSurfaces(BSPLump_t *blLump)
{
	int		i, j, count;
	int		*in;
	msurface_t **out;

	in = (void *)(mod_base + blLump->iFileOffset);
	if (blLump->iFileLength % sizeof(*in))
		Sys_Error ("Model_LoadBSPMarkSurfaces: funny lump size in %s",loadmodel->name);
	count = blLump->iFileLength / sizeof(*in);
	out = (msurface_t**)Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleLong(in[i]);
		if (j >= loadmodel->numsurfaces)
			Sys_Error ("Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}

void Model_LoadBSPSurfaceEdges(BSPLump_t *blLump)
{
	int	i, count;
	int	*in, *out;

	in = (void *)(mod_base+blLump->iFileOffset);
	if(blLump->iFileLength%sizeof(*in))
		Sys_Error ("Model_LoadBSPSurfaceEdges: funny lump size in %s",loadmodel->name);

	count	= blLump->iFileLength/sizeof(*in);
	out		= (int*)Hunk_AllocName(count*sizeof(*out),loadname);

	loadmodel->surfedges	= out;
	loadmodel->numsurfedges = count;

	for ( i=0 ; i<count ; i++)
		out[i] = LittleLong (in[i]);
}

void Model_LoadBSPPlanes(BSPLump_t *blLump)
{
	int			i, j;
	mplane_t	*out;
	BSPPlane_t 	*in;
	int			count;
	int			bits;

	in = (void *)(mod_base + blLump->iFileOffset);
	if (blLump->iFileLength % sizeof(*in))
		Sys_Error ("Model_LoadBSPPlanes: funny lump size in %s\n",loadmodel->name);
	count = blLump->iFileLength / sizeof(*in);
	out = (mplane_t*)Hunk_AllocName ( count*2*sizeof(*out), loadname);

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat(in->fNormal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist		= LittleFloat(in->fDist);
		out->type		= LittleLong(in->iType);
		out->signbits	= bits;
	}
}

float RadiusFromBounds (vec3_t mins, vec3_t maxs)
{
	int		i;
	vec3_t	corner;

	for (i=0 ; i<3 ; i++)
		corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);

	return Math_Length(corner);
}

void Model_LoadBSPSubmodels(BSPLump_t *blLump)
{
	BSPModel_t	*in;
	BSPModel_t	*out;
	int			i, j, count;

	in = (void *)(mod_base+blLump->iFileOffset);
	if (blLump->iFileLength%sizeof(*in))
		Sys_Error ("Model_LoadBSPSubmodels: funny lump size in %s",loadmodel->name);

	count	= blLump->iFileLength/sizeof(*in);
	out		= (BSPModel_t*)Hunk_AllocName ( count*sizeof(*out), loadname);

	loadmodel->submodels	= out;
	loadmodel->numsubmodels = count;

	for(i = 0; i < count; i++,in++,out++)
	{
		for(j = 0; j < 3; j++)
		{	// spread the mins / maxs by a pixel
			out->fMins[j]	= LittleFloat(in->fMins[j])-1;
			out->fMaxs[j]	= LittleFloat(in->fMaxs[j])+1;
			out->fOrigin[j]	= LittleFloat(in->fOrigin[j]);
		}

		for (j=0 ; j<BSP_MAX_HULLS ; j++)
			out->iHeadNode[j] = LittleLong (in->iHeadNode[j]);

		out->iVisLeafs	= LittleLong(in->iVisLeafs);
		out->iFirstFace = LittleLong(in->iFirstFace);
		out->iNumFaces	= LittleLong(in->iNumFaces);
	}

	// johnfitz -- check world visleafs -- adapted from bjp
	out = loadmodel->submodels;

	if(out->iVisLeafs > BSP_MAX_LEAFS)
		Sys_Error ("Model_LoadBSPSubmodels: too many visleafs (%d, max = %d) in %s", out->iVisLeafs, BSP_MAX_LEAFS, loadmodel->name);
	//johnfitz
}

/*	Update the model's clipmins and clipmaxs based on each node's plane.

	This works because of the way brushes are expanded in hull generation.
	Each brush will include all six axial planes, which bound that brush.
	Therefore, the bounding box of the hull can be constructed entirely
	from axial planes found in the clipnodes for that hull.
*/
void Mod_BoundsFromClipNode (model_t *mod, int hull, int nodenum)
{
	mplane_t		*plane;
	BSPClipNode_t	*node;

	if (nodenum < 0)
		return; //hit a leafnode

	node = &mod->clipnodes[nodenum];
	plane = mod->hulls[hull].planes + node->iPlaneNum;
	switch (plane->type)
	{

	case PLANE_X:
		if (plane->signbits == 1)
			mod->clipmins[0] = Math_Min (mod->clipmins[0], -plane->dist - mod->hulls[hull].clip_mins[0]);
		else
			mod->clipmaxs[0] = Math_Max (mod->clipmaxs[0], plane->dist - mod->hulls[hull].clip_maxs[0]);
		break;
	case PLANE_Y:
		if (plane->signbits == 2)
			mod->clipmins[1] = Math_Min (mod->clipmins[1], -plane->dist - mod->hulls[hull].clip_mins[1]);
		else
			mod->clipmaxs[1] = Math_Max (mod->clipmaxs[1], plane->dist - mod->hulls[hull].clip_maxs[1]);
		break;
	case PLANE_Z:
		if (plane->signbits == 4)
			mod->clipmins[2] = Math_Min (mod->clipmins[2], -plane->dist - mod->hulls[hull].clip_mins[2]);
		else
			mod->clipmaxs[2] = Math_Max (mod->clipmaxs[2], plane->dist - mod->hulls[hull].clip_maxs[2]);
		break;
	default:
		//skip nonaxial planes; don't need them
		break;
	}

	Mod_BoundsFromClipNode (mod, hull, node->iChildren[0]);
	Mod_BoundsFromClipNode (mod, hull, node->iChildren[1]);
}

void Model_LoadBSP(model_t *mod,void *buffer)
{
	int			version,i,j;
	BSPHeader_t	*bhHeader;
	BSPModel_t 	*bm;
	float		radius; //johnfitz

	bhHeader = (BSPHeader_t*)buffer;

	version = LittleLong(bhHeader->iVersion);
	if(version != BSP_VERSION)
		Console_ErrorMessage(false,mod->name,va("Wrong version (%i should be %i)",version,BSP_VERSION));

	loadmodel->version	= version;
	loadmodel->mType	= MODEL_BRUSH;

	// swap all the lumps
	mod_base = (byte*)bhHeader;

	for (i=0; i < sizeof(BSPHeader_t)/4; i++)
		((int*)bhHeader)[i] = LittleLong(((int*)bhHeader)[i]);

	// Load into heap
	Model_LoadBSPVertexes(&bhHeader->bLumps[LUMP_VERTEXES]);
	Model_LoadBSPEdges(&bhHeader->bLumps[LUMP_EDGES]);
	Model_LoadBSPSurfaceEdges(&bhHeader->bLumps[LUMP_SURFEDGES]);
	Model_LoadBSPTextures(&bhHeader->bLumps[LUMP_TEXTURES]);
	Model_LoadBSPLighting(&bhHeader->bLumps[LUMP_LIGHTING]);
	Model_LoadBSPPlanes(&bhHeader->bLumps[LUMP_PLANES]);
	Model_LoadBSPTextureInfo(&bhHeader->bLumps[LUMP_TEXINFO]);
	Model_LoadBSPFaces(&bhHeader->bLumps[LUMP_FACES]);
	Model_LoadBSPMarkSurfaces(&bhHeader->bLumps[LUMP_MARKSURFACES]);
	Model_LoadBSPVisibility(&bhHeader->bLumps[LUMP_VISIBILITY]);
	Model_LoadBSPLeafs(&bhHeader->bLumps[LUMP_LEAFS]);
	Model_LoadBSPNodes(&bhHeader->bLumps[LUMP_NODES]);
	Model_LoadBSPClipNodes(&bhHeader->bLumps[LUMP_CLIPNODES]);
	Model_LoadBSPEntities(&bhHeader->bLumps[LUMP_ENTITIES]);
	Model_LoadBSPSubmodels(&bhHeader->bLumps[LUMP_MODELS]);

	Mod_MakeHull0();

	mod->numframes = 2;		// regular and alternate animation

//
// set up the submodels (FIXME: this is confusing)
//
	// johnfitz -- okay, so that i stop getting confused every time i look at this loop, here's how it works:
	// we're looping through the submodels starting at 0.  Submodel 0 is the main model, so we don't have to
	// worry about clobbering data the first time through, since it's the same data.  At the end of the loop,
	// we create a new copy of the data to use the next time through.
	for (i=0 ; i<mod->numsubmodels ; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->iHeadNode[0];
		for (j=1 ; j<BSP_MAX_HULLS ; j++)
		{
			mod->hulls[j].firstclipnode = bm->iHeadNode[j];
			mod->hulls[j].lastclipnode	= mod->numclipnodes-1;
		}

		mod->firstmodelsurface	= bm->iFirstFace;
		mod->nummodelsurfaces	= bm->iNumFaces;

		Math_VectorCopy (bm->fMaxs, mod->maxs);
		Math_VectorCopy (bm->fMins, mod->mins);

		//johnfitz -- calculate rotate bounds and yaw bounds
		radius = RadiusFromBounds (mod->mins, mod->maxs);
		mod->rmaxs[0] = mod->rmaxs[1] = mod->rmaxs[2] = mod->ymaxs[0] = mod->ymaxs[1] = mod->ymaxs[2] = radius;
		mod->rmins[0] = mod->rmins[1] = mod->rmins[2] = mod->ymins[0] = mod->ymins[1] = mod->ymins[2] = -radius;
		//johnfitz

		//johnfitz -- correct physics cullboxes so that outlying clip brushes on doors and stuff are handled right
		if (i > 0 || strcmp(mod->name, sv.modelname) != 0) //skip submodel 0 of sv.worldmodel, which is the actual world
		{
			// start with the hull0 bounds
			Math_VectorCopy (mod->maxs, mod->clipmaxs);
			Math_VectorCopy (mod->mins, mod->clipmins);
		}
		//johnfitz

		mod->numleafs = bm->iVisLeafs;

		if (i < mod->numsubmodels-1)
		{
			// duplicate the basic information
			char	name[10];

			sprintf (name, "*%i", i+1);
			loadmodel = Model_FindName(name);
			*loadmodel = *mod;
			strcpy (loadmodel->name, name);
			mod = loadmodel;
		}
	}
}

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

/*
	Scripting crap
*/

typedef struct
{
	char	*cKey;

	void	(*Function)(MD2_t *mModel,char *cArg);
} MaterialKey_t;

int	iMaterialCount;

MD2_t	*mCurrentModel;

void _Material_SetDiffuseTexture(MD2_t *mModel,char *cArg)
{
	byte *bDiffuseMap = Image_LoadImage(cArg,&mModel->skinwidth,&mModel->skinheight);
	if(bDiffuseMap)
		mModel->gDiffuseTexture[iMaterialCount] = TexMgr_LoadImage(loadmodel,cArg,mModel->skinwidth,mModel->skinheight,SRC_RGBA,bDiffuseMap,cArg,0,TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

void _Material_SetFullbrightTexture(MD2_t *mModel,char *cArg)
{
	byte *bFullbrightMap = Image_LoadImage(cArg,&mModel->skinwidth,&mModel->skinheight);
	if(bFullbrightMap)
		mModel->gFullbrightTexture[iMaterialCount] = TexMgr_LoadImage(loadmodel,cArg,mModel->skinwidth,mModel->skinheight,SRC_RGBA,bFullbrightMap,cArg,0,TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

void _Material_SetSphereTexture(MD2_t *mModel,char *cArg)
{
	byte *bSphereMap = Image_LoadImage(cArg,&mModel->skinwidth,&mModel->skinheight);
	if(bSphereMap)
		mModel->gSphereTexture[iMaterialCount] = TexMgr_LoadImage(loadmodel,cArg,mModel->skinwidth,mModel->skinheight,SRC_RGBA,bSphereMap,cArg,0,TEXPREF_ALPHA);
	else
		Con_Warning("Failed to load texture %s!\n",cArg);
}

MaterialKey_t	mkMaterialKey[]=
{
	{	"$diffuse",			_Material_SetDiffuseTexture		},	// Sets the diffuse texture.
	{	"$sphere",			_Material_SetSphereTexture		},	// Sets the spheremap texture.
	{	"$fullbright",		_Material_SetFullbrightTexture	},	// Sets the fullbright texture.

	{	0	}
};

void _Material_AddSkin(char *cArg)
{
	iMaterialCount++;

	// Set defaults...
	mCurrentModel->gDiffuseTexture[iMaterialCount]		= notexture;
	mCurrentModel->gSphereTexture[iMaterialCount]		= NULL;
	mCurrentModel->gFullbrightTexture[iMaterialCount]	= NULL;
}

/*
	Everything else
*/

void Model_LoadTextures(MD2_t *mModel)
{
	char	cScriptPath[MAX_OSPATH],
			cOutName[MAX_OSPATH];

	COM_StripExtension(loadmodel->name,cOutName);

	sprintf(cScriptPath,"textures/%s.material",cOutName);	//(char*)model+model->ofs_skins+i*MAX_SKINNAME);

	iMaterialCount = -1;

	mCurrentModel = mModel;

	//if(!Script_Load(cScriptPath))
	// Allow us to load textures without materials.
	{
		Con_Warning("Failed to load material! (%s)\n",cScriptPath);

		sprintf(cScriptPath,"textures/%s",cOutName);

		_Material_AddSkin("");
		_Material_SetDiffuseTexture(mModel,cScriptPath);
	}

	if(iMaterialCount < 0 || iMaterialCount > MD2_MAX_SKINS)
		Console_ErrorMessage(true,loadmodel->name,va("Invalid number of skins (%i)",iMaterialCount));
}

/*	Calculate bounds of alias model for nonrotated, yawrotated, and fullrotated cases
*/
#if 0
void Mod_CalcAliasBounds(aliashdr_t *a)
{
	int			i,j,k;
	float		dist, yawradius, radius;
	vec3_t		v;

	//clear out all data
	for (i=0; i<3;i++)
	{
		loadmodel->mins[i] = loadmodel->ymins[i] = loadmodel->rmins[i] = 999999;
		loadmodel->maxs[i] = loadmodel->ymaxs[i] = loadmodel->rmaxs[i] = -999999;
		radius = yawradius = 0;
	}

	//process verts
	for (i=0 ; i<a->numposes; i++)
		for (j=0; j<a->numverts; j++)
		{
			for (k=0; k<3;k++)
				v[k] = poseverts[i][j].v[k] * pheader->scale[k] + pheader->scale_origin[k];

			for (k=0; k<3;k++)
			{
				loadmodel->mins[k] = Math_Min(loadmodel->mins[k], v[k]);
				loadmodel->maxs[k] = Math_Max(loadmodel->maxs[k], v[k]);
			}

			dist = v[0] * v[0] + v[1] * v[1];
			if (yawradius < dist)
				yawradius = dist;

			dist += v[2] * v[2];
			if (radius < dist)
				radius = dist;
		}

	//rbounds will be used when entity has nonzero pitch or roll
	radius = sqrt(radius);
	loadmodel->rmins[0] = loadmodel->rmins[1] = loadmodel->rmins[2] = -radius;
	loadmodel->rmaxs[0] = loadmodel->rmaxs[1] = loadmodel->rmaxs[2] = radius;

	//ybounds will be used when entity has nonzero yaw
	yawradius = sqrt(yawradius);
	loadmodel->ymins[0] = loadmodel->ymins[1] = -yawradius;
	loadmodel->ymaxs[0] = loadmodel->ymaxs[1] = yawradius;
	loadmodel->ymins[2] = loadmodel->mins[2];
	loadmodel->ymaxs[2] = loadmodel->maxs[2];
}
#endif

// [21/8/2012] TODO: Finish ~hogsy
void Model_LoadIQM(model_t *mModel,void *Buffer)
{
	IQMHeader_t	hHeader;

	// [14/9/2012] Checks ~hogsy
	if(hHeader.uiFileSize <= 0 || hHeader.uiFileSize >= MD2_MAX_SIZE)
		Sys_Error("%s is not a valid model",mModel->name);
	else if(hHeader.uiVersion != IQM_VERSION)
		Sys_Error("%s is an invalid version (expected %i, recieved %i)",IQM_VERSION,hHeader.uiVersion);
	else if(hHeader.uiNumTriangles < 1 || hHeader.uiNumTriangles > MD2_MAX_TRIANGLES)
		Sys_Error("%s has invalid number of triangles (%i)",mModel->name,hHeader.uiNumTriangles);
	else if(hHeader.uiNumVertexes < 1 || hHeader.uiNumVertexes > MD2_MAX_VERTICES)
		Sys_Error("%s has invalid number of vertices (%i)",mModel->name,hHeader.uiNumVertexes);
	// [14/9/2012] We'll check anims here instead of frames... ~hogsy
	else if(hHeader.uiNumAnims < 1 || hHeader.uiNumAnims > MD2_MAX_FRAMES)
		Sys_Error("%s has invalid number of animations (%i)",mModel->name,hHeader.uiNumAnims);
	// [14/9/2012] TODO: IQM models use multiple textures, check those! ~hogsy
}

void Model_LoadMD2(model_t *mModel,void *Buffer)
{
	int						i,j,
							iVersion,
							numframes,iSize,*pinglcmd,*poutglcmd,iStartHunk,iEnd,total;
	MD2_t					*pinmodel,*mMD2Model;
	MD2Triangle_t			*pintriangles,*pouttriangles;
	MD2Frame_t				*pinframe,*poutframe;
	MD2TextureCoordinate_t	*pST;

	iStartHunk = Hunk_LowMark();

	pinmodel = (MD2_t*)Buffer;

	iVersion = LittleLong(pinmodel->version);
	if(iVersion != MD2_VERSION)
	{
		Con_Error("%s has wrong version number (%i should be %i)\n",mModel->name,iVersion,MD2_VERSION);
		return;
	}

	iSize = LittleLong(pinmodel->ofs_end)+sizeof(MD2_t);

	mMD2Model = (MD2_t*)Hunk_AllocName(iSize,loadname);
	for(i = 0; i < 17; i++)
		((int*)mMD2Model)[i] = LittleLong(((int*)pinmodel)[i]);

	mModel->mType		= MODEL_TYPE_MD2;
	mModel->version		= iVersion;
	mModel->flags		= 0;
	mModel->numframes	= numframes = mMD2Model->num_frames;

	if(iSize <= 0 || iSize >= MD2_MAX_SIZE)
		Sys_Error("%s is not a valid model",mModel->name);
	else if(mMD2Model->ofs_skins <= 0 || mMD2Model->ofs_skins >= mMD2Model->ofs_end)
		Sys_Error("%s is not a valid model",mModel->name);
	else if(mMD2Model->ofs_st <= 0 || mMD2Model->ofs_st >= mMD2Model->ofs_end)
		Sys_Error("%s is not a valid model",mModel->name);
	else if(mMD2Model->ofs_tris <= 0 || mMD2Model->ofs_tris >= mMD2Model->ofs_end)
		Sys_Error("%s is not a valid model",mModel->name);
	else if(mMD2Model->ofs_frames <= 0 || mMD2Model->ofs_frames >= mMD2Model->ofs_end)
		Sys_Error("%s is not a valid model",mModel->name);
	else if(mMD2Model->ofs_glcmds <= 0 || mMD2Model->ofs_glcmds >= mMD2Model->ofs_end)
		Sys_Error("%s is not a valid model",mModel->name);
	else if(mMD2Model->numtris < 1 || mMD2Model->numtris > MD2_MAX_TRIANGLES)
		Sys_Error("%s has invalid number of triangles (%i)",mModel->name,mMD2Model->numtris);
	else if(mMD2Model->num_xyz < 1 || mMD2Model->num_xyz > MD2_MAX_VERTICES)
		Sys_Error("%s has invalid number of vertices (%i)",mModel->name,mMD2Model->num_xyz);
	else if(mMD2Model->num_frames < 1 || mMD2Model->num_frames > MD2_MAX_FRAMES)
		Sys_Error("%s has invalid number of frames (%i)",mModel->name,mMD2Model->num_frames);
	else if(mMD2Model->num_skins < 0 || mMD2Model->num_skins > MD2_MAX_SKINS)
		Sys_Error("%s has invalid number of skins (%i)",mModel->name,mMD2Model->num_skins);

#if 0
	Con_DPrintf("NUMST: %i\n",mMD2Model->num_st);
#endif

	for(i = 0; i < 7; i++)
		((int*)&mMD2Model->ofs_skins)[i] += sizeof(mMD2Model);

	pintriangles	= (MD2Triangle_t*)((uint8_t*)pinmodel+LittleLong(pinmodel->ofs_tris));
	pouttriangles	= (MD2Triangle_t*)((uint8_t*)mMD2Model+mMD2Model->ofs_tris);
	for(i=0; i < mMD2Model->numtris; i++)
	{
		for(j=0; j < 3; j++)
		{
			pouttriangles->index_xyz[j] = LittleShort(pintriangles->index_xyz[j]);
			pouttriangles->index_st[j]	= LittleShort(pintriangles->index_st[j]);

			if(pouttriangles->index_xyz[j] >= mMD2Model->num_xyz || pouttriangles->index_st[j] >= mMD2Model->num_st)
				Sys_Error("Model has invalid vertex indices! (%s) (%i)",mModel->name,(int)pouttriangles->index_xyz[j]);
		}

		pintriangles++;
		pouttriangles++;
	}

	pinframe	= (MD2Frame_t*)((uint8_t*)pinmodel+LittleLong(pinmodel->ofs_frames));
	poutframe	= (MD2Frame_t*)((uint8_t*)mMD2Model+mMD2Model->ofs_frames);
	for(i=0; i < numframes; i++)
	{
		for(j=0; j < 3; j++)
		{
			poutframe->scale[j] = LittleFloat(pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat(pinframe->translate[j]);
		}

		for(j=0; j < 17; j++)
			poutframe->name[j] = pinframe->name[j];

		for(j=0; j < mMD2Model->num_xyz; j++)
		{
			poutframe->verts[j].v[0] = pinframe->verts[j].v[0];
			poutframe->verts[j].v[1] = pinframe->verts[j].v[1];
			poutframe->verts[j].v[2] = pinframe->verts[j].v[2];
			poutframe->verts[j].lightnormalindex = pinframe->verts[j].lightnormalindex;
		}

		pinframe	= (MD2Frame_t*)&pinframe->verts[j].v[0];
		poutframe	= (MD2Frame_t*)&poutframe->verts[j].v[0];
	}

	pinglcmd	= (int*)((uint8_t*)pinmodel+LittleLong(pinmodel->ofs_glcmds));
	poutglcmd	= (int*)((uint8_t*)mMD2Model+mMD2Model->ofs_glcmds);
	for(i=0; i < mMD2Model->num_glcmds; i++)
		*poutglcmd++ = LittleLong(*pinglcmd++);

	// Ugh, kill me.
	{
		mMD2Model->mtcTextureCoord = (MD2TextureCoordinate_t*)malloc(mMD2Model->num_st*sizeof(MD2TextureCoordinate_t));

		pST	= (MD2TextureCoordinate_t*)((uint8_t*)pinmodel+LittleLong(pinmodel->ofs_st));
		for(i = 0; i < mMD2Model->num_st; i++)
		{
			mMD2Model->mtcTextureCoord[i].S = LittleShort(pST->S);
			mMD2Model->mtcTextureCoord[i].T = LittleShort(pST->T);

			pST++;
		}
	}

	memcpy
	(
		(char*)mMD2Model+mMD2Model->ofs_skins,
		(char*)pinmodel+mMD2Model->ofs_skins,
		mMD2Model->num_skins*MAX_QPATH
	);

	Model_LoadTextures(mMD2Model);

#if 1
	for (i=0; i<3;i++)
	{
		loadmodel->mins[i] = loadmodel->ymins[i] = loadmodel->rmins[i] = -32.0f;
		loadmodel->maxs[i] = loadmodel->ymaxs[i] = loadmodel->rmaxs[i] = 32.0f;
	}
#else
	Math_VectorSet(999999.0f,loadmodel->mins);
	Math_VectorSet(-999999.0f,loadmodel->maxs);

	for(i = 0; i < mMD2Model->num_xyz; i++)
	{
		MD2TriangleVertex_t *mVertex = (MD2TriangleVertex_t*)((int)mMD2Model+mMD2Model->ofs_frames+mMD2Model->framesize);

#if 1
		if(mVertex->v[X] < vMin[X])
			vMin[X] = mVertex->v[X];
		else if(mVertex->v[X] > vMax[X])
			vMax[X]	= mVertex->v[X];

		if(mVertex->v[Y] < vMin[Y])
			vMin[Y] = mVertex->v[Y];
		else if(mVertex->v[Y] > vMax[Y])
			vMax[Y] = mVertex->v[Y];

		if(mVertex->v[Z] < vMin[Z])
			vMin[Z] = mVertex->v[Z];
		else if(mVertex->v[Z] > vMax[Z])
			vMax[Z] = mVertex->v[Z];
#else
		for(j = 0; j < 3; j++)
		{
			loadmodel->mins[j] = loadmodel->ymins[j] = loadmodel->rmins[j] = Math_Min(loadmodel->mins[j],mVertex->v[j]);
			loadmodel->maxs[j] = loadmodel->ymaxs[j] = loadmodel->rmaxs[j] = Math_Max(loadmodel->maxs[j],mVertex->v[j]);
		}
#endif
	}
#endif

	iEnd	= Hunk_LowMark();
	total	= iEnd-iStartHunk;

	Cache_Alloc(&mModel->cache,total,loadname);
	if(!mModel->cache.data)
		return;

	memcpy(mModel->cache.data,mMD2Model,total);

	Hunk_FreeToLowMark(iStartHunk);
}

/*
	OBJ Support
*/

bool Model_LoadOBJ(model_t *mModel,void *Buffer)
{
	char	cExtension[4];
	OBJ_t	*oObject;

	mModel->mType	= MODEL_TYPE_OBJ;

	// Check if the file is a valid OBJ or not...
	ExtractFileExtension(mModel->name,cExtension);
	if(Q_strcmp(cExtension,".obj"))
		return false;

	// Parse OBJ file...
	for(;;)
	{
		char	cLine[128];

		if(fscanf(Buffer,"%s",cLine) == EOF)
			break;

		if(!Q_strcmp(cLine,"v"))
		{
			fscanf(Buffer,"%f %f %f\n",
				&oObject->ovVertex->vVertex[0],
				&oObject->ovVertex->vVertex[1],
				&oObject->ovVertex->vVertex[2]);
		}
		else if(!Q_strcmp(cLine,"vt"))
		{}
		else if(!Q_strcmp(cLine,"vn"))
		{}
		else if(!Q_strcmp(cLine,"f"))
		{}
	}

	return false;
}

/**/

void Model_PrintMemoryCache(void)
{
	int		i;
	model_t	*mod;

	Con_SafePrintf("Cached models:\n"); //johnfitz -- safeprint instead of print
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
		Con_SafePrintf ("%8p : %s\n", mod->cache.data, mod->name); //johnfitz -- safeprint instead of print

	Con_Printf ("%i models\n",mod_numknown); //johnfitz -- print the total too
}


