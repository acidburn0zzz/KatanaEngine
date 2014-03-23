#include "gipl\include\gipl.h"

#include "shared_flags.h"

#include "cmdlib.h"
#include "scriplib.h"
#include "wadlib.h"

extern  byte    *byteimage, *lbmpalette;
extern  int     byteimagewidth, byteimageheight;

#define SCRN(x,y)       (*(byteimage+(y)*byteimagewidth+x))

extern  byte    *lump_p;
extern  byte	*lumpbuffer;

extern	char	lumpname[];

