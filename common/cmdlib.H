/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __CMDLIB__
#define __CMDLIB__

#include "../platform/include/platform.h"

#include <errno.h>
#include <time.h>

typedef unsigned char byte;

// set these before calling CheckParm
extern int myargc;
extern char **myargv;

char *strlower (char *in);
int Q_strncasecmp (char *s1, char *s2, int n);
int Q_strcasecmp (char *s1, char *s2);

int	FileTime (char *path);

extern	char		qdir[1024];
extern	char		gamedir[1024];
void SetQdirFromPath (char *path);
char *ExpandPath (char *path);
char *ExpandPathAndArchive (char *path);

double I_FloatTime (void);

void	Error (const char *error, ...);
int		CheckParm (char *check);

FILE	*SafeOpenWrite (char *filename);
FILE	*SafeOpenRead (char *filename);
void	SafeRead (FILE *f, void *buffer, int count);
void	SafeWrite (FILE *f, void *buffer, int count);

int		LoadFile (char *filename, void **bufferptr);
void	SaveFile (char *filename, void *buffer, int count);

void 	DefaultExtension (char *path, char *extension);
void 	DefaultPath (char *path, char *basepath);
void 	StripFilename (char *path);
void 	StripExtension (char *path);

void 	ExtractFilePath (char *path, char *dest);
void 	ExtractFileBase (char *path, char *dest);
void	ExtractFileExtension (char *path, char *dest);

int 	ParseNum (char *str);

extern	short	(*BigShort)(short l);
extern	short	(*LittleShort)(short l);
extern	int		(*BigLong)(int l);
extern	int		(*LittleLong)(int l);
extern	float	(*BigFloat)(float l);
extern	float	(*LittleFloat)(float l);

char *COM_Parse (char *data);

extern	char	com_token[1024];
extern	bool	com_eof;

char *copystring(char *s);

void CRC_Init(unsigned short *crcvalue);
void CRC_ProcessByte(unsigned short *crcvalue, byte data);
unsigned short CRC_Value(unsigned short crcvalue);

void CreatePath (char *path);

extern	bool		archive;
extern	char		archivedir[1024];
#endif
