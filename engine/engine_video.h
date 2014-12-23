/*  Copyright (C) 2011-2015 OldTimes Software
*/
#ifndef __ENGINEVIDEO__
#define __ENGINEVIDEO__

#include <SDL.h>
#include <SDL_syswm.h>

extern cvar_t	cvVideoDrawModels,	// Should we draw models?
				cvWidth,			// The width of our window (not reliable).
				cvHeight,			// The height of our window (not reliable).
				cvFullscreen,		// Should we be fullscreen?
				cvVideoAlphaTrick,
				cvLitParticles;		// Should particles be lit or not?

#define	VIDEO_MAX_UNITS	4

extern bool	bVideoIgnoreCapabilities;

typedef struct
{
	float			fMaxAnisotropy,		        // Max anisotropy amount allowed by the hardware.
					fBitsPerPixel;

    // Texture Management
	unsigned	int	iCurrentTexture,					// Current/last binded texture.
                    uiActiveUnit,
                    uiSecondaryUnit;					// Current/last secondary texture.

	unsigned	int	uiMSAASamples,	// Number of AA samples.
					iWidth,iHeight;

	bool			bInitialized,		// Is the video system started?
					bFullscreen,		// Is the window fullscreen or not?
					bVerticalSync,		// Sync the swap interval to the refresh rate?
					bActive,			// Is the window active or not?
					bSkipUpdate,		// Skip screen update.
					bUnlocked;			// Can we change the window settings or not?

	// OpenGL Extensions
	bool	bFogCoord,					// EXT_fog_coord
			bTextureEnvAdd,				// ARB_texture_env_add
			bTextureEnvCombine;			// ARB_texture_env_combine

	SDL_SysWMinfo	sSystemInfo;
} Video_t;

typedef struct
{
	vec3_t	vVertex;

	vec2_t	vTextureCoord[VIDEO_MAX_UNITS+1];	// Texture coordinates by texture unit.

	vec4_t	vColour;							// RGBA

	vec3_t	vNormal;							// Vertex normals.
} VideoObject_t;

Video_t	Video;

extern SDL_Window	*sMainWindow;

// Video Capabilities
#define	VIDEO_ALPHA_TEST	1	// Alpha-testing
#define	VIDEO_BLEND			2	// Blending
#define	VIDEO_TEXTURE_2D	4	// Enables/disables textures.
#define	VIDEO_DEPTH_TEST	8	// Depth-testing
#define	VIDEO_TEXTURE_GEN_T	16	// Generate T coordinate.
#define	VIDEO_TEXTURE_GEN_S	32	// Generate S coordinate.
#define	VIDEO_CULL_FACE		64	// Automatically cull faces.
#define	VIDEO_STENCIL_TEST	128	// Stencil-testing
#define	VIDEO_NORMALIZE		256	// Normalization for scaled models that are lit.

#define VIDEO_TEXTURE0	0x84C0	// TMU0
#define VIDEO_TEXTURE1	0x84C1	// TMU1
#define	VIDEO_TEXTURE2	0x84C2	// TMU2
#define	VIDEO_TEXTURE3	0x84C3	// TMU3

// Primitive Types
typedef enum
{
    VIDEO_PRIMITIVE_TRIANGLES,
    VIDEO_PRIMITIVE_TRIANGLE_FAN
} VideoPrimitive_t;

// Blending Modes
typedef enum VideoBlend_e
{
    VIDEO_BLEND_IGNORE, // Don't bother changing blend mode.

    VIDEO_BLEND_ONE,    // ONE			ONE
    VIDEO_BLEND_TWO,    // SRC_ALPHA	ONE_MINUS_SRC_ALPHA
    VIDEO_BLEND_THREE,  // DST_COLOR	SRC_COLOR
    VIDEO_BLEND_FOUR    // ZERO			ZERO
} VideoBlend_t;

// Depth Modes
typedef enum VideoDepth_e
{
	VIDEO_DEPTH_IGNORE = -1,	// Don't bother changing depth mode.
	VIDEO_DEPTH_FALSE,			// Don't enable depth mask.
	VIDEO_DEPTH_TRUE			// Enable depth mask.
} VideoDepth_t;

// Shader Types
#define VIDEO_SHADER_VERTEX     0
#define VIDEO_SHADER_FRAGMENT   1

void Video_Initialize(void);
void Video_CreateWindow(void);
void Video_UpdateWindow(void);
void Video_ClearBuffer(void);
void Video_GenerateSphereCoordinates(void);
void Video_SetTexture(gltexture_t *gTexture);
void Video_SetBlend(VideoBlend_t voBlendMode, VideoDepth_t vdDepthMode);
void Video_SelectTexture(unsigned int uiTarget);
void Video_EnableCapabilities(unsigned int iCapabilities);
void Video_DisableCapabilities(unsigned int iCapabilities);
void Video_ResetCapabilities(bool bClearActive);
void Video_Process(void);
void Video_DrawFill(VideoObject_t *voFill);
void Video_DrawObject(VideoObject_t *voObject,VideoPrimitive_t vpPrimitiveType,unsigned int uiVerts);
void Video_Shutdown(void);

/*
	Draw
*/

void Draw_Particles(void);
void Draw_ResetCanvas(void);

/*
    Sprite
*/

void Sprite_Draw(entity_t *eEntity);

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
