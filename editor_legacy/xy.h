
// window system independent camera view code

typedef struct
{
	int		width, height;

	bool	timing;

	vec3_t	origin;			// at center of window
	float	scale;

	float	topclip, bottomclip;

	bool d_dirty;
} xy_t;

BOOL XYExcludeBrush(brush_t	*pb);

void XY_Init (void);
void XY_MouseDown (int x, int y, int buttons);
void XY_MouseUp(int buttons);
void XY_MouseMoved (int x, int y, int buttons);
void XY_Draw (void);
void XY_Overlay (void);

BOOL FilterBrush(brush_t *pb);
