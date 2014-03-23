/*  Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __PLATFORMMATH__
#define	__PLATFORMMATH__

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

#ifndef PLATFORM_MATH_DOUBLE
typedef	float	vec_t;
#else
typedef	double	vec_t;
#endif

typedef vec_t	vec2_t[2];
typedef vec_t	vec3_t[3];
typedef vec_t	vec4_t[4];

typedef struct
{
	vec_t	vX,
			vY,
			vZ;
} MathVector_t;

#define Math_PI 3.14159265358979323846

#define Math_Min(a,b)					(	((a)<(b))?(a):(b)	                    )
#define Math_Max(a,b)                   (	((a)>(b))?(a):(b)	                    )
#define Math_Clamp(mini,x,maxi)			(   (x)<(mini)?(mini):(x)>(maxi)?(maxi):(x) )

#define	Math_MVSet(a,b)					(	b.vX=a,b.vY=a,b.vZ=a			)
#define	Math_MVToVector(a,b)			(	b[0]=a.vX,b[1]=a.vY,b[2]=a.vZ	)

#if 0
#define	Math_VectorSubtract(a,b,c)	_Math_VectorSubtract(a,b,c)
#define Math_VectorAdd(a,b,c)		_Math_VectorAdd(a,b,c)
#define Math_VectorCopy(a,b)		_Math_VectorCopy(a,b)
#define Math_VectorScale(a,b,c)		_Math_VectorScale(a,b,c)
#else
#define Math_VectorSubtract(a,b,c)	{	c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];								}
#define Math_VectorAdd(a,b,c)		{	c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];								}
#define Math_VectorCopy(a,b)		(	b[0]=a[0],b[1]=a[1],b[2]=a[2]												)
#define Math_VectorScale(a,b,c)		{	c[0]=a[0]*b;c[1]=a[1]*b;c[2]=a[2]*b;										}
#endif
#define	Math_VectorAddValue(a,b,c)		{	c[0]=a[0]+b;c[1]=a[1]+b;c[2]=a[2]+b;										}
#define	Math_VectorSubtractValue(a,b,c)	{	c[0]=a[0]-b;c[1]=a[1]-b;c[2]=a[2]-b;										}
#define Math_CrossProduct(a,b,c)		{	c[0]=a[1]*b[2]-a[2]*b[1];c[1]=a[2]*b[0]-a[0]*b[2];c[2]=a[0]*b[1]-a[1]*b[0];	}
#define Math_VectorMA(a,b,c,d)			{	d[0]=a[0]+b*c[0];d[1]=a[1]+b*c[1];d[2]=a[2]+b*c[2];							}
#define Math_VectorInverse(a)			(	a[0]=-a[0],a[1]=-a[1],a[2]=-a[2]											)
#define Math_VectorClear(a)				(	a[0]=a[1]=a[2]=0															)
#define Math_VectorNegate(a,b)			(	b[0]=-a[0],b[1]=-a[1],b[2]=-a[2]											)
#define	Math_VectorSet(a,b)				(	b[0]=b[1]=b[2]=a															)
#define	Math_VectorDivide(a,b,c)		{	c[0]=a[0]/b;c[1]=a[1]/b;c[2]=a[2]/b;										}
#define	Math_VectorToMV(a,b)			(	b.vX=a[0],b.vY=a[1],b.vZ=a[2]												)

#define	Math_Vector4Copy(a,b)			(	b[0]=a[0],b[1]=a[1],b[2]=a[2],b[3]=a[3]	)
#define	Math_Vector4Set(a,b)			(	b[0]=b[1]=b[2]=b[3]=a					)

// [22/2/2014] Moved Eukos' random defs here ~hogsy
#define Math_Random()	((rand() & 0x7fff)/((float)0x7fff))
#define Math_CRandom()	(2.0f*((float)Math_Random()-0.5f))

#ifdef __cplusplus
}
#endif

#endif
