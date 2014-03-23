/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "editor_main.h"

/*
	Drag either multiple brushes, or select plane points from
	a single brush.
*/

bool	drag_ok;
vec3_t	drag_xvec;
vec3_t	drag_yvec;

static	int	buttonstate;
static	int	pressx, pressy;
static	vec3_t	pressdelta;
static	int	buttonx, buttony;

int		lastx, lasty;

bool	drag_first;

void AxializeVector (vec3_t v)
{
	vec3_t	a;
	float	o;
	int		i;

	if (!v[0] && !v[1])
		return;
	if (!v[1] && !v[2])
		return;
	if (!v[0] && !v[2])
		return;

	for (i=0 ; i<3 ; i++)
		a[i] = fabs((float)v[i]);
	if (a[0] > a[1] && a[0] > a[2])
		i = 0;
	else if (a[1] > a[0] && a[1] > a[2])
		i = 1;
	else
		i = 2;

	o = v[i];
	Math_VectorCopy(vec3_origin,v);
	if (o<0)
		v[i] = -1;
	else
		v[i] = 1;

}

void Drag_Setup(int x,int y,int buttons,vec3_t xaxis,vec3_t yaxis,vec3_t origin,vec3_t dir)
{
	trace_t	t;
	face_t	*f;

	// [12/8/2012] Added back in ~hogsy
	if(selected_brushes.next == &selected_brushes)
		return;

	drag_first = TRUE;
	g_qeglobals.d_num_move_points = 0;
	Math_VectorCopy(vec3_origin,pressdelta);
	pressx = x;
	pressy = y;

	Math_VectorCopy(xaxis,drag_xvec);
	AxializeVector(drag_xvec);
	Math_VectorCopy(yaxis,drag_yvec);
	AxializeVector(drag_yvec);

	switch(g_qeglobals.d_select_mode)
	{
	case sel_vertex:
		SelectVertexByRay (origin, dir);
		if (g_qeglobals.d_num_move_points)
		{
			drag_ok = TRUE;
			return;
		}
		break;
	case sel_edge:
		SelectEdgeByRay (origin, dir);
		if (g_qeglobals.d_num_move_points)
		{
			drag_ok = TRUE;
			return;
		}
	}

	// Check for direct hit first
	t = Test_Ray (origin, dir, TRUE);
	if (t.selected)
	{
		drag_ok = TRUE;

		if (buttons == (MK_LBUTTON|MK_CONTROL) )
		{
			Sys_Printf ("Shear dragging face\n");
			Brush_SelectFaceForDragging(t.brush, t.face, TRUE);
		}
		else if (buttons == (MK_LBUTTON|MK_CONTROL|MK_SHIFT) )
		{
			Sys_Printf ("Sticky dragging brush\n");
			for (f=t.brush->brush_faces ; f ; f=f->next)
				Brush_SelectFaceForDragging (t.brush, f, FALSE);
		}
		else
			Sys_Printf ("Dragging entire selection\n");

		return;
	}

	if (g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge)
		return;

	//
	// check for side hit
	//
//	if (selected_brushes.next->next != &selected_brushes)
//	{
//		Sys_Printf ("Click isn't inside multiple selection\n");
//		return;
//	}

	if (selected_brushes.next->owner->eclass->fixedsize)
	{
		Sys_Printf ("Can't stretch fixed size entities\n");
		return;
	}

	if (buttons & MK_CONTROL)
		Brush_SideSelect (selected_brushes.next, origin, dir, TRUE);
	else
		Brush_SideSelect (selected_brushes.next, origin, dir, FALSE);

	Sys_Printf ("Side stretch\n");
	drag_ok = TRUE;
}

entity_t *peLink;

void UpdateTarget(vec3_t origin, vec3_t dir)
{
	trace_t	t;
	entity_t *pe;
	int i;
	char sz[128];

	t = Test_Ray (origin, dir, 0);

	if (!t.brush)
		return;

	pe = t.brush->owner;

	if (pe == NULL)
		return;

	// is this the first?
	if (peLink != NULL)
	{

		// Get the target id from out current target
		// if there is no id, make one

		i = IntForKey(pe, "target");
		if (i <= 0)
		{
			i = GetUniqueTargetId(1);
			sprintf(sz, "%d", i);

			SetKeyValue(pe, "target", sz);
		}

		// set the target # into our src

		sprintf(sz, "%d", i);
		SetKeyValue(peLink, "targetname", sz);

		Sys_UpdateWindows(W_ENTITY);

	}

	// promote the target to the src
	peLink = pe;
}

void Drag_Begin(int x,int y,int buttons,vec3_t xaxis,vec3_t yaxis,vec3_t origin,vec3_t dir)
{
	trace_t	t;

	drag_ok = FALSE;

	Math_VectorCopy(vec3_origin,pressdelta);

	drag_first = TRUE;
	peLink = NULL;

	// [11/8/2012] Select multiple brushes ~hogsy
	if(buttons == (MK_LBUTTON | MK_CONTROL))
	{
		if(!dir[0] && !dir[1])
			Select_Ray(origin,dir,SF_ENTITIES_FIRST);	// hack for XY
		else
			Select_Ray(origin,dir,0);
		return;
	}
	// ctrl-shift LBUTTON = select single face
	else if(buttons == (MK_LBUTTON | MK_SHIFT))
	{
		Select_Deselect ();
		Select_Ray (origin, dir, SF_SINGLEFACE);
		return;
	}
	// LBUTTON + all other modifiers = manipulate selection
	else if (buttons & MK_LBUTTON)
	{
		if(selected_brushes.next == &selected_brushes)
		{
			if(!dir[0] && !dir[1])
				Select_Ray(origin,dir,SF_ENTITIES_FIRST);	// hack for XY
			else
				Select_Ray(origin,dir,0);
		}

		Drag_Setup (x, y, buttons, xaxis, yaxis, origin, dir);
		return;
	}
	// middle button = grab texture
	else if(buttons == MK_MBUTTON)
	{
		t = Test_Ray (origin, dir, FALSE);
		if (t.face)
		{
			g_qeglobals.d_new_brush_bottom_z = t.brush->mins[2];
			g_qeglobals.d_new_brush_top_z = t.brush->maxs[2];
			Texture_SetTexture (&t.face->texdef);
		}
		else
			Sys_Printf ("Did not select a texture\n");
		return;
	}
	// ctrl-middle button = set entire brush to texture
	else if(buttons == (MK_MBUTTON|MK_CONTROL) )
	{
		t = Test_Ray (origin, dir, FALSE);
		if (t.brush)
		{
			if (t.brush->brush_faces->texdef.name[0] == '(')
				Sys_Printf ("Can't change an entity texture\n");
			else
			{
				Brush_SetTexture (t.brush, &g_qeglobals.d_texturewin.texdef);
				Sys_UpdateWindows (W_ALL);
			}
		}
		else
			Console_Print(FALSE,"Didn't hit a brush.\n");
		return;
	}

	// ctrl-shift-middle button = set single face to texture
	if (buttons == (MK_MBUTTON|MK_SHIFT|MK_CONTROL) )
	{
		t = Test_Ray (origin, dir, FALSE);
		if (t.brush)
		{
			if (t.brush->brush_faces->texdef.name[0] == '(')
				Console_Print(FALSE,"Can't change an entity texture\n");
			else
			{
				t.face->texdef = g_qeglobals.d_texturewin.texdef;
				Brush_Build( t.brush );
				Sys_UpdateWindows (W_ALL);
			}
		}
		else
			Sys_Printf ("Didn't hit a btrush\n");
		return;
	}
}

void MoveSelection (vec3_t move)
{
	int		i;
	brush_t	*b;

	if (!move[0] && !move[1] && !move[2])
		return;

	Sys_UpdateWindows (W_XY|W_CAMERA);

	//
	// dragging only a part of the selection
	//
	if (g_qeglobals.d_num_move_points)
	{
		for (i=0 ; i<g_qeglobals.d_num_move_points ; i++)
			Math_VectorAdd(g_qeglobals.d_move_points[i],move,g_qeglobals.d_move_points[i]);

		for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
		{
			Brush_Build( b );
			for (i=0 ; i<3 ; i++)
				if (b->mins[i] > b->maxs[i]
				|| b->maxs[i] - b->mins[i] > 4096)
					break;	// dragged backwards or fucked up
			if (i != 3)
				break;
		}

		// if any of the brushes were crushed out of existance
		// calcel the entire move
		if (b != &selected_brushes)
		{
			Sys_Printf ("Brush dragged backwards, move canceled\n");
			for (i=0 ; i<g_qeglobals.d_num_move_points ; i++)
				Math_VectorSubtract(g_qeglobals.d_move_points[i],move,g_qeglobals.d_move_points[i]);

			for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
				Brush_Build( b );
		}

	}
	else
	{
		// if there are lots of brushes selected, just translate instead
		// of rebuilding the brushes
		if (drag_yvec[2] == 0 && selected_brushes.next->next != &selected_brushes)
		{
			Math_VectorAdd (g_qeglobals.d_select_translate, move, g_qeglobals.d_select_translate);
		}
		else
			Select_Move (move);
	}
}

void Drag_MouseMoved(int x,int y,int buttons)
{
	vec3_t	move, delta;
	int		i;
	char	movestring[128];

	if(!buttons)
	{
		drag_ok = FALSE;
		return;
	}
	else if(!drag_ok)
		return;

	// clear along one axis
	if (buttons & MK_SHIFT)
	{
		drag_first = FALSE;
		if (abs(x-pressx) > abs(y-pressy))
			y = pressy;
		else
			x = pressx;
	}


	for (i=0 ; i<3 ; i++)
	{
		move[i] = drag_xvec[i]*(x - pressx)
				+ drag_yvec[i]*(y - pressy);
		move[i] = floor(move[i]/g_qeglobals.d_gridsize+0.5)*g_qeglobals.d_gridsize;
	}

	sprintf (movestring, "Drag (%i %i %i)", (int)move[0], (int)move[1], (int)move[2]);
	Sys_Status (movestring, 0);

	Math_VectorSubtract(move,pressdelta,delta);
	MoveSelection(delta);
	Math_VectorCopy(move,pressdelta);
}

void Drag_MouseUp(void)
{
	Sys_Status("Drag completed.", 0);
	if (g_qeglobals.d_select_translate[0] || g_qeglobals.d_select_translate[1] || g_qeglobals.d_select_translate[2])
	{
		Select_Move (g_qeglobals.d_select_translate);

		Math_VectorCopy(vec3_origin,g_qeglobals.d_select_translate);

		Sys_UpdateWindows(W_CAMERA);
	}
}
