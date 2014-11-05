#include "qe3.h"
#include "mru.h"

#include "..\Shared\shared_flags.h"

int			screen_width,screen_height;
bool	have_quit;

int	update_bits;

HANDLE	bsp_process;

//===========================================

void Sys_SetTitle (char *text)
{
	char	cRes[MAX_OSPATH*2];

	sprintf(cRes,"Katana Editor (%s)",text);
	SetWindowText(g_qeglobals.d_hwndMain,cRes);
}

HCURSOR	waitcursor;

void Sys_BeginWait(void)
{
	waitcursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
}

void Sys_EndWait(void)
{
	if (waitcursor)
	{
		SetCursor (waitcursor);
		waitcursor = NULL;
	}
}

void Sys_GetCursorPos (int *x, int *y)
{
	POINT lpPoint;

	GetCursorPos (&lpPoint);
	*x = lpPoint.x;
	*y = lpPoint.y;
}

void Sys_SetCursorPos (int x, int y)
{
	SetCursorPos (x, y);
}

void Sys_UpdateWindows (int bits)
{
//	Sys_Printf("updating 0x%X\n", bits);
	update_bits |= bits;
//update_bits = -1;
}


void Sys_Beep (void)
{
	MessageBeep (MB_ICONASTERISK);
}

char	*TranslateString (char *buf)
{
	static	char	buf2[32768];
	int		i, l;
	char	*out;

	l = strlen(buf);
	out = buf2;
	for (i=0 ; i<l ; i++)
	{
		if (buf[i] == '\n')
		{
			*out++ = '\r';
			*out++ = '\n';
		}
		else
			*out++ = buf[i];
	}
	*out++ = 0;

	return buf2;
}

void Sys_ClearPrintf (void)
{
	char	text[4];

	text[0] = 0;

	SendMessage (g_qeglobals.d_hwndEdit,
		WM_SETTEXT,
		0,
		(LPARAM)text);
}

void Sys_Printf (char *text, ...)
{
	va_list argptr;
	char	buf[32768];
	char	*out;

	va_start (argptr,text);
	vsprintf (buf, text,argptr);
	va_end (argptr);

	out = TranslateString (buf);

#ifdef LATER
	Sys_Status(out);
#else
	SendMessage (g_qeglobals.d_hwndEdit,
		EM_REPLACESEL,
		0,
		(LPARAM)out);
#endif

}

double Sys_DoubleTime (void)
{
	return clock()/ 1000.0;
}

void PrintPixels (HDC hDC)
{
	int		i;
	PIXELFORMATDESCRIPTOR p[64];

	printf ("### flags color layer\n");
	for (i=1 ; i<64 ; i++)
	{
		if (!DescribePixelFormat ( hDC, i, sizeof(p[0]), &p[i]))
			break;
		printf ("%3i %5i %5i %5i\n", i,
			p[i].dwFlags,
			p[i].cColorBits,
			p[i].bReserved);
	}
	printf ("%i modes\n", i-1);
}



//==========================================================================

void QEW_StopGL( HWND hWnd, HGLRC hGLRC, HDC hDC )
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hGLRC );

	ReleaseDC( hWnd, hDC );
}
		
int QEW_SetupPixelFormat(HDC hDC, bool zbuffer )
{
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		24,								// 24-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0,						// accum bits ignored
		32,							    // depth bits
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
	};
	int pixelformat = 0;

	zbuffer = true;
	if(!zbuffer)
		pfd.cDepthBits = 0;

	if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
	{
		printf("%d",GetLastError());
		Error ("ChoosePixelFormat failed");
	}

	if (!SetPixelFormat(hDC, pixelformat, &pfd))
		Error ("SetPixelFormat failed");

	return pixelformat;
}

/*
======================================================================

FILE DIALOGS

======================================================================
*/
 
bool Window_ConfirmModified(void)
{
	if(modified)
		if(MessageBox(g_qeglobals.d_hwndMain,"This will lose changes to the map","Warning",MB_OKCANCEL) == IDCANCEL)
			return false;

	return true;
}

static OPENFILENAME ofn;       /* common dialog box structure   */ 
static char szDirName[MAX_PATH];    /* directory string              */ 
static char szFile[260];       /* filename string               */ 
static char szFileTitle[260];  /* file title string             */ 
static char szFilter[260] =     /* filter string                 */ 
	"Level file (*"LVL_EXTENSION")\0*"LVL_EXTENSION"\0\0";
static char szProjectFilter[260] =     /* filter string                 */ 
	"Project file (*.scr)\0*.scr\0\0";
static char chReplace;         /* string separator for szFilter */ 
static int i, cbString;        /* integer count variables       */ 
static HANDLE hf;              /* file handle                   */ 

void OpenDialog (void)
{
	/* 
	 * Obtain the system directory name and 
	 * store it in szDirName. 
	 */ 
 
	strcpy(szDirName, ValueForKey (g_qeglobals.d_project_entity,"$basepath") );
	strcat(szDirName, "\\"PATH_MAPS);

	/* Place the terminating null character in the szFile. */ 
 
	szFile[0] = '\0'; 
 
	/* Set the members of the OPENFILENAME structure. */ 
	ofn.lStructSize = sizeof(OPENFILENAME); 
	ofn.hwndOwner = g_qeglobals.d_hwndCamera;
	ofn.lpstrFilter = szFilter; 
	ofn.nFilterIndex = 1; 
	ofn.lpstrFile = szFile; 
	ofn.nMaxFile = sizeof(szFile); 
	ofn.lpstrFileTitle = szFileTitle; 
	ofn.nMaxFileTitle = sizeof(szFileTitle); 
	ofn.lpstrInitialDir = szDirName; 
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | 
		OFN_FILEMUSTEXIST; 

	/* Display the Open dialog box. */ 
 
	if (!GetOpenFileName(&ofn))
		return;	// canceled
 
	// Add the file in MRU.
	AddNewItem( g_qeglobals.d_lpMruMenu, ofn.lpstrFile);

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(g_qeglobals.d_hwndMain),0), 
			ID_FILE_EXIT);

	/* Open the file. */ 
 
	Map_LoadFile (ofn.lpstrFile);	
}

void ProjectDialog (void)
{
	/* 
	 * Obtain the system directory name and 
	 * store it in szDirName. 
	 */ 
 
	strcpy(szDirName,ValueForKey(g_qeglobals.d_project_entity, "$basepath") );
	strcat(szDirName,"\\scripts");

	/* Place the terminating null character in the szFile. */ 
 
	szFile[0] = '\0'; 
 
	// Set the members of the OPENFILENAME structure.
	ofn.lStructSize		= sizeof(OPENFILENAME); 
	ofn.hwndOwner		= g_qeglobals.d_hwndCamera;
	ofn.lpstrFilter		= szProjectFilter; 
	ofn.nFilterIndex	= 1; 
	ofn.lpstrFile		= szFile; 
	ofn.nMaxFile		= sizeof(szFile); 
	ofn.lpstrFileTitle	= szFileTitle; 
	ofn.nMaxFileTitle	= sizeof(szFileTitle); 
	ofn.lpstrInitialDir = szDirName; 
	ofn.Flags			= OFN_SHOWHELP|OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST; 

	// Display the Open dialog box.
	if (!GetOpenFileName(&ofn))
		return;	// canceled
 
	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(g_qeglobals.d_hwndMain),0), 
			ID_FILE_EXIT);

	/* Open the file. */ 
	if (!QE_LoadProject(ofn.lpstrFile))
		Error ("Couldn't load project file");
}

void SaveAsDialog (void)
{ 
	strcpy (szDirName,ValueForKey (g_qeglobals.d_project_entity, "$basepath") );
	strcat (szDirName,"\\"PATH_MAPS);

	// Place the terminating null character in the szFile.
	szFile[0] = '\0'; 
 
	/* Set the members of the OPENFILENAME structure. */ 
	ofn.lStructSize = sizeof(OPENFILENAME); 
	ofn.hwndOwner = g_qeglobals.d_hwndCamera;
	ofn.lpstrFilter = szFilter; 
	ofn.nFilterIndex = 1; 
	ofn.lpstrFile = szFile; 
	ofn.nMaxFile = sizeof(szFile); 
	ofn.lpstrFileTitle = szFileTitle; 
	ofn.nMaxFileTitle = sizeof(szFileTitle); 
	ofn.lpstrInitialDir = szDirName; 
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | 
		OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT; 

	/* Display the Open dialog box. */ 
 
	if (!GetSaveFileName(&ofn))
		return;	// canceled
  
	DefaultExtension(ofn.lpstrFile,LVL_EXTENSION);
	strcpy(currentmap,ofn.lpstrFile);

	// Add the file in MRU.
	AddNewItem(g_qeglobals.d_lpMruMenu, ofn.lpstrFile);

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(g_qeglobals.d_hwndMain),0),
			ID_FILE_EXIT);

	Map_SaveFile(ofn.lpstrFile,false);	// ignore region
}

/*
=======================================================

Menu modifications

=======================================================
*/

char	*bsp_commands[256];

void FillBSPMenu (void)
{
	HMENU	hmenu;
	epair_t	*ep;
	int		i;
	static int count;

	hmenu = GetSubMenu (GetMenu(g_qeglobals.d_hwndMain), MENU_BSP);

	for (i=0 ; i<count ; i++)
		DeleteMenu (hmenu, CMD_BSPCOMMAND+i, MF_BYCOMMAND);
	count = 0;

	i = 0;
	for (ep = g_qeglobals.d_project_entity->epairs ; ep ; ep=ep->next)
	{
		if (ep->key[0] == 'b' && ep->key[1] == 's' && ep->key[2] == 'p')
		{
			bsp_commands[i] = ep->key;
			AppendMenu (hmenu, MF_ENABLED|MF_STRING,
			CMD_BSPCOMMAND+i, (LPCTSTR)ep->key);
			i++;
		}
	}
	count = i;
}

//==============================================

/*	See if the BSP is done yet
*/
void CheckBspProcess(void)
{
	char	outputpath[1024],
			temppath[512];
	DWORD	exitcode;
	char	*out;
	bool	bReturn;

	if (!bsp_process)
		return;

	bReturn = GetExitCodeProcess (bsp_process, &exitcode);
	if(!bReturn)
		Error ("GetExitCodeProcess failed");
	else if(exitcode == STILL_ACTIVE)
		return;

	bsp_process = 0;

	GetTempPath(512,temppath);
	sprintf(outputpath,"%sjunk.txt",temppath);

	LoadFile(outputpath,(void **)&out);
	Sys_Printf("%s",out);
	Sys_Printf("\ncompleted.\n");
	free(out);

	Sys_Beep();

	Pointfile_Check();
}

extern int	iCameraButtonState;

HWND	hSplashScreen;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	int						argc;
	char					*argv[32];
	INITCOMMONCONTROLSEX	iCommonControls;
	MSG						msg;
	double					time, oldtime, delta;
	HACCEL					accelerators;

	g_qeglobals.d_hInstance = hInstance;

	iCommonControls.dwICC	= 0;
	iCommonControls.dwSize	= sizeof(INITCOMMONCONTROLSEX);
	
	// [27/8/2012] Initialize our common controls ~hogsy
	InitCommonControlsEx(&iCommonControls);

	screen_width	= GetSystemMetrics(SM_CXFULLSCREEN);
	screen_height	= GetSystemMetrics(SM_CYFULLSCREEN);

	accelerators = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
	if(!accelerators)
		Error("LoadAccelerators failed");

	Main_Create(hInstance);

	Console_CreateWindow();
	
	hSplashScreen = CreateDialog(hInstance,(char*)IDD_SPLASH,g_qeglobals.d_hwndMain,NULL);
	if(hSplashScreen)
	{
		ShowWindow(hSplashScreen,SW_SHOWDEFAULT);
		UpdateWindow(hSplashScreen);
		SetForegroundWindow(hSplashScreen);
	}
	else
		Console_Print(true,"Failed to create splash dialog!\n");

//	Console_CreateWindow(hInstance);

	CreateEntityWindow(hInstance);

	// the project file can be specified on the command line,
	// or implicitly found in the scripts directory
	if(lpCmdLine && strlen(lpCmdLine))
	{
		// [4/4/2013] Moved here since this shit isn't used or needed anywhere else anymore ~hogsy
		argc	= 1;
		argv[0] = "programname";

		while(*lpCmdLine && (argc < 32))
		{
			while(*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
				lpCmdLine++;

			if(*lpCmdLine)
			{
				argv[argc] = lpCmdLine;
				argc++;

				while(*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
					lpCmdLine++;

				if(*lpCmdLine)
				{
					*lpCmdLine = 0;
					lpCmdLine++;
				}
			}
		}

		if(!QE_LoadProject(argv[1]))
			Error("Couldn't load %s project file",argv[1]);
	}
	else if(!QE_LoadProject(PATH_ENGINE"/scripts/paths.script"))
		Error("Couldn't load paths.script project file!");

	WCam_Create(hInstance);
#if 0
	WZ_Create(hInstance);
	WXY_Create(hInstance);
#endif

	UpdateWindow(hSplashScreen);
	SetForegroundWindow(hSplashScreen);

	QE_Init();

	DestroyWindow(hSplashScreen);

	ShowWindow(g_qeglobals.d_hwndCamera,SW_SHOWDEFAULT);
#if 0
	ShowWindow(g_qeglobals.d_hwndXY,SW_SHOWDEFAULT);
	ShowWindow(g_qeglobals.d_hwndZ,SW_SHOWDEFAULT);
#endif

	oldtime = Sys_DoubleTime ();

	while(!have_quit)
	{
		Sys_EndWait ();		// Remove wait cursor if active

		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if (!TranslateAccelerator(g_qeglobals.d_hwndMain, accelerators, &msg) )
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}

			if (msg.message == WM_QUIT)
				have_quit = true;
		}

		CheckBspProcess ();

		time = Sys_DoubleTime ();
		delta = time - oldtime;
		oldtime = time;
		if (delta > 0.2)
			delta = 0.2;

		// Run time dependant behavior
		Cam_MouseControl((float)delta);

		// update any windows now
		if (update_bits & W_CAMERA)
		{
			InvalidateRect(g_qeglobals.d_hwndCamera, NULL, FALSE);
			UpdateWindow (g_qeglobals.d_hwndCamera);
		}

		if(update_bits & (W_Z | W_Z_OVERLAY))
		{
			InvalidateRect(g_qeglobals.d_hwndZ, NULL, FALSE);
			UpdateWindow (g_qeglobals.d_hwndZ);
		}
			
		if(update_bits & W_TEXTURE)
		{
			InvalidateRect(g_qeglobals.d_hwndTexture, NULL, FALSE);
			UpdateWindow (g_qeglobals.d_hwndEntity);
		}
	
		if(update_bits & (W_XY | W_XY_OVERLAY))
		{
			InvalidateRect(g_qeglobals.d_hwndXY, NULL, FALSE);
			UpdateWindow (g_qeglobals.d_hwndXY);
		}

		update_bits = 0;

		if(!iCameraButtonState && !have_quit)
			// If not driving in the camera view, block
			WaitMessage ();
	}

	// Return success of application
	return true;
}

