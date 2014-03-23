#ifndef INCLUDED_MD2VIEWER
#define INCLUDED_MD2VIEWER

#ifndef INCLUDED_MXWINDOW
#include "mxWindow.h"
#endif

#ifndef INCLUDED_MD2
#include "gl_model.h"
#endif

#define kAppTitle					"Katana Viewer"
#define IDC_MODEL_LOADMODEL			1001
#define IDC_MODEL_LOADOVERLAY		1002
#define IDC_MODEL_UNLOADMODEL		1003
#define IDC_MODEL_UNLOADOVERLAY		1004
#define IDC_MODEL_RECENTMODELS1		1008
#define IDC_MODEL_RECENTMODELS2		1009
#define IDC_MODEL_RECENTMODELS3		1010
#define IDC_MODEL_RECENTMODELS4		1011
#define IDC_MODEL_EXIT				1016

#define IDC_SKIN_MODELSKIN			1021
#define IDC_SKIN_OVERLAYSKIN		1022
#define IDC_SKIN_BACKGROUND			1023
#define IDC_SKIN_WATER				1024
#define IDC_SKIN_SCREENSHOT			1025

#define IDC_OPTIONS_BGCOLOR			1031
#define IDC_OPTIONS_WFCOLOR			1032
#define IDC_OPTIONS_FGCOLOR			1033
#define IDC_OPTIONS_LIGHTCOLOR		1034
#define IDC_OPTIONS_CENTERMODEL		1035
#define IDC_OPTIONS_FILTERTEXTURES	1037
#define IDC_OPTIONS_GRIDCOLOR		1038

#define IDC_HELP_GOTOHOMEPAGE		1051
#define IDC_HELP_ABOUT				1052

#define IDC_RENDERMODE				2001
#define IDC_WATER					2002
#define IDC_LIGHT					2003
#define IDC_BRIGHTNESS				2004
#define IDC_GRID				2005
#define IDC_BACKGROUND				2006
#define IDC_DRAWTEXTURE				2008
#define IDC_ANIMATION				3001
#define IDC_INTERPOLATE				3002
#define IDC_GLCOMMANDS				3003
#define IDC_PITCH					3004
#define IDC_PAUSE					3005
#define IDC_FRAME					3006
#define IDC_INCFRAME				3007
#define IDC_DECFRAME				3008
#define IDC_VIEWMODEL				3009
#define IDC_TEXTURESCALE			3010
#define IDC_TEXTUREALPHA			3011
#define IDC_FOV						3012
#define IDC_RESETVIEW				3013
#define IDC_RELOADTEX				3014

class mxTab;
class mxMenuBar;
class mxButton;
class mxLineEdit;
class mxLabel;
class mxChoice;
class mxCheckBox;
class mxSlider;
class GlWindow;

class KatanaViewer : public mxWindow
{
	mxMenuBar *mb;
	mxTab *tab;

	mxChoice *cRenderMode;
	mxCheckBox *cbWater, *cbLight, *cbBackground;

	mxChoice *cAnim;
	mxButton *bPause;
	mxButton *bViewModel;

	mxLineEdit *leFrame;
	mxLineEdit *leFoV;
	mxSlider *slfov;

	mxButton *bDecFrame, *bIncFrame, *bSetFrame;

	mxLabel *lModelInfo1, *lModelInfo2;

	md2_model_t *md2Model;
	md2_model_t *md2Weapon;

	void loadRecentFiles ();

public:

	KatanaViewer ();
	~KatanaViewer ();

	virtual int handleEvent (mxEvent *event);
	void redraw ();
	void makeScreenShot (const char *filename);
	void setRenderMode (int index);
	void centerModel ();

	GlWindow *glw;
	bool loadModel (const char *ptr, int pos);
	void setModelInfo (md2_model_t *model, int pos);
	void initAnimation (md2_model_t *model, int animation);
	mxMenuBar *getMenuBar () const { return mb; }
	bool bDumpModelInfo;
	void saveRecentFiles ();
	void initRecentFiles ();
};
extern KatanaViewer *g_md2Viewer;
#endif