#ifndef INCLUDED_GLWINDOW
#define INCLUDED_GLWINDOW

#ifndef INCLUDED_MXGLWINDOW
#include "mxGlWindow.h"
#endif

#ifndef INCLUDED_MD2VIEWER
#include "okv_main.h"
#endif

#ifndef INCLUDED_MD2
#include "gl_model.h"
#endif

enum
{
	F_WATER = 1,
	F_LIGHT = 2,
	F_GRID = 4,
	F_INTERPOLATE = 8,
	F_GLCOMMANDS = 16,
	F_PAUSE = 32,
	F_BACKGROUND = 64,
	F_DRAWTEXTURE = 128
};
enum
{
	TEXTURE_MODEL = 1,
	TEXTURE_OVERLAY = 2,
	TEXTURE_BACKGROUND = 3,
	TEXTURE_WATER = 4
};
enum
{
	RM_WIREFRAME,
	RM_FLATSHADED,
	RM_SMOOTHSHADED,
	RM_TEXTUREDFLAT,
	RM_TEXTURED
};

class GlWindow : public mxGlWindow
{
	float d_rotX, d_rotY;
	float d_transX, d_transY, d_transZ;
	md2_model_t *d_models[2];
	unsigned int d_textureNames[5];
	int d_renderMode;
	float textureScale;
	float textureAlpha;
	int TextureHeight;
	int TextureWidth;
	int d_currFrame, d_currFrame2, d_startFrame, d_endFrame;
	float d_pitch;
	int d_fov;
	float d_bgColor[3];
	float d_fgColor[3];
	float d_wfColor[3];
	float d_lightColor[3];
	float d_gridColor[3];
	float d_bias;
	int d_flags;
public:
	friend class KatanaViewer;
	GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style);
	~GlWindow ();
	virtual int handleEvent (mxEvent *event);
	virtual void draw ();
	md2_model_t *loadModel (const char *filename, int pos);
	int loadTexture (const char *filename, int name);
	void setRenderMode (int mode);
	void setFrameInfo (int startFrame, int endFrame);
	void setPitch (float pitch);
	void setFoV (int fov);
	void setBGColor (float r, float g, float b);
	void setFGColor (float r, float g, float b);
	void setWFColor (float r, float g, float b);
	void setLightColor (float r, float g, float b);
	void setGridColor (float r, float g, float b);
	void setFlag (int flag, bool enable);
	void setBrightness (int value);
	void setTexScale (float scale);
	void setTexAlpha (float alpha);
	void DumpModelInfo(md2_model_t *model, const char *filename);
	md2_model_t *getModel (int pos) const { return d_models[pos]; }
	int getRenderMode () const { return d_renderMode; }
	int getCurrFrame () const { return d_currFrame; }
	int getCurrFrame2 () const { return d_currFrame2; }
	int getStartFrame () const { return d_startFrame; }
	int getEndFrame () const { return d_endFrame; }
	void getBGColor (float *r, float *g, float *b) { *r = d_bgColor[0]; *g = d_bgColor[1]; *b = d_bgColor[2]; }
	void getFGColor (float *r, float *g, float *b) { *r = d_fgColor[0]; *g = d_fgColor[1]; *b = d_fgColor[2]; }
	void getWFColor (float *r, float *g, float *b) { *r = d_wfColor[0]; *g = d_wfColor[1]; *b = d_wfColor[2]; }
	void getLightColor (float *r, float *g, float *b) { *r = d_lightColor[0]; *g = d_lightColor[1]; *b = d_lightColor[2]; }
	void getGridColor (float *r, float *g, float *b) { *r = d_gridColor[0]; *g = d_gridColor[1]; *b = d_gridColor[2]; }
	bool getFlag (int flag) const { return ((d_flags & flag) == flag); }
	int getFlags () const { return d_flags; }

	float d_pol;
};

#ifdef WIN32
extern char modelFile[];
extern char modelTexFile[];
extern char overlayFile[];
extern char overlayTexFile[];
extern char backgroundTexFile[];
extern char waterTexFile[];
#endif

#endif
