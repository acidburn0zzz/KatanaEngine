/*	Copyright (C) 1999 Mete Ciragan
	Copyright (C) 2011-2014 OldTimes Software

	The programs and associated files contained in this
	distribution were developed by Mete Ciragan. The programs
	are not in the public domain, but they are freely
	distributable without licensing fees. These programs are
	provided without guarantee or warrantee expressed or
	implied.
*/
#ifndef INCLUDED_MXTGA
#define INCLUDED_MXTGA

#ifndef INCLUDED_MXIMAGE
#include <mx/mxImage.h>
#endif

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader_t;

mxImage *mxTgaRead (const char *filename);
bool mxTgaWrite (const char *filename, mxImage *image);

void *Z_Malloc (int size);

#endif // INCLUDED_MXTGA
