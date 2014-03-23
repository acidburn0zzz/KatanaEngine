#include "editor_camera.h"

#include "shared_editor.h"

#define	PAGEFLIPS	2

void DrawPathLines (void);

camera_t	camera;

void Camera_Initialize(void)
{
	// [26/1/2013] Camera defaults to center ~hogsy
	Math_VectorClear(camera.origin);

	camera.timing		= false;

	camera.color[RED]	= 0.3f;
	camera.color[GREEN]	= 0.3f;
	camera.color[BLUE]	= 0.3f;
}

void Cam_ChangeFloor(bool up)
{
	brush_t	*b;
	float	d, bestd, current;
	vec3_t	start, dir;

	start[0] = camera.origin[0];
	start[1] = camera.origin[1];
	start[2] = 8192;

	dir[0] = dir[1] = 0;
	dir[2] = -1;

	current = 8192.0f-(camera.origin[2]-48.0f);
	if(up)
		bestd = 0;
	else
		bestd = 16384;

	for (b=active_brushes.next ; b != &active_brushes ; b=b->next)
	{
		if(!Brush_Ray(start,dir,b,&d))
			continue;
		if(up && d < current && d > bestd)
			bestd = d;
		if(!up && d > current && d < bestd)
			bestd = d;
	}

	if(bestd == 0 || bestd == 16384)
		return;

	camera.origin[2] += current-bestd;
	Sys_UpdateWindows(W_CAMERA|W_Z_OVERLAY);
}

int	iCameraButtonState;
static	int	buttonx, buttony;
static	int	cursorx, cursory;

face_t	*side_select;

#define	ANGLE_SPEED	300
#define	MOVE_SPEED	400

void Cam_PositionDrag(void)
{
	int	x, y;

	Sys_GetCursorPos(&x,&y);
	if(x != cursorx || y != cursory)
	{
		x -= cursorx;
		Math_VectorMA(camera.origin,x,camera.vright,camera.origin);
		y -= cursory;
		camera.origin[2] -= y;

		// [31/1/2013] Update XY position ~hogsy
		if(g_qeglobals.d_savedinfo.bFollowCamera)
		{
			g_qeglobals.d_xy.origin[X] = camera.origin[X];
			g_qeglobals.d_xy.origin[Y] = camera.origin[Y];
		}

		Sys_SetCursorPos(cursorx,cursory);
		//Sys_UpdateWindows(W_CAMERA | W_XY_OVERLAY);
	}
}

void Cam_MouseDown (int x, int y, int buttons)
{
	vec3_t		dir;
	float		f, r, u;
	int			i;

	//
	// calc ray direction
	//
	u = (float)(y - camera.height/2) / (camera.width/2);
	r = (float)(x - camera.width/2) / (camera.width/2);
	f = 1;

	for (i=0 ; i<3 ; i++)
		dir[i] = camera.vpn[i] * f + camera.vright[i] * r + camera.vup[i] * u;
	Math_VectorNormalize(dir);

	Sys_GetCursorPos (&cursorx, &cursory);

	iCameraButtonState = buttons;
	buttonx = x;
	buttony = y;

	// LBUTTON = manipulate selection
	// shift-LBUTTON = select
	// middle button = grab texture
	// ctrl-middle button = set entire brush to texture
	// ctrl-shift-middle button = set single face to texture
	if(		(buttons == MK_LBUTTON)
		||	(buttons == (MK_LBUTTON | MK_SHIFT))
		||	(buttons == (MK_LBUTTON | MK_CONTROL))
		||	(buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT))
		||	(buttons == MK_MBUTTON)
		||	(buttons == (MK_MBUTTON|MK_CONTROL))
		||	(buttons == (MK_MBUTTON|MK_SHIFT|MK_CONTROL)) )
	{
		Drag_Begin(x,y,buttons,camera.vright,camera.vup,camera.origin,dir);
		return;
	}

	if(buttons & MK_RBUTTON)
	{
		Cam_MouseControl(0.1f);
		return;
	}
}

void Cam_MouseUp(void)
{
	iCameraButtonState = 0;

	Drag_MouseUp();
}

void Cam_MouseMoved(int x,int y,int buttons)
{
	iCameraButtonState = buttons;
	if (!buttons)
		return;
	buttonx = x;
	buttony = y;

	if(buttons == (MK_RBUTTON|MK_CONTROL))
	{
		Cam_PositionDrag ();
		Sys_UpdateWindows (W_XY|W_CAMERA|W_Z);
		return;
	}

	Sys_GetCursorPos (&cursorx, &cursory);

	if (buttons & (MK_LBUTTON | MK_MBUTTON) )
	{
		Drag_MouseMoved (x, y, buttons);
		Sys_UpdateWindows (W_XY|W_CAMERA|W_Z);
	}
}


vec3_t	cull1, cull2;
int		cullv1[3], cullv2[3];

void InitCull (void)
{
	int		i;

	Math_VectorSubtract(camera.vpn,camera.vright,cull1);
	Math_VectorAdd(camera.vpn,camera.vright,cull2);

	for(i = 0; i < 3; i++)
	{
		if (cull1[i] > 0)
			cullv1[i] = 3+i;
		else
			cullv1[i] = i;

		if (cull2[i] > 0)
			cullv2[i] = 3+i;
		else
			cullv2[i] = i;
	}
}

bool Camera_CullBrush(brush_t *b)
{
	int		i;
	vec3_t	point;
	float	d;

	for (i=0 ; i<3 ; i++)
		point[i] = b->mins[cullv1[i]] - camera.origin[i];

	d = Math_DotProduct(point,cull1);
	if (d < -1)
		return true;

	for (i=0 ; i<3 ; i++)
		point[i] = b->mins[cullv2[i]] - camera.origin[i];

	d = Math_DotProduct(point,cull2);
	if(d < -1)
		return true;

	return false;
}

void Camera_Draw(void)
{
	brush_t		*brush;
	entity_t	*ent;
	face_t		*face;
	char		name[32];
	float		screenaspect,yfov,*v1,*v2;
	double		start=0,end;
	int			i;

	if (!active_brushes.next)
		return;	// not valid yetw

	if (camera.timing)
		start = Sys_DoubleTime ();

	// Clear
	glViewport(0,0,camera.width,camera.height);
	glScissor(0,0,camera.width,camera.height);
	glClearColor(
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][0],
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][1],
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][2],
		0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();

	screenaspect = (float)camera.width/camera.height;
	yfov = 2*atanf((float)camera.height/camera.width)*180.0f/M_PI;
	gluPerspective(yfov,screenaspect,2.0,8192.0);

	glRotatef(-90.0f,1.0f,0,0);	    // put Z going up
	glRotatef(90.0f,0,0,1.0f);	    // put Z going up
	glRotatef(camera.angles[0],0,1.0f,0);
	glRotatef(-camera.angles[1],0,0,1.0f);
	glTranslatef(-camera.origin[0],-camera.origin[1],-camera.origin[2]);

	// [31/1/2013] TODO: Rewrite... UGH ~hogsy
	if(g_qeglobals.bShowGrid)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		if(g_qeglobals.d_gridsize > 0 && g_qeglobals.d_gridsize < 64)
		{
			glColor4f(
				g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][RED],
				g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][GREEN],
				g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][BLUE],
				0.5f);
			glLineWidth(1.0f);
			glBegin(GL_LINES);

			for(i = 0; i <= (4096/g_qeglobals.d_gridsize); i++)
			{
				glVertex2i(-4096,(i*g_qeglobals.d_gridsize)*-1);
				glVertex2i(4096,(i*g_qeglobals.d_gridsize)*-1);
				glVertex2i(-4096,i*g_qeglobals.d_gridsize);
				glVertex2i(4096,i*g_qeglobals.d_gridsize);

				glVertex2i((i*g_qeglobals.d_gridsize)*-1,-4096);
				glVertex2i((i*g_qeglobals.d_gridsize)*-1,4096);
				glVertex2i(i*g_qeglobals.d_gridsize,-4096);
				glVertex2i(i*g_qeglobals.d_gridsize,4096);
			}

			glEnd();
		}

		glColor4f(0,0,0.3f,1.0f);
		glLineWidth(2.0f);
		glBegin(GL_LINES);

		for(i = 0; i <= 64; i++)
		{
			glVertex2i(-4096,(i*64)*-1);
			glVertex2i(4096,(i*64)*-1);
			glVertex2i(-4096,i*64);
			glVertex2i(4096,i*64);

			glVertex2i((i*64)*-1,-4096);
			glVertex2i((i*64)*-1,4096);
			glVertex2i(i*64,-4096);
			glVertex2i(i*64,4096);
		}

		glEnd();

		glLineWidth(1.0f);
		glColor3f(0,0,0);
		glDisable(GL_BLEND);
	}

	InitCull();

	// Draw stuff
	switch (camera.draw_mode)
	{
	case cd_wire:
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glColor3f(1.0f,1.0f,1.0f);
		break;
	case cd_solid:
		glCullFace(GL_FRONT);
		glEnable(GL_CULL_FACE);
		glShadeModel(GL_FLAT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		break;
	case cd_texture:
		glCullFace(GL_FRONT);
		glEnable(GL_CULL_FACE);
		glShadeModel (GL_FLAT);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);
		break;
	case cd_blend:
		glCullFace(GL_FRONT);
		glEnable(GL_CULL_FACE);
		glShadeModel (GL_FLAT);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glDisable(GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}

	glMatrixMode(GL_TEXTURE);
	for (brush = active_brushes.next ; brush != &active_brushes ; brush=brush->next)
	{
		if(Camera_CullBrush(brush))
			continue;
		if(FilterBrush(brush))
			continue;

		Brush_Draw(brush);
	}
	glMatrixMode(GL_PROJECTION);

	// Now draw selected brushes
	glTranslatef (g_qeglobals.d_select_translate[0], g_qeglobals.d_select_translate[1], g_qeglobals.d_select_translate[2]);
	glMatrixMode(GL_TEXTURE);

	// draw normally
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
		Brush_Draw( brush );

	// blend on top
	glMatrixMode(GL_PROJECTION);

	glColor4f(1.0f,0.0f,0.0f,0.3f);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
		for (face=brush->brush_faces ; face ; face=face->next)
			Face_Draw( face );
	if (selected_face)
		Face_Draw(selected_face);

	// non-zbuffered outline

	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	glColor3f (1, 1, 1);
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
		for (face=brush->brush_faces ; face ; face=face->next)
			Face_Draw( face );

#if 1
	// [13/8/2012] Let us toggle this ~hogsy
	for(ent = entities.next; ent != &entities; ent = ent->next)
	{
		// [15/4/2013] Lighting preview ~hogsy
		if(g_qeglobals.d_savedinfo.bShowLightPreview)
		{
			if(!strcmp(ValueForKey(ent,"classname"),ENTITY_LIGHT))
			{
				glBegin(GL_LINE_STRIP);
				glEnd();
			}
		}

		if(g_qeglobals.d_savedinfo.bShowNames)
		{
			sprintf(name,"%s",ValueForKey(ent,"classname"));

			// [8/8/2012] Draw entity names ~hogsy
			glColor3f(0,1.0f,0);
			glRasterPos3f(ent->origin[0],ent->origin[1],ent->origin[2]+(ent->eclass->maxs[2]+2.0f));
			glCallLists(strnlen(name,sizeof(name)),GL_UNSIGNED_BYTE,name);
		}
	}
#endif

	switch(g_qeglobals.d_select_mode)
	{
	case sel_vertex:
		glPointSize(4.0f);
		glColor3f(0,1.0f,0);
		glBegin (GL_POINTS);
		// [9/8/2012] TODO: Only draw for each non-selected vert point (numpoints and move_points not in same order) ~hogsy
		for(i=0 ; i < g_qeglobals.d_numpoints; i++)
			glVertex3fv (g_qeglobals.d_points[i]);
		glEnd ();

		glPointSize(6);
		glColor3f(1.0f,0,0);
		glBegin (GL_POINTS);
		for(i = 0; i < g_qeglobals.d_num_move_points; i++)
			glVertex3fv(g_qeglobals.d_move_points[i]);
		glEnd();
		glPointSize(1);
		break;
	case sel_edge:
		glPointSize(4.0f);
		glColor3f(0,0,1.0f);
		glBegin (GL_POINTS);
		for (i=0 ; i<g_qeglobals.d_numedges ; i++)
		{
			v1 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p1];
			v2 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p2];

			glVertex3f((v1[0]+v2[0])*0.5f,(v1[1]+v2[1])*0.5f,(v1[2]+v2[2])*0.5f);
		}
		glEnd ();

		// [9/8/2012] TODO: Show selected points ~hogsy
	}

	//
	// draw pointfile
	//
	glEnable(GL_DEPTH_TEST);

	DrawPathLines ();

	if (g_qeglobals.d_pointfile_display_list)
	{
		Pointfile_Draw();
//		glCallList (g_qeglobals.d_pointfile_display_list);
	}

	// bind back to the default texture so that we don't have problems
	// elsewhere using/modifying texture maps between contexts
	glBindTexture( GL_TEXTURE_2D, 0 );
	glFinish();

//	Sys_EndWait();

	if (camera.timing)
	{
		end = Sys_DoubleTime ();
		Console_Print(FALSE,"Camera: %i ms\n", (int)(1000*(end-start)));
	}
}

