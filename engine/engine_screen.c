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

#include "platform_filesystem.h"

#include "engine_menu.h"
#include "engine_editor.h"
#include "KatGL.h"

#include "engine_video.h"
#include "engine_input.h"

/*

background clear
rendering
turtle/net/ram icons
sbar
centerprint / slow centerprint
notify lines
intermission / finale overlay
loading plaque
console
menu

required background clears
required update regions


syncronous draw mode or async
One off screen buffer, with updates either copied or xblited
Need to double buffer?


async draw will require the refresh area to be cleared, because it will be
xblited, but sync draw can just ignore it.

sync
draw

CenterPrint ()
SlowPrint ()
Screen_Update ();
Con_Printf ();

net
turn off messages option

the refresh is allways rendered, unless the console is full screen


console is:
	notify lines
	half
	full


*/

int			glx, gly, glwidth, glheight;

float		scr_con_current;
float		scr_conlines;		// lines of console to display
float		oldscreensize, oldfov, oldsbarscale, oldsbaralpha; //johnfitz -- added oldsbarscale and oldsbaralpha

cvar_t	scr_menuscale		= {"scr_menuscale", "1", TRUE};
cvar_t	scr_sbarscale		= {"scr_sbarscale", "1", TRUE};
cvar_t	scr_sbaralpha		= {"scr_sbaralpha", "1", TRUE};
cvar_t	scr_conwidth		= {"scr_conwidth", "0", TRUE};
cvar_t	scr_conscale		= {"scr_conscale", "1", true};
cvar_t	scr_crosshairscale	= {"scr_crosshairscale", "1", TRUE};
cvar_t	scr_showfps			= {"scr_showfps", "0"};
cvar_t	scr_fps_rate		= { "scr_fps_rate","0.37",TRUE};
cvar_t	scr_clock			= { "scr_clock", "0"};
cvar_t	scr_viewsize		= { "viewsize","120", true};
cvar_t	scr_fov				= {	"fov",	"90",	true	};	// 10 - 170
cvar_t	scr_conspeed		= {"scr_conspeed","300"};
cvar_t	scr_centertime		= {"scr_centertime","2"};
cvar_t	scr_showram			= {"showram","1"};
cvar_t	scr_showturtle		= { "showturtle",   "0" };
cvar_t	scr_showpause		= { "showpause",    "1" };
cvar_t	scr_printspeed		= {"scr_printspeed","8"};

bool	bScreenInitialized;		// ready to draw

qpic_t	*scr_ram,*scr_net,*scr_turtle;

int			clearconsole;
int			sb_lines;

viddef_t	vid;				// global video state

vrect_t		scr_vrect;

bool	scr_disabled_for_loading,
		scr_drawloading;

float		scr_disabled_time;

int	scr_tileclear_updates = 0; //johnfitz

void SCR_ScreenShot_f (void);

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int			scr_center_lines;
int			scr_erase_lines;
int			scr_erase_center;

/*	Called for important messages that should stay in the center of the screen
	for a few moments
*/
void SCR_CenterPrint (char *str) //update centerprint data
{
	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime.value;
	scr_centertime_start = cl.time;

// count the number of lines for centering
	scr_center_lines = 1;
	while (*str)
	{
		if (*str == '\n')
			scr_center_lines++;
		str++;
	}
}

void SCR_DrawCenterString (void) //actually do the drawing
{
	char	*start;
	int		l,j,x,y,remaining;

	GL_SetCanvas (CANVAS_MENU); //johnfitz

// the finale prints the characters one at a time
	if (cl.intermission)
		remaining = scr_printspeed.value * (cl.time - scr_centertime_start);
	else
		remaining = 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = 200*0.35;	//johnfitz -- 320x200 coordinate system
	else
		y = 48;

	do
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (start[l] == '\n' || !start[l])
				break;
		x = (320 - l*8)/2;	//johnfitz -- 320x200 coordinate system
		for (j=0 ; j<l ; j++, x+=8)
		{
			Draw_Character (x, y, start[j]);	//johnfitz -- stretch overlays
			if (!remaining--)
				return;
		}

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

void SCR_CheckDrawCenterString (void)
{
	if (scr_center_lines > scr_erase_lines)
		scr_erase_lines = scr_center_lines;

	scr_centertime_off -= host_frametime;

	if (scr_centertime_off <= 0 && !cl.intermission)
		return;
	if (key_dest != key_game)
		return;
	if (cl.bIsPaused) //johnfitz -- don't show centerprint during a pause
		return;

	SCR_DrawCenterString ();
}

//=============================================================================

float CalcFovy (float fov_x, float width, float height)
{
	float   a, x;

	if (fov_x < 1 || fov_x > 179)
			Sys_Error ("Bad fov: %f", fov_x);

	x = width/tan(fov_x/360*pMath_PI);
	a = atan (height/x);
	a = a*360.0f/pMath_PI;
	return a;
}

/*	Must be called whenever vid changes
	Internal use only
*/
static void SCR_CalcRefdef (void)
{
	float		size, scale; //johnfitz -- scale

	vid.bRecalcRefDef = false;

	scr_tileclear_updates = 0; //johnfitz

// bound viewsize
	if(scr_viewsize.value < 30)
		Cvar_Set("viewsize","30");
	else if(scr_viewsize.value > 120)
		Cvar_Set("viewsize","120");

	// Bound fov
	if(scr_fov.value < 10)
		Cvar_Set ("fov","10");
	else if(scr_fov.value > 170)
		Cvar_Set ("fov","170");

	//johnfitz -- rewrote this section
	size = scr_viewsize.value;
	scale = CLAMP (1.0, scr_sbarscale.value, (float)glwidth / 320.0);

	if (size >= 120 || cl.intermission || scr_sbaralpha.value < 1) //johnfitz -- scr_sbaralpha.value
		sb_lines = 0;
	else if (size >= 110)
		sb_lines = 24*scale;
	else
		sb_lines = 48*scale;

	size = Math_Min(scr_viewsize.value,100)/100;
	//johnfitz

	//johnfitz -- rewrote this section
	r_refdef.vrect.width = Math_Max(glwidth * size, 96); //no smaller than 96, for icons
	r_refdef.vrect.height = Math_Min(glheight * size, glheight - sb_lines); //make room for sbar
	r_refdef.vrect.x = (glwidth - r_refdef.vrect.width)/2;
	r_refdef.vrect.y = (glheight - sb_lines - r_refdef.vrect.height)/2;
	//johnfitz

	r_refdef.fov_x = scr_fov.value;
	r_refdef.fov_y = CalcFovy (r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height);

	scr_vrect = r_refdef.vrect;
}

/*	Keybinding command
*/
void SCR_SizeUp_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize.value+10);
	vid.bRecalcRefDef = true;
}

/*	Keybinding command
*/
void SCR_SizeDown_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize.value-10);
	vid.bRecalcRefDef = true;
}

/*	Called when scr_conwidth or scr_conscale changes
*/
void SCR_Conwidth_f (void)
{
	vid.conwidth = (scr_conwidth.value > 0) ? (int)scr_conwidth.value : (scr_conscale.value > 0) ? (int)(Video.iWidth/scr_conscale.value) : Video.iWidth;
	vid.conwidth = CLAMP (320, vid.conwidth,Video.iWidth);
	vid.conwidth &= 0xFFFFFFF8;
	vid.conheight = vid.conwidth *Video.iHeight/Video.iWidth;
}

void Screen_ResetFPS(void);

void SCR_Init (void)
{
	Cvar_RegisterVariable(&scr_menuscale,NULL);
	Cvar_RegisterVariable(&scr_sbarscale,NULL);
	Cvar_RegisterVariable(&scr_sbaralpha,NULL);
	Cvar_RegisterVariable(&scr_conwidth,&SCR_Conwidth_f);
	Cvar_RegisterVariable(&scr_conscale,&SCR_Conwidth_f);
	Cvar_RegisterVariable(&scr_crosshairscale,NULL);
	Cvar_RegisterVariable(&scr_showfps,&Screen_ResetFPS);
	Cvar_RegisterVariable(&scr_fps_rate,NULL);
	Cvar_RegisterVariable(&scr_clock,NULL);
	Cvar_RegisterVariable(&scr_fov,NULL);
	Cvar_RegisterVariable(&scr_viewsize,NULL);
	Cvar_RegisterVariable(&scr_conspeed,NULL);
	Cvar_RegisterVariable(&scr_showram,NULL);
	Cvar_RegisterVariable(&scr_showturtle,NULL);
	Cvar_RegisterVariable(&scr_showpause,NULL);
	Cvar_RegisterVariable(&scr_centertime,NULL);
	Cvar_RegisterVariable(&scr_printspeed,NULL);

	Cmd_AddCommand("screenshot",SCR_ScreenShot_f);
	Cmd_AddCommand("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand("sizedown",SCR_SizeDown_f);

	bScreenInitialized = true;
}

/*
	Frame-rate counter
*/

double	dHighestFPS	= 0,
		dLowestFPS	= 10000.0;

/*	Called before displaying the fps.
*/
void Screen_ResetFPS(void)
{
	dHighestFPS	= 0;
	dLowestFPS	= 10000.0;
}

void Screen_DrawFPS(void)
{
	static double	oldtime = 0,fps = 0;
	static int		oldframecount = 0;
	double			time;
	char			str[128];
	int				x,y,frames;

	time	= realtime-oldtime;
	frames	= r_framecount-oldframecount;

	if(time < 0 || frames < 0)
	{
		oldtime			= realtime;
		oldframecount	= r_framecount;
		return;
	}

	// [14/6/2012] Allow us to set our own update rate ~hogsy
	if(time > scr_fps_rate.value)
	{
		fps				= frames/time;
		oldtime			= realtime;
		oldframecount	= r_framecount;
	}

	if(scr_showfps.value) //draw it
	{
		// [9/10/2012] Set the highest and lowest counts we get ~hogsy
		if(fps > dHighestFPS)
			dHighestFPS = fps;

		if((fps < dLowestFPS) && fps >= 1)
			dLowestFPS = fps;

		sprintf(str,"%4.0f FPS (%1.0f/%1.0f)",
			fps,dHighestFPS,dLowestFPS);

		x = 320-(strlen(str)<<3);
		y = 200-8;

		if (scr_clock.value)
			y -= 8; //make room for clock

		GL_SetCanvas(CANVAS_BOTTOMRIGHT);

		R_DrawString(x,y,str);

		scr_tileclear_updates = 0;
	}
}

/**/

void SCR_DrawClock (void)
{
	char	str[9];

	if (scr_clock.value == 1)
	{
		int minutes, seconds;

		minutes = cl.time / 60;
		seconds = ((int)cl.time)%60;

		sprintf (str,"%i:%i%i", minutes, seconds/10, seconds%10);
	}
#ifdef _WIN32
	else if (scr_clock.value == 2)
	{
		int hours, minutes, seconds;
		SYSTEMTIME systime;
		char m[3] = "AM";

		GetLocalTime(&systime);
		hours	= systime.wHour;
		minutes = systime.wMinute;
		seconds = systime.wSecond;

		if (hours > 12)
			strcpy(m, "PM");
		hours = hours%12;
		if (hours == 0)
			hours = 12;

		sprintf (str,"%i:%i%i:%i%i %s", hours, minutes/10, minutes%10, seconds/10, seconds%10, m);
	}
	else if (scr_clock.value == 3)
	{
		int hours, minutes, seconds;
		SYSTEMTIME systime;

		GetLocalTime(&systime);

		hours	= systime.wHour;
		minutes = systime.wMinute;
		seconds = systime.wSecond;

		sprintf (str,"%i:%i%i:%i%i", hours%12, minutes/10, minutes%10, seconds/10, seconds%10);
	}
#endif
	else
		return;

	//draw it
	GL_SetCanvas (CANVAS_BOTTOMRIGHT);
	R_DrawString(320 - (strlen(str)<<3), 200 - 8, str);

	scr_tileclear_updates = 0;
}

void SCR_DrawDevStats (void)
{
	char	str[40];
	int		y = 25-9; //9=number of lines to print
	int		x = 0; //margin

	if (!devstats.value)
		return;

	GL_SetCanvas (CANVAS_BOTTOMLEFT);

	Draw_Fill(x,y*8,152,72,0,0,0,0.5f); //dark rectangle

	sprintf (str, "devstats |Curr Peak");
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "---------+---------");
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "Edicts   |%4i %4i", dev_stats.edicts, dev_peakstats.edicts);
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "Packet   |%4i %4i", dev_stats.packetsize, dev_peakstats.packetsize);
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "Visedicts|%4i %4i", dev_stats.visedicts, dev_peakstats.visedicts);
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "Efrags   |%4i %4i", dev_stats.efrags, dev_peakstats.efrags);
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "Dlights  |%4i %4i", dev_stats.dlights, dev_peakstats.dlights);
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "Beams    |%4i %4i", dev_stats.beams, dev_peakstats.beams);
	R_DrawString(x,(y++)*8-x,str);

	sprintf (str, "Tempents |%4i %4i", dev_stats.tempents, dev_peakstats.tempents);
	R_DrawString(x,(y++)*8-x,str);
}

void SCR_DrawTurtle (void)
{
	static int	count;

	if (!scr_showturtle.value)
		return;

	if (host_frametime < 0.1)
	{
		count = 0;
		return;
	}

	count++;
	if (count < 3)
		return;

	GL_SetCanvas (CANVAS_DEFAULT); //johnfitz

	Draw_Pic (scr_vrect.x, scr_vrect.y, scr_turtle);
}

void SCR_DrawNet(void)
{
	// [24/7/2012] Don't display when we're not even connected ~hogsy
	if(	cl.maxclients <= 1	||
		cls.demoplayback || realtime-cl.last_received_message < 0.3f)
		return;

	GL_SetCanvas(CANVAS_DEFAULT); //johnfitz

	Draw_ExternPic("textures/interface/net",1.0f,scr_vrect.x+64,scr_vrect.y,64,64);
}

void SCR_DrawPause (void)
{
	if(!cl.bIsPaused || !scr_showpause.value)
		return;

	scr_tileclear_updates = 0; //johnfitz
}

//=============================================================================

void SCR_SetUpToDrawConsole (void)
{
	//johnfitz -- let's hack away the problem of slow console when host_timescale is <0
	extern cvar_t host_timescale;
	float timescale;
	//johnfitz

	Con_CheckResize();

	if(scr_drawloading)
		return;		// never a console with loading plaque

	// Decide on the height of the console
	con_forcedup = !cl.worldmodel || cls.signon != SIGNONS;

	if(con_forcedup)
	{
		scr_conlines = glheight; //full screen //johnfitz -- glheight instead of vid.height
		scr_con_current = scr_conlines;
	}
	else if (key_dest == key_console)
		scr_conlines = glheight/2; //half screen //johnfitz -- glheight instead of vid.height
	else
		scr_conlines = 0; //none visible

	timescale = (host_timescale.value > 0) ? host_timescale.value : 1; //johnfitz -- timescale

	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed.value*host_frametime/timescale; //johnfitz -- timescale
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;
	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed.value*host_frametime/timescale; //johnfitz -- timescale
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

	if (!con_forcedup && scr_con_current)
		scr_tileclear_updates = 0; //johnfitz
}

void SCR_DrawConsole (void)
{
	if (scr_con_current)
	{
		Con_DrawConsole (scr_con_current,true);
		clearconsole = 0;
	}
	else
	{
		if (key_dest == key_game || key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}

/*
==============================================================================

						SCREEN SHOTS

==============================================================================
*/

void SCR_ScreenShot_f (void)
{
	byte	*buffer;
	char	tganame[32],checkname[MAX_OSPATH];
	int		i;

	if(!pFileSystem_CreateDirectory(va(PATH_SCREENSHOTS,com_gamedir)))
		Sys_Error("Failed to create directory!\n%s",pError_Get());

	// find a file name to save it to
	for (i = 0; i < 10000; i++)
	{
		sprintf(tganame,"screenshots/%04i.tga",i);
		sprintf(checkname,"%s/%s",com_gamedir,tganame);
		if(Sys_FileTime(checkname) == -1)
			break;	// file doesn't exist
	}

	if(i >= 10000)
	{
		Con_Printf ("SCR_ScreenShot_f: Couldn't find an unused filename\n");
		return;
	}

	//get data
	buffer = (byte*)malloc(glwidth*glheight*3);
	glReadPixels(glx,gly,glwidth,glheight,GL_RGB,GL_UNSIGNED_BYTE,buffer);

	// now write the file
	if(Image_WriteTGA(tganame,buffer,glwidth,glheight,24,false))
		Con_Printf("Wrote %s\n",tganame);
	else
		Con_Warning("Failed to write %s\n",tganame);

	free (buffer);
}

void SCR_BeginLoadingPlaque (void)
{
	S_StopAllSounds();

	if(cls.state != ca_connected || cls.signon != SIGNONS)
		return;

	// Redraw with no console and the loading plaque
	Con_ClearNotify ();

	scr_centertime_off	= 0;
	scr_con_current		= 0;
	scr_drawloading		= true;

	Menu->AddState(MENU_STATE_LOADING);

	Video_Process();

	Menu->RemoveState(MENU_STATE_LOADING);

	scr_drawloading				= false;
	scr_disabled_for_loading	= true;
	scr_disabled_time			= realtime;
}

void SCR_EndLoadingPlaque (void)
{
	scr_disabled_for_loading = false;
	Con_ClearNotify ();
}

//=============================================================================

char	*scr_notifystring;
bool	bDrawDialog;

void SCR_DrawNotifyString (void)
{
	char	*start;
	int		l,j,x,y;

	GL_SetCanvas (CANVAS_MENU); //johnfitz

	start = scr_notifystring;

	y = 200*0.35; //johnfitz -- stretched overlays

	do
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (start[l] == '\n' || !start[l])
				break;
		x = (320 - l*8)/2; //johnfitz -- stretched overlays
		for (j=0 ; j<l ; j++, x+=8)
			Draw_Character (x, y, start[j]);

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

/*	Displays a text string in the center of the screen and waits for a Y or N
	keypress.
*/
int SCR_ModalMessage (char *text, float timeout) //johnfitz -- timeout
{
	double time1, time2; //johnfitz -- timeout

	if (cls.state == ca_dedicated)
		return true;

	scr_notifystring = text;

	// draw a fresh screen
	bDrawDialog = true;
	Video_Process();
	bDrawDialog = false;

	S_ClearBuffer ();		// so dma doesn't loop current sound

	time1 = System_DoubleTime () + timeout; //johnfitz -- timeout
	time2 = 0.0f; //johnfitz -- timeout

	do
	{
		key_count = -1;		// wait for a key down and up

		Input_Process();

		if (timeout)
			time2 = System_DoubleTime (); //johnfitz -- zero timeout means wait forever.
	} while(	key_lastpress != 'y'		&&
				key_lastpress != 'n'		&&
				key_lastpress != K_ESCAPE	&&
				time2 <= time1);

//	SCR_UpdateScreen (); //johnfitz -- commented out

	//johnfitz -- timeout
	if (time2 > time1)
		return false;
	//johnfitz

	return key_lastpress == 'y';
}

void SCR_TileClear (void)
{
	if (r_refdef.vrect.x > 0)
	{
		// left
		Draw_TileClear (0,
						0,
						r_refdef.vrect.x,
						glheight - sb_lines);
		// right
		Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width,
						0,
						glwidth - r_refdef.vrect.x - r_refdef.vrect.width,
						glheight - sb_lines);
	}

	if (r_refdef.vrect.y > 0)
	{
		// top
		Draw_TileClear (r_refdef.vrect.x,
						0,
						r_refdef.vrect.width,
						r_refdef.vrect.y);
		// bottom
		Draw_TileClear (r_refdef.vrect.x,
						r_refdef.vrect.y + r_refdef.vrect.height,
						r_refdef.vrect.width,
						glheight - r_refdef.vrect.y - r_refdef.vrect.height - sb_lines);
	}
}

void V_UpdateBlend(void);	            // [24/2/2014] See engine_view.c ~hogsy

/*	This is called every frame, and can also be called explicitly to flush
	text to the screen.

	WARNING: be very careful calling this from elsewhere, because the refresh
	needs almost the entire 256k of stack space!
*/
void SCR_UpdateScreen (void)
{
	if(scr_disabled_for_loading)
	{
		if(realtime-scr_disabled_time > 60.0f)
		{
			scr_disabled_for_loading = false;

			Con_Printf ("load failed.\n");
		}
		else
			return;
	}

	if(!bScreenInitialized || !bConsoleInitialized)
		return;				// not initialized yet

	GL_BeginRendering(&glx,&gly,&glwidth,&glheight);

	// determine size of refresh window
	if(oldfov != scr_fov.value)
	{
		oldfov = scr_fov.value;
		vid.bRecalcRefDef = true;
	}

	if (oldscreensize != scr_viewsize.value)
	{
		oldscreensize = scr_viewsize.value;
		vid.bRecalcRefDef = true;
	}

	//johnfitz -- added oldsbarscale and oldsbaralpha
	if (oldsbarscale != scr_sbarscale.value)
	{
		oldsbarscale = scr_sbarscale.value;
		vid.bRecalcRefDef = true;
	}

	if (oldsbaralpha != scr_sbaralpha.value)
	{
		oldsbaralpha = scr_sbaralpha.value;
		vid.bRecalcRefDef = true;
	}
	//johnfitz

	if (vid.bRecalcRefDef)
		SCR_CalcRefdef ();

	// do 3D refresh drawing, and then update the screen
	SCR_SetUpToDrawConsole ();

	V_RenderView ();

	Draw_ResetCanvas();

	//FIXME: only call this when needed
	SCR_TileClear();

	if(bDrawDialog) //new game confirm
	{
		Menu->Draw();

		Draw_FadeScreen();
		SCR_DrawNotifyString();
	}
	else if(scr_drawloading) //loading
		Menu->Draw();
	else if(cl.intermission == 1 && key_dest == key_game) //end of level
	{}
	else if(cl.intermission == 2 && key_dest == key_game) //end of episode
		SCR_CheckDrawCenterString ();
	else
	{
		Menu->Draw();

		SCR_DrawNet();
		SCR_DrawTurtle();
		SCR_DrawPause();
		SCR_CheckDrawCenterString ();
		SCR_DrawDevStats(); //johnfitz
		Screen_DrawFPS(); //johnfitz
		SCR_DrawClock(); //johnfitz
		SCR_DrawConsole();
	}

	V_UpdateBlend(); //johnfitz -- V_UpdatePalette cleaned up and renamed
}
