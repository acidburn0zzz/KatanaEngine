/*	Copyright (C) 2011-2015 OldTimes Software
*/
#ifndef __SHAREDMATH__
#define __SHAREDMATH__

// OBSOLETE: ALL OF THIS IS BEING TRANSITIONED INTO PLATFORM_MATH

#include "platform.h"
#include "platform_math.h"

typedef int fixed8_t;

/*	Can make it easier for
	dealing with vectors :)
	[28/8/2012]
	Moved into KatMath.h ~hogsy
*/
enum	{	WIDTH,	HEIGHT			};	//	0,	1
enum	{	X,	Y,	Z				};	//	0,	1,	2
enum	{	PITCH,	YAW,	ROLL	};	//	0,	1,	2

/*	Can make it easier to
	deal with any colours :)
	[27/8/2012]
	Added ALPHA. ~hogsy
	[28/8/2012]
	Moved into KatMath.h ~hogsy
*/
enum	{	RED,	GREEN,	BLUE,	ALPHA	};	//	0,	1,	2,	3

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#define Q_rint(x)			((x)>0?(int)((x)+0.5):(int)((x)-0.5))

#define M_PI_DIV_180		(pMath_PI/180.0f)

#define DEG2RAD(a)			((a)*M_PI_DIV_180)

#define	IS_NAN(x)			(((*(int*)&x)&255<<23)==255<<23)

extern	vec3_t				vec3_origin;

#define EQUAL_EPSILON		0.001

void	_Math_VectorSubtract(vec3_t a,vec3_t b,vec3_t c);
void	_Math_VectorAdd(vec3_t a,vec3_t b,vec3_t c);
void	_Math_VectorCopy(vec3_t a,vec3_t b);
void	_Math_VectorScale(vec3_t in,vec_t scale,vec3_t out);

struct	mplane_s;

void	Math_VectorAngles(const vec3_t forward,vec3_t angles);
void	Math_AngleVectors(vec3_t angles,vec3_t forward,vec3_t right,vec3_t up);
void	Math_VectorNormalizeFast(vec3_t vVector);
void	Math_VectorMake(vec3_t veca,float scale,vec3_t vecb,vec3_t vecc);

int		Math_BoxOnPlaneSide(vec3_t emins,vec3_t emaxs,struct mplane_s *p);

double	Math_VectorLength(vec3_t a);

float	Math_AngleMod(float a);
float	Math_VectorToYaw(vec_t *vVector);

bool	Math_VectorCompare(vec3_t a,vec3_t b);

vec_t	Math_Length(vec3_t a);
vec_t	Math_VectorNormalize(vec3_t a);
vec_t	Math_ColorNormalize(vec3_t in,vec3_t out);
vec_t	Math_DotProduct(vec3_t a,vec3_t b);

#define	CLAMP(mini,x,maxi)				Math_Clamp(mini,x,maxi)						// [4/2/2014] TODO: Replace all CLAMP calls with Math_Clamp! ~hogsy

#endif
