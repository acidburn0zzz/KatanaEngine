/*	Copyright (C) 2011-2013 OldTimes Software
*/
#include "quakedef.h"

/*	The view is allowed to move slightly from it's true position for bobbing,
	but if it exceeds 8 pixels linear distance (spherical, not box), the list of
	entities sent from the server may not include everything in the pvs, especially
	when crossing a water boundary.
*/

#include "engine_video.h"

#include "shared_math.h"

cvar_t	scr_ofsx			= { "scr_ofsx",			"0",		false	};
// [10/11/2013] For development purposes, default offset for y is -5 ~hogsy
cvar_t	scr_ofsy			= { "scr_ofsy",			"0",		false	};
cvar_t	scr_ofsz			= { "scr_ofsz",			"0",		false	};
cvar_t	cl_rollspeed		= { "cl_rollspeed",		"200"				};
cvar_t	cl_rollangle		= { "cl_rollangle",		"2.0"				};
cvar_t	cl_bob				= { "cl_bob",			"0.002",	false	};
cvar_t	cl_bobcycle			= { "cl_bobcycle",		"0.43",		false	};
cvar_t	cl_bobup			= { "cl_bobup",			"0.45",		false	};
cvar_t	cSideBobUp			= {	"view_sideup",		"0.5",		false	};
cvar_t	cSideBobCycle		= { "view_sidecycle",	"0.86",		false	};
cvar_t	cSideBob			= {	"view_sidebob",		"0.007",	false	};
cvar_t	cViewModelLag		= {	"view_modellag",	"1.0"				};
cvar_t	v_kicktime			= { "v_kicktime",		"0.5",		false	};
cvar_t	v_kickroll			= { "v_kickroll",		"0.6",		false	};
cvar_t	v_kickpitch			= { "v_kickpitch",		"0.6",		false	};
cvar_t	v_gunkick			= {	"v_gunkick",		"1"					}; //johnfitz
cvar_t	v_iyaw_cycle		= { "v_iyaw_cycle",		"2",		false	};
cvar_t	v_iroll_cycle		= { "v_iroll_cycle",	"0.5",		false	};
cvar_t	v_ipitch_cycle		= { "v_ipitch_cycle",	"1",		false	};
cvar_t	v_iyaw_level		= { "v_iyaw_level",		"0.3",		false	};
cvar_t	v_iroll_level		= { "v_iroll_level",	"0.1",		false	};
cvar_t	v_ipitch_level		= { "v_ipitch_level",	"0.3",		false	};
cvar_t	v_idlescale			= { "v_idlescale",		"0",		false	};
cvar_t	crosshair			= {	"crosshair",		"1",		true	};
cvar_t	gl_cshiftpercent	= {	"gl_cshiftpercent",	"100",		false	};

float	v_dmg_time, v_dmg_roll, v_dmg_pitch;

extern	int	in_forward, in_forward2, in_back;

vec3_t	v_punchangles[2]; //johnfitz -- copied from cl.punchangle.  0 is current, 1 is previous value. never the same unless map just loaded

vec3_t	forward, right, up;

/*	Used by view and sv_user
*/
float V_CalcRoll(vec3_t angles, vec3_t velocity)
{
	float	sign,side,value;

	Math_AngleVectors(angles, forward, right, up);
	side = Math_DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	value = cl_rollangle.value;

	if (side < cl_rollspeed.value)
		side = side * value / cl_rollspeed.value;
	else
		side = value;

	return side*sign;

}

//=============================================================================

cvar_t	v_centermove	= {"v_centermove",	"0.15", false	};
cvar_t	v_centerspeed	= {"v_centerspeed",	"500"			};

void V_StartPitchDrift(void)
{
	if(cl.laststop == cl.time)
		return;		// something else is keeping it from drifting

	if (cl.bNoDrift || !cl.pitchvel)
	{
		cl.pitchvel		= v_centerspeed.value;
		cl.bNoDrift		= false;
		cl.driftmove	= 0;
	}
}

void V_StopPitchDrift (void)
{
	cl.laststop = cl.time;
	cl.bNoDrift	= true;
	cl.pitchvel = 0;
}

/*	Moves the client pitch angle towards cl.idealpitch sent by the server.

	If the user is adjusting pitch manually, either with lookup/lookdown,
	mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.

	Drifting is enabled when the center view key is hit, mlook is released and
	lookspring is non 0, or when
*/
void V_DriftPitch (void)
{
	float		delta, move;

	if (noclip_anglehack || !cl.bIsOnGround || cls.demoplayback )
	//FIXME: noclip_anglehack is set on the server, so in a nonlocal game this won't work.
	{
		cl.driftmove = 0;
		cl.pitchvel = 0;
		return;
	}

	// Don't count small mouse motion
	if(cl.bNoDrift)
	{
		if ( fabs(cl.cmd.forwardmove) < cl_forwardspeed.value)
			cl.driftmove = 0;
		else
			cl.driftmove += host_frametime;

		if ( cl.driftmove > v_centermove.value)
			V_StartPitchDrift ();
		return;
	}

	delta = cl.idealpitch - cl.viewangles[PITCH];
	if(!delta)
	{
		cl.pitchvel = 0;
		return;
	}

	move = host_frametime * cl.pitchvel;
	cl.pitchvel += host_frametime * v_centerspeed.value;

//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			cl.pitchvel = 0;
			move = delta;
		}
		cl.viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			cl.pitchvel = 0;
			move = -delta;
		}
		cl.viewangles[PITCH] -= move;
	}
}

/*
==============================================================================

	VIEW BLENDING

==============================================================================
*/

cshift_t	cshift_empty	= { {130,	80,	50	},	0	};
cshift_t	cshift_water	= { {130,	80,	50	},	128	};
cshift_t	cshift_slime	= { {0,		25,	5	},	150 };
cshift_t	cshift_lava		= { {255,	80,	0	},	150 };

vec4_t		vViewBlend;		// rgba 0.0 - 1.0

void V_ParseDamage (void)
{
	int		armor, blood;
	vec3_t	from;
	int		i;
	vec3_t	forward, right, up;
	entity_t	*ent;
	float	side;
	float	count;

	armor = MSG_ReadByte ();
	blood = MSG_ReadByte ();
	for (i=0 ; i<3 ; i++)
		from[i] = MSG_ReadCoord ();

	count = blood*0.5f+armor*0.5f;
	if(count < 10)
		count = 10;

	cl.cshifts[CSHIFT_DAMAGE].percent += 3*count;
	if (cl.cshifts[CSHIFT_DAMAGE].percent < 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;
	if (cl.cshifts[CSHIFT_DAMAGE].percent > 150)
		cl.cshifts[CSHIFT_DAMAGE].percent = 150;

	if (armor > blood)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}

	// Calculate view angle kicks
	ent = &cl_entities[cl.viewentity];

	Math_VectorSubtract(from,ent->origin,from);
	Math_VectorNormalize(from);

	Math_AngleVectors(ent->angles, forward, right, up);

	side = Math_DotProduct (from, right);
	v_dmg_roll = count*side*v_kickroll.value;

	side = Math_DotProduct (from, forward);
	v_dmg_pitch = count*side*v_kickpitch.value;

	v_dmg_time = v_kicktime.value;
}

void V_cshift_f (void)
{
	cshift_empty.destcolor[0]	= atoi(Cmd_Argv(1));
	cshift_empty.destcolor[1]	= atoi(Cmd_Argv(2));
	cshift_empty.destcolor[2]	= atoi(Cmd_Argv(3));
	cshift_empty.percent		= atoi(Cmd_Argv(4));
}

/*	When you run over an item, the server sends this command
*/
void V_BonusFlash_f(void)
{
	cl.cshifts[CSHIFT_BONUS].destcolor[0]	= 215;
	cl.cshifts[CSHIFT_BONUS].destcolor[1]	= 186;
	cl.cshifts[CSHIFT_BONUS].destcolor[2]	= 69;
	cl.cshifts[CSHIFT_BONUS].percent		= 50;
}

/*	Underwater, lava, etc each has a color shift
*/
void V_SetContentsColor(int contents)
{
	switch(contents)
	{
	case CONTENTS_LAVA:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case BSP_CONTENTS_SLIME:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	case BSP_CONTENTS_WATER:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_water;
		break;
	default:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
	}
}

void V_CalcPowerupCshift (void)
{
#if 0
	if (cl.items & IT_QUAD)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		cl.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else if (cl.items & IT_SUIT)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cl.cshifts[CSHIFT_POWERUP].percent = 20;
	}
	else if (cl.items & IT_INVISIBILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		cl.cshifts[CSHIFT_POWERUP].percent = 100;
	}
	else if (cl.items & IT_INVULNERABILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		cl.cshifts[CSHIFT_POWERUP].percent = 30;
	}
	else
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
#else
	cl.cshifts[CSHIFT_POWERUP].percent = 0;
#endif
}

// [26/3/2013] Revised ~hogsy
void View_CalculateBlend(void)
{
	vec4_t	vColor;
	int		j;

	Math_VectorClear(vColor);

	for(j = 0; j < NUM_CSHIFTS; j++)
	{
		if(!gl_cshiftpercent.value)
			continue;

		//johnfitz -- only apply leaf contents color shifts during intermission
		if(cl.intermission && j != CSHIFT_CONTENTS)
			continue;
		//johnfitz

		vColor[ALPHA] = ((cl.cshifts[j].percent*gl_cshiftpercent.value)/100.0)/255.0f;
		if(!vColor[ALPHA])
			continue;

		vColor[RED]		= vColor[RED]*(1-vColor[ALPHA])+cl.cshifts[j].destcolor[0]*vColor[ALPHA];
		vColor[GREEN]	= vColor[GREEN]*(1-vColor[ALPHA])+cl.cshifts[j].destcolor[1]*vColor[ALPHA];
		vColor[BLUE]	= vColor[BLUE]*(1-vColor[ALPHA])+cl.cshifts[j].destcolor[2]*vColor[ALPHA];
	}

	Math_VectorDivide(vColor,255.0f,vColor);
	Math_VectorCopy(vColor,vViewBlend);

	if(vViewBlend[3] > 1.0f)
		vViewBlend[3] = 1.0f;
	else if(vViewBlend[3] < 0)
		vViewBlend[3] = 0;
}

void V_UpdateBlend(void)
{
	int		i,j;
	bool	bBlendChanged = false;

	V_CalcPowerupCshift();

	for(i = 0; i < NUM_CSHIFTS; i++)
	{
		if (cl.cshifts[i].percent != cl.prev_cshifts[i].percent)
		{
			bBlendChanged = true;
			cl.prev_cshifts[i].percent = cl.cshifts[i].percent;
		}

		for (j=0 ; j<3 ; j++)
			if (cl.cshifts[i].destcolor[j] != cl.prev_cshifts[i].destcolor[j])
			{
				bBlendChanged = true;
				cl.prev_cshifts[i].destcolor[j] = cl.cshifts[i].destcolor[j];
			}
	}

	// Drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime*150;
	if(cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

	// Drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime*100;
	if(cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	if(bBlendChanged)
		View_CalculateBlend();
}

void View_PolyBlend(void)
{
	if(!gl_polyblend.value || !vViewBlend[3])
		return;

    Video_ResetCapabilities(false);

	Video_DisableCapabilities(VIDEO_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	glOrtho(0,1,1,0,-99999,99999);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	{
		VideoObject_t	voScreenPoly[]=
		{
			{	{	0,		0		},	{{0}},	{	vViewBlend[0],	vViewBlend[1],	vViewBlend[2],	vViewBlend[3]	}	},
			{	{	1.0f,	0		},	{{0}},	{	vViewBlend[0],	vViewBlend[1],	vViewBlend[2],	vViewBlend[3]	}	},
			{	{	1.0f,	1.0f	},	{{0}},	{	vViewBlend[0],	vViewBlend[1],	vViewBlend[2],	vViewBlend[3]	}	},
			{	{	0,		1.0f	},	{{0}},	{	vViewBlend[0],	vViewBlend[1],	vViewBlend[2],	vViewBlend[3]	}	}
		};

		Video_DrawFill(voScreenPoly);
	}

	Video_ResetCapabilities(true);
}

/*
==============================================================================

	VIEW RENDERING

==============================================================================
*/

float angledelta (float a)
{
	a = Math_AngleMod(a);
	if (a > 180)
		a -= 360;

	return a;
}

// [26/3/2013] Cleaned up and revised ~hogsy
void View_ModelAngle(void)
{
	float			fMove;
	vec3_t			vAngles;
	static	float	fOldYaw		= 0,
					fOldPitch	= 0;

	// [26/3/2013] Don't bother if we don't have a view model to show! ~hogsy
	if(!cl.viewent.model)
		return;

	vAngles[YAW]	= r_refdef.viewangles[YAW];
	vAngles[PITCH]	= -r_refdef.viewangles[PITCH];

	vAngles[YAW] = angledelta(vAngles[YAW]-r_refdef.viewangles[YAW])*0.4f;
	if(vAngles[YAW] > 10.0f)
		vAngles[YAW] = 10.0f;
	else if(vAngles[YAW] < -10.0f)
		vAngles[YAW] = -10.0f;

	vAngles[PITCH] = angledelta(-vAngles[PITCH]-r_refdef.viewangles[PITCH])*0.4f;
	if(vAngles[PITCH] > 10.0f)
		vAngles[PITCH] = 10.0f;
	else if(vAngles[PITCH] < -10.0f)
		vAngles[PITCH] = -10.0f;

	fMove =	host_frametime*20.0f;
	if(vAngles[YAW] > fOldYaw)
	{
		if(fOldYaw+fMove < vAngles[YAW])
			vAngles[YAW] = fOldYaw+fMove;
	}
	else
	{
		if(fOldYaw-fMove > vAngles[YAW])
			vAngles[YAW] = fOldYaw-fMove;
	}

	if(vAngles[PITCH] > fOldPitch)
	{
		if(fOldPitch+fMove < vAngles[PITCH])
			vAngles[PITCH] = fOldPitch+fMove;
	}
	else
	{
		if(fOldPitch-fMove > vAngles[PITCH])
			vAngles[PITCH] = fOldPitch-fMove;
	}

	fOldYaw		= vAngles[YAW];
	fOldPitch	= vAngles[PITCH];

	cl.viewent.angles[YAW]		= r_refdef.viewangles[YAW]+vAngles[YAW];
	cl.viewent.angles[PITCH]	= -(r_refdef.viewangles[PITCH]+vAngles[PITCH]-(sin(cl.time*1.5f)*0.2f));
	cl.viewent.angles[ROLL]		= sin(cl.time*v_iroll_cycle.value)*0.5f;
}

void V_BoundOffsets (void)
{
	entity_t *eViewEntity;

	eViewEntity = &cl_entities[cl.viewentity];
	if(eViewEntity)
	{
		/*	Absolutely bound refresh reletive to entity clipping hull
			so the view can never be inside a solid wall.
		*/
		if(r_refdef.vieworg[0] < eViewEntity->origin[0]-14.0f)
			r_refdef.vieworg[0] = eViewEntity->origin[0]-14.0f;
		else if(r_refdef.vieworg[0] > eViewEntity->origin[0]+14.0f)
			r_refdef.vieworg[0] = eViewEntity->origin[0]+14.0f;

		if(r_refdef.vieworg[1] < eViewEntity->origin[1]-14.0f)
			r_refdef.vieworg[1] = eViewEntity->origin[1]-14.0f;
		else if (r_refdef.vieworg[1] > eViewEntity->origin[1]+14.0f)
			r_refdef.vieworg[1] = eViewEntity->origin[1]+14.0f;

		if(r_refdef.vieworg[2] < eViewEntity->origin[2]-22.0f)
			r_refdef.vieworg[2] = eViewEntity->origin[2]-22.0f;
		else if (r_refdef.vieworg[2] > eViewEntity->origin[2]+30.0f)
			r_refdef.vieworg[2] = eViewEntity->origin[2]+30.0f;
	}
}

/*	Idle swaying
*/
void V_AddIdle(void)
{
	r_refdef.viewangles[ROLL]	+= v_idlescale.value*sin(cl.time*v_iroll_cycle.value)*v_iroll_level.value;
	r_refdef.viewangles[PITCH]	+= v_idlescale.value*sin(cl.time*v_ipitch_cycle.value)*v_ipitch_level.value;
	r_refdef.viewangles[YAW]	+= v_idlescale.value*sin(cl.time*v_iyaw_cycle.value)*v_iyaw_level.value;
}

/*	Roll is induced by movement and damage
*/
void V_CalcViewRoll(void)
{
	float		side;

	side = V_CalcRoll (cl_entities[cl.viewentity].angles, cl.velocity);
	r_refdef.viewangles[ROLL] += side;

	if (v_dmg_time > 0)
	{
		r_refdef.viewangles[ROLL] += v_dmg_time/v_kicktime.value*v_dmg_roll;
		r_refdef.viewangles[PITCH] += v_dmg_time/v_kicktime.value*v_dmg_pitch;
		v_dmg_time -= host_frametime;
	}

	if (cl.stats[STAT_HEALTH] <= 0)
	{
		r_refdef.viewangles[ROLL] = 80.0f;	// dead view angle
		return;
	}

}

void V_CalcIntermissionRefdef (void)
{
	entity_t	*ent, *view;
	float		old;

// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	Math_VectorCopy (ent->origin, r_refdef.vieworg);
	Math_VectorCopy (ent->angles, r_refdef.viewangles);
	view->model = NULL;

// allways idle in intermission
	old = v_idlescale.value;
	v_idlescale.value = 1;
	V_AddIdle ();
	v_idlescale.value = old;
}

/*	Adds a delay to the view model (such as a weapon) in the players view.
*/
void View_ModelDrift(vec3_t vOrigin,vec3_t vAngles,vec3_t vOldAngles)
{
	int				i;
	float			fScale,fSpeed,fDifference,
					fPitch;
	static	vec3_t	svLastFacing;
	vec3_t			vForward,vRight,vUp,
					vDifference;

	Math_AngleVectors(vAngles,vForward,vRight,vUp);

	if(host_frametime != 0.0f)
	{
		Math_VectorSubtract(vForward,svLastFacing,vDifference);

		fSpeed = 5.0f;

		fDifference = Math_VectorLength(vDifference);
		if((fDifference > cViewModelLag.value) && (cViewModelLag.value > 0.0f ))
			fSpeed *= fScale = fDifference/cViewModelLag.value;;

		for(i = 0; i < 3; i++)
			svLastFacing[i] += vDifference[i]*(fSpeed*host_frametime);

		Math_VectorNormalize(svLastFacing);

		for(i = 0; i < 3; i++)
			vOrigin[i] += (vDifference[i]*-1.0f)*5.0f;
	}

	Math_AngleVectors(vOldAngles,vForward,vRight,vUp);

	fPitch = vOldAngles[PITCH];
	if(fPitch > 180.0f)
		fPitch -= 360.0f;
	else if(fPitch < -180.0f)
		fPitch += 360.0f;

	for(i = 0; i < 3; i++)
		vOrigin[i] += vForward[i]*(-fPitch*0.035f);

	for(i = 0; i < 3; i++)
		vOrigin[i] += vRight[i]*(-fPitch*0.03f);

	for(i = 0; i < 3; i++)
		vOrigin[i] += vUp[i]*(-fPitch*0.02f );
}

void V_CalcRefdef (void)
{
	int				i;
	float			fBob[1],fCycle[1],
					delta;
	static	float	oldz = 0;
	entity_t		*ent,*view;
	vec3_t			forward,right,up,
					angles,vOldAngles;
	static	vec3_t	punch = {0,0,0}; //johnfitz -- v_gunkick

	V_DriftPitch();

	// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
	// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	// transform the view offset by the model's matrix to get the offset from
	// model origin for the view
	ent->angles[YAW]	= cl.viewangles[YAW];	// the model should face the view dir
	ent->angles[PITCH]	= -cl.viewangles[PITCH];	// the model should face the view dir

	// [10/5/2013] Forward cycle ~hogsy
	fCycle[0] = (cl.time-(int)(cl.time/cl_bobcycle.value)*cl_bobcycle.value)/cl_bobcycle.value;
	if(fCycle[0] < cl_bobup.value)
		fCycle[0] = pMath_PI*fCycle[0]/cl_bobup.value;
	else
		fCycle[0] = pMath_PI+pMath_PI*(fCycle[0]-cl_bobup.value)/(1.0f-cl_bobup.value);

	// [10/5/2013] Side cycle ~hogsy
	fCycle[1] = (cl.time-(int)(cl.time/cSideBobCycle.value)*cSideBobCycle.value)/cSideBobCycle.value;
	if(fCycle[1] < cSideBobUp.value)
		fCycle[1] = pMath_PI*fCycle[1]/cSideBobUp.value;
	else
		fCycle[1] = pMath_PI+pMath_PI*(fCycle[1]-cSideBobUp.value)/(1.0f-cSideBobUp.value);

	/*	Bob is proportional to velocity in the xy plane
		(don't count Z, or jumping messes it up)		*/
	fBob[0] = sqrt(
		cl.velocity[0]*cl.velocity[0]+
		cl.velocity[1]*cl.velocity[1])
		*cl_bob.value;
	fBob[1] = sqrt(
		cl.velocity[0]*cl.velocity[0]+
		cl.velocity[1]*cl.velocity[1])
		*cSideBob.value;

	// [10/5/2013] Bleh... Better/cleaner method? ~hogsy
	for(i = 0; i < 2; i++)
	{
		fBob[i] *= fBob[i]*0.7f*sin(fCycle[i]);
		if(fBob[i] > 4.0f)
			fBob[i] = 4.0f;
		else if(fBob[i] < -7.0f)
			fBob[i] = -7.0f;
	}

	// [10/5/2013] Save our old angles for later... ~hogsy
	Math_VectorCopy(view->angles,vOldAngles);

	// Refresh position
	Math_VectorCopy(ent->origin,r_refdef.vieworg);
	r_refdef.vieworg[2] += cl.viewheight+fBob[0];

	/*	Never let it sit exactly on a node line, because a water plane can
		dissapear when viewed with the eye exactly on it.
		The server protocol only specifies to 1/16 pixel, so add 1/32 in each axis.
	*/
	Math_VectorAddValue(r_refdef.vieworg,1.0f/32.0f,r_refdef.vieworg);

	Math_VectorCopy(cl.viewangles,r_refdef.viewangles);

	V_CalcViewRoll();
	V_AddIdle();

	// offsets
	angles[PITCH]	= -ent->angles[PITCH]; // because entity pitches are actually backward
	angles[YAW]		= ent->angles[YAW];
	angles[ROLL]	= ent->angles[ROLL];

	Math_AngleVectors(angles,forward,right,up);

	// [10/11/2013] TODO: Do this a better way, derp ~hogsy
	for(i = 0; i < 3; i++)
		r_refdef.vieworg[i] += forward[i]+-5.0f*right[i]+up[i];

	if(cl.maxclients <= 1) //johnfitz -- moved cheat-protection here from V_RenderView
		for(i=0; i<3; i++)
			r_refdef.vieworg[i] += scr_ofsx.value*forward[i]+scr_ofsy.value*right[i]+scr_ofsz.value*up[i];

	V_BoundOffsets ();

	// Set up gun position
	Math_VectorCopy(cl.viewangles,view->angles);

	Math_VectorCopy(ent->origin,view->origin);
	view->origin[2] += cl.viewheight;

	for(i = 0; i < 3; i++)
		view->origin[i] +=	(up[i]*fBob[0]*0.2f)+
							(right[i]*fBob[1]*0.3f);

	view->origin[2] += fBob[0];

	// [10/5/2013] Implemented Eukos' suggestion ~hogsy
	View_ModelDrift(view->origin,view->angles,vOldAngles);

	//johnfitz -- removed all gun position fudging code (was used to keep gun from getting covered by sbar)

	view->model	= cl.model_precache[cl.stats[STAT_WEAPON]];
	view->frame	= cl.stats[STAT_WEAPONFRAME];

	View_ModelAngle();

//johnfitz -- v_gunkick
	if(v_gunkick.value) //lerped kick
	{
		for(i = 0; i < 3; i++)
			if(vec3_origin[i] != v_punchangles[0][i])
			{
				// Speed determined by how far we need to lerp in 1/10th of a second
				delta = (v_punchangles[0][i]-v_punchangles[1][i])*host_frametime*10.0f;
				if(delta > 0)
					punch[i] = Math_Min(punch[i]+delta,v_punchangles[0][i]);
				else if(delta < 0)
					punch[i] = Math_Max(punch[i]+delta,v_punchangles[0][i]);
			}

		Math_VectorAdd(r_refdef.viewangles,punch,r_refdef.viewangles);
	}
//johnfitz

// smooth out stair step ups
	if (!noclip_anglehack && cl.bIsOnGround && ent->origin[2] - oldz > 0) //johnfitz -- added exception for noclip
	//FIXME: noclip_anglehack is set on the server, so in a nonlocal game this won't work.
	{
		float fStepTime;

		fStepTime = (float)(cl.time-cl.oldtime);
		if(fStepTime < 0)
			//FIXME	I_Error ("steptime < 0");
			fStepTime = 0;

		oldz += fStepTime*80.0f;
		if(oldz > ent->origin[2])
			oldz = ent->origin[2];

		if(ent->origin[2]-oldz > 12.0f)
			oldz = ent->origin[2]-12.0f;

		r_refdef.vieworg[2] += oldz-ent->origin[2];
		view->origin[2]		+= oldz-ent->origin[2];
	}
	else
		oldz = ent->origin[2];

	if(chase_active.value)
		Chase_UpdateForDrawing (); //johnfitz
}

/*	TODO: Draws a muzzleflash in-front of the weapon.
*/
void View_DrawMuzzleFlash(void)
{
	// Get players active weapon

	// Set up position of sprite

	// Draw

	// Reset
}

/*	The player's clipping box goes from (-16 -16 -24) to (16 16 32) from
	the entity origin, so any view position inside that will be valid
*/
extern vrect_t	scr_vrect;

void V_RenderView (void)
{
	if (con_forcedup)
		return;

	if (cl.intermission)
		V_CalcIntermissionRefdef();
	else if (!cl.bIsPaused /* && (cl.maxclients > 1 || key_dest == key_game) */)
		V_CalcRefdef();

	// [16/6/2013] Draw first since we'll draw the gun on top of this for effect :) ~hogsy
	View_DrawMuzzleFlash();

	R_RenderView();

	View_PolyBlend(); //johnfitz -- moved here from R_Renderview ();
}

/*
==============================================================================

	INIT

==============================================================================
*/

/*	Forces the pitch to become centered.
*/
void View_ForceCenter(void)
{
	cl.viewangles[PITCH] = 0;
}

void V_Init (void)
{
	Cmd_AddCommand("v_cshift", V_cshift_f);
	Cmd_AddCommand("bf", V_BonusFlash_f);
	Cmd_AddCommand("centerview", V_StartPitchDrift);
	Cmd_AddCommand("view_forcecenter",View_ForceCenter);

	Cvar_RegisterVariable (&v_centermove, NULL);
	Cvar_RegisterVariable (&v_centerspeed, NULL);

	Cvar_RegisterVariable (&v_iyaw_cycle, NULL);
	Cvar_RegisterVariable (&v_iroll_cycle, NULL);
	Cvar_RegisterVariable (&v_ipitch_cycle, NULL);
	Cvar_RegisterVariable (&v_iyaw_level, NULL);
	Cvar_RegisterVariable (&v_iroll_level, NULL);
	Cvar_RegisterVariable (&v_ipitch_level, NULL);

	Cvar_RegisterVariable (&v_idlescale, NULL);
	Cvar_RegisterVariable (&crosshair, NULL);
	Cvar_RegisterVariable (&gl_cshiftpercent, NULL);

	Cvar_RegisterVariable (&scr_ofsx, NULL);
	Cvar_RegisterVariable (&scr_ofsy, NULL);
	Cvar_RegisterVariable (&scr_ofsz, NULL);
	Cvar_RegisterVariable (&cl_rollspeed, NULL);
	Cvar_RegisterVariable (&cl_rollangle, NULL);
	Cvar_RegisterVariable (&cl_bob, NULL);
	Cvar_RegisterVariable (&cl_bobcycle, NULL);
	Cvar_RegisterVariable (&cl_bobup, NULL);
	Cvar_RegisterVariable (&v_kicktime, NULL);
	Cvar_RegisterVariable (&v_kickroll, NULL);
	Cvar_RegisterVariable (&v_kickpitch, NULL);
	Cvar_RegisterVariable (&v_gunkick, NULL); //johnfitz

	Cvar_RegisterVariable(&cSideBob,NULL);
	Cvar_RegisterVariable(&cSideBobCycle,NULL);
	Cvar_RegisterVariable(&cSideBobUp,NULL);
	Cvar_RegisterVariable(&cViewModelLag,NULL);
}


