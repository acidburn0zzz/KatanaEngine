/*	Copyright (C) 1999 Mete Ciragan
	Copyright (C) 2011-2014 OldTimes Software

	The programs and associated files contained in this
	distribution were developed by Mete Ciragan. The programs
	are not in the public domain, but they are freely
	distributable without licensing fees. These programs are
	provided without guarantee or warrantee expressed or
	implied.
*/
#include <mx/mxTga.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

mxImage *mxTgaRead (const char *filename)
{
	int				columns, rows, numPixels;
	byte			*pixbuf,*targa_rgba;
	int				row, column;
	TargaHeader_t	targa_header;
	FILE			*fTGAFile;

	fTGAFile = fopen(filename,"rb");
	if(!fTGAFile)
		return 0;

	targa_header.id_length			= fgetc(fTGAFile);
	targa_header.colormap_type		= fgetc(fTGAFile);
	targa_header.image_type			= fgetc(fTGAFile);
	targa_header.colormap_index		= (short)(fgetc(fTGAFile)+fgetc(fTGAFile)*256);
	targa_header.colormap_length	= (short)(fgetc(fTGAFile)+fgetc(fTGAFile)*256);
	targa_header.colormap_size		= fgetc(fTGAFile);
	targa_header.x_origin			= (short)(fgetc(fTGAFile)+fgetc(fTGAFile)*256);
	targa_header.y_origin			= (short)(fgetc(fTGAFile)+fgetc(fTGAFile)*256);
	targa_header.width				= (short)(fgetc(fTGAFile)+fgetc(fTGAFile)*256);
	targa_header.height				= (short)(fgetc(fTGAFile)+fgetc(fTGAFile)*256);
	targa_header.pixel_size			= fgetc(fTGAFile);
	targa_header.attributes			= fgetc(fTGAFile);

	if(	((targa_header.image_type != 2) && (targa_header.image_type != 10)) ||
		(targa_header.colormap_type != 0)									|| 
		(targa_header.pixel_size != 32 && targa_header.pixel_size != 24)) 
	{
		fclose(fTGAFile);
		return 0;
	}

	columns		= targa_header.width;
	rows		= targa_header.height;
	numPixels	= columns*rows;

	targa_rgba = (unsigned char*)malloc(numPixels*4);

	// Skip header comment
	if (targa_header.id_length != 0)
		fseek(fTGAFile,targa_header.id_length,SEEK_CUR);
	
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
					blue		= getc(fTGAFile);
					green		= getc(fTGAFile);
					red			= getc(fTGAFile);
					*pixbuf++	= red;
					*pixbuf++	= green;
					*pixbuf++	= blue;
					*pixbuf++	= 255;
					break;
				default:
					blue		= getc(fTGAFile);
					green		= getc(fTGAFile);
					red			= getc(fTGAFile);
					alphabyte	= getc(fTGAFile);
					*pixbuf++	= red;
					*pixbuf++	= green;
					*pixbuf++	= blue;
					*pixbuf++	= alphabyte;
					break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) 
	{   
		// Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) 
			{
				packetHeader	= getc(fTGAFile);
				packetSize		= 1+(packetHeader & 0x7f);

				if (packetHeader & 0x80) 
				{        
					// run-length packet
					switch (targa_header.pixel_size) 
					{
						case 24:
								blue		= getc(fTGAFile);
								green		= getc(fTGAFile);
								red			= getc(fTGAFile);
								alphabyte	= 255;
								break;
						default:
								blue		= getc(fTGAFile);
								green		= getc(fTGAFile);
								red			= getc(fTGAFile);
								alphabyte	= getc(fTGAFile);
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
				else 
				{                           
					// non run-length packet
					for(j=0;j<packetSize;j++) 
					{
						switch (targa_header.pixel_size) 
						{
							case 24:
									blue		= getc(fTGAFile);
									green		= getc(fTGAFile);
									red			= getc(fTGAFile);
									*pixbuf++	= red;
									*pixbuf++	= green;
									*pixbuf++	= blue;
									*pixbuf++	= 255;
									break;
							default:
									blue		= getc(fTGAFile);
									green		= getc(fTGAFile);
									red			= getc(fTGAFile);
									alphabyte	= getc(fTGAFile);
									*pixbuf++	= red;
									*pixbuf++	= green;
									*pixbuf++	= blue;
									*pixbuf++	= alphabyte;
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

	fclose(fTGAFile);

	mxImage *miTGAImage = new mxImage();
	if(!miTGAImage->create((int)targa_header.width,(int)targa_header.height,targa_header.pixel_size))
	{
		delete miTGAImage;
		return 0;
	}

	miTGAImage->data = targa_rgba;

	return miTGAImage;
}

bool mxTgaWrite (const char *filename, mxImage *image)
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