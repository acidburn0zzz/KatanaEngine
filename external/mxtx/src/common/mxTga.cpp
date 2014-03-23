//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxTga.cpp
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxTga.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

short	(*_BigShort) (short l);
short	(*_LittleShort) (short l);
int		(*_BigLong) (int l);
int		(*_LittleLong) (int l);
float	(*_BigFloat) (float l);
float	(*_LittleFloat) (float l);

short	BigShort(short l){return _BigShort(l);}
short	LittleShort(short l) {return _LittleShort(l);}
int		BigLong (int l) {return _BigLong(l);}
int		LittleLong (int l) {return _LittleLong(l);}
float	BigFloat (float l) {return _BigFloat(l);}
float	LittleFloat (float l) {return _LittleFloat(l);}

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

#define	Z_MAGIC		0x1d1d


typedef struct zhead_s
{
	struct zhead_s	*prev, *next;
	short	magic;
	short	tag;			// for group free
	int		size;
} zhead_t;

zhead_t		z_chain;
int		z_count, z_bytes;

void Z_Free (void *ptr)
{
	zhead_t	*z;

	z = ((zhead_t *)ptr) - 1;

	if (z->magic != Z_MAGIC)
		return;

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;
	free (z);
}

void Z_FreeTags (int tag)
{
	zhead_t	*z, *next;

	for (z=z_chain.next ; z != &z_chain ; z=next)
	{
		next = z->next;
		if (z->tag == tag)
			Z_Free ((void *)(z+1));
	}
}

void *Z_TagMalloc (int size, int tag)
{
	zhead_t	*z;
	
	size = size + sizeof(zhead_t);
	z = (zhead_t*)malloc(size);
	if (!z)
		return 0;
	memset (z, 0, size);
	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	return (void *)(z+1);
}

void *Z_Malloc (int size)
{
	return Z_TagMalloc (size, 0);
}

int FS_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int FS_FOpenFile (const char *filename, FILE **file)
{		
	*file = fopen (filename, "rb");
	return FS_filelength (*file);
}

#define	MAX_READ	0x10000		// read in blocks of 64k

void FS_Read (void *buffer, int len, FILE *f)
{
	int		block, remaining;
	int		read;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	// read in chunks for progress bar
	remaining = len;
	tries = 0;
	while (remaining)
	{
		block = remaining;
		if (block > MAX_READ)
			block = MAX_READ;
		read = fread (buf, 1, block, f);
		if (read == 0)
		{
			// we might have been trying to read from a CD
			if (!tries)
				tries = 1;
			else
				return;
		}

		if (read == -1)
			return;

		// do some progress bar thing here...

		remaining -= read;
		buf += read;
	}
}

int FS_LoadFile (const char *path, void **buffer)
{
	FILE	*h;
	byte	*buf;
	int		len;

	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = FS_FOpenFile (path, &h);
	if (!h)
	{
		if (buffer)
			*buffer = NULL;
		return -1;
	}
	
	if (!buffer)
	{
		fclose (h);
		return len;
	}

	buf		= (byte*)Z_Malloc(len);
	*buffer = buf;

	FS_Read (buf, len, h);

	fclose (h);

	return len;
}

mxImage *mxTgaRead (const char *filename)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;
	byte tmp[2];
	int *width;
	int *height;
	byte	**pic;

	*pic = NULL;

	//
	// load the file
	//
	length = FS_LoadFile (filename, (void **)&buffer);
	if (!buffer)
		return 0;

	buf_p = buffer;

	targa_header.id_length		= *buf_p++;
	targa_header.colormap_type	= *buf_p++;
	targa_header.image_type		= *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10) 
		goto breakOut;

	if (targa_header.colormap_type !=0 || (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		goto breakOut;

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = (unsigned char*)malloc (numPixels*4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2)
	{ 
		// Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) 
			{
				unsigned char red,green,blue,alphabyte;
				switch(targa_header.pixel_size) 
				{
				case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
				default:
							blue		= *buf_p++;
							green		= *buf_p++;
							red			= *buf_p++;
							alphabyte	= *buf_p++;
							*pixbuf++	= red;
							*pixbuf++	= green;
							*pixbuf++	= blue;
							*pixbuf++	= alphabyte;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) 
			{
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) 
				{        
					// run-length packet
					switch (targa_header.pixel_size) 
					{
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						default:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							default:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
						}
						column++;
						if (column==columns) 
						{ 
							// pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;

							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	Z_Free(buffer);
}



bool
mxTgaWrite (const char *filename, mxImage *image)
{
	if (!image)
		return false;

	if (image->bpp != 24)
		return false;

	FILE *file = fopen (filename, "wb");
	if (!file)
		return false;

	//
	// write header
	//
	fputc (0, file); // identFieldLength
	fputc (0, file); // colorMapType == 0, no color map
	fputc (2, file); // imageTypeCode == 2, uncompressed RGB

	word w = 0;
	fwrite (&w, sizeof (word), 1, file); // colorMapOrigin
	fwrite (&w, sizeof (word), 1, file); // colorMapLength
	fputc (0, file); // colorMapEntrySize

	fwrite (&w, sizeof (word), 1, file); // imageOriginX
	fwrite (&w, sizeof (word), 1, file); // imageOriginY

	w = (word) image->width;
	fwrite (&w, sizeof (word), 1, file); // imageWidth

	w = (word) image->height;
	fwrite (&w, sizeof (word), 1, file); // imageHeight

	fputc (24, file); // imagePixelSize
	fputc (0, file); // imageDescriptorByte

	// write no ident field

	// write no color map

	// write imagedata

	byte *data = (byte *) image->data;
	for (int y = 0; y < image->height; y++)
	{
		byte *scanline = (byte *) &data[(image->height - y - 1) * image->width * 3];
		for (int x = 0; x < image->width; x++)
		{
			fputc ((byte) scanline[x * 3 + 2], file);
			fputc ((byte) scanline[x * 3 + 1], file);
			fputc ((byte) scanline[x * 3 + 0], file);
		}
	}

	fclose (file);

	return true;
}