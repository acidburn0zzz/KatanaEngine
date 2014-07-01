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
#ifndef __ENGINEDRAW__
#define __ENGINEDRAW__

// draw.h -- these are the only functions outside the refresh allowed
// to touch the vid buffer

#include "screen.h"

typedef struct
{
	int			width, height;
	byte		data[4];			// variably sized
} qpic_t;

extern	qpic_t		*draw_disc;	// also used on sbar

void Draw_Init (void);
void Draw_Character (int x, int y, int num);
void Draw_Pic (int x, int y, qpic_t *pic);
void Draw_ConsoleBackground (void); //johnfitz -- removed parameter int lines
void Draw_BeginDisc (void);
void Draw_TileClear (int x,int y,int w,int h);
void Draw_Fill(int x,int y,int w,int h,float r,float g,float b,float alpha);
void Draw_FadeScreen(void);

qpic_t *Draw_CachePic(char *path);

void Draw_ExternPic(char *path,float alpha,int x,int y,int width,int height);

void GL_SetCanvas(int newcanvas); //johnfitz

#endif
