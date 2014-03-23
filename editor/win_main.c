#include "editor_main.h"
#include <process.h>
#include "entityw.h"

static HWND      s_hwndToolbar;

BOOL SaveRegistryInfo(const char *pszName, void *pvBuf, long lSize);
BOOL LoadRegistryInfo(const char *pszName, void *pvBuf, long *plSize);

static HWND CreateMyStatusWindow(HINSTANCE hInst);
static HWND CreateToolBar(HINSTANCE hinst);

extern int WXY_Print( void );

/*
==============================================================================

  MENU

==============================================================================
*/

void OpenDialog(void);
void SaveAsDialog(void);
void Select_Ungroup(void);

bool Window_ConfirmModified(void);

void QE_ExpandBspString (char *bspaction, char *out, char *mapname)
{
#if 0
	char	*in,
			src[1024],rsh[1024],base[256];

	ExtractFileName(mapname,base);

	sprintf (src, "%s/maps/%s", ValueForKey(g_qeglobals.d_project_entity, "remotebasepath"), base);
	strcpy (rsh, ValueForKey(g_qeglobals.d_project_entity, "rshcmd"));

	in = ValueForKey( g_qeglobals.d_project_entity, bspaction );
	while (*in)
	{
		if (in[0] == '!')
		{
			strcpy (out, rsh);
			out += strlen(rsh);
			in++;
			continue;
		}
		if (in[0] == '$')
		{
			strcpy (out, src);
			out += strlen(src);
			in++;
			continue;
		}
		if (in[0] == '@')
		{
			*out++ = '"';
			in++;
			continue;
		}
		*out++ = *in++;
	}
	*out = 0;
#else
	Console_Print(false,"QE_ExpandBspString\n");
#endif
}

void RunBsp (char *command)
{
	char	sys[1024];
	char	batpath[1024];
	char	outputpath[1024];
	char	temppath[512];
	char	name[1024];
	FILE	*hFile;
	BOOL	ret;
	PROCESS_INFORMATION ProcessInformation;
	STARTUPINFO	startupinfo;

	SetInspectorMode (W_CONSOLE);

	if (bsp_process)
	{
		Sys_Printf ("BSP is still going...\n");
		return;
	}

	GetTempPath(512, temppath);
	sprintf (outputpath, "%sjunk.txt", temppath);

	strcpy (name, currentmap);
	if(bRegionActive)
	{
		Map_SaveFile(name,false);
		StripExtension (name);
		strcat (name, ".reg");
	}

	Map_SaveFile(name,bRegionActive);

	QE_ExpandBspString (command, sys, name);

	Sys_ClearPrintf ();
	Sys_Printf ("======================================\nRunning bsp command...\n");
	Sys_Printf ("\n%s\n", sys);

	//
	// write qe3bsp.bat
	//
	sprintf (batpath, "%sqe3bsp.bat", temppath);
	hFile = fopen(batpath, "w");
	if (!hFile)
		Error ("Can't write to %s", batpath);
	fprintf (hFile, sys);
	fclose (hFile);

	//
	// write qe3bsp2.bat
	//
	sprintf (batpath, "%sqe3bsp2.bat", temppath);
	hFile = fopen(batpath, "w");
	if (!hFile)
		Error ("Can't write to %s", batpath);
	fprintf (hFile, "%sqe3bsp.bat > %s", temppath, outputpath);
	fclose (hFile);

	Pointfile_Delete ();

	GetStartupInfo (&startupinfo);

	ret = CreateProcess(
		batpath,		// pointer to name of executable module
		NULL,			// pointer to command line string
		NULL,			// pointer to process security attributes
		NULL,			// pointer to thread security attributes
		FALSE,			// handle inheritance flag
		0 /*DETACHED_PROCESS*/,		// creation flags
		NULL,			// pointer to new environment block
		NULL,			// pointer to current directory name
		&startupinfo,	// pointer to STARTUPINFO
		&ProcessInformation);

	if (!ret)
		Error ("CreateProcess failed");

	bsp_process = ProcessInformation.hProcess;

	Sleep (100);	// give the new process a chance to open it's window

	BringWindowToTop( g_qeglobals.d_hwndMain );	// pop us back on top
	SetFocus (g_qeglobals.d_hwndCamera);
}

qboolean DoColor(int iIndex)
{
	CHOOSECOLOR	cc;
	static COLORREF	custom[16];

	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = g_qeglobals.d_hwndMain;
	cc.hInstance = (HWND)g_qeglobals.d_hInstance;
	cc.rgbResult =
		(int)(g_qeglobals.d_savedinfo.colors[iIndex][0]*255) +
		(((int)(g_qeglobals.d_savedinfo.colors[iIndex][1]*255))<<8) +
		(((int)(g_qeglobals.d_savedinfo.colors[iIndex][2]*255))<<16);
	cc.lpCustColors = custom;
	cc.Flags = CC_FULLOPEN|CC_RGBINIT;
	//cc.lCustData;
	//cc.lpfnHook;
	//cc.lpTemplateName

	if (!ChooseColor(&cc))
		return FALSE;

	g_qeglobals.d_savedinfo.colors[iIndex][0] = (cc.rgbResult&255)/255.0;
	g_qeglobals.d_savedinfo.colors[iIndex][1] = ((cc.rgbResult>>8)&255)/255.0;
	g_qeglobals.d_savedinfo.colors[iIndex][2] = ((cc.rgbResult>>16)&255)/255.0;

	/*
	** scale colors so that at least one component is at 1.0F
	** if this is meant to select an entity color
	*/
	if ( iIndex == COLOR_ENTITY )
	{
		float largest = 0.0F;

		if ( g_qeglobals.d_savedinfo.colors[iIndex][0] > largest )
			largest = g_qeglobals.d_savedinfo.colors[iIndex][0];
		if ( g_qeglobals.d_savedinfo.colors[iIndex][1] > largest )
			largest = g_qeglobals.d_savedinfo.colors[iIndex][1];
		if ( g_qeglobals.d_savedinfo.colors[iIndex][2] > largest )
			largest = g_qeglobals.d_savedinfo.colors[iIndex][2];

		if ( largest == 0.0F )
		{
			g_qeglobals.d_savedinfo.colors[iIndex][0] = 1.0F;
			g_qeglobals.d_savedinfo.colors[iIndex][1] = 1.0F;
			g_qeglobals.d_savedinfo.colors[iIndex][2] = 1.0F;
		}
		else
		{
			float scaler = 1.0F / largest;

			g_qeglobals.d_savedinfo.colors[iIndex][0] *= scaler;
			g_qeglobals.d_savedinfo.colors[iIndex][1] *= scaler;
			g_qeglobals.d_savedinfo.colors[iIndex][2] *= scaler;
		}
	}

	Sys_UpdateWindows (W_ALL);

	return TRUE;
}

/* Copied from MSDN */
BOOL DoMru(HWND hWnd,WORD wId)
{
	char szFileName[128];
	OFSTRUCT of;
	BOOL fExist;

	GetMenuItem(g_qeglobals.d_lpMruMenu, wId, TRUE, szFileName, sizeof(szFileName));

	// Test if the file exists.
	fExist = OpenFile(szFileName ,&of,OF_EXIST) != HFILE_ERROR;
	if(fExist)
	{
		// Place the file on the top of MRU.
		AddNewItem(g_qeglobals.d_lpMruMenu,(LPSTR)szFileName);

		// Now perform opening this file !!!
		Map_LoadFile (szFileName);
	}
	else
		// Remove the file on MRU.
		DelMenuItem(g_qeglobals.d_lpMruMenu,wId,TRUE);

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(hWnd),0),
			ID_FILE_EXIT);

	return fExist;
}

/* handle all WM_COMMAND messages here */
LONG WINAPI CommandHandler (
	HWND    hWnd,
	WPARAM  wParam,
	LPARAM  lParam)
{
	HMENU hMenu;

	switch (LOWORD(wParam))
	{
//
// file menu
//
		case ID_FILE_EXIT:
			/* exit application */
			if (!Window_ConfirmModified())
				return TRUE;

			PostMessage (hWnd, WM_CLOSE, 0, 0L);
			break;
		case ID_FILE_OPEN:
			if (!Window_ConfirmModified())
				return TRUE;
			OpenDialog ();
			break;
		case ID_FILE_NEW:
			if (!Window_ConfirmModified())
				return TRUE;
			Map_New ();
			break;
		case ID_FILE_SAVE:
			if(!strcmp(currentmap,"unnamed"EXTENSION_MAP))
				SaveAsDialog();
			else
				Map_SaveFile(currentmap,FALSE);	// ignore region
			break;
		case ID_FILE_SAVEAS:
			SaveAsDialog ();
			break;
		case ID_FILE_LOADPROJECT:
			if (!Window_ConfirmModified())
				return TRUE;
			ProjectDialog ();
			break;
		case ID_FILE_POINTFILE:
			if (g_qeglobals.d_pointfile_display_list)
				Pointfile_Clear ();
			else
				Pointfile_Check ();
			break;
		// [8/8/2012] Added ID_EDIT_COPY ~hogsy
		case ID_EDIT_COPY:
			Select_Clone();
			break;
//
// view menu
//
		case ID_VIEW_ENTITY:
			SetInspectorMode(W_ENTITY);
			break;
		case ID_VIEW_CONSOLE:
			//Engine->ShowConsole();
			break;
		case ID_VIEW_MODEL:
			break;
		case ID_VIEW_TEXTURE:
			SetInspectorMode(W_TEXTURE);
			break;
		case ID_VIEW_FOLLOWCAM:
			g_qeglobals.d_savedinfo.bFollowCamera = !g_qeglobals.d_savedinfo.bFollowCamera;

			CheckMenuItem(GetMenu(g_qeglobals.d_hwndMain),ID_VIEW_FOLLOWCAM,MF_BYCOMMAND|(g_qeglobals.d_savedinfo.bFollowCamera ? MF_CHECKED : MF_UNCHECKED));

			// [31/1/2013] Update XY position ~hogsy
			if(g_qeglobals.d_savedinfo.bFollowCamera)
			{
				g_qeglobals.d_xy.origin[X] = camera.origin[X];
				g_qeglobals.d_xy.origin[Y] = camera.origin[Y];
			}

			Sys_UpdateWindows(W_XY);
			break;
		case ID_VIEW_100:
			g_qeglobals.d_xy.scale = 1;
			Sys_UpdateWindows (W_XY|W_XY_OVERLAY);
			break;
		case ID_VIEW_ZOOMIN:
			g_qeglobals.d_xy.scale *= 5.0/4;
			if (g_qeglobals.d_xy.scale > 16)
				g_qeglobals.d_xy.scale = 16;
			Sys_UpdateWindows (W_XY|W_XY_OVERLAY);
			break;
		case ID_VIEW_ZOOMOUT:
			g_qeglobals.d_xy.scale *= 4.0f/5;
			if (g_qeglobals.d_xy.scale < 0.1f)
				g_qeglobals.d_xy.scale = 0.1f;
			Sys_UpdateWindows (W_XY|W_XY_OVERLAY);
			break;

		case ID_VIEW_Z100:
			z.scale = 1;
			Sys_UpdateWindows (W_Z|W_Z_OVERLAY);
			break;
		case ID_VIEW_ZZOOMIN:
			z.scale *= 5.0/4;
			if (z.scale > 4)
				z.scale = 4;
			Sys_UpdateWindows (W_Z|W_Z_OVERLAY);
			break;
		case ID_VIEW_ZZOOMOUT:
			z.scale *= 4.0f/5;
			if (z.scale < 0.125)
				z.scale = 0.125;
			Sys_UpdateWindows (W_Z|W_Z_OVERLAY);
			break;

		case ID_VIEW_CENTER:
			camera.angles[ROLL] = camera.angles[PITCH] = 0;
			camera.angles[YAW] = 22.5 *
				floor( (camera.angles[YAW]+11)/22.5 );
			Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
			break;

		case ID_VIEW_UPFLOOR:
			Cam_ChangeFloor (TRUE);
			break;
		case ID_VIEW_DOWNFLOOR:
			Cam_ChangeFloor (FALSE);
			break;

		case ID_VIEW_SHOWNAMES:
			g_qeglobals.d_savedinfo.bShowNames = !g_qeglobals.d_savedinfo.bShowNames;
			CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWNAMES, MF_BYCOMMAND | (g_qeglobals.d_savedinfo.bShowNames ? MF_CHECKED : MF_UNCHECKED)  );
			Map_BuildBrushData();
			Sys_UpdateWindows (W_XY);
			break;
		case ID_VIEW_SHOWCOORDINATES:
			g_qeglobals.d_savedinfo.show_coordinates ^= 1;
			CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWCOORDINATES, MF_BYCOMMAND | (g_qeglobals.d_savedinfo.show_coordinates ? MF_CHECKED : MF_UNCHECKED)  );
			Sys_UpdateWindows (W_XY);
			break;
		case ID_VIEW_SHOWBLOCKS:
			g_qeglobals.show_blocks ^= 1;
			CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWBLOCKS, MF_BYCOMMAND | (g_qeglobals.show_blocks ? MF_CHECKED : MF_UNCHECKED)  );
			Sys_UpdateWindows (W_XY);
			break;
		case ID_VIEW_SHOWLIGHTS:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_LIGHTS ) & EXCLUDE_LIGHTS )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWLIGHTS, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWLIGHTS, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;
		case ID_VIEW_SHOWPATH:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_PATHS ) & EXCLUDE_PATHS )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWPATH, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWPATH, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWENT:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_ENT ) & EXCLUDE_ENT )
				CheckMenuItem( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWENT, MF_BYCOMMAND | MF_UNCHECKED);
			else
				CheckMenuItem( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWENT, MF_BYCOMMAND | MF_CHECKED);
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWWATER:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_WATER ) & EXCLUDE_WATER )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWATER, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWATER, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWCLIP:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_CLIP ) & EXCLUDE_CLIP )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWCLIP, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWCLIP, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWDETAIL:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_DETAIL ) & EXCLUDE_DETAIL )
			{
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWDETAIL, MF_BYCOMMAND | MF_UNCHECKED );
				SetWindowText (g_qeglobals.d_hwndCamera, "Camera View (DETAIL EXCLUDED)");
			}
			else
			{
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWDETAIL, MF_BYCOMMAND | MF_CHECKED );
				SetWindowText (g_qeglobals.d_hwndCamera, "Camera View");
			}
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWWORLD:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_WORLD ) & EXCLUDE_WORLD )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWORLD, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWORLD, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;


//
// grid menu
//
		case ID_GRID_1:
		case ID_GRID_2:
		case ID_GRID_4:
		case ID_GRID_8:
		case ID_GRID_16:
		case ID_GRID_32:
		case ID_GRID_64:
		{
			hMenu = GetMenu(hWnd);

			CheckMenuItem(hMenu, ID_GRID_1, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_2, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_4, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_8, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_16, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_32, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_64, MF_BYCOMMAND | MF_UNCHECKED);

			switch (LOWORD(wParam))
			{
				case ID_GRID_1:
					g_qeglobals.d_gridsize = 0;
					break;
				case ID_GRID_2:
					g_qeglobals.d_gridsize = 1;
					break;
				case ID_GRID_4:
					g_qeglobals.d_gridsize = 2;
					break;
				case ID_GRID_8:
					g_qeglobals.d_gridsize = 3;
					break;
				case ID_GRID_16:
					g_qeglobals.d_gridsize = 4;
					break;
				case ID_GRID_32:
					g_qeglobals.d_gridsize = 5;
					break;
				case ID_GRID_64:
					g_qeglobals.d_gridsize = 6;
					break;
			}
			g_qeglobals.d_gridsize = 1 << g_qeglobals.d_gridsize;

			CheckMenuItem(hMenu, LOWORD(wParam), MF_BYCOMMAND | MF_CHECKED);

			// [31/1/2013] Camera view now relies on this, so update it too! ~hogsy
			Sys_UpdateWindows(W_XY|W_Z|W_CAMERA);
			break;
		}
		// Texture menu
		case ID_VIEW_NEAREST:
		case ID_VIEW_NEARESTMIPMAP:
		case ID_VIEW_LINEAR:
		case ID_VIEW_BILINEAR:
		case ID_VIEW_BILINEARMIPMAP:
		case ID_VIEW_TRILINEAR:
		case ID_TEXTURES_WIREFRAME:
		case ID_TEXTURES_FLATSHADE:
			Texture_SetMode(LOWORD(wParam));
			break;
		case ID_TEXTURES_SHOWINUSE:
			Sys_BeginWait ();
			Texture_ShowInuse ();
			SetInspectorMode(W_TEXTURE);
			break;
		case ID_TEXTURES_INSPECTOR:
			DoSurface ();
			break;
#if 0
		case CMD_TEXTUREWAD:
		case CMD_TEXTUREWAD+1:
		case CMD_TEXTUREWAD+2:
		case CMD_TEXTUREWAD+3:
		case CMD_TEXTUREWAD+4:
		case CMD_TEXTUREWAD+5:
		case CMD_TEXTUREWAD+6:
		case CMD_TEXTUREWAD+7:
		case CMD_TEXTUREWAD+8:
		case CMD_TEXTUREWAD+9:
		case CMD_TEXTUREWAD+10:
		case CMD_TEXTUREWAD+11:
		case CMD_TEXTUREWAD+12:
		case CMD_TEXTUREWAD+13:
		case CMD_TEXTUREWAD+14:
		case CMD_TEXTUREWAD+15:
		case CMD_TEXTUREWAD+16:
		case CMD_TEXTUREWAD+17:
		case CMD_TEXTUREWAD+18:
		case CMD_TEXTUREWAD+19:
		case CMD_TEXTUREWAD+20:
		case CMD_TEXTUREWAD+21:
		case CMD_TEXTUREWAD+22:
		case CMD_TEXTUREWAD+23:
		case CMD_TEXTUREWAD+24:
		case CMD_TEXTUREWAD+25:
		case CMD_TEXTUREWAD+26:
		case CMD_TEXTUREWAD+27:
		case CMD_TEXTUREWAD+28:
		case CMD_TEXTUREWAD+29:
		case CMD_TEXTUREWAD+30:
		case CMD_TEXTUREWAD+31:
		case ID_TEXTURES_SHOWALL:
			Sys_BeginWait();
			Texture_ShowDirectory();
			SetInspectorMode(W_TEXTURE);
			break;
#endif

//
// bsp menu
//
		case CMD_BSPCOMMAND:
		case CMD_BSPCOMMAND+1:
		case CMD_BSPCOMMAND+2:
		case CMD_BSPCOMMAND+3:
		case CMD_BSPCOMMAND+4:
		case CMD_BSPCOMMAND+5:
		case CMD_BSPCOMMAND+6:
		case CMD_BSPCOMMAND+7:
		case CMD_BSPCOMMAND+8:
		case CMD_BSPCOMMAND+9:
		case CMD_BSPCOMMAND+10:
		case CMD_BSPCOMMAND+11:
		case CMD_BSPCOMMAND+12:
		case CMD_BSPCOMMAND+13:
		case CMD_BSPCOMMAND+14:
		case CMD_BSPCOMMAND+15:
		case CMD_BSPCOMMAND+16:
		case CMD_BSPCOMMAND+17:
		case CMD_BSPCOMMAND+18:
		case CMD_BSPCOMMAND+19:
		case CMD_BSPCOMMAND+20:
		case CMD_BSPCOMMAND+21:
		case CMD_BSPCOMMAND+22:
		case CMD_BSPCOMMAND+23:
		case CMD_BSPCOMMAND+24:
		case CMD_BSPCOMMAND+25:
		case CMD_BSPCOMMAND+26:
		case CMD_BSPCOMMAND+27:
		case CMD_BSPCOMMAND+28:
		case CMD_BSPCOMMAND+29:
		case CMD_BSPCOMMAND+30:
		case CMD_BSPCOMMAND+31:
			{
				extern	char	*bsp_commands[256];

				RunBsp (bsp_commands[LOWORD(wParam-CMD_BSPCOMMAND)]);
			}
			break;

//
// misc menu
//
		case ID_TEXTUREBK:
			DoColor(COLOR_TEXTUREBACK);
			Sys_UpdateWindows (W_ALL);
			break;

		case ID_MISC_SELECTENTITYCOLOR:
			{
				extern int inspector_mode;

				if ( ( inspector_mode == W_ENTITY ) && DoColor(COLOR_ENTITY) == TRUE )
				{
					extern void AddProp( void );

					char buffer[100];

					sprintf( buffer, "%f %f %f", g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][0],
						g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][1],
						g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][2] );

					SetWindowText( hwndEnt[EntValueField], buffer );
					SetWindowText( hwndEnt[EntKeyField], "_color" );
					AddProp();
				}
				Sys_UpdateWindows( W_ALL );
			}
			break;

		case ID_MISC_PRINTXY:
			WXY_Print();
			break;

		case ID_COLORS_XYBK:
			DoColor(COLOR_GRIDBACK);
			Sys_UpdateWindows (W_ALL);
			break;

		case ID_COLORS_MAJOR:
			DoColor(COLOR_GRIDMAJOR);
			Sys_UpdateWindows (W_ALL);
			break;

		case ID_COLORS_MINOR:
			DoColor(COLOR_GRIDMINOR);
			Sys_UpdateWindows (W_ALL);
			break;
		case ID_MISC_FINDBRUSH:
			DoFind();
			break;
		case ID_MISC_NEXTLEAKSPOT:
			Pointfile_Next();
			break;
		case ID_MISC_PREVIOUSLEAKSPOT:
			Pointfile_Prev();
			break;

//
// brush menu
//
		case ID_BRUSH_3SIDED:
			Brush_MakeSided (3);
			break;
		case ID_BRUSH_4SIDED:
			Brush_MakeSided (4);
			break;
		case ID_BRUSH_5SIDED:
			Brush_MakeSided (5);
			break;
		case ID_BRUSH_6SIDED:
			Brush_MakeSided (6);
			break;
		case ID_BRUSH_7SIDED:
			Brush_MakeSided (7);
			break;
		case ID_BRUSH_8SIDED:
			Brush_MakeSided (8);
			break;
		case ID_BRUSH_9SIDED:
			Brush_MakeSided (9);
			break;
		case ID_BRUSH_ARBITRARYSIDED:
			DoSides ();
			break;

//
// select menu
//
		case ID_BRUSH_FLIPX:
			Select_FlipAxis (0);
			break;
		case ID_BRUSH_FLIPY:
			Select_FlipAxis (1);
			break;
		case ID_BRUSH_FLIPZ:
			Select_FlipAxis (2);
			break;
		case ID_BRUSH_ROTATEX:
			Select_RotateAxis (0, 90);
			break;
		case ID_BRUSH_ROTATEY:
			Select_RotateAxis (1, 90);
			break;
		case ID_BRUSH_ROTATEZ:
			Select_RotateAxis (2, 90);
			break;

		case ID_SELECTION_ARBITRARYROTATION:
			DoRotate ();
			break;

		case ID_SELECTION_UNGROUPENTITY:
			Select_Ungroup ();
			break;

		case ID_SELECTION_CONNECT:
			ConnectEntities ();
			break;

		case ID_SELECTION_DRAGVERTECIES:
			if (g_qeglobals.d_select_mode == sel_vertex)
			{
				g_qeglobals.d_select_mode = sel_brush;
				Sys_UpdateWindows (W_ALL);
			}
			else
			{
				SetupVertexSelection ();
				if (g_qeglobals.d_numpoints)
					g_qeglobals.d_select_mode = sel_vertex;
			}
			break;
		case ID_SELECTION_DRAGEDGES:
			if (g_qeglobals.d_select_mode == sel_edge)
			{
				g_qeglobals.d_select_mode = sel_brush;
				Sys_UpdateWindows (W_ALL);
			}
			else
			{
				SetupVertexSelection ();
				if (g_qeglobals.d_numpoints)
					g_qeglobals.d_select_mode = sel_edge;
			}
			break;

		case ID_SELECTION_SELECTPARTIALTALL:
			Select_PartialTall ();
			break;
		case ID_SELECTION_SELECTCOMPLETETALL:
			Select_CompleteTall ();
			break;
		case ID_SELECTION_SELECTTOUCHING:
			Select_Touching ();
			break;
		case ID_SELECTION_SELECTINSIDE:
			Select_Inside ();
			break;
		case ID_SELECTION_CSGSUBTRACT:
			CSG_Subtract ();
			break;
		case ID_SELECTION_MAKEHOLLOW:
			CSG_MakeHollow ();
			break;
		case ID_SELECTION_CLONE:
			Select_Clone ();
			break;
		case ID_SELECTION_DELETE:
			Select_Delete ();
			break;
		case ID_SELECTION_DESELECT:
			Select_Deselect ();
			break;
		case ID_SELECTION_MAKE_DETAIL:
			Select_MakeDetail ();
			break;
		case ID_SELECTION_MAKE_STRUCTURAL:
			Select_MakeStructural ();
			break;


//
// region menu
//
		case ID_REGION_OFF:
			Map_RegionOff ();
			break;
		case ID_REGION_SETXY:
			Map_RegionXY ();
			break;
		case ID_REGION_SETTALLBRUSH:
			Map_RegionTallBrush ();
			break;
		case ID_REGION_SETBRUSH:
			Map_RegionBrush ();
			break;
		case ID_REGION_SETSELECTION:
			Map_RegionSelectedBrushes ();
			break;

		case IDMRU+1:
		case IDMRU+2:
		case IDMRU+3:
		case IDMRU+4:
		case IDMRU+5:
		case IDMRU+6:
		case IDMRU+7:
		case IDMRU+8:
		case IDMRU+9:
			DoMru(hWnd,LOWORD(wParam));
			break;

//
// help menu
//

		case ID_HELP_ABOUT:
			DoAbout();
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

LONG WINAPI WMAIN_WndProc (
	HWND    hWnd,
	UINT    uMsg,
	WPARAM  wParam,
	LPARAM  lParam)
{
	RECT	rect;
	HDC		maindc;

	GetClientRect(hWnd, &rect);

	switch (uMsg)
	{
	case WM_TIMER:
		QE_CountBrushesAndUpdateStatusBar();
		QE_CheckAutoSave();
		return 0;
	case WM_DESTROY:
		SaveMruInReg(g_qeglobals.d_lpMruMenu,"Software\\id\\QuakeEd4\\MRU");
		DeleteMruMenu(g_qeglobals.d_lpMruMenu);
		PostQuitMessage(0);
		KillTimer( hWnd, QE_TIMER0 );
		return 0;
	case WM_CREATE:
		maindc = GetDC(hWnd);
		g_qeglobals.d_lpMruMenu = CreateMruMenuDefault();
		LoadMruInReg(g_qeglobals.d_lpMruMenu,"Software\\id\\QuakeEd4\\MRU");

		// Refresh the File menu.
		PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(hWnd),0),ID_FILE_EXIT);

		return 0;
	case WM_SIZE:
		// [27/8/2012] Toolbar fix ~hogsy
		SendMessage(g_qeglobals.hToolBar,TB_AUTOSIZE,0,0);

		// resize the status window
		MoveWindow( g_qeglobals.d_hwndStatus, -100, 100, 10, 10, TRUE);
		return 0;
	case WM_KEYDOWN:
		return QE_KeyDown (wParam);
	case WM_CLOSE:
		// Call destroy window to cleanup and go away
		SaveWindowState(g_qeglobals.d_hwndXY, "xywindow");
		SaveWindowState(g_qeglobals.d_hwndCamera, "camerawindow");
		SaveWindowState(g_qeglobals.d_hwndZ, "zwindow");
		SaveWindowState(g_qeglobals.d_hwndEntity, "EntityWindow");
		SaveWindowState(g_qeglobals.d_hwndMain, "mainwindow");

		// FIXME: is this right?
		SaveRegistryInfo("SavedInfo", &g_qeglobals.d_savedinfo, sizeof(g_qeglobals.d_savedinfo));
		DestroyWindow (hWnd);
		return 0;
	case WM_COMMAND:
		return CommandHandler (hWnd, wParam, lParam);
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);
}

void Main_Create (HINSTANCE hInstance)
{
	WNDCLASSEX	wc;
	int			i;
	HMENU		hMenu;

	// Register the camera class
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= 0;
	wc.lpfnWndProc		= (WNDPROC)WMAIN_WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	// [8/8/2012] Set the icon for the main window ~hogsy
	wc.hIcon			= LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.lpszMenuName		= MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName	= "QUAKE_MAIN";
	wc.hIconSm			= 0;

	if(!RegisterClassEx(&wc))
		Error("WCam_Register: failed");

	g_qeglobals.d_hwndMain = CreateWindowEx(
		0,
		"QUAKE_MAIN",
		"Katana Editor",
		WS_OVERLAPPEDWINDOW|WS_MAXIMIZE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!g_qeglobals.d_hwndMain)
		Error ("Couldn't create main window");

	// Create a timer so that we can count brushes
	SetTimer( g_qeglobals.d_hwndMain,
			  QE_TIMER0,
			  1000,
			  NULL );

	LoadWindowState(g_qeglobals.d_hwndMain,"mainwindow");

	s_hwndToolbar = CreateToolBar(hInstance);

	g_qeglobals.d_hwndStatus = CreateMyStatusWindow(hInstance);

	// Load misc info from registry
	i = sizeof(g_qeglobals.d_savedinfo);
	LoadRegistryInfo("SavedInfo", &g_qeglobals.d_savedinfo,(long*)&i);

	if (g_qeglobals.d_savedinfo.iSize != sizeof(g_qeglobals.d_savedinfo))
	{
		// fill in new defaults
		g_qeglobals.d_savedinfo.iSize				= sizeof(g_qeglobals.d_savedinfo);
		g_qeglobals.d_savedinfo.fGamma				= 1.0f;
		g_qeglobals.d_savedinfo.iTexMenu			= ID_VIEW_NEAREST;
		g_qeglobals.d_savedinfo.exclude				= 0;
		g_qeglobals.d_savedinfo.show_coordinates	= true;
		g_qeglobals.d_savedinfo.bShowNames			= true;
		g_qeglobals.d_savedinfo.bFollowCamera		= false;

		for (i=0 ; i<3 ; i++)
		{
			g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][i]	= 0.25f;
			g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][i]		= 0;
			g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][i]		= 0.2f;
			g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][i]		= 1.0f;
			g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][i]		= 0;
		}
	}

	if ( ( hMenu = GetMenu( g_qeglobals.d_hwndMain ) ) != 0 )
	{
		/*
		** by default all of these are checked because that's how they're defined in the menu editor
		*/
		if(!g_qeglobals.d_savedinfo.bShowNames )
			CheckMenuItem( hMenu, ID_VIEW_SHOWNAMES, MF_BYCOMMAND | MF_UNCHECKED );

		if(!g_qeglobals.d_savedinfo.show_coordinates)
			CheckMenuItem( hMenu, ID_VIEW_SHOWCOORDINATES, MF_BYCOMMAND | MF_UNCHECKED );

		if(g_qeglobals.d_savedinfo.bFollowCamera)
			CheckMenuItem(hMenu,ID_VIEW_FOLLOWCAM,MF_BYCOMMAND|MF_CHECKED);

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTS )
			CheckMenuItem( hMenu, ID_VIEW_SHOWLIGHTS, MF_BYCOMMAND | MF_UNCHECKED );

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_ENT )
			CheckMenuItem( hMenu, ID_VIEW_ENTITY, MF_BYCOMMAND | MF_UNCHECKED );

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS )
			CheckMenuItem( hMenu, ID_VIEW_SHOWPATH, MF_BYCOMMAND | MF_UNCHECKED );

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_WATER )
			CheckMenuItem( hMenu, ID_VIEW_SHOWWATER, MF_BYCOMMAND | MF_UNCHECKED );

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_WORLD )
			CheckMenuItem( hMenu, ID_VIEW_SHOWWORLD, MF_BYCOMMAND | MF_UNCHECKED );

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP )
			CheckMenuItem( hMenu, ID_VIEW_SHOWCLIP, MF_BYCOMMAND | MF_UNCHECKED );
	}

	ShowWindow(g_qeglobals.d_hwndMain,SW_SHOWMAXIMIZED);
}

/*
=============================================================

REGISTRY INFO

=============================================================
*/

BOOL SaveRegistryInfo(const char *pszName, void *pvBuf, long lSize)
{
	LONG lres;
	DWORD dwDisp;
	HKEY  hKeyId;

	lres = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\id\\QuakeEd4", 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyId, &dwDisp);

	if (lres != ERROR_SUCCESS)
		return false;

	lres = RegSetValueEx(hKeyId, pszName, 0, REG_BINARY,(const BYTE *)pvBuf, lSize);

	RegCloseKey(hKeyId);

	if (lres != ERROR_SUCCESS)
		return false;

	return true;
}

BOOL LoadRegistryInfo(const char *pszName, void *pvBuf, long *plSize)
{
	HKEY  hKey;
	long lres, lType, lSize;

	if (plSize == NULL)
		plSize = &lSize;

	lres = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\id\\QuakeEd4", 0, KEY_READ, &hKey);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	lres = RegQueryValueEx(hKey, pszName, NULL, &lType, pvBuf, plSize);

	RegCloseKey(hKey);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL SaveWindowState(HWND hWnd, const char *pszName)
{
	RECT rc;

	GetWindowRect(hWnd, &rc);
	if (hWnd != g_qeglobals.d_hwndMain)
		MapWindowPoints(NULL, g_qeglobals.d_hwndMain, (POINT *)&rc, 2);
	return SaveRegistryInfo(pszName, &rc, sizeof(rc));
}

void LoadWindowState(HWND hWnd,const char *pszName)
{
	RECT rc;
	LONG lSize = sizeof(rc);

	if (LoadRegistryInfo(pszName, &rc, &lSize))
	{
		if (rc.left < 0)
			rc.left = 0;
		if (rc.top < 0)
			rc.top = 0;
		if (rc.right < rc.left + 16)
			rc.right = rc.left + 16;
		if (rc.bottom < rc.top + 16)
			rc.bottom = rc.top + 16;

		MoveWindow(hWnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
	}
}

/*
===============================================================

  STATUS WINDOW

===============================================================
*/

void Sys_UpdateStatusBar( void )
{
	extern int   g_numbrushes, g_numentities;

	char numbrushbuffer[100]="";

	sprintf(numbrushbuffer,"Brushes: %d Entities: %d",g_numbrushes,g_numentities);

	Sys_Status(numbrushbuffer,2);
}

void Sys_Status(const char *psz, int part )
{
	SendMessage(g_qeglobals.d_hwndStatus, SB_SETTEXT, part, (LPARAM)psz);
}

static HWND CreateMyStatusWindow(HINSTANCE hInst)
{
	HWND hWnd;
	int partsize[3] = { 300, 1100, -1 };

	hWnd = CreateWindowEx(
		WS_EX_TOPMOST,						// no extended styles
		STATUSCLASSNAME,                 // status bar
		"",                              // no text
		// [8/8/2012] Added WS_CLIPSIBLINGS which seems to fix some issues ~hogsy
		WS_CHILD|WS_BORDER|WS_VISIBLE|WS_CLIPSIBLINGS,  // styles
		-100, -100, 10, 10,              // x, y, cx, cy
		g_qeglobals.d_hwndMain,          // parent window
		(HMENU)100,                      // window ID
		hInst,                           // instance
		NULL);							 // window data

	SendMessage(hWnd,SB_SETPARTS,3,(long)partsize);

	return hWnd;
}

//==============================================================

#define NUMBUTTONS 18
HWND CreateToolBar(HINSTANCE hinst)
{
	TBADDBITMAP				tbab;
	TBBUTTON				tbb[NUMBUTTONS];
	int						i = 0;

	// Create a toolbar that the user can customize and that has a
	// tooltip associated with it.
	g_qeglobals.hToolBar = CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		(LPSTR)NULL,
		// [8/8/2012] Added WS_CLIPSIBLINGS to deal with clipping issues ~hogsy
		WS_CHILD|TBSTYLE_TOOLTIPS|WS_CLIPSIBLINGS,
		0,
		0,
		0,
		0,
		g_qeglobals.d_hwndMain,
		(HMENU)IDR_TOOLBAR1,
		hinst,
		NULL);

	// Send the TB_BUTTONSTRUCTSIZE message, which is required for
	// backward compatibility.
	SendMessage(g_qeglobals.hToolBar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);
	SendMessage(g_qeglobals.hToolBar,TB_SETMAXTEXTROWS,(WPARAM)0,0);					// [8/8/2012] Set TB_SETMAXTEXTROWS ~hogsy

	// Add the bitmap containing button images to the toolbar.
	tbab.hInst = hinst;
	tbab.nID   = IDR_TOOLBAR1;
	SendMessage(g_qeglobals.hToolBar,TB_ADDBITMAP,(WPARAM)NUMBUTTONS,(WPARAM)&tbab);

	// Fill the TBBUTTON array with button information, and add the
	// buttons to the toolbar.
	tbb[i].iBitmap		= 0;
	tbb[i].idCommand	= 0;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_SEP;
	tbb[i].dwData		= 0;
	tbb[i].iString		= 0;
	i++;

	tbb[i].iBitmap		= 0;
	tbb[i].idCommand	= ID_BRUSH_FLIPX;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Flip X";
	i++;

	tbb[i].iBitmap		= 2;
	tbb[i].idCommand	= ID_BRUSH_FLIPY;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Flip Y";
	i++;

	tbb[i].iBitmap		= 4;
	tbb[i].idCommand	= ID_BRUSH_FLIPZ;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Flip Z";
	i++;

	tbb[i].iBitmap		= 1;
	tbb[i].idCommand	= ID_BRUSH_ROTATEX;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Rotate X";
	i++;

	tbb[i].iBitmap		= 3;
	tbb[i].idCommand	= ID_BRUSH_ROTATEY;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Rotate Y";
	i++;

	tbb[i].iBitmap		= 5;
	tbb[i].idCommand	= ID_BRUSH_ROTATEZ;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Rotate Z";
	i++;

	tbb[i].iBitmap		= 0;
	tbb[i].idCommand	= 0;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_SEP;
	tbb[i].dwData		= 0;
	tbb[i].iString		= 0;
	i++;

	tbb[i].iBitmap		= 6;
	tbb[i].idCommand	= ID_SELECTION_SELECTCOMPLETETALL;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= 0;
	i++;

	tbb[i].iBitmap		= 7;
	tbb[i].idCommand	= ID_SELECTION_SELECTTOUCHING;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Select Touching";
	i++;

	tbb[i].iBitmap		= 8;
	tbb[i].idCommand	= ID_SELECTION_SELECTPARTIALTALL;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= 0;
	i++;

	tbb[i].iBitmap		= 9;
	tbb[i].idCommand	= ID_SELECTION_SELECTINSIDE;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Select Inside";
	i++;

	tbb[i].iBitmap		= 0;
	tbb[i].idCommand	= 0;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_SEP;
	tbb[i].dwData		= 0;
	tbb[i].iString		= 0;
	i++;

	tbb[i].iBitmap		= 10;
	tbb[i].idCommand	= ID_SELECTION_CSGSUBTRACT;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"CSG Subtract";
	i++;

	tbb[i].iBitmap		= 11;
	tbb[i].idCommand	= ID_SELECTION_MAKEHOLLOW;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Make Hollow";
	i++;

	tbb[i].iBitmap		= 12;
	tbb[i].idCommand	= ID_TEXTURES_WIREFRAME;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Wireframe";
	i++;

	tbb[i].iBitmap		= 13;
	tbb[i].idCommand	= ID_TEXTURES_FLATSHADE;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Flat";
	i++;

	tbb[i].iBitmap		= 14;
	tbb[i].idCommand	= ID_VIEW_TRILINEAR;
	tbb[i].fsState		= TBSTATE_ENABLED;
	tbb[i].fsStyle		= BTNS_BUTTON;
	tbb[i].dwData		= 0;
	tbb[i].iString		= (INT_PTR)"Textured";

	SendMessage(g_qeglobals.hToolBar, TB_ADDBUTTONS, (WPARAM)NUMBUTTONS,
		(LPARAM) (LPTBBUTTON) &tbb);

	ShowWindow(g_qeglobals.hToolBar, SW_SHOW);

	return g_qeglobals.hToolBar;
}

