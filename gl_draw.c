/*
Copyright (C) 1996-2001 Id Software, Inc.
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

#include "KatGL.h"
#include "engine_console.h"
#include "engine_video.h"
#include "engine_client.h"	// [28/7/2013] Added for precache functions ~hogsy

#define	BLOCK_WIDTH		256
#define	BLOCK_HEIGHT	BLOCK_WIDTH

cvar_t		cvConsoleAlpha		= { "screen_consolealpha",	"0.7",						true	}; //johnfitz
cvar_t		cvConsoleBackground	= { "screen_consoleback",	"",							true	};
cvar_t		cvConsoleChars		= { "screen_consolechars",	"textures/engine/conchars",	true	};

qpic_t	*draw_backtile;
qpic_t	*pic_ovr, *pic_ins; //johnfitz -- new cursor handling
qpic_t	*pic_nul; //johnfitz -- for missing gfx, don't crash

gltexture_t *gCharTexture; //johnfitz

//johnfitz -- new pics
byte pic_ovr_data[8][8] =
{
	{255,255,255,255,255,255,255,255},
	{255, 15, 15, 15, 15, 15, 15,255},
	{255, 15, 15, 15, 15, 15, 15,  2},
	{255, 15, 15, 15, 15, 15, 15,  2},
	{255, 15, 15, 15, 15, 15, 15,  2},
	{255, 15, 15, 15, 15, 15, 15,  2},
	{255, 15, 15, 15, 15, 15, 15,  2},
	{255,255,  2,  2,  2,  2,  2,  2},
};

byte pic_ins_data[9][8] =
{
	{ 15, 15,255,255,255,255,255,255},
	{ 15, 15,  2,255,255,255,255,255},
	{ 15, 15,  2,255,255,255,255,255},
	{ 15, 15,  2,255,255,255,255,255},
	{ 15, 15,  2,255,255,255,255,255},
	{ 15, 15,  2,255,255,255,255,255},
	{ 15, 15,  2,255,255,255,255,255},
	{ 15, 15,  2,255,255,255,255,255},
	{255,  2,  2,255,255,255,255,255},
};

byte pic_nul_data[8][8] =
{
	{252,252,252,252,  0,  0,  0,  0},
	{252,252,252,252,  0,  0,  0,  0},
	{252,252,252,252,  0,  0,  0,  0},
	{252,252,252,252,  0,  0,  0,  0},
	{  0,  0,  0,  0,252,252,252,252},
	{  0,  0,  0,  0,252,252,252,252},
	{  0,  0,  0,  0,252,252,252,252},
	{  0,  0,  0,  0,252,252,252,252},
};
//johnfitz

typedef struct
{
	gltexture_t *gltexture;
	float		sl, tl, sh, th;
} glpic_t;

int currentcanvas = CANVAS_NONE; //johnfitz -- for GL_SetCanvas

//==============================================================================
//
//  PIC CACHING
//
//==============================================================================

typedef struct cachepic_s
{
	char		name[MAX_QPATH];
	qpic_t		pic;
	byte		padding[32];	// for appended glpic
} cachepic_t;

#define	MAX_CACHED_PICS		128
cachepic_t	menu_cachepics[MAX_CACHED_PICS];
int			menu_numcachepics;

byte		menuplyr_pixels[4096];

//  scrap allocation
//  Allocate all the little status bar obejcts into a single texture
//  to crutch up stupid hardware / drivers

#define	MAX_SCRAPS		2

int			scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT]; //johnfitz -- removed *4 after BLOCK_HEIGHT
bool		scrap_dirty;
gltexture_t	*scrap_textures[MAX_SCRAPS]; //johnfitz

/*	Returns an index into scrap_texnums[] and the position inside it
*/
int Scrap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_SCRAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (scrap_allocated[texnum][i+j] >= best)
					break;
				if (scrap_allocated[texnum][i+j] > best2)
					best2 = scrap_allocated[texnum][i+j];
			}

			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	Sys_Error ("Scrap_AllocBlock: full"); //johnfitz -- correct function name
	return 0; //johnfitz -- shut up compiler
}

void Scrap_Upload (void)
{
	char name[8];
	int	i;

	for (i=0; i<MAX_SCRAPS; i++)
	{
		sprintf (name, "scrap%i", i);
		scrap_textures[i] = TexMgr_LoadImage (NULL, name, BLOCK_WIDTH, BLOCK_HEIGHT, SRC_INDEXED, scrap_texels[i],
			"", (unsigned)scrap_texels[i], TEXPREF_ALPHA | TEXPREF_OVERWRITE | TEXPREF_NOPICMIP);
	}

	scrap_dirty = false;
}

//==================================================================

extern gltexture_t	*gMenuTexture[128];

void Draw_ExternPic(char *path,float alpha,int x,int y,int w,int h)
{
	int	i;

	// [15/9/2013] Fixed somewhat (clean this up) ~hogsy
	for(i = 0; i < sizeof(gMenuTexture); i++)
		if(!gMenuTexture[i])
		{
			Video_SetTexture(notexture);
			break;
		}
		else if(!strcmp(gMenuTexture[i]->name,path))
		{
			Video_SetTexture(gMenuTexture[i]);
			break;
		}

	{
		VideoObject_t voPicture	[]=
		{
			{	{	x,		y,		0	},	{	{	0,		0		}	},	{	1.0f,	1.0f,	1.0f,	alpha	}	},
			{	{	x+w,	y,		0	},	{	{	1.0f,	0		}	},	{	1.0f,	1.0f,	1.0f,	alpha	}	},
			{	{	x+w,	y+h,	0	},	{	{	1.0f,	1.0f	}	},	{	1.0f,	1.0f,	1.0f,	alpha	}	},
			{	{	x,		y+h,	0	},	{	{	0,		1.0f	}	},	{	1.0f,	1.0f,	1.0f,	alpha	}	}
		};

		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		Video_DrawFill(voPicture);

		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	}
}
//==================================================================

void SwapPic (qpic_t *pic)
{
	pic->width = LittleLong(pic->width);
	pic->height = LittleLong(pic->height);
}

qpic_t	*Draw_CachePic(char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	glpic_t		*gl;

	for (pic=menu_cachepics, i=0 ; i<menu_numcachepics ; pic++, i++)
		if (!strcmp (path, pic->name))
			return &pic->pic;

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");

	menu_numcachepics++;
	strcpy(pic->name, path);

	// load the pic from disk
	dat = (qpic_t *)COM_LoadTempFile (path);
	if(!dat)
	{
		Con_Warning("Failed to load cached texture (%s)!\n", path);
		return NULL;
	}

	SwapPic(dat);

	// HACK HACK HACK --- we need to keep the bytes for
	// the translatable player picture just for the menu
	// configuration dialog
	if (!strcmp (path, "gfx/menuplyr.lmp"))
		memcpy (menuplyr_pixels, dat->data, dat->width*dat->height);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	gl = (glpic_t *)pic->pic.data;
	gl->gltexture = TexMgr_LoadImage (NULL, path, dat->width, dat->height, SRC_INDEXED, dat->data, path,
									  sizeof(int)*2, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP); //johnfitz -- TexMgr
	gl->sl = 0;
	gl->sh = (float)dat->width/(float)TexMgr_PadConditional(dat->width); //johnfitz
	gl->tl = 0;
	gl->th = (float)dat->height/(float)TexMgr_PadConditional(dat->height); //johnfitz

	return &pic->pic;
}

qpic_t *Draw_MakePic (char *name, int width, int height, byte *data)
{
	int			flags = TEXPREF_NEAREST|TEXPREF_ALPHA|TEXPREF_PERSIST|TEXPREF_NOPICMIP|TEXPREF_PAD;
	qpic_t		*pic;
	glpic_t		*gl;

	pic = (qpic_t*)Hunk_Alloc (sizeof(qpic_t) - 4 + sizeof (glpic_t));
	pic->width	= width;
	pic->height = height;

	gl = (glpic_t *)pic->data;
	gl->gltexture = TexMgr_LoadImage (NULL, name, width, height, SRC_INDEXED, data, "", (unsigned)data, flags);
	gl->sl = 0;
	gl->sh = (float)width/(float)TexMgr_PadConditional(width);
	gl->tl = 0;
	gl->th = (float)height/(float)TexMgr_PadConditional(height);

	return pic;
}

//==============================================================================
//
//  INIT
//
//==============================================================================

void Draw_LoadPics (void)
{
	unsigned int	w,h;
	byte		    *data;

	data = Image_LoadImage(cvConsoleChars.string,&w,&h);
	if(!data)
		Sys_Error("Failed to load %s!",cvConsoleChars.string);

	gCharTexture = TexMgr_LoadImage(NULL,cvConsoleChars.string,w,h,SRC_RGBA,data,cvConsoleChars.string,0,TEXPREF_NEAREST|TEXPREF_ALPHA);

//	draw_backtile = Draw_PicFromWad ("backtile",WADFILENAME);
}

extern gltexture_t *playertextures[MAX_SCOREBOARD];

void SCR_LoadPics(void);

void Draw_NewGame (void)
{
	cachepic_t	*pic;
	int			i;

	// Empty scrap and reallocate gltextures
	memset(&scrap_allocated,0,sizeof(scrap_allocated));
	memset(&scrap_texels,255,sizeof(scrap_texels));
	// [8/8/2013] Throw out effect texture sets on each map load ~hogsy
	memset(&gEffectTexture,0,sizeof(gEffectTexture));

	Scrap_Upload (); //creates 2 empty gltextures

	// reload wad pics
	Draw_LoadPics();
	SCR_LoadPics();

	// empty lmp cache
	for (pic = menu_cachepics, i = 0; i < menu_numcachepics; pic++, i++)
		pic->name[0] = 0;
	menu_numcachepics = 0;

	//clear playertexture pointers (the textures themselves were freed by texmgr_newgame)
	for(i = 0; i < MAX_SCOREBOARD; i++)
		playertextures[i] = NULL;
}

void Draw_PrecacheConsoleBackground(void)
{
	Client_PrecacheResource(RESOURCE_TEXTURE,cvConsoleBackground.string);
}

void Draw_Init (void)
{
	Cvar_RegisterVariable(&cvConsoleAlpha,NULL);
	Cvar_RegisterVariable(&cvConsoleBackground,Draw_PrecacheConsoleBackground);

	// clear scrap and allocate gltextures
	memset(&scrap_allocated, 0, sizeof(scrap_allocated));
	memset(&scrap_texels, 255, sizeof(scrap_texels));
	Scrap_Upload (); //creates 2 empty textures

	// create internal pics
	pic_ins = Draw_MakePic ("ins", 8, 9, &pic_ins_data[0][0]);
	pic_ovr = Draw_MakePic ("ovr", 8, 8, &pic_ovr_data[0][0]);
	pic_nul = Draw_MakePic ("nul", 8, 8, &pic_nul_data[0][0]);

	// load game pics
	Draw_LoadPics();
}

//==============================================================================
//
//  2D DRAWING
//
//==============================================================================

void Draw_Character(int x,int y,int num)
{
	int		row, col;
	float	frow,fcol,size;

	if(y <= -8)
		return;			// totally off screen

	num &= 255;
	if(num == 32)
		return; //don't waste verts on spaces

	row = num>>4;
	col = num&15;

	frow = row*0.0625f;
	fcol = col*0.0625f;
	size = 0.0625f;

	{
		VideoObject_t	voCharacter[]=
		{
			{	{	x,		y,		0	},	{	{	fcol,		frow		}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	},
			{	{	x+8,	y,		0	},	{	{	fcol+size,	frow		}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	},
			{	{	x+8,	y+8,	0	},	{	{	fcol+size,	frow+size	}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	},
			{	{	x,		y+8,	0	},	{	{	fcol,		frow+size	}	},	{	1.0f,	1.0f,	1.0f,	1.0f	}	}
		};

		Video_SetTexture(gCharTexture);
		Video_DrawFill(voCharacter);
	}
}

void Draw_Pic (int x, int y, qpic_t *pic)
{
#if 1
	glpic_t	*gl;

	if(scrap_dirty)
		Scrap_Upload ();

	gl = (glpic_t*)pic->data;

	Video_SetTexture(gl->gltexture);

	glBegin (GL_QUADS);
	glTexCoord2f (gl->sl, gl->tl);
	glVertex2f (x, y);
	glTexCoord2f (gl->sh, gl->tl);
	glVertex2f (x+pic->width, y);
	glTexCoord2f (gl->sh, gl->th);
	glVertex2f (x+pic->width, y+pic->height);
	glTexCoord2f (gl->sl, gl->th);
	glVertex2f (x, y+pic->height);
	glEnd();
#endif
}

/*	Only used for the player color selection menu
*/
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, int top, int bottom)
{
	static int oldtop = -2,oldbottom = -2;

	if(top != oldtop || bottom != oldbottom)
	{
		oldtop		= top;
		oldbottom	= bottom;

		TexMgr_ReloadImage(((glpic_t*)pic->data)->gltexture,top,bottom);
	}

	Draw_Pic(x,y,pic);
}

void Draw_ConsoleBackground(void)
{
	float fAlpha = 1.0f;

	GL_SetCanvas(CANVAS_CONSOLE);

	if(cls.state == ca_connected)
		fAlpha = cvConsoleAlpha.value;

	// [27/5/2013] Simplified this check ~hogsy
	if(!cvConsoleBackground.string[0])
		Draw_Fill(0,0,vid.conwidth,vid.conheight,0,0,0,fAlpha);
	else
		Draw_ExternPic(cvConsoleBackground.string,fAlpha,0,0,vid.conwidth,vid.conheight);
}

/*	This repeats a 64*64 tile graphic to fill the screen around a sized down
	refresh window.
*/
void Draw_TileClear (int x, int y, int w, int h)
{
	glpic_t	*gl;

	gl = (glpic_t *)draw_backtile->data;

	glColor3f(1,1,1);

	Video_SetTexture(gl->gltexture);

	glBegin(GL_QUADS);
	glTexCoord2f(x/64.0,y/64.0);
	glVertex2f(x,y);
	glTexCoord2f((x+w)/64.0,y/64.0);
	glVertex2f(x+w,y);
	glTexCoord2f((x+w)/64.0,(y+h)/64.0);
	glVertex2f(x+w,y+h);
	glTexCoord2f(x/64.0,(y+h)/64.0);
	glVertex2f(x,y+h);
	glEnd();
}

void Draw_Fill(int x,int y,int w,int h,float r,float g,float b,float alpha)
{
	vec4_t			vColour;
	VideoObject_t	voFill[]=
	{
		{	{	x,		y,		0	}	},
		{	{	x+w,	y,		0	}	},
		{	{	x+w,	y+h,	0	}	},
		{	{	x,		y+h,	0	}	}
	};

	vColour[0] = r; vColour[1] = g; vColour[2] = b; vColour[3] = alpha;
	Math_Vector4Copy(vColour,voFill[0].vColour);
	Math_Vector4Copy(vColour,voFill[1].vColour);
	Math_Vector4Copy(vColour,voFill[2].vColour);
	Math_Vector4Copy(vColour,voFill[3].vColour);

	Video_DrawFill(voFill);
}

void Draw_FadeScreen (void)
{
	VideoObject_t	voFade[]=
	{
		{	{	0,			0,			0,	},	{{0}},	{	1.0f,	1.0f,	1.0f,	0.5f	}	},
		{	{	glwidth,	0,			0,	},	{{0}},	{	1.0f,	1.0f,	1.0f,	0.5f	}	},
		{	{	glwidth,	glheight,	0,	},	{{0}},	{	1.0f,	1.0f,	1.0f,	0.5f	}	},
		{	{	0,			glheight,	0,	},	{{0}},	{	1.0f,	1.0f,	1.0f,	0.5f	}	},
	};

	GL_SetCanvas(CANVAS_DEFAULT);

	Video_DrawFill(voFade);
}

/*	Draws the little blue disc in the corner of the screen.
	Call before beginning any disc IO.
*/
void Draw_BeginDisc(void)
{
	int			iViewport[4]; //johnfitz
	canvastype	oldcanvas; //johnfitz

	//johnfitz -- canvas and matrix stuff
	glGetIntegerv(GL_VIEWPORT,iViewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	oldcanvas = (canvastype)currentcanvas;
	GL_SetCanvas (CANVAS_TOPRIGHT);
	currentcanvas = oldcanvas; // a bit of a hack, since GL_SetCanvas doesn't know we are going to pop the stack
	//johnfitz

	glDrawBuffer(GL_FRONT);
	Draw_ExternPic("textures/sprites/disc",1.0f,320-32,0,32,32);
	glDrawBuffer(GL_BACK);

	//johnfitz -- restore everything so that 3d rendering isn't fucked up
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glViewport(iViewport[0],iViewport[1],iViewport[2],iViewport[3]);
	//johnfitz
}

void GL_SetCanvas (int newcanvas)
{
	extern vrect_t scr_vrect;
	float s;
	int lines;

	if (newcanvas == currentcanvas)
		return;

	currentcanvas = newcanvas;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();

	switch(newcanvas)
	{
	case CANVAS_DEFAULT:
		glOrtho (0, glwidth, glheight, 0, -99999, 99999);
		glViewport (glx, gly, glwidth, glheight);
		break;
	case CANVAS_CONSOLE:
		lines = vid.conheight - (scr_con_current * vid.conheight / glheight);
		glOrtho(0,vid.conwidth,vid.conheight + lines,lines,-99999,99999);
		glViewport (glx, gly, glwidth, glheight);
		break;
	case CANVAS_MENU:
		s = Math_Min((float)glwidth / 320.0, (float)glheight / 200.0);
		s = CLAMP (1.0, scr_menuscale.value, s);
		glOrtho (0, 320, 200, 0, -99999, 99999);
		glViewport (glx, gly, glwidth, glheight);
		break;
	case CANVAS_SBAR:
		s = CLAMP (1.0, scr_sbarscale.value, (float)glwidth / 320.0);
		if (cl.gametype == GAME_DEATHMATCH)
		{
			glOrtho (0, glwidth / s, 48, 0, -99999, 99999);
			glViewport (glx, gly, glwidth, 48*s);
		}
		else
		{
			glOrtho (0, 320, 48, 0, -99999, 99999);
			glViewport (glx + (glwidth - 320*s) / 2, gly, 320*s, 48*s);
		}
		break;
	case CANVAS_WARPIMAGE:
		glOrtho (0, 128, 0, 128, -99999, 99999);
		glViewport (glx, gly+glheight-gl_warpimagesize, gl_warpimagesize, gl_warpimagesize);
		break;
	case CANVAS_CROSSHAIR: //0,0 is center of viewport
		s = CLAMP (1.0, scr_crosshairscale.value, 10.0);
		glOrtho (scr_vrect.width/-2/s, scr_vrect.width/2/s, scr_vrect.height/2/s, scr_vrect.height/-2/s, -99999, 99999);
		glViewport (scr_vrect.x, glheight - scr_vrect.y - scr_vrect.height, scr_vrect.width & ~1, scr_vrect.height & ~1);
		break;
	case CANVAS_BOTTOMLEFT: //used by devstats
		s = (float)glwidth/vid.conwidth; //use console scale
		glOrtho (0, 320, 200, 0, -99999, 99999);
		glViewport (glx, gly, 320*s, 200*s);
		break;
	case CANVAS_BOTTOMRIGHT: //used by fps/clock
		s = (float)glwidth/vid.conwidth; //use console scale
		glOrtho (0, 320, 200, 0, -99999, 99999);
		glViewport (glx+glwidth-320*s, gly, 320*s, 200*s);
		break;
	case CANVAS_TOPRIGHT: //used by disc
		s = 1;
		glOrtho (0, 320, 200, 0, -99999, 99999);
		glViewport (glx+glwidth-320*s, gly+glheight-200*s, 320*s, 200*s);
		break;
	default:
		Sys_Error ("GL_SetCanvas: bad canvas type");
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
}

void Draw_ResetCanvas(void)
{
	currentcanvas = (canvastype)-1;

	GL_SetCanvas(CANVAS_DEFAULT);

	Video_DisableCapabilities(VIDEO_BLEND|VIDEO_CULL_FACE|VIDEO_DEPTH_TEST);
	Video_EnableCapabilities(VIDEO_ALPHA_TEST);

	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

#if 0
void GL_DrawSpriteModel(entity_t *e)
{
	vec3_t			point, v_forward, v_right, v_up;
	msprite_t		*psprite;
	mspriteframe_t	*frame;
	float			*s_up, *s_right;
	float			angle, sr, cr;

	frame = R_GetSpriteFrame (e);
	psprite = currententity->model->cache.data;

	switch(psprite->type)
	{
	case SPR_VP_PARALLEL_UPRIGHT: //faces view plane, up is towards the heavens
		v_up[0] = 0;
		v_up[1] = 0;
		v_up[2] = 1;
		s_up = v_up;
		s_right = vright;
		break;
	case SPR_FACING_UPRIGHT: //faces camera origin, up is towards the heavens
		Math_VectorSubtract(currententity->origin, r_origin, v_forward);
		v_forward[2] = 0;
		Math_VectorNormalizeFast(v_forward);
		v_right[0] = v_forward[1];
		v_right[1] = -v_forward[0];
		v_right[2] = 0;
		v_up[0] = 0;
		v_up[1] = 0;
		v_up[2] = 1;
		s_up = v_up;
		s_right = v_right;
		break;
	case SPR_VP_PARALLEL: //faces view plane, up is towards the top of the screen
		s_up = vup;
		s_right = vright;
		break;
	case SPR_ORIENTED: //pitch yaw roll are independent of camera
		Math_AngleVectors(currententity->angles, v_forward, v_right, v_up);
		s_up = v_up;
		s_right = v_right;
		GL_PolygonOffset(OFFSET_DECAL);
		break;
	case SPR_VP_PARALLEL_ORIENTED: //faces view plane, but obeys roll value
		angle = currententity->angles[ROLL] * M_PI_DIV_180;
		sr = sin(angle);
		cr = cos(angle);
		v_right[0] = vright[0] * cr + vup[0] * sr;
		v_right[1] = vright[1] * cr + vup[1] * sr;
		v_right[2] = vright[2] * cr + vup[2] * sr;
		v_up[0] = vright[0] * -sr + vup[0] * cr;
		v_up[1] = vright[1] * -sr + vup[1] * cr;
		v_up[2] = vright[2] * -sr + vup[2] * cr;
		s_up = v_up;
		s_right = v_right;
		break;
	default:
		return;
	}

	glColor3f (1,1,1);

	GL_DisableMultitexture();

	Video_SetTexture(frame->gltexture);

	glEnable (GL_ALPHA_TEST);
	glBegin (GL_TRIANGLE_FAN); //was GL_QUADS, but changed to support r_showtris

	glTexCoord2f (0, frame->tmax);
	Math_VectorMA(e->origin, frame->down, s_up, point);
	Math_VectorMA (point, frame->left, s_right, point);
	glVertex3fv (point);

	glTexCoord2f (0, 0);
	Math_VectorMA (e->origin, frame->up, s_up, point);
	Math_VectorMA (point, frame->left, s_right, point);
	glVertex3fv (point);

	glTexCoord2f (frame->smax, 0);
	Math_VectorMA (e->origin, frame->up, s_up, point);
	Math_VectorMA (point, frame->right, s_right, point);
	glVertex3fv (point);

	glTexCoord2f (frame->smax, frame->tmax);
	Math_VectorMA (e->origin, frame->down, s_up, point);
	Math_VectorMA (point, frame->right, s_right, point);
	glVertex3fv (point);

	glEnd ();
	glDisable (GL_ALPHA_TEST);

	//johnfitz: offset decals
	if (psprite->type == SPR_ORIENTED)
		GL_PolygonOffset (OFFSET_NONE);
}
#endif
