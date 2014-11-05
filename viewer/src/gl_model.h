#ifndef INCLUDED_MD2
#define INCLUDED_MD2

#ifndef byte
typedef unsigned char byte;
#endif

#define MD2_MAX_TRIANGLES		16384
#define MD2_MAX_VERTICES		8192
#define MD2_MAX_TEXCOORDS		8192
#define MD2_MAX_FRAMES			512
#define MD2_MAX_SKINS			32
#define MD2_MAX_FRAMESIZE		(MD2_MAX_VERTICES * 4 + 128)

typedef struct 
{ 
   int magic; 
   int version; 
   int skinWidth; 
   int skinHeight; 
   int frameSize; 
   int numSkins; 
   int numVertices; 
   int numTexCoords; 
   int numTriangles; 
   int numGlCommands; 
   int numFrames; 
   int offsetSkins; 
   int offsetTexCoords; 
   int offsetTriangles; 
   int offsetFrames; 
   int offsetGlCommands; 
   int offsetEnd; 
} md2_header_t;

typedef struct
{
   byte vertex[3];
   byte lightNormalIndex;
} md2_alias_triangleVertex_t;

typedef struct
{
   float vertex[3];
   float normal[3];
} md2_triangleVertex_t;

typedef struct
{
   short vertexIndices[3];
   short textureIndices[3];
} md2_triangle_t;

typedef struct
{
   short s, t;
} md2_textureCoordinate_t;

typedef struct
{
   float scale[3];
   float translate[3];
   char name[16];
   md2_alias_triangleVertex_t alias_vertices[1];
} md2_alias_frame_t;

typedef struct
{
   char name[16];
   md2_triangleVertex_t *vertices;
} md2_frame_t;

typedef char md2_skin_t[64];

typedef struct
{
   float s, t;
   int vertexIndex;
} md2_glCommandVertex_t;

typedef struct
{
	md2_header_t			header;
	md2_skin_t				*skins;
	md2_textureCoordinate_t	*texCoords;
	md2_triangle_t			*triangles;
	md2_frame_t				*frames;
	int						*glCommandBuffer;
} md2_model_t;

#ifdef __cplusplus
extern "C" {
#endif

md2_model_t *md2_readModel (const char *filename);
void md2_freeModel (md2_model_t *model);
void md2_setStyle (int glcmds, int interp);
void md2_getBoundingBox (md2_model_t *model, float *minmax);
void md2_drawModel (md2_model_t *model, int frame1, int frame2, float pol);
void md2_generateLightNormals (md2_model_t *model);
int md2_getAnimationCount (md2_model_t *model);
const char *md2_getAnimationName (md2_model_t *model, int animation);
void md2_getAnimationFrames (md2_model_t *model, int animation, int *startFrame, int *endFrame);

#ifdef __cplusplus
}
#endif

#endif