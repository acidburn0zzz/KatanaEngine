/*	Copyright (C) 2011-2014 OldTimes Software
*/

#include "quakedef.h"

/*
	Font Management
*/

#include "engine_video.h"

#ifdef _WIN32
#include "SDL_ttf.h"
#else
#include <SDL/SDL_ttf.h>
#endif

typedef struct
{
	TTF_Font	*tfFont;

	gltexture_t	*gFontTexture;

	char		*cName;
} Font_t;

Font_t	fFonts[32];

void Font_Load(const char *ccName,int iSize);

void Font_Initialize(void)
{
	if(!TTF_Init())
		Sys_Error("Failed to initialize SDL_ttf!\n%s\n",TTF_GetError());

	// Load a basic set of fonts...
	Font_Load("Xolonium-Regular",24);
	Font_Load("Xolonium-Bold",24);
}

/*	Loads the given font for later usage.
	Should be used during initialization OR during game-load.
*/
void Font_Load(const char *ccName,int iSize)
{
	char		cPath[MAX_OSPATH];
	int			i;

	// Try the engine directory first, yuck
	sprintf(cPath,"./engine/fonts/%s",ccName);

	for(i = 0; i < sizeof(fFonts); i++)
		if(!strcmp(fFonts[i].cName,ccName))
			return;
		else if(!fFonts[i].tfFont)
		{
			fFonts[i].tfFont = TTF_OpenFont(ccName,iSize);
			if(!fFonts[i].tfFont)
			{
				// Try again via the game path instead...
				sprintf(cPath,"%s/fonts/%s",host_parms.basedir,ccName);
				fFonts[i].tfFont = TTF_OpenFont(ccName,iSize);
				if(!fFonts[i].tfFont)
				{
					Con_Warning("Failed to load font! (%s)\n%s\n",ccName,TTF_GetError());
					return;
				}
			}
			break;
		}
}

/*	Basic routine for drawing fonts onto the screen.
*/
void Font_Draw(Font_t *fFont,const char *ccMessage,vec3_t vPos,vec3_t vColour)
{
	SDL_Colour		sColour = {1,1,1};
	SDL_Surface		*sFontSurface;
	VideoObject_t	voFont[4];

	sFontSurface = TTF_RenderText_Blended(fFont->tfFont,ccMessage,sColour);

	/*	todo:
			need to reserve bind slot.
			need to have a method of grabbing fonts without involving SDL_ttf library...
				Font_t struct contains ttf stuff, Ugh
	*/

	voFont[0].vTextureCoord[0][0]	=
	voFont[0].vTextureCoord[0][1]	=	0;
	voFont[1].vTextureCoord[0][0]	=	1;
	voFont[1].vTextureCoord[0][1]	=	0;
	voFont[2].vTextureCoord[0][0]	=	1;
	voFont[2].vTextureCoord[0][1]	=	1;
	voFont[3].vTextureCoord[0][0]	=	0;
	voFont[3].vTextureCoord[0][1]	=	1;

	Math_VectorCopy(vPos,voFont[0].vVertex);
	Math_VectorCopy(vPos,voFont[1].vVertex);
	Math_VectorCopy(vPos,voFont[2].vVertex);
	Math_VectorCopy(vPos,voFont[3].vVertex);

	voFont[1].vVertex[0]	+= sFontSurface->w;
	voFont[2].vVertex[0]	+= sFontSurface->w;
	voFont[2].vVertex[1]	+= sFontSurface->h;
	voFont[3].vVertex[1]	+= sFontSurface->h;

	Video_DrawFill(voFont);
}

void Font_Shutdown(void)
{
	int i;

	for(i = 0; i < sizeof(fFonts); i++)
		if(fFonts[i].tfFont)
			TTF_CloseFont(fFonts[i].tfFont);
}
