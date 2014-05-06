/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform_filesystem.h"

/*
	gFileSystem

	TODO:
		A package system?
*/

/*  Creates a folder at the given path.
*/
bool gFileSystem_CreateDirectory(const char *ccPath)
{
	gError_SetFunction("gFileSystem_CreateDirectory");

#ifdef _WIN32
	if(CreateDirectory(ccPath,NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
		return true;
	else if(GetLastError() == ERROR_PATH_NOT_FOUND)
		GIPL_SetError("Failed to find an intermediate directory! (%s)\n",ccPath);
	else    // Assume it already exists.
		GIPL_SetError("Unknown error! (%s)\n",ccPath);
#else
	{
		struct stat ssBuffer;

		if(stat(ccPath,&ssBuffer) == -1)
		{
			if(mkdir(ccPath,0777) == 0)
				return true;
			else
			{
				switch(errno)
				{
				case EACCES:
					GIPL_SetError("Failed to get permission! (%s)\n",ccPath);
				case EROFS:
					GIPL_SetError("File system is read only! (%s)\n",ccPath);
				case ENAMETOOLONG:
					GIPL_SetError("Path is too long! (%s)\n",ccPath);
				default:
					GIPL_SetError("Failed to create directory! (%s)\n",ccPath);
				}
			}
		}
		else
			// [28/10/2013] Path already exists! ~hogsy
			return true;
	}
#endif

	return false;
}

/*	Returns the name of the systems
	current user.
*/
void gFileSystem_GetUserName(char *cOut)
{
	char	*cUser = "user";
#ifdef _WIN32
	DWORD	dName = sizeof(cUser);

	if(GetUserName(cUser,&dName))
#else   // Linux
    cUser = getenv("LOGNAME");
	if(strcasecmp(cUser,"user"))
#endif
	{
		int	i		= 0,
			iLength = strlen(cUser);

		// [11/5/2013] Quick fix for spaces ~hogsy
		while(i < iLength)
		{
			if(cUser[i] == ' ')
				cUser[i] = '_';
			else
				cUser[i] = (char)tolower(cUser[i]);

			i++;
		}
	}

	strcpy(cOut,cUser);
}

/*	Scans the given directory.
	On each found file it calls the given function to handle the file.
	TODO:
		Better error management.
		Finish Linux implementation.
*/
void gFileSystem_ScanDirectory(const char *ccPath,void (*vFunction)(const char *ccFile))
{
#ifdef _WIN32
	WIN32_FIND_DATA	wfdData;
	HANDLE			hFile;

	hFile = FindFirstFile(ccPath,&wfdData);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		GIPL_SetError("Recieved invalid handle! (%s)\n",ccPath);
		return;
	}
	else if(GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		GIPL_SetError("Failed to find first file! (%s)\n",ccPath);
		return;
	}

	do
	{
		// [31/7/2013] Send the file we've found! ~hogsy
		vFunction(wfdData.cFileName);
	} while(FindNextFile(hFile,&wfdData));

	FindClose(hFile);
#else	// Linux
#endif
}

/*  Based on Q_getwd.
*/
void gFileSystem_GetWorkingDirectory(char *cOutput)
{
    gError_SetFunction("gFileSystem_GetWorkingDirectory");

#ifdef _WIN32
    _getcwd(cOutput,256);
#else   // Linux
    if(!getcwd(cOutput,256))
    {
        // [3/3/2014] Simple error handling. ~hogsy
        switch(errno)
        {
        case ERANGE:
        case EINVAL:
            GIPL_SetError("Invalid size!\n");
        case EACCES:
            GIPL_SetError("Permission was denied!\n");
        }
        return;
    }
#endif

    strcat(cOutput,"\\");
}
