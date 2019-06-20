#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include "linmath.h"
#include "shader.h"
#include "primitive.h"
#include "stdafx.h"

Primitive* createPrimitive(unsigned int primitiveCount)
{
	Primitive *tmp;
	int i;
	unsigned int len = sizeof(*tmp)*primitiveCount;

	assert(primitiveCount > 0);

	tmp = malloc(len);
	memset(tmp, 0, len);

	for (i = 0; i < primitiveCount; ++i)
		mat4x4_identity(tmp[i].transf);

	return tmp;
}

void destroyPrimitive(Primitive *p, unsigned int count)
{
	int i, j;

	assert(NULL != p);

	for (i = 0; i < count; ++i)
		free(p[i].faceArray);

	free(p);
}

void setPrimitiveBuffer(Primitive *base, unsigned int position, unsigned int maxCount,
	const GLvoid *buffer, GLsizeiptr size)
{
	assert(position < maxCount);
	assert(NULL != base);

	base[position].points = buffer;
	base[position].pSize = size;
}

void initPrimitiveFaceArray(Primitive *base, unsigned int position, unsigned int maxCount,
	unsigned int faceCount)
{
	assert(position < maxCount);
	assert(NULL != base);

	base[position].faceArray = malloc(sizeof(*base[position].faceArray)*faceCount);
	base[position].faceCount = faceCount;
}

Faces* getPrimitiveFaceElement(const Primitive *base, unsigned int position, unsigned int maxCount,
	int element)
{
	assert(position < maxCount);
	assert(NULL != base);

	return &base[position].faceArray[element];
}

void setPrimitiveTransformation(Primitive *base, unsigned int position, unsigned int maxCount,
	mat4x4 matrix)
{
	assert(position < maxCount);
	assert(NULL != base);

	mat4x4_dup(base[position].transf, matrix);
}

mat4x4* getPrimitiveTransformation(Primitive *base, unsigned int position, unsigned int maxCount)
{
	assert(position < maxCount);
	assert(NULL != base);

	return &base[position].transf;
}


void initFace(Faces *face)
{
	assert(NULL != face);
	memset(face, 0, sizeof(*face));
	mat4x4_identity(face->transf);
}

void setFace(Faces *face, const GLuint *vector, unsigned int maxElements)
{
	assert(NULL != face);
	face->face = vector;
	face->count = maxElements;
}

void setFaceTransformation(Faces *face, mat4x4 matrix)
{
	assert(NULL != face);
	mat4x4_dup(face->transf, matrix);
}

mat4x4* getFaceTransformation(Faces *face)
{
	assert(NULL != face);
	return &face->transf;
}