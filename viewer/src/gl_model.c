#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <mx/GL.H>
#include "gl_model.h"

#define NUMVERTEXNORMALS 162

float avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

static int g_glcmds = 1;
static int g_interp = 1;

md2_model_t*
md2_readModel (const char *filename)
{
	FILE *file;
	md2_model_t *model;
	byte buffer[MD2_MAX_FRAMESIZE];
	int i;

	model = (md2_model_t *) malloc (sizeof (md2_model_t));
	if (!model)
		return 0;

	file = fopen (filename, "rb");
	if (!file)
	{
		free (model);
		return 0;
	}

	memset (model, 0, sizeof (md2_model_t));
	fread (&model->header, sizeof (md2_header_t), 1, file);
#if 0
	printf ("magic:\t\t%d\n", model->header.magic);
	printf ("version:\t\t%d\n", model->header.version);
	printf ("skinWidth:\t\t%d\n", model->header.skinWidth);
	printf ("skinHeight:\t\t%d\n", model->header.skinHeight);
	printf ("frameSize:\t\t%d\n", model->header.frameSize);
	printf ("numSkins:\t\t%d\n", model->header.numSkins);
	printf ("numVertices:\t\t%d\n", model->header.numVertices);
	printf ("numTexCoords:\t\t%d\n", model->header.numTexCoords);
	printf ("numTriangles:\t\t%d\n", model->header.numTriangles);
	printf ("numGlCommands:\t\t%d\n", model->header.numGlCommands);
	printf ("numFrames:\t\t%d\n", model->header.numFrames);
	printf ("offsetSkins:\t\t%d\n", model->header.offsetSkins);
	printf ("offsetTexCoords:\t%d\n", model->header.offsetTexCoords);
	printf ("offsetTriangles:\t%d\n", model->header.offsetTriangles);
	printf ("offsetFrames:\t\t%d\n", model->header.offsetFrames);
	printf ("offsetGlCommands:\t%d\n", model->header.offsetGlCommands);
	printf ("offsetEnd:\t\t%d\n", model->header.offsetEnd);
#endif
	if (model->header.magic != (int) (('2' << 24) + ('P' << 16) + ('D' << 8) + 'I'))
	{
		fclose (file);
		free (model);
		return 0;
	}

	fseek (file, model->header.offsetSkins, SEEK_SET);
	if (model->header.numSkins > 0)
	{
		model->skins = (md2_skin_t *)malloc(sizeof(md2_skin_t)*model->header.numSkins);
		if (!model->skins)
		{
			md2_freeModel (model);
			return 0;
		}

		for (i = 0; i < model->header.numSkins; i++)
			fread (&model->skins[i], sizeof (md2_skin_t), 1, file);
	}

	fseek (file, model->header.offsetTexCoords, SEEK_SET);
	if (model->header.numTexCoords > 0)
	{
		model->texCoords = (md2_textureCoordinate_t *) malloc (sizeof (md2_textureCoordinate_t) * model->header.numTexCoords);
		if (!model->texCoords)
		{
			md2_freeModel (model);
			return 0;
		}

		for (i = 0; i < model->header.numTexCoords; i++)
			fread (&model->texCoords[i], sizeof (md2_textureCoordinate_t), 1, file);
	}

	fseek (file, model->header.offsetTriangles, SEEK_SET);
	if (model->header.numTriangles)
	{
		if(model->header.numTriangles > MD2_MAX_TRIANGLES)
			return 0;

		model->triangles = (md2_triangle_t *) malloc (sizeof (md2_triangle_t) * model->header.numTriangles);
		if (!model->triangles)
		{
			md2_freeModel (model);
			return 0;
		}

		for (i = 0; i < model->header.numTriangles; i++)
			fread (&model->triangles[i], sizeof (md2_triangle_t), 1, file);
	}

	fseek (file, model->header.offsetFrames, SEEK_SET);
	if (model->header.numFrames > 0)
	{
		model->frames = (md2_frame_t *) malloc (sizeof (md2_frame_t) * model->header.numFrames);
		if (!model->frames)
		{
			md2_freeModel (model);
			return 0;
		}

		for (i = 0; i < model->header.numFrames; i++)
		{
			md2_alias_frame_t *frame = (md2_alias_frame_t *) buffer;
			int j;

			model->frames[i].vertices = (md2_triangleVertex_t *) malloc (sizeof (md2_triangleVertex_t) * model->header.numVertices);
			if (!model->frames[i].vertices)
			{
				md2_freeModel (model);
				return 0;
			}

			fread (frame, 1, model->header.frameSize, file);
			strcpy (model->frames[i].name, frame->name);
			for (j = 0; j < model->header.numVertices; j++)
			{
				model->frames[i].vertices[j].vertex[0] = (float) ((int) frame->alias_vertices[j].vertex[0]) * frame->scale[0] + frame->translate[0];
				model->frames[i].vertices[j].vertex[2] = -1* ((float) ((int) frame->alias_vertices[j].vertex[1]) * frame->scale[1] + frame->translate[1]);
				model->frames[i].vertices[j].vertex[1] = (float) ((int) frame->alias_vertices[j].vertex[2]) * frame->scale[2] + frame->translate[2];

				model->frames[i].vertices[j].normal[0] = avertexnormals[frame->alias_vertices[j].lightNormalIndex][0];
				model->frames[i].vertices[j].normal[1] = avertexnormals[frame->alias_vertices[j].lightNormalIndex][1];
				model->frames[i].vertices[j].normal[2] = avertexnormals[frame->alias_vertices[j].lightNormalIndex][2];
			}
		}
	}

	fseek(file,model->header.offsetGlCommands,SEEK_SET);
	if(model->header.numGlCommands)
	{
		model->glCommandBuffer = (int *) malloc (sizeof (int) * model->header.numGlCommands);
		if (!model->glCommandBuffer)
		{
			md2_freeModel (model);
			return 0;
		}

		fread (model->glCommandBuffer, sizeof (int), model->header.numGlCommands, file);
	}

	fclose (file);

	return model;
}

void
md2_freeModel (md2_model_t *model)
{
	if (model)
	{
		if (model->skins)
			free (model->skins);

		if (model->texCoords)
			free (model->texCoords);

		if (model->triangles)
			free (model->triangles);

		if (model->frames)
		{
			int i;

			for (i = 0; i < model->header.numFrames; i++)
			{
				if (model->frames[i].vertices)
					free (model->frames[i].vertices);
			}
			free (model->frames);
		}

		if (model->glCommandBuffer)
			free (model->glCommandBuffer);

		free (model);
	}
}

void md2_setStyle (int glcmds, int interp)
{
	g_glcmds = glcmds;
	g_interp = interp;
}

void md2_getBoundingBox (md2_model_t *model, float *minmax)
{
	int i;
	float minx, maxx;
	float miny, maxy;
	float minz, maxz;

	minx = miny = minz = 999999.0f;
	maxx = maxy = maxz = -999999.0f;

	for (i = 0; i < model->header.numVertices; i++)
	{
		md2_triangleVertex_t *v = &model->frames[0].vertices[i];

		if (v->vertex[0] < minx)
			minx = v->vertex[0];
		else if (v->vertex[0] > maxx)
			maxx = v->vertex[0];

		if (v->vertex[1] < miny)
			miny = v->vertex[1];
		else if (v->vertex[1] > maxy)
			maxy = v->vertex[1];

		if (v->vertex[2] < minz)
			minz = v->vertex[2];
		else if (v->vertex[2] > maxz)
			maxz = v->vertex[2];
	}

	minmax[0] = minx;
	minmax[1] = maxx;
	minmax[2] = miny;
	minmax[3] = maxy;
	minmax[4] = minz;
	minmax[5] = maxz;
}

void
_md2_drawModel (md2_model_t *model, int frame)
{
	int i;

	md2_frame_t *f = &model->frames[frame];

	glBegin (GL_TRIANGLES);

	for (i = 0; i < model->header.numTriangles; i++)
	{
		md2_triangle_t *t = &model->triangles[i];

		glNormal3f (f->vertices[t->vertexIndices[0]].normal[0],
			f->vertices[t->vertexIndices[0]].normal[2],
			-f->vertices[t->vertexIndices[0]].normal[1]);
		glTexCoord2f ((float) model->texCoords[t->textureIndices[0]].s / (float) model->header.skinWidth,
					(float) model->texCoords[t->textureIndices[0]].t / (float) model->header.skinHeight);
		glVertex3f (f->vertices[t->vertexIndices[0]].vertex[0],
					f->vertices[t->vertexIndices[0]].vertex[1],
					f->vertices[t->vertexIndices[0]].vertex[2]);

		glNormal3f (f->vertices[t->vertexIndices[1]].normal[0],
			f->vertices[t->vertexIndices[1]].normal[2],
			-f->vertices[t->vertexIndices[1]].normal[1]);
		glTexCoord2f ((float) model->texCoords[t->textureIndices[1]].s / (float) model->header.skinWidth,
					(float) model->texCoords[t->textureIndices[1]].t / (float) model->header.skinHeight);
		glVertex3f (f->vertices[t->vertexIndices[1]].vertex[0],
					f->vertices[t->vertexIndices[1]].vertex[1],
					f->vertices[t->vertexIndices[1]].vertex[2]);

		glNormal3f (f->vertices[t->vertexIndices[2]].normal[0],
			f->vertices[t->vertexIndices[2]].normal[2],
			-f->vertices[t->vertexIndices[2]].normal[1]);
		glTexCoord2f ((float) model->texCoords[t->textureIndices[2]].s / (float) model->header.skinWidth,
					(float) model->texCoords[t->textureIndices[2]].t / (float) model->header.skinHeight);
		glVertex3f (f->vertices[t->vertexIndices[2]].vertex[0],
					f->vertices[t->vertexIndices[2]].vertex[1],
					f->vertices[t->vertexIndices[2]].vertex[2]);
	}

	glEnd ();
}

void
_md2_drawModeli (md2_model_t *model, int frame1, int frame2, float pol)
{
	int i;

	md2_frame_t *f1 = &model->frames[frame1];
	md2_frame_t *f2 = &model->frames[frame2];

	glBegin (GL_TRIANGLES);

	for (i = 0; i < model->header.numTriangles; i++)
	{
		md2_triangle_t *t = &model->triangles[i];
		float x1, y1, z1, x2, y2, z2;
		float *n1, *n2;
		
		glTexCoord2f ((float) model->texCoords[t->textureIndices[0]].s / (float) model->header.skinWidth,
					(float) model->texCoords[t->textureIndices[0]].t / (float) model->header.skinHeight);

		n1 = f1->vertices[t->vertexIndices[0]].normal;
		n2 = f2->vertices[t->vertexIndices[0]].normal;
		glNormal3f ((1.0f - pol) * n1[0] + pol * n2[0],
			(1.0f - pol) * n1[2] + pol * n2[2],
			(1.0f - pol) * -n1[1] + pol * -n2[1]);

		x1 = f1->vertices[t->vertexIndices[0]].vertex[0];
		y1 = f1->vertices[t->vertexIndices[0]].vertex[1];
		z1 = f1->vertices[t->vertexIndices[0]].vertex[2];
		x2 = f2->vertices[t->vertexIndices[0]].vertex[0];
		y2 = f2->vertices[t->vertexIndices[0]].vertex[1];
		z2 = f2->vertices[t->vertexIndices[0]].vertex[2];
		glVertex3f (x1 + pol * (x2 - x1),
			y1 + pol * (y2 - y1),
			z1 + pol * (z2 - z1));

		glTexCoord2f ((float) model->texCoords[t->textureIndices[1]].s / (float) model->header.skinWidth,
					(float) model->texCoords[t->textureIndices[1]].t / (float) model->header.skinHeight);

		n1 = f1->vertices[t->vertexIndices[1]].normal;
		n2 = f2->vertices[t->vertexIndices[1]].normal;
		glNormal3f ((1.0f - pol) * n1[0] + pol * n2[0],
			(1.0f - pol) * n1[2] + pol * n2[2],
			(1.0f - pol) * -n1[1] + pol * -n2[1]);

		x1 = f1->vertices[t->vertexIndices[1]].vertex[0];
		y1 = f1->vertices[t->vertexIndices[1]].vertex[1];
		z1 = f1->vertices[t->vertexIndices[1]].vertex[2];
		x2 = f2->vertices[t->vertexIndices[1]].vertex[0];
		y2 = f2->vertices[t->vertexIndices[1]].vertex[1];
		z2 = f2->vertices[t->vertexIndices[1]].vertex[2];
		glVertex3f (x1 + pol * (x2 - x1),
			y1 + pol * (y2 - y1),
			z1 + pol * (z2 - z1));

		glTexCoord2f ((float) model->texCoords[t->textureIndices[2]].s / (float) model->header.skinWidth,
					(float) model->texCoords[t->textureIndices[2]].t / (float) model->header.skinHeight);

		n1 = f1->vertices[t->vertexIndices[2]].normal;
		n2 = f2->vertices[t->vertexIndices[2]].normal;
		glNormal3f ((1.0f - pol) * n1[0] + pol * n2[0],
			(1.0f - pol) * n1[2] + pol * n2[2],
			(1.0f - pol) * -n1[1] + pol * -n2[1]);

		x1 = f1->vertices[t->vertexIndices[2]].vertex[0];
		y1 = f1->vertices[t->vertexIndices[2]].vertex[1];
		z1 = f1->vertices[t->vertexIndices[2]].vertex[2];
		x2 = f2->vertices[t->vertexIndices[2]].vertex[0];
		y2 = f2->vertices[t->vertexIndices[2]].vertex[1];
		z2 = f2->vertices[t->vertexIndices[2]].vertex[2];
		glVertex3f (x1 + pol * (x2 - x1),
			y1 + pol * (y2 - y1),
			z1 + pol * (z2 - z1));
	}
	glEnd ();
}

void
_md2_drawModelg (md2_model_t *model, int frame)
{
	int i = 0;
	int val = model->glCommandBuffer[i++];

	while (val != 0)
	{
		int count;

		if (val > 0)
		{
			glBegin (GL_TRIANGLE_STRIP);
			count = val;
		}
		else
		{
			glBegin (GL_TRIANGLE_FAN);
			count = -val;
		}

		while (count--)
		{
			float s = *(float *) &model->glCommandBuffer[i++];
			float t = *(float *) &model->glCommandBuffer[i++];
			int index = model->glCommandBuffer[i++];

			glTexCoord2f (s, t);
			glNormal3f (model->frames[frame].vertices[index].normal[0],
				model->frames[frame].vertices[index].normal[2],
				-model->frames[frame].vertices[index].normal[1]);
			glVertex3f (model->frames[frame].vertices[index].vertex[0],
				model->frames[frame].vertices[index].vertex[1],
				model->frames[frame].vertices[index].vertex[2]);
		}

		glEnd ();

		val = model->glCommandBuffer[i++];
	}
}

void
_md2_drawModelgi (md2_model_t *model, int frame1, int frame2, float pol)
{
	int i = 0;
	int val = model->glCommandBuffer[i++];

	while (val != 0)
	{
		int count;

		if (val > 0)
		{
			glBegin (GL_TRIANGLE_STRIP);
			count = val;
		}
		else
		{
			glBegin (GL_TRIANGLE_FAN);
			count = -val;
		}

		while (count--)
		{
			float s = *(float *) &model->glCommandBuffer[i++];
			float t = *(float *) &model->glCommandBuffer[i++];
			int index = model->glCommandBuffer[i++];
			float x1, y1, z1, x2, y2, z2;
			float *n1 = model->frames[frame1].vertices[index].normal;
			float *n2 = model->frames[frame2].vertices[index].normal;

			glTexCoord2f (s, t);
			glNormal3f ((1.0f - pol) * n1[0] + pol * n2[0],
				(1.0f - pol) * n1[2] + pol * n2[2],
				(1.0f - pol) * -n1[1] + pol * -n2[1]);

			x1 = model->frames[frame1].vertices[index].vertex[0];
			y1 = model->frames[frame1].vertices[index].vertex[1];
			z1 = model->frames[frame1].vertices[index].vertex[2];

			x2 = model->frames[frame2].vertices[index].vertex[0];
			y2 = model->frames[frame2].vertices[index].vertex[1];
			z2 = model->frames[frame2].vertices[index].vertex[2];

			glVertex3f (x1 + pol * (x2 - x1),
				y1 + pol * (y2 - y1),
				z1 + pol * (z2 - z1));
		}

		glEnd ();

		val = model->glCommandBuffer[i++];
	}
}

void
md2_drawModel (md2_model_t *model, int frame1, int frame2, float pol)
{
	if (g_glcmds)
	{
		if (g_interp)
			_md2_drawModelgi (model, frame1, frame2, pol);
		else
			_md2_drawModelg (model, frame1);
	}
	else
	{
		if (g_interp)
			_md2_drawModeli (model, frame1, frame2, pol);
		else
			_md2_drawModel (model, frame1);
	}
}

void
_makeFacetNormal (md2_model_t *model, md2_triangle_t *t, int frame, float *fn)
{
	float v1[3], v2[3];
	double angle;

	v1[0] = model->frames[frame].vertices[t->vertexIndices[1]].vertex[0] - model->frames[frame].vertices[t->vertexIndices[0]].vertex[0];
	v1[1] = model->frames[frame].vertices[t->vertexIndices[1]].vertex[1] - model->frames[frame].vertices[t->vertexIndices[0]].vertex[1];
	v1[2] = model->frames[frame].vertices[t->vertexIndices[1]].vertex[2] - model->frames[frame].vertices[t->vertexIndices[0]].vertex[2];

	v2[0] = model->frames[frame].vertices[t->vertexIndices[2]].vertex[0] - model->frames[frame].vertices[t->vertexIndices[0]].vertex[0];
	v2[1] = model->frames[frame].vertices[t->vertexIndices[2]].vertex[1] - model->frames[frame].vertices[t->vertexIndices[0]].vertex[1];
	v2[2] = model->frames[frame].vertices[t->vertexIndices[2]].vertex[2] - model->frames[frame].vertices[t->vertexIndices[0]].vertex[2];

angle = 1;

	fn[0] = (v1[1] * v2[2] - v1[2] * v2[1]) * (float) angle;
	fn[1] = (v1[2] * v2[0] - v1[0] * v2[2]) * (float) angle;
	fn[2] = (v1[0] * v2[1] - v1[1] * v2[0]) * (float) angle;
}

void
_normalize (float *n)
{
	float l = (float) sqrt (n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);

	if (l != 0.0f)
	{
		n[0] /= l;
		n[1] /= l;
		n[2] /= l;
	}
}

void
md2_generateLightNormals (md2_model_t *model)
{
	int i;

	if (!model)
		return;

	for (i = 0; i < model->header.numFrames; i++)
	{
		int j;

		for (j = 0; j < model->header.numVertices; j++)
		{
			model->frames[i].vertices[j].normal[0] = 0.0f;
			model->frames[i].vertices[j].normal[1] = 0.0f;
			model->frames[i].vertices[j].normal[2] = 0.0f;
		}

		for (j = 0; j < model->header.numTriangles; j++)
		{
			int k;
			float facetnormal[3];

			_makeFacetNormal (model, &model->triangles[j], i, facetnormal);

			for (k = 0; k < 3; k++)
			{
				model->frames[i].vertices[model->triangles[j].vertexIndices[k]].normal[0] -= facetnormal[0];
				model->frames[i].vertices[model->triangles[j].vertexIndices[k]].normal[1] += facetnormal[2];
				model->frames[i].vertices[model->triangles[j].vertexIndices[k]].normal[2] -= facetnormal[1];
			}
		}

		for (j = 0; j < model->header.numVertices; j++)
			_normalize (model->frames[i].vertices[j].normal);
	}
}

int
md2_getAnimationCount (md2_model_t *model)
{
	int i, j, pos;
	int count;
	int lastId;
	char name[16], last[16];

	strcpy (last, model->frames[0].name);
	pos = strlen (last) - 1;
	j = 0;
	while (last[pos] >= '0' && last[pos] <= '9' && j < 2)
	{
		pos--;
		j++;
	}
	last[pos + 1] = '\0';

	lastId = 0;
	count = 0;

	for (i = 0; i <= model->header.numFrames; i++)
	{
		if (i == model->header.numFrames)
			strcpy (name, "");
		else
			strcpy (name, model->frames[i].name);
		pos = strlen (name) - 1;
		j = 0;
		while (name[pos] >= '0' && name[pos] <= '9' && j < 2)
		{
			pos--;
			j++;
		}
		name[pos + 1] = '\0';

		if (strcmp (last, name))
		{
			strcpy (last, name);
			count++;
		}
	}

	return count;
}

const char *
md2_getAnimationName (md2_model_t *model, int animation)
{
	int i, j, pos;
	int count;
	int lastId;
	static char last[32];
	char name[32];

	strcpy (last, model->frames[0].name);
	pos = strlen (last) - 1;
	j = 0;
	while (last[pos] >= '0' && last[pos] <= '9' && j < 2)
	{
		pos--;
		j++;
	}
	last[pos + 1] = '\0';

	lastId = 0;
	count = 0;

	for (i = 0; i <= model->header.numFrames; i++)
	{
		if (i == model->header.numFrames)
			strcpy (name, "");
		else
			strcpy (name, model->frames[i].name);
		pos = strlen (name) - 1;
		j = 0;
		while (name[pos] >= '0' && name[pos] <= '9' && j < 2)
		{
			pos--;
			j++;
		}
		name[pos + 1] = '\0';

		if (strcmp (last, name))
		{
			if (count == animation)
				return last;

			strcpy (last, name);
			count++;
		}
	}

	return 0;
}

void
md2_getAnimationFrames (md2_model_t *model, int animation, int *startFrame, int *endFrame)
{
	int i, j, pos;
	int count, numFrames, frameCount;
	int lastId;
	char name[16], last[16];

	strcpy (last, model->frames[0].name);
	pos = strlen (last) - 1;
	j = 0;
	while (last[pos] >= '0' && last[pos] <= '9' && j < 2)
	{
		pos--;
		j++;
	}
	last[pos + 1] = '\0';

	lastId = 0;
	count = 0;
	numFrames = 0;
	frameCount = 0;

	for (i = 0; i <= model->header.numFrames; i++)
	{
		if (i == model->header.numFrames)
			strcpy (name, "");
		else
			strcpy (name, model->frames[i].name);
		pos = strlen (name) - 1;
		j = 0;
		while (name[pos] >= '0' && name[pos] <= '9' && j < 2)
		{
			pos--;
			j++;
		}
		name[pos + 1] = '\0';

		if (strcmp (last, name))
		{
			strcpy (last, name);

			if (count == animation)
			{
				*startFrame = frameCount - numFrames;
				*endFrame = frameCount - 1;
				return;
			}

			count++;
			numFrames = 0;
		}
		frameCount++;
		numFrames++;
	}


	*startFrame = *endFrame = 0;
}
