/*  Copyright (C) 2011-2014 OldTimes Software
*/
#include "editor_main.h"
#include "io.h"

static unsigned	tex_palette[256];

static qtexture_t	*notexture;

static bool	nomips;

#define	FONT_HEIGHT	10

static HGLRC s_hglrcTexture;
static HDC	 s_hdcTexture;

int		texture_mode				= GL_LINEAR_MIPMAP_LINEAR;
int		texture_extension_number	= 1;

// current active texture directory.  if empty, show textures in use
char		texture_directory[32];	// use if texture_showinuse is FALSE
bool	texture_showinuse;

// texture layout functions
qtexture_t	*current_texture;
int			current_x, current_y, current_row;

int			texture_nummenus;
#define		MAX_TEXTUREDIRS	100
char		texture_menunames[MAX_TEXTUREDIRS][64];

bool	g_dontuse;		// set to TRUE to load the texture but not flag as used

void SelectTexture (int mx, int my);

void	Texture_MouseDown (int x, int y, int buttons);
void	Texture_MouseMoved (int x, int y, int buttons);

//=====================================================

void SortTextures(void)
{
	qtexture_t	*q, *qtemp, *qhead, *qcur, *qprev;

	// standard insertion sort
	// Take the first texture from the list and
	// add it to our new list
	if ( g_qeglobals.d_qtextures == NULL)
		return;

	qhead = g_qeglobals.d_qtextures;
	q = g_qeglobals.d_qtextures->next;
	qhead->next = NULL;

	// while there are still things on the old
	// list, keep adding them to the new list
	while (q)
	{
		qtemp = q;
		q = q->next;

		qprev = NULL;
		qcur = qhead;

		while (qcur)
		{
			// Insert it here?
			if (strcmp(qtemp->name, qcur->name) < 0)
			{
				qtemp->next = qcur;
				if (qprev)
					qprev->next = qtemp;
				else
					qhead = qtemp;
				break;
			}

			// Move on

			qprev = qcur;
			qcur = qcur->next;


			// is this one at the end?

			if (qcur == NULL)
			{
				qprev->next = qtemp;
				qtemp->next = NULL;
			}
		}


	}

	g_qeglobals.d_qtextures = qhead;
}

void SetTexParameters (void)
{
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,texture_mode);

	switch(texture_mode)
	{
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		break;
	case GL_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		break;
	}
}

void Texture_SetMode(int iMenu)
{
	int			i,iMode=0;
	HMENU		hMenu;
	qboolean	texturing = TRUE;

	hMenu = GetMenu(g_qeglobals.d_hwndMain);
	switch(iMenu)
	{
	case ID_VIEW_NEAREST:
		iMode = GL_NEAREST;
		break;
	case ID_VIEW_NEARESTMIPMAP:
		iMode = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ID_VIEW_LINEAR:
		iMode = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ID_VIEW_BILINEAR:
		iMode = GL_LINEAR;
		break;
	case ID_VIEW_BILINEARMIPMAP:
		iMode = GL_LINEAR_MIPMAP_NEAREST;
		break;
	case ID_VIEW_TRILINEAR:
		iMode = GL_LINEAR_MIPMAP_LINEAR;
		break;
	// [8/8/2012] Merged since they both do the same thing here ~hogsy
	case ID_TEXTURES_WIREFRAME:
	case ID_TEXTURES_FLATSHADE:
		iMode = 0;
		texturing = FALSE;
		break;
	}

	CheckMenuItem(hMenu, ID_VIEW_NEAREST, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_NEARESTMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEARMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_TRILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_WIREFRAME, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_FLATSHADE, MF_BYCOMMAND | MF_UNCHECKED);

	CheckMenuItem(hMenu, iMenu, MF_BYCOMMAND | MF_CHECKED);

	g_qeglobals.d_savedinfo.iTexMenu = iMenu;
	texture_mode = iMode;
	if ( texturing )
		SetTexParameters ();

	if ( !texturing && iMenu == ID_TEXTURES_WIREFRAME)
	{
		camera.draw_mode = cd_wire;
		Map_BuildBrushData();
		Sys_UpdateWindows (W_ALL);
		return;
	}
	else if ( !texturing && iMenu == ID_TEXTURES_FLATSHADE)
	{

		camera.draw_mode = cd_solid;
		Map_BuildBrushData();
		Sys_UpdateWindows (W_ALL);
		return;
	}

	for (i=1 ; i<texture_extension_number ; i++)
	{
		glBindTexture( GL_TEXTURE_2D, i );
		SetTexParameters ();
	}

	// select the default texture
	glBindTexture(GL_TEXTURE_2D,0);
	glFinish();

	if (camera.draw_mode != cd_texture)
	{
		camera.draw_mode = cd_texture;
		Map_BuildBrushData();
	}

	Sys_UpdateWindows (W_ALL);
}

typedef struct targaheader_s {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} targaheader_t;

#define TARGAHEADERSIZE 18 //size on disk

targaheader_t targa_header;

int fgetLittleShort (FILE *f)
{
	byte	b1, b2;

	b1 = fgetc(f);
	b2 = fgetc(f);

	return (short)(b1 + b2*256);
}

int fgetLittleLong (FILE *f)
{
	byte	b1, b2, b3, b4;

	b1 = fgetc(f);
	b2 = fgetc(f);
	b3 = fgetc(f);
	b4 = fgetc(f);

	return b1 + (b2<<8) + (b3<<16) + (b4<<24);
}

qtexture_t *Texture_LoadTexture(FILE *file)	//miptex_t *qtex)
{
	qtexture_t		*qTexture;
	int				columns,rows,numPixels,row,column,realrow;
	byte			*pixbuf,*targa_rgba;
	BOOL			upside_down; //johnfitz -- fix for upside-down targas

	targa_header.id_length			= fgetc(file);
	targa_header.colormap_type		= fgetc(file);
	targa_header.image_type			= fgetc(file);
	targa_header.colormap_index		= fgetLittleShort(file);
	targa_header.colormap_length	= fgetLittleShort(file);
	targa_header.colormap_size		= fgetc(file);
	targa_header.x_origin			= fgetLittleShort(file);
	targa_header.y_origin			= fgetLittleShort(file);
	targa_header.width				= fgetLittleShort(file);
	targa_header.height				= fgetLittleShort(file);
	targa_header.pixel_size			= fgetc(file);
	targa_header.attributes			= fgetc(file);

	if(targa_header.image_type != 2 && targa_header.image_type != 10)
		Error("Image_LoadTGA:  not a type 2 or type 10 targa");
	else if(targa_header.colormap_type != 0 || (targa_header.pixel_size != 32 && targa_header.pixel_size != 24))
		Error("Image_LoadTGA:  not a 24bit or 32bit targa");

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;
	upside_down = !(targa_header.attributes & 0x20); //johnfitz -- fix for upside-down targas

	targa_rgba = (byte*)malloc(numPixels*4);	//Hunk_Alloc(numPixels*4);

	if(targa_header.id_length != 0)
		fseek(file,targa_header.id_length,SEEK_CUR);  // skip TARGA image comment

	if(targa_header.image_type == 2) // Uncompressed, RGB images
	{
		for(row=rows-1; row>=0; row--)
		{
			//johnfitz -- fix for upside-down targas
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow*columns*4;
			//johnfitz
			for(column=0; column<columns; column++)
			{
				unsigned char red=0,green=0,blue=0,alphabyte=0;
				switch (targa_header.pixel_size)
				{
				case 24:
					blue	= getc(file);
					green	= getc(file);
					red		= getc(file);
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue	= getc(file);
					green	= getc(file);
					red		= getc(file);
					alphabyte = getc(file);
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				}
			}
		}
	}
	else if(targa_header.image_type == 10) // Runlength encoded RGB images
	{
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--)
		{
			//johnfitz -- fix for upside-down targas
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow*columns*4;
			//johnfitz
			for(column=0; column<columns; )
			{
				packetHeader=getc(file);
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) // run-length packet
				{
					switch (targa_header.pixel_size)
					{
					case 24:
							blue	= getc(file);
							green	= getc(file);
							red		= getc(file);
							alphabyte = 255;
							break;
					case 32:
							blue	= getc(file);
							green	= getc(file);
							red		= getc(file);
							alphabyte = getc(file);
							break;
					}

					for(j=0;j<packetSize;j++)
					{
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) // run spans across rows
						{
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;

							//johnfitz -- fix for upside-down targas
							realrow = upside_down ? row : rows - 1 - row;
							pixbuf = targa_rgba + realrow*columns*4;
							//johnfitz
						}
					}
				}
				else // non run-length packet
				{
					for(j=0;j<packetSize;j++)
					{
						switch (targa_header.pixel_size)
						{
						case 24:
								blue	= getc(file);
								green	= getc(file);
								red		= getc(file);
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = 255;
								break;
						case 32:
								blue	= getc(file);
								green	= getc(file);
								red		= getc(file);
								alphabyte = getc(file);
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = alphabyte;
								break;
						}
						column++;
						if (column==columns) // pixel packet run spans across rows
						{
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							//johnfitz -- fix for upside-down targas
							realrow = upside_down ? row : rows - 1 - row;
							pixbuf = targa_rgba + realrow*columns*4;
							//johnfitz
						}
					}
				}
			}
			breakOut:;
		}
	}

	fclose(file);

	qTexture = (qtexture_t*)qmalloc(sizeof(*qTexture));

	qTexture->height			= (int)(targa_header.height);
	qTexture->width				= (int)(targa_header.width);
	qTexture->color[0]			= 1.0f;
	qTexture->color[1]			= 1.0f;
	qTexture->color[2]			= 1.0f;
	qTexture->contents			= 0;
	qTexture->value				= 0;
	qTexture->flags				= 0;
	qTexture->texture_number	= texture_extension_number++;

	glBindTexture(GL_TEXTURE_2D,qTexture->texture_number);

	SetTexParameters();

	if (nomips)
		glTexImage2D(GL_TEXTURE_2D,0,3,qTexture->width,qTexture->height,0,GL_RGBA,GL_UNSIGNED_BYTE,targa_rgba);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D,3,qTexture->width,qTexture->height,GL_RGBA,GL_UNSIGNED_BYTE,targa_rgba);

	glBindTexture(GL_TEXTURE_2D,0);

	return qTexture;
}

/*	Create a single pixel texture of the apropriate color
*/
qtexture_t *Texture_CreateSolid (char *name)
{
	byte		data[4];
	qtexture_t	*q;

	q = (qtexture_t*)qmalloc(sizeof(*q));

	sscanf (name, "(%f %f %f)", &q->color[0], &q->color[1], &q->color[2]);

	data[0] = q->color[0]*255;
	data[1] = q->color[1]*255;
	data[2] = q->color[2]*255;
	data[3] = 255;

	q->width = q->height = 1;
	q->texture_number = texture_extension_number++;
	glBindTexture( GL_TEXTURE_2D, q->texture_number );
	SetTexParameters ();

	if (nomips)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 1, 1,GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture( GL_TEXTURE_2D, 0 );

	return q;
}

void Texture_MakeNotexture (void)
{
	qtexture_t	*q;
	byte		data[4][4];

	notexture = q = qmalloc(sizeof(*q));
	strcpy (q->name, "notexture");
	q->width = q->height = 64;

	memset (data, 0, sizeof(data));
	data[0][2] = data[3][2] = 255;

	q->color[0] = 0;
	q->color[1] = 0;
	q->color[2] = 0.5;

	q->texture_number = texture_extension_number++;
	glBindTexture( GL_TEXTURE_2D, q->texture_number );
	SetTexParameters ();

	if (nomips)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 2, 2,GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture( GL_TEXTURE_2D, 0 );
}

qtexture_t *Texture_ForName (char *name)
{
	FILE		*file;
	qtexture_t	*q;
	char		filename[1024];

	for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
		if (!strcmp(name,  q->name))
		{
			if (!g_dontuse)
				q->inuse = TRUE;
			return q;
		}

	if(name[0] == '(')
	{
		q = Texture_CreateSolid (name);
		strncpy (q->name, name, sizeof(q->name)-1);
	}
	else
	{
		// [28/8/2012] Quick fix for "liquid"/special textures which use * ~hogsy
		if(name[0] == '*')
			sprintf(filename,"%s/#%s.tga",ValueForKey(g_qeglobals.d_project_entity,"$texturepath"),name+1);
		else
			sprintf(filename,"%s/%s.tga",ValueForKey(g_qeglobals.d_project_entity,"$texturepath"),name);

		file = fopen(filename,"rb");
		if(!file)
		{
			Console_Print(FALSE,"Failed to load %s!\n",filename);
			return notexture;
		}
		else
			q = Texture_LoadTexture(file);

		strncpy (q->name, name, sizeof(q->name)-1);
		StripExtension (q->name);
	}

	if (!g_dontuse)
		q->inuse = true;
	q->next = g_qeglobals.d_qtextures;
	g_qeglobals.d_qtextures = q;

	return q;
}

void FillTextureMenu (void)
{
	HMENU	hmenu;
	int		i;
	struct _finddata_t fileinfo;
	int		handle;
	char	dirstring[1024];
	char	*path;

	hmenu = GetSubMenu(GetMenu(g_qeglobals.d_hwndMain),MENU_TEXTURE);

	// delete everything
	for(i=0; i < texture_nummenus; i++)
		DeleteMenu(hmenu,CMD_TEXTUREWAD+i, MF_BYCOMMAND);

	// add everything
	path = ValueForKey(g_qeglobals.d_project_entity,"$texturepath");

	sprintf(dirstring,"%s/*.*",path);

	handle = _findfirst (dirstring, &fileinfo);
	if (handle == -1)
		return;

	do
	{
		if (!(fileinfo.attrib & _A_SUBDIR))
			continue;
		if (fileinfo.name[0] == '.')
			continue;
		// add this directory to the menu
		AppendMenu (hmenu, MF_ENABLED|MF_STRING,
			CMD_TEXTUREWAD+texture_nummenus, (LPCTSTR)fileinfo.name);
		strcpy (texture_menunames[texture_nummenus], fileinfo.name);
		strcat (texture_menunames[texture_nummenus], "/");
		if (++texture_nummenus == MAX_TEXTUREDIRS)
			break;
	} while (_findnext( handle, &fileinfo ) != -1);

	_findclose (handle);
}

/*	A new map is being loaded, so clear inuse markers
*/
void Texture_ClearInuse (void)
{
	qtexture_t	*q;

	for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
		q->inuse = FALSE;
}

void Texture_ShowDirectory(void)
{
	struct	_finddata_t	fileinfo;
	// [23/8/2012] BUG: Have to make i = 0 here otherwise it comes back weird during launch ~hogsy
	int		handle,i=0;
	char	name[1024],cNum[32],dirstring[1024];

	texture_showinuse = FALSE;

	g_qeglobals.d_texturewin.originy = 0;
	Sys_Status("Loading textures...",0);

	// [9/8/2012] Now uses texturepath as the path ~hogsy
	sprintf(dirstring,"%s/*.tga",ValueForKey(g_qeglobals.d_project_entity,"$texturepath"));

	Console_Print(FALSE,"Scanning %s\n", dirstring);

	handle = _findfirst (dirstring, &fileinfo);
	if (handle == -1)
		return;

	g_dontuse = true;
	do
	{
		i++;

		sprintf(name,"%s",fileinfo.name);

		Sys_Status(name,1);
		Sys_Status(itoa(i,cNum,10),2);

		StripExtension(name);
		Texture_ForName(name);
	} while (_findnext( handle, &fileinfo ) != -1);
	g_dontuse = false;

	_findclose (handle);

	SortTextures();

	// [10/8/2012] BUG: Have to reset messages here otherwise they sit around ~hogsy
	Sys_Status("",2);
	Sys_Status("",1);
	Sys_Status("",0);

	// Select the first texture in the list
	if (!g_qeglobals.d_texturewin.texdef.name[0])
		SelectTexture (16, g_qeglobals.d_texturewin.height -16);
}

void Texture_ShowInuse (void)
{
	char	name[1024];
	face_t	*f;
	brush_t	*b;

	texture_showinuse = TRUE;

	g_qeglobals.d_texturewin.originy = 0;
	Sys_Status("Selecting active textures\n", 0);
	Texture_ClearInuse ();

	for (b=active_brushes.next ; b != NULL && b != &active_brushes ; b=b->next)
		for (f=b->brush_faces ; f ; f=f->next)
			Texture_ForName (f->texdef.name);

	for (b=selected_brushes.next ; b != NULL && b != &selected_brushes ; b=b->next)
		for (f=b->brush_faces ; f ; f=f->next)
			Texture_ForName (f->texdef.name);

	SortTextures();
	SetInspectorMode(W_TEXTURE);
	Sys_UpdateWindows (W_TEXTURE);

	sprintf (name, "Textures: in use");
	SetWindowText(g_qeglobals.d_hwndEntity, name);

	// select the first texture in the list
	if (!g_qeglobals.d_texturewin.texdef.name[0])
		SelectTexture (16, g_qeglobals.d_texturewin.height -16);
}

/*
============================================================================

TEXTURE LAYOUT

============================================================================
*/

void Texture_StartPos (void)
{
	current_texture = g_qeglobals.d_qtextures;
	current_x = 8;
	current_y = -8;
	current_row = 0;
}

qtexture_t *Texture_NextPos (int *x, int *y)
{
	qtexture_t	*q;

	while (1)
	{
		q = current_texture;
		if (!q)
			return q;
		current_texture = current_texture->next;
		if (q->name[0] == '(')	// fake color texture
			continue;
		if (q->inuse)
			break;			// allways show in use
		if (!texture_showinuse && strncmp (q->name, texture_directory, strlen(texture_directory)))
			continue;
		break;
	}

	if (current_x + q->width > g_qeglobals.d_texturewin.width-8 && current_row)
	{	// go to the next row unless the texture is the first on the row
		current_x = 8;
		current_y -= current_row + FONT_HEIGHT + 4;
		current_row = 0;
	}

	*x = current_x;
	*y = current_y;

	// Is our texture larger than the row? If so, grow the
	// row height to match it

	if (current_row < q->height)
		current_row = q->height;

	// never go less than 64, or the names get all crunched up
	current_x += q->width < 64 ? 64 : q->width;
	current_x += 8;

	return q;
}

/*
============================================================================

  MOUSE ACTIONS

============================================================================
*/

static	int	textures_cursorx, textures_cursory;

void Texture_SetTexture (texdef_t *texdef)
{
	qtexture_t	*q;
	int			x,y;
	char		sz[256];

	if (texdef->name[0] == '(')
	{
		Sys_Status("Can't select an entity texture\n", 0);
		return;
	}
	g_qeglobals.d_texturewin.texdef = *texdef;

	Sys_UpdateWindows (W_TEXTURE);
	sprintf(sz, "Selected texture: %s\n", texdef->name);
	Sys_Status(sz, 0);
	Select_SetTexture(texdef);

	// Scroll origin so the texture is completely on screen
	Texture_StartPos ();
	for(;;)
	{
		q = Texture_NextPos(&x,&y);
		if(!q)
			break;

		if(!strcmpi(texdef->name, q->name))
		{
			if (y > g_qeglobals.d_texturewin.originy)
			{
				g_qeglobals.d_texturewin.originy = y;
				Sys_UpdateWindows (W_TEXTURE);
				return;
			}

			if (y-q->height-2*FONT_HEIGHT < g_qeglobals.d_texturewin.originy-g_qeglobals.d_texturewin.height)
			{
				g_qeglobals.d_texturewin.originy = y-q->height-2*FONT_HEIGHT+g_qeglobals.d_texturewin.height;
				Sys_UpdateWindows (W_TEXTURE);
				return;
			}

			return;
		}
	}
}

/*	By mouse click
*/
void SelectTexture (int mx, int my)
{
	int			x,y;
	qtexture_t	*q;
	texdef_t	tex;

	my += g_qeglobals.d_texturewin.originy-g_qeglobals.d_texturewin.height;

	Texture_StartPos();

	for(;;)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;

		if (mx > x && mx - x < q->width
			&& my < y && y - my < q->height + FONT_HEIGHT)
		{
			memset (&tex, 0, sizeof(tex));
			tex.scale[0]	= 1;
			tex.scale[1]	= 1;
			tex.flags		= q->flags;
			tex.value		= q->value;
			tex.contents	= q->contents;
			strcpy(tex.name, q->name);

			Texture_SetTexture (&tex);

			return;
		}
	}

	Sys_Status("Did not select a texture\n", 0);
}

void Texture_MouseDown (int x, int y, int buttons)
{
	Sys_GetCursorPos (&textures_cursorx, &textures_cursory);

	// lbutton = select texture
	if (buttons == MK_LBUTTON )
	{
		SelectTexture (x, g_qeglobals.d_texturewin.height - 1 - y);
		return;
	}
}

void Texture_MouseMoved (int x, int y, int buttons)
{
	int scale = 1;

	if ( buttons & MK_SHIFT )
		scale = 4;

	// rbutton = drag texture origin
	if (buttons & MK_RBUTTON)
	{
		Sys_GetCursorPos (&x, &y);
		if ( y != textures_cursory)
		{
			g_qeglobals.d_texturewin.originy += ( y-textures_cursory) * scale;
			if (g_qeglobals.d_texturewin.originy > 0)
				g_qeglobals.d_texturewin.originy = 0;
			Sys_SetCursorPos (textures_cursorx, textures_cursory);
			Sys_UpdateWindows (W_TEXTURE);
		}
		return;
	}
}

/*
============================================================================

DRAWING

============================================================================
*/

int imax(int iFloor, int i) { if (i>iFloor) return iFloor; return i; }
HFONT ghFont = NULL;

void Texture_Draw2(int width, int height)
{
	qtexture_t	*q;
	int			x, y;
	char		*name;

	glClearColor (
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][0],
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][1],
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][2],
		0);
	glViewport (0,0,width,height);
	glClear(GL_COLOR_BUFFER_BIT);
//	glDisable (GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, width, g_qeglobals.d_texturewin.originy-height, g_qeglobals.d_texturewin.originy, -100, 100);
	glEnable (GL_TEXTURE_2D);

//	glPolygonMode(GL_FRONT, GL_LINE);
	g_qeglobals.d_texturewin.width = width;
	g_qeglobals.d_texturewin.height = height;

	Texture_StartPos ();

	for(;;)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;

		// Is this texture visible?
		if ( (y-q->height-FONT_HEIGHT < g_qeglobals.d_texturewin.originy)
			&& (y > g_qeglobals.d_texturewin.originy - height) )
		{
			// if in use, draw a background
			if (q->inuse && !texture_showinuse)
			{
				glLineWidth(1.0f);
				glColor3f(0.5f,1.0f,0.5f);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINE_LOOP);
				glVertex2i(x-1,y+1-FONT_HEIGHT);
				glVertex2i(x-1,y-q->height-1-FONT_HEIGHT);
				glVertex2i(x+1+q->width,y-q->height-1-FONT_HEIGHT);
				glVertex2i(x+1+q->width,y+1-FONT_HEIGHT);
				glEnd();
				glEnable(GL_TEXTURE_2D);
			}

			// Draw the texture
			glColor3f(1.0f,1.0f,1.0f);
			glBindTexture(GL_TEXTURE_2D,q->texture_number);
			glBegin(GL_QUADS);
			glTexCoord2i(0,0);
			glVertex2i(x,y-FONT_HEIGHT);
			glTexCoord2i(1,0);
			glVertex2i(x+q->width,y-FONT_HEIGHT);
			glTexCoord2i(1,1);
			glVertex2i(x+q->width,y-FONT_HEIGHT-q->height);
			glTexCoord2i(0,1);
			glVertex2i(x,y-FONT_HEIGHT-q->height);
			glEnd();
			// [9/8/2012] Unbind here otherwise it messes up our text ~hogsy
			glBindTexture(GL_TEXTURE_2D,0);

			// draw the selection border
			if (!strcmpi(g_qeglobals.d_texturewin.texdef.name, q->name))
			{
				glLineWidth(1.0f);
				glColor3f(1,0,0);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINE_LOOP);
				glVertex2i(x-2,y-FONT_HEIGHT+2);
				glVertex2i(x-2,y-FONT_HEIGHT-q->height-2);
				glVertex2i(x+2+q->width,y-FONT_HEIGHT-q->height-2);
				glVertex2i(x+2+q->width,y-FONT_HEIGHT+2);
				glEnd();
				glEnable(GL_TEXTURE_2D);
				glLineWidth(1.0f);
			}

			// draw the texture name
			glColor3f(0,1.0f,0);
			glRasterPos2f(x,y-FONT_HEIGHT+2);

			// don't draw the directory name
			for (name = q->name ; *name && *name != '/' && *name != '\\' ; name++)
				;
			if (!*name)
				name = q->name;
			else
				name++;
			glCallLists (strlen(name), GL_UNSIGNED_BYTE, name);
		}
	}

	// reset the current texture
	glBindTexture(GL_TEXTURE_2D,0);
	glFinish();
}

LONG WINAPI WTex_WndProc (
	HWND    hWnd,
	UINT    uMsg,
	WPARAM  wParam,
	LPARAM  lParam)
{
	int		xPos, yPos;
	RECT	rect;

	GetClientRect(hWnd, &rect);

	switch (uMsg)
	{
	case WM_CREATE:
#if 0
		s_hdcTexture = GetDC(hWnd);
		QEW_SetupPixelFormat(s_hdcTexture, FALSE);

		if ( ( s_hglrcTexture = wglCreateContext( s_hdcTexture ) ) == 0 )
			Error( "wglCreateContext in WTex_WndProc failed" );

		if (!wglMakeCurrent( s_hdcTexture, s_hglrcTexture ))
			Error ("wglMakeCurrent in WTex_WndProc failed");

		if (!wglShareLists( g_qeglobals.d_hglrcBase, s_hglrcTexture ) )
			Error( "wglShareLists in WTex_WndProc failed" );
#endif

		return 0;
	case WM_DESTROY:
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( s_hglrcTexture );
		ReleaseDC( hWnd, s_hdcTexture );
		return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT	ps;

			BeginPaint(hWnd, &ps);

			if ( !wglMakeCurrent( s_hdcTexture, s_hglrcTexture ) )
				Error ("wglMakeCurrent failed");
			Texture_Draw2 (rect.right-rect.left, rect.bottom-rect.top);
			SwapBuffers(s_hdcTexture);

			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		SetCapture( g_qeglobals.d_hwndTexture );
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor
		yPos = (short)HIWORD(lParam);  // vertical position of cursor

		Texture_MouseDown (xPos, yPos, wParam);
		return 0;
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor
		yPos = (short)HIWORD(lParam);  // vertical position of cursor
		if (! (wParam & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
			ReleaseCapture ();
		return 0;
	case WM_MOUSEMOVE:
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor
		yPos = (short)HIWORD(lParam);  // vertical position of cursor

		Texture_MouseMoved (xPos, yPos, wParam);
		return 0;
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);
}

/*	We need to create a seperate window for the textures
	in the inspector window, because we can't share
	gl and gdi drawing in a single window
*/
#define	TEXTURE_WINDOW_CLASS	"QTEX"
HWND CreateTextureWindow(void)
{
	WNDCLASSEX	wc;
	HWND		hwnd;

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= 0;
	wc.lpfnWndProc		= (WNDPROC)WTex_WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= g_qeglobals.d_hInstance;
	wc.hIcon			= 0;
	wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= TEXTURE_WINDOW_CLASS;
	wc.hIconSm			= 0;

	if(!RegisterClassEx(&wc))
		Error ("WCam_Register: failed");

	hwnd = CreateWindowEx(
		0,
		TEXTURE_WINDOW_CLASS,
		"Texture View",
		WS_BORDER|WS_CHILD|WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,	// size
		g_qeglobals.d_hwndEntity,	// parent window
		0,		// no menu
		g_qeglobals.d_hInstance,
		0);
	if (!hwnd)
		Error ("Couldn't create texturewindow");

	return hwnd;
}

void Texture_Flush (void)
{
}

void Texture_Initialize(void)
{
	// create the fallback texture
	Texture_MakeNotexture();

	g_qeglobals.d_qtextures = NULL;

	Sys_BeginWait();

	Texture_ShowDirectory();
}

