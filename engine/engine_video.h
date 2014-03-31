/*  Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __ENGINEVIDEO__
#define __ENGINEVIDEO__

#include "quakedef.h"

#include <SDL.h>

extern cvar_t	cvShowModels,	// Should we draw models?
				cvWidth,		// The width of our window (not reliable).
				cvHeight,		// The height of our window (not reliable).
				cvFullscreen,	// Should we be fullscreen?
				cvLitParticles;	// Should particles be lit or not?

typedef struct
{
	float			fMaxAnisotropy,		// Max anisotropy amount allowed by the hardware.
					fBitsPerPixel;

	unsigned	int	iCurrentTexture;	// Current/last binded texture.
	int				iSamples,			// Current number of samples set for AA.
					iBuffers;

	unsigned	int	iWidth,
					iHeight;

	bool			bInitialized,		// Is the video system started?
					bFullscreen,		// Is the window fullscreen or not?
					bVerticalSync,		// Sync the swap interval to the refresh rate?
					bActive,			// Is the window active or not?
					bSkipUpdate,		// Skip screen update.
					bUnlocked;			// Can we change the window settings or not?

	// OpenGL Extensions
	bool	bFogCoord,					// EXT_fog_coord
			bMultitexture,				// ARB_multitexture
			bTextureEnvAdd,				// ARB_texture_env_add
			bTextureEnvCombine;			// ARB_texture_env_combine
} Video_t;

typedef struct
{
	vec3_t	vVertex;
	vec2_t	vTextureCoord[4];	// Texture coordinates by texture unit.

	vec4_t	vColour;			// RGBA

	vec3_t	vNormal;			// Vertex normals.
} VideoObject_t;

Video_t	Video;

extern SDL_Window	*sMainWindow;

// Video Capabilities
#define	VIDEO_ALPHA_TEST	1
#define	VIDEO_BLEND			2
#define	VIDEO_TEXTURE_2D	4
#define	VIDEO_DEPTH_TEST	8	// Depth-testing.
#define	VIDEO_TEXTURE_GEN_T	16
#define	VIDEO_TEXTURE_GEN_S	32
#define	VIDEO_CULL_FACE		64
#define	VIDEO_STENCIL_TEST	128
#define	VIDEO_NORMALIZE		256	// Normalization for scaled models that are lit

#define VIDEO_TEXTURE0 0x84C0
#define VIDEO_TEXTURE1 0x84C1

// Primitive Types
typedef enum
{
    VIDEO_PRIMITIVE_TRIANGLES,
    VIDEO_PRIMITIVE_TRIANGLE_FAN
} VideoPrimitive_t;

// Blending Modes
typedef enum
{
    VIDEO_BLEND_ONE,    // ONE ONE
    VIDEO_BLEND_TWO,    // RENAME!!!

    VIDEO_BLEND_ZERO    // ZERO ZERO
} VideoBlend_t;

#define VIDEO_DEPTH_TRUE    true
#define VIDEO_DEPTH_FALSE   false
#define VIDEO_DEPTH_IGNORE  -1

// Shader Types
#define VIDEO_SHADER_VERTEX     0
#define VIDEO_SHADER_FRAGMENT   1

#if 0
typedef struct
{
    VideoPrimitive_t    vpPrimitiveType;
    VideoBlend_t        vbBlendType;

    int                 iCapabilities;

    VideoObject_t       *voObject;
} VideoInstance_t;
#endif

void Video_Initialize(void);
void Video_UpdateWindow(void);
void Video_ClearBuffer(void);
void Video_SetTexture(gltexture_t *gTexture);
void Video_SetBlend(VideoBlend_t voBlendMode,int iDepthType);
void Video_EnableMultitexture(void);
void Video_DisableMultitexture(void);
void Video_EnableCapabilities(unsigned int iCapabilities);
void Video_DisableCapabilities(unsigned int iCapabilities);
void Video_ResetCapabilities(bool bClearActive);
void Video_Process(void);
void Video_DrawFill(VideoObject_t *voFill);
void Video_DrawObject(VideoObject_t *voObject,VideoPrimitive_t vpPrimitiveType,unsigned int uiTriangles,bool bMultiTexture);
void Video_Shutdown(void);

bool Video_CreateWindow(void);

/*
	Draw
*/

void Draw_Particles(void);
void Draw_ResetCanvas(void);

/*
	Sky
*/

void Sky_Draw(void);

/*
	Light
*/

void Light_Draw(void);
void Light_Animate(void);
void Light_MarkLights(DynamicLight_t *light,int bit,mnode_t *node);

MathVector_t Light_GetSample(vec3_t vPoint);

DynamicLight_t *Light_GetDynamic(vec3_t vPoint);

/*
	World
*/

void World_Draw(void);
void World_DrawWaterTextureChains(void);

/*
	Brush
*/

void Brush_Draw(entity_t *e);

/*
	Warp
*/

void Warp_DrawWaterPoly(glpoly_t *p);

#endif
