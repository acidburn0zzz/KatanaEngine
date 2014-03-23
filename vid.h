/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others

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

// a pixel can be one, two, or four bytes
typedef byte pixel_t;

typedef struct vrect_s
{
	int				x,y,width,height;
	struct vrect_s	*pnext;
} vrect_t;

// [6/11/2013] OBSOLETE! ~hogsy
typedef struct
{
	bool			bRecalcRefDef;	// if TRUE, recalc vid-based stuff

	unsigned		rowbytes;		// may be > width if displayed in a window
	unsigned		bpp;
	unsigned		conwidth;
	unsigned		conheight;
} viddef_t;

extern	viddef_t	vid;				// global video state
//extern	unsigned short	d_8to16table[256]; //johnfitz -- never used
extern void (*vid_menudrawfn)(void);
extern void (*vid_menukeyfn)(int key);
extern void (*vid_menucmdfn)(void); //johnfitz

//johnfitz -- deleted VID_SetPalette and VID_ShiftPalette

void	VID_Update (vrect_t *rects);
// flushes the given rectangles from the view buffer to the screen
