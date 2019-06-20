#ifndef __PRIMITIVE_H
#define __PRIMITIVE_H

# include <GL/glew.h>
# include <stdlib.h>

# include "linmath.h"

/**
 * @brief Define os elementos que compõem uma face
 * 
 * Os vértices são numerados. Essa estrutura permite definir quais os vértices
 * que compõem uma face completa
 */
typedef struct Faces
{
	const GLuint	*face;	/**< vetor dos elementos que compõem uma face */
	uint 		count;	/**< quantidade de elementos nesse vetor */
	mat4x4		transf;	/**< matriz de transformação para esta face */
} Faces;

/**
 * @brief Permite a construção de primitivas mais complexas
 * 
 * Primitivas mais complexas podem ser construídas através da agregação de 
 * várias faces. Note que os pontos de definem os vértices devem ser incluídos
 * em um único array.
 */
typedef struct Primitive
{
	GLuint		id;		/**< Nome da estrutura no driver de vídeo */
	const GLvoid	*points;	/**< Array para os pontos dos vertexs */
	GLsizeiptr	pSize;		/**< Tamanho total do array de vertex. Em bytes */
	Faces		*faceArray;	/**< Array de faces. Permite a construção de primitivas mais complexas */
	GLuint		faceCount;	/**< Quantidade de elementos no array de faces */
	mat4x4		transf;		/**< Matriz de transformação de toda a primitiva */
} Primitive;

// APIs públicas

// APIs relacionadas com a estrututra Primitive

Primitive* createPrimitive(uint primitiveCount);

void destroyPrimitive(Primitive *p, uint count);

void setPrimitiveBuffer(Primitive *base, uint position, uint maxCount,
			const GLvoid *buffer, GLsizeiptr size);

void initPrimitiveFaceArray(Primitive *base, uint position, uint maxCount, 
			    uint faceCount);

Faces* getPrimitiveFaceElement(const Primitive *base, uint position, uint maxCount,
			       int element);

void setPrimitiveTransformation(Primitive *base, uint position, uint maxCount,
				mat4x4 matrix);

mat4x4* getPrimitiveTransformation(Primitive *base, uint position, uint maxCount);


// APIs relacionadas com a estrutura Faces

void initFace(Faces *face);

void setFace(Faces *face, const GLuint *vector, uint maxElements);

void setFaceTransformation(Faces *face, mat4x4 matrix);

mat4x4* getFaceTransformation(Faces *face);
#endif