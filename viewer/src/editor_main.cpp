/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "platform.h"

#include <mx/mx.h>
#include <mx/GL.H>
#include <mx/mxTga.h>

#include "okv_main.h"
#include "gl_model.h"
#include "gl_window.h"

#include "platform_module.h"

#include "shared_engine.h"

EngineExport_t	*Engine;
EngineImport_t	*Editor;

pINSTANCE	iEditorEngineInstance;

KatanaViewer *g_md2Viewer = 0;
extern bool bFilterTextures;
static char recentFiles[8][256] = 
{ 
	"", 
	"", 
	"", 
	"", 
	"", 
	"", 
	"", 
	"" 
};

void KatanaViewer::initRecentFiles ()
{
	for (int i = 0; i < 8; i++)
	{
		if (strlen (recentFiles[i]))
		{
			mb->modify (IDC_MODEL_RECENTMODELS1 + i, IDC_MODEL_RECENTMODELS1 + i, recentFiles[i]);
		}
		else
		{
			mb->modify (IDC_MODEL_RECENTMODELS1 + i, IDC_MODEL_RECENTMODELS1 + i, "(empty)");
			mb->setEnabled (IDC_MODEL_RECENTMODELS1 + i, false);
		}
	}
}

void KatanaViewer::loadRecentFiles ()
{
	char paths[256];

	strcpy (paths, mx::getApplicationPath ());
	strcat (paths, "viewer.crf");

	FILE *file = fopen (paths, "rb");
	if (file)
	{
		fread (recentFiles, sizeof recentFiles, 1, file);
		fclose (file);
	}
	else
	{
		saveRecentFiles();
		loadRecentFiles();
	}
}

void KatanaViewer::saveRecentFiles ()
{
	char path[256];

	strcpy (path, mx::getApplicationPath ());
	// [17/2/2014] Fixed, just look in our current directory for this! ~hogsy
	strcat (path, "viewer.crf");

	FILE *file = fopen (path, "wb");
	if (file)
	{
		fwrite (recentFiles, sizeof recentFiles, 1, file);
		fclose (file);
	}
}

KatanaViewer::KatanaViewer() : mxWindow (0, 0, 0, 0, 0, EDITOR_WINDOW_TITLE, mxWindow::Normal)
{
	mb = new mxMenuBar (this);
	mxMenu *menuModel = new mxMenu ();
	mxMenu *menuSkin = new mxMenu ();
	mxMenu *menuOptions = new mxMenu ();
	mxMenu *menuView = new mxMenu ();
	mxMenu *menuHelp = new mxMenu ();

	mb->addMenu ("Model", menuModel);
	mb->addMenu ("Skin", menuSkin);
	mb->addMenu ("Options", menuOptions);
	mb->addMenu ("View", menuView);
	mb->addMenu ("Help", menuHelp);

	mxMenu *menuRecentModels = new mxMenu ();
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS1);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS2);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS3);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS4);

	menuModel->add ("Load Model...", IDC_MODEL_LOADMODEL);
	menuModel->add ("Load Overlay Model...", IDC_MODEL_LOADOVERLAY);
	menuModel->addSeparator ();
	menuModel->add ("Unload Model", IDC_MODEL_UNLOADMODEL);
	menuModel->add ("Unload Overlay Model", IDC_MODEL_UNLOADOVERLAY);
	menuModel->addSeparator ();
	menuModel->addMenu ("Recent Models", menuRecentModels);
	menuModel->addSeparator ();
	menuModel->add ("Exit", IDC_MODEL_EXIT);

	menuSkin->add ("Load Model Skin...", IDC_SKIN_MODELSKIN);
	menuSkin->add ("Load Overlay Model Skin...", IDC_SKIN_OVERLAYSKIN);
	menuSkin->addSeparator ();
	menuSkin->add ("Load Background Texture...", IDC_SKIN_BACKGROUND);
	menuSkin->add ("Load Water Texture...", IDC_SKIN_WATER);
#ifdef WIN32
	menuSkin->addSeparator ();
	menuSkin->add ("Make Screenshot...", IDC_SKIN_SCREENSHOT);
#endif
	menuOptions->add ("Background Color...", IDC_OPTIONS_BGCOLOR);
	menuOptions->add ("Grid Color...", IDC_OPTIONS_GRIDCOLOR);
	menuOptions->add ("Wireframe Color...", IDC_OPTIONS_WFCOLOR);
	menuOptions->add ("Shade Color...", IDC_OPTIONS_FGCOLOR);
	menuOptions->add ("Light Color...", IDC_OPTIONS_LIGHTCOLOR);
	menuOptions->add ("Filter Textures", IDC_OPTIONS_FILTERTEXTURES);
		mb->setChecked (IDC_OPTIONS_FILTERTEXTURES, bFilterTextures);
	menuView->add ("Reset Camera", IDC_RESETVIEW);
	menuView->add ("ViewModel Camera", IDC_VIEWMODEL);
	menuView->add ("Center Model", IDC_OPTIONS_CENTERMODEL);
#ifdef WIN32
	menuHelp->add ("Go to Homepage...", IDC_HELP_GOTOHOMEPAGE);
	menuHelp->addSeparator ();
#endif
	menuHelp->add ("About", IDC_HELP_ABOUT);

	tab = new mxTab (this, 0, 0, 0, 0);
#ifdef WIN32
	SetWindowLong ((HWND) tab->getHandle (), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
#endif

	mxWindow *wAnim = new mxWindow (this, 0, 0, 0, 0);
	mxWindow *wInfo = new mxWindow (this, 0, 0, 0, 0);
	mxWindow *wView = new mxWindow (this, 0, 0, 0, 0);
	mxWindow *wTexture = new mxWindow (this, 0, 0, 0, 0);
	tab->add (wView, "View");
	tab->add (wAnim, "Animation");
	tab->add (wTexture, "Texture");
	tab->add (wInfo, "Model Info");
		cRenderMode = new mxChoice (wView, 5, 5, 200, 22, IDC_RENDERMODE);
			cRenderMode->add ("Wireframe");
			cRenderMode->add ("Flat shaded");
			cRenderMode->add ("Smooth Shaded");
			cRenderMode->add ("Textured Flat");
			cRenderMode->add ("Textured");
			cRenderMode->select (0);
		cbWater = new mxCheckBox (wView, 5, 30, 100, 22, "Enable Water", IDC_WATER);
		cbLight = new mxCheckBox (wView, 5, 55, 100, 22, "Enable Light", IDC_LIGHT);
			cbLight->setChecked (true);
		mxSlider *slBrightness = new mxSlider (wView, 105, 32, 100, 16, IDC_BRIGHTNESS);
		mxButton *bReloadTx = new mxButton (wView, 205, 32, 90, 16, "Apply Brightness", IDC_RELOADTEX);
			slBrightness->setRange (0, 100);
			slBrightness->setValue (5);
		mxCheckBox *cbGrid = new mxCheckBox (wView, 105, 55, 100, 22, "Grid", IDC_GRID);
			cbGrid->setChecked (true);
		cbBackground = new mxCheckBox (wView, 205, 55, 100, 22, "Background", IDC_BACKGROUND);
		cAnim = new mxChoice (wAnim, 5, 5, 200, 22, IDC_ANIMATION);
		mxCheckBox *cbInterp = new mxCheckBox (wAnim, 5, 30, 100, 22, "Interpolate", IDC_INTERPOLATE);
			cbInterp->setChecked (true);
		mxCheckBox *cbGlCmds = new mxCheckBox (wAnim, 105, 30, 100, 22, "Strips and Fans", IDC_GLCOMMANDS);
			cbGlCmds->setChecked (true);
		mxSlider *slPitch = new mxSlider (wAnim, 5, 55, 200, 22, IDC_PITCH);
			slPitch->setRange (1, 200);
			slPitch->setValue (50);
			mxToolTip::add (slPitch, "Frame Animation Speed (Pitch)");
		bPause = new mxButton (wAnim, 210, 5, 79, 22, "Pause", IDC_PAUSE);
		bViewModel = new mxButton (wView, 310, 5, 84, 22, "ViewModel Cam", IDC_VIEWMODEL);
		leFoV = new mxLineEdit (wView, 435, 65, 25, 16, "75");
		mxLabel *lfov = new mxLabel (wView, 315, 50, 90, 125, "Field of View:");
		slfov = new mxSlider (wView, 310, 65, 120, 16, IDC_FOV);
			slfov->setRange (1, 160);
			slfov->setValue (75);
		bDecFrame = new mxButton (wAnim, 211, 31, 20, 20, "<", IDC_DECFRAME);
			bDecFrame->setEnabled (false);
			mxToolTip::add (bDecFrame, "Decrease Current Frame");
		leFrame = new mxLineEdit (wAnim, 232, 30, 35, 22, "0");
			leFrame->setEnabled (false);
			mxToolTip::add (leFrame, "Current Frame");
		bSetFrame = new mxButton (wAnim, 232, 51, 35, 20, "Set", IDC_FRAME);
			bSetFrame->setEnabled (false);
			mxToolTip::add (bSetFrame, "Set Current Frame");
		bIncFrame = new mxButton (wAnim, 268, 31, 20, 20, ">", IDC_INCFRAME);
			bIncFrame->setEnabled (false);
			mxToolTip::add (bIncFrame, "Increase Current Frame");
		cbBackground = new mxCheckBox (wTexture, 5, 5, 100, 16, "Display Texture", IDC_DRAWTEXTURE);
		mxLabel *ltxscl = new mxLabel (wTexture, 10, 40, 150, 125, "Scale Texture:");
		mxSlider *slTexscale = new mxSlider (wTexture, 5, 55, 100, 16, IDC_TEXTURESCALE);
			slTexscale->setRange (1, 200);
			slTexscale->setValue (50);
		mxLabel *ltxalpha = new mxLabel (wTexture, 120, 40, 150, 125, "Texture Alpha:");
		mxSlider *slTexalpha = new mxSlider (wTexture, 115, 55, 100, 16, IDC_TEXTUREALPHA);
			slTexalpha->setRange (1, 100);
			slTexalpha->setValue (100);
		lModelInfo1 = new mxLabel (wInfo, 5, 5, 150, 125, "No Model.");
		lModelInfo2 = new mxLabel (wInfo, 155, 5, 150, 125, "No Overlay.");

	glw = new GlWindow(this,0,0,0,0,"",mxWindow::Normal);

	loadRecentFiles ();
	initRecentFiles ();

	setBounds (20, 20, 690, 550);
	setVisible (true);
}

KatanaViewer::~KatanaViewer ()
{
	saveRecentFiles ();
}

int KatanaViewer::handleEvent (mxEvent *event)
{
	switch (event->event)
	{
	case mxEvent::Action:
	{
		switch (event->action)
		{
		case IDC_MODEL_LOADMODEL:
		case IDC_MODEL_LOADOVERLAY:
		{
			const char *ptr = mxGetOpenFileName(this,"./../","*.md2");
			if(ptr)
			{
				if (!loadModel (ptr, event->action - IDC_MODEL_LOADMODEL))
				{
					char str[256];

					sprintf (str, "Error reading model: %s", ptr);
					mxMessageBox (this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
					break;
				}
				int i;
				char path[256];

				if (event->action == IDC_MODEL_LOADMODEL)
					strcpy (path, "[m] ");
				else
					strcpy (path, "[o] ");

				strcat (path, ptr);

				for (i = 0; i < 4; i++)
				{
					if (!mx_strcasecmp (recentFiles[i], path))
						break;
				}

				if (i < 4)
				{
					char tmp[256];
					strcpy (tmp, recentFiles[0]);
					strcpy (recentFiles[0], recentFiles[i]);
					strcpy (recentFiles[i], tmp);
				}
				else
				{
					for (i = 3; i > 0; i--)
						strcpy (recentFiles[i], recentFiles[i - 1]);

					strcpy (recentFiles[0], path);
				}
				saveRecentFiles ();
				initRecentFiles ();
			}
		}
		break;
		case IDC_MODEL_UNLOADMODEL:
			setModelInfo (0, 0);
			glw->loadModel (0, 0);
			glw->loadTexture (0, TEXTURE_MODEL);
			cAnim->removeAll ();
			glw->setFrameInfo (0, 0);
			glw->redraw ();
			break;
		case IDC_MODEL_UNLOADOVERLAY:
			setModelInfo (0, 1);
			glw->loadModel (0, 1);
			glw->loadTexture (0, TEXTURE_OVERLAY);
			glw->redraw ();
			break;
		case IDC_MODEL_RECENTMODELS1:
		case IDC_MODEL_RECENTMODELS2:
		case IDC_MODEL_RECENTMODELS3:
		case IDC_MODEL_RECENTMODELS4:
		{
			int i = event->action - IDC_MODEL_RECENTMODELS1;
			bool isModel = recentFiles[i][1] == 'm';
			char *ptr = &recentFiles[i][4];

			if (!loadModel (ptr, isModel ? 0:1))
			{
				char str[256];

				sprintf (str, "Error reading model: %s", ptr);
				mxMessageBox (this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
				break;
			}

			char tmp[256];			
			strcpy (tmp, recentFiles[0]);
			strcpy (recentFiles[0], recentFiles[i]);
			strcpy (recentFiles[i], tmp);

			initRecentFiles ();
		}
		break;
		case IDC_MODEL_EXIT:
			mx::setIdleWindow (0);
			mx::quit ();
			break;
		case IDC_SKIN_MODELSKIN:
		case IDC_SKIN_OVERLAYSKIN:
		{
			const char *ptr = mxGetOpenFileName(this,"./","*.png;*.tga");
			if(ptr)
			{
				glw->loadTexture(ptr,event->action == IDC_SKIN_MODELSKIN ? TEXTURE_MODEL:TEXTURE_OVERLAY);
				setRenderMode(4);
				glw->redraw();
			}
		}
		break;
		case IDC_SKIN_BACKGROUND:
		case IDC_SKIN_WATER:
		{
			const char *ptr = mxGetOpenFileName (this, 0, "*.*");
			if (!ptr)
				break;

			if (glw->loadTexture (ptr, event->action == IDC_SKIN_BACKGROUND ? TEXTURE_BACKGROUND:TEXTURE_WATER))
			{
				if (event->action == IDC_SKIN_BACKGROUND)
				{
					cbBackground->setChecked (true);
					glw->setFlag (F_BACKGROUND, true);
				}
				else
				{
					cbWater->setChecked (true);
					glw->setFlag (F_WATER, true);
				}

				setRenderMode (4);
				glw->redraw ();
			}
			else
				mxMessageBox (this, "Error loading texture.", EDITOR_WINDOW_TITLE, MX_MB_OK | MX_MB_ERROR);
		}
		break;
		case IDC_SKIN_SCREENSHOT:
		{
			char *ptr = (char *) mxGetSaveFileName (this, "", "*.tga");
			if (ptr)
			{
				if (!strstr(ptr, ".tga"))
					strcat(ptr, ".tga");
				makeScreenShot(ptr);
			}
		}
		break;
		case IDC_OPTIONS_BGCOLOR:
		case IDC_OPTIONS_FGCOLOR:
		case IDC_OPTIONS_WFCOLOR:
		case IDC_OPTIONS_GRIDCOLOR:
		case IDC_OPTIONS_LIGHTCOLOR:
		{
			float r, g, b;
			int ir, ig, ib;

			if (event->action == IDC_OPTIONS_BGCOLOR)
				glw->getBGColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_FGCOLOR)
				glw->getFGColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_WFCOLOR)
				glw->getWFColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
				glw->getLightColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_GRIDCOLOR)
				glw->getGridColor (&r, &g, &b);

			ir = (int) (r * 255.0f);
			ig = (int) (g * 255.0f);
			ib = (int) (b * 255.0f);
			if (mxChooseColor (this, &ir, &ig, &ib))
			{
				if (event->action == IDC_OPTIONS_BGCOLOR)
					glw->setBGColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_FGCOLOR)
					glw->setFGColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_WFCOLOR)
					glw->setWFColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
					glw->setLightColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_GRIDCOLOR)
					glw->setGridColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);

				glw->redraw ();
			}
		}
		break;

		case IDC_OPTIONS_CENTERMODEL:
		{
			centerModel ();
			glw->redraw ();
		}
		break;
		case IDC_OPTIONS_FILTERTEXTURES:
			bFilterTextures = !mb->isChecked (IDC_OPTIONS_FILTERTEXTURES);
			mb->setChecked (IDC_OPTIONS_FILTERTEXTURES, bFilterTextures);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);
			break;
#ifdef WIN32
		case IDC_HELP_GOTOHOMEPAGE:
			ShellExecute (0, "open", "http://oldtimes-software.com/", 0, 0, SW_SHOW);
			break;
#endif
		case IDC_HELP_ABOUT:
			mxMessageBox (this,
				EDITOR_WINDOW_TITLE" 1.0\n\n"
				"Left-drag to rotate.\n"
				"Right-drag to zoom.\n"
				"Shift-left-drag to x-y-pan.\n\n"
				"Build:\t" __DATE__ " - "  __TIME__ ".\n"
				"Email:\teukos@oldtimes-software.com\n\thogsy@oldtimes-software.com\n"
				"Web:\thttp://oldtimes-software.com/", "About",
				MX_MB_OK | MX_MB_INFORMATION);
			break;

		case IDC_RENDERMODE:
			setRenderMode (cRenderMode->getSelectedIndex ());
			glw->redraw ();
			break;

		case IDC_WATER:
			glw->setFlag (F_WATER, cbWater->isChecked ());
			glw->redraw ();
			break;

		case IDC_LIGHT:
			glw->setFlag (F_LIGHT, cbLight->isChecked ());
			glw->redraw ();
			break;

		case IDC_BRIGHTNESS:
			glw->setBrightness (((mxSlider *) event->widget)->getValue ());
			break;
		case IDC_GRID:
			glw->setFlag (F_GRID, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;
		case IDC_BACKGROUND:
			glw->setFlag (F_BACKGROUND, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;
		case IDC_DRAWTEXTURE:
			glw->setFlag (F_DRAWTEXTURE, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;
		case IDC_ANIMATION:
		{
			int index = cAnim->getSelectedIndex ();
			if (index >= 0)
			{
				initAnimation (glw->getModel (0), index - 1);

				if (glw->getFlag (F_PAUSE))
				{
					char str[32];
					int frame = glw->getCurrFrame ();
					sprintf (str, "%d", frame);
					leFrame->setLabel (str);
					glw->setFrameInfo (frame, frame);
					glw->redraw ();
				}
			}
		}
		break;
		case IDC_INTERPOLATE:
			glw->setFlag (F_INTERPOLATE, ((mxCheckBox *) event->widget)->isChecked ());
			break;
		case IDC_GLCOMMANDS:
			glw->setFlag (F_GLCOMMANDS, ((mxCheckBox *) event->widget)->isChecked ());
			break;
		case IDC_PITCH:
			glw->setPitch ((float) ((mxSlider *) event->widget)->getValue ());
			break;
		case IDC_VIEWMODEL:
			glw->d_transX = glw->d_transY = glw->d_transZ= 0;
			glw->d_rotX = 0.0f;
			glw->d_rotY = 90.0f;
			glw->setFoV (75);
			leFoV->setLabel ("75");
			slfov->setValue (75);
			glw->redraw ();
			break;
		case IDC_RESETVIEW:
			glw->d_rotX = 0.0f;
			glw->d_rotY = -90.0f;
			glw->d_transZ = 50;
			glw->setPitch (50.0f);
			glw->setFoV (75);
			leFoV->setLabel ("75");
			slfov->setValue (75);
			glw->redraw ();
			break;
		case IDC_FOV:
			char str2[32];
			sprintf (str2, "%i", glw->d_fov);
			leFoV->setLabel (str2);
			glw->setFoV (((mxSlider *) event->widget)->getValue ());
			glw->redraw ();
			break;
		case IDC_PAUSE:
		{
			bool pause = !glw->getFlag (F_PAUSE);;
			static int startFrame = 0, endFrame = 0, currFrame = 0, currFrame2 = 0;
			static float pol = 0;
			static int index;

			glw->setFlag (F_PAUSE, pause);
			bDecFrame->setEnabled (pause);
			leFrame->setEnabled (pause);
			bIncFrame->setEnabled (pause);
			bSetFrame->setEnabled (pause);

			if (pause)
			{
				char str[32];
				startFrame = glw->getStartFrame ();
				endFrame = glw->getEndFrame ();
				currFrame = glw->getCurrFrame ();
				currFrame2 = glw->getCurrFrame2 ();
				pol = glw->d_pol;
				sprintf (str, "%d", glw->getCurrFrame ());
				leFrame->setLabel (str);
				bPause->setLabel ("Play");
				index = cAnim->getSelectedIndex ();
			}
			else
			{
				glw->d_startFrame = startFrame;
				glw->d_endFrame = endFrame;
				glw->d_currFrame = currFrame;
				glw->d_currFrame2 = currFrame2;
				glw->d_pol = pol;
				bPause->setLabel ("Pause");
				int index2 = cAnim->getSelectedIndex ();
				if (index != index2 && index2 >= 0)
					initAnimation (glw->getModel (0), index2 - 1);
			}
		}
		break;
		case IDC_DECFRAME:
		{
			int frame = glw->getCurrFrame () - 1;
			glw->setFrameInfo (frame, frame);
			char str[32];
			sprintf (str, "%d", glw->getCurrFrame ());
			leFrame->setLabel (str);
			glw->redraw ();
		}
		break;
		case IDC_FRAME:
		{
			const char *ptr = leFrame->getLabel ();
			if (ptr)
			{
				int frame = atoi (ptr);
				glw->setFrameInfo (frame, frame);
				char str[32];
				sprintf (str, "%d", glw->getCurrFrame ());
				leFrame->setLabel (str);
				glw->redraw ();
			}
		}
		break;
		case IDC_INCFRAME:
		{
			int frame = glw->getCurrFrame () + 1;
			glw->setFrameInfo (frame, frame);
			char str[32];
			sprintf (str, "%d", glw->getCurrFrame ());
			leFrame->setLabel (str);
			glw->redraw ();
		}
		break;
		case IDC_TEXTURESCALE:
		{
			glw->textureScale =  1.0f + (float) ((mxSlider *) event->widget)->getValue () * 4.0f / 100.0f;
			glw->textureScale = glw->textureScale / 6.0f;
			glw->redraw ();
		}
		break;
		case IDC_TEXTUREALPHA:
		{
			glw->textureAlpha =  (float) ((mxSlider *) event->widget)->getValue () / 100.0f;
			glw->redraw ();
		}
		break;
		case IDC_RELOADTEX:
			glw->loadTexture (modelTexFile, TEXTURE_MODEL);
			glw->loadTexture (overlayTexFile, TEXTURE_OVERLAY);
			glw->redraw ();
		break;
		}
	}
	break;

	case mxEvent::Size:
	{
		int w = event->width;
		int h = event->height;
		int y = mb->getHeight ();
#ifdef WIN32
#define HEIGHT 120
#else
#define HEIGHT 140
		h -= 40;
#endif
		glw->setBounds (0, y, w, h - HEIGHT);
		tab->setBounds (0, y + h - HEIGHT, w, HEIGHT);
	}
	break;
	}

	return 1;
}

void KatanaViewer::redraw ()
{
	mxEvent event;

	event.event		= mxEvent::Size;
	event.width		= w2();
	event.height	= h2();

	handleEvent (&event);
}

void
KatanaViewer::makeScreenShot (const char *filename)
{
#ifdef WIN32
	redraw ();
	int w = w2 ();
	int h = h2 ();

	mxImage *image = new mxImage ();
	if (image->create (w, h, 24))
	{
		glReadBuffer (GL_FRONT);
		glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, image->data);

		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.",EDITOR_WINDOW_TITLE, MX_MB_OK | MX_MB_ERROR);

		delete image;
	}
#endif	
}

void KatanaViewer::setRenderMode (int mode)
{
	if (mode >= 0)
	{
		cRenderMode->select (mode);
		glw->setRenderMode (mode);
		glw->setFlag (F_LIGHT, mode != 0);
		cbLight->setChecked (mode != 0);
	}
}

void KatanaViewer::centerModel ()
{
	if (glw->getModel (0))
	{
		float minmax[6];
		md2_getBoundingBox (glw->getModel (0), minmax);
		glw->d_transY = (minmax[3] + minmax[2]) / 2;

		float dx = minmax[1] - minmax[0];
		float dy = minmax[3] - minmax[2];
		float dz = minmax[5] - minmax[4];

		float d = dx;
		if (dy > d)
			d = dy;

		if (dz > d)
			d = dz;

		glw->d_transZ = d * 1.2f;
		glw->d_transX = 0;

		glw->d_rotX = 0.0f;
		glw->d_rotY = -90.0f;
	}
}

bool KatanaViewer::loadModel (const char *ptr, int pos)
{
	md2_model_t *model = glw->loadModel (ptr,  pos);
	if (!model)
		return false;

	char path[512];
	char texname[512];
#ifdef WIN32
	char title[512];

	sprintf(title,EDITOR_WINDOW_TITLE" - %s",ptr);

	SetWindowText(GetActiveWindow(), title);
#endif

	strcpy (path, mx_getpath (ptr));
	mx_setcwd (path);

	strncat(texname,ptr,strlen(ptr)-4);
	sprintf(texname,"%s.tga", texname);

	if (pos == 0)
	{	
		initAnimation (model, -1);
		setModelInfo (model, 0);
		glw->loadTexture (0, TEXTURE_MODEL);
		setRenderMode (0);
		centerModel ();					
	}
	else
	{
		setModelInfo (model, 1);
		glw->loadTexture (0, TEXTURE_OVERLAY);
	}

	if(glw->loadTexture(texname, pos == 0 ? TEXTURE_MODEL:TEXTURE_OVERLAY))
		setRenderMode (4);
	else
	{
		strcat (path, "skin.pcx");
		if (glw->loadTexture (path, pos == 0 ? TEXTURE_MODEL:TEXTURE_OVERLAY))
			setRenderMode (4);
	}

	md2_generateLightNormals (model);

	glw->redraw ();

	return true;
}

void KatanaViewer::setModelInfo (md2_model_t *model, int pos)
{
	static char str[1024];

	if (model)
	{
		sprintf (str,
			"Skins: %d\n"
			"Vertices: %d\n"
			"TexCoords: %d\n"
			"Triangles: %d\n"
			"GlCommands: %d\n"
			"Frames: %d\n",
			model->header.numSkins,
			model->header.numVertices,
			model->header.numTexCoords,
			model->header.numTriangles,
			model->header.numGlCommands,
			model->header.numFrames
			);
	}
	else
	{
		if (pos == 0)
			strcpy (str, "No Model.");
		else
			strcpy (str, "No Overlay.");
	}

	if (pos == 0)
		lModelInfo1->setLabel (str);
	else
		lModelInfo2->setLabel (str);
}

void
KatanaViewer::initAnimation (md2_model_t *model, int animation)
{
	cAnim->removeAll ();

	if (!model)
		return;

	int count = md2_getAnimationCount (model);

	cAnim->add ("<all animations>");

	for (int i = 0; i < count; i++)
		cAnim->add (md2_getAnimationName (model, i));

	int startFrame, endFrame;
	md2_getAnimationFrames (model, animation, &startFrame, &endFrame);
	glw->setFrameInfo (startFrame, endFrame);

	if (animation == -1)
		glw->setFrameInfo (0, model->header.numFrames - 1);

	cAnim->select (animation + 1);
}

int main(int argc,char *argv[])
{
	// Load up the engine module...
	Engine = (EngineExport_t*)pModule_LoadInterface(
		iEditorEngineInstance,
		"./"PATH_ENGINE"/"MODULE_ENGINE,
		"Engine_Main",
		&Editor);
	if(!Engine)
	{
		mxMessageBox(NULL,pError_Get(),EDITOR_WINDOW_TITLE,MX_MB_OK|MX_MB_ERROR);
		return -1;
	}
	else if(Engine->iVersion != ENGINE_INTERFACE)
	{
		mxMessageBox(NULL,"Editor is outdated, please rebuild!",EDITOR_WINDOW_TITLE,MX_MB_OK|MX_MB_ERROR);

		pModule_Unload(iEditorEngineInstance);

		return -1;
	}

	mx_setcwd (mx::getApplicationPath ());

	mx::init (argc, argv);

	g_md2Viewer = new KatanaViewer ();
	g_md2Viewer->setMenuBar (g_md2Viewer->getMenuBar ());

	char cmdline[1024] = "";
	if (argc > 1)
	{
		strcpy (cmdline, argv[1]);
		for (int i = 2; i < argc; i++)
		{
			strcat (cmdline, " ");
			strcat (cmdline, argv[i]);
		}
	}

	if (strstr (cmdline, "-dumpinfo"))
		g_md2Viewer->bDumpModelInfo = true;

	if (strstr (cmdline, ".md2"))
	{
		g_md2Viewer->loadModel (cmdline, 0);
		int i;
		char path[256];
		strcpy (path, "[m] ");
		strcat (path, cmdline);

		for (i = 0; i < 4; i++)
			if (!mx_strcasecmp (recentFiles[i], path))
				break;

		if (i < 4)
		{
			char tmp[256];
			strcpy (tmp, recentFiles[0]);
			strcpy (recentFiles[0], recentFiles[i]);
			strcpy (recentFiles[i], tmp);
		}
		else
		{
			for (i = 3; i > 0; i--)
				strcpy (recentFiles[i], recentFiles[i - 1]);

			strcpy (recentFiles[0], path);
		}
		g_md2Viewer->saveRecentFiles ();
		g_md2Viewer->initRecentFiles ();
	}

	return mx::run();
}