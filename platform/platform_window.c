/*	Copyright (C) 1999 Mete Ciragan
	Copyright (C) 2013-2014 OldTimes Software
*/
#include "platform_window.h"

/*
	gWindow
	Window/Display management.
*/

#include "platform_system.h"

#ifdef __linux__
Display *dMainDisplay;
Window  wRootWindow;
#endif

pINSTANCE iGlobalInstance;

bool	bShowingCursor	    = true,		// Are we showing the cursor?
		bWindowInitialized	= false;	// Is the window system initialized?

int	iActive = 0,	// Number of current active windows.
	iScreen;		// Default screen.

/*	
	Display Information
*/

int	pWindow_GetScreenWidth(void)
{
#ifdef _WIN32
	return 0;
#else
	return 0;
#endif
}

int pWindow_GetScreenHeight(void)
{
#ifdef _WIN32
	return 0;
#else
	return 0;
#endif
}

int pWindow_GetMonitorCount(void)
{
#ifdef _WIN32
	return GetSystemMetrics(SM_CMONITORS);
#else
	return XScreenCount(dMainDisplay);
#endif
}

/**/

GIPLWindow_t *gWindow_Allocate(void)
{
	GIPLWindow_t *pwAllocated;

	pERROR_UPDATE;

	pwAllocated = (GIPLWindow_t*)malloc(sizeof(GIPLWindow_t));
	// Don't continue if we failed to allocate.
	if(!pwAllocated)
	{
		gWindow_MessageBox("Platform Error","Failed to allocate a new window!\n");
		return NULL;
	}

	memset(pwAllocated,0,sizeof(GIPLWindow_t));

	return pwAllocated;
}

/*	Create a new window.
*/
void gWindow_CreateWindow(GIPLWindow_t *gwWindow)
{
	pERROR_UPDATE;

	// Make sure the window has been initialized.
	if(!gwWindow)
	{
		pError_Set("Window has not been allocated!\n");
		return;
	}
	// Make sure that any platform specific window systems are set up.
	else if(!bWindowInitialized)
	{
#ifdef __linux__
		dMainDisplay = XOpenDisplay(NULL);
		if(!dMainDisplay)
		{
			pError_Set("Failed to open display!\n");
			return;
		}

		iScreen = DefaultScreen(dMainDisplay);
#else
#endif

		bWindowInitialized = true;
	}

#ifdef _WIN32
	{
		INITCOMMONCONTROLSEX	iCommonControls;
		WNDCLASSEX				wWindowClass;

		iCommonControls.dwICC	= 0;
		iCommonControls.dwSize	= sizeof(INITCOMMONCONTROLSEX);

#if 0
		if(!InitCommonControlsEx(&iCommonControls))
		{
			Platform_SetError("Failed to initialize common controls!\n");

			return;
		}
#endif

		wWindowClass.cbClsExtra		= 0;
		wWindowClass.cbSize			= sizeof(WNDCLASSEX);
		wWindowClass.cbWndExtra		= 0;
		wWindowClass.hbrBackground	= NULL;
		wWindowClass.hCursor		= LoadCursor(iGlobalInstance,IDC_ARROW);
		wWindowClass.hIcon			= LoadIcon(iGlobalInstance,IDI_APPLICATION);
		wWindowClass.hIconSm		= 0;
		wWindowClass.hInstance		= iGlobalInstance;
		wWindowClass.lpfnWndProc	= NULL;	// (WNDPROC)Platform_WindowProcedure;
		wWindowClass.lpszClassName	= gwWindow->cClass;
		wWindowClass.lpszMenuName	= 0;
		wWindowClass.style			= CS_OWNDC;

		if(!RegisterClassEx(&wWindowClass))
		{
			pError_Set("Failed to register window class!\n");
			return;
		}

		gwWindow->hWindow = CreateWindowEx(
			0,
			gwWindow->cClass,
			gwWindow->cTitle,
			WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			gwWindow->x,
			gwWindow->y,
			gwWindow->iWidth,
			gwWindow->iHeight,
			NULL,NULL,
			iGlobalInstance,
			NULL);
		if(!gwWindow->hWindow)
		{
			pError_Set("Failed to create window!\n");
			return;
		}

		ShowWindow(gwWindow->hWindow,SW_SHOWDEFAULT);

		UpdateWindow(gwWindow->hWindow);

		SetForegroundWindow(gwWindow->hWindow);

		gwWindow->hDeviceContext = GetDC(gwWindow->hWindow);
	}
#else	// Linux
	{
		// Create our window.
		gwWindow->wInstance = XCreateSimpleWindow(
			dMainDisplay,
			RootWindow(dMainDisplay,iScreen),
			gwWindow->x,
			gwWindow->y,
			gwWindow->iWidth,
			gwWindow->iHeight,
			1,
			BlackPixel(dMainDisplay,iScreen),
			WhitePixel(dMainDisplay,iScreen));
		if(!gwWindow->wInstance)
		{
			pError_Set("Failed to create window!\n");
			return;
		}

		// Set the window title.
		XStoreName(dMainDisplay,gwWindow->wInstance,gwWindow->cTitle);
	}
#endif

	gwWindow->bActive = true;

	iActive++;
}

/*
	Dialogue Windows
*/

bool gWindow_ChooseColor(GIPLWindow_t *gwParent,int *iRed,int *iGreen,int *iBlue)
{
#ifdef _WIN32
	pBYTE					bColour[3];
	CHOOSECOLOR				ccColour;
	static		COLORREF	scCustomColours[16];

	bColour[0]	= (pBYTE)*iRed;
	bColour[1]	= (pBYTE)*iGreen;
	bColour[2]	= (pBYTE)*iBlue;

	memset(&ccColour,0,sizeof(CHOOSECOLOR));

	ccColour.Flags			= CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
	ccColour.hwndOwner		= gwParent?(HWND)gwParent->hWindow:NULL;
	ccColour.lpCustColors	= scCustomColours;
	ccColour.lStructSize	= sizeof(CHOOSECOLOR);
	ccColour.rgbResult		= RGB(bColour[0],bColour[1],bColour[2]);

	if(ChooseColor(&ccColour))
	{
		*iRed	= (int)GetRValue(ccColour.rgbResult);
		*iGreen = (int)GetGValue(ccColour.rgbResult);
		*iBlue	= (int)GetBValue(ccColour.rgbResult);
		return true;
	}
#else	// Linux
#endif

	return false;
}

/*	Displays a simple dialogue window.
	TODO:
		Support for alternative dialog types? Error etc.
*/
void gWindow_MessageBox(const char *ccTitle,const char *ccMessage,...)
{
	char	cOut[2048];
	va_list	vlArguments;

	pERROR_UPDATE;

	va_start(vlArguments,ccMessage);
	vsprintf(cOut,ccMessage,vlArguments);
	va_end(vlArguments);

	// [28/10/2013] Also print the message out ~hogsy
	printf("Platform: %s",cOut);

#ifndef _WIN32
	{
		int     iDefaultScreen;
		XEvent  xEvent;
		Display *dMessageDisplay;
		Window  wMessageWindow;

		dMessageDisplay = XOpenDisplay(NULL);
		if(!dMessageDisplay)
		{
			pError_Set("Failed to open display!\n");
			return;
		}

		iDefaultScreen = DefaultScreen(dMessageDisplay);

		wMessageWindow = XCreateSimpleWindow(
			dMessageDisplay,
			RootWindow(dMessageDisplay,iDefaultScreen),
			50,50,
			512,64,
			1,
			BlackPixel(dMessageDisplay,iDefaultScreen),
			WhitePixel(dMessageDisplay,iDefaultScreen));
		XStoreName(dMessageDisplay,wMessageWindow,ccTitle);
		XSelectInput(dMessageDisplay,wMessageWindow,ExposureMask|KeyPressMask);
		XMapWindow(dMessageDisplay,wMessageWindow);

		for(;;)
		{
			XNextEvent(dMessageDisplay,&xEvent);

			if(xEvent.type == Expose)
			{
				XDrawString(dMessageDisplay,wMessageWindow,DefaultGC(dMessageDisplay,iDefaultScreen),10,10,cOut,strlen(cOut));
				XDrawString(dMessageDisplay,wMessageWindow,DefaultGC(dMessageDisplay,iDefaultScreen),10,54,"Press any key to continue...",32);
			}
			else if(xEvent.type == KeyPress)
				break;
		}

		XCloseDisplay(dMessageDisplay);
	}
#else   // Windows
	MessageBox(NULL,cOut,ccTitle,MB_SETFOREGROUND|MB_ICONERROR);
#endif
}

void gWindow_Destroy(GIPLWindow_t *gwWindow)
{
#ifdef _WIN32
	if(!gwWindow->hWindow)
		return;

	// Destroy our window.
	DestroyWindow(gwWindow->hWindow);
#else	// Linux
	// Close our display instance.
	if(dMainDisplay)
		XCloseDisplay(dMainDisplay);
#endif

	free(gwWindow);

	iActive--;
}

/*  Shows or hides the cursor for
	the active window.
*/
void gWindow_ShowCursor(bool bShow)
{
	if(bShow == bShowingCursor)
		return;

#ifdef _WIN32
	ShowCursor(bShow);
#else	// Linux
#endif

	bShowingCursor = bShow;
}

/*	Gets the position of the cursor.
	TODO:
		Move into platform_input.
*/
void gWindow_GetCursorPosition(int *iX,int *iY)
{
#ifdef _WIN32
	POINT	pPoint;

	GetCursorPos(&pPoint);

	*iX = pPoint.x;
	*iY	= pPoint.y;
#else	// Linux
#endif
}

/*
	RENDERING
*/

void gWindow_SwapBuffers(GIPLWindow_t *gwWindow)
{
#ifdef _WIN32
	SwapBuffers(gwWindow->hDeviceContext);
#else	// Linux
	//glXSwapBuffers() // todo
#endif
}
