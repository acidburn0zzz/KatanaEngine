/*	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform_filesystem.h"

/*
	gFileSystem

	TODO:
		A package system?
*/

#include "platform_system.h"

/*  Creates a folder at the given path.
*/
bool pFileSystem_CreateDirectory(const char *ccPath)
{
	pERROR_UPDATE;

#ifdef _WIN32
	if(CreateDirectory(ccPath,NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
		return true;
	else if(GetLastError() == ERROR_PATH_NOT_FOUND)
		pError_Set("Failed to find an intermediate directory! (%s)\n",ccPath);
	else    // Assume it already exists.
		pError_Set("Unknown error! (%s)\n",ccPath);
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
					pError_Set("Failed to get permission! (%s)\n",ccPath);
				case EROFS:
					pError_Set("File system is read only! (%s)\n",ccPath);
				case ENAMETOOLONG:
					pError_Set("Path is too long! (%s)\n",ccPath);
				default:
					pError_Set("Failed to create directory! (%s)\n",ccPath);
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
void pFileSystem_GetUserName(char *cOut)
{
	char	*cUser;
#ifdef _WIN32
	DWORD	dName;

	pERROR_UPDATE;

	// [16/5/2014] Set these AFTER we update active function ~hogsy
	cUser	= "user";
	dName	= sizeof(cUser);

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
void pFileSystem_ScanDirectory(const char *ccPath,void (*vFunction)(const char *ccFile))
{
#ifdef _WIN32
	WIN32_FIND_DATA	wfdData;
	HANDLE			hFile;

	hFile = FindFirstFile(ccPath,&wfdData);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		pError_Set("Recieved invalid handle! (%s)\n",ccPath);
		return;
	}
	else if(GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		pError_Set("Failed to find first file! (%s)\n",ccPath);
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
void pFileSystem_GetWorkingDirectory(char *cOut)
{
    pERROR_UPDATE;

#ifdef _WIN32
    _getcwd(cOut,256);
#else   // Linux
    if(!getcwd(cOut,256))
    {
        // [3/3/2014] Simple error handling. ~hogsy
        switch(errno)
        {
        case ERANGE:
        case EINVAL:
            pError_Set("Invalid size!\n");
        case EACCES:
            pError_Set("Permission was denied!\n");
        }
        return;
    }
#endif

    strcat(cOut,"\\");
}
