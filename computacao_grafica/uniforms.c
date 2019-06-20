#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "shader.h"
#include "primitive.h"
#include "linmath.h"

// Algumas variáveis globais
const int MAJOR = 2;
const int MINOR = 1;
const uint WIDTH = 800;
const uint HEIGHT = 600;

// Definindo algumas primitivas a ser desenhada

// Um prisma unitário
static const GLfloat prismVertex[] = {
	-1.f, -1.f, 1.f,
	1.f, -1.f, 1.f,
	1.f, -1.f, -1.f,
	-1.f, -1.f, -1.f,
	0.f, 1.f, 0.f,
};

static const GLuint prismElem[] = { // são 18 elementos
	0, 1, 4,
	1, 2, 4,
	2, 3, 4,
	3, 0, 4,
	0, 3, 2,
	2, 1, 0
};

// Um cubo unitário
static const GLfloat cubeVertex[] = {
	-1.f, -1.f, 1.f,
	1.f, -1.f, 1.f,
	1.f, 1.f, 1.f,
	-1.f, 1.f, 1.f,
	-1.f, -1.f, -1.f,
	1.f, -1.f, -1.f,
	1.f, 1.f, -1.f,
	-1.f, 1.f, -1.f
};

static const GLuint cubeElem[] = {  // são 36 elementos
	0, 1, 3,
	3, 1, 2,
	1, 5, 2,
	2, 5, 6,
	5, 4, 7,
	7, 6, 5,
	4, 0, 3,
	3, 7, 4,
	0, 4, 5,
	5, 1, 0,
	3, 2, 6,
	7, 3, 6
};

/////////////////////////////////////////////////////////////////////////


typedef struct Parameters
{
	char vertex[256];
	char fragment[256];
} Parameters;


/**
 * @brief Inicializa alguns valores no OpenGL
 */
static void initOpenGL()
{
	GLuint VertexArrayID;

	// preenche o frameBuffer com a seguinte cor
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	
	// Necessário para funcionar
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
}

/**
 * @brief Passa os valores de vértices para um buffer na memória de vídeo
 * 
 * @param p estrutura que contem um ponteiro para o buffer e o seu tamanho total
 */
static void loadVertexBuffer(Primitive *p)
{
	// Cria um buffer para armazernar, na memória de Video, os pontos que definem um objeto
	glGenBuffers(1, &p->id);
	glBindBuffer(GL_ARRAY_BUFFER, p->id);
	
	// Passa para a memória de video esses pontos
	glBufferData(GL_ARRAY_BUFFER, p->pSize, p->points, GL_STATIC_DRAW);
	
	return;
}

/**
 * @brief Permite atualizar as matrizes de cada primitiva
 * 
 * Antes de renderizar uma cena, é possível atualizar as matrizes de cada
 * primitiva. Permite realizar a animação das cenas
 * 
 * @param p <i>Array</i> de primitivas
 * @param count tamanho deste array, em elementos
 */
static void update(Primitive *p, int count)
{
	int i;

	for (i = 0; i < count; ++i) {
		mat4x4 matrix;
		mat4x4 *tmp = getPrimitiveTransformation(p, i, count);
		
		mat4x4_rotate_Y(matrix, *tmp, 0.005);
		setPrimitiveTransformation(p, i, count, matrix);
	}
}

/**
 * @brief Renderiza a cena, quadro a quadro
 * 
 * Permite a renderização da cena, quadro a quadro, de um array de primitivas.
 * Note que as matrizes de transformação são aplicadas, tanto para as definidas
 * na estrutura FACE, quanto as definidas de modo mais global, ou seja, na
 * estrutura PRIMITIVE
 * 
 * @param transf Identificador da variável transformation no shader vertex
 * @param p Array de primitivas que deve ser renderizada
 * @param count Quantidade de elementos do array
 */
static void render(GLuint transf, Primitive *p, int count)
{
	int i, j;
	
	// Clear frameBuffer
	glClear(GL_COLOR_BUFFER_BIT);

	glEnableVertexAttribArray(0);
	
	for (i = 0; i < count; ++i) {
		mat4x4 *pMatrix = getPrimitiveTransformation(p, i, count);

		glBindBuffer(GL_ARRAY_BUFFER, p[i].id);
	
		// Como acessar os dados que estão na memória de video
		glVertexAttribPointer(
			0, 		// location 0 in vertex program
			3, 		// cada vertex possui 3 pontos
			GL_FLOAT,	// tipo dos pontos
			GL_FALSE,	// não normalizar
			0, 		// pontos estão sem folgas entre eles
			(GLvoid *) 0 	// sem offset inicial
		);
	
		// Desenha a primitiva
		for (j = 0; j < p[i].faceCount; ++j)  {
			assert(NULL != p[i].faceArray);
			mat4x4 tmp;
			
			mat4x4_mul(tmp, *pMatrix, p[i].faceArray[j].transf);
			glUniformMatrix4fv(transf, 1, GL_FALSE, 
					   (GLfloat*) tmp);
			glDrawElements(
//				GL_TRIANGLES,
				GL_LINE_LOOP,
				p[i].faceArray[j].count,
				GL_UNSIGNED_INT,
				p[i].faceArray[j].face
			);
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
}


static void printHelp(int argc, char **argv)
{
	printf("Uso:\n");
	printf("%s <programa vertex> <programa fragment>\n", argv[0]);
}


static int parseParameters(int argc, char **argv, Parameters *params)
{
	if ( argc != 3 ) {
		printHelp(argc, argv);
		return -1;
	}
	strncpy(params->vertex, argv[1], 256);
	strncpy(params->fragment, argv[2], 256);
	return 0;
}


// estabelecendo um callback para erros
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	fputc('\n', stderr);
}


/**
 * @brief Inicializa um contexto openGL
 */
static GLFWwindow* initWindow(int argc, char *argv[], Parameters *params)
{
	parseParameters(argc, argv, params);
	glfwSetErrorCallback(error_callback);

	// Por enquanto, vamos evitar o redimensionamento. No futuro, 
	// vamos tirar essa linha
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	
	// Contexto OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR); // Colocar 2 para o OGL2.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR); // Colocar 1 para o OGL2.1	
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Comentar para o OGL2.1
	return glfwCreateWindow(WIDTH, HEIGHT, "Primeiro Programa em OpenGL", NULL, NULL);
}

/**
 * @brief Inicializa os buffers e instala os shaders
 * 
 * Transfere as primitivas para a memória de vídeo, instala os shaders e
 * inicializa o programa correspondente
 * 
 * @param params Estrutura que contêm os nomes dos shaders, no sistema de arquivo
 * @param transf Identificador usado para acessar a variável transformation
 *               dentro do shader vertex
 * @param p Array de primitivas que devem ser passadas para a memória de video
 * @param count Quantidade de elementos neste array
 */
static void prepare(Parameters *params, GLint *transf, Primitive *p, uint count)
{
	GLuint programa;
	int i;
	
	programa = installShaders(params->vertex, params->fragment);
	runProgram(programa);
	
	*transf = glGetUniformLocation(programa, "transformation");
	
	for (i = 0; i < count; ++i)
		loadVertexBuffer(&p[i]);
}

int main(int argc, char *argv[])
{
	GLFWwindow *window = NULL;
	int result = EXIT_SUCCESS;
	Parameters params;
	Primitive *p;
	GLint transformation;
	mat4x4 scale = { {0.5f, 0, 0, 0.3},
			 {0, 0.5f, 0, 0.4},
			 {0, 0, 1, 0},
			 {0, 0, 0, 1}
			};
	Faces *f;
	mat4x4 matrix;

	glfwInit();
	window = initWindow(argc, argv, &params);
	
	// Os valores defaults para os buffer estão oks. Se quiser verificá-los
	// Dê uma olhada em http://www.glfw.org/docs/latest/window.html#window_hints

	glfwMakeContextCurrent(window);
	
	glewExperimental = GL_TRUE;
	if ( glewInit() != GLEW_OK ) {
		printf("Erro na inicialização da biblioteca GLEW\n");
		return -1;
	}
	
	// Inicializa o OpenGL
	initOpenGL();
	
/////////////////////////////////////////////////////////////////////////
	
	// Exemplo de criação de primitivas mais complexas
	const uint vigaH = 1;
	const uint prism = 0;
	
	const uint vigaH_centro = 0;
	const uint vigaH_esquerda = 1;
	const uint vigaH_direita = 2;

	p = createPrimitive(2);
	
	mat4x4 *tmp;
	
	// Inicializando o prisma
	setPrimitiveBuffer(p, prism, 2, prismVertex, sizeof(prismVertex));
	tmp = getPrimitiveTransformation(p, prism, 2);
	mat4x4_scale_aniso(matrix, *tmp, .3, .3, .3);
	mat4x4_translate_in_place(matrix, 0.f, 1.2f, 0.f);
	setPrimitiveTransformation(p, prism, 2, matrix);

	initPrimitiveFaceArray(p, prism, 2, 1);
	f = getPrimitiveFaceElement(p, prism, 2, 0);
	initFace(f);
	setFace(f, prismElem, 18);
	
	// Inicializando a viga H
	setPrimitiveBuffer(p, vigaH, 2, cubeVertex, sizeof(cubeVertex));
	initPrimitiveFaceArray(p, vigaH, 2, 3);
	mat4x4_scale_aniso(matrix, *getPrimitiveTransformation(p, vigaH, 2), .3, .3, .3);
	setPrimitiveTransformation(p, vigaH, 2, matrix);
	
	f = getPrimitiveFaceElement(p, vigaH, 2, vigaH_centro);
	initFace(f);
	setFace(f, cubeElem, 36);
	mat4x4_scale_aniso(matrix, f->transf, 2.f, 0.2, 2.f);
	setFaceTransformation(f, matrix);
	
	f = getPrimitiveFaceElement(p, vigaH, 2, vigaH_direita);
	initFace(f);
	setFace(f, cubeElem, 36);
	mat4x4_scale_aniso(matrix, f->transf, 0.2f, 2.f, 2.f);
	mat4x4_translate_in_place(matrix, 10.f, 0.f, 0.f);
	setFaceTransformation(f, matrix);
	
	f = getPrimitiveFaceElement(p, vigaH, 2, vigaH_esquerda);
	initFace(f);
	setFace(f, cubeElem, 36);
	mat4x4_scale_aniso(matrix, f->transf, 0.2f, 2.f, 2.f);
	mat4x4_translate_in_place(matrix, -10.f, 0.f, 0.f);
	setFaceTransformation(f, matrix);
	
/////////////////////////////////////////////////////////////////////////
	
	prepare(&params, &transformation, p, 2);
	
	// Entra em loop até receber um comando de termino
	while (!glfwWindowShouldClose(window))
	{
		update(p, 2);
		render(transformation, p, 2);

		// Troca os buffers
		glfwSwapBuffers(window);

		// Processa mais eventos
		glfwPollEvents();
	}

	destroyPrimitive(p, 1);
	glfwTerminate();
	return result;
}