#include "cmdlib.H"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <direct.h>
#endif

#include "platform_filesystem.h"

#define PATHSEPERATOR   '/'

// set these before calling CheckParm
int myargc;
char **myargv;

char	com_token[1024],
		archivedir[1024];

bool	com_eof,
		archive;

/*	For abnormal program terminations
*/
void Error (const char *error, ...)
{
	va_list argptr;

	printf ("************ ERROR ************\n");

	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
	exit (1);
}

/*

qdir will hold the path up to the quake directory, including the slash

  f:\quake\
  /raid/quake/

gamedir will hold qdir + the game directory (id1, id2, etc)

  */

char		qdir[1024];
char		gamedir[1024];

void SetQdirFromPath (char *path)
{
	char	temp[1024];
	char	*c;

	if (!(path[0] == '/' || path[0] == '\\' || path[1] == ':'))
	{	// path is partial
		pFileSystem_GetWorkingDirectory(temp);
		strcat (temp, path);
		path = temp;
	}

	// search for "quake" in path

	for (c=path ; *c ; c++)
		if (!Q_strncasecmp (c, "quake", 5))
		{
			strncpy (qdir, path, c+6-path);
			printf ("qdir: %s\n", qdir);
			c += 6;
			while (*c)
			{
				if (*c == '/' || *c == '\\')
				{
					strncpy (gamedir, path, c+1-path);
					printf ("gamedir: %s\n", gamedir);
					return;
				}
				c++;
			}
			Error ("No gamedir in %s", path);
			return;
		}
	Error ("SeetQdirFromPath: no 'quake' in %s", path);
}

char *ExpandPath (char *path)
{
	static char full[1024];
	if(qdir[0] == ' ')
		Error ("ExpandPath called without qdir set");
	if (path[0] == '/' || path[0] == '\\' || path[1] == ':')
		return path;
	sprintf (full, "%s%s", qdir, path);
	return full;
}

/*	Used to archive source files
*/
void Q_CopyFile(char *from,char *to)
{
	void	*buffer;
	int		length;

	length = LoadFile (from, &buffer);
	CreatePath (to);
	SaveFile (to, buffer, length);
	free (buffer);
}

char *ExpandPathAndArchive (char *path)
{
	char	*expanded;
	char	archivename[1024];

	expanded = ExpandPath (path);

	if(archive)
	{
		sprintf(archivename,"%s/%s",archivedir,path);

		// [1/8/2013] Renamed to avoid conflict with Window libraries ~hogsy
		Q_CopyFile(expanded,archivename);
	}
	return expanded;
}

char *copystring(char *s)
{
	char	*b;
	b = (char*)malloc(strlen(s)+1);
	strcpy (b, s);
	return b;
}

double I_FloatTime (void)
{
#if 1
	time_t	t;

	time (&t);

	return (double)t;
#else
// more precise, less portable
	struct timeval tp;
	struct timezone tzp;
	static int		secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
#endif
}

/*	Returns -1 if not present
*/
int	FileTime (char *path)
{
	struct	stat	buf;

	if (stat (path,&buf) == -1)
		return -1;

	return (int)buf.st_mtime;
}

/*	Parse a token out of a string
*/
char *COM_Parse (char *data)
{
	int		c;
	int		len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

	// Skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
		{
			com_eof = true;
			return NULL;			// end of file;
		}
		data++;
	}

	// skip // comments
	if (c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

	// Skip /* comments
	if (c=='/' && data[1] == '*')
	{
		while (*data && (*data != '*' || data[1] != '/'))
			data++;
		goto skipwhite;
	}

	// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		do
		{
			c = *data++;
			if (c=='\"')
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = (char)c;
			len++;
		} while (1);
	}

	// parse single characters
	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
	{
		com_token[len] = (char)c;
		len++;
		com_token[len] = 0;
		return data+1;
	}

	// parse a regular word
	do
	{
		com_token[len] = (char)c;
		data++;
		len++;
		c = *data;
		if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
			break;
	} while (c>32);

	com_token[len] = 0;
	return data;
}

int Q_strncasecmp (char *s1, char *s2, int n)
{
	int		c1, c2;

	for(;;)
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point

		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				break;		// strings not equal
		}

		if (!c1)
			return 0;		// strings are equal
	}

	return -1;
}

int Q_strcasecmp (char *s1, char *s2)
{
	return Q_strncasecmp (s1, s2, 99999);
}

char *strlower (char *start)
{
	char	*in;
	in = start;
	while (*in)
	{
		*in = (char)tolower((int)*in);
		in++;
	}
	return start;
}

int CheckParm (char *check)
{
	int             i;

	for (i = 1;i<myargc;i++)
	{
		if ( !Q_strcasecmp(check, myargv[i]) )
			return i;
	}

	return 0;
}

FILE *SafeOpenWrite (char *filename)
{
	FILE	*f;

	f = fopen(filename, "wb");

	if (!f)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return f;
}

FILE *SafeOpenRead(char *filename)
{
	FILE	*f;

	f = fopen(filename, "rb");
	if(!f)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return f;
}

void SafeRead (FILE *f, void *buffer, int count)
{
	if ( fread (buffer, 1, count, f) != (size_t)count)
		Error ("File read failure");
}

void SafeWrite (FILE *f, void *buffer, int count)
{
	if (fwrite (buffer, 1, count, f) != (size_t)count)
		Error ("File read failure");
}

// [28/2/2013] Modified ~hogsy
int LoadFile(char *filename,void **bufferptr)
{
	FILE			*f;
	struct	stat	sFileSize;
	void			*buffer;

	// [6/3/2014] Updated to use this method so this works for the editor ~hogsy
	// [7/3/2014] Oops! Fixed this, set to read mode :) ~hogsy
	f = fopen(filename,"rb");
	if(!f)
		return -1;

	stat(filename,&sFileSize);

	buffer = malloc(sFileSize.st_size+1);
	((char*)buffer)[sFileSize.st_size] = 0;

	SafeRead(f,buffer,sFileSize.st_size);

	fclose(f);

	*bufferptr = buffer;

	return sFileSize.st_size;
}

void SaveFile (char *filename, void *buffer, int count)
{
	FILE	*f;

	f = SafeOpenWrite (filename);
	SafeWrite (f, buffer, count);
	fclose (f);
}

void DefaultExtension (char *path, char *extension)
{
	char    *src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != PATHSEPERATOR && src != path)
	{
		if (*src == '.')
			return;                 // it has an extension
		src--;
	}

	strcat (path, extension);
}

void DefaultPath (char *path, char *basepath)
{
	char    temp[128];

	if (path[0] == PATHSEPERATOR)
		return;                   // absolute path location
	strcpy (temp,path);
	strcpy (path,basepath);
	strcat (path,temp);
}

void    StripFilename (char *path)
{
	int             length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != PATHSEPERATOR)
		length--;
	path[length] = 0;
}

void    StripExtension (char *path)
{
	int             length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != '.')
	{
		length--;
		if (path[length] == '/')
			return;		// no extension
	}
	if (length)
		path[length] = 0;
}

void ExtractFilePath (char *path, char *dest)
{
	char    *src;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && *(src-1) != '/')
		src--;

	memcpy (dest, path, src-path);
	dest[src-path] = 0;
}

void ExtractFileBase (char *path, char *dest)
{
	char    *src;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && *(src-1) != '/')
		src--;

	while (*src && *src != '.')
	{
		*dest++ = *src++;
	}
	*dest = 0;
}

void ExtractFileExtension (char *path, char *dest)
{
	char    *src;

	src = path + strlen(path) - 1;

//
// back up until a . or the start
//
	while (src != path && *(src-1) != '.')
		src--;
	if (src == path)
	{
		*dest = 0;	// no extension
		return;
	}

	strcpy (dest,src);
}

int ParseHex (char *hex)
{
	char    *str;
	int    num;

	num = 0;
	str = hex;

	while (*str)
	{
		num <<= 4;
		if (*str >= '0' && *str <= '9')
			num += *str-'0';
		else if (*str >= 'a' && *str <= 'f')
			num += 10 + *str-'a';
		else if (*str >= 'A' && *str <= 'F')
			num += 10 + *str-'A';
		else
			Error ("Bad hex number: %s",hex);
		str++;
	}

	return num;
}

int ParseNum (char *str)
{
	if (str[0] == '$')
		return ParseHex (str+1);
	if (str[0] == '0' && str[1] == 'x')
		return ParseHex (str+2);
	return atol (str);
}

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

short   (*BigShort) (short l);
short   (*LittleShort) (short l);
int     (*BigLong) (int l);
int     (*LittleLong) (int l);
float   (*BigFloat) (float l);
float   (*LittleFloat) (float l);

//=============================================================================

void CreatePath(char *path)
{
	char *ofs, c;

	for(ofs = path+1; *ofs; ofs++)
	{
		c = *ofs;
		if(c == '/' || c == '\\')
		{
			// Create the directory
			*ofs = 0;
#ifdef _WIN32
			_mkdir(path);
#else
            mkdir(path,0777);
#endif
			*ofs = c;
		}
	}
}
