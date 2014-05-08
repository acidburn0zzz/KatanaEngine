/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_video.h"

/*
	KatRenderer is our future renderer code, I imagine most of this
	will be fairly incomplete before OpenKatana's release but the
	base should at least be finished by then just so it's stable...
	If not then do give me some serious scolding.

	TODO:
		- Get our desktop width and height, set initial settings to that.
		- Get the maximum supported hardware samples.
		- Move all/most API-specific code here.
*/

#include "engine_game.h"
#include "engine_input.h"
#include "engine_menu.h"

#include "platform_window.h"

SDL_Window		*sMainWindow;
SDL_GLContext	sMainContext;

#define VIDEO_MIN_WIDTH		640
#define VIDEO_MIN_HEIGHT	480
#define VIDEO_MAX_SAMPLES	16
#define VIDEO_MIN_SAMPLES	0

cvar_t	cvShowModels			= {	"video_showmodels",			"1",    false,  false,  "Toggles models."                                   };
cvar_t	cvMultisampleSamples	= {	"video_multisamplesamples",	"0",	true,   false,  "Changes the number of samples."	                };
cvar_t	cvMultisampleBuffers	= {	"video_multisamplebuffers",	"1",	true,   false,  "Changes the number of buffers."                    };
cvar_t	cvFullscreen			= {	"video_fullscreen",			"0",	true,   false,  "1: Fullscreen, 0: Windowed"	                    };
cvar_t	cvWidth					= {	"video_width",				"640",	true,   false,  "Sets the width of the window."	                    };
cvar_t	cvHeight				= {	"video_height",				"480",	true,   false,  "Sets the height of the window."	                };
cvar_t	cvVerticalSync			= {	"video_verticalsync",		"1",	true	                                                            };
cvar_t  cvVideoDebug            = { "video_debug",              "0",    false,  false,  "Provides debugging information for the renderer."  };

gltexture_t	*gDepthTexture;

bool bVideoIgnoreCapabilities = false;

/*	Initialize the renderer
*/
void Video_Initialize(void)
{
	if(Video.bInitialized)
		return;

	Con_Printf("Initializing video...\n");

	// [23/7/2013] Set default values ~hogsy
	Video.iCurrentTexture		= (unsigned int)-1;	// [29/8/2012] "To avoid unnecessary texture sets" ~hogsy
	Video.bTextureEnvCombine	=
	Video.bFogCoord				= false;
	Video.bActive				=			// Window is intially assumed active.
	Video.bUnlocked				= true;		// Video mode is initially locked.

    Cvar_RegisterVariable(&cvVideoDebug,NULL);
	Cvar_RegisterVariable(&cvMultisampleSamples,NULL);
	Cvar_RegisterVariable(&cvMultisampleBuffers,NULL);
	Cvar_RegisterVariable(&cvShowModels,NULL);
	Cvar_RegisterVariable(&cvFullscreen,NULL);
	Cvar_RegisterVariable(&cvWidth,NULL);
	Cvar_RegisterVariable(&cvHeight,NULL);
	Cvar_RegisterVariable(&cvVerticalSync,NULL);

	Cmd_AddCommand("video_restart",Video_UpdateWindow);

	// [28/7/2013] Moved check here and corrected, seems more secure ~hogsy
	if(SDL_VideoInit(NULL) < 0)
		Sys_Error("Failed to initialize video!\n%s\n",SDL_GetError());

	Video.bInitialized = true;

	// [9/7/2013] TEMP: Should honestly be called from the launcher (in a perfect world) ~hogsy
	if(!Video_CreateWindow())
		Sys_Error("Failed to create window!\n");

	SDL_DisableScreenSaver();

	// [31/10/2013] Get hardware capabilities ~hogsy
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,&Video.fMaxAnisotropy);
}

/*	Clears the color and depth buffers.
*/
void Video_ClearBuffer(void)
{
	int	iClear =
		GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT;

	// [5/9/2013] Handle the stencil buffer too ~hogsy
	if(r_shadows.value >= 2)
	{
		glClearStencil(1);

		iClear |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(iClear);
}

/*	Displays the depth buffer for testing purposes.
	Unfinished
*/
void Video_DrawDepthBuffer(void)
{
	if(!gDepthTexture)
		gDepthTexture = TexMgr_NewTexture();

	Video_SetTexture(gDepthTexture);

	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,512,512,0,GL_DEPTH_COMPONENT24,GL_UNSIGNED_BYTE,0);

	//Video_DrawFill()
}

/*	Set the gamma level.
	Unfinished
*/
void Video_SetGamma(float fAmount)
{
#if 0
	if(!SDL_SetWindowGammaRamp(sMainWindow, RAMPY DAMP! WHOOOOOO!
		Con_Warning("Failed to set gamma level!\n");
#endif
}

/*
	Window Management
*/

/*	Create our window.
*/
bool Video_CreateWindow(void)
{
	int			iFlags =
		SDL_WINDOW_RESIZABLE	|
		SDL_WINDOW_SHOWN		|
		SDL_WINDOW_OPENGL		|
		SDL_WINDOW_FULLSCREEN;
	SDL_Surface	*sIcon;

	// [2/12/2012] Normal window can't be resized ~hogsy
	iFlags &= ~SDL_WINDOW_RESIZABLE;

	if(!Video.bInitialized)
	{
		Con_Warning("Attempted to create window before video initialization!\n");
		return false;
	}

	// [15/8/2012] Figure out what resolution we're going to use ~hogsy
	if(COM_CheckParm("-window"))
	{
		Video.bFullscreen	= false;
		Video.bUnlocked		= false;
	}
	else
		// [15/8/2012] Otherwise set us as fullscreen ~hogsy
		Video.bFullscreen = (bool)cvFullscreen.value;

	if(COM_CheckParm("-width"))
	{
		Video.iWidth	= atoi(com_argv[COM_CheckParm("-width")+1]);
		Video.bUnlocked	= false;
	}
	else
		Video.iWidth = (int)cvWidth.value;

	if(COM_CheckParm("-height"))
	{
		Video.iHeight	= atoi(com_argv[COM_CheckParm("-height")+1]);
		Video.bUnlocked	= false;
	}
	else
		Video.iHeight = (int)cvHeight.value;

	if(!Video.bFullscreen)
		iFlags &= ~SDL_WINDOW_FULLSCREEN;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);

	// [1/7/2012] Ugh... ~hogsy
	if(	Video.iWidth	< VIDEO_MIN_WIDTH	||
		Video.iHeight	< VIDEO_MIN_HEIGHT)
	{
		Con_Warning("Failed to get an appropriate resolution!\n");

		Video.iWidth	= VIDEO_MIN_WIDTH;
		Video.iHeight	= VIDEO_MIN_HEIGHT;
	}

	sMainWindow = SDL_CreateWindow(
		Game->Name,				// [9/7/2013] Window name is based on the name given by Game ~hogsy
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		Video.iWidth,
		Video.iHeight,
		iFlags);
	if(!sMainWindow)
		Sys_Error("Failed to create window!\n%s\n",SDL_GetError());

	// [6/2/2014] Set the icon for the window ~hogsy
	// [25/3/2014] Grab the icon from our game directory ~hogsy
	sIcon = SDL_LoadBMP(va("%s/icon.bmp",com_gamedir));
	if(sIcon)
	{
        // [25/3/2014] Set the transparency key... ~hogsy
        SDL_SetColorKey(sIcon,true,SDL_MapRGB(sIcon->format,0,0,0));
		SDL_SetWindowIcon(sMainWindow,sIcon);
		SDL_FreeSurface(sIcon);
	}
	else
		Con_Warning("Failed to load window icon! (%s)\n",SDL_GetError());

	sMainContext = SDL_GL_CreateContext(sMainWindow);
	if(!sMainContext)
		Sys_Error("Failed to create context!\n%s\n",SDL_GetError());

	SDL_GL_SetSwapInterval(1);

	// [13/8/2012] Get any information that will be presented later ~hogsy
	gl_vendor		= (char*)glGetString(GL_VENDOR);
	gl_renderer		= (char*)glGetString(GL_RENDERER);
	gl_version		= (char*)glGetString(GL_VERSION);
	gl_extensions	= (char*)glGetString(GL_EXTENSIONS);

	// [3/6/2013] Added to fix a bug on some systems when calling wglGetExtensionString* ~hogsy
	GLeeInit();

    // [31/3/2014] Initialize AntTweakBar ~hogsy
	TwInit(TW_OPENGL,NULL);
	TwWindowSize(Video.iWidth,Video.iHeight);

	// [13/8/2012] Do we want to check for extensions? ~hogsy
	if(!COM_CheckParm("-noextensions"))
	{
		Con_DPrintf(" Checking for extensions...\n");

		if(!COM_CheckParm("-nomtex"))
			if(GLEE_ARB_multitexture)
			{
				Con_DPrintf("  ARB_multitexture\n");

				Video.bMultitexture = true;
			}

		if(!COM_CheckParm("-nocombine"))
			if(GLEE_ARB_texture_env_combine || GLEE_EXT_texture_env_combine)
			{
				Con_DPrintf("  ARB/EXT_texture_env_combine\n");

				Video.bTextureEnvCombine = true;
			}

		if(!COM_CheckParm("-noadd"))
			if(GLEE_ARB_texture_env_add || GLEE_EXT_texture_env_add)
			{
				Con_DPrintf("  ARB/EXT_texture_env_add\n");

				Video.bTextureEnvAdd = true;
			}

#ifdef KATANA_VIDEO_NEXT
		if(!GLEE_ARB_vertex_program || !GLEE_ARB_fragment_program)
			Con_Error("Unsupported video hardware!\n");

		// [5/9/2013] For future volumetric fog implementation? ~hogsy
		if(!COM_CheckParm("-nofogcoord"))
			if(GLEE_EXT_fog_coord)
			{
				Con_DPrintf("  EXT_fog_coord\n");

				Video.bFogCoord = true;
			}
#endif

#if 0
		if(!COM_CheckParm("-noswap"))
		{
			if(GLEE_WGL_EXT_swap_control)
				Con_DPrintf("  WGL_EXT_swap_control\n");
			else
				bSwapControl = false;
		}
#endif
	}

	Video_EnableCapabilities(VIDEO_TEXTURE_2D|VIDEO_ALPHA_TEST|VIDEO_NORMALIZE);

	glClearColor(0,0,0,0);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glAlphaFunc(GL_GREATER,0.666f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glDepthRange(0,1);
	glDepthFunc(GL_LEQUAL);

	vid.conwidth		= (scr_conwidth.value > 0)?(int)scr_conwidth.value:(scr_conscale.value > 0)?(int)(Video.iWidth/scr_conscale.value) : Video.iWidth;
	vid.conwidth		= Math_Clamp(320,vid.conwidth,Video.iWidth);
	vid.conwidth		&= 0xFFFFFFF8;
	vid.conheight		= vid.conwidth*Video.iHeight/Video.iWidth;
	Video.bVerticalSync	= (bool)cvVerticalSync.value;

	return true;
}

void Video_UpdateWindow(void)
{
	if(!Video.bActive)
		return;
	else if(!Video.bUnlocked)
	{
		Cvar_SetValue(cvFullscreen.name,(float)Video.bFullscreen);
		Cvar_SetValue(cvWidth.name,(float)Video.iWidth);
		Cvar_SetValue(cvHeight.name,(float)Video.iHeight);
		Cvar_SetValue(cvVerticalSync.name,(float)Video.bVerticalSync);

		Video.bUnlocked = true;
		return;
	}

	Video.iWidth	= (int)cvWidth.value;
	Video.iHeight	= (int)cvHeight.value;

	SDL_SetWindowSize(sMainWindow,Video.iWidth,Video.iHeight);

	if(Video.bVerticalSync != (bool)cvVerticalSync.value)
	{
		SDL_GL_SetSwapInterval((int)cvVerticalSync.value);

		Video.bVerticalSync = (bool)cvVerticalSync.value;
	}

	// [16/7/2013] There's gotta be a cleaner way of doing this... Ugh ~hogsy
	if(Video.bFullscreen != (bool)cvFullscreen.value)
	{
		if(SDL_SetWindowFullscreen(sMainWindow,(SDL_bool)cvFullscreen.value) < 0)
		{
			Con_Warning("Failed to set window mode!\n%s",SDL_GetError());

			// [16/7/2013] Reset the variable to the current value ~hogsy
			Cvar_SetValue(cvFullscreen.name,(float)Video.bFullscreen);
		}
		else
			Video.bFullscreen = (bool)cvFullscreen.value;
	}

	if(!cvFullscreen.value)
		// [15/7/2013] Center the window ~hogsy
		SDL_SetWindowPosition(sMainWindow,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);

    // [31/3/2014] Keep AntTweakBar informed of our windows size :) ~hogsy
    TwWindowSize(Video.iWidth,Video.iHeight);

	// [15/7/2013] Update console size ~hogsy
	vid.conwidth	= Video.iWidth & 0xFFFFFFF8;
	vid.conheight	= vid.conwidth*Video.iHeight/Video.iWidth;
}

/**/

static bool sbVideoCleanup      = false,
            sbVideoIgnoreDepth  = true;

/*	Bind our current texture.
*/
void Video_SetTexture(gltexture_t *gTexture)
{
	if(!gTexture)
		gTexture = notexture;
	// [29/8/2012] Same as the last binded texture? ~hogsy
	else if(gTexture->texnum == Video.iCurrentTexture)
		return;

	Video.iCurrentTexture = gTexture->texnum;

	gTexture->visframe = r_framecount;

	glBindTexture(GL_TEXTURE_2D,gTexture->texnum);

	if(cvVideoDebug.value)
        Con_Printf("Video: Bound texture (%s) (%i)\n",gTexture->name,Video.uiActiveUnit);
}

/*  Changes the active blending mode.
    This should be used in conjunction with the VIDEO_BLEND mode.
*/
void Video_SetBlend(VideoBlend_t voBlendMode,int iDepthType)
{
    if(iDepthType != VIDEO_DEPTH_IGNORE)
    {
        glDepthMask(iDepthType);

        sbVideoIgnoreDepth = false;
    }

    if(voBlendMode != VIDEO_BLEND_IGNORE)
    {
        switch(voBlendMode)
        {
        case VIDEO_BLEND_ZERO:
            glBlendFunc(GL_ZERO,GL_ZERO);
            break;
        case VIDEO_BLEND_ONE:
            glBlendFunc(GL_ONE,GL_ONE);
            break;
        case VIDEO_BLEND_TWO:
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            break;
        case VIDEO_BLEND_THREE:
            glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
            break;
        default:
            Sys_Error("Unknown blend mode! (%i)\n",voBlendMode);
        }
    }
}

/*
    Multitexturing Management
*/

bool bMultitextureEnabled = false;

void Video_SelectTexture(unsigned int uiTarget)
{
	unsigned int uiUnit;

	if(uiTarget == Video.uiActiveUnit)
        return;

    switch(uiTarget)
    {
    case 0:
        uiUnit = GL_TEXTURE0;
        break;
    case 1:
        uiUnit = GL_TEXTURE1;
        break;
    default:
        Sys_Error("Unknown texture unit! (%i)\n",uiTarget);
    }

	glActiveTextureARB(uiUnit);

	Video.uiActiveUnit = uiTarget;

	if(cvVideoDebug.value)
        Con_Printf("Video: Texture Unit %i\n",Video.uiActiveUnit);
}

void Video_DisableMultitexture(void)
{
//  Video_DisableCapabilities(VIDEO_TEXTURE_2D);
    Video_SelectTexture(0);
}

void Video_EnableMultitexture(void)
{
    Video_SelectTexture(1);
    Video_EnableCapabilities(VIDEO_TEXTURE_2D);
}

/*
    Drawing
*/

/*	Draw terrain.
	Unfinished
*/
void Video_DrawTerrain(VideoObject_t *voTerrain,vec3_t vScale)
{
}

/*  Draw a simple rectangle.
*/
void Video_DrawFill(VideoObject_t *voFill)
{
	Video_DrawObject(voFill,VIDEO_PRIMITIVE_TRIANGLE_FAN,4,false);
}

/*	Draw 3D object.
*/
void Video_DrawObject(
	VideoObject_t		    *voObject,
	VideoPrimitive_t        vpPrimitiveType,
	unsigned            int	uiTriangles,
	bool				    bMultiTexture)
{
	int i;

	if(!voObject)
        Sys_Error("Invalid video object!\n");
    else if(!uiTriangles)
        Sys_Error("Invalid number of triangles for video object!\n");

    // [8/5/2014] Ignore any additional changes ~hogsy
    bVideoIgnoreCapabilities = true;

	if(r_showtris.value > 0)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

		GL_PolygonOffset(OFFSET_SHOWTRIS);

		// [29/10/2013] Disable crap for tris view ~hogsy
		Video_DisableCapabilities(VIDEO_TEXTURE_2D/*|VIDEO_BLEND*/);
	}

    // [11/3/2014] Support different primitive types; TODO: Move into its own function? ~hogsy
    {
        GLenum gPrimitive;

        switch(vpPrimitiveType)
        {
        case VIDEO_PRIMITIVE_TRIANGLES:
            gPrimitive = GL_TRIANGLES;
            break;
        case VIDEO_PRIMITIVE_TRIANGLE_FAN:
            gPrimitive = GL_TRIANGLE_FAN;
            break;
        default:
            // [16/3/2014] Anything else and give us an error ~hogsy
            Sys_Error("Unknown object primitive type! (%i)\n",vpPrimitiveType);
        }

        glBegin(gPrimitive);
	}

	for(i = 0; i < uiTriangles; i++)
	{
		if(!r_showtris.value)
		{
			if(bMultiTexture)
			{
				glMultiTexCoord2fv(GL_TEXTURE0,voObject[i].vTextureCoord[0]);
				glMultiTexCoord2fv(GL_TEXTURE1,voObject[i].vTextureCoord[1]);
			}
			else
				glTexCoord2fv(voObject[i].vTextureCoord[0]);

			glColor4fv(voObject[i].vColour);
		}

		glVertex3fv(voObject[i].vVertex);
	}

	glEnd();

	if(r_showtris.value > 0)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		GL_PolygonOffset(OFFSET_NONE);

		Video_EnableCapabilities(VIDEO_TEXTURE_2D);
	}

	bVideoIgnoreCapabilities = false;

	// [20/10/2013] Reset colour value ~hogsy
	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

/*
	Capabilities management
*/

typedef struct
{
	unsigned	int	    uiFirst,
                        uiSecond;

	const       char    *ccIdentifier;
} VideoCapabilities_t;

VideoCapabilities_t	vcCapabilityList[]=
{
	{	VIDEO_ALPHA_TEST,		GL_ALPHA_TEST,      "ALPHA_TEST"    },
	{	VIDEO_BLEND,			GL_BLEND,           "BLEND"			},
	{	VIDEO_DEPTH_TEST,		GL_DEPTH_TEST,      "DEPTH_TEST"	},
	{	VIDEO_TEXTURE_2D,		GL_TEXTURE_2D,      "TEXTURE_2D"	},
	{	VIDEO_TEXTURE_GEN_S,	GL_TEXTURE_GEN_S,   "TEXTURE_GEN_S"	},
	{	VIDEO_TEXTURE_GEN_T,	GL_TEXTURE_GEN_T,   "TEXTURE_GEN_T"	},
	{	VIDEO_CULL_FACE,		GL_CULL_FACE,       "CULL_FACE"		},
	{	VIDEO_STENCIL_TEST,		GL_STENCIL_TEST,    "STENCIL_TEST"	},
	{	VIDEO_NORMALIZE,		GL_NORMALIZE,		"NORMALIZE"     },

	{   0   }
};

static unsigned int	iSavedCapabilites[4][2];

/*	Set rendering capabilities for current draw.
	Cleared using Video_DisableCapabilities.
*/
void Video_EnableCapabilities(unsigned int iCapabilities)
{
	int	i;

	if(!iCapabilities)
        return;

	for(i = 0; i < sizeof(vcCapabilityList); i++)
	{
        if(!vcCapabilityList[i].uiFirst)
            break;

		if(iCapabilities & vcCapabilityList[i].uiFirst)
		{
            if(cvVideoDebug.value)
                Con_Printf("Video: Enabling %s\n",vcCapabilityList[i].ccIdentifier);

            if(!sbVideoCleanup && !bVideoIgnoreCapabilities)
                // [24/2/2014] Collect up a list of the new capabilities we set ~hogsy
                iSavedCapabilites[Video.uiActiveUnit][VIDEO_STATE_ENABLE] |= vcCapabilityList[i].uiFirst;

			glEnable(vcCapabilityList[i].uiSecond);
		}
    }
}

/*	Disables specified capabilities for the current draw.
*/
void Video_DisableCapabilities(unsigned int iCapabilities)
{
	int	i;

	if(!iCapabilities)
        return;

	for(i = 0; i < sizeof(vcCapabilityList); i++)
	{
        if(!vcCapabilityList[i].uiFirst)
            break;

		if(iCapabilities & vcCapabilityList[i].uiFirst)
		{
            if(cvVideoDebug.value)
                Con_Printf("Video: Disabling %s\n",vcCapabilityList[i].ccIdentifier);

            if(!sbVideoCleanup && !bVideoIgnoreCapabilities)
                // [24/2/2014] Collect up a list of the new capabilities we disabled ~hogsy
                iSavedCapabilites[Video.uiActiveUnit][VIDEO_STATE_DISABLE] |= vcCapabilityList[i].uiFirst;

			glDisable(vcCapabilityList[i].uiSecond);
		}
    }
}

/*	Resets our capabilities.
	Give an argument of true to only clear the list, not the capabilities.
	Also resets active blending mode.
*/
void Video_ResetCapabilities(bool bClearActive)
{
    if(cvVideoDebug.value)
        Con_Printf("Video: Resetting capabilities...\n");

	if(bClearActive)
	{
        if(cvVideoDebug.value)
            Con_Printf("Video: Clearing active capabilities...\n");

        sbVideoCleanup = true;

        // [7/5/2014] Disable/Enable old states by unit... Ugh ~hogsy
        {
            Video_SelectTexture(1);

            Video_DisableCapabilities(iSavedCapabilites[1][0]);
            Video_EnableCapabilities(iSavedCapabilites[1][1]);

            Video_SelectTexture(0);

            Video_DisableCapabilities(iSavedCapabilites[0][0]);
            Video_EnableCapabilities(iSavedCapabilites[0][1]);
        }

        if(sbVideoIgnoreDepth)
            Video_SetBlend(VIDEO_BLEND_TWO,VIDEO_DEPTH_IGNORE);
        else
            // [12/3/2014] Reset our blend mode too. ~hogsy
            Video_SetBlend(VIDEO_BLEND_TWO,VIDEO_DEPTH_TRUE);

        // [28/3/2014] Set defau;ts ~hogsy
        sbVideoIgnoreDepth  = true;
		sbVideoCleanup      = false;

		if(cvVideoDebug.value)
            Con_Printf("Video: Finished clearing capabilities.\n");
	}

    // [7/5/2014] Clear out capability list ~hogsy
    iSavedCapabilites[1][0] =
    iSavedCapabilites[1][1] =
    iSavedCapabilites[0][0] =
    iSavedCapabilites[0][1] = 0;
}

/*
    Shader Management
    Unfinished!
*/

/*  Simple whipped up function to demo shader processing
    then simple error management. Replace this...
*/
void Video_ProcessShader(int iType)
{
    int             iState;
    unsigned int    uiShader;

    if(iType == VIDEO_SHADER_FRAGMENT)
        uiShader = glCreateShader(GL_VERTEX_SHADER);
    else
        uiShader = glCreateShader(GL_FRAGMENT_SHADER);

//    glShaderSource(uiShader,1,&uiShader,NULL);
    glCompileShader(uiShader);

    glGetShaderiv(uiShader,GL_COMPILE_STATUS,&iState);
    if(!iState)
    {
        char cLog[512];

        glGetShaderInfoLog(uiShader,512,NULL,cLog);

        // [12/3/2014] Spit a log out to the console ~hogsy
        Con_Warning("Failed to compile shader!\n%s",cLog);
        return;
    }

//    glShaderSource()
}

/**/

/*	Main rendering loop.
	Unfinished
*/
void Video_Process(void)
{
#ifndef KATANA_VIDEO_NEXT	// Legacy renderer
    if(cvVideoDebug.value)
        Con_Printf("Video: Start of frame\n");

	SCR_UpdateScreen();

//	Menu->Draw();

    // [31/3/2014] Draw any ATB windows ~hogsy
	TwDraw();

	GL_EndRendering();

    if(cvVideoDebug.value)
    {
        Con_Printf("Video: End of frame\n");
        if(cvVideoDebug.value != 2)
            Cvar_SetValue("video_debug",0);
    }
#else	// New renderer
	static vec4_t vLight =
	{	0,	10.0f,	20.0f,	1.0f	};

	if(!Video.bInitialized)
		return;

	Video_ClearBuffer();

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);

	glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB,3,vLight);

#if 0	// start
	{
		GLenum gBuffers[]=
		{
			GL_COLOR_ATTACHMENT0_EXT,
			GL_COLOR_ATTACHMENT1_EXT,
			GL_COLOR_ATTACHMENT2_EXT
		};

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,gFBO);
		glPushAttrib(GL_VIEWPORT_BIT);

		Video_ClearBuffer();

		glActiveTextureARB(VIDEO_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glDrawBuffers(3,gBuffers);
	}
#endif

	// Do stuff here
	SCR_UpdateScreen();

#if 0	// end
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	glPopAttrib();
#endif

	// [13/7/2013] Draw any >screen< elements from either Game or Menu ~hogsy
	Game->Draw();
	Menu->Draw();

	SDL_GL_SwapWindow(sMainWindow);
#endif
}

void Video_Shutdown(void)
{
	if(!Video.bInitialized)
		return;

    TwTerminate();

	// [6/12/2012] Delete our context ~hogsy
	if(sMainContext)
		SDL_GL_DeleteContext(sMainContext);

	// [6/12/2012] Destroy our window ~hogsy
	if(sMainWindow)
		SDL_DestroyWindow(sMainWindow);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	Video.bInitialized = false;
}
