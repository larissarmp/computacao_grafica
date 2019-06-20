#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"

static const size_t PAGE = 1024;

/**
 * Permite criar shaders que estão na memória
 * 
 * @param shaderType O tipo do shader usado
 * @param lines Quantidade de linhas que definem o shader
 * @param source Endereço da memória onde está definido o shader
 * @return Idendificador do shader para o objeto OpenGL. Valor 0 significa 
 *         que não houve sucesso na operação
 */
GLuint loadAndCompileShaderFromMemory(GLenum shaderType, GLsizei lines, const GLchar **source)
{
	GLuint shader = 0;
	GLint params = 0; // auxiliar nas chamadas de informação do OpenGL

	printf("Compilando o objeto do tipo \"");
	if ( GL_VERTEX_SHADER == shaderType )
		printf("Vertex shader");
	else
		printf("Fragment shader");
	printf("\"\n");
	
	shader = glCreateShader(shaderType);
	
	// carrega o código fonte para o driver openGL
	glShaderSource(shader, lines, source, NULL);
	
	// Compila o fonte
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
	if ( GL_FALSE == params ) {
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		char *log = malloc((logLength + 1)*sizeof(*log));
		glGetShaderInfoLog(shader, logLength, NULL, log);
		log[logLength] = '\0';
		printf("Output error: %s", log);
		free(log);
		shader = 0;
		goto end;
	}
	
	printf("Sucesso na compilação\n");
end:
	return shader;
}

/**
 * Carrega um shader que está em disco e o coloca na memória principal
 * 
 * @param shaderType O tipo do shader usado
 * @param name O nome e caminho do shader que está no sistema de arquivo
 * @return Idendificador do shader para o objeto OpenGL. Valor 0 significa 
 *         que não houve sucesso na operação
 */
GLuint loadAndCompileShaderFromFile(GLenum shaderType, const char *name)
{
	GLuint shader = 0; // valor retornado. Default: teve erro
	int i = 0;
	
	FILE *fp = NULL;
	ssize_t read = 0; // quantidade de bytes lido do arquivo
	size_t bufferLen = 0; // necessário para a função getline
	
	int line = 0; // número de linhas do arquivo fonte
	char **source = NULL; // conteúdo de cada linha
	size_t sourceLen = line / PAGE + 1; // usado para administrar o tamanho do buffer

	printf("Lendo o arquivo: %s\n", name);
	
	source  = calloc(sourceLen*PAGE, sizeof(char*));
	
	// Carrega o arquivo fonte na memória
	fp = fopen(name, "r");
	if ( NULL == fp ) {
		printf("Incapaz de abrir ou ler o arquivo shader %s\n", name);
		goto clearMemory;
	}
	while ( (read = getline(&source[line], &bufferLen, fp)) != -1 ) {
		++line;
		bufferLen = 0;
		if ( (line / PAGE + 1) > sourceLen ) {
			++sourceLen;
			char **tmp = calloc(sourceLen*PAGE, sizeof(char*));
			if ( NULL == tmp ) {
				printf("Memória insuficiente\n");
				goto closeFile;
			}
			for (i = 0; i < line; ++i)
				tmp[i] = source[i];
			free(source);
			source = tmp;
		}
	}
	
	shader = loadAndCompileShaderFromMemory(shaderType, line, (const GLchar **) source);

closeFile:
	fclose(fp);
clearMemory:
	for (i = 0; i < line; ++i)
		free(source[i]);
	free(source);
	
	return shader;
}

/**
 * Instala os shaders em um novo programa, no driver de vídeo
 * 
 * Instalar os shaders, compila e associa a um novo programa. Sinaliza para 
 * o driver de vídeo que eles podem ser apagador. Assim, na remoção do programa,
 * eles são automaticamente apagados.
 * 
 * @param vertex Nome do shader vertex, no sistema de arquivo
 * @param fragment Nome do shader fragment, no sistema de arquivo
 * @return Identificador do programa que foi instalado. Retorna 0 se não for
 *         sucedido
 */
GLuint installShaders(const char *vertex, const char *fragment)
{
	GLuint program = 0;
	GLuint vtx, frag;
	
	program = glCreateProgram();
	
	vtx = loadAndCompileShaderFromFile(GL_VERTEX_SHADER, vertex);
	if ( 0 == vtx ) {
		printf("Erro no carregamento do shader vertex: %s\n", vertex);
		goto deleteProgram;
	}
	glAttachShader(program, vtx);
	
	frag = loadAndCompileShaderFromFile(GL_FRAGMENT_SHADER, fragment);
	if ( 0 == frag ) {
		printf("Erro no carregamento do shader fragment: %s\n", fragment);
		goto deleteVertex;
	}
	glAttachShader(program, frag);
	
	// Depois que foi anexado, pode-se apagar os shaders. Somente serão
	// efetivados após a destruição do programa
	glDeleteShader(vtx);
	glDeleteShader(frag);
	return program;
	
deleteVertex:
	glDeleteShader(vtx);
deleteProgram:
	glDeleteProgram(program);
	exit(EXIT_FAILURE);
}

/**
 * Executa o programa
 * 
 * @param program Identificador do programa que deve ser ativado
 */
void runProgram(GLuint program)
{
	GLint params = 0;
	
	glLinkProgram(program);
	
	glGetProgramiv(program, GL_LINK_STATUS, &params);
	if ( GL_FALSE == params ) {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char *log = malloc((logLength + 1)*sizeof(*log));
		glGetProgramInfoLog(program, logLength, NULL, log);
		log[logLength] = '\0';
		printf("Output error: %s", log);
		free(log);
		goto deleteProgram;
	}
	
	glUseProgram(program);
	printf("Programa criado com sucesso\n");
	return;
	
deleteProgram:
	glDeleteProgram(program);
	exit(EXIT_FAILURE);
}
