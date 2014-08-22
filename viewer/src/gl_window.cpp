#include <mx/mx.h>
#include <mx/mxMessageBox.h>
#include <mx/mxTga.h>
#include <mx/mxPcx.h>
#include <mx/gl.h>

#include <GL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "gl_model.h"
#include "gl_window.h"

#ifdef WIN32
char modelFile[256] = "";
char modelTexFile[256] = "";
char overlayFile[256] = "";
char overlayTexFile[256] = "";
char backgroundTexFile[256] = "";
char waterTexFile[256] = "";
#endif
float fps;

bool bFilterTextures = true;

GlWindow::GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxGlWindow (parent, x, y, w, h, label, style)
{
	d_rotX = 0.0f;
	d_rotY = -90.0f;
	d_transX = d_transY = 0;
	d_transZ = 50;
	d_models[0] = 0;
	d_models[1] = 0;
	d_textureNames[0] = 0;
	d_textureNames[1] = 0;
	d_textureNames[2] = 0;
	d_textureNames[3] = 0;

	setFrameInfo (0, 0);
	setRenderMode (0);
	setFlag (F_WATER, false);
	setFlag (F_LIGHT, true);
	setFlag (F_GRID, true);
	setFlag (F_INTERPOLATE, true);
	setFlag (F_GLCOMMANDS, true);
	setFlag (F_PAUSE, false);
	setFlag (F_BACKGROUND, false);
	setFlag (F_DRAWTEXTURE, false);
	
	setPitch (50.0f);
	setFoV (75);
	setTexScale (50.0f);
	setTexAlpha (100.0f);
	setBGColor (0.1f, 0.1f, 0.1f);
	setFGColor (1.0f, 1.0f, 1.0f);
	setWFColor (1.0f, 1.0f, 1.0f);
	setLightColor (1.0f, 1.0f, 1.0f);
	setGridColor (0.4f, 0.4f, 0.4f);
	setBrightness (5);

//	loadTexture ("data/textures/#water1.tga", TEXTURE_WATER);
//	loadTexture ("data/textures/sky/skyart.tga", TEXTURE_BACKGROUND);

	glCullFace (GL_FRONT);

	mx::setIdleWindow (this);
}

GlWindow::~GlWindow ()
{
	mx::setIdleWindow (0);

	loadTexture (0, TEXTURE_MODEL);
	loadTexture (0, TEXTURE_OVERLAY);
	loadTexture (0, TEXTURE_BACKGROUND);
	loadTexture (0, TEXTURE_WATER);

	loadModel (0, 0);
	loadModel (0, 1);
}

int
GlWindow::handleEvent (mxEvent *event)
{
	static float oldrx = 0, oldry = 0, oldtz = 50, oldtx = 0, oldty = 0;
	static int oldx, oldy;

	switch (event->event)
	{
	case mxEvent::MouseDown:
		oldrx = d_rotX;
		oldry = d_rotY;
		oldtx = d_transX;
		oldty = d_transY;
		oldtz = d_transZ;
		oldx = event->x;
		oldy = event->y;
		break;
	case mxEvent::MouseDrag:
		if (event->buttons & mxEvent::MouseLeftButton)
		{
			if (event->modifiers & mxEvent::KeyShift)
			{
				d_transX = oldtx - (float) (event->x - oldx);
				d_transY = oldty + (float) (event->y - oldy);
			}
			else
			{
				d_rotX = oldrx + (float) (event->y - oldy);
				d_rotY = oldry + (float) (event->x - oldx);
			}
		}
		else if (event->buttons & mxEvent::MouseRightButton)
			d_transZ = oldtz + (float) (event->y - oldy);

		redraw ();
		break;
	case mxEvent::Idle:
	{
		static int timer = 0, lastTimer = 0;

		if (getFlag (F_PAUSE))
			return 0;

		lastTimer = timer;
		timer = mx::getTickCount ();

		float diff = (float) (timer - lastTimer);
		fps = 1/ (diff / 1000.0f);
		d_pol += diff / d_pitch;

		if (d_pol > 1.0f)
		{
			d_pol = 0.0f;
			d_currFrame++;
			d_currFrame2++;

			if (d_currFrame > d_endFrame)
				d_currFrame = d_startFrame;

			if (d_currFrame2 > d_endFrame)
				d_currFrame2 = d_startFrame;
		}

		redraw ();
	}
	break;
	}

	return 1;
}

void GlWindow::draw ()
{
	glClearColor (d_bgColor[0], d_bgColor[1], d_bgColor[2], 0.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport (0, 0, w2 (), h2 ());

	if (getFlag (F_DRAWTEXTURE) && d_textureNames[TEXTURE_MODEL])
	{
		float w = TextureWidth * textureScale; 
		float h = TextureHeight * textureScale;

		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0.0f, (float) w2 (), (float) h2 (), 0.0f, 1.0f, -1.0f);

		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
		glLoadIdentity ();

		glDisable (GL_LIGHTING);
		glDisable (GL_CULL_FACE);
		glDisable (GL_DEPTH_TEST);
		glEnable (GL_TEXTURE_2D);
		glColor4f (1.0f, 1.0f, 1.0f, 0.3f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

		float x = ((float) w2 () - w) / 2;
		float y = ((float) h2 () - w) / 2;

		glDisable (GL_TEXTURE_2D);
		glColor4f (0.6f, 0.6f, 0.6f, 1.0f);
		glRectf (x - 2, y - 2, x  + w + 2, y + h + 2);

		glEnable (GL_TEXTURE_2D);
		glColor4f (1.0f, 1.0f, 1.0f, 1.0f);


		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_MODEL]);

		glBegin (GL_TRIANGLE_STRIP);

			glTexCoord2f (0, 0);
			glVertex2f (x, y);

			glTexCoord2f (1, 0);
			glVertex2f (x + w, y);

			glTexCoord2f (0, 1);
			glVertex2f (x, y + h);

			glTexCoord2f (1, 1);
			glVertex2f (x + w, y + h);

			glEnd ();

			glPopMatrix ();

			glClear (GL_DEPTH_BUFFER_BIT);
			glBindTexture (GL_TEXTURE_2D, 0);
		return;
	}

	if (getFlag (F_BACKGROUND) && d_textureNames[TEXTURE_BACKGROUND])
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f);

		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
		glLoadIdentity ();

		glDisable (GL_LIGHTING);
		glDisable (GL_CULL_FACE);
		glDisable (GL_DEPTH_TEST);
		glEnable (GL_TEXTURE_2D);

		glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_BACKGROUND]);

		glBegin (GL_QUADS);

		glTexCoord2f (0, 0);
		glVertex2f (0, 0);

		glTexCoord2f (0, 1);
		glVertex2f (0, 1);

		glTexCoord2f (1, 1);
		glVertex2f (1, 1);

		glTexCoord2f (1, 0);
		glVertex2f (1, 0);

		glEnd ();

		glPopMatrix ();
	}
			glMatrixMode (GL_PROJECTION);
			glLoadIdentity ();
			gluPerspective (d_fov, (GLfloat) w () / (GLfloat) h (), 1.0f, 1024.0f);
		
			glMatrixMode (GL_MODELVIEW);
			glPushMatrix ();
			glLoadIdentity ();

	if (getFlag (F_LIGHT))
	{
		GLfloat lp[4] = { 0, 0, d_transZ, 1 };
		GLfloat lc[4] = { d_lightColor[0], d_lightColor[1], d_lightColor[2], 1.0f };
		GLfloat ls[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv (GL_LIGHT0, GL_POSITION, lp);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, lc);
		glLightfv (GL_LIGHT0, GL_SPECULAR, ls);
	}

			glPixelTransferf (GL_RED_SCALE, 1.0f + 5.0f * d_bias);
			glPixelTransferf (GL_GREEN_SCALE, 1.0f + 5.0f * d_bias);
			glPixelTransferf (GL_BLUE_SCALE, 1.0f + 5.0f * d_bias);

			glTranslatef (-d_transX, -d_transY, -d_transZ);

			glRotatef (d_rotX, 1, 0, 0);
			glRotatef (d_rotY, 0, 1, 0);


		GLfloat ms[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, ms);
		glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 80.0f);

	if (getFlag (F_LIGHT))
	{
		glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
	}
	else
	{
		glDisable (GL_LIGHTING);
		glDisable (GL_LIGHT0);
	}

	if (d_renderMode == RM_WIREFRAME)
	{
		GLfloat md[4] = { d_wfColor[0], d_wfColor[1], d_wfColor[2], 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
		glColor3f (d_wfColor[0], d_wfColor[1], d_wfColor[2]);

		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);

	}
	else if (d_renderMode == RM_FLATSHADED || d_renderMode == RM_SMOOTHSHADED)
	{
		glColor3f (d_fgColor[0], d_fgColor[1], d_fgColor[2]);
		GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);

		if (d_renderMode == RM_FLATSHADED)
			glShadeModel (GL_FLAT);
		else
			glShadeModel (GL_SMOOTH);
	}
	else if (d_renderMode == RM_TEXTURED || d_renderMode == RM_TEXTUREDFLAT)
	{
		glColor4f (1.0f, 1.0f, 1.0f, textureAlpha);
		GLfloat md[4] = { 1.0f, 1.0f, 1.0f, textureAlpha };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);

		if(textureAlpha < 1.0f)
		{
				glEnable (GL_BLEND);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);
		if (d_renderMode == RM_TEXTUREDFLAT)
			glShadeModel (GL_FLAT);
		else
			glShadeModel (GL_SMOOTH);
	}

	if (d_models[0] && !d_models[1])
	{
		if (!d_textureNames[TEXTURE_MODEL] && (d_renderMode == RM_TEXTURED || d_renderMode == RM_TEXTUREDFLAT) )
		{
			GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
			glDisable (GL_TEXTURE_2D);
		}

		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_MODEL]);
		if (d_currFrame < d_models[0]->header.numFrames &&
			d_currFrame2 < d_models[0]->header.numFrames)
			md2_drawModel (d_models[0], d_currFrame, d_currFrame2, d_pol);

		if (d_renderMode == RM_WIREFRAME )
		{
			GLfloat md6[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md6);

			glEnable (GL_POINT_SMOOTH);
			glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glPointSize(4);
			glColor3f (1.0f, 0.0f, 0.0f);
			glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
			glDisable (GL_TEXTURE_2D);
			glDisable (GL_CULL_FACE);
			glEnable (GL_DEPTH_TEST);
			md2_drawModel (d_models[0], d_currFrame, d_currFrame2, d_pol);
		}
	}

	if (d_models[1] && !d_models[0])
	{
		if (d_renderMode == RM_TEXTURED || d_renderMode == RM_TEXTUREDFLAT)
		{
			if (!d_textureNames[TEXTURE_OVERLAY])
			{
				GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
				glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
				glDisable (GL_TEXTURE_2D);				
			}
			else
			{
				GLfloat md[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
				glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
			}
		}
		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_OVERLAY]);
		if (d_currFrame < d_models[1]->header.numFrames &&
			d_currFrame2 < d_models[1]->header.numFrames)
			md2_drawModel (d_models[1], d_currFrame, d_currFrame2, d_pol);
	}

	if (d_models[1] && d_models[0])
	{
		if ((!d_textureNames[TEXTURE_MODEL] && !d_textureNames[TEXTURE_OVERLAY]) && (d_renderMode == RM_TEXTURED || d_renderMode == RM_TEXTUREDFLAT) )
		{
			GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
			glDisable (GL_TEXTURE_2D);
		}

		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_MODEL]);

		if (d_currFrame < d_models[0]->header.numFrames &&
			d_currFrame2 < d_models[0]->header.numFrames)
			md2_drawModel (d_models[0], d_currFrame, d_currFrame2, d_pol);

		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_OVERLAY]);

		if (d_currFrame < d_models[1]->header.numFrames &&
			d_currFrame2 < d_models[1]->header.numFrames)
			md2_drawModel (d_models[1], d_currFrame, d_currFrame2, d_pol);

		if (d_renderMode == RM_WIREFRAME )
		{
			GLfloat md6[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md6);

			glEnable (GL_POINT_SMOOTH);
			glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glPointSize(4);
			glColor3f (1.0f, 0.0f, 0.0f);
			glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
			glDisable (GL_TEXTURE_2D);
			glDisable (GL_CULL_FACE);
			glEnable (GL_DEPTH_TEST);
			md2_drawModel (d_models[0], d_currFrame, d_currFrame2, d_pol);
			md2_drawModel (d_models[1], d_currFrame, d_currFrame2, d_pol);
		}
	}

	if (getFlag (F_GRID))
	{
		glDisable (GL_LIGHTING);

		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f (d_gridColor[0], d_gridColor[1], d_gridColor[2], 1.0f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);

		float x		= -100.0f;
		float y		= -90.0f;

		glBegin (GL_QUADS);

		for (int t = 0; t < 200; t = t + 10)
		{
			for (int i = 0; i < 200; i = i + 10)
			{	
				glTexCoord2f (0.0f, 0.0f);
				glVertex3f (x + t, -24.0f, x + i);
				glTexCoord2f (1.0f, 0.0f);
				glVertex3f (y + t, -24.0f, x + i);
				glTexCoord2f (1.0f, 1.0f);
				glVertex3f (y + t, -24.0f, y + i);
				glTexCoord2f (0.0f, 1.0f);
				glVertex3f (x + t, -24.0f, y + i);
			}
		}

		glEnd ();
		glDisable (GL_BLEND);
	}

	if (getFlag (F_WATER) && d_textureNames[TEXTURE_WATER])
	{
		glDisable (GL_LIGHTING);
		glNormal3f (0, 1, 0);
		glDisable (GL_CULL_FACE);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_BLEND);
		glEnable (GL_DEPTH_TEST);
		glColor4f (1.0f, 1.0f, 1.0f, 0.3f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_WATER]);

		float x		= -200.0f;
		float y		= -180.0f;

		glBegin (GL_QUADS);

		for (int t = 0; t < 400; t = t + 20)
		{
		for (int i = 0; i < 400; i = i + 20)
		{	
			glTexCoord2f (0.0f, 0.0f);
			glVertex3f (x + t, 0.0f, x + i);
			glTexCoord2f (1.0f, 0.0f);
			glVertex3f (y + t, 0.0f, x + i);
			glTexCoord2f (1.0f, 1.0f);
			glVertex3f (y + t, 0.0f, y + i);
			glTexCoord2f (0.0f, 1.0f);
			glVertex3f (x + t, 0.0f, y + i);
		}
		}

		glEnd ();
		glDisable (GL_BLEND);
	}

	glPopMatrix ();
}

void GlWindow::DumpModelInfo(md2_model_t *model, const char *filename)
{
	char path[256];
	strncat (path, filename, strlen(filename) -  4);
	sprintf (path, "%s.txt", path);

	FILE *file = fopen (path, "wb");
	if (file)
	{
		fprintf (file, "magic:\t\t%d\n", model->header.magic);
		fprintf (file, "version:\t\t%d\n", model->header.version);
		fprintf (file, "skinWidth:\t\t%d\n", model->header.skinWidth);
		fprintf (file, "skinHeight:\t\t%d\n", model->header.skinHeight);
		fprintf (file, "frameSize:\t\t%d\n", model->header.frameSize);
		fprintf (file, "numSkins:\t\t%d\n", model->header.numSkins);
		fprintf (file, "numVertices:\t\t%d\n", model->header.numVertices);
		fprintf (file, "numTexCoords:\t\t%d\n", model->header.numTexCoords);
		fprintf (file, "numTriangles:\t\t%d\n", model->header.numTriangles);
		fprintf (file, "numGlCommands:\t\t%d\n", model->header.numGlCommands);
		fprintf (file, "numFrames:\t\t%d\n", model->header.numFrames);
		fprintf (file, "offsetSkins:\t\t%d\n", model->header.offsetSkins);
		fprintf (file, "offsetTexCoords:\t%d\n", model->header.offsetTexCoords);
		fprintf (file, "offsetTriangles:\t%d\n", model->header.offsetTriangles);
		fprintf (file, "offsetFrames:\t\t%d\n", model->header.offsetFrames);
		fprintf (file, "offsetGlCommands:\t%d\n", model->header.offsetGlCommands);
		fprintf (file, "offsetEnd:\t\t%d\n", model->header.offsetEnd);
		fclose (file);
	}
}

md2_model_t *GlWindow::loadModel (const char *filename, int pos)
{
	if (d_models[pos] != 0)
	{
		md2_freeModel (d_models[pos]);
		d_models[pos] = 0;
#ifdef WIN32
		if (pos == 0)
			strcpy (modelFile, "");
		else
			strcpy (overlayFile, "");
#endif
	}

	if (!filename || !strlen (filename))
		return 0;

	d_models[pos] = md2_readModel (filename);

#ifdef WIN32
	if (pos == 0)
		strcpy (modelFile, filename);
	else
		strcpy (overlayFile, filename);
#endif

	if(g_md2Viewer->bDumpModelInfo)
	{
		DumpModelInfo(d_models[pos], filename);
	}

	return d_models[pos];
}

#include "shared_png.h"

int GlWindow::loadTexture (const char *filename, int name)
{
	if (!filename || !strlen (filename))
	{
		if (d_textureNames[name])
			d_textureNames[name] = 0;

#ifdef WIN32
		if (name == TEXTURE_MODEL)
			strcpy (modelTexFile, "");
		else if (name == TEXTURE_OVERLAY)
			strcpy (overlayTexFile, "");
		else if (name == TEXTURE_BACKGROUND)
			strcpy (backgroundTexFile, "");
		else if (name == TEXTURE_WATER)
			strcpy (waterTexFile, "");
#endif
		return 0;
	}

	mxImage *image = 0;

	if(strstr (filename, ".tga"))
		image = mxTgaRead (filename);
	else if(strstr(filename,".png"))
	{
		// [24/2/2014] TODO: Png support... ~hogsy
		//lodepng_decode32_file((unsigned char**)image->data,(UINT32*)&image->width,(UINT32*)&image->height,filename);
	}
	else if(strstr (_strlwr ((char *) filename), ".pcx"))
	{
		mxImage *tmp = mxPcxRead (filename);
		if (tmp)
		{
			image = new mxImage ();
			if (image && image->create (tmp->width, tmp->height, 24))
			{
				byte *dataout = (byte *) image->data;
				byte *datain = (byte *) tmp->data;
				byte *palette = (byte *) tmp->palette;
				int ptr = 0;
				for (int y = 0; y < tmp->height; y++)
				{
					for (int x = 0; x < tmp->width; x++)
					{
						dataout[ptr++] = palette[datain[y * tmp->width + x] * 3 + 0];
						dataout[ptr++] = palette[datain[y * tmp->width + x] * 3 + 1];
						dataout[ptr++] = palette[datain[y * tmp->width + x] * 3 + 2];
					}
				}
			}
			else
			{
				if (image)
					delete image;
				image = 0;
			}
		}

		if (tmp)
			delete tmp;
	}

	if (image)
	{
#ifdef WIN32
		if (name == TEXTURE_MODEL)
			strcpy (modelTexFile, filename);
		else if (name == TEXTURE_OVERLAY)
			strcpy (overlayTexFile, filename);
		else if (name == TEXTURE_BACKGROUND)
			strcpy (backgroundTexFile, filename);
		else if (name == TEXTURE_WATER)
			strcpy (waterTexFile, filename);
#endif
		d_textureNames[name] = name;

		int w = TextureWidth = image->width;
		int h = TextureHeight = image->height;

		mxImage *image2 = new mxImage ();
		image2->create (w, h, 24);

		gluScaleImage(GL_RGB, w, h, GL_UNSIGNED_BYTE, image->data, w, h, GL_UNSIGNED_BYTE, image2->data);
		delete image;

		glBindTexture (GL_TEXTURE_2D, d_textureNames[name]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR:GL_NEAREST);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, image2->width, image2->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image2->data);
		glBindTexture (GL_TEXTURE_2D, 0);

		delete image2;

		return name;
	}

	return 0;
}

void
GlWindow::setRenderMode (int mode)
{
	d_renderMode = mode;
}

void
GlWindow::setFrameInfo (int startFrame, int endFrame)
{
	if (d_models[0])
	{
		d_startFrame = startFrame;
		d_endFrame = endFrame;

		if (d_startFrame >= d_models[0]->header.numFrames)
			d_startFrame = d_models[0]->header.numFrames - 1;
		else if (d_startFrame < 0)
			d_startFrame = 0;

		if (d_endFrame >= d_models[0]->header.numFrames)
			d_endFrame = d_models[0]->header.numFrames - 1;
		else if (d_endFrame < 0)
			d_endFrame = 0;

		d_currFrame = d_startFrame;
		d_currFrame2 = d_startFrame + 1;

		if (d_currFrame >= d_models[0]->header.numFrames)
			d_currFrame = d_models[0]->header.numFrames - 1;

		if (d_currFrame2 >= d_models[0]->header.numFrames)
			d_currFrame2 = 0;
	}
	else
	{
		d_startFrame = d_endFrame = d_currFrame = d_currFrame2 = 0;
	}

	d_pol = 0;
}

void GlWindow::setPitch (float pitch)
{
	d_pitch = pitch;
	if (d_pitch < 0.0f)
		d_pitch = 0.0f;
}

void GlWindow::setFoV (int fov)
{
	d_fov = fov;
}

void GlWindow::setBGColor (float r, float g, float b)
{
	d_bgColor[0] = r;
	d_bgColor[1] = g;
	d_bgColor[2] = b;
}

void GlWindow::setFGColor (float r, float g, float b)
{
	d_fgColor[0] = r;
	d_fgColor[1] = g;
	d_fgColor[2] = b;
}

void
GlWindow::setWFColor (float r, float g, float b)
{
	d_wfColor[0] = r;
	d_wfColor[1] = g;
	d_wfColor[2] = b;
}

void
GlWindow::setLightColor (float r, float g, float b)
{
	d_lightColor[0] = r;
	d_lightColor[1] = g;
	d_lightColor[2] = b;
}

void
GlWindow::setGridColor (float r, float g, float b)
{
	d_gridColor[0] = r;
	d_gridColor[1] = g;
	d_gridColor[2] = b;
}

void
GlWindow::setFlag (int flag, bool enable)
{
	if (enable)
		d_flags |= flag;
	else
		d_flags &= ~flag;

	md2_setStyle ((int) getFlag (F_GLCOMMANDS), (int) getFlag (F_INTERPOLATE));
}

void GlWindow::setBrightness (int value)
{
	d_bias = (float) value / 100.0f;
	redraw();
}

void GlWindow::setTexScale (float scale)
{
	textureScale = scale / 100.0f;
	if (textureScale < 0.0f)
		textureScale = 0.0f;
}

void GlWindow::setTexAlpha (float alpha)
{
	textureAlpha = alpha / 100.0f;
	if (textureAlpha < 0.0f)
		textureAlpha = 0.0f;
}