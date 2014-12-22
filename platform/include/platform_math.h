/*  Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2011-2015 OldTimes Software
*/
#ifndef __PLATFORMMATH__
#define	__PLATFORMMATH__

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

#ifndef PLATFORM_MATH_DOUBLE
typedef	float	vec_t;

#define pMath_PI			3.14159265358979323846f
#define	pMath_PI_DIV180		(pMath_PI/180.0f)
#define	pMath_EPSILON_ON	0.1f					// Point on plane side epsilon.
#define	pMath_EPSILON_EQUAL	0.001f
#else
typedef	double	vec_t;

#define pMath_PI			3.14159265358979323846
#define	pMath_PI_DIV180		(pMath_PI/180.0)
#define	pMath_EPSILON_ON	0.1						// Point on plane side epsilon.
#define	pMath_EPSILON_EQUAL	0.001
#endif

#define	pMath_RINT(a)		((a)>0?(int)((a)+0.5):(int)((a)-0.5))
#define	pMath_DEG2RAD(a)	((a)*pMath_PI_DIV180)
#define	pMath_ISNAN(a)		(((*(int*)&a)&255<<23)==255<<23)

typedef vec_t	MathVector2_t[2],MathVector3_t[3],MathVector4_t[4];
// For compatability...
#define	vec2_t	MathVector2_t
#define	vec3_t	MathVector3_t
#define	vec4_t	MathVector4_t

typedef struct
{
	vec_t	vX,
			vY,
			vZ;
} MathVector_t;

extern MathVector_t		mvOrigin;
extern MathVector2_t	mv2Origin;
extern MathVector3_t	mv3Origin;
extern MathVector4_t	mv4Origin;

enum MathWH_t
{
	pWIDTH,	pHEIGHT	// 0, 1
};
// For compatability...
#define	WIDTH	pWIDTH
#define	HEIGHT	pHEIGHT

enum MathXYZ_t
{
	pX, pY, pZ	// 0, 1, 2
};

enum MathPYR_t
{
	pPITCH, pYAW, pROLL	// 0, 1, 2
};
// For compatability...
#define PITCH	pPITCH
#define	YAW		pYAW
#define	ROLL	pROLL

enum MathRGB_t
{
	pRED, pGREEN, pBLUE, pALPHA	// 0, 1, 2, 3
};
// For compatability...
#define	RED		pRED
#define	GREEN	pGREEN
#define	BLUE	pBLUE
#define	ALPHA	pALPHA

/*
	Utility Defines
*/

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

#define	Math_Vector2Copy(a,b)			(	b[0]=a[0],b[1]=a[1]	)
#define	Math_Vector2Set(a,b)			(	b[0]=b[1]=a			)

#define	Math_Vector4Copy(a,b)			(	b[0]=a[0],b[1]=a[1],b[2]=a[2],b[3]=a[3]	)
#define	Math_Vector4Set(a,b)			(	b[0]=b[1]=b[2]=b[3]=a					)

#define Math_Random()	((rand() & 0x7fff)/((float)0x7fff))
#define Math_CRandom()	(2.0f*((float)Math_Random()-0.5f))

/*
	Utility Functions
*/

void	_Math_VectorSubtract(vec3_t a, vec3_t b, vec3_t c);
void	_Math_VectorAdd(vec3_t a, vec3_t b, vec3_t c);
void	_Math_VectorCopy(vec3_t a,vec3_t b);
void	_Math_VectorScale(vec3_t in,vec_t scale,vec3_t out);

struct	mplane_s;

void	Math_VectorAngles(const vec3_t forward, vec3_t angles);
void	Math_AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void	Math_VectorNormalizeFast(vec3_t vVector);
void	Math_VectorMake(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

double	Math_VectorLength(vec3_t a);

float	Math_AngleMod(float a);
float	Math_VectorToYaw(vec_t *vVector);

bool	Math_VectorCompare(vec3_t a, vec3_t b);

vec_t	Math_Length(vec3_t a);
vec_t	Math_VectorNormalize(vec3_t a);
vec_t	Math_ColorNormalize(vec3_t in, vec3_t out);
vec_t	Math_DotProduct(vec3_t a, vec3_t b);

MathVector_t	Math_VectorToAngles(vec3_t vValue);

#ifdef __cplusplus
}
#endif

#endif
