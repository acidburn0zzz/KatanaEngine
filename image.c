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

#include "platform/include/platform_filesystem.h"

#include "shared_png.h"

char loadfilename[MAX_OSPATH]; //file scope so that error messages can use it

/*	Returns a pointer to hunk allocated RGBA data
*/
byte *Image_LoadImage(char *name, unsigned int *width, unsigned int *height)
{
	FILE	*f;

	sprintf(loadfilename,"%s.tga",name);
	COM_FOpenFile(loadfilename,&f);
	if(f)
		return Image_LoadTGA(f,width,height);

	// [26/1/2014] PNG support ~hogsy
	sprintf(loadfilename,"%s.png",name);
	COM_FOpenFile(loadfilename,&f);
	if(f)
	{
		unsigned	int	uiDecode;
		byte			*bImage;

		uiDecode = lodepng_decode32_file(&bImage,(uint32_t*)width,(uint32_t*)height,va("%s/%s",com_gamedir,loadfilename));
		if(uiDecode)
			Con_Warning("Failed to load PNG! (%s) (%s)\n",loadfilename,lodepng_error_text(uiDecode));
		// [26/1/2014] Else because if we have another method we can try after this, let's go for it ~hogsy
		else
			return bImage;
	}

	// [26/1/2014] TODO: JPEG support! ~hogsy

	return NULL;
}

//==============================================================================
//
//  TGA
//
//==============================================================================

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

/*	Writes RGB or RGBA data to a TGA file

	returns TRUE if successful

	TODO: support BGRA and BGR formats (since opengl can return them, and we don't have to swap)
*/
bool Image_WriteTGA(char *name,byte *data,int width,int height,int bpp,bool upsidedown)
{
	int		handle, i, size, temp, bytes;
	char	pathname[MAX_OSPATH];
	byte	header[TARGAHEADERSIZE];

	if(!gFileSystem_CreateDirectory(com_gamedir)) //if we've switched to a nonexistant gamedir, create it now so we don't crash
		Sys_Error("Failed to create directory!\n");
	sprintf (pathname, "%s/%s", com_gamedir, name);
	handle = Sys_FileOpenWrite (pathname);
	if (handle == -1)
		return false;

	Q_memset (&header, 0, TARGAHEADERSIZE);
	header[2]	= 2; // uncompressed type
	header[12]	= width&255;
	header[13]	= width>>8;
	header[14]	= height&255;
	header[15]	= height>>8;
	header[16]	= bpp; // pixel size
	if (upsidedown)
		header[17] = 0x20; //upside-down attribute

	// swap red and blue bytes
	bytes = bpp/8;
	size = width*height*bytes;
	for (i=0; i<size; i+=bytes)
	{
		temp = data[i];
		data[i] = data[i+2];
		data[i+2] = temp;
	}

	Sys_FileWrite (handle, &header, TARGAHEADERSIZE);
	Sys_FileWrite (handle, data, size);
	Sys_FileClose (handle);

	return true;
}

byte *Image_LoadTGA (FILE *fin, unsigned int *width, unsigned int *height)
{
	int		columns,rows,numPixels,row,column,realrow;
	byte	*pixbuf,*targa_rgba;
	bool	upside_down;                                //johnfitz -- fix for upside-down targas

	targa_header.id_length			= fgetc(fin);
	targa_header.colormap_type		= fgetc(fin);
	targa_header.image_type			= fgetc(fin);
	targa_header.colormap_index		= fgetLittleShort(fin);
	targa_header.colormap_length	= fgetLittleShort(fin);
	targa_header.colormap_size		= fgetc(fin);
	targa_header.x_origin			= fgetLittleShort(fin);
	targa_header.y_origin			= fgetLittleShort(fin);
	targa_header.width				= fgetLittleShort(fin);
	targa_header.height				= fgetLittleShort(fin);
	targa_header.pixel_size			= fgetc(fin);
	targa_header.attributes			= fgetc(fin);

	if(targa_header.image_type != 2 && targa_header.image_type != 10)
		Sys_Error("Image_LoadTGA: %s is not a type 2 or type 10 targa\n",loadfilename);
	else if(targa_header.colormap_type != 0 || (targa_header.pixel_size != 32 && targa_header.pixel_size != 24))
		Sys_Error("Image_LoadTGA: %s is not a 24bit or 32bit targa\n",loadfilename);

	columns     = targa_header.width;
	rows        = targa_header.height;
	numPixels   = columns * rows;
	upside_down = !(targa_header.attributes & 0x20); //johnfitz -- fix for upside-down targas

	targa_rgba = (byte*)Hunk_Alloc (numPixels*4);

	if (targa_header.id_length != 0)
		fseek(fin, targa_header.id_length, SEEK_CUR);  // skip TARGA image comment

	if (targa_header.image_type==2) // Uncompressed, RGB images
	{
		for(row=rows-1; row>=0; row--)
		{
			//johnfitz -- fix for upside-down targas
			realrow = upside_down ? row : rows - 1 - row;
			pixbuf = targa_rgba + realrow*columns*4;
			//johnfitz
			for(column=0; column<columns; column++)
			{
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size)
				{
				case 24:
						blue = getc(fin);
						green = getc(fin);
						red = getc(fin);
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = 255;
						break;
				default:
						blue = getc(fin);
						green = getc(fin);
						red = getc(fin);
						alphabyte = getc(fin);
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
			for(column = 0; column < columns;)
			{
				packetHeader    =getc(fin);
				packetSize      = 1+(packetHeader & 0x7f);
				if (packetHeader & 0x80) // run-length packet
				{
					switch (targa_header.pixel_size)
					{
					case 24:
							blue        = getc(fin);
							green       = getc(fin);
							red         = getc(fin);
							alphabyte   = 255;
							break;
					default:
							blue        = getc(fin);
							green       = getc(fin);
							red         = getc(fin);
							alphabyte   = getc(fin);
							break;
					}

					for(j=0;j<packetSize;j++)
					{
						*pixbuf++   = red;
						*pixbuf++   = green;
						*pixbuf++   = blue;
						*pixbuf++   = alphabyte;

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
								blue = getc(fin);
								green = getc(fin);
								red = getc(fin);
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = 255;
								break;
						default:
								blue = getc(fin);
								green = getc(fin);
								red = getc(fin);
								alphabyte = getc(fin);
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

	fclose(fin);

	*width = (int)(targa_header.width);
	*height = (int)(targa_header.height);
	return targa_rgba;
}

/*
	Portable Network Graphics
*/
