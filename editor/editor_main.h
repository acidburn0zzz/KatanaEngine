/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __EDITORMAIN__
#define __EDITORMAIN__

// Platform Library
#include "platform.h"
#include "platform_window.h"
#include "platform_module.h"

// Shared Library
#include "shared_flags.h"
#include "shared_math.h"
#include "shared_module.h"
#include "shared_engine.h"

EngineExport_t  *Engine;

#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../common/cmdlib.H"

#include "parse.h"
#include "lbmlib.h"
#include "qedefs.h"

typedef struct
{
	vec3_t	normal;
	double	dist;
	int		type;
} plane_t;

#include "qfiles.h"
#include "textures.h"
#include "brush.h"
#include "entity.h"
#include "map.h"
#include "select.h"
#include "camera.h"
#include "xy.h"
#include "z.h"
#include "editor_console.h"

typedef struct
{
	int		p1, p2;
	face_t	*f1, *f2;
} pedge_t;

typedef struct
{
	int			iSize,
				iTexMenu;			// Nearest, linear, etc.
	float		fGamma;				// Gamma for textures.
	char		szProject[256];		// Last project loaded.
	vec3_t		colors[COLOR_LAST];

	bool		bShowNames,
				show_coordinates,
				bShowLightPreview,
				bFollowCamera;

	int			exclude;
} SavedInfo_t;

//
// system functions
//
void    Sys_UpdateStatusBar( void );
void    Sys_UpdateWindows (int bits);
void    Sys_Beep (void);
void    Sys_ClearPrintf (void);
void    Sys_Printf (char *text, ...);
double	Sys_DoubleTime (void);
void    Sys_GetCursorPos (int *x, int *y);
void    Sys_SetCursorPos (int x, int y);
void    Sys_SetTitle (char *text);
void    Sys_BeginWait (void);
void    Sys_EndWait (void);
void    Sys_Status(const char *psz, int part);

/*
** most of the QE globals are stored in this structure
*/
typedef struct
{
	bool		bShowGrid;
	int      d_gridsize;

	int      d_num_entities;

	entity_t    *d_project_entity;

	float       d_new_brush_bottom_z,
                d_new_brush_top_z;

	gINSTANCE   d_hInstance;

    GtkWidget   *gwMainWindow,
                *gwEditWindow,
                *gwEntityWindow,
                *gwTextureWindow,
                *gwConsoleWindow,
                *gwStatusBar,
                *gwToolBar;

	vec3_t    d_points[MAX_POINTS];
	int       d_numpoints;
	pedge_t   d_edges[MAX_EDGES];
	int       d_numedges;

	int       d_num_move_points;
	float    *d_move_points[1024];

	qtexture_t	*d_qtextures;

	texturewin_t d_texturewin;

	int	         d_pointfile_display_list;

	xy_t         d_xy;

	SavedInfo_t  d_savedinfo;

	int          d_workcount;

	// connect entities uses the last two brushes selected
	int			 d_select_count;
	brush_t		*d_select_order[2];
	vec3_t       d_select_translate;    // for dragging w/o making new display lists
	select_t     d_select_mode;

	int		     d_font_list;
	int          d_parsed_brushes;

	bool		show_blocks;

	char		*cTextureWindow;
} QEGlobals_t;

void *qmalloc (int size);
char *copystring (char *s);

void Pointfile_Delete (void);
void Pointfile_Check (void);
void Pointfile_Next (void);
void Pointfile_Prev (void);
void Pointfile_Clear (void);
void Pointfile_Draw( void );
void Pointfile_Load( void );

//
// drag.c
//
void Drag_Begin (int x, int y, int buttons,
		   vec3_t xaxis, vec3_t yaxis,
		   vec3_t origin, vec3_t dir);
void Drag_MouseMoved (int x, int y, int buttons);
void Drag_MouseUp (void);

//
// csg.c
//
void CSG_MakeHollow (void);
void CSG_Subtract (void);

//
// vertsel.c
//

void SetupVertexSelection (void);
void SelectEdgeByRay (vec3_t org, vec3_t dir);
void SelectVertexByRay (vec3_t org, vec3_t dir);

void ConnectEntities (void);

extern	int	update_bits;

extern	int	screen_width;
extern	int	screen_height;

char	*TranslateString (char *buf);

void ProjectDialog (void);

void FillBSPMenu (void);

//
// win_main.c
//
void Main_Create (gINSTANCE hInstance);

extern BOOL SaveRegistryInfo(const char *pszName, void *pvBuf, long lSize);
extern BOOL loadRegistryInfo(const char *pszName, void *pvBuf, long *plSize);

//
// entityw.c
//
BOOL CreateEntityWindow(gINSTANCE hInstance);
void FillClassList (void);
BOOL UpdateEntitySel(eclass_t *pec);
void SetInspectorMode(int iType);
void SetSpawnFlags(void);
void GetSpawnFlags(void);
void SetKeyValuePairs(void);
extern void BuildGammaTable(float g);


// win_dlg.c

void DoFind(void);
void DoRotate(void);
void DoSides(void);
void DoAbout(void);
void DoSurface(void);

/*
** QE function declarations
*/
void		QE_CheckAutoSave( void );
void		QE_ConvertDOSToUnixName( char *dst, const char *src );
void		QE_CountBrushesAndUpdateStatusBar( void );
void		QE_ExpandBspString (char *bspaction, char *out, char *mapname);
void		QE_Init (void);

bool	QE_KeyDown(int key);
bool	QE_LoadProject(char *projectfile);
bool	QE_SingleBrush(void);

/*
** extern declarations
*/
extern QEGlobals_t   g_qeglobals;

/*
	Main Editor Functions
*/

void Editor_Shutdown(void);

#endif
