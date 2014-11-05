#include "editor_camera.h"

LONG WINAPI WCam_WndProc (
	HWND    hWnd,
	UINT    uMsg,
	WPARAM  wParam,
	LPARAM  lParam)
{
	int		fwKeys, xPos, yPos;

	GetClientRect(hWnd,&g_qeglobals.rRectCamera);

	switch (uMsg)
	{
	case WM_CREATE:
		{
			HFONT	hfont;
			
			g_qeglobals.d_hdcBase = GetDC(hWnd);
			QEW_SetupPixelFormat(g_qeglobals.d_hdcBase, TRUE);

			if ( ( g_qeglobals.d_hglrcBase = wglCreateContext( g_qeglobals.d_hdcBase ) ) == 0 )
				Error ("wglCreateContext failed");
			else if (!wglMakeCurrent( g_qeglobals.d_hdcBase, g_qeglobals.d_hglrcBase ))
				Error ("wglMakeCurrent failed");

			Texture_SetMode(g_qeglobals.d_savedinfo.iTexMenu);

			// Create GL font
			hfont = CreateFont(
				10,						// logical height of font 
				7,						// logical average character width 
				0,						// angle of escapement 
				0,						// base-line orientation angle 
				FW_NORMAL,				// font weight 
				0,						// italic attribute flag 
				0,						// underline attribute flag 
				0,						// strikeout attribute flag 
				ANSI_CHARSET,			// character set identifier 
				OUT_DEFAULT_PRECIS,		// output precision 
				CLIP_DEFAULT_PRECIS,	// clipping precision 
				DEFAULT_QUALITY,		// output quality 
				0,						// pitch and family 
				0 						// pointer to typeface name string 
				);

			if ( !hfont )
				Error( "couldn't create font" );

			SelectObject(g_qeglobals.d_hdcBase, hfont);

			if ( ( g_qeglobals.d_font_list = glGenLists (256) ) == 0 )
				Error( "couldn't create font dlists" );
			
			// create the bitmap display lists
			// we're making images of glyphs 0 thru 255
			if(!wglUseFontBitmaps(g_qeglobals.d_hdcBase,1,255,g_qeglobals.d_font_list))
				// [8/8/2012] Fixed typo ~hogsy
				Error("wglUseFontBitmaps failed");
			
			// indicate start of glyph display lists
			glListBase(g_qeglobals.d_font_list);
		}
		return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			
			if (!wglMakeCurrent( g_qeglobals.d_hdcBase, g_qeglobals.d_hglrcBase ))
				Error ("wglMakeCurrent failed");

			if(BeginPaint(hWnd,&ps))
			{
				Camera_Draw();

				EndPaint(hWnd, &ps);
			
				SwapBuffers(g_qeglobals.d_hdcBase);
			}
		}

		return 0;
	case WM_KEYDOWN:
		if ( QE_KeyDown (wParam) )
			return 0;
		else 
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if(GetTopWindow(g_qeglobals.d_hwndMain) != hWnd)
			BringWindowToTop(hWnd);
		
		SetFocus (g_qeglobals.d_hwndCamera);
		SetCapture (g_qeglobals.d_hwndCamera);
		fwKeys = wParam;        // key flags 
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		yPos = (short)HIWORD(lParam);  // vertical position of cursor 
		yPos = (int)g_qeglobals.rRectCamera.bottom - 1 - yPos;

		Cam_MouseDown (xPos, yPos, fwKeys);

		return 0;
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		fwKeys = wParam;        // key flags 
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		yPos = (short)HIWORD(lParam);  // vertical position of cursor 
		yPos = (int)g_qeglobals.rRectCamera.bottom - 1 - yPos;

		Cam_MouseUp();

		if(!(fwKeys & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
			ReleaseCapture ();
		return 0;
	case WM_MOUSEMOVE:
		fwKeys = wParam;        // key flags 
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		yPos = (short)HIWORD(lParam);  // vertical position of cursor 
		yPos = (int)g_qeglobals.rRectCamera.bottom - 1 - yPos;
		Cam_MouseMoved (xPos, yPos, fwKeys);
		return 0;
	case WM_SIZE:
		camera.width = g_qeglobals.rRectCamera.right;
		camera.height = g_qeglobals.rRectCamera.bottom;
		InvalidateRect(g_qeglobals.d_hwndCamera, NULL, FALSE);
		return 0;
	case WM_KILLFOCUS:
	case WM_SETFOCUS:
		SendMessage( hWnd, WM_NCACTIVATE, uMsg == WM_SETFOCUS, 0 );
		return 0;
	case WM_NCCALCSIZE:// don't let windows copy pixels
		DefWindowProc (hWnd, uMsg, wParam, lParam);
		return WVR_REDRAW;
	case WM_CLOSE:
		DestroyWindow (hWnd);
		return 0;
	case WM_DESTROY:
		QEW_StopGL( hWnd, g_qeglobals.d_hglrcBase, g_qeglobals.d_hdcBase );
		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

void WCam_Create(HINSTANCE hInstance)
{
	RECT		rRect,rStatus;
	WNDCLASSEX	wc;
	char		*title;

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= 0;
	wc.lpfnWndProc		= (WNDPROC)WCam_WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(NULL,IDI_APPLICATION);
	wc.hIconSm			= LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;	//(char*)IDR_MENU2;
	wc.lpszClassName	= CAMERA_WINDOW_CLASS;

	if(!RegisterClassEx(&wc))
		Error ("WCam_Register: failed");

	if(g_qeglobals.d_savedinfo.exclude & EXCLUDE_DETAIL)
		title = "Camera (DETAIL EXCLUDED)";
	else
		title = "Camera";

	GetClientRect(g_qeglobals.d_hwndMain,&rRect);
	GetClientRect(g_qeglobals.d_hwndStatus,&rStatus);

	g_qeglobals.d_hwndCamera = CreateWindowEx(
		0,
		CAMERA_WINDOW_CLASS,
		title,
		QE3_STYLE|WS_SYSMENU,
		0,0,512,512,	//rRect.left+(ZWIN_WIDTH+2),
						//rRect.top+TWIN_HEIGHT,
						//CAMERA_WIDTH,
						//(rRect.top+rRect.bottom)-((rStatus.bottom*4)-24),
		g_qeglobals.d_hwndMain,			// parent window
		NULL,							// no menu
		hInstance,
		NULL);
	if(!g_qeglobals.d_hwndCamera)
		Error("Failed to create Camera window!");

//	LoadWindowState(g_qeglobals.d_hwndCamera,"camerawindow");
//	ShowWindow(g_qeglobals.d_hwndCamera,SW_SHOWNOACTIVATE);
}
