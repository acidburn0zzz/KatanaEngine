/*	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2002-2009 John Fitzgibbons and others
	Copyright (C) 2011-2014 OldTimes Software

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "quakedef.h"

/*	Entities never clip against themselves, or their owner.
	Line of sight checks trace->crosscontent, but bullets don't.
*/

typedef struct
{
	vec3_t		boxmins, boxmaxs;// enclose the test object along entire move
	float		*mins, *maxs;	// size of the moving object
	vec3_t		mins2, maxs2;	// size when clipping against mosnters
	float		*start, *end;
	trace_t		trace;
	int			type;
	edict_t		*passedict;
} moveclip_t;


int SV_HullPointContents (hull_t *hull, int num, vec3_t p);

// ClearLink is used for new headnodes
void ClearLink(link_t *l)
{
	l->prev = l->next = l;
}

void RemoveLink(link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

void InsertLinkBefore(link_t *l,link_t *before)
{
	l->next			= before;
	l->prev			= before->prev;
	l->prev->next	= l;
	l->next->prev	= l;
}

void InsertLinkAfter(link_t *l,link_t *after)
{
	l->next			= after->next;
	l->prev			= after;
	l->prev->next	= l;
	l->next->prev	= l;
}

/*
===============================================================================

HULL BOXES

===============================================================================
*/


static	hull_t			box_hull;
static	BSPClipNode_t	box_clipnodes[6];
static	mplane_t		box_planes[6];

/*	Set up the planes and clipnodes so that the six floats of a bounding box
	can just be stored out and get a proper hull_t structure.
*/
void SV_InitBoxHull (void)
{
	int		i;
	int		side;

	box_hull.clipnodes = box_clipnodes;
	box_hull.planes = box_planes;
	box_hull.firstclipnode = 0;
	box_hull.lastclipnode = 5;

	for (i=0 ; i<6 ; i++)
	{
		box_clipnodes[i].iPlaneNum = i;

		side = i&1;

		box_clipnodes[i].iChildren[side] = BSP_CONTENTS_EMPTY;
		if (i != 5)
			box_clipnodes[i].iChildren[side^1] = i + 1;
		else
			box_clipnodes[i].iChildren[side^1] = BSP_CONTENTS_SOLID;

		box_planes[i].type = i>>1;
		box_planes[i].normal[i>>1] = 1;
	}

}


/*	To keep everything totally uniform, bounding boxes are turned into small
	BSP trees instead of being compared directly.
*/
hull_t	*SV_HullForBox (vec3_t mins, vec3_t maxs)
{
	box_planes[0].dist = maxs[0];
	box_planes[1].dist = mins[0];
	box_planes[2].dist = maxs[1];
	box_planes[3].dist = mins[1];
	box_planes[4].dist = maxs[2];
	box_planes[5].dist = mins[2];

	return &box_hull;
}

/*	Returns a hull that can be used for testing or clipping an object of mins/maxs
	size.
	Offset is filled in to contain the adjustment that must be added to the
	testing object's origin to get a point to use with the returned hull.
*/
hull_t *SV_HullForEntity (edict_t *ent, vec3_t mins, vec3_t maxs, vec3_t offset)
{
	model_t		*model;
	vec3_t		size,hullmins,hullmaxs;
	hull_t		*hull;

	// Decide which clipping hull to use, based on the size
	if(ent->Physics.iSolid == SOLID_BSP)
	{
		// Explicit hulls in the BSP model
		if(ent->v.movetype != MOVETYPE_PUSH)
		{
			Con_Warning("SOLID_BSP without MOVETYPE_PUSH!\n");

			return NULL;
		}

		model = sv.models[(int)ent->v.modelindex];
		if(!model || model->mType != MODEL_BRUSH)
		{
			Con_Warning("MOVETYPE_PUSH with a non bsp model!\n");

			return NULL;
		}

		Math_VectorSubtract (maxs, mins, size);

		if (size[0] < 3)
			hull = &model->hulls[0];
		else if (size[0] <= 32)
			hull = &model->hulls[1];
		else
			hull = &model->hulls[2];

		// Calculate an offset value to center the origin
		Math_VectorSubtract (hull->clip_mins, mins, offset);
		Math_VectorAdd (offset, ent->v.origin, offset);
	}
	else
	{
		// Create a temp hull from bounding box sizes
		Math_VectorSubtract (ent->v.mins, maxs, hullmins);
		Math_VectorSubtract (ent->v.maxs, mins, hullmaxs);
		hull = SV_HullForBox (hullmins, hullmaxs);

		Math_VectorCopy (ent->v.origin, offset);
	}

	return hull;
}

/*
===============================================================================

ENTITY AREA CHECKING

===============================================================================
*/

typedef struct areanode_s
{
	int		axis;		// -1 = leaf node
	float	dist;
	struct areanode_s	*children[2];
	link_t	trigger_edicts;
	link_t	solid_edicts;
} areanode_t;

#define	AREA_DEPTH	4
#define	AREA_NODES	32

static	areanode_t	sv_areanodes[AREA_NODES];
static	int			sv_numareanodes;

areanode_t *SV_CreateAreaNode (int depth, vec3_t mins, vec3_t maxs)
{
	areanode_t	*anode;
	vec3_t		size,
				mins1,maxs1,mins2,maxs2;

	anode = &sv_areanodes[sv_numareanodes];
	sv_numareanodes++;

	ClearLink (&anode->trigger_edicts);
	ClearLink (&anode->solid_edicts);

	if (depth == AREA_DEPTH)
	{
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}

	Math_VectorSubtract (maxs, mins, size);
	if (size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = 1;

	anode->dist = 0.5*(maxs[anode->axis]+mins[anode->axis]);
	Math_VectorCopy(mins,mins1);
	Math_VectorCopy(mins,mins2);
	Math_VectorCopy(maxs,maxs1);
	Math_VectorCopy(maxs,maxs2);

	maxs1[anode->axis] = mins2[anode->axis] = anode->dist;

	anode->children[0] = SV_CreateAreaNode (depth+1, mins2, maxs2);
	anode->children[1] = SV_CreateAreaNode (depth+1, mins1, maxs1);

	return anode;
}

void SV_ClearWorld (void)
{
	SV_InitBoxHull ();

	memset (sv_areanodes, 0, sizeof(sv_areanodes));
	sv_numareanodes = 0;
	SV_CreateAreaNode (0, sv.worldmodel->mins, sv.worldmodel->maxs);
}

void SV_UnlinkEdict(edict_t *ent)
{
	if(!ent->area.prev)
		return;		// not linked in anywhere

	RemoveLink(&ent->area);

	ent->area.prev = ent->area.next = NULL;
}

void SV_TouchLinks ( edict_t *ent, areanode_t *node )
{
	link_t		*l, *next;
	edict_t		*touch,*eOldOther;
	int			old_self;

// touch linked edicts
	for(l = node->trigger_edicts.next; l != &node->trigger_edicts; l = next)
	{
		//johnfitz -- fixes a crash when a touch function deletes an entity which comes later in the list
		if(!l)
		{
			Con_Warning("Recieved a null link!\n");
			break;
		}
		//johnfitz

		next = l->next;
		touch = EDICT_FROM_AREA(l);
		if (touch == ent)
			continue;
		if (!touch->v.TouchFunction || touch->Physics.iSolid != SOLID_TRIGGER)
			continue;
		if(ent->v.absmin[0] > touch->v.absmax[0]
		|| ent->v.absmin[1] > touch->v.absmax[1]
		|| ent->v.absmin[2] > touch->v.absmax[2]
		|| ent->v.absmax[0] < touch->v.absmin[0]
		|| ent->v.absmax[1] < touch->v.absmin[1]
		|| ent->v.absmax[2] < touch->v.absmin[2])
			continue;

		old_self	= pr_global_struct.self;
		eOldOther	= pr_global_struct.eOther;

		pr_global_struct.self	= EDICT_TO_PROG(touch);
		pr_global_struct.eOther	= ent;

		if(touch->v.TouchFunction)
			touch->v.TouchFunction(touch,ent);

		//johnfitz -- the PR_ExecuteProgram above can alter the linked edicts -- fix from tyrquake
		if(next != l->next && l->next)
		{
			Con_Printf ("SV_TouchLinks: next != l->next\n");
			next = l->next;
		}
		//johnfitz

		pr_global_struct.self	= old_self;
		pr_global_struct.eOther = eOldOther;
	}

	// Recurse down both sides
	if(node->axis == -1)
		return;

	if ( ent->v.absmax[node->axis] > node->dist )
		SV_TouchLinks ( ent, node->children[0] );
	if ( ent->v.absmin[node->axis] < node->dist )
		SV_TouchLinks ( ent, node->children[1] );
}

void SV_FindTouchedLeafs (edict_t *ent, mnode_t *node)
{
	mplane_t	*splitplane;
	mleaf_t		*leaf;
	int			sides;
	int			leafnum;

	if (node->contents == BSP_CONTENTS_SOLID)
		return;

// add an efrag if the node is a leaf

	if ( node->contents < 0)
	{
		if (ent->num_leafs == MAX_ENT_LEAFS)
			return;

		leaf = (mleaf_t *)node;
		leafnum = leaf - sv.worldmodel->leafs - 1;

		ent->leafnums[ent->num_leafs] = leafnum;
		ent->num_leafs++;
		return;
	}

// NODE_MIXED

	splitplane = node->plane;
	sides = Math_BoxOnPlaneSide(ent->v.absmin, ent->v.absmax, splitplane);

// recurse down the contacted sides
	if (sides & 1)
		SV_FindTouchedLeafs (ent, node->children[0]);

	if (sides & 2)
		SV_FindTouchedLeafs (ent, node->children[1]);
}

void SV_LinkEdict(edict_t *ent,bool touch_triggers)
{
	areanode_t	*node;

	if(ent->area.prev)
		SV_UnlinkEdict(ent);	// unlink from old position

	if(ent == sv.edicts || ent->free)
		return;

	// Set the abs box
	Math_VectorAdd(ent->v.origin,ent->v.mins,ent->v.absmin);
	Math_VectorAdd(ent->v.origin,ent->v.maxs,ent->v.absmax);

	// to make items easier to pick up and allow them to be grabbed off
	// of shelves, the abs sizes are expanded
	if(ent->v.flags & FL_ITEM)
	{
		ent->v.absmin[0] -= 15.0f;
		ent->v.absmin[1] -= 15.0f;
		ent->v.absmax[0] += 15.0f;
		ent->v.absmax[1] += 15.0f;
	}
	else
	{
		// because movement is clipped an epsilon away from an actual edge,
		// we must fully check even when bounding boxes don't quite touch
		ent->v.absmin[0] -= 1.0f;
		ent->v.absmin[1] -= 1.0f;
		ent->v.absmin[2] -= 1.0f;
		ent->v.absmax[0] += 1.0f;
		ent->v.absmax[1] += 1.0f;
		ent->v.absmax[2] += 1.0f;
	}

// link to PVS leafs
	ent->num_leafs = 0;
	if (ent->v.modelindex)
		SV_FindTouchedLeafs (ent, sv.worldmodel->nodes);

	if (ent->Physics.iSolid == SOLID_NOT)
		return;
	else if((ent->Physics.iSolid == SOLID_BSP) && (ent->v.angles[0] || ent->v.angles[1] || ent->v.angles[2]))
	{
		float	fMaximum,fMinimum;
		int		i;

		// http://www.quake-1.com/docs/quakesrc.org/72.html
		fMaximum	= Math_DotProduct(ent->v.mins,ent->v.mins);
		fMinimum	= Math_DotProduct(ent->v.maxs,ent->v.maxs);
		if(fMaximum < fMinimum)
		{
			fMaximum = fMinimum;
			fMaximum = sqrt(fMaximum);
			for(i = 0; i < 3; i++)
			{
				ent->v.absmin[i] = ent->v.origin[i]-fMaximum;
				ent->v.absmax[i] = ent->v.origin[i]+fMaximum;
			}
		}
	}
	else
	{
		Math_VectorAdd(ent->v.origin,ent->v.mins,ent->v.absmin);
		Math_VectorAdd(ent->v.origin,ent->v.maxs,ent->v.absmax);
	}

// find the first node that the ent's box crosses
	node = sv_areanodes;
	for(;;)
	{
		if (node->axis == -1)
			break;
		if (ent->v.absmin[node->axis] > node->dist)
			node = node->children[0];
		else if (ent->v.absmax[node->axis] < node->dist)
			node = node->children[1];
		else
			break;		// crosses the node
	}

	// link it in
	if (ent->Physics.iSolid == SOLID_TRIGGER)
		InsertLinkBefore(&ent->area,&node->trigger_edicts);
	else
		InsertLinkBefore(&ent->area,&node->solid_edicts);

	// if touch_triggers, touch all entities at this node and decend for more
	if (touch_triggers)
		SV_TouchLinks ( ent, sv_areanodes );
}

int SV_HullPointContents (hull_t *hull, int num, vec3_t p)
{
	float			d;
	BSPClipNode_t	*node;
	mplane_t		*plane;

	while (num >= 0)
	{
		if (num < hull->firstclipnode || num > hull->lastclipnode)
			Sys_Error ("SV_HullPointContents: bad node number");

		node = hull->clipnodes + num;
		plane = hull->planes + node->iPlaneNum;

		if (plane->type < 3)
			d = p[plane->type] - plane->dist;
		else
			d = Math_DotProduct (plane->normal, p) - plane->dist;
		if (d < 0)
			num = node->iChildren[1];
		else
			num = node->iChildren[0];
	}

	return num;
}

int SV_PointContents (vec3_t p)
{
	int		cont;

	cont = SV_HullPointContents (&sv.worldmodel->hulls[0], 0, p);
	if (cont <= BSP_CONTENTS_0 && cont >= BSP_CONTENTS_DOWN)
		cont = BSP_CONTENTS_WATER;
	return cont;
}

int SV_TruePointContents (vec3_t p)
{
	return SV_HullPointContents (&sv.worldmodel->hulls[0], 0, p);
}

//===========================================================================

/*	This could be a lot more efficient...
*/
edict_t	*SV_TestEntityPosition (edict_t *ent)
{
	trace_t	trace;

	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, ent->v.origin, 0, ent);
	if (trace.bStartSolid)
		return sv.edicts;

	return NULL;
}

// 1/32 epsilon to keep floating point happy
#define	DIST_EPSILON	(0.03125)

bool SV_RecursiveHullCheck (hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace)
{
	float			t1,t2,
					frac,
					midf;
	int				i,side;
	BSPClipNode_t	*node;
	mplane_t		*plane;
	vec3_t			mid;

// check for empty
	if (num < 0)
	{
		if (num != BSP_CONTENTS_SOLID)
		{
			trace->bAllSolid = false;

			if (num == BSP_CONTENTS_EMPTY)
				trace->bOpen = true;
			else
				trace->bWater = true;
		}
		else
			trace->bStartSolid = true;

		return true;		// empty
	}

	if (num < hull->firstclipnode || num > hull->lastclipnode)
		Sys_Error ("SV_RecursiveHullCheck: bad node number");

	// Find the point distances
	node	= hull->clipnodes+num;
	plane	= hull->planes+node->iPlaneNum;

	if (plane->type < 3)
	{
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
	}
	else
	{
		t1 = Math_DotProduct(plane->normal,p1)-plane->dist;
		t2 = Math_DotProduct(plane->normal,p2)-plane->dist;
	}

#if 1
	if (t1 >= 0 && t2 >= 0)
		return SV_RecursiveHullCheck (hull, node->iChildren[0], p1f, p2f, p1, p2, trace);
	if (t1 < 0 && t2 < 0)
		return SV_RecursiveHullCheck (hull, node->iChildren[1], p1f, p2f, p1, p2, trace);
#else
	if ( (t1 >= DIST_EPSILON && t2 >= DIST_EPSILON) || (t2 > t1 && t1 >= 0) )
		return SV_RecursiveHullCheck (hull, node->children[0], p1f, p2f, p1, p2, trace);
	if ( (t1 <= -DIST_EPSILON && t2 <= -DIST_EPSILON) || (t2 < t1 && t1 <= 0) )
		return SV_RecursiveHullCheck (hull, node->children[1], p1f, p2f, p1, p2, trace);
#endif

// put the crosspoint DIST_EPSILON pixels on the near side
	if (t1 < 0)
		frac = (t1 + DIST_EPSILON)/(t1-t2);
	else
		frac = (t1 - DIST_EPSILON)/(t1-t2);

	if(frac < 0)
		frac = 0;
	else if(frac > 1.0f)
		frac = 1.0f;

	midf = p1f + (p2f - p1f)*frac;
	for (i=0 ; i<3 ; i++)
		mid[i] = p1[i] + frac*(p2[i] - p1[i]);

	side = (t1 < 0);

// move up to the node
	if(!SV_RecursiveHullCheck (hull, node->iChildren[side], p1f, midf, p1, mid, trace) )
		return false;

#ifdef PARANOID
	if (SV_HullPointContents (sv_hullmodel, mid, node->children[side])
	== BSP_CONTENTS_SOLID)
	{
		Con_Printf ("mid PointInHullSolid\n");
		return false;
	}
#endif

	if(SV_HullPointContents (hull, node->iChildren[side^1], mid) != BSP_CONTENTS_SOLID)
// go past the node
		return SV_RecursiveHullCheck (hull, node->iChildren[side^1], midf, p2f, mid, p2, trace);

	if(trace->bAllSolid)
		return false;		// never got out of the solid area

	// The other side of the node is solid, this is the impact point
	if(!side)
	{
		Math_VectorCopy (plane->normal, trace->plane.normal);
		trace->plane.dist = plane->dist;
	}
	else
	{
		Math_VectorSubtract (vec3_origin, plane->normal, trace->plane.normal);
		trace->plane.dist = -plane->dist;
	}

	while(SV_HullPointContents (hull, hull->firstclipnode, mid) == BSP_CONTENTS_SOLID)
	{
		// Shouldn't really happen, but does occasionally
		frac -= 0.1f;
		if (frac < 0)
		{
			trace->fraction = midf;

			Math_VectorCopy (mid, trace->endpos);

			Con_DPrintf ("backup past 0\n");

			return false;
		}
		midf = p1f + (p2f - p1f)*frac;
		for (i=0 ; i<3 ; i++)
			mid[i] = p1[i] + frac*(p2[i] - p1[i]);
	}

	trace->fraction = midf;
	Math_VectorCopy (mid, trace->endpos);

	return false;
}

/*	Handles selection or creation of a clipping hull, and offseting (and
	eventually rotation) of the end points
*/
trace_t SV_ClipMoveToEntity(edict_t *ent, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	trace_t		trace;
	vec3_t		offset,start_l,end_l,a,forward,right,up,temp;
	hull_t		*hull;

	// Fill in a default trace
	memset (&trace, 0, sizeof(trace_t));
	trace.fraction	= 1;
	trace.bAllSolid = true;
	Math_VectorCopy (end, trace.endpos);

	// Get the clipping hull
	hull = SV_HullForEntity(ent,mins,maxs,offset);

	Math_VectorSubtract (start, offset, start_l);
	Math_VectorSubtract (end, offset, end_l);

	// Rotate start and end into the models frame of reference
	if (ent->Physics.iSolid == SOLID_BSP &&
		(fabs(ent->v.angles[0]) > 1 || fabs(ent->v.angles[1]) > 1 || fabs(ent->v.angles[2]) > 1) )
	{
		vec3_t	forward,right,up,temp;

		Math_AngleVectors(ent->v.angles,forward,right,up);

		Math_VectorCopy (start_l, temp);
		start_l[0] = Math_DotProduct(temp,forward);
		start_l[1] = -Math_DotProduct(temp,right);
		start_l[2] = Math_DotProduct(temp,up);

		Math_VectorCopy (end_l, temp);
		end_l[0] = Math_DotProduct(temp,forward);
		end_l[1] = -Math_DotProduct(temp,right);
		end_l[2] = Math_DotProduct(temp,up);
	}

	// Trace a line through the apropriate clipping hull
	SV_RecursiveHullCheck (hull, hull->firstclipnode, 0, 1, start_l, end_l, &trace);

	// Rotate endpos back to world frame of reference
	if (ent->Physics.iSolid == SOLID_BSP &&
		(fabs(ent->v.angles[0]) > 1 || fabs(ent->v.angles[1]) > 1 || fabs(ent->v.angles[2]) > 1) )
	{
		if (trace.fraction != 1)
		{
			Math_VectorNegate(ent->v.angles,a);
			Math_AngleVectors(a,forward,right,up);

			Math_VectorCopy(trace.endpos,temp);
			trace.endpos[0] = Math_DotProduct(temp,forward);
			trace.endpos[1] = -Math_DotProduct(temp,right);
			trace.endpos[2] = Math_DotProduct(temp,up);

			Math_VectorCopy (trace.plane.normal, temp);
			trace.plane.normal[0] = Math_DotProduct(temp,forward);
			trace.plane.normal[1] = -Math_DotProduct(temp,right);
			trace.plane.normal[2] = Math_DotProduct(temp,up);
		}
	}

	// Fix trace up by the offset
	if (trace.fraction != 1)
		Math_VectorAdd(trace.endpos, offset, trace.endpos);

	// Did we clip the move?
	if(trace.fraction < 1 || trace.bStartSolid)
		trace.ent = ent;

	return trace;
}

/*	Mins and maxs enclose the entire area swept by the move
*/
void SV_ClipToLinks(areanode_t *node,moveclip_t *clip)
{
	link_t		*l,*next;
	edict_t		*eTouch;
	trace_t		trace;

	// Touch linked edicts
	for(l = node->solid_edicts.next; l != &node->solid_edicts; l = next)
	{
		next = l->next;

		eTouch = EDICT_FROM_AREA(l);
		if(eTouch->Physics.iSolid == SOLID_NOT)
			continue;
		else if(eTouch == clip->passedict)
			continue;
		else if(eTouch->Physics.iSolid == SOLID_TRIGGER)
			Sys_Error ("Trigger in clipping list");

		if(clip->type == MOVE_NOMONSTERS && eTouch->Physics.iSolid != SOLID_BSP)
			continue;

		if(clip->boxmins[0] > eTouch->v.absmax[0]
		|| clip->boxmins[1] > eTouch->v.absmax[1]
		|| clip->boxmins[2] > eTouch->v.absmax[2]
		|| clip->boxmaxs[0] < eTouch->v.absmin[0]
		|| clip->boxmaxs[1] < eTouch->v.absmin[1]
		|| clip->boxmaxs[2] < eTouch->v.absmin[2] )
			continue;

		if (clip->passedict && clip->passedict->v.size[0] && !eTouch->v.size[0])
			continue;	// points never interact

		// Might intersect, so do an exact clip
		if(clip->trace.bAllSolid)
			return;

		// [4/8/2012] Removed upon request ~hogsy
		// [12/4/2013] Reimplemented to use new eIgnore ~hogsy
		if(clip->passedict && eTouch->Physics.eIgnore)
		{
			if(eTouch->Physics.eIgnore == clip->passedict)
				continue;	// Don't clip against own missiles
			if(clip->passedict->Physics.eIgnore == eTouch)
				continue;	// Don't clip against owner
		}

		// [28/6/2013] Shouldn't really be refering to the monster struct in engine-side code, like we're doing here... ~hogsy
		if(eTouch->monster.iType > 2)
			trace = SV_ClipMoveToEntity(eTouch,clip->start,clip->mins2,clip->maxs2,clip->end);
		else
			trace = SV_ClipMoveToEntity(eTouch, clip->start, clip->mins, clip->maxs, clip->end);

		if(trace.bAllSolid || trace.bStartSolid || trace.fraction < clip->trace.fraction)
		{
			trace.ent = eTouch;
		 	if (clip->trace.bStartSolid)
			{
				clip->trace				= trace;
				clip->trace.bStartSolid	= true;
			}
			else
				clip->trace = trace;
		}
		else if (trace.bStartSolid)
			clip->trace.bStartSolid = true;
	}

	// Recurse down both sides
	if (node->axis == -1)
		return;

	if ( clip->boxmaxs[node->axis] > node->dist )
		SV_ClipToLinks ( node->children[0], clip );
	if ( clip->boxmins[node->axis] < node->dist )
		SV_ClipToLinks ( node->children[1], clip );
}

void SV_MoveBounds (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, vec3_t boxmins, vec3_t boxmaxs)
{
#if 0
	// debug to test against everything
	boxmins[0] = boxmins[1] = boxmins[2] = -9999;
	boxmaxs[0] = boxmaxs[1] = boxmaxs[2] = 9999;
#else
	int	i;

	for(i = 0; i < 3; i++)
	{
		if(end[i] > start[i])
		{
			boxmins[i] = start[i] + mins[i] - 1;
			boxmaxs[i] = end[i] + maxs[i] + 1;
		}
		else
		{
			boxmins[i] = end[i] + mins[i] - 1;
			boxmaxs[i] = start[i] + maxs[i] + 1;
		}
	}
#endif
}

trace_t SV_Move(vec3_t start,vec3_t mins,vec3_t maxs,vec3_t end,int type,edict_t *passedict)
{
	moveclip_t	clip;
	int			i;

	memset ( &clip, 0, sizeof ( moveclip_t ) );

	// clip to world
	clip.trace		= SV_ClipMoveToEntity ( sv.edicts, start, mins, maxs, end );
	clip.start		= start;
	clip.end		= end;
	clip.mins		= mins;
	clip.maxs		= maxs;
	clip.type		= type;
	clip.passedict	= passedict;

	if(type == MOVE_MISSILE)
	{
		for(i = 0; i < 3; i++)
		{
			clip.mins2[i] = -15.0f;
			clip.maxs2[i] = 15.0f;
		}
	}
	else
	{
		Math_VectorCopy (mins, clip.mins2);
		Math_VectorCopy (maxs, clip.maxs2);
	}

	// create the bounding box of the entire move
	SV_MoveBounds ( start, clip.mins2, clip.maxs2, end, clip.boxmins, clip.boxmaxs );

	// clip to entities
	SV_ClipToLinks ( sv_areanodes, &clip );

	return clip.trace;
}
