/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "shared_math.h"

/*
	Shared Math Library
*/

#if	_MSC_VER
#include <xmmintrin.h>
#endif

vec3_t	vec3_origin	=
		{	0,	0,	0	};

#ifdef KATANA
#include "quakedef.h"

int Math_BoxOnPlaneSide(vec3_t emins,vec3_t emaxs,mplane_t *p)
{
	float	dist1,dist2;
	int		sides;

	// Fast axial cases
	if(p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}

	switch(p->signbits)
	{
	case 0:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
	dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	return sides;
}
#endif

MathVector_t Math_VectorToAngles(vec3_t vValue)
{
	float			fForward,fYaw,fPitch;
	MathVector_t	mvResult;

	if(vValue[1] == 0 && vValue[0] == 0)
	{
		fYaw = 0;
		if(vValue[2] > 0)
			fPitch = 90.0f;
		else
			fPitch = 270.0f;
	}
	else
	{
		fYaw = (float)(atan2(vValue[1],vValue[0])*180/pMath_PI);
		if(fYaw < 0)
			fYaw += 360.0f;

		fForward	= (float)sqrt(vValue[0]*vValue[0]+vValue[1]*vValue[1]);
		fPitch		= (float)(atan2(vValue[2],fForward)*180/pMath_PI);
		if(fPitch < 0)
			fPitch += 360.0f;
	}

	mvResult.vX	= fPitch;
	mvResult.vY	= fYaw;
	mvResult.vZ	= 0;

	return mvResult;
}

float Math_VectorToYaw(vec_t *vVector)
{
	float fResult;

	if(vVector[1] == 0 && vVector[0] == 0)
		fResult = 0;
	else
	{
		fResult = (float)(atan2(vVector[1],vVector[0])*180.0f/pMath_PI);
		if(fResult < 0)
			fResult += 360.0f;
	}

	return fResult;
}

float Math_AngleMod(float a)
{
	a = (360.0f/65536)*((int)(a*(65536/360.0f))&65535);

	return a;
}

vec_t Math_Length(vec3_t a)
{
	int		i;
	float	l;

	l = 0;
	for(i=0;i<3;i++)
		l += a[i]*a[i];
	l = (float)sqrt(l);

	return l;
}

double Math_VectorLength(vec3_t a)
{
	int		i;
	double	length;

	length = 0;
	for (i=0 ; i< 3 ; i++)
		length += a[i]*a[i];
	length = sqrt(length);

	return length;
}

vec_t Math_VectorNormalize(vec3_t a)
{
	vec_t	i,l = (vec_t)Math_VectorLength(a);
	if(l)
	{
		i = 1.0f/l;
		Math_VectorScale(a,i,a);
	}

	return l;
}

void Math_VectorNormalizeFast(vec3_t vVector)
{
	float y,fNumber;

	fNumber = Math_DotProduct(vVector,vVector);
	if(fNumber != 0.0f)
	{
		*((long*)&y) = 0x5f3759df-((*(long*)&fNumber) >> 1);
		y = y*(1.5f-(fNumber*0.5f*y*y));

		Math_VectorScale(vVector,y,vVector);
	}
}

bool Math_VectorCompare(vec3_t a,vec3_t b)
{
	int i;

	for(i = 0; i < 3; i++)
		if(a[i] != b[i])
			return false;

	return true;
}

void Math_AngleVectors(vec3_t angles,vec3_t forward,vec3_t right,vec3_t up)
{
	float	angle,sr,sp,sy,cr,cp,cy;

	angle	= angles[YAW]*((float)pMath_PI*2/360);
	sy		= (float)sin(angle);
	cy		= (float)cos(angle);
	angle	= angles[PITCH]*((float)pMath_PI*2/360);
	sp		= (float)sin(angle);
	cp		= (float)cos(angle);
	angle	= angles[ROLL]*((float)pMath_PI*2/360);
	sr		= (float)sin(angle);
	cr		= (float)cos(angle);

	if(forward)
	{
		forward[0]	= cp*cy;
		forward[1]	= cp*sy;
		forward[2]	= -sp;
	}
	if(right)
	{
		right[0]	= (-1*sr*sp*cy+-1*cr*-sy);
		right[1]	= (-1*sr*sp*sy+-1*cr*cy);
		right[2]	= -1*sr*cp;
	}
	if(up)
	{
		up[0]		= (cr*sp*cy+-sr*-sy);
		up[1]		= (cr*sp*sy+-sr*cy);
		up[2]		= cr*cp;
	}
}

void Math_VectorAngles(const vec3_t forward,vec3_t angles)
{
	vec3_t temp;

	temp[0] = forward[0];
	temp[1] = forward[1];
	temp[2] = 0;

	angles[PITCH]	= (float)-atan2(forward[2],Math_Length(temp))/(float)M_PI_DIV_180;
	angles[YAW]		= (float)atan2(forward[1],forward[0])/(float)M_PI_DIV_180;
	angles[ROLL]	= 0;
}

vec_t Math_DotProduct(vec3_t a,vec3_t b)
{
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

void _Math_VectorCopy(vec3_t a,vec3_t b)
{
	b[0] = a[0];
	b[1] = a[1];
	b[2] = a[2];
}

void _Math_VectorScale(vec3_t in,vec_t scale,vec3_t out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

void _Math_VectorAdd(vec3_t a,vec3_t b,vec3_t c)
{
	c[0] = a[0]+b[0];
	c[1] = a[1]+b[1];
	c[2] = a[2]+b[2];
}

void _Math_VectorSubtract(vec3_t a,vec3_t b,vec3_t c)
{
	c[0] = a[0]-b[0];
	c[1] = a[1]-b[1];
	c[2] = a[2]-b[2];
}

// [23/2/2013] Added Math_VectorMake ~eukos
void Math_VectorMake(vec3_t veca,float scale,vec3_t vecb,vec3_t vecc)
{
	vecc[0] = veca[0]+scale*vecb[0];
	vecc[1] = veca[1]+scale*vecb[1];
	vecc[2] = veca[2]+scale*vecb[2];
}

// [1/8/2012] Added Math_ColorNormalize (stolen from Quake 2) ~hogsy
vec_t Math_ColorNormalize(vec3_t in,vec3_t out)
{
	float	max,scale;

	max = in[0];
	if(in[1] > max)
		max = in[1];
	else if(in[2] > max)
		max = in[2];

	if(!max)
		return 0;

	scale = 1.0f/max;

	Math_VectorScale(in,scale,out);

	return max;
}
